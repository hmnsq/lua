/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ldblib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkstack (lua_State *L, lua_State *L1, ಪೂರ್ಣಾಂಕ n) {
  ಆದರೆ (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ಆದರೆ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_setmetatable (lua_State *L) {
  ಪೂರ್ಣಾಂಕ t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ಹಿಂತಿರುಗಿ 1;  /* return 1st argument */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_getuservalue (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = (ಪೂರ್ಣಾಂಕ)luaL_optinteger(L, 2, 1);
  ಆದರೆ (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    ಹಿಂತಿರುಗಿ 2;
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_setuservalue (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = (ಪೂರ್ಣಾಂಕ)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  ಆದರೆ (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
ಸ್ಥಿರ lua_State *getthread (lua_State *L, ಪೂರ್ಣಾಂಕ *arg) {
  ಆದರೆ (lua_isthread(L, 1)) {
    *arg = 1;
    ಹಿಂತಿರುಗಿ lua_tothread(L, 1);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    *arg = 0;
    ಹಿಂತಿರುಗಿ L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
ಸ್ಥಿರ ಶೂನ್ಯ settabss (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

ಸ್ಥಿರ ಶೂನ್ಯ settabsi (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k, ಪೂರ್ಣಾಂಕ v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

ಸ್ಥಿರ ಶೂನ್ಯ settabsb (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k, ಪೂರ್ಣಾಂಕ v) {
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
ಸ್ಥಿರ ಶೂನ್ಯ treatstackoption (lua_State *L, lua_State *L1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname) {
  ಆದರೆ (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  ಇಲ್ಲದಿದ್ದರೆ
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_getinfo (lua_State *L) {
  lua_Debug ar;
  ಪೂರ್ಣಾಂಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  ಆದರೆ (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* stack level */
    ಆದರೆ (!lua_getstack(L1, (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಆದರೆ (!lua_getinfo(L1, options, &ar))
    ಹಿಂತಿರುಗಿ luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  ಆದರೆ (strchr(options, 'S')) {
    lua_pushlstring(L, ar.ಮೂಲ, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  ಆದರೆ (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  ಆದರೆ (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  ಆದರೆ (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  ಆದರೆ (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  ಆದರೆ (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  ಆದರೆ (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  ಆದರೆ (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  ಹಿಂತಿರುಗಿ 1;  /* return table */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_getlocal (lua_State *L) {
  ಪೂರ್ಣಾಂಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ಪೂರ್ಣಾಂಕ nvar = (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  ಆದರೆ (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, ಖಾಲಿ, nvar));  /* push local name */
    ಹಿಂತಿರುಗಿ 1;  /* return only name (there is no value) */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* stack-level argument */
    lua_Debug ar;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
    ಪೂರ್ಣಾಂಕ level = (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, arg + 1);
    ಆದರೆ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      ಹಿಂತಿರುಗಿ luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    ಆದರೆ (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      ಹಿಂತಿರುಗಿ 2;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      luaL_pushfail(L);  /* no name (nor value) */
      ಹಿಂತಿರುಗಿ 1;
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_setlocal (lua_State *L) {
  ಪೂರ್ಣಾಂಕ arg;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  ಪೂರ್ಣಾಂಕ level = (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, arg + 1);
  ಪೂರ್ಣಾಂಕ nvar = (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, arg + 2);
  ಆದರೆ (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    ಹಿಂತಿರುಗಿ luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  ಆದರೆ (name == ಖಾಲಿ)
    lua_ತೆಗೆ(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ auxupvalue (lua_State *L, ಪೂರ್ಣಾಂಕ get) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  ಪೂರ್ಣಾಂಕ n = (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /*ಆವರಣ*/
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  ಆದರೆ (name == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 0;
  lua_pushstring(L, name);
  lua_ಸೇರಿಸು(L, -(get+1));  /* no-op if get is false */
  ಹಿಂತಿರುಗಿ get + 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_getupvalue (lua_State *L) {
  ಹಿಂತಿರುಗಿ auxupvalue(L, 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  ಹಿಂತಿರುಗಿ auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
ಸ್ಥಿರ ಶೂನ್ಯ *checkupval (lua_State *L, ಪೂರ್ಣಾಂಕ argf, ಪೂರ್ಣಾಂಕ argnup, ಪೂರ್ಣಾಂಕ *pnup) {
  ಶೂನ್ಯ *id;
  ಪೂರ್ಣಾಂಕ nup = (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /*ಆವರಣ*/
  id = lua_upvalueid(L, argf, nup);
  ಆದರೆ (pnup) {
    luaL_argcheck(L, id != ಖಾಲಿ, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  ಹಿಂತಿರುಗಿ id;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_upvalueid (lua_State *L) {
  ಶೂನ್ಯ *id = checkupval(L, 1, 2, ಖಾಲಿ);
  ಆದರೆ (id != ಖಾಲಿ)
    lua_pushlightuserdata(L, id);
  ಇಲ್ಲದಿದ್ದರೆ
    luaL_pushfail(L);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_upvaluejoin (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  ಹಿಂತಿರುಗಿ 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
ಸ್ಥಿರ ಶೂನ್ಯ hookf (lua_State *L, lua_Debug *ar) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  ಆದರೆ (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(ಪೂರ್ಣಾಂಕ)ar->event]);  /* push event name */
    ಆದರೆ (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    ಇಲ್ಲದಿದ್ದರೆ lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ makemask (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *smask, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2) {
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2 = 0;
  ಆದರೆ (strchr(smask, 'c')) ಮುಖವಾಡ_2 |= LUA_MASKCALL;
  ಆದರೆ (strchr(smask, 'r')) ಮುಖವಾಡ_2 |= LUA_MASKRET;
  ಆದರೆ (strchr(smask, 'l')) ಮುಖವಾಡ_2 |= LUA_MASKLINE;
  ಆದರೆ (ಎಣಿಕೆ_2 > 0) ಮುಖವಾಡ_2 |= LUA_MASKCOUNT;
  ಹಿಂತಿರುಗಿ ಮುಖವಾಡ_2;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
ಸ್ಥಿರ ಅಕ್ಷರ *unmakemask (ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2, ಅಕ್ಷರ *smask) {
  ಪೂರ್ಣಾಂಕ i = 0;
  ಆದರೆ (ಮುಖವಾಡ_2 & LUA_MASKCALL) smask[i++] = 'c';
  ಆದರೆ (ಮುಖವಾಡ_2 & LUA_MASKRET) smask[i++] = 'r';
  ಆದರೆ (ಮುಖವಾಡ_2 & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  ಹಿಂತಿರುಗಿ smask;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_sethook (lua_State *L) {
  ಪೂರ್ಣಾಂಕ arg, ಮುಖವಾಡ_2, ಎಣಿಕೆ_2;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  ಆದರೆ (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = ಖಾಲಿ; ಮುಖವಾಡ_2 = 0; ಎಣಿಕೆ_2 = 0;  /* turn off hooks */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    ಎಣಿಕೆ_2 = (ಪೂರ್ಣಾಂಕ)luaL_optinteger(L, arg + 3, 0);
    func = hookf; ಮುಖವಾಡ_2 = makemask(smask, ಎಣಿಕೆ_2);
  }
  ಆದರೆ (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, ಮುಖವಾಡ_2, ಎಣಿಕೆ_2);
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_gethook (lua_State *L) {
  ಪೂರ್ಣಾಂಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ಅಕ್ಷರ buff[5];
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2 = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  ಆದರೆ (hook == ಖಾಲಿ) {  /* no hook? */
    luaL_pushfail(L);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  ಇಲ್ಲದಿದ್ದರೆ {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(ಮುಖವಾಡ_2, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  ಹಿಂತಿರುಗಿ 3;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_debug (lua_State *L) {
  ಗಾಗಿ (;;) {
    ಅಕ್ಷರ ಬಫರ್[250];
    lua_writestringerror("%s", "lua_debug> ");
    ಆದರೆ (fgets(ಬಫರ್, ಗಾತ್ರ(ಬಫರ್), stdin) == ಖಾಲಿ ||
        strcmp(ಬಫರ್, "cont\n") == 0)
      ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ (luaL_loadbufferx(L, ಬಫರ್, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಬಫರ್), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, ಖಾಲಿ));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ db_traceback (lua_State *L) {
  ಪೂರ್ಣಾಂಕ arg;
  lua_State *L1 = getthread(L, &arg);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = lua_tostring(L, arg + 1);
  ಆದರೆ (ಸಂದೇಶ == ಖಾಲಿ && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ level = (ಪೂರ್ಣಾಂಕ)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, ಸಂದೇಶ, level);
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg dblib[] = {
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
  {ಖಾಲಿ, ಖಾಲಿ}
};


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  ಹಿಂತಿರುಗಿ 1;
}

