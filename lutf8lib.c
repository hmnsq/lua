/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#define lutf8lib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


#define MAXUNICODE	0x10FFFFu

#define MAXUTF		0x7FFFFFFFu


#define MSGInvalid	"invalid UTF-8 code"


#define iscont(c)	(((c) & 0xC0) == 0x80)
#define iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
स्थिर lua_Integer u_posrelat (lua_Integer pos, माप_प्रकार len) {
  जर (pos >= 0) परत pos;
  अन्यथा जर (0u - (माप_प्रकार)pos > len) परत 0;
  अन्यथा परत (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
स्थिर स्थिरांक अक्षर *utf8_decode (स्थिरांक अक्षर *s, l_uint32 *val, पूर्णांक strict) {
  स्थिर स्थिरांक l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  अचिन्हित पूर्णांक c = (अचिन्हित अक्षर)s[0];
  l_uint32 res = 0;  /* final result */
  जर (c < 0x80)  /* ASCII? */
    res = c;
  अन्यथा जर (c >= 0xfe)  /* c >= 1111 1110b ? */
    परत रिक्त;  /* would need six or more continuation bytes */
  अन्यथा {
    पूर्णांक मोजणी = 0;  /* to count number of continuation bytes */
    साठी (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      अचिन्हित पूर्णांक cc = (अचिन्हित अक्षर)s[++मोजणी];  /* read next byte */
      जर (!iscont(cc))  /* not a continuation byte? */
        परत रिक्त;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(मोजणी <= 5);
    res |= ((l_uint32)(c & 0x7F) << (मोजणी * 5));  /* add first byte */
    जर (res > MAXUTF || res < limits[मोजणी])
      परत रिक्त;  /* invalid byte sequence */
    s += मोजणी;  /* skip continuation bytes read */
  }
  जर (strict) {
    /* check for invalid code points; too large or surrogates */
    जर (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      परत रिक्त;
  }
  जर (val) *val = res;
  परत s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
स्थिर पूर्णांक utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  माप_प्रकार len;  /* string length in bytes */
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  पूर्णांक lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  पर्यंत (posi <= posj) {
    स्थिरांक अक्षर *s1 = utf8_decode(s + posi, रिक्त, !lax);
    जर (s1 == रिक्त) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      परत 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  परत 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
स्थिर पूर्णांक codepoint (lua_State *L) {
  माप_प्रकार len;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  पूर्णांक lax = lua_toboolean(L, 4);
  पूर्णांक n;
  स्थिरांक अक्षर *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  जर (posi > pose) परत 0;  /* empty interval; return no values */
  जर (pose - posi >= पूर्णांक_उच्च)  /* (lua_Integer -> int) overflow? */
    परत luaL_error(L, "string slice too long");
  n = (पूर्णांक)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  साठी (s += posi - 1; s < se;) {
    l_uint32 code;
    s = utf8_decode(s, &code, !lax);
    जर (s == रिक्त)
      परत luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(code));
    n++;
  }
  परत n;
}


स्थिर शून्य pushutfchar (lua_State *L, पूर्णांक arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, code <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(अचिन्हित दीर्घ, code));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
स्थिर पूर्णांक utfchar (lua_State *L) {
  पूर्णांक n = lua_gettop(L);  /* number of arguments */
  जर (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  अन्यथा {
    पूर्णांक i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    साठी (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  परत 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
स्थिर पूर्णांक byteoffset (lua_State *L) {
  माप_प्रकार len;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  जर (n == 0) {
    /* find beginning of current byte sequence */
    पर्यंत (posi > 0 && iscontp(s + posi)) posi--;
  }
  अन्यथा {
    जर (iscontp(s + posi))
      परत luaL_error(L, "initial position is a continuation byte");
    जर (n < 0) {
      पर्यंत (n < 0 && posi > 0) {  /* move back */
        करा {  /* find beginning of previous character */
          posi--;
        } पर्यंत (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    अन्यथा {
      n--;  /* do not move for 1st character */
      पर्यंत (n > 0 && posi < (lua_Integer)len) {
        करा {  /* find beginning of next character */
          posi++;
        } पर्यंत (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  जर (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    परत 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  जर ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    जर (iscont(s[posi]))
      परत luaL_error(L, "initial position is a continuation byte");
    पर्यंत (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  परत 2;
}


स्थिर पूर्णांक iter_aux (lua_State *L, पूर्णांक strict) {
  माप_प्रकार len;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  जर (n < len) {
    पर्यंत (iscontp(s + n)) n++;  /* go to next character */
  }
  जर (n >= len)  /* (also handles original 'n' being negative) */
    परत 0;  /* no more codepoints */
  अन्यथा {
    l_uint32 code;
    स्थिरांक अक्षर *next = utf8_decode(s + n, &code, strict);
    जर (next == रिक्त || iscontp(next))
      परत luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(code));
    परत 2;
  }
}


स्थिर पूर्णांक iter_auxstrict (lua_State *L) {
  परत iter_aux(L, 1);
}

स्थिर पूर्णांक iter_auxlax (lua_State *L) {
  परत iter_aux(L, 0);
}


स्थिर पूर्णांक iter_codes (lua_State *L) {
  पूर्णांक lax = lua_toboolean(L, 2);
  स्थिरांक अक्षर *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  परत 3;
}


/* pattern to match a single UTF-8 character */
#define UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


स्थिर स्थिरांक luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", रिक्त},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णांक luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, आकार(UTF8PATT)/आकार(अक्षर) - 1);
  lua_setfield(L, -2, "charpattern");
  परत 1;
}

