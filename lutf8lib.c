/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
நிலையான lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  எனில் (pos >= 0) திரும்பு pos;
  இல்லையெனில் எனில் (0u - (size_t)pos > len) திரும்பு 0;
  இல்லையெனில் திரும்பு (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
நிலையான மாறிலி எழுத்து *utf8_decode (மாறிலி எழுத்து *s, l_uint32 *val, முழு strict) {
  நிலையான மாறிலி l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned முழு c = (unsigned எழுத்து)s[0];
  l_uint32 res = 0;  /* final result */
  எனில் (c < 0x80)  /* ASCII? */
    res = c;
  இல்லையெனில் எனில் (c >= 0xfe)  /* c >= 1111 1110b ? */
    திரும்பு வெற்று;  /* would need six or more continuation bytes */
  இல்லையெனில் {
    முழு மொத்தம் = 0;  /* to count number of continuation bytes */
    க்கு (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned முழு cc = (unsigned எழுத்து)s[++மொத்தம்];  /* read next byte */
      எனில் (!iscont(cc))  /* not a continuation byte? */
        திரும்பு வெற்று;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(மொத்தம் <= 5);
    res |= ((l_uint32)(c & 0x7F) << (மொத்தம் * 5));  /* add first byte */
    எனில் (res > MAXUTF || res < limits[மொத்தம்])
      திரும்பு வெற்று;  /* invalid byte sequence */
    s += மொத்தம்;  /* skip continuation bytes read */
  }
  எனில் (strict) {
    /* check for invalid code points; too large or surrogates */
    எனில் (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      திரும்பு வெற்று;
  }
  எனில் (val) *val = res;
  திரும்பு s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
நிலையான முழு utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  முழு lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  போது (posi <= posj) {
    மாறிலி எழுத்து *s1 = utf8_decode(s + posi, வெற்று, !lax);
    எனில் (s1 == வெற்று) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      திரும்பு 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  திரும்பு 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
நிலையான முழு codepoint (lua_State *L) {
  size_t len;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  முழு lax = lua_toboolean(L, 4);
  முழு n;
  மாறிலி எழுத்து *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  எனில் (posi > pose) திரும்பு 0;  /* empty interval; return no values */
  எனில் (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    திரும்பு luaL_error(L, "string slice too long");
  n = (முழு)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  க்கு (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    எனில் (s == வெற்று)
      திரும்பு luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  திரும்பு n;
}


நிலையான வெறுமை pushutfchar (lua_State *L, முழு arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
நிலையான முழு utfchar (lua_State *L) {
  முழு n = lua_gettop(L);  /* number of arguments */
  எனில் (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  இல்லையெனில் {
    முழு i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    க்கு (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  திரும்பு 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
நிலையான முழு byteoffset (lua_State *L) {
  size_t len;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  எனில் (n == 0) {
    /* find beginning of current byte sequence */
    போது (posi > 0 && iscontp(s + posi)) posi--;
  }
  இல்லையெனில் {
    எனில் (iscontp(s + posi))
      திரும்பு luaL_error(L, "initial position is a continuation byte");
    எனில் (n < 0) {
      போது (n < 0 && posi > 0) {  /* move back */
        செய் {  /* find beginning of previous character */
          posi--;
        } போது (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    இல்லையெனில் {
      n--;  /* do not move for 1st character */
      போது (n > 0 && posi < (lua_Integer)len) {
        செய் {  /* find beginning of next character */
          posi++;
        } போது (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  எனில் (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    திரும்பு 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  எனில் ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    எனில் (iscont(s[posi]))
      திரும்பு luaL_error(L, "initial position is a continuation byte");
    போது (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  திரும்பு 2;
}


நிலையான முழு iter_aux (lua_State *L, முழு strict) {
  size_t len;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  எனில் (n < len) {
    போது (iscontp(s + n)) n++;  /* go to next character */
  }
  எனில் (n >= len)  /* (also handles original 'n' being negative) */
    திரும்பு 0;  /* no more codepoints */
  இல்லையெனில் {
    l_uint32 code;
    மாறிலி எழுத்து *next = utf8_decode(s + n, &code, strict);
    எனில் (next == வெற்று || iscontp(next))
      திரும்பு luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    திரும்பு 2;
  }
}


நிலையான முழு iter_auxstrict (lua_State *L) {
  திரும்பு iter_aux(L, 1);
}

நிலையான முழு iter_auxlax (lua_State *L) {
  திரும்பு iter_aux(L, 0);
}


நிலையான முழு iter_codes (lua_State *L) {
  முழு lax = lua_toboolean(L, 2);
  மாறிலி எழுத்து *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  திரும்பு 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


நிலையான மாறிலி luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", வெற்று},
  {வெற்று, வெற்று}
};


LUAMOD_API முழு luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, அளவு(UTF8PATT)/அளவு(எழுத்து) - 1);
  lua_setfield(L, -2, "charpattern");
  திரும்பு 1;
}

