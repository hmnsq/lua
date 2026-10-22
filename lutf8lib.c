/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
ಸ್ಥಿರ lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  ಯದಿ (pos >= 0) ಪ್ರತ್ಯಾಗಮ pos;
  ಅನ್ಯಥಾ ಯದಿ (0u - (size_t)pos > len) ಪ್ರತ್ಯಾಗಮ 0;
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *utf8_decode (ಧ್ರುವ ವರ್ಣ *s, l_uint32 *val, ಪೂರ್ಣಾಙ್ಕ strict) {
  ಸ್ಥಿರ ಧ್ರುವ l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned ಪೂರ್ಣಾಙ್ಕ c = (unsigned ವರ್ಣ)s[0];
  l_uint32 res = 0;  /* final result */
  ಯದಿ (c < 0x80)  /* ASCII? */
    res = c;
  ಅನ್ಯಥಾ ಯದಿ (c >= 0xfe)  /* c >= 1111 1110b ? */
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* would need six or more continuation bytes */
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;  /* to count number of continuation bytes */
    ಪ್ರತಿ (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned ಪೂರ್ಣಾಙ್ಕ cc = (unsigned ವರ್ಣ)s[++ಎಣಿಕೆ];  /* read next byte */
      ಯದಿ (!iscont(cc))  /* not a continuation byte? */
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(ಎಣಿಕೆ <= 5);
    res |= ((l_uint32)(c & 0x7F) << (ಎಣಿಕೆ * 5));  /* add first byte */
    ಯದಿ (res > MAXUTF || res < limits[ಎಣಿಕೆ])
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* invalid byte sequence */
    s += ಎಣಿಕೆ;  /* skip continuation bytes read */
  }
  ಯದಿ (strict) {
    /* check for invalid code points; too large or surrogates */
    ಯದಿ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಯದಿ (val) *val = res;
  ಪ್ರತ್ಯಾಗಮ s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  ಪೂರ್ಣಾಙ್ಕ lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  ಯಾವತ್ (posi <= posj) {
    ಧ್ರುವ ವರ್ಣ *s1 = utf8_decode(s + posi, ರಿಕ್ತ, !lax);
    ಯದಿ (s1 == ರಿಕ್ತ) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ಪ್ರತ್ಯಾಗಮ 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ codepoint (lua_State *L) {
  size_t len;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  ಪೂರ್ಣಾಙ್ಕ lax = lua_toboolean(L, 4);
  ಪೂರ್ಣಾಙ್ಕ n;
  ಧ್ರುವ ವರ್ಣ *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  ಯದಿ (posi > pose) ಪ್ರತ್ಯಾಗಮ 0;  /* empty interval; return no values */
  ಯದಿ (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "string slice too long");
  n = (ಪೂರ್ಣಾಙ್ಕ)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  ಪ್ರತಿ (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    ಯದಿ (s == ರಿಕ್ತ)
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  ಪ್ರತ್ಯಾಗಮ n;
}


ಸ್ಥಿರ ಶೂನ್ಯ pushutfchar (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ utfchar (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);  /* number of arguments */
  ಯದಿ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    ಪ್ರತಿ (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ byteoffset (lua_State *L) {
  size_t len;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  ಯದಿ (n == 0) {
    /* find beginning of current byte sequence */
    ಯಾವತ್ (posi > 0 && iscontp(s + posi)) posi--;
  }
  ಅನ್ಯಥಾ {
    ಯದಿ (iscontp(s + posi))
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "initial position is a continuation byte");
    ಯದಿ (n < 0) {
      ಯಾವತ್ (n < 0 && posi > 0) {  /* move back */
        ಕುರು {  /* find beginning of previous character */
          posi--;
        } ಯಾವತ್ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    ಅನ್ಯಥಾ {
      n--;  /* do not move for 1st character */
      ಯಾವತ್ (n > 0 && posi < (lua_Integer)len) {
        ಕುರು {  /* find beginning of next character */
          posi++;
        } ಯಾವತ್ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  ಯದಿ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  ಯದಿ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    ಯದಿ (iscont(s[posi]))
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "initial position is a continuation byte");
    ಯಾವತ್ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ಪ್ರತ್ಯಾಗಮ 2;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ iter_aux (lua_State *L, ಪೂರ್ಣಾಙ್ಕ strict) {
  size_t len;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  ಯದಿ (n < len) {
    ಯಾವತ್ (iscontp(s + n)) n++;  /* go to next character */
  }
  ಯದಿ (n >= len)  /* (also handles original 'n' being negative) */
    ಪ್ರತ್ಯಾಗಮ 0;  /* no more codepoints */
  ಅನ್ಯಥಾ {
    l_uint32 code;
    ಧ್ರುವ ವರ್ಣ *next = utf8_decode(s + n, &code, strict);
    ಯದಿ (next == ರಿಕ್ತ || iscontp(next))
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    ಪ್ರತ್ಯಾಗಮ 2;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ iter_auxstrict (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ iter_aux(L, 1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ iter_auxlax (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ iter_aux(L, 0);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ iter_codes (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ lax = lua_toboolean(L, 2);
  ಧ್ರುವ ವರ್ಣ *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ಪ್ರತ್ಯಾಗಮ 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ರಿಕ್ತ},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, ಪರಿಮಾಣ(UTF8PATT)/ಪರಿಮಾಣ(ವರ್ಣ) - 1);
  lua_setfield(L, -2, "charpattern");
  ಪ್ರತ್ಯಾಗಮ 1;
}

