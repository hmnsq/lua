/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#ضمن "lprefix.h"


#ضمن <ctype.h>
#ضمن <float.h>
#ضمن <limits.h>
#ضمن <locale.h>
#ضمن <math.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#אם !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


ساكن صحيح str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  החזר 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
ساكن size_t posrelatI (lua_Integer pos, size_t len) {
  אם (pos > 0)
    החזר (size_t)pos;
  אחרת אם (pos == 0)
    החזר 1;
  אחרת אם (pos < -(lua_Integer)len)  /* inverted comparison */
    החזר 1;  /* clip to 1 */
  אחרת החזר len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ساكن size_t getendpos (lua_State *L, صحيح arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  אם (pos > (lua_Integer)len)
    החזר len;
  אחרת אם (pos >= 0)
    החזר (size_t)pos;
  אחרת אם (pos < -(lua_Integer)len)
    החזר 0;
  אחרת החזר len + (size_t)pos + 1;
}


ساكن صحيح str_sub (lua_State *L) {
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  אם (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  אחרת lua_pushliteral(L, "");
  החזר 1;
}


ساكن صحيح str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  עבור (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  החזר 1;
}


ساكن صحيح str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  עבור (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  החזר 1;
}


ساكن صحيح str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  עבור (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  החזר 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
ساكن صحيح str_rep (lua_State *L) {
  size_t len, lsep;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ثابت حرف *sep = luaL_optlstring(L, 3, "", &lsep);
  אם (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  אחרת אם (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    החזר luaL_error(L, "resulting string too large");
  אחרת {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    حرف *p = luaL_buffinitsize(L, &b, totallen);
    כלעוד (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * حجم(حرف)); p += len;
      אם (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * حجم(حرف)); p += lsep;
      }
    }
    memcpy(p, s, len * حجم(حرف));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  החזר 1;
}


ساكن صحيح str_byte (lua_State *L) {
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  صحيح n, i;
  אם (posi > pose) החזר 0;  /* empty interval; return no values */
  אם (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    החזר luaL_error(L, "string slice too long");
  n = (صحيح)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  עבור (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  החזר n;
}


ساكن صحيح str_char (lua_State *L) {
  صحيح n = lua_gettop(L);  /* number of arguments */
  صحيح i;
  luaL_Buffer b;
  حرف *p = luaL_buffinitsize(L, &b, cast_uint(n));
  עבור (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  החזר 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
بنية str_Writer {
  صحيح init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


ساكن صحيح writer (lua_State *L, ثابت فراغ *b, size_t size, فراغ *ud) {
  بنية str_Writer *state = (بنية str_Writer *)ud;
  אם (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  אם (b == عدم) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  אחרת
    luaL_addlstring(&state->B, (ثابت حرف *)b, size);
  החזר 0;
}


ساكن صحيح str_dump (lua_State *L) {
  بنية str_Writer state;
  صحيح strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  החזר 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#אם defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ساكن ثابت luaL_Reg stringmetamethods[] = {
  {"__index", عدم},  /* placeholder */
  {عدم, عدم}
};

#אחרת		/* }{ */

ساكن صحيح tonum (lua_State *L, صحيح arg) {
  אם (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    החזר 1;
  }
  אחרת {  /* check whether it is a numerical string */
    size_t len;
    ثابت حرف *s = lua_tolstring(L, arg, &len);
    החזר (s != عدم && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
ساكن فراغ trymt (lua_State *L, ثابت حرف *mtkey, ثابت حرف *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  אם (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_insert(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ساكن صحيح arith (lua_State *L, صحيح op, ثابت حرف *mtname) {
  אם (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  אחרת
    trymt(L, mtname, mtname + 2);
  החזר 1;
}


ساكن صحيح arith_add (lua_State *L) {
  החזר arith(L, LUA_OPADD, "__add");
}

ساكن صحيح arith_sub (lua_State *L) {
  החזר arith(L, LUA_OPSUB, "__sub");
}

ساكن صحيح arith_mul (lua_State *L) {
  החזר arith(L, LUA_OPMUL, "__mul");
}

ساكن صحيح arith_mod (lua_State *L) {
  החזר arith(L, LUA_OPMOD, "__mod");
}

ساكن صحيح arith_pow (lua_State *L) {
  החזר arith(L, LUA_OPPOW, "__pow");
}

ساكن صحيح arith_div (lua_State *L) {
  החזר arith(L, LUA_OPDIV, "__div");
}

ساكن صحيح arith_idiv (lua_State *L) {
  החזר arith(L, LUA_OPIDIV, "__idiv");
}

ساكن صحيح arith_unm (lua_State *L) {
  החזר arith(L, LUA_OPUNM, "__unm");
}


ساكن ثابت luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", عدم},  /* placeholder */
  {عدم, عدم}
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


تعريفنوع بنية MatchState {
  ثابت حرف *src_init;  /* init of source string */
  ثابت حرف *src_end;  /* end ('\0') of source string */
  ثابت حرف *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  صحيح matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  صحيح level;  /* total number of captures (finished or unfinished) */
  بنية {
    ثابت حرف *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
ساكن ثابت حرف *match (MatchState *ms, ثابت حرف *s, ثابت حرف *p);


/* maximum recursion depth for 'match' */
#אם !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


ساكن صحيح check_capture (MatchState *ms, صحيح l) {
  l -= '1';
  אם (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    החזר luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  החזר l;
}


ساكن صحيح capture_to_close (MatchState *ms) {
  صحيح level = ms->level;
  עבור (level--; level>=0; level--)
    אם (ms->capture[level].len == CAP_UNFINISHED) החזר level;
  החזר luaL_error(ms->L, "invalid pattern capture");
}


ساكن ثابت حرف *classend (MatchState *ms, ثابت حرف *p) {
  بدل (*p++) {
    حالة L_ESC: {
      אם (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      החזר p+1;
    }
    حالة '[': {
      אם (*p == '^') p++;
      עשה {  /* look for a ']' */
        אם (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        אם (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } כלעוד (*p != ']');
      החזר p+1;
    }
    افتراضي: {
      החזר p;
    }
  }
}


ساكن صحيح match_class (صحيح c, صحيح cl) {
  صحيح res;
  بدل (tolower(cl)) {
    حالة 'a' : res = isalpha(c); اكسر;
    حالة 'c' : res = iscntrl(c); اكسر;
    حالة 'd' : res = isdigit(c); اكسر;
    حالة 'g' : res = isgraph(c); اكسر;
    حالة 'l' : res = islower(c); اكسر;
    حالة 'p' : res = ispunct(c); اكسر;
    حالة 's' : res = isspace(c); اكسر;
    حالة 'u' : res = isupper(c); اكسر;
    حالة 'w' : res = isalnum(c); اكسر;
    حالة 'x' : res = isxdigit(c); اكسر;
    حالة 'z' : res = (c == 0); اكسر;  /* deprecated option */
    افتراضي: החזר (cl == c);
  }
  החזר (islower(cl) ? res : !res);
}


ساكن صحيح matchbracketclass (صحيح c, ثابت حرف *p, ثابت حرف *ec) {
  صحيح sig = 1;
  אם (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  כלעוד (++p < ec) {
    אם (*p == L_ESC) {
      p++;
      אם (match_class(c, cast_uchar(*p)))
        החזר sig;
    }
    אחרת אם ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      אם (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        החזר sig;
    }
    אחרת אם (cast_uchar(*p) == c) החזר sig;
  }
  החזר !sig;
}


ساكن صحيح singlematch (MatchState *ms, ثابت حرف *s, ثابت حرف *p,
                        ثابت حرف *ep) {
  אם (s >= ms->src_end)
    החזר 0;
  אחרת {
    صحيح c = cast_uchar(*s);
    بدل (*p) {
      حالة '.': החזר 1;  /* matches any char */
      حالة L_ESC: החזר match_class(c, cast_uchar(*(p+1)));
      حالة '[': החזר matchbracketclass(c, p, ep-1);
      افتراضي:  החזר (cast_uchar(*p) == c);
    }
  }
}


ساكن ثابت حرف *matchbalance (MatchState *ms, ثابت حرف *s,
                                   ثابت حرف *p) {
  אם (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  אם (*s != *p) החזר عدم;
  אחרת {
    صحيح b = *p;
    صحيح e = *(p+1);
    صحيح cont = 1;
    כלעוד (++s < ms->src_end) {
      אם (*s == e) {
        אם (--cont == 0) החזר s+1;
      }
      אחרת אם (*s == b) cont++;
    }
  }
  החזר عدم;  /* string ends out of balance */
}


ساكن ثابت حرف *max_expand (MatchState *ms, ثابت حرف *s,
                                 ثابت حرف *p, ثابت حرف *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  כלעוד (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  כלעוד (i>=0) {
    ثابت حرف *res = match(ms, (s+i), ep+1);
    אם (res) החזר res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  החזר عدم;
}


ساكن ثابت حرف *min_expand (MatchState *ms, ثابت حرف *s,
                                 ثابت حرف *p, ثابت حرف *ep) {
  עבור (;;) {
    ثابت حرف *res = match(ms, s, ep+1);
    אם (res != عدم)
      החזר res;
    אחרת אם (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    אחרת החזר عدم;
  }
}


ساكن ثابت حرف *start_capture (MatchState *ms, ثابت حرف *s,
                                    ثابت حرف *p, صحيح what) {
  ثابت حرف *res;
  صحيح level = ms->level;
  אם (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  אם ((res=match(ms, s, p)) == عدم)  /* match failed? */
    ms->level--;  /* undo capture */
  החזר res;
}


ساكن ثابت حرف *end_capture (MatchState *ms, ثابت حرف *s,
                                  ثابت حرف *p) {
  صحيح l = capture_to_close(ms);
  ثابت حرف *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  אם ((res = match(ms, s, p)) == عدم)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  החזר res;
}


ساكن ثابت حرف *match_capture (MatchState *ms, ثابت حرف *s, صحيح l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  אם ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    החזר s+len;
  אחרת החזר عدم;
}


ساكن ثابت حرف *match (MatchState *ms, ثابت حرف *s, ثابت حرف *p) {
  אם (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  אם (p != ms->p_end) {  /* end of pattern? */
    بدل (*p) {
      حالة '(': {  /* start capture */
        אם (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        אחרת
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        اكسر;
      }
      حالة ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        اكسر;
      }
      حالة '$': {
        אם ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          اذهب dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : عدم;  /* check end of string */
        اكسر;
      }
      حالة L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        بدل (*(p + 1)) {
          حالة 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            אם (s != عدم) {
              p += 4; اذهب init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            اكسر;
          }
          حالة 'f': {  /* frontier? */
            ثابت حرف *ep; حرف previous;
            p += 2;
            אם (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            אם (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; اذهب init;  /* return match(ms, s, ep); */
            }
            s = عدم;  /* match failed */
            اكسر;
          }
          حالة '0': حالة '1': حالة '2': حالة '3':
          حالة '4': حالة '5': حالة '6': حالة '7':
          حالة '8': حالة '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            אם (s != عدم) {
              p += 2; اذهب init;  /* return match(ms, s, p + 2) */
            }
            اكسر;
          }
          افتراضي: اذهب dflt;
        }
        اكسر;
      }
      افتراضي: dflt: {  /* pattern class plus optional suffix */
        ثابت حرف *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        אם (!singlematch(ms, s, p, ep)) {
          אם (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; اذهب init;  /* return match(ms, s, ep + 1); */
          }
          אחרת  /* '+' or no suffix */
            s = عدم;  /* fail */
        }
        אחרת {  /* matched once */
          بدل (*ep) {  /* handle optional suffix */
            حالة '?': {  /* optional */
              ثابت حرف *res;
              אם ((res = match(ms, s + 1, ep + 1)) != عدم)
                s = res;
              אחרת {
                p = ep + 1; اذهب init;  /* else return match(ms, s, ep + 1); */
              }
              اكسر;
            }
            حالة '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            حالة '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              اكسر;
            حالة '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              اكسر;
            افتراضي:  /* no suffix */
              s++; p = ep; اذهب init;  /* return match(ms, s + 1, ep); */
          }
        }
        اكسر;
      }
    }
  }
  ms->matchdepth++;
  החזר s;
}



ساكن ثابت حرف *lmemfind (ثابت حرف *s1, size_t l1,
                               ثابت حرف *s2, size_t l2) {
  אם (l2 == 0) החזר s1;  /* empty strings are everywhere */
  אחרת אם (l2 > l1) החזר عدم;  /* avoids a negative 'l1' */
  אחרת {
    ثابت حرف *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    כלעוד (l1 > 0 && (init = (ثابت حرف *)memchr(s1, *s2, l1)) != عدم) {
      init++;   /* 1st char is already checked */
      אם (memcmp(init, s2+1, l2) == 0)
        החזר init-1;
      אחרת {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    החזר عدم;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
ساكن ptrdiff_t get_onecapture (MatchState *ms, صحيح i, ثابت حرف *s,
                              ثابت حرف *e, ثابت حرف **cap) {
  אם (i >= ms->level) {
    אם (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    החזר (e - s);
  }
  אחרת {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    אם (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    אחרת אם (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    החזר capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ساكن فراغ push_onecapture (MatchState *ms, صحيح i, ثابت حرف *s,
                                                    ثابت حرف *e) {
  ثابت حرف *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  אם (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ساكن صحيح push_captures (MatchState *ms, ثابت حرف *s, ثابت حرف *e) {
  صحيح i;
  صحيح nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  עבור (i = 0; i < nlevels; i++)
    push_onecapture(ms, i, s, e);
  החזר nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ساكن صحيح nospecials (ثابت حرف *p, size_t l) {
  size_t upto = 0;
  עשה {
    אם (strpbrk(p + upto, SPECIALS))
      החזר 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } כלעוד (upto <= l);
  החזר 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
ساكن فراغ prepstate (MatchState *ms, lua_State *L,
                       ثابت حرف *s, size_t ls, ثابت حرف *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
ساكن فراغ reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


ساكن صحيح str_find_aux (lua_State *L, صحيح find) {
  size_t ls, lp;
  ثابت حرف *s = luaL_checklstring(L, 1, &ls);
  ثابت حرف *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  אם (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    החזר 1;
  }
  /* explicit request or no special characters? */
  אם (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ثابت حرف *s2 = lmemfind(s + init, ls - init, p, lp);
    אם (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      החזר 2;
    }
  }
  אחרת {
    MatchState ms;
    ثابت حرف *s1 = s + init;
    صحيح anchor = (*p == '^');
    אם (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    עשה {
      ثابت حرف *res;
      reprepstate(&ms);
      אם ((res=match(&ms, s1, p)) != عدم) {
        אם (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          החזר push_captures(&ms, عدم, 0) + 2;
        }
        אחרת
          החזר push_captures(&ms, s1, res);
      }
    } כלעוד (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  החזר 1;
}


ساكن صحيح str_find (lua_State *L) {
  החזר str_find_aux(L, 1);
}


ساكن صحيح str_match (lua_State *L) {
  החזר str_find_aux(L, 0);
}


/* state for 'gmatch' */
تعريفنوع بنية GMatchState {
  ثابت حرف *src;  /* current position */
  ثابت حرف *p;  /* pattern */
  ثابت حرف *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


ساكن صحيح gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ثابت حرف *src;
  gm->ms.L = L;
  עבור (src = gm->src; src <= gm->ms.src_end; src++) {
    ثابت حرف *e;
    reprepstate(&gm->ms);
    אם ((e = match(&gm->ms, src, gm->p)) != عدم && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      החזר push_captures(&gm->ms, src, e);
    }
  }
  החזר 0;  /* not found */
}


ساكن صحيح gmatch (lua_State *L) {
  size_t ls, lp;
  ثابت حرف *s = luaL_checklstring(L, 1, &ls);
  ثابت حرف *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, حجم(GMatchState), 0);
  אם (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = عدم;
  lua_pushcclosure(L, gmatch_aux, 3);
  החזר 1;
}


ساكن فراغ add_s (MatchState *ms, luaL_Buffer *b, ثابت حرف *s,
                                                   ثابت حرف *e) {
  size_t l;
  lua_State *L = ms->L;
  ثابت حرف *news = lua_tolstring(L, 3, &l);
  ثابت حرف *p;
  כלעוד ((p = (حرف *)memchr(news, L_ESC, l)) != عدم) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    אם (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    אחרת אם (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    אחרת אם (isdigit(cast_uchar(*p))) {  /* '%n' */
      ثابت حرف *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      אם (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      אחרת
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    אחרת
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
ساكن صحيح add_value (MatchState *ms, luaL_Buffer *b, ثابت حرف *s,
                                      ثابت حرف *e, صحيح tr) {
  lua_State *L = ms->L;
  بدل (tr) {
    حالة LUA_TFUNCTION: {  /* call the function */
      صحيح n;
      lua_pushvalue(L, 3);  /* push the function */
      n = push_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      اكسر;
    }
    حالة LUA_TTABLE: {  /* index the table */
      push_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      اكسر;
    }
    افتراضي: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      החזר 1;  /* something changed */
    }
  }
  אם (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_pop(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    החזר 0;  /* no changes */
  }
  אחרת אם (l_unlikely(!lua_isstring(L, -1)))
    החזר luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  אחרת {
    luaL_addvalue(b);  /* add result to accumulator */
    החזר 1;  /* something changed */
  }
}


ساكن صحيح str_gsub (lua_State *L) {
  size_t srcl, lp;
  ثابت حرف *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ثابت حرف *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  ثابت حرف *lastmatch = عدم;  /* end of last match */
  صحيح tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  صحيح anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  صحيح changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  אם (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  כלעוד (n < max_s) {
    ثابت حرف *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    אם ((e = match(&ms, src, p)) != عدم && e != lastmatch) {  /* match? */
      n++;
      changed = add_value(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    אחרת אם (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    אחרת اكسر;  /* end of subject */
    אם (anchor) اكسر;
  }
  אם (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  אחרת {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  החזר 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#אם !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(حجم(LUA_NUMBER_FRMLEN)/حجم(حرف))


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
ساكن lua_Number adddigit (حرف *buff, unsigned n, lua_Number x) {
  lua_Number dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  صحيح d = (صحيح)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  החזר x - dd;  /* return what is left */
}


ساكن صحيح num2straux (حرف *buff, unsigned sz, lua_Number x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  אם (x != x || x == (lua_Number)HUGE_VAL || x == -(lua_Number)HUGE_VAL)
    החזר l_sprintf(buff, sz, LUA_NUMBER_FMT, (LUAI_UACNUMBER)x);
  אחרת אם (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    החזר l_sprintf(buff, sz, LUA_NUMBER_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  אחרת {
    صحيح e;
    lua_Number m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    אם (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    אם (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      עשה {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } כלעוד (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    החזר cast_int(n);
  }
}


ساكن صحيح lua_number2strx (lua_State *L, حرف *buff, unsigned sz,
                            ثابت حرف *fmt, lua_Number x) {
  صحيح n = num2straux(buff, sz, x);
  אם (fmt[SIZELENMOD] == 'A') {
    صحيح i;
    עבור (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  אחרת אם (l_unlikely(fmt[SIZELENMOD] != 'a'))
    החזר luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  החזר n;
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
#אם !defined(L_FMTFLAGSF)

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


ساكن فراغ addquoted (luaL_Buffer *b, ثابت حرف *s, size_t len) {
  luaL_addchar(b, '"');
  כלעוד (len--) {
    אם (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    אחרת אם (iscntrl(cast_uchar(*s))) {
      حرف buff[10];
      אם (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, حجم(buff), "\\%d", (صحيح)cast_uchar(*s));
      אחרת
        l_sprintf(buff, حجم(buff), "\\%03d", (صحيح)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    אחרת
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
ساكن صحيح quotefloat (lua_State *L, حرف *buff, lua_Number n) {
  ثابت حرف *s;  /* for the fixed representations */
  אם (n == (lua_Number)HUGE_VAL)  /* inf? */
    s = "1e9999";
  אחרת אם (n == -(lua_Number)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  אחרת אם (n != n)  /* NaN? */
    s = "(0/0)";
  אחרת {  /* format number as hexadecimal */
    صحيح  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_NUMBER_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    אם (memchr(buff, '.', cast_uint(nb)) == عدم) {  /* no dot? */
      حرف point = lua_getlocaledecpoint();  /* try locale point */
      حرف *ppoint = (حرف *)memchr(buff, point, cast_uint(nb));
      אם (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    החזר nb;
  }
  /* for the fixed representations */
  החזר l_sprintf(buff, MAX_ITEM, "%s", s);
}


ساكن فراغ addliteral (lua_State *L, luaL_Buffer *b, صحيح arg) {
  بدل (lua_type(L, arg)) {
    حالة LUA_TSTRING: {
      size_t len;
      ثابت حرف *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      اكسر;
    }
    حالة LUA_TNUMBER: {
      حرف *buff = luaL_prepbuffsize(b, MAX_ITEM);
      صحيح nb;
      אם (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      אחרת {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ثابت حرف *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      اكسر;
    }
    حالة LUA_TNIL: حالة LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, عدم);
      luaL_addvalue(b);
      اكسر;
    }
    افتراضي: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


ساكن ثابت حرف *get2digits (ثابت حرف *s) {
  אם (isdigit(cast_uchar(*s))) {
    s++;
    אם (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  החזר s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
ساكن فراغ checkformat (lua_State *L, ثابت حرف *form, ثابت حرف *flags,
                                       صحيح precision) {
  ثابت حرف *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  אם (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    אם (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  אם (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
ساكن ثابت حرف *getformat (lua_State *L, ثابت حرف *strfrmt,
                                            حرف *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  אם (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * حجم(حرف));
  *(form + len) = '\0';
  החזר strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
ساكن فراغ addlenmod (حرف *form, ثابت حرف *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  حرف spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


ساكن صحيح str_format (lua_State *L) {
  صحيح top = lua_gettop(L);
  صحيح arg = 1;
  size_t sfl;
  ثابت حرف *strfrmt = luaL_checklstring(L, arg, &sfl);
  ثابت حرف *strfrmt_end = strfrmt+sfl;
  ثابت حرف *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  כלעוד (strfrmt < strfrmt_end) {
    אם (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    אחרת אם (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    אחרת { /* format item */
      حرف form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      حرف *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      صحيح nb = 0;  /* number of bytes in result */
      אם (++arg > top)
        החזר luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      بدل (*strfrmt++) {
        حالة 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (صحيح)luaL_checkinteger(L, arg));
          اكسر;
        }
        حالة 'd': حالة 'i':
          flags = L_FMTFLAGSI;
          اذهب intcase;
        حالة 'u':
          flags = L_FMTFLAGSU;
          اذهب intcase;
        حالة 'o': حالة 'x': حالة 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          اكسر;
        }
        حالة 'a': حالة 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          اكسر;
        حالة 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        حالة 'e': حالة 'E': حالة 'g': حالة 'G': {
          lua_Number n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          اكسر;
        }
        حالة 'p': {
          ثابت فراغ *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          אם (p == عدم) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          اكسر;
        }
        حالة 'q': {
          אם (form[2] != '\0')  /* modifiers? */
            החזר luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          اكسر;
        }
        حالة 's': {
          size_t l;
          ثابت حرف *s = luaL_tolstring(L, arg, &l);
          אם (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          אחרת {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            אם (strchr(form, '.') == عدم && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            אחרת {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_pop(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          اكسر;
        }
        افتراضي: {  /* also treat cases 'pnLlh' */
          החזר luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  החזר 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#אם !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((صحيح)حجم(lua_Integer))


/* dummy union to get native endianness */
ساكن ثابت اتحاد {
  صحيح dummy;
  حرف little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
تعريفنوع بنية Header {
  lua_State *L;
  صحيح islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
تعريفنوع تعداد KOption {
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
ساكن صحيح digit (صحيح c) { החזר '0' <= c && c <= '9'; }

ساكن size_t getnum (ثابت حرف **fmt, size_t df) {
  אם (!digit(**fmt))  /* no number? */
    החזר df;  /* return default value */
  אחרת {
    size_t a = 0;
    עשה {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } כלעוד (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    החזר a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ساكن unsigned getnumlimit (Header *h, ثابت حرف **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  אם (l_unlikely((sz - 1u) >= MAXINTSIZE))
    החזר cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  החזר cast_uint(sz);
}


/*
** Initialize Header
*/
ساكن فراغ initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
ساكن KOption getoption (Header *h, ثابت حرف **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  بنية cD { حرف c; اتحاد { LUAI_MAXALIGN; } u; };
  صحيح opt = *((*fmt)++);
  *size = 0;  /* default */
  بدل (opt) {
    حالة 'b': *size = حجم(حرف); החזר Kint;
    حالة 'B': *size = حجم(حرف); החזר Kuint;
    حالة 'h': *size = حجم(short); החזר Kint;
    حالة 'H': *size = حجم(short); החזר Kuint;
    حالة 'l': *size = حجم(long); החזר Kint;
    حالة 'L': *size = حجم(long); החזר Kuint;
    حالة 'j': *size = حجم(lua_Integer); החזר Kint;
    حالة 'J': *size = حجم(lua_Integer); החזר Kuint;
    حالة 'T': *size = حجم(size_t); החזר Kuint;
    حالة 'f': *size = حجم(عائم); החזר Kfloat;
    حالة 'n': *size = حجم(lua_Number); החזר Knumber;
    حالة 'd': *size = حجم(مزدوج); החזר Kdouble;
    حالة 'i': *size = getnumlimit(h, fmt, حجم(صحيح)); החזר Kint;
    حالة 'I': *size = getnumlimit(h, fmt, حجم(صحيح)); החזר Kuint;
    حالة 's': *size = getnumlimit(h, fmt, حجم(size_t)); החזר Kstring;
    حالة 'c':
      *size = getnum(fmt, cast_sizet(-1));
      אם (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      החזר Kchar;
    حالة 'z': החזר Kzstr;
    حالة 'x': *size = 1; החזר Kpadding;
    حالة 'X': החזר Kpaddalign;
    حالة ' ': اكسر;
    حالة '<': h->islittle = 1; اكسر;
    حالة '>': h->islittle = 0; اكسر;
    حالة '=': h->islittle = nativeendian.little; اكسر;
    حالة '!': {
      ثابت size_t maxalign = offsetof(بنية cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      اكسر;
    }
    افتراضي: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  החזר Knop;
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
ساكن KOption getdetails (Header *h, size_t totalsize, ثابت حرف **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  אם (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    אם (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  אם (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  אחרת {
    אם (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    אם (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    אחרת {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  החזר opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
ساكن فراغ packint (luaL_Buffer *b, lua_Unsigned n,
                     صحيح islittle, unsigned size, صحيح neg) {
  حرف *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (حرف)(n & MC);  /* first byte */
  עבור (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (حرف)(n & MC);
  }
  אם (neg && size > SZINT) {  /* negative number need sign extension? */
    עבור (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (حرف)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
ساكن فراغ copywithendian (حرف *dest, ثابت حرف *src,
                            unsigned size, صحيح islittle) {
  אם (islittle == nativeendian.little)
    memcpy(dest, src, size);
  אחרת {
    dest += size - 1;
    כלעוד (size-- != 0)
      *(dest--) = *(src++);
  }
}


ساكن صحيح str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  ثابت حرف *fmt = luaL_checkstring(L, 1);  /* format string */
  صحيح arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  כלעוד (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    כלעוד (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    بدل (opt) {
      حالة Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        אם (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        اكسر;
      }
      حالة Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        אם (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        اكسر;
      }
      حالة Kfloat: {  /* C float */
        عائم f = (عائم)luaL_checknumber(L, arg);  /* get argument */
        حرف *buff = luaL_prepbuffsize(&b, حجم(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (حرف *)&f, حجم(f), h.islittle);
        luaL_addsize(&b, size);
        اكسر;
      }
      حالة Knumber: {  /* Lua float */
        lua_Number f = luaL_checknumber(L, arg);  /* get argument */
        حرف *buff = luaL_prepbuffsize(&b, حجم(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (حرف *)&f, حجم(f), h.islittle);
        luaL_addsize(&b, size);
        اكسر;
      }
      حالة Kdouble: {  /* C double */
        مزدوج f = (مزدوج)luaL_checknumber(L, arg);  /* get argument */
        حرف *buff = luaL_prepbuffsize(&b, حجم(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (حرف *)&f, حجم(f), h.islittle);
        luaL_addsize(&b, size);
        اكسر;
      }
      حالة Kchar: {  /* fixed-size string */
        size_t len;
        ثابت حرف *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        אם (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          حرف *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        اكسر;
      }
      حالة Kstring: {  /* strings with length count */
        size_t len;
        ثابت حرف *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= حجم(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        اكسر;
      }
      حالة Kzstr: {  /* zero-terminated string */
        size_t len;
        ثابت حرف *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        اكسر;
      }
      حالة Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      حالة Kpaddalign: حالة Knop:
        arg--;  /* undo increment */
        اكسر;
    }
  }
  luaL_pushresult(&b);
  החזר 1;
}


ساكن صحيح str_packsize (lua_State *L) {
  Header h;
  ثابت حرف *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  כלעוד (*fmt != '\0') {
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
  החזר 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
ساكن lua_Integer unpackint (lua_State *L, ثابت حرف *str,
                              صحيح islittle, صحيح size, صحيح issigned) {
  lua_Unsigned res = 0;
  صحيح i;
  صحيح limit = (size  <= SZINT) ? size : SZINT;
  עבור (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned حرف)str[islittle ? i : size - 1 - i];
  }
  אם (size < SZINT) {  /* real size smaller than lua_Integer? */
    אם (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  אחרת אם (size > SZINT) {  /* must check unread bytes */
    صحيح mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    עבור (i = limit; i < size; i++) {
      אם (l_unlikely((unsigned حرف)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  החזר (lua_Integer)res;
}


ساكن صحيح str_unpack (lua_State *L) {
  Header h;
  ثابت حرف *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ثابت حرف *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  صحيح n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  כלעוד (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    بدل (opt) {
      حالة Kint:
      حالة Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        اكسر;
      }
      حالة Kfloat: {
        عائم f;
        copywithendian((حرف *)&f, data + pos, حجم(f), h.islittle);
        lua_pushnumber(L, (lua_Number)f);
        اكسر;
      }
      حالة Knumber: {
        lua_Number f;
        copywithendian((حرف *)&f, data + pos, حجم(f), h.islittle);
        lua_pushnumber(L, f);
        اكسر;
      }
      حالة Kdouble: {
        مزدوج f;
        copywithendian((حرف *)&f, data + pos, حجم(f), h.islittle);
        lua_pushnumber(L, (lua_Number)f);
        اكسر;
      }
      حالة Kchar: {
        lua_pushlstring(L, data + pos, size);
        اكسر;
      }
      حالة Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        اكسر;
      }
      حالة Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        اكسر;
      }
      حالة Kpaddalign: حالة Kpadding: حالة Knop:
        n--;  /* undo increment */
        اكسر;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  החזר n + 1;
}

/* }====================================================== */


ساكن ثابت luaL_Reg strlib[] = {
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
  {عدم, عدم}
};


ساكن فراغ createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_pop(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_pop(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API صحيح luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  החזר 1;
}

