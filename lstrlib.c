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


স্থির পূর্ণাঙ্ক str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  প্রত্যাগম 1;
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
    প্রত্যাগম (size_t)pos;
  অন্যথা যদি (pos == 0)
    প্রত্যাগম 1;
  অন্যথা যদি (pos < -(lua_Integer)len)  /* inverted comparison */
    প্রত্যাগম 1;  /* clip to 1 */
  অন্যথা প্রত্যাগম len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
স্থির size_t getendpos (lua_State *L, পূর্ণাঙ্ক arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  যদি (pos > (lua_Integer)len)
    প্রত্যাগম len;
  অন্যথা যদি (pos >= 0)
    প্রত্যাগম (size_t)pos;
  অন্যথা যদি (pos < -(lua_Integer)len)
    প্রত্যাগম 0;
  অন্যথা প্রত্যাগম len + (size_t)pos + 1;
}


স্থির পূর্ণাঙ্ক str_sub (lua_State *L) {
  size_t l;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  যদি (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  অন্যথা lua_pushliteral(L, "");
  প্রত্যাগম 1;
}


স্থির পূর্ণাঙ্ক str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &l);
  বর্ণ *p = luaL_buffinitsize(L, &b, l);
  প্রতি (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  প্রত্যাগম 1;
}


স্থির পূর্ণাঙ্ক str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &l);
  বর্ণ *p = luaL_buffinitsize(L, &b, l);
  প্রতি (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  প্রত্যাগম 1;
}


