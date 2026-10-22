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
#जर !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


स्थिर पूर्णांक str_len (lua_State *L) {
  माप_प्रकार l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  परत 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
स्थिर माप_प्रकार posrelatI (lua_Integer pos, माप_प्रकार len) {
  जर (pos > 0)
    परत (माप_प्रकार)pos;
  अन्यथा जर (pos == 0)
    परत 1;
  अन्यथा जर (pos < -(lua_Integer)len)  /* inverted comparison */
    परत 1;  /* clip to 1 */
  अन्यथा परत len + (माप_प्रकार)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
स्थिर माप_प्रकार getendpos (lua_State *L, पूर्णांक arg, lua_Integer def,
                         माप_प्रकार len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  जर (pos > (lua_Integer)len)
    परत len;
  अन्यथा जर (pos >= 0)
    परत (माप_प्रकार)pos;
  अन्यथा जर (pos < -(lua_Integer)len)
    परत 0;
  अन्यथा परत len + (माप_प्रकार)pos + 1;
}


स्थिर पूर्णांक str_sub (lua_State *L) {
  माप_प्रकार l;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &l);
  माप_प्रकार start = posrelatI(luaL_checkinteger(L, 2), l);
  माप_प्रकार end = getendpos(L, 3, -1, l);
  जर (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  अन्यथा lua_pushliteral(L, "");
  परत 1;
}


स्थिर पूर्णांक str_reverse (lua_State *L) {
  माप_प्रकार l, i;
  luaL_Buffer b;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &l);
  अक्षर *p = luaL_buffinitsize(L, &b, l);
  साठी (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  परत 1;
}


स्थिर पूर्णांक str_lower (lua_State *L) {
  माप_प्रकार l;
  माप_प्रकार i;
  luaL_Buffer b;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &l);
  अक्षर *p = luaL_buffinitsize(L, &b, l);
  साठी (i=0; i<l; i++)
    p[i] = cast_char(छोटे(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  परत 1;
}


स्थिर पूर्णांक str_upper (lua_State *L) {
  माप_प्रकार l;
  माप_प्रकार i;
  luaL_Buffer b;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &l);
  अक्षर *p = luaL_buffinitsize(L, &b, l);
  साठी (i=0; i<l; i++)
    p[i] = cast_char(बड़े(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  परत 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
स्थिर पूर्णांक str_rep (lua_State *L) {
  माप_प्रकार len, lsep;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  स्थिरांक अक्षर *sep = luaL_optlstring(L, 3, "", &lsep);
  जर (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  अन्यथा जर (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    परत luaL_error(L, "resulting string too large");
  अन्यथा {
    माप_प्रकार totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    अक्षर *p = luaL_buffinitsize(L, &b, totallen);
    पर्यंत (n-- > 1) {  /* first n-1 copies (followed by separator) */
      स_नकल(p, s, len * आकार(अक्षर)); p += len;
      जर (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        स_नकल(p, sep, lsep * आकार(अक्षर)); p += lsep;
      }
    }
    स_नकल(p, s, len * आकार(अक्षर));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  परत 1;
}


स्थिर पूर्णांक str_byte (lua_State *L) {
  माप_प्रकार l;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  माप_प्रकार posi = posrelatI(pi, l);
  माप_प्रकार pose = getendpos(L, 3, pi, l);
  पूर्णांक n, i;
  जर (posi > pose) परत 0;  /* empty interval; return no values */
  जर (l_unlikely(pose - posi >= (माप_प्रकार)पूर्णांक_उच्च))  /* arithmetic overflow? */
    परत luaL_error(L, "string slice too long");
  n = (पूर्णांक)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  साठी (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  परत n;
}


स्थिर पूर्णांक str_char (lua_State *L) {
  पूर्णांक n = lua_gettop(L);  /* number of arguments */
  पूर्णांक i;
  luaL_Buffer b;
  अक्षर *p = luaL_buffinitsize(L, &b, cast_uint(n));
  साठी (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)अच_अक्षर_उच्च, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  परत 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
संरचना str_Writer {
  पूर्णांक init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


स्थिर पूर्णांक writer (lua_State *L, स्थिरांक शून्य *b, माप_प्रकार size, शून्य *ud) {
  संरचना str_Writer *state = (संरचना str_Writer *)ud;
  जर (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  जर (b == रिक्त) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  अन्यथा
    luaL_addlstring(&state->B, (स्थिरांक अक्षर *)b, size);
  परत 0;
}


स्थिर पूर्णांक str_dump (lua_State *L) {
  संरचना str_Writer state;
  पूर्णांक strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  परत 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#जर defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

स्थिर स्थिरांक luaL_Reg stringmetamethods[] = {
  {"__index", रिक्त},  /* placeholder */
  {रिक्त, रिक्त}
};

#अन्यथा		/* }{ */

स्थिर पूर्णांक tonum (lua_State *L, पूर्णांक arg) {
  जर (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    परत 1;
  }
  अन्यथा {  /* check whether it is a numerical string */
    माप_प्रकार len;
    स्थिरांक अक्षर *s = lua_tolstring(L, arg, &len);
    परत (s != रिक्त && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
स्थिर शून्य trymt (lua_State *L, स्थिरांक अक्षर *mtkey, स्थिरांक अक्षर *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  जर (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_घाला(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


स्थिर पूर्णांक arith (lua_State *L, पूर्णांक op, स्थिरांक अक्षर *mtname) {
  जर (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  अन्यथा
    trymt(L, mtname, mtname + 2);
  परत 1;
}


स्थिर पूर्णांक arith_add (lua_State *L) {
  परत arith(L, LUA_OPADD, "__add");
}

स्थिर पूर्णांक arith_sub (lua_State *L) {
  परत arith(L, LUA_OPSUB, "__sub");
}

स्थिर पूर्णांक arith_mul (lua_State *L) {
  परत arith(L, LUA_OPMUL, "__mul");
}

स्थिर पूर्णांक arith_mod (lua_State *L) {
  परत arith(L, LUA_OPMOD, "__mod");
}

स्थिर पूर्णांक arith_pow (lua_State *L) {
  परत arith(L, LUA_OPPOW, "__pow");
}

स्थिर पूर्णांक arith_div (lua_State *L) {
  परत arith(L, LUA_OPDIV, "__div");
}

स्थिर पूर्णांक arith_idiv (lua_State *L) {
  परत arith(L, LUA_OPIDIV, "__idiv");
}

स्थिर पूर्णांक arith_unm (lua_State *L) {
  परत arith(L, LUA_OPUNM, "__unm");
}


स्थिर स्थिरांक luaL_Reg stringmetamethods[] = {
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


प्रकारव्याख्या संरचना MatchState {
  स्थिरांक अक्षर *src_init;  /* init of source string */
  स्थिरांक अक्षर *src_end;  /* end ('\0') of source string */
  स्थिरांक अक्षर *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  पूर्णांक matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  पूर्णांक level;  /* total number of captures (finished or unfinished) */
  संरचना {
    स्थिरांक अक्षर *init;
    सूचक_भेद_प्रकार len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
स्थिर स्थिरांक अक्षर *match (MatchState *ms, स्थिरांक अक्षर *s, स्थिरांक अक्षर *p);


/* maximum recursion depth for 'match' */
#जर !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


स्थिर पूर्णांक check_capture (MatchState *ms, पूर्णांक l) {
  l -= '1';
  जर (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    परत luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  परत l;
}


स्थिर पूर्णांक capture_to_close (MatchState *ms) {
  पूर्णांक level = ms->level;
  साठी (level--; level>=0; level--)
    जर (ms->capture[level].len == CAP_UNFINISHED) परत level;
  परत luaL_error(ms->L, "invalid pattern capture");
}


स्थिर स्थिरांक अक्षर *classend (MatchState *ms, स्थिरांक अक्षर *p) {
  निवड (*p++) {
    स्थिती L_ESC: {
      जर (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      परत p+1;
    }
    स्थिती '[': {
      जर (*p == '^') p++;
      करा {  /* look for a ']' */
        जर (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        जर (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } पर्यंत (*p != ']');
      परत p+1;
    }
    पूर्वनिर्धारित: {
      परत p;
    }
  }
}


स्थिर पूर्णांक match_class (पूर्णांक c, पूर्णांक cl) {
  पूर्णांक res;
  निवड (छोटे(cl)) {
    स्थिती 'a' : res = है_अक्षर(c); थांबा;
    स्थिती 'c' : res = है_नियंत्रण(c); थांबा;
    स्थिती 'd' : res = है_अंक(c); थांबा;
    स्थिती 'g' : res = है_चित्र(c); थांबा;
    स्थिती 'l' : res = है_छोटा(c); थांबा;
    स्थिती 'p' : res = है_विराम(c); थांबा;
    स्थिती 's' : res = है_खाली(c); थांबा;
    स्थिती 'u' : res = है_बड़ा(c); थांबा;
    स्थिती 'w' : res = है_अक्षर_अंक(c); थांबा;
    स्थिती 'x' : res = है_षष्ठादशक(c); थांबा;
    स्थिती 'z' : res = (c == 0); थांबा;  /* deprecated option */
    पूर्वनिर्धारित: परत (cl == c);
  }
  परत (है_छोटा(cl) ? res : !res);
}


स्थिर पूर्णांक matchbracketclass (पूर्णांक c, स्थिरांक अक्षर *p, स्थिरांक अक्षर *ec) {
  पूर्णांक sig = 1;
  जर (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  पर्यंत (++p < ec) {
    जर (*p == L_ESC) {
      p++;
      जर (match_class(c, cast_uchar(*p)))
        परत sig;
    }
    अन्यथा जर ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      जर (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        परत sig;
    }
    अन्यथा जर (cast_uchar(*p) == c) परत sig;
  }
  परत !sig;
}


स्थिर पूर्णांक singlematch (MatchState *ms, स्थिरांक अक्षर *s, स्थिरांक अक्षर *p,
                        स्थिरांक अक्षर *ep) {
  जर (s >= ms->src_end)
    परत 0;
  अन्यथा {
    पूर्णांक c = cast_uchar(*s);
    निवड (*p) {
      स्थिती '.': परत 1;  /* matches any char */
      स्थिती L_ESC: परत match_class(c, cast_uchar(*(p+1)));
      स्थिती '[': परत matchbracketclass(c, p, ep-1);
      पूर्वनिर्धारित:  परत (cast_uchar(*p) == c);
    }
  }
}


स्थिर स्थिरांक अक्षर *matchbalance (MatchState *ms, स्थिरांक अक्षर *s,
                                   स्थिरांक अक्षर *p) {
  जर (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  जर (*s != *p) परत रिक्त;
  अन्यथा {
    पूर्णांक b = *p;
    पूर्णांक e = *(p+1);
    पूर्णांक cont = 1;
    पर्यंत (++s < ms->src_end) {
      जर (*s == e) {
        जर (--cont == 0) परत s+1;
      }
      अन्यथा जर (*s == b) cont++;
    }
  }
  परत रिक्त;  /* string ends out of balance */
}


स्थिर स्थिरांक अक्षर *max_expand (MatchState *ms, स्थिरांक अक्षर *s,
                                 स्थिरांक अक्षर *p, स्थिरांक अक्षर *ep) {
  सूचक_भेद_प्रकार i = 0;  /* counts maximum expand for item */
  पर्यंत (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  पर्यंत (i>=0) {
    स्थिरांक अक्षर *res = match(ms, (s+i), ep+1);
    जर (res) परत res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  परत रिक्त;
}


स्थिर स्थिरांक अक्षर *min_expand (MatchState *ms, स्थिरांक अक्षर *s,
                                 स्थिरांक अक्षर *p, स्थिरांक अक्षर *ep) {
  साठी (;;) {
    स्थिरांक अक्षर *res = match(ms, s, ep+1);
    जर (res != रिक्त)
      परत res;
    अन्यथा जर (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    अन्यथा परत रिक्त;
  }
}


स्थिर स्थिरांक अक्षर *start_capture (MatchState *ms, स्थिरांक अक्षर *s,
                                    स्थिरांक अक्षर *p, पूर्णांक what) {
  स्थिरांक अक्षर *res;
  पूर्णांक level = ms->level;
  जर (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  जर ((res=match(ms, s, p)) == रिक्त)  /* match failed? */
    ms->level--;  /* undo capture */
  परत res;
}


स्थिर स्थिरांक अक्षर *end_capture (MatchState *ms, स्थिरांक अक्षर *s,
                                  स्थिरांक अक्षर *p) {
  पूर्णांक l = capture_to_close(ms);
  स्थिरांक अक्षर *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  जर ((res = match(ms, s, p)) == रिक्त)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  परत res;
}


स्थिर स्थिरांक अक्षर *match_capture (MatchState *ms, स्थिरांक अक्षर *s, पूर्णांक l) {
  माप_प्रकार len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  जर ((माप_प्रकार)(ms->src_end-s) >= len &&
      स_भेद(ms->capture[l].init, s, len) == 0)
    परत s+len;
  अन्यथा परत रिक्त;
}


स्थिर स्थिरांक अक्षर *match (MatchState *ms, स्थिरांक अक्षर *s, स्थिरांक अक्षर *p) {
  जर (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  जर (p != ms->p_end) {  /* end of pattern? */
    निवड (*p) {
      स्थिती '(': {  /* start capture */
        जर (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        अन्यथा
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        थांबा;
      }
      स्थिती ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        थांबा;
      }
      स्थिती '$': {
        जर ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          जा dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : रिक्त;  /* check end of string */
        थांबा;
      }
      स्थिती L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        निवड (*(p + 1)) {
          स्थिती 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            जर (s != रिक्त) {
              p += 4; जा init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            थांबा;
          }
          स्थिती 'f': {  /* frontier? */
            स्थिरांक अक्षर *ep; अक्षर previous;
            p += 2;
            जर (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            जर (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; जा init;  /* return match(ms, s, ep); */
            }
            s = रिक्त;  /* match failed */
            थांबा;
          }
          स्थिती '0': स्थिती '1': स्थिती '2': स्थिती '3':
          स्थिती '4': स्थिती '5': स्थिती '6': स्थिती '7':
          स्थिती '8': स्थिती '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            जर (s != रिक्त) {
              p += 2; जा init;  /* return match(ms, s, p + 2) */
            }
            थांबा;
          }
          पूर्वनिर्धारित: जा dflt;
        }
        थांबा;
      }
      पूर्वनिर्धारित: dflt: {  /* pattern class plus optional suffix */
        स्थिरांक अक्षर *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        जर (!singlematch(ms, s, p, ep)) {
          जर (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; जा init;  /* return match(ms, s, ep + 1); */
          }
          अन्यथा  /* '+' or no suffix */
            s = रिक्त;  /* fail */
        }
        अन्यथा {  /* matched once */
          निवड (*ep) {  /* handle optional suffix */
            स्थिती '?': {  /* optional */
              स्थिरांक अक्षर *res;
              जर ((res = match(ms, s + 1, ep + 1)) != रिक्त)
                s = res;
              अन्यथा {
                p = ep + 1; जा init;  /* else return match(ms, s, ep + 1); */
              }
              थांबा;
            }
            स्थिती '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            स्थिती '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              थांबा;
            स्थिती '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              थांबा;
            पूर्वनिर्धारित:  /* no suffix */
              s++; p = ep; जा init;  /* return match(ms, s + 1, ep); */
          }
        }
        थांबा;
      }
    }
  }
  ms->matchdepth++;
  परत s;
}



स्थिर स्थिरांक अक्षर *lmemfind (स्थिरांक अक्षर *s1, माप_प्रकार l1,
                               स्थिरांक अक्षर *s2, माप_प्रकार l2) {
  जर (l2 == 0) परत s1;  /* empty strings are everywhere */
  अन्यथा जर (l2 > l1) परत रिक्त;  /* avoids a negative 'l1' */
  अन्यथा {
    स्थिरांक अक्षर *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    पर्यंत (l1 > 0 && (init = (स्थिरांक अक्षर *)स_प्रथम(s1, *s2, l1)) != रिक्त) {
      init++;   /* 1st char is already checked */
      जर (स_भेद(init, s2+1, l2) == 0)
        परत init-1;
      अन्यथा {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    परत रिक्त;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
स्थिर सूचक_भेद_प्रकार get_onecapture (MatchState *ms, पूर्णांक i, स्थिरांक अक्षर *s,
                              स्थिरांक अक्षर *e, स्थिरांक अक्षर **cap) {
  जर (i >= ms->level) {
    जर (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    परत (e - s);
  }
  अन्यथा {
    सूचक_भेद_प्रकार capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    जर (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    अन्यथा जर (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    परत capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
स्थिर शून्य ढकला_onecapture (MatchState *ms, पूर्णांक i, स्थिरांक अक्षर *s,
                                                    स्थिरांक अक्षर *e) {
  स्थिरांक अक्षर *cap;
  सूचक_भेद_प्रकार l = get_onecapture(ms, i, s, e, &cap);
  जर (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


स्थिर पूर्णांक ढकला_captures (MatchState *ms, स्थिरांक अक्षर *s, स्थिरांक अक्षर *e) {
  पूर्णांक i;
  पूर्णांक nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  साठी (i = 0; i < nlevels; i++)
    ढकला_onecapture(ms, i, s, e);
  परत nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
स्थिर पूर्णांक nospecials (स्थिरांक अक्षर *p, माप_प्रकार l) {
  माप_प्रकार upto = 0;
  करा {
    जर (strpbrk(p + upto, SPECIALS))
      परत 0;  /* pattern has a special character */
    upto += म_माप(p + upto) + 1;  /* may have more after \0 */
  } पर्यंत (upto <= l);
  परत 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
स्थिर शून्य prepstate (MatchState *ms, lua_State *L,
                       स्थिरांक अक्षर *s, माप_प्रकार ls, स्थिरांक अक्षर *p, माप_प्रकार lp) {
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


स्थिर पूर्णांक str_find_aux (lua_State *L, पूर्णांक find) {
  माप_प्रकार ls, lp;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &ls);
  स्थिरांक अक्षर *p = luaL_checklstring(L, 2, &lp);
  माप_प्रकार init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  जर (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    परत 1;
  }
  /* explicit request or no special characters? */
  जर (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    स्थिरांक अक्षर *s2 = lmemfind(s + init, ls - init, p, lp);
    जर (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      परत 2;
    }
  }
  अन्यथा {
    MatchState ms;
    स्थिरांक अक्षर *s1 = s + init;
    पूर्णांक anchor = (*p == '^');
    जर (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    करा {
      स्थिरांक अक्षर *res;
      reprepstate(&ms);
      जर ((res=match(&ms, s1, p)) != रिक्त) {
        जर (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          परत ढकला_captures(&ms, रिक्त, 0) + 2;
        }
        अन्यथा
          परत ढकला_captures(&ms, s1, res);
      }
    } पर्यंत (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  परत 1;
}


स्थिर पूर्णांक str_find (lua_State *L) {
  परत str_find_aux(L, 1);
}


स्थिर पूर्णांक str_match (lua_State *L) {
  परत str_find_aux(L, 0);
}


/* state for 'gmatch' */
प्रकारव्याख्या संरचना GMatchState {
  स्थिरांक अक्षर *src;  /* current position */
  स्थिरांक अक्षर *p;  /* pattern */
  स्थिरांक अक्षर *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


स्थिर पूर्णांक gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  स्थिरांक अक्षर *src;
  gm->ms.L = L;
  साठी (src = gm->src; src <= gm->ms.src_end; src++) {
    स्थिरांक अक्षर *e;
    reprepstate(&gm->ms);
    जर ((e = match(&gm->ms, src, gm->p)) != रिक्त && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      परत ढकला_captures(&gm->ms, src, e);
    }
  }
  परत 0;  /* not found */
}


स्थिर पूर्णांक gmatch (lua_State *L) {
  माप_प्रकार ls, lp;
  स्थिरांक अक्षर *s = luaL_checklstring(L, 1, &ls);
  स्थिरांक अक्षर *p = luaL_checklstring(L, 2, &lp);
  माप_प्रकार init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, आकार(GMatchState), 0);
  जर (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = रिक्त;
  lua_pushcclosure(L, gmatch_aux, 3);
  परत 1;
}


स्थिर शून्य add_s (MatchState *ms, luaL_Buffer *b, स्थिरांक अक्षर *s,
                                                   स्थिरांक अक्षर *e) {
  माप_प्रकार l;
  lua_State *L = ms->L;
  स्थिरांक अक्षर *news = lua_tolstring(L, 3, &l);
  स्थिरांक अक्षर *p;
  पर्यंत ((p = (अक्षर *)स_प्रथम(news, L_ESC, l)) != रिक्त) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    जर (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    अन्यथा जर (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    अन्यथा जर (है_अंक(cast_uchar(*p))) {  /* '%n' */
      स्थिरांक अक्षर *cap;
      सूचक_भेद_प्रकार resl = get_onecapture(ms, *p - '1', s, e, &cap);
      जर (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      अन्यथा
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    अन्यथा
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
स्थिर पूर्णांक add_मूल्य (MatchState *ms, luaL_Buffer *b, स्थिरांक अक्षर *s,
                                      स्थिरांक अक्षर *e, पूर्णांक tr) {
  lua_State *L = ms->L;
  निवड (tr) {
    स्थिती LUA_TFUNCTION: {  /* call the function */
      पूर्णांक n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ढकला_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      थांबा;
    }
    स्थिती LUA_TTABLE: {  /* index the table */
      ढकला_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      थांबा;
    }
    पूर्वनिर्धारित: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      परत 1;  /* something changed */
    }
  }
  जर (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_काढा(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    परत 0;  /* no changes */
  }
  अन्यथा जर (l_unlikely(!lua_isstring(L, -1)))
    परत luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  अन्यथा {
    luaL_addvalue(b);  /* add result to accumulator */
    परत 1;  /* something changed */
  }
}


स्थिर पूर्णांक str_gsub (lua_State *L) {
  माप_प्रकार srcl, lp;
  स्थिरांक अक्षर *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  स्थिरांक अक्षर *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  स्थिरांक अक्षर *lastmatch = रिक्त;  /* end of last match */
  पूर्णांक tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  पूर्णांक anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  पूर्णांक changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  जर (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  पर्यंत (n < max_s) {
    स्थिरांक अक्षर *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    जर ((e = match(&ms, src, p)) != रिक्त && e != lastmatch) {  /* match? */
      n++;
      changed = add_मूल्य(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    अन्यथा जर (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    अन्यथा थांबा;  /* end of subject */
    जर (anchor) थांबा;
  }
  जर (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  अन्यथा {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  परत 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#जर !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(आकार(LUA_संख्या_FRMLEN)/आकार(अक्षर))


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
स्थिर lua_संख्या adddigit (अक्षर *buff, अचिन्हित n, lua_संख्या x) {
  lua_संख्या dd = l_mathop(न्यूनमान)(x);  /* get integer part from 'x' */
  पूर्णांक d = (पूर्णांक)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  परत x - dd;  /* return what is left */
}


स्थिर पूर्णांक num2straux (अक्षर *buff, अचिन्हित sz, lua_संख्या x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  जर (x != x || x == (lua_संख्या)विशाल_मान || x == -(lua_संख्या)विशाल_मान)
    परत l_sprintf(buff, sz, LUA_संख्या_FMT, (LUAI_UACNUMBER)x);
  अन्यथा जर (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    परत l_sprintf(buff, sz, LUA_संख्या_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  अन्यथा {
    पूर्णांक e;
    lua_संख्या m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    अचिन्हित n = 0;  /* character count */
    जर (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    जर (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      करा {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } पर्यंत (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    परत cast_int(n);
  }
}


स्थिर पूर्णांक lua_number2strx (lua_State *L, अक्षर *buff, अचिन्हित sz,
                            स्थिरांक अक्षर *fmt, lua_संख्या x) {
  पूर्णांक n = num2straux(buff, sz, x);
  जर (fmt[SIZELENMOD] == 'A') {
    पूर्णांक i;
    साठी (i = 0; i < n; i++)
      buff[i] = cast_char(बड़े(cast_uchar(buff[i])));
  }
  अन्यथा जर (l_unlikely(fmt[SIZELENMOD] != 'a'))
    परत luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  परत n;
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
#जर !defined(L_FMTFLAGSF)

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


स्थिर शून्य addquoted (luaL_Buffer *b, स्थिरांक अक्षर *s, माप_प्रकार len) {
  luaL_addchar(b, '"');
  पर्यंत (len--) {
    जर (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    अन्यथा जर (है_नियंत्रण(cast_uchar(*s))) {
      अक्षर buff[10];
      जर (!है_अंक(cast_uchar(*(s+1))))
        l_sprintf(buff, आकार(buff), "\\%d", (पूर्णांक)cast_uchar(*s));
      अन्यथा
        l_sprintf(buff, आकार(buff), "\\%03d", (पूर्णांक)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    अन्यथा
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
स्थिर पूर्णांक quotefloat (lua_State *L, अक्षर *buff, lua_संख्या n) {
  स्थिरांक अक्षर *s;  /* for the fixed representations */
  जर (n == (lua_संख्या)विशाल_मान)  /* inf? */
    s = "1e9999";
  अन्यथा जर (n == -(lua_संख्या)विशाल_मान)  /* -inf? */
    s = "-1e9999";
  अन्यथा जर (n != n)  /* NaN? */
    s = "(0/0)";
  अन्यथा {  /* format number as hexadecimal */
    पूर्णांक  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_संख्या_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    जर (स_प्रथम(buff, '.', cast_uint(nb)) == रिक्त) {  /* no dot? */
      अक्षर point = lua_getlocaledecpoint();  /* try locale point */
      अक्षर *ppoint = (अक्षर *)स_प्रथम(buff, point, cast_uint(nb));
      जर (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    परत nb;
  }
  /* for the fixed representations */
  परत l_sprintf(buff, MAX_ITEM, "%s", s);
}


स्थिर शून्य addliteral (lua_State *L, luaL_Buffer *b, पूर्णांक arg) {
  निवड (lua_type(L, arg)) {
    स्थिती LUA_TSTRING: {
      माप_प्रकार len;
      स्थिरांक अक्षर *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      थांबा;
    }
    स्थिती LUA_TNUMBER: {
      अक्षर *buff = luaL_prepbuffsize(b, MAX_ITEM);
      पूर्णांक nb;
      जर (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      अन्यथा {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        स्थिरांक अक्षर *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      थांबा;
    }
    स्थिती LUA_TNIL: स्थिती LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, रिक्त);
      luaL_addvalue(b);
      थांबा;
    }
    पूर्वनिर्धारित: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


स्थिर स्थिरांक अक्षर *get2digits (स्थिरांक अक्षर *s) {
  जर (है_अंक(cast_uchar(*s))) {
    s++;
    जर (है_अंक(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  परत s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
स्थिर शून्य checkformat (lua_State *L, स्थिरांक अक्षर *form, स्थिरांक अक्षर *flags,
                                       पूर्णांक precision) {
  स्थिरांक अक्षर *spec = form + 1;  /* skip '%' */
  spec += म_अखोज(spec, flags);  /* skip flags */
  जर (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    जर (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  जर (!है_अक्षर(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
स्थिर स्थिरांक अक्षर *getformat (lua_State *L, स्थिरांक अक्षर *strfrmt,
                                            अक्षर *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  माप_प्रकार len = म_अखोज(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  जर (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  स_नकल(form, strfrmt, len * आकार(अक्षर));
  *(form + len) = '\0';
  परत strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
स्थिर शून्य addlenmod (अक्षर *form, स्थिरांक अक्षर *lenmod) {
  माप_प्रकार l = म_माप(form);
  माप_प्रकार lm = म_माप(lenmod);
  अक्षर spec = form[l - 1];
  म_नकल(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


स्थिर पूर्णांक str_format (lua_State *L) {
  पूर्णांक शीर्ष = lua_gettop(L);
  पूर्णांक arg = 1;
  माप_प्रकार sfl;
  स्थिरांक अक्षर *strfrmt = luaL_checklstring(L, arg, &sfl);
  स्थिरांक अक्षर *strfrmt_end = strfrmt+sfl;
  स्थिरांक अक्षर *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  पर्यंत (strfrmt < strfrmt_end) {
    जर (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    अन्यथा जर (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    अन्यथा { /* format item */
      अक्षर form[MAX_FORMAT];  /* to store the format ('%...') */
      अचिन्हित maxitem = MAX_ITEM;  /* maximum length for the result */
      अक्षर *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      पूर्णांक nb = 0;  /* number of bytes in result */
      जर (++arg > शीर्ष)
        परत luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      निवड (*strfrmt++) {
        स्थिती 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (पूर्णांक)luaL_checkinteger(L, arg));
          थांबा;
        }
        स्थिती 'd': स्थिती 'i':
          flags = L_FMTFLAGSI;
          जा intcase;
        स्थिती 'u':
          flags = L_FMTFLAGSU;
          जा intcase;
        स्थिती 'o': स्थिती 'x': स्थिती 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          थांबा;
        }
        स्थिती 'a': स्थिती 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_संख्या_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          थांबा;
        स्थिती 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        स्थिती 'e': स्थिती 'E': स्थिती 'g': स्थिती 'G': {
          lua_संख्या n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_संख्या_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          थांबा;
        }
        स्थिती 'p': {
          स्थिरांक शून्य *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          जर (p == रिक्त) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[म_माप(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          थांबा;
        }
        स्थिती 'q': {
          जर (form[2] != '\0')  /* modifiers? */
            परत luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          थांबा;
        }
        स्थिती 's': {
          माप_प्रकार l;
          स्थिरांक अक्षर *s = luaL_tolstring(L, arg, &l);
          जर (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          अन्यथा {
            luaL_argcheck(L, l == म_माप(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            जर (म_अक्षर(form, '.') == रिक्त && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            अन्यथा {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_काढा(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          थांबा;
        }
        पूर्वनिर्धारित: {  /* also treat cases 'pnLlh' */
          परत luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  परत 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#जर !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	अक्षर_बिट

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((पूर्णांक)आकार(lua_Integer))


/* dummy union to get native endianness */
स्थिर स्थिरांक संघ {
  पूर्णांक dummy;
  अक्षर little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
प्रकारव्याख्या संरचना Header {
  lua_State *L;
  पूर्णांक islittle;
  अचिन्हित maxalign;
} Header;


/*
** options for pack/unpack
*/
प्रकारव्याख्या गणना KOption {
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
स्थिर पूर्णांक digit (पूर्णांक c) { परत '0' <= c && c <= '9'; }

स्थिर माप_प्रकार getnum (स्थिरांक अक्षर **fmt, माप_प्रकार df) {
  जर (!digit(**fmt))  /* no number? */
    परत df;  /* return default value */
  अन्यथा {
    माप_प्रकार a = 0;
    करा {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } पर्यंत (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    परत a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
स्थिर अचिन्हित getnumlimit (Header *h, स्थिरांक अक्षर **fmt, माप_प्रकार df) {
  माप_प्रकार sz = getnum(fmt, df);
  जर (l_unlikely((sz - 1u) >= MAXINTSIZE))
    परत cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  परत cast_uint(sz);
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
स्थिर KOption getoption (Header *h, स्थिरांक अक्षर **fmt, माप_प्रकार *size) {
  /* dummy structure to get native alignment requirements */
  संरचना cD { अक्षर c; संघ { LUAI_MAXALIGN; } u; };
  पूर्णांक opt = *((*fmt)++);
  *size = 0;  /* default */
  निवड (opt) {
    स्थिती 'b': *size = आकार(अक्षर); परत Kint;
    स्थिती 'B': *size = आकार(अक्षर); परत Kuint;
    स्थिती 'h': *size = आकार(लघु); परत Kint;
    स्थिती 'H': *size = आकार(लघु); परत Kuint;
    स्थिती 'l': *size = आकार(दीर्घ); परत Kint;
    स्थिती 'L': *size = आकार(दीर्घ); परत Kuint;
    स्थिती 'j': *size = आकार(lua_Integer); परत Kint;
    स्थिती 'J': *size = आकार(lua_Integer); परत Kuint;
    स्थिती 'T': *size = आकार(माप_प्रकार); परत Kuint;
    स्थिती 'f': *size = आकार(दशांश); परत Kfloat;
    स्थिती 'n': *size = आकार(lua_संख्या); परत Knumber;
    स्थिती 'd': *size = आकार(द्विगुण); परत Kdouble;
    स्थिती 'i': *size = getnumlimit(h, fmt, आकार(पूर्णांक)); परत Kint;
    स्थिती 'I': *size = getnumlimit(h, fmt, आकार(पूर्णांक)); परत Kuint;
    स्थिती 's': *size = getnumlimit(h, fmt, आकार(माप_प्रकार)); परत Kstring;
    स्थिती 'c':
      *size = getnum(fmt, cast_sizet(-1));
      जर (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      परत Kchar;
    स्थिती 'z': परत Kzstr;
    स्थिती 'x': *size = 1; परत Kpadding;
    स्थिती 'X': परत Kpaddalign;
    स्थिती ' ': थांबा;
    स्थिती '<': h->islittle = 1; थांबा;
    स्थिती '>': h->islittle = 0; थांबा;
    स्थिती '=': h->islittle = nativeendian.little; थांबा;
    स्थिती '!': {
      स्थिरांक माप_प्रकार maxalign = दुरत्व(संरचना cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      थांबा;
    }
    पूर्वनिर्धारित: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  परत Knop;
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
स्थिर KOption getdetails (Header *h, माप_प्रकार totalsize, स्थिरांक अक्षर **fmt,
                           माप_प्रकार *psize, अचिन्हित *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  माप_प्रकार align = *psize;  /* usually, alignment follows size */
  जर (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    जर (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  जर (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  अन्यथा {
    जर (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    जर (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    अन्यथा {
      /* 'szmoda' = totalsize % align */
      अचिन्हित szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  परत opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
स्थिर शून्य packint (luaL_Buffer *b, lua_Unsigned n,
                     पूर्णांक islittle, अचिन्हित size, पूर्णांक neg) {
  अक्षर *buff = luaL_prepbuffsize(b, size);
  अचिन्हित i;
  buff[islittle ? 0 : size - 1] = (अक्षर)(n & MC);  /* first byte */
  साठी (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (अक्षर)(n & MC);
  }
  जर (neg && size > SZINT) {  /* negative number need sign extension? */
    साठी (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (अक्षर)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
स्थिर शून्य copywithendian (अक्षर *dest, स्थिरांक अक्षर *src,
                            अचिन्हित size, पूर्णांक islittle) {
  जर (islittle == nativeendian.little)
    स_नकल(dest, src, size);
  अन्यथा {
    dest += size - 1;
    पर्यंत (size-- != 0)
      *(dest--) = *(src++);
  }
}


स्थिर पूर्णांक str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  स्थिरांक अक्षर *fmt = luaL_checkstring(L, 1);  /* format string */
  पूर्णांक arg = 1;  /* current argument to pack */
  माप_प्रकार totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  पर्यंत (*fmt != '\0') {
    अचिन्हित ntoalign;
    माप_प्रकार size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    पर्यंत (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    निवड (opt) {
      स्थिती Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        जर (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        थांबा;
      }
      स्थिती Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        जर (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        थांबा;
      }
      स्थिती Kfloat: {  /* C float */
        दशांश f = (दशांश)luaL_checknumber(L, arg);  /* get argument */
        अक्षर *buff = luaL_prepbuffsize(&b, आकार(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (अक्षर *)&f, आकार(f), h.islittle);
        luaL_addsize(&b, size);
        थांबा;
      }
      स्थिती Knumber: {  /* Lua float */
        lua_संख्या f = luaL_checknumber(L, arg);  /* get argument */
        अक्षर *buff = luaL_prepbuffsize(&b, आकार(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (अक्षर *)&f, आकार(f), h.islittle);
        luaL_addsize(&b, size);
        थांबा;
      }
      स्थिती Kdouble: {  /* C double */
        द्विगुण f = (द्विगुण)luaL_checknumber(L, arg);  /* get argument */
        अक्षर *buff = luaL_prepbuffsize(&b, आकार(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (अक्षर *)&f, आकार(f), h.islittle);
        luaL_addsize(&b, size);
        थांबा;
      }
      स्थिती Kchar: {  /* fixed-size string */
        माप_प्रकार len;
        स्थिरांक अक्षर *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        जर (len < size) {  /* does it need padding? */
          माप_प्रकार psize = size - len;  /* pad size */
          अक्षर *buff = luaL_prepbuffsize(&b, psize);
          स_रखो(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        थांबा;
      }
      स्थिती Kstring: {  /* strings with length count */
        माप_प्रकार len;
        स्थिरांक अक्षर *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= आकार(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        थांबा;
      }
      स्थिती Kzstr: {  /* zero-terminated string */
        माप_प्रकार len;
        स्थिरांक अक्षर *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, म_माप(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        थांबा;
      }
      स्थिती Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      स्थिती Kpaddalign: स्थिती Knop:
        arg--;  /* undo increment */
        थांबा;
    }
  }
  luaL_pushresult(&b);
  परत 1;
}


स्थिर पूर्णांक str_packsize (lua_State *L) {
  Header h;
  स्थिरांक अक्षर *fmt = luaL_checkstring(L, 1);  /* format string */
  माप_प्रकार totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  पर्यंत (*fmt != '\0') {
    अचिन्हित ntoalign;
    माप_प्रकार size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    size += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_SIZE - size,
                     1, "format result too large");
    totalsize += size;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  परत 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
स्थिर lua_Integer unpackint (lua_State *L, स्थिरांक अक्षर *str,
                              पूर्णांक islittle, पूर्णांक size, पूर्णांक issigned) {
  lua_Unsigned res = 0;
  पूर्णांक i;
  पूर्णांक limit = (size  <= SZINT) ? size : SZINT;
  साठी (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(अचिन्हित अक्षर)str[islittle ? i : size - 1 - i];
  }
  जर (size < SZINT) {  /* real size smaller than lua_Integer? */
    जर (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  अन्यथा जर (size > SZINT) {  /* must check unread bytes */
    पूर्णांक mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    साठी (i = limit; i < size; i++) {
      जर (l_unlikely((अचिन्हित अक्षर)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  परत (lua_Integer)res;
}


स्थिर पूर्णांक str_unpack (lua_State *L) {
  Header h;
  स्थिरांक अक्षर *fmt = luaL_checkstring(L, 1);
  माप_प्रकार ld;
  स्थिरांक अक्षर *data = luaL_checklstring(L, 2, &ld);
  माप_प्रकार pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  पूर्णांक n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  पर्यंत (*fmt != '\0') {
    अचिन्हित ntoalign;
    माप_प्रकार size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    निवड (opt) {
      स्थिती Kint:
      स्थिती Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        थांबा;
      }
      स्थिती Kfloat: {
        दशांश f;
        copywithendian((अक्षर *)&f, data + pos, आकार(f), h.islittle);
        lua_pushnumber(L, (lua_संख्या)f);
        थांबा;
      }
      स्थिती Knumber: {
        lua_संख्या f;
        copywithendian((अक्षर *)&f, data + pos, आकार(f), h.islittle);
        lua_pushnumber(L, f);
        थांबा;
      }
      स्थिती Kdouble: {
        द्विगुण f;
        copywithendian((अक्षर *)&f, data + pos, आकार(f), h.islittle);
        lua_pushnumber(L, (lua_संख्या)f);
        थांबा;
      }
      स्थिती Kchar: {
        lua_pushlstring(L, data + pos, size);
        थांबा;
      }
      स्थिती Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        थांबा;
      }
      स्थिती Kzstr: {
        माप_प्रकार len = म_माप(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        थांबा;
      }
      स्थिती Kpaddalign: स्थिती Kpadding: स्थिती Knop:
        n--;  /* undo increment */
        थांबा;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  परत n + 1;
}

/* }====================================================== */


स्थिर स्थिरांक luaL_Reg strlib[] = {
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
  lua_काढा(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_काढा(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API पूर्णांक luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  परत 1;
}

