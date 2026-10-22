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
#إذا !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


ساكن صحيح str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  أرجع 1;
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
  إذا (pos > 0)
    أرجع (size_t)pos;
  وإلا إذا (pos == 0)
    أرجع 1;
  وإلا إذا (pos < -(lua_Integer)len)  /* inverted comparison */
    أرجع 1;  /* clip to 1 */
  وإلا أرجع len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ساكن size_t getendpos (lua_State *L, صحيح arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  إذا (pos > (lua_Integer)len)
    أرجع len;
  وإلا إذا (pos >= 0)
    أرجع (size_t)pos;
  وإلا إذا (pos < -(lua_Integer)len)
    أرجع 0;
  وإلا أرجع len + (size_t)pos + 1;
}


ساكن صحيح str_sub (lua_State *L) {
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  إذا (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  وإلا lua_pushliteral(L, "");
  أرجع 1;
}


ساكن صحيح str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  لكل (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  أرجع 1;
}


ساكن صحيح str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  لكل (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  أرجع 1;
}


ساكن صحيح str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  لكل (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  أرجع 1;
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
  إذا (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  وإلا إذا (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    أرجع luaL_error(L, "resulting string too large");
  وإلا {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    حرف *p = luaL_buffinitsize(L, &b, totallen);
    طالما (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * حجم(حرف)); p += len;
      إذا (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * حجم(حرف)); p += lsep;
      }
    }
    memcpy(p, s, len * حجم(حرف));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  أرجع 1;
}


ساكن صحيح str_byte (lua_State *L) {
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  صحيح n, i;
  إذا (posi > pose) أرجع 0;  /* empty interval; return no values */
  إذا (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    أرجع luaL_error(L, "string slice too long");
  n = (صحيح)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  لكل (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  أرجع n;
}


ساكن صحيح str_char (lua_State *L) {
  صحيح n = lua_gettop(L);  /* number of arguments */
  صحيح i;
  luaL_Buffer b;
  حرف *p = luaL_buffinitsize(L, &b, cast_uint(n));
  لكل (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  أرجع 1;
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
  إذا (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  إذا (b == عدم) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  وإلا
    luaL_addlstring(&state->B, (ثابت حرف *)b, size);
  أرجع 0;
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
  أرجع 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#إذا defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ساكن ثابت luaL_Reg stringmetamethods[] = {
  {"__index", عدم},  /* placeholder */
  {عدم, عدم}
};

#وإلا		/* }{ */

ساكن صحيح tonum (lua_State *L, صحيح arg) {
  إذا (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    أرجع 1;
  }
  وإلا {  /* check whether it is a numerical string */
    size_t len;
    ثابت حرف *s = lua_tolstring(L, arg, &len);
    أرجع (s != عدم && lua_stringtonumber(L, s) == len + 1);
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
  إذا (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_قىستۇرۇش(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ساكن صحيح arith (lua_State *L, صحيح op, ثابت حرف *mtname) {
  إذا (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  وإلا
    trymt(L, mtname, mtname + 2);
  أرجع 1;
}


ساكن صحيح arith_add (lua_State *L) {
  أرجع arith(L, LUA_OPADD, "__add");
}

ساكن صحيح arith_sub (lua_State *L) {
  أرجع arith(L, LUA_OPSUB, "__sub");
}

ساكن صحيح arith_mul (lua_State *L) {
  أرجع arith(L, LUA_OPMUL, "__mul");
}

ساكن صحيح arith_mod (lua_State *L) {
  أرجع arith(L, LUA_OPMOD, "__mod");
}

ساكن صحيح arith_pow (lua_State *L) {
  أرجع arith(L, LUA_OPPOW, "__pow");
}

ساكن صحيح arith_div (lua_State *L) {
  أرجع arith(L, LUA_OPDIV, "__div");
}

ساكن صحيح arith_idiv (lua_State *L) {
  أرجع arith(L, LUA_OPIDIV, "__idiv");
}

ساكن صحيح arith_unm (lua_State *L) {
  أرجع arith(L, LUA_OPUNM, "__unm");
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
#إذا !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


ساكن صحيح check_capture (MatchState *ms, صحيح l) {
  l -= '1';
  إذا (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    أرجع luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  أرجع l;
}


ساكن صحيح capture_to_close (MatchState *ms) {
  صحيح level = ms->level;
  لكل (level--; level>=0; level--)
    إذا (ms->capture[level].len == CAP_UNFINISHED) أرجع level;
  أرجع luaL_error(ms->L, "invalid pattern capture");
}


ساكن ثابت حرف *classend (MatchState *ms, ثابت حرف *p) {
  بدل (*p++) {
    حالة L_ESC: {
      إذا (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      أرجع p+1;
    }
    حالة '[': {
      إذا (*p == '^') p++;
      افعل {  /* look for a ']' */
        إذا (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        إذا (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } طالما (*p != ']');
      أرجع p+1;
    }
    افتراضي: {
      أرجع p;
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
    افتراضي: أرجع (cl == c);
  }
  أرجع (islower(cl) ? res : !res);
}


ساكن صحيح matchbracketclass (صحيح c, ثابت حرف *p, ثابت حرف *ec) {
  صحيح sig = 1;
  إذا (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  طالما (++p < ec) {
    إذا (*p == L_ESC) {
      p++;
      إذا (match_class(c, cast_uchar(*p)))
        أرجع sig;
    }
    وإلا إذا ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      إذا (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        أرجع sig;
    }
    وإلا إذا (cast_uchar(*p) == c) أرجع sig;
  }
  أرجع !sig;
}


ساكن صحيح singlematch (MatchState *ms, ثابت حرف *s, ثابت حرف *p,
                        ثابت حرف *ep) {
  إذا (s >= ms->src_end)
    أرجع 0;
  وإلا {
    صحيح c = cast_uchar(*s);
    بدل (*p) {
      حالة '.': أرجع 1;  /* matches any char */
      حالة L_ESC: أرجع match_class(c, cast_uchar(*(p+1)));
      حالة '[': أرجع matchbracketclass(c, p, ep-1);
      افتراضي:  أرجع (cast_uchar(*p) == c);
    }
  }
}


ساكن ثابت حرف *matchbalance (MatchState *ms, ثابت حرف *s,
                                   ثابت حرف *p) {
  إذا (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  إذا (*s != *p) أرجع عدم;
  وإلا {
    صحيح b = *p;
    صحيح e = *(p+1);
    صحيح cont = 1;
    طالما (++s < ms->src_end) {
      إذا (*s == e) {
        إذا (--cont == 0) أرجع s+1;
      }
      وإلا إذا (*s == b) cont++;
    }
  }
  أرجع عدم;  /* string ends out of balance */
}


ساكن ثابت حرف *max_expand (MatchState *ms, ثابت حرف *s,
                                 ثابت حرف *p, ثابت حرف *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  طالما (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  طالما (i>=0) {
    ثابت حرف *res = match(ms, (s+i), ep+1);
    إذا (res) أرجع res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  أرجع عدم;
}


ساكن ثابت حرف *min_expand (MatchState *ms, ثابت حرف *s,
                                 ثابت حرف *p, ثابت حرف *ep) {
  لكل (;;) {
    ثابت حرف *res = match(ms, s, ep+1);
    إذا (res != عدم)
      أرجع res;
    وإلا إذا (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    وإلا أرجع عدم;
  }
}


ساكن ثابت حرف *start_capture (MatchState *ms, ثابت حرف *s,
                                    ثابت حرف *p, صحيح what) {
  ثابت حرف *res;
  صحيح level = ms->level;
  إذا (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  إذا ((res=match(ms, s, p)) == عدم)  /* match failed? */
    ms->level--;  /* undo capture */
  أرجع res;
}


ساكن ثابت حرف *end_capture (MatchState *ms, ثابت حرف *s,
                                  ثابت حرف *p) {
  صحيح l = capture_to_close(ms);
  ثابت حرف *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  إذا ((res = match(ms, s, p)) == عدم)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  أرجع res;
}


ساكن ثابت حرف *match_capture (MatchState *ms, ثابت حرف *s, صحيح l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  إذا ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    أرجع s+len;
  وإلا أرجع عدم;
}


ساكن ثابت حرف *match (MatchState *ms, ثابت حرف *s, ثابت حرف *p) {
  إذا (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  إذا (p != ms->p_end) {  /* end of pattern? */
    بدل (*p) {
      حالة '(': {  /* start capture */
        إذا (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        وإلا
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        اكسر;
      }
      حالة ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        اكسر;
      }
      حالة '$': {
        إذا ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          اذهب dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : عدم;  /* check end of string */
        اكسر;
      }
      حالة L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        بدل (*(p + 1)) {
          حالة 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            إذا (s != عدم) {
              p += 4; اذهب init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            اكسر;
          }
          حالة 'f': {  /* frontier? */
            ثابت حرف *ep; حرف previous;
            p += 2;
            إذا (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            إذا (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
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
            إذا (s != عدم) {
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
        إذا (!singlematch(ms, s, p, ep)) {
          إذا (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; اذهب init;  /* return match(ms, s, ep + 1); */
          }
          وإلا  /* '+' or no suffix */
            s = عدم;  /* fail */
        }
        وإلا {  /* matched once */
          بدل (*ep) {  /* handle optional suffix */
            حالة '?': {  /* optional */
              ثابت حرف *res;
              إذا ((res = match(ms, s + 1, ep + 1)) != عدم)
                s = res;
              وإلا {
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
  أرجع s;
}



ساكن ثابت حرف *lmemfind (ثابت حرف *s1, size_t l1,
                               ثابت حرف *s2, size_t l2) {
  إذا (l2 == 0) أرجع s1;  /* empty strings are everywhere */
  وإلا إذا (l2 > l1) أرجع عدم;  /* avoids a negative 'l1' */
  وإلا {
    ثابت حرف *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    طالما (l1 > 0 && (init = (ثابت حرف *)memchr(s1, *s2, l1)) != عدم) {
      init++;   /* 1st char is already checked */
      إذا (memcmp(init, s2+1, l2) == 0)
        أرجع init-1;
      وإلا {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    أرجع عدم;  /* not found */
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
  إذا (i >= ms->level) {
    إذا (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    أرجع (e - s);
  }
  وإلا {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    إذا (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    وإلا إذا (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    أرجع capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ساكن فراغ ئىتتىرىش_onecapture (MatchState *ms, صحيح i, ثابت حرف *s,
                                                    ثابت حرف *e) {
  ثابت حرف *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  إذا (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ساكن صحيح ئىتتىرىش_captures (MatchState *ms, ثابت حرف *s, ثابت حرف *e) {
  صحيح i;
  صحيح nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  لكل (i = 0; i < nlevels; i++)
    ئىتتىرىش_onecapture(ms, i, s, e);
  أرجع nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ساكن صحيح nospecials (ثابت حرف *p, size_t l) {
  size_t upto = 0;
  افعل {
    إذا (strpbrk(p + upto, SPECIALS))
      أرجع 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } طالما (upto <= l);
  أرجع 1;  /* no special chars found */
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
  إذا (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    أرجع 1;
  }
  /* explicit request or no special characters? */
  إذا (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ثابت حرف *s2 = lmemfind(s + init, ls - init, p, lp);
    إذا (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      أرجع 2;
    }
  }
  وإلا {
    MatchState ms;
    ثابت حرف *s1 = s + init;
    صحيح anchor = (*p == '^');
    إذا (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    افعل {
      ثابت حرف *res;
      reprepstate(&ms);
      إذا ((res=match(&ms, s1, p)) != عدم) {
        إذا (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          أرجع ئىتتىرىش_captures(&ms, عدم, 0) + 2;
        }
        وإلا
          أرجع ئىتتىرىش_captures(&ms, s1, res);
      }
    } طالما (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  أرجع 1;
}


ساكن صحيح str_find (lua_State *L) {
  أرجع str_find_aux(L, 1);
}


ساكن صحيح str_match (lua_State *L) {
  أرجع str_find_aux(L, 0);
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
  لكل (src = gm->src; src <= gm->ms.src_end; src++) {
    ثابت حرف *e;
    reprepstate(&gm->ms);
    إذا ((e = match(&gm->ms, src, gm->p)) != عدم && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      أرجع ئىتتىرىش_captures(&gm->ms, src, e);
    }
  }
  أرجع 0;  /* not found */
}


ساكن صحيح gmatch (lua_State *L) {
  size_t ls, lp;
  ثابت حرف *s = luaL_checklstring(L, 1, &ls);
  ثابت حرف *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, حجم(GMatchState), 0);
  إذا (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = عدم;
  lua_pushcclosure(L, gmatch_aux, 3);
  أرجع 1;
}


ساكن فراغ add_s (MatchState *ms, luaL_Buffer *b, ثابت حرف *s,
                                                   ثابت حرف *e) {
  size_t l;
  lua_State *L = ms->L;
  ثابت حرف *news = lua_tolstring(L, 3, &l);
  ثابت حرف *p;
  طالما ((p = (حرف *)memchr(news, L_ESC, l)) != عدم) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    إذا (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    وإلا إذا (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    وإلا إذا (isdigit(cast_uchar(*p))) {  /* '%n' */
      ثابت حرف *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      إذا (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      وإلا
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    وإلا
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
ساكن صحيح add_قىممەت (MatchState *ms, luaL_Buffer *b, ثابت حرف *s,
                                      ثابت حرف *e, صحيح tr) {
  lua_State *L = ms->L;
  بدل (tr) {
    حالة LUA_TFUNCTION: {  /* call the function */
      صحيح n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ئىتتىرىش_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      اكسر;
    }
    حالة LUA_TTABLE: {  /* index the table */
      ئىتتىرىش_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      اكسر;
    }
    افتراضي: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      أرجع 1;  /* something changed */
    }
  }
  إذا (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_چىقىرىش(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    أرجع 0;  /* no changes */
  }
  وإلا إذا (l_unlikely(!lua_isstring(L, -1)))
    أرجع luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  وإلا {
    luaL_addvalue(b);  /* add result to accumulator */
    أرجع 1;  /* something changed */
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
  إذا (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  طالما (n < max_s) {
    ثابت حرف *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    إذا ((e = match(&ms, src, p)) != عدم && e != lastmatch) {  /* match? */
      n++;
      changed = add_قىممەت(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    وإلا إذا (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    وإلا اكسر;  /* end of subject */
    إذا (anchor) اكسر;
  }
  إذا (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  وإلا {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  أرجع 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#إذا !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(حجم(LUA_سان_FRMLEN)/حجم(حرف))


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
ساكن lua_سان adddigit (حرف *buff, unsigned n, lua_سان x) {
  lua_سان dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  صحيح d = (صحيح)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  أرجع x - dd;  /* return what is left */
}


ساكن صحيح num2straux (حرف *buff, unsigned sz, lua_سان x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  إذا (x != x || x == (lua_سان)HUGE_VAL || x == -(lua_سان)HUGE_VAL)
    أرجع l_sprintf(buff, sz, LUA_سان_FMT, (LUAI_UACNUMBER)x);
  وإلا إذا (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    أرجع l_sprintf(buff, sz, LUA_سان_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  وإلا {
    صحيح e;
    lua_سان m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    إذا (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    إذا (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      افعل {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } طالما (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    أرجع cast_int(n);
  }
}


ساكن صحيح lua_number2strx (lua_State *L, حرف *buff, unsigned sz,
                            ثابت حرف *fmt, lua_سان x) {
  صحيح n = num2straux(buff, sz, x);
  إذا (fmt[SIZELENMOD] == 'A') {
    صحيح i;
    لكل (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  وإلا إذا (l_unlikely(fmt[SIZELENMOD] != 'a'))
    أرجع luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  أرجع n;
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
#إذا !defined(L_FMTFLAGSF)

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
  طالما (len--) {
    إذا (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    وإلا إذا (iscntrl(cast_uchar(*s))) {
      حرف buff[10];
      إذا (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, حجم(buff), "\\%d", (صحيح)cast_uchar(*s));
      وإلا
        l_sprintf(buff, حجم(buff), "\\%03d", (صحيح)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    وإلا
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
ساكن صحيح quotefloat (lua_State *L, حرف *buff, lua_سان n) {
  ثابت حرف *s;  /* for the fixed representations */
  إذا (n == (lua_سان)HUGE_VAL)  /* inf? */
    s = "1e9999";
  وإلا إذا (n == -(lua_سان)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  وإلا إذا (n != n)  /* NaN? */
    s = "(0/0)";
  وإلا {  /* format number as hexadecimal */
    صحيح  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_سان_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    إذا (memchr(buff, '.', cast_uint(nb)) == عدم) {  /* no dot? */
      حرف point = lua_getlocaledecpoint();  /* try locale point */
      حرف *ppoint = (حرف *)memchr(buff, point, cast_uint(nb));
      إذا (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    أرجع nb;
  }
  /* for the fixed representations */
  أرجع l_sprintf(buff, MAX_ITEM, "%s", s);
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
      إذا (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      وإلا {  /* integers */
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
  إذا (isdigit(cast_uchar(*s))) {
    s++;
    إذا (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  أرجع s;
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
  إذا (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    إذا (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  إذا (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
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
  إذا (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * حجم(حرف));
  *(form + len) = '\0';
  أرجع strfrmt + len - 1;
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
  صحيح چوققا = lua_gettop(L);
  صحيح arg = 1;
  size_t sfl;
  ثابت حرف *strfrmt = luaL_checklstring(L, arg, &sfl);
  ثابت حرف *strfrmt_end = strfrmt+sfl;
  ثابت حرف *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  طالما (strfrmt < strfrmt_end) {
    إذا (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    وإلا إذا (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    وإلا { /* format item */
      حرف form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      حرف *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      صحيح nb = 0;  /* number of bytes in result */
      إذا (++arg > چوققا)
        أرجع luaL_argerror(L, arg, "no value");
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
          addlenmod(form, LUA_سان_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          اكسر;
        حالة 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        حالة 'e': حالة 'E': حالة 'g': حالة 'G': {
          lua_سان n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_سان_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          اكسر;
        }
        حالة 'p': {
          ثابت فراغ *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          إذا (p == عدم) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          اكسر;
        }
        حالة 'q': {
          إذا (form[2] != '\0')  /* modifiers? */
            أرجع luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          اكسر;
        }
        حالة 's': {
          size_t l;
          ثابت حرف *s = luaL_tolstring(L, arg, &l);
          إذا (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          وإلا {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            إذا (strchr(form, '.') == عدم && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            وإلا {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_چىقىرىش(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          اكسر;
        }
        افتراضي: {  /* also treat cases 'pnLlh' */
          أرجع luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  أرجع 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#إذا !defined(LUAL_PACKPADBYTE)
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
ساكن صحيح digit (صحيح c) { أرجع '0' <= c && c <= '9'; }

ساكن size_t getnum (ثابت حرف **fmt, size_t df) {
  إذا (!digit(**fmt))  /* no number? */
    أرجع df;  /* return default value */
  وإلا {
    size_t a = 0;
    افعل {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } طالما (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    أرجع a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ساكن unsigned getnumlimit (Header *h, ثابت حرف **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  إذا (l_unlikely((sz - 1u) >= MAXINTSIZE))
    أرجع cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  أرجع cast_uint(sz);
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
    حالة 'b': *size = حجم(حرف); أرجع Kint;
    حالة 'B': *size = حجم(حرف); أرجع Kuint;
    حالة 'h': *size = حجم(short); أرجع Kint;
    حالة 'H': *size = حجم(short); أرجع Kuint;
    حالة 'l': *size = حجم(long); أرجع Kint;
    حالة 'L': *size = حجم(long); أرجع Kuint;
    حالة 'j': *size = حجم(lua_Integer); أرجع Kint;
    حالة 'J': *size = حجم(lua_Integer); أرجع Kuint;
    حالة 'T': *size = حجم(size_t); أرجع Kuint;
    حالة 'f': *size = حجم(عائم); أرجع Kfloat;
    حالة 'n': *size = حجم(lua_سان); أرجع Knumber;
    حالة 'd': *size = حجم(مزدوج); أرجع Kdouble;
    حالة 'i': *size = getnumlimit(h, fmt, حجم(صحيح)); أرجع Kint;
    حالة 'I': *size = getnumlimit(h, fmt, حجم(صحيح)); أرجع Kuint;
    حالة 's': *size = getnumlimit(h, fmt, حجم(size_t)); أرجع Kstring;
    حالة 'c':
      *size = getnum(fmt, cast_sizet(-1));
      إذا (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      أرجع Kchar;
    حالة 'z': أرجع Kzstr;
    حالة 'x': *size = 1; أرجع Kpadding;
    حالة 'X': أرجع Kpaddalign;
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
  أرجع Knop;
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
  إذا (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    إذا (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  إذا (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  وإلا {
    إذا (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    إذا (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    وإلا {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  أرجع opt;
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
  لكل (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (حرف)(n & MC);
  }
  إذا (neg && size > SZINT) {  /* negative number need sign extension? */
    لكل (i = SZINT; i < size; i++)  /* correct extra bytes */
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
  إذا (islittle == nativeendian.little)
    memcpy(dest, src, size);
  وإلا {
    dest += size - 1;
    طالما (size-- != 0)
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
  طالما (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    طالما (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    بدل (opt) {
      حالة Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        إذا (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        اكسر;
      }
      حالة Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        إذا (size < SZINT)  /* need overflow check? */
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
        lua_سان f = luaL_checknumber(L, arg);  /* get argument */
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
        إذا (len < size) {  /* does it need padding? */
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
  أرجع 1;
}


ساكن صحيح str_packsize (lua_State *L) {
  Header h;
  ثابت حرف *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  طالما (*fmt != '\0') {
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
  أرجع 1;
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
  لكل (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned حرف)str[islittle ? i : size - 1 - i];
  }
  إذا (size < SZINT) {  /* real size smaller than lua_Integer? */
    إذا (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  وإلا إذا (size > SZINT) {  /* must check unread bytes */
    صحيح mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    لكل (i = limit; i < size; i++) {
      إذا (l_unlikely((unsigned حرف)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  أرجع (lua_Integer)res;
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
  طالما (*fmt != '\0') {
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
        lua_pushnumber(L, (lua_سان)f);
        اكسر;
      }
      حالة Knumber: {
        lua_سان f;
        copywithendian((حرف *)&f, data + pos, حجم(f), h.islittle);
        lua_pushnumber(L, f);
        اكسر;
      }
      حالة Kdouble: {
        مزدوج f;
        copywithendian((حرف *)&f, data + pos, حجم(f), h.islittle);
        lua_pushnumber(L, (lua_سان)f);
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
  أرجع n + 1;
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
  lua_چىقىرىش(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_چىقىرىش(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API صحيح luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  أرجع 1;
}

