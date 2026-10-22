/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#परिभाषय lstrlib_c
#परिभाषय LUA_LIB

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
#यदि !defined(LUA_MAXCAPTURES)
#परिभाषय LUA_MAXCAPTURES		32
#यदि_अन्तः


स्थिर पूर्णाङ्क str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  प्रत्यागम 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
स्थिर size_t posrelatI (lua_Integer pos, size_t len) {
  यदि (pos > 0)
    प्रत्यागम (size_t)pos;
  अन्यथा यदि (pos == 0)
    प्रत्यागम 1;
  अन्यथा यदि (pos < -(lua_Integer)len)  /* inverted comparison */
    प्रत्यागम 1;  /* clip to 1 */
  अन्यथा प्रत्यागम len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
स्थिर size_t getendpos (lua_State *L, पूर्णाङ्क arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  यदि (pos > (lua_Integer)len)
    प्रत्यागम len;
  अन्यथा यदि (pos >= 0)
    प्रत्यागम (size_t)pos;
  अन्यथा यदि (pos < -(lua_Integer)len)
    प्रत्यागम 0;
  अन्यथा प्रत्यागम len + (size_t)pos + 1;
}


स्थिर पूर्णाङ्क str_sub (lua_State *L) {
  size_t l;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t अन्तः = getendpos(L, 3, -1, l);
  यदि (start <= अन्तः)
    lua_pushlstring(L, s + start - 1, (अन्तः - start) + 1);
  अन्यथा lua_pushliteral(L, "");
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_reverse (lua_State *L) {
  size_t l, i;
  lua_L_बफर b;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  वर्ण *p = luaL_buffinitsize(L, &b, l);
  प्रति (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_अधस्तनम् (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_बफर b;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  वर्ण *p = luaL_buffinitsize(L, &b, l);
  प्रति (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_ऊर्ध्वस्थम् (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_बफर b;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  वर्ण *p = luaL_buffinitsize(L, &b, l);
  प्रति (i=0; i<l; i++)
    p[i] = cast_char(महाक्षरम्(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  प्रत्यागम 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
स्थिर पूर्णाङ्क str_rep (lua_State *L) {
  size_t len, lsep;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ध्रुव वर्ण *sep = luaL_optlstring(L, 3, "", &lsep);
  यदि (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  अन्यथा यदि (l_unlikely(len > MAX_आकार - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_आकार) / n))
    प्रत्यागम luaL_error(L, "resulting string too large");
  अन्यथा {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    lua_L_बफर b;
    वर्ण *p = luaL_buffinitsize(L, &b, totallen);
    यावत् (n-- > 1) {  /* first n-1 copies (followed by separator) */
      स्मृतिं_प्रतिलिख(p, s, len * परिमाण(वर्ण)); p += len;
      यदि (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        स्मृतिं_प्रतिलिख(p, sep, lsep * परिमाण(वर्ण)); p += lsep;
      }
    }
    स्मृतिं_प्रतिलिख(p, s, len * परिमाण(वर्ण));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_byte (lua_State *L) {
  size_t l;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  पूर्णाङ्क n, i;
  यदि (posi > pose) प्रत्यागम 0;  /* empty interval; return no values */
  यदि (l_unlikely(pose - posi >= (size_t)अधिकतम_पूर्णाङ्कः))  /* arithmetic overflow? */
    प्रत्यागम luaL_error(L, "string slice too long");
  n = (पूर्णाङ्क)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  प्रति (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  प्रत्यागम n;
}


स्थिर पूर्णाङ्क str_char (lua_State *L) {
  पूर्णाङ्क n = lua_gettop(L);  /* number of arguments */
  पूर्णाङ्क i;
  lua_L_बफर b;
  वर्ण *p = luaL_buffinitsize(L, &b, cast_uint(n));
  प्रति (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  प्रत्यागम 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
संरचना str_Writer {
  पूर्णाङ्क init;  /* true iff buffer has been initialized */
  lua_L_बफर B;
};


स्थिर पूर्णाङ्क writer (lua_State *L, ध्रुव शून्य *b, size_t आकार, शून्य *ud) {
  संरचना str_Writer *state = (संरचना str_Writer *)ud;
  यदि (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  यदि (b == रिक्त) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  अन्यथा
    luaL_addlstring(&state->B, (ध्रुव वर्ण *)b, आकार);
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क str_dump (lua_State *L) {
  संरचना str_Writer state;
  पूर्णाङ्क strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  प्रत्यागम 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#यदि defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

स्थिर ध्रुव luaL_Reg stringmetamethods[] = {
  {"__index", रिक्त},  /* placeholder */
  {रिक्त, रिक्त}
};

#अन्यथा		/* }{ */

स्थिर पूर्णाङ्क tonum (lua_State *L, पूर्णाङ्क arg) {
  यदि (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    प्रत्यागम 1;
  }
  अन्यथा {  /* check whether it is a numerical string */
    size_t len;
    ध्रुव वर्ण *s = lua_tolstring(L, arg, &len);
    प्रत्यागम (s != रिक्त && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
स्थिर शून्य trymt (lua_State *L, ध्रुव वर्ण *mtkey, ध्रुव वर्ण *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  यदि (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_निवेशय(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


स्थिर पूर्णाङ्क arith (lua_State *L, पूर्णाङ्क op, ध्रुव वर्ण *mtname) {
  यदि (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  अन्यथा
    trymt(L, mtname, mtname + 2);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क arith_सङ्कलय (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPADD, "__add");
}

स्थिर पूर्णाङ्क arith_sub (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPSUB, "__sub");
}

स्थिर पूर्णाङ्क arith_mul (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPMUL, "__mul");
}

स्थिर पूर्णाङ्क arith_mod (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPMOD, "__mod");
}

स्थिर पूर्णाङ्क arith_pow (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPPOW, "__pow");
}

स्थिर पूर्णाङ्क arith_div (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPDIV, "__div");
}

स्थिर पूर्णाङ्क arith_idiv (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPIDIV, "__idiv");
}

स्थिर पूर्णाङ्क arith_unm (lua_State *L) {
  प्रत्यागम arith(L, LUA_OPUNM, "__unm");
}


स्थिर ध्रुव luaL_Reg stringmetamethods[] = {
  {"__add", arith_सङ्कलय},
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

#यदि_अन्तः		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#परिभाषय CAP_UNFINISHED	(-1)
#परिभाषय CAP_POSITION	(-2)


प्रकारनिर्देश संरचना मेलनम्_State {
  ध्रुव वर्ण *src_init;  /* init of source string */
  ध्रुव वर्ण *src_अन्तः;  /* end ('\0') of source string */
  ध्रुव वर्ण *p_अन्तः;  /* end ('\0') of pattern */
  lua_State *L;
  पूर्णाङ्क matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  पूर्णाङ्क level;  /* total number of captures (finished or unfinished) */
  संरचना {
    ध्रुव वर्ण *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} मेलनम्_State;


/* recursive function */
स्थिर ध्रुव वर्ण *मेलनम् (मेलनम्_State *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *p);


/* maximum recursion depth for 'match' */
#यदि !defined(MAXCCALLS)
#परिभाषय MAXCCALLS	200
#यदि_अन्तः


#परिभाषय L_ESC		'%'
#परिभाषय SPECIALS	"^$*+?.([%-"


स्थिर पूर्णाङ्क check_capture (मेलनम्_State *ms, पूर्णाङ्क l) {
  l -= '1';
  यदि (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    प्रत्यागम luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  प्रत्यागम l;
}


स्थिर पूर्णाङ्क capture_गन्तव्यम्_close (मेलनम्_State *ms) {
  पूर्णाङ्क level = ms->level;
  प्रति (level--; level>=0; level--)
    यदि (ms->capture[level].len == CAP_UNFINISHED) प्रत्यागम level;
  प्रत्यागम luaL_error(ms->L, "invalid pattern capture");
}


स्थिर ध्रुव वर्ण *classend (मेलनम्_State *ms, ध्रुव वर्ण *p) {
  विकल्प (*p++) {
    पक्ष L_ESC: {
      यदि (l_unlikely(p == ms->p_अन्तः))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      प्रत्यागम p+1;
    }
    पक्ष '[': {
      यदि (*p == '^') p++;
      कुरु {  /* look for a ']' */
        यदि (l_unlikely(p == ms->p_अन्तः))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        यदि (*(p++) == L_ESC && p < ms->p_अन्तः)
          p++;  /* skip escapes (e.g. '%]') */
      } यावत् (*p != ']');
      प्रत्यागम p+1;
    }
    स्वभाव: {
      प्रत्यागम p;
    }
  }
}


स्थिर पूर्णाङ्क मेलनम्_class (पूर्णाङ्क c, पूर्णाङ्क cl) {
  पूर्णाङ्क res;
  विकल्प (tolower(cl)) {
    पक्ष 'a' : res = अक्षरम्_वा(c); भङ्ग;
    पक्ष 'c' : res = iscntrl(c); भङ्ग;
    पक्ष 'd' : res = अङ्कः_वा(c); भङ्ग;
    पक्ष 'g' : res = isgraph(c); भङ्ग;
    पक्ष 'l' : res = islower(c); भङ्ग;
    पक्ष 'p' : res = ispunct(c); भङ्ग;
    पक्ष 's' : res = रिक्तम्_वा(c); भङ्ग;
    पक्ष 'u' : res = isupper(c); भङ्ग;
    पक्ष 'w' : res = अक्षरम्_अङ्कः_वा(c); भङ्ग;
    पक्ष 'x' : res = isxdigit(c); भङ्ग;
    पक्ष 'z' : res = (c == 0); भङ्ग;  /* deprecated option */
    स्वभाव: प्रत्यागम (cl == c);
  }
  प्रत्यागम (islower(cl) ? res : !res);
}


स्थिर पूर्णाङ्क matchbracketclass (पूर्णाङ्क c, ध्रुव वर्ण *p, ध्रुव वर्ण *ec) {
  पूर्णाङ्क sig = 1;
  यदि (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  यावत् (++p < ec) {
    यदि (*p == L_ESC) {
      p++;
      यदि (मेलनम्_class(c, cast_uchar(*p)))
        प्रत्यागम sig;
    }
    अन्यथा यदि ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      यदि (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        प्रत्यागम sig;
    }
    अन्यथा यदि (cast_uchar(*p) == c) प्रत्यागम sig;
  }
  प्रत्यागम !sig;
}


स्थिर पूर्णाङ्क singlematch (मेलनम्_State *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *p,
                        ध्रुव वर्ण *ep) {
  यदि (s >= ms->src_अन्तः)
    प्रत्यागम 0;
  अन्यथा {
    पूर्णाङ्क c = cast_uchar(*s);
    विकल्प (*p) {
      पक्ष '.': प्रत्यागम 1;  /* matches any char */
      पक्ष L_ESC: प्रत्यागम मेलनम्_class(c, cast_uchar(*(p+1)));
      पक्ष '[': प्रत्यागम matchbracketclass(c, p, ep-1);
      स्वभाव:  प्रत्यागम (cast_uchar(*p) == c);
    }
  }
}


स्थिर ध्रुव वर्ण *matchbalance (मेलनम्_State *ms, ध्रुव वर्ण *s,
                                   ध्रुव वर्ण *p) {
  यदि (l_unlikely(p >= ms->p_अन्तः - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  यदि (*s != *p) प्रत्यागम रिक्त;
  अन्यथा {
    पूर्णाङ्क b = *p;
    पूर्णाङ्क e = *(p+1);
    पूर्णाङ्क cont = 1;
    यावत् (++s < ms->src_अन्तः) {
      यदि (*s == e) {
        यदि (--cont == 0) प्रत्यागम s+1;
      }
      अन्यथा यदि (*s == b) cont++;
    }
  }
  प्रत्यागम रिक्त;  /* string ends out of balance */
}


स्थिर ध्रुव वर्ण *max_expand (मेलनम्_State *ms, ध्रुव वर्ण *s,
                                 ध्रुव वर्ण *p, ध्रुव वर्ण *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  यावत् (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  यावत् (i>=0) {
    ध्रुव वर्ण *res = मेलनम्(ms, (s+i), ep+1);
    यदि (res) प्रत्यागम res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  प्रत्यागम रिक्त;
}


स्थिर ध्रुव वर्ण *min_expand (मेलनम्_State *ms, ध्रुव वर्ण *s,
                                 ध्रुव वर्ण *p, ध्रुव वर्ण *ep) {
  प्रति (;;) {
    ध्रुव वर्ण *res = मेलनम्(ms, s, ep+1);
    यदि (res != रिक्त)
      प्रत्यागम res;
    अन्यथा यदि (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    अन्यथा प्रत्यागम रिक्त;
  }
}


स्थिर ध्रुव वर्ण *start_capture (मेलनम्_State *ms, ध्रुव वर्ण *s,
                                    ध्रुव वर्ण *p, पूर्णाङ्क what) {
  ध्रुव वर्ण *res;
  पूर्णाङ्क level = ms->level;
  यदि (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  यदि ((res=मेलनम्(ms, s, p)) == रिक्त)  /* match failed? */
    ms->level--;  /* undo capture */
  प्रत्यागम res;
}


स्थिर ध्रुव वर्ण *अन्तः_capture (मेलनम्_State *ms, ध्रुव वर्ण *s,
                                  ध्रुव वर्ण *p) {
  पूर्णाङ्क l = capture_गन्तव्यम्_close(ms);
  ध्रुव वर्ण *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  यदि ((res = मेलनम्(ms, s, p)) == रिक्त)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  प्रत्यागम res;
}


स्थिर ध्रुव वर्ण *मेलनम्_capture (मेलनम्_State *ms, ध्रुव वर्ण *s, पूर्णाङ्क l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  यदि ((size_t)(ms->src_अन्तः-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    प्रत्यागम s+len;
  अन्यथा प्रत्यागम रिक्त;
}


स्थिर ध्रुव वर्ण *मेलनम् (मेलनम्_State *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *p) {
  यदि (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  यदि (p != ms->p_अन्तः) {  /* end of pattern? */
    विकल्प (*p) {
      पक्ष '(': {  /* start capture */
        यदि (*(p + 1) == ')')  /* position capture? */
          s = start_capture(ms, s, p + 2, CAP_POSITION);
        अन्यथा
          s = start_capture(ms, s, p + 1, CAP_UNFINISHED);
        भङ्ग;
      }
      पक्ष ')': {  /* end capture */
        s = अन्तः_capture(ms, s, p + 1);
        भङ्ग;
      }
      पक्ष '$': {
        यदि ((p + 1) != ms->p_अन्तः)  /* is the '$' the last char in pattern? */
          गच्छ dflt;  /* no; go to default */
        s = (s == ms->src_अन्तः) ? s : रिक्त;  /* check end of string */
        भङ्ग;
      }
      पक्ष L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        विकल्प (*(p + 1)) {
          पक्ष 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            यदि (s != रिक्त) {
              p += 4; गच्छ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            भङ्ग;
          }
          पक्ष 'f': {  /* frontier? */
            ध्रुव वर्ण *ep; वर्ण previous;
            p += 2;
            यदि (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            यदि (!matchbracketclass(cast_uchar(previous), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; गच्छ init;  /* return match(ms, s, ep); */
            }
            s = रिक्त;  /* match failed */
            भङ्ग;
          }
          पक्ष '0': पक्ष '1': पक्ष '2': पक्ष '3':
          पक्ष '4': पक्ष '5': पक्ष '6': पक्ष '7':
          पक्ष '8': पक्ष '9': {  /* capture results (%0-%9)? */
            s = मेलनम्_capture(ms, s, cast_uchar(*(p + 1)));
            यदि (s != रिक्त) {
              p += 2; गच्छ init;  /* return match(ms, s, p + 2) */
            }
            भङ्ग;
          }
          स्वभाव: गच्छ dflt;
        }
        भङ्ग;
      }
      स्वभाव: dflt: {  /* pattern class plus optional suffix */
        ध्रुव वर्ण *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        यदि (!singlematch(ms, s, p, ep)) {
          यदि (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; गच्छ init;  /* return match(ms, s, ep + 1); */
          }
          अन्यथा  /* '+' or no suffix */
            s = रिक्त;  /* fail */
        }
        अन्यथा {  /* matched once */
          विकल्प (*ep) {  /* handle optional suffix */
            पक्ष '?': {  /* optional */
              ध्रुव वर्ण *res;
              यदि ((res = मेलनम्(ms, s + 1, ep + 1)) != रिक्त)
                s = res;
              अन्यथा {
                p = ep + 1; गच्छ init;  /* else return match(ms, s, ep + 1); */
              }
              भङ्ग;
            }
            पक्ष '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            पक्ष '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              भङ्ग;
            पक्ष '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              भङ्ग;
            स्वभाव:  /* no suffix */
              s++; p = ep; गच्छ init;  /* return match(ms, s + 1, ep); */
          }
        }
        भङ्ग;
      }
    }
  }
  ms->matchdepth++;
  प्रत्यागम s;
}



स्थिर ध्रुव वर्ण *lmemfind (ध्रुव वर्ण *s1, size_t l1,
                               ध्रुव वर्ण *s2, size_t l2) {
  यदि (l2 == 0) प्रत्यागम s1;  /* empty strings are everywhere */
  अन्यथा यदि (l2 > l1) प्रत्यागम रिक्त;  /* avoids a negative 'l1' */
  अन्यथा {
    ध्रुव वर्ण *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    यावत् (l1 > 0 && (init = (ध्रुव वर्ण *)memchr(s1, *s2, l1)) != रिक्त) {
      init++;   /* 1st char is already checked */
      यदि (memcmp(init, s2+1, l2) == 0)
        प्रत्यागम init-1;
      अन्यथा {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    प्रत्यागम रिक्त;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
स्थिर ptrdiff_t get_onecapture (मेलनम्_State *ms, पूर्णाङ्क i, ध्रुव वर्ण *s,
                              ध्रुव वर्ण *e, ध्रुव वर्ण **cap) {
  यदि (i >= ms->level) {
    यदि (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    प्रत्यागम (e - s);
  }
  अन्यथा {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    यदि (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    अन्यथा यदि (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    प्रत्यागम capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
स्थिर शून्य प्रक्षिप_onecapture (मेलनम्_State *ms, पूर्णाङ्क i, ध्रुव वर्ण *s,
                                                    ध्रुव वर्ण *e) {
  ध्रुव वर्ण *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  यदि (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


स्थिर पूर्णाङ्क प्रक्षिप_captures (मेलनम्_State *ms, ध्रुव वर्ण *s, ध्रुव वर्ण *e) {
  पूर्णाङ्क i;
  पूर्णाङ्क nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  प्रति (i = 0; i < nlevels; i++)
    प्रक्षिप_onecapture(ms, i, s, e);
  प्रत्यागम nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
स्थिर पूर्णाङ्क nospecials (ध्रुव वर्ण *p, size_t l) {
  size_t upto = 0;
  कुरु {
    यदि (strpbrk(p + upto, SPECIALS))
      प्रत्यागम 0;  /* pattern has a special character */
    upto += वाक्यदैर्घ्यम्(p + upto) + 1;  /* may have more after \0 */
  } यावत् (upto <= l);
  प्रत्यागम 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
स्थिर शून्य prepstate (मेलनम्_State *ms, lua_State *L,
                       ध्रुव वर्ण *s, size_t ls, ध्रुव वर्ण *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_अन्तः = s + ls;
  ms->p_अन्तः = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
स्थिर शून्य reprepstate (मेलनम्_State *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


स्थिर पूर्णाङ्क str_find_aux (lua_State *L, पूर्णाङ्क find) {
  size_t ls, lp;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &ls);
  ध्रुव वर्ण *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  यदि (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    प्रत्यागम 1;
  }
  /* explicit request or no special characters? */
  यदि (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ध्रुव वर्ण *s2 = lmemfind(s + init, ls - init, p, lp);
    यदि (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      प्रत्यागम 2;
    }
  }
  अन्यथा {
    मेलनम्_State ms;
    ध्रुव वर्ण *s1 = s + init;
    पूर्णाङ्क anchor = (*p == '^');
    यदि (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    कुरु {
      ध्रुव वर्ण *res;
      reprepstate(&ms);
      यदि ((res=मेलनम्(&ms, s1, p)) != रिक्त) {
        यदि (find) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /* start */
          lua_pushinteger(L, ct_diff2S(res - s));   /*अन्तः*/
          प्रत्यागम प्रक्षिप_captures(&ms, रिक्त, 0) + 2;
        }
        अन्यथा
          प्रत्यागम प्रक्षिप_captures(&ms, s1, res);
      }
    } यावत् (s1++ < ms.src_अन्तः && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_find (lua_State *L) {
  प्रत्यागम str_find_aux(L, 1);
}


स्थिर पूर्णाङ्क str_मेलनम् (lua_State *L) {
  प्रत्यागम str_find_aux(L, 0);
}


/* state for 'gmatch' */
प्रकारनिर्देश संरचना GMatchState {
  ध्रुव वर्ण *src;  /* current position */
  ध्रुव वर्ण *p;  /*आदर्शः*/
  ध्रुव वर्ण *lastmatch;  /* end of last match */
  मेलनम्_State ms;  /* match state */
} GMatchState;


स्थिर पूर्णाङ्क gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ध्रुव वर्ण *src;
  gm->ms.L = L;
  प्रति (src = gm->src; src <= gm->ms.src_अन्तः; src++) {
    ध्रुव वर्ण *e;
    reprepstate(&gm->ms);
    यदि ((e = मेलनम्(&gm->ms, src, gm->p)) != रिक्त && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      प्रत्यागम प्रक्षिप_captures(&gm->ms, src, e);
    }
  }
  प्रत्यागम 0;  /* not found */
}


स्थिर पूर्णाङ्क gmatch (lua_State *L) {
  size_t ls, lp;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &ls);
  ध्रुव वर्ण *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, परिमाण(GMatchState), 0);
  यदि (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = रिक्त;
  lua_pushcclosure(L, gmatch_aux, 3);
  प्रत्यागम 1;
}


स्थिर शून्य सङ्कलय_s (मेलनम्_State *ms, lua_L_बफर *b, ध्रुव वर्ण *s,
                                                   ध्रुव वर्ण *e) {
  size_t l;
  lua_State *L = ms->L;
  ध्रुव वर्ण *news = lua_tolstring(L, 3, &l);
  ध्रुव वर्ण *p;
  यावत् ((p = (वर्ण *)memchr(news, L_ESC, l)) != रिक्त) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    यदि (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    अन्यथा यदि (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    अन्यथा यदि (अङ्कः_वा(cast_uchar(*p))) {  /* '%n' */
      ध्रुव वर्ण *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      यदि (resl == CAP_POSITION)
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
स्थिर पूर्णाङ्क सङ्कलय_मानम् (मेलनम्_State *ms, lua_L_बफर *b, ध्रुव वर्ण *s,
                                      ध्रुव वर्ण *e, पूर्णाङ्क tr) {
  lua_State *L = ms->L;
  विकल्प (tr) {
    पक्ष LUA_TFUNCTION: {  /* call the function */
      पूर्णाङ्क n;
      lua_pushvalue(L, 3);  /* push the function */
      n = प्रक्षिप_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      भङ्ग;
    }
    पक्ष LUA_TTABLE: {  /* index the table */
      प्रक्षिप_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      भङ्ग;
    }
    स्वभाव: {  /* LUA_TNUMBER or LUA_TSTRING */
      सङ्कलय_s(ms, b, s, e);  /* add value to the buffer */
      प्रत्यागम 1;  /* something changed */
    }
  }
  यदि (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_निष्कासय(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    प्रत्यागम 0;  /* no changes */
  }
  अन्यथा यदि (l_unlikely(!lua_isstring(L, -1)))
    प्रत्यागम luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  अन्यथा {
    luaL_addvalue(b);  /* add result to accumulator */
    प्रत्यागम 1;  /* something changed */
  }
}


स्थिर पूर्णाङ्क str_gsub (lua_State *L) {
  size_t srcl, lp;
  ध्रुव वर्ण *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ध्रुव वर्ण *p = luaL_checklstring(L, 2, &lp);  /*आदर्शः*/
  ध्रुव वर्ण *lastmatch = रिक्त;  /* end of last match */
  पूर्णाङ्क tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  पूर्णाङ्क anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  पूर्णाङ्क परिवर्तितम् = 0;  /* change flag */
  मेलनम्_State ms;
  lua_L_बफर b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  यदि (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  यावत् (n < max_s) {
    ध्रुव वर्ण *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    यदि ((e = मेलनम्(&ms, src, p)) != रिक्त && e != lastmatch) {  /* match? */
      n++;
      परिवर्तितम् = सङ्कलय_मानम्(&ms, &b, src, e, tr) || परिवर्तितम्;
      src = lastmatch = e;
    }
    अन्यथा यदि (src < ms.src_अन्तः)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    अन्यथा भङ्ग;  /* end of subject */
    यदि (anchor) भङ्ग;
  }
  यदि (!परिवर्तितम्)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  अन्यथा {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_अन्तः - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  प्रत्यागम 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#यदि !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#परिभाषय SIZELENMOD	(परिमाण(LUA_सङ्ख्या_FRMLEN)/परिमाण(वर्ण))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#परिभाषय L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
स्थिर lua_सङ्ख्या adddigit (वर्ण *buff, अचिह्नित n, lua_सङ्ख्या x) {
  lua_सङ्ख्या dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  पूर्णाङ्क d = (पूर्णाङ्क)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  प्रत्यागम x - dd;  /* return what is left */
}


स्थिर पूर्णाङ्क num2straux (वर्ण *buff, अचिह्नित sz, lua_सङ्ख्या x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  यदि (x != x || x == (lua_सङ्ख्या)HUGE_VAL || x == -(lua_सङ्ख्या)HUGE_VAL)
    प्रत्यागम l_sprintf(buff, sz, LUA_सङ्ख्या_FMT, (LUAI_UACNUMBER)x);
  अन्यथा यदि (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    प्रत्यागम l_sprintf(buff, sz, LUA_सङ्ख्या_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  अन्यथा {
    पूर्णाङ्क e;
    lua_सङ्ख्या m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    अचिह्नित n = 0;  /* character count */
    यदि (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    यदि (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      कुरु {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } यावत् (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    प्रत्यागम cast_int(n);
  }
}


स्थिर पूर्णाङ्क lua_number2strx (lua_State *L, वर्ण *buff, अचिह्नित sz,
                            ध्रुव वर्ण *fmt, lua_सङ्ख्या x) {
  पूर्णाङ्क n = num2straux(buff, sz, x);
  यदि (fmt[SIZELENMOD] == 'A') {
    पूर्णाङ्क i;
    प्रति (i = 0; i < n; i++)
      buff[i] = cast_char(महाक्षरम्(cast_uchar(buff[i])));
  }
  अन्यथा यदि (l_unlikely(fmt[SIZELENMOD] != 'a'))
    प्रत्यागम luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  प्रत्यागम n;
}

#यदि_अन्तः				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#परिभाषय MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#परिभाषय MAX_ITEM	120


/* valid flags in a format specification */
#यदि !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#परिभाषय L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#परिभाषय L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#परिभाषय L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#परिभाषय L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#परिभाषय L_FMTFLAGSC	"-"

#यदि_अन्तः


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#परिभाषय MAX_FORMAT	32


स्थिर शून्य addquoted (lua_L_बफर *b, ध्रुव वर्ण *s, size_t len) {
  luaL_addchar(b, '"');
  यावत् (len--) {
    यदि (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    अन्यथा यदि (iscntrl(cast_uchar(*s))) {
      वर्ण buff[10];
      यदि (!अङ्कः_वा(cast_uchar(*(s+1))))
        l_sprintf(buff, परिमाण(buff), "\\%d", (पूर्णाङ्क)cast_uchar(*s));
      अन्यथा
        l_sprintf(buff, परिमाण(buff), "\\%03d", (पूर्णाङ्क)cast_uchar(*s));
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
स्थिर पूर्णाङ्क quotefloat (lua_State *L, वर्ण *buff, lua_सङ्ख्या n) {
  ध्रुव वर्ण *s;  /* for the fixed representations */
  यदि (n == (lua_सङ्ख्या)HUGE_VAL)  /* inf? */
    s = "1e9999";
  अन्यथा यदि (n == -(lua_सङ्ख्या)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  अन्यथा यदि (n != n)  /* NaN? */
    s = "(0/0)";
  अन्यथा {  /* format number as hexadecimal */
    पूर्णाङ्क  nb = lua_number2strx(L, buff, MAX_ITEM,
                                 "%" LUA_सङ्ख्या_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    यदि (memchr(buff, '.', cast_uint(nb)) == रिक्त) {  /* no dot? */
      वर्ण point = lua_getlocaledecpoint();  /* try locale point */
      वर्ण *ppoint = (वर्ण *)memchr(buff, point, cast_uint(nb));
      यदि (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    प्रत्यागम nb;
  }
  /* for the fixed representations */
  प्रत्यागम l_sprintf(buff, MAX_ITEM, "%s", s);
}


स्थिर शून्य addliteral (lua_State *L, lua_L_बफर *b, पूर्णाङ्क arg) {
  विकल्प (lua_type(L, arg)) {
    पक्ष LUA_TSTRING: {
      size_t len;
      ध्रुव वर्ण *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      भङ्ग;
    }
    पक्ष LUA_TNUMBER: {
      वर्ण *buff = luaL_prepbuffsize(b, MAX_ITEM);
      पूर्णाङ्क nb;
      यदि (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      अन्यथा {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ध्रुव वर्ण *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ITEM, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      भङ्ग;
    }
    पक्ष LUA_TNIL: पक्ष LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, रिक्त);
      luaL_addvalue(b);
      भङ्ग;
    }
    स्वभाव: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


स्थिर ध्रुव वर्ण *get2digits (ध्रुव वर्ण *s) {
  यदि (अङ्कः_वा(cast_uchar(*s))) {
    s++;
    यदि (अङ्कः_वा(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  प्रत्यागम s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
स्थिर शून्य checkformat (lua_State *L, ध्रुव वर्ण *form, ध्रुव वर्ण *flags,
                                       पूर्णाङ्क precision) {
  ध्रुव वर्ण *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  यदि (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    यदि (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  यदि (!अक्षरम्_वा(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
स्थिर ध्रुव वर्ण *getformat (lua_State *L, ध्रुव वर्ण *strfrmt,
                                            वर्ण *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  यदि (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  स्मृतिं_प्रतिलिख(form, strfrmt, len * परिमाण(वर्ण));
  *(form + len) = '\0';
  प्रत्यागम strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
स्थिर शून्य addlenmod (वर्ण *form, ध्रुव वर्ण *lenmod) {
  size_t l = वाक्यदैर्घ्यम्(form);
  size_t lm = वाक्यदैर्घ्यम्(lenmod);
  वर्ण spec = form[l - 1];
  वाक्यं_प्रतिलिख(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


स्थिर पूर्णाङ्क str_format (lua_State *L) {
  पूर्णाङ्क शिखरम् = lua_gettop(L);
  पूर्णाङ्क arg = 1;
  size_t sfl;
  ध्रुव वर्ण *strfrmt = luaL_checklstring(L, arg, &sfl);
  ध्रुव वर्ण *strfrmt_अन्तः = strfrmt+sfl;
  ध्रुव वर्ण *flags;
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  यावत् (strfrmt < strfrmt_अन्तः) {
    यदि (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    अन्यथा यदि (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    अन्यथा { /* format item */
      वर्ण form[MAX_FORMAT];  /* to store the format ('%...') */
      अचिह्नित maxitem = MAX_ITEM;  /* maximum length for the result */
      वर्ण *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      पूर्णाङ्क nb = 0;  /* number of bytes in result */
      यदि (++arg > शिखरम्)
        प्रत्यागम luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      विकल्प (*strfrmt++) {
        पक्ष 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (पूर्णाङ्क)luaL_checkinteger(L, arg));
          भङ्ग;
        }
        पक्ष 'd': पक्ष 'i':
          flags = L_FMTFLAGSI;
          गच्छ intcase;
        पक्ष 'u':
          flags = L_FMTFLAGSU;
          गच्छ intcase;
        पक्ष 'o': पक्ष 'x': पक्ष 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          भङ्ग;
        }
        पक्ष 'a': पक्ष 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_सङ्ख्या_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          भङ्ग;
        पक्ष 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        पक्ष 'e': पक्ष 'E': पक्ष 'g': पक्ष 'G': {
          lua_सङ्ख्या n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_सङ्ख्या_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          भङ्ग;
        }
        पक्ष 'p': {
          ध्रुव शून्य *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          यदि (p == रिक्त) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /*परिणाम*/
            form[वाक्यदैर्घ्यम्(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          भङ्ग;
        }
        पक्ष 'q': {
          यदि (form[2] != '\0')  /* modifiers? */
            प्रत्यागम luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          भङ्ग;
        }
        पक्ष 's': {
          size_t l;
          ध्रुव वर्ण *s = luaL_tolstring(L, arg, &l);
          यदि (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          अन्यथा {
            luaL_argcheck(L, l == वाक्यदैर्घ्यम्(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            यदि (strchr(form, '.') == रिक्त && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            अन्यथा {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_निष्कासय(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          भङ्ग;
        }
        स्वभाव: {  /* also treat cases 'pnLlh' */
          प्रत्यागम luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  प्रत्यागम 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#यदि !defined(LUAL_PACKPADBYTE)
#परिभाषय LUAL_PACKPADBYTE		0x00
#यदि_अन्तः

/* maximum size for the binary representation of an integer */
#परिभाषय MAXINTSIZE	16

/* number of bits in a character */
#परिभाषय NB	CHAR_BIT

/* mask for one character (NB 1's) */
#परिभाषय MC	((1 << NB) - 1)

/* size of a lua_Integer */
#परिभाषय SZINT	((पूर्णाङ्क)परिमाण(lua_Integer))


/* dummy union to get native endianness */
स्थिर ध्रुव सङ्घ {
  पूर्णाङ्क dummy;
  वर्ण little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
प्रकारनिर्देश संरचना शीर्षकम् {
  lua_State *L;
  पूर्णाङ्क islittle;
  अचिह्नित maxalign;
} शीर्षकम्;


/*
** options for pack/unpack
*/
प्रकारनिर्देश गणना KOption {
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
स्थिर पूर्णाङ्क digit (पूर्णाङ्क c) { प्रत्यागम '0' <= c && c <= '9'; }

स्थिर size_t getnum (ध्रुव वर्ण **fmt, size_t df) {
  यदि (!digit(**fmt))  /* no number? */
    प्रत्यागम df;  /* return default value */
  अन्यथा {
    size_t a = 0;
    कुरु {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } यावत् (digit(**fmt) && a <= (MAX_आकार - 9)/10);
    प्रत्यागम a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
स्थिर अचिह्नित getnumlimit (शीर्षकम् *h, ध्रुव वर्ण **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  यदि (l_unlikely((sz - 1u) >= MAXINTSIZE))
    प्रत्यागम cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  प्रत्यागम cast_uint(sz);
}


/*
** Initialize Header
*/
स्थिर शून्य initheader (lua_State *L, शीर्षकम् *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
स्थिर KOption getoption (शीर्षकम् *h, ध्रुव वर्ण **fmt, size_t *आकार) {
  /* dummy structure to get native alignment requirements */
  संरचना cD { वर्ण c; सङ्घ { LUAI_MAXALIGN; } u; };
  पूर्णाङ्क opt = *((*fmt)++);
  *आकार = 0;  /* default */
  विकल्प (opt) {
    पक्ष 'b': *आकार = परिमाण(वर्ण); प्रत्यागम Kint;
    पक्ष 'B': *आकार = परिमाण(वर्ण); प्रत्यागम Kuint;
    पक्ष 'h': *आकार = परिमाण(ह्रस्व); प्रत्यागम Kint;
    पक्ष 'H': *आकार = परिमाण(ह्रस्व); प्रत्यागम Kuint;
    पक्ष 'l': *आकार = परिमाण(दीर्घ); प्रत्यागम Kint;
    पक्ष 'L': *आकार = परिमाण(दीर्घ); प्रत्यागम Kuint;
    पक्ष 'j': *आकार = परिमाण(lua_Integer); प्रत्यागम Kint;
    पक्ष 'J': *आकार = परिमाण(lua_Integer); प्रत्यागम Kuint;
    पक्ष 'T': *आकार = परिमाण(size_t); प्रत्यागम Kuint;
    पक्ष 'f': *आकार = परिमाण(भिन्न); प्रत्यागम Kfloat;
    पक्ष 'n': *आकार = परिमाण(lua_सङ्ख्या); प्रत्यागम Knumber;
    पक्ष 'd': *आकार = परिमाण(द्विगुण); प्रत्यागम Kdouble;
    पक्ष 'i': *आकार = getnumlimit(h, fmt, परिमाण(पूर्णाङ्क)); प्रत्यागम Kint;
    पक्ष 'I': *आकार = getnumlimit(h, fmt, परिमाण(पूर्णाङ्क)); प्रत्यागम Kuint;
    पक्ष 's': *आकार = getnumlimit(h, fmt, परिमाण(size_t)); प्रत्यागम Kstring;
    पक्ष 'c':
      *आकार = getnum(fmt, cast_sizet(-1));
      यदि (l_unlikely(*आकार == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      प्रत्यागम Kchar;
    पक्ष 'z': प्रत्यागम Kzstr;
    पक्ष 'x': *आकार = 1; प्रत्यागम Kpadding;
    पक्ष 'X': प्रत्यागम Kpaddalign;
    पक्ष ' ': भङ्ग;
    पक्ष '<': h->islittle = 1; भङ्ग;
    पक्ष '>': h->islittle = 0; भङ्ग;
    पक्ष '=': h->islittle = nativeendian.little; भङ्ग;
    पक्ष '!': {
      ध्रुव size_t maxalign = offsetof(संरचना cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      भङ्ग;
    }
    स्वभाव: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  प्रत्यागम Knop;
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
स्थिर KOption getdetails (शीर्षकम् *h, size_t totalsize, ध्रुव वर्ण **fmt,
                           size_t *psize, अचिह्नित *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  यदि (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    यदि (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  यदि (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  अन्यथा {
    यदि (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    यदि (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    अन्यथा {
      /* 'szmoda' = totalsize % align */
      अचिह्नित szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  प्रत्यागम opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
स्थिर शून्य packint (lua_L_बफर *b, lua_Unsigned n,
                     पूर्णाङ्क islittle, अचिह्नित आकार, पूर्णाङ्क neg) {
  वर्ण *buff = luaL_prepbuffsize(b, आकार);
  अचिह्नित i;
  buff[islittle ? 0 : आकार - 1] = (वर्ण)(n & MC);  /* first byte */
  प्रति (i = 1; i < आकार; i++) {
    n >>= NB;
    buff[islittle ? i : आकार - 1 - i] = (वर्ण)(n & MC);
  }
  यदि (neg && आकार > SZINT) {  /* negative number need sign extension? */
    प्रति (i = SZINT; i < आकार; i++)  /* correct extra bytes */
      buff[islittle ? i : आकार - 1 - i] = (वर्ण)MC;
  }
  luaL_addsize(b, आकार);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
स्थिर शून्य copywithendian (वर्ण *dest, ध्रुव वर्ण *src,
                            अचिह्नित आकार, पूर्णाङ्क islittle) {
  यदि (islittle == nativeendian.little)
    स्मृतिं_प्रतिलिख(dest, src, आकार);
  अन्यथा {
    dest += आकार - 1;
    यावत् (आकार-- != 0)
      *(dest--) = *(src++);
  }
}


स्थिर पूर्णाङ्क str_pack (lua_State *L) {
  lua_L_बफर b;
  शीर्षकम् h;
  ध्रुव वर्ण *fmt = luaL_checkstring(L, 1);  /* format string */
  पूर्णाङ्क arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  यावत् (*fmt != '\0') {
    अचिह्नित ntoalign;
    size_t आकार;
    KOption opt = getdetails(&h, totalsize, &fmt, &आकार, &ntoalign);
    luaL_argcheck(L, आकार + ntoalign <= MAX_आकार - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + आकार;
    यावत् (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    विकल्प (opt) {
      पक्ष Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        यदि (आकार < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((आकार * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(आकार), (n < 0));
        भङ्ग;
      }
      पक्ष Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        यदि (आकार < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (आकार * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(आकार), 0);
        भङ्ग;
      }
      पक्ष Kfloat: {  /* C float */
        भिन्न f = (भिन्न)luaL_checknumber(L, arg);  /* get argument */
        वर्ण *buff = luaL_prepbuffsize(&b, परिमाण(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (वर्ण *)&f, परिमाण(f), h.islittle);
        luaL_addsize(&b, आकार);
        भङ्ग;
      }
      पक्ष Knumber: {  /* Lua float */
        lua_सङ्ख्या f = luaL_checknumber(L, arg);  /* get argument */
        वर्ण *buff = luaL_prepbuffsize(&b, परिमाण(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (वर्ण *)&f, परिमाण(f), h.islittle);
        luaL_addsize(&b, आकार);
        भङ्ग;
      }
      पक्ष Kdouble: {  /* C double */
        द्विगुण f = (द्विगुण)luaL_checknumber(L, arg);  /* get argument */
        वर्ण *buff = luaL_prepbuffsize(&b, परिमाण(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (वर्ण *)&f, परिमाण(f), h.islittle);
        luaL_addsize(&b, आकार);
        भङ्ग;
      }
      पक्ष Kchar: {  /* fixed-size string */
        size_t len;
        ध्रुव वर्ण *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= आकार, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        यदि (len < आकार) {  /* does it need padding? */
          size_t psize = आकार - len;  /* pad size */
          वर्ण *buff = luaL_prepbuffsize(&b, psize);
          स्मृतिं_पूरय(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        भङ्ग;
      }
      पक्ष Kstring: {  /* strings with length count */
        size_t len;
        ध्रुव वर्ण *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, आकार >= परिमाण(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (आकार * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(आकार), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        भङ्ग;
      }
      पक्ष Kzstr: {  /* zero-terminated string */
        size_t len;
        ध्रुव वर्ण *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, वाक्यदैर्घ्यम्(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        भङ्ग;
      }
      पक्ष Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      पक्ष Kpaddalign: पक्ष Knop:
        arg--;  /* undo increment */
        भङ्ग;
    }
  }
  luaL_pushresult(&b);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क str_packsize (lua_State *L) {
  शीर्षकम् h;
  ध्रुव वर्ण *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  यावत् (*fmt != '\0') {
    अचिह्नित ntoalign;
    size_t आकार;
    KOption opt = getdetails(&h, totalsize, &fmt, &आकार, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    आकार += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_आकार - आकार,
                     1, "format result too large");
    totalsize += आकार;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  प्रत्यागम 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
स्थिर lua_Integer unpackint (lua_State *L, ध्रुव वर्ण *str,
                              पूर्णाङ्क islittle, पूर्णाङ्क आकार, पूर्णाङ्क issigned) {
  lua_Unsigned res = 0;
  पूर्णाङ्क i;
  पूर्णाङ्क सीमा = (आकार  <= SZINT) ? आकार : SZINT;
  प्रति (i = सीमा - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(अचिह्नित वर्ण)str[islittle ? i : आकार - 1 - i];
  }
  यदि (आकार < SZINT) {  /* real size smaller than lua_Integer? */
    यदि (issigned) {  /* needs sign extension? */
      lua_Unsigned आवरणम्_2 = (lua_Unsigned)1 << (आकार*NB - 1);
      res = ((res ^ आवरणम्_2) - आवरणम्_2);  /* do sign extension */
    }
  }
  अन्यथा यदि (आकार > SZINT) {  /* must check unread bytes */
    पूर्णाङ्क आवरणम्_2 = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    प्रति (i = सीमा; i < आकार; i++) {
      यदि (l_unlikely((अचिह्नित वर्ण)str[islittle ? i : आकार - 1 - i] != आवरणम्_2))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", आकार);
    }
  }
  प्रत्यागम (lua_Integer)res;
}


स्थिर पूर्णाङ्क str_unpack (lua_State *L) {
  शीर्षकम् h;
  ध्रुव वर्ण *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ध्रुव वर्ण *दत्त = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  पूर्णाङ्क n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  यावत् (*fmt != '\0') {
    अचिह्नित ntoalign;
    size_t आकार;
    KOption opt = getdetails(&h, pos, &fmt, &आकार, &ntoalign);
    luaL_argcheck(L, ntoalign + आकार <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    विकल्प (opt) {
      पक्ष Kint:
      पक्ष Kuint: {
        lua_Integer res = unpackint(L, दत्त + pos, h.islittle,
                                       cast_int(आकार), (opt == Kint));
        lua_pushinteger(L, res);
        भङ्ग;
      }
      पक्ष Kfloat: {
        भिन्न f;
        copywithendian((वर्ण *)&f, दत्त + pos, परिमाण(f), h.islittle);
        lua_pushnumber(L, (lua_सङ्ख्या)f);
        भङ्ग;
      }
      पक्ष Knumber: {
        lua_सङ्ख्या f;
        copywithendian((वर्ण *)&f, दत्त + pos, परिमाण(f), h.islittle);
        lua_pushnumber(L, f);
        भङ्ग;
      }
      पक्ष Kdouble: {
        द्विगुण f;
        copywithendian((वर्ण *)&f, दत्त + pos, परिमाण(f), h.islittle);
        lua_pushnumber(L, (lua_सङ्ख्या)f);
        भङ्ग;
      }
      पक्ष Kchar: {
        lua_pushlstring(L, दत्त + pos, आकार);
        भङ्ग;
      }
      पक्ष Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, दत्त + pos,
                                          h.islittle, cast_int(आकार), 0);
        luaL_argcheck(L, len <= ld - pos - आकार, 2, "data string too short");
        lua_pushlstring(L, दत्त + pos + आकार, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        भङ्ग;
      }
      पक्ष Kzstr: {
        size_t len = वाक्यदैर्घ्यम्(दत्त + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, दत्त + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        भङ्ग;
      }
      पक्ष Kpaddalign: पक्ष Kpadding: पक्ष Knop:
        n--;  /* undo increment */
        भङ्ग;
    }
    pos += आकार;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  प्रत्यागम n + 1;
}

/* }====================================================== */


स्थिर ध्रुव luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_find},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_अधस्तनम्},
  {"match", str_मेलनम्},
  {"rep", str_rep},
  {"reverse", str_reverse},
  {"sub", str_sub},
  {"upper", str_ऊर्ध्वस्थम्},
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
  lua_निष्कासय(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_निष्कासय(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API पूर्णाङ्क luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  प्रत्यागम 1;
}

