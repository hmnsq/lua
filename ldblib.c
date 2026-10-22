/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "lualib.h"
#અન્તર્ભાવ "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
સ્થિર અચલ અક્ષર *અચલ HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
સ્થિર શૂન્ય checkstack (lua_State *L, lua_State *L1, પૂર્ણાંક n) {
  જો (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


સ્થિર પૂર્ણાંક db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  પાછા 1;
}


સ્થિર પૂર્ણાંક db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  જો (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક db_setmetatable (lua_State *L) {
  પૂર્ણાંક t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  પાછા 1;  /* return 1st argument */
}


સ્થિર પૂર્ણાંક db_getuservalue (lua_State *L) {
  પૂર્ણાંક n = (પૂર્ણાંક)luaL_optinteger(L, 2, 1);
  જો (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  નહીંતર જો (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    પાછા 2;
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક db_setuservalue (lua_State *L) {
  પૂર્ણાંક n = (પૂર્ણાંક)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  જો (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  પાછા 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
સ્થિર lua_State *getthread (lua_State *L, પૂર્ણાંક *arg) {
  જો (lua_isthread(L, 1)) {
    *arg = 1;
    પાછા lua_tothread(L, 1);
  }
  નહીંતર {
    *arg = 0;
    પાછા L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
સ્થિર શૂન્ય settabss (lua_State *L, અચલ અક્ષર *k, અચલ અક્ષર *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

સ્થિર શૂન્ય settabsi (lua_State *L, અચલ અક્ષર *k, પૂર્ણાંક v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

સ્થિર શૂન્ય settabsb (lua_State *L, અચલ અક્ષર *k, પૂર્ણાંક v) {
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
સ્થિર શૂન્ય treatstackoption (lua_State *L, lua_State *L1, અચલ અક્ષર *fname) {
  જો (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  નહીંતર
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
સ્થિર પૂર્ણાંક db_getinfo (lua_State *L) {
  lua_Debug ar;
  પૂર્ણાંક arg;
  lua_State *L1 = getthread(L, &arg);
  અચલ અક્ષર *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  જો (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  નહીંતર {  /* stack level */
    જો (!lua_getstack(L1, (પૂર્ણાંક)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      પાછા 1;
    }
  }
  જો (!lua_getinfo(L1, options, &ar))
    પાછા luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  જો (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  જો (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  જો (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  જો (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  જો (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  જો (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  જો (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  જો (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  પાછા 1;  /* return table */
}


સ્થિર પૂર્ણાંક db_getlocal (lua_State *L) {
  પૂર્ણાંક arg;
  lua_State *L1 = getthread(L, &arg);
  પૂર્ણાંક nvar = (પૂર્ણાંક)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  જો (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, ખાલી, nvar));  /* push local name */
    પાછા 1;  /* return only name (there is no value) */
  }
  નહીંતર {  /* stack-level argument */
    lua_Debug ar;
    અચલ અક્ષર *name;
    પૂર્ણાંક level = (પૂર્ણાંક)luaL_checkinteger(L, arg + 1);
    જો (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      પાછા luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    જો (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      પાછા 2;
    }
    નહીંતર {
      luaL_pushfail(L);  /* no name (nor value) */
      પાછા 1;
    }
  }
}


સ્થિર પૂર્ણાંક db_setlocal (lua_State *L) {
  પૂર્ણાંક arg;
  અચલ અક્ષર *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  પૂર્ણાંક level = (પૂર્ણાંક)luaL_checkinteger(L, arg + 1);
  પૂર્ણાંક nvar = (પૂર્ણાંક)luaL_checkinteger(L, arg + 2);
  જો (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    પાછા luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  જો (name == ખાલી)
    lua_કાઢો(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  પાછા 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
સ્થિર પૂર્ણાંક auxupvalue (lua_State *L, પૂર્ણાંક get) {
  અચલ અક્ષર *name;
  પૂર્ણાંક n = (પૂર્ણાંક)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  જો (name == ખાલી) પાછા 0;
  lua_pushstring(L, name);
  lua_ઉમેરો(L, -(get+1));  /* no-op if get is false */
  પાછા get + 1;
}


સ્થિર પૂર્ણાંક db_getupvalue (lua_State *L) {
  પાછા auxupvalue(L, 1);
}


સ્થિર પૂર્ણાંક db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  પાછા auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
સ્થિર શૂન્ય *checkupval (lua_State *L, પૂર્ણાંક argf, પૂર્ણાંક argnup, પૂર્ણાંક *pnup) {
  શૂન્ય *id;
  પૂર્ણાંક nup = (પૂર્ણાંક)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  જો (pnup) {
    luaL_argcheck(L, id != ખાલી, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  પાછા id;
}


સ્થિર પૂર્ણાંક db_upvalueid (lua_State *L) {
  શૂન્ય *id = checkupval(L, 1, 2, ખાલી);
  જો (id != ખાલી)
    lua_pushlightuserdata(L, id);
  નહીંતર
    luaL_pushfail(L);
  પાછા 1;
}


સ્થિર પૂર્ણાંક db_upvaluejoin (lua_State *L) {
  પૂર્ણાંક n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  પાછા 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
સ્થિર શૂન્ય hookf (lua_State *L, lua_Debug *ar) {
  સ્થિર અચલ અક્ષર *અચલ hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  જો (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(પૂર્ણાંક)ar->event]);  /* push event name */
    જો (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    નહીંતર lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
સ્થિર પૂર્ણાંક makemask (અચલ અક્ષર *smask, પૂર્ણાંક ગણતરી) {
  પૂર્ણાંક mask = 0;
  જો (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  જો (strchr(smask, 'r')) mask |= LUA_MASKRET;
  જો (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  જો (ગણતરી > 0) mask |= LUA_MASKCOUNT;
  પાછા mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
સ્થિર અક્ષર *unmakemask (પૂર્ણાંક mask, અક્ષર *smask) {
  પૂર્ણાંક i = 0;
  જો (mask & LUA_MASKCALL) smask[i++] = 'c';
  જો (mask & LUA_MASKRET) smask[i++] = 'r';
  જો (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  પાછા smask;
}


સ્થિર પૂર્ણાંક db_sethook (lua_State *L) {
  પૂર્ણાંક arg, mask, ગણતરી;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  જો (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = ખાલી; mask = 0; ગણતરી = 0;  /* turn off hooks */
  }
  નહીંતર {
    અચલ અક્ષર *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    ગણતરી = (પૂર્ણાંક)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, ગણતરી);
  }
  જો (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, ગણતરી);
  પાછા 0;
}


સ્થિર પૂર્ણાંક db_gethook (lua_State *L) {
  પૂર્ણાંક arg;
  lua_State *L1 = getthread(L, &arg);
  અક્ષર buff[5];
  પૂર્ણાંક mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  જો (hook == ખાલી) {  /* no hook? */
    luaL_pushfail(L);
    પાછા 1;
  }
  નહીંતર જો (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  નહીંતર {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  પાછા 3;
}


સ્થિર પૂર્ણાંક db_debug (lua_State *L) {
  માટે (;;) {
    અક્ષર buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    જો (fgets(buffer, કદ(buffer), stdin) == ખાલી ||
        strcmp(buffer, "cont\n") == 0)
      પાછા 0;
    જો (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, ખાલી));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


સ્થિર પૂર્ણાંક db_traceback (lua_State *L) {
  પૂર્ણાંક arg;
  lua_State *L1 = getthread(L, &arg);
  અચલ અક્ષર *સંદેશ = lua_tostring(L, arg + 1);
  જો (સંદેશ == ખાલી && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  નહીંતર {
    પૂર્ણાંક level = (પૂર્ણાંક)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, સંદેશ, level);
  }
  પાછા 1;
}


સ્થિર અચલ luaL_Reg dblib[] = {
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
  {ખાલી, ખાલી}
};


LUAMOD_API પૂર્ણાંક luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  પાછા 1;
}

