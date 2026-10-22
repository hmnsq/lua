/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#شامل "lprefix.h"


#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
ایستا ثابت نویسه *ثابت HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
ایستا تهی checkstack (lua_State *L, lua_State *L1, صحیح n) {
  اگر (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


ایستا صحیح db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  بازگشت 1;
}


ایستا صحیح db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  اگر (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  بازگشت 1;
}


ایستا صحیح db_setmetatable (lua_State *L) {
  صحیح t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  بازگشت 1;  /* return 1st argument */
}


ایستا صحیح db_getuservalue (lua_State *L) {
  صحیح n = (صحیح)luaL_optinteger(L, 2, 1);
  اگر (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  وگرنه اگر (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    بازگشت 2;
  }
  بازگشت 1;
}


ایستا صحیح db_setuservalue (lua_State *L) {
  صحیح n = (صحیح)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  اگر (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  بازگشت 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
ایستا lua_State *getthread (lua_State *L, صحیح *arg) {
  اگر (lua_isthread(L, 1)) {
    *arg = 1;
    بازگشت lua_tothread(L, 1);
  }
  وگرنه {
    *arg = 0;
    بازگشت L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
ایستا تهی settabss (lua_State *L, ثابت نویسه *k, ثابت نویسه *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

ایستا تهی settabsi (lua_State *L, ثابت نویسه *k, صحیح v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

ایستا تهی settabsb (lua_State *L, ثابت نویسه *k, صحیح v) {
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
ایستا تهی treatstackoption (lua_State *L, lua_State *L1, ثابت نویسه *fname) {
  اگر (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  وگرنه
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
ایستا صحیح db_getinfo (lua_State *L) {
  lua_Debug ar;
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  ثابت نویسه *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  اگر (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  وگرنه {  /* stack level */
    اگر (!lua_getstack(L1, (صحیح)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      بازگشت 1;
    }
  }
  اگر (!lua_getinfo(L1, options, &ar))
    بازگشت luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  اگر (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  اگر (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  اگر (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  اگر (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  اگر (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  اگر (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  اگر (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  اگر (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  بازگشت 1;  /* return table */
}


ایستا صحیح db_getlocal (lua_State *L) {
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  صحیح nvar = (صحیح)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  اگر (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, پوچ, nvar));  /* push local name */
    بازگشت 1;  /* return only name (there is no value) */
  }
  وگرنه {  /* stack-level argument */
    lua_Debug ar;
    ثابت نویسه *name;
    صحیح level = (صحیح)luaL_checkinteger(L, arg + 1);
    اگر (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      بازگشت luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    اگر (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      بازگشت 2;
    }
    وگرنه {
      luaL_pushfail(L);  /* no name (nor value) */
      بازگشت 1;
    }
  }
}


ایستا صحیح db_setlocal (lua_State *L) {
  صحیح arg;
  ثابت نویسه *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  صحیح level = (صحیح)luaL_checkinteger(L, arg + 1);
  صحیح nvar = (صحیح)luaL_checkinteger(L, arg + 2);
  اگر (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    بازگشت luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  اگر (name == پوچ)
    lua_برداشت(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  بازگشت 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
ایستا صحیح auxupvalue (lua_State *L, صحیح get) {
  ثابت نویسه *name;
  صحیح n = (صحیح)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  اگر (name == پوچ) بازگشت 0;
  lua_pushstring(L, name);
  lua_درج(L, -(get+1));  /* no-op if get is false */
  بازگشت get + 1;
}


ایستا صحیح db_getupvalue (lua_State *L) {
  بازگشت auxupvalue(L, 1);
}


ایستا صحیح db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  بازگشت auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
ایستا تهی *checkupval (lua_State *L, صحیح argf, صحیح argnup, صحیح *pnup) {
  تهی *id;
  صحیح nup = (صحیح)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  اگر (pnup) {
    luaL_argcheck(L, id != پوچ, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  بازگشت id;
}


ایستا صحیح db_upvalueid (lua_State *L) {
  تهی *id = checkupval(L, 1, 2, پوچ);
  اگر (id != پوچ)
    lua_pushlightuserdata(L, id);
  وگرنه
    luaL_pushfail(L);
  بازگشت 1;
}


ایستا صحیح db_upvaluejoin (lua_State *L) {
  صحیح n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  بازگشت 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
ایستا تهی hookf (lua_State *L, lua_Debug *ar) {
  ایستا ثابت نویسه *ثابت hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  اگر (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(صحیح)ar->event]);  /* push event name */
    اگر (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    وگرنه lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
ایستا صحیح makemask (ثابت نویسه *smask, صحیح شمار) {
  صحیح mask = 0;
  اگر (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  اگر (strchr(smask, 'r')) mask |= LUA_MASKRET;
  اگر (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  اگر (شمار > 0) mask |= LUA_MASKCOUNT;
  بازگشت mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
ایستا نویسه *unmakemask (صحیح mask, نویسه *smask) {
  صحیح i = 0;
  اگر (mask & LUA_MASKCALL) smask[i++] = 'c';
  اگر (mask & LUA_MASKRET) smask[i++] = 'r';
  اگر (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  بازگشت smask;
}


ایستا صحیح db_sethook (lua_State *L) {
  صحیح arg, mask, شمار;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  اگر (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = پوچ; mask = 0; شمار = 0;  /* turn off hooks */
  }
  وگرنه {
    ثابت نویسه *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    شمار = (صحیح)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, شمار);
  }
  اگر (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, شمار);
  بازگشت 0;
}


ایستا صحیح db_gethook (lua_State *L) {
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  نویسه buff[5];
  صحیح mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  اگر (hook == پوچ) {  /* no hook? */
    luaL_pushfail(L);
    بازگشت 1;
  }
  وگرنه اگر (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  وگرنه {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  بازگشت 3;
}


ایستا صحیح db_debug (lua_State *L) {
  برای (;;) {
    نویسه buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    اگر (fgets(buffer, اندازه(buffer), stdin) == پوچ ||
        strcmp(buffer, "cont\n") == 0)
      بازگشت 0;
    اگر (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, پوچ));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


ایستا صحیح db_traceback (lua_State *L) {
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  ثابت نویسه *پیام = lua_tostring(L, arg + 1);
  اگر (پیام == پوچ && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  وگرنه {
    صحیح level = (صحیح)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, پیام, level);
  }
  بازگشت 1;
}


ایستا ثابت luaL_Reg dblib[] = {
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
  {پوچ, پوچ}
};


LUAMOD_API صحیح luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  بازگشت 1;
}

