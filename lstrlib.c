/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#నిర్వచించు lstrlib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <ctype.h>
#అన్తర్భావ <float.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <locale.h>
#అన్తర్భావ <math.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#ఒకవేళ !defined(LUA_MAXCAPTURES)
#నిర్వచించు LUA_MAXCAPTURES		32
#అయితే_ముగింపు


స్థిర పూర్ణాంకం str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  తిరిగి 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
స్థిర size_t posrelatI (lua_Integer pos, size_t len) {
  ఒకవేళ (pos > 0)
    తిరిగి (size_t)pos;
  లేకపోతే ఒకవేళ (pos == 0)
    తిరిగి 1;
  లేకపోతే ఒకవేళ (pos < -(lua_Integer)len)  /* inverted comparison */
    తిరిగి 1;  /* clip to 1 */
  లేకపోతే తిరిగి len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
స్థిర size_t getendpos (lua_State *L, పూర్ణాంకం arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  ఒకవేళ (pos > (lua_Integer)len)
    తిరిగి len;
  లేకపోతే ఒకవేళ (pos >= 0)
    తిరిగి (size_t)pos;
  లేకపోతే ఒకవేళ (pos < -(lua_Integer)len)
    తిరిగి 0;
  లేకపోతే తిరిగి len + (size_t)pos + 1;
}


స్థిర పూర్ణాంకం str_sub (lua_State *L) {
  size_t l;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &l);
  size_t ప్రారంభం = posrelatI(luaL_checkinteger(L, 2), l);
  size_t చివర = getendpos(L, 3, -1, l);
  ఒకవేళ (ప్రారంభం <= చివర)
    lua_pushlstring(L, s + ప్రారంభం - 1, (చివర - ప్రారంభం) + 1);
  లేకపోతే lua_pushliteral(L, "");
  తిరిగి 1;
}


