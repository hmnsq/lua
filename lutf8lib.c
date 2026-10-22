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
  만약 (pos >= 0) 반환 pos;
  아니면 만약 (0u - (size_t)pos > len) 반환 0;
  아니면 반환 (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
static const char *utf8_decode (const char *s, l_uint32 *val, 정수 strict) {
  static const l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned 정수 c = (unsigned char)s[0];
  l_uint32 res = 0;  /* final result */
  만약 (c < 0x80)  /* ASCII? */
    res = c;
  아니면 만약 (c >= 0xfe)  /* c >= 1111 1110b ? */
    반환 NULL;  /* would need six or more continuation bytes */
  아니면 {
    정수 count = 0;  /* to count number of continuation bytes */
    위해 (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned 정수 cc = (unsigned char)s[++count];  /* read next byte */
      만약 (!iscont(cc))  /* not a continuation byte? */
        반환 NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(count <= 5);
    res |= ((l_uint32)(c & 0x7F) << (count * 5));  /* add first byte */
    만약 (res > MAXUTF || res < limits[count])
      반환 NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  만약 (strict) {
    /* check for invalid code points; too large or surrogates */
    만약 (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      반환 NULL;
  }
  만약 (val) *val = res;
  반환 s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static 정수 utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  정수 lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  동안 (posi <= posj) {
    const char *s1 = utf8_decode(s + posi, NULL, !lax);
    만약 (s1 == NULL) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      반환 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  반환 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static 정수 codepoint (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  정수 lax = lua_toboolean(L, 4);
  정수 n;
  const char *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  만약 (posi > pose) 반환 0;  /* empty interval; return no values */
  만약 (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    반환 luaL_error(L, "string slice too long");
  n = (정수)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  위해 (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    만약 (s == NULL)
      반환 luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  반환 n;
}


static void pushutfchar (lua_State *L, 정수 arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static 정수 utfchar (lua_State *L) {
  정수 n = lua_gettop(L);  /* number of arguments */
  만약 (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  아니면 {
    정수 i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    위해 (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  반환 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
static 정수 byteoffset (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  만약 (n == 0) {
    /* find beginning of current byte sequence */
    동안 (posi > 0 && iscontp(s + posi)) posi--;
  }
  아니면 {
    만약 (iscontp(s + posi))
      반환 luaL_error(L, "initial position is a continuation byte");
    만약 (n < 0) {
      동안 (n < 0 && posi > 0) {  /* move back */
        do {  /* find beginning of previous character */
          posi--;
        } 동안 (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    아니면 {
      n--;  /* do not move for 1st character */
      동안 (n > 0 && posi < (lua_Integer)len) {
        do {  /* find beginning of next character */
          posi++;
        } 동안 (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  만약 (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    반환 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  만약 ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    만약 (iscont(s[posi]))
      반환 luaL_error(L, "initial position is a continuation byte");
    동안 (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  반환 2;
}


static 정수 iter_aux (lua_State *L, 정수 strict) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  만약 (n < len) {
    동안 (iscontp(s + n)) n++;  /* go to next character */
  }
  만약 (n >= len)  /* (also handles original 'n' being negative) */
    반환 0;  /* no more codepoints */
  아니면 {
    l_uint32 code;
    const char *next = utf8_decode(s + n, &code, strict);
    만약 (next == NULL || iscontp(next))
      반환 luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    반환 2;
  }
}


static 정수 iter_auxstrict (lua_State *L) {
  반환 iter_aux(L, 1);
}

static 정수 iter_auxlax (lua_State *L) {
  반환 iter_aux(L, 0);
}


static 정수 iter_codes (lua_State *L) {
  정수 lax = lua_toboolean(L, 2);
  const char *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  반환 3;
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


LUAMOD_API 정수 luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, sizeof(UTF8PATT)/sizeof(char) - 1);
  lua_setfield(L, -2, "charpattern");
  반환 1;
}

