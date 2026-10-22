/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#ضمن "lprefix.h"


#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
ساكن ثابت حرف *ثابت HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
ساكن فراغ checkstack (lua_State *L, lua_State *L1, صحيح n) {
  إذا (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


ساكن صحيح db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  إرجاع 1;
}


ساكن صحيح db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  إذا (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  إرجاع 1;
}


ساكن صحيح db_setmetatable (lua_State *L) {
  صحيح t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  إرجاع 1;  /* return 1st argument */
}


ساكن صحيح db_getuservalue (lua_State *L) {
  صحيح n = (صحيح)luaL_optinteger(L, 2, 1);
  إذا (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  وإلا إذا (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    إرجاع 2;
  }
  إرجاع 1;
}


ساكن صحيح db_setuservalue (lua_State *L) {
  صحيح n = (صحيح)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  إذا (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  إرجاع 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
ساكن lua_State *getthread (lua_State *L, صحيح *arg) {
  إذا (lua_isthread(L, 1)) {
    *arg = 1;
    إرجاع lua_tothread(L, 1);
  }
  وإلا {
    *arg = 0;
    إرجاع L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
ساكن فراغ settabss (lua_State *L, ثابت حرف *k, ثابت حرف *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

ساكن فراغ settabsi (lua_State *L, ثابت حرف *k, صحيح v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

ساكن فراغ settabsb (lua_State *L, ثابت حرف *k, صحيح v) {
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
ساكن فراغ treatstackoption (lua_State *L, lua_State *L1, ثابت حرف *fname) {
  إذا (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  وإلا
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
ساكن صحيح db_getinfo (lua_State *L) {
  lua_Debug ar;
  صحيح arg;
  lua_State *L1 = getthread(L, &arg);
  ثابت حرف *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  إذا (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  وإلا {  /* stack level */
    إذا (!lua_getstack(L1, (صحيح)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      إرجاع 1;
    }
  }
  إذا (!lua_getinfo(L1, options, &ar))
    إرجاع luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  إذا (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  إذا (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  إذا (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  إذا (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  إذا (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  إذا (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  إذا (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  إذا (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  إرجاع 1;  /* return table */
}


ساكن صحيح db_getlocal (lua_State *L) {
  صحيح arg;
  lua_State *L1 = getthread(L, &arg);
  صحيح nvar = (صحيح)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  إذا (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, عدم, nvar));  /* push local name */
    إرجاع 1;  /* return only name (there is no value) */
  }
  وإلا {  /* stack-level argument */
    lua_Debug ar;
    ثابت حرف *name;
    صحيح level = (صحيح)luaL_checkinteger(L, arg + 1);
    إذا (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      إرجاع luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    إذا (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      إرجاع 2;
    }
    وإلا {
      luaL_pushfail(L);  /* no name (nor value) */
      إرجاع 1;
    }
  }
}


ساكن صحيح db_setlocal (lua_State *L) {
  صحيح arg;
  ثابت حرف *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  صحيح level = (صحيح)luaL_checkinteger(L, arg + 1);
  صحيح nvar = (صحيح)luaL_checkinteger(L, arg + 2);
  إذا (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    إرجاع luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  إذا (name == عدم)
    lua_اسحب(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  إرجاع 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
ساكن صحيح auxupvalue (lua_State *L, صحيح get) {
  ثابت حرف *name;
  صحيح n = (صحيح)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  إذا (name == عدم) إرجاع 0;
  lua_pushstring(L, name);
  lua_أدرج(L, -(get+1));  /* no-op if get is false */
  إرجاع get + 1;
}


ساكن صحيح db_getupvalue (lua_State *L) {
  إرجاع auxupvalue(L, 1);
}


ساكن صحيح db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  إرجاع auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
ساكن فراغ *checkupval (lua_State *L, صحيح argf, صحيح argnup, صحيح *pnup) {
  فراغ *id;
  صحيح nup = (صحيح)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  إذا (pnup) {
    luaL_argcheck(L, id != عدم, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  إرجاع id;
}


ساكن صحيح db_upvalueid (lua_State *L) {
  فراغ *id = checkupval(L, 1, 2, عدم);
  إذا (id != عدم)
    lua_pushlightuserdata(L, id);
  وإلا
    luaL_pushfail(L);
  إرجاع 1;
}


ساكن صحيح db_upvaluejoin (lua_State *L) {
  صحيح n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  إرجاع 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
ساكن فراغ hookf (lua_State *L, lua_Debug *ar) {
  ساكن ثابت حرف *ثابت hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  إذا (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(صحيح)ar->event]);  /* push event name */
    إذا (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    وإلا lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
ساكن صحيح makemask (ثابت حرف *smask, صحيح تعداد_2) {
  صحيح mask = 0;
  إذا (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  إذا (strchr(smask, 'r')) mask |= LUA_MASKRET;
  إذا (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  إذا (تعداد_2 > 0) mask |= LUA_MASKCOUNT;
  إرجاع mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
ساكن حرف *unmakemask (صحيح mask, حرف *smask) {
  صحيح i = 0;
  إذا (mask & LUA_MASKCALL) smask[i++] = 'c';
  إذا (mask & LUA_MASKRET) smask[i++] = 'r';
  إذا (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  إرجاع smask;
}


ساكن صحيح db_sethook (lua_State *L) {
  صحيح arg, mask, تعداد_2;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  إذا (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = عدم; mask = 0; تعداد_2 = 0;  /* turn off hooks */
  }
  وإلا {
    ثابت حرف *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    تعداد_2 = (صحيح)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, تعداد_2);
  }
  إذا (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, تعداد_2);
  إرجاع 0;
}


ساكن صحيح db_gethook (lua_State *L) {
  صحيح arg;
  lua_State *L1 = getthread(L, &arg);
  حرف buff[5];
  صحيح mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  إذا (hook == عدم) {  /* no hook? */
    luaL_pushfail(L);
    إرجاع 1;
  }
  وإلا إذا (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  وإلا {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  إرجاع 3;
}


ساكن صحيح db_debug (lua_State *L) {
  لكل (;;) {
    حرف buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    إذا (fgets(buffer, حجم(buffer), stdin) == عدم ||
        strcmp(buffer, "cont\n") == 0)
      إرجاع 0;
    إذا (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, عدم));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


ساكن صحيح db_traceback (lua_State *L) {
  صحيح arg;
  lua_State *L1 = getthread(L, &arg);
  ثابت حرف *رسالة = lua_tostring(L, arg + 1);
  إذا (رسالة == عدم && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  وإلا {
    صحيح level = (صحيح)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, رسالة, level);
  }
  إرجاع 1;
}


ساكن ثابت luaL_Reg dblib[] = {
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
  {عدم, عدم}
};


LUAMOD_API صحيح luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  إرجاع 1;
}

