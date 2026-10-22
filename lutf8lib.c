/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
স্থির lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  যদি (pos >= 0) ফেরত pos;
  অন্যথা যদি (0u - (size_t)pos > len) ফেরত 0;
  অন্যথা ফেরত (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
স্থির ধ্রুবক অক্ষর *utf8_decode (ধ্রুবক অক্ষর *s, l_uint32 *val, পূর্ণসংখ্যা strict) {
  স্থির ধ্রুবক l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned পূর্ণসংখ্যা c = (unsigned অক্ষর)s[0];
  l_uint32 res = 0;  /* final result */
  যদি (c < 0x80)  /* ASCII? */
    res = c;
  অন্যথা যদি (c >= 0xfe)  /* c >= 1111 1110b ? */
    ফেরত ফাঁকা;  /* would need six or more continuation bytes */
  অন্যথা {
    পূর্ণসংখ্যা গুনতি = 0;  /* to count number of continuation bytes */
    জন্য (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned পূর্ণসংখ্যা cc = (unsigned অক্ষর)s[++গুনতি];  /* read next byte */
      যদি (!iscont(cc))  /* not a continuation byte? */
        ফেরত ফাঁকা;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(গুনতি <= 5);
    res |= ((l_uint32)(c & 0x7F) << (গুনতি * 5));  /* add first byte */
    যদি (res > MAXUTF || res < limits[গুনতি])
      ফেরত ফাঁকা;  /* invalid byte sequence */
    s += গুনতি;  /* skip continuation bytes read */
  }
  যদি (strict) {
    /* check for invalid code points; too large or surrogates */
    যদি (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ফেরত ফাঁকা;
  }
  যদি (val) *val = res;
  ফেরত s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
স্থির পূর্ণসংখ্যা utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  পূর্ণসংখ্যা lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  যখন (posi <= posj) {
    ধ্রুবক অক্ষর *s1 = utf8_decode(s + posi, ফাঁকা, !lax);
    যদি (s1 == ফাঁকা) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ফেরত 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ফেরত 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
স্থির পূর্ণসংখ্যা codepoint (lua_State *L) {
  size_t len;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  পূর্ণসংখ্যা lax = lua_toboolean(L, 4);
  পূর্ণসংখ্যা n;
  ধ্রুবক অক্ষর *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  যদি (posi > pose) ফেরত 0;  /* empty interval; return no values */
  যদি (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    ফেরত luaL_error(L, "string slice too long");
  n = (পূর্ণসংখ্যা)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  জন্য (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    যদি (s == ফাঁকা)
      ফেরত luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  ফেরত n;
}


স্থির শূন্য pushutfchar (lua_State *L, পূর্ণসংখ্যা arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
স্থির পূর্ণসংখ্যা utfchar (lua_State *L) {
  পূর্ণসংখ্যা n = lua_gettop(L);  /* number of arguments */
  যদি (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  অন্যথা {
    পূর্ণসংখ্যা i;
    lua_L_বাফার b;
    luaL_buffinit(L, &b);
    জন্য (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ফেরত 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
স্থির পূর্ণসংখ্যা byteoffset (lua_State *L) {
  size_t len;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  যদি (n == 0) {
    /* find beginning of current byte sequence */
    যখন (posi > 0 && iscontp(s + posi)) posi--;
  }
  অন্যথা {
    যদি (iscontp(s + posi))
      ফেরত luaL_error(L, "initial position is a continuation byte");
    যদি (n < 0) {
      যখন (n < 0 && posi > 0) {  /* move back */
        করো {  /* find beginning of previous character */
          posi--;
        } যখন (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    অন্যথা {
      n--;  /* do not move for 1st character */
      যখন (n > 0 && posi < (lua_Integer)len) {
        করো {  /* find beginning of next character */
          posi++;
        } যখন (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  যদি (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ফেরত 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  যদি ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    যদি (iscont(s[posi]))
      ফেরত luaL_error(L, "initial position is a continuation byte");
    যখন (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ফেরত 2;
}


স্থির পূর্ণসংখ্যা iter_aux (lua_State *L, পূর্ণসংখ্যা strict) {
  size_t len;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  যদি (n < len) {
    যখন (iscontp(s + n)) n++;  /* go to next character */
  }
  যদি (n >= len)  /* (also handles original 'n' being negative) */
    ফেরত 0;  /* no more codepoints */
  অন্যথা {
    l_uint32 code;
    ধ্রুবক অক্ষর *পরবর্তী = utf8_decode(s + n, &code, strict);
    যদি (পরবর্তী == ফাঁকা || iscontp(পরবর্তী))
      ফেরত luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    ফেরত 2;
  }
}


স্থির পূর্ণসংখ্যা iter_auxstrict (lua_State *L) {
  ফেরত iter_aux(L, 1);
}

স্থির পূর্ণসংখ্যা iter_auxlax (lua_State *L) {
  ফেরত iter_aux(L, 0);
}


স্থির পূর্ণসংখ্যা iter_codes (lua_State *L) {
  পূর্ণসংখ্যা lax = lua_toboolean(L, 2);
  ধ্রুবক অক্ষর *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ফেরত 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


স্থির ধ্রুবক luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ফাঁকা},
  {ফাঁকা, ফাঁকা}
};


LUAMOD_API পূর্ণসংখ্যা luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, আকার(UTF8PATT)/আকার(অক্ষর) - 1);
  lua_setfield(L, -2, "charpattern");
  ফেরত 1;
}

