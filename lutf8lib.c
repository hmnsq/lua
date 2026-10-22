/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lutf8lib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXUNICODE	0x10FFFFu

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXUTF		0x7FFFFFFFu


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MSGInvalid	"invalid UTF-8 code"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ iscont(c)	(((c) & 0xC0) == 0x80)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
ਸਥਿਰ lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  ਜੇ (pos >= 0) ਵਾਪਸ pos;
  ਨਹੀਂਤਾਂ ਜੇ (0u - (size_t)pos > len) ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ ਵਾਪਸ (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *utf8_decode (ਅਚਲ ਅੱਖਰ *s, l_uint32 *val, ਪੂਰਨ strict) {
  ਸਥਿਰ ਅਚਲ l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ c = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)s[0];
  l_uint32 res = 0;  /* final result */
  ਜੇ (c < 0x80)  /* ASCII? */
    res = c;
  ਨਹੀਂਤਾਂ ਜੇ (c >= 0xfe)  /* c >= 1111 1110b ? */
    ਵਾਪਸ ਨਿਰਾ;  /* would need six or more continuation bytes */
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ ਗਿਣਤੀ_2 = 0;  /* to count number of continuation bytes */
    ਲਈ (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cc = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)s[++ਗਿਣਤੀ_2];  /* read next byte */
      ਜੇ (!iscont(cc))  /* not a continuation byte? */
        ਵਾਪਸ ਨਿਰਾ;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(ਗਿਣਤੀ_2 <= 5);
    res |= ((l_uint32)(c & 0x7F) << (ਗਿਣਤੀ_2 * 5));  /* add first byte */
    ਜੇ (res > MAXUTF || res < limits[ਗਿਣਤੀ_2])
      ਵਾਪਸ ਨਿਰਾ;  /* invalid byte sequence */
    s += ਗਿਣਤੀ_2;  /* skip continuation bytes read */
  }
  ਜੇ (strict) {
    /* check for invalid code points; too large or surrogates */
    ਜੇ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ਵਾਪਸ ਨਿਰਾ;
  }
  ਜੇ (val) *val = res;
  ਵਾਪਸ s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
ਸਥਿਰ ਪੂਰਨ utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  ਪੂਰਨ lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  ਜਦੋਂ (posi <= posj) {
    ਅਚਲ ਅੱਖਰ *s1 = utf8_decode(s + posi, ਨਿਰਾ, !lax);
    ਜੇ (s1 == ਨਿਰਾ) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ਵਾਪਸ 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ਵਾਪਸ 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
ਸਥਿਰ ਪੂਰਨ codepoint (lua_State *L) {
  size_t len;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  ਪੂਰਨ lax = lua_toboolean(L, 4);
  ਪੂਰਨ n;
  ਅਚਲ ਅੱਖਰ *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  ਜੇ (posi > pose) ਵਾਪਸ 0;  /* empty interval; return no values */
  ਜੇ (pose - posi >= ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)  /* (lua_Integer -> int) overflow? */
    ਵਾਪਸ luaL_error(L, "string slice too long");
  n = (ਪੂਰਨ)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  ਲਈ (s += posi - 1; s < se;) {
    l_uint32 ਕੋਡ;
    s = utf8_decode(s, &ਕੋਡ, !lax);
    ਜੇ (s == ਨਿਰਾ)
      ਵਾਪਸ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(ਕੋਡ));
    n++;
  }
  ਵਾਪਸ n;
}


ਸਥਿਰ ਖਾਲੀ pushutfchar (lua_State *L, ਪੂਰਨ arg) {
  lua_Unsigned ਕੋਡ = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, ਕੋਡ <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ, ਕੋਡ));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
ਸਥਿਰ ਪੂਰਨ utfchar (lua_State *L) {
  ਪੂਰਨ n = lua_gettop(L);  /* number of arguments */
  ਜੇ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ i;
    lua_L_ਬਫਰ b;
    luaL_buffinit(L, &b);
    ਲਈ (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ਵਾਪਸ 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
ਸਥਿਰ ਪੂਰਨ byteoffset (lua_State *L) {
  size_t len;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  ਜੇ (n == 0) {
    /* find beginning of current byte sequence */
    ਜਦੋਂ (posi > 0 && iscontp(s + posi)) posi--;
  }
  ਨਹੀਂਤਾਂ {
    ਜੇ (iscontp(s + posi))
      ਵਾਪਸ luaL_error(L, "initial position is a continuation byte");
    ਜੇ (n < 0) {
      ਜਦੋਂ (n < 0 && posi > 0) {  /* move back */
        ਕਰੋ {  /* find beginning of previous character */
          posi--;
        } ਜਦੋਂ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    ਨਹੀਂਤਾਂ {
      n--;  /* do not move for 1st character */
      ਜਦੋਂ (n > 0 && posi < (lua_Integer)len) {
        ਕਰੋ {  /* find beginning of next character */
          posi++;
        } ਜਦੋਂ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  ਜੇ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ਵਾਪਸ 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  ਜੇ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    ਜੇ (iscont(s[posi]))
      ਵਾਪਸ luaL_error(L, "initial position is a continuation byte");
    ਜਦੋਂ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ਵਾਪਸ 2;
}


ਸਥਿਰ ਪੂਰਨ iter_aux (lua_State *L, ਪੂਰਨ strict) {
  size_t len;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  ਜੇ (n < len) {
    ਜਦੋਂ (iscontp(s + n)) n++;  /* go to next character */
  }
  ਜੇ (n >= len)  /* (also handles original 'n' being negative) */
    ਵਾਪਸ 0;  /* no more codepoints */
  ਨਹੀਂਤਾਂ {
    l_uint32 ਕੋਡ;
    ਅਚਲ ਅੱਖਰ *ਅਗਲਾ = utf8_decode(s + n, &ਕੋਡ, strict);
    ਜੇ (ਅਗਲਾ == ਨਿਰਾ || iscontp(ਅਗਲਾ))
      ਵਾਪਸ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(ਕੋਡ));
    ਵਾਪਸ 2;
  }
}


ਸਥਿਰ ਪੂਰਨ iter_auxstrict (lua_State *L) {
  ਵਾਪਸ iter_aux(L, 1);
}

ਸਥਿਰ ਪੂਰਨ iter_auxlax (lua_State *L) {
  ਵਾਪਸ iter_aux(L, 0);
}


ਸਥਿਰ ਪੂਰਨ iter_codes (lua_State *L) {
  ਪੂਰਨ lax = lua_toboolean(L, 2);
  ਅਚਲ ਅੱਖਰ *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ਵਾਪਸ 3;
}


/* pattern to match a single UTF-8 character */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


ਸਥਿਰ ਅਚਲ luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ਨਿਰਾ},
  {ਨਿਰਾ, ਨਿਰਾ}
};


LUAMOD_API ਪੂਰਨ luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, ਆਕਾਰ(UTF8PATT)/ਆਕਾਰ(ਅੱਖਰ) - 1);
  lua_setfield(L, -2, "charpattern");
  ਵਾਪਸ 1;
}

