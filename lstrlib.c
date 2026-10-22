/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <float.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <locale.h>
#অন্তর্ভাব <math.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#যদি !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


স্থির পূর্ণসংখ্যা str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  ফেরত 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
স্থির size_t posrelatI (lua_Integer pos, size_t len) {
  যদি (pos > 0)
    ফেরত (size_t)pos;
  অন্যথা যদি (pos == 0)
    ফেরত 1;
  অন্যথা যদি (pos < -(lua_Integer)len)  /* inverted comparison */
    ফেরত 1;  /* clip to 1 */
  অন্যথা ফেরত len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
স্থির size_t getendpos (lua_State *L, পূর্ণসংখ্যা arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  যদি (pos > (lua_Integer)len)
    ফেরত len;
  অন্যথা যদি (pos >= 0)
    ফেরত (size_t)pos;
  অন্যথা যদি (pos < -(lua_Integer)len)
    ফেরত 0;
  অন্যথা ফেরত len + (size_t)pos + 1;
}


স্থির পূর্ণসংখ্যা str_sub (lua_State *L) {
  size_t l;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  যদি (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  অন্যথা lua_pushliteral(L, "");
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা str_reverse (lua_State *L) {
  size_t l, i;
  lua_L_বাফার b;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &l);
  অক্ষর *p = luaL_buffinitsize(L, &b, l);
  জন্য (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা str_lower (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_বাফার b;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &l);
  অক্ষর *p = luaL_buffinitsize(L, &b, l);
  জন্য (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা str_upper (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_বাফার b;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &l);
  অক্ষর *p = luaL_buffinitsize(L, &b, l);
  জন্য (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ফেরত 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
স্থির পূর্ণসংখ্যা str_rep (lua_State *L) {
  size_t len, lsep;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ধ্রুবক অক্ষর *sep = luaL_optlstring(L, 3, "", &lsep);
  যদি (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  অন্যথা যদি (l_unlikely(len > MAX_আকার - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_আকার) / n))
    ফেরত luaL_error(L, "resulting string too large");
  অন্যথা {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    lua_L_বাফার b;
    অক্ষর *p = luaL_buffinitsize(L, &b, totallen);
    যখন (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * আকার(অক্ষর)); p += len;
      যদি (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * আকার(অক্ষর)); p += lsep;
      }
    }
    memcpy(p, s, len * আকার(অক্ষর));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা str_byte (lua_State *L) {
  size_t l;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  পূর্ণসংখ্যা n, i;
  যদি (posi > pose) ফেরত 0;  /* empty interval; return no values */
  যদি (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    ফেরত luaL_error(L, "string slice too long");
  n = (পূর্ণসংখ্যা)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  জন্য (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  ফেরত n;
}


স্থির পূর্ণসংখ্যা str_char (lua_State *L) {
  পূর্ণসংখ্যা n = lua_gettop(L);  /* number of arguments */
  পূর্ণসংখ্যা i;
  lua_L_বাফার b;
  অক্ষর *p = luaL_buffinitsize(L, &b, cast_uint(n));
  জন্য (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  ফেরত 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
কাঠামো str_Writer {
  পূর্ণসংখ্যা init;  /* true iff buffer has been initialized */
  lua_L_বাফার B;
};


স্থির পূর্ণসংখ্যা writer (lua_State *L, ধ্রুবক শূন্য *b, size_t আকার_2, শূন্য *ud) {
  কাঠামো str_Writer *state = (কাঠামো str_Writer *)ud;
  যদি (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  যদি (b == ফাঁকা) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  অন্যথা
    luaL_addlstring(&state->B, (ধ্রুবক অক্ষর *)b, আকার_2);
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা str_dump (lua_State *L) {
  কাঠামো str_Writer state;
  পূর্ণসংখ্যা strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  ফেরত 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#যদি defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

স্থির ধ্রুবক luaL_Reg stringmetamethods[] = {
  {"__index", ফাঁকা},  /* placeholder */
  {ফাঁকা, ফাঁকা}
};

#অন্যথা		/* }{ */

স্থির পূর্ণসংখ্যা tonum (lua_State *L, পূর্ণসংখ্যা arg) {
  যদি (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    ফেরত 1;
  }
  অন্যথা {  /* check whether it is a numerical string */
    size_t len;
    ধ্রুবক অক্ষর *s = lua_tolstring(L, arg, &len);
    ফেরত (s != ফাঁকা && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
স্থির শূন্য trymt (lua_State *L, ধ্রুবক অক্ষর *mtkey, ধ্রুবক অক্ষর *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  যদি (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_ঢোকাও(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


স্থির পূর্ণসংখ্যা arith (lua_State *L, পূর্ণসংখ্যা op, ধ্রুবক অক্ষর *mtname) {
  যদি (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  অন্যথা
    trymt(L, mtname, mtname + 2);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা arith_add (lua_State *L) {
  ফেরত arith(L, LUA_OPADD, "__add");
}

স্থির পূর্ণসংখ্যা arith_sub (lua_State *L) {
  ফেরত arith(L, LUA_OPSUB, "__sub");
}

স্থির পূর্ণসংখ্যা arith_mul (lua_State *L) {
  ফেরত arith(L, LUA_OPMUL, "__mul");
}

স্থির পূর্ণসংখ্যা arith_mod (lua_State *L) {
  ফেরত arith(L, LUA_OPMOD, "__mod");
}

স্থির পূর্ণসংখ্যা arith_pow (lua_State *L) {
  ফেরত arith(L, LUA_OPPOW, "__pow");
}

স্থির পূর্ণসংখ্যা arith_div (lua_State *L) {
  ফেরত arith(L, LUA_OPDIV, "__div");
}

স্থির পূর্ণসংখ্যা arith_idiv (lua_State *L) {
  ফেরত arith(L, LUA_OPIDIV, "__idiv");
}

স্থির পূর্ণসংখ্যা arith_unm (lua_State *L) {
  ফেরত arith(L, LUA_OPUNM, "__unm");
}


স্থির ধ্রুবক luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", ফাঁকা},  /* placeholder */
  {ফাঁকা, ফাঁকা}
};

#endif		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#define CAP_UNFINISHED	(-1)
#define CAP_POSITION	(-2)


প্রকারলেখ কাঠামো MatchState {
  ধ্রুবক অক্ষর *src_init;  /* init of source string */
  ধ্রুবক অক্ষর *src_end;  /* end ('\0') of source string */
  ধ্রুবক অক্ষর *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  পূর্ণসংখ্যা matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  পূর্ণসংখ্যা level;  /* total number of captures (finished or unfinished) */
  কাঠামো {
    ধ্রুবক অক্ষর *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
স্থির ধ্রুবক অক্ষর *match (MatchState *ms, ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *p);


/* maximum recursion depth for 'match' */
#যদি !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


স্থির পূর্ণসংখ্যা check_capture (MatchState *ms, পূর্ণসংখ্যা l) {
  l -= '1';
  যদি (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    ফেরত luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  ফেরত l;
}


স্থির পূর্ণসংখ্যা capture_to_close (MatchState *ms) {
  পূর্ণসংখ্যা level = ms->level;
  জন্য (level--; level>=0; level--)
    যদি (ms->capture[level].len == CAP_UNFINISHED) ফেরত level;
  ফেরত luaL_error(ms->L, "invalid pattern capture");
}


স্থির ধ্রুবক অক্ষর *classend (MatchState *ms, ধ্রুবক অক্ষর *p) {
  নির্বাচন (*p++) {
    ক্ষেত্র L_ESC: {
      যদি (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      ফেরত p+1;
    }
    ক্ষেত্র '[': {
      যদি (*p == '^') p++;
      করো {  /* look for a ']' */
        যদি (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        যদি (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } যখন (*p != ']');
      ফেরত p+1;
    }
    পূর্বনির্ধারিত: {
      ফেরত p;
    }
  }
}


স্থির পূর্ণসংখ্যা match_class (পূর্ণসংখ্যা c, পূর্ণসংখ্যা cl) {
  পূর্ণসংখ্যা res;
  নির্বাচন (tolower(cl)) {
    ক্ষেত্র 'a' : res = isalpha(c); বিরতি;
    ক্ষেত্র 'c' : res = iscntrl(c); বিরতি;
    ক্ষেত্র 'd' : res = isdigit(c); বিরতি;
    ক্ষেত্র 'g' : res = isgraph(c); বিরতি;
    ক্ষেত্র 'l' : res = islower(c); বিরতি;
    ক্ষেত্র 'p' : res = ispunct(c); বিরতি;
    ক্ষেত্র 's' : res = isspace(c); বিরতি;
    ক্ষেত্র 'u' : res = isupper(c); বিরতি;
    ক্ষেত্র 'w' : res = isalnum(c); বিরতি;
    ক্ষেত্র 'x' : res = isxdigit(c); বিরতি;
    ক্ষেত্র 'z' : res = (c == 0); বিরতি;  /* deprecated option */
    পূর্বনির্ধারিত: ফেরত (cl == c);
  }
  ফেরত (islower(cl) ? res : !res);
}


স্থির পূর্ণসংখ্যা matchbracketclass (পূর্ণসংখ্যা c, ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *ec) {
  পূর্ণসংখ্যা sig = 1;
  যদি (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  যখন (++p < ec) {
    যদি (*p == L_ESC) {
      p++;
      যদি (match_class(c, cast_uchar(*p)))
        ফেরত sig;
    }
    অন্যথা যদি ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      যদি (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        ফেরত sig;
    }
    অন্যথা যদি (cast_uchar(*p) == c) ফেরত sig;
  }
  ফেরত !sig;
}


স্থির পূর্ণসংখ্যা singlematch (MatchState *ms, ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *p,
                        ধ্রুবক অক্ষর *ep) {
  যদি (s >= ms->src_end)
    ফেরত 0;
  অন্যথা {
    পূর্ণসংখ্যা c = cast_uchar(*s);
    নির্বাচন (*p) {
      ক্ষেত্র '.': ফেরত 1;  /* matches any char */
      ক্ষেত্র L_ESC: ফেরত match_class(c, cast_uchar(*(p+1)));
      ক্ষেত্র '[': ফেরত matchbracketclass(c, p, ep-1);
      পূর্বনির্ধারিত:  ফেরত (cast_uchar(*p) == c);
    }
  }
}


স্থির ধ্রুবক অক্ষর *matchbalance (MatchState *ms, ধ্রুবক অক্ষর *s,
                                   ধ্রুবক অক্ষর *p) {
  যদি (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  যদি (*s != *p) ফেরত ফাঁকা;
  অন্যথা {
    পূর্ণসংখ্যা b = *p;
    পূর্ণসংখ্যা e = *(p+1);
    পূর্ণসংখ্যা cont = 1;
    যখন (++s < ms->src_end) {
      যদি (*s == e) {
        যদি (--cont == 0) ফেরত s+1;
      }
      অন্যথা যদি (*s == b) cont++;
    }
  }
  ফেরত ফাঁকা;  /* string ends out of balance */
}


স্থির ধ্রুবক অক্ষর *max_expand (MatchState *ms, ধ্রুবক অক্ষর *s,
                                 ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  যখন (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  যখন (i>=0) {
    ধ্রুবক অক্ষর *res = match(ms, (s+i), ep+1);
    যদি (res) ফেরত res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  ফেরত ফাঁকা;
}


স্থির ধ্রুবক অক্ষর *min_expand (MatchState *ms, ধ্রুবক অক্ষর *s,
                                 ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *ep) {
  জন্য (;;) {
    ধ্রুবক অক্ষর *res = match(ms, s, ep+1);
    যদি (res != ফাঁকা)
      ফেরত res;
    অন্যথা যদি (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    অন্যথা ফেরত ফাঁকা;
  }
}


স্থির ধ্রুবক অক্ষর *start_capture (MatchState *ms, ধ্রুবক অক্ষর *s,
                                    ধ্রুবক অক্ষর *p, পূর্ণসংখ্যা what) {
  ধ্রুবক অক্ষর *res;
  পূর্ণসংখ্যা level = ms->level;
  যদি (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  যদি ((res=match(ms, s, p)) == ফাঁকা)  /* match failed? */
    ms->level--;  /* undo capture */
  ফেরত res;
}


স্থির ধ্রুবক অক্ষর *end_capture (MatchState *ms, ধ্রুবক অক্ষর *s,
                                  ধ্রুবক অক্ষর *p) {
  পূর্ণসংখ্যা l = capture_to_close(ms);
  ধ্রুবক অক্ষর *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  যদি ((res = match(ms, s, p)) == ফাঁকা)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  ফেরত res;
}


স্থির ধ্রুবক অক্ষর *match_capture (MatchState *ms, ধ্রুবক অক্ষর *s, পূর্ণসংখ্যা l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  যদি ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    ফেরত s+len;
  অন্যথা ফেরত ফাঁকা;
}


স্থির ধ্রুবক অক্ষর *match (MatchState *ms, ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *p) {
  যদি (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  যদি (p != ms->p_end) {  /* end of pattern? */
    নির্বাচন (*p) {
      ক্ষেত্র '(': {  /* start capture */
        যদি (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        অন্যথা
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        বিরতি;
      }
      ক্ষেত্র ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        বিরতি;
      }
      ক্ষেত্র '$': {
        যদি ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          যাও dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : ফাঁকা;  /* check end of string */
        বিরতি;
      }
      ক্ষেত্র L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        নির্বাচন (*(p + 1)) {
          ক্ষেত্র 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            যদি (s != ফাঁকা) {
              p += 4; যাও init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            বিরতি;
          }
          ক্ষেত্র 'f': {  /* frontier? */
            ধ্রুবক অক্ষর *ep; অক্ষর previous;
            p += 2;
            যদি (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            যদি (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; যাও init;  /* return match(ms, s, ep); */
            }
            s = ফাঁকা;  /* match failed */
            বিরতি;
          }
          ক্ষেত্র '0': ক্ষেত্র '1': ক্ষেত্র '2': ক্ষেত্র '3':
          ক্ষেত্র '4': ক্ষেত্র '5': ক্ষেত্র '6': ক্ষেত্র '7':
          ক্ষেত্র '8': ক্ষেত্র '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            যদি (s != ফাঁকা) {
              p += 2; যাও init;  /* return match(ms, s, p + 2) */
            }
            বিরতি;
          }
          পূর্বনির্ধারিত: যাও dflt;
        }
        বিরতি;
      }
      পূর্বনির্ধারিত: dflt: {  /* pattern class plus optional suffix */
        ধ্রুবক অক্ষর *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        যদি (!singlematch(ms, s, p, ep)) {
          যদি (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; যাও init;  /* return match(ms, s, ep + 1); */
          }
          অন্যথা  /* '+' or no suffix */
            s = ফাঁকা;  /* fail */
        }
        অন্যথা {  /* matched once */
          নির্বাচন (*ep) {  /* handle optional suffix */
            ক্ষেত্র '?': {  /* optional */
              ধ্রুবক অক্ষর *res;
              যদি ((res = match(ms, s + 1, ep + 1)) != ফাঁকা)
                s = res;
              অন্যথা {
                p = ep + 1; যাও init;  /* else return match(ms, s, ep + 1); */
              }
              বিরতি;
            }
            ক্ষেত্র '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            ক্ষেত্র '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              বিরতি;
            ক্ষেত্র '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              বিরতি;
            পূর্বনির্ধারিত:  /* no suffix */
              s++; p = ep; যাও init;  /* return match(ms, s + 1, ep); */
          }
        }
        বিরতি;
      }
    }
  }
  ms->matchdepth++;
  ফেরত s;
}



স্থির ধ্রুবক অক্ষর *lmemfind (ধ্রুবক অক্ষর *s1, size_t l1,
                               ধ্রুবক অক্ষর *s2, size_t l2) {
  যদি (l2 == 0) ফেরত s1;  /* empty strings are everywhere */
  অন্যথা যদি (l2 > l1) ফেরত ফাঁকা;  /* avoids a negative 'l1' */
  অন্যথা {
    ধ্রুবক অক্ষর *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    যখন (l1 > 0 && (init = (ধ্রুবক অক্ষর *)memchr(s1, *s2, l1)) != ফাঁকা) {
      init++;   /* 1st char is already checked */
      যদি (memcmp(init, s2+1, l2) == 0)
        ফেরত init-1;
      অন্যথা {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    ফেরত ফাঁকা;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
স্থির ptrdiff_t get_onecapture (MatchState *ms, পূর্ণসংখ্যা i, ধ্রুবক অক্ষর *s,
                              ধ্রুবক অক্ষর *e, ধ্রুবক অক্ষর **cap) {
  যদি (i >= ms->level) {
    যদি (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    ফেরত (e - s);
  }
  অন্যথা {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    যদি (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    অন্যথা যদি (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    ফেরত capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
স্থির শূন্য ঠেলো_onecapture (MatchState *ms, পূর্ণসংখ্যা i, ধ্রুবক অক্ষর *s,
                                                    ধ্রুবক অক্ষর *e) {
  ধ্রুবক অক্ষর *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  যদি (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


স্থির পূর্ণসংখ্যা ঠেলো_captures (MatchState *ms, ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *e) {
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  জন্য (i = 0; i < nlevels; i++)
    ঠেলো_onecapture(ms, i, s, e);
  ফেরত nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
স্থির পূর্ণসংখ্যা nospecials (ধ্রুবক অক্ষর *p, size_t l) {
  size_t upto = 0;
  করো {
    যদি (strpbrk(p + upto, SPECIALS))
      ফেরত 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } যখন (upto <= l);
  ফেরত 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
স্থির শূন্য prepstate (MatchState *ms, lua_State *L,
                       ধ্রুবক অক্ষর *s, size_t ls, ধ্রুবক অক্ষর *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
স্থির শূন্য reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


স্থির পূর্ণসংখ্যা str_find_aux (lua_State *L, পূর্ণসংখ্যা find) {
  size_t ls, lp;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &ls);
  ধ্রুবক অক্ষর *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  যদি (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    ফেরত 1;
  }
  /* explicit request or no special characters? */
  যদি (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ধ্রুবক অক্ষর *s2 = lmemfind(s + init, ls - init, p, lp);
    যদি (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      ফেরত 2;
    }
  }
  অন্যথা {
    MatchState ms;
    ধ্রুবক অক্ষর *s1 = s + init;
    পূর্ণসংখ্যা anchor = (*p == '^');
    যদি (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    করো {
      ধ্রুবক অক্ষর *res;
      reprepstate(&ms);
      যদি ((res=match(&ms, s1, p)) != ফাঁকা) {
        যদি (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          ফেরত ঠেলো_captures(&ms, ফাঁকা, 0) + 2;
        }
        অন্যথা
          ফেরত ঠেলো_captures(&ms, s1, res);
      }
    } যখন (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা str_find (lua_State *L) {
  ফেরত str_find_aux(L, 1);
}


স্থির পূর্ণসংখ্যা str_match (lua_State *L) {
  ফেরত str_find_aux(L, 0);
}


/* state for 'gmatch' */
প্রকারলেখ কাঠামো GMatchState {
  ধ্রুবক অক্ষর *src;  /* current position */
  ধ্রুবক অক্ষর *p;  /* pattern */
  ধ্রুবক অক্ষর *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


স্থির পূর্ণসংখ্যা gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ধ্রুবক অক্ষর *src;
  gm->ms.L = L;
  জন্য (src = gm->src; src <= gm->ms.src_end; src++) {
    ধ্রুবক অক্ষর *e;
    reprepstate(&gm->ms);
    যদি ((e = match(&gm->ms, src, gm->p)) != ফাঁকা && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      ফেরত ঠেলো_captures(&gm->ms, src, e);
    }
  }
  ফেরত 0;  /* not found */
}


স্থির পূর্ণসংখ্যা gmatch (lua_State *L) {
  size_t ls, lp;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &ls);
  ধ্রুবক অক্ষর *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, আকার(GMatchState), 0);
  যদি (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = ফাঁকা;
  lua_pushcclosure(L, gmatch_aux, 3);
  ফেরত 1;
}


স্থির শূন্য add_s (MatchState *ms, lua_L_বাফার *b, ধ্রুবক অক্ষর *s,
                                                   ধ্রুবক অক্ষর *e) {
  size_t l;
  lua_State *L = ms->L;
  ধ্রুবক অক্ষর *news = lua_tolstring(L, 3, &l);
  ধ্রুবক অক্ষর *p;
  যখন ((p = (অক্ষর *)memchr(news, L_ESC, l)) != ফাঁকা) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    যদি (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    অন্যথা যদি (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    অন্যথা যদি (isdigit(cast_uchar(*p))) {  /* '%n' */
      ধ্রুবক অক্ষর *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      যদি (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      অন্যথা
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    অন্যথা
      luaL_error(L, "invalid use of '%c' in replacement string", L_ESC);
    l -= ct_diff2sz(p + 1 - news);
    news = p + 1;
  }
  luaL_addlstring(b, news, l);
}


/*
** Add the replacement value to the string buffer 'b'.
** Return true if the original string was changed. (Function calls and
** table indexing resulting in nil or false do not change the subject.)
*/
স্থির পূর্ণসংখ্যা add_মান (MatchState *ms, lua_L_বাফার *b, ধ্রুবক অক্ষর *s,
                                      ধ্রুবক অক্ষর *e, পূর্ণসংখ্যা tr) {
  lua_State *L = ms->L;
  নির্বাচন (tr) {
    ক্ষেত্র LUA_TFUNCTION: {  /* call the function */
      পূর্ণসংখ্যা n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ঠেলো_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      বিরতি;
    }
    ক্ষেত্র LUA_TTABLE: {  /* index the table */
      ঠেলো_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      বিরতি;
    }
    পূর্বনির্ধারিত: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      ফেরত 1;  /* something changed */
    }
  }
  যদি (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_বের_করো(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    ফেরত 0;  /* no changes */
  }
  অন্যথা যদি (l_unlikely(!lua_isstring(L, -1)))
    ফেরত luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  অন্যথা {
    luaL_addvalue(b);  /* add result to accumulator */
    ফেরত 1;  /* something changed */
  }
}


স্থির পূর্ণসংখ্যা str_gsub (lua_State *L) {
  size_t srcl, lp;
  ধ্রুবক অক্ষর *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ধ্রুবক অক্ষর *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  ধ্রুবক অক্ষর *lastmatch = ফাঁকা;  /* end of last match */
  পূর্ণসংখ্যা tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  পূর্ণসংখ্যা anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  পূর্ণসংখ্যা changed = 0;  /* change flag */
  MatchState ms;
  lua_L_বাফার b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  যদি (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  যখন (n < max_s) {
    ধ্রুবক অক্ষর *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    যদি ((e = match(&ms, src, p)) != ফাঁকা && e != lastmatch) {  /* match? */
      n++;
      changed = add_মান(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    অন্যথা যদি (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    অন্যথা বিরতি;  /* end of subject */
    যদি (anchor) বিরতি;
  }
  যদি (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  অন্যথা {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  ফেরত 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#যদি !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(আকার(LUA_সংখ্যা_FRMLEN)/আকার(অক্ষর))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#define L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
স্থির lua_সংখ্যা adddigit (অক্ষর *buff, unsigned n, lua_সংখ্যা x) {
  lua_সংখ্যা dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  পূর্ণসংখ্যা d = (পূর্ণসংখ্যা)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  ফেরত x - dd;  /* return what is left */
}


স্থির পূর্ণসংখ্যা num2straux (অক্ষর *buff, unsigned sz, lua_সংখ্যা x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  যদি (x != x || x == (lua_সংখ্যা)HUGE_VAL || x == -(lua_সংখ্যা)HUGE_VAL)
    ফেরত l_sprintf(buff, sz, LUA_সংখ্যা_FMT, (LUAI_UACNUMBER)x);
  অন্যথা যদি (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    ফেরত l_sprintf(buff, sz, LUA_সংখ্যা_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  অন্যথা {
    পূর্ণসংখ্যা e;
    lua_সংখ্যা m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    যদি (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    যদি (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      করো {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } যখন (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    ফেরত cast_int(n);
  }
}


স্থির পূর্ণসংখ্যা lua_number2strx (lua_State *L, অক্ষর *buff, unsigned sz,
                            ধ্রুবক অক্ষর *fmt, lua_সংখ্যা x) {
  পূর্ণসংখ্যা n = num2straux(buff, sz, x);
  যদি (fmt[SIZELENMOD] == 'A') {
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  অন্যথা যদি (l_unlikely(fmt[SIZELENMOD] != 'a'))
    ফেরত luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  ফেরত n;
}

#endif				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#define MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#define MAX_ITEM	120


/* valid flags in a format specification */
#যদি !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#define L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#define L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#define L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#define L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#define L_FMTFLAGSC	"-"

#endif


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#define MAX_FORMAT	32


স্থির শূন্য addquoted (lua_L_বাফার *b, ধ্রুবক অক্ষর *s, size_t len) {
  luaL_addchar(b, '"');
  যখন (len--) {
    যদি (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    অন্যথা যদি (iscntrl(cast_uchar(*s))) {
      অক্ষর buff[10];
      যদি (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, আকার(buff), "\\%d", (পূর্ণসংখ্যা)cast_uchar(*s));
      অন্যথা
        l_sprintf(buff, আকার(buff), "\\%03d", (পূর্ণসংখ্যা)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    অন্যথা
      luaL_addchar(b, *s);
    s++;
  }
  luaL_addchar(b, '"');
}


/*
** Serialize a floating-point number in such a way that it can be
** scanned back by Lua. Use hexadecimal format for "common" numbers
** (to preserve precision); inf, -inf, and NaN are handled separately.
** (NaN cannot be expressed as a numeral, so we write '(0/0)' for it.)
*/
স্থির পূর্ণসংখ্যা quotefloat (lua_State *L, অক্ষর *buff, lua_সংখ্যা n) {
  ধ্রুবক অক্ষর *s;  /* for the fixed representations */
  যদি (n == (lua_সংখ্যা)HUGE_VAL)  /* inf? */
    s = "1e9999";
  অন্যথা যদি (n == -(lua_সংখ্যা)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  অন্যথা যদি (n != n)  /* NaN? */
    s = "(0/0)";
  অন্যথা {  /* format number as hexadecimal */
    পূর্ণসংখ্যা  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_সংখ্যা_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    যদি (memchr(buff, '.', cast_uint(nb)) == ফাঁকা) {  /* no dot? */
      অক্ষর point = lua_getlocaledecpoint();  /* try locale point */
      অক্ষর *ppoint = (অক্ষর *)memchr(buff, point, cast_uint(nb));
      যদি (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    ফেরত nb;
  }
  /* for the fixed representations */
  ফেরত l_sprintf(buff, MAX_ITEM, "%s", s);
}


স্থির শূন্য addliteral (lua_State *L, lua_L_বাফার *b, পূর্ণসংখ্যা arg) {
  নির্বাচন (lua_type(L, arg)) {
    ক্ষেত্র LUA_TSTRING: {
      size_t len;
      ধ্রুবক অক্ষর *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      বিরতি;
    }
    ক্ষেত্র LUA_TNUMBER: {
      অক্ষর *buff = luaL_prepbuffsize(b, MAX_ITEM);
      পূর্ণসংখ্যা nb;
      যদি (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      অন্যথা {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ধ্রুবক অক্ষর *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      বিরতি;
    }
    ক্ষেত্র LUA_TNIL: ক্ষেত্র LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, ফাঁকা);
      luaL_addvalue(b);
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


স্থির ধ্রুবক অক্ষর *get2digits (ধ্রুবক অক্ষর *s) {
  যদি (isdigit(cast_uchar(*s))) {
    s++;
    যদি (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  ফেরত s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
স্থির শূন্য checkformat (lua_State *L, ধ্রুবক অক্ষর *form, ধ্রুবক অক্ষর *flags,
                                       পূর্ণসংখ্যা precision) {
  ধ্রুবক অক্ষর *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  যদি (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    যদি (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  যদি (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
স্থির ধ্রুবক অক্ষর *getformat (lua_State *L, ধ্রুবক অক্ষর *strfrmt,
                                            অক্ষর *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  যদি (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * আকার(অক্ষর));
  *(form + len) = '\0';
  ফেরত strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
স্থির শূন্য addlenmod (অক্ষর *form, ধ্রুবক অক্ষর *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  অক্ষর spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


স্থির পূর্ণসংখ্যা str_format (lua_State *L) {
  পূর্ণসংখ্যা শীর্ষ = lua_gettop(L);
  পূর্ণসংখ্যা arg = 1;
  size_t sfl;
  ধ্রুবক অক্ষর *strfrmt = luaL_checklstring(L, arg, &sfl);
  ধ্রুবক অক্ষর *strfrmt_end = strfrmt+sfl;
  ধ্রুবক অক্ষর *flags;
  lua_L_বাফার b;
  luaL_buffinit(L, &b);
  যখন (strfrmt < strfrmt_end) {
    যদি (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    অন্যথা যদি (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    অন্যথা { /* format item */
      অক্ষর form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      অক্ষর *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      পূর্ণসংখ্যা nb = 0;  /* number of bytes in result */
      যদি (++arg > শীর্ষ)
        ফেরত luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      নির্বাচন (*strfrmt++) {
        ক্ষেত্র 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (পূর্ণসংখ্যা)luaL_checkinteger(L, arg));
          বিরতি;
        }
        ক্ষেত্র 'd': ক্ষেত্র 'i':
          flags = L_FMTFLAGSI;
          যাও intcase;
        ক্ষেত্র 'u':
          flags = L_FMTFLAGSU;
          যাও intcase;
        ক্ষেত্র 'o': ক্ষেত্র 'x': ক্ষেত্র 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          বিরতি;
        }
        ক্ষেত্র 'a': ক্ষেত্র 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_সংখ্যা_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          বিরতি;
        ক্ষেত্র 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        ক্ষেত্র 'e': ক্ষেত্র 'E': ক্ষেত্র 'g': ক্ষেত্র 'G': {
          lua_সংখ্যা n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_সংখ্যা_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          বিরতি;
        }
        ক্ষেত্র 'p': {
          ধ্রুবক শূন্য *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          যদি (p == ফাঁকা) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /*ফলাফল*/
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          বিরতি;
        }
        ক্ষেত্র 'q': {
          যদি (form[2] != '\0')  /* modifiers? */
            ফেরত luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          বিরতি;
        }
        ক্ষেত্র 's': {
          size_t l;
          ধ্রুবক অক্ষর *s = luaL_tolstring(L, arg, &l);
          যদি (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          অন্যথা {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            যদি (strchr(form, '.') == ফাঁকা && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            অন্যথা {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_বের_করো(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          বিরতি;
        }
        পূর্বনির্ধারিত: {  /* also treat cases 'pnLlh' */
          ফেরত luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  ফেরত 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#যদি !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((পূর্ণসংখ্যা)আকার(lua_Integer))


/* dummy union to get native endianness */
স্থির ধ্রুবক সংঘ {
  পূর্ণসংখ্যা dummy;
  অক্ষর little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
প্রকারলেখ কাঠামো Header {
  lua_State *L;
  পূর্ণসংখ্যা islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
প্রকারলেখ গণনা KOption {
  Kint,		/* signed integers */
  Kuint,	/* unsigned integers */
  Kfloat,	/* single-precision floating-point numbers */
  Knumber,	/* Lua "native" floating-point numbers */
  Kdouble,	/* double-precision floating-point numbers */
  Kchar,	/* fixed-length strings */
  Kstring,	/* strings with prefixed length */
  Kzstr,	/* zero-terminated strings */
  Kpadding,	/* padding */
  Kpaddalign,	/* padding for alignment */
  Knop		/* no-op (configuration or spaces) */
} KOption;


/*
** Read an integer numeral from string 'fmt' or return 'df' if
** there is no numeral
*/
স্থির পূর্ণসংখ্যা digit (পূর্ণসংখ্যা c) { ফেরত '0' <= c && c <= '9'; }

স্থির size_t getnum (ধ্রুবক অক্ষর **fmt, size_t df) {
  যদি (!digit(**fmt))  /* no number? */
    ফেরত df;  /* return default value */
  অন্যথা {
    size_t a = 0;
    করো {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } যখন (digit(**fmt) && a <= (MAX_আকার - 9)/10);
    ফেরত a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
স্থির unsigned getnumlimit (Header *h, ধ্রুবক অক্ষর **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  যদি (l_unlikely((sz - 1u) >= MAXINTSIZE))
    ফেরত cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  ফেরত cast_uint(sz);
}


/*
** Initialize Header
*/
স্থির শূন্য initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
স্থির KOption getoption (Header *h, ধ্রুবক অক্ষর **fmt, size_t *আকার_2) {
  /* dummy structure to get native alignment requirements */
  কাঠামো cD { অক্ষর c; সংঘ { LUAI_MAXALIGN; } u; };
  পূর্ণসংখ্যা opt = *((*fmt)++);
  *আকার_2 = 0;  /* default */
  নির্বাচন (opt) {
    ক্ষেত্র 'b': *আকার_2 = আকার(অক্ষর); ফেরত Kint;
    ক্ষেত্র 'B': *আকার_2 = আকার(অক্ষর); ফেরত Kuint;
    ক্ষেত্র 'h': *আকার_2 = আকার(short); ফেরত Kint;
    ক্ষেত্র 'H': *আকার_2 = আকার(short); ফেরত Kuint;
    ক্ষেত্র 'l': *আকার_2 = আকার(long); ফেরত Kint;
    ক্ষেত্র 'L': *আকার_2 = আকার(long); ফেরত Kuint;
    ক্ষেত্র 'j': *আকার_2 = আকার(lua_Integer); ফেরত Kint;
    ক্ষেত্র 'J': *আকার_2 = আকার(lua_Integer); ফেরত Kuint;
    ক্ষেত্র 'T': *আকার_2 = আকার(size_t); ফেরত Kuint;
    ক্ষেত্র 'f': *আকার_2 = আকার(দশমিক); ফেরত Kfloat;
    ক্ষেত্র 'n': *আকার_2 = আকার(lua_সংখ্যা); ফেরত Knumber;
    ক্ষেত্র 'd': *আকার_2 = আকার(দ্বিগুণ); ফেরত Kdouble;
    ক্ষেত্র 'i': *আকার_2 = getnumlimit(h, fmt, আকার(পূর্ণসংখ্যা)); ফেরত Kint;
    ক্ষেত্র 'I': *আকার_2 = getnumlimit(h, fmt, আকার(পূর্ণসংখ্যা)); ফেরত Kuint;
    ক্ষেত্র 's': *আকার_2 = getnumlimit(h, fmt, আকার(size_t)); ফেরত Kstring;
    ক্ষেত্র 'c':
      *আকার_2 = getnum(fmt, cast_sizet(-1));
      যদি (l_unlikely(*আকার_2 == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      ফেরত Kchar;
    ক্ষেত্র 'z': ফেরত Kzstr;
    ক্ষেত্র 'x': *আকার_2 = 1; ফেরত Kpadding;
    ক্ষেত্র 'X': ফেরত Kpaddalign;
    ক্ষেত্র ' ': বিরতি;
    ক্ষেত্র '<': h->islittle = 1; বিরতি;
    ক্ষেত্র '>': h->islittle = 0; বিরতি;
    ক্ষেত্র '=': h->islittle = nativeendian.little; বিরতি;
    ক্ষেত্র '!': {
      ধ্রুবক size_t maxalign = offsetof(কাঠামো cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      বিরতি;
    }
    পূর্বনির্ধারিত: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  ফেরত Knop;
}


/*
** Read, classify, and fill other details about the next option.
** 'psize' is filled with option's size, 'notoalign' with its
** alignment requirements.
** Local variable 'size' gets the size to be aligned. (Kpadal option
** always gets its full alignment, other options are limited by
** the maximum alignment ('maxalign'). Kchar option needs no alignment
** despite its size.
*/
স্থির KOption getdetails (Header *h, size_t totalsize, ধ্রুবক অক্ষর **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  যদি (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    যদি (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  যদি (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  অন্যথা {
    যদি (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    যদি (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    অন্যথা {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  ফেরত opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
স্থির শূন্য packint (lua_L_বাফার *b, lua_Unsigned n,
                     পূর্ণসংখ্যা islittle, unsigned আকার_2, পূর্ণসংখ্যা neg) {
  অক্ষর *buff = luaL_prepbuffsize(b, আকার_2);
  unsigned i;
  buff[islittle ? 0 : আকার_2 - 1] = (অক্ষর)(n & MC);  /* first byte */
  জন্য (i = 1; i < আকার_2; i++) {
    n >>= NB;
    buff[islittle ? i : আকার_2 - 1 - i] = (অক্ষর)(n & MC);
  }
  যদি (neg && আকার_2 > SZINT) {  /* negative number need sign extension? */
    জন্য (i = SZINT; i < আকার_2; i++)  /* correct extra bytes */
      buff[islittle ? i : আকার_2 - 1 - i] = (অক্ষর)MC;
  }
  luaL_addsize(b, আকার_2);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
স্থির শূন্য copywithendian (অক্ষর *dest, ধ্রুবক অক্ষর *src,
                            unsigned আকার_2, পূর্ণসংখ্যা islittle) {
  যদি (islittle == nativeendian.little)
    memcpy(dest, src, আকার_2);
  অন্যথা {
    dest += আকার_2 - 1;
    যখন (আকার_2-- != 0)
      *(dest--) = *(src++);
  }
}


স্থির পূর্ণসংখ্যা str_pack (lua_State *L) {
  lua_L_বাফার b;
  Header h;
  ধ্রুবক অক্ষর *fmt = luaL_checkstring(L, 1);  /* format string */
  পূর্ণসংখ্যা arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  যখন (*fmt != '\0') {
    unsigned ntoalign;
    size_t আকার_2;
    KOption opt = getdetails(&h, totalsize, &fmt, &আকার_2, &ntoalign);
    luaL_argcheck(L, আকার_2 + ntoalign <= MAX_আকার - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + আকার_2;
    যখন (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    নির্বাচন (opt) {
      ক্ষেত্র Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        যদি (আকার_2 < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((আকার_2 * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(আকার_2), (n < 0));
        বিরতি;
      }
      ক্ষেত্র Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        যদি (আকার_2 < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (আকার_2 * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(আকার_2), 0);
        বিরতি;
      }
      ক্ষেত্র Kfloat: {  /* C float */
        দশমিক f = (দশমিক)luaL_checknumber(L, arg);  /* get argument */
        অক্ষর *buff = luaL_prepbuffsize(&b, আকার(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (অক্ষর *)&f, আকার(f), h.islittle);
        luaL_addsize(&b, আকার_2);
        বিরতি;
      }
      ক্ষেত্র Knumber: {  /* Lua float */
        lua_সংখ্যা f = luaL_checknumber(L, arg);  /* get argument */
        অক্ষর *buff = luaL_prepbuffsize(&b, আকার(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (অক্ষর *)&f, আকার(f), h.islittle);
        luaL_addsize(&b, আকার_2);
        বিরতি;
      }
      ক্ষেত্র Kdouble: {  /* C double */
        দ্বিগুণ f = (দ্বিগুণ)luaL_checknumber(L, arg);  /* get argument */
        অক্ষর *buff = luaL_prepbuffsize(&b, আকার(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (অক্ষর *)&f, আকার(f), h.islittle);
        luaL_addsize(&b, আকার_2);
        বিরতি;
      }
      ক্ষেত্র Kchar: {  /* fixed-size string */
        size_t len;
        ধ্রুবক অক্ষর *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= আকার_2, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        যদি (len < আকার_2) {  /* does it need padding? */
          size_t psize = আকার_2 - len;  /* pad size */
          অক্ষর *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        বিরতি;
      }
      ক্ষেত্র Kstring: {  /* strings with length count */
        size_t len;
        ধ্রুবক অক্ষর *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, আকার_2 >= আকার(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (আকার_2 * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(আকার_2), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        বিরতি;
      }
      ক্ষেত্র Kzstr: {  /* zero-terminated string */
        size_t len;
        ধ্রুবক অক্ষর *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        বিরতি;
      }
      ক্ষেত্র Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      ক্ষেত্র Kpaddalign: ক্ষেত্র Knop:
        arg--;  /* undo increment */
        বিরতি;
    }
  }
  luaL_pushresult(&b);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা str_packsize (lua_State *L) {
  Header h;
  ধ্রুবক অক্ষর *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  যখন (*fmt != '\0') {
    unsigned ntoalign;
    size_t আকার_2;
    KOption opt = getdetails(&h, totalsize, &fmt, &আকার_2, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    আকার_2 += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_আকার - আকার_2,
                     1, "format result too large");
    totalsize += আকার_2;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  ফেরত 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
স্থির lua_Integer unpackint (lua_State *L, ধ্রুবক অক্ষর *str,
                              পূর্ণসংখ্যা islittle, পূর্ণসংখ্যা আকার_2, পূর্ণসংখ্যা issigned) {
  lua_Unsigned res = 0;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা limit = (আকার_2  <= SZINT) ? আকার_2 : SZINT;
  জন্য (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned অক্ষর)str[islittle ? i : আকার_2 - 1 - i];
  }
  যদি (আকার_2 < SZINT) {  /* real size smaller than lua_Integer? */
    যদি (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (আকার_2*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  অন্যথা যদি (আকার_2 > SZINT) {  /* must check unread bytes */
    পূর্ণসংখ্যা mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    জন্য (i = limit; i < আকার_2; i++) {
      যদি (l_unlikely((unsigned অক্ষর)str[islittle ? i : আকার_2 - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", আকার_2);
    }
  }
  ফেরত (lua_Integer)res;
}


স্থির পূর্ণসংখ্যা str_unpack (lua_State *L) {
  Header h;
  ধ্রুবক অক্ষর *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ধ্রুবক অক্ষর *তথ্য = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  পূর্ণসংখ্যা n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  যখন (*fmt != '\0') {
    unsigned ntoalign;
    size_t আকার_2;
    KOption opt = getdetails(&h, pos, &fmt, &আকার_2, &ntoalign);
    luaL_argcheck(L, ntoalign + আকার_2 <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    নির্বাচন (opt) {
      ক্ষেত্র Kint:
      ক্ষেত্র Kuint: {
        lua_Integer res = unpackint(L, তথ্য + pos, h.islittle,
                                       cast_int(আকার_2), (opt == Kint));
        lua_pushinteger(L, res);
        বিরতি;
      }
      ক্ষেত্র Kfloat: {
        দশমিক f;
        copywithendian((অক্ষর *)&f, তথ্য + pos, আকার(f), h.islittle);
        lua_pushnumber(L, (lua_সংখ্যা)f);
        বিরতি;
      }
      ক্ষেত্র Knumber: {
        lua_সংখ্যা f;
        copywithendian((অক্ষর *)&f, তথ্য + pos, আকার(f), h.islittle);
        lua_pushnumber(L, f);
        বিরতি;
      }
      ক্ষেত্র Kdouble: {
        দ্বিগুণ f;
        copywithendian((অক্ষর *)&f, তথ্য + pos, আকার(f), h.islittle);
        lua_pushnumber(L, (lua_সংখ্যা)f);
        বিরতি;
      }
      ক্ষেত্র Kchar: {
        lua_pushlstring(L, তথ্য + pos, আকার_2);
        বিরতি;
      }
      ক্ষেত্র Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, তথ্য + pos,
                                          h.islittle, cast_int(আকার_2), 0);
        luaL_argcheck(L, len <= ld - pos - আকার_2, 2, "data string too short");
        lua_pushlstring(L, তথ্য + pos + আকার_2, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        বিরতি;
      }
      ক্ষেত্র Kzstr: {
        size_t len = strlen(তথ্য + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, তথ্য + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        বিরতি;
      }
      ক্ষেত্র Kpaddalign: ক্ষেত্র Kpadding: ক্ষেত্র Knop:
        n--;  /* undo increment */
        বিরতি;
    }
    pos += আকার_2;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  ফেরত n + 1;
}

/* }====================================================== */


স্থির ধ্রুবক luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_find},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_lower},
  {"match", str_match},
  {"rep", str_rep},
  {"reverse", str_reverse},
  {"sub", str_sub},
  {"upper", str_upper},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {ফাঁকা, ফাঁকা}
};


স্থির শূন্য createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_বের_করো(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_বের_করো(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API পূর্ণসংখ্যা luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  ফেরত 1;
}

