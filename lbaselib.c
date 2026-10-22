/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#تعریف_کن lbaselib_c
#تعریف_کن LUA_LIB

#ضمن "lprefix.h"


#ضمن <ctype.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


ساكن صحيح luaB_print (lua_State *L) {
  صحيح n = lua_gettop(L);  /* number of arguments */
  صحيح i;
  لكل (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    ثابت حرف *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    إذا (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_برداشت(L, 1);  /* pop result */
  }
  lua_writeline();
  أرجع 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
ساكن صحيح luaB_warn (lua_State *L) {
  صحيح n = lua_gettop(L);  /* number of arguments */
  صحيح i;
  luaL_checkstring(L, 1);  /* at least one argument */
  لكل (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  لكل (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  أرجع 0;
}


#تعریف_کن SPACECHARS	" \f\n\r\t\v"

ساكن ثابت حرف *b_str2int (ثابت حرف *s, بیعلامت پایه, lua_Integer *pn) {
  lua_Unsigned n = 0;
  صحيح neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  إذا (*s == '-') { s++; neg = 1; }  /* handle sign */
  وإلا إذا (*s == '+') s++;
  إذا (!آیا_حرف_یا_رقم(cast_uchar(*s)))  /* no digit? */
    أرجع عدم;
  افعل {
    بیعلامت digit = cast_uint(آیا_رقم(cast_uchar(*s))
                               ? *s - '0'
                               : (به_بزرگ(cast_uchar(*s)) - 'A') + 10);
    إذا (digit >= پایه) أرجع عدم;  /* invalid numeral */
    n = n * پایه + digit;
    s++;
  } طالما (آیا_حرف_یا_رقم(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  أرجع s;
}


ساكن صحيح luaB_tonumber (lua_State *L) {
  إذا (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    إذا (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      أرجع 1;
    }
    وإلا {
      size_t l;
      ثابت حرف *s = lua_tolstring(L, 1, &l);
      إذا (s != عدم && lua_stringtonumber(L, s) == l + 1)
        أرجع 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  وإلا {
    size_t l;
    ثابت حرف *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer پایه = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= پایه && پایه <= 36, 2, "base out of range");
    إذا (b_str2int(s, cast_uint(پایه), &n) == s + l) {
      lua_pushinteger(L, n);
      أرجع 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  أرجع 1;
}


ساكن صحيح luaB_error (lua_State *L) {
  صحيح level = (صحيح)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  إذا (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  أرجع lua_error(L);
}


ساكن صحيح luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  إذا (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    أرجع 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  أرجع 1;  /* returns either __metatable field (if present) or metatable */
}


ساكن صحيح luaB_setmetatable (lua_State *L) {
  صحيح t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  إذا (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    أرجع luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  أرجع 1;
}


ساكن صحيح luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  أرجع 1;
}


ساكن صحيح luaB_rawlen (lua_State *L) {
  صحيح t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  أرجع 1;
}


ساكن صحيح luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  أرجع 1;
}

ساكن صحيح luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  أرجع 1;
}


ساكن صحيح pushmode (lua_State *L, صحيح oldmode) {
  إذا (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  وإلا
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  أرجع 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#تعریف_کن checkvalres(res) { إذا (res == -1) اكسر; }

ساكن صحيح luaB_collectgarbage (lua_State *L) {
  ساكن ثابت حرف *ثابت opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", عدم};
  ساكن ثابت حرف optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  صحيح o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  بدل (o) {
    حالة LUA_GCCOUNT: {
      صحيح k = lua_gc(L, o);
      صحيح b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_عدد)k + ((lua_عدد)b/1024));
      أرجع 1;
    }
    حالة LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      صحيح res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      أرجع 1;
    }
    حالة LUA_GCISRUNNING: {
      صحيح res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      أرجع 1;
    }
    حالة LUA_GCGEN: {
      أرجع pushmode(L, lua_gc(L, o));
    }
    حالة LUA_GCINC: {
      أرجع pushmode(L, lua_gc(L, o));
    }
    حالة LUA_GCPARAM: {
      ساكن ثابت حرف *ثابت params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", عدم};
      ساكن ثابت حرف pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      صحيح p = pnum[luaL_checkoption(L, 2, عدم, params)];
      lua_Integer مقدار = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (صحيح)مقدار));
      أرجع 1;
    }
    افتراضي: {
      صحيح res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      أرجع 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  أرجع 1;
}


ساكن صحيح luaB_type (lua_State *L) {
  صحيح t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  أرجع 1;
}


ساكن صحيح lua_B_بعدی (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  إذا (lua_بعدی(L, 1))
    أرجع 2;
  وإلا {
    lua_pushnil(L);
    أرجع 1;
  }
}


ساكن صحيح pairscont (lua_State *L, صحيح وضعیت, lua_KContext k) {
  (فراغ)L; (فراغ)وضعیت; (فراغ)k;  /*استفادهنشده*/
  أرجع 4;  /* __pairs did all the work, just return its results */
}

ساكن صحيح lua_B_جفتها (lua_State *L) {
  luaL_checkany(L, 1);
  إذا (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, lua_B_بعدی);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  وإلا {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  أرجع 4;
}


/*
** Traversal function for 'ipairs'
*/
ساكن صحيح ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  أرجع (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
ساكن صحيح luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  أرجع 3;
}


ساكن صحيح بارگیری_aux (lua_State *L, صحيح وضعیت, صحيح envidx) {
  إذا (l_likely(وضعیت == LUA_OK)) {
    إذا (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      إذا (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_برداشت(L, 1);  /* remove 'env' if not used by previous call */
    }
    أرجع 1;
  }
  وإلا {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_درج(L, -2);  /* put before error message */
    أرجع 2;  /* return fail plus error message */
  }
}


ساكن ثابت حرف *get_نما (lua_State *L, صحيح idx) {
  ثابت حرف *نما = luaL_optstring(L, idx, عدم);
  إذا (نما != عدم && strchr(نما, 'B') != عدم) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  أرجع نما;
}


ساكن صحيح luaB_loadfile (lua_State *L) {
  ثابت حرف *fname = luaL_optstring(L, 1, عدم);
  ثابت حرف *نما = get_نما(L, 2);
  صحيح env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  صحيح وضعیت = luaL_loadfilex(L, fname, نما);
  أرجع بارگیری_aux(L, وضعیت, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
ساكن ثابت حرف *generic_reader (lua_State *L, فراغ *ud, size_t *اندازه) {
  صحيح *firstcall = cast(صحيح *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  إذا (*firstcall)
    *firstcall = 0;
  وإلا
    lua_برداشت(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  إذا (lua_isnil(L, -1)) {
    *اندازه = 0;
    أرجع عدم;
  }
  وإلا إذا (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  أرجع lua_tolstring(L, -1, اندازه);
}


ساكن صحيح lua_B_بارگیری (lua_State *L) {
  صحيح وضعیت;
  size_t l;
  ثابت حرف *s = lua_tolstring(L, 1, &l);
  ثابت حرف *نما = get_نما(L, 3);
  صحيح env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  إذا (s != عدم) {  /* loading a string? */
    ثابت حرف *chunkname = luaL_optstring(L, 2, s);
    وضعیت = luaL_loadbufferx(L, s, l, chunkname, نما);
  }
  وإلا {  /* loading from a reader function */
    صحيح firstcall = 1;  /* userdata for generic_reader */
    ثابت حرف *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    وضعیت = lua_بارگیری(L, generic_reader, &firstcall, chunkname, نما);
  }
  أرجع بارگیری_aux(L, وضعیت, env);
}

/* }====================================================== */


ساكن صحيح dofilecont (lua_State *L, صحيح d1, lua_KContext d2) {
  (فراغ)d1;  (فراغ)d2;  /* only to match 'lua_Kfunction' prototype */
  أرجع lua_gettop(L) - 1;
}


ساكن صحيح luaB_dofile (lua_State *L) {
  ثابت حرف *fname = luaL_optstring(L, 1, عدم);
  lua_settop(L, 1);
  إذا (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    أرجع lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  أرجع dofilecont(L, 0, 0);
}


ساكن صحيح luaB_assert (lua_State *L) {
  إذا (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    أرجع lua_gettop(L);  /* return all arguments */
  وإلا {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    أرجع luaB_error(L);  /* call 'error' */
  }
}


ساكن صحيح luaB_select (lua_State *L) {
  صحيح n = lua_gettop(L);
  إذا (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    أرجع 1;
  }
  وإلا {
    lua_Integer i = luaL_checkinteger(L, 1);
    إذا (i < 0) i = n + i;
    وإلا إذا (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    أرجع n - (صحيح)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
ساكن صحيح finishpcall (lua_State *L, صحيح وضعیت, lua_KContext extra) {
  إذا (l_unlikely(وضعیت != LUA_OK && وضعیت != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    أرجع 2;  /* return false, msg */
  }
  وإلا
    أرجع lua_gettop(L) - (صحيح)extra;  /* return all results */
}


ساكن صحيح luaB_pcall (lua_State *L) {
  صحيح وضعیت;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_درج(L, 1);  /* put it in place */
  وضعیت = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  أرجع finishpcall(L, وضعیت, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
ساكن صحيح luaB_xpcall (lua_State *L) {
  صحيح وضعیت;
  صحيح n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  وضعیت = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  أرجع finishpcall(L, وضعیت, 2);
}


ساكن صحيح luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, عدم);
  أرجع 1;
}


ساكن ثابت luaL_Reg پایه_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", lua_B_بارگیری},
  {"next", lua_B_بعدی},
  {"pairs", lua_B_جفتها},
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
  {LUA_GNAME, عدم},
  {"_VERSION", عدم},
  {عدم, عدم}
};


LUAMOD_API صحيح luaopen_پایه (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, پایه_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_نسخه);
  lua_setfield(L, -2, "_VERSION");
  أرجع 1;
}

