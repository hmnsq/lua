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
static void checkstack (lua_State *L, lua_State *L1, 정수 n) {
  만약 (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


static 정수 db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  반환 1;
}


static 정수 db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  만약 (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  반환 1;
}


static 정수 db_setmetatable (lua_State *L) {
  정수 t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  반환 1;  /* return 1st argument */
}


static 정수 db_getuservalue (lua_State *L) {
  정수 n = (정수)luaL_optinteger(L, 2, 1);
  만약 (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  아니면 만약 (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    반환 2;
  }
  반환 1;
}


static 정수 db_setuservalue (lua_State *L) {
  정수 n = (정수)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  만약 (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  반환 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
static lua_State *getthread (lua_State *L, 정수 *arg) {
  만약 (lua_isthread(L, 1)) {
    *arg = 1;
    반환 lua_tothread(L, 1);
  }
  아니면 {
    *arg = 0;
    반환 L;  /* function will operate over current thread */
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

static void settabsi (lua_State *L, const char *k, 정수 v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

static void settabsb (lua_State *L, const char *k, 정수 v) {
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
  만약 (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  아니면
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
static 정수 db_getinfo (lua_State *L) {
  lua_Debug ar;
  정수 arg;
  lua_State *L1 = getthread(L, &arg);
  const char *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  만약 (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  아니면 {  /* stack level */
    만약 (!lua_getstack(L1, (정수)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      반환 1;
    }
  }
  만약 (!lua_getinfo(L1, options, &ar))
    반환 luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  만약 (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  만약 (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  만약 (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  만약 (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  만약 (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  만약 (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  만약 (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  만약 (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  반환 1;  /* return table */
}


static 정수 db_getlocal (lua_State *L) {
  정수 arg;
  lua_State *L1 = getthread(L, &arg);
  정수 nvar = (정수)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  만약 (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, NULL, nvar));  /* push local name */
    반환 1;  /* return only name (there is no value) */
  }
  아니면 {  /* stack-level argument */
    lua_Debug ar;
    const char *name;
    정수 level = (정수)luaL_checkinteger(L, arg + 1);
    만약 (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      반환 luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    만약 (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      반환 2;
    }
    아니면 {
      luaL_pushfail(L);  /* no name (nor value) */
      반환 1;
    }
  }
}


static 정수 db_setlocal (lua_State *L) {
  정수 arg;
  const char *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  정수 level = (정수)luaL_checkinteger(L, arg + 1);
  정수 nvar = (정수)luaL_checkinteger(L, arg + 2);
  만약 (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    반환 luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  만약 (name == NULL)
    lua_pop(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  반환 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
static 정수 auxupvalue (lua_State *L, 정수 get) {
  const char *name;
  정수 n = (정수)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  만약 (name == NULL) 반환 0;
  lua_pushstring(L, name);
  lua_insert(L, -(get+1));  /* no-op if get is false */
  반환 get + 1;
}


static 정수 db_getupvalue (lua_State *L) {
  반환 auxupvalue(L, 1);
}


static 정수 db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  반환 auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
static void *checkupval (lua_State *L, 정수 argf, 정수 argnup, 정수 *pnup) {
  void *id;
  정수 nup = (정수)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  만약 (pnup) {
    luaL_argcheck(L, id != NULL, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  반환 id;
}


static 정수 db_upvalueid (lua_State *L) {
  void *id = checkupval(L, 1, 2, NULL);
  만약 (id != NULL)
    lua_pushlightuserdata(L, id);
  아니면
    luaL_pushfail(L);
  반환 1;
}


static 정수 db_upvaluejoin (lua_State *L) {
  정수 n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  반환 0;
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
  만약 (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(정수)ar->event]);  /* push event name */
    만약 (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    아니면 lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
static 정수 makemask (const char *smask, 정수 count) {
  정수 mask = 0;
  만약 (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  만약 (strchr(smask, 'r')) mask |= LUA_MASKRET;
  만약 (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  만약 (count > 0) mask |= LUA_MASKCOUNT;
  반환 mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
static char *unmakemask (정수 mask, char *smask) {
  정수 i = 0;
  만약 (mask & LUA_MASKCALL) smask[i++] = 'c';
  만약 (mask & LUA_MASKRET) smask[i++] = 'r';
  만약 (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  반환 smask;
}


static 정수 db_sethook (lua_State *L) {
  정수 arg, mask, count;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  만약 (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = NULL; mask = 0; count = 0;  /* turn off hooks */
  }
  아니면 {
    const char *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    count = (정수)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, count);
  }
  만약 (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  반환 0;
}


static 정수 db_gethook (lua_State *L) {
  정수 arg;
  lua_State *L1 = getthread(L, &arg);
  char buff[5];
  정수 mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  만약 (hook == NULL) {  /* no hook? */
    luaL_pushfail(L);
    반환 1;
  }
  아니면 만약 (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  아니면 {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  반환 3;
}


static 정수 db_debug (lua_State *L) {
  위해 (;;) {
    char buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    만약 (fgets(buffer, sizeof(buffer), stdin) == NULL ||
        strcmp(buffer, "cont\n") == 0)
      반환 0;
    만약 (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, NULL));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


static 정수 db_traceback (lua_State *L) {
  정수 arg;
  lua_State *L1 = getthread(L, &arg);
  const char *msg = lua_tostring(L, arg + 1);
  만약 (msg == NULL && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  아니면 {
    정수 level = (정수)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, msg, level);
  }
  반환 1;
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


LUAMOD_API 정수 luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  반환 1;
}

