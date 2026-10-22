/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "llimits.h"


স্থির পূর্ণসংখ্যা luaB_print (lua_State *L) {
  পূর্ণসংখ্যা n = lua_gettop(L);  /* number of arguments */
  পূর্ণসংখ্যা i;
  জন্য (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    ধ্রুবক অক্ষর *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    যদি (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_বের_করো(L, 1);  /* pop result */
  }
  lua_writeline();
  ফেরত 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
স্থির পূর্ণসংখ্যা luaB_warn (lua_State *L) {
  পূর্ণসংখ্যা n = lua_gettop(L);  /* number of arguments */
  পূর্ণসংখ্যা i;
  luaL_checkstring(L, 1);  /* at least one argument */
  জন্য (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  জন্য (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  ফেরত 0;
}


#define SPACECHARS	" \f\n\r\t\v"

স্থির ধ্রুবক অক্ষর *b_str2int (ধ্রুবক অক্ষর *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  পূর্ণসংখ্যা neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  যদি (*s == '-') { s++; neg = 1; }  /* handle sign */
  অন্যথা যদি (*s == '+') s++;
  যদি (!isalnum(cast_uchar(*s)))  /* no digit? */
    ফেরত ফাঁকা;
  করো {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    যদি (digit >= base) ফেরত ফাঁকা;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } যখন (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  ফেরত s;
}


স্থির পূর্ণসংখ্যা luaB_tonumber (lua_State *L) {
  যদি (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    যদি (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      ফেরত 1;
    }
    অন্যথা {
      size_t l;
      ধ্রুবক অক্ষর *s = lua_tolstring(L, 1, &l);
      যদি (s != ফাঁকা && lua_stringtonumber(L, s) == l + 1)
        ফেরত 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  অন্যথা {
    size_t l;
    ধ্রুবক অক্ষর *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    যদি (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      ফেরত 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা luaB_error (lua_State *L) {
  পূর্ণসংখ্যা level = (পূর্ণসংখ্যা)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  যদি (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  ফেরত lua_error(L);
}


স্থির পূর্ণসংখ্যা luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  যদি (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    ফেরত 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  ফেরত 1;  /* returns either __metatable field (if present) or metatable */
}


স্থির পূর্ণসংখ্যা luaB_setmetatable (lua_State *L) {
  পূর্ণসংখ্যা t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  যদি (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    ফেরত luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা luaB_rawlen (lua_State *L) {
  পূর্ণসংখ্যা t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  ফেরত 1;
}

স্থির পূর্ণসংখ্যা luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা pushmode (lua_State *L, পূর্ণসংখ্যা oldmode) {
  যদি (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  অন্যথা
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  ফেরত 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { যদি (res == -1) বিরতি; }

স্থির পূর্ণসংখ্যা luaB_collectgarbage (lua_State *L) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", ফাঁকা};
  স্থির ধ্রুবক অক্ষর optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  পূর্ণসংখ্যা o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  নির্বাচন (o) {
    ক্ষেত্র LUA_GCCOUNT: {
      পূর্ণসংখ্যা k = lua_gc(L, o);
      পূর্ণসংখ্যা b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_সংখ্যা)k + ((lua_সংখ্যা)b/1024));
      ফেরত 1;
    }
    ক্ষেত্র LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      পূর্ণসংখ্যা res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      ফেরত 1;
    }
    ক্ষেত্র LUA_GCISRUNNING: {
      পূর্ণসংখ্যা res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      ফেরত 1;
    }
    ক্ষেত্র LUA_GCGEN: {
      ফেরত pushmode(L, lua_gc(L, o));
    }
    ক্ষেত্র LUA_GCINC: {
      ফেরত pushmode(L, lua_gc(L, o));
    }
    ক্ষেত্র LUA_GCPARAM: {
      স্থির ধ্রুবক অক্ষর *ধ্রুবক params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", ফাঁকা};
      স্থির ধ্রুবক অক্ষর pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      পূর্ণসংখ্যা p = pnum[luaL_checkoption(L, 2, ফাঁকা, params)];
      lua_Integer মান = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (পূর্ণসংখ্যা)মান));
      ফেরত 1;
    }
    পূর্বনির্ধারিত: {
      পূর্ণসংখ্যা res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      ফেরত 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা luaB_type (lua_State *L) {
  পূর্ণসংখ্যা t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা lua_B_পরবর্তী (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  যদি (lua_পরবর্তী(L, 1))
    ফেরত 2;
  অন্যথা {
    lua_pushnil(L);
    ফেরত 1;
  }
}


স্থির পূর্ণসংখ্যা pairscont (lua_State *L, পূর্ণসংখ্যা status, lua_KContext k) {
  (শূন্য)L; (শূন্য)status; (শূন্য)k;  /* unused */
  ফেরত 4;  /* __pairs did all the work, just return its results */
}

স্থির পূর্ণসংখ্যা luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  যদি (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, lua_B_পরবর্তী);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  অন্যথা {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  ফেরত 4;
}


/*
** Traversal function for 'ipairs'
*/
স্থির পূর্ণসংখ্যা ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  ফেরত (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
স্থির পূর্ণসংখ্যা luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  ফেরত 3;
}


স্থির পূর্ণসংখ্যা load_aux (lua_State *L, পূর্ণসংখ্যা status, পূর্ণসংখ্যা envidx) {
  যদি (l_likely(status == LUA_OK)) {
    যদি (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      যদি (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_বের_করো(L, 1);  /* remove 'env' if not used by previous call */
    }
    ফেরত 1;
  }
  অন্যথা {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_ঢোকাও(L, -2);  /* put before error message */
    ফেরত 2;  /* return fail plus error message */
  }
}


স্থির ধ্রুবক অক্ষর *getMode (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক অক্ষর *mode = luaL_optstring(L, idx, ফাঁকা);
  যদি (mode != ফাঁকা && strchr(mode, 'B') != ফাঁকা) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  ফেরত mode;
}


স্থির পূর্ণসংখ্যা luaB_loadfile (lua_State *L) {
  ধ্রুবক অক্ষর *fname = luaL_optstring(L, 1, ফাঁকা);
  ধ্রুবক অক্ষর *mode = getMode(L, 2);
  পূর্ণসংখ্যা env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  পূর্ণসংখ্যা status = luaL_loadfilex(L, fname, mode);
  ফেরত load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
স্থির ধ্রুবক অক্ষর *generic_reader (lua_State *L, শূন্য *ud, size_t *আকার_2) {
  পূর্ণসংখ্যা *firstcall = cast(পূর্ণসংখ্যা *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  যদি (*firstcall)
    *firstcall = 0;
  অন্যথা
    lua_বের_করো(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  যদি (lua_isnil(L, -1)) {
    *আকার_2 = 0;
    ফেরত ফাঁকা;
  }
  অন্যথা যদি (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  ফেরত lua_tolstring(L, -1, আকার_2);
}


স্থির পূর্ণসংখ্যা luaB_load (lua_State *L) {
  পূর্ণসংখ্যা status;
  size_t l;
  ধ্রুবক অক্ষর *s = lua_tolstring(L, 1, &l);
  ধ্রুবক অক্ষর *mode = getMode(L, 3);
  পূর্ণসংখ্যা env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  যদি (s != ফাঁকা) {  /* loading a string? */
    ধ্রুবক অক্ষর *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  অন্যথা {  /* loading from a reader function */
    পূর্ণসংখ্যা firstcall = 1;  /* userdata for generic_reader */
    ধ্রুবক অক্ষর *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  ফেরত load_aux(L, status, env);
}

/* }====================================================== */


স্থির পূর্ণসংখ্যা dofilecont (lua_State *L, পূর্ণসংখ্যা d1, lua_KContext d2) {
  (শূন্য)d1;  (শূন্য)d2;  /* only to match 'lua_Kfunction' prototype */
  ফেরত lua_gettop(L) - 1;
}


স্থির পূর্ণসংখ্যা luaB_dofile (lua_State *L) {
  ধ্রুবক অক্ষর *fname = luaL_optstring(L, 1, ফাঁকা);
  lua_settop(L, 1);
  যদি (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    ফেরত lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  ফেরত dofilecont(L, 0, 0);
}


স্থির পূর্ণসংখ্যা luaB_assert (lua_State *L) {
  যদি (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    ফেরত lua_gettop(L);  /* return all arguments */
  অন্যথা {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    ফেরত luaB_error(L);  /* call 'error' */
  }
}


স্থির পূর্ণসংখ্যা luaB_select (lua_State *L) {
  পূর্ণসংখ্যা n = lua_gettop(L);
  যদি (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    ফেরত 1;
  }
  অন্যথা {
    lua_Integer i = luaL_checkinteger(L, 1);
    যদি (i < 0) i = n + i;
    অন্যথা যদি (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    ফেরত n - (পূর্ণসংখ্যা)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
স্থির পূর্ণসংখ্যা finishpcall (lua_State *L, পূর্ণসংখ্যা status, lua_KContext extra) {
  যদি (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    ফেরত 2;  /* return false, msg */
  }
  অন্যথা
    ফেরত lua_gettop(L) - (পূর্ণসংখ্যা)extra;  /* return all results */
}


স্থির পূর্ণসংখ্যা luaB_pcall (lua_State *L) {
  পূর্ণসংখ্যা status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_ঢোকাও(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  ফেরত finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
স্থির পূর্ণসংখ্যা luaB_xpcall (lua_State *L) {
  পূর্ণসংখ্যা status;
  পূর্ণসংখ্যা n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  ফেরত finishpcall(L, status, 2);
}


স্থির পূর্ণসংখ্যা luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, ফাঁকা);
  ফেরত 1;
}


স্থির ধ্রুবক luaL_Reg base_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", luaB_load},
  {"next", lua_B_পরবর্তী},
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
  {LUA_GNAME, ফাঁকা},
  {"_VERSION", ফাঁকা},
  {ফাঁকা, ফাঁকা}
};


LUAMOD_API পূর্ণসংখ্যা luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  ফেরত 1;
}

