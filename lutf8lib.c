/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
ସ୍ଥିର lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  ଯଦି (pos >= 0) ଫେରନ୍ତୁ pos;
  ଅନ୍ୟଥା ଯଦି (0u - (size_t)pos > len) ଫେରନ୍ତୁ 0;
  ଅନ୍ୟଥା ଫେରନ୍ତୁ (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *utf8_decode (ଅଚଳ ଅକ୍ଷର *s, l_uint32 *val, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strict) {
  ସ୍ଥିର ଅଚଳ l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = (unsigned ଅକ୍ଷର)s[0];
  l_uint32 res = 0;  /* final result */
  ଯଦି (c < 0x80)  /* ASCII? */
    res = c;
  ଅନ୍ୟଥା ଯଦି (c >= 0xfe)  /* c >= 1111 1110b ? */
    ଫେରନ୍ତୁ ଖାଲି;  /* would need six or more continuation bytes */
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;  /* to count number of continuation bytes */
    ପାଇଁ (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cc = (unsigned ଅକ୍ଷର)s[++ଗଣନା_2];  /* read next byte */
      ଯଦି (!iscont(cc))  /* not a continuation byte? */
        ଫେରନ୍ତୁ ଖାଲି;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(ଗଣନା_2 <= 5);
    res |= ((l_uint32)(c & 0x7F) << (ଗଣନା_2 * 5));  /* add first byte */
    ଯଦି (res > MAXUTF || res < limits[ଗଣନା_2])
      ଫେରନ୍ତୁ ଖାଲି;  /* invalid byte sequence */
    s += ଗଣନା_2;  /* skip continuation bytes read */
  }
  ଯଦି (strict) {
    /* check for invalid code points; too large or surrogates */
    ଯଦି (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ଫେରନ୍ତୁ ଖାଲି;
  }
  ଯଦି (val) *val = res;
  ଫେରନ୍ତୁ s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  ଯେତେବେଳେ (posi <= posj) {
    ଅଚଳ ଅକ୍ଷର *s1 = utf8_decode(s + posi, ଖାଲି, !lax);
    ଯଦି (s1 == ଖାଲି) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ଫେରନ୍ତୁ 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ଫେରନ୍ତୁ 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା codepoint (lua_State *L) {
  size_t len;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lax = lua_toboolean(L, 4);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ଅଚଳ ଅକ୍ଷର *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  ଯଦି (posi > pose) ଫେରନ୍ତୁ 0;  /* empty interval; return no values */
  ଯଦି (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    ଫେରନ୍ତୁ luaL_error(L, "string slice too long");
  n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  ପାଇଁ (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    ଯଦି (s == ଖାଲି)
      ଫେରନ୍ତୁ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  ଫେରନ୍ତୁ n;
}


ସ୍ଥିର ଶୂନ୍ୟ pushutfchar (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା utfchar (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);  /* number of arguments */
  ଯଦି (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    ପାଇଁ (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ଫେରନ୍ତୁ 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା byteoffset (lua_State *L) {
  size_t len;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  ଯଦି (n == 0) {
    /* find beginning of current byte sequence */
    ଯେତେବେଳେ (posi > 0 && iscontp(s + posi)) posi--;
  }
  ଅନ୍ୟଥା {
    ଯଦି (iscontp(s + posi))
      ଫେରନ୍ତୁ luaL_error(L, "initial position is a continuation byte");
    ଯଦି (n < 0) {
      ଯେତେବେଳେ (n < 0 && posi > 0) {  /* move back */
        କର {  /* find beginning of previous character */
          posi--;
        } ଯେତେବେଳେ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    ଅନ୍ୟଥା {
      n--;  /* do not move for 1st character */
      ଯେତେବେଳେ (n > 0 && posi < (lua_Integer)len) {
        କର {  /* find beginning of next character */
          posi++;
        } ଯେତେବେଳେ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  ଯଦି (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ଫେରନ୍ତୁ 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  ଯଦି ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    ଯଦି (iscont(s[posi]))
      ଫେରନ୍ତୁ luaL_error(L, "initial position is a continuation byte");
    ଯେତେବେଳେ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ଫେରନ୍ତୁ 2;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା iter_aux (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strict) {
  size_t len;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  ଯଦି (n < len) {
    ଯେତେବେଳେ (iscontp(s + n)) n++;  /* go to next character */
  }
  ଯଦି (n >= len)  /* (also handles original 'n' being negative) */
    ଫେରନ୍ତୁ 0;  /* no more codepoints */
  ଅନ୍ୟଥା {
    l_uint32 code;
    ଅଚଳ ଅକ୍ଷର *next = utf8_decode(s + n, &code, strict);
    ଯଦି (next == ଖାଲି || iscontp(next))
      ଫେରନ୍ତୁ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    ଫେରନ୍ତୁ 2;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା iter_auxstrict (lua_State *L) {
  ଫେରନ୍ତୁ iter_aux(L, 1);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା iter_auxlax (lua_State *L) {
  ଫେରନ୍ତୁ iter_aux(L, 0);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା iter_codes (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lax = lua_toboolean(L, 2);
  ଅଚଳ ଅକ୍ଷର *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ଫେରନ୍ତୁ 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


ସ୍ଥିର ଅଚଳ luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ଖାଲି},
  {ଖାଲି, ଖାଲି}
};


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, ଆକାର(UTF8PATT)/ଆକାର(ଅକ୍ଷର) - 1);
  lua_setfield(L, -2, "charpattern");
  ଫେରନ୍ତୁ 1;
}

