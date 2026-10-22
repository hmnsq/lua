/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lstrlib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <float.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <locale.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#ಆದರೆ !defined(LUA_MAXCAPTURES)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MAXCAPTURES		32
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
ಸ್ಥಿರ size_t posrelatI (lua_Integer pos, size_t len) {
  ಆದರೆ (pos > 0)
    ಹಿಂತಿರುಗಿ (size_t)pos;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pos == 0)
    ಹಿಂತಿರುಗಿ 1;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pos < -(lua_Integer)len)  /* inverted comparison */
    ಹಿಂತಿರುಗಿ 1;  /* clip to 1 */
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ಸ್ಥಿರ size_t getendpos (lua_State *L, ಪೂರ್ಣಾಂಕ arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  ಆದರೆ (pos > (lua_Integer)len)
    ಹಿಂತಿರುಗಿ len;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pos >= 0)
    ಹಿಂತಿರುಗಿ (size_t)pos;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pos < -(lua_Integer)len)
    ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ len + (size_t)pos + 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_sub (lua_State *L) {
  size_t l;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &l);
  size_t ಆರಂಭ = posrelatI(luaL_checkinteger(L, 2), l);
  size_t ಕೊನೆ = getendpos(L, 3, -1, l);
  ಆದರೆ (ಆರಂಭ <= ಕೊನೆ)
    lua_pushlstring(L, s + ಆರಂಭ - 1, (ಕೊನೆ - ಆರಂಭ) + 1);
  ಇಲ್ಲದಿದ್ದರೆ lua_pushliteral(L, "");
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_ಹಿಮ್ಮುಖ (lua_State *L) {
  size_t l, i;
  lua_L_ಬಫರ್ b;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &l);
  ಅಕ್ಷರ *p = luaL_buffinitsize(L, &b, l);
  ಗಾಗಿ (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_ಅಧೋ (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_ಬಫರ್ b;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &l);
  ಅಕ್ಷರ *p = luaL_buffinitsize(L, &b, l);
  ಗಾಗಿ (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_ಊರ್ಧ್ವ (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_ಬಫರ್ b;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &l);
  ಅಕ್ಷರ *p = luaL_buffinitsize(L, &b, l);
  ಗಾಗಿ (i=0; i<l; i++)
    p[i] = cast_char(ದೊಡ್ಡ_ಅಕ್ಷರ(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_rep (lua_State *L) {
  size_t len, lsep;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sep = luaL_optlstring(L, 3, "", &lsep);
  ಆದರೆ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l_unlikely(len > MAX_ಗಾತ್ರ - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_ಗಾತ್ರ) / n))
    ಹಿಂತಿರುಗಿ luaL_error(L, "resulting string too large");
  ಇಲ್ಲದಿದ್ದರೆ {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    lua_L_ಬಫರ್ b;
    ಅಕ್ಷರ *p = luaL_buffinitsize(L, &b, totallen);
    ಆಗ (n-- > 1) {  /* first n-1 copies (followed by separator) */
      ಸ್ಮರಣೆ_ನಕಲು(p, s, len * ಗಾತ್ರ(ಅಕ್ಷರ)); p += len;
      ಆದರೆ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        ಸ್ಮರಣೆ_ನಕಲು(p, sep, lsep * ಗಾತ್ರ(ಅಕ್ಷರ)); p += lsep;
      }
    }
    ಸ್ಮರಣೆ_ನಕಲು(p, s, len * ಗಾತ್ರ(ಅಕ್ಷರ));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_byte (lua_State *L) {
  size_t l;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  ಪೂರ್ಣಾಂಕ n, i;
  ಆದರೆ (posi > pose) ಹಿಂತಿರುಗಿ 0;  /* empty interval; return no values */
  ಆದರೆ (l_unlikely(pose - posi >= (size_t)ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ))  /* arithmetic overflow? */
    ಹಿಂತಿರುಗಿ luaL_error(L, "string slice too long");
  n = (ಪೂರ್ಣಾಂಕ)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  ಗಾಗಿ (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  ಹಿಂತಿರುಗಿ n;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_char (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);  /* number of arguments */
  ಪೂರ್ಣಾಂಕ i;
  lua_L_ಬಫರ್ b;
  ಅಕ್ಷರ *p = luaL_buffinitsize(L, &b, cast_uint(n));
  ಗಾಗಿ (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
ರಚನೆ str_Writer {
  ಪೂರ್ಣಾಂಕ init;  /* true iff buffer has been initialized */
  lua_L_ಬಫರ್ B;
};


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ writer (lua_State *L, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b, size_t ಗಾತ್ರ_2, ಶೂನ್ಯ *ud) {
  ರಚನೆ str_Writer *state = (ರಚನೆ str_Writer *)ud;
  ಆದರೆ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  ಆದರೆ (b == ಖಾಲಿ) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    luaL_addlstring(&state->B, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)b, ಗಾತ್ರ_2);
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_dump (lua_State *L) {
  ರಚನೆ str_Writer state;
  ಪೂರ್ಣಾಂಕ strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  ಹಿಂತಿರುಗಿ 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#ಆದರೆ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg stringmetamethods[] = {
  {"__index", ಖಾಲಿ},  /* placeholder */
  {ಖಾಲಿ, ಖಾಲಿ}
};

#ಇಲ್ಲದಿದ್ದರೆ		/* }{ */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tonum (lua_State *L, ಪೂರ್ಣಾಂಕ arg) {
  ಆದರೆ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* check whether it is a numerical string */
    size_t len;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_tolstring(L, arg, &len);
    ಹಿಂತಿರುಗಿ (s != ಖಾಲಿ && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
ಸ್ಥಿರ ಶೂನ್ಯ trymt (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mtkey, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  ಆದರೆ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_ಸೇರಿಸು(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith (lua_State *L, ಪೂರ್ಣಾಂಕ op, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mtname) {
  ಆದರೆ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  ಇಲ್ಲದಿದ್ದರೆ
    trymt(L, mtname, mtname + 2);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_ಕೂಡಿಸು (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPADD, "__add");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_sub (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPSUB, "__sub");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_mul (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPMUL, "__mul");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_mod (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPMOD, "__mod");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_pow (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPPOW, "__pow");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_div (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPDIV, "__div");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_idiv (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPIDIV, "__idiv");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ arith_unm (lua_State *L) {
  ಹಿಂತಿರುಗಿ arith(L, LUA_OPUNM, "__unm");
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg stringmetamethods[] = {
  {"__add", arith_ಕೂಡಿಸು},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", ಖಾಲಿ},  /* placeholder */
  {ಖಾಲಿ, ಖಾಲಿ}
};

#ಆದರೆ_ಅಂತ್ಯ		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#ವ್ಯಾಖ್ಯಾನಿಸು CAP_UNFINISHED	(-1)
#ವ್ಯಾಖ್ಯಾನಿಸು CAP_POSITION	(-2)


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಹೊಂದಿಕೆ_State {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src_init;  /* init of source string */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src_ಕೊನೆ;  /* end ('\0') of source string */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ಕೊನೆ;  /* end ('\0') of pattern */
  lua_State *L;
  ಪೂರ್ಣಾಂಕ matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  ಪೂರ್ಣಾಂಕ level;  /* total number of captures (finished or unfinished) */
  ರಚನೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} ಹೊಂದಿಕೆ_State;


/* recursive function */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಹೊಂದಿಕೆ (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p);


/* maximum recursion depth for 'match' */
#ಆದರೆ !defined(MAXCCALLS)
#ವ್ಯಾಖ್ಯಾನಿಸು MAXCCALLS	200
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿಸು L_ESC		'%'
#ವ್ಯಾಖ್ಯಾನಿಸು SPECIALS	"^$*+?.([%-"


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_capture (ಹೊಂದಿಕೆ_State *ms, ಪೂರ್ಣಾಂಕ l) {
  l -= '1';
  ಆದರೆ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    ಹಿಂತಿರುಗಿ luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  ಹಿಂತಿರುಗಿ l;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ capture_ಗಮ್ಯ_close (ಹೊಂದಿಕೆ_State *ms) {
  ಪೂರ್ಣಾಂಕ level = ms->level;
  ಗಾಗಿ (level--; level>=0; level--)
    ಆದರೆ (ms->capture[level].len == CAP_UNFINISHED) ಹಿಂತಿರುಗಿ level;
  ಹಿಂತಿರುಗಿ luaL_error(ms->L, "invalid pattern capture");
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *classend (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p) {
  ಆಯ್ಕೆ (*p++) {
    ಸ್ಥಿತಿ L_ESC: {
      ಆದರೆ (l_unlikely(p == ms->p_ಕೊನೆ))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      ಹಿಂತಿರುಗಿ p+1;
    }
    ಸ್ಥಿತಿ '[': {
      ಆದರೆ (*p == '^') p++;
      ಮಾಡು {  /* look for a ']' */
        ಆದರೆ (l_unlikely(p == ms->p_ಕೊನೆ))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        ಆದರೆ (*(p++) == L_ESC && p < ms->p_ಕೊನೆ)
          p++;  /* skip escapes (e.g. '%]') */
      } ಆಗ (*p != ']');
      ಹಿಂತಿರುಗಿ p+1;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಹಿಂತಿರುಗಿ p;
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಹೊಂದಿಕೆ_class (ಪೂರ್ಣಾಂಕ c, ಪೂರ್ಣಾಂಕ cl) {
  ಪೂರ್ಣಾಂಕ res;
  ಆಯ್ಕೆ (tolower(cl)) {
    ಸ್ಥಿತಿ 'a' : res = ಅಕ್ಷರವೇ(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'c' : res = iscntrl(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'd' : res = ಅಂಕಿಯೇ(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'g' : res = isgraph(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'l' : res = islower(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'p' : res = ispunct(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 's' : res = ಖಾಲಿಯೇ(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'u' : res = isupper(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'w' : res = ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'x' : res = isxdigit(c); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'z' : res = (c == 0); ನಿಲ್ಲಿಸು;  /* deprecated option */
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ (cl == c);
  }
  ಹಿಂತಿರುಗಿ (islower(cl) ? res : !res);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ matchbracketclass (ಪೂರ್ಣಾಂಕ c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ec) {
  ಪೂರ್ಣಾಂಕ sig = 1;
  ಆದರೆ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  ಆಗ (++p < ec) {
    ಆದರೆ (*p == L_ESC) {
      p++;
      ಆದರೆ (ಹೊಂದಿಕೆ_class(c, cast_uchar(*p)))
        ಹಿಂತಿರುಗಿ sig;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      ಆದರೆ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        ಹಿಂತಿರುಗಿ sig;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cast_uchar(*p) == c) ಹಿಂತಿರುಗಿ sig;
  }
  ಹಿಂತಿರುಗಿ !sig;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ singlematch (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p,
                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ep) {
  ಆದರೆ (s >= ms->src_ಕೊನೆ)
    ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ c = cast_uchar(*s);
    ಆಯ್ಕೆ (*p) {
      ಸ್ಥಿತಿ '.': ಹಿಂತಿರುಗಿ 1;  /* matches any char */
      ಸ್ಥಿತಿ L_ESC: ಹಿಂತಿರುಗಿ ಹೊಂದಿಕೆ_class(c, cast_uchar(*(p+1)));
      ಸ್ಥಿತಿ '[': ಹಿಂತಿರುಗಿ matchbracketclass(c, p, ep-1);
      ಪೂರ್ವನಿಯೋಜಿತ:  ಹಿಂತಿರುಗಿ (cast_uchar(*p) == c);
    }
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *matchbalance (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p) {
  ಆದರೆ (l_unlikely(p >= ms->p_ಕೊನೆ - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  ಆದರೆ (*s != *p) ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ b = *p;
    ಪೂರ್ಣಾಂಕ e = *(p+1);
    ಪೂರ್ಣಾಂಕ cont = 1;
    ಆಗ (++s < ms->src_ಕೊನೆ) {
      ಆದರೆ (*s == e) {
        ಆದರೆ (--cont == 0) ಹಿಂತಿರುಗಿ s+1;
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*s == b) cont++;
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* string ends out of balance */
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *max_expand (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  ಆಗ (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  ಆಗ (i>=0) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *res = ಹೊಂದಿಕೆ(ms, (s+i), ep+1);
    ಆದರೆ (res) ಹಿಂತಿರುಗಿ res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *min_expand (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ep) {
  ಗಾಗಿ (;;) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *res = ಹೊಂದಿಕೆ(ms, s, ep+1);
    ಆದರೆ (res != ಖಾಲಿ)
      ಹಿಂತಿರುಗಿ res;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ_capture (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಪೂರ್ಣಾಂಕ what) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *res;
  ಪೂರ್ಣಾಂಕ level = ms->level;
  ಆದರೆ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  ಆದರೆ ((res=ಹೊಂದಿಕೆ(ms, s, p)) == ಖಾಲಿ)  /* match failed? */
    ms->level--;  /* undo capture */
  ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ_capture (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p) {
  ಪೂರ್ಣಾಂಕ l = capture_ಗಮ್ಯ_close(ms);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  ಆದರೆ ((res = ಹೊಂದಿಕೆ(ms, s, p)) == ಖಾಲಿ)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  ಹಿಂತಿರುಗಿ res;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಹೊಂದಿಕೆ_capture (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  ಆದರೆ ((size_t)(ms->src_ಕೊನೆ-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    ಹಿಂತಿರುಗಿ s+len;
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಹೊಂದಿಕೆ (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p) {
  ಆದರೆ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  ಆದರೆ (p != ms->p_ಕೊನೆ) {  /* end of pattern? */
    ಆಯ್ಕೆ (*p) {
      ಸ್ಥಿತಿ '(': {  /* start capture */
        ಆದರೆ (*(p + 1) == ')')  /* position capture? */
          s = ಆರಂಭ_capture(ms, s, p + 2, CAP_POSITION);
        ಇಲ್ಲದಿದ್ದರೆ
          s = ಆರಂಭ_capture(ms, s, p + 1, CAP_UNFINISHED);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ ')': {  /* end capture */
        s = ಕೊನೆ_capture(ms, s, p + 1);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ '$': {
        ಆದರೆ ((p + 1) != ms->p_ಕೊನೆ)  /* is the '$' the last char in pattern? */
          ಹೋಗು dflt;  /* no; go to default */
        s = (s == ms->src_ಕೊನೆ) ? s : ಖಾಲಿ;  /* check end of string */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        ಆಯ್ಕೆ (*(p + 1)) {
          ಸ್ಥಿತಿ 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            ಆದರೆ (s != ಖಾಲಿ) {
              p += 4; ಹೋಗು init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            ನಿಲ್ಲಿಸು;
          }
          ಸ್ಥಿತಿ 'f': {  /* frontier? */
            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ep; ಅಕ್ಷರ ಹಿಂದಿನ;
            p += 2;
            ಆದರೆ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            ಹಿಂದಿನ = (s == ms->src_init) ? '\0' : *(s - 1);
            ಆದರೆ (!matchbracketclass(cast_uchar(ಹಿಂದಿನ), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; ಹೋಗು init;  /* return match(ms, s, ep); */
            }
            s = ಖಾಲಿ;  /* match failed */
            ನಿಲ್ಲಿಸು;
          }
          ಸ್ಥಿತಿ '0': ಸ್ಥಿತಿ '1': ಸ್ಥಿತಿ '2': ಸ್ಥಿತಿ '3':
          ಸ್ಥಿತಿ '4': ಸ್ಥಿತಿ '5': ಸ್ಥಿತಿ '6': ಸ್ಥಿತಿ '7':
          ಸ್ಥಿತಿ '8': ಸ್ಥಿತಿ '9': {  /* capture results (%0-%9)? */
            s = ಹೊಂದಿಕೆ_capture(ms, s, cast_uchar(*(p + 1)));
            ಆದರೆ (s != ಖಾಲಿ) {
              p += 2; ಹೋಗು init;  /* return match(ms, s, p + 2) */
            }
            ನಿಲ್ಲಿಸು;
          }
          ಪೂರ್ವನಿಯೋಜಿತ: ಹೋಗು dflt;
        }
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: dflt: {  /* pattern class plus optional suffix */
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        ಆದರೆ (!singlematch(ms, s, p, ep)) {
          ಆದರೆ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; ಹೋಗು init;  /* return match(ms, s, ep + 1); */
          }
          ಇಲ್ಲದಿದ್ದರೆ  /* '+' or no suffix */
            s = ಖಾಲಿ;  /* fail */
        }
        ಇಲ್ಲದಿದ್ದರೆ {  /* matched once */
          ಆಯ್ಕೆ (*ep) {  /* handle optional suffix */
            ಸ್ಥಿತಿ '?': {  /* optional */
              ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *res;
              ಆದರೆ ((res = ಹೊಂದಿಕೆ(ms, s + 1, ep + 1)) != ಖಾಲಿ)
                s = res;
              ಇಲ್ಲದಿದ್ದರೆ {
                p = ep + 1; ಹೋಗು init;  /* else return match(ms, s, ep + 1); */
              }
              ನಿಲ್ಲಿಸು;
            }
            ಸ್ಥಿತಿ '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            ಸ್ಥಿತಿ '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:  /* no suffix */
              s++; p = ep; ಹೋಗು init;  /* return match(ms, s + 1, ep); */
          }
        }
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ms->matchdepth++;
  ಹಿಂತಿರುಗಿ s;
}



ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lmemfind (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1, size_t l1,
                               ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2, size_t l2) {
  ಆದರೆ (l2 == 0) ಹಿಂತಿರುಗಿ s1;  /* empty strings are everywhere */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l2 > l1) ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* avoids a negative 'l1' */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    ಆಗ (l1 > 0 && (init = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)memchr(s1, *s2, l1)) != ಖಾಲಿ) {
      init++;   /* 1st char is already checked */
      ಆದರೆ (memcmp(init, s2+1, l2) == 0)
        ಹಿಂತಿರುಗಿ init-1;
      ಇಲ್ಲದಿದ್ದರೆ {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
ಸ್ಥಿರ ptrdiff_t get_onecapture (ಹೊಂದಿಕೆ_State *ms, ಪೂರ್ಣಾಂಕ i, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                              ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **cap) {
  ಆದರೆ (i >= ms->level) {
    ಆದರೆ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    ಹಿಂತಿರುಗಿ (e - s);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    ಆದರೆ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    ಹಿಂತಿರುಗಿ capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತಳ್ಳು_onecapture (ಹೊಂದಿಕೆ_State *ms, ಪೂರ್ಣಾಂಕ i, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                                    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  ಆದರೆ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತಳ್ಳು_captures (ಹೊಂದಿಕೆ_State *ms, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  ಗಾಗಿ (i = 0; i < nlevels; i++)
    ತಳ್ಳು_onecapture(ms, i, s, e);
  ಹಿಂತಿರುಗಿ nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nospecials (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, size_t l) {
  size_t upto = 0;
  ಮಾಡು {
    ಆದರೆ (strpbrk(p + upto, SPECIALS))
      ಹಿಂತಿರುಗಿ 0;  /* pattern has a special character */
    upto += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p + upto) + 1;  /* may have more after \0 */
  } ಆಗ (upto <= l);
  ಹಿಂತಿರುಗಿ 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
ಸ್ಥಿರ ಶೂನ್ಯ prepstate (ಹೊಂದಿಕೆ_State *ms, lua_State *L,
                       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_ಕೊನೆ = s + ls;
  ms->p_ಕೊನೆ = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
ಸ್ಥಿರ ಶೂನ್ಯ reprepstate (ಹೊಂದಿಕೆ_State *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_ಕಂಡುಹಿಡಿ_aux (lua_State *L, ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ) {
  size_t ls, lp;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &ls);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  ಆದರೆ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    ಹಿಂತಿರುಗಿ 1;
  }
  /* explicit request or no special characters? */
  ಆದರೆ (ಕಂಡುಹಿಡಿ && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2 = lmemfind(s + init, ls - init, p, lp);
    ಆದರೆ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      ಹಿಂತಿರುಗಿ 2;
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಹೊಂದಿಕೆ_State ms;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1 = s + init;
    ಪೂರ್ಣಾಂಕ anchor = (*p == '^');
    ಆದರೆ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    ಮಾಡು {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *res;
      reprepstate(&ms);
      ಆದರೆ ((res=ಹೊಂದಿಕೆ(&ms, s1, p)) != ಖಾಲಿ) {
        ಆದರೆ (ಕಂಡುಹಿಡಿ) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /*ಆರಂಭ*/
          lua_pushinteger(L, ct_diff2S(res - s));   /*ಕೊನೆ*/
          ಹಿಂತಿರುಗಿ ತಳ್ಳು_captures(&ms, ಖಾಲಿ, 0) + 2;
        }
        ಇಲ್ಲದಿದ್ದರೆ
          ಹಿಂತಿರುಗಿ ತಳ್ಳು_captures(&ms, s1, res);
      }
    } ಆಗ (s1++ < ms.src_ಕೊನೆ && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_ಕಂಡುಹಿಡಿ (lua_State *L) {
  ಹಿಂತಿರುಗಿ str_ಕಂಡುಹಿಡಿ_aux(L, 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_ಹೊಂದಿಕೆ (lua_State *L) {
  ಹಿಂತಿರುಗಿ str_ಕಂಡುಹಿಡಿ_aux(L, 0);
}


/* state for 'gmatch' */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ GMatchState {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src;  /* current position */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;  /*ನಮೂನೆ*/
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lastmatch;  /* end of last match */
  ಹೊಂದಿಕೆ_State ms;  /* match state */
} GMatchState;


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src;
  gm->ms.L = L;
  ಗಾಗಿ (src = gm->src; src <= gm->ms.src_ಕೊನೆ; src++) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e;
    reprepstate(&gm->ms);
    ಆದರೆ ((e = ಹೊಂದಿಕೆ(&gm->ms, src, gm->p)) != ಖಾಲಿ && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      ಹಿಂತಿರುಗಿ ತಳ್ಳು_captures(&gm->ms, src, e);
    }
  }
  ಹಿಂತಿರುಗಿ 0;  /* not found */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gmatch (lua_State *L) {
  size_t ls, lp;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &ls);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, ಗಾತ್ರ(GMatchState), 0);
  ಆದರೆ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = ಖಾಲಿ;
  lua_pushcclosure(L, gmatch_aux, 3);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_s (ಹೊಂದಿಕೆ_State *ms, lua_L_ಬಫರ್ *b, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e) {
  size_t l;
  lua_State *L = ms->L;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *news = lua_tolstring(L, 3, &l);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
  ಆಗ ((p = (ಅಕ್ಷರ *)memchr(news, L_ESC, l)) != ಖಾಲಿ) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    ಆದರೆ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಂಕಿಯೇ(cast_uchar(*p))) {  /* '%n' */
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      ಆದರೆ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      ಇಲ್ಲದಿದ್ದರೆ
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    ಇಲ್ಲದಿದ್ದರೆ
      luaL_error(L, "invalid use of '%c' in replacement string", L_ESC);
    l -= ct_diff2sz(p + 1 - news);
    news = p + 1;
  }
  luaL_addlstring(b, news, l);
}


/*
** Add the replacement value to the string buffer 'b'.
** Return true if the original string was changed. (Function calls and
** table indexing resulting in nil or false do not change the subject.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_ಮೌಲ್ಯ (ಹೊಂದಿಕೆ_State *ms, lua_L_ಬಫರ್ *b, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e, ಪೂರ್ಣಾಂಕ tr) {
  lua_State *L = ms->L;
  ಆಯ್ಕೆ (tr) {
    ಸ್ಥಿತಿ LUA_TFUNCTION: {  /* call the function */
      ಪೂರ್ಣಾಂಕ n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ತಳ್ಳು_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_TTABLE: {  /* index the table */
      ತಳ್ಳು_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* LUA_TNUMBER or LUA_TSTRING */
      ಕೂಡಿಸು_s(ms, b, s, e);  /* add value to the buffer */
      ಹಿಂತಿರುಗಿ 1;  /* something changed */
    }
  }
  ಆದರೆ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_ತೆಗೆ(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    ಹಿಂತಿರುಗಿ 0;  /* no changes */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l_unlikely(!lua_isstring(L, -1)))
    ಹಿಂತಿರುಗಿ luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  ಇಲ್ಲದಿದ್ದರೆ {
    luaL_addvalue(b);  /* add result to accumulator */
    ಹಿಂತಿರುಗಿ 1;  /* something changed */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_gsub (lua_State *L) {
  size_t srcl, lp;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = luaL_checklstring(L, 2, &lp);  /*ನಮೂನೆ*/
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lastmatch = ಖಾಲಿ;  /* end of last match */
  ಪೂರ್ಣಾಂಕ tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  ಪೂರ್ಣಾಂಕ anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  ಪೂರ್ಣಾಂಕ ಬದಲಾಯಿತು = 0;  /* change flag */
  ಹೊಂದಿಕೆ_State ms;
  lua_L_ಬಫರ್ b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  ಆದರೆ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  ಆಗ (n < max_s) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    ಆದರೆ ((e = ಹೊಂದಿಕೆ(&ms, src, p)) != ಖಾಲಿ && e != lastmatch) {  /* match? */
      n++;
      ಬದಲಾಯಿತು = ಕೂಡಿಸು_ಮೌಲ್ಯ(&ms, &b, src, e, tr) || ಬದಲಾಯಿತು;
      src = lastmatch = e;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (src < ms.src_ಕೊನೆ)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    ಇಲ್ಲದಿದ್ದರೆ ನಿಲ್ಲಿಸು;  /* end of subject */
    ಆದರೆ (anchor) ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ (!ಬದಲಾಯಿತು)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  ಇಲ್ಲದಿದ್ದರೆ {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_ಕೊನೆ - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  ಹಿಂತಿರುಗಿ 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#ಆದರೆ !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#ವ್ಯಾಖ್ಯಾನಿಸು SIZELENMOD	(ಗಾತ್ರ(LUA_ಸಂಖ್ಯೆ_FRMLEN)/ಗಾತ್ರ(ಅಕ್ಷರ))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ adddigit (ಅಕ್ಷರ *buff, ಚಿಹ್ನೆರಹಿತ n, lua_ಸಂಖ್ಯೆ x) {
  lua_ಸಂಖ್ಯೆ dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  ಪೂರ್ಣಾಂಕ d = (ಪೂರ್ಣಾಂಕ)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  ಹಿಂತಿರುಗಿ x - dd;  /* return what is left */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ num2straux (ಅಕ್ಷರ *buff, ಚಿಹ್ನೆರಹಿತ sz, lua_ಸಂಖ್ಯೆ x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  ಆದರೆ (x != x || x == (lua_ಸಂಖ್ಯೆ)HUGE_VAL || x == -(lua_ಸಂಖ್ಯೆ)HUGE_VAL)
    ಹಿಂತಿರುಗಿ l_sprintf(buff, sz, LUA_ಸಂಖ್ಯೆ_FMT, (LUAI_UACNUMBER)x);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    ಹಿಂತಿರುಗಿ l_sprintf(buff, sz, LUA_ಸಂಖ್ಯೆ_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ e;
    lua_ಸಂಖ್ಯೆ m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    ಚಿಹ್ನೆರಹಿತ n = 0;  /* character count */
    ಆದರೆ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    ಆದರೆ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      ಮಾಡು {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } ಆಗ (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    ಹಿಂತಿರುಗಿ cast_int(n);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lua_number2strx (lua_State *L, ಅಕ್ಷರ *buff, ಚಿಹ್ನೆರಹಿತ sz,
                            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, lua_ಸಂಖ್ಯೆ x) {
  ಪೂರ್ಣಾಂಕ n = num2straux(buff, sz, x);
  ಆದರೆ (fmt[SIZELENMOD] == 'A') {
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < n; i++)
      buff[i] = cast_char(ದೊಡ್ಡ_ಅಕ್ಷರ(cast_uchar(buff[i])));
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    ಹಿಂತಿರುಗಿ luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  ಹಿಂತಿರುಗಿ n;
}

#ಆದರೆ_ಅಂತ್ಯ				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_ಅಂಶ	120


/* valid flags in a format specification */
#ಆದರೆ !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#ವ್ಯಾಖ್ಯಾನಿಸು L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#ವ್ಯಾಖ್ಯಾನಿಸು L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#ವ್ಯಾಖ್ಯಾನಿಸು L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#ವ್ಯಾಖ್ಯಾನಿಸು L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#ವ್ಯಾಖ್ಯಾನಿಸು L_FMTFLAGSC	"-"

#ಆದರೆ_ಅಂತ್ಯ


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_FORMAT	32


ಸ್ಥಿರ ಶೂನ್ಯ addquoted (lua_L_ಬಫರ್ *b, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t len) {
  luaL_addchar(b, '"');
  ಆಗ (len--) {
    ಆದರೆ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (iscntrl(cast_uchar(*s))) {
      ಅಕ್ಷರ buff[10];
      ಆದರೆ (!ಅಂಕಿಯೇ(cast_uchar(*(s+1))))
        l_sprintf(buff, ಗಾತ್ರ(buff), "\\%d", (ಪೂರ್ಣಾಂಕ)cast_uchar(*s));
      ಇಲ್ಲದಿದ್ದರೆ
        l_sprintf(buff, ಗಾತ್ರ(buff), "\\%03d", (ಪೂರ್ಣಾಂಕ)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    ಇಲ್ಲದಿದ್ದರೆ
      luaL_addchar(b, *s);
    s++;
  }
  luaL_addchar(b, '"');
}


/*
** Serialize a floating-point number in such a way that it can be
** scanned back by Lua. Use hexadecimal format for "common" numbers
** (to preserve precision); inf, -inf, and NaN are handled separately.
** (NaN cannot be expressed as a numeral, so we write '(0/0)' for it.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ quotefloat (lua_State *L, ಅಕ್ಷರ *buff, lua_ಸಂಖ್ಯೆ n) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;  /* for the fixed representations */
  ಆದರೆ (n == (lua_ಸಂಖ್ಯೆ)HUGE_VAL)  /* inf? */
    s = "1e9999";
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (n == -(lua_ಸಂಖ್ಯೆ)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (n != n)  /* NaN? */
    s = "(0/0)";
  ಇಲ್ಲದಿದ್ದರೆ {  /* format number as hexadecimal */
    ಪೂರ್ಣಾಂಕ  nb = lua_number2strx(L, buff, MAX_ಅಂಶ,
                                 "%" LUA_ಸಂಖ್ಯೆ_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    ಆದರೆ (memchr(buff, '.', cast_uint(nb)) == ಖಾಲಿ) {  /* no dot? */
      ಅಕ್ಷರ ಬಿಂದು = lua_getlocaledecpoint();  /* try locale point */
      ಅಕ್ಷರ *ppoint = (ಅಕ್ಷರ *)memchr(buff, ಬಿಂದು, cast_uint(nb));
      ಆದರೆ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    ಹಿಂತಿರುಗಿ nb;
  }
  /* for the fixed representations */
  ಹಿಂತಿರುಗಿ l_sprintf(buff, MAX_ಅಂಶ, "%s", s);
}


ಸ್ಥಿರ ಶೂನ್ಯ addliteral (lua_State *L, lua_L_ಬಫರ್ *b, ಪೂರ್ಣಾಂಕ arg) {
  ಆಯ್ಕೆ (lua_type(L, arg)) {
    ಸ್ಥಿತಿ LUA_TSTRING: {
      size_t len;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_TNUMBER: {
      ಅಕ್ಷರ *buff = luaL_prepbuffsize(b, MAX_ಅಂಶ);
      ಪೂರ್ಣಾಂಕ nb;
      ಆದರೆ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      ಇಲ್ಲದಿದ್ದರೆ {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ಅಂಶ, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_TNIL: ಸ್ಥಿತಿ LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, ಖಾಲಿ);
      luaL_addvalue(b);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get2digits (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s) {
  ಆದರೆ (ಅಂಕಿಯೇ(cast_uchar(*s))) {
    s++;
    ಆದರೆ (ಅಂಕಿಯೇ(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  ಹಿಂತಿರುಗಿ s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkformat (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *form, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *flags,
                                       ಪೂರ್ಣಾಂಕ precision) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  ಆದರೆ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    ಆದರೆ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  ಆದರೆ (!ಅಕ್ಷರವೇ(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getformat (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *strfrmt,
                                            ಅಕ್ಷರ *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  ಆದರೆ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  ಸ್ಮರಣೆ_ನಕಲು(form, strfrmt, len * ಗಾತ್ರ(ಅಕ್ಷರ));
  *(form + len) = '\0';
  ಹಿಂತಿರುಗಿ strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
ಸ್ಥಿರ ಶೂನ್ಯ addlenmod (ಅಕ್ಷರ *form, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lenmod) {
  size_t l = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(form);
  size_t lm = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(lenmod);
  ಅಕ್ಷರ spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_format (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ತುದಿ = lua_gettop(L);
  ಪೂರ್ಣಾಂಕ arg = 1;
  size_t sfl;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *strfrmt = luaL_checklstring(L, arg, &sfl);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *strfrmt_ಕೊನೆ = strfrmt+sfl;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *flags;
  lua_L_ಬಫರ್ b;
  luaL_buffinit(L, &b);
  ಆಗ (strfrmt < strfrmt_ಕೊನೆ) {
    ಆದರೆ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    ಇಲ್ಲದಿದ್ದರೆ { /* format item */
      ಅಕ್ಷರ form[MAX_FORMAT];  /* to store the format ('%...') */
      ಚಿಹ್ನೆರಹಿತ maxitem = MAX_ಅಂಶ;  /* maximum length for the result */
      ಅಕ್ಷರ *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      ಪೂರ್ಣಾಂಕ nb = 0;  /* number of bytes in result */
      ಆದರೆ (++arg > ತುದಿ)
        ಹಿಂತಿರುಗಿ luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      ಆಯ್ಕೆ (*strfrmt++) {
        ಸ್ಥಿತಿ 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, arg));
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ 'd': ಸ್ಥಿತಿ 'i':
          flags = L_FMTFLAGSI;
          ಹೋಗು intcase;
        ಸ್ಥಿತಿ 'u':
          flags = L_FMTFLAGSU;
          ಹೋಗು intcase;
        ಸ್ಥಿತಿ 'o': ಸ್ಥಿತಿ 'x': ಸ್ಥಿತಿ 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ 'a': ಸ್ಥಿತಿ 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ಸಂಖ್ಯೆ_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        ಸ್ಥಿತಿ 'e': ಸ್ಥಿತಿ 'E': ಸ್ಥಿತಿ 'g': ಸ್ಥಿತಿ 'G': {
          lua_ಸಂಖ್ಯೆ n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ಸಂಖ್ಯೆ_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ 'p': {
          ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          ಆದರೆ (p == ಖಾಲಿ) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /*ಫಲಿತಾಂಶ*/
            form[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ 'q': {
          ಆದರೆ (form[2] != '\0')  /* modifiers? */
            ಹಿಂತಿರುಗಿ luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          ನಿಲ್ಲಿಸು;
        }
        ಸ್ಥಿತಿ 's': {
          size_t l;
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_tolstring(L, arg, &l);
          ಆದರೆ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          ಇಲ್ಲದಿದ್ದರೆ {
            luaL_argcheck(L, l == ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            ಆದರೆ (strchr(form, '.') == ಖಾಲಿ && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            ಇಲ್ಲದಿದ್ದರೆ {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_ತೆಗೆ(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          ನಿಲ್ಲಿಸು;
        }
        ಪೂರ್ವನಿಯೋಜಿತ: {  /* also treat cases 'pnLlh' */
          ಹಿಂತಿರುಗಿ luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  ಹಿಂತಿರುಗಿ 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#ಆದರೆ !defined(LUAL_PACKPADBYTE)
#ವ್ಯಾಖ್ಯಾನಿಸು LUAL_PACKPADBYTE		0x00
#ಆದರೆ_ಅಂತ್ಯ

/* maximum size for the binary representation of an integer */
#ವ್ಯಾಖ್ಯಾನಿಸು MAXINTSIZE	16

/* number of bits in a character */
#ವ್ಯಾಖ್ಯಾನಿಸು NB	CHAR_BIT

/* mask for one character (NB 1's) */
#ವ್ಯಾಖ್ಯಾನಿಸು MC	((1 << NB) - 1)

/* size of a lua_Integer */
#ವ್ಯಾಖ್ಯಾನಿಸು SZINT	((ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(lua_Integer))


/* dummy union to get native endianness */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಒಕ್ಕೂಟ {
  ಪೂರ್ಣಾಂಕ dummy;
  ಅಕ್ಷರ little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಶೀರ್ಷಿಕೆ {
  lua_State *L;
  ಪೂರ್ಣಾಂಕ islittle;
  ಚಿಹ್ನೆರಹಿತ maxalign;
} ಶೀರ್ಷಿಕೆ;


/*
** options for pack/unpack
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ KOption {
  Kint,		/* signed integers */
  Kuint,	/* unsigned integers */
  Kfloat,	/* single-precision floating-point numbers */
  Knumber,	/* Lua "native" floating-point numbers */
  Kdouble,	/* double-precision floating-point numbers */
  Kchar,	/* fixed-length strings */
  Kstring,	/* strings with prefixed length */
  Kzstr,	/* zero-terminated strings */
  Kpadding,	/* padding */
  Kpaddalign,	/* padding for alignment */
  Knop		/* no-op (configuration or spaces) */
} KOption;


/*
** Read an integer numeral from string 'fmt' or return 'df' if
** there is no numeral
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ digit (ಪೂರ್ಣಾಂಕ c) { ಹಿಂತಿರುಗಿ '0' <= c && c <= '9'; }

ಸ್ಥಿರ size_t getnum (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **fmt, size_t df) {
  ಆದರೆ (!digit(**fmt))  /* no number? */
    ಹಿಂತಿರುಗಿ df;  /* return default value */
  ಇಲ್ಲದಿದ್ದರೆ {
    size_t a = 0;
    ಮಾಡು {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } ಆಗ (digit(**fmt) && a <= (MAX_ಗಾತ್ರ - 9)/10);
    ಹಿಂತಿರುಗಿ a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ getnumlimit (ಶೀರ್ಷಿಕೆ *h, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  ಆದರೆ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    ಹಿಂತಿರುಗಿ cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  ಹಿಂತಿರುಗಿ cast_uint(sz);
}


/*
** Initialize Header
*/
ಸ್ಥಿರ ಶೂನ್ಯ initheader (lua_State *L, ಶೀರ್ಷಿಕೆ *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
ಸ್ಥಿರ KOption getoption (ಶೀರ್ಷಿಕೆ *h, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **fmt, size_t *ಗಾತ್ರ_2) {
  /* dummy structure to get native alignment requirements */
  ರಚನೆ cD { ಅಕ್ಷರ c; ಒಕ್ಕೂಟ { LUAI_MAXALIGN; } u; };
  ಪೂರ್ಣಾಂಕ opt = *((*fmt)++);
  *ಗಾತ್ರ_2 = 0;  /* default */
  ಆಯ್ಕೆ (opt) {
    ಸ್ಥಿತಿ 'b': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ಅಕ್ಷರ); ಹಿಂತಿರುಗಿ Kint;
    ಸ್ಥಿತಿ 'B': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ಅಕ್ಷರ); ಹಿಂತಿರುಗಿ Kuint;
    ಸ್ಥಿತಿ 'h': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ಗಿಡ್ಡ); ಹಿಂತಿರುಗಿ Kint;
    ಸ್ಥಿತಿ 'H': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ಗಿಡ್ಡ); ಹಿಂತಿರುಗಿ Kuint;
    ಸ್ಥಿತಿ 'l': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ದೀರ್ಘ); ಹಿಂತಿರುಗಿ Kint;
    ಸ್ಥಿತಿ 'L': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ದೀರ್ಘ); ಹಿಂತಿರುಗಿ Kuint;
    ಸ್ಥಿತಿ 'j': *ಗಾತ್ರ_2 = ಗಾತ್ರ(lua_Integer); ಹಿಂತಿರುಗಿ Kint;
    ಸ್ಥಿತಿ 'J': *ಗಾತ್ರ_2 = ಗಾತ್ರ(lua_Integer); ಹಿಂತಿರುಗಿ Kuint;
    ಸ್ಥಿತಿ 'T': *ಗಾತ್ರ_2 = ಗಾತ್ರ(size_t); ಹಿಂತಿರುಗಿ Kuint;
    ಸ್ಥಿತಿ 'f': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ದಶಮಾಂಶ); ಹಿಂತಿರುಗಿ Kfloat;
    ಸ್ಥಿತಿ 'n': *ಗಾತ್ರ_2 = ಗಾತ್ರ(lua_ಸಂಖ್ಯೆ); ಹಿಂತಿರುಗಿ Knumber;
    ಸ್ಥಿತಿ 'd': *ಗಾತ್ರ_2 = ಗಾತ್ರ(ದ್ವಿಗುಣ); ಹಿಂತಿರುಗಿ Kdouble;
    ಸ್ಥಿತಿ 'i': *ಗಾತ್ರ_2 = getnumlimit(h, fmt, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)); ಹಿಂತಿರುಗಿ Kint;
    ಸ್ಥಿತಿ 'I': *ಗಾತ್ರ_2 = getnumlimit(h, fmt, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)); ಹಿಂತಿರುಗಿ Kuint;
    ಸ್ಥಿತಿ 's': *ಗಾತ್ರ_2 = getnumlimit(h, fmt, ಗಾತ್ರ(size_t)); ಹಿಂತಿರುಗಿ Kstring;
    ಸ್ಥಿತಿ 'c':
      *ಗಾತ್ರ_2 = getnum(fmt, cast_sizet(-1));
      ಆದರೆ (l_unlikely(*ಗಾತ್ರ_2 == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      ಹಿಂತಿರುಗಿ Kchar;
    ಸ್ಥಿತಿ 'z': ಹಿಂತಿರುಗಿ Kzstr;
    ಸ್ಥಿತಿ 'x': *ಗಾತ್ರ_2 = 1; ಹಿಂತಿರುಗಿ Kpadding;
    ಸ್ಥಿತಿ 'X': ಹಿಂತಿರುಗಿ Kpaddalign;
    ಸ್ಥಿತಿ ' ': ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '<': h->islittle = 1; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '>': h->islittle = 0; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '=': h->islittle = nativeendian.little; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '!': {
      ಸ್ಥಿರಾಂಕ size_t maxalign = offsetof(ರಚನೆ cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  ಹಿಂತಿರುಗಿ Knop;
}


/*
** Read, classify, and fill other details about the next option.
** 'psize' is filled with option's size, 'notoalign' with its
** alignment requirements.
** Local variable 'size' gets the size to be aligned. (Kpadal option
** always gets its full alignment, other options are limited by
** the maximum alignment ('maxalign'). Kchar option needs no alignment
** despite its size.
*/
ಸ್ಥಿರ KOption getdetails (ಶೀರ್ಷಿಕೆ *h, size_t totalsize, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **fmt,
                           size_t *psize, ಚಿಹ್ನೆರಹಿತ *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  ಆದರೆ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    ಆದರೆ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  ಆದರೆ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    ಆದರೆ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      /* 'szmoda' = totalsize % align */
      ಚಿಹ್ನೆರಹಿತ szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  ಹಿಂತಿರುಗಿ opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
ಸ್ಥಿರ ಶೂನ್ಯ packint (lua_L_ಬಫರ್ *b, lua_Unsigned n,
                     ಪೂರ್ಣಾಂಕ islittle, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ neg) {
  ಅಕ್ಷರ *buff = luaL_prepbuffsize(b, ಗಾತ್ರ_2);
  ಚಿಹ್ನೆರಹಿತ i;
  buff[islittle ? 0 : ಗಾತ್ರ_2 - 1] = (ಅಕ್ಷರ)(n & MC);  /* first byte */
  ಗಾಗಿ (i = 1; i < ಗಾತ್ರ_2; i++) {
    n >>= NB;
    buff[islittle ? i : ಗಾತ್ರ_2 - 1 - i] = (ಅಕ್ಷರ)(n & MC);
  }
  ಆದರೆ (neg && ಗಾತ್ರ_2 > SZINT) {  /* negative number need sign extension? */
    ಗಾಗಿ (i = SZINT; i < ಗಾತ್ರ_2; i++)  /* correct extra bytes */
      buff[islittle ? i : ಗಾತ್ರ_2 - 1 - i] = (ಅಕ್ಷರ)MC;
  }
  luaL_addsize(b, ಗಾತ್ರ_2);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
ಸ್ಥಿರ ಶೂನ್ಯ copywithendian (ಅಕ್ಷರ *dest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src,
                            ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ islittle) {
  ಆದರೆ (islittle == nativeendian.little)
    ಸ್ಮರಣೆ_ನಕಲು(dest, src, ಗಾತ್ರ_2);
  ಇಲ್ಲದಿದ್ದರೆ {
    dest += ಗಾತ್ರ_2 - 1;
    ಆಗ (ಗಾತ್ರ_2-- != 0)
      *(dest--) = *(src++);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_pack (lua_State *L) {
  lua_L_ಬಫರ್ b;
  ಶೀರ್ಷಿಕೆ h;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt = luaL_checkstring(L, 1);  /* format string */
  ಪೂರ್ಣಾಂಕ arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  ಆಗ (*fmt != '\0') {
    ಚಿಹ್ನೆರಹಿತ ntoalign;
    size_t ಗಾತ್ರ_2;
    KOption opt = getdetails(&h, totalsize, &fmt, &ಗಾತ್ರ_2, &ntoalign);
    luaL_argcheck(L, ಗಾತ್ರ_2 + ntoalign <= MAX_ಗಾತ್ರ - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + ಗಾತ್ರ_2;
    ಆಗ (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    ಆಯ್ಕೆ (opt) {
      ಸ್ಥಿತಿ Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ಆದರೆ (ಗಾತ್ರ_2 < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((ಗಾತ್ರ_2 * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(ಗಾತ್ರ_2), (n < 0));
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ಆದರೆ (ಗಾತ್ರ_2 < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (ಗಾತ್ರ_2 * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(ಗಾತ್ರ_2), 0);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kfloat: {  /* C float */
        ದಶಮಾಂಶ f = (ದಶಮಾಂಶ)luaL_checknumber(L, arg);  /* get argument */
        ಅಕ್ಷರ *buff = luaL_prepbuffsize(&b, ಗಾತ್ರ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ಅಕ್ಷರ *)&f, ಗಾತ್ರ(f), h.islittle);
        luaL_addsize(&b, ಗಾತ್ರ_2);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Knumber: {  /* Lua float */
        lua_ಸಂಖ್ಯೆ f = luaL_checknumber(L, arg);  /* get argument */
        ಅಕ್ಷರ *buff = luaL_prepbuffsize(&b, ಗಾತ್ರ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ಅಕ್ಷರ *)&f, ಗಾತ್ರ(f), h.islittle);
        luaL_addsize(&b, ಗಾತ್ರ_2);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kdouble: {  /* C double */
        ದ್ವಿಗುಣ f = (ದ್ವಿಗುಣ)luaL_checknumber(L, arg);  /* get argument */
        ಅಕ್ಷರ *buff = luaL_prepbuffsize(&b, ಗಾತ್ರ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ಅಕ್ಷರ *)&f, ಗಾತ್ರ(f), h.islittle);
        luaL_addsize(&b, ಗಾತ್ರ_2);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kchar: {  /* fixed-size string */
        size_t len;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= ಗಾತ್ರ_2, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        ಆದರೆ (len < ಗಾತ್ರ_2) {  /* does it need padding? */
          size_t psize = ಗಾತ್ರ_2 - len;  /* pad size */
          ಅಕ್ಷರ *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kstring: {  /* strings with length count */
        size_t len;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, ಗಾತ್ರ_2 >= ಗಾತ್ರ(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (ಗಾತ್ರ_2 * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(ಗಾತ್ರ_2), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kzstr: {  /* zero-terminated string */
        size_t len;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      ಸ್ಥಿತಿ Kpaddalign: ಸ್ಥಿತಿ Knop:
        arg--;  /* undo increment */
        ನಿಲ್ಲಿಸು;
    }
  }
  luaL_pushresult(&b);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_packsize (lua_State *L) {
  ಶೀರ್ಷಿಕೆ h;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  ಆಗ (*fmt != '\0') {
    ಚಿಹ್ನೆರಹಿತ ntoalign;
    size_t ಗಾತ್ರ_2;
    KOption opt = getdetails(&h, totalsize, &fmt, &ಗಾತ್ರ_2, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    ಗಾತ್ರ_2 += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_ಗಾತ್ರ - ಗಾತ್ರ_2,
                     1, "format result too large");
    totalsize += ಗಾತ್ರ_2;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
ಸ್ಥಿರ lua_Integer unpackint (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str,
                              ಪೂರ್ಣಾಂಕ islittle, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ issigned) {
  lua_Unsigned res = 0;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ = (ಗಾತ್ರ_2  <= SZINT) ? ಗಾತ್ರ_2 : SZINT;
  ಗಾಗಿ (i = ಎಲ್ಲೆ - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)str[islittle ? i : ಗಾತ್ರ_2 - 1 - i];
  }
  ಆದರೆ (ಗಾತ್ರ_2 < SZINT) {  /* real size smaller than lua_Integer? */
    ಆದರೆ (issigned) {  /* needs sign extension? */
      lua_Unsigned ಮುಖವಾಡ_2 = (lua_Unsigned)1 << (ಗಾತ್ರ_2*NB - 1);
      res = ((res ^ ಮುಖವಾಡ_2) - ಮುಖವಾಡ_2);  /* do sign extension */
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 > SZINT) {  /* must check unread bytes */
    ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2 = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    ಗಾಗಿ (i = ಎಲ್ಲೆ; i < ಗಾತ್ರ_2; i++) {
      ಆದರೆ (l_unlikely((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)str[islittle ? i : ಗಾತ್ರ_2 - 1 - i] != ಮುಖವಾಡ_2))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", ಗಾತ್ರ_2);
    }
  }
  ಹಿಂತಿರುಗಿ (lua_Integer)res;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str_unpack (lua_State *L) {
  ಶೀರ್ಷಿಕೆ h;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ದತ್ತಾಂಶ = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  ಪೂರ್ಣಾಂಕ n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  ಆಗ (*fmt != '\0') {
    ಚಿಹ್ನೆರಹಿತ ntoalign;
    size_t ಗಾತ್ರ_2;
    KOption opt = getdetails(&h, pos, &fmt, &ಗಾತ್ರ_2, &ntoalign);
    luaL_argcheck(L, ntoalign + ಗಾತ್ರ_2 <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    ಆಯ್ಕೆ (opt) {
      ಸ್ಥಿತಿ Kint:
      ಸ್ಥಿತಿ Kuint: {
        lua_Integer res = unpackint(L, ದತ್ತಾಂಶ + pos, h.islittle,
                                       cast_int(ಗಾತ್ರ_2), (opt == Kint));
        lua_pushinteger(L, res);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kfloat: {
        ದಶಮಾಂಶ f;
        copywithendian((ಅಕ್ಷರ *)&f, ದತ್ತಾಂಶ + pos, ಗಾತ್ರ(f), h.islittle);
        lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)f);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Knumber: {
        lua_ಸಂಖ್ಯೆ f;
        copywithendian((ಅಕ್ಷರ *)&f, ದತ್ತಾಂಶ + pos, ಗಾತ್ರ(f), h.islittle);
        lua_pushnumber(L, f);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kdouble: {
        ದ್ವಿಗುಣ f;
        copywithendian((ಅಕ್ಷರ *)&f, ದತ್ತಾಂಶ + pos, ಗಾತ್ರ(f), h.islittle);
        lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)f);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kchar: {
        lua_pushlstring(L, ದತ್ತಾಂಶ + pos, ಗಾತ್ರ_2);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, ದತ್ತಾಂಶ + pos,
                                          h.islittle, cast_int(ಗಾತ್ರ_2), 0);
        luaL_argcheck(L, len <= ld - pos - ಗಾತ್ರ_2, 2, "data string too short");
        lua_pushlstring(L, ದತ್ತಾಂಶ + pos + ಗಾತ್ರ_2, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kzstr: {
        size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ದತ್ತಾಂಶ + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, ದತ್ತಾಂಶ + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ Kpaddalign: ಸ್ಥಿತಿ Kpadding: ಸ್ಥಿತಿ Knop:
        n--;  /* undo increment */
        ನಿಲ್ಲಿಸು;
    }
    pos += ಗಾತ್ರ_2;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  ಹಿಂತಿರುಗಿ n + 1;
}

/* }====================================================== */


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_ಕಂಡುಹಿಡಿ},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_ಅಧೋ},
  {"match", str_ಹೊಂದಿಕೆ},
  {"rep", str_rep},
  {"reverse", str_ಹಿಮ್ಮುಖ},
  {"sub", str_sub},
  {"upper", str_ಊರ್ಧ್ವ},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {ಖಾಲಿ, ಖಾಲಿ}
};


ಸ್ಥಿರ ಶೂನ್ಯ createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_ತೆಗೆ(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_ತೆಗೆ(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  ಹಿಂತಿರುಗಿ 1;
}

