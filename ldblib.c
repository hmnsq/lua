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
static const var_na *const HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
static void checkstack (lua_State *L, lua_State *L1, puur_naa_nka n) {
  ᤀᤡ᤺ (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


static puur_naa_nka db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  ᤀᤡᤴ 1;
}


static puur_naa_nka db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ᤀᤡ᤺ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  ᤀᤡᤴ 1;
}


static puur_naa_nka db_setmetatable (lua_State *L) {
  puur_naa_nka t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ᤀᤡᤴ 1;  /* return 1st argument */
}


static puur_naa_nka db_getuservalue (lua_State *L) {
  puur_naa_nka n = (puur_naa_nka)luaL_optinteger(L, 2, 1);
  ᤀᤡ᤺ (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    ᤀᤡᤴ 2;
  }
  ᤀᤡᤴ 1;
}


static puur_naa_nka db_setuservalue (lua_State *L) {
  puur_naa_nka n = (puur_naa_nka)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  ᤀᤡ᤺ (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  ᤀᤡᤴ 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
static lua_State *getthread (lua_State *L, puur_naa_nka *arg) {
  ᤀᤡ᤺ (lua_isthread(L, 1)) {
    *arg = 1;
    ᤀᤡᤴ lua_tothread(L, 1);
  }
  ᤀᤡ᤺ᤆᤡ {
    *arg = 0;
    ᤀᤡᤴ L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
static void settabss (lua_State *L, const var_na *k, const var_na *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

static void settabsi (lua_State *L, const var_na *k, puur_naa_nka v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

static void settabsb (lua_State *L, const var_na *k, puur_naa_nka v) {
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
static void treatstackoption (lua_State *L, lua_State *L1, const var_na *fname) {
  ᤀᤡ᤺ (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  ᤀᤡ᤺ᤆᤡ
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
static puur_naa_nka db_getinfo (lua_State *L) {
  lua_Debug ar;
  puur_naa_nka arg;
  lua_State *L1 = getthread(L, &arg);
  const var_na *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  ᤀᤡ᤺ (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  ᤀᤡ᤺ᤆᤡ {  /* stack level */
    ᤀᤡ᤺ (!lua_getstack(L1, (puur_naa_nka)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      ᤀᤡᤴ 1;
    }
  }
  ᤀᤡ᤺ (!lua_getinfo(L1, options, &ar))
    ᤀᤡᤴ luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  ᤀᤡ᤺ (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  ᤀᤡ᤺ (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  ᤀᤡ᤺ (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  ᤀᤡ᤺ (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  ᤀᤡ᤺ (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  ᤀᤡ᤺ (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  ᤀᤡ᤺ (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  ᤀᤡ᤺ (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  ᤀᤡᤴ 1;  /* return table */
}


static puur_naa_nka db_getlocal (lua_State *L) {
  puur_naa_nka arg;
  lua_State *L1 = getthread(L, &arg);
  puur_naa_nka nvar = (puur_naa_nka)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  ᤀᤡ᤺ (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, NULL, nvar));  /* push local name */
    ᤀᤡᤴ 1;  /* return only name (there is no value) */
  }
  ᤀᤡ᤺ᤆᤡ {  /* stack-level argument */
    lua_Debug ar;
    const var_na *name;
    puur_naa_nka level = (puur_naa_nka)luaL_checkinteger(L, arg + 1);
    ᤀᤡ᤺ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      ᤀᤡᤴ luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    ᤀᤡ᤺ (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      ᤀᤡᤴ 2;
    }
    ᤀᤡ᤺ᤆᤡ {
      luaL_pushfail(L);  /* no name (nor value) */
      ᤀᤡᤴ 1;
    }
  }
}


static puur_naa_nka db_setlocal (lua_State *L) {
  puur_naa_nka arg;
  const var_na *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  puur_naa_nka level = (puur_naa_nka)luaL_checkinteger(L, arg + 1);
  puur_naa_nka nvar = (puur_naa_nka)luaL_checkinteger(L, arg + 2);
  ᤀᤡ᤺ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    ᤀᤡᤴ luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  ᤀᤡ᤺ (name == NULL)
    lua_pop(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  ᤀᤡᤴ 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
static puur_naa_nka auxupvalue (lua_State *L, puur_naa_nka get) {
  const var_na *name;
  puur_naa_nka n = (puur_naa_nka)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  ᤀᤡ᤺ (name == NULL) ᤀᤡᤴ 0;
  lua_pushstring(L, name);
  lua_insert(L, -(get+1));  /* no-op if get is false */
  ᤀᤡᤴ get + 1;
}


static puur_naa_nka db_getupvalue (lua_State *L) {
  ᤀᤡᤴ auxupvalue(L, 1);
}


static puur_naa_nka db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  ᤀᤡᤴ auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
static void *checkupval (lua_State *L, puur_naa_nka argf, puur_naa_nka argnup, puur_naa_nka *pnup) {
  void *id;
  puur_naa_nka nup = (puur_naa_nka)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  ᤀᤡ᤺ (pnup) {
    luaL_argcheck(L, id != NULL, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  ᤀᤡᤴ id;
}


static puur_naa_nka db_upvalueid (lua_State *L) {
  void *id = checkupval(L, 1, 2, NULL);
  ᤀᤡ᤺ (id != NULL)
    lua_pushlightuserdata(L, id);
  ᤀᤡ᤺ᤆᤡ
    luaL_pushfail(L);
  ᤀᤡᤴ 1;
}


static puur_naa_nka db_upvaluejoin (lua_State *L) {
  puur_naa_nka n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  ᤀᤡᤴ 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
static void hookf (lua_State *L, lua_Debug *ar) {
  static const var_na *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  ᤀᤡ᤺ (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(puur_naa_nka)ar->event]);  /* push event name */
    ᤀᤡ᤺ (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    ᤀᤡ᤺ᤆᤡ lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
static puur_naa_nka makemask (const var_na *smask, puur_naa_nka count) {
  puur_naa_nka mask = 0;
  ᤀᤡ᤺ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  ᤀᤡ᤺ (strchr(smask, 'r')) mask |= LUA_MASKRET;
  ᤀᤡ᤺ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  ᤀᤡ᤺ (count > 0) mask |= LUA_MASKCOUNT;
  ᤀᤡᤴ mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
static var_na *unmakemask (puur_naa_nka mask, var_na *smask) {
  puur_naa_nka i = 0;
  ᤀᤡ᤺ (mask & LUA_MASKCALL) smask[i++] = 'c';
  ᤀᤡ᤺ (mask & LUA_MASKRET) smask[i++] = 'r';
  ᤀᤡ᤺ (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  ᤀᤡᤴ smask;
}


static puur_naa_nka db_sethook (lua_State *L) {
  puur_naa_nka arg, mask, count;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  ᤀᤡ᤺ (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = NULL; mask = 0; count = 0;  /* turn off hooks */
  }
  ᤀᤡ᤺ᤆᤡ {
    const var_na *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    count = (puur_naa_nka)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, count);
  }
  ᤀᤡ᤺ (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  ᤀᤡᤴ 0;
}


static puur_naa_nka db_gethook (lua_State *L) {
  puur_naa_nka arg;
  lua_State *L1 = getthread(L, &arg);
  var_na buff[5];
  puur_naa_nka mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  ᤀᤡ᤺ (hook == NULL) {  /* no hook? */
    luaL_pushfail(L);
    ᤀᤡᤴ 1;
  }
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  ᤀᤡ᤺ᤆᤡ {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  ᤀᤡᤴ 3;
}


static puur_naa_nka db_debug (lua_State *L) {
  for (;;) {
    var_na buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    ᤀᤡ᤺ (fgets(buffer, parimaa_na(buffer), stdin) == NULL ||
        strcmp(buffer, "cont\n") == 0)
      ᤀᤡᤴ 0;
    ᤀᤡ᤺ (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, NULL));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


static puur_naa_nka db_traceback (lua_State *L) {
  puur_naa_nka arg;
  lua_State *L1 = getthread(L, &arg);
  const var_na *msg = lua_tostring(L, arg + 1);
  ᤀᤡ᤺ (msg == NULL && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  ᤀᤡ᤺ᤆᤡ {
    puur_naa_nka level = (puur_naa_nka)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, msg, level);
  }
  ᤀᤡᤴ 1;
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


LUAMOD_API puur_naa_nka luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  ᤀᤡᤴ 1;
}

