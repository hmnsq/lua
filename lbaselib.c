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


स्थिर पूर्णाङ्क luaB_print (lua_State *L) {
  पूर्णाङ्क n = lua_gettop(L);  /* number of arguments */
  पूर्णाङ्क i;
  अत्थाय (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    ध्रुव वर्ण *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    सचे (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_निष्कासय(L, 1);  /* pop result */
  }
  lua_writeline();
  पटिनिवत्त 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
स्थिर पूर्णाङ्क luaB_warn (lua_State *L) {
  पूर्णाङ्क n = lua_gettop(L);  /* number of arguments */
  पूर्णाङ्क i;
  luaL_checkstring(L, 1);  /* at least one argument */
  अत्थाय (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  अत्थाय (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  पटिनिवत्त 0;
}


#define SPACECHARS	" \f\n\r\t\v"

स्थिर ध्रुव वर्ण *b_str2int (ध्रुव वर्ण *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  पूर्णाङ्क neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  सचे (*s == '-') { s++; neg = 1; }  /* handle sign */
  अञ्ञथा सचे (*s == '+') s++;
  सचे (!isalnum(cast_uchar(*s)))  /* no digit? */
    पटिनिवत्त रिक्त;
  करोहि {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    सचे (digit >= base) पटिनिवत्त रिक्त;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } याव (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  पटिनिवत्त s;
}


स्थिर पूर्णाङ्क luaB_tonumber (lua_State *L) {
  सचे (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    सचे (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      पटिनिवत्त 1;
    }
    अञ्ञथा {
      size_t l;
      ध्रुव वर्ण *s = lua_tolstring(L, 1, &l);
      सचे (s != रिक्त && lua_stringtonumber(L, s) == l + 1)
        पटिनिवत्त 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  अञ्ञथा {
    size_t l;
    ध्रुव वर्ण *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    सचे (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      पटिनिवत्त 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क luaB_error (lua_State *L) {
  पूर्णाङ्क level = (पूर्णाङ्क)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  सचे (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  पटिनिवत्त lua_error(L);
}


स्थिर पूर्णाङ्क luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  सचे (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    पटिनिवत्त 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  पटिनिवत्त 1;  /* returns either __metatable field (if present) or metatable */
}


स्थिर पूर्णाङ्क luaB_setmetatable (lua_State *L) {
  पूर्णाङ्क t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  सचे (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    पटिनिवत्त luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क luaB_rawlen (lua_State *L) {
  पूर्णाङ्क t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  पटिनिवत्त 1;
}

स्थिर पूर्णाङ्क luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क pushmode (lua_State *L, पूर्णाङ्क oldmode) {
  सचे (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  अञ्ञथा
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  पटिनिवत्त 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { सचे (res == -1) भङ्ग; }

स्थिर पूर्णाङ्क luaB_collectgarbage (lua_State *L) {
  स्थिर ध्रुव वर्ण *ध्रुव opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", रिक्त};
  स्थिर ध्रुव वर्ण optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  पूर्णाङ्क o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  विकल्प (o) {
    पक्ष LUA_GCCOUNT: {
      पूर्णाङ्क k = lua_gc(L, o);
      पूर्णाङ्क b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_सङ्ख्या)k + ((lua_सङ्ख्या)b/1024));
      पटिनिवत्त 1;
    }
    पक्ष LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      पूर्णाङ्क res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      पटिनिवत्त 1;
    }
    पक्ष LUA_GCISRUNNING: {
      पूर्णाङ्क res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      पटिनिवत्त 1;
    }
    पक्ष LUA_GCGEN: {
      पटिनिवत्त pushmode(L, lua_gc(L, o));
    }
    पक्ष LUA_GCINC: {
      पटिनिवत्त pushmode(L, lua_gc(L, o));
    }
    पक्ष LUA_GCPARAM: {
      स्थिर ध्रुव वर्ण *ध्रुव params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", रिक्त};
      स्थिर ध्रुव वर्ण pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      पूर्णाङ्क p = pnum[luaL_checkoption(L, 2, रिक्त, params)];
      lua_Integer मानम् = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (पूर्णाङ्क)मानम्));
      पटिनिवत्त 1;
    }
    स्वभाव: {
      पूर्णाङ्क res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      पटिनिवत्त 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क luaB_type (lua_State *L) {
  पूर्णाङ्क t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  सचे (lua_next(L, 1))
    पटिनिवत्त 2;
  अञ्ञथा {
    lua_pushnil(L);
    पटिनिवत्त 1;
  }
}


स्थिर पूर्णाङ्क pairscont (lua_State *L, पूर्णाङ्क status, lua_KContext k) {
  (शून्य)L; (शून्य)status; (शून्य)k;  /* unused */
  पटिनिवत्त 4;  /* __pairs did all the work, just return its results */
}

स्थिर पूर्णाङ्क luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  सचे (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  अञ्ञथा {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  पटिनिवत्त 4;
}


/*
** Traversal function for 'ipairs'
*/
स्थिर पूर्णाङ्क ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  पटिनिवत्त (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
स्थिर पूर्णाङ्क luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  पटिनिवत्त 3;
}


स्थिर पूर्णाङ्क load_aux (lua_State *L, पूर्णाङ्क status, पूर्णाङ्क envidx) {
  सचे (l_likely(status == LUA_OK)) {
    सचे (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      सचे (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_निष्कासय(L, 1);  /* remove 'env' if not used by previous call */
    }
    पटिनिवत्त 1;
  }
  अञ्ञथा {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_निवेशय(L, -2);  /* put before error message */
    पटिनिवत्त 2;  /* return fail plus error message */
  }
}


स्थिर ध्रुव वर्ण *getMode (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव वर्ण *mode = luaL_optstring(L, idx, रिक्त);
  सचे (mode != रिक्त && strchr(mode, 'B') != रिक्त) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  पटिनिवत्त mode;
}


स्थिर पूर्णाङ्क luaB_loadfile (lua_State *L) {
  ध्रुव वर्ण *fname = luaL_optstring(L, 1, रिक्त);
  ध्रुव वर्ण *mode = getMode(L, 2);
  पूर्णाङ्क env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  पूर्णाङ्क status = luaL_loadfilex(L, fname, mode);
  पटिनिवत्त load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
स्थिर ध्रुव वर्ण *generic_reader (lua_State *L, शून्य *ud, size_t *size) {
  पूर्णाङ्क *firstcall = cast(पूर्णाङ्क *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  सचे (*firstcall)
    *firstcall = 0;
  अञ्ञथा
    lua_निष्कासय(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  सचे (lua_isnil(L, -1)) {
    *size = 0;
    पटिनिवत्त रिक्त;
  }
  अञ्ञथा सचे (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  पटिनिवत्त lua_tolstring(L, -1, size);
}


स्थिर पूर्णाङ्क luaB_load (lua_State *L) {
  पूर्णाङ्क status;
  size_t l;
  ध्रुव वर्ण *s = lua_tolstring(L, 1, &l);
  ध्रुव वर्ण *mode = getMode(L, 3);
  पूर्णाङ्क env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  सचे (s != रिक्त) {  /* loading a string? */
    ध्रुव वर्ण *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  अञ्ञथा {  /* loading from a reader function */
    पूर्णाङ्क firstcall = 1;  /* userdata for generic_reader */
    ध्रुव वर्ण *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  पटिनिवत्त load_aux(L, status, env);
}

/* }====================================================== */


स्थिर पूर्णाङ्क dofilecont (lua_State *L, पूर्णाङ्क d1, lua_KContext d2) {
  (शून्य)d1;  (शून्य)d2;  /* only to match 'lua_Kfunction' prototype */
  पटिनिवत्त lua_gettop(L) - 1;
}


स्थिर पूर्णाङ्क luaB_dofile (lua_State *L) {
  ध्रुव वर्ण *fname = luaL_optstring(L, 1, रिक्त);
  lua_settop(L, 1);
  सचे (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    पटिनिवत्त lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  पटिनिवत्त dofilecont(L, 0, 0);
}


स्थिर पूर्णाङ्क luaB_assert (lua_State *L) {
  सचे (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    पटिनिवत्त lua_gettop(L);  /* return all arguments */
  अञ्ञथा {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    पटिनिवत्त luaB_error(L);  /* call 'error' */
  }
}


स्थिर पूर्णाङ्क luaB_select (lua_State *L) {
  पूर्णाङ्क n = lua_gettop(L);
  सचे (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    पटिनिवत्त 1;
  }
  अञ्ञथा {
    lua_Integer i = luaL_checkinteger(L, 1);
    सचे (i < 0) i = n + i;
    अञ्ञथा सचे (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    पटिनिवत्त n - (पूर्णाङ्क)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
स्थिर पूर्णाङ्क finishpcall (lua_State *L, पूर्णाङ्क status, lua_KContext extra) {
  सचे (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    पटिनिवत्त 2;  /* return false, msg */
  }
  अञ्ञथा
    पटिनिवत्त lua_gettop(L) - (पूर्णाङ्क)extra;  /* return all results */
}


स्थिर पूर्णाङ्क luaB_pcall (lua_State *L) {
  पूर्णाङ्क status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_निवेशय(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  पटिनिवत्त finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
स्थिर पूर्णाङ्क luaB_xpcall (lua_State *L) {
  पूर्णाङ्क status;
  पूर्णाङ्क n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  पटिनिवत्त finishpcall(L, status, 2);
}


स्थिर पूर्णाङ्क luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, रिक्त);
  पटिनिवत्त 1;
}


स्थिर ध्रुव luaL_Reg base_funcs[] = {
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


LUAMOD_API पूर्णाङ्क luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  पटिनिवत्त 1;
}

