/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
ایستا lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  اگر (pos >= 0) بازگشت pos;
  وگرنه اگر (0u - (size_t)pos > len) بازگشت 0;
  وگرنه بازگشت (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
ایستا ثابت نویسه *utf8_decode (ثابت نویسه *s, l_uint32 *val, صحیح strict) {
  ایستا ثابت l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned صحیح c = (unsigned نویسه)s[0];
  l_uint32 res = 0;  /* final result */
  اگر (c < 0x80)  /* ASCII? */
    res = c;
  وگرنه اگر (c >= 0xfe)  /* c >= 1111 1110b ? */
    بازگشت پوچ;  /* would need six or more continuation bytes */
  وگرنه {
    صحیح شمار = 0;  /* to count number of continuation bytes */
    برای (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned صحیح cc = (unsigned نویسه)s[++شمار];  /* read next byte */
      اگر (!iscont(cc))  /* not a continuation byte? */
        بازگشت پوچ;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(شمار <= 5);
    res |= ((l_uint32)(c & 0x7F) << (شمار * 5));  /* add first byte */
    اگر (res > MAXUTF || res < limits[شمار])
      بازگشت پوچ;  /* invalid byte sequence */
    s += شمار;  /* skip continuation bytes read */
  }
  اگر (strict) {
    /* check for invalid code points; too large or surrogates */
    اگر (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      بازگشت پوچ;
  }
  اگر (val) *val = res;
  بازگشت s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
ایستا صحیح utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ثابت نویسه *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  صحیح lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  وقتی (posi <= posj) {
    ثابت نویسه *s1 = utf8_decode(s + posi, پوچ, !lax);
    اگر (s1 == پوچ) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      بازگشت 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  بازگشت 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
ایستا صحیح codepoint (lua_State *L) {
  size_t len;
  ثابت نویسه *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  صحیح lax = lua_toboolean(L, 4);
  صحیح n;
  ثابت نویسه *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  اگر (posi > pose) بازگشت 0;  /* empty interval; return no values */
  اگر (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    بازگشت luaL_error(L, "string slice too long");
  n = (صحیح)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  برای (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    اگر (s == پوچ)
      بازگشت luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  بازگشت n;
}


ایستا تهی pushutfchar (lua_State *L, صحیح arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
ایستا صحیح utfchar (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  اگر (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  وگرنه {
    صحیح i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    برای (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  بازگشت 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
ایستا صحیح byteoffset (lua_State *L) {
  size_t len;
  ثابت نویسه *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  اگر (n == 0) {
    /* find beginning of current byte sequence */
    وقتی (posi > 0 && iscontp(s + posi)) posi--;
  }
  وگرنه {
    اگر (iscontp(s + posi))
      بازگشت luaL_error(L, "initial position is a continuation byte");
    اگر (n < 0) {
      وقتی (n < 0 && posi > 0) {  /* move back */
        انجامبده {  /* find beginning of previous character */
          posi--;
        } وقتی (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    وگرنه {
      n--;  /* do not move for 1st character */
      وقتی (n > 0 && posi < (lua_Integer)len) {
        انجامبده {  /* find beginning of next character */
          posi++;
        } وقتی (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  اگر (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    بازگشت 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  اگر ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    اگر (iscont(s[posi]))
      بازگشت luaL_error(L, "initial position is a continuation byte");
    وقتی (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  بازگشت 2;
}


ایستا صحیح iter_aux (lua_State *L, صحیح strict) {
  size_t len;
  ثابت نویسه *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  اگر (n < len) {
    وقتی (iscontp(s + n)) n++;  /* go to next character */
  }
  اگر (n >= len)  /* (also handles original 'n' being negative) */
    بازگشت 0;  /* no more codepoints */
  وگرنه {
    l_uint32 code;
    ثابت نویسه *next = utf8_decode(s + n, &code, strict);
    اگر (next == پوچ || iscontp(next))
      بازگشت luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    بازگشت 2;
  }
}


ایستا صحیح iter_auxstrict (lua_State *L) {
  بازگشت iter_aux(L, 1);
}

ایستا صحیح iter_auxlax (lua_State *L) {
  بازگشت iter_aux(L, 0);
}


ایستا صحیح iter_codes (lua_State *L) {
  صحیح lax = lua_toboolean(L, 2);
  ثابت نویسه *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  بازگشت 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


ایستا ثابت luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", پوچ},
  {پوچ, پوچ}
};


LUAMOD_API صحیح luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, اندازه(UTF8PATT)/اندازه(نویسه) - 1);
  lua_setfield(L, -2, "charpattern");
  بازگشت 1;
}

