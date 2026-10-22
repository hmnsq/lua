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
#အကယ်၍ !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


static ကိန်းပြည့် str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  ပြန်ပေး 1;
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
  အကယ်၍ (pos > 0)
    ပြန်ပေး (size_t)pos;
  မဟုတ်ပါက အကယ်၍ (pos == 0)
    ပြန်ပေး 1;
  မဟုတ်ပါက အကယ်၍ (pos < -(lua_Integer)len)  /* inverted comparison */
    ပြန်ပေး 1;  /* clip to 1 */
  မဟုတ်ပါက ပြန်ပေး len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
static size_t getendpos (lua_State *L, ကိန်းပြည့် arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  အကယ်၍ (pos > (lua_Integer)len)
    ပြန်ပေး len;
  မဟုတ်ပါက အကယ်၍ (pos >= 0)
    ပြန်ပေး (size_t)pos;
  မဟုတ်ပါက အကယ်၍ (pos < -(lua_Integer)len)
    ပြန်ပေး 0;
  မဟုတ်ပါက ပြန်ပေး len + (size_t)pos + 1;
}


static ကိန်းပြည့် str_sub (lua_State *L) {
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  အကယ်၍ (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  မဟုတ်ပါက lua_pushliteral(L, "");
  ပြန်ပေး 1;
}


static ကိန်းပြည့် str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  const var_na *s = luaL_checklstring(L, 1, &l);
  var_na *p = luaL_buffinitsize(L, &b, l);
  အတွက် (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const var_na *s = luaL_checklstring(L, 1, &l);
  var_na *p = luaL_buffinitsize(L, &b, l);
  အတွက် (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const var_na *s = luaL_checklstring(L, 1, &l);
  var_na *p = luaL_buffinitsize(L, &b, l);
  အတွက် (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ပြန်ပေး 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
static ကိန်းပြည့် str_rep (lua_State *L) {
  size_t len, lsep;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  const var_na *sep = luaL_optlstring(L, 3, "", &lsep);
  အကယ်၍ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  မဟုတ်ပါက အကယ်၍ (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    ပြန်ပေး luaL_error(L, "resulting string too large");
  မဟုတ်ပါက {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    var_na *p = luaL_buffinitsize(L, &b, totallen);
    နေစဉ် (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * parimaa_na(var_na)); p += len;
      အကယ်၍ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * parimaa_na(var_na)); p += lsep;
      }
    }
    memcpy(p, s, len * parimaa_na(var_na));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် str_byte (lua_State *L) {
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  ကိန်းပြည့် n, i;
  အကယ်၍ (posi > pose) ပြန်ပေး 0;  /* empty interval; return no values */
  အကယ်၍ (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    ပြန်ပေး luaL_error(L, "string slice too long");
  n = (ကိန်းပြည့်)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  အတွက် (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  ပြန်ပေး n;
}


static ကိန်းပြည့် str_char (lua_State *L) {
  ကိန်းပြည့် n = lua_gettop(L);  /* number of arguments */
  ကိန်းပြည့် i;
  luaL_Buffer b;
  var_na *p = luaL_buffinitsize(L, &b, cast_uint(n));
  အတွက် (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  ပြန်ပေး 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
sa_nracanaa str_Writer {
  ကိန်းပြည့် init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


static ကိန်းပြည့် writer (lua_State *L, const void *b, size_t size, void *ud) {
  sa_nracanaa str_Writer *state = (sa_nracanaa str_Writer *)ud;
  အကယ်၍ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  အကယ်၍ (b == NULL) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  မဟုတ်ပါက
    luaL_addlstring(&state->B, (const var_na *)b, size);
  ပြန်ပေး 0;
}


static ကိန်းပြည့် str_dump (lua_State *L) {
  sa_nracanaa str_Writer state;
  ကိန်းပြည့် strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  ပြန်ပေး 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#အကယ်၍ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

static const luaL_Reg stringmetamethods[] = {
  {"__index", NULL},  /* placeholder */
  {NULL, NULL}
};

#မဟုတ်ပါက		/* }{ */

static ကိန်းပြည့် tonum (lua_State *L, ကိန်းပြည့် arg) {
  အကယ်၍ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    ပြန်ပေး 1;
  }
  မဟုတ်ပါက {  /* check whether it is a numerical string */
    size_t len;
    const var_na *s = lua_tolstring(L, arg, &len);
    ပြန်ပေး (s != NULL && lua_stringtonumber(L, s) == len + 1);
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
  အကယ်၍ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_insert(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


static ကိန်းပြည့် arith (lua_State *L, ကိန်းပြည့် op, const var_na *mtname) {
  အကယ်၍ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  မဟုတ်ပါက
    trymt(L, mtname, mtname + 2);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် arith_add (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPADD, "__add");
}

static ကိန်းပြည့် arith_sub (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPSUB, "__sub");
}

static ကိန်းပြည့် arith_mul (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPMUL, "__mul");
}

static ကိန်းပြည့် arith_mod (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPMOD, "__mod");
}

static ကိန်းပြည့် arith_pow (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPPOW, "__pow");
}

static ကိန်းပြည့် arith_div (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPDIV, "__div");
}

static ကိန်းပြည့် arith_idiv (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPIDIV, "__idiv");
}

static ကိန်းပြည့် arith_unm (lua_State *L) {
  ပြန်ပေး arith(L, LUA_OPUNM, "__unm");
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
  ကိန်းပြည့် matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  ကိန်းပြည့် level;  /* total number of captures (finished or unfinished) */
  sa_nracanaa {
    const var_na *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
static const var_na *match (MatchState *ms, const var_na *s, const var_na *p);


/* maximum recursion depth for 'match' */
#အကယ်၍ !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


static ကိန်းပြည့် check_capture (MatchState *ms, ကိန်းပြည့် l) {
  l -= '1';
  အကယ်၍ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    ပြန်ပေး luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  ပြန်ပေး l;
}


static ကိန်းပြည့် capture_to_close (MatchState *ms) {
  ကိန်းပြည့် level = ms->level;
  အတွက် (level--; level>=0; level--)
    အကယ်၍ (ms->capture[level].len == CAP_UNFINISHED) ပြန်ပေး level;
  ပြန်ပေး luaL_error(ms->L, "invalid pattern capture");
}


static const var_na *classend (MatchState *ms, const var_na *p) {
  switch (*p++) {
    pak_sa L_ESC: {
      အကယ်၍ (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      ပြန်ပေး p+1;
    }
    pak_sa '[': {
      အကယ်၍ (*p == '^') p++;
      do {  /* look for a ']' */
        အကယ်၍ (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        အကယ်၍ (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } နေစဉ် (*p != ']');
      ပြန်ပေး p+1;
    }
    default: {
      ပြန်ပေး p;
    }
  }
}


static ကိန်းပြည့် match_class (ကိန်းပြည့် c, ကိန်းပြည့် cl) {
  ကိန်းပြည့် res;
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
    default: ပြန်ပေး (cl == c);
  }
  ပြန်ပေး (islower(cl) ? res : !res);
}


static ကိန်းပြည့် matchbracketclass (ကိန်းပြည့် c, const var_na *p, const var_na *ec) {
  ကိန်းပြည့် sig = 1;
  အကယ်၍ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  နေစဉ် (++p < ec) {
    အကယ်၍ (*p == L_ESC) {
      p++;
      အကယ်၍ (match_class(c, cast_uchar(*p)))
        ပြန်ပေး sig;
    }
    မဟုတ်ပါက အကယ်၍ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      အကယ်၍ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        ပြန်ပေး sig;
    }
    မဟုတ်ပါက အကယ်၍ (cast_uchar(*p) == c) ပြန်ပေး sig;
  }
  ပြန်ပေး !sig;
}


static ကိန်းပြည့် singlematch (MatchState *ms, const var_na *s, const var_na *p,
                        const var_na *ep) {
  အကယ်၍ (s >= ms->src_end)
    ပြန်ပေး 0;
  မဟုတ်ပါက {
    ကိန်းပြည့် c = cast_uchar(*s);
    switch (*p) {
      pak_sa '.': ပြန်ပေး 1;  /* matches any char */
      pak_sa L_ESC: ပြန်ပေး match_class(c, cast_uchar(*(p+1)));
      pak_sa '[': ပြန်ပေး matchbracketclass(c, p, ep-1);
      default:  ပြန်ပေး (cast_uchar(*p) == c);
    }
  }
}


static const var_na *matchbalance (MatchState *ms, const var_na *s,
                                   const var_na *p) {
  အကယ်၍ (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  အကယ်၍ (*s != *p) ပြန်ပေး NULL;
  မဟုတ်ပါက {
    ကိန်းပြည့် b = *p;
    ကိန်းပြည့် e = *(p+1);
    ကိန်းပြည့် cont = 1;
    နေစဉ် (++s < ms->src_end) {
      အကယ်၍ (*s == e) {
        အကယ်၍ (--cont == 0) ပြန်ပေး s+1;
      }
      မဟုတ်ပါက အကယ်၍ (*s == b) cont++;
    }
  }
  ပြန်ပေး NULL;  /* string ends out of balance */
}


static const var_na *max_expand (MatchState *ms, const var_na *s,
                                 const var_na *p, const var_na *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  နေစဉ် (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  နေစဉ် (i>=0) {
    const var_na *res = match(ms, (s+i), ep+1);
    အကယ်၍ (res) ပြန်ပေး res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  ပြန်ပေး NULL;
}


static const var_na *min_expand (MatchState *ms, const var_na *s,
                                 const var_na *p, const var_na *ep) {
  အတွက် (;;) {
    const var_na *res = match(ms, s, ep+1);
    အကယ်၍ (res != NULL)
      ပြန်ပေး res;
    မဟုတ်ပါက အကယ်၍ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    မဟုတ်ပါက ပြန်ပေး NULL;
  }
}


static const var_na *start_capture (MatchState *ms, const var_na *s,
                                    const var_na *p, ကိန်းပြည့် what) {
  const var_na *res;
  ကိန်းပြည့် level = ms->level;
  အကယ်၍ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  အကယ်၍ ((res=match(ms, s, p)) == NULL)  /* match failed? */
    ms->level--;  /* undo capture */
  ပြန်ပေး res;
}


static const var_na *end_capture (MatchState *ms, const var_na *s,
                                  const var_na *p) {
  ကိန်းပြည့် l = capture_to_close(ms);
  const var_na *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  အကယ်၍ ((res = match(ms, s, p)) == NULL)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  ပြန်ပေး res;
}


static const var_na *match_capture (MatchState *ms, const var_na *s, ကိန်းပြည့် l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  အကယ်၍ ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    ပြန်ပေး s+len;
  မဟုတ်ပါက ပြန်ပေး NULL;
}


static const var_na *match (MatchState *ms, const var_na *s, const var_na *p) {
  အကယ်၍ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  အကယ်၍ (p != ms->p_end) {  /* end of pattern? */
    switch (*p) {
      pak_sa '(': {  /* start capture */
        အကယ်၍ (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        မဟုတ်ပါက
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        bha_nga;
      }
      pak_sa ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        bha_nga;
      }
      pak_sa '$': {
        အကယ်၍ ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          goto dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : NULL;  /* check end of string */
        bha_nga;
      }
      pak_sa L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        switch (*(p + 1)) {
          pak_sa 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            အကယ်၍ (s != NULL) {
              p += 4; goto init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            bha_nga;
          }
          pak_sa 'f': {  /* frontier? */
            const var_na *ep; var_na previous;
            p += 2;
            အကယ်၍ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            အကယ်၍ (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
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
            အကယ်၍ (s != NULL) {
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
        အကယ်၍ (!singlematch(ms, s, p, ep)) {
          အကယ်၍ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; goto init;  /* return match(ms, s, ep + 1); */
          }
          မဟုတ်ပါက  /* '+' or no suffix */
            s = NULL;  /* fail */
        }
        မဟုတ်ပါက {  /* matched once */
          switch (*ep) {  /* handle optional suffix */
            pak_sa '?': {  /* optional */
              const var_na *res;
              အကယ်၍ ((res = match(ms, s + 1, ep + 1)) != NULL)
                s = res;
              မဟုတ်ပါက {
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
  ပြန်ပေး s;
}



static const var_na *lmemfind (const var_na *s1, size_t l1,
                               const var_na *s2, size_t l2) {
  အကယ်၍ (l2 == 0) ပြန်ပေး s1;  /* empty strings are everywhere */
  မဟုတ်ပါက အကယ်၍ (l2 > l1) ပြန်ပေး NULL;  /* avoids a negative 'l1' */
  မဟုတ်ပါက {
    const var_na *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    နေစဉ် (l1 > 0 && (init = (const var_na *)memchr(s1, *s2, l1)) != NULL) {
      init++;   /* 1st char is already checked */
      အကယ်၍ (memcmp(init, s2+1, l2) == 0)
        ပြန်ပေး init-1;
      မဟုတ်ပါက {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    ပြန်ပေး NULL;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
static ptrdiff_t get_onecapture (MatchState *ms, ကိန်းပြည့် i, const var_na *s,
                              const var_na *e, const var_na **cap) {
  အကယ်၍ (i >= ms->level) {
    အကယ်၍ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    ပြန်ပေး (e - s);
  }
  မဟုတ်ပါက {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    အကယ်၍ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    မဟုတ်ပါက အကယ်၍ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    ပြန်ပေး capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
static void push_onecapture (MatchState *ms, ကိန်းပြည့် i, const var_na *s,
                                                    const var_na *e) {
  const var_na *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  အကယ်၍ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


static ကိန်းပြည့် push_captures (MatchState *ms, const var_na *s, const var_na *e) {
  ကိန်းပြည့် i;
  ကိန်းပြည့် nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  အတွက် (i = 0; i < nlevels; i++)
    push_onecapture(ms, i, s, e);
  ပြန်ပေး nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
static ကိန်းပြည့် nospecials (const var_na *p, size_t l) {
  size_t upto = 0;
  do {
    အကယ်၍ (strpbrk(p + upto, SPECIALS))
      ပြန်ပေး 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } နေစဉ် (upto <= l);
  ပြန်ပေး 1;  /* no special chars found */
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


static ကိန်းပြည့် str_find_aux (lua_State *L, ကိန်းပြည့် find) {
  size_t ls, lp;
  const var_na *s = luaL_checklstring(L, 1, &ls);
  const var_na *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  အကယ်၍ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    ပြန်ပေး 1;
  }
  /* explicit request or no special characters? */
  အကယ်၍ (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    const var_na *s2 = lmemfind(s + init, ls - init, p, lp);
    အကယ်၍ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      ပြန်ပေး 2;
    }
  }
  မဟုတ်ပါက {
    MatchState ms;
    const var_na *s1 = s + init;
    ကိန်းပြည့် anchor = (*p == '^');
    အကယ်၍ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    do {
      const var_na *res;
      reprepstate(&ms);
      အကယ်၍ ((res=match(&ms, s1, p)) != NULL) {
        အကယ်၍ (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          ပြန်ပေး push_captures(&ms, NULL, 0) + 2;
        }
        မဟုတ်ပါက
          ပြန်ပေး push_captures(&ms, s1, res);
      }
    } နေစဉ် (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  ပြန်ပေး 1;
}


static ကိန်းပြည့် str_find (lua_State *L) {
  ပြန်ပေး str_find_aux(L, 1);
}


static ကိန်းပြည့် str_match (lua_State *L) {
  ပြန်ပေး str_find_aux(L, 0);
}


/* state for 'gmatch' */
typedef sa_nracanaa GMatchState {
  const var_na *src;  /* current position */
  const var_na *p;  /* pattern */
  const var_na *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


static ကိန်းပြည့် gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  const var_na *src;
  gm->ms.L = L;
  အတွက် (src = gm->src; src <= gm->ms.src_end; src++) {
    const var_na *e;
    reprepstate(&gm->ms);
    အကယ်၍ ((e = match(&gm->ms, src, gm->p)) != NULL && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      ပြန်ပေး push_captures(&gm->ms, src, e);
    }
  }
  ပြန်ပေး 0;  /* not found */
}


static ကိန်းပြည့် gmatch (lua_State *L) {
  size_t ls, lp;
  const var_na *s = luaL_checklstring(L, 1, &ls);
  const var_na *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, parimaa_na(GMatchState), 0);
  အကယ်၍ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = NULL;
  lua_pushcclosure(L, gmatch_aux, 3);
  ပြန်ပေး 1;
}


static void add_s (MatchState *ms, luaL_Buffer *b, const var_na *s,
                                                   const var_na *e) {
  size_t l;
  lua_State *L = ms->L;
  const var_na *news = lua_tolstring(L, 3, &l);
  const var_na *p;
  နေစဉ် ((p = (var_na *)memchr(news, L_ESC, l)) != NULL) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    အကယ်၍ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    မဟုတ်ပါက အကယ်၍ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    မဟုတ်ပါက အကယ်၍ (isdigit(cast_uchar(*p))) {  /* '%n' */
      const var_na *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      အကယ်၍ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      မဟုတ်ပါက
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    မဟုတ်ပါက
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
static ကိန်းပြည့် add_value (MatchState *ms, luaL_Buffer *b, const var_na *s,
                                      const var_na *e, ကိန်းပြည့် tr) {
  lua_State *L = ms->L;
  switch (tr) {
    pak_sa LUA_TFUNCTION: {  /* call the function */
      ကိန်းပြည့် n;
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
      ပြန်ပေး 1;  /* something changed */
    }
  }
  အကယ်၍ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_pop(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    ပြန်ပေး 0;  /* no changes */
  }
  မဟုတ်ပါက အကယ်၍ (l_unlikely(!lua_isstring(L, -1)))
    ပြန်ပေး luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  မဟုတ်ပါက {
    luaL_addvalue(b);  /* add result to accumulator */
    ပြန်ပေး 1;  /* something changed */
  }
}


static ကိန်းပြည့် str_gsub (lua_State *L) {
  size_t srcl, lp;
  const var_na *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  const var_na *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  const var_na *lastmatch = NULL;  /* end of last match */
  ကိန်းပြည့် tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  ကိန်းပြည့် anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  ကိန်းပြည့် changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  အကယ်၍ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  နေစဉ် (n < max_s) {
    const var_na *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    အကယ်၍ ((e = match(&ms, src, p)) != NULL && e != lastmatch) {  /* match? */
      n++;
      changed = add_value(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    မဟုတ်ပါက အကယ်၍ (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    မဟုတ်ပါက bha_nga;  /* end of subject */
    အကယ်၍ (anchor) bha_nga;
  }
  အကယ်၍ (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  မဟုတ်ပါက {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  ပြန်ပေး 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#အကယ်၍ !defined(lua_number2strx)	/* { */

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
  ကိန်းပြည့် d = (ကိန်းပြည့်)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  ပြန်ပေး x - dd;  /* return what is left */
}


static ကိန်းပြည့် num2straux (var_na *buff, unsigned sz, lua_Number x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  အကယ်၍ (x != x || x == (lua_Number)HUGE_VAL || x == -(lua_Number)HUGE_VAL)
    ပြန်ပေး l_sprintf(buff, sz, LUA_NUMBER_FMT, (LUAI_UACNUMBER)x);
  မဟုတ်ပါက အကယ်၍ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    ပြန်ပေး l_sprintf(buff, sz, LUA_NUMBER_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  မဟုတ်ပါက {
    ကိန်းပြည့် e;
    lua_Number m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    အကယ်၍ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    အကယ်၍ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      do {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } နေစဉ် (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    ပြန်ပေး cast_int(n);
  }
}


static ကိန်းပြည့် lua_number2strx (lua_State *L, var_na *buff, unsigned sz,
                            const var_na *fmt, lua_Number x) {
  ကိန်းပြည့် n = num2straux(buff, sz, x);
  အကယ်၍ (fmt[SIZELENMOD] == 'A') {
    ကိန်းပြည့် i;
    အတွက် (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  မဟုတ်ပါက အကယ်၍ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    ပြန်ပေး luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  ပြန်ပေး n;
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
#အကယ်၍ !defined(L_FMTFLAGSF)

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
  နေစဉ် (len--) {
    အကယ်၍ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    မဟုတ်ပါက အကယ်၍ (iscntrl(cast_uchar(*s))) {
      var_na buff[10];
      အကယ်၍ (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, parimaa_na(buff), "\\%d", (ကိန်းပြည့်)cast_uchar(*s));
      မဟုတ်ပါက
        l_sprintf(buff, parimaa_na(buff), "\\%03d", (ကိန်းပြည့်)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    မဟုတ်ပါက
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
static ကိန်းပြည့် quotefloat (lua_State *L, var_na *buff, lua_Number n) {
  const var_na *s;  /* for the fixed representations */
  အကယ်၍ (n == (lua_Number)HUGE_VAL)  /* inf? */
    s = "1e9999";
  မဟုတ်ပါက အကယ်၍ (n == -(lua_Number)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  မဟုတ်ပါက အကယ်၍ (n != n)  /* NaN? */
    s = "(0/0)";
  မဟုတ်ပါက {  /* format number as hexadecimal */
    ကိန်းပြည့်  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_NUMBER_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    အကယ်၍ (memchr(buff, '.', cast_uint(nb)) == NULL) {  /* no dot? */
      var_na point = lua_getlocaledecpoint();  /* try locale point */
      var_na *ppoint = (var_na *)memchr(buff, point, cast_uint(nb));
      အကယ်၍ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    ပြန်ပေး nb;
  }
  /* for the fixed representations */
  ပြန်ပေး l_sprintf(buff, MAX_ITEM, "%s", s);
}


static void addliteral (lua_State *L, luaL_Buffer *b, ကိန်းပြည့် arg) {
  switch (lua_type(L, arg)) {
    pak_sa LUA_TSTRING: {
      size_t len;
      const var_na *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      bha_nga;
    }
    pak_sa LUA_TNUMBER: {
      var_na *buff = luaL_prepbuffsize(b, MAX_ITEM);
      ကိန်းပြည့် nb;
      အကယ်၍ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      မဟုတ်ပါက {  /* integers */
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
  အကယ်၍ (isdigit(cast_uchar(*s))) {
    s++;
    အကယ်၍ (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  ပြန်ပေး s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
static void checkformat (lua_State *L, const var_na *form, const var_na *flags,
                                       ကိန်းပြည့် precision) {
  const var_na *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  အကယ်၍ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    အကယ်၍ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  အကယ်၍ (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
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
  အကယ်၍ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * parimaa_na(var_na));
  *(form + len) = '\0';
  ပြန်ပေး strfrmt + len - 1;
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


static ကိန်းပြည့် str_format (lua_State *L) {
  ကိန်းပြည့် top = lua_gettop(L);
  ကိန်းပြည့် arg = 1;
  size_t sfl;
  const var_na *strfrmt = luaL_checklstring(L, arg, &sfl);
  const var_na *strfrmt_end = strfrmt+sfl;
  const var_na *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  နေစဉ် (strfrmt < strfrmt_end) {
    အကယ်၍ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    မဟုတ်ပါက အကယ်၍ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    မဟုတ်ပါက { /* format item */
      var_na form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      var_na *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      ကိန်းပြည့် nb = 0;  /* number of bytes in result */
      အကယ်၍ (++arg > top)
        ပြန်ပေး luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      switch (*strfrmt++) {
        pak_sa 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (ကိန်းပြည့်)luaL_checkinteger(L, arg));
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
          အကယ်၍ (p == NULL) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          bha_nga;
        }
        pak_sa 'q': {
          အကယ်၍ (form[2] != '\0')  /* modifiers? */
            ပြန်ပေး luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          bha_nga;
        }
        pak_sa 's': {
          size_t l;
          const var_na *s = luaL_tolstring(L, arg, &l);
          အကယ်၍ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          မဟုတ်ပါက {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            အကယ်၍ (strchr(form, '.') == NULL && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            မဟုတ်ပါက {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_pop(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          bha_nga;
        }
        default: {  /* also treat cases 'pnLlh' */
          ပြန်ပေး luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  ပြန်ပေး 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#အကယ်၍ !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((ကိန်းပြည့်)parimaa_na(lua_Integer))


/* dummy union to get native endianness */
static const sa_ngha {
  ကိန်းပြည့် dummy;
  var_na little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
typedef sa_nracanaa Header {
  lua_State *L;
  ကိန်းပြည့် islittle;
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
static ကိန်းပြည့် digit (ကိန်းပြည့် c) { ပြန်ပေး '0' <= c && c <= '9'; }

static size_t getnum (const var_na **fmt, size_t df) {
  အကယ်၍ (!digit(**fmt))  /* no number? */
    ပြန်ပေး df;  /* return default value */
  မဟုတ်ပါက {
    size_t a = 0;
    do {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } နေစဉ် (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    ပြန်ပေး a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
static unsigned getnumlimit (Header *h, const var_na **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  အကယ်၍ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    ပြန်ပေး cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  ပြန်ပေး cast_uint(sz);
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
  ကိန်းပြည့် opt = *((*fmt)++);
  *size = 0;  /* default */
  switch (opt) {
    pak_sa 'b': *size = parimaa_na(var_na); ပြန်ပေး Kint;
    pak_sa 'B': *size = parimaa_na(var_na); ပြန်ပေး Kuint;
    pak_sa 'h': *size = parimaa_na(short); ပြန်ပေး Kint;
    pak_sa 'H': *size = parimaa_na(short); ပြန်ပေး Kuint;
    pak_sa 'l': *size = parimaa_na(long); ပြန်ပေး Kint;
    pak_sa 'L': *size = parimaa_na(long); ပြန်ပေး Kuint;
    pak_sa 'j': *size = parimaa_na(lua_Integer); ပြန်ပေး Kint;
    pak_sa 'J': *size = parimaa_na(lua_Integer); ပြန်ပေး Kuint;
    pak_sa 'T': *size = parimaa_na(size_t); ပြန်ပေး Kuint;
    pak_sa 'f': *size = parimaa_na(float); ပြန်ပေး Kfloat;
    pak_sa 'n': *size = parimaa_na(lua_Number); ပြန်ပေး Knumber;
    pak_sa 'd': *size = parimaa_na(dviguu_na); ပြန်ပေး Kdouble;
    pak_sa 'i': *size = getnumlimit(h, fmt, parimaa_na(ကိန်းပြည့်)); ပြန်ပေး Kint;
    pak_sa 'I': *size = getnumlimit(h, fmt, parimaa_na(ကိန်းပြည့်)); ပြန်ပေး Kuint;
    pak_sa 's': *size = getnumlimit(h, fmt, parimaa_na(size_t)); ပြန်ပေး Kstring;
    pak_sa 'c':
      *size = getnum(fmt, cast_sizet(-1));
      အကယ်၍ (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      ပြန်ပေး Kchar;
    pak_sa 'z': ပြန်ပေး Kzstr;
    pak_sa 'x': *size = 1; ပြန်ပေး Kpadding;
    pak_sa 'X': ပြန်ပေး Kpaddalign;
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
  ပြန်ပေး Knop;
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
  အကယ်၍ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    အကယ်၍ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  အကယ်၍ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  မဟုတ်ပါက {
    အကယ်၍ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    အကယ်၍ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    မဟုတ်ပါက {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  ပြန်ပေး opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
static void packint (luaL_Buffer *b, lua_Unsigned n,
                     ကိန်းပြည့် islittle, unsigned size, ကိန်းပြည့် neg) {
  var_na *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (var_na)(n & MC);  /* first byte */
  အတွက် (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (var_na)(n & MC);
  }
  အကယ်၍ (neg && size > SZINT) {  /* negative number need sign extension? */
    အတွက် (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (var_na)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
static void copywithendian (var_na *dest, const var_na *src,
                            unsigned size, ကိန်းပြည့် islittle) {
  အကယ်၍ (islittle == nativeendian.little)
    memcpy(dest, src, size);
  မဟုတ်ပါက {
    dest += size - 1;
    နေစဉ် (size-- != 0)
      *(dest--) = *(src++);
  }
}


static ကိန်းပြည့် str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  const var_na *fmt = luaL_checkstring(L, 1);  /* format string */
  ကိန်းပြည့် arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  နေစဉ် (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    နေစဉ် (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    switch (opt) {
      pak_sa Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        အကယ်၍ (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        bha_nga;
      }
      pak_sa Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        အကယ်၍ (size < SZINT)  /* need overflow check? */
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
        အကယ်၍ (len < size) {  /* does it need padding? */
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
  ပြန်ပေး 1;
}


static ကိန်းပြည့် str_packsize (lua_State *L) {
  Header h;
  const var_na *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  နေစဉ် (*fmt != '\0') {
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
  ပြန်ပေး 1;
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
                              ကိန်းပြည့် islittle, ကိန်းပြည့် size, ကိန်းပြည့် issigned) {
  lua_Unsigned res = 0;
  ကိန်းပြည့် i;
  ကိန်းပြည့် limit = (size  <= SZINT) ? size : SZINT;
  အတွက် (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned var_na)str[islittle ? i : size - 1 - i];
  }
  အကယ်၍ (size < SZINT) {  /* real size smaller than lua_Integer? */
    အကယ်၍ (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  မဟုတ်ပါက အကယ်၍ (size > SZINT) {  /* must check unread bytes */
    ကိန်းပြည့် mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    အတွက် (i = limit; i < size; i++) {
      အကယ်၍ (l_unlikely((unsigned var_na)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  ပြန်ပေး (lua_Integer)res;
}


static ကိန်းပြည့် str_unpack (lua_State *L) {
  Header h;
  const var_na *fmt = luaL_checkstring(L, 1);
  size_t ld;
  const var_na *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  ကိန်းပြည့် n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  နေစဉ် (*fmt != '\0') {
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
  ပြန်ပေး n + 1;
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
LUAMOD_API ကိန်းပြည့် luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  ပြန်ပေး 1;
}

