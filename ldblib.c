/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#घोषणा ldblib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
अटल स्थिर अक्षर *स्थिर HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
अटल व्योम checkstack (lua_State *L, lua_State *L1, पूर्णांक n) {
  अगर (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


अटल पूर्णांक db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  वापस 1;
}


अटल पूर्णांक db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  अगर (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  वापस 1;
}


अटल पूर्णांक db_setmetatable (lua_State *L) {
  पूर्णांक t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  वापस 1;  /* return 1st argument */
}


अटल पूर्णांक db_getuservalue (lua_State *L) {
  पूर्णांक n = (पूर्णांक)luaL_optinteger(L, 2, 1);
  अगर (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  अन्यथा अगर (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    वापस 2;
  }
  वापस 1;
}


अटल पूर्णांक db_setuservalue (lua_State *L) {
  पूर्णांक n = (पूर्णांक)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  अगर (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  वापस 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
अटल lua_State *getthread (lua_State *L, पूर्णांक *arg) {
  अगर (lua_isthread(L, 1)) {
    *arg = 1;
    वापस lua_tothread(L, 1);
  }
  अन्यथा {
    *arg = 0;
    वापस L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
अटल व्योम settabss (lua_State *L, स्थिर अक्षर *k, स्थिर अक्षर *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

अटल व्योम settabsi (lua_State *L, स्थिर अक्षर *k, पूर्णांक v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

अटल व्योम settabsb (lua_State *L, स्थिर अक्षर *k, पूर्णांक v) {
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
अटल व्योम treatstackoption (lua_State *L, lua_State *L1, स्थिर अक्षर *fname) {
  अगर (L == L1)
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
अटल पूर्णांक db_getinfo (lua_State *L) {
  lua_Debug ar;
  पूर्णांक arg;
  lua_State *L1 = getthread(L, &arg);
  स्थिर अक्षर *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  अगर (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  अन्यथा {  /* stack level */
    अगर (!lua_getstack(L1, (पूर्णांक)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      वापस 1;
    }
  }
  अगर (!lua_getinfo(L1, options, &ar))
    वापस luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  अगर (म_अक्षर(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  अगर (म_अक्षर(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  अगर (म_अक्षर(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  अगर (म_अक्षर(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  अगर (म_अक्षर(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  अगर (म_अक्षर(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  अगर (म_अक्षर(options, 'L'))
    treatstackoption(L, L1, "activelines");
  अगर (म_अक्षर(options, 'f'))
    treatstackoption(L, L1, "func");
  वापस 1;  /* return table */
}


अटल पूर्णांक db_getlocal (lua_State *L) {
  पूर्णांक arg;
  lua_State *L1 = getthread(L, &arg);
  पूर्णांक nvar = (पूर्णांक)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  अगर (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, शून्य, nvar));  /* push local name */
    वापस 1;  /* return only name (there is no value) */
  }
  अन्यथा {  /* stack-level argument */
    lua_Debug ar;
    स्थिर अक्षर *name;
    पूर्णांक level = (पूर्णांक)luaL_checkinteger(L, arg + 1);
    अगर (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      वापस luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    अगर (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      वापस 2;
    }
    अन्यथा {
      luaL_pushfail(L);  /* no name (nor value) */
      वापस 1;
    }
  }
}


अटल पूर्णांक db_setlocal (lua_State *L) {
  पूर्णांक arg;
  स्थिर अक्षर *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  पूर्णांक level = (पूर्णांक)luaL_checkinteger(L, arg + 1);
  पूर्णांक nvar = (पूर्णांक)luaL_checkinteger(L, arg + 2);
  अगर (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    वापस luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  अगर (name == शून्य)
    lua_निकालो(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  वापस 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
अटल पूर्णांक auxupvalue (lua_State *L, पूर्णांक get) {
  स्थिर अक्षर *name;
  पूर्णांक n = (पूर्णांक)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  अगर (name == शून्य) वापस 0;
  lua_pushstring(L, name);
  lua_जोड़ो(L, -(get+1));  /* no-op if get is false */
  वापस get + 1;
}


अटल पूर्णांक db_getupvalue (lua_State *L) {
  वापस auxupvalue(L, 1);
}


अटल पूर्णांक db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  वापस auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
अटल व्योम *checkupval (lua_State *L, पूर्णांक argf, पूर्णांक argnup, पूर्णांक *pnup) {
  व्योम *id;
  पूर्णांक nup = (पूर्णांक)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  अगर (pnup) {
    luaL_argcheck(L, id != शून्य, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  वापस id;
}


अटल पूर्णांक db_upvalueid (lua_State *L) {
  व्योम *id = checkupval(L, 1, 2, शून्य);
  अगर (id != शून्य)
    lua_pushlightuserdata(L, id);
  अन्यथा
    luaL_pushfail(L);
  वापस 1;
}


अटल पूर्णांक db_upvaluejoin (lua_State *L) {
  पूर्णांक n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  वापस 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
अटल व्योम hookf (lua_State *L, lua_Debug *ar) {
  अटल स्थिर अक्षर *स्थिर hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  अगर (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(पूर्णांक)ar->event]);  /* push event name */
    अगर (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    अन्यथा lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
अटल पूर्णांक makemask (स्थिर अक्षर *smask, पूर्णांक गिनती) {
  पूर्णांक mask = 0;
  अगर (म_अक्षर(smask, 'c')) mask |= LUA_MASKCALL;
  अगर (म_अक्षर(smask, 'r')) mask |= LUA_MASKRET;
  अगर (म_अक्षर(smask, 'l')) mask |= LUA_MASKLINE;
  अगर (गिनती > 0) mask |= LUA_MASKCOUNT;
  वापस mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
अटल अक्षर *unmakemask (पूर्णांक mask, अक्षर *smask) {
  पूर्णांक i = 0;
  अगर (mask & LUA_MASKCALL) smask[i++] = 'c';
  अगर (mask & LUA_MASKRET) smask[i++] = 'r';
  अगर (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  वापस smask;
}


अटल पूर्णांक db_sethook (lua_State *L) {
  पूर्णांक arg, mask, गिनती;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  अगर (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = शून्य; mask = 0; गिनती = 0;  /* turn off hooks */
  }
  अन्यथा {
    स्थिर अक्षर *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    गिनती = (पूर्णांक)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, गिनती);
  }
  अगर (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, गिनती);
  वापस 0;
}


अटल पूर्णांक db_gethook (lua_State *L) {
  पूर्णांक arg;
  lua_State *L1 = getthread(L, &arg);
  अक्षर buff[5];
  पूर्णांक mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  अगर (hook == शून्य) {  /* no hook? */
    luaL_pushfail(L);
    वापस 1;
  }
  अन्यथा अगर (hook != hookf)  /* external hook? */
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
  वापस 3;
}


अटल पूर्णांक db_debug (lua_State *L) {
  क्रम (;;) {
    अक्षर बफर[250];
    lua_writestringerror("%s", "lua_debug> ");
    अगर (ख_माला_लो(बफर, माप(बफर), मानक_निवेश) == शून्य ||
        म_भेद(बफर, "cont\n") == 0)
      वापस 0;
    अगर (luaL_loadbufferx(L, बफर, म_माप(बफर), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, शून्य));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


अटल पूर्णांक db_traceback (lua_State *L) {
  पूर्णांक arg;
  lua_State *L1 = getthread(L, &arg);
  स्थिर अक्षर *संदेश = lua_tostring(L, arg + 1);
  अगर (संदेश == शून्य && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  अन्यथा {
    पूर्णांक level = (पूर्णांक)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, संदेश, level);
  }
  वापस 1;
}


अटल स्थिर luaL_Reg dblib[] = {
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
  {शून्य, शून्य}
};


LUAMOD_API पूर्णांक luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  वापस 1;
}

