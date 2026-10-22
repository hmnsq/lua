/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#define lbaselib_c
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_print (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);  /* number of arguments */
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    ಧ್ರುವ ವರ್ಣ *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    ಯದಿ (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_ತೆಗೆ(L, 1);  /* pop result */
  }
  lua_writeline();
  ಪ್ರತ್ಯಾಗಮ 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_warn (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);  /* number of arguments */
  ಪೂರ್ಣಾಙ್ಕ i;
  luaL_checkstring(L, 1);  /* at least one argument */
  ಪ್ರತಿ (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  ಪ್ರತಿ (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  ಪ್ರತ್ಯಾಗಮ 0;
}


#define SPACECHARS	" \f\n\r\t\v"

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *b_str2int (ಧ್ರುವ ವರ್ಣ *s, unsigned base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  ಪೂರ್ಣಾಙ್ಕ neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  ಯದಿ (*s == '-') { s++; neg = 1; }  /* handle sign */
  ಅನ್ಯಥಾ ಯದಿ (*s == '+') s++;
  ಯದಿ (!isalnum(cast_uchar(*s)))  /* no digit? */
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  ಕುರು {
    unsigned digit = cast_uint(isdigit(cast_uchar(*s))
                               ? *s - '0'
                               : (toupper(cast_uchar(*s)) - 'A') + 10);
    ಯದಿ (digit >= base) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } ಯಾವತ್ (isalnum(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  ಪ್ರತ್ಯಾಗಮ s;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_tonumber (lua_State *L) {
  ಯದಿ (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    ಯದಿ (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಅನ್ಯಥಾ {
      size_t l;
      ಧ್ರುವ ವರ್ಣ *s = lua_tolstring(L, 1, &l);
      ಯದಿ (s != ರಿಕ್ತ && lua_stringtonumber(L, s) == l + 1)
        ಪ್ರತ್ಯಾಗಮ 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  ಅನ್ಯಥಾ {
    size_t l;
    ಧ್ರುವ ವರ್ಣ *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    ಯದಿ (b_str2int(s, cast_uint(base), &n) == s + l) {
      lua_pushinteger(L, n);
      ಪ್ರತ್ಯಾಗಮ 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_error (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ level = (ಪೂರ್ಣಾಙ್ಕ)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  ಯದಿ (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  ಪ್ರತ್ಯಾಗಮ lua_error(L);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ಯದಿ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    ಪ್ರತ್ಯಾಗಮ 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  ಪ್ರತ್ಯಾಗಮ 1;  /* returns either __metatable field (if present) or metatable */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_setmetatable (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  ಯದಿ (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_rawlen (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pushmode (lua_State *L, ಪೂರ್ಣಾಙ್ಕ oldmode) {
  ಯದಿ (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  ಅನ್ಯಥಾ
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { ಯದಿ (res == -1) ಭಙ್ಗ; }

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_collectgarbage (lua_State *L) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", ರಿಕ್ತ};
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  ಪೂರ್ಣಾಙ್ಕ o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  ವಿಕಲ್ಪ (o) {
    ಪಕ್ಷ LUA_GCCOUNT: {
      ಪೂರ್ಣಾಙ್ಕ k = lua_gc(L, o);
      ಪೂರ್ಣಾಙ್ಕ b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)k + ((lua_ಸಂಖ್ಯೆ)b/1024));
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      ಪೂರ್ಣಾಙ್ಕ res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ LUA_GCISRUNNING: {
      ಪೂರ್ಣಾಙ್ಕ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪಕ್ಷ LUA_GCGEN: {
      ಪ್ರತ್ಯಾಗಮ pushmode(L, lua_gc(L, o));
    }
    ಪಕ್ಷ LUA_GCINC: {
      ಪ್ರತ್ಯಾಗಮ pushmode(L, lua_gc(L, o));
    }
    ಪಕ್ಷ LUA_GCPARAM: {
      ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", ರಿಕ್ತ};
      ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      ಪೂರ್ಣಾಙ್ಕ p = pnum[luaL_checkoption(L, 2, ರಿಕ್ತ, params)];
      lua_Integer ಮೌಲ್ಯ = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (ಪೂರ್ಣಾಙ್ಕ)ಮೌಲ್ಯ));
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಸ್ವಭಾವ: {
      ಪೂರ್ಣಾಙ್ಕ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_type (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_next (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  ಯದಿ (lua_next(L, 1))
    ಪ್ರತ್ಯಾಗಮ 2;
  ಅನ್ಯಥಾ {
    lua_pushnil(L);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pairscont (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status, lua_KContext k) {
  (ಶೂನ್ಯ)L; (ಶೂನ್ಯ)status; (ಶೂನ್ಯ)k;  /* unused */
  ಪ್ರತ್ಯಾಗಮ 4;  /* __pairs did all the work, just return its results */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  ಯದಿ (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, luaB_next);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  ಅನ್ಯಥಾ {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  ಪ್ರತ್ಯಾಗಮ 4;
}


/*
** Traversal function for 'ipairs'
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  ಪ್ರತ್ಯಾಗಮ (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  ಪ್ರತ್ಯಾಗಮ 3;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ load_aux (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status, ಪೂರ್ಣಾಙ್ಕ envidx) {
  ಯದಿ (l_likely(status == LUA_OK)) {
    ಯದಿ (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      ಯದಿ (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_ತೆಗೆ(L, 1);  /* remove 'env' if not used by previous call */
    }
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_ಸೇರಿಸು(L, -2);  /* put before error message */
    ಪ್ರತ್ಯಾಗಮ 2;  /* return fail plus error message */
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getMode (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ ವರ್ಣ *mode = luaL_optstring(L, idx, ರಿಕ್ತ);
  ಯದಿ (mode != ರಿಕ್ತ && strchr(mode, 'B') != ರಿಕ್ತ) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  ಪ್ರತ್ಯಾಗಮ mode;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_loadfile (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *fname = luaL_optstring(L, 1, ರಿಕ್ತ);
  ಧ್ರುವ ವರ್ಣ *mode = getMode(L, 2);
  ಪೂರ್ಣಾಙ್ಕ env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  ಪೂರ್ಣಾಙ್ಕ status = luaL_loadfilex(L, fname, mode);
  ಪ್ರತ್ಯಾಗಮ load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *generic_reader (lua_State *L, ಶೂನ್ಯ *ud, size_t *size) {
  ಪೂರ್ಣಾಙ್ಕ *firstcall = cast(ಪೂರ್ಣಾಙ್ಕ *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  ಯದಿ (*firstcall)
    *firstcall = 0;
  ಅನ್ಯಥಾ
    lua_ತೆಗೆ(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  ಯದಿ (lua_isnil(L, -1)) {
    *size = 0;
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಅನ್ಯಥಾ ಯದಿ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  ಪ್ರತ್ಯಾಗಮ lua_tolstring(L, -1, size);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_load (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ status;
  size_t l;
  ಧ್ರುವ ವರ್ಣ *s = lua_tolstring(L, 1, &l);
  ಧ್ರುವ ವರ್ಣ *mode = getMode(L, 3);
  ಪೂರ್ಣಾಙ್ಕ env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  ಯದಿ (s != ರಿಕ್ತ) {  /* loading a string? */
    ಧ್ರುವ ವರ್ಣ *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  ಅನ್ಯಥಾ {  /* loading from a reader function */
    ಪೂರ್ಣಾಙ್ಕ firstcall = 1;  /* userdata for generic_reader */
    ಧ್ರುವ ವರ್ಣ *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    status = lua_load(L, generic_reader, &firstcall, chunkname, mode);
  }
  ಪ್ರತ್ಯಾಗಮ load_aux(L, status, env);
}

/* }====================================================== */


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dofilecont (lua_State *L, ಪೂರ್ಣಾಙ್ಕ d1, lua_KContext d2) {
  (ಶೂನ್ಯ)d1;  (ಶೂನ್ಯ)d2;  /* only to match 'lua_Kfunction' prototype */
  ಪ್ರತ್ಯಾಗಮ lua_gettop(L) - 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_dofile (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *fname = luaL_optstring(L, 1, ರಿಕ್ತ);
  lua_settop(L, 1);
  ಯದಿ (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    ಪ್ರತ್ಯಾಗಮ lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  ಪ್ರತ್ಯಾಗಮ dofilecont(L, 0, 0);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_assert (lua_State *L) {
  ಯದಿ (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    ಪ್ರತ್ಯಾಗಮ lua_gettop(L);  /* return all arguments */
  ಅನ್ಯಥಾ {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    ಪ್ರತ್ಯಾಗಮ luaB_error(L);  /* call 'error' */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_select (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);
  ಯದಿ (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ {
    lua_Integer i = luaL_checkinteger(L, 1);
    ಯದಿ (i < 0) i = n + i;
    ಅನ್ಯಥಾ ಯದಿ (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    ಪ್ರತ್ಯಾಗಮ n - (ಪೂರ್ಣಾಙ್ಕ)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ finishpcall (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status, lua_KContext extra) {
  ಯದಿ (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    ಪ್ರತ್ಯಾಗಮ 2;  /* return false, msg */
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ lua_gettop(L) - (ಪೂರ್ಣಾಙ್ಕ)extra;  /* return all results */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_pcall (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_ಸೇರಿಸು(L, 1);  /* put it in place */
  status = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  ಪ್ರತ್ಯಾಗಮ finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_xpcall (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ status;
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  status = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  ಪ್ರತ್ಯಾಗಮ finishpcall(L, status, 2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg base_funcs[] = {
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
  {LUA_GNAME, ರಿಕ್ತ},
  {"_VERSION", ರಿಕ್ತ},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_base (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_VERSION);
  lua_setfield(L, -2, "_VERSION");
  ಪ್ರತ್ಯಾಗಮ 1;
}

