/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#include "lprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
static const char *const HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
static void checkstack (lua_State *L, lua_State *L1, цяло n) {
  ако (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


static цяло db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  върни 1;
}


static цяло db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ако (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  върни 1;
}


static цяло db_setmetatable (lua_State *L) {
  цяло t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  върни 1;  /* return 1st argument */
}


static цяло db_getuservalue (lua_State *L) {
  цяло n = (цяло)luaL_optinteger(L, 2, 1);
  ако (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  иначе ако (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    върни 2;
  }
  върни 1;
}


static цяло db_setuservalue (lua_State *L) {
  цяло n = (цяло)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  ако (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  върни 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
static lua_State *getthread (lua_State *L, цяло *arg) {
  ако (lua_isthread(L, 1)) {
    *arg = 1;
    върни lua_tothread(L, 1);
  }
  иначе {
    *arg = 0;
    върни L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
static void settabss (lua_State *L, const char *k, const char *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

static void settabsi (lua_State *L, const char *k, цяло v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

static void settabsb (lua_State *L, const char *k, цяло v) {
  lua_pushboolean(L, v);
  lua_setfield(L, -2, k);
}


/*
** In function 'db_getinfo', the call to 'lua_getinfo' may push
** results on the stack; later it creates the result table to put
** these objects. Function 'treatstackoption' puts the result from
** 'lua_getinfo' on top of the result table so that it can call
** 'lua_setfield'.
*/
static void treatstackoption (lua_State *L, lua_State *L1, const char *fname) {
  ако (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  иначе
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
static цяло db_getinfo (lua_State *L) {
  lua_Debug ar;
  цяло arg;
  lua_State *L1 = getthread(L, &arg);
  const char *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  ако (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  иначе {  /* stack level */
    ако (!lua_getstack(L1, (цяло)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      върни 1;
    }
  }
  ако (!lua_getinfo(L1, options, &ar))
    върни luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  ако (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  ако (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  ако (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  ако (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  ако (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  ако (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  ако (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  ако (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  върни 1;  /* return table */
}


static цяло db_getlocal (lua_State *L) {
  цяло arg;
  lua_State *L1 = getthread(L, &arg);
  цяло nvar = (цяло)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  ако (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, NULL, nvar));  /* push local name */
    върни 1;  /* return only name (there is no value) */
  }
  иначе {  /* stack-level argument */
    lua_Debug ar;
    const char *name;
    цяло level = (цяло)luaL_checkinteger(L, arg + 1);
    ако (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      върни luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    ако (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      върни 2;
    }
    иначе {
      luaL_pushfail(L);  /* no name (nor value) */
      върни 1;
    }
  }
}


static цяло db_setlocal (lua_State *L) {
  цяло arg;
  const char *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  цяло level = (цяло)luaL_checkinteger(L, arg + 1);
  цяло nvar = (цяло)luaL_checkinteger(L, arg + 2);
  ако (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    върни luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  ако (name == NULL)
    lua_pop(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  върни 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
static цяло auxupvalue (lua_State *L, цяло get) {
  const char *name;
  цяло n = (цяло)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  ако (name == NULL) върни 0;
  lua_pushstring(L, name);
  lua_insert(L, -(get+1));  /* no-op if get is false */
  върни get + 1;
}


static цяло db_getupvalue (lua_State *L) {
  върни auxupvalue(L, 1);
}


static цяло db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  върни auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
static void *checkupval (lua_State *L, цяло argf, цяло argnup, цяло *pnup) {
  void *id;
  цяло nup = (цяло)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  ако (pnup) {
    luaL_argcheck(L, id != NULL, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  върни id;
}


static цяло db_upvalueid (lua_State *L) {
  void *id = checkupval(L, 1, 2, NULL);
  ако (id != NULL)
    lua_pushlightuserdata(L, id);
  иначе
    luaL_pushfail(L);
  върни 1;
}


static цяло db_upvaluejoin (lua_State *L) {
  цяло n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  върни 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
static void hookf (lua_State *L, lua_Debug *ar) {
  static const char *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  ако (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(цяло)ar->event]);  /* push event name */
    ако (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    иначе lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
static цяло makemask (const char *smask, цяло count) {
  цяло mask = 0;
  ако (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  ако (strchr(smask, 'r')) mask |= LUA_MASKRET;
  ако (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  ако (count > 0) mask |= LUA_MASKCOUNT;
  върни mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
static char *unmakemask (цяло mask, char *smask) {
  цяло i = 0;
  ако (mask & LUA_MASKCALL) smask[i++] = 'c';
  ако (mask & LUA_MASKRET) smask[i++] = 'r';
  ако (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  върни smask;
}


static цяло db_sethook (lua_State *L) {
  цяло arg, mask, count;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  ако (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = NULL; mask = 0; count = 0;  /* turn off hooks */
  }
  иначе {
    const char *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    count = (цяло)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, count);
  }
  ако (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
    /* table just created; initialize it */
    lua_pushliteral(L, "k");
    lua_setfield(L, -2, "__mode");  /** hooktable.__mode = "k" */
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);  /* metatable(hooktable) = hooktable */
  }
  checkstack(L, L1, 1);
  lua_pushthread(L1); lua_xmove(L1, L, 1);  /* key (thread) */
  lua_pushvalue(L, arg + 1);  /* value (hook function) */
  lua_rawset(L, -3);  /* hooktable[L1] = new Lua hook */
  lua_sethook(L1, func, mask, count);
  върни 0;
}


static цяло db_gethook (lua_State *L) {
  цяло arg;
  lua_State *L1 = getthread(L, &arg);
  char buff[5];
  цяло mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  ако (hook == NULL) {  /* no hook? */
    luaL_pushfail(L);
    върни 1;
  }
  иначе ако (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  иначе {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  върни 3;
}


static цяло db_debug (lua_State *L) {
  за (;;) {
    char buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    ако (fgets(buffer, sizeof(buffer), stdin) == NULL ||
        strcmp(buffer, "cont\n") == 0)
      върни 0;
    ако (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, NULL));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


static цяло db_traceback (lua_State *L) {
  цяло arg;
  lua_State *L1 = getthread(L, &arg);
  const char *msg = lua_tostring(L, arg + 1);
  ако (msg == NULL && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  иначе {
    цяло level = (цяло)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, msg, level);
  }
  върни 1;
}


static const luaL_Reg dblib[] = {
  {"debug", db_debug},
  {"getuservalue", db_getuservalue},
  {"gethook", db_gethook},
  {"getinfo", db_getinfo},
  {"getlocal", db_getlocal},
  {"getregistry", db_getregistry},
  {"getmetatable", db_getmetatable},
  {"getupvalue", db_getupvalue},
  {"upvaluejoin", db_upvaluejoin},
  {"upvalueid", db_upvalueid},
  {"setuservalue", db_setuservalue},
  {"sethook", db_sethook},
  {"setlocal", db_setlocal},
  {"setmetatable", db_setmetatable},
  {"setupvalue", db_setupvalue},
  {"traceback", db_traceback},
  {NULL, NULL}
};


LUAMOD_API цяло luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  върни 1;
}