স্থির পূর্ণাঙ্ক str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &l);
  বর্ণ *p = luaL_buffinitsize(L, &b, l);
  প্রতি (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  প্রত্যাগম 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
স্থির পূর্ণাঙ্ক str_rep (lua_State *L) {
  size_t len, lsep;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ধ্রুব বর্ণ *sep = luaL_optlstring(L, 3, "", &lsep);
  যদি (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  অন্যথা যদি (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    প্রত্যাগম luaL_error(L, "resulting string too large");
  অন্যথা {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    বর্ণ *p = luaL_buffinitsize(L, &b, totallen);
    যাবত্ (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * পরিমাণ(বর্ণ)); p += len;
      যদি (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * পরিমাণ(বর্ণ)); p += lsep;
      }
    }
    memcpy(p, s, len * পরিমাণ(বর্ণ));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  প্রত্যাগম 1;
}


স্থির পূর্ণাঙ্ক str_byte (lua_State *L) {
  size_t l;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  পূর্ণাঙ্ক n, i;
  যদি (posi > pose) প্রত্যাগম 0;  /* empty interval; return no values */
  যদি (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    প্রত্যাগম luaL_error(L, "string slice too long");
  n = (পূর্ণাঙ্ক)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  প্রতি (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  প্রত্যাগম n;
}


স্থির পূর্ণাঙ্ক str_char (lua_State *L) {
  পূর্ণাঙ্ক n = lua_gettop(L);  /* number of arguments */
  পূর্ণাঙ্ক i;
  luaL_Buffer b;
  বর্ণ *p = luaL_buffinitsize(L, &b, cast_uint(n));
  প্রতি (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  প্রত্যাগম 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
সংরচনা str_Writer {
  পূর্ণাঙ্ক init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


স্থির পূর্ণাঙ্ক writer (lua_State *L, ধ্রুব শূন্য *b, size_t size, শূন্য *ud) {
  সংরচনা str_Writer *state = (সংরচনা str_Writer *)ud;
  যদি (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  যদি (b == রিক্ত) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  অন্যথা
    luaL_addlstring(&state->B, (ধ্রুব বর্ণ *)b, size);
  প্রত্যাগম 0;
}


স্থির পূর্ণাঙ্ক str_dump (lua_State *L) {
  সংরচনা str_Writer state;
  পূর্ণাঙ্ক strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  প্রত্যাগম 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#যদি defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

স্থির ধ্রুব luaL_Reg stringmetamethods[] = {
  {"__index", রিক্ত},  /* placeholder */
  {রিক্ত, রিক্ত}
};

#অন্যথা		/* }{ */

স্থির পূর্ণাঙ্ক tonum (lua_State *L, পূর্ণাঙ্ক arg) {
  যদি (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    প্রত্যাগম 1;
  }
  অন্যথা {  /* check whether it is a numerical string */
    size_t len;
    ধ্রুব বর্ণ *s = lua_tolstring(L, arg, &len);
    প্রত্যাগম (s != রিক্ত && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
স্থির শূন্য trymt (lua_State *L, ধ্রুব বর্ণ *mtkey, ধ্রুব বর্ণ *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  যদি (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_ঢোকাও(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


স্থির পূর্ণাঙ্ক arith (lua_State *L, পূর্ণাঙ্ক op, ধ্রুব বর্ণ *mtname) {
  যদি (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  অন্যথা
    trymt(L, mtname, mtname + 2);
  প্রত্যাগম 1;
}


স্থির পূর্ণাঙ্ক arith_add (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPADD, "__add");
}

স্থির পূর্ণাঙ্ক arith_sub (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPSUB, "__sub");
}

স্থির পূর্ণাঙ্ক arith_mul (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPMUL, "__mul");
}

স্থির পূর্ণাঙ্ক arith_mod (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPMOD, "__mod");
}

স্থির পূর্ণাঙ্ক arith_pow (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPPOW, "__pow");
}

স্থির পূর্ণাঙ্ক arith_div (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPDIV, "__div");
}

স্থির পূর্ণাঙ্ক arith_idiv (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPIDIV, "__idiv");
}

স্থির পূর্ণাঙ্ক arith_unm (lua_State *L) {
  প্রত্যাগম arith(L, LUA_OPUNM, "__unm");
}


স্থির ধ্রুব luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", রিক্ত},  /* placeholder */
  {রিক্ত, রিক্ত}
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


প্রকারনির্দেশ সংরচনা MatchState {
  ধ্রুব বর্ণ *src_init;  /* init of source string */
  ধ্রুব বর্ণ *src_end;  /* end ('\0') of source string */
  ধ্রুব বর্ণ *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  পূর্ণাঙ্ক matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  পূর্ণাঙ্ক level;  /* total number of captures (finished or unfinished) */
  সংরচনা {
    ধ্রুব বর্ণ *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
স্থির ধ্রুব বর্ণ *match (MatchState *ms, ধ্রুব বর্ণ *s, ধ্রুব বর্ণ *p);


/* maximum recursion depth for 'match' */
#যদি !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


স্থির পূর্ণাঙ্ক check_capture (MatchState *ms, পূর্ণাঙ্ক l) {
  l -= '1';
  যদি (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    প্রত্যাগম luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  প্রত্যাগম l;
}


স্থির পূর্ণাঙ্ক capture_to_close (MatchState *ms) {
  পূর্ণাঙ্ক level = ms->level;
  প্রতি (level--; level>=0; level--)
    যদি (ms->capture[level].len == CAP_UNFINISHED) প্রত্যাগম level;
  প্রত্যাগম luaL_error(ms->L, "invalid pattern capture");
}


স্থির ধ্রুব বর্ণ *classend (MatchState *ms, ধ্রুব বর্ণ *p) {
  বিকল্প (*p++) {
    পক্ষ L_ESC: {
      যদি (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      প্রত্যাগম p+1;
    }
    পক্ষ '[': {
      যদি (*p == '^') p++;
      কুরু {  /* look for a ']' */
        যদি (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        যদি (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } যাবত্ (*p != ']');
      প্রত্যাগম p+1;
    }
    স্বভাব: {
      প্রত্যাগম p;
    }
  }
}


স্থির পূর্ণাঙ্ক match_class (পূর্ণাঙ্ক c, পূর্ণাঙ্ক cl) {
  পূর্ণাঙ্ক res;
  বিকল্প (tolower(cl)) {
    পক্ষ 'a' : res = isalpha(c); ভঙ্গ;
    পক্ষ 'c' : res = iscntrl(c); ভঙ্গ;
    পক্ষ 'd' : res = isdigit(c); ভঙ্গ;
    পক্ষ 'g' : res = isgraph(c); ভঙ্গ;
    পক্ষ 'l' : res = islower(c); ভঙ্গ;
    পক্ষ 'p' : res = ispunct(c); ভঙ্গ;
    পক্ষ 's' : res = isspace(c); ভঙ্গ;
    পক্ষ 'u' : res = isupper(c); ভঙ্গ;
    পক্ষ 'w' : res = isalnum(c); ভঙ্গ;
    পক্ষ 'x' : res = isxdigit(c); ভঙ্গ;
    পক্ষ 'z' : res = (c == 0); ভঙ্গ;  /* deprecated option */
    স্বভাব: প্রত্যাগম (cl == c);
  }
  প্রত্যাগম (islower(cl) ? res : !res);
}


স্থির পূর্ণাঙ্ক matchbracketclass (পূর্ণাঙ্ক c, ধ্রুব বর্ণ *p, ধ্রুব বর্ণ *ec) {
  পূর্ণাঙ্ক sig = 1;
  যদি (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  যাবত্ (++p < ec) {
    যদি (*p == L_ESC) {
      p++;
      যদি (match_class(c, cast_uchar(*p)))
        প্রত্যাগম sig;
    }
    অন্যথা যদি ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      যদি (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        প্রত্যাগম sig;
    }
    অন্যথা যদি (cast_uchar(*p) == c) প্রত্যাগম sig;
  }
  প্রত্যাগম !sig;
}


স্থির পূর্ণাঙ্ক singlematch (MatchState *ms, ধ্রুব বর্ণ *s, ধ্রুব বর্ণ *p,
                        ধ্রুব বর্ণ *ep) {
  যদি (s >= ms->src_end)
    প্রত্যাগম 0;
  অন্যথা {
    পূর্ণাঙ্ক c = cast_uchar(*s);
    বিকল্প (*p) {
      পক্ষ '.': প্রত্যাগম 1;  /* matches any char */
      পক্ষ L_ESC: প্রত্যাগম match_class(c, cast_uchar(*(p+1)));
      পক্ষ '[': প্রত্যাগম matchbracketclass(c, p, ep-1);
      স্বভাব:  প্রত্যাগম (cast_uchar(*p) == c);
    }
  }
}


স্থির ধ্রুব বর্ণ *matchbalance (MatchState *ms, ধ্রুব বর্ণ *s,
                                   ধ্রুব বর্ণ *p) {
  যদি (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  যদি (*s != *p) প্রত্যাগম রিক্ত;
  অন্যথা {
    পূর্ণাঙ্ক b = *p;
    পূর্ণাঙ্ক e = *(p+1);
    পূর্ণাঙ্ক cont = 1;
    যাবত্ (++s < ms->src_end) {
      যদি (*s == e) {
        যদি (--cont == 0) প্রত্যাগম s+1;
      }
      অন্যথা যদি (*s == b) cont++;
    }
  }
  প্রত্যাগম রিক্ত;  /* string ends out of balance */
}


স্থির ধ্রুব বর্ণ *max_expand (MatchState *ms, ধ্রুব বর্ণ *s,
                                 ধ্রুব বর্ণ *p, ধ্রুব বর্ণ *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  যাবত্ (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  যাবত্ (i>=0) {
    ধ্রুব বর্ণ *res = match(ms, (s+i), ep+1);
    যদি (res) প্রত্যাগম res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  প্রত্যাগম রিক্ত;
}


স্থির ধ্রুব বর্ণ *min_expand (MatchState *ms, ধ্রুব বর্ণ *s,
                                 ধ্রুব বর্ণ *p, ধ্রুব বর্ণ *ep) {
  প্রতি (;;) {
    ধ্রুব বর্ণ *res = match(ms, s, ep+1);
    যদি (res != রিক্ত)
      প্রত্যাগম res;
    অন্যথা যদি (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    অন্যথা প্রত্যাগম রিক্ত;
  }
}


স্থির ধ্রুব বর্ণ *start_capture (MatchState *ms, ধ্রুব বর্ণ *s,
                                    ধ্রুব বর্ণ *p, পূর্ণাঙ্ক what) {
  ধ্রুব বর্ণ *res;
  পূর্ণাঙ্ক level = ms->level;
  যদি (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  যদি ((res=match(ms, s, p)) == রিক্ত)  /* match failed? */
    ms->level--;  /* undo capture */
  প্রত্যাগম res;
}


স্থির ধ্রুব বর্ণ *end_capture (MatchState *ms, ধ্রুব বর্ণ *s,
                                  ধ্রুব বর্ণ *p) {
  পূর্ণাঙ্ক l = capture_to_close(ms);
  ধ্রুব বর্ণ *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  যদি ((res = match(ms, s, p)) == রিক্ত)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  প্রত্যাগম res;
}


স্থির ধ্রুব বর্ণ *match_capture (MatchState *ms, ধ্রুব বর্ণ *s, পূর্ণাঙ্ক l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  যদি ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    প্রত্যাগম s+len;
  অন্যথা প্রত্যাগম রিক্ত;
}


স্থির ধ্রুব বর্ণ *match (MatchState *ms, ধ্রুব বর্ণ *s, ধ্রুব বর্ণ *p) {
  যদি (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  যদি (p != ms->p_end) {  /* end of pattern? */
    বিকল্প (*p) {
      পক্ষ '(': {  /* start capture */
        যদি (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        অন্যথা
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        ভঙ্গ;
      }
      পক্ষ ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        ভঙ্গ;
      }
      পক্ষ '$': {
        যদি ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          গচ্ছ dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : রিক্ত;  /* check end of string */
        ভঙ্গ;
      }
      পক্ষ L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        বিকল্প (*(p + 1)) {
          পক্ষ 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            যদি (s != রিক্ত) {
              p += 4; গচ্ছ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            ভঙ্গ;
          }
          পক্ষ 'f': {  /* frontier? */
            ধ্রুব বর্ণ *ep; বর্ণ previous;
            p += 2;
            যদি (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            যদি (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; গচ্ছ init;  /* return match(ms, s, ep); */
            }
            s = রিক্ত;  /* match failed */
            ভঙ্গ;
          }
          পক্ষ '0': পক্ষ '1': পক্ষ '2': পক্ষ '3':
          পক্ষ '4': পক্ষ '5': পক্ষ '6': পক্ষ '7':
          পক্ষ '8': পক্ষ '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            যদি (s != রিক্ত) {
              p += 2; গচ্ছ init;  /* return match(ms, s, p + 2) */
            }
            ভঙ্গ;
          }
          স্বভাব: গচ্ছ dflt;
        }
        ভঙ্গ;
      }
      স্বভাব: dflt: {  /* pattern class plus optional suffix */
        ধ্রুব বর্ণ *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        যদি (!singlematch(ms, s, p, ep)) {
          যদি (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; গচ্ছ init;  /* return match(ms, s, ep + 1); */
          }
          অন্যথা  /* '+' or no suffix */
            s = রিক্ত;  /* fail */
        }
        অন্যথা {  /* matched once */
          বিকল্প (*ep) {  /* handle optional suffix */
            পক্ষ '?': {  /* optional */
              ধ্রুব বর্ণ *res;
              যদি ((res = match(ms, s + 1, ep + 1)) != রিক্ত)
                s = res;
              অন্যথা {
                p = ep + 1; গচ্ছ init;  /* else return match(ms, s, ep + 1); */
              }
              ভঙ্গ;
            }
            পক্ষ '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            পক্ষ '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              ভঙ্গ;
            পক্ষ '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              ভঙ্গ;
            স্বভাব:  /* no suffix */
              s++; p = ep; গচ্ছ init;  /* return match(ms, s + 1, ep); */
          }
        }
        ভঙ্গ;
      }
    }
  }
  ms->matchdepth++;
  প্রত্যাগম s;
}



স্থির ধ্রুব বর্ণ *lmemfind (ধ্রুব বর্ণ *s1, size_t l1,
                               ধ্রুব বর্ণ *s2, size_t l2) {
  যদি (l2 == 0) প্রত্যাগম s1;  /* empty strings are everywhere */
  অন্যথা যদি (l2 > l1) প্রত্যাগম রিক্ত;  /* avoids a negative 'l1' */
  অন্যথা {
    ধ্রুব বর্ণ *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    যাবত্ (l1 > 0 && (init = (ধ্রুব বর্ণ *)memchr(s1, *s2, l1)) != রিক্ত) {
      init++;   /* 1st char is already checked */
      যদি (memcmp(init, s2+1, l2) == 0)
        প্রত্যাগম init-1;
      অন্যথা {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    প্রত্যাগম রিক্ত;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
স্থির ptrdiff_t get_onecapture (MatchState *ms, পূর্ণাঙ্ক i, ধ্রুব বর্ণ *s,
                              ধ্রুব বর্ণ *e, ধ্রুব বর্ণ **cap) {
  যদি (i >= ms->level) {
    যদি (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    প্রত্যাগম (e - s);
  }
  অন্যথা {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    যদি (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    অন্যথা যদি (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    প্রত্যাগম capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
স্থির শূন্য ঠেলো_onecapture (MatchState *ms, পূর্ণাঙ্ক i, ধ্রুব বর্ণ *s,
                                                    ধ্রুব বর্ণ *e) {
  ধ্রুব বর্ণ *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  যদি (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


স্থির পূর্ণাঙ্ক ঠেলো_captures (MatchState *ms, ধ্রুব বর্ণ *s, ধ্রুব বর্ণ *e) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  প্রতি (i = 0; i < nlevels; i++)
    ঠেলো_onecapture(ms, i, s, e);
  প্রত্যাগম nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
স্থির পূর্ণাঙ্ক nospecials (ধ্রুব বর্ণ *p, size_t l) {
  size_t upto = 0;
  কুরু {
    যদি (strpbrk(p + upto, SPECIALS))
      প্রত্যাগম 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } যাবত্ (upto <= l);
  প্রত্যাগম 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
স্থির শূন্য prepstate (MatchState *ms, lua_State *L,
                       ধ্রুব বর্ণ *s, size_t ls, ধ্রুব বর্ণ *p, size_t lp) {
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


স্থির পূর্ণাঙ্ক str_find_aux (lua_State *L, পূর্ণাঙ্ক find) {
  size_t ls, lp;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &ls);
  ধ্রুব বর্ণ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  যদি (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    প্রত্যাগম 1;
  }
  /* explicit request or no special characters? */
  যদি (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ধ্রুব বর্ণ *s2 = lmemfind(s + init, ls - init, p, lp);
    যদি (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      প্রত্যাগম 2;
    }
  }
  অন্যথা {
    MatchState ms;
    ধ্রুব বর্ণ *s1 = s + init;
    পূর্ণাঙ্ক anchor = (*p == '^');
    যদি (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    কুরু {
      ধ্রুব বর্ণ *res;
      reprepstate(&ms);
      যদি ((res=match(&ms, s1, p)) != রিক্ত) {
        যদি (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          প্রত্যাগম ঠেলো_captures(&ms, রিক্ত, 0) + 2;
        }
        অন্যথা
          প্রত্যাগম ঠেলো_captures(&ms, s1, res);
      }
    } যাবত্ (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  প্রত্যাগম 1;
}


স্থির পূর্ণাঙ্ক str_find (lua_State *L) {
  প্রত্যাগম str_find_aux(L, 1);
}


স্থির পূর্ণাঙ্ক str_match (lua_State *L) {
  প্রত্যাগম str_find_aux(L, 0);
}


/* state for 'gmatch' */
প্রকারনির্দেশ সংরচনা GMatchState {
  ধ্রুব বর্ণ *src;  /* current position */
  ধ্রুব বর্ণ *p;  /* pattern */
  ধ্রুব বর্ণ *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


স্থির পূর্ণাঙ্ক gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ধ্রুব বর্ণ *src;
  gm->ms.L = L;
  প্রতি (src = gm->src; src <= gm->ms.src_end; src++) {
    ধ্রুব বর্ণ *e;
    reprepstate(&gm->ms);
    যদি ((e = match(&gm->ms, src, gm->p)) != রিক্ত && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      প্রত্যাগম ঠেলো_captures(&gm->ms, src, e);
    }
  }
  প্রত্যাগম 0;  /* not found */
}


স্থির পূর্ণাঙ্ক gmatch (lua_State *L) {
  size_t ls, lp;
  ধ্রুব বর্ণ *s = luaL_checklstring(L, 1, &ls);
  ধ্রুব বর্ণ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, পরিমাণ(GMatchState), 0);
  যদি (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = রিক্ত;
  lua_pushcclosure(L, gmatch_aux, 3);
  প্রত্যাগম 1;
}


স্থির শূন্য add_s (MatchState *ms, luaL_Buffer *b, ধ্রুব বর্ণ *s,
                                                   ধ্রুব বর্ণ *e) {
  size_t l;
  lua_State *L = ms->L;
  ধ্রুব বর্ণ *news = lua_tolstring(L, 3, &l);
  ধ্রুব বর্ণ *p;
  যাবত্ ((p = (বর্ণ *)memchr(news, L_ESC, l)) != রিক্ত) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    যদি (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    অন্যথা যদি (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    অন্যথা যদি (isdigit(cast_uchar(*p))) {  /* '%n' */
      ধ্রুব বর্ণ *cap;
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
স্থির পূর্ণাঙ্ক add_মান (MatchState *ms, luaL_Buffer *b, ধ্রুব বর্ণ *s,
                                      ধ্রুব বর্ণ *e, পূর্ণাঙ্ক tr) {
  lua_State *L = ms->L;
  বিকল্প (tr) {
    পক্ষ LUA_TFUNCTION: {  /* call the function */
      পূর্ণাঙ্ক n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ঠেলো_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      ভঙ্গ;
    }
    পক্ষ LUA_TTABLE: {  /* index the table */
      ঠেলো_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      ভঙ্গ;
    }
    স্বভাব: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      প্রত্যাগম 1;  /* something changed */
    }
  }
  যদি (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_বের_করো(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    প্রত্যাগম 0;  /* no changes */
  }
  অন্যথা যদি (l_unlikely(!lua_isstring(L, -1)))
    প্রত্যাগম luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  অন্যথা {
    luaL_addvalue(b);  /* add result to accumulator */
    প্রত্যাগম 1;  /* something changed */
  }
}


স্থির পূর্ণাঙ্ক str_gsub (lua_State *L) {
  size_t srcl, lp;
  ধ্রুব বর্ণ *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ধ্রুব বর্ণ *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  ধ্রুব বর্ণ *lastmatch = রিক্ত;  /* end of last match */
  পূর্ণাঙ্ক tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  পূর্ণাঙ্ক anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  পূর্ণাঙ্ক changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  যদি (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  যাবত্ (n < max_s) {
    ধ্রুব বর্ণ *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    যদি ((e = match(&ms, src, p)) != রিক্ত && e != lastmatch) {  /* match? */
      n++;
      changed = add_মান(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    অন্যথা যদি (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    অন্যথা ভঙ্গ;  /* end of subject */
    যদি (anchor) ভঙ্গ;
  }
  যদি (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  অন্যথা {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  প্রত্যাগম 2;
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

#define SIZELENMOD	(পরিমাণ(LUA_সংখ্যা_FRMLEN)/পরিমাণ(বর্ণ))


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
স্থির lua_সংখ্যা adddigit (বর্ণ *buff, unsigned n, lua_সংখ্যা x) {
  lua_সংখ্যা dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  পূর্ণাঙ্ক d = (পূর্ণাঙ্ক)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  প্রত্যাগম x - dd;  /* return what is left */
}


স্থির পূর্ণাঙ্ক num2straux (বর্ণ *buff, unsigned sz, lua_সংখ্যা x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  যদি (x != x || x == (lua_সংখ্যা)HUGE_VAL || x == -(lua_সংখ্যা)HUGE_VAL)
    প্রত্যাগম l_sprintf(buff, sz, LUA_সংখ্যা_FMT, (LUAI_UACNUMBER)x);
  অন্যথা যদি (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    প্রত্যাগম l_sprintf(buff, sz, LUA_সংখ্যা_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  অন্যথা {
    পূর্ণাঙ্ক e;
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
      কুরু {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } যাবত্ (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    প্রত্যাগম cast_int(n);
  }
}


স্থির পূর্ণাঙ্ক lua_number2strx (lua_State *L, বর্ণ *buff, unsigned sz,
                            ধ্রুব বর্ণ *fmt, lua_সংখ্যা x) {
  পূর্ণাঙ্ক n = num2straux(buff, sz, x);
  যদি (fmt[SIZELENMOD] == 'A') {
    পূর্ণাঙ্ক i;
    প্রতি (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  অন্যথা যদি (l_unlikely(fmt[SIZELENMOD] != 'a'))
    প্রত্যাগম luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  প্রত্যাগম n;
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


স্থির শূন্য addquoted (luaL_Buffer *b, ধ্রুব বর্ণ *s, size_t len) {
  luaL_addchar(b, '"');
  যাবত্ (len--) {
    যদি (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    অন্যথা যদি (iscntrl(cast_uchar(*s))) {
      বর্ণ buff[10];
      যদি (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, পরিমাণ(buff), "\\%d", (পূর্ণাঙ্ক)cast_uchar(*s));
      অন্যথা
        l_sprintf(buff, পরিমাণ(buff), "\\%03d", (পূর্ণাঙ্ক)cast_uchar(*s));
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
স্থির পূর্ণাঙ্ক quotefloat (lua_State *L, বর্ণ *buff, lua_সংখ্যা n) {
  ধ্রুব বর্ণ *s;  /* for the fixed representations */
  যদি (n == (lua_সংখ্যা)HUGE_VAL)  /* inf? */
    s = "1e9999";
  অন্যথা যদি (n == -(lua_সংখ্যা)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  অন্যথা যদি (n != n)  /* NaN? */
    s = "(0/0)";
  অন্যথা {  /* format number as hexadecimal */
    পূর্ণাঙ্ক  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_সংখ্যা_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    যদি (memchr(buff, '.', cast_uint(nb)) == রিক্ত) {  /* no dot? */
      বর্ণ point = lua_getlocaledecpoint();  /* try locale point */
      বর্ণ *ppoint = (বর্ণ *)memchr(buff, point, cast_uint(nb));
      যদি (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    প্রত্যাগম nb;
  }
  /* for the fixed representations */
  প্রত্যাগম l_sprintf(buff, MAX_ITEM, "%s", s);
}


স্থির শূন্য addliteral (lua_State *L, luaL_Buffer *b, পূর্ণাঙ্ক arg) {
  বিকল্প (lua_type(L, arg)) {
    পক্ষ LUA_TSTRING: {
      size_t len;
      ধ্রুব বর্ণ *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      ভঙ্গ;
    }
    পক্ষ LUA_TNUMBER: {
      বর্ণ *buff = luaL_prepbuffsize(b, MAX_ITEM);
      পূর্ণাঙ্ক nb;
      যদি (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      অন্যথা {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ধ্রুব বর্ণ *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      ভঙ্গ;
    }
    পক্ষ LUA_TNIL: পক্ষ LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, রিক্ত);
      luaL_addvalue(b);
      ভঙ্গ;
    }
    স্বভাব: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


স্থির ধ্রুব বর্ণ *get2digits (ধ্রুব বর্ণ *s) {
  যদি (isdigit(cast_uchar(*s))) {
    s++;
    যদি (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  প্রত্যাগম s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
স্থির শূন্য checkformat (lua_State *L, ধ্রুব বর্ণ *form, ধ্রুব বর্ণ *flags,
                                       পূর্ণাঙ্ক precision) {
  ধ্রুব বর্ণ *spec = form + 1;  /* skip '%' */
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
স্থির ধ্রুব বর্ণ *getformat (lua_State *L, ধ্রুব বর্ণ *strfrmt,
                                            বর্ণ *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  যদি (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * পরিমাণ(বর্ণ));
  *(form + len) = '\0';
  প্রত্যাগম strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
স্থির শূন্য addlenmod (বর্ণ *form, ধ্রুব বর্ণ *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  বর্ণ spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


স্থির পূর্ণাঙ্ক str_format (lua_State *L) {
  পূর্ণাঙ্ক শীর্ষ = lua_gettop(L);
  পূর্ণাঙ্ক arg = 1;
  size_t sfl;
  ধ্রুব বর্ণ *strfrmt = luaL_checklstring(L, arg, &sfl);
  ধ্রুব বর্ণ *strfrmt_end = strfrmt+sfl;
  ধ্রুব বর্ণ *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  যাবত্ (strfrmt < strfrmt_end) {
    যদি (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    অন্যথা যদি (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    অন্যথা { /* format item */
      বর্ণ form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      বর্ণ *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      পূর্ণাঙ্ক nb = 0;  /* number of bytes in result */
      যদি (++arg > শীর্ষ)
        প্রত্যাগম luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      বিকল্প (*strfrmt++) {
        পক্ষ 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (পূর্ণাঙ্ক)luaL_checkinteger(L, arg));
          ভঙ্গ;
        }
        পক্ষ 'd': পক্ষ 'i':
          flags = L_FMTFLAGSI;
          গচ্ছ intcase;
        পক্ষ 'u':
          flags = L_FMTFLAGSU;
          গচ্ছ intcase;
        পক্ষ 'o': পক্ষ 'x': পক্ষ 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          ভঙ্গ;
        }
        পক্ষ 'a': পক্ষ 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_সংখ্যা_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          ভঙ্গ;
        পক্ষ 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        পক্ষ 'e': পক্ষ 'E': পক্ষ 'g': পক্ষ 'G': {
          lua_সংখ্যা n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_সংখ্যা_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          ভঙ্গ;
        }
        পক্ষ 'p': {
          ধ্রুব শূন্য *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          যদি (p == রিক্ত) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          ভঙ্গ;
        }
        পক্ষ 'q': {
          যদি (form[2] != '\0')  /* modifiers? */
            প্রত্যাগম luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          ভঙ্গ;
        }
        পক্ষ 's': {
          size_t l;
          ধ্রুব বর্ণ *s = luaL_tolstring(L, arg, &l);
          যদি (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          অন্যথা {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            যদি (strchr(form, '.') == রিক্ত && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            অন্যথা {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_বের_করো(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          ভঙ্গ;
        }
        স্বভাব: {  /* also treat cases 'pnLlh' */
          প্রত্যাগম luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  প্রত্যাগম 1;
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
#define SZINT	((পূর্ণাঙ্ক)পরিমাণ(lua_Integer))


/* dummy union to get native endianness */
স্থির ধ্রুব সঙ্ঘ {
  পূর্ণাঙ্ক dummy;
  বর্ণ little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
প্রকারনির্দেশ সংরচনা Header {
  lua_State *L;
  পূর্ণাঙ্ক islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
প্রকারনির্দেশ গণনা KOption {
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
স্থির পূর্ণাঙ্ক digit (পূর্ণাঙ্ক c) { প্রত্যাগম '0' <= c && c <= '9'; }

স্থির size_t getnum (ধ্রুব বর্ণ **fmt, size_t df) {
  যদি (!digit(**fmt))  /* no number? */
    প্রত্যাগম df;  /* return default value */
  অন্যথা {
    size_t a = 0;
    কুরু {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } যাবত্ (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    প্রত্যাগম a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
স্থির unsigned getnumlimit (Header *h, ধ্রুব বর্ণ **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  যদি (l_unlikely((sz - 1u) >= MAXINTSIZE))
    প্রত্যাগম cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  প্রত্যাগম cast_uint(sz);
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
স্থির KOption getoption (Header *h, ধ্রুব বর্ণ **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  সংরচনা cD { বর্ণ c; সঙ্ঘ { LUAI_MAXALIGN; } u; };
  পূর্ণাঙ্ক opt = *((*fmt)++);
  *size = 0;  /* default */
  বিকল্প (opt) {
    পক্ষ 'b': *size = পরিমাণ(বর্ণ); প্রত্যাগম Kint;
    পক্ষ 'B': *size = পরিমাণ(বর্ণ); প্রত্যাগম Kuint;
    পক্ষ 'h': *size = পরিমাণ(short); প্রত্যাগম Kint;
    পক্ষ 'H': *size = পরিমাণ(short); প্রত্যাগম Kuint;
    পক্ষ 'l': *size = পরিমাণ(long); প্রত্যাগম Kint;
    পক্ষ 'L': *size = পরিমাণ(long); প্রত্যাগম Kuint;
    পক্ষ 'j': *size = পরিমাণ(lua_Integer); প্রত্যাগম Kint;
    পক্ষ 'J': *size = পরিমাণ(lua_Integer); প্রত্যাগম Kuint;
    পক্ষ 'T': *size = পরিমাণ(size_t); প্রত্যাগম Kuint;
    পক্ষ 'f': *size = পরিমাণ(ভিন্ন); প্রত্যাগম Kfloat;
    পক্ষ 'n': *size = পরিমাণ(lua_সংখ্যা); প্রত্যাগম Knumber;
    পক্ষ 'd': *size = পরিমাণ(দ্বিগুণ); প্রত্যাগম Kdouble;
    পক্ষ 'i': *size = getnumlimit(h, fmt, পরিমাণ(পূর্ণাঙ্ক)); প্রত্যাগম Kint;
    পক্ষ 'I': *size = getnumlimit(h, fmt, পরিমাণ(পূর্ণাঙ্ক)); প্রত্যাগম Kuint;
    পক্ষ 's': *size = getnumlimit(h, fmt, পরিমাণ(size_t)); প্রত্যাগম Kstring;
    পক্ষ 'c':
      *size = getnum(fmt, cast_sizet(-1));
      যদি (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      প্রত্যাগম Kchar;
    পক্ষ 'z': প্রত্যাগম Kzstr;
    পক্ষ 'x': *size = 1; প্রত্যাগম Kpadding;
    পক্ষ 'X': প্রত্যাগম Kpaddalign;
    পক্ষ ' ': ভঙ্গ;
    পক্ষ '<': h->islittle = 1; ভঙ্গ;
    পক্ষ '>': h->islittle = 0; ভঙ্গ;
    পক্ষ '=': h->islittle = nativeendian.little; ভঙ্গ;
    পক্ষ '!': {
      ধ্রুব size_t maxalign = offsetof(সংরচনা cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      ভঙ্গ;
    }
    স্বভাব: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  প্রত্যাগম Knop;
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
স্থির KOption getdetails (Header *h, size_t totalsize, ধ্রুব বর্ণ **fmt,
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
  প্রত্যাগম opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
স্থির শূন্য packint (luaL_Buffer *b, lua_Unsigned n,
                     পূর্ণাঙ্ক islittle, unsigned size, পূর্ণাঙ্ক neg) {
  বর্ণ *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (বর্ণ)(n & MC);  /* first byte */
  প্রতি (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (বর্ণ)(n & MC);
  }
  যদি (neg && size > SZINT) {  /* negative number need sign extension? */
    প্রতি (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (বর্ণ)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
স্থির শূন্য copywithendian (বর্ণ *dest, ধ্রুব বর্ণ *src,
                            unsigned size, পূর্ণাঙ্ক islittle) {
  যদি (islittle == nativeendian.little)
    memcpy(dest, src, size);
  অন্যথা {
    dest += size - 1;
    যাবত্ (size-- != 0)
      *(dest--) = *(src++);
  }
}


স্থির পূর্ণাঙ্ক str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  ধ্রুব বর্ণ *fmt = luaL_checkstring(L, 1);  /* format string */
  পূর্ণাঙ্ক arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  যাবত্ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    যাবত্ (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    বিকল্প (opt) {
      পক্ষ Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        যদি (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        ভঙ্গ;
      }
      পক্ষ Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        যদি (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        ভঙ্গ;
      }
      পক্ষ Kfloat: {  /* C float */
        ভিন্ন f = (ভিন্ন)luaL_checknumber(L, arg);  /* get argument */
        বর্ণ *buff = luaL_prepbuffsize(&b, পরিমাণ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (বর্ণ *)&f, পরিমাণ(f), h.islittle);
        luaL_addsize(&b, size);
        ভঙ্গ;
      }
      পক্ষ Knumber: {  /* Lua float */
        lua_সংখ্যা f = luaL_checknumber(L, arg);  /* get argument */
        বর্ণ *buff = luaL_prepbuffsize(&b, পরিমাণ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (বর্ণ *)&f, পরিমাণ(f), h.islittle);
        luaL_addsize(&b, size);
        ভঙ্গ;
      }
      পক্ষ Kdouble: {  /* C double */
        দ্বিগুণ f = (দ্বিগুণ)luaL_checknumber(L, arg);  /* get argument */
        বর্ণ *buff = luaL_prepbuffsize(&b, পরিমাণ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (বর্ণ *)&f, পরিমাণ(f), h.islittle);
        luaL_addsize(&b, size);
        ভঙ্গ;
      }
      পক্ষ Kchar: {  /* fixed-size string */
        size_t len;
        ধ্রুব বর্ণ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        যদি (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          বর্ণ *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        ভঙ্গ;
      }
      পক্ষ Kstring: {  /* strings with length count */
        size_t len;
        ধ্রুব বর্ণ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= পরিমাণ(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        ভঙ্গ;
      }
      পক্ষ Kzstr: {  /* zero-terminated string */
        size_t len;
        ধ্রুব বর্ণ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        ভঙ্গ;
      }
      পক্ষ Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      পক্ষ Kpaddalign: পক্ষ Knop:
        arg--;  /* undo increment */
        ভঙ্গ;
    }
  }
  luaL_pushresult(&b);
  প্রত্যাগম 1;
}


স্থির পূর্ণাঙ্ক str_packsize (lua_State *L) {
  Header h;
  ধ্রুব বর্ণ *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  যাবত্ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    size += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_SIZE - size,
                     1, "format result too large");
    totalsize += size;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  প্রত্যাগম 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
স্থির lua_Integer unpackint (lua_State *L, ধ্রুব বর্ণ *str,
                              পূর্ণাঙ্ক islittle, পূর্ণাঙ্ক size, পূর্ণাঙ্ক issigned) {
  lua_Unsigned res = 0;
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক limit = (size  <= SZINT) ? size : SZINT;
  প্রতি (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned বর্ণ)str[islittle ? i : size - 1 - i];
  }
  যদি (size < SZINT) {  /* real size smaller than lua_Integer? */
    যদি (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  অন্যথা যদি (size > SZINT) {  /* must check unread bytes */
    পূর্ণাঙ্ক mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    প্রতি (i = limit; i < size; i++) {
      যদি (l_unlikely((unsigned বর্ণ)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  প্রত্যাগম (lua_Integer)res;
}


স্থির পূর্ণাঙ্ক str_unpack (lua_State *L) {
  Header h;
  ধ্রুব বর্ণ *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ধ্রুব বর্ণ *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  পূর্ণাঙ্ক n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  যাবত্ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    বিকল্প (opt) {
      পক্ষ Kint:
      পক্ষ Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        ভঙ্গ;
      }
      পক্ষ Kfloat: {
        ভিন্ন f;
        copywithendian((বর্ণ *)&f, data + pos, পরিমাণ(f), h.islittle);
        lua_pushnumber(L, (lua_সংখ্যা)f);
        ভঙ্গ;
      }
      পক্ষ Knumber: {
        lua_সংখ্যা f;
        copywithendian((বর্ণ *)&f, data + pos, পরিমাণ(f), h.islittle);
        lua_pushnumber(L, f);
        ভঙ্গ;
      }
      পক্ষ Kdouble: {
        দ্বিগুণ f;
        copywithendian((বর্ণ *)&f, data + pos, পরিমাণ(f), h.islittle);
        lua_pushnumber(L, (lua_সংখ্যা)f);
        ভঙ্গ;
      }
      পক্ষ Kchar: {
        lua_pushlstring(L, data + pos, size);
        ভঙ্গ;
      }
      পক্ষ Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        ভঙ্গ;
      }
      পক্ষ Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        ভঙ্গ;
      }
      পক্ষ Kpaddalign: পক্ষ Kpadding: পক্ষ Knop:
        n--;  /* undo increment */
        ভঙ্গ;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  প্রত্যাগম n + 1;
}

/* }====================================================== */


স্থির ধ্রুব luaL_Reg strlib[] = {
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
  {রিক্ত, রিক্ত}
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
LUAMOD_API পূর্ণাঙ্ক luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  প্রত্যাগম 1;
}

