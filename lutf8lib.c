/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
സ്ഥിതം lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  എങ്കിൽ (pos >= 0) തിരികെ pos;
  അല്ലെങ്കിൽ എങ്കിൽ (0u - (size_t)pos > len) തിരികെ 0;
  അല്ലെങ്കിൽ തിരികെ (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *utf8_decode (സ്ഥിരം അക്ഷരം *s, l_uint32 *val, പൂർണ്ണസംഖ്യ strict) {
  സ്ഥിതം സ്ഥിരം l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned പൂർണ്ണസംഖ്യ c = (unsigned അക്ഷരം)s[0];
  l_uint32 res = 0;  /* final result */
  എങ്കിൽ (c < 0x80)  /* ASCII? */
    res = c;
  അല്ലെങ്കിൽ എങ്കിൽ (c >= 0xfe)  /* c >= 1111 1110b ? */
    തിരികെ ശൂന്യ;  /* would need six or more continuation bytes */
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ എണ്ണം = 0;  /* to count number of continuation bytes */
    വേണ്ടി (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned പൂർണ്ണസംഖ്യ cc = (unsigned അക്ഷരം)s[++എണ്ണം];  /* read next byte */
      എങ്കിൽ (!iscont(cc))  /* not a continuation byte? */
        തിരികെ ശൂന്യ;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(എണ്ണം <= 5);
    res |= ((l_uint32)(c & 0x7F) << (എണ്ണം * 5));  /* add first byte */
    എങ്കിൽ (res > MAXUTF || res < limits[എണ്ണം])
      തിരികെ ശൂന്യ;  /* invalid byte sequence */
    s += എണ്ണം;  /* skip continuation bytes read */
  }
  എങ്കിൽ (strict) {
    /* check for invalid code points; too large or surrogates */
    എങ്കിൽ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      തിരികെ ശൂന്യ;
  }
  എങ്കിൽ (val) *val = res;
  തിരികെ s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  പൂർണ്ണസംഖ്യ lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  ആകുമ്പോൾ (posi <= posj) {
    സ്ഥിരം അക്ഷരം *s1 = utf8_decode(s + posi, ശൂന്യ, !lax);
    എങ്കിൽ (s1 == ശൂന്യ) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      തിരികെ 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  തിരികെ 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ codepoint (lua_State *L) {
  size_t len;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  പൂർണ്ണസംഖ്യ lax = lua_toboolean(L, 4);
  പൂർണ്ണസംഖ്യ n;
  സ്ഥിരം അക്ഷരം *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  എങ്കിൽ (posi > pose) തിരികെ 0;  /* empty interval; return no values */
  എങ്കിൽ (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    തിരികെ luaL_error(L, "string slice too long");
  n = (പൂർണ്ണസംഖ്യ)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  വേണ്ടി (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    എങ്കിൽ (s == ശൂന്യ)
      തിരികെ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  തിരികെ n;
}


സ്ഥിതം ശൂന്യം pushutfchar (lua_State *L, പൂർണ്ണസംഖ്യ arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ utfchar (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);  /* number of arguments */
  എങ്കിൽ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    വേണ്ടി (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  തിരികെ 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ byteoffset (lua_State *L) {
  size_t len;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  എങ്കിൽ (n == 0) {
    /* find beginning of current byte sequence */
    ആകുമ്പോൾ (posi > 0 && iscontp(s + posi)) posi--;
  }
  അല്ലെങ്കിൽ {
    എങ്കിൽ (iscontp(s + posi))
      തിരികെ luaL_error(L, "initial position is a continuation byte");
    എങ്കിൽ (n < 0) {
      ആകുമ്പോൾ (n < 0 && posi > 0) {  /* move back */
        ചെയ്യുക {  /* find beginning of previous character */
          posi--;
        } ആകുമ്പോൾ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    അല്ലെങ്കിൽ {
      n--;  /* do not move for 1st character */
      ആകുമ്പോൾ (n > 0 && posi < (lua_Integer)len) {
        ചെയ്യുക {  /* find beginning of next character */
          posi++;
        } ആകുമ്പോൾ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  എങ്കിൽ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    തിരികെ 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  എങ്കിൽ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    എങ്കിൽ (iscont(s[posi]))
      തിരികെ luaL_error(L, "initial position is a continuation byte");
    ആകുമ്പോൾ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  തിരികെ 2;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ iter_aux (lua_State *L, പൂർണ്ണസംഖ്യ strict) {
  size_t len;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  എങ്കിൽ (n < len) {
    ആകുമ്പോൾ (iscontp(s + n)) n++;  /* go to next character */
  }
  എങ്കിൽ (n >= len)  /* (also handles original 'n' being negative) */
    തിരികെ 0;  /* no more codepoints */
  അല്ലെങ്കിൽ {
    l_uint32 code;
    സ്ഥിരം അക്ഷരം *next = utf8_decode(s + n, &code, strict);
    എങ്കിൽ (next == ശൂന്യ || iscontp(next))
      തിരികെ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    തിരികെ 2;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ iter_auxstrict (lua_State *L) {
  തിരികെ iter_aux(L, 1);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ iter_auxlax (lua_State *L) {
  തിരികെ iter_aux(L, 0);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ iter_codes (lua_State *L) {
  പൂർണ്ണസംഖ്യ lax = lua_toboolean(L, 2);
  സ്ഥിരം അക്ഷരം *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  തിരികെ 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


സ്ഥിതം സ്ഥിരം luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ശൂന്യ},
  {ശൂന്യ, ശൂന്യ}
};


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, വലുപ്പം(UTF8PATT)/വലുപ്പം(അക്ഷരം) - 1);
  lua_setfield(L, -2, "charpattern");
  തിരികെ 1;
}

