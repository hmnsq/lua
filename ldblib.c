/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
स्थिर अचर वर्ण *अचर HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
स्थिर शून्य checkstack (lua_State *L, lua_State *L1, पूर्णाङ्क n) {
  यदि (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


स्थिर पूर्णाङ्क db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  फर्काऊ 1;
}


स्थिर पूर्णाङ्क db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  यदि (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  फर्काऊ 1;
}


स्थिर पूर्णाङ्क db_setmetatable (lua_State *L) {
  पूर्णाङ्क t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  फर्काऊ 1;  /* return 1st argument */
}


स्थिर पूर्णाङ्क db_getuservalue (lua_State *L) {
  पूर्णाङ्क n = (पूर्णाङ्क)luaL_optinteger(L, 2, 1);
  यदि (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  अन्यथा यदि (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    फर्काऊ 2;
  }
  फर्काऊ 1;
}


स्थिर पूर्णाङ्क db_setuservalue (lua_State *L) {
  पूर्णाङ्क n = (पूर्णाङ्क)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  यदि (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  फर्काऊ 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
स्थिर lua_State *getthread (lua_State *L, पूर्णाङ्क *arg) {
  यदि (lua_isthread(L, 1)) {
    *arg = 1;
    फर्काऊ lua_tothread(L, 1);
  }
  अन्यथा {
    *arg = 0;
    फर्काऊ L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
स्थिर शून्य settabss (lua_State *L, अचर वर्ण *k, अचर वर्ण *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

स्थिर शून्य settabsi (lua_State *L, अचर वर्ण *k, पूर्णाङ्क v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

स्थिर शून्य settabsb (lua_State *L, अचर वर्ण *k, पूर्णाङ्क v) {
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
स्थिर शून्य treatstackoption (lua_State *L, lua_State *L1, अचर वर्ण *fname) {
  यदि (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  अन्यथा
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
स्थिर पूर्णाङ्क db_getinfo (lua_State *L) {
  lua_Debug ar;
  पूर्णाङ्क arg;
  lua_State *L1 = getthread(L, &arg);
  अचर वर्ण *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  यदि (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  अन्यथा {  /* stack level */
    यदि (!lua_getstack(L1, (पूर्णाङ्क)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      फर्काऊ 1;
    }
  }
  यदि (!lua_getinfo(L1, options, &ar))
    फर्काऊ luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  यदि (म_अक्षर(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  यदि (म_अक्षर(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  यदि (म_अक्षर(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  यदि (म_अक्षर(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  यदि (म_अक्षर(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  यदि (म_अक्षर(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  यदि (म_अक्षर(options, 'L'))
    treatstackoption(L, L1, "activelines");
  यदि (म_अक्षर(options, 'f'))
    treatstackoption(L, L1, "func");
  फर्काऊ 1;  /* return table */
}


स्थिर पूर्णाङ्क db_getlocal (lua_State *L) {
  पूर्णाङ्क arg;
  lua_State *L1 = getthread(L, &arg);
  पूर्णाङ्क nvar = (पूर्णाङ्क)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  यदि (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, रिक्त, nvar));  /* push local name */
    फर्काऊ 1;  /* return only name (there is no value) */
  }
  अन्यथा {  /* stack-level argument */
    lua_Debug ar;
    अचर वर्ण *name;
    पूर्णाङ्क level = (पूर्णाङ्क)luaL_checkinteger(L, arg + 1);
    यदि (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      फर्काऊ luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    यदि (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      फर्काऊ 2;
    }
    अन्यथा {
      luaL_pushfail(L);  /* no name (nor value) */
      फर्काऊ 1;
    }
  }
}


स्थिर पूर्णाङ्क db_setlocal (lua_State *L) {
  पूर्णाङ्क arg;
  अचर वर्ण *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  पूर्णाङ्क level = (पूर्णाङ्क)luaL_checkinteger(L, arg + 1);
  पूर्णाङ्क nvar = (पूर्णाङ्क)luaL_checkinteger(L, arg + 2);
  यदि (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    फर्काऊ luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  यदि (name == रिक्त)
    lua_झिक(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  फर्काऊ 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
स्थिर पूर्णाङ्क auxupvalue (lua_State *L, पूर्णाङ्क get) {
  अचर वर्ण *name;
  पूर्णाङ्क n = (पूर्णाङ्क)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  यदि (name == रिक्त) फर्काऊ 0;
  lua_pushstring(L, name);
  lua_घुसाउ(L, -(get+1));  /* no-op if get is false */
  फर्काऊ get + 1;
}


स्थिर पूर्णाङ्क db_getupvalue (lua_State *L) {
  फर्काऊ auxupvalue(L, 1);
}


स्थिर पूर्णाङ्क db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  फर्काऊ auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
स्थिर शून्य *checkupval (lua_State *L, पूर्णाङ्क argf, पूर्णाङ्क argnup, पूर्णाङ्क *pnup) {
  शून्य *id;
  पूर्णाङ्क nup = (पूर्णाङ्क)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  यदि (pnup) {
    luaL_argcheck(L, id != रिक्त, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  फर्काऊ id;
}


स्थिर पूर्णाङ्क db_upvalueid (lua_State *L) {
  शून्य *id = checkupval(L, 1, 2, रिक्त);
  यदि (id != रिक्त)
    lua_pushlightuserdata(L, id);
  अन्यथा
    luaL_pushfail(L);
  फर्काऊ 1;
}


स्थिर पूर्णाङ्क db_upvaluejoin (lua_State *L) {
  पूर्णाङ्क n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  फर्काऊ 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
स्थिर शून्य hookf (lua_State *L, lua_Debug *ar) {
  स्थिर अचर वर्ण *अचर hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  यदि (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(पूर्णाङ्क)ar->event]);  /* push event name */
    यदि (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    अन्यथा lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
स्थिर पूर्णाङ्क makemask (अचर वर्ण *smask, पूर्णाङ्क गन्ती) {
  पूर्णाङ्क mask = 0;
  यदि (म_अक्षर(smask, 'c')) mask |= LUA_MASKCALL;
  यदि (म_अक्षर(smask, 'r')) mask |= LUA_MASKRET;
  यदि (म_अक्षर(smask, 'l')) mask |= LUA_MASKLINE;
  यदि (गन्ती > 0) mask |= LUA_MASKCOUNT;
  फर्काऊ mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
स्थिर वर्ण *unmakemask (पूर्णाङ्क mask, वर्ण *smask) {
  पूर्णाङ्क i = 0;
  यदि (mask & LUA_MASKCALL) smask[i++] = 'c';
  यदि (mask & LUA_MASKRET) smask[i++] = 'r';
  यदि (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  फर्काऊ smask;
}


स्थिर पूर्णाङ्क db_sethook (lua_State *L) {
  पूर्णाङ्क arg, mask, गन्ती;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  यदि (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = रिक्त; mask = 0; गन्ती = 0;  /* turn off hooks */
  }
  अन्यथा {
    अचर वर्ण *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    गन्ती = (पूर्णाङ्क)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, गन्ती);
  }
  यदि (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, गन्ती);
  फर्काऊ 0;
}


स्थिर पूर्णाङ्क db_gethook (lua_State *L) {
  पूर्णाङ्क arg;
  lua_State *L1 = getthread(L, &arg);
  वर्ण buff[5];
  पूर्णाङ्क mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  यदि (hook == रिक्त) {  /* no hook? */
    luaL_pushfail(L);
    फर्काऊ 1;
  }
  अन्यथा यदि (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  अन्यथा {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  फर्काऊ 3;
}


स्थिर पूर्णाङ्क db_debug (lua_State *L) {
  लागि (;;) {
    वर्ण buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    यदि (ख_माला_लो(buffer, परिमाण(buffer), मानक_निवेश) == रिक्त ||
        म_भेद(buffer, "cont\n") == 0)
      फर्काऊ 0;
    यदि (luaL_loadbufferx(L, buffer, म_माप(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, रिक्त));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


स्थिर पूर्णाङ्क db_traceback (lua_State *L) {
  पूर्णाङ्क arg;
  lua_State *L1 = getthread(L, &arg);
  अचर वर्ण *सन्देश = lua_tostring(L, arg + 1);
  यदि (सन्देश == रिक्त && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  अन्यथा {
    पूर्णाङ्क level = (पूर्णाङ्क)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, सन्देश, level);
  }
  फर्काऊ 1;
}


स्थिर अचर luaL_Reg dblib[] = {
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
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णाङ्क luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  फर्काऊ 1;
}

