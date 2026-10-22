/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#నిర్వచించు ldblib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
స్థిర శూన్య checkstack (lua_State *L, lua_State *L1, పూర్ణాంకం n) {
  ఒకవేళ (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


స్థిర పూర్ణాంకం db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ఒకవేళ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం db_setmetatable (lua_State *L) {
  పూర్ణాంకం t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  తిరిగి 1;  /* return 1st argument */
}


స్థిర పూర్ణాంకం db_getuservalue (lua_State *L) {
  పూర్ణాంకం n = (పూర్ణాంకం)luaL_optinteger(L, 2, 1);
  ఒకవేళ (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  లేకపోతే ఒకవేళ (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    తిరిగి 2;
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం db_setuservalue (lua_State *L) {
  పూర్ణాంకం n = (పూర్ణాంకం)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  ఒకవేళ (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  తిరిగి 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
స్థిర lua_State *getthread (lua_State *L, పూర్ణాంకం *arg) {
  ఒకవేళ (lua_isthread(L, 1)) {
    *arg = 1;
    తిరిగి lua_tothread(L, 1);
  }
  లేకపోతే {
    *arg = 0;
    తిరిగి L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
స్థిర శూన్య settabss (lua_State *L, స్థిరరాశి అక్షరం *k, స్థిరరాశి అక్షరం *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

స్థిర శూన్య settabsi (lua_State *L, స్థిరరాశి అక్షరం *k, పూర్ణాంకం v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

స్థిర శూన్య settabsb (lua_State *L, స్థిరరాశి అక్షరం *k, పూర్ణాంకం v) {
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
స్థిర శూన్య treatstackoption (lua_State *L, lua_State *L1, స్థిరరాశి అక్షరం *fname) {
  ఒకవేళ (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  లేకపోతే
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
స్థిర పూర్ణాంకం db_getinfo (lua_State *L) {
  lua_Debug ar;
  పూర్ణాంకం arg;
  lua_State *L1 = getthread(L, &arg);
  స్థిరరాశి అక్షరం *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  ఒకవేళ (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  లేకపోతే {  /* stack level */
    ఒకవేళ (!lua_getstack(L1, (పూర్ణాంకం)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      తిరిగి 1;
    }
  }
  ఒకవేళ (!lua_getinfo(L1, options, &ar))
    తిరిగి luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  ఒకవేళ (strchr(options, 'S')) {
    lua_pushlstring(L, ar.ఆధారం, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  ఒకవేళ (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  ఒకవేళ (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  ఒకవేళ (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  ఒకవేళ (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  ఒకవేళ (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  ఒకవేళ (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  ఒకవేళ (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  తిరిగి 1;  /* return table */
}


స్థిర పూర్ణాంకం db_getlocal (lua_State *L) {
  పూర్ణాంకం arg;
  lua_State *L1 = getthread(L, &arg);
  పూర్ణాంకం nvar = (పూర్ణాంకం)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  ఒకవేళ (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, ఖాళీ, nvar));  /* push local name */
    తిరిగి 1;  /* return only name (there is no value) */
  }
  లేకపోతే {  /* stack-level argument */
    lua_Debug ar;
    స్థిరరాశి అక్షరం *name;
    పూర్ణాంకం level = (పూర్ణాంకం)luaL_checkinteger(L, arg + 1);
    ఒకవేళ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      తిరిగి luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    ఒకవేళ (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      తిరిగి 2;
    }
    లేకపోతే {
      luaL_pushfail(L);  /* no name (nor value) */
      తిరిగి 1;
    }
  }
}


స్థిర పూర్ణాంకం db_setlocal (lua_State *L) {
  పూర్ణాంకం arg;
  స్థిరరాశి అక్షరం *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  పూర్ణాంకం level = (పూర్ణాంకం)luaL_checkinteger(L, arg + 1);
  పూర్ణాంకం nvar = (పూర్ణాంకం)luaL_checkinteger(L, arg + 2);
  ఒకవేళ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    తిరిగి luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  ఒకవేళ (name == ఖాళీ)
    lua_తీయి(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  తిరిగి 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
స్థిర పూర్ణాంకం auxupvalue (lua_State *L, పూర్ణాంకం get) {
  స్థిరరాశి అక్షరం *name;
  పూర్ణాంకం n = (పూర్ణాంకం)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /*సంవృతం*/
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  ఒకవేళ (name == ఖాళీ) తిరిగి 0;
  lua_pushstring(L, name);
  lua_చొప్పించు(L, -(get+1));  /* no-op if get is false */
  తిరిగి get + 1;
}


స్థిర పూర్ణాంకం db_getupvalue (lua_State *L) {
  తిరిగి auxupvalue(L, 1);
}


స్థిర పూర్ణాంకం db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  తిరిగి auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
స్థిర శూన్య *checkupval (lua_State *L, పూర్ణాంకం argf, పూర్ణాంకం argnup, పూర్ణాంకం *pnup) {
  శూన్య *id;
  పూర్ణాంకం nup = (పూర్ణాంకం)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /*సంవృతం*/
  id = lua_upvalueid(L, argf, nup);
  ఒకవేళ (pnup) {
    luaL_argcheck(L, id != ఖాళీ, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  తిరిగి id;
}


స్థిర పూర్ణాంకం db_upvalueid (lua_State *L) {
  శూన్య *id = checkupval(L, 1, 2, ఖాళీ);
  ఒకవేళ (id != ఖాళీ)
    lua_pushlightuserdata(L, id);
  లేకపోతే
    luaL_pushfail(L);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం db_upvaluejoin (lua_State *L) {
  పూర్ణాంకం n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  తిరిగి 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
స్థిర శూన్య hookf (lua_State *L, lua_Debug *ar) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  ఒకవేళ (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(పూర్ణాంకం)ar->event]);  /* push event name */
    ఒకవేళ (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    లేకపోతే lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
స్థిర పూర్ణాంకం makemask (స్థిరరాశి అక్షరం *smask, పూర్ణాంకం లెక్క) {
  పూర్ణాంకం ముసుగు_2 = 0;
  ఒకవేళ (strchr(smask, 'c')) ముసుగు_2 |= LUA_MASKCALL;
  ఒకవేళ (strchr(smask, 'r')) ముసుగు_2 |= LUA_MASKRET;
  ఒకవేళ (strchr(smask, 'l')) ముసుగు_2 |= LUA_MASKLINE;
  ఒకవేళ (లెక్క > 0) ముసుగు_2 |= LUA_MASKCOUNT;
  తిరిగి ముసుగు_2;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
స్థిర అక్షరం *unmakemask (పూర్ణాంకం ముసుగు_2, అక్షరం *smask) {
  పూర్ణాంకం i = 0;
  ఒకవేళ (ముసుగు_2 & LUA_MASKCALL) smask[i++] = 'c';
  ఒకవేళ (ముసుగు_2 & LUA_MASKRET) smask[i++] = 'r';
  ఒకవేళ (ముసుగు_2 & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  తిరిగి smask;
}


స్థిర పూర్ణాంకం db_sethook (lua_State *L) {
  పూర్ణాంకం arg, ముసుగు_2, లెక్క;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  ఒకవేళ (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = ఖాళీ; ముసుగు_2 = 0; లెక్క = 0;  /* turn off hooks */
  }
  లేకపోతే {
    స్థిరరాశి అక్షరం *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    లెక్క = (పూర్ణాంకం)luaL_optinteger(L, arg + 3, 0);
    func = hookf; ముసుగు_2 = makemask(smask, లెక్క);
  }
  ఒకవేళ (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, ముసుగు_2, లెక్క);
  తిరిగి 0;
}


స్థిర పూర్ణాంకం db_gethook (lua_State *L) {
  పూర్ణాంకం arg;
  lua_State *L1 = getthread(L, &arg);
  అక్షరం buff[5];
  పూర్ణాంకం ముసుగు_2 = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  ఒకవేళ (hook == ఖాళీ) {  /* no hook? */
    luaL_pushfail(L);
    తిరిగి 1;
  }
  లేకపోతే ఒకవేళ (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  లేకపోతే {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(ముసుగు_2, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  తిరిగి 3;
}


స్థిర పూర్ణాంకం db_debug (lua_State *L) {
  కొరకు (;;) {
    అక్షరం బఫరు[250];
    lua_writestringerror("%s", "lua_debug> ");
    ఒకవేళ (fgets(బఫరు, పరిమాణం(బఫరు), stdin) == ఖాళీ ||
        strcmp(బఫరు, "cont\n") == 0)
      తిరిగి 0;
    ఒకవేళ (luaL_loadbufferx(L, బఫరు, అక్షరశ్రేణి_పొడవు(బఫరు), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, ఖాళీ));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


స్థిర పూర్ణాంకం db_traceback (lua_State *L) {
  పూర్ణాంకం arg;
  lua_State *L1 = getthread(L, &arg);
  స్థిరరాశి అక్షరం *సందేశం = lua_tostring(L, arg + 1);
  ఒకవేళ (సందేశం == ఖాళీ && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  లేకపోతే {
    పూర్ణాంకం level = (పూర్ణాంకం)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, సందేశం, level);
  }
  తిరిగి 1;
}


స్థిర స్థిరరాశి luaL_Reg dblib[] = {
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
  {ఖాళీ, ఖాళీ}
};


LUAMOD_API పూర్ణాంకం luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  తిరిగి 1;
}

