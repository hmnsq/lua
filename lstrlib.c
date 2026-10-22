/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <float.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <locale.h>
#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#ଯଦି !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  ଫେରନ୍ତୁ 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
ସ୍ଥିର size_t posrelatI (lua_Integer pos, size_t len) {
  ଯଦି (pos > 0)
    ଫେରନ୍ତୁ (size_t)pos;
  ଅନ୍ୟଥା ଯଦି (pos == 0)
    ଫେରନ୍ତୁ 1;
  ଅନ୍ୟଥା ଯଦି (pos < -(lua_Integer)len)  /* inverted comparison */
    ଫେରନ୍ତୁ 1;  /* clip to 1 */
  ଅନ୍ୟଥା ଫେରନ୍ତୁ len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ସ୍ଥିର size_t getendpos (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  ଯଦି (pos > (lua_Integer)len)
    ଫେରନ୍ତୁ len;
  ଅନ୍ୟଥା ଯଦି (pos >= 0)
    ଫେରନ୍ତୁ (size_t)pos;
  ଅନ୍ୟଥା ଯଦି (pos < -(lua_Integer)len)
    ଫେରନ୍ତୁ 0;
  ଅନ୍ୟଥା ଫେରନ୍ତୁ len + (size_t)pos + 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_sub (lua_State *L) {
  size_t l;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  ଯଦି (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  ଅନ୍ୟଥା lua_pushliteral(L, "");
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &l);
  ଅକ୍ଷର *p = luaL_buffinitsize(L, &b, l);
  ପାଇଁ (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &l);
  ଅକ୍ଷର *p = luaL_buffinitsize(L, &b, l);
  ପାଇଁ (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &l);
  ଅକ୍ଷର *p = luaL_buffinitsize(L, &b, l);
  ପାଇଁ (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ଫେରନ୍ତୁ 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_rep (lua_State *L) {
  size_t len, lsep;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ଅଚଳ ଅକ୍ଷର *sep = luaL_optlstring(L, 3, "", &lsep);
  ଯଦି (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  ଅନ୍ୟଥା ଯଦି (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    ଫେରନ୍ତୁ luaL_error(L, "resulting string too large");
  ଅନ୍ୟଥା {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    ଅକ୍ଷର *p = luaL_buffinitsize(L, &b, totallen);
    ଯେତେବେଳେ (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * ଆକାର(ଅକ୍ଷର)); p += len;
      ଯଦି (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * ଆକାର(ଅକ୍ଷର)); p += lsep;
      }
    }
    memcpy(p, s, len * ଆକାର(ଅକ୍ଷର));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_byte (lua_State *L) {
  size_t l;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, i;
  ଯଦି (posi > pose) ଫେରନ୍ତୁ 0;  /* empty interval; return no values */
  ଯଦି (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    ଫେରନ୍ତୁ luaL_error(L, "string slice too long");
  n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  ପାଇଁ (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  ଫେରନ୍ତୁ n;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_char (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);  /* number of arguments */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  luaL_Buffer b;
  ଅକ୍ଷର *p = luaL_buffinitsize(L, &b, cast_uint(n));
  ପାଇଁ (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  ଫେରନ୍ତୁ 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
ସଂରଚନା str_Writer {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା writer (lua_State *L, ଅଚଳ ଶୂନ୍ୟ *b, size_t size, ଶୂନ୍ୟ *ud) {
  ସଂରଚନା str_Writer *state = (ସଂରଚନା str_Writer *)ud;
  ଯଦି (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  ଯଦି (b == ଖାଲି) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  ଅନ୍ୟଥା
    luaL_addlstring(&state->B, (ଅଚଳ ଅକ୍ଷର *)b, size);
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_dump (lua_State *L) {
  ସଂରଚନା str_Writer state;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  ଫେରନ୍ତୁ 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#ଯଦି defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ସ୍ଥିର ଅଚଳ luaL_Reg stringmetamethods[] = {
  {"__index", ଖାଲି},  /* placeholder */
  {ଖାଲି, ଖାଲି}
};

#ଅନ୍ୟଥା		/* }{ */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tonum (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  ଯଦି (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା {  /* check whether it is a numerical string */
    size_t len;
    ଅଚଳ ଅକ୍ଷର *s = lua_tolstring(L, arg, &len);
    ଫେରନ୍ତୁ (s != ଖାଲି && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
ସ୍ଥିର ଶୂନ୍ୟ trymt (lua_State *L, ଅଚଳ ଅକ୍ଷର *mtkey, ଅଚଳ ଅକ୍ଷର *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  ଯଦି (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_ଭର୍ତ୍ତି(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଅଚଳ ଅକ୍ଷର *mtname) {
  ଯଦି (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  ଅନ୍ୟଥା
    trymt(L, mtname, mtname + 2);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_add (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPADD, "__add");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_sub (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPSUB, "__sub");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_mul (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPMUL, "__mul");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_mod (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPMOD, "__mod");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_pow (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPPOW, "__pow");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_div (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPDIV, "__div");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_idiv (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPIDIV, "__idiv");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା arith_unm (lua_State *L) {
  ଫେରନ୍ତୁ arith(L, LUA_OPUNM, "__unm");
}


ସ୍ଥିର ଅଚଳ luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", ଖାଲି},  /* placeholder */
  {ଖାଲି, ଖାଲି}
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


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା MatchState {
  ଅଚଳ ଅକ୍ଷର *src_init;  /* init of source string */
  ଅଚଳ ଅକ୍ଷର *src_end;  /* end ('\0') of source string */
  ଅଚଳ ଅକ୍ଷର *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level;  /* total number of captures (finished or unfinished) */
  ସଂରଚନା {
    ଅଚଳ ଅକ୍ଷର *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *match (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *p);


/* maximum recursion depth for 'match' */
#ଯଦି !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_capture (MatchState *ms, ପୂର୍ଣ୍ଣସଂଖ୍ୟା l) {
  l -= '1';
  ଯଦି (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    ଫେରନ୍ତୁ luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  ଫେରନ୍ତୁ l;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା capture_to_close (MatchState *ms) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = ms->level;
  ପାଇଁ (level--; level>=0; level--)
    ଯଦି (ms->capture[level].len == CAP_UNFINISHED) ଫେରନ୍ତୁ level;
  ଫେରନ୍ତୁ luaL_error(ms->L, "invalid pattern capture");
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *classend (MatchState *ms, ଅଚଳ ଅକ୍ଷର *p) {
  ବାଛ (*p++) {
    ସ୍ଥିତି L_ESC: {
      ଯଦି (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      ଫେରନ୍ତୁ p+1;
    }
    ସ୍ଥିତି '[': {
      ଯଦି (*p == '^') p++;
      କର {  /* look for a ']' */
        ଯଦି (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        ଯଦି (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } ଯେତେବେଳେ (*p != ']');
      ଫେରନ୍ତୁ p+1;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ଫେରନ୍ତୁ p;
    }
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା match_class (ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cl) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  ବାଛ (tolower(cl)) {
    ସ୍ଥିତି 'a' : res = isalpha(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'c' : res = iscntrl(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'd' : res = isdigit(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'g' : res = isgraph(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'l' : res = islower(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'p' : res = ispunct(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 's' : res = isspace(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'u' : res = isupper(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'w' : res = isalnum(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'x' : res = isxdigit(c); ଭାଙ୍ଗ;
    ସ୍ଥିତି 'z' : res = (c == 0); ଭାଙ୍ଗ;  /* deprecated option */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ (cl == c);
  }
  ଫେରନ୍ତୁ (islower(cl) ? res : !res);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା matchbracketclass (ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *ec) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig = 1;
  ଯଦି (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  ଯେତେବେଳେ (++p < ec) {
    ଯଦି (*p == L_ESC) {
      p++;
      ଯଦି (match_class(c, cast_uchar(*p)))
        ଫେରନ୍ତୁ sig;
    }
    ଅନ୍ୟଥା ଯଦି ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      ଯଦି (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        ଫେରନ୍ତୁ sig;
    }
    ଅନ୍ୟଥା ଯଦି (cast_uchar(*p) == c) ଫେରନ୍ତୁ sig;
  }
  ଫେରନ୍ତୁ !sig;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା singlematch (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *p,
                        ଅଚଳ ଅକ୍ଷର *ep) {
  ଯଦି (s >= ms->src_end)
    ଫେରନ୍ତୁ 0;
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = cast_uchar(*s);
    ବାଛ (*p) {
      ସ୍ଥିତି '.': ଫେରନ୍ତୁ 1;  /* matches any char */
      ସ୍ଥିତି L_ESC: ଫେରନ୍ତୁ match_class(c, cast_uchar(*(p+1)));
      ସ୍ଥିତି '[': ଫେରନ୍ତୁ matchbracketclass(c, p, ep-1);
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  ଫେରନ୍ତୁ (cast_uchar(*p) == c);
    }
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *matchbalance (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s,
                                   ଅଚଳ ଅକ୍ଷର *p) {
  ଯଦି (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  ଯଦି (*s != *p) ଫେରନ୍ତୁ ଖାଲି;
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = *p;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା e = *(p+1);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cont = 1;
    ଯେତେବେଳେ (++s < ms->src_end) {
      ଯଦି (*s == e) {
        ଯଦି (--cont == 0) ଫେରନ୍ତୁ s+1;
      }
      ଅନ୍ୟଥା ଯଦି (*s == b) cont++;
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* string ends out of balance */
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *max_expand (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s,
                                 ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  ଯେତେବେଳେ (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  ଯେତେବେଳେ (i>=0) {
    ଅଚଳ ଅକ୍ଷର *res = match(ms, (s+i), ep+1);
    ଯଦି (res) ଫେରନ୍ତୁ res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  ଫେରନ୍ତୁ ଖାଲି;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *min_expand (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s,
                                 ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *ep) {
  ପାଇଁ (;;) {
    ଅଚଳ ଅକ୍ଷର *res = match(ms, s, ep+1);
    ଯଦି (res != ଖାଲି)
      ଫେରନ୍ତୁ res;
    ଅନ୍ୟଥା ଯଦି (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    ଅନ୍ୟଥା ଫେରନ୍ତୁ ଖାଲି;
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *start_capture (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s,
                                    ଅଚଳ ଅକ୍ଷର *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା what) {
  ଅଚଳ ଅକ୍ଷର *res;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = ms->level;
  ଯଦି (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  ଯଦି ((res=match(ms, s, p)) == ଖାଲି)  /* match failed? */
    ms->level--;  /* undo capture */
  ଫେରନ୍ତୁ res;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *end_capture (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s,
                                  ଅଚଳ ଅକ୍ଷର *p) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା l = capture_to_close(ms);
  ଅଚଳ ଅକ୍ଷର *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  ଯଦି ((res = match(ms, s, p)) == ଖାଲି)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  ଫେରନ୍ତୁ res;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *match_capture (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  ଯଦି ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    ଫେରନ୍ତୁ s+len;
  ଅନ୍ୟଥା ଫେରନ୍ତୁ ଖାଲି;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *match (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *p) {
  ଯଦି (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  ଯଦି (p != ms->p_end) {  /* end of pattern? */
    ବାଛ (*p) {
      ସ୍ଥିତି '(': {  /* start capture */
        ଯଦି (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        ଅନ୍ୟଥା
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି '$': {
        ଯଦି ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          ଯାଅ dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : ଖାଲି;  /* check end of string */
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        ବାଛ (*(p + 1)) {
          ସ୍ଥିତି 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            ଯଦି (s != ଖାଲି) {
              p += 4; ଯାଅ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            ଭାଙ୍ଗ;
          }
          ସ୍ଥିତି 'f': {  /* frontier? */
            ଅଚଳ ଅକ୍ଷର *ep; ଅକ୍ଷର previous;
            p += 2;
            ଯଦି (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            ଯଦି (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; ଯାଅ init;  /* return match(ms, s, ep); */
            }
            s = ଖାଲି;  /* match failed */
            ଭାଙ୍ଗ;
          }
          ସ୍ଥିତି '0': ସ୍ଥିତି '1': ସ୍ଥିତି '2': ସ୍ଥିତି '3':
          ସ୍ଥିତି '4': ସ୍ଥିତି '5': ସ୍ଥିତି '6': ସ୍ଥିତି '7':
          ସ୍ଥିତି '8': ସ୍ଥିତି '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            ଯଦି (s != ଖାଲି) {
              p += 2; ଯାଅ init;  /* return match(ms, s, p + 2) */
            }
            ଭାଙ୍ଗ;
          }
          ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଯାଅ dflt;
        }
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: dflt: {  /* pattern class plus optional suffix */
        ଅଚଳ ଅକ୍ଷର *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        ଯଦି (!singlematch(ms, s, p, ep)) {
          ଯଦି (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; ଯାଅ init;  /* return match(ms, s, ep + 1); */
          }
          ଅନ୍ୟଥା  /* '+' or no suffix */
            s = ଖାଲି;  /* fail */
        }
        ଅନ୍ୟଥା {  /* matched once */
          ବାଛ (*ep) {  /* handle optional suffix */
            ସ୍ଥିତି '?': {  /* optional */
              ଅଚଳ ଅକ୍ଷର *res;
              ଯଦି ((res = match(ms, s + 1, ep + 1)) != ଖାଲି)
                s = res;
              ଅନ୍ୟଥା {
                p = ep + 1; ଯାଅ init;  /* else return match(ms, s, ep + 1); */
              }
              ଭାଙ୍ଗ;
            }
            ସ୍ଥିତି '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            ସ୍ଥିତି '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              ଭାଙ୍ଗ;
            ସ୍ଥିତି '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  /* no suffix */
              s++; p = ep; ଯାଅ init;  /* return match(ms, s + 1, ep); */
          }
        }
        ଭାଙ୍ଗ;
      }
    }
  }
  ms->matchdepth++;
  ଫେରନ୍ତୁ s;
}



ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *lmemfind (ଅଚଳ ଅକ୍ଷର *s1, size_t l1,
                               ଅଚଳ ଅକ୍ଷର *s2, size_t l2) {
  ଯଦି (l2 == 0) ଫେରନ୍ତୁ s1;  /* empty strings are everywhere */
  ଅନ୍ୟଥା ଯଦି (l2 > l1) ଫେରନ୍ତୁ ଖାଲି;  /* avoids a negative 'l1' */
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    ଯେତେବେଳେ (l1 > 0 && (init = (ଅଚଳ ଅକ୍ଷର *)memchr(s1, *s2, l1)) != ଖାଲି) {
      init++;   /* 1st char is already checked */
      ଯଦି (memcmp(init, s2+1, l2) == 0)
        ଫେରନ୍ତୁ init-1;
      ଅନ୍ୟଥା {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    ଫେରନ୍ତୁ ଖାଲି;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
ସ୍ଥିର ptrdiff_t get_onecapture (MatchState *ms, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଅଚଳ ଅକ୍ଷର *s,
                              ଅଚଳ ଅକ୍ଷର *e, ଅଚଳ ଅକ୍ଷର **cap) {
  ଯଦି (i >= ms->level) {
    ଯଦି (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    ଫେରନ୍ତୁ (e - s);
  }
  ଅନ୍ୟଥା {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    ଯଦି (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    ଅନ୍ୟଥା ଯଦି (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    ଫେରନ୍ତୁ capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ସ୍ଥିର ଶୂନ୍ୟ ଠେଲ_onecapture (MatchState *ms, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଅଚଳ ଅକ୍ଷର *s,
                                                    ଅଚଳ ଅକ୍ଷର *e) {
  ଅଚଳ ଅକ୍ଷର *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  ଯଦି (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଠେଲ_captures (MatchState *ms, ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *e) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  ପାଇଁ (i = 0; i < nlevels; i++)
    ଠେଲ_onecapture(ms, i, s, e);
  ଫେରନ୍ତୁ nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nospecials (ଅଚଳ ଅକ୍ଷର *p, size_t l) {
  size_t upto = 0;
  କର {
    ଯଦି (strpbrk(p + upto, SPECIALS))
      ଫେରନ୍ତୁ 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } ଯେତେବେଳେ (upto <= l);
  ଫେରନ୍ତୁ 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
ସ୍ଥିର ଶୂନ୍ୟ prepstate (MatchState *ms, lua_State *L,
                       ଅଚଳ ଅକ୍ଷର *s, size_t ls, ଅଚଳ ଅକ୍ଷର *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
ସ୍ଥିର ଶୂନ୍ୟ reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_find_aux (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା find) {
  size_t ls, lp;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &ls);
  ଅଚଳ ଅକ୍ଷର *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  ଯଦି (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    ଫେରନ୍ତୁ 1;
  }
  /* explicit request or no special characters? */
  ଯଦି (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ଅଚଳ ଅକ୍ଷର *s2 = lmemfind(s + init, ls - init, p, lp);
    ଯଦି (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      ଫେରନ୍ତୁ 2;
    }
  }
  ଅନ୍ୟଥା {
    MatchState ms;
    ଅଚଳ ଅକ୍ଷର *s1 = s + init;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା anchor = (*p == '^');
    ଯଦି (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    କର {
      ଅଚଳ ଅକ୍ଷର *res;
      reprepstate(&ms);
      ଯଦି ((res=match(&ms, s1, p)) != ଖାଲି) {
        ଯଦି (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          ଫେରନ୍ତୁ ଠେଲ_captures(&ms, ଖାଲି, 0) + 2;
        }
        ଅନ୍ୟଥା
          ଫେରନ୍ତୁ ଠେଲ_captures(&ms, s1, res);
      }
    } ଯେତେବେଳେ (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_find (lua_State *L) {
  ଫେରନ୍ତୁ str_find_aux(L, 1);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_match (lua_State *L) {
  ଫେରନ୍ତୁ str_find_aux(L, 0);
}


/* state for 'gmatch' */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା GMatchState {
  ଅଚଳ ଅକ୍ଷର *src;  /* current position */
  ଅଚଳ ଅକ୍ଷର *p;  /* pattern */
  ଅଚଳ ଅକ୍ଷର *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ଅଚଳ ଅକ୍ଷର *src;
  gm->ms.L = L;
  ପାଇଁ (src = gm->src; src <= gm->ms.src_end; src++) {
    ଅଚଳ ଅକ୍ଷର *e;
    reprepstate(&gm->ms);
    ଯଦି ((e = match(&gm->ms, src, gm->p)) != ଖାଲି && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      ଫେରନ୍ତୁ ଠେଲ_captures(&gm->ms, src, e);
    }
  }
  ଫେରନ୍ତୁ 0;  /* not found */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gmatch (lua_State *L) {
  size_t ls, lp;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &ls);
  ଅଚଳ ଅକ୍ଷର *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, ଆକାର(GMatchState), 0);
  ଯଦି (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = ଖାଲି;
  lua_pushcclosure(L, gmatch_aux, 3);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ଶୂନ୍ୟ add_s (MatchState *ms, luaL_Buffer *b, ଅଚଳ ଅକ୍ଷର *s,
                                                   ଅଚଳ ଅକ୍ଷର *e) {
  size_t l;
  lua_State *L = ms->L;
  ଅଚଳ ଅକ୍ଷର *news = lua_tolstring(L, 3, &l);
  ଅଚଳ ଅକ୍ଷର *p;
  ଯେତେବେଳେ ((p = (ଅକ୍ଷର *)memchr(news, L_ESC, l)) != ଖାଲି) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    ଯଦି (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    ଅନ୍ୟଥା ଯଦି (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    ଅନ୍ୟଥା ଯଦି (isdigit(cast_uchar(*p))) {  /* '%n' */
      ଅଚଳ ଅକ୍ଷର *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      ଯଦି (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      ଅନ୍ୟଥା
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    ଅନ୍ୟଥା
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_ମୂଲ୍ୟ (MatchState *ms, luaL_Buffer *b, ଅଚଳ ଅକ୍ଷର *s,
                                      ଅଚଳ ଅକ୍ଷର *e, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tr) {
  lua_State *L = ms->L;
  ବାଛ (tr) {
    ସ୍ଥିତି LUA_TFUNCTION: {  /* call the function */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ଠେଲ_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_TTABLE: {  /* index the table */
      ଠେଲ_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      ଫେରନ୍ତୁ 1;  /* something changed */
    }
  }
  ଯଦି (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_କାଢ଼(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    ଫେରନ୍ତୁ 0;  /* no changes */
  }
  ଅନ୍ୟଥା ଯଦି (l_unlikely(!lua_isstring(L, -1)))
    ଫେରନ୍ତୁ luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  ଅନ୍ୟଥା {
    luaL_addvalue(b);  /* add result to accumulator */
    ଫେରନ୍ତୁ 1;  /* something changed */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_gsub (lua_State *L) {
  size_t srcl, lp;
  ଅଚଳ ଅକ୍ଷର *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ଅଚଳ ଅକ୍ଷର *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  ଅଚଳ ଅକ୍ଷର *lastmatch = ଖାଲି;  /* end of last match */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  ଯଦି (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  ଯେତେବେଳେ (n < max_s) {
    ଅଚଳ ଅକ୍ଷର *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    ଯଦି ((e = match(&ms, src, p)) != ଖାଲି && e != lastmatch) {  /* match? */
      n++;
      changed = add_ମୂଲ୍ୟ(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    ଅନ୍ୟଥା ଯଦି (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    ଅନ୍ୟଥା ଭାଙ୍ଗ;  /* end of subject */
    ଯଦି (anchor) ଭାଙ୍ଗ;
  }
  ଯଦି (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  ଅନ୍ୟଥା {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  ଫେରନ୍ତୁ 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#ଯଦି !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(ଆକାର(LUA_ସଂଖ୍ୟା_FRMLEN)/ଆକାର(ଅକ୍ଷର))


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
ସ୍ଥିର lua_ସଂଖ୍ୟା adddigit (ଅକ୍ଷର *buff, unsigned n, lua_ସଂଖ୍ୟା x) {
  lua_ସଂଖ୍ୟା dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା d = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  ଫେରନ୍ତୁ x - dd;  /* return what is left */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା num2straux (ଅକ୍ଷର *buff, unsigned sz, lua_ସଂଖ୍ୟା x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  ଯଦି (x != x || x == (lua_ସଂଖ୍ୟା)HUGE_VAL || x == -(lua_ସଂଖ୍ୟା)HUGE_VAL)
    ଫେରନ୍ତୁ l_sprintf(buff, sz, LUA_ସଂଖ୍ୟା_FMT, (LUAI_UACNUMBER)x);
  ଅନ୍ୟଥା ଯଦି (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    ଫେରନ୍ତୁ l_sprintf(buff, sz, LUA_ସଂଖ୍ୟା_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା e;
    lua_ସଂଖ୍ୟା m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    ଯଦି (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    ଯଦି (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      କର {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } ଯେତେବେଳେ (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    ଫେରନ୍ତୁ cast_int(n);
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_number2strx (lua_State *L, ଅକ୍ଷର *buff, unsigned sz,
                            ଅଚଳ ଅକ୍ଷର *fmt, lua_ସଂଖ୍ୟା x) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = num2straux(buff, sz, x);
  ଯଦି (fmt[SIZELENMOD] == 'A') {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  ଅନ୍ୟଥା ଯଦି (l_unlikely(fmt[SIZELENMOD] != 'a'))
    ଫେରନ୍ତୁ luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  ଫେରନ୍ତୁ n;
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
#ଯଦି !defined(L_FMTFLAGSF)

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


ସ୍ଥିର ଶୂନ୍ୟ addquoted (luaL_Buffer *b, ଅଚଳ ଅକ୍ଷର *s, size_t len) {
  luaL_addchar(b, '"');
  ଯେତେବେଳେ (len--) {
    ଯଦି (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    ଅନ୍ୟଥା ଯଦି (iscntrl(cast_uchar(*s))) {
      ଅକ୍ଷର buff[10];
      ଯଦି (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, ଆକାର(buff), "\\%d", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)cast_uchar(*s));
      ଅନ୍ୟଥା
        l_sprintf(buff, ଆକାର(buff), "\\%03d", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    ଅନ୍ୟଥା
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା quotefloat (lua_State *L, ଅକ୍ଷର *buff, lua_ସଂଖ୍ୟା n) {
  ଅଚଳ ଅକ୍ଷର *s;  /* for the fixed representations */
  ଯଦି (n == (lua_ସଂଖ୍ୟା)HUGE_VAL)  /* inf? */
    s = "1e9999";
  ଅନ୍ୟଥା ଯଦି (n == -(lua_ସଂଖ୍ୟା)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  ଅନ୍ୟଥା ଯଦି (n != n)  /* NaN? */
    s = "(0/0)";
  ଅନ୍ୟଥା {  /* format number as hexadecimal */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_ସଂଖ୍ୟା_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    ଯଦି (memchr(buff, '.', cast_uint(nb)) == ଖାଲି) {  /* no dot? */
      ଅକ୍ଷର point = lua_getlocaledecpoint();  /* try locale point */
      ଅକ୍ଷର *ppoint = (ଅକ୍ଷର *)memchr(buff, point, cast_uint(nb));
      ଯଦି (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    ଫେରନ୍ତୁ nb;
  }
  /* for the fixed representations */
  ଫେରନ୍ତୁ l_sprintf(buff, MAX_ITEM, "%s", s);
}


ସ୍ଥିର ଶୂନ୍ୟ addliteral (lua_State *L, luaL_Buffer *b, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  ବାଛ (lua_type(L, arg)) {
    ସ୍ଥିତି LUA_TSTRING: {
      size_t len;
      ଅଚଳ ଅକ୍ଷର *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_TNUMBER: {
      ଅକ୍ଷର *buff = luaL_prepbuffsize(b, MAX_ITEM);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nb;
      ଯଦି (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      ଅନ୍ୟଥା {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ଅଚଳ ଅକ୍ଷର *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_TNIL: ସ୍ଥିତି LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, ଖାଲି);
      luaL_addvalue(b);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *get2digits (ଅଚଳ ଅକ୍ଷର *s) {
  ଯଦି (isdigit(cast_uchar(*s))) {
    s++;
    ଯଦି (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  ଫେରନ୍ତୁ s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
ସ୍ଥିର ଶୂନ୍ୟ checkformat (lua_State *L, ଅଚଳ ଅକ୍ଷର *form, ଅଚଳ ଅକ୍ଷର *flags,
                                       ପୂର୍ଣ୍ଣସଂଖ୍ୟା precision) {
  ଅଚଳ ଅକ୍ଷର *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  ଯଦି (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    ଯଦି (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  ଯଦି (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getformat (lua_State *L, ଅଚଳ ଅକ୍ଷର *strfrmt,
                                            ଅକ୍ଷର *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  ଯଦି (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * ଆକାର(ଅକ୍ଷର));
  *(form + len) = '\0';
  ଫେରନ୍ତୁ strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
ସ୍ଥିର ଶୂନ୍ୟ addlenmod (ଅକ୍ଷର *form, ଅଚଳ ଅକ୍ଷର *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  ଅକ୍ଷର spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_format (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ = lua_gettop(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg = 1;
  size_t sfl;
  ଅଚଳ ଅକ୍ଷର *strfrmt = luaL_checklstring(L, arg, &sfl);
  ଅଚଳ ଅକ୍ଷର *strfrmt_end = strfrmt+sfl;
  ଅଚଳ ଅକ୍ଷର *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  ଯେତେବେଳେ (strfrmt < strfrmt_end) {
    ଯଦି (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    ଅନ୍ୟଥା ଯଦି (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    ଅନ୍ୟଥା { /* format item */
      ଅକ୍ଷର form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      ଅକ୍ଷର *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nb = 0;  /* number of bytes in result */
      ଯଦି (++arg > ଶୀର୍ଷ)
        ଫେରନ୍ତୁ luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      ବାଛ (*strfrmt++) {
        ସ୍ଥିତି 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, arg));
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି 'd': ସ୍ଥିତି 'i':
          flags = L_FMTFLAGSI;
          ଯାଅ intcase;
        ସ୍ଥିତି 'u':
          flags = L_FMTFLAGSU;
          ଯାଅ intcase;
        ସ୍ଥିତି 'o': ସ୍ଥିତି 'x': ସ୍ଥିତି 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି 'a': ସ୍ଥିତି 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ସଂଖ୍ୟା_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          ଭାଙ୍ଗ;
        ସ୍ଥିତି 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        ସ୍ଥିତି 'e': ସ୍ଥିତି 'E': ସ୍ଥିତି 'g': ସ୍ଥିତି 'G': {
          lua_ସଂଖ୍ୟା n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ସଂଖ୍ୟା_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି 'p': {
          ଅଚଳ ଶୂନ୍ୟ *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          ଯଦି (p == ଖାଲି) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି 'q': {
          ଯଦି (form[2] != '\0')  /* modifiers? */
            ଫେରନ୍ତୁ luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          ଭାଙ୍ଗ;
        }
        ସ୍ଥିତି 's': {
          size_t l;
          ଅଚଳ ଅକ୍ଷର *s = luaL_tolstring(L, arg, &l);
          ଯଦି (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          ଅନ୍ୟଥା {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            ଯଦି (strchr(form, '.') == ଖାଲି && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            ଅନ୍ୟଥା {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_କାଢ଼(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          ଭାଙ୍ଗ;
        }
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {  /* also treat cases 'pnLlh' */
          ଫେରନ୍ତୁ luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  ଫେରନ୍ତୁ 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#ଯଦି !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(lua_Integer))


/* dummy union to get native endianness */
ସ୍ଥିର ଅଚଳ ସଂଘ {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dummy;
  ଅକ୍ଷର little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା Header {
  lua_State *L;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା KOption {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା digit (ପୂର୍ଣ୍ଣସଂଖ୍ୟା c) { ଫେରନ୍ତୁ '0' <= c && c <= '9'; }

ସ୍ଥିର size_t getnum (ଅଚଳ ଅକ୍ଷର **fmt, size_t df) {
  ଯଦି (!digit(**fmt))  /* no number? */
    ଫେରନ୍ତୁ df;  /* return default value */
  ଅନ୍ୟଥା {
    size_t a = 0;
    କର {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } ଯେତେବେଳେ (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    ଫେରନ୍ତୁ a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ସ୍ଥିର unsigned getnumlimit (Header *h, ଅଚଳ ଅକ୍ଷର **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  ଯଦି (l_unlikely((sz - 1u) >= MAXINTSIZE))
    ଫେରନ୍ତୁ cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  ଫେରନ୍ତୁ cast_uint(sz);
}


/*
** Initialize Header
*/
ସ୍ଥିର ଶୂନ୍ୟ initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
ସ୍ଥିର KOption getoption (Header *h, ଅଚଳ ଅକ୍ଷର **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  ସଂରଚନା cD { ଅକ୍ଷର c; ସଂଘ { LUAI_MAXALIGN; } u; };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt = *((*fmt)++);
  *size = 0;  /* default */
  ବାଛ (opt) {
    ସ୍ଥିତି 'b': *size = ଆକାର(ଅକ୍ଷର); ଫେରନ୍ତୁ Kint;
    ସ୍ଥିତି 'B': *size = ଆକାର(ଅକ୍ଷର); ଫେରନ୍ତୁ Kuint;
    ସ୍ଥିତି 'h': *size = ଆକାର(short); ଫେରନ୍ତୁ Kint;
    ସ୍ଥିତି 'H': *size = ଆକାର(short); ଫେରନ୍ତୁ Kuint;
    ସ୍ଥିତି 'l': *size = ଆକାର(long); ଫେରନ୍ତୁ Kint;
    ସ୍ଥିତି 'L': *size = ଆକାର(long); ଫେରନ୍ତୁ Kuint;
    ସ୍ଥିତି 'j': *size = ଆକାର(lua_Integer); ଫେରନ୍ତୁ Kint;
    ସ୍ଥିତି 'J': *size = ଆକାର(lua_Integer); ଫେରନ୍ତୁ Kuint;
    ସ୍ଥିତି 'T': *size = ଆକାର(size_t); ଫେରନ୍ତୁ Kuint;
    ସ୍ଥିତି 'f': *size = ଆକାର(ଦଶମିକ); ଫେରନ୍ତୁ Kfloat;
    ସ୍ଥିତି 'n': *size = ଆକାର(lua_ସଂଖ୍ୟା); ଫେରନ୍ତୁ Knumber;
    ସ୍ଥିତି 'd': *size = ଆକାର(ଦ୍ୱିଗୁଣ); ଫେରନ୍ତୁ Kdouble;
    ସ୍ଥିତି 'i': *size = getnumlimit(h, fmt, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)); ଫେରନ୍ତୁ Kint;
    ସ୍ଥିତି 'I': *size = getnumlimit(h, fmt, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)); ଫେରନ୍ତୁ Kuint;
    ସ୍ଥିତି 's': *size = getnumlimit(h, fmt, ଆକାର(size_t)); ଫେରନ୍ତୁ Kstring;
    ସ୍ଥିତି 'c':
      *size = getnum(fmt, cast_sizet(-1));
      ଯଦି (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      ଫେରନ୍ତୁ Kchar;
    ସ୍ଥିତି 'z': ଫେରନ୍ତୁ Kzstr;
    ସ୍ଥିତି 'x': *size = 1; ଫେରନ୍ତୁ Kpadding;
    ସ୍ଥିତି 'X': ଫେରନ୍ତୁ Kpaddalign;
    ସ୍ଥିତି ' ': ଭାଙ୍ଗ;
    ସ୍ଥିତି '<': h->islittle = 1; ଭାଙ୍ଗ;
    ସ୍ଥିତି '>': h->islittle = 0; ଭାଙ୍ଗ;
    ସ୍ଥିତି '=': h->islittle = nativeendian.little; ଭାଙ୍ଗ;
    ସ୍ଥିତି '!': {
      ଅଚଳ size_t maxalign = offsetof(ସଂରଚନା cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  ଫେରନ୍ତୁ Knop;
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
ସ୍ଥିର KOption getdetails (Header *h, size_t totalsize, ଅଚଳ ଅକ୍ଷର **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  ଯଦି (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    ଯଦି (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  ଯଦି (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  ଅନ୍ୟଥା {
    ଯଦି (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    ଯଦି (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    ଅନ୍ୟଥା {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  ଫେରନ୍ତୁ opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
ସ୍ଥିର ଶୂନ୍ୟ packint (luaL_Buffer *b, lua_Unsigned n,
                     ପୂର୍ଣ୍ଣସଂଖ୍ୟା islittle, unsigned size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା neg) {
  ଅକ୍ଷର *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (ଅକ୍ଷର)(n & MC);  /* first byte */
  ପାଇଁ (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (ଅକ୍ଷର)(n & MC);
  }
  ଯଦି (neg && size > SZINT) {  /* negative number need sign extension? */
    ପାଇଁ (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (ଅକ୍ଷର)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
ସ୍ଥିର ଶୂନ୍ୟ copywithendian (ଅକ୍ଷର *dest, ଅଚଳ ଅକ୍ଷର *src,
                            unsigned size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା islittle) {
  ଯଦି (islittle == nativeendian.little)
    memcpy(dest, src, size);
  ଅନ୍ୟଥା {
    dest += size - 1;
    ଯେତେବେଳେ (size-- != 0)
      *(dest--) = *(src++);
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  ଅଚଳ ଅକ୍ଷର *fmt = luaL_checkstring(L, 1);  /* format string */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  ଯେତେବେଳେ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    ଯେତେବେଳେ (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    ବାଛ (opt) {
      ସ୍ଥିତି Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ଯଦି (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ଯଦି (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kfloat: {  /* C float */
        ଦଶମିକ f = (ଦଶମିକ)luaL_checknumber(L, arg);  /* get argument */
        ଅକ୍ଷର *buff = luaL_prepbuffsize(&b, ଆକାର(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ଅକ୍ଷର *)&f, ଆକାର(f), h.islittle);
        luaL_addsize(&b, size);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Knumber: {  /* Lua float */
        lua_ସଂଖ୍ୟା f = luaL_checknumber(L, arg);  /* get argument */
        ଅକ୍ଷର *buff = luaL_prepbuffsize(&b, ଆକାର(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ଅକ୍ଷର *)&f, ଆକାର(f), h.islittle);
        luaL_addsize(&b, size);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kdouble: {  /* C double */
        ଦ୍ୱିଗୁଣ f = (ଦ୍ୱିଗୁଣ)luaL_checknumber(L, arg);  /* get argument */
        ଅକ୍ଷର *buff = luaL_prepbuffsize(&b, ଆକାର(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ଅକ୍ଷର *)&f, ଆକାର(f), h.islittle);
        luaL_addsize(&b, size);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kchar: {  /* fixed-size string */
        size_t len;
        ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        ଯଦି (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          ଅକ୍ଷର *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kstring: {  /* strings with length count */
        size_t len;
        ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= ଆକାର(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kzstr: {  /* zero-terminated string */
        size_t len;
        ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      ସ୍ଥିତି Kpaddalign: ସ୍ଥିତି Knop:
        arg--;  /* undo increment */
        ଭାଙ୍ଗ;
    }
  }
  luaL_pushresult(&b);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_packsize (lua_State *L) {
  Header h;
  ଅଚଳ ଅକ୍ଷର *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  ଯେତେବେଳେ (*fmt != '\0') {
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
  ଫେରନ୍ତୁ 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
ସ୍ଥିର lua_Integer unpackint (lua_State *L, ଅଚଳ ଅକ୍ଷର *str,
                              ପୂର୍ଣ୍ଣସଂଖ୍ୟା islittle, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା issigned) {
  lua_Unsigned res = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit = (size  <= SZINT) ? size : SZINT;
  ପାଇଁ (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned ଅକ୍ଷର)str[islittle ? i : size - 1 - i];
  }
  ଯଦି (size < SZINT) {  /* real size smaller than lua_Integer? */
    ଯଦି (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  ଅନ୍ୟଥା ଯଦି (size > SZINT) {  /* must check unread bytes */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    ପାଇଁ (i = limit; i < size; i++) {
      ଯଦି (l_unlikely((unsigned ଅକ୍ଷର)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  ଫେରନ୍ତୁ (lua_Integer)res;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା str_unpack (lua_State *L) {
  Header h;
  ଅଚଳ ଅକ୍ଷର *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ଅଚଳ ଅକ୍ଷର *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  ଯେତେବେଳେ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    ବାଛ (opt) {
      ସ୍ଥିତି Kint:
      ସ୍ଥିତି Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kfloat: {
        ଦଶମିକ f;
        copywithendian((ଅକ୍ଷର *)&f, data + pos, ଆକାର(f), h.islittle);
        lua_pushnumber(L, (lua_ସଂଖ୍ୟା)f);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Knumber: {
        lua_ସଂଖ୍ୟା f;
        copywithendian((ଅକ୍ଷର *)&f, data + pos, ଆକାର(f), h.islittle);
        lua_pushnumber(L, f);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kdouble: {
        ଦ୍ୱିଗୁଣ f;
        copywithendian((ଅକ୍ଷର *)&f, data + pos, ଆକାର(f), h.islittle);
        lua_pushnumber(L, (lua_ସଂଖ୍ୟା)f);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kchar: {
        lua_pushlstring(L, data + pos, size);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି Kpaddalign: ସ୍ଥିତି Kpadding: ସ୍ଥିତି Knop:
        n--;  /* undo increment */
        ଭାଙ୍ଗ;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  ଫେରନ୍ତୁ n + 1;
}

/* }====================================================== */


ସ୍ଥିର ଅଚଳ luaL_Reg strlib[] = {
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
  {ଖାଲି, ଖାଲି}
};


ସ୍ଥିର ଶୂନ୍ୟ createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_କାଢ଼(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_କାଢ଼(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  ଫେରନ୍ତୁ 1;
}

