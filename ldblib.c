/*
** $Id: ldblib.c $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

#define ldblib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
ସ୍ଥିର ଶୂନ୍ୟ checkstack (lua_State *L, lua_State *L1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ଯଦି (l_unlikely(L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_getregistry (lua_State *L) {
  lua_pushvalue(L, LUA_REGISTRYINDEX);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ଯଦି (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);  /* no metatable */
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_setmetatable (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = lua_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ଫେରନ୍ତୁ 1;  /* return 1st argument */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_getuservalue (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_optinteger(L, 2, 1);
  ଯଦି (lua_type(L, 1) != LUA_TUSERDATA)
    luaL_pushfail(L);
  ଅନ୍ୟଥା ଯଦି (lua_getiuservalue(L, 1, n) != LUA_TNONE) {
    lua_pushboolean(L, 1);
    ଫେରନ୍ତୁ 2;
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_setuservalue (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  ଯଦି (!lua_setiuservalue(L, 1, n))
    luaL_pushfail(L);
  ଫେରନ୍ତୁ 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
ସ୍ଥିର lua_State *getthread (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *arg) {
  ଯଦି (lua_isthread(L, 1)) {
    *arg = 1;
    ଫେରନ୍ତୁ lua_tothread(L, 1);
  }
  ଅନ୍ୟଥା {
    *arg = 0;
    ଫେରନ୍ତୁ L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'lua_settable', used by 'db_getinfo' to put results
** from 'lua_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
ସ୍ଥିର ଶୂନ୍ୟ settabss (lua_State *L, ଅଚଳ ଅକ୍ଷର *k, ଅଚଳ ଅକ୍ଷର *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

ସ୍ଥିର ଶୂନ୍ୟ settabsi (lua_State *L, ଅଚଳ ଅକ୍ଷର *k, ପୂର୍ଣ୍ଣସଂଖ୍ୟା v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

ସ୍ଥିର ଶୂନ୍ୟ settabsb (lua_State *L, ଅଚଳ ଅକ୍ଷର *k, ପୂର୍ଣ୍ଣସଂଖ୍ୟା v) {
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
ସ୍ଥିର ଶୂନ୍ୟ treatstackoption (lua_State *L, lua_State *L1, ଅଚଳ ଅକ୍ଷର *fname) {
  ଯଦି (L == L1)
    lua_rotate(L, -2, 1);  /* exchange object and table */
  ଅନ୍ୟଥା
    lua_xmove(L1, L, 1);  /* move object to the "main" stack */
  lua_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'lua_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'lua_getinfo'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_getinfo (lua_State *L) {
  lua_Debug ar;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg;
  lua_State *L1 = getthread(L, &arg);
  ଅଚଳ ଅକ୍ଷର *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  ଯଦି (lua_isfunction(L, arg + 1)) {  /* info about a function? */
    options = lua_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    lua_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    lua_xmove(L, L1, 1);
  }
  ଅନ୍ୟଥା {  /* stack level */
    ଯଦି (!lua_getstack(L1, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, arg + 1), &ar)) {
      luaL_pushfail(L);  /* level out of range */
      ଫେରନ୍ତୁ 1;
    }
  }
  ଯଦି (!lua_getinfo(L1, options, &ar))
    ଫେରନ୍ତୁ luaL_argerror(L, arg+2, "invalid option");
  lua_newtable(L);  /* table to collect results */
  ଯଦି (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  ଯଦି (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  ଯଦି (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  ଯଦି (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  ଯଦି (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  ଯଦି (strchr(options, 't')) {
    settabsb(L, "istailcall", ar.istailcall);
    settabsi(L, "extraargs", ar.extraargs);
  }
  ଯଦି (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  ଯଦି (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  ଫେରନ୍ତୁ 1;  /* return table */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_getlocal (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg;
  lua_State *L1 = getthread(L, &arg);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvar = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  ଯଦି (lua_isfunction(L, arg + 1)) {  /* function argument? */
    lua_pushvalue(L, arg + 1);  /* push function */
    lua_pushstring(L, lua_getlocal(L, ଖାଲି, nvar));  /* push local name */
    ଫେରନ୍ତୁ 1;  /* return only name (there is no value) */
  }
  ଅନ୍ୟଥା {  /* stack-level argument */
    lua_Debug ar;
    ଅଚଳ ଅକ୍ଷର *name;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, arg + 1);
    ଯଦି (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
      ଫେରନ୍ତୁ luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    ଯଦି (name) {
      lua_xmove(L1, L, 1);  /* move local value */
      lua_pushstring(L, name);  /* push name */
      lua_rotate(L, -2, 1);  /* re-order */
      ଫେରନ୍ତୁ 2;
    }
    ଅନ୍ୟଥା {
      luaL_pushfail(L);  /* no name (nor value) */
      ଫେରନ୍ତୁ 1;
    }
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_setlocal (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg;
  ଅଚଳ ଅକ୍ଷର *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, arg + 1);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nvar = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, arg + 2);
  ଯଦି (l_unlikely(!lua_getstack(L1, level, &ar)))  /* out of range? */
    ଫେରନ୍ତୁ luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  ଯଦି (name == ଖାଲି)
    lua_କାଢ଼(L1, 1);  /* pop value (if not popped by 'lua_setlocal') */
  lua_pushstring(L, name);
  ଫେରନ୍ତୁ 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା auxupvalue (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା get) {
  ଅଚଳ ଅକ୍ଷର *name;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  ଯଦି (name == ଖାଲି) ଫେରନ୍ତୁ 0;
  lua_pushstring(L, name);
  lua_ଭର୍ତ୍ତି(L, -(get+1));  /* no-op if get is false */
  ଫେରନ୍ତୁ get + 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_getupvalue (lua_State *L) {
  ଫେରନ୍ତୁ auxupvalue(L, 1);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  ଫେରନ୍ତୁ auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
ସ୍ଥିର ଶୂନ୍ୟ *checkupval (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argnup, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnup) {
  ଶୂନ୍ୟ *id;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nup = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = lua_upvalueid(L, argf, nup);
  ଯଦି (pnup) {
    luaL_argcheck(L, id != ଖାଲି, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  ଫେରନ୍ତୁ id;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_upvalueid (lua_State *L) {
  ଶୂନ୍ୟ *id = checkupval(L, 1, 2, ଖାଲି);
  ଯଦି (id != ଖାଲି)
    lua_pushlightuserdata(L, id);
  ଅନ୍ୟଥା
    luaL_pushfail(L);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_upvaluejoin (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !lua_iscfunction(L, 1), 1, "Lua function expected");
  luaL_argcheck(L, !lua_iscfunction(L, 3), 3, "Lua function expected");
  lua_upvaluejoin(L, 1, n1, 3, n2);
  ଫେରନ୍ତୁ 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
ସ୍ଥିର ଶୂନ୍ୟ hookf (lua_State *L, lua_Debug *ar) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  lua_pushthread(L);
  ଯଦି (lua_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    lua_pushstring(L, hooknames[(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ar->event]);  /* push event name */
    ଯଦି (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);  /* push current line */
    ଅନ୍ୟଥା lua_pushnil(L);
    lua_assert(lua_getinfo(L, "lS", ar));
    lua_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା makemask (ଅଚଳ ଅକ୍ଷର *smask, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = 0;
  ଯଦି (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  ଯଦି (strchr(smask, 'r')) mask |= LUA_MASKRET;
  ଯଦି (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  ଯଦି (ଗଣନା_2 > 0) mask |= LUA_MASKCOUNT;
  ଫେରନ୍ତୁ mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
ସ୍ଥିର ଅକ୍ଷର *unmakemask (ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask, ଅକ୍ଷର *smask) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
  ଯଦି (mask & LUA_MASKCALL) smask[i++] = 'c';
  ଯଦି (mask & LUA_MASKRET) smask[i++] = 'r';
  ଯଦି (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  ଫେରନ୍ତୁ smask;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_sethook (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, mask, ଗଣନା_2;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  ଯଦି (lua_isnoneornil(L, arg+1)) {  /* no hook? */
    lua_settop(L, arg+1);
    func = ଖାଲି; mask = 0; ଗଣନା_2 = 0;  /* turn off hooks */
  }
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    ଗଣନା_2 = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, ଗଣନା_2);
  }
  ଯଦି (!luaL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
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
  lua_sethook(L1, func, mask, ଗଣନା_2);
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_gethook (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg;
  lua_State *L1 = getthread(L, &arg);
  ଅକ୍ଷର buff[5];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  ଯଦି (hook == ଖାଲି) {  /* no hook? */
    luaL_pushfail(L);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା ଯଦି (hook != hookf)  /* external hook? */
    lua_pushliteral(L, "external hook");
  ଅନ୍ୟଥା {  /* hook table must exist */
    lua_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);   /* 1st result = hooktable[L1] */
    lua_remove(L, -2);  /* remove hook table */
  }
  lua_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  lua_pushinteger(L, lua_gethookcount(L1));  /* 3rd result = count */
  ଫେରନ୍ତୁ 3;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_debug (lua_State *L) {
  ପାଇଁ (;;) {
    ଅକ୍ଷର buffer[250];
    lua_writestringerror("%s", "lua_debug> ");
    ଯଦି (fgets(buffer, ଆକାର(buffer), stdin) == ଖାଲି ||
        strcmp(buffer, "cont\n") == 0)
      ଫେରନ୍ତୁ 0;
    ଯଦି (luaL_loadbufferx(L, buffer, strlen(buffer), "=(debug command)", "t") ||
        lua_pcall(L, 0, 0, 0))
      lua_writestringerror("%s\n", luaL_tolstring(L, -1, ଖାଲି));
    lua_settop(L, 0);  /* remove eventual returns */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା db_traceback (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg;
  lua_State *L1 = getthread(L, &arg);
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = lua_tostring(L, arg + 1);
  ଯଦି (ସନ୍ଦେଶ == ଖାଲି && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    lua_pushvalue(L, arg + 1);  /* return it untouched */
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, ସନ୍ଦେଶ, level);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଅଚଳ luaL_Reg dblib[] = {
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
  {ଖାଲି, ଖାଲି}
};


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_debug (lua_State *L) {
  luaL_newlib(L, dblib);
  ଫେରନ୍ତୁ 1;
}

