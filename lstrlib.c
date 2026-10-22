/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

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
#ಯದಿ !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  ಪ್ರತ್ಯಾಗಮ 1;
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
  ಯದಿ (pos > 0)
    ಪ್ರತ್ಯಾಗಮ (size_t)pos;
  ಅನ್ಯಥಾ ಯದಿ (pos == 0)
    ಪ್ರತ್ಯಾಗಮ 1;
  ಅನ್ಯಥಾ ಯದಿ (pos < -(lua_Integer)len)  /* inverted comparison */
    ಪ್ರತ್ಯಾಗಮ 1;  /* clip to 1 */
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ಸ್ಥಿರ size_t getendpos (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  ಯದಿ (pos > (lua_Integer)len)
    ಪ್ರತ್ಯಾಗಮ len;
  ಅನ್ಯಥಾ ಯದಿ (pos >= 0)
    ಪ್ರತ್ಯಾಗಮ (size_t)pos;
  ಅನ್ಯಥಾ ಯದಿ (pos < -(lua_Integer)len)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ len + (size_t)pos + 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_sub (lua_State *L) {
  size_t l;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  ಯದಿ (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  ಅನ್ಯಥಾ lua_pushliteral(L, "");
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &l);
  ವರ್ಣ *p = luaL_buffinitsize(L, &b, l);
  ಪ್ರತಿ (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &l);
  ವರ್ಣ *p = luaL_buffinitsize(L, &b, l);
  ಪ್ರತಿ (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &l);
  ವರ್ಣ *p = luaL_buffinitsize(L, &b, l);
  ಪ್ರತಿ (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_rep (lua_State *L) {
  size_t len, lsep;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ಧ್ರುವ ವರ್ಣ *sep = luaL_optlstring(L, 3, "", &lsep);
  ಯದಿ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  ಅನ್ಯಥಾ ಯದಿ (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "resulting string too large");
  ಅನ್ಯಥಾ {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    ವರ್ಣ *p = luaL_buffinitsize(L, &b, totallen);
    ಯಾವತ್ (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * ಪರಿಮಾಣ(ವರ್ಣ)); p += len;
      ಯದಿ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * ಪರಿಮಾಣ(ವರ್ಣ)); p += lsep;
      }
    }
    memcpy(p, s, len * ಪರಿಮಾಣ(ವರ್ಣ));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_byte (lua_State *L) {
  size_t l;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  ಪೂರ್ಣಾಙ್ಕ n, i;
  ಯದಿ (posi > pose) ಪ್ರತ್ಯಾಗಮ 0;  /* empty interval; return no values */
  ಯದಿ (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "string slice too long");
  n = (ಪೂರ್ಣಾಙ್ಕ)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  ಪ್ರತಿ (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  ಪ್ರತ್ಯಾಗಮ n;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_char (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);  /* number of arguments */
  ಪೂರ್ಣಾಙ್ಕ i;
  luaL_Buffer b;
  ವರ್ಣ *p = luaL_buffinitsize(L, &b, cast_uint(n));
  ಪ್ರತಿ (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
ಸಂರಚನಾ str_Writer {
  ಪೂರ್ಣಾಙ್ಕ init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ writer (lua_State *L, ಧ್ರುವ ಶೂನ್ಯ *b, size_t size, ಶೂನ್ಯ *ud) {
  ಸಂರಚನಾ str_Writer *state = (ಸಂರಚನಾ str_Writer *)ud;
  ಯದಿ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  ಯದಿ (b == ರಿಕ್ತ) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  ಅನ್ಯಥಾ
    luaL_addlstring(&state->B, (ಧ್ರುವ ವರ್ಣ *)b, size);
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_dump (lua_State *L) {
  ಸಂರಚನಾ str_Writer state;
  ಪೂರ್ಣಾಙ್ಕ strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  ಪ್ರತ್ಯಾಗಮ 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#ಯದಿ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ಸ್ಥಿರ ಧ್ರುವ luaL_Reg stringmetamethods[] = {
  {"__index", ರಿಕ್ತ},  /* placeholder */
  {ರಿಕ್ತ, ರಿಕ್ತ}
};

#ಅನ್ಯಥಾ		/* }{ */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tonum (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg) {
  ಯದಿ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ {  /* check whether it is a numerical string */
    size_t len;
    ಧ್ರುವ ವರ್ಣ *s = lua_tolstring(L, arg, &len);
    ಪ್ರತ್ಯಾಗಮ (s != ರಿಕ್ತ && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
ಸ್ಥಿರ ಶೂನ್ಯ trymt (lua_State *L, ಧ್ರುವ ವರ್ಣ *mtkey, ಧ್ರುವ ವರ್ಣ *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  ಯದಿ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_ಸೇರಿಸು(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op, ಧ್ರುವ ವರ್ಣ *mtname) {
  ಯದಿ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  ಅನ್ಯಥಾ
    trymt(L, mtname, mtname + 2);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_add (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPADD, "__add");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_sub (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPSUB, "__sub");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_mul (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPMUL, "__mul");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_mod (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPMOD, "__mod");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_pow (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPPOW, "__pow");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_div (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPDIV, "__div");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_idiv (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPIDIV, "__idiv");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ arith_unm (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ arith(L, LUA_OPUNM, "__unm");
}


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", ರಿಕ್ತ},  /* placeholder */
  {ರಿಕ್ತ, ರಿಕ್ತ}
};

#endif		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#define CAP_UNFINISHED	(-1)
#define CAP_POSITION	(-2)


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ MatchState {
  ಧ್ರುವ ವರ್ಣ *src_init;  /* init of source string */
  ಧ್ರುವ ವರ್ಣ *src_end;  /* end ('\0') of source string */
  ಧ್ರುವ ವರ್ಣ *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  ಪೂರ್ಣಾಙ್ಕ matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  ಪೂರ್ಣಾಙ್ಕ level;  /* total number of captures (finished or unfinished) */
  ಸಂರಚನಾ {
    ಧ್ರುವ ವರ್ಣ *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *match (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *p);


/* maximum recursion depth for 'match' */
#ಯದಿ !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_capture (MatchState *ms, ಪೂರ್ಣಾಙ್ಕ l) {
  l -= '1';
  ಯದಿ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    ಪ್ರತ್ಯಾಗಮ luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  ಪ್ರತ್ಯಾಗಮ l;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ capture_to_close (MatchState *ms) {
  ಪೂರ್ಣಾಙ್ಕ level = ms->level;
  ಪ್ರತಿ (level--; level>=0; level--)
    ಯದಿ (ms->capture[level].len == CAP_UNFINISHED) ಪ್ರತ್ಯಾಗಮ level;
  ಪ್ರತ್ಯಾಗಮ luaL_error(ms->L, "invalid pattern capture");
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *classend (MatchState *ms, ಧ್ರುವ ವರ್ಣ *p) {
  ವಿಕಲ್ಪ (*p++) {
    ಪಕ್ಷ L_ESC: {
      ಯದಿ (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      ಪ್ರತ್ಯಾಗಮ p+1;
    }
    ಪಕ್ಷ '[': {
      ಯದಿ (*p == '^') p++;
      ಕುರು {  /* look for a ']' */
        ಯದಿ (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        ಯದಿ (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } ಯಾವತ್ (*p != ']');
      ಪ್ರತ್ಯಾಗಮ p+1;
    }
    ಸ್ವಭಾವ: {
      ಪ್ರತ್ಯಾಗಮ p;
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ match_class (ಪೂರ್ಣಾಙ್ಕ c, ಪೂರ್ಣಾಙ್ಕ cl) {
  ಪೂರ್ಣಾಙ್ಕ res;
  ವಿಕಲ್ಪ (tolower(cl)) {
    ಪಕ್ಷ 'a' : res = isalpha(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'c' : res = iscntrl(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'd' : res = isdigit(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'g' : res = isgraph(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'l' : res = islower(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'p' : res = ispunct(c); ಭಙ್ಗ;
    ಪಕ್ಷ 's' : res = isspace(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'u' : res = isupper(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'w' : res = isalnum(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'x' : res = isxdigit(c); ಭಙ್ಗ;
    ಪಕ್ಷ 'z' : res = (c == 0); ಭಙ್ಗ;  /* deprecated option */
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ (cl == c);
  }
  ಪ್ರತ್ಯಾಗಮ (islower(cl) ? res : !res);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ matchbracketclass (ಪೂರ್ಣಾಙ್ಕ c, ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *ec) {
  ಪೂರ್ಣಾಙ್ಕ sig = 1;
  ಯದಿ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  ಯಾವತ್ (++p < ec) {
    ಯದಿ (*p == L_ESC) {
      p++;
      ಯದಿ (match_class(c, cast_uchar(*p)))
        ಪ್ರತ್ಯಾಗಮ sig;
    }
    ಅನ್ಯಥಾ ಯದಿ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      ಯದಿ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        ಪ್ರತ್ಯಾಗಮ sig;
    }
    ಅನ್ಯಥಾ ಯದಿ (cast_uchar(*p) == c) ಪ್ರತ್ಯಾಗಮ sig;
  }
  ಪ್ರತ್ಯಾಗಮ !sig;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ singlematch (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *p,
                        ಧ್ರುವ ವರ್ಣ *ep) {
  ಯದಿ (s >= ms->src_end)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ c = cast_uchar(*s);
    ವಿಕಲ್ಪ (*p) {
      ಪಕ್ಷ '.': ಪ್ರತ್ಯಾಗಮ 1;  /* matches any char */
      ಪಕ್ಷ L_ESC: ಪ್ರತ್ಯಾಗಮ match_class(c, cast_uchar(*(p+1)));
      ಪಕ್ಷ '[': ಪ್ರತ್ಯಾಗಮ matchbracketclass(c, p, ep-1);
      ಸ್ವಭಾವ:  ಪ್ರತ್ಯಾಗಮ (cast_uchar(*p) == c);
    }
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *matchbalance (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s,
                                   ಧ್ರುವ ವರ್ಣ *p) {
  ಯದಿ (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  ಯದಿ (*s != *p) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ b = *p;
    ಪೂರ್ಣಾಙ್ಕ e = *(p+1);
    ಪೂರ್ಣಾಙ್ಕ cont = 1;
    ಯಾವತ್ (++s < ms->src_end) {
      ಯದಿ (*s == e) {
        ಯದಿ (--cont == 0) ಪ್ರತ್ಯಾಗಮ s+1;
      }
      ಅನ್ಯಥಾ ಯದಿ (*s == b) cont++;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* string ends out of balance */
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *max_expand (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s,
                                 ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  ಯಾವತ್ (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  ಯಾವತ್ (i>=0) {
    ಧ್ರುವ ವರ್ಣ *res = match(ms, (s+i), ep+1);
    ಯದಿ (res) ಪ್ರತ್ಯಾಗಮ res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *min_expand (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s,
                                 ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *ep) {
  ಪ್ರತಿ (;;) {
    ಧ್ರುವ ವರ್ಣ *res = match(ms, s, ep+1);
    ಯದಿ (res != ರಿಕ್ತ)
      ಪ್ರತ್ಯಾಗಮ res;
    ಅನ್ಯಥಾ ಯದಿ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *start_capture (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s,
                                    ಧ್ರುವ ವರ್ಣ *p, ಪೂರ್ಣಾಙ್ಕ what) {
  ಧ್ರುವ ವರ್ಣ *res;
  ಪೂರ್ಣಾಙ್ಕ level = ms->level;
  ಯದಿ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  ಯದಿ ((res=match(ms, s, p)) == ರಿಕ್ತ)  /* match failed? */
    ms->level--;  /* undo capture */
  ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *end_capture (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s,
                                  ಧ್ರುವ ವರ್ಣ *p) {
  ಪೂರ್ಣಾಙ್ಕ l = capture_to_close(ms);
  ಧ್ರುವ ವರ್ಣ *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  ಯದಿ ((res = match(ms, s, p)) == ರಿಕ್ತ)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  ಪ್ರತ್ಯಾಗಮ res;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *match_capture (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  ಯದಿ ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    ಪ್ರತ್ಯಾಗಮ s+len;
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *match (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *p) {
  ಯದಿ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  ಯದಿ (p != ms->p_end) {  /* end of pattern? */
    ವಿಕಲ್ಪ (*p) {
      ಪಕ್ಷ '(': {  /* start capture */
        ಯದಿ (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        ಅನ್ಯಥಾ
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ '$': {
        ಯದಿ ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          ಗಚ್ಛ dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : ರಿಕ್ತ;  /* check end of string */
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        ವಿಕಲ್ಪ (*(p + 1)) {
          ಪಕ್ಷ 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            ಯದಿ (s != ರಿಕ್ತ) {
              p += 4; ಗಚ್ಛ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            ಭಙ್ಗ;
          }
          ಪಕ್ಷ 'f': {  /* frontier? */
            ಧ್ರುವ ವರ್ಣ *ep; ವರ್ಣ previous;
            p += 2;
            ಯದಿ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            ಯದಿ (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; ಗಚ್ಛ init;  /* return match(ms, s, ep); */
            }
            s = ರಿಕ್ತ;  /* match failed */
            ಭಙ್ಗ;
          }
          ಪಕ್ಷ '0': ಪಕ್ಷ '1': ಪಕ್ಷ '2': ಪಕ್ಷ '3':
          ಪಕ್ಷ '4': ಪಕ್ಷ '5': ಪಕ್ಷ '6': ಪಕ್ಷ '7':
          ಪಕ್ಷ '8': ಪಕ್ಷ '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            ಯದಿ (s != ರಿಕ್ತ) {
              p += 2; ಗಚ್ಛ init;  /* return match(ms, s, p + 2) */
            }
            ಭಙ್ಗ;
          }
          ಸ್ವಭಾವ: ಗಚ್ಛ dflt;
        }
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ: dflt: {  /* pattern class plus optional suffix */
        ಧ್ರುವ ವರ್ಣ *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        ಯದಿ (!singlematch(ms, s, p, ep)) {
          ಯದಿ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; ಗಚ್ಛ init;  /* return match(ms, s, ep + 1); */
          }
          ಅನ್ಯಥಾ  /* '+' or no suffix */
            s = ರಿಕ್ತ;  /* fail */
        }
        ಅನ್ಯಥಾ {  /* matched once */
          ವಿಕಲ್ಪ (*ep) {  /* handle optional suffix */
            ಪಕ್ಷ '?': {  /* optional */
              ಧ್ರುವ ವರ್ಣ *res;
              ಯದಿ ((res = match(ms, s + 1, ep + 1)) != ರಿಕ್ತ)
                s = res;
              ಅನ್ಯಥಾ {
                p = ep + 1; ಗಚ್ಛ init;  /* else return match(ms, s, ep + 1); */
              }
              ಭಙ್ಗ;
            }
            ಪಕ್ಷ '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            ಪಕ್ಷ '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              ಭಙ್ಗ;
            ಪಕ್ಷ '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              ಭಙ್ಗ;
            ಸ್ವಭಾವ:  /* no suffix */
              s++; p = ep; ಗಚ್ಛ init;  /* return match(ms, s + 1, ep); */
          }
        }
        ಭಙ್ಗ;
      }
    }
  }
  ms->matchdepth++;
  ಪ್ರತ್ಯಾಗಮ s;
}



ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *lmemfind (ಧ್ರುವ ವರ್ಣ *s1, size_t l1,
                               ಧ್ರುವ ವರ್ಣ *s2, size_t l2) {
  ಯದಿ (l2 == 0) ಪ್ರತ್ಯಾಗಮ s1;  /* empty strings are everywhere */
  ಅನ್ಯಥಾ ಯದಿ (l2 > l1) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* avoids a negative 'l1' */
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    ಯಾವತ್ (l1 > 0 && (init = (ಧ್ರುವ ವರ್ಣ *)memchr(s1, *s2, l1)) != ರಿಕ್ತ) {
      init++;   /* 1st char is already checked */
      ಯದಿ (memcmp(init, s2+1, l2) == 0)
        ಪ್ರತ್ಯಾಗಮ init-1;
      ಅನ್ಯಥಾ {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
ಸ್ಥಿರ ptrdiff_t get_onecapture (MatchState *ms, ಪೂರ್ಣಾಙ್ಕ i, ಧ್ರುವ ವರ್ಣ *s,
                              ಧ್ರುವ ವರ್ಣ *e, ಧ್ರುವ ವರ್ಣ **cap) {
  ಯದಿ (i >= ms->level) {
    ಯದಿ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    ಪ್ರತ್ಯಾಗಮ (e - s);
  }
  ಅನ್ಯಥಾ {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    ಯದಿ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    ಅನ್ಯಥಾ ಯದಿ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    ಪ್ರತ್ಯಾಗಮ capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತಳ್ಳು_onecapture (MatchState *ms, ಪೂರ್ಣಾಙ್ಕ i, ಧ್ರುವ ವರ್ಣ *s,
                                                    ಧ್ರುವ ವರ್ಣ *e) {
  ಧ್ರುವ ವರ್ಣ *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  ಯದಿ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ತಳ್ಳು_captures (MatchState *ms, ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *e) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  ಪ್ರತಿ (i = 0; i < nlevels; i++)
    ತಳ್ಳು_onecapture(ms, i, s, e);
  ಪ್ರತ್ಯಾಗಮ nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nospecials (ಧ್ರುವ ವರ್ಣ *p, size_t l) {
  size_t upto = 0;
  ಕುರು {
    ಯದಿ (strpbrk(p + upto, SPECIALS))
      ಪ್ರತ್ಯಾಗಮ 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } ಯಾವತ್ (upto <= l);
  ಪ್ರತ್ಯಾಗಮ 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
ಸ್ಥಿರ ಶೂನ್ಯ prepstate (MatchState *ms, lua_State *L,
                       ಧ್ರುವ ವರ್ಣ *s, size_t ls, ಧ್ರುವ ವರ್ಣ *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
ಸ್ಥಿರ ಶೂನ್ಯ reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_find_aux (lua_State *L, ಪೂರ್ಣಾಙ್ಕ find) {
  size_t ls, lp;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &ls);
  ಧ್ರುವ ವರ್ಣ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  ಯದಿ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  /* explicit request or no special characters? */
  ಯದಿ (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ಧ್ರುವ ವರ್ಣ *s2 = lmemfind(s + init, ls - init, p, lp);
    ಯದಿ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      ಪ್ರತ್ಯಾಗಮ 2;
    }
  }
  ಅನ್ಯಥಾ {
    MatchState ms;
    ಧ್ರುವ ವರ್ಣ *s1 = s + init;
    ಪೂರ್ಣಾಙ್ಕ anchor = (*p == '^');
    ಯದಿ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    ಕುರು {
      ಧ್ರುವ ವರ್ಣ *res;
      reprepstate(&ms);
      ಯದಿ ((res=match(&ms, s1, p)) != ರಿಕ್ತ) {
        ಯದಿ (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          ಪ್ರತ್ಯಾಗಮ ತಳ್ಳು_captures(&ms, ರಿಕ್ತ, 0) + 2;
        }
        ಅನ್ಯಥಾ
          ಪ್ರತ್ಯಾಗಮ ತಳ್ಳು_captures(&ms, s1, res);
      }
    } ಯಾವತ್ (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_find (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ str_find_aux(L, 1);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_match (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ str_find_aux(L, 0);
}


/* state for 'gmatch' */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ GMatchState {
  ಧ್ರುವ ವರ್ಣ *src;  /* current position */
  ಧ್ರುವ ವರ್ಣ *p;  /* pattern */
  ಧ್ರುವ ವರ್ಣ *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ಧ್ರುವ ವರ್ಣ *src;
  gm->ms.L = L;
  ಪ್ರತಿ (src = gm->src; src <= gm->ms.src_end; src++) {
    ಧ್ರುವ ವರ್ಣ *e;
    reprepstate(&gm->ms);
    ಯದಿ ((e = match(&gm->ms, src, gm->p)) != ರಿಕ್ತ && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      ಪ್ರತ್ಯಾಗಮ ತಳ್ಳು_captures(&gm->ms, src, e);
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;  /* not found */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gmatch (lua_State *L) {
  size_t ls, lp;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &ls);
  ಧ್ರುವ ವರ್ಣ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, ಪರಿಮಾಣ(GMatchState), 0);
  ಯದಿ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = ರಿಕ್ತ;
  lua_pushcclosure(L, gmatch_aux, 3);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಶೂನ್ಯ add_s (MatchState *ms, luaL_Buffer *b, ಧ್ರುವ ವರ್ಣ *s,
                                                   ಧ್ರುವ ವರ್ಣ *e) {
  size_t l;
  lua_State *L = ms->L;
  ಧ್ರುವ ವರ್ಣ *news = lua_tolstring(L, 3, &l);
  ಧ್ರುವ ವರ್ಣ *p;
  ಯಾವತ್ ((p = (ವರ್ಣ *)memchr(news, L_ESC, l)) != ರಿಕ್ತ) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    ಯದಿ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    ಅನ್ಯಥಾ ಯದಿ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    ಅನ್ಯಥಾ ಯದಿ (isdigit(cast_uchar(*p))) {  /* '%n' */
      ಧ್ರುವ ವರ್ಣ *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      ಯದಿ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      ಅನ್ಯಥಾ
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ add_ಮೌಲ್ಯ (MatchState *ms, luaL_Buffer *b, ಧ್ರುವ ವರ್ಣ *s,
                                      ಧ್ರುವ ವರ್ಣ *e, ಪೂರ್ಣಾಙ್ಕ tr) {
  lua_State *L = ms->L;
  ವಿಕಲ್ಪ (tr) {
    ಪಕ್ಷ LUA_TFUNCTION: {  /* call the function */
      ಪೂರ್ಣಾಙ್ಕ n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ತಳ್ಳು_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_TTABLE: {  /* index the table */
      ತಳ್ಳು_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      ಪ್ರತ್ಯಾಗಮ 1;  /* something changed */
    }
  }
  ಯದಿ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_ತೆಗೆ(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    ಪ್ರತ್ಯಾಗಮ 0;  /* no changes */
  }
  ಅನ್ಯಥಾ ಯದಿ (l_unlikely(!lua_isstring(L, -1)))
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  ಅನ್ಯಥಾ {
    luaL_addvalue(b);  /* add result to accumulator */
    ಪ್ರತ್ಯಾಗಮ 1;  /* something changed */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_gsub (lua_State *L) {
  size_t srcl, lp;
  ಧ್ರುವ ವರ್ಣ *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ಧ್ರುವ ವರ್ಣ *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  ಧ್ರುವ ವರ್ಣ *lastmatch = ರಿಕ್ತ;  /* end of last match */
  ಪೂರ್ಣಾಙ್ಕ tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  ಪೂರ್ಣಾಙ್ಕ anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  ಪೂರ್ಣಾಙ್ಕ changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  ಯದಿ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  ಯಾವತ್ (n < max_s) {
    ಧ್ರುವ ವರ್ಣ *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    ಯದಿ ((e = match(&ms, src, p)) != ರಿಕ್ತ && e != lastmatch) {  /* match? */
      n++;
      changed = add_ಮೌಲ್ಯ(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    ಅನ್ಯಥಾ ಯದಿ (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    ಅನ್ಯಥಾ ಭಙ್ಗ;  /* end of subject */
    ಯದಿ (anchor) ಭಙ್ಗ;
  }
  ಯದಿ (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  ಅನ್ಯಥಾ {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  ಪ್ರತ್ಯಾಗಮ 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#ಯದಿ !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(ಪರಿಮಾಣ(LUA_ಸಂಖ್ಯೆ_FRMLEN)/ಪರಿಮಾಣ(ವರ್ಣ))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#define L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ adddigit (ವರ್ಣ *buff, unsigned n, lua_ಸಂಖ್ಯೆ x) {
  lua_ಸಂಖ್ಯೆ dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  ಪೂರ್ಣಾಙ್ಕ d = (ಪೂರ್ಣಾಙ್ಕ)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  ಪ್ರತ್ಯಾಗಮ x - dd;  /* return what is left */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ num2straux (ವರ್ಣ *buff, unsigned sz, lua_ಸಂಖ್ಯೆ x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  ಯದಿ (x != x || x == (lua_ಸಂಖ್ಯೆ)HUGE_VAL || x == -(lua_ಸಂಖ್ಯೆ)HUGE_VAL)
    ಪ್ರತ್ಯಾಗಮ l_sprintf(buff, sz, LUA_ಸಂಖ್ಯೆ_FMT, (LUAI_UACNUMBER)x);
  ಅನ್ಯಥಾ ಯದಿ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    ಪ್ರತ್ಯಾಗಮ l_sprintf(buff, sz, LUA_ಸಂಖ್ಯೆ_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ e;
    lua_ಸಂಖ್ಯೆ m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    ಯದಿ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    ಯದಿ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      ಕುರು {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } ಯಾವತ್ (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    ಪ್ರತ್ಯಾಗಮ cast_int(n);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ lua_number2strx (lua_State *L, ವರ್ಣ *buff, unsigned sz,
                            ಧ್ರುವ ವರ್ಣ *fmt, lua_ಸಂಖ್ಯೆ x) {
  ಪೂರ್ಣಾಙ್ಕ n = num2straux(buff, sz, x);
  ಯದಿ (fmt[SIZELENMOD] == 'A') {
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  ಅನ್ಯಥಾ ಯದಿ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  ಪ್ರತ್ಯಾಗಮ n;
}

#endif				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#define MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#define MAX_ITEM	120


/* valid flags in a format specification */
#ಯದಿ !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#define L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#define L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#define L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#define L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#define L_FMTFLAGSC	"-"

#endif


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#define MAX_FORMAT	32


ಸ್ಥಿರ ಶೂನ್ಯ addquoted (luaL_Buffer *b, ಧ್ರುವ ವರ್ಣ *s, size_t len) {
  luaL_addchar(b, '"');
  ಯಾವತ್ (len--) {
    ಯದಿ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    ಅನ್ಯಥಾ ಯದಿ (iscntrl(cast_uchar(*s))) {
      ವರ್ಣ buff[10];
      ಯದಿ (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, ಪರಿಮಾಣ(buff), "\\%d", (ಪೂರ್ಣಾಙ್ಕ)cast_uchar(*s));
      ಅನ್ಯಥಾ
        l_sprintf(buff, ಪರಿಮಾಣ(buff), "\\%03d", (ಪೂರ್ಣಾಙ್ಕ)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ quotefloat (lua_State *L, ವರ್ಣ *buff, lua_ಸಂಖ್ಯೆ n) {
  ಧ್ರುವ ವರ್ಣ *s;  /* for the fixed representations */
  ಯದಿ (n == (lua_ಸಂಖ್ಯೆ)HUGE_VAL)  /* inf? */
    s = "1e9999";
  ಅನ್ಯಥಾ ಯದಿ (n == -(lua_ಸಂಖ್ಯೆ)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  ಅನ್ಯಥಾ ಯದಿ (n != n)  /* NaN? */
    s = "(0/0)";
  ಅನ್ಯಥಾ {  /* format number as hexadecimal */
    ಪೂರ್ಣಾಙ್ಕ  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_ಸಂಖ್ಯೆ_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    ಯದಿ (memchr(buff, '.', cast_uint(nb)) == ರಿಕ್ತ) {  /* no dot? */
      ವರ್ಣ point = lua_getlocaledecpoint();  /* try locale point */
      ವರ್ಣ *ppoint = (ವರ್ಣ *)memchr(buff, point, cast_uint(nb));
      ಯದಿ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    ಪ್ರತ್ಯಾಗಮ nb;
  }
  /* for the fixed representations */
  ಪ್ರತ್ಯಾಗಮ l_sprintf(buff, MAX_ITEM, "%s", s);
}


ಸ್ಥಿರ ಶೂನ್ಯ addliteral (lua_State *L, luaL_Buffer *b, ಪೂರ್ಣಾಙ್ಕ arg) {
  ವಿಕಲ್ಪ (lua_type(L, arg)) {
    ಪಕ್ಷ LUA_TSTRING: {
      size_t len;
      ಧ್ರುವ ವರ್ಣ *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_TNUMBER: {
      ವರ್ಣ *buff = luaL_prepbuffsize(b, MAX_ITEM);
      ಪೂರ್ಣಾಙ್ಕ nb;
      ಯದಿ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      ಅನ್ಯಥಾ {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ಧ್ರುವ ವರ್ಣ *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_TNIL: ಪಕ್ಷ LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, ರಿಕ್ತ);
      luaL_addvalue(b);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *get2digits (ಧ್ರುವ ವರ್ಣ *s) {
  ಯದಿ (isdigit(cast_uchar(*s))) {
    s++;
    ಯದಿ (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  ಪ್ರತ್ಯಾಗಮ s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkformat (lua_State *L, ಧ್ರುವ ವರ್ಣ *form, ಧ್ರುವ ವರ್ಣ *flags,
                                       ಪೂರ್ಣಾಙ್ಕ precision) {
  ಧ್ರುವ ವರ್ಣ *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  ಯದಿ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    ಯದಿ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  ಯದಿ (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getformat (lua_State *L, ಧ್ರುವ ವರ್ಣ *strfrmt,
                                            ವರ್ಣ *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  ಯದಿ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * ಪರಿಮಾಣ(ವರ್ಣ));
  *(form + len) = '\0';
  ಪ್ರತ್ಯಾಗಮ strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
ಸ್ಥಿರ ಶೂನ್ಯ addlenmod (ವರ್ಣ *form, ಧ್ರುವ ವರ್ಣ *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  ವರ್ಣ spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_format (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ ತುದಿ = lua_gettop(L);
  ಪೂರ್ಣಾಙ್ಕ arg = 1;
  size_t sfl;
  ಧ್ರುವ ವರ್ಣ *strfrmt = luaL_checklstring(L, arg, &sfl);
  ಧ್ರುವ ವರ್ಣ *strfrmt_end = strfrmt+sfl;
  ಧ್ರುವ ವರ್ಣ *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  ಯಾವತ್ (strfrmt < strfrmt_end) {
    ಯದಿ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    ಅನ್ಯಥಾ ಯದಿ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    ಅನ್ಯಥಾ { /* format item */
      ವರ್ಣ form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      ವರ್ಣ *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      ಪೂರ್ಣಾಙ್ಕ nb = 0;  /* number of bytes in result */
      ಯದಿ (++arg > ತುದಿ)
        ಪ್ರತ್ಯಾಗಮ luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      ವಿಕಲ್ಪ (*strfrmt++) {
        ಪಕ್ಷ 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, arg));
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ 'd': ಪಕ್ಷ 'i':
          flags = L_FMTFLAGSI;
          ಗಚ್ಛ intcase;
        ಪಕ್ಷ 'u':
          flags = L_FMTFLAGSU;
          ಗಚ್ಛ intcase;
        ಪಕ್ಷ 'o': ಪಕ್ಷ 'x': ಪಕ್ಷ 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ 'a': ಪಕ್ಷ 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ಸಂಖ್ಯೆ_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          ಭಙ್ಗ;
        ಪಕ್ಷ 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        ಪಕ್ಷ 'e': ಪಕ್ಷ 'E': ಪಕ್ಷ 'g': ಪಕ್ಷ 'G': {
          lua_ಸಂಖ್ಯೆ n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ಸಂಖ್ಯೆ_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ 'p': {
          ಧ್ರುವ ಶೂನ್ಯ *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          ಯದಿ (p == ರಿಕ್ತ) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ 'q': {
          ಯದಿ (form[2] != '\0')  /* modifiers? */
            ಪ್ರತ್ಯಾಗಮ luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          ಭಙ್ಗ;
        }
        ಪಕ್ಷ 's': {
          size_t l;
          ಧ್ರುವ ವರ್ಣ *s = luaL_tolstring(L, arg, &l);
          ಯದಿ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          ಅನ್ಯಥಾ {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            ಯದಿ (strchr(form, '.') == ರಿಕ್ತ && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            ಅನ್ಯಥಾ {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_ತೆಗೆ(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          ಭಙ್ಗ;
        }
        ಸ್ವಭಾವ: {  /* also treat cases 'pnLlh' */
          ಪ್ರತ್ಯಾಗಮ luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  ಪ್ರತ್ಯಾಗಮ 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#ಯದಿ !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(lua_Integer))


/* dummy union to get native endianness */
ಸ್ಥಿರ ಧ್ರುವ ಸಙ್ಘ {
  ಪೂರ್ಣಾಙ್ಕ dummy;
  ವರ್ಣ little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Header {
  lua_State *L;
  ಪೂರ್ಣಾಙ್ಕ islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ KOption {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ digit (ಪೂರ್ಣಾಙ್ಕ c) { ಪ್ರತ್ಯಾಗಮ '0' <= c && c <= '9'; }

ಸ್ಥಿರ size_t getnum (ಧ್ರುವ ವರ್ಣ **fmt, size_t df) {
  ಯದಿ (!digit(**fmt))  /* no number? */
    ಪ್ರತ್ಯಾಗಮ df;  /* return default value */
  ಅನ್ಯಥಾ {
    size_t a = 0;
    ಕುರು {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } ಯಾವತ್ (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    ಪ್ರತ್ಯಾಗಮ a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ಸ್ಥಿರ unsigned getnumlimit (Header *h, ಧ್ರುವ ವರ್ಣ **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  ಯದಿ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    ಪ್ರತ್ಯಾಗಮ cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  ಪ್ರತ್ಯಾಗಮ cast_uint(sz);
}


/*
** Initialize Header
*/
ಸ್ಥಿರ ಶೂನ್ಯ initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
ಸ್ಥಿರ KOption getoption (Header *h, ಧ್ರುವ ವರ್ಣ **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  ಸಂರಚನಾ cD { ವರ್ಣ c; ಸಙ್ಘ { LUAI_MAXALIGN; } u; };
  ಪೂರ್ಣಾಙ್ಕ opt = *((*fmt)++);
  *size = 0;  /* default */
  ವಿಕಲ್ಪ (opt) {
    ಪಕ್ಷ 'b': *size = ಪರಿಮಾಣ(ವರ್ಣ); ಪ್ರತ್ಯಾಗಮ Kint;
    ಪಕ್ಷ 'B': *size = ಪರಿಮಾಣ(ವರ್ಣ); ಪ್ರತ್ಯಾಗಮ Kuint;
    ಪಕ್ಷ 'h': *size = ಪರಿಮಾಣ(short); ಪ್ರತ್ಯಾಗಮ Kint;
    ಪಕ್ಷ 'H': *size = ಪರಿಮಾಣ(short); ಪ್ರತ್ಯಾಗಮ Kuint;
    ಪಕ್ಷ 'l': *size = ಪರಿಮಾಣ(long); ಪ್ರತ್ಯಾಗಮ Kint;
    ಪಕ್ಷ 'L': *size = ಪರಿಮಾಣ(long); ಪ್ರತ್ಯಾಗಮ Kuint;
    ಪಕ್ಷ 'j': *size = ಪರಿಮಾಣ(lua_Integer); ಪ್ರತ್ಯಾಗಮ Kint;
    ಪಕ್ಷ 'J': *size = ಪರಿಮಾಣ(lua_Integer); ಪ್ರತ್ಯಾಗಮ Kuint;
    ಪಕ್ಷ 'T': *size = ಪರಿಮಾಣ(size_t); ಪ್ರತ್ಯಾಗಮ Kuint;
    ಪಕ್ಷ 'f': *size = ಪರಿಮಾಣ(ಭಿನ್ನ); ಪ್ರತ್ಯಾಗಮ Kfloat;
    ಪಕ್ಷ 'n': *size = ಪರಿಮಾಣ(lua_ಸಂಖ್ಯೆ); ಪ್ರತ್ಯಾಗಮ Knumber;
    ಪಕ್ಷ 'd': *size = ಪರಿಮಾಣ(ದ್ವಿಗುಣ); ಪ್ರತ್ಯಾಗಮ Kdouble;
    ಪಕ್ಷ 'i': *size = getnumlimit(h, fmt, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)); ಪ್ರತ್ಯಾಗಮ Kint;
    ಪಕ್ಷ 'I': *size = getnumlimit(h, fmt, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ)); ಪ್ರತ್ಯಾಗಮ Kuint;
    ಪಕ್ಷ 's': *size = getnumlimit(h, fmt, ಪರಿಮಾಣ(size_t)); ಪ್ರತ್ಯಾಗಮ Kstring;
    ಪಕ್ಷ 'c':
      *size = getnum(fmt, cast_sizet(-1));
      ಯದಿ (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      ಪ್ರತ್ಯಾಗಮ Kchar;
    ಪಕ್ಷ 'z': ಪ್ರತ್ಯಾಗಮ Kzstr;
    ಪಕ್ಷ 'x': *size = 1; ಪ್ರತ್ಯಾಗಮ Kpadding;
    ಪಕ್ಷ 'X': ಪ್ರತ್ಯಾಗಮ Kpaddalign;
    ಪಕ್ಷ ' ': ಭಙ್ಗ;
    ಪಕ್ಷ '<': h->islittle = 1; ಭಙ್ಗ;
    ಪಕ್ಷ '>': h->islittle = 0; ಭಙ್ಗ;
    ಪಕ್ಷ '=': h->islittle = nativeendian.little; ಭಙ್ಗ;
    ಪಕ್ಷ '!': {
      ಧ್ರುವ size_t maxalign = offsetof(ಸಂರಚನಾ cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  ಪ್ರತ್ಯಾಗಮ Knop;
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
ಸ್ಥಿರ KOption getdetails (Header *h, size_t totalsize, ಧ್ರುವ ವರ್ಣ **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  ಯದಿ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    ಯದಿ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  ಯದಿ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  ಅನ್ಯಥಾ {
    ಯದಿ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    ಯದಿ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    ಅನ್ಯಥಾ {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  ಪ್ರತ್ಯಾಗಮ opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
ಸ್ಥಿರ ಶೂನ್ಯ packint (luaL_Buffer *b, lua_Unsigned n,
                     ಪೂರ್ಣಾಙ್ಕ islittle, unsigned size, ಪೂರ್ಣಾಙ್ಕ neg) {
  ವರ್ಣ *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (ವರ್ಣ)(n & MC);  /* first byte */
  ಪ್ರತಿ (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (ವರ್ಣ)(n & MC);
  }
  ಯದಿ (neg && size > SZINT) {  /* negative number need sign extension? */
    ಪ್ರತಿ (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (ವರ್ಣ)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
ಸ್ಥಿರ ಶೂನ್ಯ copywithendian (ವರ್ಣ *dest, ಧ್ರುವ ವರ್ಣ *src,
                            unsigned size, ಪೂರ್ಣಾಙ್ಕ islittle) {
  ಯದಿ (islittle == nativeendian.little)
    memcpy(dest, src, size);
  ಅನ್ಯಥಾ {
    dest += size - 1;
    ಯಾವತ್ (size-- != 0)
      *(dest--) = *(src++);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  ಧ್ರುವ ವರ್ಣ *fmt = luaL_checkstring(L, 1);  /* format string */
  ಪೂರ್ಣಾಙ್ಕ arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  ಯಾವತ್ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    ಯಾವತ್ (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    ವಿಕಲ್ಪ (opt) {
      ಪಕ್ಷ Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ಯದಿ (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ಯದಿ (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kfloat: {  /* C float */
        ಭಿನ್ನ f = (ಭಿನ್ನ)luaL_checknumber(L, arg);  /* get argument */
        ವರ್ಣ *buff = luaL_prepbuffsize(&b, ಪರಿಮಾಣ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ವರ್ಣ *)&f, ಪರಿಮಾಣ(f), h.islittle);
        luaL_addsize(&b, size);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Knumber: {  /* Lua float */
        lua_ಸಂಖ್ಯೆ f = luaL_checknumber(L, arg);  /* get argument */
        ವರ್ಣ *buff = luaL_prepbuffsize(&b, ಪರಿಮಾಣ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ವರ್ಣ *)&f, ಪರಿಮಾಣ(f), h.islittle);
        luaL_addsize(&b, size);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kdouble: {  /* C double */
        ದ್ವಿಗುಣ f = (ದ್ವಿಗುಣ)luaL_checknumber(L, arg);  /* get argument */
        ವರ್ಣ *buff = luaL_prepbuffsize(&b, ಪರಿಮಾಣ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ವರ್ಣ *)&f, ಪರಿಮಾಣ(f), h.islittle);
        luaL_addsize(&b, size);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kchar: {  /* fixed-size string */
        size_t len;
        ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        ಯದಿ (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          ವರ್ಣ *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kstring: {  /* strings with length count */
        size_t len;
        ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= ಪರಿಮಾಣ(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kzstr: {  /* zero-terminated string */
        size_t len;
        ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      ಪಕ್ಷ Kpaddalign: ಪಕ್ಷ Knop:
        arg--;  /* undo increment */
        ಭಙ್ಗ;
    }
  }
  luaL_pushresult(&b);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_packsize (lua_State *L) {
  Header h;
  ಧ್ರುವ ವರ್ಣ *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  ಯಾವತ್ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    size += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_SIZE - size,
                     1, "format result too large");
    totalsize += size;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
ಸ್ಥಿರ lua_Integer unpackint (lua_State *L, ಧ್ರುವ ವರ್ಣ *str,
                              ಪೂರ್ಣಾಙ್ಕ islittle, ಪೂರ್ಣಾಙ್ಕ size, ಪೂರ್ಣಾಙ್ಕ issigned) {
  lua_Unsigned res = 0;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ limit = (size  <= SZINT) ? size : SZINT;
  ಪ್ರತಿ (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned ವರ್ಣ)str[islittle ? i : size - 1 - i];
  }
  ಯದಿ (size < SZINT) {  /* real size smaller than lua_Integer? */
    ಯದಿ (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  ಅನ್ಯಥಾ ಯದಿ (size > SZINT) {  /* must check unread bytes */
    ಪೂರ್ಣಾಙ್ಕ mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    ಪ್ರತಿ (i = limit; i < size; i++) {
      ಯದಿ (l_unlikely((unsigned ವರ್ಣ)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  ಪ್ರತ್ಯಾಗಮ (lua_Integer)res;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ str_unpack (lua_State *L) {
  Header h;
  ಧ್ರುವ ವರ್ಣ *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ಧ್ರುವ ವರ್ಣ *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  ಪೂರ್ಣಾಙ್ಕ n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  ಯಾವತ್ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    ವಿಕಲ್ಪ (opt) {
      ಪಕ್ಷ Kint:
      ಪಕ್ಷ Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kfloat: {
        ಭಿನ್ನ f;
        copywithendian((ವರ್ಣ *)&f, data + pos, ಪರಿಮಾಣ(f), h.islittle);
        lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)f);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Knumber: {
        lua_ಸಂಖ್ಯೆ f;
        copywithendian((ವರ್ಣ *)&f, data + pos, ಪರಿಮಾಣ(f), h.islittle);
        lua_pushnumber(L, f);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kdouble: {
        ದ್ವಿಗುಣ f;
        copywithendian((ವರ್ಣ *)&f, data + pos, ಪರಿಮಾಣ(f), h.islittle);
        lua_pushnumber(L, (lua_ಸಂಖ್ಯೆ)f);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kchar: {
        lua_pushlstring(L, data + pos, size);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ Kpaddalign: ಪಕ್ಷ Kpadding: ಪಕ್ಷ Knop:
        n--;  /* undo increment */
        ಭಙ್ಗ;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  ಪ್ರತ್ಯಾಗಮ n + 1;
}

/* }====================================================== */


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_find},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_lower},
  {"match", str_match},
  {"rep", str_rep},
  {"reverse", str_reverse},
  {"sub", str_sub},
  {"upper", str_upper},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {ರಿಕ್ತ, ರಿಕ್ತ}
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
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  ಪ್ರತ್ಯಾಗಮ 1;
}