స్థిర పూర్ణాంకం str_తిరగేయి (lua_State *L) {
  size_t l, i;
  lua_L_బఫరు b;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &l);
  అక్షరం *p = luaL_buffinitsize(L, &b, l);
  కొరకు (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం str_క్రింది (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_బఫరు b;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &l);
  అక్షరం *p = luaL_buffinitsize(L, &b, l);
  కొరకు (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం str_పై (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_బఫరు b;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &l);
  అక్షరం *p = luaL_buffinitsize(L, &b, l);
  కొరకు (i=0; i<l; i++)
    p[i] = cast_char(పెద్ద_అక్షరం(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  తిరిగి 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
స్థిర పూర్ణాంకం str_rep (lua_State *L) {
  size_t len, lsep;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  స్థిరరాశి అక్షరం *sep = luaL_optlstring(L, 3, "", &lsep);
  ఒకవేళ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  లేకపోతే ఒకవేళ (l_unlikely(len > MAX_కొలత - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_కొలత) / n))
    తిరిగి luaL_error(L, "resulting string too large");
  లేకపోతే {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    lua_L_బఫరు b;
    అక్షరం *p = luaL_buffinitsize(L, &b, totallen);
    అయితే (n-- > 1) {  /* first n-1 copies (followed by separator) */
      జ్ఞాపకం_నకలు(p, s, len * పరిమాణం(అక్షరం)); p += len;
      ఒకవేళ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        జ్ఞాపకం_నకలు(p, sep, lsep * పరిమాణం(అక్షరం)); p += lsep;
      }
    }
    జ్ఞాపకం_నకలు(p, s, len * పరిమాణం(అక్షరం));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం str_byte (lua_State *L) {
  size_t l;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  పూర్ణాంకం n, i;
  ఒకవేళ (posi > pose) తిరిగి 0;  /* empty interval; return no values */
  ఒకవేళ (l_unlikely(pose - posi >= (size_t)గరిష్ఠ_పూర్ణాంకం))  /* arithmetic overflow? */
    తిరిగి luaL_error(L, "string slice too long");
  n = (పూర్ణాంకం)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  కొరకు (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  తిరిగి n;
}


స్థిర పూర్ణాంకం str_char (lua_State *L) {
  పూర్ణాంకం n = lua_gettop(L);  /* number of arguments */
  పూర్ణాంకం i;
  lua_L_బఫరు b;
  అక్షరం *p = luaL_buffinitsize(L, &b, cast_uint(n));
  కొరకు (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  తిరిగి 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
నిర్మాణం str_Writer {
  పూర్ణాంకం init;  /* true iff buffer has been initialized */
  lua_L_బఫరు B;
};


స్థిర పూర్ణాంకం writer (lua_State *L, స్థిరరాశి శూన్య *b, size_t కొలత, శూన్య *ud) {
  నిర్మాణం str_Writer *state = (నిర్మాణం str_Writer *)ud;
  ఒకవేళ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  ఒకవేళ (b == ఖాళీ) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  లేకపోతే
    luaL_addlstring(&state->B, (స్థిరరాశి అక్షరం *)b, కొలత);
  తిరిగి 0;
}


స్థిర పూర్ణాంకం str_dump (lua_State *L) {
  నిర్మాణం str_Writer state;
  పూర్ణాంకం strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  తిరిగి 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#ఒకవేళ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

స్థిర స్థిరరాశి luaL_Reg stringmetamethods[] = {
  {"__index", ఖాళీ},  /* placeholder */
  {ఖాళీ, ఖాళీ}
};

#లేకపోతే		/* }{ */

స్థిర పూర్ణాంకం tonum (lua_State *L, పూర్ణాంకం arg) {
  ఒకవేళ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    తిరిగి 1;
  }
  లేకపోతే {  /* check whether it is a numerical string */
    size_t len;
    స్థిరరాశి అక్షరం *s = lua_tolstring(L, arg, &len);
    తిరిగి (s != ఖాళీ && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
స్థిర శూన్య trymt (lua_State *L, స్థిరరాశి అక్షరం *mtkey, స్థిరరాశి అక్షరం *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  ఒకవేళ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_చొప్పించు(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


స్థిర పూర్ణాంకం arith (lua_State *L, పూర్ణాంకం op, స్థిరరాశి అక్షరం *mtname) {
  ఒకవేళ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  లేకపోతే
    trymt(L, mtname, mtname + 2);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం arith_కూడు (lua_State *L) {
  తిరిగి arith(L, LUA_OPADD, "__add");
}

స్థిర పూర్ణాంకం arith_sub (lua_State *L) {
  తిరిగి arith(L, LUA_OPSUB, "__sub");
}

స్థిర పూర్ణాంకం arith_mul (lua_State *L) {
  తిరిగి arith(L, LUA_OPMUL, "__mul");
}

స్థిర పూర్ణాంకం arith_mod (lua_State *L) {
  తిరిగి arith(L, LUA_OPMOD, "__mod");
}

స్థిర పూర్ణాంకం arith_pow (lua_State *L) {
  తిరిగి arith(L, LUA_OPPOW, "__pow");
}

స్థిర పూర్ణాంకం arith_div (lua_State *L) {
  తిరిగి arith(L, LUA_OPDIV, "__div");
}

స్థిర పూర్ణాంకం arith_idiv (lua_State *L) {
  తిరిగి arith(L, LUA_OPIDIV, "__idiv");
}

స్థిర పూర్ణాంకం arith_unm (lua_State *L) {
  తిరిగి arith(L, LUA_OPUNM, "__unm");
}


స్థిర స్థిరరాశి luaL_Reg stringmetamethods[] = {
  {"__add", arith_కూడు},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", ఖాళీ},  /* placeholder */
  {ఖాళీ, ఖాళీ}
};

#అయితే_ముగింపు		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#నిర్వచించు CAP_UNFINISHED	(-1)
#నిర్వచించు CAP_POSITION	(-2)


రకనిర్వచనం నిర్మాణం సరిపోలిక_State {
  స్థిరరాశి అక్షరం *src_init;  /* init of source string */
  స్థిరరాశి అక్షరం *src_చివర;  /* end ('\0') of source string */
  స్థిరరాశి అక్షరం *p_చివర;  /* end ('\0') of pattern */
  lua_State *L;
  పూర్ణాంకం matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  పూర్ణాంకం level;  /* total number of captures (finished or unfinished) */
  నిర్మాణం {
    స్థిరరాశి అక్షరం *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} సరిపోలిక_State;


/* recursive function */
స్థిర స్థిరరాశి అక్షరం *సరిపోలిక (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *p);


/* maximum recursion depth for 'match' */
#ఒకవేళ !defined(MAXCCALLS)
#నిర్వచించు MAXCCALLS	200
#అయితే_ముగింపు


#నిర్వచించు L_ESC		'%'
#నిర్వచించు SPECIALS	"^$*+?.([%-"


స్థిర పూర్ణాంకం check_capture (సరిపోలిక_State *ms, పూర్ణాంకం l) {
  l -= '1';
  ఒకవేళ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    తిరిగి luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  తిరిగి l;
}


స్థిర పూర్ణాంకం capture_గమ్యం_close (సరిపోలిక_State *ms) {
  పూర్ణాంకం level = ms->level;
  కొరకు (level--; level>=0; level--)
    ఒకవేళ (ms->capture[level].len == CAP_UNFINISHED) తిరిగి level;
  తిరిగి luaL_error(ms->L, "invalid pattern capture");
}


స్థిర స్థిరరాశి అక్షరం *classend (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *p) {
  ఎంపిక (*p++) {
    స్థితి L_ESC: {
      ఒకవేళ (l_unlikely(p == ms->p_చివర))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      తిరిగి p+1;
    }
    స్థితి '[': {
      ఒకవేళ (*p == '^') p++;
      చేయి {  /* look for a ']' */
        ఒకవేళ (l_unlikely(p == ms->p_చివర))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        ఒకవేళ (*(p++) == L_ESC && p < ms->p_చివర)
          p++;  /* skip escapes (e.g. '%]') */
      } అయితే (*p != ']');
      తిరిగి p+1;
    }
    అప్రమేయ: {
      తిరిగి p;
    }
  }
}


స్థిర పూర్ణాంకం సరిపోలిక_class (పూర్ణాంకం c, పూర్ణాంకం cl) {
  పూర్ణాంకం res;
  ఎంపిక (tolower(cl)) {
    స్థితి 'a' : res = అక్షరమా(c); ఆపు;
    స్థితి 'c' : res = iscntrl(c); ఆపు;
    స్థితి 'd' : res = అంకెనా(c); ఆపు;
    స్థితి 'g' : res = isgraph(c); ఆపు;
    స్థితి 'l' : res = islower(c); ఆపు;
    స్థితి 'p' : res = ispunct(c); ఆపు;
    స్థితి 's' : res = ఖాళీనా(c); ఆపు;
    స్థితి 'u' : res = isupper(c); ఆపు;
    స్థితి 'w' : res = అక్షరమా_అంకెనా(c); ఆపు;
    స్థితి 'x' : res = isxdigit(c); ఆపు;
    స్థితి 'z' : res = (c == 0); ఆపు;  /* deprecated option */
    అప్రమేయ: తిరిగి (cl == c);
  }
  తిరిగి (islower(cl) ? res : !res);
}


స్థిర పూర్ణాంకం matchbracketclass (పూర్ణాంకం c, స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *ec) {
  పూర్ణాంకం sig = 1;
  ఒకవేళ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  అయితే (++p < ec) {
    ఒకవేళ (*p == L_ESC) {
      p++;
      ఒకవేళ (సరిపోలిక_class(c, cast_uchar(*p)))
        తిరిగి sig;
    }
    లేకపోతే ఒకవేళ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      ఒకవేళ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        తిరిగి sig;
    }
    లేకపోతే ఒకవేళ (cast_uchar(*p) == c) తిరిగి sig;
  }
  తిరిగి !sig;
}


స్థిర పూర్ణాంకం singlematch (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *p,
                        స్థిరరాశి అక్షరం *ep) {
  ఒకవేళ (s >= ms->src_చివర)
    తిరిగి 0;
  లేకపోతే {
    పూర్ణాంకం c = cast_uchar(*s);
    ఎంపిక (*p) {
      స్థితి '.': తిరిగి 1;  /* matches any char */
      స్థితి L_ESC: తిరిగి సరిపోలిక_class(c, cast_uchar(*(p+1)));
      స్థితి '[': తిరిగి matchbracketclass(c, p, ep-1);
      అప్రమేయ:  తిరిగి (cast_uchar(*p) == c);
    }
  }
}


స్థిర స్థిరరాశి అక్షరం *matchbalance (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s,
                                   స్థిరరాశి అక్షరం *p) {
  ఒకవేళ (l_unlikely(p >= ms->p_చివర - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  ఒకవేళ (*s != *p) తిరిగి ఖాళీ;
  లేకపోతే {
    పూర్ణాంకం b = *p;
    పూర్ణాంకం e = *(p+1);
    పూర్ణాంకం cont = 1;
    అయితే (++s < ms->src_చివర) {
      ఒకవేళ (*s == e) {
        ఒకవేళ (--cont == 0) తిరిగి s+1;
      }
      లేకపోతే ఒకవేళ (*s == b) cont++;
    }
  }
  తిరిగి ఖాళీ;  /* string ends out of balance */
}


స్థిర స్థిరరాశి అక్షరం *max_expand (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s,
                                 స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  అయితే (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  అయితే (i>=0) {
    స్థిరరాశి అక్షరం *res = సరిపోలిక(ms, (s+i), ep+1);
    ఒకవేళ (res) తిరిగి res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  తిరిగి ఖాళీ;
}


స్థిర స్థిరరాశి అక్షరం *min_expand (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s,
                                 స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *ep) {
  కొరకు (;;) {
    స్థిరరాశి అక్షరం *res = సరిపోలిక(ms, s, ep+1);
    ఒకవేళ (res != ఖాళీ)
      తిరిగి res;
    లేకపోతే ఒకవేళ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    లేకపోతే తిరిగి ఖాళీ;
  }
}


స్థిర స్థిరరాశి అక్షరం *ప్రారంభం_capture (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s,
                                    స్థిరరాశి అక్షరం *p, పూర్ణాంకం what) {
  స్థిరరాశి అక్షరం *res;
  పూర్ణాంకం level = ms->level;
  ఒకవేళ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  ఒకవేళ ((res=సరిపోలిక(ms, s, p)) == ఖాళీ)  /* match failed? */
    ms->level--;  /* undo capture */
  తిరిగి res;
}


స్థిర స్థిరరాశి అక్షరం *చివర_capture (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s,
                                  స్థిరరాశి అక్షరం *p) {
  పూర్ణాంకం l = capture_గమ్యం_close(ms);
  స్థిరరాశి అక్షరం *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  ఒకవేళ ((res = సరిపోలిక(ms, s, p)) == ఖాళీ)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  తిరిగి res;
}


స్థిర స్థిరరాశి అక్షరం *సరిపోలిక_capture (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s, పూర్ణాంకం l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  ఒకవేళ ((size_t)(ms->src_చివర-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    తిరిగి s+len;
  లేకపోతే తిరిగి ఖాళీ;
}


స్థిర స్థిరరాశి అక్షరం *సరిపోలిక (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *p) {
  ఒకవేళ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  ఒకవేళ (p != ms->p_చివర) {  /* end of pattern? */
    ఎంపిక (*p) {
      స్థితి '(': {  /* start capture */
        ఒకవేళ (*(p + 1) == ')')  /* position capture? */
          s = ప్రారంభం_capture(ms, s, p + 2, CAP_POSITION);
        లేకపోతే
          s = ప్రారంభం_capture(ms, s, p + 1, CAP_UNFINISHED);
        ఆపు;
      }
      స్థితి ')': {  /* end capture */
        s = చివర_capture(ms, s, p + 1);
        ఆపు;
      }
      స్థితి '$': {
        ఒకవేళ ((p + 1) != ms->p_చివర)  /* is the '$' the last char in pattern? */
          వెళ్ళు dflt;  /* no; go to default */
        s = (s == ms->src_చివర) ? s : ఖాళీ;  /* check end of string */
        ఆపు;
      }
      స్థితి L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        ఎంపిక (*(p + 1)) {
          స్థితి 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            ఒకవేళ (s != ఖాళీ) {
              p += 4; వెళ్ళు init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            ఆపు;
          }
          స్థితి 'f': {  /* frontier? */
            స్థిరరాశి అక్షరం *ep; అక్షరం మునుపటి;
            p += 2;
            ఒకవేళ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            మునుపటి = (s == ms->src_init) ? '\0' : *(s - 1);
            ఒకవేళ (!matchbracketclass(cast_uchar(మునుపటి), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; వెళ్ళు init;  /* return match(ms, s, ep); */
            }
            s = ఖాళీ;  /* match failed */
            ఆపు;
          }
          స్థితి '0': స్థితి '1': స్థితి '2': స్థితి '3':
          స్థితి '4': స్థితి '5': స్థితి '6': స్థితి '7':
          స్థితి '8': స్థితి '9': {  /* capture results (%0-%9)? */
            s = సరిపోలిక_capture(ms, s, cast_uchar(*(p + 1)));
            ఒకవేళ (s != ఖాళీ) {
              p += 2; వెళ్ళు init;  /* return match(ms, s, p + 2) */
            }
            ఆపు;
          }
          అప్రమేయ: వెళ్ళు dflt;
        }
        ఆపు;
      }
      అప్రమేయ: dflt: {  /* pattern class plus optional suffix */
        స్థిరరాశి అక్షరం *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        ఒకవేళ (!singlematch(ms, s, p, ep)) {
          ఒకవేళ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; వెళ్ళు init;  /* return match(ms, s, ep + 1); */
          }
          లేకపోతే  /* '+' or no suffix */
            s = ఖాళీ;  /* fail */
        }
        లేకపోతే {  /* matched once */
          ఎంపిక (*ep) {  /* handle optional suffix */
            స్థితి '?': {  /* optional */
              స్థిరరాశి అక్షరం *res;
              ఒకవేళ ((res = సరిపోలిక(ms, s + 1, ep + 1)) != ఖాళీ)
                s = res;
              లేకపోతే {
                p = ep + 1; వెళ్ళు init;  /* else return match(ms, s, ep + 1); */
              }
              ఆపు;
            }
            స్థితి '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            స్థితి '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              ఆపు;
            స్థితి '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              ఆపు;
            అప్రమేయ:  /* no suffix */
              s++; p = ep; వెళ్ళు init;  /* return match(ms, s + 1, ep); */
          }
        }
        ఆపు;
      }
    }
  }
  ms->matchdepth++;
  తిరిగి s;
}



స్థిర స్థిరరాశి అక్షరం *lmemfind (స్థిరరాశి అక్షరం *s1, size_t l1,
                               స్థిరరాశి అక్షరం *s2, size_t l2) {
  ఒకవేళ (l2 == 0) తిరిగి s1;  /* empty strings are everywhere */
  లేకపోతే ఒకవేళ (l2 > l1) తిరిగి ఖాళీ;  /* avoids a negative 'l1' */
  లేకపోతే {
    స్థిరరాశి అక్షరం *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    అయితే (l1 > 0 && (init = (స్థిరరాశి అక్షరం *)memchr(s1, *s2, l1)) != ఖాళీ) {
      init++;   /* 1st char is already checked */
      ఒకవేళ (memcmp(init, s2+1, l2) == 0)
        తిరిగి init-1;
      లేకపోతే {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    తిరిగి ఖాళీ;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
స్థిర ptrdiff_t get_onecapture (సరిపోలిక_State *ms, పూర్ణాంకం i, స్థిరరాశి అక్షరం *s,
                              స్థిరరాశి అక్షరం *e, స్థిరరాశి అక్షరం **cap) {
  ఒకవేళ (i >= ms->level) {
    ఒకవేళ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    తిరిగి (e - s);
  }
  లేకపోతే {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    ఒకవేళ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    లేకపోతే ఒకవేళ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    తిరిగి capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
స్థిర శూన్య నెట్టు_onecapture (సరిపోలిక_State *ms, పూర్ణాంకం i, స్థిరరాశి అక్షరం *s,
                                                    స్థిరరాశి అక్షరం *e) {
  స్థిరరాశి అక్షరం *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  ఒకవేళ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


స్థిర పూర్ణాంకం నెట్టు_captures (సరిపోలిక_State *ms, స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *e) {
  పూర్ణాంకం i;
  పూర్ణాంకం nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  కొరకు (i = 0; i < nlevels; i++)
    నెట్టు_onecapture(ms, i, s, e);
  తిరిగి nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
స్థిర పూర్ణాంకం nospecials (స్థిరరాశి అక్షరం *p, size_t l) {
  size_t upto = 0;
  చేయి {
    ఒకవేళ (strpbrk(p + upto, SPECIALS))
      తిరిగి 0;  /* pattern has a special character */
    upto += అక్షరశ్రేణి_పొడవు(p + upto) + 1;  /* may have more after \0 */
  } అయితే (upto <= l);
  తిరిగి 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
స్థిర శూన్య prepstate (సరిపోలిక_State *ms, lua_State *L,
                       స్థిరరాశి అక్షరం *s, size_t ls, స్థిరరాశి అక్షరం *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_చివర = s + ls;
  ms->p_చివర = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
స్థిర శూన్య reprepstate (సరిపోలిక_State *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


స్థిర పూర్ణాంకం str_కనుగొను_aux (lua_State *L, పూర్ణాంకం కనుగొను) {
  size_t ls, lp;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &ls);
  స్థిరరాశి అక్షరం *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  ఒకవేళ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    తిరిగి 1;
  }
  /* explicit request or no special characters? */
  ఒకవేళ (కనుగొను && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    స్థిరరాశి అక్షరం *s2 = lmemfind(s + init, ls - init, p, lp);
    ఒకవేళ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      తిరిగి 2;
    }
  }
  లేకపోతే {
    సరిపోలిక_State ms;
    స్థిరరాశి అక్షరం *s1 = s + init;
    పూర్ణాంకం anchor = (*p == '^');
    ఒకవేళ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    చేయి {
      స్థిరరాశి అక్షరం *res;
      reprepstate(&ms);
      ఒకవేళ ((res=సరిపోలిక(&ms, s1, p)) != ఖాళీ) {
        ఒకవేళ (కనుగొను) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /*ప్రారంభం*/
          lua_pushinteger(L, ct_diff2S(res - s));   /*చివర*/
          తిరిగి నెట్టు_captures(&ms, ఖాళీ, 0) + 2;
        }
        లేకపోతే
          తిరిగి నెట్టు_captures(&ms, s1, res);
      }
    } అయితే (s1++ < ms.src_చివర && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  తిరిగి 1;
}


స్థిర పూర్ణాంకం str_కనుగొను (lua_State *L) {
  తిరిగి str_కనుగొను_aux(L, 1);
}


స్థిర పూర్ణాంకం str_సరిపోలిక (lua_State *L) {
  తిరిగి str_కనుగొను_aux(L, 0);
}


/* state for 'gmatch' */
రకనిర్వచనం నిర్మాణం GMatchState {
  స్థిరరాశి అక్షరం *src;  /* current position */
  స్థిరరాశి అక్షరం *p;  /*నమూనా*/
  స్థిరరాశి అక్షరం *lastmatch;  /* end of last match */
  సరిపోలిక_State ms;  /* match state */
} GMatchState;


స్థిర పూర్ణాంకం gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  స్థిరరాశి అక్షరం *src;
  gm->ms.L = L;
  కొరకు (src = gm->src; src <= gm->ms.src_చివర; src++) {
    స్థిరరాశి అక్షరం *e;
    reprepstate(&gm->ms);
    ఒకవేళ ((e = సరిపోలిక(&gm->ms, src, gm->p)) != ఖాళీ && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      తిరిగి నెట్టు_captures(&gm->ms, src, e);
    }
  }
  తిరిగి 0;  /* not found */
}


స్థిర పూర్ణాంకం gmatch (lua_State *L) {
  size_t ls, lp;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &ls);
  స్థిరరాశి అక్షరం *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, పరిమాణం(GMatchState), 0);
  ఒకవేళ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = ఖాళీ;
  lua_pushcclosure(L, gmatch_aux, 3);
  తిరిగి 1;
}


స్థిర శూన్య కూడు_s (సరిపోలిక_State *ms, lua_L_బఫరు *b, స్థిరరాశి అక్షరం *s,
                                                   స్థిరరాశి అక్షరం *e) {
  size_t l;
  lua_State *L = ms->L;
  స్థిరరాశి అక్షరం *news = lua_tolstring(L, 3, &l);
  స్థిరరాశి అక్షరం *p;
  అయితే ((p = (అక్షరం *)memchr(news, L_ESC, l)) != ఖాళీ) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    ఒకవేళ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    లేకపోతే ఒకవేళ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    లేకపోతే ఒకవేళ (అంకెనా(cast_uchar(*p))) {  /* '%n' */
      స్థిరరాశి అక్షరం *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      ఒకవేళ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      లేకపోతే
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    లేకపోతే
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
స్థిర పూర్ణాంకం కూడు_విలువ (సరిపోలిక_State *ms, lua_L_బఫరు *b, స్థిరరాశి అక్షరం *s,
                                      స్థిరరాశి అక్షరం *e, పూర్ణాంకం tr) {
  lua_State *L = ms->L;
  ఎంపిక (tr) {
    స్థితి LUA_TFUNCTION: {  /* call the function */
      పూర్ణాంకం n;
      lua_pushvalue(L, 3);  /* push the function */
      n = నెట్టు_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      ఆపు;
    }
    స్థితి LUA_TTABLE: {  /* index the table */
      నెట్టు_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      ఆపు;
    }
    అప్రమేయ: {  /* LUA_TNUMBER or LUA_TSTRING */
      కూడు_s(ms, b, s, e);  /* add value to the buffer */
      తిరిగి 1;  /* something changed */
    }
  }
  ఒకవేళ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_తీయి(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    తిరిగి 0;  /* no changes */
  }
  లేకపోతే ఒకవేళ (l_unlikely(!lua_isstring(L, -1)))
    తిరిగి luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  లేకపోతే {
    luaL_addvalue(b);  /* add result to accumulator */
    తిరిగి 1;  /* something changed */
  }
}


స్థిర పూర్ణాంకం str_gsub (lua_State *L) {
  size_t srcl, lp;
  స్థిరరాశి అక్షరం *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  స్థిరరాశి అక్షరం *p = luaL_checklstring(L, 2, &lp);  /*నమూనా*/
  స్థిరరాశి అక్షరం *lastmatch = ఖాళీ;  /* end of last match */
  పూర్ణాంకం tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  పూర్ణాంకం anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  పూర్ణాంకం మారింది = 0;  /* change flag */
  సరిపోలిక_State ms;
  lua_L_బఫరు b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  ఒకవేళ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  అయితే (n < max_s) {
    స్థిరరాశి అక్షరం *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    ఒకవేళ ((e = సరిపోలిక(&ms, src, p)) != ఖాళీ && e != lastmatch) {  /* match? */
      n++;
      మారింది = కూడు_విలువ(&ms, &b, src, e, tr) || మారింది;
      src = lastmatch = e;
    }
    లేకపోతే ఒకవేళ (src < ms.src_చివర)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    లేకపోతే ఆపు;  /* end of subject */
    ఒకవేళ (anchor) ఆపు;
  }
  ఒకవేళ (!మారింది)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  లేకపోతే {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_చివర - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  తిరిగి 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#ఒకవేళ !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#నిర్వచించు SIZELENMOD	(పరిమాణం(LUA_సంఖ్య_FRMLEN)/పరిమాణం(అక్షరం))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#నిర్వచించు L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
స్థిర lua_సంఖ్య adddigit (అక్షరం *buff, గుర్తులేని n, lua_సంఖ్య x) {
  lua_సంఖ్య dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  పూర్ణాంకం d = (పూర్ణాంకం)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  తిరిగి x - dd;  /* return what is left */
}


స్థిర పూర్ణాంకం num2straux (అక్షరం *buff, గుర్తులేని sz, lua_సంఖ్య x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  ఒకవేళ (x != x || x == (lua_సంఖ్య)HUGE_VAL || x == -(lua_సంఖ్య)HUGE_VAL)
    తిరిగి l_sprintf(buff, sz, LUA_సంఖ్య_FMT, (LUAI_UACNUMBER)x);
  లేకపోతే ఒకవేళ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    తిరిగి l_sprintf(buff, sz, LUA_సంఖ్య_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  లేకపోతే {
    పూర్ణాంకం e;
    lua_సంఖ్య m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    గుర్తులేని n = 0;  /* character count */
    ఒకవేళ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    ఒకవేళ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      చేయి {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } అయితే (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    తిరిగి cast_int(n);
  }
}


స్థిర పూర్ణాంకం lua_number2strx (lua_State *L, అక్షరం *buff, గుర్తులేని sz,
                            స్థిరరాశి అక్షరం *fmt, lua_సంఖ్య x) {
  పూర్ణాంకం n = num2straux(buff, sz, x);
  ఒకవేళ (fmt[SIZELENMOD] == 'A') {
    పూర్ణాంకం i;
    కొరకు (i = 0; i < n; i++)
      buff[i] = cast_char(పెద్ద_అక్షరం(cast_uchar(buff[i])));
  }
  లేకపోతే ఒకవేళ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    తిరిగి luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  తిరిగి n;
}

#అయితే_ముగింపు				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#నిర్వచించు MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#నిర్వచించు MAX_అంశం	120


/* valid flags in a format specification */
#ఒకవేళ !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#నిర్వచించు L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#నిర్వచించు L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#నిర్వచించు L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#నిర్వచించు L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#నిర్వచించు L_FMTFLAGSC	"-"

#అయితే_ముగింపు


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#నిర్వచించు MAX_FORMAT	32


స్థిర శూన్య addquoted (lua_L_బఫరు *b, స్థిరరాశి అక్షరం *s, size_t len) {
  luaL_addchar(b, '"');
  అయితే (len--) {
    ఒకవేళ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    లేకపోతే ఒకవేళ (iscntrl(cast_uchar(*s))) {
      అక్షరం buff[10];
      ఒకవేళ (!అంకెనా(cast_uchar(*(s+1))))
        l_sprintf(buff, పరిమాణం(buff), "\\%d", (పూర్ణాంకం)cast_uchar(*s));
      లేకపోతే
        l_sprintf(buff, పరిమాణం(buff), "\\%03d", (పూర్ణాంకం)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    లేకపోతే
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
స్థిర పూర్ణాంకం quotefloat (lua_State *L, అక్షరం *buff, lua_సంఖ్య n) {
  స్థిరరాశి అక్షరం *s;  /* for the fixed representations */
  ఒకవేళ (n == (lua_సంఖ్య)HUGE_VAL)  /* inf? */
    s = "1e9999";
  లేకపోతే ఒకవేళ (n == -(lua_సంఖ్య)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  లేకపోతే ఒకవేళ (n != n)  /* NaN? */
    s = "(0/0)";
  లేకపోతే {  /* format number as hexadecimal */
    పూర్ణాంకం  nb = lua_number2strx(L, buff, MAX_అంశం,
                                 "%" LUA_సంఖ్య_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    ఒకవేళ (memchr(buff, '.', cast_uint(nb)) == ఖాళీ) {  /* no dot? */
      అక్షరం బిందువు = lua_getlocaledecpoint();  /* try locale point */
      అక్షరం *ppoint = (అక్షరం *)memchr(buff, బిందువు, cast_uint(nb));
      ఒకవేళ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    తిరిగి nb;
  }
  /* for the fixed representations */
  తిరిగి l_sprintf(buff, MAX_అంశం, "%s", s);
}


స్థిర శూన్య addliteral (lua_State *L, lua_L_బఫరు *b, పూర్ణాంకం arg) {
  ఎంపిక (lua_type(L, arg)) {
    స్థితి LUA_TSTRING: {
      size_t len;
      స్థిరరాశి అక్షరం *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      ఆపు;
    }
    స్థితి LUA_TNUMBER: {
      అక్షరం *buff = luaL_prepbuffsize(b, MAX_అంశం);
      పూర్ణాంకం nb;
      ఒకవేళ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      లేకపోతే {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        స్థిరరాశి అక్షరం *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_అంశం, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      ఆపు;
    }
    స్థితి LUA_TNIL: స్థితి LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, ఖాళీ);
      luaL_addvalue(b);
      ఆపు;
    }
    అప్రమేయ: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


స్థిర స్థిరరాశి అక్షరం *get2digits (స్థిరరాశి అక్షరం *s) {
  ఒకవేళ (అంకెనా(cast_uchar(*s))) {
    s++;
    ఒకవేళ (అంకెనా(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  తిరిగి s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
స్థిర శూన్య checkformat (lua_State *L, స్థిరరాశి అక్షరం *form, స్థిరరాశి అక్షరం *flags,
                                       పూర్ణాంకం precision) {
  స్థిరరాశి అక్షరం *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  ఒకవేళ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    ఒకవేళ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  ఒకవేళ (!అక్షరమా(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
స్థిర స్థిరరాశి అక్షరం *getformat (lua_State *L, స్థిరరాశి అక్షరం *strfrmt,
                                            అక్షరం *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  ఒకవేళ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  జ్ఞాపకం_నకలు(form, strfrmt, len * పరిమాణం(అక్షరం));
  *(form + len) = '\0';
  తిరిగి strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
స్థిర శూన్య addlenmod (అక్షరం *form, స్థిరరాశి అక్షరం *lenmod) {
  size_t l = అక్షరశ్రేణి_పొడవు(form);
  size_t lm = అక్షరశ్రేణి_పొడవు(lenmod);
  అక్షరం spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


స్థిర పూర్ణాంకం str_format (lua_State *L) {
  పూర్ణాంకం శిఖరం = lua_gettop(L);
  పూర్ణాంకం arg = 1;
  size_t sfl;
  స్థిరరాశి అక్షరం *strfrmt = luaL_checklstring(L, arg, &sfl);
  స్థిరరాశి అక్షరం *strfrmt_చివర = strfrmt+sfl;
  స్థిరరాశి అక్షరం *flags;
  lua_L_బఫరు b;
  luaL_buffinit(L, &b);
  అయితే (strfrmt < strfrmt_చివర) {
    ఒకవేళ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    లేకపోతే ఒకవేళ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    లేకపోతే { /* format item */
      అక్షరం form[MAX_FORMAT];  /* to store the format ('%...') */
      గుర్తులేని maxitem = MAX_అంశం;  /* maximum length for the result */
      అక్షరం *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      పూర్ణాంకం nb = 0;  /* number of bytes in result */
      ఒకవేళ (++arg > శిఖరం)
        తిరిగి luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      ఎంపిక (*strfrmt++) {
        స్థితి 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (పూర్ణాంకం)luaL_checkinteger(L, arg));
          ఆపు;
        }
        స్థితి 'd': స్థితి 'i':
          flags = L_FMTFLAGSI;
          వెళ్ళు intcase;
        స్థితి 'u':
          flags = L_FMTFLAGSU;
          వెళ్ళు intcase;
        స్థితి 'o': స్థితి 'x': స్థితి 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          ఆపు;
        }
        స్థితి 'a': స్థితి 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_సంఖ్య_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          ఆపు;
        స్థితి 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        స్థితి 'e': స్థితి 'E': స్థితి 'g': స్థితి 'G': {
          lua_సంఖ్య n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_సంఖ్య_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          ఆపు;
        }
        స్థితి 'p': {
          స్థిరరాశి శూన్య *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          ఒకవేళ (p == ఖాళీ) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /*ఫలితం*/
            form[అక్షరశ్రేణి_పొడవు(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          ఆపు;
        }
        స్థితి 'q': {
          ఒకవేళ (form[2] != '\0')  /* modifiers? */
            తిరిగి luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          ఆపు;
        }
        స్థితి 's': {
          size_t l;
          స్థిరరాశి అక్షరం *s = luaL_tolstring(L, arg, &l);
          ఒకవేళ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          లేకపోతే {
            luaL_argcheck(L, l == అక్షరశ్రేణి_పొడవు(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            ఒకవేళ (strchr(form, '.') == ఖాళీ && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            లేకపోతే {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_తీయి(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          ఆపు;
        }
        అప్రమేయ: {  /* also treat cases 'pnLlh' */
          తిరిగి luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  తిరిగి 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#ఒకవేళ !defined(LUAL_PACKPADBYTE)
#నిర్వచించు LUAL_PACKPADBYTE		0x00
#అయితే_ముగింపు

/* maximum size for the binary representation of an integer */
#నిర్వచించు MAXINTSIZE	16

/* number of bits in a character */
#నిర్వచించు NB	CHAR_BIT

/* mask for one character (NB 1's) */
#నిర్వచించు MC	((1 << NB) - 1)

/* size of a lua_Integer */
#నిర్వచించు SZINT	((పూర్ణాంకం)పరిమాణం(lua_Integer))


/* dummy union to get native endianness */
స్థిర స్థిరరాశి సంఘం {
  పూర్ణాంకం dummy;
  అక్షరం little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
రకనిర్వచనం నిర్మాణం శీర్షిక {
  lua_State *L;
  పూర్ణాంకం islittle;
  గుర్తులేని maxalign;
} శీర్షిక;


/*
** options for pack/unpack
*/
రకనిర్వచనం గణన KOption {
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
స్థిర పూర్ణాంకం digit (పూర్ణాంకం c) { తిరిగి '0' <= c && c <= '9'; }

స్థిర size_t getnum (స్థిరరాశి అక్షరం **fmt, size_t df) {
  ఒకవేళ (!digit(**fmt))  /* no number? */
    తిరిగి df;  /* return default value */
  లేకపోతే {
    size_t a = 0;
    చేయి {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } అయితే (digit(**fmt) && a <= (MAX_కొలత - 9)/10);
    తిరిగి a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
స్థిర గుర్తులేని getnumlimit (శీర్షిక *h, స్థిరరాశి అక్షరం **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  ఒకవేళ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    తిరిగి cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  తిరిగి cast_uint(sz);
}


/*
** Initialize Header
*/
స్థిర శూన్య initheader (lua_State *L, శీర్షిక *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
స్థిర KOption getoption (శీర్షిక *h, స్థిరరాశి అక్షరం **fmt, size_t *కొలత) {
  /* dummy structure to get native alignment requirements */
  నిర్మాణం cD { అక్షరం c; సంఘం { LUAI_MAXALIGN; } u; };
  పూర్ణాంకం opt = *((*fmt)++);
  *కొలత = 0;  /* default */
  ఎంపిక (opt) {
    స్థితి 'b': *కొలత = పరిమాణం(అక్షరం); తిరిగి Kint;
    స్థితి 'B': *కొలత = పరిమాణం(అక్షరం); తిరిగి Kuint;
    స్థితి 'h': *కొలత = పరిమాణం(పొట్టి); తిరిగి Kint;
    స్థితి 'H': *కొలత = పరిమాణం(పొట్టి); తిరిగి Kuint;
    స్థితి 'l': *కొలత = పరిమాణం(దీర్ఘ); తిరిగి Kint;
    స్థితి 'L': *కొలత = పరిమాణం(దీర్ఘ); తిరిగి Kuint;
    స్థితి 'j': *కొలత = పరిమాణం(lua_Integer); తిరిగి Kint;
    స్థితి 'J': *కొలత = పరిమాణం(lua_Integer); తిరిగి Kuint;
    స్థితి 'T': *కొలత = పరిమాణం(size_t); తిరిగి Kuint;
    స్థితి 'f': *కొలత = పరిమాణం(దశాంశం); తిరిగి Kfloat;
    స్థితి 'n': *కొలత = పరిమాణం(lua_సంఖ్య); తిరిగి Knumber;
    స్థితి 'd': *కొలత = పరిమాణం(ద్విగుణ); తిరిగి Kdouble;
    స్థితి 'i': *కొలత = getnumlimit(h, fmt, పరిమాణం(పూర్ణాంకం)); తిరిగి Kint;
    స్థితి 'I': *కొలత = getnumlimit(h, fmt, పరిమాణం(పూర్ణాంకం)); తిరిగి Kuint;
    స్థితి 's': *కొలత = getnumlimit(h, fmt, పరిమాణం(size_t)); తిరిగి Kstring;
    స్థితి 'c':
      *కొలత = getnum(fmt, cast_sizet(-1));
      ఒకవేళ (l_unlikely(*కొలత == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      తిరిగి Kchar;
    స్థితి 'z': తిరిగి Kzstr;
    స్థితి 'x': *కొలత = 1; తిరిగి Kpadding;
    స్థితి 'X': తిరిగి Kpaddalign;
    స్థితి ' ': ఆపు;
    స్థితి '<': h->islittle = 1; ఆపు;
    స్థితి '>': h->islittle = 0; ఆపు;
    స్థితి '=': h->islittle = nativeendian.little; ఆపు;
    స్థితి '!': {
      స్థిరరాశి size_t maxalign = offsetof(నిర్మాణం cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      ఆపు;
    }
    అప్రమేయ: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  తిరిగి Knop;
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
స్థిర KOption getdetails (శీర్షిక *h, size_t totalsize, స్థిరరాశి అక్షరం **fmt,
                           size_t *psize, గుర్తులేని *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  ఒకవేళ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    ఒకవేళ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  ఒకవేళ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  లేకపోతే {
    ఒకవేళ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    ఒకవేళ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    లేకపోతే {
      /* 'szmoda' = totalsize % align */
      గుర్తులేని szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  తిరిగి opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
స్థిర శూన్య packint (lua_L_బఫరు *b, lua_Unsigned n,
                     పూర్ణాంకం islittle, గుర్తులేని కొలత, పూర్ణాంకం neg) {
  అక్షరం *buff = luaL_prepbuffsize(b, కొలత);
  గుర్తులేని i;
  buff[islittle ? 0 : కొలత - 1] = (అక్షరం)(n & MC);  /* first byte */
  కొరకు (i = 1; i < కొలత; i++) {
    n >>= NB;
    buff[islittle ? i : కొలత - 1 - i] = (అక్షరం)(n & MC);
  }
  ఒకవేళ (neg && కొలత > SZINT) {  /* negative number need sign extension? */
    కొరకు (i = SZINT; i < కొలత; i++)  /* correct extra bytes */
      buff[islittle ? i : కొలత - 1 - i] = (అక్షరం)MC;
  }
  luaL_addsize(b, కొలత);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
స్థిర శూన్య copywithendian (అక్షరం *dest, స్థిరరాశి అక్షరం *src,
                            గుర్తులేని కొలత, పూర్ణాంకం islittle) {
  ఒకవేళ (islittle == nativeendian.little)
    జ్ఞాపకం_నకలు(dest, src, కొలత);
  లేకపోతే {
    dest += కొలత - 1;
    అయితే (కొలత-- != 0)
      *(dest--) = *(src++);
  }
}


స్థిర పూర్ణాంకం str_pack (lua_State *L) {
  lua_L_బఫరు b;
  శీర్షిక h;
  స్థిరరాశి అక్షరం *fmt = luaL_checkstring(L, 1);  /* format string */
  పూర్ణాంకం arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  అయితే (*fmt != '\0') {
    గుర్తులేని ntoalign;
    size_t కొలత;
    KOption opt = getdetails(&h, totalsize, &fmt, &కొలత, &ntoalign);
    luaL_argcheck(L, కొలత + ntoalign <= MAX_కొలత - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + కొలత;
    అయితే (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    ఎంపిక (opt) {
      స్థితి Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ఒకవేళ (కొలత < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((కొలత * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(కొలత), (n < 0));
        ఆపు;
      }
      స్థితి Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ఒకవేళ (కొలత < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (కొలత * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(కొలత), 0);
        ఆపు;
      }
      స్థితి Kfloat: {  /* C float */
        దశాంశం f = (దశాంశం)luaL_checknumber(L, arg);  /* get argument */
        అక్షరం *buff = luaL_prepbuffsize(&b, పరిమాణం(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (అక్షరం *)&f, పరిమాణం(f), h.islittle);
        luaL_addsize(&b, కొలత);
        ఆపు;
      }
      స్థితి Knumber: {  /* Lua float */
        lua_సంఖ్య f = luaL_checknumber(L, arg);  /* get argument */
        అక్షరం *buff = luaL_prepbuffsize(&b, పరిమాణం(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (అక్షరం *)&f, పరిమాణం(f), h.islittle);
        luaL_addsize(&b, కొలత);
        ఆపు;
      }
      స్థితి Kdouble: {  /* C double */
        ద్విగుణ f = (ద్విగుణ)luaL_checknumber(L, arg);  /* get argument */
        అక్షరం *buff = luaL_prepbuffsize(&b, పరిమాణం(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (అక్షరం *)&f, పరిమాణం(f), h.islittle);
        luaL_addsize(&b, కొలత);
        ఆపు;
      }
      స్థితి Kchar: {  /* fixed-size string */
        size_t len;
        స్థిరరాశి అక్షరం *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= కొలత, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        ఒకవేళ (len < కొలత) {  /* does it need padding? */
          size_t psize = కొలత - len;  /* pad size */
          అక్షరం *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        ఆపు;
      }
      స్థితి Kstring: {  /* strings with length count */
        size_t len;
        స్థిరరాశి అక్షరం *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, కొలత >= పరిమాణం(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (కొలత * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(కొలత), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        ఆపు;
      }
      స్థితి Kzstr: {  /* zero-terminated string */
        size_t len;
        స్థిరరాశి అక్షరం *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, అక్షరశ్రేణి_పొడవు(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        ఆపు;
      }
      స్థితి Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      స్థితి Kpaddalign: స్థితి Knop:
        arg--;  /* undo increment */
        ఆపు;
    }
  }
  luaL_pushresult(&b);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం str_packsize (lua_State *L) {
  శీర్షిక h;
  స్థిరరాశి అక్షరం *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  అయితే (*fmt != '\0') {
    గుర్తులేని ntoalign;
    size_t కొలత;
    KOption opt = getdetails(&h, totalsize, &fmt, &కొలత, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    కొలత += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_కొలత - కొలత,
                     1, "format result too large");
    totalsize += కొలత;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  తిరిగి 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
స్థిర lua_Integer unpackint (lua_State *L, స్థిరరాశి అక్షరం *str,
                              పూర్ణాంకం islittle, పూర్ణాంకం కొలత, పూర్ణాంకం issigned) {
  lua_Unsigned res = 0;
  పూర్ణాంకం i;
  పూర్ణాంకం హద్దు = (కొలత  <= SZINT) ? కొలత : SZINT;
  కొరకు (i = హద్దు - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(గుర్తులేని అక్షరం)str[islittle ? i : కొలత - 1 - i];
  }
  ఒకవేళ (కొలత < SZINT) {  /* real size smaller than lua_Integer? */
    ఒకవేళ (issigned) {  /* needs sign extension? */
      lua_Unsigned ముసుగు_2 = (lua_Unsigned)1 << (కొలత*NB - 1);
      res = ((res ^ ముసుగు_2) - ముసుగు_2);  /* do sign extension */
    }
  }
  లేకపోతే ఒకవేళ (కొలత > SZINT) {  /* must check unread bytes */
    పూర్ణాంకం ముసుగు_2 = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    కొరకు (i = హద్దు; i < కొలత; i++) {
      ఒకవేళ (l_unlikely((గుర్తులేని అక్షరం)str[islittle ? i : కొలత - 1 - i] != ముసుగు_2))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", కొలత);
    }
  }
  తిరిగి (lua_Integer)res;
}


స్థిర పూర్ణాంకం str_unpack (lua_State *L) {
  శీర్షిక h;
  స్థిరరాశి అక్షరం *fmt = luaL_checkstring(L, 1);
  size_t ld;
  స్థిరరాశి అక్షరం *దత్తాంశం = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  పూర్ణాంకం n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  అయితే (*fmt != '\0') {
    గుర్తులేని ntoalign;
    size_t కొలత;
    KOption opt = getdetails(&h, pos, &fmt, &కొలత, &ntoalign);
    luaL_argcheck(L, ntoalign + కొలత <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    ఎంపిక (opt) {
      స్థితి Kint:
      స్థితి Kuint: {
        lua_Integer res = unpackint(L, దత్తాంశం + pos, h.islittle,
                                       cast_int(కొలత), (opt == Kint));
        lua_pushinteger(L, res);
        ఆపు;
      }
      స్థితి Kfloat: {
        దశాంశం f;
        copywithendian((అక్షరం *)&f, దత్తాంశం + pos, పరిమాణం(f), h.islittle);
        lua_pushnumber(L, (lua_సంఖ్య)f);
        ఆపు;
      }
      స్థితి Knumber: {
        lua_సంఖ్య f;
        copywithendian((అక్షరం *)&f, దత్తాంశం + pos, పరిమాణం(f), h.islittle);
        lua_pushnumber(L, f);
        ఆపు;
      }
      స్థితి Kdouble: {
        ద్విగుణ f;
        copywithendian((అక్షరం *)&f, దత్తాంశం + pos, పరిమాణం(f), h.islittle);
        lua_pushnumber(L, (lua_సంఖ్య)f);
        ఆపు;
      }
      స్థితి Kchar: {
        lua_pushlstring(L, దత్తాంశం + pos, కొలత);
        ఆపు;
      }
      స్థితి Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, దత్తాంశం + pos,
                                          h.islittle, cast_int(కొలత), 0);
        luaL_argcheck(L, len <= ld - pos - కొలత, 2, "data string too short");
        lua_pushlstring(L, దత్తాంశం + pos + కొలత, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        ఆపు;
      }
      స్థితి Kzstr: {
        size_t len = అక్షరశ్రేణి_పొడవు(దత్తాంశం + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, దత్తాంశం + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        ఆపు;
      }
      స్థితి Kpaddalign: స్థితి Kpadding: స్థితి Knop:
        n--;  /* undo increment */
        ఆపు;
    }
    pos += కొలత;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  తిరిగి n + 1;
}

/* }====================================================== */


స్థిర స్థిరరాశి luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_కనుగొను},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_క్రింది},
  {"match", str_సరిపోలిక},
  {"rep", str_rep},
  {"reverse", str_తిరగేయి},
  {"sub", str_sub},
  {"upper", str_పై},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {ఖాళీ, ఖాళీ}
};


స్థిర శూన్య createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_తీయి(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_తీయి(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API పూర్ణాంకం luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  తిరిగి 1;
}

