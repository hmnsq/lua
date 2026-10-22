/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <ctype.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


स्थिर पूर्णांक luaB_print (lua_State *L) {
  पूर्णांक n = lua_gettop(L);  /* number of arguments */
  पूर्णांक i;
  साठी (i = 1; i <= n; i++) {  /* for each argument */
    माप_प्रकार l;
    स्थिरांक अक्षर *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    जर (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_काढा(L, 1);  /* pop result */
  }
  lua_writeline();
  परत 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
स्थिर पूर्णांक luaB_warn (lua_State *L) {
  पूर्णांक n = lua_gettop(L);  /* number of arguments */
  पूर्णांक i;
  luaL_checkstring(L, 1);  /* at least one argument */
  साठी (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  साठी (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  परत 0;
}


#define SPACECHARS	" \f\n\r\t\v"

स्थिर स्थिरांक अक्षर *b_str2int (स्थिरांक अक्षर *s, अचिन्हित base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  पूर्णांक neg = 0;
  s += म_अखोज(s, SPACECHARS);  /* skip initial spaces */
  जर (*s == '-') { s++; neg = 1; }  /* handle sign */
  अन्यथा जर (*s == '+') s++;
  जर (!है_अक्षर_अंक(cast_uchar(*s)))  /* no digit? */
    परत रिक्त;
  करा {
    अचिन्हित digit = cast_uint(है_अंक(cast_uchar(*s))
                               ? *s - '0'
                               : (बड़े(cast_uchar(*s)) - 'A') + 10);
    जर (digit >= base) परत रिक्त;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } पर्यंत (है_अक्षर_अंक(cast_uchar(*s)));
  s += म_अखोज(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  परत s;
}


स्थिर पूर्णांक luaB_tonumber (lua_State *L) {
  जर (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    जर (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      परत 1;
    }
    अन्यथा {
      माप_प्रकार l;
      स्थिरांक अक्षर *s = lua_tolstring(L, 1, &l);
      जर (s != रिक्त && lua_stringtonumber(L, s) == l + 1)
        परत 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  अन्यथा {
    माप_प्रकार l;
    स्थिरांक अक्षर *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    जर (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      परत 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  परत 1;
}


स्थिर पूर्णांक luaB_error (lua_State *L) {
  पूर्णांक level = (पूर्णांक)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  जर (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  परत lua_error(L);
}


स्थिर पूर्णांक luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  जर (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    परत 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  परत 1;  /* returns either __metatable field (if present) or metatable */
}


स्थिर पूर्णांक luaB_setmetatable (lua_State *L) {
  पूर्णांक t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  जर (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    परत luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  परत 1;
}


स्थिर पूर्णांक luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  परत 1;
}


स्थिर पूर्णांक luaB_rawlen (lua_State *L) {
  पूर्णांक t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  परत 1;
}


स्थिर पूर्णांक luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  परत 1;
}

स्थिर पूर्णांक luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  परत 1;
}


स्थिर पूर्णांक pushmode (lua_State *L, पूर्णांक oldmode) {
  जर (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  अन्यथा
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  परत 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { जर (res == -1) थांबा; }

स्थिर पूर्णांक luaB_collectgarbage (lua_State *L) {
  स्थिर स्थिरांक अक्षर *स्थिरांक opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", रिक्त};
  स्थिर स्थिरांक अक्षर optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  पूर्णांक o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  निवड (o) {
    स्थिती LUA_GCCOUNT: {
      पूर्णांक k = lua_gc(L, o);
      पूर्णांक b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_संख्या)k + ((lua_संख्या)b/1024));
      परत 1;
    }
    स्थिती LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      पूर्णांक res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      परत 1;
    }
    स्थिती LUA_GCISRUNNING: {
      पूर्णांक res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      परत 1;
    }
    स्थिती LUA_GCGEN: {
      परत pushmode(L, lua_gc(L, o));
    }
    स्थिती LUA_GCINC: {
      परत pushmode(L, lua_gc(L, o));
    }
    स्थिती LUA_GCPARAM: {
      स्थिर स्थिरांक अक्षर *स्थिरांक params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", रिक्त};
      स्थिर स्थिरांक अक्षर pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      पूर्णांक p = pnum[luaL_checkoption(L, 2, रिक्त, params)];
      lua_Integer मूल्य = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (पूर्णांक)मूल्य));
      परत 1;
    }
    पूर्वनिर्धारित: {
      पूर्णांक res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      परत 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  परत 1;
}


स्थिर पूर्णांक luaB_type (lua_State *L) {
  पूर्णांक t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  परत 1;
}


स्थिर पूर्णांक luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  जर (lua_next(L, 1))
    परत 2;
  अन्यथा {
    lua_pushnil(L);
    परत 1;
  }
}


स्थिर पूर्णांक pairscont (lua_State *L, पूर्णांक status, lua_KContext k) {
  (शून्य)L; (शून्य)status; (शून्य)k;  /* unused */
  परत 4;  /* __pairs did all the work, just return its results */
}

स्थिर पूर्णांक luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  जर (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  अन्यथा {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  परत 4;
}


/*
** Traversal function for 'ipairs'
*/
स्थिर पूर्णांक ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  परत (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
स्थिर पूर्णांक luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  परत 3;
}


स्थिर पूर्णांक load_aux (lua_State *L, पूर्णांक status, पूर्णांक envidx) {
  जर (l_likely(status == LUA_OK)) {
    जर (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      जर (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_काढा(L, 1);  /* remove 'env' if not used by previous call */
    }
    परत 1;
  }
  अन्यथा {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_घाला(L, -2);  /* put before error message */
    परत 2;  /* return fail plus error message */
  }
}


स्थिर स्थिरांक अक्षर *getMode (lua_State *L, पूर्णांक idx) {
  स्थिरांक अक्षर *mode = luaL_optstring(L, idx, रिक्त);
  जर (mode != रिक्त && म_अक्षर(mode, 'B') != रिक्त) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  परत mode;
}


स्थिर पूर्णांक luaB_loadfile (lua_State *L) {
  स्थिरांक अक्षर *fname = luaL_optstring(L, 1, रिक्त);
  स्थिरांक अक्षर *mode = getMode(L, 2);
  पूर्णांक env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  पूर्णांक status = luaL_loadfilex(L, fname, mode);
  परत load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
स्थिर स्थिरांक अक्षर *generic_reader (lua_State *L, शून्य *ud, माप_प्रकार *size) {
  पूर्णांक *firstcall = cast(पूर्णांक *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  जर (*firstcall)
    *firstcall = 0;
  अन्यथा
    lua_काढा(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  जर (lua_isnil(L, -1)) {
    *size = 0;
    परत रिक्त;
  }
  अन्यथा जर (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  परत lua_tolstring(L, -1, size);
}


स्थिर पूर्णांक luaB_load (lua_State *L) {
  पूर्णांक status;
  माप_प्रकार l;
  स्थिरांक अक्षर *s = lua_tolstring(L, 1, &l);
  स्थिरांक अक्षर *mode = getMode(L, 3);
  पूर्णांक env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  जर (s != रिक्त) {  /* loading a string? */
    स्थिरांक अक्षर *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  अन्यथा {  /* loading from a reader function */
    पूर्णांक firstcall = 1;  /* userdata for generic_reader */
    स्थिरांक अक्षर *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  परत load_aux(L, status, env);
}

/* }====================================================== */


स्थिर पूर्णांक dofilecont (lua_State *L, पूर्णांक d1, lua_KContext d2) {
  (शून्य)d1;  (शून्य)d2;  /* only to match 'lua_Kfunction' prototype */
  परत lua_gettop(L) - 1;
}


स्थिर पूर्णांक luaB_dofile (lua_State *L) {
  स्थिरांक अक्षर *fname = luaL_optstring(L, 1, रिक्त);
  lua_settop(L, 1);
  जर (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    परत lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  परत dofilecont(L, 0, 0);
}


स्थिर पूर्णांक luaB_assert (lua_State *L) {
  जर (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    परत lua_gettop(L);  /* return all arguments */
  अन्यथा {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    परत luaB_error(L);  /* call 'error' */
  }
}


स्थिर पूर्णांक luaB_select (lua_State *L) {
  पूर्णांक n = lua_gettop(L);
  जर (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    परत 1;
  }
  अन्यथा {
    lua_Integer i = luaL_checkinteger(L, 1);
    जर (i < 0) i = n + i;
    अन्यथा जर (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    परत n - (पूर्णांक)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
स्थिर पूर्णांक finishpcall (lua_State *L, पूर्णांक status, lua_KContext extra) {
  जर (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    परत 2;  /* return false, msg */
  }
  अन्यथा
    परत lua_gettop(L) - (पूर्णांक)extra;  /* return all results */
}


स्थिर पूर्णांक luaB_pcall (lua_State *L) {
  पूर्णांक status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_घाला(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  परत finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
स्थिर पूर्णांक luaB_xpcall (lua_State *L) {
  पूर्णांक status;
  पूर्णांक n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  परत finishpcall(L, status, 2);
}


स्थिर पूर्णांक luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, रिक्त);
  परत 1;
}


स्थिर स्थिरांक luaL_Reg base_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", luaB_load},
  {"next", luaB_next},
  {"pairs", luaB_pairs},
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
  {LUA_GNAME, रिक्त},
  {"_VERSION", रिक्त},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णांक luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  परत 1;
}

