/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
স্থির ধ্রুবক অক্ষর *ধ্রুবক HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
স্থির শূন্য checkstack (lua_State *L, lua_State *L1, পূর্ণসংখ্যা n) {
  যদি (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


স্থির পূর্ণসংখ্যা db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  যদি (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা db_setmetatable (lua_State *L) {
  পূর্ণসংখ্যা t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ফেরত 1;  /* return 1st argument */
}


স্থির পূর্ণসংখ্যা db_getuservalue (lua_State *L) {
  পূর্ণসংখ্যা n = (পূর্ণসংখ্যা)luaL_optinteger(L, 2, 1);
  যদি (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  অন্যথা যদি (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    ফেরত 2;
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা db_setuservalue (lua_State *L) {
  পূর্ণসংখ্যা n = (পূর্ণসংখ্যা)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  যদি (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  ফেরত 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
স্থির lua_State *getthread (lua_State *L, পূর্ণসংখ্যা *arg) {
  যদি (lua_isthread(L, 1)) {
    *arg = 1;
    ফেরত lua_tothread(L, 1);
  }
  অন্যথা {
    *arg = 0;
    ফেরত L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
স্থির শূন্য settabss (lua_State *L, ধ্রুবক অক্ষর *k, ধ্রুবক অক্ষর *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

স্থির শূন্য settabsi (lua_State *L, ধ্রুবক অক্ষর *k, পূর্ণসংখ্যা v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

স্থির শূন্য settabsb (lua_State *L, ধ্রুবক অক্ষর *k, পূর্ণসংখ্যা v) {
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
স্থির শূন্য treatstackoption (lua_State *L, lua_State *L1, ধ্রুবক অক্ষর *fname) {
  যদি (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  অন্যথা
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
স্থির পূর্ণসংখ্যা db_getinfo (lua_State *L) {
  lua_Debug ar;
  পূর্ণসংখ্যা arg;
  lua_State *L1 = getthread(L, &arg);
  ধ্রুবক অক্ষর *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  যদি (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  অন্যথা {  /* stack level */
    যদি (!lua_getstack(L1, (পূর্ণসংখ্যা)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      ফেরত 1;
    }
  }
  যদি (!lua_getinfo(L1, options, &ar))
    ফেরত luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  যদি (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  যদি (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  যদি (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  যদি (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  যদি (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  যদি (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  যদি (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  যদি (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  ফেরত 1;  /* return table */
}


স্থির পূর্ণসংখ্যা db_getlocal (lua_State *L) {
  পূর্ণসংখ্যা arg;
  lua_State *L1 = getthread(L, &arg);
  পূর্ণসংখ্যা nvar = (পূর্ণসংখ্যা)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  যদি (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, ফাঁকা, nvar));  /* push local name */
    ফেরত 1;  /* return only name (there is no value) */
  }
  অন্যথা {  /* stack-level argument */
    lua_Debug ar;
    ধ্রুবক অক্ষর *name;
    পূর্ণসংখ্যা level = (পূর্ণসংখ্যা)luaL_checkinteger(L, arg + 1);
    যদি (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      ফেরত luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    যদি (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      ফেরত 2;
    }
    অন্যথা {
      luaL_pushfail(L);  /* no name (nor value) */
      ফেরত 1;
    }
  }
}


স্থির পূর্ণসংখ্যা db_setlocal (lua_State *L) {
  পূর্ণসংখ্যা arg;
  ধ্রুবক অক্ষর *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  পূর্ণসংখ্যা level = (পূর্ণসংখ্যা)luaL_checkinteger(L, arg + 1);
  পূর্ণসংখ্যা nvar = (পূর্ণসংখ্যা)luaL_checkinteger(L, arg + 2);
  যদি (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    ফেরত luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  যদি (name == ফাঁকা)
    lua_বের_করো(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  ফেরত 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
স্থির পূর্ণসংখ্যা auxupvalue (lua_State *L, পূর্ণসংখ্যা get) {
  ধ্রুবক অক্ষর *name;
  পূর্ণসংখ্যা n = (পূর্ণসংখ্যা)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  যদি (name == ফাঁকা) ফেরত 0;
  lua_pushstring(L, name);
  lua_ঢোকাও(L, -(get+1));  /* no-op if get is false */
  ফেরত get + 1;
}


স্থির পূর্ণসংখ্যা db_getupvalue (lua_State *L) {
  ফেরত auxupvalue(L, 1);
}


স্থির পূর্ণসংখ্যা db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  ফেরত auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
স্থির শূন্য *checkupval (lua_State *L, পূর্ণসংখ্যা argf, পূর্ণসংখ্যা argnup, পূর্ণসংখ্যা *pnup) {
  শূন্য *id;
  পূর্ণসংখ্যা nup = (পূর্ণসংখ্যা)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  যদি (pnup) {
    luaL_argcheck(L, id != ফাঁকা, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  ফেরত id;
}


স্থির পূর্ণসংখ্যা db_upvalueid (lua_State *L) {
  শূন্য *id = checkupval(L, 1, 2, ফাঁকা);
  যদি (id != ফাঁকা)
    lua_pushlightuserdata(L, id);
  অন্যথা
    luaL_pushfail(L);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা db_upvaluejoin (lua_State *L) {
  পূর্ণসংখ্যা n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  ফেরত 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
স্থির শূন্য hookf (lua_State *L, lua_Debug *ar) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  যদি (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(পূর্ণসংখ্যা)ar->event]);  /* push event name */
    যদি (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    অন্যথা lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
স্থির পূর্ণসংখ্যা makemask (ধ্রুবক অক্ষর *smask, পূর্ণসংখ্যা গুনতি) {
  পূর্ণসংখ্যা mask = 0;
  যদি (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  যদি (strchr(smask, 'r')) mask |= LUA_MASKRET;
  যদি (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  যদি (গুনতি > 0) mask |= LUA_MASKCOUNT;
  ফেরত mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
স্থির অক্ষর *unmakemask (পূর্ণসংখ্যা mask, অক্ষর *smask) {
  পূর্ণসংখ্যা i = 0;
  যদি (mask & LUA_MASKCALL) smask[i++] = 'c';
  যদি (mask & LUA_MASKRET) smask[i++] = 'r';
  যদি (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  ফেরত smask;
}


স্থির পূর্ণসংখ্যা db_sethook (lua_State *L) {
  পূর্ণসংখ্যা arg, mask, গুনতি;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  যদি (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = ফাঁকা; mask = 0; গুনতি = 0;  /* turn off hooks */
  }
  অন্যথা {
    ধ্রুবক অক্ষর *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    গুনতি = (পূর্ণসংখ্যা)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, গুনতি);
  }
  যদি (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, গুনতি);
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা db_gethook (lua_State *L) {
  পূর্ণসংখ্যা arg;
  lua_State *L1 = getthread(L, &arg);
  অক্ষর buff[5];
  পূর্ণসংখ্যা mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  যদি (hook == ফাঁকা) {  /* no hook? */
    luaL_pushfail(L);
    ফেরত 1;
  }
  অন্যথা যদি (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  অন্যথা {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  ফেরত 3;
}


স্থির পূর্ণসংখ্যা db_debug (lua_State *L) {
  জন্য (;;) {
    অক্ষর বাফার[250];
    lua_writestringerror("%s", "lua_debug> ");
    যদি (fgets(বাফার, আকার(বাফার), stdin) == ফাঁকা ||
        strcmp(বাফার, "cont\n") == 0)
      ফেরত 0;
    যদি (luaL_loadbufferx(L, বাফার, strlen(বাফার), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, ফাঁকা));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


স্থির পূর্ণসংখ্যা db_traceback (lua_State *L) {
  পূর্ণসংখ্যা arg;
  lua_State *L1 = getthread(L, &arg);
  ধ্রুবক অক্ষর *বার্তা = lua_tostring(L, arg + 1);
  যদি (বার্তা == ফাঁকা && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  অন্যথা {
    পূর্ণসংখ্যা level = (পূর্ণসংখ্যা)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, বার্তা, level);
  }
  ফেরত 1;
}


স্থির ধ্রুবক luaL_Reg dblib[] = {
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
  {ফাঁকা, ফাঁকা}
};


LUAMOD_API পূর্ণসংখ্যা luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  ফেরত 1;
}

