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
  အကယ်၍ (pos >= 0) ပြန်ပေး pos;
  မဟုတ်ပါက အကယ်၍ (0u - (size_t)pos > len) ပြန်ပေး 0;
  မဟုတ်ပါက ပြန်ပေး (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
static const var_na *utf8_decode (const var_na *s, l_uint32 *val, ကိန်းပြည့် strict) {
  static const l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned ကိန်းပြည့် c = (unsigned var_na)s[0];
  l_uint32 res = 0;  /* final result */
  အကယ်၍ (c < 0x80)  /* ASCII? */
    res = c;
  မဟုတ်ပါက အကယ်၍ (c >= 0xfe)  /* c >= 1111 1110b ? */
    ပြန်ပေး NULL;  /* would need six or more continuation bytes */
  မဟုတ်ပါက {
    ကိန်းပြည့် count = 0;  /* to count number of continuation bytes */
    အတွက် (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned ကိန်းပြည့် cc = (unsigned var_na)s[++count];  /* read next byte */
      အကယ်၍ (!iscont(cc))  /* not a continuation byte? */
        ပြန်ပေး NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(count <= 5);
    res |= ((l_uint32)(c & 0x7F) << (count * 5));  /* add first byte */
    အကယ်၍ (res > MAXUTF || res < limits[count])
      ပြန်ပေး NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  အကယ်၍ (strict) {
    /* check for invalid code points; too large or surrogates */
    အကယ်၍ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ပြန်ပေး NULL;
  }
  အကယ်၍ (val) *val = res;
  ပြန်ပေး s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static ကိန်းပြည့် utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  ကိန်းပြည့် lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  နေစဉ် (posi <= posj) {
    const var_na *s1 = utf8_decode(s + posi, NULL, !lax);
    အကယ်၍ (s1 == NULL) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ပြန်ပေး 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ပြန်ပေး 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static ကိန်းပြည့် codepoint (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  ကိန်းပြည့် lax = lua_toboolean(L, 4);
  ကိန်းပြည့် n;
  const var_na *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  အကယ်၍ (posi > pose) ပြန်ပေး 0;  /* empty interval; return no values */
  အကယ်၍ (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    ပြန်ပေး luaL_error(L, "string slice too long");
  n = (ကိန်းပြည့်)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  အတွက် (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    အကယ်၍ (s == NULL)
      ပြန်ပေး luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  ပြန်ပေး n;
}


static void pushutfchar (lua_State *L, ကိန်းပြည့် arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static ကိန်းပြည့် utfchar (lua_State *L) {
  ကိန်းပြည့် n = lua_gettop(L);  /* number of arguments */
  အကယ်၍ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  မဟုတ်ပါက {
    ကိန်းပြည့် i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    အတွက် (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ပြန်ပေး 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
static ကိန်းပြည့် byteoffset (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  အကယ်၍ (n == 0) {
    /* find beginning of current byte sequence */
    နေစဉ် (posi > 0 && iscontp(s + posi)) posi--;
  }
  မဟုတ်ပါက {
    အကယ်၍ (iscontp(s + posi))
      ပြန်ပေး luaL_error(L, "initial position is a continuation byte");
    အကယ်၍ (n < 0) {
      နေစဉ် (n < 0 && posi > 0) {  /* move back */
        do {  /* find beginning of previous character */
          posi--;
        } နေစဉ် (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    မဟုတ်ပါက {
      n--;  /* do not move for 1st character */
      နေစဉ် (n > 0 && posi < (lua_Integer)len) {
        do {  /* find beginning of next character */
          posi++;
        } နေစဉ် (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  အကယ်၍ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ပြန်ပေး 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  အကယ်၍ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    အကယ်၍ (iscont(s[posi]))
      ပြန်ပေး luaL_error(L, "initial position is a continuation byte");
    နေစဉ် (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ပြန်ပေး 2;
}


static ကိန်းပြည့် iter_aux (lua_State *L, ကိန်းပြည့် strict) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  အကယ်၍ (n < len) {
    နေစဉ် (iscontp(s + n)) n++;  /* go to next character */
  }
  အကယ်၍ (n >= len)  /* (also handles original 'n' being negative) */
    ပြန်ပေး 0;  /* no more codepoints */
  မဟုတ်ပါက {
    l_uint32 code;
    const var_na *next = utf8_decode(s + n, &code, strict);
    အကယ်၍ (next == NULL || iscontp(next))
      ပြန်ပေး luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    ပြန်ပေး 2;
  }
}


static ကိန်းပြည့် iter_auxstrict (lua_State *L) {
  ပြန်ပေး iter_aux(L, 1);
}

static ကိန်းပြည့် iter_auxlax (lua_State *L) {
  ပြန်ပေး iter_aux(L, 0);
}


static ကိန်းပြည့် iter_codes (lua_State *L) {
  ကိန်းပြည့် lax = lua_toboolean(L, 2);
  const var_na *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ပြန်ပေး 3;
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


LUAMOD_API ကိန်းပြည့် luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, parimaa_na(UTF8PATT)/parimaa_na(var_na) - 1);
  lua_setfield(L, -2, "charpattern");
  ပြန်ပေး 1;
}

