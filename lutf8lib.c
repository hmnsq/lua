/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lutf8lib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


#ವ್ಯಾಖ್ಯಾನಿಸು MAXUNICODE	0x10FFFFu

#ವ್ಯಾಖ್ಯಾನಿಸು MAXUTF		0x7FFFFFFFu


#ವ್ಯಾಖ್ಯಾನಿಸು MSGInvalid	"invalid UTF-8 code"


#ವ್ಯಾಖ್ಯಾನಿಸು iscont(c)	(((c) & 0xC0) == 0x80)
#ವ್ಯಾಖ್ಯಾನಿಸು iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
ಸ್ಥಿರ lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  ಆದರೆ (pos >= 0) ಹಿಂತಿರುಗಿ pos;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (0u - (size_t)pos > len) ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *utf8_decode (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, l_uint32 *val, ಪೂರ್ಣಾಂಕ strict) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ c = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)s[0];
  l_uint32 res = 0;  /* final result */
  ಆದರೆ (c < 0x80)  /* ASCII? */
    res = c;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c >= 0xfe)  /* c >= 1111 1110b ? */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* would need six or more continuation bytes */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;  /* to count number of continuation bytes */
    ಗಾಗಿ (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cc = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)s[++ಎಣಿಕೆ_2];  /* read next byte */
      ಆದರೆ (!iscont(cc))  /* not a continuation byte? */
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(ಎಣಿಕೆ_2 <= 5);
    res |= ((l_uint32)(c & 0x7F) << (ಎಣಿಕೆ_2 * 5));  /* add first byte */
    ಆದರೆ (res > MAXUTF || res < limits[ಎಣಿಕೆ_2])
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* invalid byte sequence */
    s += ಎಣಿಕೆ_2;  /* skip continuation bytes read */
  }
  ಆದರೆ (strict) {
    /* check for invalid code points; too large or surrogates */
    ಆದರೆ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಆದರೆ (val) *val = res;
  ಹಿಂತಿರುಗಿ s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  ಪೂರ್ಣಾಂಕ lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  ಆಗ (posi <= posj) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1 = utf8_decode(s + posi, ಖಾಲಿ, !lax);
    ಆದರೆ (s1 == ಖಾಲಿ) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ಹಿಂತಿರುಗಿ 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ codepoint (lua_State *L) {
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  ಪೂರ್ಣಾಂಕ lax = lua_toboolean(L, 4);
  ಪೂರ್ಣಾಂಕ n;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  ಆದರೆ (posi > pose) ಹಿಂತಿರುಗಿ 0;  /* empty interval; return no values */
  ಆದರೆ (pose - posi >= ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)  /* (lua_Integer -> int) overflow? */
    ಹಿಂತಿರುಗಿ luaL_error(L, "string slice too long");
  n = (ಪೂರ್ಣಾಂಕ)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  ಗಾಗಿ (s += posi - 1; s < se;) {
    l_uint32 ಸಂಕೇತ_2;
    s = utf8_decode(s, &ಸಂಕೇತ_2, !lax);
    ಆದರೆ (s == ಖಾಲಿ)
      ಹಿಂತಿರುಗಿ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(ಸಂಕೇತ_2));
    n++;
  }
  ಹಿಂತಿರುಗಿ n;
}


ಸ್ಥಿರ ಶೂನ್ಯ pushutfchar (lua_State *L, ಪೂರ್ಣಾಂಕ arg) {
  lua_Unsigned ಸಂಕೇತ_2 = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, ಸಂಕೇತ_2 <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ, ಸಂಕೇತ_2));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ utfchar (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);  /* number of arguments */
  ಆದರೆ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ i;
    lua_L_ಬಫರ್ b;
    luaL_buffinit(L, &b);
    ಗಾಗಿ (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ಹಿಂತಿರುಗಿ 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ byteoffset (lua_State *L) {
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  ಆದರೆ (n == 0) {
    /* find beginning of current byte sequence */
    ಆಗ (posi > 0 && iscontp(s + posi)) posi--;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (iscontp(s + posi))
      ಹಿಂತಿರುಗಿ luaL_error(L, "initial position is a continuation byte");
    ಆದರೆ (n < 0) {
      ಆಗ (n < 0 && posi > 0) {  /* move back */
        ಮಾಡು {  /* find beginning of previous character */
          posi--;
        } ಆಗ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      n--;  /* do not move for 1st character */
      ಆಗ (n > 0 && posi < (lua_Integer)len) {
        ಮಾಡು {  /* find beginning of next character */
          posi++;
        } ಆಗ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  ಆದರೆ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ಹಿಂತಿರುಗಿ 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  ಆದರೆ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    ಆದರೆ (iscont(s[posi]))
      ಹಿಂತಿರುಗಿ luaL_error(L, "initial position is a continuation byte");
    ಆಗ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ಹಿಂತಿರುಗಿ 2;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ iter_aux (lua_State *L, ಪೂರ್ಣಾಂಕ strict) {
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  ಆದರೆ (n < len) {
    ಆಗ (iscontp(s + n)) n++;  /* go to next character */
  }
  ಆದರೆ (n >= len)  /* (also handles original 'n' being negative) */
    ಹಿಂತಿರುಗಿ 0;  /* no more codepoints */
  ಇಲ್ಲದಿದ್ದರೆ {
    l_uint32 ಸಂಕೇತ_2;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮುಂದಿನ = utf8_decode(s + n, &ಸಂಕೇತ_2, strict);
    ಆದರೆ (ಮುಂದಿನ == ಖಾಲಿ || iscontp(ಮುಂದಿನ))
      ಹಿಂತಿರುಗಿ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(ಸಂಕೇತ_2));
    ಹಿಂತಿರುಗಿ 2;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ iter_auxstrict (lua_State *L) {
  ಹಿಂತಿರುಗಿ iter_aux(L, 1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ iter_auxlax (lua_State *L) {
  ಹಿಂತಿರುಗಿ iter_aux(L, 0);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ iter_codes (lua_State *L) {
  ಪೂರ್ಣಾಂಕ lax = lua_toboolean(L, 2);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ಹಿಂತಿರುಗಿ 3;
}


/* pattern to match a single UTF-8 character */
#ವ್ಯಾಖ್ಯಾನಿಸು UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ಖಾಲಿ},
  {ಖಾಲಿ, ಖಾಲಿ}
};


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, ಗಾತ್ರ(UTF8PATT)/ಗಾತ್ರ(ಅಕ್ಷರ) - 1);
  lua_setfield(L, -2, "charpattern");
  ಹಿಂತಿರುಗಿ 1;
}

