/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#घोषणा lstrlib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <ctype.h>
#समावेश <float.h>
#समावेश <limits.h>
#समावेश <locale.h>
#समावेश <math.h>
#समावेश <stddef.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#अगर !defined(LUA_MAXCAPTURES)
#घोषणा LUA_MAXCAPTURES		32
#पूर्ण_अगर


अटल पूर्णांक str_len (lua_State *L) {
  माप_प्रकार l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  वापस 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
अटल माप_प्रकार posrelatI (lua_Integer pos, माप_प्रकार len) {
  अगर (pos > 0)
    वापस (माप_प्रकार)pos;
  अन्यथा अगर (pos == 0)
    वापस 1;
  अन्यथा अगर (pos < -(lua_Integer)len)  /* inverted comparison */
    वापस 1;  /* clip to 1 */
  अन्यथा वापस len + (माप_प्रकार)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
अटल माप_प्रकार getendpos (lua_State *L, पूर्णांक arg, lua_Integer def,
                         माप_प्रकार len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  अगर (pos > (lua_Integer)len)
    वापस len;
  अन्यथा अगर (pos >= 0)
    वापस (माप_प्रकार)pos;
  अन्यथा अगर (pos < -(lua_Integer)len)
    वापस 0;
  अन्यथा वापस len + (माप_प्रकार)pos + 1;
}


अटल पूर्णांक str_sub (lua_State *L) {
  माप_प्रकार l;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &l);
  माप_प्रकार start = posrelatI(luaL_checkinteger(L, 2), l);
  माप_प्रकार end = getendpos(L, 3, -1, l);
  अगर (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  अन्यथा lua_pushliteral(L, "");
  वापस 1;
}


अटल पूर्णांक str_reverse (lua_State *L) {
  माप_प्रकार l, i;
  lua_L_बफर b;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &l);
  अक्षर *p = luaL_buffinitsize(L, &b, l);
  क्रम (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  वापस 1;
}


अटल पूर्णांक str_lower (lua_State *L) {
  माप_प्रकार l;
  माप_प्रकार i;
  lua_L_बफर b;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &l);
  अक्षर *p = luaL_buffinitsize(L, &b, l);
  क्रम (i=0; i<l; i++)
    p[i] = cast_char(छोटे(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  वापस 1;
}


अटल पूर्णांक str_upper (lua_State *L) {
  माप_प्रकार l;
  माप_प्रकार i;
  lua_L_बफर b;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &l);
  अक्षर *p = luaL_buffinitsize(L, &b, l);
  क्रम (i=0; i<l; i++)
    p[i] = cast_char(बड़े(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  वापस 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
अटल पूर्णांक str_rep (lua_State *L) {
  माप_प्रकार len, lsep;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  स्थिर अक्षर *sep = luaL_optlstring(L, 3, "", &lsep);
  अगर (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  अन्यथा अगर (l_unlikely(len > MAX_आकार - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_आकार) / n))
    वापस luaL_error(L, "resulting string too large");
  अन्यथा {
    माप_प्रकार totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    lua_L_बफर b;
    अक्षर *p = luaL_buffinitsize(L, &b, totallen);
    जबतक (n-- > 1) {  /* first n-1 copies (followed by separator) */
      स_नकल(p, s, len * माप(अक्षर)); p += len;
      अगर (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        स_नकल(p, sep, lsep * माप(अक्षर)); p += lsep;
      }
    }
    स_नकल(p, s, len * माप(अक्षर));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  वापस 1;
}


अटल पूर्णांक str_byte (lua_State *L) {
  माप_प्रकार l;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  माप_प्रकार posi = posrelatI(pi, l);
  माप_प्रकार pose = getendpos(L, 3, pi, l);
  पूर्णांक n, i;
  अगर (posi > pose) वापस 0;  /* empty interval; return no values */
  अगर (l_unlikely(pose - posi >= (माप_प्रकार)पूर्णांक_उच्च))  /* arithmetic overflow? */
    वापस luaL_error(L, "string slice too long");
  n = (पूर्णांक)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  क्रम (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  वापस n;
}


अटल पूर्णांक str_char (lua_State *L) {
  पूर्णांक n = lua_gettop(L);  /* number of arguments */
  पूर्णांक i;
  lua_L_बफर b;
  अक्षर *p = luaL_buffinitsize(L, &b, cast_uint(n));
  क्रम (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)अच_अक्षर_उच्च, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  वापस 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
काष्ठा str_Writer {
  पूर्णांक init;  /* true iff buffer has been initialized */
  lua_L_बफर B;
};


अटल पूर्णांक writer (lua_State *L, स्थिर व्योम *b, माप_प्रकार आकार, व्योम *ud) {
  काष्ठा str_Writer *state = (काष्ठा str_Writer *)ud;
  अगर (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  अगर (b == शून्य) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  अन्यथा
    luaL_addlstring(&state->B, (स्थिर अक्षर *)b, आकार);
  वापस 0;
}


अटल पूर्णांक str_dump (lua_State *L) {
  काष्ठा str_Writer state;
  पूर्णांक strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  वापस 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#अगर defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

अटल स्थिर luaL_Reg stringmetamethods[] = {
  {"__index", शून्य},  /* placeholder */
  {शून्य, शून्य}
};

#अन्यथा		/* }{ */

अटल पूर्णांक tonum (lua_State *L, पूर्णांक arg) {
  अगर (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    वापस 1;
  }
  अन्यथा {  /* check whether it is a numerical string */
    माप_प्रकार len;
    स्थिर अक्षर *s = lua_tolstring(L, arg, &len);
    वापस (s != शून्य && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
अटल व्योम trymt (lua_State *L, स्थिर अक्षर *mtkey, स्थिर अक्षर *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  अगर (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_जोड़ो(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


अटल पूर्णांक arith (lua_State *L, पूर्णांक op, स्थिर अक्षर *mtname) {
  अगर (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  अन्यथा
    trymt(L, mtname, mtname + 2);
  वापस 1;
}


अटल पूर्णांक arith_add (lua_State *L) {
  वापस arith(L, LUA_OPADD, "__add");
}

अटल पूर्णांक arith_sub (lua_State *L) {
  वापस arith(L, LUA_OPSUB, "__sub");
}

अटल पूर्णांक arith_mul (lua_State *L) {
  वापस arith(L, LUA_OPMUL, "__mul");
}

अटल पूर्णांक arith_mod (lua_State *L) {
  वापस arith(L, LUA_OPMOD, "__mod");
}

अटल पूर्णांक arith_pow (lua_State *L) {
  वापस arith(L, LUA_OPPOW, "__pow");
}

अटल पूर्णांक arith_div (lua_State *L) {
  वापस arith(L, LUA_OPDIV, "__div");
}

अटल पूर्णांक arith_idiv (lua_State *L) {
  वापस arith(L, LUA_OPIDIV, "__idiv");
}

अटल पूर्णांक arith_unm (lua_State *L) {
  वापस arith(L, LUA_OPUNM, "__unm");
}


अटल स्थिर luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", शून्य},  /* placeholder */
  {शून्य, शून्य}
};

#पूर्ण_अगर		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#घोषणा CAP_UNFINISHED	(-1)
#घोषणा CAP_POSITION	(-2)


प्रकार काष्ठा MatchState {
  स्थिर अक्षर *src_init;  /* init of source string */
  स्थिर अक्षर *src_end;  /* end ('\0') of source string */
  स्थिर अक्षर *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  पूर्णांक matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  पूर्णांक level;  /* total number of captures (finished or unfinished) */
  काष्ठा {
    स्थिर अक्षर *init;
    सूचक_भेद_प्रकार len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
अटल स्थिर अक्षर *match (MatchState *ms, स्थिर अक्षर *s, स्थिर अक्षर *p);


/* maximum recursion depth for 'match' */
#अगर !defined(MAXCCALLS)
#घोषणा MAXCCALLS	200
#पूर्ण_अगर


#घोषणा L_ESC		'%'
#घोषणा SPECIALS	"^$*+?.([%-"


अटल पूर्णांक check_capture (MatchState *ms, पूर्णांक l) {
  l -= '1';
  अगर (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    वापस luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  वापस l;
}


अटल पूर्णांक capture_to_close (MatchState *ms) {
  पूर्णांक level = ms->level;
  क्रम (level--; level>=0; level--)
    अगर (ms->capture[level].len == CAP_UNFINISHED) वापस level;
  वापस luaL_error(ms->L, "invalid pattern capture");
}


अटल स्थिर अक्षर *classend (MatchState *ms, स्थिर अक्षर *p) {
  चयन (*p++) {
    हाल L_ESC: {
      अगर (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      वापस p+1;
    }
    हाल '[': {
      अगर (*p == '^') p++;
      करो {  /* look for a ']' */
        अगर (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        अगर (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } जबतक (*p != ']');
      वापस p+1;
    }
    शेष: {
      वापस p;
    }
  }
}


अटल पूर्णांक match_class (पूर्णांक c, पूर्णांक cl) {
  पूर्णांक res;
  चयन (छोटे(cl)) {
    हाल 'a' : res = है_अक्षर(c); अवरोध;
    हाल 'c' : res = है_नियंत्रण(c); अवरोध;
    हाल 'd' : res = है_अंक(c); अवरोध;
    हाल 'g' : res = है_चित्र(c); अवरोध;
    हाल 'l' : res = है_छोटा(c); अवरोध;
    हाल 'p' : res = है_विराम(c); अवरोध;
    हाल 's' : res = है_खाली(c); अवरोध;
    हाल 'u' : res = है_बड़ा(c); अवरोध;
    हाल 'w' : res = है_अक्षर_अंक(c); अवरोध;
    हाल 'x' : res = है_षष्ठादशक(c); अवरोध;
    हाल 'z' : res = (c == 0); अवरोध;  /* deprecated option */
    शेष: वापस (cl == c);
  }
  वापस (है_छोटा(cl) ? res : !res);
}


अटल पूर्णांक matchbracketclass (पूर्णांक c, स्थिर अक्षर *p, स्थिर अक्षर *ec) {
  पूर्णांक sig = 1;
  अगर (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  जबतक (++p < ec) {
    अगर (*p == L_ESC) {
      p++;
      अगर (match_class(c, cast_uchar(*p)))
        वापस sig;
    }
    अन्यथा अगर ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      अगर (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        वापस sig;
    }
    अन्यथा अगर (cast_uchar(*p) == c) वापस sig;
  }
  वापस !sig;
}


अटल पूर्णांक singlematch (MatchState *ms, स्थिर अक्षर *s, स्थिर अक्षर *p,
                        स्थिर अक्षर *ep) {
  अगर (s >= ms->src_end)
    वापस 0;
  अन्यथा {
    पूर्णांक c = cast_uchar(*s);
    चयन (*p) {
      हाल '.': वापस 1;  /* matches any char */
      हाल L_ESC: वापस match_class(c, cast_uchar(*(p+1)));
      हाल '[': वापस matchbracketclass(c, p, ep-1);
      शेष:  वापस (cast_uchar(*p) == c);
    }
  }
}


अटल स्थिर अक्षर *matchbalance (MatchState *ms, स्थिर अक्षर *s,
                                   स्थिर अक्षर *p) {
  अगर (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  अगर (*s != *p) वापस शून्य;
  अन्यथा {
    पूर्णांक b = *p;
    पूर्णांक e = *(p+1);
    पूर्णांक cont = 1;
    जबतक (++s < ms->src_end) {
      अगर (*s == e) {
        अगर (--cont == 0) वापस s+1;
      }
      अन्यथा अगर (*s == b) cont++;
    }
  }
  वापस शून्य;  /* string ends out of balance */
}


अटल स्थिर अक्षर *max_expand (MatchState *ms, स्थिर अक्षर *s,
                                 स्थिर अक्षर *p, स्थिर अक्षर *ep) {
  सूचक_भेद_प्रकार i = 0;  /* counts maximum expand for item */
  जबतक (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  जबतक (i>=0) {
    स्थिर अक्षर *res = match(ms, (s+i), ep+1);
    अगर (res) वापस res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  वापस शून्य;
}


अटल स्थिर अक्षर *min_expand (MatchState *ms, स्थिर अक्षर *s,
                                 स्थिर अक्षर *p, स्थिर अक्षर *ep) {
  क्रम (;;) {
    स्थिर अक्षर *res = match(ms, s, ep+1);
    अगर (res != शून्य)
      वापस res;
    अन्यथा अगर (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    अन्यथा वापस शून्य;
  }
}


अटल स्थिर अक्षर *start_capture (MatchState *ms, स्थिर अक्षर *s,
                                    स्थिर अक्षर *p, पूर्णांक what) {
  स्थिर अक्षर *res;
  पूर्णांक level = ms->level;
  अगर (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  अगर ((res=match(ms, s, p)) == शून्य)  /* match failed? */
    ms->level--;  /* undo capture */
  वापस res;
}


अटल स्थिर अक्षर *end_capture (MatchState *ms, स्थिर अक्षर *s,
                                  स्थिर अक्षर *p) {
  पूर्णांक l = capture_to_close(ms);
  स्थिर अक्षर *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  अगर ((res = match(ms, s, p)) == शून्य)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  वापस res;
}


अटल स्थिर अक्षर *match_capture (MatchState *ms, स्थिर अक्षर *s, पूर्णांक l) {
  माप_प्रकार len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  अगर ((माप_प्रकार)(ms->src_end-s) >= len &&
      स_भेद(ms->capture[l].init, s, len) == 0)
    वापस s+len;
  अन्यथा वापस शून्य;
}


अटल स्थिर अक्षर *match (MatchState *ms, स्थिर अक्षर *s, स्थिर अक्षर *p) {
  अगर (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  अगर (p != ms->p_end) {  /* end of pattern? */
    चयन (*p) {
      हाल '(': {  /* start capture */
        अगर (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        अन्यथा
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        अवरोध;
      }
      हाल ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        अवरोध;
      }
      हाल '$': {
        अगर ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          जाओ dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : शून्य;  /* check end of string */
        अवरोध;
      }
      हाल L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        चयन (*(p + 1)) {
          हाल 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            अगर (s != शून्य) {
              p += 4; जाओ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            अवरोध;
          }
          हाल 'f': {  /* frontier? */
            स्थिर अक्षर *ep; अक्षर previous;
            p += 2;
            अगर (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            अगर (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; जाओ init;  /* return match(ms, s, ep); */
            }
            s = शून्य;  /* match failed */
            अवरोध;
          }
          हाल '0': हाल '1': हाल '2': हाल '3':
          हाल '4': हाल '5': हाल '6': हाल '7':
          हाल '8': हाल '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            अगर (s != शून्य) {
              p += 2; जाओ init;  /* return match(ms, s, p + 2) */
            }
            अवरोध;
          }
          शेष: जाओ dflt;
        }
        अवरोध;
      }
      शेष: dflt: {  /* pattern class plus optional suffix */
        स्थिर अक्षर *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        अगर (!singlematch(ms, s, p, ep)) {
          अगर (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; जाओ init;  /* return match(ms, s, ep + 1); */
          }
          अन्यथा  /* '+' or no suffix */
            s = शून्य;  /* fail */
        }
        अन्यथा {  /* matched once */
          चयन (*ep) {  /* handle optional suffix */
            हाल '?': {  /* optional */
              स्थिर अक्षर *res;
              अगर ((res = match(ms, s + 1, ep + 1)) != शून्य)
                s = res;
              अन्यथा {
                p = ep + 1; जाओ init;  /* else return match(ms, s, ep + 1); */
              }
              अवरोध;
            }
            हाल '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            हाल '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              अवरोध;
            हाल '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              अवरोध;
            शेष:  /* no suffix */
              s++; p = ep; जाओ init;  /* return match(ms, s + 1, ep); */
          }
        }
        अवरोध;
      }
    }
  }
  ms->matchdepth++;
  वापस s;
}



अटल स्थिर अक्षर *lmemfind (स्थिर अक्षर *s1, माप_प्रकार l1,
                               स्थिर अक्षर *s2, माप_प्रकार l2) {
  अगर (l2 == 0) वापस s1;  /* empty strings are everywhere */
  अन्यथा अगर (l2 > l1) वापस शून्य;  /* avoids a negative 'l1' */
  अन्यथा {
    स्थिर अक्षर *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    जबतक (l1 > 0 && (init = (स्थिर अक्षर *)स_प्रथम(s1, *s2, l1)) != शून्य) {
      init++;   /* 1st char is already checked */
      अगर (स_भेद(init, s2+1, l2) == 0)
        वापस init-1;
      अन्यथा {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    वापस शून्य;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
अटल सूचक_भेद_प्रकार get_onecapture (MatchState *ms, पूर्णांक i, स्थिर अक्षर *s,
                              स्थिर अक्षर *e, स्थिर अक्षर **cap) {
  अगर (i >= ms->level) {
    अगर (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    वापस (e - s);
  }
  अन्यथा {
    सूचक_भेद_प्रकार capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    अगर (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    अन्यथा अगर (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    वापस capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
अटल व्योम धकेलो_onecapture (MatchState *ms, पूर्णांक i, स्थिर अक्षर *s,
                                                    स्थिर अक्षर *e) {
  स्थिर अक्षर *cap;
  सूचक_भेद_प्रकार l = get_onecapture(ms, i, s, e, &cap);
  अगर (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


अटल पूर्णांक धकेलो_captures (MatchState *ms, स्थिर अक्षर *s, स्थिर अक्षर *e) {
  पूर्णांक i;
  पूर्णांक nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  क्रम (i = 0; i < nlevels; i++)
    धकेलो_onecapture(ms, i, s, e);
  वापस nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
अटल पूर्णांक nospecials (स्थिर अक्षर *p, माप_प्रकार l) {
  माप_प्रकार upto = 0;
  करो {
    अगर (strpbrk(p + upto, SPECIALS))
      वापस 0;  /* pattern has a special character */
    upto += म_माप(p + upto) + 1;  /* may have more after \0 */
  } जबतक (upto <= l);
  वापस 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
अटल व्योम prepstate (MatchState *ms, lua_State *L,
                       स्थिर अक्षर *s, माप_प्रकार ls, स्थिर अक्षर *p, माप_प्रकार lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
अटल व्योम reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


अटल पूर्णांक str_find_aux (lua_State *L, पूर्णांक find) {
  माप_प्रकार ls, lp;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &ls);
  स्थिर अक्षर *p = luaL_checklstring(L, 2, &lp);
  माप_प्रकार init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  अगर (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    वापस 1;
  }
  /* explicit request or no special characters? */
  अगर (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    स्थिर अक्षर *s2 = lmemfind(s + init, ls - init, p, lp);
    अगर (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      वापस 2;
    }
  }
  अन्यथा {
    MatchState ms;
    स्थिर अक्षर *s1 = s + init;
    पूर्णांक anchor = (*p == '^');
    अगर (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    करो {
      स्थिर अक्षर *res;
      reprepstate(&ms);
      अगर ((res=match(&ms, s1, p)) != शून्य) {
        अगर (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          वापस धकेलो_captures(&ms, शून्य, 0) + 2;
        }
        अन्यथा
          वापस धकेलो_captures(&ms, s1, res);
      }
    } जबतक (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  वापस 1;
}


अटल पूर्णांक str_find (lua_State *L) {
  वापस str_find_aux(L, 1);
}


अटल पूर्णांक str_match (lua_State *L) {
  वापस str_find_aux(L, 0);
}


/* state for 'gmatch' */
प्रकार काष्ठा GMatchState {
  स्थिर अक्षर *src;  /* current position */
  स्थिर अक्षर *p;  /* pattern */
  स्थिर अक्षर *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


अटल पूर्णांक gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  स्थिर अक्षर *src;
  gm->ms.L = L;
  क्रम (src = gm->src; src <= gm->ms.src_end; src++) {
    स्थिर अक्षर *e;
    reprepstate(&gm->ms);
    अगर ((e = match(&gm->ms, src, gm->p)) != शून्य && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      वापस धकेलो_captures(&gm->ms, src, e);
    }
  }
  वापस 0;  /* not found */
}


अटल पूर्णांक gmatch (lua_State *L) {
  माप_प्रकार ls, lp;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &ls);
  स्थिर अक्षर *p = luaL_checklstring(L, 2, &lp);
  माप_प्रकार init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, माप(GMatchState), 0);
  अगर (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = शून्य;
  lua_pushcclosure(L, gmatch_aux, 3);
  वापस 1;
}


अटल व्योम add_s (MatchState *ms, lua_L_बफर *b, स्थिर अक्षर *s,
                                                   स्थिर अक्षर *e) {
  माप_प्रकार l;
  lua_State *L = ms->L;
  स्थिर अक्षर *news = lua_tolstring(L, 3, &l);
  स्थिर अक्षर *p;
  जबतक ((p = (अक्षर *)स_प्रथम(news, L_ESC, l)) != शून्य) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    अगर (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    अन्यथा अगर (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    अन्यथा अगर (है_अंक(cast_uchar(*p))) {  /* '%n' */
      स्थिर अक्षर *cap;
      सूचक_भेद_प्रकार resl = get_onecapture(ms, *p - '1', s, e, &cap);
      अगर (resl == CAP_POSITION)
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
अटल पूर्णांक add_मूल्य (MatchState *ms, lua_L_बफर *b, स्थिर अक्षर *s,
                                      स्थिर अक्षर *e, पूर्णांक tr) {
  lua_State *L = ms->L;
  चयन (tr) {
    हाल LUA_TFUNCTION: {  /* call the function */
      पूर्णांक n;
      lua_pushvalue(L, 3);  /* push the function */
      n = धकेलो_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      अवरोध;
    }
    हाल LUA_TTABLE: {  /* index the table */
      धकेलो_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      अवरोध;
    }
    शेष: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      वापस 1;  /* something changed */
    }
  }
  अगर (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_निकालो(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    वापस 0;  /* no changes */
  }
  अन्यथा अगर (l_unlikely(!lua_isstring(L, -1)))
    वापस luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  अन्यथा {
    luaL_addvalue(b);  /* add result to accumulator */
    वापस 1;  /* something changed */
  }
}


अटल पूर्णांक str_gsub (lua_State *L) {
  माप_प्रकार srcl, lp;
  स्थिर अक्षर *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  स्थिर अक्षर *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  स्थिर अक्षर *lastmatch = शून्य;  /* end of last match */
  पूर्णांक tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  पूर्णांक anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  पूर्णांक changed = 0;  /* change flag */
  MatchState ms;
  lua_L_बफर b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  अगर (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  जबतक (n < max_s) {
    स्थिर अक्षर *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    अगर ((e = match(&ms, src, p)) != शून्य && e != lastmatch) {  /* match? */
      n++;
      changed = add_मूल्य(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    अन्यथा अगर (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    अन्यथा अवरोध;  /* end of subject */
    अगर (anchor) अवरोध;
  }
  अगर (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  अन्यथा {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  वापस 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#अगर !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#घोषणा SIZELENMOD	(माप(LUA_संख्या_FRMLEN)/माप(अक्षर))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#घोषणा L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
अटल lua_संख्या adddigit (अक्षर *buff, अचिन्हित n, lua_संख्या x) {
  lua_संख्या dd = l_mathop(न्यूनमान)(x);  /* get integer part from 'x' */
  पूर्णांक d = (पूर्णांक)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  वापस x - dd;  /* return what is left */
}


अटल पूर्णांक num2straux (अक्षर *buff, अचिन्हित sz, lua_संख्या x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  अगर (x != x || x == (lua_संख्या)विशाल_मान || x == -(lua_संख्या)विशाल_मान)
    वापस l_sprintf(buff, sz, LUA_संख्या_FMT, (LUAI_UACNUMBER)x);
  अन्यथा अगर (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    वापस l_sprintf(buff, sz, LUA_संख्या_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  अन्यथा {
    पूर्णांक e;
    lua_संख्या m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    अचिन्हित n = 0;  /* character count */
    अगर (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    अगर (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      करो {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } जबतक (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    वापस cast_int(n);
  }
}


अटल पूर्णांक lua_number2strx (lua_State *L, अक्षर *buff, अचिन्हित sz,
                            स्थिर अक्षर *fmt, lua_संख्या x) {
  पूर्णांक n = num2straux(buff, sz, x);
  अगर (fmt[SIZELENMOD] == 'A') {
    पूर्णांक i;
    क्रम (i = 0; i < n; i++)
      buff[i] = cast_char(बड़े(cast_uchar(buff[i])));
  }
  अन्यथा अगर (l_unlikely(fmt[SIZELENMOD] != 'a'))
    वापस luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  वापस n;
}

#पूर्ण_अगर				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#घोषणा MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#घोषणा MAX_ITEM	120


/* valid flags in a format specification */
#अगर !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#घोषणा L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#घोषणा L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#घोषणा L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#घोषणा L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#घोषणा L_FMTFLAGSC	"-"

#पूर्ण_अगर


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#घोषणा MAX_FORMAT	32


अटल व्योम addquoted (lua_L_बफर *b, स्थिर अक्षर *s, माप_प्रकार len) {
  luaL_addchar(b, '"');
  जबतक (len--) {
    अगर (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    अन्यथा अगर (है_नियंत्रण(cast_uchar(*s))) {
      अक्षर buff[10];
      अगर (!है_अंक(cast_uchar(*(s+1))))
        l_sprintf(buff, माप(buff), "\\%d", (पूर्णांक)cast_uchar(*s));
      अन्यथा
        l_sprintf(buff, माप(buff), "\\%03d", (पूर्णांक)cast_uchar(*s));
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
अटल पूर्णांक quotefloat (lua_State *L, अक्षर *buff, lua_संख्या n) {
  स्थिर अक्षर *s;  /* for the fixed representations */
  अगर (n == (lua_संख्या)विशाल_मान)  /* inf? */
    s = "1e9999";
  अन्यथा अगर (n == -(lua_संख्या)विशाल_मान)  /* -inf? */
    s = "-1e9999";
  अन्यथा अगर (n != n)  /* NaN? */
    s = "(0/0)";
  अन्यथा {  /* format number as hexadecimal */
    पूर्णांक  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_संख्या_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    अगर (स_प्रथम(buff, '.', cast_uint(nb)) == शून्य) {  /* no dot? */
      अक्षर point = lua_getlocaledecpoint();  /* try locale point */
      अक्षर *ppoint = (अक्षर *)स_प्रथम(buff, point, cast_uint(nb));
      अगर (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    वापस nb;
  }
  /* for the fixed representations */
  वापस l_sprintf(buff, MAX_ITEM, "%s", s);
}


अटल व्योम addliteral (lua_State *L, lua_L_बफर *b, पूर्णांक arg) {
  चयन (lua_type(L, arg)) {
    हाल LUA_TSTRING: {
      माप_प्रकार len;
      स्थिर अक्षर *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      अवरोध;
    }
    हाल LUA_TNUMBER: {
      अक्षर *buff = luaL_prepbuffsize(b, MAX_ITEM);
      पूर्णांक nb;
      अगर (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      अन्यथा {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        स्थिर अक्षर *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      अवरोध;
    }
    हाल LUA_TNIL: हाल LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, शून्य);
      luaL_addvalue(b);
      अवरोध;
    }
    शेष: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


अटल स्थिर अक्षर *get2digits (स्थिर अक्षर *s) {
  अगर (है_अंक(cast_uchar(*s))) {
    s++;
    अगर (है_अंक(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  वापस s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
अटल व्योम checkformat (lua_State *L, स्थिर अक्षर *form, स्थिर अक्षर *flags,
                                       पूर्णांक precision) {
  स्थिर अक्षर *spec = form + 1;  /* skip '%' */
  spec += म_अखोज(spec, flags);  /* skip flags */
  अगर (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    अगर (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  अगर (!है_अक्षर(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
अटल स्थिर अक्षर *getformat (lua_State *L, स्थिर अक्षर *strfrmt,
                                            अक्षर *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  माप_प्रकार len = म_अखोज(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  अगर (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  स_नकल(form, strfrmt, len * माप(अक्षर));
  *(form + len) = '\0';
  वापस strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
अटल व्योम addlenmod (अक्षर *form, स्थिर अक्षर *lenmod) {
  माप_प्रकार l = म_माप(form);
  माप_प्रकार lm = म_माप(lenmod);
  अक्षर spec = form[l - 1];
  म_नकल(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


अटल पूर्णांक str_format (lua_State *L) {
  पूर्णांक शीर्ष = lua_gettop(L);
  पूर्णांक arg = 1;
  माप_प्रकार sfl;
  स्थिर अक्षर *strfrmt = luaL_checklstring(L, arg, &sfl);
  स्थिर अक्षर *strfrmt_end = strfrmt+sfl;
  स्थिर अक्षर *flags;
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  जबतक (strfrmt < strfrmt_end) {
    अगर (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    अन्यथा अगर (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    अन्यथा { /* format item */
      अक्षर form[MAX_FORMAT];  /* to store the format ('%...') */
      अचिन्हित maxitem = MAX_ITEM;  /* maximum length for the result */
      अक्षर *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      पूर्णांक nb = 0;  /* number of bytes in result */
      अगर (++arg > शीर्ष)
        वापस luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      चयन (*strfrmt++) {
        हाल 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (पूर्णांक)luaL_checkinteger(L, arg));
          अवरोध;
        }
        हाल 'd': हाल 'i':
          flags = L_FMTFLAGSI;
          जाओ intcase;
        हाल 'u':
          flags = L_FMTFLAGSU;
          जाओ intcase;
        हाल 'o': हाल 'x': हाल 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          अवरोध;
        }
        हाल 'a': हाल 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_संख्या_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          अवरोध;
        हाल 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        हाल 'e': हाल 'E': हाल 'g': हाल 'G': {
          lua_संख्या n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_संख्या_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          अवरोध;
        }
        हाल 'p': {
          स्थिर व्योम *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          अगर (p == शून्य) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /*परिणाम*/
            form[म_माप(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          अवरोध;
        }
        हाल 'q': {
          अगर (form[2] != '\0')  /* modifiers? */
            वापस luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          अवरोध;
        }
        हाल 's': {
          माप_प्रकार l;
          स्थिर अक्षर *s = luaL_tolstring(L, arg, &l);
          अगर (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          अन्यथा {
            luaL_argcheck(L, l == म_माप(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            अगर (म_अक्षर(form, '.') == शून्य && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            अन्यथा {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_निकालो(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          अवरोध;
        }
        शेष: {  /* also treat cases 'pnLlh' */
          वापस luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  वापस 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#अगर !defined(LUAL_PACKPADBYTE)
#घोषणा LUAL_PACKPADBYTE		0x00
#पूर्ण_अगर

/* maximum size for the binary representation of an integer */
#घोषणा MAXINTSIZE	16

/* number of bits in a character */
#घोषणा NB	अक्षर_बिट

/* mask for one character (NB 1's) */
#घोषणा MC	((1 << NB) - 1)

/* size of a lua_Integer */
#घोषणा SZINT	((पूर्णांक)माप(lua_Integer))


/* dummy union to get native endianness */
अटल स्थिर जोड़ {
  पूर्णांक dummy;
  अक्षर little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
प्रकार काष्ठा Header {
  lua_State *L;
  पूर्णांक islittle;
  अचिन्हित maxalign;
} Header;


/*
** options for pack/unpack
*/
प्रकार क्रमागत KOption {
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
अटल पूर्णांक digit (पूर्णांक c) { वापस '0' <= c && c <= '9'; }

अटल माप_प्रकार getnum (स्थिर अक्षर **fmt, माप_प्रकार df) {
  अगर (!digit(**fmt))  /* no number? */
    वापस df;  /* return default value */
  अन्यथा {
    माप_प्रकार a = 0;
    करो {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } जबतक (digit(**fmt) && a <= (MAX_आकार - 9)/10);
    वापस a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
अटल अचिन्हित getnumlimit (Header *h, स्थिर अक्षर **fmt, माप_प्रकार df) {
  माप_प्रकार sz = getnum(fmt, df);
  अगर (l_unlikely((sz - 1u) >= MAXINTSIZE))
    वापस cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  वापस cast_uint(sz);
}


/*
** Initialize Header
*/
अटल व्योम initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
अटल KOption getoption (Header *h, स्थिर अक्षर **fmt, माप_प्रकार *आकार) {
  /* dummy structure to get native alignment requirements */
  काष्ठा cD { अक्षर c; जोड़ { LUAI_MAXALIGN; } u; };
  पूर्णांक opt = *((*fmt)++);
  *आकार = 0;  /* default */
  चयन (opt) {
    हाल 'b': *आकार = माप(अक्षर); वापस Kint;
    हाल 'B': *आकार = माप(अक्षर); वापस Kuint;
    हाल 'h': *आकार = माप(लघु); वापस Kint;
    हाल 'H': *आकार = माप(लघु); वापस Kuint;
    हाल 'l': *आकार = माप(दीर्घ); वापस Kint;
    हाल 'L': *आकार = माप(दीर्घ); वापस Kuint;
    हाल 'j': *आकार = माप(lua_Integer); वापस Kint;
    हाल 'J': *आकार = माप(lua_Integer); वापस Kuint;
    हाल 'T': *आकार = माप(माप_प्रकार); वापस Kuint;
    हाल 'f': *आकार = माप(भग्न); वापस Kfloat;
    हाल 'n': *आकार = माप(lua_संख्या); वापस Knumber;
    हाल 'd': *आकार = माप(द्विगुन); वापस Kdouble;
    हाल 'i': *आकार = getnumlimit(h, fmt, माप(पूर्णांक)); वापस Kint;
    हाल 'I': *आकार = getnumlimit(h, fmt, माप(पूर्णांक)); वापस Kuint;
    हाल 's': *आकार = getnumlimit(h, fmt, माप(माप_प्रकार)); वापस Kstring;
    हाल 'c':
      *आकार = getnum(fmt, cast_sizet(-1));
      अगर (l_unlikely(*आकार == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      वापस Kchar;
    हाल 'z': वापस Kzstr;
    हाल 'x': *आकार = 1; वापस Kpadding;
    हाल 'X': वापस Kpaddalign;
    हाल ' ': अवरोध;
    हाल '<': h->islittle = 1; अवरोध;
    हाल '>': h->islittle = 0; अवरोध;
    हाल '=': h->islittle = nativeendian.little; अवरोध;
    हाल '!': {
      स्थिर माप_प्रकार maxalign = दुरत्व(काष्ठा cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      अवरोध;
    }
    शेष: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  वापस Knop;
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
अटल KOption getdetails (Header *h, माप_प्रकार totalsize, स्थिर अक्षर **fmt,
                           माप_प्रकार *psize, अचिन्हित *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  माप_प्रकार align = *psize;  /* usually, alignment follows size */
  अगर (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    अगर (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  अगर (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  अन्यथा {
    अगर (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    अगर (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    अन्यथा {
      /* 'szmoda' = totalsize % align */
      अचिन्हित szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  वापस opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
अटल व्योम packint (lua_L_बफर *b, lua_Unsigned n,
                     पूर्णांक islittle, अचिन्हित आकार, पूर्णांक neg) {
  अक्षर *buff = luaL_prepbuffsize(b, आकार);
  अचिन्हित i;
  buff[islittle ? 0 : आकार - 1] = (अक्षर)(n & MC);  /* first byte */
  क्रम (i = 1; i < आकार; i++) {
    n >>= NB;
    buff[islittle ? i : आकार - 1 - i] = (अक्षर)(n & MC);
  }
  अगर (neg && आकार > SZINT) {  /* negative number need sign extension? */
    क्रम (i = SZINT; i < आकार; i++)  /* correct extra bytes */
      buff[islittle ? i : आकार - 1 - i] = (अक्षर)MC;
  }
  luaL_addsize(b, आकार);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
अटल व्योम copywithendian (अक्षर *dest, स्थिर अक्षर *src,
                            अचिन्हित आकार, पूर्णांक islittle) {
  अगर (islittle == nativeendian.little)
    स_नकल(dest, src, आकार);
  अन्यथा {
    dest += आकार - 1;
    जबतक (आकार-- != 0)
      *(dest--) = *(src++);
  }
}


अटल पूर्णांक str_pack (lua_State *L) {
  lua_L_बफर b;
  Header h;
  स्थिर अक्षर *fmt = luaL_checkstring(L, 1);  /* format string */
  पूर्णांक arg = 1;  /* current argument to pack */
  माप_प्रकार totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  जबतक (*fmt != '\0') {
    अचिन्हित ntoalign;
    माप_प्रकार आकार;
    KOption opt = getdetails(&h, totalsize, &fmt, &आकार, &ntoalign);
    luaL_argcheck(L, आकार + ntoalign <= MAX_आकार - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + आकार;
    जबतक (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    चयन (opt) {
      हाल Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        अगर (आकार < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((आकार * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(आकार), (n < 0));
        अवरोध;
      }
      हाल Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        अगर (आकार < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (आकार * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(आकार), 0);
        अवरोध;
      }
      हाल Kfloat: {  /* C float */
        भग्न f = (भग्न)luaL_checknumber(L, arg);  /* get argument */
        अक्षर *buff = luaL_prepbuffsize(&b, माप(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (अक्षर *)&f, माप(f), h.islittle);
        luaL_addsize(&b, आकार);
        अवरोध;
      }
      हाल Knumber: {  /* Lua float */
        lua_संख्या f = luaL_checknumber(L, arg);  /* get argument */
        अक्षर *buff = luaL_prepbuffsize(&b, माप(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (अक्षर *)&f, माप(f), h.islittle);
        luaL_addsize(&b, आकार);
        अवरोध;
      }
      हाल Kdouble: {  /* C double */
        द्विगुन f = (द्विगुन)luaL_checknumber(L, arg);  /* get argument */
        अक्षर *buff = luaL_prepbuffsize(&b, माप(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (अक्षर *)&f, माप(f), h.islittle);
        luaL_addsize(&b, आकार);
        अवरोध;
      }
      हाल Kchar: {  /* fixed-size string */
        माप_प्रकार len;
        स्थिर अक्षर *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= आकार, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        अगर (len < आकार) {  /* does it need padding? */
          माप_प्रकार psize = आकार - len;  /* pad size */
          अक्षर *buff = luaL_prepbuffsize(&b, psize);
          स_रखो(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        अवरोध;
      }
      हाल Kstring: {  /* strings with length count */
        माप_प्रकार len;
        स्थिर अक्षर *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, आकार >= माप(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (आकार * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(आकार), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        अवरोध;
      }
      हाल Kzstr: {  /* zero-terminated string */
        माप_प्रकार len;
        स्थिर अक्षर *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, म_माप(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        अवरोध;
      }
      हाल Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      हाल Kpaddalign: हाल Knop:
        arg--;  /* undo increment */
        अवरोध;
    }
  }
  luaL_pushresult(&b);
  वापस 1;
}


अटल पूर्णांक str_packsize (lua_State *L) {
  Header h;
  स्थिर अक्षर *fmt = luaL_checkstring(L, 1);  /* format string */
  माप_प्रकार totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  जबतक (*fmt != '\0') {
    अचिन्हित ntoalign;
    माप_प्रकार आकार;
    KOption opt = getdetails(&h, totalsize, &fmt, &आकार, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    आकार += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_आकार - आकार,
                     1, "format result too large");
    totalsize += आकार;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  वापस 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
अटल lua_Integer unpackint (lua_State *L, स्थिर अक्षर *str,
                              पूर्णांक islittle, पूर्णांक आकार, पूर्णांक issigned) {
  lua_Unsigned res = 0;
  पूर्णांक i;
  पूर्णांक limit = (आकार  <= SZINT) ? आकार : SZINT;
  क्रम (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(अचिन्हित अक्षर)str[islittle ? i : आकार - 1 - i];
  }
  अगर (आकार < SZINT) {  /* real size smaller than lua_Integer? */
    अगर (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (आकार*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  अन्यथा अगर (आकार > SZINT) {  /* must check unread bytes */
    पूर्णांक mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    क्रम (i = limit; i < आकार; i++) {
      अगर (l_unlikely((अचिन्हित अक्षर)str[islittle ? i : आकार - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", आकार);
    }
  }
  वापस (lua_Integer)res;
}


अटल पूर्णांक str_unpack (lua_State *L) {
  Header h;
  स्थिर अक्षर *fmt = luaL_checkstring(L, 1);
  माप_प्रकार ld;
  स्थिर अक्षर *तथ्य = luaL_checklstring(L, 2, &ld);
  माप_प्रकार pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  पूर्णांक n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  जबतक (*fmt != '\0') {
    अचिन्हित ntoalign;
    माप_प्रकार आकार;
    KOption opt = getdetails(&h, pos, &fmt, &आकार, &ntoalign);
    luaL_argcheck(L, ntoalign + आकार <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    चयन (opt) {
      हाल Kint:
      हाल Kuint: {
        lua_Integer res = unpackint(L, तथ्य + pos, h.islittle,
                                       cast_int(आकार), (opt == Kint));
        lua_pushinteger(L, res);
        अवरोध;
      }
      हाल Kfloat: {
        भग्न f;
        copywithendian((अक्षर *)&f, तथ्य + pos, माप(f), h.islittle);
        lua_pushnumber(L, (lua_संख्या)f);
        अवरोध;
      }
      हाल Knumber: {
        lua_संख्या f;
        copywithendian((अक्षर *)&f, तथ्य + pos, माप(f), h.islittle);
        lua_pushnumber(L, f);
        अवरोध;
      }
      हाल Kdouble: {
        द्विगुन f;
        copywithendian((अक्षर *)&f, तथ्य + pos, माप(f), h.islittle);
        lua_pushnumber(L, (lua_संख्या)f);
        अवरोध;
      }
      हाल Kchar: {
        lua_pushlstring(L, तथ्य + pos, आकार);
        अवरोध;
      }
      हाल Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, तथ्य + pos,
                                          h.islittle, cast_int(आकार), 0);
        luaL_argcheck(L, len <= ld - pos - आकार, 2, "data string too short");
        lua_pushlstring(L, तथ्य + pos + आकार, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        अवरोध;
      }
      हाल Kzstr: {
        माप_प्रकार len = म_माप(तथ्य + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, तथ्य + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        अवरोध;
      }
      हाल Kpaddalign: हाल Kpadding: हाल Knop:
        n--;  /* undo increment */
        अवरोध;
    }
    pos += आकार;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  वापस n + 1;
}

/* }====================================================== */


अटल स्थिर luaL_Reg strlib[] = {
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
  {शून्य, शून्य}
};


अटल व्योम createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_निकालो(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_निकालो(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API पूर्णांक luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  वापस 1;
}

