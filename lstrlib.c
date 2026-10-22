/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#include "lprefix.h"


#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#ប្រសិនបើ !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


static ចំនួនគត់ str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  ត្រឡប់ 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
static size_t posrelatI (lua_Integer pos, size_t len) {
  ប្រសិនបើ (pos > 0)
    ត្រឡប់ (size_t)pos;
  បើមិនដូច្នេះ ប្រសិនបើ (pos == 0)
    ត្រឡប់ 1;
  បើមិនដូច្នេះ ប្រសិនបើ (pos < -(lua_Integer)len)  /* inverted comparison */
    ត្រឡប់ 1;  /* clip to 1 */
  បើមិនដូច្នេះ ត្រឡប់ len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
static size_t getendpos (lua_State *L, ចំនួនគត់ arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  ប្រសិនបើ (pos > (lua_Integer)len)
    ត្រឡប់ len;
  បើមិនដូច្នេះ ប្រសិនបើ (pos >= 0)
    ត្រឡប់ (size_t)pos;
  បើមិនដូច្នេះ ប្រសិនបើ (pos < -(lua_Integer)len)
    ត្រឡប់ 0;
  បើមិនដូច្នេះ ត្រឡប់ len + (size_t)pos + 1;
}


static ចំនួនគត់ str_sub (lua_State *L) {
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  ប្រសិនបើ (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  បើមិនដូច្នេះ lua_pushliteral(L, "");
  ត្រឡប់ 1;
}


static ចំនួនគត់ str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  const var_na *s = luaL_checklstring(L, 1, &l);
  var_na *p = luaL_buffinitsize(L, &b, l);
  សម្រាប់ (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  ត្រឡប់ 1;
}


static ចំនួនគត់ str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const var_na *s = luaL_checklstring(L, 1, &l);
  var_na *p = luaL_buffinitsize(L, &b, l);
  សម្រាប់ (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ត្រឡប់ 1;
}


static ចំនួនគត់ str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const var_na *s = luaL_checklstring(L, 1, &l);
  var_na *p = luaL_buffinitsize(L, &b, l);
  សម្រាប់ (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ត្រឡប់ 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
static ចំនួនគត់ str_rep (lua_State *L) {
  size_t len, lsep;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  const var_na *sep = luaL_optlstring(L, 3, "", &lsep);
  ប្រសិនបើ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  បើមិនដូច្នេះ ប្រសិនបើ (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    ត្រឡប់ luaL_error(L, "resulting string too large");
  បើមិនដូច្នេះ {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    var_na *p = luaL_buffinitsize(L, &b, totallen);
    ខណៈពេល (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * parimaa_na(var_na)); p += len;
      ប្រសិនបើ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * parimaa_na(var_na)); p += lsep;
      }
    }
    memcpy(p, s, len * parimaa_na(var_na));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  ត្រឡប់ 1;
}


static ចំនួនគត់ str_byte (lua_State *L) {
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  ចំនួនគត់ n, i;
  ប្រសិនបើ (posi > pose) ត្រឡប់ 0;  /* empty interval; return no values */
  ប្រសិនបើ (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    ត្រឡប់ luaL_error(L, "string slice too long");
  n = (ចំនួនគត់)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  សម្រាប់ (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  ត្រឡប់ n;
}


static ចំនួនគត់ str_char (lua_State *L) {
  ចំនួនគត់ n = lua_gettop(L);  /* number of arguments */
  ចំនួនគត់ i;
  luaL_Buffer b;
  var_na *p = luaL_buffinitsize(L, &b, cast_uint(n));
  សម្រាប់ (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  ត្រឡប់ 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
sa_nracanaa str_Writer {
  ចំនួនគត់ init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


static ចំនួនគត់ writer (lua_State *L, const void *b, size_t size, void *ud) {
  sa_nracanaa str_Writer *state = (sa_nracanaa str_Writer *)ud;
  ប្រសិនបើ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  ប្រសិនបើ (b == NULL) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  បើមិនដូច្នេះ
    luaL_addlstring(&state->B, (const var_na *)b, size);
  ត្រឡប់ 0;
}


static ចំនួនគត់ str_dump (lua_State *L) {
  sa_nracanaa str_Writer state;
  ចំនួនគត់ strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  ត្រឡប់ 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#ប្រសិនបើ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

static const luaL_Reg stringmetamethods[] = {
  {"__index", NULL},  /* placeholder */
  {NULL, NULL}
};

#បើមិនដូច្នេះ		/* }{ */

static ចំនួនគត់ tonum (lua_State *L, ចំនួនគត់ arg) {
  ប្រសិនបើ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    ត្រឡប់ 1;
  }
  បើមិនដូច្នេះ {  /* check whether it is a numerical string */
    size_t len;
    const var_na *s = lua_tolstring(L, arg, &len);
    ត្រឡប់ (s != NULL && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
static void trymt (lua_State *L, const var_na *mtkey, const var_na *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  ប្រសិនបើ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_insert(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


static ចំនួនគត់ arith (lua_State *L, ចំនួនគត់ op, const var_na *mtname) {
  ប្រសិនបើ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  បើមិនដូច្នេះ
    trymt(L, mtname, mtname + 2);
  ត្រឡប់ 1;
}


static ចំនួនគត់ arith_add (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPADD, "__add");
}

static ចំនួនគត់ arith_sub (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPSUB, "__sub");
}

static ចំនួនគត់ arith_mul (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPMUL, "__mul");
}

static ចំនួនគត់ arith_mod (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPMOD, "__mod");
}

static ចំនួនគត់ arith_pow (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPPOW, "__pow");
}

static ចំនួនគត់ arith_div (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPDIV, "__div");
}

static ចំនួនគត់ arith_idiv (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPIDIV, "__idiv");
}

static ចំនួនគត់ arith_unm (lua_State *L) {
  ត្រឡប់ arith(L, LUA_OPUNM, "__unm");
}


static const luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", NULL},  /* placeholder */
  {NULL, NULL}
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


typedef sa_nracanaa MatchState {
  const var_na *src_init;  /* init of source string */
  const var_na *src_end;  /* end ('\0') of source string */
  const var_na *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  ចំនួនគត់ matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  ចំនួនគត់ level;  /* total number of captures (finished or unfinished) */
  sa_nracanaa {
    const var_na *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
static const var_na *match (MatchState *ms, const var_na *s, const var_na *p);


/* maximum recursion depth for 'match' */
#ប្រសិនបើ !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


static ចំនួនគត់ check_capture (MatchState *ms, ចំនួនគត់ l) {
  l -= '1';
  ប្រសិនបើ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    ត្រឡប់ luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  ត្រឡប់ l;
}


static ចំនួនគត់ capture_to_close (MatchState *ms) {
  ចំនួនគត់ level = ms->level;
  សម្រាប់ (level--; level>=0; level--)
    ប្រសិនបើ (ms->capture[level].len == CAP_UNFINISHED) ត្រឡប់ level;
  ត្រឡប់ luaL_error(ms->L, "invalid pattern capture");
}


static const var_na *classend (MatchState *ms, const var_na *p) {
  switch (*p++) {
    pak_sa L_ESC: {
      ប្រសិនបើ (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      ត្រឡប់ p+1;
    }
    pak_sa '[': {
      ប្រសិនបើ (*p == '^') p++;
      do {  /* look for a ']' */
        ប្រសិនបើ (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        ប្រសិនបើ (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } ខណៈពេល (*p != ']');
      ត្រឡប់ p+1;
    }
    default: {
      ត្រឡប់ p;
    }
  }
}


static ចំនួនគត់ match_class (ចំនួនគត់ c, ចំនួនគត់ cl) {
  ចំនួនគត់ res;
  switch (tolower(cl)) {
    pak_sa 'a' : res = isalpha(c); bha_nga;
    pak_sa 'c' : res = iscntrl(c); bha_nga;
    pak_sa 'd' : res = isdigit(c); bha_nga;
    pak_sa 'g' : res = isgraph(c); bha_nga;
    pak_sa 'l' : res = islower(c); bha_nga;
    pak_sa 'p' : res = ispunct(c); bha_nga;
    pak_sa 's' : res = isspace(c); bha_nga;
    pak_sa 'u' : res = isupper(c); bha_nga;
    pak_sa 'w' : res = isalnum(c); bha_nga;
    pak_sa 'x' : res = isxdigit(c); bha_nga;
    pak_sa 'z' : res = (c == 0); bha_nga;  /* deprecated option */
    default: ត្រឡប់ (cl == c);
  }
  ត្រឡប់ (islower(cl) ? res : !res);
}


static ចំនួនគត់ matchbracketclass (ចំនួនគត់ c, const var_na *p, const var_na *ec) {
  ចំនួនគត់ sig = 1;
  ប្រសិនបើ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  ខណៈពេល (++p < ec) {
    ប្រសិនបើ (*p == L_ESC) {
      p++;
      ប្រសិនបើ (match_class(c, cast_uchar(*p)))
        ត្រឡប់ sig;
    }
    បើមិនដូច្នេះ ប្រសិនបើ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      ប្រសិនបើ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        ត្រឡប់ sig;
    }
    បើមិនដូច្នេះ ប្រសិនបើ (cast_uchar(*p) == c) ត្រឡប់ sig;
  }
  ត្រឡប់ !sig;
}


static ចំនួនគត់ singlematch (MatchState *ms, const var_na *s, const var_na *p,
                        const var_na *ep) {
  ប្រសិនបើ (s >= ms->src_end)
    ត្រឡប់ 0;
  បើមិនដូច្នេះ {
    ចំនួនគត់ c = cast_uchar(*s);
    switch (*p) {
      pak_sa '.': ត្រឡប់ 1;  /* matches any char */
      pak_sa L_ESC: ត្រឡប់ match_class(c, cast_uchar(*(p+1)));
      pak_sa '[': ត្រឡប់ matchbracketclass(c, p, ep-1);
      default:  ត្រឡប់ (cast_uchar(*p) == c);
    }
  }
}


static const var_na *matchbalance (MatchState *ms, const var_na *s,
                                   const var_na *p) {
  ប្រសិនបើ (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  ប្រសិនបើ (*s != *p) ត្រឡប់ NULL;
  បើមិនដូច្នេះ {
    ចំនួនគត់ b = *p;
    ចំនួនគត់ e = *(p+1);
    ចំនួនគត់ cont = 1;
    ខណៈពេល (++s < ms->src_end) {
      ប្រសិនបើ (*s == e) {
        ប្រសិនបើ (--cont == 0) ត្រឡប់ s+1;
      }
      បើមិនដូច្នេះ ប្រសិនបើ (*s == b) cont++;
    }
  }
  ត្រឡប់ NULL;  /* string ends out of balance */
}


static const var_na *max_expand (MatchState *ms, const var_na *s,
                                 const var_na *p, const var_na *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  ខណៈពេល (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  ខណៈពេល (i>=0) {
    const var_na *res = match(ms, (s+i), ep+1);
    ប្រសិនបើ (res) ត្រឡប់ res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  ត្រឡប់ NULL;
}


static const var_na *min_expand (MatchState *ms, const var_na *s,
                                 const var_na *p, const var_na *ep) {
  សម្រាប់ (;;) {
    const var_na *res = match(ms, s, ep+1);
    ប្រសិនបើ (res != NULL)
      ត្រឡប់ res;
    បើមិនដូច្នេះ ប្រសិនបើ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    បើមិនដូច្នេះ ត្រឡប់ NULL;
  }
}


static const var_na *start_capture (MatchState *ms, const var_na *s,
                                    const var_na *p, ចំនួនគត់ what) {
  const var_na *res;
  ចំនួនគត់ level = ms->level;
  ប្រសិនបើ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  ប្រសិនបើ ((res=match(ms, s, p)) == NULL)  /* match failed? */
    ms->level--;  /* undo capture */
  ត្រឡប់ res;
}


static const var_na *end_capture (MatchState *ms, const var_na *s,
                                  const var_na *p) {
  ចំនួនគត់ l = capture_to_close(ms);
  const var_na *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  ប្រសិនបើ ((res = match(ms, s, p)) == NULL)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  ត្រឡប់ res;
}


static const var_na *match_capture (MatchState *ms, const var_na *s, ចំនួនគត់ l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  ប្រសិនបើ ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    ត្រឡប់ s+len;
  បើមិនដូច្នេះ ត្រឡប់ NULL;
}


static const var_na *match (MatchState *ms, const var_na *s, const var_na *p) {
  ប្រសិនបើ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  ប្រសិនបើ (p != ms->p_end) {  /* end of pattern? */
    switch (*p) {
      pak_sa '(': {  /* start capture */
        ប្រសិនបើ (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        បើមិនដូច្នេះ
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        bha_nga;
      }
      pak_sa ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        bha_nga;
      }
      pak_sa '$': {
        ប្រសិនបើ ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          goto dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : NULL;  /* check end of string */
        bha_nga;
      }
      pak_sa L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        switch (*(p + 1)) {
          pak_sa 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            ប្រសិនបើ (s != NULL) {
              p += 4; goto init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            bha_nga;
          }
          pak_sa 'f': {  /* frontier? */
            const var_na *ep; var_na previous;
            p += 2;
            ប្រសិនបើ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            ប្រសិនបើ (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; goto init;  /* return match(ms, s, ep); */
            }
            s = NULL;  /* match failed */
            bha_nga;
          }
          pak_sa '0': pak_sa '1': pak_sa '2': pak_sa '3':
          pak_sa '4': pak_sa '5': pak_sa '6': pak_sa '7':
          pak_sa '8': pak_sa '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            ប្រសិនបើ (s != NULL) {
              p += 2; goto init;  /* return match(ms, s, p + 2) */
            }
            bha_nga;
          }
          default: goto dflt;
        }
        bha_nga;
      }
      default: dflt: {  /* pattern class plus optional suffix */
        const var_na *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        ប្រសិនបើ (!singlematch(ms, s, p, ep)) {
          ប្រសិនបើ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; goto init;  /* return match(ms, s, ep + 1); */
          }
          បើមិនដូច្នេះ  /* '+' or no suffix */
            s = NULL;  /* fail */
        }
        បើមិនដូច្នេះ {  /* matched once */
          switch (*ep) {  /* handle optional suffix */
            pak_sa '?': {  /* optional */
              const var_na *res;
              ប្រសិនបើ ((res = match(ms, s + 1, ep + 1)) != NULL)
                s = res;
              បើមិនដូច្នេះ {
                p = ep + 1; goto init;  /* else return match(ms, s, ep + 1); */
              }
              bha_nga;
            }
            pak_sa '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            pak_sa '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              bha_nga;
            pak_sa '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              bha_nga;
            default:  /* no suffix */
              s++; p = ep; goto init;  /* return match(ms, s + 1, ep); */
          }
        }
        bha_nga;
      }
    }
  }
  ms->matchdepth++;
  ត្រឡប់ s;
}



static const var_na *lmemfind (const var_na *s1, size_t l1,
                               const var_na *s2, size_t l2) {
  ប្រសិនបើ (l2 == 0) ត្រឡប់ s1;  /* empty strings are everywhere */
  បើមិនដូច្នេះ ប្រសិនបើ (l2 > l1) ត្រឡប់ NULL;  /* avoids a negative 'l1' */
  បើមិនដូច្នេះ {
    const var_na *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    ខណៈពេល (l1 > 0 && (init = (const var_na *)memchr(s1, *s2, l1)) != NULL) {
      init++;   /* 1st char is already checked */
      ប្រសិនបើ (memcmp(init, s2+1, l2) == 0)
        ត្រឡប់ init-1;
      បើមិនដូច្នេះ {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    ត្រឡប់ NULL;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
static ptrdiff_t get_onecapture (MatchState *ms, ចំនួនគត់ i, const var_na *s,
                              const var_na *e, const var_na **cap) {
  ប្រសិនបើ (i >= ms->level) {
    ប្រសិនបើ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    ត្រឡប់ (e - s);
  }
  បើមិនដូច្នេះ {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    ប្រសិនបើ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    បើមិនដូច្នេះ ប្រសិនបើ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    ត្រឡប់ capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
static void push_onecapture (MatchState *ms, ចំនួនគត់ i, const var_na *s,
                                                    const var_na *e) {
  const var_na *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  ប្រសិនបើ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


static ចំនួនគត់ push_captures (MatchState *ms, const var_na *s, const var_na *e) {
  ចំនួនគត់ i;
  ចំនួនគត់ nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  សម្រាប់ (i = 0; i < nlevels; i++)
    push_onecapture(ms, i, s, e);
  ត្រឡប់ nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
static ចំនួនគត់ nospecials (const var_na *p, size_t l) {
  size_t upto = 0;
  do {
    ប្រសិនបើ (strpbrk(p + upto, SPECIALS))
      ត្រឡប់ 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } ខណៈពេល (upto <= l);
  ត្រឡប់ 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
static void prepstate (MatchState *ms, lua_State *L,
                       const var_na *s, size_t ls, const var_na *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
static void reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


static ចំនួនគត់ str_find_aux (lua_State *L, ចំនួនគត់ find) {
  size_t ls, lp;
  const var_na *s = luaL_checklstring(L, 1, &ls);
  const var_na *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  ប្រសិនបើ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    ត្រឡប់ 1;
  }
  /* explicit request or no special characters? */
  ប្រសិនបើ (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    const var_na *s2 = lmemfind(s + init, ls - init, p, lp);
    ប្រសិនបើ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      ត្រឡប់ 2;
    }
  }
  បើមិនដូច្នេះ {
    MatchState ms;
    const var_na *s1 = s + init;
    ចំនួនគត់ anchor = (*p == '^');
    ប្រសិនបើ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    do {
      const var_na *res;
      reprepstate(&ms);
      ប្រសិនបើ ((res=match(&ms, s1, p)) != NULL) {
        ប្រសិនបើ (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          ត្រឡប់ push_captures(&ms, NULL, 0) + 2;
        }
        បើមិនដូច្នេះ
          ត្រឡប់ push_captures(&ms, s1, res);
      }
    } ខណៈពេល (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  ត្រឡប់ 1;
}


static ចំនួនគត់ str_find (lua_State *L) {
  ត្រឡប់ str_find_aux(L, 1);
}


static ចំនួនគត់ str_match (lua_State *L) {
  ត្រឡប់ str_find_aux(L, 0);
}


/* state for 'gmatch' */
typedef sa_nracanaa GMatchState {
  const var_na *src;  /* current position */
  const var_na *p;  /* pattern */
  const var_na *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


static ចំនួនគត់ gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  const var_na *src;
  gm->ms.L = L;
  សម្រាប់ (src = gm->src; src <= gm->ms.src_end; src++) {
    const var_na *e;
    reprepstate(&gm->ms);
    ប្រសិនបើ ((e = match(&gm->ms, src, gm->p)) != NULL && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      ត្រឡប់ push_captures(&gm->ms, src, e);
    }
  }
  ត្រឡប់ 0;  /* not found */
}


static ចំនួនគត់ gmatch (lua_State *L) {
  size_t ls, lp;
  const var_na *s = luaL_checklstring(L, 1, &ls);
  const var_na *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, parimaa_na(GMatchState), 0);
  ប្រសិនបើ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = NULL;
  lua_pushcclosure(L, gmatch_aux, 3);
  ត្រឡប់ 1;
}


static void add_s (MatchState *ms, luaL_Buffer *b, const var_na *s,
                                                   const var_na *e) {
  size_t l;
  lua_State *L = ms->L;
  const var_na *news = lua_tolstring(L, 3, &l);
  const var_na *p;
  ខណៈពេល ((p = (var_na *)memchr(news, L_ESC, l)) != NULL) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    ប្រសិនបើ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    បើមិនដូច្នេះ ប្រសិនបើ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    បើមិនដូច្នេះ ប្រសិនបើ (isdigit(cast_uchar(*p))) {  /* '%n' */
      const var_na *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      ប្រសិនបើ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      បើមិនដូច្នេះ
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    បើមិនដូច្នេះ
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
static ចំនួនគត់ add_value (MatchState *ms, luaL_Buffer *b, const var_na *s,
                                      const var_na *e, ចំនួនគត់ tr) {
  lua_State *L = ms->L;
  switch (tr) {
    pak_sa LUA_TFUNCTION: {  /* call the function */
      ចំនួនគត់ n;
      lua_pushvalue(L, 3);  /* push the function */
      n = push_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      bha_nga;
    }
    pak_sa LUA_TTABLE: {  /* index the table */
      push_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      bha_nga;
    }
    default: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      ត្រឡប់ 1;  /* something changed */
    }
  }
  ប្រសិនបើ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_pop(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    ត្រឡប់ 0;  /* no changes */
  }
  បើមិនដូច្នេះ ប្រសិនបើ (l_unlikely(!lua_isstring(L, -1)))
    ត្រឡប់ luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  បើមិនដូច្នេះ {
    luaL_addvalue(b);  /* add result to accumulator */
    ត្រឡប់ 1;  /* something changed */
  }
}


static ចំនួនគត់ str_gsub (lua_State *L) {
  size_t srcl, lp;
  const var_na *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  const var_na *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  const var_na *lastmatch = NULL;  /* end of last match */
  ចំនួនគត់ tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  ចំនួនគត់ anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  ចំនួនគត់ changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  ប្រសិនបើ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  ខណៈពេល (n < max_s) {
    const var_na *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    ប្រសិនបើ ((e = match(&ms, src, p)) != NULL && e != lastmatch) {  /* match? */
      n++;
      changed = add_value(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    បើមិនដូច្នេះ ប្រសិនបើ (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    បើមិនដូច្នេះ bha_nga;  /* end of subject */
    ប្រសិនបើ (anchor) bha_nga;
  }
  ប្រសិនបើ (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  បើមិនដូច្នេះ {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  ត្រឡប់ 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#ប្រសិនបើ !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(parimaa_na(LUA_NUMBER_FRMLEN)/parimaa_na(var_na))


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
static lua_Number adddigit (var_na *buff, unsigned n, lua_Number x) {
  lua_Number dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  ចំនួនគត់ d = (ចំនួនគត់)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  ត្រឡប់ x - dd;  /* return what is left */
}


static ចំនួនគត់ num2straux (var_na *buff, unsigned sz, lua_Number x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  ប្រសិនបើ (x != x || x == (lua_Number)HUGE_VAL || x == -(lua_Number)HUGE_VAL)
    ត្រឡប់ l_sprintf(buff, sz, LUA_NUMBER_FMT, (LUAI_UACNUMBER)x);
  បើមិនដូច្នេះ ប្រសិនបើ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    ត្រឡប់ l_sprintf(buff, sz, LUA_NUMBER_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  បើមិនដូច្នេះ {
    ចំនួនគត់ e;
    lua_Number m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    ប្រសិនបើ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    ប្រសិនបើ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      do {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } ខណៈពេល (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    ត្រឡប់ cast_int(n);
  }
}


static ចំនួនគត់ lua_number2strx (lua_State *L, var_na *buff, unsigned sz,
                            const var_na *fmt, lua_Number x) {
  ចំនួនគត់ n = num2straux(buff, sz, x);
  ប្រសិនបើ (fmt[SIZELENMOD] == 'A') {
    ចំនួនគត់ i;
    សម្រាប់ (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  បើមិនដូច្នេះ ប្រសិនបើ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    ត្រឡប់ luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  ត្រឡប់ n;
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
#ប្រសិនបើ !defined(L_FMTFLAGSF)

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


static void addquoted (luaL_Buffer *b, const var_na *s, size_t len) {
  luaL_addchar(b, '"');
  ខណៈពេល (len--) {
    ប្រសិនបើ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    បើមិនដូច្នេះ ប្រសិនបើ (iscntrl(cast_uchar(*s))) {
      var_na buff[10];
      ប្រសិនបើ (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, parimaa_na(buff), "\\%d", (ចំនួនគត់)cast_uchar(*s));
      បើមិនដូច្នេះ
        l_sprintf(buff, parimaa_na(buff), "\\%03d", (ចំនួនគត់)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    បើមិនដូច្នេះ
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
static ចំនួនគត់ quotefloat (lua_State *L, var_na *buff, lua_Number n) {
  const var_na *s;  /* for the fixed representations */
  ប្រសិនបើ (n == (lua_Number)HUGE_VAL)  /* inf? */
    s = "1e9999";
  បើមិនដូច្នេះ ប្រសិនបើ (n == -(lua_Number)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  បើមិនដូច្នេះ ប្រសិនបើ (n != n)  /* NaN? */
    s = "(0/0)";
  បើមិនដូច្នេះ {  /* format number as hexadecimal */
    ចំនួនគត់  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_NUMBER_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    ប្រសិនបើ (memchr(buff, '.', cast_uint(nb)) == NULL) {  /* no dot? */
      var_na point = lua_getlocaledecpoint();  /* try locale point */
      var_na *ppoint = (var_na *)memchr(buff, point, cast_uint(nb));
      ប្រសិនបើ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    ត្រឡប់ nb;
  }
  /* for the fixed representations */
  ត្រឡប់ l_sprintf(buff, MAX_ITEM, "%s", s);
}


static void addliteral (lua_State *L, luaL_Buffer *b, ចំនួនគត់ arg) {
  switch (lua_type(L, arg)) {
    pak_sa LUA_TSTRING: {
      size_t len;
      const var_na *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      bha_nga;
    }
    pak_sa LUA_TNUMBER: {
      var_na *buff = luaL_prepbuffsize(b, MAX_ITEM);
      ចំនួនគត់ nb;
      ប្រសិនបើ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      បើមិនដូច្នេះ {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        const var_na *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      bha_nga;
    }
    pak_sa LUA_TNIL: pak_sa LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, NULL);
      luaL_addvalue(b);
      bha_nga;
    }
    default: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


static const var_na *get2digits (const var_na *s) {
  ប្រសិនបើ (isdigit(cast_uchar(*s))) {
    s++;
    ប្រសិនបើ (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  ត្រឡប់ s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
static void checkformat (lua_State *L, const var_na *form, const var_na *flags,
                                       ចំនួនគត់ precision) {
  const var_na *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  ប្រសិនបើ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    ប្រសិនបើ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  ប្រសិនបើ (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
static const var_na *getformat (lua_State *L, const var_na *strfrmt,
                                            var_na *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  ប្រសិនបើ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * parimaa_na(var_na));
  *(form + len) = '\0';
  ត្រឡប់ strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
static void addlenmod (var_na *form, const var_na *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  var_na spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


static ចំនួនគត់ str_format (lua_State *L) {
  ចំនួនគត់ top = lua_gettop(L);
  ចំនួនគត់ arg = 1;
  size_t sfl;
  const var_na *strfrmt = luaL_checklstring(L, arg, &sfl);
  const var_na *strfrmt_end = strfrmt+sfl;
  const var_na *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  ខណៈពេល (strfrmt < strfrmt_end) {
    ប្រសិនបើ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    បើមិនដូច្នេះ ប្រសិនបើ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    បើមិនដូច្នេះ { /* format item */
      var_na form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      var_na *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      ចំនួនគត់ nb = 0;  /* number of bytes in result */
      ប្រសិនបើ (++arg > top)
        ត្រឡប់ luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      switch (*strfrmt++) {
        pak_sa 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (ចំនួនគត់)luaL_checkinteger(L, arg));
          bha_nga;
        }
        pak_sa 'd': pak_sa 'i':
          flags = L_FMTFLAGSI;
          goto intcase;
        pak_sa 'u':
          flags = L_FMTFLAGSU;
          goto intcase;
        pak_sa 'o': pak_sa 'x': pak_sa 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          bha_nga;
        }
        pak_sa 'a': pak_sa 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          bha_nga;
        pak_sa 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        pak_sa 'e': pak_sa 'E': pak_sa 'g': pak_sa 'G': {
          lua_Number n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_NUMBER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          bha_nga;
        }
        pak_sa 'p': {
          const void *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          ប្រសិនបើ (p == NULL) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          bha_nga;
        }
        pak_sa 'q': {
          ប្រសិនបើ (form[2] != '\0')  /* modifiers? */
            ត្រឡប់ luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          bha_nga;
        }
        pak_sa 's': {
          size_t l;
          const var_na *s = luaL_tolstring(L, arg, &l);
          ប្រសិនបើ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          បើមិនដូច្នេះ {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            ប្រសិនបើ (strchr(form, '.') == NULL && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            បើមិនដូច្នេះ {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_pop(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          bha_nga;
        }
        default: {  /* also treat cases 'pnLlh' */
          ត្រឡប់ luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  ត្រឡប់ 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#ប្រសិនបើ !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((ចំនួនគត់)parimaa_na(lua_Integer))


/* dummy union to get native endianness */
static const sa_ngha {
  ចំនួនគត់ dummy;
  var_na little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
typedef sa_nracanaa Header {
  lua_State *L;
  ចំនួនគត់ islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
typedef ga_nanaa KOption {
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
static ចំនួនគត់ digit (ចំនួនគត់ c) { ត្រឡប់ '0' <= c && c <= '9'; }

static size_t getnum (const var_na **fmt, size_t df) {
  ប្រសិនបើ (!digit(**fmt))  /* no number? */
    ត្រឡប់ df;  /* return default value */
  បើមិនដូច្នេះ {
    size_t a = 0;
    do {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } ខណៈពេល (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    ត្រឡប់ a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
static unsigned getnumlimit (Header *h, const var_na **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  ប្រសិនបើ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    ត្រឡប់ cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  ត្រឡប់ cast_uint(sz);
}


/*
** Initialize Header
*/
static void initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
static KOption getoption (Header *h, const var_na **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  sa_nracanaa cD { var_na c; sa_ngha { LUAI_MAXALIGN; } u; };
  ចំនួនគត់ opt = *((*fmt)++);
  *size = 0;  /* default */
  switch (opt) {
    pak_sa 'b': *size = parimaa_na(var_na); ត្រឡប់ Kint;
    pak_sa 'B': *size = parimaa_na(var_na); ត្រឡប់ Kuint;
    pak_sa 'h': *size = parimaa_na(short); ត្រឡប់ Kint;
    pak_sa 'H': *size = parimaa_na(short); ត្រឡប់ Kuint;
    pak_sa 'l': *size = parimaa_na(long); ត្រឡប់ Kint;
    pak_sa 'L': *size = parimaa_na(long); ត្រឡប់ Kuint;
    pak_sa 'j': *size = parimaa_na(lua_Integer); ត្រឡប់ Kint;
    pak_sa 'J': *size = parimaa_na(lua_Integer); ត្រឡប់ Kuint;
    pak_sa 'T': *size = parimaa_na(size_t); ត្រឡប់ Kuint;
    pak_sa 'f': *size = parimaa_na(float); ត្រឡប់ Kfloat;
    pak_sa 'n': *size = parimaa_na(lua_Number); ត្រឡប់ Knumber;
    pak_sa 'd': *size = parimaa_na(dviguu_na); ត្រឡប់ Kdouble;
    pak_sa 'i': *size = getnumlimit(h, fmt, parimaa_na(ចំនួនគត់)); ត្រឡប់ Kint;
    pak_sa 'I': *size = getnumlimit(h, fmt, parimaa_na(ចំនួនគត់)); ត្រឡប់ Kuint;
    pak_sa 's': *size = getnumlimit(h, fmt, parimaa_na(size_t)); ត្រឡប់ Kstring;
    pak_sa 'c':
      *size = getnum(fmt, cast_sizet(-1));
      ប្រសិនបើ (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      ត្រឡប់ Kchar;
    pak_sa 'z': ត្រឡប់ Kzstr;
    pak_sa 'x': *size = 1; ត្រឡប់ Kpadding;
    pak_sa 'X': ត្រឡប់ Kpaddalign;
    pak_sa ' ': bha_nga;
    pak_sa '<': h->islittle = 1; bha_nga;
    pak_sa '>': h->islittle = 0; bha_nga;
    pak_sa '=': h->islittle = nativeendian.little; bha_nga;
    pak_sa '!': {
      const size_t maxalign = offsetof(sa_nracanaa cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      bha_nga;
    }
    default: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  ត្រឡប់ Knop;
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
static KOption getdetails (Header *h, size_t totalsize, const var_na **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  ប្រសិនបើ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    ប្រសិនបើ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  ប្រសិនបើ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  បើមិនដូច្នេះ {
    ប្រសិនបើ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    ប្រសិនបើ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    បើមិនដូច្នេះ {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  ត្រឡប់ opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
static void packint (luaL_Buffer *b, lua_Unsigned n,
                     ចំនួនគត់ islittle, unsigned size, ចំនួនគត់ neg) {
  var_na *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (var_na)(n & MC);  /* first byte */
  សម្រាប់ (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (var_na)(n & MC);
  }
  ប្រសិនបើ (neg && size > SZINT) {  /* negative number need sign extension? */
    សម្រាប់ (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (var_na)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
static void copywithendian (var_na *dest, const var_na *src,
                            unsigned size, ចំនួនគត់ islittle) {
  ប្រសិនបើ (islittle == nativeendian.little)
    memcpy(dest, src, size);
  បើមិនដូច្នេះ {
    dest += size - 1;
    ខណៈពេល (size-- != 0)
      *(dest--) = *(src++);
  }
}


static ចំនួនគត់ str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  const var_na *fmt = luaL_checkstring(L, 1);  /* format string */
  ចំនួនគត់ arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  ខណៈពេល (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    ខណៈពេល (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    switch (opt) {
      pak_sa Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ប្រសិនបើ (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        bha_nga;
      }
      pak_sa Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ប្រសិនបើ (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        bha_nga;
      }
      pak_sa Kfloat: {  /* C float */
        float f = (float)luaL_checknumber(L, arg);  /* get argument */
        var_na *buff = luaL_prepbuffsize(&b, parimaa_na(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (var_na *)&f, parimaa_na(f), h.islittle);
        luaL_addsize(&b, size);
        bha_nga;
      }
      pak_sa Knumber: {  /* Lua float */
        lua_Number f = luaL_checknumber(L, arg);  /* get argument */
        var_na *buff = luaL_prepbuffsize(&b, parimaa_na(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (var_na *)&f, parimaa_na(f), h.islittle);
        luaL_addsize(&b, size);
        bha_nga;
      }
      pak_sa Kdouble: {  /* C double */
        dviguu_na f = (dviguu_na)luaL_checknumber(L, arg);  /* get argument */
        var_na *buff = luaL_prepbuffsize(&b, parimaa_na(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (var_na *)&f, parimaa_na(f), h.islittle);
        luaL_addsize(&b, size);
        bha_nga;
      }
      pak_sa Kchar: {  /* fixed-size string */
        size_t len;
        const var_na *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        ប្រសិនបើ (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          var_na *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        bha_nga;
      }
      pak_sa Kstring: {  /* strings with length count */
        size_t len;
        const var_na *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= parimaa_na(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        bha_nga;
      }
      pak_sa Kzstr: {  /* zero-terminated string */
        size_t len;
        const var_na *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        bha_nga;
      }
      pak_sa Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      pak_sa Kpaddalign: pak_sa Knop:
        arg--;  /* undo increment */
        bha_nga;
    }
  }
  luaL_pushresult(&b);
  ត្រឡប់ 1;
}


static ចំនួនគត់ str_packsize (lua_State *L) {
  Header h;
  const var_na *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  ខណៈពេល (*fmt != '\0') {
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
  ត្រឡប់ 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
static lua_Integer unpackint (lua_State *L, const var_na *str,
                              ចំនួនគត់ islittle, ចំនួនគត់ size, ចំនួនគត់ issigned) {
  lua_Unsigned res = 0;
  ចំនួនគត់ i;
  ចំនួនគត់ limit = (size  <= SZINT) ? size : SZINT;
  សម្រាប់ (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned var_na)str[islittle ? i : size - 1 - i];
  }
  ប្រសិនបើ (size < SZINT) {  /* real size smaller than lua_Integer? */
    ប្រសិនបើ (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  បើមិនដូច្នេះ ប្រសិនបើ (size > SZINT) {  /* must check unread bytes */
    ចំនួនគត់ mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    សម្រាប់ (i = limit; i < size; i++) {
      ប្រសិនបើ (l_unlikely((unsigned var_na)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  ត្រឡប់ (lua_Integer)res;
}


static ចំនួនគត់ str_unpack (lua_State *L) {
  Header h;
  const var_na *fmt = luaL_checkstring(L, 1);
  size_t ld;
  const var_na *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  ចំនួនគត់ n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  ខណៈពេល (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    switch (opt) {
      pak_sa Kint:
      pak_sa Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        bha_nga;
      }
      pak_sa Kfloat: {
        float f;
        copywithendian((var_na *)&f, data + pos, parimaa_na(f), h.islittle);
        lua_pushnumber(L, (lua_Number)f);
        bha_nga;
      }
      pak_sa Knumber: {
        lua_Number f;
        copywithendian((var_na *)&f, data + pos, parimaa_na(f), h.islittle);
        lua_pushnumber(L, f);
        bha_nga;
      }
      pak_sa Kdouble: {
        dviguu_na f;
        copywithendian((var_na *)&f, data + pos, parimaa_na(f), h.islittle);
        lua_pushnumber(L, (lua_Number)f);
        bha_nga;
      }
      pak_sa Kchar: {
        lua_pushlstring(L, data + pos, size);
        bha_nga;
      }
      pak_sa Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        bha_nga;
      }
      pak_sa Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        bha_nga;
      }
      pak_sa Kpaddalign: pak_sa Kpadding: pak_sa Knop:
        n--;  /* undo increment */
        bha_nga;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  ត្រឡប់ n + 1;
}

/* }====================================================== */


static const luaL_Reg strlib[] = {
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
  {NULL, NULL}
};


static void createmetatable (lua_State *L) {
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
LUAMOD_API ចំនួនគត់ luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  ត្រឡប់ 1;
}

