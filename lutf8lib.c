/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#ضمن "lprefix.h"


#ضمن <limits.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
ساكن lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  إذا (pos >= 0) أرجع pos;
  وإلا إذا (0u - (size_t)pos > len) أرجع 0;
  وإلا أرجع (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
ساكن ثابت حرف *utf8_decode (ثابت حرف *s, l_uint32 *val, صحيح strict) {
  ساكن ثابت l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned صحيح c = (unsigned حرف)s[0];
  l_uint32 res = 0;  /* final result */
  إذا (c < 0x80)  /* ASCII? */
    res = c;
  وإلا إذا (c >= 0xfe)  /* c >= 1111 1110b ? */
    أرجع عدم;  /* would need six or more continuation bytes */
  وإلا {
    صحيح سانى = 0;  /* to count number of continuation bytes */
    لكل (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned صحيح cc = (unsigned حرف)s[++سانى];  /* read next byte */
      إذا (!iscont(cc))  /* not a continuation byte? */
        أرجع عدم;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(سانى <= 5);
    res |= ((l_uint32)(c & 0x7F) << (سانى * 5));  /* add first byte */
    إذا (res > MAXUTF || res < limits[سانى])
      أرجع عدم;  /* invalid byte sequence */
    s += سانى;  /* skip continuation bytes read */
  }
  إذا (strict) {
    /* check for invalid code points; too large or surrogates */
    إذا (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      أرجع عدم;
  }
  إذا (val) *val = res;
  أرجع s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
ساكن صحيح utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  صحيح lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  طالما (posi <= posj) {
    ثابت حرف *s1 = utf8_decode(s + posi, عدم, !lax);
    إذا (s1 == عدم) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      أرجع 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  أرجع 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
ساكن صحيح codepoint (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  صحيح lax = lua_toboolean(L, 4);
  صحيح n;
  ثابت حرف *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  إذا (posi > pose) أرجع 0;  /* empty interval; return no values */
  إذا (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    أرجع luaL_error(L, "string slice too long");
  n = (صحيح)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  لكل (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    إذا (s == عدم)
      أرجع luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  أرجع n;
}


ساكن فراغ pushutfchar (lua_State *L, صحيح arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
ساكن صحيح utfchar (lua_State *L) {
  صحيح n = lua_gettop(L);  /* number of arguments */
  إذا (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  وإلا {
    صحيح i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    لكل (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  أرجع 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
ساكن صحيح byteoffset (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  إذا (n == 0) {
    /* find beginning of current byte sequence */
    طالما (posi > 0 && iscontp(s + posi)) posi--;
  }
  وإلا {
    إذا (iscontp(s + posi))
      أرجع luaL_error(L, "initial position is a continuation byte");
    إذا (n < 0) {
      طالما (n < 0 && posi > 0) {  /* move back */
        افعل {  /* find beginning of previous character */
          posi--;
        } طالما (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    وإلا {
      n--;  /* do not move for 1st character */
      طالما (n > 0 && posi < (lua_Integer)len) {
        افعل {  /* find beginning of next character */
          posi++;
        } طالما (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  إذا (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    أرجع 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  إذا ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    إذا (iscont(s[posi]))
      أرجع luaL_error(L, "initial position is a continuation byte");
    طالما (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  أرجع 2;
}


ساكن صحيح iter_aux (lua_State *L, صحيح strict) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  إذا (n < len) {
    طالما (iscontp(s + n)) n++;  /* go to next character */
  }
  إذا (n >= len)  /* (also handles original 'n' being negative) */
    أرجع 0;  /* no more codepoints */
  وإلا {
    l_uint32 code;
    ثابت حرف *next = utf8_decode(s + n, &code, strict);
    إذا (next == عدم || iscontp(next))
      أرجع luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    أرجع 2;
  }
}


ساكن صحيح iter_auxstrict (lua_State *L) {
  أرجع iter_aux(L, 1);
}

ساكن صحيح iter_auxlax (lua_State *L) {
  أرجع iter_aux(L, 0);
}


ساكن صحيح iter_codes (lua_State *L) {
  صحيح lax = lua_toboolean(L, 2);
  ثابت حرف *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  أرجع 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


ساكن ثابت luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", عدم},
  {عدم, عدم}
};


LUAMOD_API صحيح luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, حجم(UTF8PATT)/حجم(حرف) - 1);
  lua_setfield(L, -2, "charpattern");
  أرجع 1;
}

