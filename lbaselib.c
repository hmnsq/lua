/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#شامل "lprefix.h"


#شامل <ctype.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


ایستا صحیح luaB_print (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  صحیح i;
  برای (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    ثابت نویسه *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    اگر (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_برداشت(L, 1);  /* pop result */
  }
  lua_writeline();
  بازگشت 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
ایستا صحیح luaB_warn (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  صحیح i;
  luaL_checkstring(L, 1);  /* at least one argument */
  برای (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  برای (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  بازگشت 0;
}


#define SPACECHARS	" \f\n\r\t\v"

ایستا ثابت نویسه *b_str2int (ثابت نویسه *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  صحیح neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  اگر (*s == '-') { s++; neg = 1; }  /* handle sign */
  وگرنه اگر (*s == '+') s++;
  اگر (!isalnum(cast_uchar(*s)))  /* no digit? */
    بازگشت پوچ;
  انجامبده {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    اگر (digit >= base) بازگشت پوچ;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } وقتی (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  بازگشت s;
}


ایستا صحیح luaB_tonumber (lua_State *L) {
  اگر (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    اگر (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      بازگشت 1;
    }
    وگرنه {
      size_t l;
      ثابت نویسه *s = lua_tolstring(L, 1, &l);
      اگر (s != پوچ && lua_stringtonumber(L, s) == l + 1)
        بازگشت 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  وگرنه {
    size_t l;
    ثابت نویسه *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    اگر (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      بازگشت 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  بازگشت 1;
}


ایستا صحیح luaB_error (lua_State *L) {
  صحیح level = (صحیح)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  اگر (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  بازگشت lua_error(L);
}


ایستا صحیح luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  اگر (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    بازگشت 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  بازگشت 1;  /* returns either __metatable field (if present) or metatable */
}


ایستا صحیح luaB_setmetatable (lua_State *L) {
  صحیح t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  اگر (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    بازگشت luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  بازگشت 1;
}


ایستا صحیح luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  بازگشت 1;
}


ایستا صحیح luaB_rawlen (lua_State *L) {
  صحیح t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  بازگشت 1;
}


ایستا صحیح luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  بازگشت 1;
}

ایستا صحیح luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  بازگشت 1;
}


ایستا صحیح pushmode (lua_State *L, صحیح oldmode) {
  اگر (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  وگرنه
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  بازگشت 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { اگر (res == -1) بشکن; }

ایستا صحیح luaB_collectgarbage (lua_State *L) {
  ایستا ثابت نویسه *ثابت opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", پوچ};
  ایستا ثابت نویسه optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  صحیح o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  انتخاب (o) {
    حالت LUA_GCCOUNT: {
      صحیح k = lua_gc(L, o);
      صحیح b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_عدد)k + ((lua_عدد)b/1024));
      بازگشت 1;
    }
    حالت LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      صحیح res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      بازگشت 1;
    }
    حالت LUA_GCISRUNNING: {
      صحیح res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      بازگشت 1;
    }
    حالت LUA_GCGEN: {
      بازگشت pushmode(L, lua_gc(L, o));
    }
    حالت LUA_GCINC: {
      بازگشت pushmode(L, lua_gc(L, o));
    }
    حالت LUA_GCPARAM: {
      ایستا ثابت نویسه *ثابت params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", پوچ};
      ایستا ثابت نویسه pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      صحیح p = pnum[luaL_checkoption(L, 2, پوچ, params)];
      lua_Integer مقدار = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (صحیح)مقدار));
      بازگشت 1;
    }
    پیشفرض: {
      صحیح res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      بازگشت 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  بازگشت 1;
}


ایستا صحیح luaB_type (lua_State *L) {
  صحیح t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  بازگشت 1;
}


ایستا صحیح luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  اگر (lua_next(L, 1))
    بازگشت 2;
  وگرنه {
    lua_pushnil(L);
    بازگشت 1;
  }
}


ایستا صحیح pairscont (lua_State *L, صحیح status, lua_KContext k) {
  (تهی)L; (تهی)status; (تهی)k;  /* unused */
  بازگشت 4;  /* __pairs did all the work, just return its results */
}

ایستا صحیح luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  اگر (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  وگرنه {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  بازگشت 4;
}


/*
** Traversal function for 'ipairs'
*/
ایستا صحیح ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  بازگشت (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
ایستا صحیح luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  بازگشت 3;
}


ایستا صحیح load_aux (lua_State *L, صحیح status, صحیح envidx) {
  اگر (l_likely(status == LUA_OK)) {
    اگر (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      اگر (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_برداشت(L, 1);  /* remove 'env' if not used by previous call */
    }
    بازگشت 1;
  }
  وگرنه {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_درج(L, -2);  /* put before error message */
    بازگشت 2;  /* return fail plus error message */
  }
}


ایستا ثابت نویسه *getMode (lua_State *L, صحیح idx) {
  ثابت نویسه *mode = luaL_optstring(L, idx, پوچ);
  اگر (mode != پوچ && strchr(mode, 'B') != پوچ) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  بازگشت mode;
}


ایستا صحیح luaB_loadfile (lua_State *L) {
  ثابت نویسه *fname = luaL_optstring(L, 1, پوچ);
  ثابت نویسه *mode = getMode(L, 2);
  صحیح env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  صحیح status = luaL_loadfilex(L, fname, mode);
  بازگشت load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
ایستا ثابت نویسه *generic_reader (lua_State *L, تهی *ud, size_t *size) {
  صحیح *firstcall = cast(صحیح *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  اگر (*firstcall)
    *firstcall = 0;
  وگرنه
    lua_برداشت(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  اگر (lua_isnil(L, -1)) {
    *size = 0;
    بازگشت پوچ;
  }
  وگرنه اگر (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  بازگشت lua_tolstring(L, -1, size);
}


ایستا صحیح luaB_load (lua_State *L) {
  صحیح status;
  size_t l;
  ثابت نویسه *s = lua_tolstring(L, 1, &l);
  ثابت نویسه *mode = getMode(L, 3);
  صحیح env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  اگر (s != پوچ) {  /* loading a string? */
    ثابت نویسه *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  وگرنه {  /* loading from a reader function */
    صحیح firstcall = 1;  /* userdata for generic_reader */
    ثابت نویسه *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  بازگشت load_aux(L, status, env);
}

/* }====================================================== */


ایستا صحیح dofilecont (lua_State *L, صحیح d1, lua_KContext d2) {
  (تهی)d1;  (تهی)d2;  /* only to match 'lua_Kfunction' prototype */
  بازگشت lua_gettop(L) - 1;
}


ایستا صحیح luaB_dofile (lua_State *L) {
  ثابت نویسه *fname = luaL_optstring(L, 1, پوچ);
  lua_settop(L, 1);
  اگر (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    بازگشت lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  بازگشت dofilecont(L, 0, 0);
}


ایستا صحیح luaB_assert (lua_State *L) {
  اگر (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    بازگشت lua_gettop(L);  /* return all arguments */
  وگرنه {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    بازگشت luaB_error(L);  /* call 'error' */
  }
}


ایستا صحیح luaB_select (lua_State *L) {
  صحیح n = lua_gettop(L);
  اگر (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    بازگشت 1;
  }
  وگرنه {
    lua_Integer i = luaL_checkinteger(L, 1);
    اگر (i < 0) i = n + i;
    وگرنه اگر (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    بازگشت n - (صحیح)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
ایستا صحیح finishpcall (lua_State *L, صحیح status, lua_KContext extra) {
  اگر (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    بازگشت 2;  /* return false, msg */
  }
  وگرنه
    بازگشت lua_gettop(L) - (صحیح)extra;  /* return all results */
}


ایستا صحیح luaB_pcall (lua_State *L) {
  صحیح status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_درج(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  بازگشت finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
ایستا صحیح luaB_xpcall (lua_State *L) {
  صحیح status;
  صحیح n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  بازگشت finishpcall(L, status, 2);
}


ایستا صحیح luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, پوچ);
  بازگشت 1;
}


ایستا ثابت luaL_Reg base_funcs[] = {
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
  {LUA_GNAME, پوچ},
  {"_VERSION", پوچ},
  {پوچ, پوچ}
};


LUAMOD_API صحیح luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  بازگشت 1;
}

