/*
** $Id: lutf8lib.c $
** Standard library for UTF-8 manipulation
** See Copyright Notice in lua.h
*/

#నిర్వచించు lutf8lib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


#నిర్వచించు MAXUNICODE	0x10FFFFu

#నిర్వచించు MAXUTF		0x7FFFFFFFu


#నిర్వచించు MSGInvalid	"invalid UTF-8 code"


#నిర్వచించు iscont(c)	(((c) & 0xC0) == 0x80)
#నిర్వచించు iscontp(p)	iscont(*(p))


/* from strlib */
/* translate a relative string position: negative means back from end */
స్థిర lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  ఒకవేళ (pos >= 0) తిరిగి pos;
  లేకపోతే ఒకవేళ (0u - (size_t)pos > len) తిరిగి 0;
  లేకపోతే తిరిగి (lua_Integer)len + pos + 1;
}


/*
** Decode one UTF-8 sequence, returning NULL if byte sequence is
** invalid.  The array 'limits' stores the minimum value for each
** sequence length, to check for overlong representations. Its first
** entry forces an error for non-ASCII bytes with no continuation
** bytes (count == 0).
*/
స్థిర స్థిరరాశి అక్షరం *utf8_decode (స్థిరరాశి అక్షరం *s, l_uint32 *val, పూర్ణాంకం strict) {
  స్థిర స్థిరరాశి l_uint32 limits[] =
        {~(l_uint32)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  గుర్తులేని పూర్ణాంకం c = (గుర్తులేని అక్షరం)s[0];
  l_uint32 res = 0;  /* final result */
  ఒకవేళ (c < 0x80)  /* ASCII? */
    res = c;
  లేకపోతే ఒకవేళ (c >= 0xfe)  /* c >= 1111 1110b ? */
    తిరిగి ఖాళీ;  /* would need six or more continuation bytes */
  లేకపోతే {
    పూర్ణాంకం లెక్క = 0;  /* to count number of continuation bytes */
    కొరకు (; c & 0x40; c <<= 1) {  /* while it needs continuation bytes... */
      గుర్తులేని పూర్ణాంకం cc = (గుర్తులేని అక్షరం)s[++లెక్క];  /* read next byte */
      ఒకవేళ (!iscont(cc))  /* not a continuation byte? */
        తిరిగి ఖాళీ;  /* invalid byte sequence */
      res = (res << 6) | (cc & 0x3F);  /* add lower 6 bits from cont. byte */
    }
    lua_assert(లెక్క <= 5);
    res |= ((l_uint32)(c & 0x7F) << (లెక్క * 5));  /* add first byte */
    ఒకవేళ (res > MAXUTF || res < limits[లెక్క])
      తిరిగి ఖాళీ;  /* invalid byte sequence */
    s += లెక్క;  /* skip continuation bytes read */
  }
  ఒకవేళ (strict) {
    /* check for invalid code points; too large or surrogates */
    ఒకవేళ (res > MAXUNICODE || (0xD800u <= res && res <= 0xDFFFu))
      తిరిగి ఖాళీ;
  }
  ఒకవేళ (val) *val = res;
  తిరిగి s + 1;  /* +1 to include first byte */
}


/*
** utf8len(s [, i [, j [, lax]]]) --> number of characters that
** start in the range [i,j], or nil + current position if 's' is not
** well formed in that interval
*/
స్థిర పూర్ణాంకం utflen (lua_State *L) {
  lua_Integer n = 0;  /* counter for the number of characters */
  size_t len;  /* string length in bytes */
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  పూర్ణాంకం lax = lua_toboolean(L, 4);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 2,
                   "initial position out of bounds");
  luaL_argcheck(L, --posj < (lua_Integer)len, 3,
                   "final position out of bounds");
  అయితే (posi <= posj) {
    స్థిరరాశి అక్షరం *s1 = utf8_decode(s + posi, ఖాళీ, !lax);
    ఒకవేళ (s1 == ఖాళీ) {  /* conversion error? */
      luaL_pushfail(L);  /* return fail ... */
      lua_pushinteger(L, posi + 1);  /* ... and current position */
      తిరిగి 2;
    }
    posi = ct_diff2S(s1 - s);
    n++;
  }
  lua_pushinteger(L, n);
  తిరిగి 1;
}


/*
** codepoint(s, [i, [j [, lax]]]) -> returns codepoints for all
** characters that start in the range [i,j]
*/
స్థిర పూర్ణాంకం codepoint (lua_State *L) {
  size_t len;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  పూర్ణాంకం lax = lua_toboolean(L, 4);
  పూర్ణాంకం n;
  స్థిరరాశి అక్షరం *se;
  luaL_argcheck(L, posi >= 1, 2, "out of bounds");
  luaL_argcheck(L, pose <= (lua_Integer)len, 3, "out of bounds");
  ఒకవేళ (posi > pose) తిరిగి 0;  /* empty interval; return no values */
  ఒకవేళ (pose - posi >= గరిష్ఠ_పూర్ణాంకం)  /* (lua_Integer -> int) overflow? */
    తిరిగి luaL_error(L, "string slice too long");
  n = (పూర్ణాంకం)(pose -  posi) + 1;  /* upper bound for number of returns */
  luaL_checkstack(L, n, "string slice too long");
  n = 0;  /* count the number of returns */
  se = s + pose;  /* string end */
  కొరకు (s += posi - 1; s < se;) {
    l_uint32 సంకేతం_2;
    s = utf8_decode(s, &సంకేతం_2, !lax);
    ఒకవేళ (s == ఖాళీ)
      తిరిగి luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(సంకేతం_2));
    n++;
  }
  తిరిగి n;
}


