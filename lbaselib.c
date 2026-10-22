/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#నిర్వచించు lbaselib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


స్థిర పూర్ణాంకం luaB_print (lua_State *L) {
  పూర్ణాంకం n = lua_gettop(L);  /* number of arguments */
  పూర్ణాంకం i;
  కొరకు (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    స్థిరరాశి అక్షరం *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    ఒకవేళ (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_తీయి(L, 1);  /* pop result */
  }
  lua_writeline();
  తిరిగి 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
స్థిర పూర్ణాంకం luaB_warn (lua_State *L) {
  పూర్ణాంకం n = lua_gettop(L);  /* number of arguments */
  పూర్ణాంకం i;
  luaL_checkstring(L, 1);  /* at least one argument */
  కొరకు (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  కొరకు (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  తిరిగి 0;
}


#నిర్వచించు SPACECHARS	" \f\n\r\t\v"

స్థిర స్థిరరాశి అక్షరం *b_str2int (స్థిరరాశి అక్షరం *s, గుర్తులేని పునాది, lua_Integer *pn) {
  lua_Unsigned n = 0;
  పూర్ణాంకం neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  ఒకవేళ (*s == '-') { s++; neg = 1; }  /* handle sign */
  లేకపోతే ఒకవేళ (*s == '+') s++;
  ఒకవేళ (!అక్షరమా_అంకెనా(cast_uchar(*s)))  /* no digit? */
    తిరిగి ఖాళీ;
  చేయి {
    గుర్తులేని digit = cast_uint(అంకెనా(cast_uchar(*s))
                               ? *s - '0'
                               : (పెద్ద_అక్షరం(cast_uchar(*s)) - 'A') + 10);
    ఒకవేళ (digit >= పునాది) తిరిగి ఖాళీ;  /* invalid numeral */
    n = n * పునాది + digit;
    s++;
  } అయితే (అక్షరమా_అంకెనా(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  తిరిగి s;
}


స్థిర పూర్ణాంకం luaB_tonumber (lua_State *L) {
  ఒకవేళ (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    ఒకవేళ (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      తిరిగి 1;
    }
    లేకపోతే {
      size_t l;
      స్థిరరాశి అక్షరం *s = lua_tolstring(L, 1, &l);
      ఒకవేళ (s != ఖాళీ && lua_stringtonumber(L, s) == l + 1)
        తిరిగి 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  లేకపోతే {
    size_t l;
    స్థిరరాశి అక్షరం *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer పునాది = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= పునాది && పునాది <= 36, 2, "base out of range");
    ఒకవేళ (b_str2int(s, cast_uint(పునాది), &n) == s + l) {
      lua_pushinteger(L, n);
      తిరిగి 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_error (lua_State *L) {
  పూర్ణాంకం level = (పూర్ణాంకం)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  ఒకవేళ (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  తిరిగి lua_error(L);
}


స్థిర పూర్ణాంకం luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ఒకవేళ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    తిరిగి 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  తిరిగి 1;  /* returns either __metatable field (if present) or metatable */
}


స్థిర పూర్ణాంకం luaB_setmetatable (lua_State *L) {
  పూర్ణాంకం t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  ఒకవేళ (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    తిరిగి luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_rawlen (lua_State *L) {
  పూర్ణాంకం t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  తిరిగి 1;
}

స్థిర పూర్ణాంకం luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం pushmode (lua_State *L, పూర్ణాంకం oldmode) {
  ఒకవేళ (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  లేకపోతే
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  తిరిగి 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#నిర్వచించు checkvalres(res) { ఒకవేళ (res == -1) ఆపు; }

స్థిర పూర్ణాంకం luaB_collectgarbage (lua_State *L) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", ఖాళీ};
  స్థిర స్థిరరాశి అక్షరం optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  పూర్ణాంకం o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  ఎంపిక (o) {
    స్థితి LUA_GCCOUNT: {
      పూర్ణాంకం k = lua_gc(L, o);
      పూర్ణాంకం b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_సంఖ్య)k + ((lua_సంఖ్య)b/1024));
      తిరిగి 1;
    }
    స్థితి LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      పూర్ణాంకం res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      తిరిగి 1;
    }
    స్థితి LUA_GCISRUNNING: {
      పూర్ణాంకం res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      తిరిగి 1;
    }
    స్థితి LUA_GCGEN: {
      తిరిగి pushmode(L, lua_gc(L, o));
    }
    స్థితి LUA_GCINC: {
      తిరిగి pushmode(L, lua_gc(L, o));
    }
    స్థితి LUA_GCPARAM: {
      స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", ఖాళీ};
      స్థిర స్థిరరాశి అక్షరం pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      పూర్ణాంకం p = pnum[luaL_checkoption(L, 2, ఖాళీ, params)];
      lua_Integer విలువ = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (పూర్ణాంకం)విలువ));
      తిరిగి 1;
    }
    అప్రమేయ: {
      పూర్ణాంకం res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      తిరిగి 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_type (lua_State *L) {
  పూర్ణాంకం t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం lua_B_తదుపరి (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  ఒకవేళ (lua_తదుపరి(L, 1))
    తిరిగి 2;
  లేకపోతే {
    lua_pushnil(L);
    తిరిగి 1;
  }
}


స్థిర పూర్ణాంకం pairscont (lua_State *L, పూర్ణాంకం స్థితి_2, lua_KContext k) {
  (శూన్య)L; (శూన్య)స్థితి_2; (శూన్య)k;  /*వాడని*/
  తిరిగి 4;  /* __pairs did all the work, just return its results */
}

స్థిర పూర్ణాంకం lua_B_జతలు (lua_State *L) {
  luaL_checkany(L, 1);
  ఒకవేళ (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, lua_B_తదుపరి);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  లేకపోతే {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  తిరిగి 4;
}


/*
** Traversal function for 'ipairs'
*/
స్థిర పూర్ణాంకం ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  తిరిగి (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
స్థిర పూర్ణాంకం luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  తిరిగి 3;
}


స్థిర పూర్ణాంకం ఎక్కించు_aux (lua_State *L, పూర్ణాంకం స్థితి_2, పూర్ణాంకం envidx) {
  ఒకవేళ (l_likely(స్థితి_2 == LUA_OK)) {
    ఒకవేళ (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      ఒకవేళ (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_తీయి(L, 1);  /* remove 'env' if not used by previous call */
    }
    తిరిగి 1;
  }
  లేకపోతే {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_చొప్పించు(L, -2);  /* put before error message */
    తిరిగి 2;  /* return fail plus error message */
  }
}


స్థిర స్థిరరాశి అక్షరం *get_బహుళకం (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి అక్షరం *బహుళకం = luaL_optstring(L, idx, ఖాళీ);
  ఒకవేళ (బహుళకం != ఖాళీ && strchr(బహుళకం, 'B') != ఖాళీ) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  తిరిగి బహుళకం;
}


స్థిర పూర్ణాంకం luaB_loadfile (lua_State *L) {
  స్థిరరాశి అక్షరం *fname = luaL_optstring(L, 1, ఖాళీ);
  స్థిరరాశి అక్షరం *బహుళకం = get_బహుళకం(L, 2);
  పూర్ణాంకం env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  పూర్ణాంకం స్థితి_2 = luaL_loadfilex(L, fname, బహుళకం);
  తిరిగి ఎక్కించు_aux(L, స్థితి_2, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
స్థిర స్థిరరాశి అక్షరం *generic_reader (lua_State *L, శూన్య *ud, size_t *కొలత) {
  పూర్ణాంకం *firstcall = cast(పూర్ణాంకం *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  ఒకవేళ (*firstcall)
    *firstcall = 0;
  లేకపోతే
    lua_తీయి(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  ఒకవేళ (lua_isnil(L, -1)) {
    *కొలత = 0;
    తిరిగి ఖాళీ;
  }
  లేకపోతే ఒకవేళ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  తిరిగి lua_tolstring(L, -1, కొలత);
}


స్థిర పూర్ణాంకం lua_B_ఎక్కించు (lua_State *L) {
  పూర్ణాంకం స్థితి_2;
  size_t l;
  స్థిరరాశి అక్షరం *s = lua_tolstring(L, 1, &l);
  స్థిరరాశి అక్షరం *బహుళకం = get_బహుళకం(L, 3);
  పూర్ణాంకం env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  ఒకవేళ (s != ఖాళీ) {  /* loading a string? */
    స్థిరరాశి అక్షరం *chunkname = luaL_optstring(L, 2, s);
    స్థితి_2 = luaL_loadbufferx(L, s, l, chunkname, బహుళకం);
  }
  లేకపోతే {  /* loading from a reader function */
    పూర్ణాంకం firstcall = 1;  /* userdata for generic_reader */
    స్థిరరాశి అక్షరం *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    స్థితి_2 = lua_ఎక్కించు(L, generic_reader, &firstcall, chunkname, బహుళకం);
  }
  తిరిగి ఎక్కించు_aux(L, స్థితి_2, env);
}

/* }====================================================== */


స్థిర పూర్ణాంకం dofilecont (lua_State *L, పూర్ణాంకం d1, lua_KContext d2) {
  (శూన్య)d1;  (శూన్య)d2;  /* only to match 'lua_Kfunction' prototype */
  తిరిగి lua_gettop(L) - 1;
}


స్థిర పూర్ణాంకం luaB_dofile (lua_State *L) {
  స్థిరరాశి అక్షరం *fname = luaL_optstring(L, 1, ఖాళీ);
  lua_settop(L, 1);
  ఒకవేళ (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    తిరిగి lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  తిరిగి dofilecont(L, 0, 0);
}


స్థిర పూర్ణాంకం luaB_assert (lua_State *L) {
  ఒకవేళ (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    తిరిగి lua_gettop(L);  /* return all arguments */
  లేకపోతే {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    తిరిగి luaB_error(L);  /* call 'error' */
  }
}


స్థిర పూర్ణాంకం luaB_select (lua_State *L) {
  పూర్ణాంకం n = lua_gettop(L);
  ఒకవేళ (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    తిరిగి 1;
  }
  లేకపోతే {
    lua_Integer i = luaL_checkinteger(L, 1);
    ఒకవేళ (i < 0) i = n + i;
    లేకపోతే ఒకవేళ (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    తిరిగి n - (పూర్ణాంకం)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
స్థిర పూర్ణాంకం finishpcall (lua_State *L, పూర్ణాంకం స్థితి_2, lua_KContext extra) {
  ఒకవేళ (l_unlikely(స్థితి_2 != LUA_OK && స్థితి_2 != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    తిరిగి 2;  /* return false, msg */
  }
  లేకపోతే
    తిరిగి lua_gettop(L) - (పూర్ణాంకం)extra;  /* return all results */
}


స్థిర పూర్ణాంకం luaB_pcall (lua_State *L) {
  పూర్ణాంకం స్థితి_2;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_చొప్పించు(L, 1);  /* put it in place */
  స్థితి_2 = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  తిరిగి finishpcall(L, స్థితి_2, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
స్థిర పూర్ణాంకం luaB_xpcall (lua_State *L) {
  పూర్ణాంకం స్థితి_2;
  పూర్ణాంకం n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  స్థితి_2 = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  తిరిగి finishpcall(L, స్థితి_2, 2);
}


స్థిర పూర్ణాంకం luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, ఖాళీ);
  తిరిగి 1;
}


స్థిర స్థిరరాశి luaL_Reg పునాది_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", lua_B_ఎక్కించు},
  {"next", lua_B_తదుపరి},
  {"pairs", lua_B_జతలు},
  {"pcall", luaB_pcall},
  {"print", luaB_print},
  {"warn", luaB_warn},
  {"rawequal", luaB_rawequal},
  {"rawlen", luaB_rawlen},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"select", luaB_select},
  {"setmetatable", luaB_setmetatable},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"type", luaB_type},
  {"xpcall", luaB_xpcall},
  /* placeholders */
  {LUA_GNAME, ఖాళీ},
  {"_VERSION", ఖాళీ},
  {ఖాళీ, ఖాళీ}
};


LUAMOD_API పూర్ణాంకం luaopen_పునాది (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, పునాది_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_సంస్కరణ);
  lua_setfield(L, -2, "_VERSION");
  తిరిగి 1;
}

