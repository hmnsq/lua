/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#تعریف_کرو lutf8lib_c
#تعریف_کرو LUA_LIB

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


#تعریف_کرو MAXUNICODE	0x10FFFFu

#تعریف_کرو MAXUTF		0x7FFFFFFFu


#تعریف_کرو MSGInvalid	"invalid UTF-8 code"


#تعریف_کرو iscont(c)	(((c) & 0xC0) == 0x80)
#تعریف_کرو iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
ساکن lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  اگر (pos >= 0) واپس pos;
  ورنہ اگر (0u - (size_t)pos > len) واپس 0;
  ورنہ واپس (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
ساکن ثابت حرف *utf8_decode (ثابت حرف *s, l_uint32 *val, صحیح strict) {
  ساکن ثابت l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  غیرعلامتی صحیح c = (غیرعلامتی حرف)s[0];
  l_uint32 res = 0;  /* final result */
  اگر (c < 0x80)  /* ASCII? */
    res = c;
  ورنہ اگر (c >= 0xfe)  /* c >= 1111 1110b ? */
    واپس تہی;  /* would need six or more continuation bytes */
  ورنہ {
    صحیح گنتی = 0;  /* to count number of continuation bytes */
    برائے (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      غیرعلامتی صحیح cc = (غیرعلامتی حرف)s[++گنتی];  /* read next byte */
      اگر (!iscont(cc))  /* not a continuation byte? */
        واپس تہی;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(گنتی <= 5);
    res |= ((l_uint32)(c & 0x7F) << (گنتی * 5));  /* add first byte */
    اگر (res > MAXUTF || res < limits[گنتی])
      واپس تہی;  /* invalid byte sequence */
    s += گنتی;  /* skip continuation bytes read */
  }
  اگر (strict) {
    /* check for invalid code points; too large or surrogates */
    اگر (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      واپس تہی;
  }
  اگر (val) *val = res;
  واپس s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
ساکن صحیح utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  صحیح lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  جب (posi <= posj) {
    ثابت حرف *s1 = utf8_decode(s + posi, تہی, !lax);
    اگر (s1 == تہی) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      واپس 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  واپس 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
ساکن صحیح codepoint (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  صحیح lax = lua_toboolean(L, 4);
  صحیح n;
  ثابت حرف *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  اگر (posi > pose) واپس 0;  /* empty interval; return no values */
  اگر (pose - posi >= زیادہ_سے_زیادہ_عدد)  /* (lua_Integer -> int) overflow? */
    واپس luaL_error(L, "string slice too long");
  n = (صحیح)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  برائے (s += posi - 1; s < se;) {
    l_uint32 رمز;
    s = utf8_decode(s, &رمز, !lax);
    اگر (s == تہی)
      واپس luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(رمز));
    n++;
  }
  واپس n;
}


ساکن خالی pushutfchar (lua_State *L, صحیح arg) {
  lua_Unsigned رمز = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, رمز <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(غیرعلامتی طویل, رمز));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
ساکن صحیح utfchar (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  اگر (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  ورنہ {
    صحیح i;
    lua_L_بفر b;
    luaL_buffinit(L, &b);
    برائے (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  واپس 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
ساکن صحیح byteoffset (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  اگر (n == 0) {
    /* find beginning of current byte sequence */
    جب (posi > 0 && iscontp(s + posi)) posi--;
  }
  ورنہ {
    اگر (iscontp(s + posi))
      واپس luaL_error(L, "initial position is a continuation byte");
    اگر (n < 0) {
      جب (n < 0 && posi > 0) {  /* move back */
        کرو {  /* find beginning of previous character */
          posi--;
        } جب (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    ورنہ {
      n--;  /* do not move for 1st character */
      جب (n > 0 && posi < (lua_Integer)len) {
        کرو {  /* find beginning of next character */
          posi++;
        } جب (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  اگر (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    واپس 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  اگر ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    اگر (iscont(s[posi]))
      واپس luaL_error(L, "initial position is a continuation byte");
    جب (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  واپس 2;
}


ساکن صحیح iter_aux (lua_State *L, صحیح strict) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  اگر (n < len) {
    جب (iscontp(s + n)) n++;  /* go to next character */
  }
  اگر (n >= len)  /* (also handles original 'n' being negative) */
    واپس 0;  /* no more codepoints */
  ورنہ {
    l_uint32 رمز;
    ثابت حرف *اگلا = utf8_decode(s + n, &رمز, strict);
    اگر (اگلا == تہی || iscontp(اگلا))
      واپس luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(رمز));
    واپس 2;
  }
}


ساکن صحیح iter_auxstrict (lua_State *L) {
  واپس iter_aux(L, 1);
}

ساکن صحیح iter_auxlax (lua_State *L) {
  واپس iter_aux(L, 0);
}


ساکن صحیح iter_codes (lua_State *L) {
  صحیح lax = lua_toboolean(L, 2);
  ثابت حرف *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  واپس 3;
}


/* pattern to match a single UTF-8 character */
#تعریف_کرو UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


ساکن ثابت luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", تہی},
  {تہی, تہی}
};


LUAMOD_API صحیح luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, حجم(UTF8PATT)/حجم(حرف) - 1);
  lua_setfield(L, -2, "charpattern");
  واپس 1;
}

