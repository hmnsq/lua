/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <ctype.h>
#अन्तर्भाव <float.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <locale.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#जइ !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


स्थिर पूर्णाङ्क str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  प्रत्यागम 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
स्थिर size_t posrelatI (lua_Integer pos, size_t len) {
  जइ (pos > 0)
    प्रत्यागम (size_t)pos;
  अण्णहा जइ (pos == 0)
    प्रत्यागम 1;
  अण्णहा जइ (pos < -(lua_Integer)len)  /* inverted comparison */
    प्रत्यागम 1;  /* clip to 1 */
  अण्णहा प्रत्यागम len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
स्थिर size_t getendpos (lua_State *L, पूर्णाङ्क arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  जइ (pos > (lua_Integer)len)
    प्रत्यागम len;
  अण्णहा जइ (pos >= 0)
    प्रत्यागम (size_t)pos;
  अण्णहा जइ (pos < -(lua_Integer)len)
    प्रत्यागम 0;
  अण्णहा प्रत्यागम len + (size_t)pos + 1;
}


स्थिर पूर्णाङ्क str_sub (lua_State *L) {
  size_t l;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  जइ (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  अण्णहा lua_pushliteral(L, "");
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  वर्ण *p = luaL_buffinitsize(L, &b, l);
  प्रति (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  वर्ण *p = luaL_buffinitsize(L, &b, l);
  प्रति (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  वर्ण *p = luaL_buffinitsize(L, &b, l);
  प्रति (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  प्रत्यागम 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
स्थिर पूर्णाङ्क str_rep (lua_State *L) {
  size_t len, lsep;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ध्रुव वर्ण *sep = luaL_optlstring(L, 3, "", &lsep);
  जइ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  अण्णहा जइ (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    प्रत्यागम luaL_error(L, "resulting string too large");
  अण्णहा {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    वर्ण *p = luaL_buffinitsize(L, &b, totallen);
    जाव (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * परिमाण(वर्ण)); p += len;
      जइ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * परिमाण(वर्ण)); p += lsep;
      }
    }
    memcpy(p, s, len * परिमाण(वर्ण));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_byte (lua_State *L) {
  size_t l;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  पूर्णाङ्क n, i;
  जइ (posi > pose) प्रत्यागम 0;  /* empty interval; return no values */
  जइ (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    प्रत्यागम luaL_error(L, "string slice too long");
  n = (पूर्णाङ्क)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  प्रति (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  प्रत्यागम n;
}


स्थिर पूर्णाङ्क str_char (lua_State *L) {
  पूर्णाङ्क n = lua_gettop(L);  /* number of arguments */
  पूर्णाङ्क i;
  luaL_Buffer b;
  वर्ण *p = luaL_buffinitsize(L, &b, cast_uint(n));
  प्रति (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  प्रत्यागम 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
संरचना str_Writer {
  पूर्णाङ्क init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


स्थिर पूर्णाङ्क writer (lua_State *L, ध्रुव शून्य *b, size_t size, शून्य *ud) {
  संरचना str_Writer *state = (संरचना str_Writer *)ud;
  जइ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  जइ (b == रिक्त) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  अण्णहा
    luaL_addlstring(&state->B, (ध्रुव वर्ण *)b, size);
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क str_dump (lua_State *L) {
  संरचना str_Writer state;
  पूर्णाङ्क strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  प्रत्यागम 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#जइ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

स्थिर ध्रुव luaL_Reg stringmetamethods[] = {
  {"__index", रिक्त},  /* placeholder */
  {रिक्त, रिक्त}
};

#अण्णहा		/* }{ */

स्थिर पूर्णाङ्क tonum (lua_State *L, पूर्णाङ्क arg) {
  जइ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    प्रत्यागम 1;
  }
  अण्णहा {  /* check whether it is a numerical string */
    size_t len;
    ध्रुव वर्ण *s = lua_tolstring(L, arg, &len);
    प्रत्यागम (s != रिक्त && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
स्थिर शून्य trymt (lua_State *L, ध्रुव वर्ण *mtkey, ध्रुव वर्ण *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  जइ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_निवेशय(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


स्थिर पूर्णाङ्क arith (lua_State *L, पूर्णाङ्क op, ध्रुव वर्ण *mtname) {
  जइ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  अण्णहा
    trymt(L, mtname, mtname + 2);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क arith_add (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPADD, "__add");
}

स्थिर पूर्णाङ्क arith_sub (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPSUB, "__sub");
}

स्थिर पूर्णाङ्क arith_mul (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPMUL, "__mul");
}

स्थिर पूर्णाङ्क arith_mod (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPMOD, "__mod");
}

स्थिर पूर्णाङ्क arith_pow (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPPOW, "__pow");
}

स्थिर पूर्णाङ्क arith_div (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPDIV, "__div");
}

स्थिर पूर्णाङ्क arith_idiv (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPIDIV, "__idiv");
}

स्थिर पूर्णाङ्क arith_unm (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPUNM, "__unm");
}


स्थिर ध्रुव luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", रिक्त},  /* placeholder */
  {रिक्त, रिक्त}
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


प्रकारनिर्देश संरचना MatchState {
  ध्रुव वर्ण *src_init;  /* init of source string */
  ध्रुव वर्ण *src_end;  /* end ('\0') of source string */
  ध्रुव वर्ण *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  पूर्णाङ्क matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  पूर्णाङ्क level;  /* total number of captures (finished or unfinished) */
  संरचना {
    ध्रुव वर्ण *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
स्थिर ध्रुव वर्ण *match (MatchState *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *p);


/* maximum recursion depth for 'match' */
#जइ !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


स्थिर पूर्णाङ्क check_capture (MatchState *ms, पूर्णाङ्क l) {
  l -= '1';
  जइ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    प्रत्यागम luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  प्रत्यागम l;
}


स्थिर पूर्णाङ्क capture_to_close (MatchState *ms) {
  पूर्णाङ्क level = ms->level;
  प्रति (level--; level>=0; level--)
    जइ (ms->capture[level].len == CAP_UNFINISHED) प्रत्यागम level;
  प्रत्यागम luaL_error(ms->L, "invalid pattern capture");
}


स्थिर ध्रुव वर्ण *classend (MatchState *ms, ध्रुव वर्ण *p) {
  विकल्प (*p++) {
    पक्ष L_ESC: {
      जइ (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      प्रत्यागम p+1;
    }
    पक्ष '[': {
      जइ (*p == '^') p++;
      करेहि {  /* look for a ']' */
        जइ (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        जइ (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } जाव (*p != ']');
      प्रत्यागम p+1;
    }
    स्वभाव: {
      प्रत्यागम p;
    }
  }
}


स्थिर पूर्णाङ्क match_class (पूर्णाङ्क c, पूर्णाङ्क cl) {
  पूर्णाङ्क res;
  विकल्प (tolower(cl)) {
    पक्ष 'a' : res = isalpha(c); भङ्ग;
    पक्ष 'c' : res = iscntrl(c); भङ्ग;
    पक्ष 'd' : res = isdigit(c); भङ्ग;
    पक्ष 'g' : res = isgraph(c); भङ्ग;
    पक्ष 'l' : res = islower(c); भङ्ग;
    पक्ष 'p' : res = ispunct(c); भङ्ग;
    पक्ष 's' : res = isspace(c); भङ्ग;
    पक्ष 'u' : res = isupper(c); भङ्ग;
    पक्ष 'w' : res = isalnum(c); भङ्ग;
    पक्ष 'x' : res = isxdigit(c); भङ्ग;
    पक्ष 'z' : res = (c == 0); भङ्ग;  /* deprecated option */
    स्वभाव: प्रत्यागम (cl == c);
  }
  प्रत्यागम (islower(cl) ? res : !res);
}


स्थिर पूर्णाङ्क matchbracketclass (पूर्णाङ्क c, ध्रुव वर्ण *p, ध्रुव वर्ण *ec) {
  पूर्णाङ्क sig = 1;
  जइ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  जाव (++p < ec) {
    जइ (*p == L_ESC) {
      p++;
      जइ (match_class(c, cast_uchar(*p)))
        प्रत्यागम sig;
    }
    अण्णहा जइ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      जइ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        प्रत्यागम sig;
    }
    अण्णहा जइ (cast_uchar(*p) == c) प्रत्यागम sig;
  }
  प्रत्यागम !sig;
}


स्थिर पूर्णाङ्क singlematch (MatchState *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *p,
                        ध्रुव वर्ण *ep) {
  जइ (s >= ms->src_end)
    प्रत्यागम 0;
  अण्णहा {
    पूर्णाङ्क c = cast_uchar(*s);
    विकल्प (*p) {
      पक्ष '.': प्रत्यागम 1;  /* matches any char */
      पक्ष L_ESC: प्रत्यागम match_class(c, cast_uchar(*(p+1)));
      पक्ष '[': प्रत्यागम matchbracketclass(c, p, ep-1);
      स्वभाव:  प्रत्यागम (cast_uchar(*p) == c);
    }
  }
}


स्थिर ध्रुव वर्ण *matchbalance (MatchState *ms, ध्रुव वर्ण *s,
                                   ध्रुव वर्ण *p) {
  जइ (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  जइ (*s != *p) प्रत्यागम रिक्त;
  अण्णहा {
    पूर्णाङ्क b = *p;
    पूर्णाङ्क e = *(p+1);
    पूर्णाङ्क cont = 1;
    जाव (++s < ms->src_end) {
      जइ (*s == e) {
        जइ (--cont == 0) प्रत्यागम s+1;
      }
      अण्णहा जइ (*s == b) cont++;
    }
  }
  प्रत्यागम रिक्त;  /* string ends out of balance */
}


स्थिर ध्रुव वर्ण *max_expand (MatchState *ms, ध्रुव वर्ण *s,
                                 ध्रुव वर्ण *p, ध्रुव वर्ण *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  जाव (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  जाव (i>=0) {
    ध्रुव वर्ण *res = match(ms, (s+i), ep+1);
    जइ (res) प्रत्यागम res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  प्रत्यागम रिक्त;
}


स्थिर ध्रुव वर्ण *min_expand (MatchState *ms, ध्रुव वर्ण *s,
                                 ध्रुव वर्ण *p, ध्रुव वर्ण *ep) {
  प्रति (;;) {
    ध्रुव वर्ण *res = match(ms, s, ep+1);
    जइ (res != रिक्त)
      प्रत्यागम res;
    अण्णहा जइ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    अण्णहा प्रत्यागम रिक्त;
  }
}


स्थिर ध्रुव वर्ण *start_capture (MatchState *ms, ध्रुव वर्ण *s,
                                    ध्रुव वर्ण *p, पूर्णाङ्क what) {
  ध्रुव वर्ण *res;
  पूर्णाङ्क level = ms->level;
  जइ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  जइ ((res=match(ms, s, p)) == रिक्त)  /* match failed? */
    ms->level--;  /* undo capture */
  प्रत्यागम res;
}


स्थिर ध्रुव वर्ण *end_capture (MatchState *ms, ध्रुव वर्ण *s,
                                  ध्रुव वर्ण *p) {
  पूर्णाङ्क l = capture_to_close(ms);
  ध्रुव वर्ण *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  जइ ((res = match(ms, s, p)) == रिक्त)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  प्रत्यागम res;
}


स्थिर ध्रुव वर्ण *match_capture (MatchState *ms, ध्रुव वर्ण *s, पूर्णाङ्क l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  जइ ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    प्रत्यागम s+len;
  अण्णहा प्रत्यागम रिक्त;
}


स्थिर ध्रुव वर्ण *match (MatchState *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *p) {
  जइ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  जइ (p != ms->p_end) {  /* end of pattern? */
    विकल्प (*p) {
      पक्ष '(': {  /* start capture */
        जइ (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        अण्णहा
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        भङ्ग;
      }
      पक्ष ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        भङ्ग;
      }
      पक्ष '$': {
        जइ ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          गच्छ dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : रिक्त;  /* check end of string */
        भङ्ग;
      }
      पक्ष L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        विकल्प (*(p + 1)) {
          पक्ष 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            जइ (s != रिक्त) {
              p += 4; गच्छ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            भङ्ग;
          }
          पक्ष 'f': {  /* frontier? */
            ध्रुव वर्ण *ep; वर्ण previous;
            p += 2;
            जइ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            जइ (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; गच्छ init;  /* return match(ms, s, ep); */
            }
            s = रिक्त;  /* match failed */
            भङ्ग;
          }
          पक्ष '0': पक्ष '1': पक्ष '2': पक्ष '3':
          पक्ष '4': पक्ष '5': पक्ष '6': पक्ष '7':
          पक्ष '8': पक्ष '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            जइ (s != रिक्त) {
              p += 2; गच्छ init;  /* return match(ms, s, p + 2) */
            }
            भङ्ग;
          }
          स्वभाव: गच्छ dflt;
        }
        भङ्ग;
      }
      स्वभाव: dflt: {  /* pattern class plus optional suffix */
        ध्रुव वर्ण *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        जइ (!singlematch(ms, s, p, ep)) {
          जइ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; गच्छ init;  /* return match(ms, s, ep + 1); */
          }
          अण्णहा  /* '+' or no suffix */
            s = रिक्त;  /* fail */
        }
        अण्णहा {  /* matched once */
          विकल्प (*ep) {  /* handle optional suffix */
            पक्ष '?': {  /* optional */
              ध्रुव वर्ण *res;
              जइ ((res = match(ms, s + 1, ep + 1)) != रिक्त)
                s = res;
              अण्णहा {
                p = ep + 1; गच्छ init;  /* else return match(ms, s, ep + 1); */
              }
              भङ्ग;
            }
            पक्ष '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            पक्ष '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              भङ्ग;
            पक्ष '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              भङ्ग;
            स्वभाव:  /* no suffix */
              s++; p = ep; गच्छ init;  /* return match(ms, s + 1, ep); */
          }
        }
        भङ्ग;
      }
    }
  }
  ms->matchdepth++;
  प्रत्यागम s;
}



स्थिर ध्रुव वर्ण *lmemfind (ध्रुव वर्ण *s1, size_t l1,
                               ध्रुव वर्ण *s2, size_t l2) {
  जइ (l2 == 0) प्रत्यागम s1;  /* empty strings are everywhere */
  अण्णहा जइ (l2 > l1) प्रत्यागम रिक्त;  /* avoids a negative 'l1' */
  अण्णहा {
    ध्रुव वर्ण *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    जाव (l1 > 0 && (init = (ध्रुव वर्ण *)memchr(s1, *s2, l1)) != रिक्त) {
      init++;   /* 1st char is already checked */
      जइ (memcmp(init, s2+1, l2) == 0)
        प्रत्यागम init-1;
      अण्णहा {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    प्रत्यागम रिक्त;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
स्थिर ptrdiff_t get_onecapture (MatchState *ms, पूर्णाङ्क i, ध्रुव वर्ण *s,
                              ध्रुव वर्ण *e, ध्रुव वर्ण **cap) {
  जइ (i >= ms->level) {
    जइ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    प्रत्यागम (e - s);
  }
  अण्णहा {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    जइ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    अण्णहा जइ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    प्रत्यागम capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
स्थिर शून्य प्रक्षिप_onecapture (MatchState *ms, पूर्णाङ्क i, ध्रुव वर्ण *s,
                                                    ध्रुव वर्ण *e) {
  ध्रुव वर्ण *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  जइ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


स्थिर पूर्णाङ्क प्रक्षिप_captures (MatchState *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *e) {
  पूर्णाङ्क i;
  पूर्णाङ्क nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  प्रति (i = 0; i < nlevels; i++)
    प्रक्षिप_onecapture(ms, i, s, e);
  प्रत्यागम nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
स्थिर पूर्णाङ्क nospecials (ध्रुव वर्ण *p, size_t l) {
  size_t upto = 0;
  करेहि {
    जइ (strpbrk(p + upto, SPECIALS))
      प्रत्यागम 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } जाव (upto <= l);
  प्रत्यागम 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
स्थिर शून्य prepstate (MatchState *ms, lua_State *L,
                       ध्रुव वर्ण *s, size_t ls, ध्रुव वर्ण *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
स्थिर शून्य reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


स्थिर पूर्णाङ्क str_find_aux (lua_State *L, पूर्णाङ्क find) {
  size_t ls, lp;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &ls);
  ध्रुव वर्ण *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  जइ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    प्रत्यागम 1;
  }
  /* explicit request or no special characters? */
  जइ (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ध्रुव वर्ण *s2 = lmemfind(s + init, ls - init, p, lp);
    जइ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      प्रत्यागम 2;
    }
  }
  अण्णहा {
    MatchState ms;
    ध्रुव वर्ण *s1 = s + init;
    पूर्णाङ्क anchor = (*p == '^');
    जइ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    करेहि {
      ध्रुव वर्ण *res;
      reprepstate(&ms);
      जइ ((res=match(&ms, s1, p)) != रिक्त) {
        जइ (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          प्रत्यागम प्रक्षिप_captures(&ms, रिक्त, 0) + 2;
        }
        अण्णहा
          प्रत्यागम प्रक्षिप_captures(&ms, s1, res);
      }
    } जाव (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_find (lua_State *L) {
  प्रत्यागम str_find_aux(L, 1);
}


स्थिर पूर्णाङ्क str_match (lua_State *L) {
  प्रत्यागम str_find_aux(L, 0);
}


/* state for 'gmatch' */
प्रकारनिर्देश संरचना GMatchState {
  ध्रुव वर्ण *src;  /* current position */
  ध्रुव वर्ण *p;  /* pattern */
  ध्रुव वर्ण *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


स्थिर पूर्णाङ्क gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ध्रुव वर्ण *src;
  gm->ms.L = L;
  प्रति (src = gm->src; src <= gm->ms.src_end; src++) {
    ध्रुव वर्ण *e;
    reprepstate(&gm->ms);
    जइ ((e = match(&gm->ms, src, gm->p)) != रिक्त && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      प्रत्यागम प्रक्षिप_captures(&gm->ms, src, e);
    }
  }
  प्रत्यागम 0;  /* not found */
}


स्थिर पूर्णाङ्क gmatch (lua_State *L) {
  size_t ls, lp;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &ls);
  ध्रुव वर्ण *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, परिमाण(GMatchState), 0);
  जइ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = रिक्त;
  lua_pushcclosure(L, gmatch_aux, 3);
  प्रत्यागम 1;
}


स्थिर शून्य add_s (MatchState *ms, luaL_Buffer *b, ध्रुव वर्ण *s,
                                                   ध्रुव वर्ण *e) {
  size_t l;
  lua_State *L = ms->L;
  ध्रुव वर्ण *news = lua_tolstring(L, 3, &l);
  ध्रुव वर्ण *p;
  जाव ((p = (वर्ण *)memchr(news, L_ESC, l)) != रिक्त) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    जइ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    अण्णहा जइ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    अण्णहा जइ (isdigit(cast_uchar(*p))) {  /* '%n' */
      ध्रुव वर्ण *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      जइ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      अण्णहा
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    अण्णहा
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
स्थिर पूर्णाङ्क add_मानम् (MatchState *ms, luaL_Buffer *b, ध्रुव वर्ण *s,
                                      ध्रुव वर्ण *e, पूर्णाङ्क tr) {
  lua_State *L = ms->L;
  विकल्प (tr) {
    पक्ष LUA_TFUNCTION: {  /* call the function */
      पूर्णाङ्क n;
      lua_pushvalue(L, 3);  /* push the function */
      n = प्रक्षिप_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      भङ्ग;
    }
    पक्ष LUA_TTABLE: {  /* index the table */
      प्रक्षिप_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      भङ्ग;
    }
    स्वभाव: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      प्रत्यागम 1;  /* something changed */
    }
  }
  जइ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_निष्कासय(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    प्रत्यागम 0;  /* no changes */
  }
  अण्णहा जइ (l_unlikely(!lua_isstring(L, -1)))
    प्रत्यागम luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  अण्णहा {
    luaL_addvalue(b);  /* add result to accumulator */
    प्रत्यागम 1;  /* something changed */
  }
}


स्थिर पूर्णाङ्क str_gsub (lua_State *L) {
  size_t srcl, lp;
  ध्रुव वर्ण *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ध्रुव वर्ण *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  ध्रुव वर्ण *lastmatch = रिक्त;  /* end of last match */
  पूर्णाङ्क tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  पूर्णाङ्क anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  पूर्णाङ्क changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  जइ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  जाव (n < max_s) {
    ध्रुव वर्ण *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    जइ ((e = match(&ms, src, p)) != रिक्त && e != lastmatch) {  /* match? */
      n++;
      changed = add_मानम्(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    अण्णहा जइ (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    अण्णहा भङ्ग;  /* end of subject */
    जइ (anchor) भङ्ग;
  }
  जइ (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  अण्णहा {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  प्रत्यागम 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#जइ !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(परिमाण(LUA_सङ्ख्या_FRMLEN)/परिमाण(वर्ण))


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
स्थिर lua_सङ्ख्या adddigit (वर्ण *buff, unsigned n, lua_सङ्ख्या x) {
  lua_सङ्ख्या dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  पूर्णाङ्क d = (पूर्णाङ्क)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  प्रत्यागम x - dd;  /* return what is left */
}


स्थिर पूर्णाङ्क num2straux (वर्ण *buff, unsigned sz, lua_सङ्ख्या x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  जइ (x != x || x == (lua_सङ्ख्या)HUGE_VAL || x == -(lua_सङ्ख्या)HUGE_VAL)
    प्रत्यागम l_sprintf(buff, sz, LUA_सङ्ख्या_FMT, (LUAI_UACNUMBER)x);
  अण्णहा जइ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    प्रत्यागम l_sprintf(buff, sz, LUA_सङ्ख्या_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  अण्णहा {
    पूर्णाङ्क e;
    lua_सङ्ख्या m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    जइ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    जइ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      करेहि {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } जाव (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    प्रत्यागम cast_int(n);
  }
}


स्थिर पूर्णाङ्क lua_number2strx (lua_State *L, वर्ण *buff, unsigned sz,
                            ध्रुव वर्ण *fmt, lua_सङ्ख्या x) {
  पूर्णाङ्क n = num2straux(buff, sz, x);
  जइ (fmt[SIZELENMOD] == 'A') {
    पूर्णाङ्क i;
    प्रति (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  अण्णहा जइ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    प्रत्यागम luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  प्रत्यागम n;
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
#जइ !defined(L_FMTFLAGSF)

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


स्थिर शून्य addquoted (luaL_Buffer *b, ध्रुव वर्ण *s, size_t len) {
  luaL_addchar(b, '"');
  जाव (len--) {
    जइ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    अण्णहा जइ (iscntrl(cast_uchar(*s))) {
      वर्ण buff[10];
      जइ (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, परिमाण(buff), "\\%d", (पूर्णाङ्क)cast_uchar(*s));
      अण्णहा
        l_sprintf(buff, परिमाण(buff), "\\%03d", (पूर्णाङ्क)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    अण्णहा
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
स्थिर पूर्णाङ्क quotefloat (lua_State *L, वर्ण *buff, lua_सङ्ख्या n) {
  ध्रुव वर्ण *s;  /* for the fixed representations */
  जइ (n == (lua_सङ्ख्या)HUGE_VAL)  /* inf? */
    s = "1e9999";
  अण्णहा जइ (n == -(lua_सङ्ख्या)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  अण्णहा जइ (n != n)  /* NaN? */
    s = "(0/0)";
  अण्णहा {  /* format number as hexadecimal */
    पूर्णाङ्क  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_सङ्ख्या_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    जइ (memchr(buff, '.', cast_uint(nb)) == रिक्त) {  /* no dot? */
      वर्ण point = lua_getlocaledecpoint();  /* try locale point */
      वर्ण *ppoint = (वर्ण *)memchr(buff, point, cast_uint(nb));
      जइ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    प्रत्यागम nb;
  }
  /* for the fixed representations */
  प्रत्यागम l_sprintf(buff, MAX_ITEM, "%s", s);
}


स्थिर शून्य addliteral (lua_State *L, luaL_Buffer *b, पूर्णाङ्क arg) {
  विकल्प (lua_type(L, arg)) {
    पक्ष LUA_TSTRING: {
      size_t len;
      ध्रुव वर्ण *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      भङ्ग;
    }
    पक्ष LUA_TNUMBER: {
      वर्ण *buff = luaL_prepbuffsize(b, MAX_ITEM);
      पूर्णाङ्क nb;
      जइ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      अण्णहा {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ध्रुव वर्ण *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      भङ्ग;
    }
    पक्ष LUA_TNIL: पक्ष LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, रिक्त);
      luaL_addvalue(b);
      भङ्ग;
    }
    स्वभाव: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


स्थिर ध्रुव वर्ण *get2digits (ध्रुव वर्ण *s) {
  जइ (isdigit(cast_uchar(*s))) {
    s++;
    जइ (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  प्रत्यागम s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
स्थिर शून्य checkformat (lua_State *L, ध्रुव वर्ण *form, ध्रुव वर्ण *flags,
                                       पूर्णाङ्क precision) {
  ध्रुव वर्ण *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  जइ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    जइ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  जइ (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
स्थिर ध्रुव वर्ण *getformat (lua_State *L, ध्रुव वर्ण *strfrmt,
                                            वर्ण *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  जइ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * परिमाण(वर्ण));
  *(form + len) = '\0';
  प्रत्यागम strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
स्थिर शून्य addlenmod (वर्ण *form, ध्रुव वर्ण *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  वर्ण spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


स्थिर पूर्णाङ्क str_format (lua_State *L) {
  पूर्णाङ्क शिखरम् = lua_gettop(L);
  पूर्णाङ्क arg = 1;
  size_t sfl;
  ध्रुव वर्ण *strfrmt = luaL_checklstring(L, arg, &sfl);
  ध्रुव वर्ण *strfrmt_end = strfrmt+sfl;
  ध्रुव वर्ण *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  जाव (strfrmt < strfrmt_end) {
    जइ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    अण्णहा जइ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    अण्णहा { /* format item */
      वर्ण form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      वर्ण *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      पूर्णाङ्क nb = 0;  /* number of bytes in result */
      जइ (++arg > शिखरम्)
        प्रत्यागम luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      विकल्प (*strfrmt++) {
        पक्ष 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (पूर्णाङ्क)luaL_checkinteger(L, arg));
          भङ्ग;
        }
        पक्ष 'd': पक्ष 'i':
          flags = L_FMTFLAGSI;
          गच्छ intcase;
        पक्ष 'u':
          flags = L_FMTFLAGSU;
          गच्छ intcase;
        पक्ष 'o': पक्ष 'x': पक्ष 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          भङ्ग;
        }
        पक्ष 'a': पक्ष 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_सङ्ख्या_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          भङ्ग;
        पक्ष 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        पक्ष 'e': पक्ष 'E': पक्ष 'g': पक्ष 'G': {
          lua_सङ्ख्या n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_सङ्ख्या_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          भङ्ग;
        }
        पक्ष 'p': {
          ध्रुव शून्य *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          जइ (p == रिक्त) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          भङ्ग;
        }
        पक्ष 'q': {
          जइ (form[2] != '\0')  /* modifiers? */
            प्रत्यागम luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          भङ्ग;
        }
        पक्ष 's': {
          size_t l;
          ध्रुव वर्ण *s = luaL_tolstring(L, arg, &l);
          जइ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          अण्णहा {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            जइ (strchr(form, '.') == रिक्त && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            अण्णहा {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_निष्कासय(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          भङ्ग;
        }
        स्वभाव: {  /* also treat cases 'pnLlh' */
          प्रत्यागम luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  प्रत्यागम 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#जइ !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((पूर्णाङ्क)परिमाण(lua_Integer))


/* dummy union to get native endianness */
स्थिर ध्रुव सङ्घ {
  पूर्णाङ्क dummy;
  वर्ण little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
प्रकारनिर्देश संरचना Header {
  lua_State *L;
  पूर्णाङ्क islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
प्रकारनिर्देश गणना KOption {
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
स्थिर पूर्णाङ्क digit (पूर्णाङ्क c) { प्रत्यागम '0' <= c && c <= '9'; }

स्थिर size_t getnum (ध्रुव वर्ण **fmt, size_t df) {
  जइ (!digit(**fmt))  /* no number? */
    प्रत्यागम df;  /* return default value */
  अण्णहा {
    size_t a = 0;
    करेहि {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } जाव (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    प्रत्यागम a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
स्थिर unsigned getnumlimit (Header *h, ध्रुव वर्ण **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  जइ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    प्रत्यागम cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  प्रत्यागम cast_uint(sz);
}


/*
** Initialize Header
*/
स्थिर शून्य initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
स्थिर KOption getoption (Header *h, ध्रुव वर्ण **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  संरचना cD { वर्ण c; सङ्घ { LUAI_MAXALIGN; } u; };
  पूर्णाङ्क opt = *((*fmt)++);
  *size = 0;  /* default */
  विकल्प (opt) {
    पक्ष 'b': *size = परिमाण(वर्ण); प्रत्यागम Kint;
    पक्ष 'B': *size = परिमाण(वर्ण); प्रत्यागम Kuint;
    पक्ष 'h': *size = परिमाण(short); प्रत्यागम Kint;
    पक्ष 'H': *size = परिमाण(short); प्रत्यागम Kuint;
    पक्ष 'l': *size = परिमाण(long); प्रत्यागम Kint;
    पक्ष 'L': *size = परिमाण(long); प्रत्यागम Kuint;
    पक्ष 'j': *size = परिमाण(lua_Integer); प्रत्यागम Kint;
    पक्ष 'J': *size = परिमाण(lua_Integer); प्रत्यागम Kuint;
    पक्ष 'T': *size = परिमाण(size_t); प्रत्यागम Kuint;
    पक्ष 'f': *size = परिमाण(भिन्न); प्रत्यागम Kfloat;
    पक्ष 'n': *size = परिमाण(lua_सङ्ख्या); प्रत्यागम Knumber;
    पक्ष 'd': *size = परिमाण(द्विगुण); प्रत्यागम Kdouble;
    पक्ष 'i': *size = getnumlimit(h, fmt, परिमाण(पूर्णाङ्क)); प्रत्यागम Kint;
    पक्ष 'I': *size = getnumlimit(h, fmt, परिमाण(पूर्णाङ्क)); प्रत्यागम Kuint;
    पक्ष 's': *size = getnumlimit(h, fmt, परिमाण(size_t)); प्रत्यागम Kstring;
    पक्ष 'c':
      *size = getnum(fmt, cast_sizet(-1));
      जइ (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      प्रत्यागम Kchar;
    पक्ष 'z': प्रत्यागम Kzstr;
    पक्ष 'x': *size = 1; प्रत्यागम Kpadding;
    पक्ष 'X': प्रत्यागम Kpaddalign;
    पक्ष ' ': भङ्ग;
    पक्ष '<': h->islittle = 1; भङ्ग;
    पक्ष '>': h->islittle = 0; भङ्ग;
    पक्ष '=': h->islittle = nativeendian.little; भङ्ग;
    पक्ष '!': {
      ध्रुव size_t maxalign = offsetof(संरचना cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      भङ्ग;
    }
    स्वभाव: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  प्रत्यागम Knop;
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
स्थिर KOption getdetails (Header *h, size_t totalsize, ध्रुव वर्ण **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  जइ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    जइ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  जइ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  अण्णहा {
    जइ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    जइ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    अण्णहा {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  प्रत्यागम opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
स्थिर शून्य packint (luaL_Buffer *b, lua_Unsigned n,
                     पूर्णाङ्क islittle, unsigned size, पूर्णाङ्क neg) {
  वर्ण *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (वर्ण)(n & MC);  /* first byte */
  प्रति (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (वर्ण)(n & MC);
  }
  जइ (neg && size > SZINT) {  /* negative number need sign extension? */
    प्रति (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (वर्ण)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
स्थिर शून्य copywithendian (वर्ण *dest, ध्रुव वर्ण *src,
                            unsigned size, पूर्णाङ्क islittle) {
  जइ (islittle == nativeendian.little)
    memcpy(dest, src, size);
  अण्णहा {
    dest += size - 1;
    जाव (size-- != 0)
      *(dest--) = *(src++);
  }
}


स्थिर पूर्णाङ्क str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  ध्रुव वर्ण *fmt = luaL_checkstring(L, 1);  /* format string */
  पूर्णाङ्क arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  जाव (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    जाव (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    विकल्प (opt) {
      पक्ष Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        जइ (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        भङ्ग;
      }
      पक्ष Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        जइ (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        भङ्ग;
      }
      पक्ष Kfloat: {  /* C float */
        भिन्न f = (भिन्न)luaL_checknumber(L, arg);  /* get argument */
        वर्ण *buff = luaL_prepbuffsize(&b, परिमाण(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (वर्ण *)&f, परिमाण(f), h.islittle);
        luaL_addsize(&b, size);
        भङ्ग;
      }
      पक्ष Knumber: {  /* Lua float */
        lua_सङ्ख्या f = luaL_checknumber(L, arg);  /* get argument */
        वर्ण *buff = luaL_prepbuffsize(&b, परिमाण(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (वर्ण *)&f, परिमाण(f), h.islittle);
        luaL_addsize(&b, size);
        भङ्ग;
      }
      पक्ष Kdouble: {  /* C double */
        द्विगुण f = (द्विगुण)luaL_checknumber(L, arg);  /* get argument */
        वर्ण *buff = luaL_prepbuffsize(&b, परिमाण(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (वर्ण *)&f, परिमाण(f), h.islittle);
        luaL_addsize(&b, size);
        भङ्ग;
      }
      पक्ष Kchar: {  /* fixed-size string */
        size_t len;
        ध्रुव वर्ण *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        जइ (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          वर्ण *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        भङ्ग;
      }
      पक्ष Kstring: {  /* strings with length count */
        size_t len;
        ध्रुव वर्ण *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= परिमाण(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        भङ्ग;
      }
      पक्ष Kzstr: {  /* zero-terminated string */
        size_t len;
        ध्रुव वर्ण *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        भङ्ग;
      }
      पक्ष Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      पक्ष Kpaddalign: पक्ष Knop:
        arg--;  /* undo increment */
        भङ्ग;
    }
  }
  luaL_pushresult(&b);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_packsize (lua_State *L) {
  Header h;
  ध्रुव वर्ण *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  जाव (*fmt != '\0') {
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
  प्रत्यागम 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
स्थिर lua_Integer unpackint (lua_State *L, ध्रुव वर्ण *str,
                              पूर्णाङ्क islittle, पूर्णाङ्क size, पूर्णाङ्क issigned) {
  lua_Unsigned res = 0;
  पूर्णाङ्क i;
  पूर्णाङ्क limit = (size  <= SZINT) ? size : SZINT;
  प्रति (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned वर्ण)str[islittle ? i : size - 1 - i];
  }
  जइ (size < SZINT) {  /* real size smaller than lua_Integer? */
    जइ (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  अण्णहा जइ (size > SZINT) {  /* must check unread bytes */
    पूर्णाङ्क mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    प्रति (i = limit; i < size; i++) {
      जइ (l_unlikely((unsigned वर्ण)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  प्रत्यागम (lua_Integer)res;
}


स्थिर पूर्णाङ्क str_unpack (lua_State *L) {
  Header h;
  ध्रुव वर्ण *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ध्रुव वर्ण *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  पूर्णाङ्क n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  जाव (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    विकल्प (opt) {
      पक्ष Kint:
      पक्ष Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        भङ्ग;
      }
      पक्ष Kfloat: {
        भिन्न f;
        copywithendian((वर्ण *)&f, data + pos, परिमाण(f), h.islittle);
        lua_pushnumber(L, (lua_सङ्ख्या)f);
        भङ्ग;
      }
      पक्ष Knumber: {
        lua_सङ्ख्या f;
        copywithendian((वर्ण *)&f, data + pos, परिमाण(f), h.islittle);
        lua_pushnumber(L, f);
        भङ्ग;
      }
      पक्ष Kdouble: {
        द्विगुण f;
        copywithendian((वर्ण *)&f, data + pos, परिमाण(f), h.islittle);
        lua_pushnumber(L, (lua_सङ्ख्या)f);
        भङ्ग;
      }
      पक्ष Kchar: {
        lua_pushlstring(L, data + pos, size);
        भङ्ग;
      }
      पक्ष Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        भङ्ग;
      }
      पक्ष Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        भङ्ग;
      }
      पक्ष Kpaddalign: पक्ष Kpadding: पक्ष Knop:
        n--;  /* undo increment */
        भङ्ग;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  प्रत्यागम n + 1;
}

/* }====================================================== */


स्थिर ध्रुव luaL_Reg strlib[] = {
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
  {रिक्त, रिक्त}
};


स्थिर शून्य createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_निष्कासय(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_निष्कासय(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API पूर्णाङ्क luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  प्रत्यागम 1;
}

