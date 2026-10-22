/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
സ്ഥിതം ശൂന്യം checkstack (lua_State *L, lua_State *L1, പൂർണ്ണസംഖ്യ n) {
  എങ്കിൽ (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  എങ്കിൽ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_setmetatable (lua_State *L) {
  പൂർണ്ണസംഖ്യ t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  തിരികെ 1;  /* return 1st argument */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_getuservalue (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = (പൂർണ്ണസംഖ്യ)luaL_optinteger(L, 2, 1);
  എങ്കിൽ (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  അല്ലെങ്കിൽ എങ്കിൽ (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    തിരികെ 2;
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_setuservalue (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = (പൂർണ്ണസംഖ്യ)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  എങ്കിൽ (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  തിരികെ 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
സ്ഥിതം lua_State *getthread (lua_State *L, പൂർണ്ണസംഖ്യ *arg) {
  എങ്കിൽ (lua_isthread(L, 1)) {
    *arg = 1;
    തിരികെ lua_tothread(L, 1);
  }
  അല്ലെങ്കിൽ {
    *arg = 0;
    തിരികെ L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
സ്ഥിതം ശൂന്യം settabss (lua_State *L, സ്ഥിരം അക്ഷരം *k, സ്ഥിരം അക്ഷരം *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

സ്ഥിതം ശൂന്യം settabsi (lua_State *L, സ്ഥിരം അക്ഷരം *k, പൂർണ്ണസംഖ്യ v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

സ്ഥിതം ശൂന്യം settabsb (lua_State *L, സ്ഥിരം അക്ഷരം *k, പൂർണ്ണസംഖ്യ v) {
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
സ്ഥിതം ശൂന്യം treatstackoption (lua_State *L, lua_State *L1, സ്ഥിരം അക്ഷരം *fname) {
  എങ്കിൽ (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  അല്ലെങ്കിൽ
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ db_getinfo (lua_State *L) {
  lua_Debug ar;
  പൂർണ്ണസംഖ്യ arg;
  lua_State *L1 = getthread(L, &arg);
  സ്ഥിരം അക്ഷരം *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  എങ്കിൽ (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  അല്ലെങ്കിൽ {  /* stack level */
    എങ്കിൽ (!lua_getstack(L1, (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      തിരികെ 1;
    }
  }
  എങ്കിൽ (!lua_getinfo(L1, options, &ar))
    തിരികെ luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  എങ്കിൽ (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  എങ്കിൽ (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  എങ്കിൽ (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  എങ്കിൽ (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  എങ്കിൽ (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  എങ്കിൽ (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  എങ്കിൽ (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  എങ്കിൽ (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  തിരികെ 1;  /* return table */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_getlocal (lua_State *L) {
  പൂർണ്ണസംഖ്യ arg;
  lua_State *L1 = getthread(L, &arg);
  പൂർണ്ണസംഖ്യ nvar = (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  എങ്കിൽ (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, ശൂന്യ, nvar));  /* push local name */
    തിരികെ 1;  /* return only name (there is no value) */
  }
  അല്ലെങ്കിൽ {  /* stack-level argument */
    lua_Debug ar;
    സ്ഥിരം അക്ഷരം *name;
    പൂർണ്ണസംഖ്യ level = (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, arg + 1);
    എങ്കിൽ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      തിരികെ luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    എങ്കിൽ (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      തിരികെ 2;
    }
    അല്ലെങ്കിൽ {
      luaL_pushfail(L);  /* no name (nor value) */
      തിരികെ 1;
    }
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_setlocal (lua_State *L) {
  പൂർണ്ണസംഖ്യ arg;
  സ്ഥിരം അക്ഷരം *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  പൂർണ്ണസംഖ്യ level = (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, arg + 1);
  പൂർണ്ണസംഖ്യ nvar = (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, arg + 2);
  എങ്കിൽ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    തിരികെ luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  എങ്കിൽ (name == ശൂന്യ)
    lua_എടുക്കുക(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  തിരികെ 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ auxupvalue (lua_State *L, പൂർണ്ണസംഖ്യ get) {
  സ്ഥിരം അക്ഷരം *name;
  പൂർണ്ണസംഖ്യ n = (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  എങ്കിൽ (name == ശൂന്യ) തിരികെ 0;
  lua_pushstring(L, name);
  lua_തിരുകുക(L, -(get+1));  /* no-op if get is false */
  തിരികെ get + 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_getupvalue (lua_State *L) {
  തിരികെ auxupvalue(L, 1);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  തിരികെ auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
സ്ഥിതം ശൂന്യം *checkupval (lua_State *L, പൂർണ്ണസംഖ്യ argf, പൂർണ്ണസംഖ്യ argnup, പൂർണ്ണസംഖ്യ *pnup) {
  ശൂന്യം *id;
  പൂർണ്ണസംഖ്യ nup = (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  എങ്കിൽ (pnup) {
    luaL_argcheck(L, id != ശൂന്യ, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  തിരികെ id;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_upvalueid (lua_State *L) {
  ശൂന്യം *id = checkupval(L, 1, 2, ശൂന്യ);
  എങ്കിൽ (id != ശൂന്യ)
    lua_pushlightuserdata(L, id);
  അല്ലെങ്കിൽ
    luaL_pushfail(L);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_upvaluejoin (lua_State *L) {
  പൂർണ്ണസംഖ്യ n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  തിരികെ 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
സ്ഥിതം ശൂന്യം hookf (lua_State *L, lua_Debug *ar) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  എങ്കിൽ (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(പൂർണ്ണസംഖ്യ)ar->event]);  /* push event name */
    എങ്കിൽ (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    അല്ലെങ്കിൽ lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ makemask (സ്ഥിരം അക്ഷരം *smask, പൂർണ്ണസംഖ്യ എണ്ണം) {
  പൂർണ്ണസംഖ്യ mask = 0;
  എങ്കിൽ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  എങ്കിൽ (strchr(smask, 'r')) mask |= LUA_MASKRET;
  എങ്കിൽ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  എങ്കിൽ (എണ്ണം > 0) mask |= LUA_MASKCOUNT;
  തിരികെ mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
സ്ഥിതം അക്ഷരം *unmakemask (പൂർണ്ണസംഖ്യ mask, അക്ഷരം *smask) {
  പൂർണ്ണസംഖ്യ i = 0;
  എങ്കിൽ (mask & LUA_MASKCALL) smask[i++] = 'c';
  എങ്കിൽ (mask & LUA_MASKRET) smask[i++] = 'r';
  എങ്കിൽ (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  തിരികെ smask;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_sethook (lua_State *L) {
  പൂർണ്ണസംഖ്യ arg, mask, എണ്ണം;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  എങ്കിൽ (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = ശൂന്യ; mask = 0; എണ്ണം = 0;  /* turn off hooks */
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    എണ്ണം = (പൂർണ്ണസംഖ്യ)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, എണ്ണം);
  }
  എങ്കിൽ (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, എണ്ണം);
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_gethook (lua_State *L) {
  പൂർണ്ണസംഖ്യ arg;
  lua_State *L1 = getthread(L, &arg);
  അക്ഷരം buff[5];
  പൂർണ്ണസംഖ്യ mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  എങ്കിൽ (hook == ശൂന്യ) {  /* no hook? */
    luaL_pushfail(L);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  അല്ലെങ്കിൽ {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  തിരികെ 3;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_debug (lua_State *L) {
  വേണ്ടി (;;) {
    അക്ഷരം buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    എങ്കിൽ (fgets(buffer, വലുപ്പം(buffer), stdin) == ശൂന്യ ||
        strcmp(buffer, "cont\n") == 0)
      തിരികെ 0;
    എങ്കിൽ (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, ശൂന്യ));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ db_traceback (lua_State *L) {
  പൂർണ്ണസംഖ്യ arg;
  lua_State *L1 = getthread(L, &arg);
  സ്ഥിരം അക്ഷരം *സന്ദേശം = lua_tostring(L, arg + 1);
  എങ്കിൽ (സന്ദേശം == ശൂന്യ && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ level = (പൂർണ്ണസംഖ്യ)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, സന്ദേശം, level);
  }
  തിരികെ 1;
}


സ്ഥിതം സ്ഥിരം luaL_Reg dblib[] = {
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
  {ശൂന്യ, ശൂന്യ}
};


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  തിരികെ 1;
}

