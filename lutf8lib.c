/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
स्थिर lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  जइ (pos >= 0) प्रत्यागम pos;
  अण्णहा जइ (0u - (size_t)pos > len) प्रत्यागम 0;
  अण्णहा प्रत्यागम (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
स्थिर ध्रुव वर्ण *utf8_decode (ध्रुव वर्ण *s, l_uint32 *val, पूर्णाङ्क strict) {
  स्थिर ध्रुव l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned पूर्णाङ्क c = (unsigned वर्ण)s[0];
  l_uint32 res = 0;  /* final result */
  जइ (c < 0x80)  /* ASCII? */
    res = c;
  अण्णहा जइ (c >= 0xfe)  /* c >= 1111 1110b ? */
    प्रत्यागम रिक्त;  /* would need six or more continuation bytes */
  अण्णहा {
    पूर्णाङ्क गणनम् = 0;  /* to count number of continuation bytes */
    प्रति (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned पूर्णाङ्क cc = (unsigned वर्ण)s[++गणनम्];  /* read next byte */
      जइ (!iscont(cc))  /* not a continuation byte? */
        प्रत्यागम रिक्त;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(गणनम् <= 5);
    res |= ((l_uint32)(c & 0x7F) << (गणनम् * 5));  /* add first byte */
    जइ (res > MAXUTF || res < limits[गणनम्])
      प्रत्यागम रिक्त;  /* invalid byte sequence */
    s += गणनम्;  /* skip continuation bytes read */
  }
  जइ (strict) {
    /* check for invalid code points; too large or surrogates */
    जइ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      प्रत्यागम रिक्त;
  }
  जइ (val) *val = res;
  प्रत्यागम s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
स्थिर पूर्णाङ्क utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  पूर्णाङ्क lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  जाव (posi <= posj) {
    ध्रुव वर्ण *s1 = utf8_decode(s + posi, रिक्त, !lax);
    जइ (s1 == रिक्त) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      प्रत्यागम 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  प्रत्यागम 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
स्थिर पूर्णाङ्क codepoint (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  पूर्णाङ्क lax = lua_toboolean(L, 4);
  पूर्णाङ्क n;
  ध्रुव वर्ण *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  जइ (posi > pose) प्रत्यागम 0;  /* empty interval; return no values */
  जइ (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    प्रत्यागम luaL_error(L, "string slice too long");
  n = (पूर्णाङ्क)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  प्रति (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    जइ (s == रिक्त)
      प्रत्यागम luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  प्रत्यागम n;
}


स्थिर शून्य pushutfchar (lua_State *L, पूर्णाङ्क arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
स्थिर पूर्णाङ्क utfchar (lua_State *L) {
  पूर्णाङ्क n = lua_gettop(L);  /* number of arguments */
  जइ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  अण्णहा {
    पूर्णाङ्क i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    प्रति (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  प्रत्यागम 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
स्थिर पूर्णाङ्क byteoffset (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  जइ (n == 0) {
    /* find beginning of current byte sequence */
    जाव (posi > 0 && iscontp(s + posi)) posi--;
  }
  अण्णहा {
    जइ (iscontp(s + posi))
      प्रत्यागम luaL_error(L, "initial position is a continuation byte");
    जइ (n < 0) {
      जाव (n < 0 && posi > 0) {  /* move back */
        करेहि {  /* find beginning of previous character */
          posi--;
        } जाव (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    अण्णहा {
      n--;  /* do not move for 1st character */
      जाव (n > 0 && posi < (lua_Integer)len) {
        करेहि {  /* find beginning of next character */
          posi++;
        } जाव (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  जइ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    प्रत्यागम 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  जइ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    जइ (iscont(s[posi]))
      प्रत्यागम luaL_error(L, "initial position is a continuation byte");
    जाव (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  प्रत्यागम 2;
}


स्थिर पूर्णाङ्क iter_aux (lua_State *L, पूर्णाङ्क strict) {
  size_t len;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  जइ (n < len) {
    जाव (iscontp(s + n)) n++;  /* go to next character */
  }
  जइ (n >= len)  /* (also handles original 'n' being negative) */
    प्रत्यागम 0;  /* no more codepoints */
  अण्णहा {
    l_uint32 code;
    ध्रुव वर्ण *next = utf8_decode(s + n, &code, strict);
    जइ (next == रिक्त || iscontp(next))
      प्रत्यागम luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    प्रत्यागम 2;
  }
}


स्थिर पूर्णाङ्क iter_auxstrict (lua_State *L) {
  प्रत्यागम iter_aux(L, 1);
}

स्थिर पूर्णाङ्क iter_auxlax (lua_State *L) {
  प्रत्यागम iter_aux(L, 0);
}


स्थिर पूर्णाङ्क iter_codes (lua_State *L) {
  पूर्णाङ्क lax = lua_toboolean(L, 2);
  ध्रुव वर्ण *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  प्रत्यागम 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


स्थिर ध्रुव luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", रिक्त},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णाङ्क luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, परिमाण(UTF8PATT)/परिमाण(वर्ण) - 1);
  lua_setfield(L, -2, "charpattern");
  प्रत्यागम 1;
}