స్థిర శూన్య pushutfchar (lua_State *L, పూర్ణాంకం arg) {
  lua_Unsigned సంకేతం_2 = (lua_Unsigned)luaL_checkinteger(L, arg);
  luaL_argcheck(L, సంకేతం_2 <= MAXUTF, arg, "value out of range");
  lua_pushfstring(L, "%U", cast(గుర్తులేని దీర్ఘ, సంకేతం_2));
}


/*
** utfchar(n1, n2, ...)  -> char(n1)..char(n2)...
*/
స్థిర పూర్ణాంకం utfchar (lua_State *L) {
  పూర్ణాంకం n = lua_gettop(L);  /* number of arguments */
  ఒకవేళ (n == 1)  /* optimize common case of single char */
    pushutfchar(L, 1);
  లేకపోతే {
    పూర్ణాంకం i;
    lua_L_బఫరు b;
    luaL_buffinit(L, &b);
    కొరకు (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  తిరిగి 1;
}


/*
** offset(s, n, [i])  -> indices where n-th character counting from
**   position 'i' starts and ends; 0 means character at 'i'.
*/
స్థిర పూర్ణాంకం byteoffset (lua_State *L) {
  size_t len;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &len);
  lua_Integer n  = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : cast_st2S(len) + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  luaL_argcheck(L, 1 <= posi && --posi <= (lua_Integer)len, 3,
                   "position out of bounds");
  ఒకవేళ (n == 0) {
    /* find beginning of current byte sequence */
    అయితే (posi > 0 && iscontp(s + posi)) posi--;
  }
  లేకపోతే {
    ఒకవేళ (iscontp(s + posi))
      తిరిగి luaL_error(L, "initial position is a continuation byte");
    ఒకవేళ (n < 0) {
      అయితే (n < 0 && posi > 0) {  /* move back */
        చేయి {  /* find beginning of previous character */
          posi--;
        } అయితే (posi > 0 && iscontp(s + posi));
        n++;
      }
    }
    లేకపోతే {
      n--;  /* do not move for 1st character */
      అయితే (n > 0 && posi < (lua_Integer)len) {
        చేయి {  /* find beginning of next character */
          posi++;
        } అయితే (iscontp(s + posi));  /* (cannot pass final '\0') */
        n--;
      }
    }
  }
  ఒకవేళ (n != 0) {  /* did not find given character? */
    luaL_pushfail(L);
    తిరిగి 1;
  }
  lua_pushinteger(L, posi + 1);  /* initial position */
  ఒకవేళ ((s[posi] & 0x80) != 0) {  /* multi-byte character? */
    ఒకవేళ (iscont(s[posi]))
      తిరిగి luaL_error(L, "initial position is a continuation byte");
    అయితే (iscontp(s + posi + 1))
      posi++;  /* skip to last continuation byte */
  }
  /* else one-byte character: final position is the initial one */
  lua_pushinteger(L, posi + 1);  /* 'posi' now is the final position */
  తిరిగి 2;
}


స్థిర పూర్ణాంకం iter_aux (lua_State *L, పూర్ణాంకం strict) {
  size_t len;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointeger(L, 2);
  ఒకవేళ (n < len) {
    అయితే (iscontp(s + n)) n++;  /* go to next character */
  }
  ఒకవేళ (n >= len)  /* (also handles original 'n' being negative) */
    తిరిగి 0;  /* no more codepoints */
  లేకపోతే {
    l_uint32 సంకేతం_2;
    స్థిరరాశి అక్షరం *తదుపరి = utf8_decode(s + n, &సంకేతం_2, strict);
    ఒకవేళ (తదుపరి == ఖాళీ || iscontp(తదుపరి))
      తిరిగి luaL_error(L, MSGInvalid);
    lua_pushinteger(L, l_castU2S(n + 1));
    lua_pushinteger(L, l_castU2S(సంకేతం_2));
    తిరిగి 2;
  }
}


స్థిర పూర్ణాంకం iter_auxstrict (lua_State *L) {
  తిరిగి iter_aux(L, 1);
}

స్థిర పూర్ణాంకం iter_auxlax (lua_State *L) {
  తిరిగి iter_aux(L, 0);
}


స్థిర పూర్ణాంకం iter_codes (lua_State *L) {
  పూర్ణాంకం lax = lua_toboolean(L, 2);
  స్థిరరాశి అక్షరం *s = luaL_checkstring(L, 1);
  luaL_argcheck(L, !iscontp(s), 1, MSGInvalid);
  lua_pushcfunction(L, lax ? iter_auxlax : iter_auxstrict);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  తిరిగి 3;
}


/* pattern to match a single UTF-8 character */
#నిర్వచించు UTF8PATT	"[\0-\x7F\xC2-\xFD][\x80-\xBF]*"


స్థిర స్థిరరాశి luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},
  /* placeholders */
  {"charpattern", ఖాళీ},
  {ఖాళీ, ఖాళీ}
};


LUAMOD_API పూర్ణాంకం luaopen_utf8 (lua_State *L) {
  luaL_newlib(L, funcs);
  lua_pushlstring(L, UTF8PATT, పరిమాణం(UTF8PATT)/పరిమాణం(అక్షరం) - 1);
  lua_setfield(L, -2, "charpattern");
  తిరిగి 1;
}

