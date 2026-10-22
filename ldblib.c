/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#تعریف_کرو ldblib_c
#تعریف_کرو LUA_LIB

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
ساکن ثابت حرف *ثابت HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
ساکن خالی checkstack (lua_State *L, lua_State *L1, صحیح n) {
  اگر (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


ساکن صحیح db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  واپس 1;
}


ساکن صحیح db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  اگر (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  واپس 1;
}


ساکن صحیح db_setmetatable (lua_State *L) {
  صحیح t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  واپس 1;  /* return 1st argument */
}


ساکن صحیح db_getuservalue (lua_State *L) {
  صحیح n = (صحیح)luaL_optinteger(L, 2, 1);
  اگر (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  ورنہ اگر (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    واپس 2;
  }
  واپس 1;
}


ساکن صحیح db_setuservalue (lua_State *L) {
  صحیح n = (صحیح)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  اگر (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  واپس 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
ساکن lua_State *getthread (lua_State *L, صحیح *arg) {
  اگر (lua_isthread(L, 1)) {
    *arg = 1;
    واپس lua_tothread(L, 1);
  }
  ورنہ {
    *arg = 0;
    واپس L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
ساکن خالی settabss (lua_State *L, ثابت حرف *k, ثابت حرف *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

ساکن خالی settabsi (lua_State *L, ثابت حرف *k, صحیح v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

ساکن خالی settabsb (lua_State *L, ثابت حرف *k, صحیح v) {
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
ساکن خالی treatstackoption (lua_State *L, lua_State *L1, ثابت حرف *fname) {
  اگر (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  ورنہ
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
ساکن صحیح db_getinfo (lua_State *L) {
  lua_Debug ar;
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  ثابت حرف *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  اگر (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  ورنہ {  /* stack level */
    اگر (!lua_getstack(L1, (صحیح)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      واپس 1;
    }
  }
  اگر (!lua_getinfo(L1, options, &ar))
    واپس luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  اگر (strchr(options, 'S')) {
    lua_pushlstring(L, ar.ماخذ, ar.srclen);
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
  واپس 1;  /* return table */
}


ساکن صحیح db_getlocal (lua_State *L) {
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  صحیح nvar = (صحیح)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  اگر (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, تہی, nvar));  /* push local name */
    واپس 1;  /* return only name (there is no value) */
  }
  ورنہ {  /* stack-level argument */
    lua_Debug ar;
    ثابت حرف *name;
    صحیح level = (صحیح)luaL_checkinteger(L, arg + 1);
    اگر (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      واپس luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    اگر (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      واپس 2;
    }
    ورنہ {
      luaL_pushfail(L);  /* no name (nor value) */
      واپس 1;
    }
  }
}


ساکن صحیح db_setlocal (lua_State *L) {
  صحیح arg;
  ثابت حرف *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  صحیح level = (صحیح)luaL_checkinteger(L, arg + 1);
  صحیح nvar = (صحیح)luaL_checkinteger(L, arg + 2);
  اگر (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    واپس luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  اگر (name == تہی)
    lua_نکالو(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  واپس 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
ساکن صحیح auxupvalue (lua_State *L, صحیح get) {
  ثابت حرف *name;
  صحیح n = (صحیح)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /*احاطہ*/
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  اگر (name == تہی) واپس 0;
  lua_pushstring(L, name);
  lua_داخل_کرو(L, -(get+1));  /* no-op if get is false */
  واپس get + 1;
}


ساکن صحیح db_getupvalue (lua_State *L) {
  واپس auxupvalue(L, 1);
}


ساکن صحیح db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  واپس auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
ساکن خالی *checkupval (lua_State *L, صحیح argf, صحیح argnup, صحیح *pnup) {
  خالی *id;
  صحیح nup = (صحیح)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /*احاطہ*/
  id = lua_upvalueid(L, argf, nup);
  اگر (pnup) {
    luaL_argcheck(L, id != تہی, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  واپس id;
}


ساکن صحیح db_upvalueid (lua_State *L) {
  خالی *id = checkupval(L, 1, 2, تہی);
  اگر (id != تہی)
    lua_pushlightuserdata(L, id);
  ورنہ
    luaL_pushfail(L);
  واپس 1;
}


ساکن صحیح db_upvaluejoin (lua_State *L) {
  صحیح n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  واپس 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
ساکن خالی hookf (lua_State *L, lua_Debug *ar) {
  ساکن ثابت حرف *ثابت hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  اگر (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(صحیح)ar->event]);  /* push event name */
    اگر (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    ورنہ lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
ساکن صحیح makemask (ثابت حرف *smask, صحیح گنتی) {
  صحیح نقاب_2 = 0;
  اگر (strchr(smask, 'c')) نقاب_2 |= LUA_MASKCALL;
  اگر (strchr(smask, 'r')) نقاب_2 |= LUA_MASKRET;
  اگر (strchr(smask, 'l')) نقاب_2 |= LUA_MASKLINE;
  اگر (گنتی > 0) نقاب_2 |= LUA_MASKCOUNT;
  واپس نقاب_2;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
ساکن حرف *unmakemask (صحیح نقاب_2, حرف *smask) {
  صحیح i = 0;
  اگر (نقاب_2 & LUA_MASKCALL) smask[i++] = 'c';
  اگر (نقاب_2 & LUA_MASKRET) smask[i++] = 'r';
  اگر (نقاب_2 & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  واپس smask;
}


ساکن صحیح db_sethook (lua_State *L) {
  صحیح arg, نقاب_2, گنتی;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  اگر (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = تہی; نقاب_2 = 0; گنتی = 0;  /* turn off hooks */
  }
  ورنہ {
    ثابت حرف *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    گنتی = (صحیح)luaL_optinteger(L, arg + 3, 0);
    func = hookf; نقاب_2 = makemask(smask, گنتی);
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
  lua_sethook(L1, func, نقاب_2, گنتی);
  واپس 0;
}


ساکن صحیح db_gethook (lua_State *L) {
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  حرف buff[5];
  صحیح نقاب_2 = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  اگر (hook == تہی) {  /* no hook? */
    luaL_pushfail(L);
    واپس 1;
  }
  ورنہ اگر (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  ورنہ {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(نقاب_2, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  واپس 3;
}


ساکن صحیح db_debug (lua_State *L) {
  برائے (;;) {
    حرف بفر[250];
    lua_writestringerror("%s", "lua_debug> ");
    اگر (fgets(بفر, حجم(بفر), stdin) == تہی ||
        strcmp(بفر, "cont\n") == 0)
      واپس 0;
    اگر (luaL_loadbufferx(L, بفر, لڑی_لمبائی(بفر), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, تہی));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


ساکن صحیح db_traceback (lua_State *L) {
  صحیح arg;
  lua_State *L1 = getthread(L, &arg);
  ثابت حرف *پیغام = lua_tostring(L, arg + 1);
  اگر (پیغام == تہی && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  ورنہ {
    صحیح level = (صحیح)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, پیغام, level);
  }
  واپس 1;
}


ساکن ثابت luaL_Reg dblib[] = {
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
  {تہی, تہی}
};


LUAMOD_API صحیح luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  واپس 1;
}

