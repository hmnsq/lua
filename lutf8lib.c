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
  если (pos >= 0) вернуть pos;
  иначе если (0u - (size_t)pos > len) вернуть 0;
  иначе вернуть (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
static const char *utf8_decode (const char *s, l_uint32 *val, целое strict) {
  static const l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned целое c = (unsigned char)s[0];
  l_uint32 res = 0;  /* final result */
  если (c < 0x80)  /* ASCII? */
    res = c;
  иначе если (c >= 0xfe)  /* c >= 1111 1110b ? */
    вернуть NULL;  /* would need six or more continuation bytes */
  иначе {
    целое count = 0;  /* to count number of continuation bytes */
    для (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned целое cc = (unsigned char)s[++count];  /* read next byte */
      если (!iscont(cc))  /* not a continuation byte? */
        вернуть NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(count <= 5);
    res |= ((l_uint32)(c & 0x7F) << (count * 5));  /* add first byte */
    если (res > MAXUTF || res < limits[count])
      вернуть NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  если (strict) {
    /* check for invalid code points; too large or surrogates */
    если (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      вернуть NULL;
  }
  если (val) *val = res;
  вернуть s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static целое utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  целое lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  пока (posi <= posj) {
    const char *s1 = utf8_decode(s + posi, NULL, !lax);
    если (s1 == NULL) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      вернуть 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  вернуть 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static целое codepoint (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  целое lax = lua_toboolean(L, 4);
  целое n;
  const char *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  если (posi > pose) вернуть 0;  /* empty interval; return no values */
  если (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    вернуть luaL_error(L, "string slice too long");
  n = (целое)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  для (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    если (s == NULL)
      вернуть luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  вернуть n;
}


static void pushutfchar (lua_State *L, целое arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static целое utfchar (lua_State *L) {
  целое n = lua_gettop(L);  /* number of arguments */
  если (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  иначе {
    целое i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    для (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  вернуть 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
static целое byteoffset (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  если (n == 0) {
    /* find beginning of current byte sequence */
    пока (posi > 0 && iscontp(s + posi)) posi--;
  }
  иначе {
    если (iscontp(s + posi))
      вернуть luaL_error(L, "initial position is a continuation byte");
    если (n < 0) {
      пока (n < 0 && posi > 0) {  /* move back */
        do {  /* find beginning of previous character */
          posi--;
        } пока (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    иначе {
      n--;  /* do not move for 1st character */
      пока (n > 0 && posi < (lua_Integer)len) {
        do {  /* find beginning of next character */
          posi++;
        } пока (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  если (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    вернуть 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  если ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    если (iscont(s[posi]))
      вернуть luaL_error(L, "initial position is a continuation byte");
    пока (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  вернуть 2;
}


static целое iter_aux (lua_State *L, целое strict) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  если (n < len) {
    пока (iscontp(s + n)) n++;  /* go to next character */
  }
  если (n >= len)  /* (also handles original 'n' being negative) */
    вернуть 0;  /* no more codepoints */
  иначе {
    l_uint32 code;
    const char *next = utf8_decode(s + n, &code, strict);
    если (next == NULL || iscontp(next))
      вернуть luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    вернуть 2;
  }
}


static целое iter_auxstrict (lua_State *L) {
  вернуть iter_aux(L, 1);
}

static целое iter_auxlax (lua_State *L) {
  вернуть iter_aux(L, 0);
}


static целое iter_codes (lua_State *L) {
  целое lax = lua_toboolean(L, 2);
  const char *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  вернуть 3;
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


LUAMOD_API целое luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, sizeof(UTF8PATT)/sizeof(char) - 1);
  lua_setfield(L, -2, "charpattern");
  вернуть 1;
}

