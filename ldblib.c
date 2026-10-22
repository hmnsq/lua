/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lauxlib.h"
#অন্তর্ভাৱ "lualib.h"
#অন্তর্ভাৱ "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
স্থির শূন্য checkstack (lua_State *L, lua_State *L1, পূর্ণাঙ্ক n) {
  যদি (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


স্থির পূর্ণাঙ্ক db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  যদি (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক db_setmetatable (lua_State *L) {
  পূর্ণাঙ্ক t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ঘূৰাই 1;  /* return 1st argument */
}


স্থির পূর্ণাঙ্ক db_getuservalue (lua_State *L) {
  পূর্ণাঙ্ক n = (পূর্ণাঙ্ক)luaL_optinteger(L, 2, 1);
  যদি (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  অন্যথা যদি (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    ঘূৰাই 2;
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক db_setuservalue (lua_State *L) {
  পূর্ণাঙ্ক n = (পূর্ণাঙ্ক)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  যদি (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  ঘূৰাই 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
স্থির lua_State *getthread (lua_State *L, পূর্ণাঙ্ক *arg) {
  যদি (lua_isthread(L, 1)) {
    *arg = 1;
    ঘূৰাই lua_tothread(L, 1);
  }
  অন্যথা {
    *arg = 0;
    ঘূৰাই L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
স্থির শূন্য settabss (lua_State *L, ধ্রুৱ ৱর্ণ *k, ধ্রুৱ ৱর্ণ *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

স্থির শূন্য settabsi (lua_State *L, ধ্রুৱ ৱর্ণ *k, পূর্ণাঙ্ক v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

স্থির শূন্য settabsb (lua_State *L, ধ্রুৱ ৱর্ণ *k, পূর্ণাঙ্ক v) {
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
স্থির শূন্য treatstackoption (lua_State *L, lua_State *L1, ধ্রুৱ ৱর্ণ *fname) {
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
স্থির পূর্ণাঙ্ক db_getinfo (lua_State *L) {
  lua_Debug ar;
  পূর্ণাঙ্ক arg;
  lua_State *L1 = getthread(L, &arg);
  ধ্রুৱ ৱর্ণ *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  যদি (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  অন্যথা {  /* stack level */
    যদি (!lua_getstack(L1, (পূর্ণাঙ্ক)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      ঘূৰাই 1;
    }
  }
  যদি (!lua_getinfo(L1, options, &ar))
    ঘূৰাই luaL_argerror(L, arg+2, "invalid option");
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
  ঘূৰাই 1;  /* return table */
}


স্থির পূর্ণাঙ্ক db_getlocal (lua_State *L) {
  পূর্ণাঙ্ক arg;
  lua_State *L1 = getthread(L, &arg);
  পূর্ণাঙ্ক nvar = (পূর্ণাঙ্ক)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  যদি (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, রিক্ত, nvar));  /* push local name */
    ঘূৰাই 1;  /* return only name (there is no value) */
  }
  অন্যথা {  /* stack-level argument */
    lua_Debug ar;
    ধ্রুৱ ৱর্ণ *name;
    পূর্ণাঙ্ক level = (পূর্ণাঙ্ক)luaL_checkinteger(L, arg + 1);
    যদি (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      ঘূৰাই luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    যদি (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      ঘূৰাই 2;
    }
    অন্যথা {
      luaL_pushfail(L);  /* no name (nor value) */
      ঘূৰাই 1;
    }
  }
}


স্থির পূর্ণাঙ্ক db_setlocal (lua_State *L) {
  পূর্ণাঙ্ক arg;
  ধ্রুৱ ৱর্ণ *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  পূর্ণাঙ্ক level = (পূর্ণাঙ্ক)luaL_checkinteger(L, arg + 1);
  পূর্ণাঙ্ক nvar = (পূর্ণাঙ্ক)luaL_checkinteger(L, arg + 2);
  যদি (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    ঘূৰাই luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  যদি (name == রিক্ত)
    lua_বেৰ_কৰো(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  ঘূৰাই 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
স্থির পূর্ণাঙ্ক auxupvalue (lua_State *L, পূর্ণাঙ্ক get) {
  ধ্রুৱ ৱর্ণ *name;
  পূর্ণাঙ্ক n = (পূর্ণাঙ্ক)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  যদি (name == রিক্ত) ঘূৰাই 0;
  lua_pushstring(L, name);
  lua_ঢোকাও(L, -(get+1));  /* no-op if get is false */
  ঘূৰাই get + 1;
}


স্থির পূর্ণাঙ্ক db_getupvalue (lua_State *L) {
  ঘূৰাই auxupvalue(L, 1);
}


স্থির পূর্ণাঙ্ক db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  ঘূৰাই auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
স্থির শূন্য *checkupval (lua_State *L, পূর্ণাঙ্ক argf, পূর্ণাঙ্ক argnup, পূর্ণাঙ্ক *pnup) {
  শূন্য *id;
  পূর্ণাঙ্ক nup = (পূর্ণাঙ্ক)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  যদি (pnup) {
    luaL_argcheck(L, id != রিক্ত, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  ঘূৰাই id;
}


স্থির পূর্ণাঙ্ক db_upvalueid (lua_State *L) {
  শূন্য *id = checkupval(L, 1, 2, রিক্ত);
  যদি (id != রিক্ত)
    lua_pushlightuserdata(L, id);
  অন্যথা
    luaL_pushfail(L);
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক db_upvaluejoin (lua_State *L) {
  পূর্ণাঙ্ক n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  ঘূৰাই 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
স্থির শূন্য hookf (lua_State *L, lua_Debug *ar) {
  স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  যদি (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(পূর্ণাঙ্ক)ar->event]);  /* push event name */
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
স্থির পূর্ণাঙ্ক makemask (ধ্রুৱ ৱর্ণ *smask, পূর্ণাঙ্ক গুনতি) {
  পূর্ণাঙ্ক mask = 0;
  যদি (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  যদি (strchr(smask, 'r')) mask |= LUA_MASKRET;
  যদি (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  যদি (গুনতি > 0) mask |= LUA_MASKCOUNT;
  ঘূৰাই mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
স্থির ৱর্ণ *unmakemask (পূর্ণাঙ্ক mask, ৱর্ণ *smask) {
  পূর্ণাঙ্ক i = 0;
  যদি (mask & LUA_MASKCALL) smask[i++] = 'c';
  যদি (mask & LUA_MASKRET) smask[i++] = 'r';
  যদি (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  ঘূৰাই smask;
}


স্থির পূর্ণাঙ্ক db_sethook (lua_State *L) {
  পূর্ণাঙ্ক arg, mask, গুনতি;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  যদি (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = রিক্ত; mask = 0; গুনতি = 0;  /* turn off hooks */
  }
  অন্যথা {
    ধ্রুৱ ৱর্ণ *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    গুনতি = (পূর্ণাঙ্ক)luaL_optinteger(L, arg + 3, 0);
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
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক db_gethook (lua_State *L) {
  পূর্ণাঙ্ক arg;
  lua_State *L1 = getthread(L, &arg);
  ৱর্ণ buff[5];
  পূর্ণাঙ্ক mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  যদি (hook == রিক্ত) {  /* no hook? */
    luaL_pushfail(L);
    ঘূৰাই 1;
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
  ঘূৰাই 3;
}


স্থির পূর্ণাঙ্ক db_debug (lua_State *L) {
  বাবে (;;) {
    ৱর্ণ buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    যদি (fgets(buffer, পরিমাণ(buffer), stdin) == রিক্ত ||
        strcmp(buffer, "cont\n") == 0)
      ঘূৰাই 0;
    যদি (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, রিক্ত));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


স্থির পূর্ণাঙ্ক db_traceback (lua_State *L) {
  পূর্ণাঙ্ক arg;
  lua_State *L1 = getthread(L, &arg);
  ধ্রুৱ ৱর্ণ *বাৰ্তা = lua_tostring(L, arg + 1);
  যদি (বাৰ্তা == রিক্ত && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  অন্যথা {
    পূর্ণাঙ্ক level = (পূর্ণাঙ্ক)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, বাৰ্তা, level);
  }
  ঘূৰাই 1;
}


স্থির ধ্রুৱ luaL_Reg dblib[] = {
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
  {রিক্ত, রিক্ত}
};


LUAMOD_API পূর্ণাঙ্ক luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  ঘূৰাই 1;
}

