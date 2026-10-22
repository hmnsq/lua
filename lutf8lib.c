/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "lualib.h"
#અન્તર્ભાવ "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
સ્થિર lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  જો (pos >= 0) પાછા pos;
  નહીંતર જો (0u - (size_t)pos > len) પાછા 0;
  નહીંતર પાછા (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
સ્થિર અચલ અક્ષર *utf8_decode (અચલ અક્ષર *s, l_uint32 *val, પૂર્ણાંક strict) {
  સ્થિર અચલ l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned પૂર્ણાંક c = (unsigned અક્ષર)s[0];
  l_uint32 res = 0;  /* final result */
  જો (c < 0x80)  /* ASCII? */
    res = c;
  નહીંતર જો (c >= 0xfe)  /* c >= 1111 1110b ? */
    પાછા ખાલી;  /* would need six or more continuation bytes */
  નહીંતર {
    પૂર્ણાંક ગણતરી = 0;  /* to count number of continuation bytes */
    માટે (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned પૂર્ણાંક cc = (unsigned અક્ષર)s[++ગણતરી];  /* read next byte */
      જો (!iscont(cc))  /* not a continuation byte? */
        પાછા ખાલી;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(ગણતરી <= 5);
    res |= ((l_uint32)(c & 0x7F) << (ગણતરી * 5));  /* add first byte */
    જો (res > MAXUTF || res < limits[ગણતરી])
      પાછા ખાલી;  /* invalid byte sequence */
    s += ગણતરી;  /* skip continuation bytes read */
  }
  જો (strict) {
    /* check for invalid code points; too large or surrogates */
    જો (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      પાછા ખાલી;
  }
  જો (val) *val = res;
  પાછા s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
સ્થિર પૂર્ણાંક utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  અચલ અક્ષર *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  પૂર્ણાંક lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  જ્યારે (posi <= posj) {
    અચલ અક્ષર *s1 = utf8_decode(s + posi, ખાલી, !lax);
    જો (s1 == ખાલી) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      પાછા 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  પાછા 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
સ્થિર પૂર્ણાંક codepoint (lua_State *L) {
  size_t len;
  અચલ અક્ષર *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  પૂર્ણાંક lax = lua_toboolean(L, 4);
  પૂર્ણાંક n;
  અચલ અક્ષર *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  જો (posi > pose) પાછા 0;  /* empty interval; return no values */
  જો (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    પાછા luaL_error(L, "string slice too long");
  n = (પૂર્ણાંક)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  માટે (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    જો (s == ખાલી)
      પાછા luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  પાછા n;
}


સ્થિર શૂન્ય pushutfchar (lua_State *L, પૂર્ણાંક arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
સ્થિર પૂર્ણાંક utfchar (lua_State *L) {
  પૂર્ણાંક n = lua_gettop(L);  /* number of arguments */
  જો (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  નહીંતર {
    પૂર્ણાંક i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    માટે (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  પાછા 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
સ્થિર પૂર્ણાંક byteoffset (lua_State *L) {
  size_t len;
  અચલ અક્ષર *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  જો (n == 0) {
    /* find beginning of current byte sequence */
    જ્યારે (posi > 0 && iscontp(s + posi)) posi--;
  }
  નહીંતર {
    જો (iscontp(s + posi))
      પાછા luaL_error(L, "initial position is a continuation byte");
    જો (n < 0) {
      જ્યારે (n < 0 && posi > 0) {  /* move back */
        કરો {  /* find beginning of previous character */
          posi--;
        } જ્યારે (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    નહીંતર {
      n--;  /* do not move for 1st character */
      જ્યારે (n > 0 && posi < (lua_Integer)len) {
        કરો {  /* find beginning of next character */
          posi++;
        } જ્યારે (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  જો (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    પાછા 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  જો ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    જો (iscont(s[posi]))
      પાછા luaL_error(L, "initial position is a continuation byte");
    જ્યારે (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  પાછા 2;
}


સ્થિર પૂર્ણાંક iter_aux (lua_State *L, પૂર્ણાંક strict) {
  size_t len;
  અચલ અક્ષર *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  જો (n < len) {
    જ્યારે (iscontp(s + n)) n++;  /* go to next character */
  }
  જો (n >= len)  /* (also handles original 'n' being negative) */
    પાછા 0;  /* no more codepoints */
  નહીંતર {
    l_uint32 code;
    અચલ અક્ષર *next = utf8_decode(s + n, &code, strict);
    જો (next == ખાલી || iscontp(next))
      પાછા luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    પાછા 2;
  }
}


સ્થિર પૂર્ણાંક iter_auxstrict (lua_State *L) {
  પાછા iter_aux(L, 1);
}

સ્થિર પૂર્ણાંક iter_auxlax (lua_State *L) {
  પાછા iter_aux(L, 0);
}


સ્થિર પૂર્ણાંક iter_codes (lua_State *L) {
  પૂર્ણાંક lax = lua_toboolean(L, 2);
  અચલ અક્ષર *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  પાછા 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


સ્થિર અચલ luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ખાલી},
  {ખાલી, ખાલી}
};


LUAMOD_API પૂર્ણાંક luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, કદ(UTF8PATT)/કદ(અક્ષર) - 1);
  lua_setfield(L, -2, "charpattern");
  પાછા 1;
}

