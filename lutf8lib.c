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
  නම් (pos >= 0) ආපසු pos;
  නැතහොත් නම් (0u - (size_t)pos > len) ආපසු 0;
  නැතහොත් ආපසු (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
static const var_na *utf8_decode (const var_na *s, l_uint32 *val, පූර්ණ_සංඛ්‍යා strict) {
  static const l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned පූර්ණ_සංඛ්‍යා c = (unsigned var_na)s[0];
  l_uint32 res = 0;  /* final result */
  නම් (c < 0x80)  /* ASCII? */
    res = c;
  නැතහොත් නම් (c >= 0xfe)  /* c >= 1111 1110b ? */
    ආපසු NULL;  /* would need six or more continuation bytes */
  නැතහොත් {
    පූර්ණ_සංඛ්‍යා count = 0;  /* to count number of continuation bytes */
    සඳහා (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned පූර්ණ_සංඛ්‍යා cc = (unsigned var_na)s[++count];  /* read next byte */
      නම් (!iscont(cc))  /* not a continuation byte? */
        ආපසු NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(count <= 5);
    res |= ((l_uint32)(c & 0x7F) << (count * 5));  /* add first byte */
    නම් (res > MAXUTF || res < limits[count])
      ආපසු NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  නම් (strict) {
    /* check for invalid code points; too large or surrogates */
    නම් (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ආපසු NULL;
  }
  නම් (val) *val = res;
  ආපසු s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static පූර්ණ_සංඛ්‍යා utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  පූර්ණ_සංඛ්‍යා lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  අතරතුර (posi <= posj) {
    const var_na *s1 = utf8_decode(s + posi, NULL, !lax);
    නම් (s1 == NULL) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ආපසු 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ආපසු 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static පූර්ණ_සංඛ්‍යා codepoint (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  පූර්ණ_සංඛ්‍යා lax = lua_toboolean(L, 4);
  පූර්ණ_සංඛ්‍යා n;
  const var_na *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  නම් (posi > pose) ආපසු 0;  /* empty interval; return no values */
  නම් (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    ආපසු luaL_error(L, "string slice too long");
  n = (පූර්ණ_සංඛ්‍යා)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  සඳහා (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    නම් (s == NULL)
      ආපසු luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  ආපසු n;
}


static void pushutfchar (lua_State *L, පූර්ණ_සංඛ්‍යා arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static පූර්ණ_සංඛ්‍යා utfchar (lua_State *L) {
  පූර්ණ_සංඛ්‍යා n = lua_gettop(L);  /* number of arguments */
  නම් (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  නැතහොත් {
    පූර්ණ_සංඛ්‍යා i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    සඳහා (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ආපසු 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
static පූර්ණ_සංඛ්‍යා byteoffset (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  නම් (n == 0) {
    /* find beginning of current byte sequence */
    අතරතුර (posi > 0 && iscontp(s + posi)) posi--;
  }
  නැතහොත් {
    නම් (iscontp(s + posi))
      ආපසු luaL_error(L, "initial position is a continuation byte");
    නම් (n < 0) {
      අතරතුර (n < 0 && posi > 0) {  /* move back */
        do {  /* find beginning of previous character */
          posi--;
        } අතරතුර (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    නැතහොත් {
      n--;  /* do not move for 1st character */
      අතරතුර (n > 0 && posi < (lua_Integer)len) {
        do {  /* find beginning of next character */
          posi++;
        } අතරතුර (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  නම් (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ආපසු 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  නම් ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    නම් (iscont(s[posi]))
      ආපසු luaL_error(L, "initial position is a continuation byte");
    අතරතුර (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ආපසු 2;
}


static පූර්ණ_සංඛ්‍යා iter_aux (lua_State *L, පූර්ණ_සංඛ්‍යා strict) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  නම් (n < len) {
    අතරතුර (iscontp(s + n)) n++;  /* go to next character */
  }
  නම් (n >= len)  /* (also handles original 'n' being negative) */
    ආපසු 0;  /* no more codepoints */
  නැතහොත් {
    l_uint32 code;
    const var_na *next = utf8_decode(s + n, &code, strict);
    නම් (next == NULL || iscontp(next))
      ආපසු luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    ආපසු 2;
  }
}


static පූර්ණ_සංඛ්‍යා iter_auxstrict (lua_State *L) {
  ආපසු iter_aux(L, 1);
}

static පූර්ණ_සංඛ්‍යා iter_auxlax (lua_State *L) {
  ආපසු iter_aux(L, 0);
}


static පූර්ණ_සංඛ්‍යා iter_codes (lua_State *L) {
  පූර්ණ_සංඛ්‍යා lax = lua_toboolean(L, 2);
  const var_na *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ආපසු 3;
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


LUAMOD_API පූර්ණ_සංඛ්‍යා luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, parimaa_na(UTF8PATT)/parimaa_na(var_na) - 1);
  lua_setfield(L, -2, "charpattern");
  ආපසු 1;
}

