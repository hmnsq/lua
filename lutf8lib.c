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
  ถ้า (pos >= 0) คืน pos;
  มิฉะนั้น ถ้า (0u - (size_t)pos > len) คืน 0;
  มิฉะนั้น คืน (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
static const var_na *utf8_decode (const var_na *s, l_uint32 *val, จำนวนเต็ม strict) {
  static const l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned จำนวนเต็ม c = (unsigned var_na)s[0];
  l_uint32 res = 0;  /* final result */
  ถ้า (c < 0x80)  /* ASCII? */
    res = c;
  มิฉะนั้น ถ้า (c >= 0xfe)  /* c >= 1111 1110b ? */
    คืน NULL;  /* would need six or more continuation bytes */
  มิฉะนั้น {
    จำนวนเต็ม count = 0;  /* to count number of continuation bytes */
    สำหรับ (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned จำนวนเต็ม cc = (unsigned var_na)s[++count];  /* read next byte */
      ถ้า (!iscont(cc))  /* not a continuation byte? */
        คืน NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(count <= 5);
    res |= ((l_uint32)(c & 0x7F) << (count * 5));  /* add first byte */
    ถ้า (res > MAXUTF || res < limits[count])
      คืน NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  ถ้า (strict) {
    /* check for invalid code points; too large or surrogates */
    ถ้า (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      คืน NULL;
  }
  ถ้า (val) *val = res;
  คืน s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static จำนวนเต็ม utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  จำนวนเต็ม lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  ขณะ (posi <= posj) {
    const var_na *s1 = utf8_decode(s + posi, NULL, !lax);
    ถ้า (s1 == NULL) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      คืน 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  คืน 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static จำนวนเต็ม codepoint (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  จำนวนเต็ม lax = lua_toboolean(L, 4);
  จำนวนเต็ม n;
  const var_na *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  ถ้า (posi > pose) คืน 0;  /* empty interval; return no values */
  ถ้า (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    คืน luaL_error(L, "string slice too long");
  n = (จำนวนเต็ม)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  สำหรับ (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    ถ้า (s == NULL)
      คืน luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  คืน n;
}


static void pushutfchar (lua_State *L, จำนวนเต็ม arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static จำนวนเต็ม utfchar (lua_State *L) {
  จำนวนเต็ม n = lua_gettop(L);  /* number of arguments */
  ถ้า (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  มิฉะนั้น {
    จำนวนเต็ม i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    สำหรับ (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  คืน 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
static จำนวนเต็ม byteoffset (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  ถ้า (n == 0) {
    /* find beginning of current byte sequence */
    ขณะ (posi > 0 && iscontp(s + posi)) posi--;
  }
  มิฉะนั้น {
    ถ้า (iscontp(s + posi))
      คืน luaL_error(L, "initial position is a continuation byte");
    ถ้า (n < 0) {
      ขณะ (n < 0 && posi > 0) {  /* move back */
        do {  /* find beginning of previous character */
          posi--;
        } ขณะ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    มิฉะนั้น {
      n--;  /* do not move for 1st character */
      ขณะ (n > 0 && posi < (lua_Integer)len) {
        do {  /* find beginning of next character */
          posi++;
        } ขณะ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  ถ้า (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    คืน 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  ถ้า ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    ถ้า (iscont(s[posi]))
      คืน luaL_error(L, "initial position is a continuation byte");
    ขณะ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  คืน 2;
}


static จำนวนเต็ม iter_aux (lua_State *L, จำนวนเต็ม strict) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  ถ้า (n < len) {
    ขณะ (iscontp(s + n)) n++;  /* go to next character */
  }
  ถ้า (n >= len)  /* (also handles original 'n' being negative) */
    คืน 0;  /* no more codepoints */
  มิฉะนั้น {
    l_uint32 code;
    const var_na *next = utf8_decode(s + n, &code, strict);
    ถ้า (next == NULL || iscontp(next))
      คืน luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    คืน 2;
  }
}


static จำนวนเต็ม iter_auxstrict (lua_State *L) {
  คืน iter_aux(L, 1);
}

static จำนวนเต็ม iter_auxlax (lua_State *L) {
  คืน iter_aux(L, 0);
}


static จำนวนเต็ม iter_codes (lua_State *L) {
  จำนวนเต็ม lax = lua_toboolean(L, 2);
  const var_na *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  คืน 3;
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


LUAMOD_API จำนวนเต็ม luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, parimaa_na(UTF8PATT)/parimaa_na(var_na) - 1);
  lua_setfield(L, -2, "charpattern");
  คืน 1;
}

