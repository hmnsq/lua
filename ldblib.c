/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
நிலையான மாறிலி எழுத்து *மாறிலி HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
நிலையான வெறுமை checkstack (lua_State *L, lua_State *L1, முழு n) {
  எனில் (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


நிலையான முழு db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  திரும்பு 1;
}


நிலையான முழு db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  எனில் (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  திரும்பு 1;
}


நிலையான முழு db_setmetatable (lua_State *L) {
  முழு t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  திரும்பு 1;  /* return 1st argument */
}


நிலையான முழு db_getuservalue (lua_State *L) {
  முழு n = (முழு)luaL_optinteger(L, 2, 1);
  எனில் (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  இல்லையெனில் எனில் (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    திரும்பு 2;
  }
  திரும்பு 1;
}


நிலையான முழு db_setuservalue (lua_State *L) {
  முழு n = (முழு)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  எனில் (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  திரும்பு 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
நிலையான lua_State *getthread (lua_State *L, முழு *arg) {
  எனில் (lua_isthread(L, 1)) {
    *arg = 1;
    திரும்பு lua_tothread(L, 1);
  }
  இல்லையெனில் {
    *arg = 0;
    திரும்பு L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
நிலையான வெறுமை settabss (lua_State *L, மாறிலி எழுத்து *k, மாறிலி எழுத்து *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

நிலையான வெறுமை settabsi (lua_State *L, மாறிலி எழுத்து *k, முழு v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

நிலையான வெறுமை settabsb (lua_State *L, மாறிலி எழுத்து *k, முழு v) {
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
நிலையான வெறுமை treatstackoption (lua_State *L, lua_State *L1, மாறிலி எழுத்து *fname) {
  எனில் (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  இல்லையெனில்
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
நிலையான முழு db_getinfo (lua_State *L) {
  lua_Debug ar;
  முழு arg;
  lua_State *L1 = getthread(L, &arg);
  மாறிலி எழுத்து *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  எனில் (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  இல்லையெனில் {  /* stack level */
    எனில் (!lua_getstack(L1, (முழு)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      திரும்பு 1;
    }
  }
  எனில் (!lua_getinfo(L1, options, &ar))
    திரும்பு luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  எனில் (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  எனில் (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  எனில் (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  எனில் (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  எனில் (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  எனில் (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  எனில் (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  எனில் (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  திரும்பு 1;  /* return table */
}


நிலையான முழு db_getlocal (lua_State *L) {
  முழு arg;
  lua_State *L1 = getthread(L, &arg);
  முழு nvar = (முழு)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  எனில் (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, வெற்று, nvar));  /* push local name */
    திரும்பு 1;  /* return only name (there is no value) */
  }
  இல்லையெனில் {  /* stack-level argument */
    lua_Debug ar;
    மாறிலி எழுத்து *name;
    முழு level = (முழு)luaL_checkinteger(L, arg + 1);
    எனில் (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      திரும்பு luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    எனில் (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      திரும்பு 2;
    }
    இல்லையெனில் {
      luaL_pushfail(L);  /* no name (nor value) */
      திரும்பு 1;
    }
  }
}


நிலையான முழு db_setlocal (lua_State *L) {
  முழு arg;
  மாறிலி எழுத்து *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  முழு level = (முழு)luaL_checkinteger(L, arg + 1);
  முழு nvar = (முழு)luaL_checkinteger(L, arg + 2);
  எனில் (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    திரும்பு luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  எனில் (name == வெற்று)
    lua_எடு(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  திரும்பு 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
நிலையான முழு auxupvalue (lua_State *L, முழு get) {
  மாறிலி எழுத்து *name;
  முழு n = (முழு)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  எனில் (name == வெற்று) திரும்பு 0;
  lua_pushstring(L, name);
  lua_செருகு(L, -(get+1));  /* no-op if get is false */
  திரும்பு get + 1;
}


நிலையான முழு db_getupvalue (lua_State *L) {
  திரும்பு auxupvalue(L, 1);
}


நிலையான முழு db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  திரும்பு auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
நிலையான வெறுமை *checkupval (lua_State *L, முழு argf, முழு argnup, முழு *pnup) {
  வெறுமை *id;
  முழு nup = (முழு)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  எனில் (pnup) {
    luaL_argcheck(L, id != வெற்று, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  திரும்பு id;
}


நிலையான முழு db_upvalueid (lua_State *L) {
  வெறுமை *id = checkupval(L, 1, 2, வெற்று);
  எனில் (id != வெற்று)
    lua_pushlightuserdata(L, id);
  இல்லையெனில்
    luaL_pushfail(L);
  திரும்பு 1;
}


நிலையான முழு db_upvaluejoin (lua_State *L) {
  முழு n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  திரும்பு 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
நிலையான வெறுமை hookf (lua_State *L, lua_Debug *ar) {
  நிலையான மாறிலி எழுத்து *மாறிலி hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  எனில் (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(முழு)ar->event]);  /* push event name */
    எனில் (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    இல்லையெனில் lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
நிலையான முழு makemask (மாறிலி எழுத்து *smask, முழு மொத்தம்) {
  முழு mask = 0;
  எனில் (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  எனில் (strchr(smask, 'r')) mask |= LUA_MASKRET;
  எனில் (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  எனில் (மொத்தம் > 0) mask |= LUA_MASKCOUNT;
  திரும்பு mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
நிலையான எழுத்து *unmakemask (முழு mask, எழுத்து *smask) {
  முழு i = 0;
  எனில் (mask & LUA_MASKCALL) smask[i++] = 'c';
  எனில் (mask & LUA_MASKRET) smask[i++] = 'r';
  எனில் (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  திரும்பு smask;
}


நிலையான முழு db_sethook (lua_State *L) {
  முழு arg, mask, மொத்தம்;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  எனில் (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = வெற்று; mask = 0; மொத்தம் = 0;  /* turn off hooks */
  }
  இல்லையெனில் {
    மாறிலி எழுத்து *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    மொத்தம் = (முழு)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, மொத்தம்);
  }
  எனில் (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, மொத்தம்);
  திரும்பு 0;
}


நிலையான முழு db_gethook (lua_State *L) {
  முழு arg;
  lua_State *L1 = getthread(L, &arg);
  எழுத்து buff[5];
  முழு mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  எனில் (hook == வெற்று) {  /* no hook? */
    luaL_pushfail(L);
    திரும்பு 1;
  }
  இல்லையெனில் எனில் (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  இல்லையெனில் {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  திரும்பு 3;
}


நிலையான முழு db_debug (lua_State *L) {
  க்கு (;;) {
    எழுத்து buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    எனில் (fgets(buffer, அளவு(buffer), stdin) == வெற்று ||
        strcmp(buffer, "cont\n") == 0)
      திரும்பு 0;
    எனில் (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, வெற்று));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


நிலையான முழு db_traceback (lua_State *L) {
  முழு arg;
  lua_State *L1 = getthread(L, &arg);
  மாறிலி எழுத்து *செய்தி = lua_tostring(L, arg + 1);
  எனில் (செய்தி == வெற்று && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  இல்லையெனில் {
    முழு level = (முழு)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, செய்தி, level);
  }
  திரும்பு 1;
}


நிலையான மாறிலி luaL_Reg dblib[] = {
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
  {வெற்று, வெற்று}
};


LUAMOD_API முழு luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  திரும்பு 1;
}

