/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#تعریف_کرو lstrlib_c
#تعریف_کرو LUA_LIB

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
#تعریف_کرو LUA_MAXCAPTURES		32
#اگر_ختم


ساکن صحیح str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  واپس 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
ساکن size_t posrelatI (lua_Integer pos, size_t len) {
  اگر (pos > 0)
    واپس (size_t)pos;
  ورنہ اگر (pos == 0)
    واپس 1;
  ورنہ اگر (pos < -(lua_Integer)len)  /* inverted comparison */
    واپس 1;  /* clip to 1 */
  ورنہ واپس len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ساکن size_t getendpos (lua_State *L, صحیح arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  اگر (pos > (lua_Integer)len)
    واپس len;
  ورنہ اگر (pos >= 0)
    واپس (size_t)pos;
  ورنہ اگر (pos < -(lua_Integer)len)
    واپس 0;
  ورنہ واپس len + (size_t)pos + 1;
}


ساکن صحیح str_sub (lua_State *L) {
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  size_t آغاز = posrelatI(luaL_checkinteger(L, 2), l);
  size_t انتہا = getendpos(L, 3, -1, l);
  اگر (آغاز <= انتہا)
    lua_pushlstring(L, s + آغاز - 1, (انتہا - آغاز) + 1);
  ورنہ lua_pushliteral(L, "");
  واپس 1;
}


ساکن صحیح str_الٹ (lua_State *L) {
  size_t l, i;
  lua_L_بفر b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  برائے (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  واپس 1;
}


ساکن صحیح str_زیریں (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_بفر b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  برائے (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  واپس 1;
}


ساکن صحیح str_بالائی (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_بفر b;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  حرف *p = luaL_buffinitsize(L, &b, l);
  برائے (i=0; i<l; i++)
    p[i] = cast_char(بڑا_حرف(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  واپس 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
ساکن صحیح str_rep (lua_State *L) {
  size_t len, lsep;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ثابت حرف *sep = luaL_optlstring(L, 3, "", &lsep);
  اگر (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  ورنہ اگر (l_unlikely(len > MAX_سائز - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_سائز) / n))
    واپس luaL_error(L, "resulting string too large");
  ورنہ {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    lua_L_بفر b;
    حرف *p = luaL_buffinitsize(L, &b, totallen);
    جب (n-- > 1) {  /* first n-1 copies (followed by separator) */
      حافظہ_نقل(p, s, len * حجم(حرف)); p += len;
      اگر (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        حافظہ_نقل(p, sep, lsep * حجم(حرف)); p += lsep;
      }
    }
    حافظہ_نقل(p, s, len * حجم(حرف));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  واپس 1;
}


ساکن صحیح str_byte (lua_State *L) {
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  صحیح n, i;
  اگر (posi > pose) واپس 0;  /* empty interval; return no values */
  اگر (l_unlikely(pose - posi >= (size_t)زیادہ_سے_زیادہ_عدد))  /* arithmetic overflow? */
    واپس luaL_error(L, "string slice too long");
  n = (صحیح)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  برائے (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  واپس n;
}


ساکن صحیح str_char (lua_State *L) {
  صحیح n = lua_gettop(L);  /* number of arguments */
  صحیح i;
  lua_L_بفر b;
  حرف *p = luaL_buffinitsize(L, &b, cast_uint(n));
  برائے (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  واپس 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
ساخت str_Writer {
  صحیح init;  /* true iff buffer has been initialized */
  lua_L_بفر B;
};


ساکن صحیح writer (lua_State *L, ثابت خالی *b, size_t سائز, خالی *ud) {
  ساخت str_Writer *state = (ساخت str_Writer *)ud;
  اگر (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  اگر (b == تہی) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  ورنہ
    luaL_addlstring(&state->B, (ثابت حرف *)b, سائز);
  واپس 0;
}


ساکن صحیح str_dump (lua_State *L) {
  ساخت str_Writer state;
  صحیح strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  واپس 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#اگر defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ساکن ثابت luaL_Reg stringmetamethods[] = {
  {"__index", تہی},  /* placeholder */
  {تہی, تہی}
};

#ورنہ		/* }{ */

ساکن صحیح tonum (lua_State *L, صحیح arg) {
  اگر (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    واپس 1;
  }
  ورنہ {  /* check whether it is a numerical string */
    size_t len;
    ثابت حرف *s = lua_tolstring(L, arg, &len);
    واپس (s != تہی && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
ساکن خالی trymt (lua_State *L, ثابت حرف *mtkey, ثابت حرف *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  اگر (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_داخل_کرو(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ساکن صحیح arith (lua_State *L, صحیح op, ثابت حرف *mtname) {
  اگر (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  ورنہ
    trymt(L, mtname, mtname + 2);
  واپس 1;
}


ساکن صحیح arith_جوڑو (lua_State *L) {
  واپس arith(L, LUA_OPADD, "__add");
}

ساکن صحیح arith_sub (lua_State *L) {
  واپس arith(L, LUA_OPSUB, "__sub");
}

ساکن صحیح arith_mul (lua_State *L) {
  واپس arith(L, LUA_OPMUL, "__mul");
}

ساکن صحیح arith_mod (lua_State *L) {
  واپس arith(L, LUA_OPMOD, "__mod");
}

ساکن صحیح arith_pow (lua_State *L) {
  واپس arith(L, LUA_OPPOW, "__pow");
}

ساکن صحیح arith_div (lua_State *L) {
  واپس arith(L, LUA_OPDIV, "__div");
}

ساکن صحیح arith_idiv (lua_State *L) {
  واپس arith(L, LUA_OPIDIV, "__idiv");
}

ساکن صحیح arith_unm (lua_State *L) {
  واپس arith(L, LUA_OPUNM, "__unm");
}


ساکن ثابت luaL_Reg stringmetamethods[] = {
  {"__add", arith_جوڑو},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", تہی},  /* placeholder */
  {تہی, تہی}
};

#اگر_ختم		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#تعریف_کرو CAP_UNFINISHED	(-1)
#تعریف_کرو CAP_POSITION	(-2)


قسمتعریف ساخت مطابقت_State {
  ثابت حرف *src_init;  /* init of source string */
  ثابت حرف *src_انتہا;  /* end ('\0') of source string */
  ثابت حرف *p_انتہا;  /* end ('\0') of pattern */
  lua_State *L;
  صحیح matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  صحیح level;  /* total number of captures (finished or unfinished) */
  ساخت {
    ثابت حرف *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} مطابقت_State;


/* recursive function */
ساکن ثابت حرف *مطابقت (مطابقت_State *ms, ثابت حرف *s, ثابت حرف *p);


/* maximum recursion depth for 'match' */
#اگر !defined(MAXCCALLS)
#تعریف_کرو MAXCCALLS	200
#اگر_ختم


#تعریف_کرو L_ESC		'%'
#تعریف_کرو SPECIALS	"^$*+?.([%-"


ساکن صحیح check_capture (مطابقت_State *ms, صحیح l) {
  l -= '1';
  اگر (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    واپس luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  واپس l;
}


ساکن صحیح capture_مقصد_close (مطابقت_State *ms) {
  صحیح level = ms->level;
  برائے (level--; level>=0; level--)
    اگر (ms->capture[level].len == CAP_UNFINISHED) واپس level;
  واپس luaL_error(ms->L, "invalid pattern capture");
}


ساکن ثابت حرف *classend (مطابقت_State *ms, ثابت حرف *p) {
  انتخاب (*p++) {
    حالت L_ESC: {
      اگر (l_unlikely(p == ms->p_انتہا))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      واپس p+1;
    }
    حالت '[': {
      اگر (*p == '^') p++;
      کرو {  /* look for a ']' */
        اگر (l_unlikely(p == ms->p_انتہا))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        اگر (*(p++) == L_ESC && p < ms->p_انتہا)
          p++;  /* skip escapes (e.g. '%]') */
      } جب (*p != ']');
      واپس p+1;
    }
    طےشدہ: {
      واپس p;
    }
  }
}


ساکن صحیح مطابقت_class (صحیح c, صحیح cl) {
  صحیح res;
  انتخاب (tolower(cl)) {
    حالت 'a' : res = حرف_ہے(c); توڑو;
    حالت 'c' : res = iscntrl(c); توڑو;
    حالت 'd' : res = ہندسہ_ہے(c); توڑو;
    حالت 'g' : res = isgraph(c); توڑو;
    حالت 'l' : res = islower(c); توڑو;
    حالت 'p' : res = ispunct(c); توڑو;
    حالت 's' : res = خلا_ہے(c); توڑو;
    حالت 'u' : res = isupper(c); توڑو;
    حالت 'w' : res = حرف_یا_ہندسہ(c); توڑو;
    حالت 'x' : res = isxdigit(c); توڑو;
    حالت 'z' : res = (c == 0); توڑو;  /* deprecated option */
    طےشدہ: واپس (cl == c);
  }
  واپس (islower(cl) ? res : !res);
}


ساکن صحیح matchbracketclass (صحیح c, ثابت حرف *p, ثابت حرف *ec) {
  صحیح sig = 1;
  اگر (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  جب (++p < ec) {
    اگر (*p == L_ESC) {
      p++;
      اگر (مطابقت_class(c, cast_uchar(*p)))
        واپس sig;
    }
    ورنہ اگر ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      اگر (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        واپس sig;
    }
    ورنہ اگر (cast_uchar(*p) == c) واپس sig;
  }
  واپس !sig;
}


ساکن صحیح singlematch (مطابقت_State *ms, ثابت حرف *s, ثابت حرف *p,
                        ثابت حرف *ep) {
  اگر (s >= ms->src_انتہا)
    واپس 0;
  ورنہ {
    صحیح c = cast_uchar(*s);
    انتخاب (*p) {
      حالت '.': واپس 1;  /* matches any char */
      حالت L_ESC: واپس مطابقت_class(c, cast_uchar(*(p+1)));
      حالت '[': واپس matchbracketclass(c, p, ep-1);
      طےشدہ:  واپس (cast_uchar(*p) == c);
    }
  }
}


ساکن ثابت حرف *matchbalance (مطابقت_State *ms, ثابت حرف *s,
                                   ثابت حرف *p) {
  اگر (l_unlikely(p >= ms->p_انتہا - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  اگر (*s != *p) واپس تہی;
  ورنہ {
    صحیح b = *p;
    صحیح e = *(p+1);
    صحیح cont = 1;
    جب (++s < ms->src_انتہا) {
      اگر (*s == e) {
        اگر (--cont == 0) واپس s+1;
      }
      ورنہ اگر (*s == b) cont++;
    }
  }
  واپس تہی;  /* string ends out of balance */
}


ساکن ثابت حرف *max_expand (مطابقت_State *ms, ثابت حرف *s,
                                 ثابت حرف *p, ثابت حرف *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  جب (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  جب (i>=0) {
    ثابت حرف *res = مطابقت(ms, (s+i), ep+1);
    اگر (res) واپس res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  واپس تہی;
}


ساکن ثابت حرف *min_expand (مطابقت_State *ms, ثابت حرف *s,
                                 ثابت حرف *p, ثابت حرف *ep) {
  برائے (;;) {
    ثابت حرف *res = مطابقت(ms, s, ep+1);
    اگر (res != تہی)
      واپس res;
    ورنہ اگر (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    ورنہ واپس تہی;
  }
}


ساکن ثابت حرف *آغاز_capture (مطابقت_State *ms, ثابت حرف *s,
                                    ثابت حرف *p, صحیح what) {
  ثابت حرف *res;
  صحیح level = ms->level;
  اگر (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  اگر ((res=مطابقت(ms, s, p)) == تہی)  /* match failed? */
    ms->level--;  /* undo capture */
  واپس res;
}


ساکن ثابت حرف *انتہا_capture (مطابقت_State *ms, ثابت حرف *s,
                                  ثابت حرف *p) {
  صحیح l = capture_مقصد_close(ms);
  ثابت حرف *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  اگر ((res = مطابقت(ms, s, p)) == تہی)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  واپس res;
}


ساکن ثابت حرف *مطابقت_capture (مطابقت_State *ms, ثابت حرف *s, صحیح l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  اگر ((size_t)(ms->src_انتہا-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    واپس s+len;
  ورنہ واپس تہی;
}


ساکن ثابت حرف *مطابقت (مطابقت_State *ms, ثابت حرف *s, ثابت حرف *p) {
  اگر (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  اگر (p != ms->p_انتہا) {  /* end of pattern? */
    انتخاب (*p) {
      حالت '(': {  /* start capture */
        اگر (*(p + 1) == ')')  /* position capture? */
          s = آغاز_capture(ms, s, p + 2, CAP_POSITION);
        ورنہ
          s = آغاز_capture(ms, s, p + 1, CAP_UNFINISHED);
        توڑو;
      }
      حالت ')': {  /* end capture */
        s = انتہا_capture(ms, s, p + 1);
        توڑو;
      }
      حالت '$': {
        اگر ((p + 1) != ms->p_انتہا)  /* is the '$' the last char in pattern? */
          جاؤ dflt;  /* no; go to default */
        s = (s == ms->src_انتہا) ? s : تہی;  /* check end of string */
        توڑو;
      }
      حالت L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        انتخاب (*(p + 1)) {
          حالت 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            اگر (s != تہی) {
              p += 4; جاؤ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            توڑو;
          }
          حالت 'f': {  /* frontier? */
            ثابت حرف *ep; حرف پچھلا;
            p += 2;
            اگر (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            پچھلا = (s == ms->src_init) ? '\0' : *(s - 1);
            اگر (!matchbracketclass(cast_uchar(پچھلا), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; جاؤ init;  /* return match(ms, s, ep); */
            }
            s = تہی;  /* match failed */
            توڑو;
          }
          حالت '0': حالت '1': حالت '2': حالت '3':
          حالت '4': حالت '5': حالت '6': حالت '7':
          حالت '8': حالت '9': {  /* capture results (%0-%9)? */
            s = مطابقت_capture(ms, s, cast_uchar(*(p + 1)));
            اگر (s != تہی) {
              p += 2; جاؤ init;  /* return match(ms, s, p + 2) */
            }
            توڑو;
          }
          طےشدہ: جاؤ dflt;
        }
        توڑو;
      }
      طےشدہ: dflt: {  /* pattern class plus optional suffix */
        ثابت حرف *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        اگر (!singlematch(ms, s, p, ep)) {
          اگر (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; جاؤ init;  /* return match(ms, s, ep + 1); */
          }
          ورنہ  /* '+' or no suffix */
            s = تہی;  /* fail */
        }
        ورنہ {  /* matched once */
          انتخاب (*ep) {  /* handle optional suffix */
            حالت '?': {  /* optional */
              ثابت حرف *res;
              اگر ((res = مطابقت(ms, s + 1, ep + 1)) != تہی)
                s = res;
              ورنہ {
                p = ep + 1; جاؤ init;  /* else return match(ms, s, ep + 1); */
              }
              توڑو;
            }
            حالت '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            حالت '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              توڑو;
            حالت '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              توڑو;
            طےشدہ:  /* no suffix */
              s++; p = ep; جاؤ init;  /* return match(ms, s + 1, ep); */
          }
        }
        توڑو;
      }
    }
  }
  ms->matchdepth++;
  واپس s;
}



ساکن ثابت حرف *lmemfind (ثابت حرف *s1, size_t l1,
                               ثابت حرف *s2, size_t l2) {
  اگر (l2 == 0) واپس s1;  /* empty strings are everywhere */
  ورنہ اگر (l2 > l1) واپس تہی;  /* avoids a negative 'l1' */
  ورنہ {
    ثابت حرف *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    جب (l1 > 0 && (init = (ثابت حرف *)memchr(s1, *s2, l1)) != تہی) {
      init++;   /* 1st char is already checked */
      اگر (memcmp(init, s2+1, l2) == 0)
        واپس init-1;
      ورنہ {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    واپس تہی;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
ساکن ptrdiff_t get_onecapture (مطابقت_State *ms, صحیح i, ثابت حرف *s,
                              ثابت حرف *e, ثابت حرف **cap) {
  اگر (i >= ms->level) {
    اگر (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    واپس (e - s);
  }
  ورنہ {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    اگر (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    ورنہ اگر (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    واپس capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ساکن خالی ڈالو_onecapture (مطابقت_State *ms, صحیح i, ثابت حرف *s,
                                                    ثابت حرف *e) {
  ثابت حرف *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  اگر (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ساکن صحیح ڈالو_captures (مطابقت_State *ms, ثابت حرف *s, ثابت حرف *e) {
  صحیح i;
  صحیح nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  برائے (i = 0; i < nlevels; i++)
    ڈالو_onecapture(ms, i, s, e);
  واپس nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ساکن صحیح nospecials (ثابت حرف *p, size_t l) {
  size_t upto = 0;
  کرو {
    اگر (strpbrk(p + upto, SPECIALS))
      واپس 0;  /* pattern has a special character */
    upto += لڑی_لمبائی(p + upto) + 1;  /* may have more after \0 */
  } جب (upto <= l);
  واپس 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
ساکن خالی prepstate (مطابقت_State *ms, lua_State *L,
                       ثابت حرف *s, size_t ls, ثابت حرف *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_انتہا = s + ls;
  ms->p_انتہا = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
ساکن خالی reprepstate (مطابقت_State *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


ساکن صحیح str_ڈھونڈو_aux (lua_State *L, صحیح ڈھونڈو) {
  size_t ls, lp;
  ثابت حرف *s = luaL_checklstring(L, 1, &ls);
  ثابت حرف *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  اگر (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    واپس 1;
  }
  /* explicit request or no special characters? */
  اگر (ڈھونڈو && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ثابت حرف *s2 = lmemfind(s + init, ls - init, p, lp);
    اگر (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      واپس 2;
    }
  }
  ورنہ {
    مطابقت_State ms;
    ثابت حرف *s1 = s + init;
    صحیح anchor = (*p == '^');
    اگر (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    کرو {
      ثابت حرف *res;
      reprepstate(&ms);
      اگر ((res=مطابقت(&ms, s1, p)) != تہی) {
        اگر (ڈھونڈو) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /*آغاز*/
          lua_pushinteger(L, ct_diff2S(res - s));   /*انتہا*/
          واپس ڈالو_captures(&ms, تہی, 0) + 2;
        }
        ورنہ
          واپس ڈالو_captures(&ms, s1, res);
      }
    } جب (s1++ < ms.src_انتہا && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  واپس 1;
}


ساکن صحیح str_ڈھونڈو (lua_State *L) {
  واپس str_ڈھونڈو_aux(L, 1);
}


ساکن صحیح str_مطابقت (lua_State *L) {
  واپس str_ڈھونڈو_aux(L, 0);
}


/* state for 'gmatch' */
قسمتعریف ساخت GMatchState {
  ثابت حرف *src;  /* current position */
  ثابت حرف *p;  /*نمونہ*/
  ثابت حرف *lastmatch;  /* end of last match */
  مطابقت_State ms;  /* match state */
} GMatchState;


ساکن صحیح gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ثابت حرف *src;
  gm->ms.L = L;
  برائے (src = gm->src; src <= gm->ms.src_انتہا; src++) {
    ثابت حرف *e;
    reprepstate(&gm->ms);
    اگر ((e = مطابقت(&gm->ms, src, gm->p)) != تہی && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      واپس ڈالو_captures(&gm->ms, src, e);
    }
  }
  واپس 0;  /* not found */
}


ساکن صحیح gmatch (lua_State *L) {
  size_t ls, lp;
  ثابت حرف *s = luaL_checklstring(L, 1, &ls);
  ثابت حرف *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, حجم(GMatchState), 0);
  اگر (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = تہی;
  lua_pushcclosure(L, gmatch_aux, 3);
  واپس 1;
}


ساکن خالی جوڑو_s (مطابقت_State *ms, lua_L_بفر *b, ثابت حرف *s,
                                                   ثابت حرف *e) {
  size_t l;
  lua_State *L = ms->L;
  ثابت حرف *news = lua_tolstring(L, 3, &l);
  ثابت حرف *p;
  جب ((p = (حرف *)memchr(news, L_ESC, l)) != تہی) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    اگر (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    ورنہ اگر (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    ورنہ اگر (ہندسہ_ہے(cast_uchar(*p))) {  /* '%n' */
      ثابت حرف *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      اگر (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      ورنہ
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    ورنہ
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
ساکن صحیح جوڑو_قدر (مطابقت_State *ms, lua_L_بفر *b, ثابت حرف *s,
                                      ثابت حرف *e, صحیح tr) {
  lua_State *L = ms->L;
  انتخاب (tr) {
    حالت LUA_TFUNCTION: {  /* call the function */
      صحیح n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ڈالو_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      توڑو;
    }
    حالت LUA_TTABLE: {  /* index the table */
      ڈالو_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      توڑو;
    }
    طےشدہ: {  /* LUA_TNUMBER or LUA_TSTRING */
      جوڑو_s(ms, b, s, e);  /* add value to the buffer */
      واپس 1;  /* something changed */
    }
  }
  اگر (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_نکالو(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    واپس 0;  /* no changes */
  }
  ورنہ اگر (l_unlikely(!lua_isstring(L, -1)))
    واپس luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  ورنہ {
    luaL_addvalue(b);  /* add result to accumulator */
    واپس 1;  /* something changed */
  }
}


ساکن صحیح str_gsub (lua_State *L) {
  size_t srcl, lp;
  ثابت حرف *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ثابت حرف *p = luaL_checklstring(L, 2, &lp);  /*نمونہ*/
  ثابت حرف *lastmatch = تہی;  /* end of last match */
  صحیح tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  صحیح anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  صحیح بدلا = 0;  /* change flag */
  مطابقت_State ms;
  lua_L_بفر b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  اگر (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  جب (n < max_s) {
    ثابت حرف *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    اگر ((e = مطابقت(&ms, src, p)) != تہی && e != lastmatch) {  /* match? */
      n++;
      بدلا = جوڑو_قدر(&ms, &b, src, e, tr) || بدلا;
      src = lastmatch = e;
    }
    ورنہ اگر (src < ms.src_انتہا)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    ورنہ توڑو;  /* end of subject */
    اگر (anchor) توڑو;
  }
  اگر (!بدلا)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  ورنہ {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_انتہا - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  واپس 2;
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

#تعریف_کرو SIZELENMOD	(حجم(LUA_نمبر_FRMLEN)/حجم(حرف))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#تعریف_کرو L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
ساکن lua_نمبر adddigit (حرف *buff, غیرعلامتی n, lua_نمبر x) {
  lua_نمبر dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  صحیح d = (صحیح)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  واپس x - dd;  /* return what is left */
}


ساکن صحیح num2straux (حرف *buff, غیرعلامتی sz, lua_نمبر x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  اگر (x != x || x == (lua_نمبر)HUGE_VAL || x == -(lua_نمبر)HUGE_VAL)
    واپس l_sprintf(buff, sz, LUA_نمبر_FMT, (LUAI_UACNUMBER)x);
  ورنہ اگر (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    واپس l_sprintf(buff, sz, LUA_نمبر_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  ورنہ {
    صحیح e;
    lua_نمبر m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    غیرعلامتی n = 0;  /* character count */
    اگر (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    اگر (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      کرو {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } جب (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    واپس cast_int(n);
  }
}


ساکن صحیح lua_number2strx (lua_State *L, حرف *buff, غیرعلامتی sz,
                            ثابت حرف *fmt, lua_نمبر x) {
  صحیح n = num2straux(buff, sz, x);
  اگر (fmt[SIZELENMOD] == 'A') {
    صحیح i;
    برائے (i = 0; i < n; i++)
      buff[i] = cast_char(بڑا_حرف(cast_uchar(buff[i])));
  }
  ورنہ اگر (l_unlikely(fmt[SIZELENMOD] != 'a'))
    واپس luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  واپس n;
}

#اگر_ختم				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#تعریف_کرو MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#تعریف_کرو MAX_شے	120


/* valid flags in a format specification */
#اگر !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#تعریف_کرو L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#تعریف_کرو L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#تعریف_کرو L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#تعریف_کرو L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#تعریف_کرو L_FMTFLAGSC	"-"

#اگر_ختم


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#تعریف_کرو MAX_FORMAT	32


ساکن خالی addquoted (lua_L_بفر *b, ثابت حرف *s, size_t len) {
  luaL_addchar(b, '"');
  جب (len--) {
    اگر (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    ورنہ اگر (iscntrl(cast_uchar(*s))) {
      حرف buff[10];
      اگر (!ہندسہ_ہے(cast_uchar(*(s+1))))
        l_sprintf(buff, حجم(buff), "\\%d", (صحیح)cast_uchar(*s));
      ورنہ
        l_sprintf(buff, حجم(buff), "\\%03d", (صحیح)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    ورنہ
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
ساکن صحیح quotefloat (lua_State *L, حرف *buff, lua_نمبر n) {
  ثابت حرف *s;  /* for the fixed representations */
  اگر (n == (lua_نمبر)HUGE_VAL)  /* inf? */
    s = "1e9999";
  ورنہ اگر (n == -(lua_نمبر)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  ورنہ اگر (n != n)  /* NaN? */
    s = "(0/0)";
  ورنہ {  /* format number as hexadecimal */
    صحیح  nb = lua_number2strx(L, buff, MAX_شے,
                                 "%" LUA_نمبر_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    اگر (memchr(buff, '.', cast_uint(nb)) == تہی) {  /* no dot? */
      حرف نقطہ = lua_getlocaledecpoint();  /* try locale point */
      حرف *ppoint = (حرف *)memchr(buff, نقطہ, cast_uint(nb));
      اگر (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    واپس nb;
  }
  /* for the fixed representations */
  واپس l_sprintf(buff, MAX_شے, "%s", s);
}


ساکن خالی addliteral (lua_State *L, lua_L_بفر *b, صحیح arg) {
  انتخاب (lua_type(L, arg)) {
    حالت LUA_TSTRING: {
      size_t len;
      ثابت حرف *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      توڑو;
    }
    حالت LUA_TNUMBER: {
      حرف *buff = luaL_prepbuffsize(b, MAX_شے);
      صحیح nb;
      اگر (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      ورنہ {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ثابت حرف *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_شے, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      توڑو;
    }
    حالت LUA_TNIL: حالت LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, تہی);
      luaL_addvalue(b);
      توڑو;
    }
    طےشدہ: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


ساکن ثابت حرف *get2digits (ثابت حرف *s) {
  اگر (ہندسہ_ہے(cast_uchar(*s))) {
    s++;
    اگر (ہندسہ_ہے(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  واپس s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
ساکن خالی checkformat (lua_State *L, ثابت حرف *form, ثابت حرف *flags,
                                       صحیح precision) {
  ثابت حرف *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  اگر (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    اگر (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  اگر (!حرف_ہے(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
ساکن ثابت حرف *getformat (lua_State *L, ثابت حرف *strfrmt,
                                            حرف *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  اگر (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  حافظہ_نقل(form, strfrmt, len * حجم(حرف));
  *(form + len) = '\0';
  واپس strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
ساکن خالی addlenmod (حرف *form, ثابت حرف *lenmod) {
  size_t l = لڑی_لمبائی(form);
  size_t lm = لڑی_لمبائی(lenmod);
  حرف spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


ساکن صحیح str_format (lua_State *L) {
  صحیح چوٹی = lua_gettop(L);
  صحیح arg = 1;
  size_t sfl;
  ثابت حرف *strfrmt = luaL_checklstring(L, arg, &sfl);
  ثابت حرف *strfrmt_انتہا = strfrmt+sfl;
  ثابت حرف *flags;
  lua_L_بفر b;
  luaL_buffinit(L, &b);
  جب (strfrmt < strfrmt_انتہا) {
    اگر (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    ورنہ اگر (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    ورنہ { /* format item */
      حرف form[MAX_FORMAT];  /* to store the format ('%...') */
      غیرعلامتی maxitem = MAX_شے;  /* maximum length for the result */
      حرف *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      صحیح nb = 0;  /* number of bytes in result */
      اگر (++arg > چوٹی)
        واپس luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      انتخاب (*strfrmt++) {
        حالت 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (صحیح)luaL_checkinteger(L, arg));
          توڑو;
        }
        حالت 'd': حالت 'i':
          flags = L_FMTFLAGSI;
          جاؤ intcase;
        حالت 'u':
          flags = L_FMTFLAGSU;
          جاؤ intcase;
        حالت 'o': حالت 'x': حالت 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          توڑو;
        }
        حالت 'a': حالت 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_نمبر_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          توڑو;
        حالت 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        حالت 'e': حالت 'E': حالت 'g': حالت 'G': {
          lua_نمبر n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_نمبر_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          توڑو;
        }
        حالت 'p': {
          ثابت خالی *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          اگر (p == تہی) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /*نتیجہ*/
            form[لڑی_لمبائی(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          توڑو;
        }
        حالت 'q': {
          اگر (form[2] != '\0')  /* modifiers? */
            واپس luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          توڑو;
        }
        حالت 's': {
          size_t l;
          ثابت حرف *s = luaL_tolstring(L, arg, &l);
          اگر (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          ورنہ {
            luaL_argcheck(L, l == لڑی_لمبائی(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            اگر (strchr(form, '.') == تہی && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            ورنہ {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_نکالو(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          توڑو;
        }
        طےشدہ: {  /* also treat cases 'pnLlh' */
          واپس luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  واپس 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#اگر !defined(LUAL_PACKPADBYTE)
#تعریف_کرو LUAL_PACKPADBYTE		0x00
#اگر_ختم

/* maximum size for the binary representation of an integer */
#تعریف_کرو MAXINTSIZE	16

/* number of bits in a character */
#تعریف_کرو NB	CHAR_BIT

/* mask for one character (NB 1's) */
#تعریف_کرو MC	((1 << NB) - 1)

/* size of a lua_Integer */
#تعریف_کرو SZINT	((صحیح)حجم(lua_Integer))


/* dummy union to get native endianness */
ساکن ثابت اتحاد {
  صحیح dummy;
  حرف little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
قسمتعریف ساخت سرخی {
  lua_State *L;
  صحیح islittle;
  غیرعلامتی maxalign;
} سرخی;


/*
** options for pack/unpack
*/
قسمتعریف شمار KOption {
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
ساکن صحیح digit (صحیح c) { واپس '0' <= c && c <= '9'; }

ساکن size_t getnum (ثابت حرف **fmt, size_t df) {
  اگر (!digit(**fmt))  /* no number? */
    واپس df;  /* return default value */
  ورنہ {
    size_t a = 0;
    کرو {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } جب (digit(**fmt) && a <= (MAX_سائز - 9)/10);
    واپس a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ساکن غیرعلامتی getnumlimit (سرخی *h, ثابت حرف **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  اگر (l_unlikely((sz - 1u) >= MAXINTSIZE))
    واپس cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  واپس cast_uint(sz);
}


/*
** Initialize Header
*/
ساکن خالی initheader (lua_State *L, سرخی *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
ساکن KOption getoption (سرخی *h, ثابت حرف **fmt, size_t *سائز) {
  /* dummy structure to get native alignment requirements */
  ساخت cD { حرف c; اتحاد { LUAI_MAXALIGN; } u; };
  صحیح opt = *((*fmt)++);
  *سائز = 0;  /* default */
  انتخاب (opt) {
    حالت 'b': *سائز = حجم(حرف); واپس Kint;
    حالت 'B': *سائز = حجم(حرف); واپس Kuint;
    حالت 'h': *سائز = حجم(مختصر); واپس Kint;
    حالت 'H': *سائز = حجم(مختصر); واپس Kuint;
    حالت 'l': *سائز = حجم(طویل); واپس Kint;
    حالت 'L': *سائز = حجم(طویل); واپس Kuint;
    حالت 'j': *سائز = حجم(lua_Integer); واپس Kint;
    حالت 'J': *سائز = حجم(lua_Integer); واپس Kuint;
    حالت 'T': *سائز = حجم(size_t); واپس Kuint;
    حالت 'f': *سائز = حجم(اعشاری); واپس Kfloat;
    حالت 'n': *سائز = حجم(lua_نمبر); واپس Knumber;
    حالت 'd': *سائز = حجم(دوہرا); واپس Kdouble;
    حالت 'i': *سائز = getnumlimit(h, fmt, حجم(صحیح)); واپس Kint;
    حالت 'I': *سائز = getnumlimit(h, fmt, حجم(صحیح)); واپس Kuint;
    حالت 's': *سائز = getnumlimit(h, fmt, حجم(size_t)); واپس Kstring;
    حالت 'c':
      *سائز = getnum(fmt, cast_sizet(-1));
      اگر (l_unlikely(*سائز == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      واپس Kchar;
    حالت 'z': واپس Kzstr;
    حالت 'x': *سائز = 1; واپس Kpadding;
    حالت 'X': واپس Kpaddalign;
    حالت ' ': توڑو;
    حالت '<': h->islittle = 1; توڑو;
    حالت '>': h->islittle = 0; توڑو;
    حالت '=': h->islittle = nativeendian.little; توڑو;
    حالت '!': {
      ثابت size_t maxalign = offsetof(ساخت cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      توڑو;
    }
    طےشدہ: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  واپس Knop;
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
ساکن KOption getdetails (سرخی *h, size_t totalsize, ثابت حرف **fmt,
                           size_t *psize, غیرعلامتی *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  اگر (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    اگر (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  اگر (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  ورنہ {
    اگر (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    اگر (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    ورنہ {
      /* 'szmoda' = totalsize % align */
      غیرعلامتی szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  واپس opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
ساکن خالی packint (lua_L_بفر *b, lua_Unsigned n,
                     صحیح islittle, غیرعلامتی سائز, صحیح neg) {
  حرف *buff = luaL_prepbuffsize(b, سائز);
  غیرعلامتی i;
  buff[islittle ? 0 : سائز - 1] = (حرف)(n & MC);  /* first byte */
  برائے (i = 1; i < سائز; i++) {
    n >>= NB;
    buff[islittle ? i : سائز - 1 - i] = (حرف)(n & MC);
  }
  اگر (neg && سائز > SZINT) {  /* negative number need sign extension? */
    برائے (i = SZINT; i < سائز; i++)  /* correct extra bytes */
      buff[islittle ? i : سائز - 1 - i] = (حرف)MC;
  }
  luaL_addsize(b, سائز);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
ساکن خالی copywithendian (حرف *dest, ثابت حرف *src,
                            غیرعلامتی سائز, صحیح islittle) {
  اگر (islittle == nativeendian.little)
    حافظہ_نقل(dest, src, سائز);
  ورنہ {
    dest += سائز - 1;
    جب (سائز-- != 0)
      *(dest--) = *(src++);
  }
}


ساکن صحیح str_pack (lua_State *L) {
  lua_L_بفر b;
  سرخی h;
  ثابت حرف *fmt = luaL_checkstring(L, 1);  /* format string */
  صحیح arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  جب (*fmt != '\0') {
    غیرعلامتی ntoalign;
    size_t سائز;
    KOption opt = getdetails(&h, totalsize, &fmt, &سائز, &ntoalign);
    luaL_argcheck(L, سائز + ntoalign <= MAX_سائز - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + سائز;
    جب (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    انتخاب (opt) {
      حالت Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        اگر (سائز < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((سائز * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(سائز), (n < 0));
        توڑو;
      }
      حالت Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        اگر (سائز < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (سائز * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(سائز), 0);
        توڑو;
      }
      حالت Kfloat: {  /* C float */
        اعشاری f = (اعشاری)luaL_checknumber(L, arg);  /* get argument */
        حرف *buff = luaL_prepbuffsize(&b, حجم(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (حرف *)&f, حجم(f), h.islittle);
        luaL_addsize(&b, سائز);
        توڑو;
      }
      حالت Knumber: {  /* Lua float */
        lua_نمبر f = luaL_checknumber(L, arg);  /* get argument */
        حرف *buff = luaL_prepbuffsize(&b, حجم(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (حرف *)&f, حجم(f), h.islittle);
        luaL_addsize(&b, سائز);
        توڑو;
      }
      حالت Kdouble: {  /* C double */
        دوہرا f = (دوہرا)luaL_checknumber(L, arg);  /* get argument */
        حرف *buff = luaL_prepbuffsize(&b, حجم(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (حرف *)&f, حجم(f), h.islittle);
        luaL_addsize(&b, سائز);
        توڑو;
      }
      حالت Kchar: {  /* fixed-size string */
        size_t len;
        ثابت حرف *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= سائز, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        اگر (len < سائز) {  /* does it need padding? */
          size_t psize = سائز - len;  /* pad size */
          حرف *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        توڑو;
      }
      حالت Kstring: {  /* strings with length count */
        size_t len;
        ثابت حرف *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, سائز >= حجم(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (سائز * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(سائز), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        توڑو;
      }
      حالت Kzstr: {  /* zero-terminated string */
        size_t len;
        ثابت حرف *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, لڑی_لمبائی(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        توڑو;
      }
      حالت Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      حالت Kpaddalign: حالت Knop:
        arg--;  /* undo increment */
        توڑو;
    }
  }
  luaL_pushresult(&b);
  واپس 1;
}


ساکن صحیح str_packsize (lua_State *L) {
  سرخی h;
  ثابت حرف *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  جب (*fmt != '\0') {
    غیرعلامتی ntoalign;
    size_t سائز;
    KOption opt = getdetails(&h, totalsize, &fmt, &سائز, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    سائز += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_سائز - سائز,
                     1, "format result too large");
    totalsize += سائز;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  واپس 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
ساکن lua_Integer unpackint (lua_State *L, ثابت حرف *str,
                              صحیح islittle, صحیح سائز, صحیح issigned) {
  lua_Unsigned res = 0;
  صحیح i;
  صحیح حد_بندی = (سائز  <= SZINT) ? سائز : SZINT;
  برائے (i = حد_بندی - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(غیرعلامتی حرف)str[islittle ? i : سائز - 1 - i];
  }
  اگر (سائز < SZINT) {  /* real size smaller than lua_Integer? */
    اگر (issigned) {  /* needs sign extension? */
      lua_Unsigned نقاب_2 = (lua_Unsigned)1 << (سائز*NB - 1);
      res = ((res ^ نقاب_2) - نقاب_2);  /* do sign extension */
    }
  }
  ورنہ اگر (سائز > SZINT) {  /* must check unread bytes */
    صحیح نقاب_2 = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    برائے (i = حد_بندی; i < سائز; i++) {
      اگر (l_unlikely((غیرعلامتی حرف)str[islittle ? i : سائز - 1 - i] != نقاب_2))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", سائز);
    }
  }
  واپس (lua_Integer)res;
}


ساکن صحیح str_unpack (lua_State *L) {
  سرخی h;
  ثابت حرف *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ثابت حرف *ڈیٹا = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  صحیح n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  جب (*fmt != '\0') {
    غیرعلامتی ntoalign;
    size_t سائز;
    KOption opt = getdetails(&h, pos, &fmt, &سائز, &ntoalign);
    luaL_argcheck(L, ntoalign + سائز <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    انتخاب (opt) {
      حالت Kint:
      حالت Kuint: {
        lua_Integer res = unpackint(L, ڈیٹا + pos, h.islittle,
                                       cast_int(سائز), (opt == Kint));
        lua_pushinteger(L, res);
        توڑو;
      }
      حالت Kfloat: {
        اعشاری f;
        copywithendian((حرف *)&f, ڈیٹا + pos, حجم(f), h.islittle);
        lua_pushnumber(L, (lua_نمبر)f);
        توڑو;
      }
      حالت Knumber: {
        lua_نمبر f;
        copywithendian((حرف *)&f, ڈیٹا + pos, حجم(f), h.islittle);
        lua_pushnumber(L, f);
        توڑو;
      }
      حالت Kdouble: {
        دوہرا f;
        copywithendian((حرف *)&f, ڈیٹا + pos, حجم(f), h.islittle);
        lua_pushnumber(L, (lua_نمبر)f);
        توڑو;
      }
      حالت Kchar: {
        lua_pushlstring(L, ڈیٹا + pos, سائز);
        توڑو;
      }
      حالت Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, ڈیٹا + pos,
                                          h.islittle, cast_int(سائز), 0);
        luaL_argcheck(L, len <= ld - pos - سائز, 2, "data string too short");
        lua_pushlstring(L, ڈیٹا + pos + سائز, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        توڑو;
      }
      حالت Kzstr: {
        size_t len = لڑی_لمبائی(ڈیٹا + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, ڈیٹا + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        توڑو;
      }
      حالت Kpaddalign: حالت Kpadding: حالت Knop:
        n--;  /* undo increment */
        توڑو;
    }
    pos += سائز;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  واپس n + 1;
}

/* }====================================================== */


ساکن ثابت luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_ڈھونڈو},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_زیریں},
  {"match", str_مطابقت},
  {"rep", str_rep},
  {"reverse", str_الٹ},
  {"sub", str_sub},
  {"upper", str_بالائی},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {تہی, تہی}
};


ساکن خالی createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_نکالو(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_نکالو(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API صحیح luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  واپس 1;
}

