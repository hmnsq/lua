/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#include "lprefix.h"


#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
static lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  nếu (pos >= 0) trả_về pos;
  không_thì nếu (0u - (size_t)pos > len) trả_về 0;
  không_thì trả_về (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
static const char *utf8_decode (const char *s, l_uint32 *val, số_nguyên strict) {
  static const l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned số_nguyên c = (unsigned char)s[0];
  l_uint32 res = 0;  /* final result */
  nếu (c < 0x80)  /* ASCII? */
    res = c;
  không_thì nếu (c >= 0xfe)  /* c >= 1111 1110b ? */
    trả_về NULL;  /* would need six or more continuation bytes */
  không_thì {
    số_nguyên count = 0;  /* to count number of continuation bytes */
    cho (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned số_nguyên cc = (unsigned char)s[++count];  /* read next byte */
      nếu (!iscont(cc))  /* not a continuation byte? */
        trả_về NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(count <= 5);
    res |= ((l_uint32)(c & 0x7F) << (count * 5));  /* add first byte */
    nếu (res > MAXUTF || res < limits[count])
      trả_về NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  nếu (strict) {
    /* check for invalid code points; too large or surrogates */
    nếu (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      trả_về NULL;
  }
  nếu (val) *val = res;
  trả_về s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static số_nguyên utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  số_nguyên lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  trong_khi (posi <= posj) {
    const char *s1 = utf8_decode(s + posi, NULL, !lax);
    nếu (s1 == NULL) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      trả_về 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  trả_về 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static số_nguyên codepoint (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  số_nguyên lax = lua_toboolean(L, 4);
  số_nguyên n;
  const char *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  nếu (posi > pose) trả_về 0;  /* empty interval; return no values */
  nếu (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    trả_về luaL_error(L, "string slice too long");
  n = (số_nguyên)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  cho (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    nếu (s == NULL)
      trả_về luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  trả_về n;
}


static void pushutfchar (lua_State *L, số_nguyên arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static số_nguyên utfchar (lua_State *L) {
  số_nguyên n = lua_gettop(L);  /* number of arguments */
  nếu (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  không_thì {
    số_nguyên i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    cho (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  trả_về 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
static số_nguyên byteoffset (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  nếu (n == 0) {
    /* find beginning of current byte sequence */
    trong_khi (posi > 0 && iscontp(s + posi)) posi--;
  }
  không_thì {
    nếu (iscontp(s + posi))
      trả_về luaL_error(L, "initial position is a continuation byte");
    nếu (n < 0) {
      trong_khi (n < 0 && posi > 0) {  /* move back */
        do {  /* find beginning of previous character */
          posi--;
        } trong_khi (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    không_thì {
      n--;  /* do not move for 1st character */
      trong_khi (n > 0 && posi < (lua_Integer)len) {
        do {  /* find beginning of next character */
          posi++;
        } trong_khi (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  nếu (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    trả_về 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  nếu ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    nếu (iscont(s[posi]))
      trả_về luaL_error(L, "initial position is a continuation byte");
    trong_khi (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  trả_về 2;
}


static số_nguyên iter_aux (lua_State *L, số_nguyên strict) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  nếu (n < len) {
    trong_khi (iscontp(s + n)) n++;  /* go to next character */
  }
  nếu (n >= len)  /* (also handles original 'n' being negative) */
    trả_về 0;  /* no more codepoints */
  không_thì {
    l_uint32 code;
    const char *next = utf8_decode(s + n, &code, strict);
    nếu (next == NULL || iscontp(next))
      trả_về luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    trả_về 2;
  }
}


static số_nguyên iter_auxstrict (lua_State *L) {
  trả_về iter_aux(L, 1);
}

static số_nguyên iter_auxlax (lua_State *L) {
  trả_về iter_aux(L, 0);
}


static số_nguyên iter_codes (lua_State *L) {
  số_nguyên lax = lua_toboolean(L, 2);
  const char *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  trả_về 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


static const luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", NULL},
  {NULL, NULL}
};


LUAMOD_API số_nguyên luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, sizeof(UTF8PATT)/sizeof(char) - 1);
  lua_setfield(L, -2, "charpattern");
  trả_về 1;
}

