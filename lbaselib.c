/*
** $Id: lbaselib.c $
** Basic library
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lbaselib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_print (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);  /* number of arguments */
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_tolstring(L, i, &l);  /* convert it to string */
    ಆದರೆ (i > 1)  /* not the first element? */
      lua_writestring("\t", 1);  /* add a tab before it */
    lua_writestring(s, l);  /* print it */
    lua_ತೆಗೆ(L, 1);  /* pop result */
  }
  lua_writeline();
  ಹಿಂತಿರುಗಿ 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_warn (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);  /* number of arguments */
  ಪೂರ್ಣಾಂಕ i;
  luaL_checkstring(L, 1);  /* at least one argument */
  ಗಾಗಿ (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  ಗಾಗಿ (i = 1; i < n; i++)  /* compose warning */
    lua_warning(L, lua_tostring(L, i), 1);
  lua_warning(L, lua_tostring(L, n), 0);  /* close warning */
  ಹಿಂತಿರುಗಿ 0;
}


#ವ್ಯಾಖ್ಯಾನಿಸು SPACECHARS	" \f\n\r\t\v"

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b_str2int (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಚಿಹ್ನೆರಹಿತ ತಳ, lua_Integer *pn) {
  lua_Unsigned n = 0;
  ಪೂರ್ಣಾಂಕ neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  ಆದರೆ (*s == '-') { s++; neg = 1; }  /* handle sign */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*s == '+') s++;
  ಆದರೆ (!ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(cast_uchar(*s)))  /* no digit? */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  ಮಾಡು {
    ಚಿಹ್ನೆರಹಿತ digit = cast_uint(ಅಂಕಿಯೇ(cast_uchar(*s))
                               ? *s - '0'
                               : (ದೊಡ್ಡ_ಅಕ್ಷರ(cast_uchar(*s)) - 'A') + 10);
    ಆದರೆ (digit >= ತಳ) ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* invalid numeral */
    n = n * ತಳ + digit;
    s++;
  } ಆಗ (ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(cast_uchar(*s)));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  ಹಿಂತಿರುಗಿ s;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_tonumber (lua_State *L) {
  ಆದರೆ (lua_isnoneornil(L, 2)) {  /* standard conversion? */
    ಆದರೆ (lua_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      lua_settop(L, 1);  /* yes; return it */
      ಹಿಂತಿರುಗಿ 1;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      size_t l;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_tolstring(L, 1, &l);
      ಆದರೆ (s != ಖಾಲಿ && lua_stringtonumber(L, s) == l + 1)
        ಹಿಂತಿರುಗಿ 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    size_t l;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
    lua_Integer n = 0;  /* to avoid warnings */
    lua_Integer ತಳ = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = lua_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= ತಳ && ತಳ <= 36, 2, "base out of range");
    ಆದರೆ (b_str2int(s, cast_uint(ತಳ), &n) == s + l) {
      lua_pushinteger(L, n);
      ಹಿಂತಿರುಗಿ 1;
    }  /* else not a number */
  }  /* else not a number */
  luaL_pushfail(L);  /* not a number */
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_error (lua_State *L) {
  ಪೂರ್ಣಾಂಕ level = (ಪೂರ್ಣಾಂಕ)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  ಆದರೆ (lua_type(L, 1) == LUA_TSTRING && level > 0) {
    luaL_where(L, level);   /* add extra information */
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  ಹಿಂತಿರುಗಿ lua_error(L);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  ಆದರೆ (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    ಹಿಂತಿರುಗಿ 1;  /* no metatable */
  }
  luaL_getmetafield(L, 1, "__metatable");
  ಹಿಂತಿರುಗಿ 1;  /* returns either __metatable field (if present) or metatable */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_setmetatable (lua_State *L) {
  ಪೂರ್ಣಾಂಕ t = lua_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  ಆದರೆ (l_unlikely(luaL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    ಹಿಂತಿರುಗಿ luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_rawlen (lua_State *L) {
  ಪೂರ್ಣಾಂಕ t = lua_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  lua_pushinteger(L, l_castU2S(lua_rawlen(L, 1)));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pushmode (lua_State *L, ಪೂರ್ಣಾಂಕ oldmode) {
  ಆದರೆ (oldmode == -1)
    luaL_pushfail(L);  /* invalid call to 'lua_gc' */
  ಇಲ್ಲದಿದ್ದರೆ
    lua_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  ಹಿಂತಿರುಗಿ 1;
}


/*
** check whether call to 'lua_gc' was valid (not inside a finalizer)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು checkvalres(res) { ಆದರೆ (res == -1) ನಿಲ್ಲಿಸು; }

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_collectgarbage (lua_State *L) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ opts[] = {"stop", "restart", "collect",
    "count", "step", "isrunning", "generational", "incremental",
    "param", ಖಾಲಿ};
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LUA_GCSTEP, LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC,
    LUA_GCPARAM};
  ಪೂರ್ಣಾಂಕ o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  ಆಯ್ಕೆ (o) {
    ಸ್ಥಿತಿ LUA_GCCOUNT: {
      ಪೂರ್ಣಾಂಕ k = lua_gc(L, o);
      ಪೂರ್ಣಾಂಕ b = lua_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)k + ((lua_ಸಂಖ್ಯೆ)b/1024));
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ LUA_GCSTEP: {
      lua_Integer n = luaL_optinteger(L, 2, 0);
      ಪೂರ್ಣಾಂಕ res = lua_gc(L, o, cast_sizet(n));
      checkvalres(res);
      lua_pushboolean(L, res);
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ LUA_GCISRUNNING: {
      ಪೂರ್ಣಾಂಕ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushboolean(L, res);
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ LUA_GCGEN: {
      ಹಿಂತಿರುಗಿ pushmode(L, lua_gc(L, o));
    }
    ಸ್ಥಿತಿ LUA_GCINC: {
      ಹಿಂತಿರುಗಿ pushmode(L, lua_gc(L, o));
    }
    ಸ್ಥಿತಿ LUA_GCPARAM: {
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ params[] = {
        "minormul", "majorminor", "minormajor",
        "pause", "stepmul", "stepsize", ಖಾಲಿ};
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ pnum[] = {
        LUA_GCPMINORMUL, LUA_GCPMAJORMINOR, LUA_GCPMINORMAJOR,
        LUA_GCPPAUSE, LUA_GCPSTEPMUL, LUA_GCPSTEPSIZE};
      ಪೂರ್ಣಾಂಕ p = pnum[luaL_checkoption(L, 2, ಖಾಲಿ, params)];
      lua_Integer ಮೌಲ್ಯ = luaL_optinteger(L, 3, -1);
      lua_pushinteger(L, lua_gc(L, o, p, (ಪೂರ್ಣಾಂಕ)ಮೌಲ್ಯ));
      ಹಿಂತಿರುಗಿ 1;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪೂರ್ಣಾಂಕ res = lua_gc(L, o);
      checkvalres(res);
      lua_pushinteger(L, res);
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  luaL_pushfail(L);  /* invalid call (inside a finalizer) */
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_type (lua_State *L) {
  ಪೂರ್ಣಾಂಕ t = lua_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  lua_pushstring(L, lua_typename(L, t));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lua_B_ಮುಂದಿನ (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);  /* create a 2nd argument if there isn't one */
  ಆದರೆ (lua_ಮುಂದಿನ(L, 1))
    ಹಿಂತಿರುಗಿ 2;
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushnil(L);
    ಹಿಂತಿರುಗಿ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pairscont (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, lua_KContext k) {
  (ಶೂನ್ಯ)L; (ಶೂನ್ಯ)ಸ್ಥಿತಿ_2; (ಶೂನ್ಯ)k;  /*ಬಳಸದ*/
  ಹಿಂತಿರುಗಿ 4;  /* __pairs did all the work, just return its results */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lua_B_ಜೋಡಿಗಳು (lua_State *L) {
  luaL_checkany(L, 1);
  ಆದರೆ (luaL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    lua_pushcfunction(L, lua_B_ಮುಂದಿನ);  /* will return generator and */
    lua_pushvalue(L, 1);  /* state */
    lua_pushnil(L);  /* initial value */
    lua_pushnil(L);  /* to-be-closed object */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushvalue(L, 1);  /* argument 'self' to metamethod */
    lua_callk(L, 1, 4, 0, pairscont);  /* get 4 values from metamethod */
  }
  ಹಿಂತಿರುಗಿ 4;
}


/*
** Traversal function for 'ipairs'
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = luaL_intop(+, i, 1);
  lua_pushinteger(L, i);
  ಹಿಂತಿರುಗಿ (lua_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcfunction(L, ipairsaux);  /* iteration function */
  lua_pushvalue(L, 1);  /* state */
  lua_pushinteger(L, 0);  /* initial value */
  ಹಿಂತಿರುಗಿ 3;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತುಂಬು_aux (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, ಪೂರ್ಣಾಂಕ envidx) {
  ಆದರೆ (l_likely(ಸ್ಥಿತಿ_2 == LUA_OK)) {
    ಆದರೆ (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      ಆದರೆ (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_ತೆಗೆ(L, 1);  /* remove 'env' if not used by previous call */
    }
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* error (message is on top of the stack) */
    luaL_pushfail(L);
    lua_ಸೇರಿಸು(L, -2);  /* put before error message */
    ಹಿಂತಿರುಗಿ 2;  /* return fail plus error message */
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_ಬಹುಲಕ (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ = luaL_optstring(L, idx, ಖಾಲಿ);
  ಆದರೆ (ಬಹುಲಕ != ಖಾಲಿ && strchr(ಬಹುಲಕ, 'B') != ಖಾಲಿ) {
    /* Lua code cannot use fixed buffers */
    luaL_argerror(L, idx, "invalid mode");
  }
  ಹಿಂತಿರುಗಿ ಬಹುಲಕ;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_loadfile (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname = luaL_optstring(L, 1, ಖಾಲಿ);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ = get_ಬಹುಲಕ(L, 2);
  ಪೂರ್ಣಾಂಕ env = (!lua_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = luaL_loadfilex(L, fname, ಬಹುಲಕ);
  ಹಿಂತಿರುಗಿ ತುಂಬು_aux(L, ಸ್ಥಿತಿ_2, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** Reader for generic 'load' function.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *generic_reader (lua_State *L, ಶೂನ್ಯ *ud, size_t *ಗಾತ್ರ_2) {
  ಪೂರ್ಣಾಂಕ *firstcall = cast(ಪೂರ್ಣಾಂಕ *, ud);
  luaL_checkstack(L, 2, "too many nested functions");
  ಆದರೆ (*firstcall)
    *firstcall = 0;
  ಇಲ್ಲದಿದ್ದರೆ
    lua_ತೆಗೆ(L, 1);  /* remove previous result */
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  ಆದರೆ (lua_isnil(L, -1)) {
    *ಗಾತ್ರ_2 = 0;
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l_unlikely(!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  ಹಿಂತಿರುಗಿ lua_tolstring(L, -1, ಗಾತ್ರ_2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lua_B_ತುಂಬು (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  size_t l;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_tolstring(L, 1, &l);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ = get_ಬಹುಲಕ(L, 3);
  ಪೂರ್ಣಾಂಕ env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  ಆದರೆ (s != ಖಾಲಿ) {  /* loading a string? */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *chunkname = luaL_optstring(L, 2, s);
    ಸ್ಥಿತಿ_2 = luaL_loadbufferx(L, s, l, chunkname, ಬಹುಲಕ);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* loading from a reader function */
    ಪೂರ್ಣಾಂಕ firstcall = 1;  /* userdata for generic_reader */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    ಸ್ಥಿತಿ_2 = lua_ತುಂಬು(L, generic_reader, &firstcall, chunkname, ಬಹುಲಕ);
  }
  ಹಿಂತಿರುಗಿ ತುಂಬು_aux(L, ಸ್ಥಿತಿ_2, env);
}

/* }====================================================== */


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dofilecont (lua_State *L, ಪೂರ್ಣಾಂಕ d1, lua_KContext d2) {
  (ಶೂನ್ಯ)d1;  (ಶೂನ್ಯ)d2;  /* only to match 'lua_Kfunction' prototype */
  ಹಿಂತಿರುಗಿ lua_gettop(L) - 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_dofile (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname = luaL_optstring(L, 1, ಖಾಲಿ);
  lua_settop(L, 1);
  ಆದರೆ (l_unlikely(luaL_loadfilex(L, fname, "bt") != LUA_OK))
    ಹಿಂತಿರುಗಿ lua_error(L);
  lua_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  ಹಿಂತಿರುಗಿ dofilecont(L, 0, 0);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_assert (lua_State *L) {
  ಆದರೆ (l_likely(lua_toboolean(L, 1)))  /* condition is true? */
    ಹಿಂತಿರುಗಿ lua_gettop(L);  /* return all arguments */
  ಇಲ್ಲದಿದ್ದರೆ {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    lua_remove(L, 1);  /* remove it */
    lua_pushliteral(L, "assertion failed!");  /* default message */
    lua_settop(L, 1);  /* leave only message (default if no other one) */
    ಹಿಂತಿರುಗಿ luaB_error(L);  /* call 'error' */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_select (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);
  ಆದರೆ (lua_type(L, 1) == LUA_TSTRING && *lua_tostring(L, 1) == '#') {
    lua_pushinteger(L, n-1);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_Integer i = luaL_checkinteger(L, 1);
    ಆದರೆ (i < 0) i = n + i;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    ಹಿಂತಿರುಗಿ n - (ಪೂರ್ಣಾಂಕ)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ finishpcall (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, lua_KContext extra) {
  ಆದರೆ (l_unlikely(ಸ್ಥಿತಿ_2 != LUA_OK && ಸ್ಥಿತಿ_2 != LUA_YIELD)) {  /* error? */
    lua_pushboolean(L, 0);  /* first result (false) */
    lua_pushvalue(L, -2);  /* error message */
    ಹಿಂತಿರುಗಿ 2;  /* return false, msg */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ lua_gettop(L) - (ಪೂರ್ಣಾಂಕ)extra;  /* return all results */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_pcall (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);  /* first result if no errors */
  lua_ಸೇರಿಸು(L, 1);  /* put it in place */
  ಸ್ಥಿತಿ_2 = lua_pcallk(L, lua_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  ಹಿಂತಿರುಗಿ finishpcall(L, ಸ್ಥಿತಿ_2, 0);
}


/*
** Do a protected call with error handling. After 'lua_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_xpcall (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  lua_pushboolean(L, 1);  /* first result */
  lua_pushvalue(L, 1);  /* function */
  lua_rotate(L, 3, 2);  /* move them below function's arguments */
  ಸ್ಥಿತಿ_2 = lua_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  ಹಿಂತಿರುಗಿ finishpcall(L, ಸ್ಥಿತಿ_2, 2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg ತಳ_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", lua_B_ತುಂಬು},
  {"next", lua_B_ಮುಂದಿನ},
  {"pairs", lua_B_ಜೋಡಿಗಳು},
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
  {LUA_GNAME, ಖಾಲಿ},
  {"_VERSION", ಖಾಲಿ},
  {ಖಾಲಿ, ಖಾಲಿ}
};


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_ತಳ (lua_State *L) {
  /* open lib into global table */
  lua_pushglobaltable(L);
  luaL_setfuncs(L, ತಳ_funcs, 0);
  /* set global _G */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  lua_pushliteral(L, LUA_ಆವೃತ್ತಿ);
  lua_setfield(L, -2, "_VERSION");
  ಹಿಂತಿರುಗಿ 1;
}

