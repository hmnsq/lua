/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#شامل "lprefix.h"


#شامل <ctype.h>
#شامل <float.h>
#شامل <limits.h>
#شامل <locale.h>
#شامل <math.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#اگر !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


ایستا صحیح str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  بازگشت 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
ایستا size_t posrelatI (lua_Integer pos, size_t len) {
  اگر (pos > 0)
    بازگشت (size_t)pos;
  وگرنه اگر (pos == 0)
    بازگشت 1;
  وگرنه اگر (pos < -(lua_Integer)len)  /* inverted comparison */
    بازگشت 1;  /* clip to 1 */
  وگرنه بازگشت len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ایستا size_t getendpos (lua_State *L, صحیح arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  اگر (pos > (lua_Integer)len)
    بازگشت len;
  وگرنه اگر (pos >= 0)
    بازگشت (size_t)pos;
  وگرنه اگر (pos < -(lua_Integer)len)
    بازگشت 0;
  وگرنه بازگشت len + (size_t)pos + 1;
}


ایستا صحیح str_sub (lua_State *L) {
  size_t l;
  ثابت نویسه *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  اگر (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  وگرنه lua_pushliteral(L, "");
  بازگشت 1;
}


ایستا صحیح str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  ثابت نویسه *s = luaL_checklstring(L, 1, &l);
  نویسه *p = luaL_buffinitsize(L, &b, l);
  برای (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  بازگشت 1;
}


ایستا صحیح str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ثابت نویسه *s = luaL_checklstring(L, 1, &l);
  نویسه *p = luaL_buffinitsize(L, &b, l);
  برای (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  بازگشت 1;
}


ایستا صحیح str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ثابت نویسه *s = luaL_checklstring(L, 1, &l);
  نویسه *p = luaL_buffinitsize(L, &b, l);
  برای (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  بازگشت 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
ایستا صحیح str_rep (lua_State *L) {
  size_t len, lsep;
  ثابت نویسه *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ثابت نویسه *sep = luaL_optlstring(L, 3, "", &lsep);
  اگر (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  وگرنه اگر (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    بازگشت luaL_error(L, "resulting string too large");
  وگرنه {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    نویسه *p = luaL_buffinitsize(L, &b, totallen);
    وقتی (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * اندازه(نویسه)); p += len;
      اگر (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * اندازه(نویسه)); p += lsep;
      }
    }
    memcpy(p, s, len * اندازه(نویسه));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  بازگشت 1;
}


ایستا صحیح str_byte (lua_State *L) {
  size_t l;
  ثابت نویسه *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  صحیح n, i;
  اگر (posi > pose) بازگشت 0;  /* empty interval; return no values */
  اگر (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    بازگشت luaL_error(L, "string slice too long");
  n = (صحیح)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  برای (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  بازگشت n;
}


ایستا صحیح str_char (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  صحیح i;
  luaL_Buffer b;
  نویسه *p = luaL_buffinitsize(L, &b, cast_uint(n));
  برای (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  بازگشت 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
ساختار str_Writer {
  صحیح init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


ایستا صحیح writer (lua_State *L, ثابت تهی *b, size_t size, تهی *ud) {
  ساختار str_Writer *state = (ساختار str_Writer *)ud;
  اگر (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  اگر (b == پوچ) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  وگرنه
    luaL_addlstring(&state->B, (ثابت نویسه *)b, size);
  بازگشت 0;
}


ایستا صحیح str_dump (lua_State *L) {
  ساختار str_Writer state;
  صحیح strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  بازگشت 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#اگر defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ایستا ثابت luaL_Reg stringmetamethods[] = {
  {"__index", پوچ},  /* placeholder */
  {پوچ, پوچ}
};

#وگرنه		/* }{ */

ایستا صحیح tonum (lua_State *L, صحیح arg) {
  اگر (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    بازگشت 1;
  }
  وگرنه {  /* check whether it is a numerical string */
    size_t len;
    ثابت نویسه *s = lua_tolstring(L, arg, &len);
    بازگشت (s != پوچ && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
ایستا تهی trymt (lua_State *L, ثابت نویسه *mtkey, ثابت نویسه *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  اگر (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_درج(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ایستا صحیح arith (lua_State *L, صحیح op, ثابت نویسه *mtname) {
  اگر (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  وگرنه
    trymt(L, mtname, mtname + 2);
  بازگشت 1;
}


ایستا صحیح arith_add (lua_State *L) {
  بازگشت arith(L, LUA_OPADD, "__add");
}

ایستا صحیح arith_sub (lua_State *L) {
  بازگشت arith(L, LUA_OPSUB, "__sub");
}

ایستا صحیح arith_mul (lua_State *L) {
  بازگشت arith(L, LUA_OPMUL, "__mul");
}

ایستا صحیح arith_mod (lua_State *L) {
  بازگشت arith(L, LUA_OPMOD, "__mod");
}

ایستا صحیح arith_pow (lua_State *L) {
  بازگشت arith(L, LUA_OPPOW, "__pow");
}

ایستا صحیح arith_div (lua_State *L) {
  بازگشت arith(L, LUA_OPDIV, "__div");
}

ایستا صحیح arith_idiv (lua_State *L) {
  بازگشت arith(L, LUA_OPIDIV, "__idiv");
}

ایستا صحیح arith_unm (lua_State *L) {
  بازگشت arith(L, LUA_OPUNM, "__unm");
}


ایستا ثابت luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", پوچ},  /* placeholder */
  {پوچ, پوچ}
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


تعریفنوع ساختار MatchState {
  ثابت نویسه *src_init;  /* init of source string */
  ثابت نویسه *src_end;  /* end ('\0') of source string */
  ثابت نویسه *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  صحیح matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  صحیح level;  /* total number of captures (finished or unfinished) */
  ساختار {
    ثابت نویسه *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
ایستا ثابت نویسه *match (MatchState *ms, ثابت نویسه *s, ثابت نویسه *p);


/* maximum recursion depth for 'match' */
#اگر !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


ایستا صحیح check_capture (MatchState *ms, صحیح l) {
  l -= '1';
  اگر (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    بازگشت luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  بازگشت l;
}


ایستا صحیح capture_to_close (MatchState *ms) {
  صحیح level = ms->level;
  برای (level--; level>=0; level--)
    اگر (ms->capture[level].len == CAP_UNFINISHED) بازگشت level;
  بازگشت luaL_error(ms->L, "invalid pattern capture");
}


ایستا ثابت نویسه *classend (MatchState *ms, ثابت نویسه *p) {
  انتخاب (*p++) {
    حالت L_ESC: {
      اگر (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      بازگشت p+1;
    }
    حالت '[': {
      اگر (*p == '^') p++;
      انجامبده {  /* look for a ']' */
        اگر (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        اگر (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } وقتی (*p != ']');
      بازگشت p+1;
    }
    پیشفرض: {
      بازگشت p;
    }
  }
}


ایستا صحیح match_class (صحیح c, صحیح cl) {
  صحیح res;
  انتخاب (tolower(cl)) {
    حالت 'a' : res = isalpha(c); بشکن;
    حالت 'c' : res = iscntrl(c); بشکن;
    حالت 'd' : res = isdigit(c); بشکن;
    حالت 'g' : res = isgraph(c); بشکن;
    حالت 'l' : res = islower(c); بشکن;
    حالت 'p' : res = ispunct(c); بشکن;
    حالت 's' : res = isspace(c); بشکن;
    حالت 'u' : res = isupper(c); بشکن;
    حالت 'w' : res = isalnum(c); بشکن;
    حالت 'x' : res = isxdigit(c); بشکن;
    حالت 'z' : res = (c == 0); بشکن;  /* deprecated option */
    پیشفرض: بازگشت (cl == c);
  }
  بازگشت (islower(cl) ? res : !res);
}


ایستا صحیح matchbracketclass (صحیح c, ثابت نویسه *p, ثابت نویسه *ec) {
  صحیح sig = 1;
  اگر (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  وقتی (++p < ec) {
    اگر (*p == L_ESC) {
      p++;
      اگر (match_class(c, cast_uchar(*p)))
        بازگشت sig;
    }
    وگرنه اگر ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      اگر (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        بازگشت sig;
    }
    وگرنه اگر (cast_uchar(*p) == c) بازگشت sig;
  }
  بازگشت !sig;
}


ایستا صحیح singlematch (MatchState *ms, ثابت نویسه *s, ثابت نویسه *p,
                        ثابت نویسه *ep) {
  اگر (s >= ms->src_end)
    بازگشت 0;
  وگرنه {
    صحیح c = cast_uchar(*s);
    انتخاب (*p) {
      حالت '.': بازگشت 1;  /* matches any char */
      حالت L_ESC: بازگشت match_class(c, cast_uchar(*(p+1)));
      حالت '[': بازگشت matchbracketclass(c, p, ep-1);
      پیشفرض:  بازگشت (cast_uchar(*p) == c);
    }
  }
}


ایستا ثابت نویسه *matchbalance (MatchState *ms, ثابت نویسه *s,
                                   ثابت نویسه *p) {
  اگر (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  اگر (*s != *p) بازگشت پوچ;
  وگرنه {
    صحیح b = *p;
    صحیح e = *(p+1);
    صحیح cont = 1;
    وقتی (++s < ms->src_end) {
      اگر (*s == e) {
        اگر (--cont == 0) بازگشت s+1;
      }
      وگرنه اگر (*s == b) cont++;
    }
  }
  بازگشت پوچ;  /* string ends out of balance */
}


ایستا ثابت نویسه *max_expand (MatchState *ms, ثابت نویسه *s,
                                 ثابت نویسه *p, ثابت نویسه *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  وقتی (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  وقتی (i>=0) {
    ثابت نویسه *res = match(ms, (s+i), ep+1);
    اگر (res) بازگشت res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  بازگشت پوچ;
}


ایستا ثابت نویسه *min_expand (MatchState *ms, ثابت نویسه *s,
                                 ثابت نویسه *p, ثابت نویسه *ep) {
  برای (;;) {
    ثابت نویسه *res = match(ms, s, ep+1);
    اگر (res != پوچ)
      بازگشت res;
    وگرنه اگر (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    وگرنه بازگشت پوچ;
  }
}


ایستا ثابت نویسه *start_capture (MatchState *ms, ثابت نویسه *s,
                                    ثابت نویسه *p, صحیح what) {
  ثابت نویسه *res;
  صحیح level = ms->level;
  اگر (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  اگر ((res=match(ms, s, p)) == پوچ)  /* match failed? */
    ms->level--;  /* undo capture */
  بازگشت res;
}


ایستا ثابت نویسه *end_capture (MatchState *ms, ثابت نویسه *s,
                                  ثابت نویسه *p) {
  صحیح l = capture_to_close(ms);
  ثابت نویسه *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  اگر ((res = match(ms, s, p)) == پوچ)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  بازگشت res;
}


ایستا ثابت نویسه *match_capture (MatchState *ms, ثابت نویسه *s, صحیح l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  اگر ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    بازگشت s+len;
  وگرنه بازگشت پوچ;
}


ایستا ثابت نویسه *match (MatchState *ms, ثابت نویسه *s, ثابت نویسه *p) {
  اگر (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  اگر (p != ms->p_end) {  /* end of pattern? */
    انتخاب (*p) {
      حالت '(': {  /* start capture */
        اگر (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        وگرنه
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        بشکن;
      }
      حالت ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        بشکن;
      }
      حالت '$': {
        اگر ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          برو dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : پوچ;  /* check end of string */
        بشکن;
      }
      حالت L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        انتخاب (*(p + 1)) {
          حالت 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            اگر (s != پوچ) {
              p += 4; برو init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            بشکن;
          }
          حالت 'f': {  /* frontier? */
            ثابت نویسه *ep; نویسه previous;
            p += 2;
            اگر (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            اگر (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; برو init;  /* return match(ms, s, ep); */
            }
            s = پوچ;  /* match failed */
            بشکن;
          }
          حالت '0': حالت '1': حالت '2': حالت '3':
          حالت '4': حالت '5': حالت '6': حالت '7':
          حالت '8': حالت '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            اگر (s != پوچ) {
              p += 2; برو init;  /* return match(ms, s, p + 2) */
            }
            بشکن;
          }
          پیشفرض: برو dflt;
        }
        بشکن;
      }
      پیشفرض: dflt: {  /* pattern class plus optional suffix */
        ثابت نویسه *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        اگر (!singlematch(ms, s, p, ep)) {
          اگر (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; برو init;  /* return match(ms, s, ep + 1); */
          }
          وگرنه  /* '+' or no suffix */
            s = پوچ;  /* fail */
        }
        وگرنه {  /* matched once */
          انتخاب (*ep) {  /* handle optional suffix */
            حالت '?': {  /* optional */
              ثابت نویسه *res;
              اگر ((res = match(ms, s + 1, ep + 1)) != پوچ)
                s = res;
              وگرنه {
                p = ep + 1; برو init;  /* else return match(ms, s, ep + 1); */
              }
              بشکن;
            }
            حالت '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            حالت '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              بشکن;
            حالت '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              بشکن;
            پیشفرض:  /* no suffix */
              s++; p = ep; برو init;  /* return match(ms, s + 1, ep); */
          }
        }
        بشکن;
      }
    }
  }
  ms->matchdepth++;
  بازگشت s;
}



ایستا ثابت نویسه *lmemfind (ثابت نویسه *s1, size_t l1,
                               ثابت نویسه *s2, size_t l2) {
  اگر (l2 == 0) بازگشت s1;  /* empty strings are everywhere */
  وگرنه اگر (l2 > l1) بازگشت پوچ;  /* avoids a negative 'l1' */
  وگرنه {
    ثابت نویسه *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    وقتی (l1 > 0 && (init = (ثابت نویسه *)memchr(s1, *s2, l1)) != پوچ) {
      init++;   /* 1st char is already checked */
      اگر (memcmp(init, s2+1, l2) == 0)
        بازگشت init-1;
      وگرنه {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    بازگشت پوچ;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
ایستا ptrdiff_t get_onecapture (MatchState *ms, صحیح i, ثابت نویسه *s,
                              ثابت نویسه *e, ثابت نویسه **cap) {
  اگر (i >= ms->level) {
    اگر (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    بازگشت (e - s);
  }
  وگرنه {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    اگر (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    وگرنه اگر (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    بازگشت capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ایستا تهی افزودن_onecapture (MatchState *ms, صحیح i, ثابت نویسه *s,
                                                    ثابت نویسه *e) {
  ثابت نویسه *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  اگر (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ایستا صحیح افزودن_captures (MatchState *ms, ثابت نویسه *s, ثابت نویسه *e) {
  صحیح i;
  صحیح nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  برای (i = 0; i < nlevels; i++)
    افزودن_onecapture(ms, i, s, e);
  بازگشت nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ایستا صحیح nospecials (ثابت نویسه *p, size_t l) {
  size_t upto = 0;
  انجامبده {
    اگر (strpbrk(p + upto, SPECIALS))
      بازگشت 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } وقتی (upto <= l);
  بازگشت 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
ایستا تهی prepstate (MatchState *ms, lua_State *L,
                       ثابت نویسه *s, size_t ls, ثابت نویسه *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
ایستا تهی reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


ایستا صحیح str_find_aux (lua_State *L, صحیح find) {
  size_t ls, lp;
  ثابت نویسه *s = luaL_checklstring(L, 1, &ls);
  ثابت نویسه *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  اگر (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    بازگشت 1;
  }
  /* explicit request or no special characters? */
  اگر (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ثابت نویسه *s2 = lmemfind(s + init, ls - init, p, lp);
    اگر (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      بازگشت 2;
    }
  }
  وگرنه {
    MatchState ms;
    ثابت نویسه *s1 = s + init;
    صحیح anchor = (*p == '^');
    اگر (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    انجامبده {
      ثابت نویسه *res;
      reprepstate(&ms);
      اگر ((res=match(&ms, s1, p)) != پوچ) {
        اگر (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          بازگشت افزودن_captures(&ms, پوچ, 0) + 2;
        }
        وگرنه
          بازگشت افزودن_captures(&ms, s1, res);
      }
    } وقتی (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  بازگشت 1;
}


ایستا صحیح str_find (lua_State *L) {
  بازگشت str_find_aux(L, 1);
}


ایستا صحیح str_match (lua_State *L) {
  بازگشت str_find_aux(L, 0);
}


/* state for 'gmatch' */
تعریفنوع ساختار GMatchState {
  ثابت نویسه *src;  /* current position */
  ثابت نویسه *p;  /* pattern */
  ثابت نویسه *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


ایستا صحیح gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ثابت نویسه *src;
  gm->ms.L = L;
  برای (src = gm->src; src <= gm->ms.src_end; src++) {
    ثابت نویسه *e;
    reprepstate(&gm->ms);
    اگر ((e = match(&gm->ms, src, gm->p)) != پوچ && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      بازگشت افزودن_captures(&gm->ms, src, e);
    }
  }
  بازگشت 0;  /* not found */
}


ایستا صحیح gmatch (lua_State *L) {
  size_t ls, lp;
  ثابت نویسه *s = luaL_checklstring(L, 1, &ls);
  ثابت نویسه *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, اندازه(GMatchState), 0);
  اگر (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = پوچ;
  lua_pushcclosure(L, gmatch_aux, 3);
  بازگشت 1;
}


ایستا تهی add_s (MatchState *ms, luaL_Buffer *b, ثابت نویسه *s,
                                                   ثابت نویسه *e) {
  size_t l;
  lua_State *L = ms->L;
  ثابت نویسه *news = lua_tolstring(L, 3, &l);
  ثابت نویسه *p;
  وقتی ((p = (نویسه *)memchr(news, L_ESC, l)) != پوچ) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    اگر (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    وگرنه اگر (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    وگرنه اگر (isdigit(cast_uchar(*p))) {  /* '%n' */
      ثابت نویسه *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      اگر (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      وگرنه
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    وگرنه
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
ایستا صحیح add_مقدار (MatchState *ms, luaL_Buffer *b, ثابت نویسه *s,
                                      ثابت نویسه *e, صحیح tr) {
  lua_State *L = ms->L;
  انتخاب (tr) {
    حالت LUA_TFUNCTION: {  /* call the function */
      صحیح n;
      lua_pushvalue(L, 3);  /* push the function */
      n = افزودن_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      بشکن;
    }
    حالت LUA_TTABLE: {  /* index the table */
      افزودن_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      بشکن;
    }
    پیشفرض: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      بازگشت 1;  /* something changed */
    }
  }
  اگر (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_برداشت(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    بازگشت 0;  /* no changes */
  }
  وگرنه اگر (l_unlikely(!lua_isstring(L, -1)))
    بازگشت luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  وگرنه {
    luaL_addvalue(b);  /* add result to accumulator */
    بازگشت 1;  /* something changed */
  }
}


ایستا صحیح str_gsub (lua_State *L) {
  size_t srcl, lp;
  ثابت نویسه *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ثابت نویسه *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  ثابت نویسه *lastmatch = پوچ;  /* end of last match */
  صحیح tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  صحیح anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  صحیح changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  اگر (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  وقتی (n < max_s) {
    ثابت نویسه *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    اگر ((e = match(&ms, src, p)) != پوچ && e != lastmatch) {  /* match? */
      n++;
      changed = add_مقدار(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    وگرنه اگر (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    وگرنه بشکن;  /* end of subject */
    اگر (anchor) بشکن;
  }
  اگر (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  وگرنه {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  بازگشت 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#اگر !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(اندازه(LUA_عدد_FRMLEN)/اندازه(نویسه))


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
ایستا lua_عدد adddigit (نویسه *buff, unsigned n, lua_عدد x) {
  lua_عدد dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  صحیح d = (صحیح)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  بازگشت x - dd;  /* return what is left */
}


ایستا صحیح num2straux (نویسه *buff, unsigned sz, lua_عدد x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  اگر (x != x || x == (lua_عدد)HUGE_VAL || x == -(lua_عدد)HUGE_VAL)
    بازگشت l_sprintf(buff, sz, LUA_عدد_FMT, (LUAI_UACNUMBER)x);
  وگرنه اگر (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    بازگشت l_sprintf(buff, sz, LUA_عدد_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  وگرنه {
    صحیح e;
    lua_عدد m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    اگر (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    اگر (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      انجامبده {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } وقتی (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    بازگشت cast_int(n);
  }
}


ایستا صحیح lua_number2strx (lua_State *L, نویسه *buff, unsigned sz,
                            ثابت نویسه *fmt, lua_عدد x) {
  صحیح n = num2straux(buff, sz, x);
  اگر (fmt[SIZELENMOD] == 'A') {
    صحیح i;
    برای (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  وگرنه اگر (l_unlikely(fmt[SIZELENMOD] != 'a'))
    بازگشت luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  بازگشت n;
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
#اگر !defined(L_FMTFLAGSF)

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


ایستا تهی addquoted (luaL_Buffer *b, ثابت نویسه *s, size_t len) {
  luaL_addchar(b, '"');
  وقتی (len--) {
    اگر (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    وگرنه اگر (iscntrl(cast_uchar(*s))) {
      نویسه buff[10];
      اگر (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, اندازه(buff), "\\%d", (صحیح)cast_uchar(*s));
      وگرنه
        l_sprintf(buff, اندازه(buff), "\\%03d", (صحیح)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    وگرنه
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
ایستا صحیح quotefloat (lua_State *L, نویسه *buff, lua_عدد n) {
  ثابت نویسه *s;  /* for the fixed representations */
  اگر (n == (lua_عدد)HUGE_VAL)  /* inf? */
    s = "1e9999";
  وگرنه اگر (n == -(lua_عدد)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  وگرنه اگر (n != n)  /* NaN? */
    s = "(0/0)";
  وگرنه {  /* format number as hexadecimal */
    صحیح  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_عدد_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    اگر (memchr(buff, '.', cast_uint(nb)) == پوچ) {  /* no dot? */
      نویسه point = lua_getlocaledecpoint();  /* try locale point */
      نویسه *ppoint = (نویسه *)memchr(buff, point, cast_uint(nb));
      اگر (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    بازگشت nb;
  }
  /* for the fixed representations */
  بازگشت l_sprintf(buff, MAX_ITEM, "%s", s);
}


ایستا تهی addliteral (lua_State *L, luaL_Buffer *b, صحیح arg) {
  انتخاب (lua_type(L, arg)) {
    حالت LUA_TSTRING: {
      size_t len;
      ثابت نویسه *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      بشکن;
    }
    حالت LUA_TNUMBER: {
      نویسه *buff = luaL_prepbuffsize(b, MAX_ITEM);
      صحیح nb;
      اگر (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      وگرنه {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ثابت نویسه *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      بشکن;
    }
    حالت LUA_TNIL: حالت LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, پوچ);
      luaL_addvalue(b);
      بشکن;
    }
    پیشفرض: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


ایستا ثابت نویسه *get2digits (ثابت نویسه *s) {
  اگر (isdigit(cast_uchar(*s))) {
    s++;
    اگر (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  بازگشت s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
ایستا تهی checkformat (lua_State *L, ثابت نویسه *form, ثابت نویسه *flags,
                                       صحیح precision) {
  ثابت نویسه *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  اگر (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    اگر (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  اگر (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
ایستا ثابت نویسه *getformat (lua_State *L, ثابت نویسه *strfrmt,
                                            نویسه *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  اگر (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * اندازه(نویسه));
  *(form + len) = '\0';
  بازگشت strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
ایستا تهی addlenmod (نویسه *form, ثابت نویسه *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  نویسه spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


ایستا صحیح str_format (lua_State *L) {
  صحیح بالاترین = lua_gettop(L);
  صحیح arg = 1;
  size_t sfl;
  ثابت نویسه *strfrmt = luaL_checklstring(L, arg, &sfl);
  ثابت نویسه *strfrmt_end = strfrmt+sfl;
  ثابت نویسه *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  وقتی (strfrmt < strfrmt_end) {
    اگر (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    وگرنه اگر (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    وگرنه { /* format item */
      نویسه form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      نویسه *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      صحیح nb = 0;  /* number of bytes in result */
      اگر (++arg > بالاترین)
        بازگشت luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      انتخاب (*strfrmt++) {
        حالت 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (صحیح)luaL_checkinteger(L, arg));
          بشکن;
        }
        حالت 'd': حالت 'i':
          flags = L_FMTFLAGSI;
          برو intcase;
        حالت 'u':
          flags = L_FMTFLAGSU;
          برو intcase;
        حالت 'o': حالت 'x': حالت 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          بشکن;
        }
        حالت 'a': حالت 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_عدد_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          بشکن;
        حالت 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        حالت 'e': حالت 'E': حالت 'g': حالت 'G': {
          lua_عدد n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_عدد_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          بشکن;
        }
        حالت 'p': {
          ثابت تهی *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          اگر (p == پوچ) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          بشکن;
        }
        حالت 'q': {
          اگر (form[2] != '\0')  /* modifiers? */
            بازگشت luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          بشکن;
        }
        حالت 's': {
          size_t l;
          ثابت نویسه *s = luaL_tolstring(L, arg, &l);
          اگر (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          وگرنه {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            اگر (strchr(form, '.') == پوچ && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            وگرنه {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_برداشت(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          بشکن;
        }
        پیشفرض: {  /* also treat cases 'pnLlh' */
          بازگشت luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  بازگشت 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#اگر !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((صحیح)اندازه(lua_Integer))


/* dummy union to get native endianness */
ایستا ثابت اجتماع {
  صحیح dummy;
  نویسه little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
تعریفنوع ساختار Header {
  lua_State *L;
  صحیح islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
تعریفنوع شمارش KOption {
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
ایستا صحیح digit (صحیح c) { بازگشت '0' <= c && c <= '9'; }

ایستا size_t getnum (ثابت نویسه **fmt, size_t df) {
  اگر (!digit(**fmt))  /* no number? */
    بازگشت df;  /* return default value */
  وگرنه {
    size_t a = 0;
    انجامبده {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } وقتی (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    بازگشت a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ایستا unsigned getnumlimit (Header *h, ثابت نویسه **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  اگر (l_unlikely((sz - 1u) >= MAXINTSIZE))
    بازگشت cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  بازگشت cast_uint(sz);
}


/*
** Initialize Header
*/
ایستا تهی initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
ایستا KOption getoption (Header *h, ثابت نویسه **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  ساختار cD { نویسه c; اجتماع { LUAI_MAXALIGN; } u; };
  صحیح opt = *((*fmt)++);
  *size = 0;  /* default */
  انتخاب (opt) {
    حالت 'b': *size = اندازه(نویسه); بازگشت Kint;
    حالت 'B': *size = اندازه(نویسه); بازگشت Kuint;
    حالت 'h': *size = اندازه(short); بازگشت Kint;
    حالت 'H': *size = اندازه(short); بازگشت Kuint;
    حالت 'l': *size = اندازه(long); بازگشت Kint;
    حالت 'L': *size = اندازه(long); بازگشت Kuint;
    حالت 'j': *size = اندازه(lua_Integer); بازگشت Kint;
    حالت 'J': *size = اندازه(lua_Integer); بازگشت Kuint;
    حالت 'T': *size = اندازه(size_t); بازگشت Kuint;
    حالت 'f': *size = اندازه(اعشاری); بازگشت Kfloat;
    حالت 'n': *size = اندازه(lua_عدد); بازگشت Knumber;
    حالت 'd': *size = اندازه(دوگانه); بازگشت Kdouble;
    حالت 'i': *size = getnumlimit(h, fmt, اندازه(صحیح)); بازگشت Kint;
    حالت 'I': *size = getnumlimit(h, fmt, اندازه(صحیح)); بازگشت Kuint;
    حالت 's': *size = getnumlimit(h, fmt, اندازه(size_t)); بازگشت Kstring;
    حالت 'c':
      *size = getnum(fmt, cast_sizet(-1));
      اگر (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      بازگشت Kchar;
    حالت 'z': بازگشت Kzstr;
    حالت 'x': *size = 1; بازگشت Kpadding;
    حالت 'X': بازگشت Kpaddalign;
    حالت ' ': بشکن;
    حالت '<': h->islittle = 1; بشکن;
    حالت '>': h->islittle = 0; بشکن;
    حالت '=': h->islittle = nativeendian.little; بشکن;
    حالت '!': {
      ثابت size_t maxalign = offsetof(ساختار cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      بشکن;
    }
    پیشفرض: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  بازگشت Knop;
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
ایستا KOption getdetails (Header *h, size_t totalsize, ثابت نویسه **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  اگر (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    اگر (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  اگر (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  وگرنه {
    اگر (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    اگر (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    وگرنه {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  بازگشت opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
ایستا تهی packint (luaL_Buffer *b, lua_Unsigned n,
                     صحیح islittle, unsigned size, صحیح neg) {
  نویسه *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (نویسه)(n & MC);  /* first byte */
  برای (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (نویسه)(n & MC);
  }
  اگر (neg && size > SZINT) {  /* negative number need sign extension? */
    برای (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (نویسه)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
ایستا تهی copywithendian (نویسه *dest, ثابت نویسه *src,
                            unsigned size, صحیح islittle) {
  اگر (islittle == nativeendian.little)
    memcpy(dest, src, size);
  وگرنه {
    dest += size - 1;
    وقتی (size-- != 0)
      *(dest--) = *(src++);
  }
}


ایستا صحیح str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  ثابت نویسه *fmt = luaL_checkstring(L, 1);  /* format string */
  صحیح arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  وقتی (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    وقتی (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    انتخاب (opt) {
      حالت Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        اگر (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        بشکن;
      }
      حالت Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        اگر (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        بشکن;
      }
      حالت Kfloat: {  /* C float */
        اعشاری f = (اعشاری)luaL_checknumber(L, arg);  /* get argument */
        نویسه *buff = luaL_prepbuffsize(&b, اندازه(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (نویسه *)&f, اندازه(f), h.islittle);
        luaL_addsize(&b, size);
        بشکن;
      }
      حالت Knumber: {  /* Lua float */
        lua_عدد f = luaL_checknumber(L, arg);  /* get argument */
        نویسه *buff = luaL_prepbuffsize(&b, اندازه(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (نویسه *)&f, اندازه(f), h.islittle);
        luaL_addsize(&b, size);
        بشکن;
      }
      حالت Kdouble: {  /* C double */
        دوگانه f = (دوگانه)luaL_checknumber(L, arg);  /* get argument */
        نویسه *buff = luaL_prepbuffsize(&b, اندازه(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (نویسه *)&f, اندازه(f), h.islittle);
        luaL_addsize(&b, size);
        بشکن;
      }
      حالت Kchar: {  /* fixed-size string */
        size_t len;
        ثابت نویسه *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        اگر (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          نویسه *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        بشکن;
      }
      حالت Kstring: {  /* strings with length count */
        size_t len;
        ثابت نویسه *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= اندازه(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        بشکن;
      }
      حالت Kzstr: {  /* zero-terminated string */
        size_t len;
        ثابت نویسه *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        بشکن;
      }
      حالت Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      حالت Kpaddalign: حالت Knop:
        arg--;  /* undo increment */
        بشکن;
    }
  }
  luaL_pushresult(&b);
  بازگشت 1;
}


ایستا صحیح str_packsize (lua_State *L) {
  Header h;
  ثابت نویسه *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  وقتی (*fmt != '\0') {
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
  بازگشت 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
ایستا lua_Integer unpackint (lua_State *L, ثابت نویسه *str,
                              صحیح islittle, صحیح size, صحیح issigned) {
  lua_Unsigned res = 0;
  صحیح i;
  صحیح limit = (size  <= SZINT) ? size : SZINT;
  برای (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned نویسه)str[islittle ? i : size - 1 - i];
  }
  اگر (size < SZINT) {  /* real size smaller than lua_Integer? */
    اگر (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  وگرنه اگر (size > SZINT) {  /* must check unread bytes */
    صحیح mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    برای (i = limit; i < size; i++) {
      اگر (l_unlikely((unsigned نویسه)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  بازگشت (lua_Integer)res;
}


ایستا صحیح str_unpack (lua_State *L) {
  Header h;
  ثابت نویسه *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ثابت نویسه *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  صحیح n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  وقتی (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    انتخاب (opt) {
      حالت Kint:
      حالت Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        بشکن;
      }
      حالت Kfloat: {
        اعشاری f;
        copywithendian((نویسه *)&f, data + pos, اندازه(f), h.islittle);
        lua_pushnumber(L, (lua_عدد)f);
        بشکن;
      }
      حالت Knumber: {
        lua_عدد f;
        copywithendian((نویسه *)&f, data + pos, اندازه(f), h.islittle);
        lua_pushnumber(L, f);
        بشکن;
      }
      حالت Kdouble: {
        دوگانه f;
        copywithendian((نویسه *)&f, data + pos, اندازه(f), h.islittle);
        lua_pushnumber(L, (lua_عدد)f);
        بشکن;
      }
      حالت Kchar: {
        lua_pushlstring(L, data + pos, size);
        بشکن;
      }
      حالت Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        بشکن;
      }
      حالت Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        بشکن;
      }
      حالت Kpaddalign: حالت Kpadding: حالت Knop:
        n--;  /* undo increment */
        بشکن;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  بازگشت n + 1;
}

/* }====================================================== */


ایستا ثابت luaL_Reg strlib[] = {
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
  {پوچ, پوچ}
};


ایستا تهی createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_برداشت(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_برداشت(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API صحیح luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  بازگشت 1;
}

