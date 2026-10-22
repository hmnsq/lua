/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


நிலையான முழு luaB_print (lua_State *L) {
  முழு n = lua_gettop(L);  /* number of arguments */
  முழு i;
  க்கு (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    மாறிலி எழுத்து *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    எனில் (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_எடு(L, 1);  /* pop result */
  }
  lua_writeline();
  திரும்பு 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
நிலையான முழு luaB_warn (lua_State *L) {
  முழு n = lua_gettop(L);  /* number of arguments */
  முழு i;
  luaL_checkstring(L, 1);  /* at least one argument */
  க்கு (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  க்கு (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  திரும்பு 0;
}


#define SPACECHARS	" \f\n\r\t\v"

நிலையான மாறிலி எழுத்து *b_str2int (மாறிலி எழுத்து *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  முழு neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  எனில் (*s == '-') { s++; neg = 1; }  /* handle sign */
  இல்லையெனில் எனில் (*s == '+') s++;
  எனில் (!isalnum(cast_uchar(*s)))  /* no digit? */
    திரும்பு வெற்று;
  செய் {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    எனில் (digit >= base) திரும்பு வெற்று;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } போது (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  திரும்பு s;
}


நிலையான முழு luaB_tonumber (lua_State *L) {
  எனில் (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    எனில் (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      திரும்பு 1;
    }
    இல்லையெனில் {
      size_t l;
      மாறிலி எழுத்து *s = lua_tolstring(L, 1, &l);
      எனில் (s != வெற்று && lua_stringtonumber(L, s) == l + 1)
        திரும்பு 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  இல்லையெனில் {
    size_t l;
    மாறிலி எழுத்து *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    எனில் (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      திரும்பு 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  திரும்பு 1;
}


நிலையான முழு luaB_error (lua_State *L) {
  முழு level = (முழு)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  எனில் (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  திரும்பு lua_error(L);
}


நிலையான முழு luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  எனில் (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    திரும்பு 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  திரும்பு 1;  /* returns either __metatable field (if present) or metatable */
}


நிலையான முழு luaB_setmetatable (lua_State *L) {
  முழு t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  எனில் (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    திரும்பு luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  திரும்பு 1;
}


நிலையான முழு luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  திரும்பு 1;
}


நிலையான முழு luaB_rawlen (lua_State *L) {
  முழு t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  திரும்பு 1;
}


நிலையான முழு luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  திரும்பு 1;
}

நிலையான முழு luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  திரும்பு 1;
}


நிலையான முழு pushmode (lua_State *L, முழு oldmode) {
  எனில் (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  இல்லையெனில்
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  திரும்பு 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { எனில் (res == -1) நிறுத்து; }

நிலையான முழு luaB_collectgarbage (lua_State *L) {
  நிலையான மாறிலி எழுத்து *மாறிலி opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", வெற்று};
  நிலையான மாறிலி எழுத்து optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  முழு o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  தேர்வு (o) {
    நிலை LUA_GCCOUNT: {
      முழு k = lua_gc(L, o);
      முழு b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_எண்)k + ((lua_எண்)b/1024));
      திரும்பு 1;
    }
    நிலை LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      முழு res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      திரும்பு 1;
    }
    நிலை LUA_GCISRUNNING: {
      முழு res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      திரும்பு 1;
    }
    நிலை LUA_GCGEN: {
      திரும்பு pushmode(L, lua_gc(L, o));
    }
    நிலை LUA_GCINC: {
      திரும்பு pushmode(L, lua_gc(L, o));
    }
    நிலை LUA_GCPARAM: {
      நிலையான மாறிலி எழுத்து *மாறிலி params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", வெற்று};
      நிலையான மாறிலி எழுத்து pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      முழு p = pnum[luaL_checkoption(L, 2, வெற்று, params)];
      lua_Integer மதிப்பு = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (முழு)மதிப்பு));
      திரும்பு 1;
    }
    இயல்பு: {
      முழு res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      திரும்பு 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  திரும்பு 1;
}


நிலையான முழு luaB_type (lua_State *L) {
  முழு t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  திரும்பு 1;
}


நிலையான முழு luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  எனில் (lua_next(L, 1))
    திரும்பு 2;
  இல்லையெனில் {
    lua_pushnil(L);
    திரும்பு 1;
  }
}


நிலையான முழு pairscont (lua_State *L, முழு status, lua_KContext k) {
  (வெறுமை)L; (வெறுமை)status; (வெறுமை)k;  /* unused */
  திரும்பு 4;  /* __pairs did all the work, just return its results */
}

நிலையான முழு luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  எனில் (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  இல்லையெனில் {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  திரும்பு 4;
}


/*
** Traversal function for 'ipairs'
*/
நிலையான முழு ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  திரும்பு (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
நிலையான முழு luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  திரும்பு 3;
}


நிலையான முழு load_aux (lua_State *L, முழு status, முழு envidx) {
  எனில் (l_likely(status == LUA_OK)) {
    எனில் (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      எனில் (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_எடு(L, 1);  /* remove 'env' if not used by previous call */
    }
    திரும்பு 1;
  }
  இல்லையெனில் {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_செருகு(L, -2);  /* put before error message */
    திரும்பு 2;  /* return fail plus error message */
  }
}


நிலையான மாறிலி எழுத்து *getMode (lua_State *L, முழு idx) {
  மாறிலி எழுத்து *mode = luaL_optstring(L, idx, வெற்று);
  எனில் (mode != வெற்று && strchr(mode, 'B') != வெற்று) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  திரும்பு mode;
}


நிலையான முழு luaB_loadfile (lua_State *L) {
  மாறிலி எழுத்து *fname = luaL_optstring(L, 1, வெற்று);
  மாறிலி எழுத்து *mode = getMode(L, 2);
  முழு env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  முழு status = luaL_loadfilex(L, fname, mode);
  திரும்பு load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
நிலையான மாறிலி எழுத்து *generic_reader (lua_State *L, வெறுமை *ud, size_t *size) {
  முழு *firstcall = cast(முழு *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  எனில் (*firstcall)
    *firstcall = 0;
  இல்லையெனில்
    lua_எடு(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  எனில் (lua_isnil(L, -1)) {
    *size = 0;
    திரும்பு வெற்று;
  }
  இல்லையெனில் எனில் (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  திரும்பு lua_tolstring(L, -1, size);
}


நிலையான முழு luaB_load (lua_State *L) {
  முழு status;
  size_t l;
  மாறிலி எழுத்து *s = lua_tolstring(L, 1, &l);
  மாறிலி எழுத்து *mode = getMode(L, 3);
  முழு env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  எனில் (s != வெற்று) {  /* loading a string? */
    மாறிலி எழுத்து *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  இல்லையெனில் {  /* loading from a reader function */
    முழு firstcall = 1;  /* userdata for generic_reader */
    மாறிலி எழுத்து *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  திரும்பு load_aux(L, status, env);
}

/* }====================================================== */


நிலையான முழு dofilecont (lua_State *L, முழு d1, lua_KContext d2) {
  (வெறுமை)d1;  (வெறுமை)d2;  /* only to match 'lua_Kfunction' prototype */
  திரும்பு lua_gettop(L) - 1;
}


நிலையான முழு luaB_dofile (lua_State *L) {
  மாறிலி எழுத்து *fname = luaL_optstring(L, 1, வெற்று);
  lua_settop(L, 1);
  எனில் (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    திரும்பு lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  திரும்பு dofilecont(L, 0, 0);
}


நிலையான முழு luaB_assert (lua_State *L) {
  எனில் (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    திரும்பு lua_gettop(L);  /* return all arguments */
  இல்லையெனில் {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    திரும்பு luaB_error(L);  /* call 'error' */
  }
}


நிலையான முழு luaB_select (lua_State *L) {
  முழு n = lua_gettop(L);
  எனில் (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    திரும்பு 1;
  }
  இல்லையெனில் {
    lua_Integer i = luaL_checkinteger(L, 1);
    எனில் (i < 0) i = n + i;
    இல்லையெனில் எனில் (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    திரும்பு n - (முழு)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
நிலையான முழு finishpcall (lua_State *L, முழு status, lua_KContext extra) {
  எனில் (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    திரும்பு 2;  /* return false, msg */
  }
  இல்லையெனில்
    திரும்பு lua_gettop(L) - (முழு)extra;  /* return all results */
}


நிலையான முழு luaB_pcall (lua_State *L) {
  முழு status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_செருகு(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  திரும்பு finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
நிலையான முழு luaB_xpcall (lua_State *L) {
  முழு status;
  முழு n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  திரும்பு finishpcall(L, status, 2);
}


நிலையான முழு luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, வெற்று);
  திரும்பு 1;
}


நிலையான மாறிலி luaL_Reg base_funcs[] = {
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
  {LUA_GNAME, வெற்று},
  {"_VERSION", வெற்று},
  {வெற்று, வெற்று}
};


LUAMOD_API முழு luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  திரும்பு 1;
}

