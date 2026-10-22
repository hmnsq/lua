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
  ຖ້າ (pos >= 0) ສົ່ງຄືນ pos;
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (0u - (size_t)pos > len) ສົ່ງຄືນ 0;
  ບໍ່ດັ່ງນັ້ນ ສົ່ງຄືນ (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
static const var_na *utf8_decode (const var_na *s, l_uint32 *val, ຈຳນວນເຕັມ strict) {
  static const l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned ຈຳນວນເຕັມ c = (unsigned var_na)s[0];
  l_uint32 res = 0;  /* final result */
  ຖ້າ (c < 0x80)  /* ASCII? */
    res = c;
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (c >= 0xfe)  /* c >= 1111 1110b ? */
    ສົ່ງຄືນ NULL;  /* would need six or more continuation bytes */
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ count = 0;  /* to count number of continuation bytes */
    ສຳລັບ (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      unsigned ຈຳນວນເຕັມ cc = (unsigned var_na)s[++count];  /* read next byte */
      ຖ້າ (!iscont(cc))  /* not a continuation byte? */
        ສົ່ງຄືນ NULL;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(count <= 5);
    res |= ((l_uint32)(c & 0x7F) << (count * 5));  /* add first byte */
    ຖ້າ (res > MAXUTF || res < limits[count])
      ສົ່ງຄືນ NULL;  /* invalid byte sequence */
    s += count;  /* skip continuation bytes read */
  }
  ຖ້າ (strict) {
    /* check for invalid code points; too large or surrogates */
    ຖ້າ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      ສົ່ງຄືນ NULL;
  }
  ຖ້າ (val) *val = res;
  ສົ່ງຄືນ s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
static ຈຳນວນເຕັມ utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  ຈຳນວນເຕັມ lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  ໃນຂະນະທີ່ (posi <= posj) {
    const var_na *s1 = utf8_decode(s + posi, NULL, !lax);
    ຖ້າ (s1 == NULL) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      ສົ່ງຄືນ 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  ສົ່ງຄືນ 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
static ຈຳນວນເຕັມ codepoint (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  ຈຳນວນເຕັມ lax = lua_toboolean(L, 4);
  ຈຳນວນເຕັມ n;
  const var_na *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  ຖ້າ (posi > pose) ສົ່ງຄືນ 0;  /* empty interval; return no values */
  ຖ້າ (pose - posi >= INT_MAX)  /* (lua_Integer -> int) overflow? */
    ສົ່ງຄືນ luaL_error(L, "string slice too long");
  n = (ຈຳນວນເຕັມ)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  ສຳລັບ (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    ຖ້າ (s == NULL)
      ສົ່ງຄືນ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  ສົ່ງຄືນ n;
}


static void pushutfchar (lua_State *L, ຈຳນວນເຕັມ arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(unsigned long, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
static ຈຳນວນເຕັມ utfchar (lua_State *L) {
  ຈຳນວນເຕັມ n = lua_gettop(L);  /* number of arguments */
  ຖ້າ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    ສຳລັບ (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  ສົ່ງຄືນ 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
static ຈຳນວນເຕັມ byteoffset (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  ຖ້າ (n == 0) {
    /* find beginning of current byte sequence */
    ໃນຂະນະທີ່ (posi > 0 && iscontp(s + posi)) posi--;
  }
  ບໍ່ດັ່ງນັ້ນ {
    ຖ້າ (iscontp(s + posi))
      ສົ່ງຄືນ luaL_error(L, "initial position is a continuation byte");
    ຖ້າ (n < 0) {
      ໃນຂະນະທີ່ (n < 0 && posi > 0) {  /* move back */
        do {  /* find beginning of previous character */
          posi--;
        } ໃນຂະນະທີ່ (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    ບໍ່ດັ່ງນັ້ນ {
      n--;  /* do not move for 1st character */
      ໃນຂະນະທີ່ (n > 0 && posi < (lua_Integer)len) {
        do {  /* find beginning of next character */
          posi++;
        } ໃນຂະນະທີ່ (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  ຖ້າ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    ສົ່ງຄືນ 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  ຖ້າ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    ຖ້າ (iscont(s[posi]))
      ສົ່ງຄືນ luaL_error(L, "initial position is a continuation byte");
    ໃນຂະນະທີ່ (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  ສົ່ງຄືນ 2;
}


static ຈຳນວນເຕັມ iter_aux (lua_State *L, ຈຳນວນເຕັມ strict) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  ຖ້າ (n < len) {
    ໃນຂະນະທີ່ (iscontp(s + n)) n++;  /* go to next character */
  }
  ຖ້າ (n >= len)  /* (also handles original 'n' being negative) */
    ສົ່ງຄືນ 0;  /* no more codepoints */
  ບໍ່ດັ່ງນັ້ນ {
    l_uint32 code;
    const var_na *next = utf8_decode(s + n, &code, strict);
    ຖ້າ (next == NULL || iscontp(next))
      ສົ່ງຄືນ luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    ສົ່ງຄືນ 2;
  }
}


static ຈຳນວນເຕັມ iter_auxstrict (lua_State *L) {
  ສົ່ງຄືນ iter_aux(L, 1);
}

static ຈຳນວນເຕັມ iter_auxlax (lua_State *L) {
  ສົ່ງຄືນ iter_aux(L, 0);
}


static ຈຳນວນເຕັມ iter_codes (lua_State *L) {
  ຈຳນວນເຕັມ lax = lua_toboolean(L, 2);
  const var_na *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  ສົ່ງຄືນ 3;
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


LUAMOD_API ຈຳນວນເຕັມ luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, parimaa_na(UTF8PATT)/parimaa_na(var_na) - 1);
  lua_setfield(L, -2, "charpattern");
  ສົ່ງຄືນ 1;
}

