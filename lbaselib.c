/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_print (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);  /* number of arguments */
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    സ്ഥിരം അക്ഷരം *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    എങ്കിൽ (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_എടുക്കുക(L, 1);  /* pop result */
  }
  lua_writeline();
  തിരികെ 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_warn (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);  /* number of arguments */
  പൂർണ്ണസംഖ്യ i;
  luaL_checkstring(L, 1);  /* at least one argument */
  വേണ്ടി (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  വേണ്ടി (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  തിരികെ 0;
}


#define SPACECHARS	" \f\n\r\t\v"

സ്ഥിതം സ്ഥിരം അക്ഷരം *b_str2int (സ്ഥിരം അക്ഷരം *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  പൂർണ്ണസംഖ്യ neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  എങ്കിൽ (*s == '-') { s++; neg = 1; }  /* handle sign */
  അല്ലെങ്കിൽ എങ്കിൽ (*s == '+') s++;
  എങ്കിൽ (!isalnum(cast_uchar(*s)))  /* no digit? */
    തിരികെ ശൂന്യ;
  ചെയ്യുക {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    എങ്കിൽ (digit >= base) തിരികെ ശൂന്യ;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } ആകുമ്പോൾ (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  തിരികെ s;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_tonumber (lua_State *L) {
  എങ്കിൽ (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    എങ്കിൽ (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      തിരികെ 1;
    }
    അല്ലെങ്കിൽ {
      size_t l;
      സ്ഥിരം അക്ഷരം *s = lua_tolstring(L, 1, &l);
      എങ്കിൽ (s != ശൂന്യ && lua_stringtonumber(L, s) == l + 1)
        തിരികെ 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  അല്ലെങ്കിൽ {
    size_t l;
    സ്ഥിരം അക്ഷരം *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    എങ്കിൽ (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      തിരികെ 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_error (lua_State *L) {
  പൂർണ്ണസംഖ്യ level = (പൂർണ്ണസംഖ്യ)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  എങ്കിൽ (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  തിരികെ lua_error(L);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  എങ്കിൽ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    തിരികെ 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  തിരികെ 1;  /* returns either __metatable field (if present) or metatable */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_setmetatable (lua_State *L) {
  പൂർണ്ണസംഖ്യ t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  എങ്കിൽ (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    തിരികെ luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_rawlen (lua_State *L) {
  പൂർണ്ണസംഖ്യ t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ pushmode (lua_State *L, പൂർണ്ണസംഖ്യ oldmode) {
  എങ്കിൽ (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  അല്ലെങ്കിൽ
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  തിരികെ 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { എങ്കിൽ (res == -1) നിർത്തുക; }

സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_collectgarbage (lua_State *L) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", ശൂന്യ};
  സ്ഥിതം സ്ഥിരം അക്ഷരം optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  പൂർണ്ണസംഖ്യ o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  തിരഞ്ഞെടുപ്പ് (o) {
    അവസ്ഥ LUA_GCCOUNT: {
      പൂർണ്ണസംഖ്യ k = lua_gc(L, o);
      പൂർണ്ണസംഖ്യ b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_സംഖ്യ)k + ((lua_സംഖ്യ)b/1024));
      തിരികെ 1;
    }
    അവസ്ഥ LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      പൂർണ്ണസംഖ്യ res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      തിരികെ 1;
    }
    അവസ്ഥ LUA_GCISRUNNING: {
      പൂർണ്ണസംഖ്യ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      തിരികെ 1;
    }
    അവസ്ഥ LUA_GCGEN: {
      തിരികെ pushmode(L, lua_gc(L, o));
    }
    അവസ്ഥ LUA_GCINC: {
      തിരികെ pushmode(L, lua_gc(L, o));
    }
    അവസ്ഥ LUA_GCPARAM: {
      സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", ശൂന്യ};
      സ്ഥിതം സ്ഥിരം അക്ഷരം pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      പൂർണ്ണസംഖ്യ p = pnum[luaL_checkoption(L, 2, ശൂന്യ, params)];
      lua_Integer മൂല്യം = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (പൂർണ്ണസംഖ്യ)മൂല്യം));
      തിരികെ 1;
    }
    സ്ഥിരസ്ഥിതി: {
      പൂർണ്ണസംഖ്യ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      തിരികെ 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_type (lua_State *L) {
  പൂർണ്ണസംഖ്യ t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  എങ്കിൽ (lua_next(L, 1))
    തിരികെ 2;
  അല്ലെങ്കിൽ {
    lua_pushnil(L);
    തിരികെ 1;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ pairscont (lua_State *L, പൂർണ്ണസംഖ്യ status, lua_KContext k) {
  (ശൂന്യം)L; (ശൂന്യം)status; (ശൂന്യം)k;  /* unused */
  തിരികെ 4;  /* __pairs did all the work, just return its results */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  എങ്കിൽ (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  അല്ലെങ്കിൽ {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  തിരികെ 4;
}


/*
** Traversal function for 'ipairs'
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  തിരികെ (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  തിരികെ 3;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ load_aux (lua_State *L, പൂർണ്ണസംഖ്യ status, പൂർണ്ണസംഖ്യ envidx) {
  എങ്കിൽ (l_likely(status == LUA_OK)) {
    എങ്കിൽ (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      എങ്കിൽ (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_എടുക്കുക(L, 1);  /* remove 'env' if not used by previous call */
    }
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_തിരുകുക(L, -2);  /* put before error message */
    തിരികെ 2;  /* return fail plus error message */
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *getMode (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം അക്ഷരം *mode = luaL_optstring(L, idx, ശൂന്യ);
  എങ്കിൽ (mode != ശൂന്യ && strchr(mode, 'B') != ശൂന്യ) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  തിരികെ mode;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_loadfile (lua_State *L) {
  സ്ഥിരം അക്ഷരം *fname = luaL_optstring(L, 1, ശൂന്യ);
  സ്ഥിരം അക്ഷരം *mode = getMode(L, 2);
  പൂർണ്ണസംഖ്യ env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  പൂർണ്ണസംഖ്യ status = luaL_loadfilex(L, fname, mode);
  തിരികെ load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *generic_reader (lua_State *L, ശൂന്യം *ud, size_t *size) {
  പൂർണ്ണസംഖ്യ *firstcall = cast(പൂർണ്ണസംഖ്യ *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  എങ്കിൽ (*firstcall)
    *firstcall = 0;
  അല്ലെങ്കിൽ
    lua_എടുക്കുക(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  എങ്കിൽ (lua_isnil(L, -1)) {
    *size = 0;
    തിരികെ ശൂന്യ;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  തിരികെ lua_tolstring(L, -1, size);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_load (lua_State *L) {
  പൂർണ്ണസംഖ്യ status;
  size_t l;
  സ്ഥിരം അക്ഷരം *s = lua_tolstring(L, 1, &l);
  സ്ഥിരം അക്ഷരം *mode = getMode(L, 3);
  പൂർണ്ണസംഖ്യ env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  എങ്കിൽ (s != ശൂന്യ) {  /* loading a string? */
    സ്ഥിരം അക്ഷരം *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  അല്ലെങ്കിൽ {  /* loading from a reader function */
    പൂർണ്ണസംഖ്യ firstcall = 1;  /* userdata for generic_reader */
    സ്ഥിരം അക്ഷരം *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  തിരികെ load_aux(L, status, env);
}

/* }====================================================== */


സ്ഥിതം പൂർണ്ണസംഖ്യ dofilecont (lua_State *L, പൂർണ്ണസംഖ്യ d1, lua_KContext d2) {
  (ശൂന്യം)d1;  (ശൂന്യം)d2;  /* only to match 'lua_Kfunction' prototype */
  തിരികെ lua_gettop(L) - 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_dofile (lua_State *L) {
  സ്ഥിരം അക്ഷരം *fname = luaL_optstring(L, 1, ശൂന്യ);
  lua_settop(L, 1);
  എങ്കിൽ (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    തിരികെ lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  തിരികെ dofilecont(L, 0, 0);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_assert (lua_State *L) {
  എങ്കിൽ (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    തിരികെ lua_gettop(L);  /* return all arguments */
  അല്ലെങ്കിൽ {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    തിരികെ luaB_error(L);  /* call 'error' */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_select (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);
  എങ്കിൽ (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ {
    lua_Integer i = luaL_checkinteger(L, 1);
    എങ്കിൽ (i < 0) i = n + i;
    അല്ലെങ്കിൽ എങ്കിൽ (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    തിരികെ n - (പൂർണ്ണസംഖ്യ)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ finishpcall (lua_State *L, പൂർണ്ണസംഖ്യ status, lua_KContext extra) {
  എങ്കിൽ (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    തിരികെ 2;  /* return false, msg */
  }
  അല്ലെങ്കിൽ
    തിരികെ lua_gettop(L) - (പൂർണ്ണസംഖ്യ)extra;  /* return all results */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_pcall (lua_State *L) {
  പൂർണ്ണസംഖ്യ status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_തിരുകുക(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  തിരികെ finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_xpcall (lua_State *L) {
  പൂർണ്ണസംഖ്യ status;
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  തിരികെ finishpcall(L, status, 2);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, ശൂന്യ);
  തിരികെ 1;
}


സ്ഥിതം സ്ഥിരം luaL_Reg base_funcs[] = {
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
  {LUA_GNAME, ശൂന്യ},
  {"_VERSION", ശൂന്യ},
  {ശൂന്യ, ശൂന്യ}
};


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  തിരികെ 1;
}

