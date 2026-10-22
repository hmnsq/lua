/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <float.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <locale.h>
#அந்தர்பாவ <math.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#எனில் !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


நிலையான முழு str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  திரும்பு 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
நிலையான size_t posrelatI (lua_Integer pos, size_t len) {
  எனில் (pos > 0)
    திரும்பு (size_t)pos;
  இல்லையெனில் எனில் (pos == 0)
    திரும்பு 1;
  இல்லையெனில் எனில் (pos < -(lua_Integer)len)  /* inverted comparison */
    திரும்பு 1;  /* clip to 1 */
  இல்லையெனில் திரும்பு len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
நிலையான size_t getendpos (lua_State *L, முழு arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  எனில் (pos > (lua_Integer)len)
    திரும்பு len;
  இல்லையெனில் எனில் (pos >= 0)
    திரும்பு (size_t)pos;
  இல்லையெனில் எனில் (pos < -(lua_Integer)len)
    திரும்பு 0;
  இல்லையெனில் திரும்பு len + (size_t)pos + 1;
}


நிலையான முழு str_sub (lua_State *L) {
  size_t l;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  எனில் (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  இல்லையெனில் lua_pushliteral(L, "");
  திரும்பு 1;
}


நிலையான முழு str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &l);
  எழுத்து *p = luaL_buffinitsize(L, &b, l);
  க்கு (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  திரும்பு 1;
}


நிலையான முழு str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &l);
  எழுத்து *p = luaL_buffinitsize(L, &b, l);
  க்கு (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  திரும்பு 1;
}


