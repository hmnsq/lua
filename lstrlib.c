/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#define lstrlib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <float.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <locale.h>
#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#എങ്കിൽ !defined(LUA_MAXCAPTURES)
#define LUA_MAXCAPTURES		32
#endif


സ്ഥിതം പൂർണ്ണസംഖ്യ str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  തിരികെ 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
സ്ഥിതം size_t posrelatI (lua_Integer pos, size_t len) {
  എങ്കിൽ (pos > 0)
    തിരികെ (size_t)pos;
  അല്ലെങ്കിൽ എങ്കിൽ (pos == 0)
    തിരികെ 1;
  അല്ലെങ്കിൽ എങ്കിൽ (pos < -(lua_Integer)len)  /* inverted comparison */
    തിരികെ 1;  /* clip to 1 */
  അല്ലെങ്കിൽ തിരികെ len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
സ്ഥിതം size_t getendpos (lua_State *L, പൂർണ്ണസംഖ്യ arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  എങ്കിൽ (pos > (lua_Integer)len)
    തിരികെ len;
  അല്ലെങ്കിൽ എങ്കിൽ (pos >= 0)
    തിരികെ (size_t)pos;
  അല്ലെങ്കിൽ എങ്കിൽ (pos < -(lua_Integer)len)
    തിരികെ 0;
  അല്ലെങ്കിൽ തിരികെ len + (size_t)pos + 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_sub (lua_State *L) {
  size_t l;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  എങ്കിൽ (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  അല്ലെങ്കിൽ lua_pushliteral(L, "");
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &l);
  അക്ഷരം *p = luaL_buffinitsize(L, &b, l);
  വേണ്ടി (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &l);
  അക്ഷരം *p = luaL_buffinitsize(L, &b, l);
  വേണ്ടി (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &l);
  അക്ഷരം *p = luaL_buffinitsize(L, &b, l);
  വേണ്ടി (i=0; i<l; i++)
    p[i] = cast_char(toupper(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  തിരികെ 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ str_rep (lua_State *L) {
  size_t len, lsep;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  സ്ഥിരം അക്ഷരം *sep = luaL_optlstring(L, 3, "", &lsep);
  എങ്കിൽ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  അല്ലെങ്കിൽ എങ്കിൽ (l_unlikely(len > MAX_SIZE - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_SIZE) / n))
    തിരികെ luaL_error(L, "resulting string too large");
  അല്ലെങ്കിൽ {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    luaL_Buffer b;
    അക്ഷരം *p = luaL_buffinitsize(L, &b, totallen);
    ആകുമ്പോൾ (n-- > 1) {  /* first n-1 copies (followed by separator) */
      memcpy(p, s, len * വലുപ്പം(അക്ഷരം)); p += len;
      എങ്കിൽ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        memcpy(p, sep, lsep * വലുപ്പം(അക്ഷരം)); p += lsep;
      }
    }
    memcpy(p, s, len * വലുപ്പം(അക്ഷരം));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_byte (lua_State *L) {
  size_t l;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  പൂർണ്ണസംഖ്യ n, i;
  എങ്കിൽ (posi > pose) തിരികെ 0;  /* empty interval; return no values */
  എങ്കിൽ (l_unlikely(pose - posi >= (size_t)INT_MAX))  /* arithmetic overflow? */
    തിരികെ luaL_error(L, "string slice too long");
  n = (പൂർണ്ണസംഖ്യ)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  വേണ്ടി (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  തിരികെ n;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_char (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);  /* number of arguments */
  പൂർണ്ണസംഖ്യ i;
  luaL_Buffer b;
  അക്ഷരം *p = luaL_buffinitsize(L, &b, cast_uint(n));
  വേണ്ടി (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  തിരികെ 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
ഘടന str_Writer {
  പൂർണ്ണസംഖ്യ init;  /* true iff buffer has been initialized */
  luaL_Buffer B;
};


സ്ഥിതം പൂർണ്ണസംഖ്യ writer (lua_State *L, സ്ഥിരം ശൂന്യം *b, size_t size, ശൂന്യം *ud) {
  ഘടന str_Writer *state = (ഘടന str_Writer *)ud;
  എങ്കിൽ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  എങ്കിൽ (b == ശൂന്യ) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  അല്ലെങ്കിൽ
    luaL_addlstring(&state->B, (സ്ഥിരം അക്ഷരം *)b, size);
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_dump (lua_State *L) {
  ഘടന str_Writer state;
  പൂർണ്ണസംഖ്യ strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  തിരികെ 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#എങ്കിൽ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

സ്ഥിതം സ്ഥിരം luaL_Reg stringmetamethods[] = {
  {"__index", ശൂന്യ},  /* placeholder */
  {ശൂന്യ, ശൂന്യ}
};

#അല്ലെങ്കിൽ		/* }{ */

സ്ഥിതം പൂർണ്ണസംഖ്യ tonum (lua_State *L, പൂർണ്ണസംഖ്യ arg) {
  എങ്കിൽ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ {  /* check whether it is a numerical string */
    size_t len;
    സ്ഥിരം അക്ഷരം *s = lua_tolstring(L, arg, &len);
    തിരികെ (s != ശൂന്യ && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
സ്ഥിതം ശൂന്യം trymt (lua_State *L, സ്ഥിരം അക്ഷരം *mtkey, സ്ഥിരം അക്ഷരം *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  എങ്കിൽ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_തിരുകുക(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ arith (lua_State *L, പൂർണ്ണസംഖ്യ op, സ്ഥിരം അക്ഷരം *mtname) {
  എങ്കിൽ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  അല്ലെങ്കിൽ
    trymt(L, mtname, mtname + 2);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ arith_add (lua_State *L) {
  തിരികെ arith(L, LUA_OPADD, "__add");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ arith_sub (lua_State *L) {
  തിരികെ arith(L, LUA_OPSUB, "__sub");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ arith_mul (lua_State *L) {
  തിരികെ arith(L, LUA_OPMUL, "__mul");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ arith_mod (lua_State *L) {
  തിരികെ arith(L, LUA_OPMOD, "__mod");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ arith_pow (lua_State *L) {
  തിരികെ arith(L, LUA_OPPOW, "__pow");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ arith_div (lua_State *L) {
  തിരികെ arith(L, LUA_OPDIV, "__div");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ arith_idiv (lua_State *L) {
  തിരികെ arith(L, LUA_OPIDIV, "__idiv");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ arith_unm (lua_State *L) {
  തിരികെ arith(L, LUA_OPUNM, "__unm");
}


സ്ഥിതം സ്ഥിരം luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", ശൂന്യ},  /* placeholder */
  {ശൂന്യ, ശൂന്യ}
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


തരനിർവചനം ഘടന MatchState {
  സ്ഥിരം അക്ഷരം *src_init;  /* init of source string */
  സ്ഥിരം അക്ഷരം *src_end;  /* end ('\0') of source string */
  സ്ഥിരം അക്ഷരം *p_end;  /* end ('\0') of pattern */
  lua_State *L;
  പൂർണ്ണസംഖ്യ matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  പൂർണ്ണസംഖ്യ level;  /* total number of captures (finished or unfinished) */
  ഘടന {
    സ്ഥിരം അക്ഷരം *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} MatchState;


/* recursive function */
സ്ഥിതം സ്ഥിരം അക്ഷരം *match (MatchState *ms, സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *p);


/* maximum recursion depth for 'match' */
#എങ്കിൽ !defined(MAXCCALLS)
#define MAXCCALLS	200
#endif


#define L_ESC		'%'
#define SPECIALS	"^$*+?.([%-"


സ്ഥിതം പൂർണ്ണസംഖ്യ check_capture (MatchState *ms, പൂർണ്ണസംഖ്യ l) {
  l -= '1';
  എങ്കിൽ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    തിരികെ luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  തിരികെ l;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ capture_to_close (MatchState *ms) {
  പൂർണ്ണസംഖ്യ level = ms->level;
  വേണ്ടി (level--; level>=0; level--)
    എങ്കിൽ (ms->capture[level].len == CAP_UNFINISHED) തിരികെ level;
  തിരികെ luaL_error(ms->L, "invalid pattern capture");
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *classend (MatchState *ms, സ്ഥിരം അക്ഷരം *p) {
  തിരഞ്ഞെടുപ്പ് (*p++) {
    അവസ്ഥ L_ESC: {
      എങ്കിൽ (l_unlikely(p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      തിരികെ p+1;
    }
    അവസ്ഥ '[': {
      എങ്കിൽ (*p == '^') p++;
      ചെയ്യുക {  /* look for a ']' */
        എങ്കിൽ (l_unlikely(p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        എങ്കിൽ (*(p++) == L_ESC && p < ms->p_end)
          p++;  /* skip escapes (e.g. '%]') */
      } ആകുമ്പോൾ (*p != ']');
      തിരികെ p+1;
    }
    സ്ഥിരസ്ഥിതി: {
      തിരികെ p;
    }
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ match_class (പൂർണ്ണസംഖ്യ c, പൂർണ്ണസംഖ്യ cl) {
  പൂർണ്ണസംഖ്യ res;
  തിരഞ്ഞെടുപ്പ് (tolower(cl)) {
    അവസ്ഥ 'a' : res = isalpha(c); നിർത്തുക;
    അവസ്ഥ 'c' : res = iscntrl(c); നിർത്തുക;
    അവസ്ഥ 'd' : res = isdigit(c); നിർത്തുക;
    അവസ്ഥ 'g' : res = isgraph(c); നിർത്തുക;
    അവസ്ഥ 'l' : res = islower(c); നിർത്തുക;
    അവസ്ഥ 'p' : res = ispunct(c); നിർത്തുക;
    അവസ്ഥ 's' : res = isspace(c); നിർത്തുക;
    അവസ്ഥ 'u' : res = isupper(c); നിർത്തുക;
    അവസ്ഥ 'w' : res = isalnum(c); നിർത്തുക;
    അവസ്ഥ 'x' : res = isxdigit(c); നിർത്തുക;
    അവസ്ഥ 'z' : res = (c == 0); നിർത്തുക;  /* deprecated option */
    സ്ഥിരസ്ഥിതി: തിരികെ (cl == c);
  }
  തിരികെ (islower(cl) ? res : !res);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ matchbracketclass (പൂർണ്ണസംഖ്യ c, സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *ec) {
  പൂർണ്ണസംഖ്യ sig = 1;
  എങ്കിൽ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  ആകുമ്പോൾ (++p < ec) {
    എങ്കിൽ (*p == L_ESC) {
      p++;
      എങ്കിൽ (match_class(c, cast_uchar(*p)))
        തിരികെ sig;
    }
    അല്ലെങ്കിൽ എങ്കിൽ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      എങ്കിൽ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        തിരികെ sig;
    }
    അല്ലെങ്കിൽ എങ്കിൽ (cast_uchar(*p) == c) തിരികെ sig;
  }
  തിരികെ !sig;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ singlematch (MatchState *ms, സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *p,
                        സ്ഥിരം അക്ഷരം *ep) {
  എങ്കിൽ (s >= ms->src_end)
    തിരികെ 0;
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ c = cast_uchar(*s);
    തിരഞ്ഞെടുപ്പ് (*p) {
      അവസ്ഥ '.': തിരികെ 1;  /* matches any char */
      അവസ്ഥ L_ESC: തിരികെ match_class(c, cast_uchar(*(p+1)));
      അവസ്ഥ '[': തിരികെ matchbracketclass(c, p, ep-1);
      സ്ഥിരസ്ഥിതി:  തിരികെ (cast_uchar(*p) == c);
    }
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *matchbalance (MatchState *ms, സ്ഥിരം അക്ഷരം *s,
                                   സ്ഥിരം അക്ഷരം *p) {
  എങ്കിൽ (l_unlikely(p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  എങ്കിൽ (*s != *p) തിരികെ ശൂന്യ;
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ b = *p;
    പൂർണ്ണസംഖ്യ e = *(p+1);
    പൂർണ്ണസംഖ്യ cont = 1;
    ആകുമ്പോൾ (++s < ms->src_end) {
      എങ്കിൽ (*s == e) {
        എങ്കിൽ (--cont == 0) തിരികെ s+1;
      }
      അല്ലെങ്കിൽ എങ്കിൽ (*s == b) cont++;
    }
  }
  തിരികെ ശൂന്യ;  /* string ends out of balance */
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *max_expand (MatchState *ms, സ്ഥിരം അക്ഷരം *s,
                                 സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  ആകുമ്പോൾ (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  ആകുമ്പോൾ (i>=0) {
    സ്ഥിരം അക്ഷരം *res = match(ms, (s+i), ep+1);
    എങ്കിൽ (res) തിരികെ res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  തിരികെ ശൂന്യ;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *min_expand (MatchState *ms, സ്ഥിരം അക്ഷരം *s,
                                 സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *ep) {
  വേണ്ടി (;;) {
    സ്ഥിരം അക്ഷരം *res = match(ms, s, ep+1);
    എങ്കിൽ (res != ശൂന്യ)
      തിരികെ res;
    അല്ലെങ്കിൽ എങ്കിൽ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    അല്ലെങ്കിൽ തിരികെ ശൂന്യ;
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *start_capture (MatchState *ms, സ്ഥിരം അക്ഷരം *s,
                                    സ്ഥിരം അക്ഷരം *p, പൂർണ്ണസംഖ്യ what) {
  സ്ഥിരം അക്ഷരം *res;
  പൂർണ്ണസംഖ്യ level = ms->level;
  എങ്കിൽ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  എങ്കിൽ ((res=match(ms, s, p)) == ശൂന്യ)  /* match failed? */
    ms->level--;  /* undo capture */
  തിരികെ res;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *end_capture (MatchState *ms, സ്ഥിരം അക്ഷരം *s,
                                  സ്ഥിരം അക്ഷരം *p) {
  പൂർണ്ണസംഖ്യ l = capture_to_close(ms);
  സ്ഥിരം അക്ഷരം *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  എങ്കിൽ ((res = match(ms, s, p)) == ശൂന്യ)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  തിരികെ res;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *match_capture (MatchState *ms, സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  എങ്കിൽ ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    തിരികെ s+len;
  അല്ലെങ്കിൽ തിരികെ ശൂന്യ;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *match (MatchState *ms, സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *p) {
  എങ്കിൽ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  എങ്കിൽ (p != ms->p_end) {  /* end of pattern? */
    തിരഞ്ഞെടുപ്പ് (*p) {
      അവസ്ഥ '(': {  /* start capture */
        എങ്കിൽ (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        അല്ലെങ്കിൽ
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        നിർത്തുക;
      }
      അവസ്ഥ ')': {  /* end capture */
        s = end_capture(ms, s, p + 1);
        നിർത്തുക;
      }
      അവസ്ഥ '$': {
        എങ്കിൽ ((p + 1) != ms->p_end)  /* is the '$' the last char in pattern? */
          പോകുക dflt;  /* no; go to default */
        s = (s == ms->src_end) ? s : ശൂന്യ;  /* check end of string */
        നിർത്തുക;
      }
      അവസ്ഥ L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        തിരഞ്ഞെടുപ്പ് (*(p + 1)) {
          അവസ്ഥ 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            എങ്കിൽ (s != ശൂന്യ) {
              p += 4; പോകുക init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            നിർത്തുക;
          }
          അവസ്ഥ 'f': {  /* frontier? */
            സ്ഥിരം അക്ഷരം *ep; അക്ഷരം previous;
            p += 2;
            എങ്കിൽ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            എങ്കിൽ (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; പോകുക init;  /* return match(ms, s, ep); */
            }
            s = ശൂന്യ;  /* match failed */
            നിർത്തുക;
          }
          അവസ്ഥ '0': അവസ്ഥ '1': അവസ്ഥ '2': അവസ്ഥ '3':
          അവസ്ഥ '4': അവസ്ഥ '5': അവസ്ഥ '6': അവസ്ഥ '7':
          അവസ്ഥ '8': അവസ്ഥ '9': {  /* capture results (%0-%9)? */
            s = match_capture(ms, s, cast_uchar(*(p + 1)));
            എങ്കിൽ (s != ശൂന്യ) {
              p += 2; പോകുക init;  /* return match(ms, s, p + 2) */
            }
            നിർത്തുക;
          }
          സ്ഥിരസ്ഥിതി: പോകുക dflt;
        }
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: dflt: {  /* pattern class plus optional suffix */
        സ്ഥിരം അക്ഷരം *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        എങ്കിൽ (!singlematch(ms, s, p, ep)) {
          എങ്കിൽ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; പോകുക init;  /* return match(ms, s, ep + 1); */
          }
          അല്ലെങ്കിൽ  /* '+' or no suffix */
            s = ശൂന്യ;  /* fail */
        }
        അല്ലെങ്കിൽ {  /* matched once */
          തിരഞ്ഞെടുപ്പ് (*ep) {  /* handle optional suffix */
            അവസ്ഥ '?': {  /* optional */
              സ്ഥിരം അക്ഷരം *res;
              എങ്കിൽ ((res = match(ms, s + 1, ep + 1)) != ശൂന്യ)
                s = res;
              അല്ലെങ്കിൽ {
                p = ep + 1; പോകുക init;  /* else return match(ms, s, ep + 1); */
              }
              നിർത്തുക;
            }
            അവസ്ഥ '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            അവസ്ഥ '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              നിർത്തുക;
            അവസ്ഥ '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              നിർത്തുക;
            സ്ഥിരസ്ഥിതി:  /* no suffix */
              s++; p = ep; പോകുക init;  /* return match(ms, s + 1, ep); */
          }
        }
        നിർത്തുക;
      }
    }
  }
  ms->matchdepth++;
  തിരികെ s;
}



സ്ഥിതം സ്ഥിരം അക്ഷരം *lmemfind (സ്ഥിരം അക്ഷരം *s1, size_t l1,
                               സ്ഥിരം അക്ഷരം *s2, size_t l2) {
  എങ്കിൽ (l2 == 0) തിരികെ s1;  /* empty strings are everywhere */
  അല്ലെങ്കിൽ എങ്കിൽ (l2 > l1) തിരികെ ശൂന്യ;  /* avoids a negative 'l1' */
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    ആകുമ്പോൾ (l1 > 0 && (init = (സ്ഥിരം അക്ഷരം *)memchr(s1, *s2, l1)) != ശൂന്യ) {
      init++;   /* 1st char is already checked */
      എങ്കിൽ (memcmp(init, s2+1, l2) == 0)
        തിരികെ init-1;
      അല്ലെങ്കിൽ {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    തിരികെ ശൂന്യ;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
സ്ഥിതം ptrdiff_t get_onecapture (MatchState *ms, പൂർണ്ണസംഖ്യ i, സ്ഥിരം അക്ഷരം *s,
                              സ്ഥിരം അക്ഷരം *e, സ്ഥിരം അക്ഷരം **cap) {
  എങ്കിൽ (i >= ms->level) {
    എങ്കിൽ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    തിരികെ (e - s);
  }
  അല്ലെങ്കിൽ {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    എങ്കിൽ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    അല്ലെങ്കിൽ എങ്കിൽ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    തിരികെ capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
സ്ഥിതം ശൂന്യം തള്ളുക_onecapture (MatchState *ms, പൂർണ്ണസംഖ്യ i, സ്ഥിരം അക്ഷരം *s,
                                                    സ്ഥിരം അക്ഷരം *e) {
  സ്ഥിരം അക്ഷരം *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  എങ്കിൽ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ തള്ളുക_captures (MatchState *ms, സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *e) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  വേണ്ടി (i = 0; i < nlevels; i++)
    തള്ളുക_onecapture(ms, i, s, e);
  തിരികെ nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
സ്ഥിതം പൂർണ്ണസംഖ്യ nospecials (സ്ഥിരം അക്ഷരം *p, size_t l) {
  size_t upto = 0;
  ചെയ്യുക {
    എങ്കിൽ (strpbrk(p + upto, SPECIALS))
      തിരികെ 0;  /* pattern has a special character */
    upto += strlen(p + upto) + 1;  /* may have more after \0 */
  } ആകുമ്പോൾ (upto <= l);
  തിരികെ 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
സ്ഥിതം ശൂന്യം prepstate (MatchState *ms, lua_State *L,
                       സ്ഥിരം അക്ഷരം *s, size_t ls, സ്ഥിരം അക്ഷരം *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
സ്ഥിതം ശൂന്യം reprepstate (MatchState *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_find_aux (lua_State *L, പൂർണ്ണസംഖ്യ find) {
  size_t ls, lp;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &ls);
  സ്ഥിരം അക്ഷരം *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  എങ്കിൽ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    തിരികെ 1;
  }
  /* explicit request or no special characters? */
  എങ്കിൽ (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    സ്ഥിരം അക്ഷരം *s2 = lmemfind(s + init, ls - init, p, lp);
    എങ്കിൽ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      തിരികെ 2;
    }
  }
  അല്ലെങ്കിൽ {
    MatchState ms;
    സ്ഥിരം അക്ഷരം *s1 = s + init;
    പൂർണ്ണസംഖ്യ anchor = (*p == '^');
    എങ്കിൽ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    ചെയ്യുക {
      സ്ഥിരം അക്ഷരം *res;
      reprepstate(&ms);
      എങ്കിൽ ((res=match(&ms, s1, p)) != ശൂന്യ) {
        എങ്കിൽ (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /* end */
          തിരികെ തള്ളുക_captures(&ms, ശൂന്യ, 0) + 2;
        }
        അല്ലെങ്കിൽ
          തിരികെ തള്ളുക_captures(&ms, s1, res);
      }
    } ആകുമ്പോൾ (s1++ < ms.src_end && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_find (lua_State *L) {
  തിരികെ str_find_aux(L, 1);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_match (lua_State *L) {
  തിരികെ str_find_aux(L, 0);
}


/* state for 'gmatch' */
തരനിർവചനം ഘടന GMatchState {
  സ്ഥിരം അക്ഷരം *src;  /* current position */
  സ്ഥിരം അക്ഷരം *p;  /* pattern */
  സ്ഥിരം അക്ഷരം *lastmatch;  /* end of last match */
  MatchState ms;  /* match state */
} GMatchState;


സ്ഥിതം പൂർണ്ണസംഖ്യ gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  സ്ഥിരം അക്ഷരം *src;
  gm->ms.L = L;
  വേണ്ടി (src = gm->src; src <= gm->ms.src_end; src++) {
    സ്ഥിരം അക്ഷരം *e;
    reprepstate(&gm->ms);
    എങ്കിൽ ((e = match(&gm->ms, src, gm->p)) != ശൂന്യ && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      തിരികെ തള്ളുക_captures(&gm->ms, src, e);
    }
  }
  തിരികെ 0;  /* not found */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ gmatch (lua_State *L) {
  size_t ls, lp;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &ls);
  സ്ഥിരം അക്ഷരം *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, വലുപ്പം(GMatchState), 0);
  എങ്കിൽ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = ശൂന്യ;
  lua_pushcclosure(L, gmatch_aux, 3);
  തിരികെ 1;
}


സ്ഥിതം ശൂന്യം add_s (MatchState *ms, luaL_Buffer *b, സ്ഥിരം അക്ഷരം *s,
                                                   സ്ഥിരം അക്ഷരം *e) {
  size_t l;
  lua_State *L = ms->L;
  സ്ഥിരം അക്ഷരം *news = lua_tolstring(L, 3, &l);
  സ്ഥിരം അക്ഷരം *p;
  ആകുമ്പോൾ ((p = (അക്ഷരം *)memchr(news, L_ESC, l)) != ശൂന്യ) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    എങ്കിൽ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    അല്ലെങ്കിൽ എങ്കിൽ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    അല്ലെങ്കിൽ എങ്കിൽ (isdigit(cast_uchar(*p))) {  /* '%n' */
      സ്ഥിരം അക്ഷരം *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      എങ്കിൽ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      അല്ലെങ്കിൽ
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    അല്ലെങ്കിൽ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ add_മൂല്യം (MatchState *ms, luaL_Buffer *b, സ്ഥിരം അക്ഷരം *s,
                                      സ്ഥിരം അക്ഷരം *e, പൂർണ്ണസംഖ്യ tr) {
  lua_State *L = ms->L;
  തിരഞ്ഞെടുപ്പ് (tr) {
    അവസ്ഥ LUA_TFUNCTION: {  /* call the function */
      പൂർണ്ണസംഖ്യ n;
      lua_pushvalue(L, 3);  /* push the function */
      n = തള്ളുക_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      നിർത്തുക;
    }
    അവസ്ഥ LUA_TTABLE: {  /* index the table */
      തള്ളുക_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {  /* LUA_TNUMBER or LUA_TSTRING */
      add_s(ms, b, s, e);  /* add value to the buffer */
      തിരികെ 1;  /* something changed */
    }
  }
  എങ്കിൽ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_എടുക്കുക(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    തിരികെ 0;  /* no changes */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (l_unlikely(!lua_isstring(L, -1)))
    തിരികെ luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  അല്ലെങ്കിൽ {
    luaL_addvalue(b);  /* add result to accumulator */
    തിരികെ 1;  /* something changed */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_gsub (lua_State *L) {
  size_t srcl, lp;
  സ്ഥിരം അക്ഷരം *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  സ്ഥിരം അക്ഷരം *p = luaL_checklstring(L, 2, &lp);  /* pattern */
  സ്ഥിരം അക്ഷരം *lastmatch = ശൂന്യ;  /* end of last match */
  പൂർണ്ണസംഖ്യ tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  പൂർണ്ണസംഖ്യ anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  പൂർണ്ണസംഖ്യ changed = 0;  /* change flag */
  MatchState ms;
  luaL_Buffer b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  എങ്കിൽ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  ആകുമ്പോൾ (n < max_s) {
    സ്ഥിരം അക്ഷരം *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    എങ്കിൽ ((e = match(&ms, src, p)) != ശൂന്യ && e != lastmatch) {  /* match? */
      n++;
      changed = add_മൂല്യം(&ms, &b, src, e, tr) || changed;
      src = lastmatch = e;
    }
    അല്ലെങ്കിൽ എങ്കിൽ (src < ms.src_end)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    അല്ലെങ്കിൽ നിർത്തുക;  /* end of subject */
    എങ്കിൽ (anchor) നിർത്തുക;
  }
  എങ്കിൽ (!changed)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  അല്ലെങ്കിൽ {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_end - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  തിരികെ 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#എങ്കിൽ !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#define SIZELENMOD	(വലുപ്പം(LUA_സംഖ്യ_FRMLEN)/വലുപ്പം(അക്ഷരം))


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
സ്ഥിതം lua_സംഖ്യ adddigit (അക്ഷരം *buff, unsigned n, lua_സംഖ്യ x) {
  lua_സംഖ്യ dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  പൂർണ്ണസംഖ്യ d = (പൂർണ്ണസംഖ്യ)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  തിരികെ x - dd;  /* return what is left */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ num2straux (അക്ഷരം *buff, unsigned sz, lua_സംഖ്യ x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  എങ്കിൽ (x != x || x == (lua_സംഖ്യ)HUGE_VAL || x == -(lua_സംഖ്യ)HUGE_VAL)
    തിരികെ l_sprintf(buff, sz, LUA_സംഖ്യ_FMT, (LUAI_UACNUMBER)x);
  അല്ലെങ്കിൽ എങ്കിൽ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    തിരികെ l_sprintf(buff, sz, LUA_സംഖ്യ_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ e;
    lua_സംഖ്യ m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    unsigned n = 0;  /* character count */
    എങ്കിൽ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    എങ്കിൽ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      ചെയ്യുക {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } ആകുമ്പോൾ (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    തിരികെ cast_int(n);
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ lua_number2strx (lua_State *L, അക്ഷരം *buff, unsigned sz,
                            സ്ഥിരം അക്ഷരം *fmt, lua_സംഖ്യ x) {
  പൂർണ്ണസംഖ്യ n = num2straux(buff, sz, x);
  എങ്കിൽ (fmt[SIZELENMOD] == 'A') {
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < n; i++)
      buff[i] = cast_char(toupper(cast_uchar(buff[i])));
  }
  അല്ലെങ്കിൽ എങ്കിൽ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    തിരികെ luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  തിരികെ n;
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
#എങ്കിൽ !defined(L_FMTFLAGSF)

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


സ്ഥിതം ശൂന്യം addquoted (luaL_Buffer *b, സ്ഥിരം അക്ഷരം *s, size_t len) {
  luaL_addchar(b, '"');
  ആകുമ്പോൾ (len--) {
    എങ്കിൽ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    അല്ലെങ്കിൽ എങ്കിൽ (iscntrl(cast_uchar(*s))) {
      അക്ഷരം buff[10];
      എങ്കിൽ (!isdigit(cast_uchar(*(s+1))))
        l_sprintf(buff, വലുപ്പം(buff), "\\%d", (പൂർണ്ണസംഖ്യ)cast_uchar(*s));
      അല്ലെങ്കിൽ
        l_sprintf(buff, വലുപ്പം(buff), "\\%03d", (പൂർണ്ണസംഖ്യ)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    അല്ലെങ്കിൽ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ quotefloat (lua_State *L, അക്ഷരം *buff, lua_സംഖ്യ n) {
  സ്ഥിരം അക്ഷരം *s;  /* for the fixed representations */
  എങ്കിൽ (n == (lua_സംഖ്യ)HUGE_VAL)  /* inf? */
    s = "1e9999";
  അല്ലെങ്കിൽ എങ്കിൽ (n == -(lua_സംഖ്യ)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  അല്ലെങ്കിൽ എങ്കിൽ (n != n)  /* NaN? */
    s = "(0/0)";
  അല്ലെങ്കിൽ {  /* format number as hexadecimal */
    പൂർണ്ണസംഖ്യ  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_സംഖ്യ_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    എങ്കിൽ (memchr(buff, '.', cast_uint(nb)) == ശൂന്യ) {  /* no dot? */
      അക്ഷരം point = lua_getlocaledecpoint();  /* try locale point */
      അക്ഷരം *ppoint = (അക്ഷരം *)memchr(buff, point, cast_uint(nb));
      എങ്കിൽ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    തിരികെ nb;
  }
  /* for the fixed representations */
  തിരികെ l_sprintf(buff, MAX_ITEM, "%s", s);
}


സ്ഥിതം ശൂന്യം addliteral (lua_State *L, luaL_Buffer *b, പൂർണ്ണസംഖ്യ arg) {
  തിരഞ്ഞെടുപ്പ് (lua_type(L, arg)) {
    അവസ്ഥ LUA_TSTRING: {
      size_t len;
      സ്ഥിരം അക്ഷരം *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_TNUMBER: {
      അക്ഷരം *buff = luaL_prepbuffsize(b, MAX_ITEM);
      പൂർണ്ണസംഖ്യ nb;
      എങ്കിൽ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      അല്ലെങ്കിൽ {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        സ്ഥിരം അക്ഷരം *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_TNIL: അവസ്ഥ LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, ശൂന്യ);
      luaL_addvalue(b);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *get2digits (സ്ഥിരം അക്ഷരം *s) {
  എങ്കിൽ (isdigit(cast_uchar(*s))) {
    s++;
    എങ്കിൽ (isdigit(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  തിരികെ s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
സ്ഥിതം ശൂന്യം checkformat (lua_State *L, സ്ഥിരം അക്ഷരം *form, സ്ഥിരം അക്ഷരം *flags,
                                       പൂർണ്ണസംഖ്യ precision) {
  സ്ഥിരം അക്ഷരം *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  എങ്കിൽ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    എങ്കിൽ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  എങ്കിൽ (!isalpha(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *getformat (lua_State *L, സ്ഥിരം അക്ഷരം *strfrmt,
                                            അക്ഷരം *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  എങ്കിൽ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * വലുപ്പം(അക്ഷരം));
  *(form + len) = '\0';
  തിരികെ strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
സ്ഥിതം ശൂന്യം addlenmod (അക്ഷരം *form, സ്ഥിരം അക്ഷരം *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  അക്ഷരം spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_format (lua_State *L) {
  പൂർണ്ണസംഖ്യ മുകൾ = lua_gettop(L);
  പൂർണ്ണസംഖ്യ arg = 1;
  size_t sfl;
  സ്ഥിരം അക്ഷരം *strfrmt = luaL_checklstring(L, arg, &sfl);
  സ്ഥിരം അക്ഷരം *strfrmt_end = strfrmt+sfl;
  സ്ഥിരം അക്ഷരം *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  ആകുമ്പോൾ (strfrmt < strfrmt_end) {
    എങ്കിൽ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    അല്ലെങ്കിൽ എങ്കിൽ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    അല്ലെങ്കിൽ { /* format item */
      അക്ഷരം form[MAX_FORMAT];  /* to store the format ('%...') */
      unsigned maxitem = MAX_ITEM;  /* maximum length for the result */
      അക്ഷരം *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      പൂർണ്ണസംഖ്യ nb = 0;  /* number of bytes in result */
      എങ്കിൽ (++arg > മുകൾ)
        തിരികെ luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      തിരഞ്ഞെടുപ്പ് (*strfrmt++) {
        അവസ്ഥ 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, arg));
          നിർത്തുക;
        }
        അവസ്ഥ 'd': അവസ്ഥ 'i':
          flags = L_FMTFLAGSI;
          പോകുക intcase;
        അവസ്ഥ 'u':
          flags = L_FMTFLAGSU;
          പോകുക intcase;
        അവസ്ഥ 'o': അവസ്ഥ 'x': അവസ്ഥ 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          നിർത്തുക;
        }
        അവസ്ഥ 'a': അവസ്ഥ 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_സംഖ്യ_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          നിർത്തുക;
        അവസ്ഥ 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        അവസ്ഥ 'e': അവസ്ഥ 'E': അവസ്ഥ 'g': അവസ്ഥ 'G': {
          lua_സംഖ്യ n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_സംഖ്യ_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          നിർത്തുക;
        }
        അവസ്ഥ 'p': {
          സ്ഥിരം ശൂന്യം *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          എങ്കിൽ (p == ശൂന്യ) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /* result */
            form[strlen(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          നിർത്തുക;
        }
        അവസ്ഥ 'q': {
          എങ്കിൽ (form[2] != '\0')  /* modifiers? */
            തിരികെ luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          നിർത്തുക;
        }
        അവസ്ഥ 's': {
          size_t l;
          സ്ഥിരം അക്ഷരം *s = luaL_tolstring(L, arg, &l);
          എങ്കിൽ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          അല്ലെങ്കിൽ {
            luaL_argcheck(L, l == strlen(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            എങ്കിൽ (strchr(form, '.') == ശൂന്യ && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            അല്ലെങ്കിൽ {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_എടുക്കുക(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          നിർത്തുക;
        }
        സ്ഥിരസ്ഥിതി: {  /* also treat cases 'pnLlh' */
          തിരികെ luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  തിരികെ 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#എങ്കിൽ !defined(LUAL_PACKPADBYTE)
#define LUAL_PACKPADBYTE		0x00
#endif

/* maximum size for the binary representation of an integer */
#define MAXINTSIZE	16

/* number of bits in a character */
#define NB	CHAR_BIT

/* mask for one character (NB 1's) */
#define MC	((1 << NB) - 1)

/* size of a lua_Integer */
#define SZINT	((പൂർണ്ണസംഖ്യ)വലുപ്പം(lua_Integer))


/* dummy union to get native endianness */
സ്ഥിതം സ്ഥിരം സംഘം {
  പൂർണ്ണസംഖ്യ dummy;
  അക്ഷരം little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
തരനിർവചനം ഘടന Header {
  lua_State *L;
  പൂർണ്ണസംഖ്യ islittle;
  unsigned maxalign;
} Header;


/*
** options for pack/unpack
*/
തരനിർവചനം എണ്ണൽ KOption {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ digit (പൂർണ്ണസംഖ്യ c) { തിരികെ '0' <= c && c <= '9'; }

സ്ഥിതം size_t getnum (സ്ഥിരം അക്ഷരം **fmt, size_t df) {
  എങ്കിൽ (!digit(**fmt))  /* no number? */
    തിരികെ df;  /* return default value */
  അല്ലെങ്കിൽ {
    size_t a = 0;
    ചെയ്യുക {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } ആകുമ്പോൾ (digit(**fmt) && a <= (MAX_SIZE - 9)/10);
    തിരികെ a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
സ്ഥിതം unsigned getnumlimit (Header *h, സ്ഥിരം അക്ഷരം **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  എങ്കിൽ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    തിരികെ cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  തിരികെ cast_uint(sz);
}


/*
** Initialize Header
*/
സ്ഥിതം ശൂന്യം initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
സ്ഥിതം KOption getoption (Header *h, സ്ഥിരം അക്ഷരം **fmt, size_t *size) {
  /* dummy structure to get native alignment requirements */
  ഘടന cD { അക്ഷരം c; സംഘം { LUAI_MAXALIGN; } u; };
  പൂർണ്ണസംഖ്യ opt = *((*fmt)++);
  *size = 0;  /* default */
  തിരഞ്ഞെടുപ്പ് (opt) {
    അവസ്ഥ 'b': *size = വലുപ്പം(അക്ഷരം); തിരികെ Kint;
    അവസ്ഥ 'B': *size = വലുപ്പം(അക്ഷരം); തിരികെ Kuint;
    അവസ്ഥ 'h': *size = വലുപ്പം(short); തിരികെ Kint;
    അവസ്ഥ 'H': *size = വലുപ്പം(short); തിരികെ Kuint;
    അവസ്ഥ 'l': *size = വലുപ്പം(long); തിരികെ Kint;
    അവസ്ഥ 'L': *size = വലുപ്പം(long); തിരികെ Kuint;
    അവസ്ഥ 'j': *size = വലുപ്പം(lua_Integer); തിരികെ Kint;
    അവസ്ഥ 'J': *size = വലുപ്പം(lua_Integer); തിരികെ Kuint;
    അവസ്ഥ 'T': *size = വലുപ്പം(size_t); തിരികെ Kuint;
    അവസ്ഥ 'f': *size = വലുപ്പം(ദശാംശം); തിരികെ Kfloat;
    അവസ്ഥ 'n': *size = വലുപ്പം(lua_സംഖ്യ); തിരികെ Knumber;
    അവസ്ഥ 'd': *size = വലുപ്പം(ഇരട്ട); തിരികെ Kdouble;
    അവസ്ഥ 'i': *size = getnumlimit(h, fmt, വലുപ്പം(പൂർണ്ണസംഖ്യ)); തിരികെ Kint;
    അവസ്ഥ 'I': *size = getnumlimit(h, fmt, വലുപ്പം(പൂർണ്ണസംഖ്യ)); തിരികെ Kuint;
    അവസ്ഥ 's': *size = getnumlimit(h, fmt, വലുപ്പം(size_t)); തിരികെ Kstring;
    അവസ്ഥ 'c':
      *size = getnum(fmt, cast_sizet(-1));
      എങ്കിൽ (l_unlikely(*size == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      തിരികെ Kchar;
    അവസ്ഥ 'z': തിരികെ Kzstr;
    അവസ്ഥ 'x': *size = 1; തിരികെ Kpadding;
    അവസ്ഥ 'X': തിരികെ Kpaddalign;
    അവസ്ഥ ' ': നിർത്തുക;
    അവസ്ഥ '<': h->islittle = 1; നിർത്തുക;
    അവസ്ഥ '>': h->islittle = 0; നിർത്തുക;
    അവസ്ഥ '=': h->islittle = nativeendian.little; നിർത്തുക;
    അവസ്ഥ '!': {
      സ്ഥിരം size_t maxalign = offsetof(ഘടന cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  തിരികെ Knop;
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
സ്ഥിതം KOption getdetails (Header *h, size_t totalsize, സ്ഥിരം അക്ഷരം **fmt,
                           size_t *psize, unsigned *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  എങ്കിൽ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    എങ്കിൽ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  എങ്കിൽ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  അല്ലെങ്കിൽ {
    എങ്കിൽ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    എങ്കിൽ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    അല്ലെങ്കിൽ {
      /* 'szmoda' = totalsize % align */
      unsigned szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  തിരികെ opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
സ്ഥിതം ശൂന്യം packint (luaL_Buffer *b, lua_Unsigned n,
                     പൂർണ്ണസംഖ്യ islittle, unsigned size, പൂർണ്ണസംഖ്യ neg) {
  അക്ഷരം *buff = luaL_prepbuffsize(b, size);
  unsigned i;
  buff[islittle ? 0 : size - 1] = (അക്ഷരം)(n & MC);  /* first byte */
  വേണ്ടി (i = 1; i < size; i++) {
    n >>= NB;
    buff[islittle ? i : size - 1 - i] = (അക്ഷരം)(n & MC);
  }
  എങ്കിൽ (neg && size > SZINT) {  /* negative number need sign extension? */
    വേണ്ടി (i = SZINT; i < size; i++)  /* correct extra bytes */
      buff[islittle ? i : size - 1 - i] = (അക്ഷരം)MC;
  }
  luaL_addsize(b, size);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
സ്ഥിതം ശൂന്യം copywithendian (അക്ഷരം *dest, സ്ഥിരം അക്ഷരം *src,
                            unsigned size, പൂർണ്ണസംഖ്യ islittle) {
  എങ്കിൽ (islittle == nativeendian.little)
    memcpy(dest, src, size);
  അല്ലെങ്കിൽ {
    dest += size - 1;
    ആകുമ്പോൾ (size-- != 0)
      *(dest--) = *(src++);
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  സ്ഥിരം അക്ഷരം *fmt = luaL_checkstring(L, 1);  /* format string */
  പൂർണ്ണസംഖ്യ arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  ആകുമ്പോൾ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    luaL_argcheck(L, size + ntoalign <= MAX_SIZE - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + size;
    ആകുമ്പോൾ (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    തിരഞ്ഞെടുപ്പ് (opt) {
      അവസ്ഥ Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        എങ്കിൽ (size < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((size * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), (n < 0));
        നിർത്തുക;
      }
      അവസ്ഥ Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        എങ്കിൽ (size < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (size * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(size), 0);
        നിർത്തുക;
      }
      അവസ്ഥ Kfloat: {  /* C float */
        ദശാംശം f = (ദശാംശം)luaL_checknumber(L, arg);  /* get argument */
        അക്ഷരം *buff = luaL_prepbuffsize(&b, വലുപ്പം(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (അക്ഷരം *)&f, വലുപ്പം(f), h.islittle);
        luaL_addsize(&b, size);
        നിർത്തുക;
      }
      അവസ്ഥ Knumber: {  /* Lua float */
        lua_സംഖ്യ f = luaL_checknumber(L, arg);  /* get argument */
        അക്ഷരം *buff = luaL_prepbuffsize(&b, വലുപ്പം(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (അക്ഷരം *)&f, വലുപ്പം(f), h.islittle);
        luaL_addsize(&b, size);
        നിർത്തുക;
      }
      അവസ്ഥ Kdouble: {  /* C double */
        ഇരട്ട f = (ഇരട്ട)luaL_checknumber(L, arg);  /* get argument */
        അക്ഷരം *buff = luaL_prepbuffsize(&b, വലുപ്പം(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (അക്ഷരം *)&f, വലുപ്പം(f), h.islittle);
        luaL_addsize(&b, size);
        നിർത്തുക;
      }
      അവസ്ഥ Kchar: {  /* fixed-size string */
        size_t len;
        സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= size, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        എങ്കിൽ (len < size) {  /* does it need padding? */
          size_t psize = size - len;  /* pad size */
          അക്ഷരം *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        നിർത്തുക;
      }
      അവസ്ഥ Kstring: {  /* strings with length count */
        size_t len;
        സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, size >= വലുപ്പം(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (size * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(size), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        നിർത്തുക;
      }
      അവസ്ഥ Kzstr: {  /* zero-terminated string */
        size_t len;
        സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, strlen(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        നിർത്തുക;
      }
      അവസ്ഥ Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      അവസ്ഥ Kpaddalign: അവസ്ഥ Knop:
        arg--;  /* undo increment */
        നിർത്തുക;
    }
  }
  luaL_pushresult(&b);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_packsize (lua_State *L) {
  Header h;
  സ്ഥിരം അക്ഷരം *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  ആകുമ്പോൾ (*fmt != '\0') {
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
  തിരികെ 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
സ്ഥിതം lua_Integer unpackint (lua_State *L, സ്ഥിരം അക്ഷരം *str,
                              പൂർണ്ണസംഖ്യ islittle, പൂർണ്ണസംഖ്യ size, പൂർണ്ണസംഖ്യ issigned) {
  lua_Unsigned res = 0;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ limit = (size  <= SZINT) ? size : SZINT;
  വേണ്ടി (i = limit - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(unsigned അക്ഷരം)str[islittle ? i : size - 1 - i];
  }
  എങ്കിൽ (size < SZINT) {  /* real size smaller than lua_Integer? */
    എങ്കിൽ (issigned) {  /* needs sign extension? */
      lua_Unsigned mask = (lua_Unsigned)1 << (size*NB - 1);
      res = ((res ^ mask) - mask);  /* do sign extension */
    }
  }
  അല്ലെങ്കിൽ എങ്കിൽ (size > SZINT) {  /* must check unread bytes */
    പൂർണ്ണസംഖ്യ mask = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    വേണ്ടി (i = limit; i < size; i++) {
      എങ്കിൽ (l_unlikely((unsigned അക്ഷരം)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  തിരികെ (lua_Integer)res;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ str_unpack (lua_State *L) {
  Header h;
  സ്ഥിരം അക്ഷരം *fmt = luaL_checkstring(L, 1);
  size_t ld;
  സ്ഥിരം അക്ഷരം *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  പൂർണ്ണസംഖ്യ n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  ആകുമ്പോൾ (*fmt != '\0') {
    unsigned ntoalign;
    size_t size;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    luaL_argcheck(L, ntoalign + size <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    തിരഞ്ഞെടുപ്പ് (opt) {
      അവസ്ഥ Kint:
      അവസ്ഥ Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle,
                                       cast_int(size), (opt == Kint));
        lua_pushinteger(L, res);
        നിർത്തുക;
      }
      അവസ്ഥ Kfloat: {
        ദശാംശം f;
        copywithendian((അക്ഷരം *)&f, data + pos, വലുപ്പം(f), h.islittle);
        lua_pushnumber(L, (lua_സംഖ്യ)f);
        നിർത്തുക;
      }
      അവസ്ഥ Knumber: {
        lua_സംഖ്യ f;
        copywithendian((അക്ഷരം *)&f, data + pos, വലുപ്പം(f), h.islittle);
        lua_pushnumber(L, f);
        നിർത്തുക;
      }
      അവസ്ഥ Kdouble: {
        ഇരട്ട f;
        copywithendian((അക്ഷരം *)&f, data + pos, വലുപ്പം(f), h.islittle);
        lua_pushnumber(L, (lua_സംഖ്യ)f);
        നിർത്തുക;
      }
      അവസ്ഥ Kchar: {
        lua_pushlstring(L, data + pos, size);
        നിർത്തുക;
      }
      അവസ്ഥ Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, data + pos,
                                          h.islittle, cast_int(size), 0);
        luaL_argcheck(L, len <= ld - pos - size, 2, "data string too short");
        lua_pushlstring(L, data + pos + size, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        നിർത്തുക;
      }
      അവസ്ഥ Kzstr: {
        size_t len = strlen(data + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        നിർത്തുക;
      }
      അവസ്ഥ Kpaddalign: അവസ്ഥ Kpadding: അവസ്ഥ Knop:
        n--;  /* undo increment */
        നിർത്തുക;
    }
    pos += size;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  തിരികെ n + 1;
}

/* }====================================================== */


സ്ഥിതം സ്ഥിരം luaL_Reg strlib[] = {
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
  {ശൂന്യ, ശൂന്യ}
};


സ്ഥിതം ശൂന്യം createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_എടുക്കുക(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_എടുക്കുക(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  തിരികെ 1;
}

