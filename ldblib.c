/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkstack (lua_State *L, lua_State *L1, ಪೂರ್ಣಾಙ್ಕ n) {
  ಯದಿ (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ಯದಿ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_setmetatable (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ಪ್ರತ್ಯಾಗಮ 1;  /* return 1st argument */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_getuservalue (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = (ಪೂರ್ಣಾಙ್ಕ)luaL_optinteger(L, 2, 1);
  ಯದಿ (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  ಅನ್ಯಥಾ ಯದಿ (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    ಪ್ರತ್ಯಾಗಮ 2;
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_setuservalue (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = (ಪೂರ್ಣಾಙ್ಕ)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  ಯದಿ (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
ಸ್ಥಿರ lua_State *getthread (lua_State *L, ಪೂರ್ಣಾಙ್ಕ *arg) {
  ಯದಿ (lua_isthread(L, 1)) {
    *arg = 1;
    ಪ್ರತ್ಯಾಗಮ lua_tothread(L, 1);
  }
  ಅನ್ಯಥಾ {
    *arg = 0;
    ಪ್ರತ್ಯಾಗಮ L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
ಸ್ಥಿರ ಶೂನ್ಯ settabss (lua_State *L, ಧ್ರುವ ವರ್ಣ *k, ಧ್ರುವ ವರ್ಣ *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

ಸ್ಥಿರ ಶೂನ್ಯ settabsi (lua_State *L, ಧ್ರುವ ವರ್ಣ *k, ಪೂರ್ಣಾಙ್ಕ v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

ಸ್ಥಿರ ಶೂನ್ಯ settabsb (lua_State *L, ಧ್ರುವ ವರ್ಣ *k, ಪೂರ್ಣಾಙ್ಕ v) {
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
ಸ್ಥಿರ ಶೂನ್ಯ treatstackoption (lua_State *L, lua_State *L1, ಧ್ರುವ ವರ್ಣ *fname) {
  ಯದಿ (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  ಅನ್ಯಥಾ
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_getinfo (lua_State *L) {
  lua_Debug ar;
  ಪೂರ್ಣಾಙ್ಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ಧ್ರುವ ವರ್ಣ *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  ಯದಿ (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  ಅನ್ಯಥಾ {  /* stack level */
    ಯದಿ (!lua_getstack(L1, (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಯದಿ (!lua_getinfo(L1, options, &ar))
    ಪ್ರತ್ಯಾಗಮ luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  ಯದಿ (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  ಯದಿ (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  ಯದಿ (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  ಯದಿ (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  ಯದಿ (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  ಯದಿ (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  ಯದಿ (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  ಯದಿ (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  ಪ್ರತ್ಯಾಗಮ 1;  /* return table */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_getlocal (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ಪೂರ್ಣಾಙ್ಕ nvar = (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  ಯದಿ (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, ರಿಕ್ತ, nvar));  /* push local name */
    ಪ್ರತ್ಯಾಗಮ 1;  /* return only name (there is no value) */
  }
  ಅನ್ಯಥಾ {  /* stack-level argument */
    lua_Debug ar;
    ಧ್ರುವ ವರ್ಣ *name;
    ಪೂರ್ಣಾಙ್ಕ level = (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, arg + 1);
    ಯದಿ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      ಪ್ರತ್ಯಾಗಮ luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    ಯದಿ (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      ಪ್ರತ್ಯಾಗಮ 2;
    }
    ಅನ್ಯಥಾ {
      luaL_pushfail(L);  /* no name (nor value) */
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_setlocal (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ arg;
  ಧ್ರುವ ವರ್ಣ *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  ಪೂರ್ಣಾಙ್ಕ level = (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, arg + 1);
  ಪೂರ್ಣಾಙ್ಕ nvar = (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, arg + 2);
  ಯದಿ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    ಪ್ರತ್ಯಾಗಮ luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  ಯದಿ (name == ರಿಕ್ತ)
    lua_ತೆಗೆ(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ auxupvalue (lua_State *L, ಪೂರ್ಣಾಙ್ಕ get) {
  ಧ್ರುವ ವರ್ಣ *name;
  ಪೂರ್ಣಾಙ್ಕ n = (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  ಯದಿ (name == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 0;
  lua_pushstring(L, name);
  lua_ಸೇರಿಸು(L, -(get+1));  /* no-op if get is false */
  ಪ್ರತ್ಯಾಗಮ get + 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_getupvalue (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ auxupvalue(L, 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  ಪ್ರತ್ಯಾಗಮ auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
ಸ್ಥಿರ ಶೂನ್ಯ *checkupval (lua_State *L, ಪೂರ್ಣಾಙ್ಕ argf, ಪೂರ್ಣಾಙ್ಕ argnup, ಪೂರ್ಣಾಙ್ಕ *pnup) {
  ಶೂನ್ಯ *id;
  ಪೂರ್ಣಾಙ್ಕ nup = (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  ಯದಿ (pnup) {
    luaL_argcheck(L, id != ರಿಕ್ತ, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  ಪ್ರತ್ಯಾಗಮ id;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_upvalueid (lua_State *L) {
  ಶೂನ್ಯ *id = checkupval(L, 1, 2, ರಿಕ್ತ);
  ಯದಿ (id != ರಿಕ್ತ)
    lua_pushlightuserdata(L, id);
  ಅನ್ಯಥಾ
    luaL_pushfail(L);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_upvaluejoin (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  ಪ್ರತ್ಯಾಗಮ 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
ಸ್ಥಿರ ಶೂನ್ಯ hookf (lua_State *L, lua_Debug *ar) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  ಯದಿ (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(ಪೂರ್ಣಾಙ್ಕ)ar->event]);  /* push event name */
    ಯದಿ (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    ಅನ್ಯಥಾ lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ makemask (ಧ್ರುವ ವರ್ಣ *smask, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ) {
  ಪೂರ್ಣಾಙ್ಕ mask = 0;
  ಯದಿ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  ಯದಿ (strchr(smask, 'r')) mask |= LUA_MASKRET;
  ಯದಿ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  ಯದಿ (ಎಣಿಕೆ > 0) mask |= LUA_MASKCOUNT;
  ಪ್ರತ್ಯಾಗಮ mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
ಸ್ಥಿರ ವರ್ಣ *unmakemask (ಪೂರ್ಣಾಙ್ಕ mask, ವರ್ಣ *smask) {
  ಪೂರ್ಣಾಙ್ಕ i = 0;
  ಯದಿ (mask & LUA_MASKCALL) smask[i++] = 'c';
  ಯದಿ (mask & LUA_MASKRET) smask[i++] = 'r';
  ಯದಿ (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  ಪ್ರತ್ಯಾಗಮ smask;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_sethook (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ arg, mask, ಎಣಿಕೆ;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  ಯದಿ (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = ರಿಕ್ತ; mask = 0; ಎಣಿಕೆ = 0;  /* turn off hooks */
  }
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    ಎಣಿಕೆ = (ಪೂರ್ಣಾಙ್ಕ)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, ಎಣಿಕೆ);
  }
  ಯದಿ (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, ಎಣಿಕೆ);
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_gethook (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ವರ್ಣ buff[5];
  ಪೂರ್ಣಾಙ್ಕ mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  ಯದಿ (hook == ರಿಕ್ತ) {  /* no hook? */
    luaL_pushfail(L);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ ಯದಿ (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  ಅನ್ಯಥಾ {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  ಪ್ರತ್ಯಾಗಮ 3;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_debug (lua_State *L) {
  ಪ್ರತಿ (;;) {
    ವರ್ಣ buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    ಯದಿ (fgets(buffer, ಪರಿಮಾಣ(buffer), stdin) == ರಿಕ್ತ ||
        strcmp(buffer, "cont\n") == 0)
      ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, ರಿಕ್ತ));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ db_traceback (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = lua_tostring(L, arg + 1);
  ಯದಿ (ಸಂದೇಶ == ರಿಕ್ತ && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ level = (ಪೂರ್ಣಾಙ್ಕ)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, ಸಂದೇಶ, level);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg dblib[] = {
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
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  ಪ್ರತ್ಯಾಗಮ 1;
}