நிலையான முழு str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &l);
  எழுத்து *p = luaL_buffinitsize(L, &b, l);
  க்கு (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  திரும்பு 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
நிலையான முழு str_rep (lua_State *L) {
  size_t len, lsep;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  மாறிலி எழுத்து *sep = luaL_optlstring(L, 3, "", &lsep);
  எனில் (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  இல்லையெனில் எனில் (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    திரும்பு luaL_error(L, "resulting string too large");
  இல்லையெனில் {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    எழுத்து *p = luaL_buffinitsize(L, &b, totallen);
    போது (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * அளவு(எழுத்து)); p += len;
      எனில் (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * அளவு(எழுத்து)); p += lsep;
      }
    }
    memcpy(p, s, len * அளவு(எழுத்து));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  திரும்பு 1;
}


நிலையான முழு str_byte (lua_State *L) {
  size_t l;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  முழு n, i;
  எனில் (posi > pose) திரும்பு 0;  /* empty interval; return no values */
  எனில் (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    திரும்பு luaL_error(L, "string slice too long");
  n = (முழு)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  க்கு (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  திரும்பு n;
}


நிலையான முழு str_char (lua_State *L) {
  முழு n = lua_gettop(L);  /* number of arguments */
  முழு i;
  luaL_Buffer b;
  எழுத்து *p = luaL_buffinitsize(L, &b, cast_uint(n));
  க்கு (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  திரும்பு 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
கட்டமைப்பு str_Writer {
  முழு init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


நிலையான முழு writer (lua_State *L, மாறிலி வெறுமை *b, size_t size, வெறுமை *ud) {
  கட்டமைப்பு str_Writer *state = (கட்டமைப்பு str_Writer *)ud;
  எனில் (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  எனில் (b == வெற்று) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  இல்லையெனில்
    luaL_addlstring(&state->B, (மாறிலி எழுத்து *)b, size);
  திரும்பு 0;
}


நிலையான முழு str_dump (lua_State *L) {
  கட்டமைப்பு str_Writer state;
  முழு strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  திரும்பு 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#எனில் defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

நிலையான மாறிலி luaL_Reg stringmetamethods[] = {
  {"__index", வெற்று},  /* placeholder */
  {வெற்று, வெற்று}
};

#இல்லையெனில்		/* }{ */

நிலையான முழு tonum (lua_State *L, முழு arg) {
  எனில் (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    திரும்பு 1;
  }
  இல்லையெனில் {  /* check whether it is a numerical string */
    size_t len;
    மாறிலி எழுத்து *s = lua_tolstring(L, arg, &len);
    திரும்பு (s != வெற்று && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
நிலையான வெறுமை trymt (lua_State *L, மாறிலி எழுத்து *mtkey, மாறிலி எழுத்து *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  எனில் (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_செருகு(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


நிலையான முழு arith (lua_State *L, முழு op, மாறிலி எழுத்து *mtname) {
  எனில் (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  இல்லையெனில்
    trymt(L, mtname, mtname + 2);
  திரும்பு 1;
}


நிலையான முழு arith_add (lua_State *L) {
  திரும்பு arith(L, LUA_OPADD, "__add");
}

நிலையான முழு arith_sub (lua_State *L) {
  திரும்பு arith(L, LUA_OPSUB, "__sub");
}

நிலையான முழு arith_mul (lua_State *L) {
  திரும்பு arith(L, LUA_OPMUL, "__mul");
}

நிலையான முழு arith_mod (lua_State *L) {
  திரும்பு arith(L, LUA_OPMOD, "__mod");
}

நிலையான முழு arith_pow (lua_State *L) {
  திரும்பு arith(L, LUA_OPPOW, "__pow");
}

நிலையான முழு arith_div (lua_State *L) {
  திரும்பு arith(L, LUA_OPDIV, "__div");
}

நிலையான முழு arith_idiv (lua_State *L) {
  திரும்பு arith(L, LUA_OPIDIV, "__idiv");
}

நிலையான முழு arith_unm (lua_State *L) {
  திரும்பு arith(L, LUA_OPUNM, "__unm");
}


நிலையான மாறிலி luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", வெற்று},  /* placeholder */
  {வெற்று, வெற்று}
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


வகைவரை கட்டமைப்பு MatchState {
  மாறிலி எழுத்து *src_init;  /* init of source string */
  மாறிலி எழுத்து *src_end;  /* end ('\0') of source string */
  மாறிலி எழுத்து *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  முழு matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  முழு level;  /* total number of captures (finished or unfinished) */
  கட்டமைப்பு {
    மாறிலி எழுத்து *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
நிலையான மாறிலி எழுத்து *match (MatchState *ms, மாறிலி எழுத்து *s, மாறிலி எழுத்து *p);


/* maximum recursion depth for 'match' */
#எனில் !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


நிலையான முழு check_capture (MatchState *ms, முழு l) {
  l -= '1';
  எனில் (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    திரும்பு luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  திரும்பு l;
}


நிலையான முழு capture_to_close (MatchState *ms) {
  முழு level = ms->level;
  க்கு (level--; level>=0; level--)
    எனில் (ms->capture[level].len == CAP_UNFINISHED) திரும்பு level;
  திரும்பு luaL_error(ms->L, "invalid pattern capture");
}


நிலையான மாறிலி எழுத்து *classend (MatchState *ms, மாறிலி எழுத்து *p) {
  தேர்வு (*p++) {
    நிலை L_ESC: {
      எனில் (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      திரும்பு p+1;
    }
    நிலை '[': {
      எனில் (*p == '^') p++;
      செய் {  /* look for a ']' */
        எனில் (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        எனில் (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } போது (*p != ']');
      திரும்பு p+1;
    }
    இயல்பு: {
      திரும்பு p;
    }
  }
}


நிலையான முழு match_class (முழு c, முழு cl) {
  முழு res;
  தேர்வு (tolower(cl)) {
    நிலை 'a' : res = isalpha(c); நிறுத்து;
    நிலை 'c' : res = iscntrl(c); நிறுத்து;
    நிலை 'd' : res = isdigit(c); நிறுத்து;
    நிலை 'g' : res = isgraph(c); நிறுத்து;
    நிலை 'l' : res = islower(c); நிறுத்து;
    நிலை 'p' : res = ispunct(c); நிறுத்து;
    நிலை 's' : res = isspace(c); நிறுத்து;
    நிலை 'u' : res = isupper(c); நிறுத்து;
    நிலை 'w' : res = isalnum(c); நிறுத்து;
    நிலை 'x' : res = isxdigit(c); நிறுத்து;
    நிலை 'z' : res = (c == 0); நிறுத்து;  /* deprecated option */
    இயல்பு: திரும்பு (cl == c);
  }
  திரும்பு (islower(cl) ? res : !res);
}


நிலையான முழு matchbracketclass (முழு c, மாறிலி எழுத்து *p, மாறிலி எழுத்து *ec) {
  முழு sig = 1;
  எனில் (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  போது (++p < ec) {
    எனில் (*p == L_ESC) {
      p++;
      எனில் (match_class(c, cast_uchar(*p)))
        திரும்பு sig;
    }
    இல்லையெனில் எனில் ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      எனில் (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        திரும்பு sig;
    }
    இல்லையெனில் எனில் (cast_uchar(*p) == c) திரும்பு sig;
  }
  திரும்பு !sig;
}


நிலையான முழு singlematch (MatchState *ms, மாறிலி எழுத்து *s, மாறிலி எழுத்து *p,
                        மாறிலி எழுத்து *ep) {
  எனில் (s >= ms->src_end)
    திரும்பு 0;
  இல்லையெனில் {
    முழு c = cast_uchar(*s);
    தேர்வு (*p) {
      நிலை '.': திரும்பு 1;  /* matches any char */
      நிலை L_ESC: திரும்பு match_class(c, cast_uchar(*(p+1)));
      நிலை '[': திரும்பு matchbracketclass(c, p, ep-1);
      இயல்பு:  திரும்பு (cast_uchar(*p) == c);
    }
  }
}


நிலையான மாறிலி எழுத்து *matchbalance (MatchState *ms, மாறிலி எழுத்து *s,
                                   மாறிலி எழுத்து *p) {
  எனில் (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  எனில் (*s != *p) திரும்பு வெற்று;
  இல்லையெனில் {
    முழு b = *p;
    முழு e = *(p+1);
    முழு cont = 1;
    போது (++s < ms->src_end) {
      எனில் (*s == e) {
        எனில் (--cont == 0) திரும்பு s+1;
      }
      இல்லையெனில் எனில் (*s == b) cont++;
    }
  }
  திரும்பு வெற்று;  /* string ends out of balance */
}


நிலையான மாறிலி எழுத்து *max_expand (MatchState *ms, மாறிலி எழுத்து *s,
                                 மாறிலி எழுத்து *p, மாறிலி எழுத்து *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  போது (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  போது (i>=0) {
    மாறிலி எழுத்து *res = match(ms, (s+i), ep+1);
    எனில் (res) திரும்பு res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  திரும்பு வெற்று;
}


நிலையான மாறிலி எழுத்து *min_expand (MatchState *ms, மாறிலி எழுத்து *s,
                                 மாறிலி எழுத்து *p, மாறிலி எழுத்து *ep) {
  க்கு (;;) {
    மாறிலி எழுத்து *res = match(ms, s, ep+1);
    எனில் (res != வெற்று)
      திரும்பு res;
    இல்லையெனில் எனில் (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    இல்லையெனில் திரும்பு வெற்று;
  }
}


நிலையான மாறிலி எழுத்து *start_capture (MatchState *ms, மாறிலி எழுத்து *s,
                                    மாறிலி எழுத்து *p, முழு what) {
  மாறிலி எழுத்து *res;
  முழு level = ms->level;
  எனில் (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  எனில் ((res=match(ms, s, p)) == வெற்று)  /* match failed? */
    ms->level--;  /* undo capture */
  திரும்பு res;
}


நிலையான மாறிலி எழுத்து *end_capture (MatchState *ms, மாறிலி எழுத்து *s,
                                  மாறிலி எழுத்து *p) {
  முழு l = capture_to_close(ms);
  மாறிலி எழுத்து *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  எனில் ((res = match(ms, s, p)) == வெற்று)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  திரும்பு res;
}


நிலையான மாறிலி எழுத்து *match_capture (MatchState *ms, மாறிலி எழுத்து *s, முழு l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  எனில் ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    திரும்பு s+len;
  இல்லையெனில் திரும்பு வெற்று;
}


நிலையான மாறிலி எழுத்து *match (MatchState *ms, மாறிலி எழுத்து *s, மாறிலி எழுத்து *p) {
  எனில் (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  எனில் (p != ms->p_end) {  /* end of pattern? */
    தேர்வு (*p) {
      நிலை '(': {  /* start capture */
        எனில் (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        இல்லையெனில்
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        நிறுத்து;
      }
      நிலை ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        நிறுத்து;
      }
      நிலை '$': {
        எனில் ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          செல் dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : வெற்று;  /* check end of string */
        நிறுத்து;
      }
      நிலை L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        தேர்வு (*(p + 1)) {
          நிலை 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            எனில் (s != வெற்று) {
              p += 4; செல் init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            நிறுத்து;
          }
          நிலை 'f': {  /* frontier? */
            மாறிலி எழுத்து *ep; எழுத்து previous;
            p += 2;
            எனில் (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            எனில் (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; செல் init;  /* return match(ms, s, ep); */
            }
            s = வெற்று;  /* match failed */
            நிறுத்து;
          }
          நிலை '0': நிலை '1': நிலை '2': நிலை '3':
          நிலை '4': நிலை '5': நிலை '6': நிலை '7':
          நிலை '8': நிலை '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            எனில் (s != வெற்று) {
              p += 2; செல் init;  /* return match(ms, s, p + 2) */
            }
            நிறுத்து;
          }
          இயல்பு: செல் dflt;
        }
        நிறுத்து;
      }
      இயல்பு: dflt: {  /* pattern class plus optional suffix */
        மாறிலி எழுத்து *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        எனில் (!singlematch(ms, s, p, ep)) {
          எனில் (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; செல் init;  /* return match(ms, s, ep + 1); */
          }
          இல்லையெனில்  /* '+' or no suffix */
            s = வெற்று;  /* fail */
        }
        இல்லையெனில் {  /* matched once */
          தேர்வு (*ep) {  /* handle optional suffix */
            நிலை '?': {  /* optional */
              மாறிலி எழுத்து *res;
              எனில் ((res = match(ms, s + 1, ep + 1)) != வெற்று)
                s = res;
              இல்லையெனில் {
                p = ep + 1; செல் init;  /* else return match(ms, s, ep + 1); */
              }
              நிறுத்து;
            }
            நிலை '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            நிலை '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              நிறுத்து;
            நிலை '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              நிறுத்து;
            இயல்பு:  /* no suffix */
              s++; p = ep; செல் init;  /* return match(ms, s + 1, ep); */
          }
        }
        நிறுத்து;
      }
    }
  }
  ms->matchdepth++;
  திரும்பு s;
}



நிலையான மாறிலி எழுத்து *lmemfind (மாறிலி எழுத்து *s1, size_t l1,
                               மாறிலி எழுத்து *s2, size_t l2) {
  எனில் (l2 == 0) திரும்பு s1;  /* empty strings are everywhere */
  இல்லையெனில் எனில் (l2 > l1) திரும்பு வெற்று;  /* avoids a negative 'l1' */
  இல்லையெனில் {
    மாறிலி எழுத்து *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    போது (l1 > 0 && (init = (மாறிலி எழுத்து *)memchr(s1, *s2, l1)) != வெற்று) {
      init++;   /* 1st char is already checked */
      எனில் (memcmp(init, s2+1, l2) == 0)
        திரும்பு init-1;
      இல்லையெனில் {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    திரும்பு வெற்று;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
நிலையான ptrdiff_t get_onecapture (MatchState *ms, முழு i, மாறிலி எழுத்து *s,
                              மாறிலி எழுத்து *e, மாறிலி எழுத்து **cap) {
  எனில் (i >= ms->level) {
    எனில் (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    திரும்பு (e - s);
  }
  இல்லையெனில் {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    எனில் (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    இல்லையெனில் எனில் (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    திரும்பு capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
நிலையான வெறுமை தள்ளு_onecapture (MatchState *ms, முழு i, மாறிலி எழுத்து *s,
                                                    மாறிலி எழுத்து *e) {
  மாறிலி எழுத்து *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  எனில் (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


நிலையான முழு தள்ளு_captures (MatchState *ms, மாறிலி எழுத்து *s, மாறிலி எழுத்து *e) {
  முழு i;
  முழு nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  க்கு (i = 0; i < nlevels; i++)
    தள்ளு_onecapture(ms, i, s, e);
  திரும்பு nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
நிலையான முழு nospecials (மாறிலி எழுத்து *p, size_t l) {
  size_t upto = 0;
  செய் {
    எனில் (strpbrk(p + upto, SPECIALS))
      திரும்பு 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } போது (upto <= l);
  திரும்பு 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
நிலையான வெறுமை prepstate (MatchState *ms, lua_State *L,
                       மாறிலி எழுத்து *s, size_t ls, மாறிலி எழுத்து *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
நிலையான வெறுமை reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


நிலையான முழு str_find_aux (lua_State *L, முழு find) {
  size_t ls, lp;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &ls);
  மாறிலி எழுத்து *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  எனில் (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    திரும்பு 1;
  }
  /* explicit request or no special characters? */
  எனில் (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    மாறிலி எழுத்து *s2 = lmemfind(s + init, ls - init, p, lp);
    எனில் (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      திரும்பு 2;
    }
  }
  இல்லையெனில் {
    MatchState ms;
    மாறிலி எழுத்து *s1 = s + init;
    முழு anchor = (*p == '^');
    எனில் (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    செய் {
      மாறிலி எழுத்து *res;
      reprepstate(&ms);
      எனில் ((res=match(&ms, s1, p)) != வெற்று) {
        எனில் (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          திரும்பு தள்ளு_captures(&ms, வெற்று, 0) + 2;
        }
        இல்லையெனில்
          திரும்பு தள்ளு_captures(&ms, s1, res);
      }
    } போது (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  திரும்பு 1;
}


நிலையான முழு str_find (lua_State *L) {
  திரும்பு str_find_aux(L, 1);
}


நிலையான முழு str_match (lua_State *L) {
  திரும்பு str_find_aux(L, 0);
}


/* state for 'gmatch' */
வகைவரை கட்டமைப்பு GMatchState {
  மாறிலி எழுத்து *src;  /* current position */
  மாறிலி எழுத்து *p;  /* pattern */
  மாறிலி எழுத்து *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


நிலையான முழு gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  மாறிலி எழுத்து *src;
  gm->ms.L = L;
  க்கு (src = gm->src; src <= gm->ms.src_end; src++) {
    மாறிலி எழுத்து *e;
    reprepstate(&gm->ms);
    எனில் ((e = match(&gm->ms, src, gm->p)) != வெற்று && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      திரும்பு தள்ளு_captures(&gm->ms, src, e);
    }
  }
  திரும்பு 0;  /* not found */
}


நிலையான முழு gmatch (lua_State *L) {
  size_t ls, lp;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &ls);
  மாறிலி எழுத்து *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, அளவு(GMatchState), 0);
  எனில் (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = வெற்று;
  lua_pushcclosure(L, gmatch_aux, 3);
  திரும்பு 1;
}


நிலையான வெறுமை add_s (MatchState *ms, luaL_Buffer *b, மாறிலி எழுத்து *s,
                                                   மாறிலி எழுத்து *e) {
  size_t l;
  lua_State *L = ms->L;
  மாறிலி எழுத்து *news = lua_tolstring(L, 3, &l);
  மாறிலி எழுத்து *p;
  போது ((p = (எழுத்து *)memchr(news, L_ESC, l)) != வெற்று) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    எனில் (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    இல்லையெனில் எனில் (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    இல்லையெனில் எனில் (isdigit(cast_uchar(*p))) {  /* '%n' */
      மாறிலி எழுத்து *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      எனில் (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      இல்லையெனில்
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    இல்லையெனில்
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
நிலையான முழு add_மதிப்பு (MatchState *ms, luaL_Buffer *b, மாறிலி எழுத்து *s,
                                      மாறிலி எழுத்து *e, முழு tr) {
  lua_State *L = ms->L;
  தேர்வு (tr) {
    நிலை LUA_TFUNCTION: {  /* call the function */
      முழு n;
      lua_pushvalue(L, 3);  /* push the function */
      n = தள்ளு_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      நிறுத்து;
    }
    நிலை LUA_TTABLE: {  /* index the table */
      தள்ளு_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      நிறுத்து;
    }
    இயல்பு: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      திரும்பு 1;  /* something changed */
    }
  }
  எனில் (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_எடு(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    திரும்பு 0;  /* no changes */
  }
  இல்லையெனில் எனில் (l_unlikely(!lua_isstring(L, -1)))
    திரும்பு luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  இல்லையெனில் {
    luaL_addvalue(b);  /* add result to accumulator */
    திரும்பு 1;  /* something changed */
  }
}


நிலையான முழு str_gsub (lua_State *L) {
  size_t srcl, lp;
  மாறிலி எழுத்து *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  மாறிலி எழுத்து *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  மாறிலி எழுத்து *lastmatch = வெற்று;  /* end of last match */
  முழு tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  முழு anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  முழு changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  எனில் (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  போது (n < max_s) {
    மாறிலி எழுத்து *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    எனில் ((e = match(&ms, src, p)) != வெற்று && e != lastmatch) {  /* match? */
      n++;
      changed = add_மதிப்பு(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    இல்லையெனில் எனில் (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    இல்லையெனில் நிறுத்து;  /* end of subject */
    எனில் (anchor) நிறுத்து;
  }
  எனில் (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  இல்லையெனில் {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  திரும்பு 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#எனில் !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(அளவு(LUA_எண்_FRMLEN)/அளவு(எழுத்து))


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
நிலையான lua_எண் adddigit (எழுத்து *buff, unsigned n, lua_எண் x) {
  lua_எண் dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  முழு d = (முழு)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  திரும்பு x - dd;  /* return what is left */
}


நிலையான முழு num2straux (எழுத்து *buff, unsigned sz, lua_எண் x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  எனில் (x != x || x == (lua_எண்)HUGE_VAL || x == -(lua_எண்)HUGE_VAL)
    திரும்பு l_sprintf(buff, sz, LUA_எண்_FMT, (LUAI_UACNUMBER)x);
  இல்லையெனில் எனில் (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    திரும்பு l_sprintf(buff, sz, LUA_எண்_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  இல்லையெனில் {
    முழு e;
    lua_எண் m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    எனில் (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    எனில் (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      செய் {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } போது (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    திரும்பு cast_int(n);
  }
}


நிலையான முழு lua_number2strx (lua_State *L, எழுத்து *buff, unsigned sz,
                            மாறிலி எழுத்து *fmt, lua_எண் x) {
  முழு n = num2straux(buff, sz, x);
  எனில் (fmt[SIZELENMOD] == 'A') {
    முழு i;
    க்கு (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  இல்லையெனில் எனில் (l_unlikely(fmt[SIZELENMOD] != 'a'))
    திரும்பு luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  திரும்பு n;
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
#எனில் !defined(L_FMTFLAGSF)

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


நிலையான வெறுமை addquoted (luaL_Buffer *b, மாறிலி எழுத்து *s, size_t len) {
  luaL_addchar(b, '"');
  போது (len--) {
    எனில் (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    இல்லையெனில் எனில் (iscntrl(cast_uchar(*s))) {
      எழுத்து buff[10];
      எனில் (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, அளவு(buff), "\\%d", (முழு)cast_uchar(*s));
      இல்லையெனில்
        l_sprintf(buff, அளவு(buff), "\\%03d", (முழு)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    இல்லையெனில்
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
நிலையான முழு quotefloat (lua_State *L, எழுத்து *buff, lua_எண் n) {
  மாறிலி எழுத்து *s;  /* for the fixed representations */
  எனில் (n == (lua_எண்)HUGE_VAL)  /* inf? */
    s = "1e9999";
  இல்லையெனில் எனில் (n == -(lua_எண்)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  இல்லையெனில் எனில் (n != n)  /* NaN? */
    s = "(0/0)";
  இல்லையெனில் {  /* format number as hexadecimal */
    முழு  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_எண்_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    எனில் (memchr(buff, '.', cast_uint(nb)) == வெற்று) {  /* no dot? */
      எழுத்து point = lua_getlocaledecpoint();  /* try locale point */
      எழுத்து *ppoint = (எழுத்து *)memchr(buff, point, cast_uint(nb));
      எனில் (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    திரும்பு nb;
  }
  /* for the fixed representations */
  திரும்பு l_sprintf(buff, MAX_ITEM, "%s", s);
}


நிலையான வெறுமை addliteral (lua_State *L, luaL_Buffer *b, முழு arg) {
  தேர்வு (lua_type(L, arg)) {
    நிலை LUA_TSTRING: {
      size_t len;
      மாறிலி எழுத்து *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      நிறுத்து;
    }
    நிலை LUA_TNUMBER: {
      எழுத்து *buff = luaL_prepbuffsize(b, MAX_ITEM);
      முழு nb;
      எனில் (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      இல்லையெனில் {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        மாறிலி எழுத்து *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      நிறுத்து;
    }
    நிலை LUA_TNIL: நிலை LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, வெற்று);
      luaL_addvalue(b);
      நிறுத்து;
    }
    இயல்பு: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


நிலையான மாறிலி எழுத்து *get2digits (மாறிலி எழுத்து *s) {
  எனில் (isdigit(cast_uchar(*s))) {
    s++;
    எனில் (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  திரும்பு s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
நிலையான வெறுமை checkformat (lua_State *L, மாறிலி எழுத்து *form, மாறிலி எழுத்து *flags,
                                       முழு precision) {
  மாறிலி எழுத்து *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  எனில் (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    எனில் (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  எனில் (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
நிலையான மாறிலி எழுத்து *getformat (lua_State *L, மாறிலி எழுத்து *strfrmt,
                                            எழுத்து *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  எனில் (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * அளவு(எழுத்து));
  *(form + len) = '\0';
  திரும்பு strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
நிலையான வெறுமை addlenmod (எழுத்து *form, மாறிலி எழுத்து *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  எழுத்து spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


நிலையான முழு str_format (lua_State *L) {
  முழு உச்சி = lua_gettop(L);
  முழு arg = 1;
  size_t sfl;
  மாறிலி எழுத்து *strfrmt = luaL_checklstring(L, arg, &sfl);
  மாறிலி எழுத்து *strfrmt_end = strfrmt+sfl;
  மாறிலி எழுத்து *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  போது (strfrmt < strfrmt_end) {
    எனில் (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    இல்லையெனில் எனில் (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    இல்லையெனில் { /* format item */
      எழுத்து form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      எழுத்து *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      முழு nb = 0;  /* number of bytes in result */
      எனில் (++arg > உச்சி)
        திரும்பு luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      தேர்வு (*strfrmt++) {
        நிலை 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (முழு)luaL_checkinteger(L, arg));
          நிறுத்து;
        }
        நிலை 'd': நிலை 'i':
          flags = L_FMTFLAGSI;
          செல் intcase;
        நிலை 'u':
          flags = L_FMTFLAGSU;
          செல் intcase;
        நிலை 'o': நிலை 'x': நிலை 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          நிறுத்து;
        }
        நிலை 'a': நிலை 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_எண்_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          நிறுத்து;
        நிலை 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        நிலை 'e': நிலை 'E': நிலை 'g': நிலை 'G': {
          lua_எண் n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_எண்_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          நிறுத்து;
        }
        நிலை 'p': {
          மாறிலி வெறுமை *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          எனில் (p == வெற்று) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          நிறுத்து;
        }
        நிலை 'q': {
          எனில் (form[2] != '\0')  /* modifiers? */
            திரும்பு luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          நிறுத்து;
        }
        நிலை 's': {
          size_t l;
          மாறிலி எழுத்து *s = luaL_tolstring(L, arg, &l);
          எனில் (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          இல்லையெனில் {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            எனில் (strchr(form, '.') == வெற்று && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            இல்லையெனில் {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_எடு(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          நிறுத்து;
        }
        இயல்பு: {  /* also treat cases 'pnLlh' */
          திரும்பு luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  திரும்பு 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#எனில் !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((முழு)அளவு(lua_Integer))


/* dummy union to get native endianness */
நிலையான மாறிலி ஒன்றிப்பு {
  முழு dummy;
  எழுத்து little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
வகைவரை கட்டமைப்பு Header {
  lua_State *L;
  முழு islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
வகைவரை எண்ணிக்கை KOption {
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
நிலையான முழு digit (முழு c) { திரும்பு '0' <= c && c <= '9'; }

நிலையான size_t getnum (மாறிலி எழுத்து **fmt, size_t df) {
  எனில் (!digit(**fmt))  /* no number? */
    திரும்பு df;  /* return default value */
  இல்லையெனில் {
    size_t a = 0;
    செய் {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } போது (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    திரும்பு a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
நிலையான unsigned getnumlimit (Header *h, மாறிலி எழுத்து **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  எனில் (l_unlikely((sz - 1u) >= MAXINTSIZE))
    திரும்பு cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  திரும்பு cast_uint(sz);
}


/*
** Initialize Header
*/
நிலையான வெறுமை initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
நிலையான KOption getoption (Header *h, மாறிலி எழுத்து **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  கட்டமைப்பு cD { எழுத்து c; ஒன்றிப்பு { LUAI_MAXALIGN; } u; };
  முழு opt = *((*fmt)++);
  *size = 0;  /* default */
  தேர்வு (opt) {
    நிலை 'b': *size = அளவு(எழுத்து); திரும்பு Kint;
    நிலை 'B': *size = அளவு(எழுத்து); திரும்பு Kuint;
    நிலை 'h': *size = அளவு(short); திரும்பு Kint;
    நிலை 'H': *size = அளவு(short); திரும்பு Kuint;
    நிலை 'l': *size = அளவு(long); திரும்பு Kint;
    நிலை 'L': *size = அளவு(long); திரும்பு Kuint;
    நிலை 'j': *size = அளவு(lua_Integer); திரும்பு Kint;
    நிலை 'J': *size = அளவு(lua_Integer); திரும்பு Kuint;
    நிலை 'T': *size = அளவு(size_t); திரும்பு Kuint;
    நிலை 'f': *size = அளவு(தசம); திரும்பு Kfloat;
    நிலை 'n': *size = அளவு(lua_எண்); திரும்பு Knumber;
    நிலை 'd': *size = அளவு(இரட்டை); திரும்பு Kdouble;
    நிலை 'i': *size = getnumlimit(h, fmt, அளவு(முழு)); திரும்பு Kint;
    நிலை 'I': *size = getnumlimit(h, fmt, அளவு(முழு)); திரும்பு Kuint;
    நிலை 's': *size = getnumlimit(h, fmt, அளவு(size_t)); திரும்பு Kstring;
    நிலை 'c':
      *size = getnum(fmt, cast_sizet(-1));
      எனில் (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      திரும்பு Kchar;
    நிலை 'z': திரும்பு Kzstr;
    நிலை 'x': *size = 1; திரும்பு Kpadding;
    நிலை 'X': திரும்பு Kpaddalign;
    நிலை ' ': நிறுத்து;
    நிலை '<': h->islittle = 1; நிறுத்து;
    நிலை '>': h->islittle = 0; நிறுத்து;
    நிலை '=': h->islittle = nativeendian.little; நிறுத்து;
    நிலை '!': {
      மாறிலி size_t maxalign = offsetof(கட்டமைப்பு cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      நிறுத்து;
    }
    இயல்பு: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  திரும்பு Knop;
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
நிலையான KOption getdetails (Header *h, size_t totalsize, மாறிலி எழுத்து **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  எனில் (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    எனில் (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  எனில் (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  இல்லையெனில் {
    எனில் (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    எனில் (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    இல்லையெனில் {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  திரும்பு opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
நிலையான வெறுமை packint (luaL_Buffer *b, lua_Unsigned n,
                     முழு islittle, unsigned size, முழு neg) {
  எழுத்து *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (எழுத்து)(n & MC);  /* first byte */
  க்கு (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (எழுத்து)(n & MC);
  }
  எனில் (neg && size > SZINT) {  /* negative number need sign extension? */
    க்கு (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (எழுத்து)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
நிலையான வெறுமை copywithendian (எழுத்து *dest, மாறிலி எழுத்து *src,
                            unsigned size, முழு islittle) {
  எனில் (islittle == nativeendian.little)
    memcpy(dest, src, size);
  இல்லையெனில் {
    dest += size - 1;
    போது (size-- != 0)
      *(dest--) = *(src++);
  }
}


நிலையான முழு str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  மாறிலி எழுத்து *fmt = luaL_checkstring(L, 1);  /* format string */
  முழு arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  போது (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    போது (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    தேர்வு (opt) {
      நிலை Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        எனில் (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        நிறுத்து;
      }
      நிலை Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        எனில் (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        நிறுத்து;
      }
      நிலை Kfloat: {  /* C float */
        தசம f = (தசம)luaL_checknumber(L, arg);  /* get argument */
        எழுத்து *buff = luaL_prepbuffsize(&b, அளவு(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (எழுத்து *)&f, அளவு(f), h.islittle);
        luaL_addsize(&b, size);
        நிறுத்து;
      }
      நிலை Knumber: {  /* Lua float */
        lua_எண் f = luaL_checknumber(L, arg);  /* get argument */
        எழுத்து *buff = luaL_prepbuffsize(&b, அளவு(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (எழுத்து *)&f, அளவு(f), h.islittle);
        luaL_addsize(&b, size);
        நிறுத்து;
      }
      நிலை Kdouble: {  /* C double */
        இரட்டை f = (இரட்டை)luaL_checknumber(L, arg);  /* get argument */
        எழுத்து *buff = luaL_prepbuffsize(&b, அளவு(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (எழுத்து *)&f, அளவு(f), h.islittle);
        luaL_addsize(&b, size);
        நிறுத்து;
      }
      நிலை Kchar: {  /* fixed-size string */
        size_t len;
        மாறிலி எழுத்து *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        எனில் (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          எழுத்து *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        நிறுத்து;
      }
      நிலை Kstring: {  /* strings with length count */
        size_t len;
        மாறிலி எழுத்து *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= அளவு(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        நிறுத்து;
      }
      நிலை Kzstr: {  /* zero-terminated string */
        size_t len;
        மாறிலி எழுத்து *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        நிறுத்து;
      }
      நிலை Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      நிலை Kpaddalign: நிலை Knop:
        arg--;  /* undo increment */
        நிறுத்து;
    }
  }
  luaL_pushresult(&b);
  திரும்பு 1;
}


நிலையான முழு str_packsize (lua_State *L) {
  Header h;
  மாறிலி எழுத்து *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  போது (*fmt != '\0') {
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
  திரும்பு 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
நிலையான lua_Integer unpackint (lua_State *L, மாறிலி எழுத்து *str,
                              முழு islittle, முழு size, முழு issigned) {
  lua_Unsigned res = 0;
  முழு i;
  முழு limit = (size  <= SZINT) ? size : SZINT;
  க்கு (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned எழுத்து)str[islittle ? i : size - 1 - i];
  }
  எனில் (size < SZINT) {  /* real size smaller than lua_Integer? */
    எனில் (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  இல்லையெனில் எனில் (size > SZINT) {  /* must check unread bytes */
    முழு mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    க்கு (i = limit; i < size; i++) {
      எனில் (l_unlikely((unsigned எழுத்து)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  திரும்பு (lua_Integer)res;
}


நிலையான முழு str_unpack (lua_State *L) {
  Header h;
  மாறிலி எழுத்து *fmt = luaL_checkstring(L, 1);
  size_t ld;
  மாறிலி எழுத்து *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  முழு n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  போது (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    தேர்வு (opt) {
      நிலை Kint:
      நிலை Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        நிறுத்து;
      }
      நிலை Kfloat: {
        தசம f;
        copywithendian((எழுத்து *)&f, data + pos, அளவு(f), h.islittle);
        lua_pushnumber(L, (lua_எண்)f);
        நிறுத்து;
      }
      நிலை Knumber: {
        lua_எண் f;
        copywithendian((எழுத்து *)&f, data + pos, அளவு(f), h.islittle);
        lua_pushnumber(L, f);
        நிறுத்து;
      }
      நிலை Kdouble: {
        இரட்டை f;
        copywithendian((எழுத்து *)&f, data + pos, அளவு(f), h.islittle);
        lua_pushnumber(L, (lua_எண்)f);
        நிறுத்து;
      }
      நிலை Kchar: {
        lua_pushlstring(L, data + pos, size);
        நிறுத்து;
      }
      நிலை Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        நிறுத்து;
      }
      நிலை Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        நிறுத்து;
      }
      நிலை Kpaddalign: நிலை Kpadding: நிலை Knop:
        n--;  /* undo increment */
        நிறுத்து;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  திரும்பு n + 1;
}

/* }====================================================== */


நிலையான மாறிலி luaL_Reg strlib[] = {
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
  {வெற்று, வெற்று}
};


நிலையான வெறுமை createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_எடு(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_எடு(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API முழு luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  திரும்பு 1;
}

