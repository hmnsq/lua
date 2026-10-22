/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lauxlib.h"
#অন্তর্ভাৱ "lualib.h"
#অন্তর্ভাৱ "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
স্থির lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  যদি (pos >= 0) ঘূৰাই pos;
  অন্যথা যদি (0u - (size_t)pos > len) ঘূৰাই 0;
  অন্যথা ঘূৰাই (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
স্থির ধ্রুৱ ৱর্ণ *utf8_decode (ধ্রুৱ ৱর্ণ *s, l_uint32 *val, পূর্ণাঙ্ক strict) {
  স্থির ধ্রুৱ l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned পূর্ণাঙ্ক c = (unsigned ৱর্ণ)s[0];
  l_uint32 res = 0;  /* final result */
  যদি (c < 0x80)  /* ASCII? */
    res = c;
  অন্যথা যদি (c >= 0xfe)  /* c >= 1111 1110b ? */
    ঘূৰাই রিক্ত;  /* would need six or more continuation bytes */
  অন্যথা {
    পূর্ণাঙ্ক গুনতি = 0;  /* to count number of continuation bytes */
    বাবে (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned পূর্ণাঙ্ক cc = (unsigned ৱর্ণ)s[++গুনতি];  /* read next byte */
      যদি (!iscont(cc))  /* not a continuation byte? */
        ঘূৰাই রিক্ত;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(গুনতি <= 5);
    res |= ((l_uint32)(c & 0x7F) << (গুনতি * 5));  /* add first byte */
    যদি (res > MAXUTF || res < limits[গুনতি])
      ঘূৰাই রিক্ত;  /* invalid byte sequence */
    s += গুনতি;  /* skip continuation bytes read */
  }
  যদি (strict) {
    /* check for invalid code points; too large or surrogates */
    যদি (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ঘূৰাই রিক্ত;
  }
  যদি (val) *val = res;
  ঘূৰাই s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
স্থির পূর্ণাঙ্ক utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  পূর্ণাঙ্ক lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  যেতিয়ালৈ (posi <= posj) {
    ধ্রুৱ ৱর্ণ *s1 = utf8_decode(s + posi, রিক্ত, !lax);
    যদি (s1 == রিক্ত) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ঘূৰাই 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ঘূৰাই 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
স্থির পূর্ণাঙ্ক codepoint (lua_State *L) {
  size_t len;
  ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  পূর্ণাঙ্ক lax = lua_toboolean(L, 4);
  পূর্ণাঙ্ক n;
  ধ্রুৱ ৱর্ণ *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  যদি (posi > pose) ঘূৰাই 0;  /* empty interval; return no values */
  যদি (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    ঘূৰাই luaL_error(L, "string slice too long");
  n = (পূর্ণাঙ্ক)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  বাবে (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    যদি (s == রিক্ত)
      ঘূৰাই luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  ঘূৰাই n;
}


স্থির শূন্য pushutfchar (lua_State *L, পূর্ণাঙ্ক arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
স্থির পূর্ণাঙ্ক utfchar (lua_State *L) {
  পূর্ণাঙ্ক n = lua_gettop(L);  /* number of arguments */
  যদি (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  অন্যথা {
    পূর্ণাঙ্ক i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    বাবে (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ঘূৰাই 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
স্থির পূর্ণাঙ্ক byteoffset (lua_State *L) {
  size_t len;
  ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  যদি (n == 0) {
    /* find beginning of current byte sequence */
    যেতিয়ালৈ (posi > 0 && iscontp(s + posi)) posi--;
  }
  অন্যথা {
    যদি (iscontp(s + posi))
      ঘূৰাই luaL_error(L, "initial position is a continuation byte");
    যদি (n < 0) {
      যেতিয়ালৈ (n < 0 && posi > 0) {  /* move back */
        কৰা {  /* find beginning of previous character */
          posi--;
        } যেতিয়ালৈ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    অন্যথা {
      n--;  /* do not move for 1st character */
      যেতিয়ালৈ (n > 0 && posi < (lua_Integer)len) {
        কৰা {  /* find beginning of next character */
          posi++;
        } যেতিয়ালৈ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  যদি (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ঘূৰাই 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  যদি ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    যদি (iscont(s[posi]))
      ঘূৰাই luaL_error(L, "initial position is a continuation byte");
    যেতিয়ালৈ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ঘূৰাই 2;
}


স্থির পূর্ণাঙ্ক iter_aux (lua_State *L, পূর্ণাঙ্ক strict) {
  size_t len;
  ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  যদি (n < len) {
    যেতিয়ালৈ (iscontp(s + n)) n++;  /* go to next character */
  }
  যদি (n >= len)  /* (also handles original 'n' being negative) */
    ঘূৰাই 0;  /* no more codepoints */
  অন্যথা {
    l_uint32 code;
    ধ্রুৱ ৱর্ণ *next = utf8_decode(s + n, &code, strict);
    যদি (next == রিক্ত || iscontp(next))
      ঘূৰাই luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    ঘূৰাই 2;
  }
}


স্থির পূর্ণাঙ্ক iter_auxstrict (lua_State *L) {
  ঘূৰাই iter_aux(L, 1);
}

স্থির পূর্ণাঙ্ক iter_auxlax (lua_State *L) {
  ঘূৰাই iter_aux(L, 0);
}


স্থির পূর্ণাঙ্ক iter_codes (lua_State *L) {
  পূর্ণাঙ্ক lax = lua_toboolean(L, 2);
  ধ্রুৱ ৱর্ণ *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ঘূৰাই 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


স্থির ধ্রুৱ luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", রিক্ত},
  {রিক্ত, রিক্ত}
};


LUAMOD_API পূর্ণাঙ্ক luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, পরিমাণ(UTF8PATT)/পরিমাণ(ৱর্ণ) - 1);
  lua_setfield(L, -2, "charpattern");
  ঘূৰাই 1;
}

