/*
** $Id: lstrlib.c $
** Standard library for string operations and pattern-matching
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lstrlib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <float.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <locale.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


/*
** maximum number of captures that a pattern can do during
** pattern-matching. This limit is arbitrary, but must fit in
** an unsigned char.
*/
#ਜੇ !defined(LUA_MAXCAPTURES)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MAXCAPTURES		32
#ਜੇ_ਅੰਤ


ਸਥਿਰ ਪੂਰਨ str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  ਵਾਪਸ 1;
}


/*
** translate a relative initial string position
** (negative means back from end): clip result to [1, inf).
** The length of any string in Lua must fit in a lua_Integer,
** so there are no overflows in the casts.
** The inverted comparison avoids a possible overflow
** computing '-pos'.
*/
ਸਥਿਰ size_t posrelatI (lua_Integer pos, size_t len) {
  ਜੇ (pos > 0)
    ਵਾਪਸ (size_t)pos;
  ਨਹੀਂਤਾਂ ਜੇ (pos == 0)
    ਵਾਪਸ 1;
  ਨਹੀਂਤਾਂ ਜੇ (pos < -(lua_Integer)len)  /* inverted comparison */
    ਵਾਪਸ 1;  /* clip to 1 */
  ਨਹੀਂਤਾਂ ਵਾਪਸ len + (size_t)pos + 1;
}


/*
** Gets an optional ending string position from argument 'arg',
** with default value 'def'.
** Negative means back from end: clip result to [0, len]
*/
ਸਥਿਰ size_t getendpos (lua_State *L, ਪੂਰਨ arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  ਜੇ (pos > (lua_Integer)len)
    ਵਾਪਸ len;
  ਨਹੀਂਤਾਂ ਜੇ (pos >= 0)
    ਵਾਪਸ (size_t)pos;
  ਨਹੀਂਤਾਂ ਜੇ (pos < -(lua_Integer)len)
    ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ ਵਾਪਸ len + (size_t)pos + 1;
}


ਸਥਿਰ ਪੂਰਨ str_sub (lua_State *L) {
  size_t l;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &l);
  size_t ਸ਼ੁਰੂ = posrelatI(luaL_checkinteger(L, 2), l);
  size_t ਸਿਰਾ = getendpos(L, 3, -1, l);
  ਜੇ (ਸ਼ੁਰੂ <= ਸਿਰਾ)
    lua_pushlstring(L, s + ਸ਼ੁਰੂ - 1, (ਸਿਰਾ - ਸ਼ੁਰੂ) + 1);
  ਨਹੀਂਤਾਂ lua_pushliteral(L, "");
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ str_ਉਲਟ (lua_State *L) {
  size_t l, i;
  lua_L_ਬਫਰ b;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &l);
  ਅੱਖਰ *p = luaL_buffinitsize(L, &b, l);
  ਲਈ (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ str_ਹੇਠਲਾ (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_ਬਫਰ b;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &l);
  ਅੱਖਰ *p = luaL_buffinitsize(L, &b, l);
  ਲਈ (i=0; i<l; i++)
    p[i] = cast_char(tolower(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ str_ਉੱਤਲਾ (lua_State *L) {
  size_t l;
  size_t i;
  lua_L_ਬਫਰ b;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &l);
  ਅੱਖਰ *p = luaL_buffinitsize(L, &b, l);
  ਲਈ (i=0; i<l; i++)
    p[i] = cast_char(ਵੱਡਾ_ਅੱਖਰ(cast_uchar(s[i])));
  luaL_pushresultsize(&b, l);
  ਵਾਪਸ 1;
}


/*
** MAX_SIZE is limited both by size_t and lua_Integer.
** When x <= MAX_SIZE, x can be safely cast to size_t or lua_Integer.
*/
ਸਥਿਰ ਪੂਰਨ str_rep (lua_State *L) {
  size_t len, lsep;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  ਅਚਲ ਅੱਖਰ *sep = luaL_optlstring(L, 3, "", &lsep);
  ਜੇ (n <= 0 || (len | lsep) == 0)
    lua_pushliteral(L, "");  /* no repetitions or both strings empty */
  ਨਹੀਂਤਾਂ ਜੇ (l_unlikely(len > MAX_ਮਾਪ - lsep ||
               cast_st2S(len + lsep) > cast_st2S(MAX_ਮਾਪ) / n))
    ਵਾਪਸ luaL_error(L, "resulting string too large");
  ਨਹੀਂਤਾਂ {
    size_t totallen = (cast_sizet(n) * (len + lsep)) - lsep;
    lua_L_ਬਫਰ b;
    ਅੱਖਰ *p = luaL_buffinitsize(L, &b, totallen);
    ਜਦੋਂ (n-- > 1) {  /* first n-1 copies (followed by separator) */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, s, len * ਆਕਾਰ(ਅੱਖਰ)); p += len;
      ਜੇ (lsep > 0) {  /* empty 'memcpy' is not that cheap */
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, sep, lsep * ਆਕਾਰ(ਅੱਖਰ)); p += lsep;
      }
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, s, len * ਆਕਾਰ(ਅੱਖਰ));  /* last copy without separator */
    luaL_pushresultsize(&b, totallen);
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ str_byte (lua_State *L) {
  size_t l;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  ਪੂਰਨ n, i;
  ਜੇ (posi > pose) ਵਾਪਸ 0;  /* empty interval; return no values */
  ਜੇ (l_unlikely(pose - posi >= (size_t)ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ))  /* arithmetic overflow? */
    ਵਾਪਸ luaL_error(L, "string slice too long");
  n = (ਪੂਰਨ)(pose -  posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  ਲਈ (i=0; i<n; i++)
    lua_pushinteger(L, cast_uchar(s[posi + cast_uint(i) - 1]));
  ਵਾਪਸ n;
}


ਸਥਿਰ ਪੂਰਨ str_char (lua_State *L) {
  ਪੂਰਨ n = lua_gettop(L);  /* number of arguments */
  ਪੂਰਨ i;
  lua_L_ਬਫਰ b;
  ਅੱਖਰ *p = luaL_buffinitsize(L, &b, cast_uint(n));
  ਲਈ (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    luaL_argcheck(L, c <= (lua_Unsigned)UCHAR_MAX, i, "value out of range");
    p[i - 1] = cast_char(cast_uchar(c));
  }
  luaL_pushresultsize(&b, cast_uint(n));
  ਵਾਪਸ 1;
}


/*
** Buffer to store the result of 'string.dump'. It must be initialized
** after the call to 'lua_dump', to ensure that the function is on the
** top of the stack when 'lua_dump' is called. ('luaL_buffinit' might
** push stuff.)
*/
ਬਣਤਰ str_Writer {
  ਪੂਰਨ init;  /* true iff buffer has been initialized */
  lua_L_ਬਫਰ B;
};


ਸਥਿਰ ਪੂਰਨ writer (lua_State *L, ਅਚਲ ਖਾਲੀ *b, size_t ਮਾਪ, ਖਾਲੀ *ud) {
  ਬਣਤਰ str_Writer *state = (ਬਣਤਰ str_Writer *)ud;
  ਜੇ (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  ਜੇ (b == ਨਿਰਾ) {  /* finishing dump? */
    luaL_pushresult(&state->B);  /* push result */
    lua_replace(L, 1);  /* move it to reserved slot */
  }
  ਨਹੀਂਤਾਂ
    luaL_addlstring(&state->B, (ਅਚਲ ਅੱਖਰ *)b, ਮਾਪ);
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ str_dump (lua_State *L) {
  ਬਣਤਰ str_Writer state;
  ਪੂਰਨ strip = lua_toboolean(L, 2);
  luaL_argcheck(L, lua_type(L, 1) == LUA_TFUNCTION && !lua_iscfunction(L, 1),
                   1, "Lua function expected");
  /* ensure function is on the top of the stack and vacate slot 1 */
  lua_pushvalue(L, 1);
  state.init = 0;
  lua_dump(L, writer, &state, strip);
  lua_settop(L, 1);  /* leave final result on top */
  ਵਾਪਸ 1;
}



/*
** {======================================================
** METAMETHODS
** =======================================================
*/

#ਜੇ defined(LUA_NOCVTS2N)	/* { */

/* no coercion from strings to numbers */

ਸਥਿਰ ਅਚਲ luaL_Reg stringmetamethods[] = {
  {"__index", ਨਿਰਾ},  /* placeholder */
  {ਨਿਰਾ, ਨਿਰਾ}
};

#ਨਹੀਂਤਾਂ		/* }{ */

ਸਥਿਰ ਪੂਰਨ tonum (lua_State *L, ਪੂਰਨ arg) {
  ਜੇ (lua_type(L, arg) == LUA_TNUMBER) {  /* already a number? */
    lua_pushvalue(L, arg);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ {  /* check whether it is a numerical string */
    size_t len;
    ਅਚਲ ਅੱਖਰ *s = lua_tolstring(L, arg, &len);
    ਵਾਪਸ (s != ਨਿਰਾ && lua_stringtonumber(L, s) == len + 1);
  }
}


/*
** To be here, either the first operand was a string or the first
** operand didn't have a corresponding metamethod. (Otherwise, that
** other metamethod would have been called.) So, if this metamethod
** doesn't work, the only other option would be for the second
** operand to have a different metamethod.
*/
ਸਥਿਰ ਖਾਲੀ trymt (lua_State *L, ਅਚਲ ਅੱਖਰ *mtkey, ਅਚਲ ਅੱਖਰ *opname) {
  lua_settop(L, 2);  /* back to the original arguments */
  ਜੇ (l_unlikely(lua_type(L, 2) == LUA_TSTRING ||
                 !luaL_getmetafield(L, 2, mtkey)))
    luaL_error(L, "attempt to %s a '%s' with a '%s'", opname,
                  luaL_typename(L, -2), luaL_typename(L, -1));
  lua_ਪਾਓ(L, -3);  /* put metamethod before arguments */
  lua_call(L, 2, 1);  /* call metamethod */
}


ਸਥਿਰ ਪੂਰਨ arith (lua_State *L, ਪੂਰਨ op, ਅਚਲ ਅੱਖਰ *mtname) {
  ਜੇ (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);  /* result will be on the top */
  ਨਹੀਂਤਾਂ
    trymt(L, mtname, mtname + 2);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ arith_ਜੋੜੋ (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPADD, "__add");
}

ਸਥਿਰ ਪੂਰਨ arith_sub (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPSUB, "__sub");
}

ਸਥਿਰ ਪੂਰਨ arith_mul (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPMUL, "__mul");
}

ਸਥਿਰ ਪੂਰਨ arith_mod (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPMOD, "__mod");
}

ਸਥਿਰ ਪੂਰਨ arith_pow (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPPOW, "__pow");
}

ਸਥਿਰ ਪੂਰਨ arith_div (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPDIV, "__div");
}

ਸਥਿਰ ਪੂਰਨ arith_idiv (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPIDIV, "__idiv");
}

ਸਥਿਰ ਪੂਰਨ arith_unm (lua_State *L) {
  ਵਾਪਸ arith(L, LUA_OPUNM, "__unm");
}


ਸਥਿਰ ਅਚਲ luaL_Reg stringmetamethods[] = {
  {"__add", arith_ਜੋੜੋ},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", ਨਿਰਾ},  /* placeholder */
  {ਨਿਰਾ, ਨਿਰਾ}
};

#ਜੇ_ਅੰਤ		/* } */

/* }====================================================== */

/*
** {======================================================
** PATTERN MATCHING
** =======================================================
*/


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAP_UNFINISHED	(-1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAP_POSITION	(-2)


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਮੇਲ_State {
  ਅਚਲ ਅੱਖਰ *src_init;  /* init of source string */
  ਅਚਲ ਅੱਖਰ *src_ਸਿਰਾ;  /* end ('\0') of source string */
  ਅਚਲ ਅੱਖਰ *p_ਸਿਰਾ;  /* end ('\0') of pattern */
  lua_State *L;
  ਪੂਰਨ matchdepth;  /* control for recursive depth (to avoid C stack overflow) */
  ਪੂਰਨ level;  /* total number of captures (finished or unfinished) */
  ਬਣਤਰ {
    ਅਚਲ ਅੱਖਰ *init;
    ptrdiff_t len;  /* length or special value (CAP_*) */
  } capture[LUA_MAXCAPTURES];
} ਮੇਲ_State;


/* recursive function */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਮੇਲ (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *p);


/* maximum recursion depth for 'match' */
#ਜੇ !defined(MAXCCALLS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXCCALLS	200
#ਜੇ_ਅੰਤ


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_ESC		'%'
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPECIALS	"^$*+?.([%-"


ਸਥਿਰ ਪੂਰਨ check_capture (ਮੇਲ_State *ms, ਪੂਰਨ l) {
  l -= '1';
  ਜੇ (l_unlikely(l < 0 || l >= ms->level ||
                 ms->capture[l].len == CAP_UNFINISHED))
    ਵਾਪਸ luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  ਵਾਪਸ l;
}


ਸਥਿਰ ਪੂਰਨ capture_ਮੰਜ਼ਿਲ_close (ਮੇਲ_State *ms) {
  ਪੂਰਨ level = ms->level;
  ਲਈ (level--; level>=0; level--)
    ਜੇ (ms->capture[level].len == CAP_UNFINISHED) ਵਾਪਸ level;
  ਵਾਪਸ luaL_error(ms->L, "invalid pattern capture");
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *classend (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *p) {
  ਚੋਣ (*p++) {
    ਹਾਲਤ L_ESC: {
      ਜੇ (l_unlikely(p == ms->p_ਸਿਰਾ))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      ਵਾਪਸ p+1;
    }
    ਹਾਲਤ '[': {
      ਜੇ (*p == '^') p++;
      ਕਰੋ {  /* look for a ']' */
        ਜੇ (l_unlikely(p == ms->p_ਸਿਰਾ))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        ਜੇ (*(p++) == L_ESC && p < ms->p_ਸਿਰਾ)
          p++;  /* skip escapes (e.g. '%]') */
      } ਜਦੋਂ (*p != ']');
      ਵਾਪਸ p+1;
    }
    ਮੂਲ: {
      ਵਾਪਸ p;
    }
  }
}


ਸਥਿਰ ਪੂਰਨ ਮੇਲ_class (ਪੂਰਨ c, ਪੂਰਨ cl) {
  ਪੂਰਨ res;
  ਚੋਣ (tolower(cl)) {
    ਹਾਲਤ 'a' : res = ਅੱਖਰ_ਹੈ(c); ਰੋਕੋ;
    ਹਾਲਤ 'c' : res = iscntrl(c); ਰੋਕੋ;
    ਹਾਲਤ 'd' : res = ਅੰਕ_ਹੈ(c); ਰੋਕੋ;
    ਹਾਲਤ 'g' : res = isgraph(c); ਰੋਕੋ;
    ਹਾਲਤ 'l' : res = islower(c); ਰੋਕੋ;
    ਹਾਲਤ 'p' : res = ispunct(c); ਰੋਕੋ;
    ਹਾਲਤ 's' : res = ਖਾਲੀ_ਥਾਂ_ਹੈ(c); ਰੋਕੋ;
    ਹਾਲਤ 'u' : res = isupper(c); ਰੋਕੋ;
    ਹਾਲਤ 'w' : res = ਅੱਖਰ_ਜਾਂ_ਅੰਕ(c); ਰੋਕੋ;
    ਹਾਲਤ 'x' : res = isxdigit(c); ਰੋਕੋ;
    ਹਾਲਤ 'z' : res = (c == 0); ਰੋਕੋ;  /* deprecated option */
    ਮੂਲ: ਵਾਪਸ (cl == c);
  }
  ਵਾਪਸ (islower(cl) ? res : !res);
}


ਸਥਿਰ ਪੂਰਨ matchbracketclass (ਪੂਰਨ c, ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *ec) {
  ਪੂਰਨ sig = 1;
  ਜੇ (*(p+1) == '^') {
    sig = 0;
    p++;  /* skip the '^' */
  }
  ਜਦੋਂ (++p < ec) {
    ਜੇ (*p == L_ESC) {
      p++;
      ਜੇ (ਮੇਲ_class(c, cast_uchar(*p)))
        ਵਾਪਸ sig;
    }
    ਨਹੀਂਤਾਂ ਜੇ ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      ਜੇ (cast_uchar(*(p-2)) <= c && c <= cast_uchar(*p))
        ਵਾਪਸ sig;
    }
    ਨਹੀਂਤਾਂ ਜੇ (cast_uchar(*p) == c) ਵਾਪਸ sig;
  }
  ਵਾਪਸ !sig;
}


ਸਥਿਰ ਪੂਰਨ singlematch (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *p,
                        ਅਚਲ ਅੱਖਰ *ep) {
  ਜੇ (s >= ms->src_ਸਿਰਾ)
    ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ c = cast_uchar(*s);
    ਚੋਣ (*p) {
      ਹਾਲਤ '.': ਵਾਪਸ 1;  /* matches any char */
      ਹਾਲਤ L_ESC: ਵਾਪਸ ਮੇਲ_class(c, cast_uchar(*(p+1)));
      ਹਾਲਤ '[': ਵਾਪਸ matchbracketclass(c, p, ep-1);
      ਮੂਲ:  ਵਾਪਸ (cast_uchar(*p) == c);
    }
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *matchbalance (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s,
                                   ਅਚਲ ਅੱਖਰ *p) {
  ਜੇ (l_unlikely(p >= ms->p_ਸਿਰਾ - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  ਜੇ (*s != *p) ਵਾਪਸ ਨਿਰਾ;
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ b = *p;
    ਪੂਰਨ e = *(p+1);
    ਪੂਰਨ cont = 1;
    ਜਦੋਂ (++s < ms->src_ਸਿਰਾ) {
      ਜੇ (*s == e) {
        ਜੇ (--cont == 0) ਵਾਪਸ s+1;
      }
      ਨਹੀਂਤਾਂ ਜੇ (*s == b) cont++;
    }
  }
  ਵਾਪਸ ਨਿਰਾ;  /* string ends out of balance */
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *max_expand (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s,
                                 ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *ep) {
  ptrdiff_t i = 0;  /* counts maximum expand for item */
  ਜਦੋਂ (singlematch(ms, s + i, p, ep))
    i++;
  /* keeps trying to match with the maximum repetitions */
  ਜਦੋਂ (i>=0) {
    ਅਚਲ ਅੱਖਰ *res = ਮੇਲ(ms, (s+i), ep+1);
    ਜੇ (res) ਵਾਪਸ res;
    i--;  /* else didn't match; reduce 1 repetition to try again */
  }
  ਵਾਪਸ ਨਿਰਾ;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *min_expand (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s,
                                 ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *ep) {
  ਲਈ (;;) {
    ਅਚਲ ਅੱਖਰ *res = ਮੇਲ(ms, s, ep+1);
    ਜੇ (res != ਨਿਰਾ)
      ਵਾਪਸ res;
    ਨਹੀਂਤਾਂ ਜੇ (singlematch(ms, s, p, ep))
      s++;  /* try with one more repetition */
    ਨਹੀਂਤਾਂ ਵਾਪਸ ਨਿਰਾ;
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ_capture (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s,
                                    ਅਚਲ ਅੱਖਰ *p, ਪੂਰਨ what) {
  ਅਚਲ ਅੱਖਰ *res;
  ਪੂਰਨ level = ms->level;
  ਜੇ (level >= LUA_MAXCAPTURES) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  ਜੇ ((res=ਮੇਲ(ms, s, p)) == ਨਿਰਾ)  /* match failed? */
    ms->level--;  /* undo capture */
  ਵਾਪਸ res;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਸਿਰਾ_capture (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s,
                                  ਅਚਲ ਅੱਖਰ *p) {
  ਪੂਰਨ l = capture_ਮੰਜ਼ਿਲ_close(ms);
  ਅਚਲ ਅੱਖਰ *res;
  ms->capture[l].len = s - ms->capture[l].init;  /* close capture */
  ਜੇ ((res = ਮੇਲ(ms, s, p)) == ਨਿਰਾ)  /* match failed? */
    ms->capture[l].len = CAP_UNFINISHED;  /* undo capture */
  ਵਾਪਸ res;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਮੇਲ_capture (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ l) {
  size_t len;
  l = check_capture(ms, l);
  len = cast_sizet(ms->capture[l].len);
  ਜੇ ((size_t)(ms->src_ਸਿਰਾ-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    ਵਾਪਸ s+len;
  ਨਹੀਂਤਾਂ ਵਾਪਸ ਨਿਰਾ;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਮੇਲ (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *p) {
  ਜੇ (l_unlikely(ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init: /* using goto to optimize tail recursion */
  ਜੇ (p != ms->p_ਸਿਰਾ) {  /* end of pattern? */
    ਚੋਣ (*p) {
      ਹਾਲਤ '(': {  /* start capture */
        ਜੇ (*(p + 1) == ')')  /* position capture? */
          s = ਸ਼ੁਰੂ_capture(ms, s, p + 2, CAP_POSITION);
        ਨਹੀਂਤਾਂ
          s = ਸ਼ੁਰੂ_capture(ms, s, p + 1, CAP_UNFINISHED);
        ਰੋਕੋ;
      }
      ਹਾਲਤ ')': {  /* end capture */
        s = ਸਿਰਾ_capture(ms, s, p + 1);
        ਰੋਕੋ;
      }
      ਹਾਲਤ '$': {
        ਜੇ ((p + 1) != ms->p_ਸਿਰਾ)  /* is the '$' the last char in pattern? */
          ਜਾਓ dflt;  /* no; go to default */
        s = (s == ms->src_ਸਿਰਾ) ? s : ਨਿਰਾ;  /* check end of string */
        ਰੋਕੋ;
      }
      ਹਾਲਤ L_ESC: {  /* escaped sequences not in the format class[*+?-]? */
        ਚੋਣ (*(p + 1)) {
          ਹਾਲਤ 'b': {  /* balanced string? */
            s = matchbalance(ms, s, p + 2);
            ਜੇ (s != ਨਿਰਾ) {
              p += 4; ਜਾਓ init;  /* return match(ms, s, p + 4); */
            }  /* else fail (s == NULL) */
            ਰੋਕੋ;
          }
          ਹਾਲਤ 'f': {  /* frontier? */
            ਅਚਲ ਅੱਖਰ *ep; ਅੱਖਰ ਪਿਛਲਾ;
            p += 2;
            ਜੇ (l_unlikely(*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);  /* points to what is next */
            ਪਿਛਲਾ = (s == ms->src_init) ? '\0' : *(s - 1);
            ਜੇ (!matchbracketclass(cast_uchar(ਪਿਛਲਾ), p, ep - 1) &&
               matchbracketclass(cast_uchar(*s), p, ep - 1)) {
              p = ep; ਜਾਓ init;  /* return match(ms, s, ep); */
            }
            s = ਨਿਰਾ;  /* match failed */
            ਰੋਕੋ;
          }
          ਹਾਲਤ '0': ਹਾਲਤ '1': ਹਾਲਤ '2': ਹਾਲਤ '3':
          ਹਾਲਤ '4': ਹਾਲਤ '5': ਹਾਲਤ '6': ਹਾਲਤ '7':
          ਹਾਲਤ '8': ਹਾਲਤ '9': {  /* capture results (%0-%9)? */
            s = ਮੇਲ_capture(ms, s, cast_uchar(*(p + 1)));
            ਜੇ (s != ਨਿਰਾ) {
              p += 2; ਜਾਓ init;  /* return match(ms, s, p + 2) */
            }
            ਰੋਕੋ;
          }
          ਮੂਲ: ਜਾਓ dflt;
        }
        ਰੋਕੋ;
      }
      ਮੂਲ: dflt: {  /* pattern class plus optional suffix */
        ਅਚਲ ਅੱਖਰ *ep = classend(ms, p);  /* points to optional suffix */
        /* does not match at least once? */
        ਜੇ (!singlematch(ms, s, p, ep)) {
          ਜੇ (*ep == '*' || *ep == '?' || *ep == '-') {  /* accept empty? */
            p = ep + 1; ਜਾਓ init;  /* return match(ms, s, ep + 1); */
          }
          ਨਹੀਂਤਾਂ  /* '+' or no suffix */
            s = ਨਿਰਾ;  /* fail */
        }
        ਨਹੀਂਤਾਂ {  /* matched once */
          ਚੋਣ (*ep) {  /* handle optional suffix */
            ਹਾਲਤ '?': {  /* optional */
              ਅਚਲ ਅੱਖਰ *res;
              ਜੇ ((res = ਮੇਲ(ms, s + 1, ep + 1)) != ਨਿਰਾ)
                s = res;
              ਨਹੀਂਤਾਂ {
                p = ep + 1; ਜਾਓ init;  /* else return match(ms, s, ep + 1); */
              }
              ਰੋਕੋ;
            }
            ਹਾਲਤ '+':  /* 1 or more repetitions */
              s++;  /* 1 match already done */
              /* FALLTHROUGH */
            ਹਾਲਤ '*':  /* 0 or more repetitions */
              s = max_expand(ms, s, p, ep);
              ਰੋਕੋ;
            ਹਾਲਤ '-':  /* 0 or more repetitions (minimum) */
              s = min_expand(ms, s, p, ep);
              ਰੋਕੋ;
            ਮੂਲ:  /* no suffix */
              s++; p = ep; ਜਾਓ init;  /* return match(ms, s + 1, ep); */
          }
        }
        ਰੋਕੋ;
      }
    }
  }
  ms->matchdepth++;
  ਵਾਪਸ s;
}



ਸਥਿਰ ਅਚਲ ਅੱਖਰ *lmemfind (ਅਚਲ ਅੱਖਰ *s1, size_t l1,
                               ਅਚਲ ਅੱਖਰ *s2, size_t l2) {
  ਜੇ (l2 == 0) ਵਾਪਸ s1;  /* empty strings are everywhere */
  ਨਹੀਂਤਾਂ ਜੇ (l2 > l1) ਵਾਪਸ ਨਿਰਾ;  /* avoids a negative 'l1' */
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *init;  /* to search for a '*s2' inside 's1' */
    l2--;  /* 1st char will be checked by 'memchr' */
    l1 = l1-l2;  /* 's2' cannot be found after that */
    ਜਦੋਂ (l1 > 0 && (init = (ਅਚਲ ਅੱਖਰ *)memchr(s1, *s2, l1)) != ਨਿਰਾ) {
      init++;   /* 1st char is already checked */
      ਜੇ (memcmp(init, s2+1, l2) == 0)
        ਵਾਪਸ init-1;
      ਨਹੀਂਤਾਂ {  /* correct 'l1' and 's1' to try again */
        l1 -= ct_diff2sz(init - s1);
        s1 = init;
      }
    }
    ਵਾਪਸ ਨਿਰਾ;  /* not found */
  }
}


/*
** get information about the i-th capture. If there are no captures
** and 'i==0', return information about the whole match, which
** is the range 's'..'e'. If the capture is a string, return
** its length and put its address in '*cap'. If it is an integer
** (a position), push it on the stack and return CAP_POSITION.
*/
ਸਥਿਰ ptrdiff_t get_onecapture (ਮੇਲ_State *ms, ਪੂਰਨ i, ਅਚਲ ਅੱਖਰ *s,
                              ਅਚਲ ਅੱਖਰ *e, ਅਚਲ ਅੱਖਰ **cap) {
  ਜੇ (i >= ms->level) {
    ਜੇ (l_unlikely(i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    ਵਾਪਸ (e - s);
  }
  ਨਹੀਂਤਾਂ {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    ਜੇ (l_unlikely(capl == CAP_UNFINISHED))
      luaL_error(ms->L, "unfinished capture");
    ਨਹੀਂਤਾਂ ਜੇ (capl == CAP_POSITION)
      lua_pushinteger(ms->L,
          ct_diff2S(ms->capture[i].init - ms->src_init) + 1);
    ਵਾਪਸ capl;
  }
}


/*
** Push the i-th capture on the stack.
*/
ਸਥਿਰ ਖਾਲੀ ਧੱਕੋ_onecapture (ਮੇਲ_State *ms, ਪੂਰਨ i, ਅਚਲ ਅੱਖਰ *s,
                                                    ਅਚਲ ਅੱਖਰ *e) {
  ਅਚਲ ਅੱਖਰ *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  ਜੇ (l != CAP_POSITION)
    lua_pushlstring(ms->L, cap, cast_sizet(l));
  /* else position was already pushed */
}


ਸਥਿਰ ਪੂਰਨ ਧੱਕੋ_captures (ਮੇਲ_State *ms, ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *e) {
  ਪੂਰਨ i;
  ਪੂਰਨ nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  ਲਈ (i = 0; i < nlevels; i++)
    ਧੱਕੋ_onecapture(ms, i, s, e);
  ਵਾਪਸ nlevels;  /* number of strings pushed */
}


/* check whether pattern has no special characters */
ਸਥਿਰ ਪੂਰਨ nospecials (ਅਚਲ ਅੱਖਰ *p, size_t l) {
  size_t upto = 0;
  ਕਰੋ {
    ਜੇ (strpbrk(p + upto, SPECIALS))
      ਵਾਪਸ 0;  /* pattern has a special character */
    upto += ਲੜੀ_ਲੰਬਾਈ(p + upto) + 1;  /* may have more after \0 */
  } ਜਦੋਂ (upto <= l);
  ਵਾਪਸ 1;  /* no special chars found */
}


/*
** Prepare state for matches. These fields are not affected by each match.
*/
ਸਥਿਰ ਖਾਲੀ prepstate (ਮੇਲ_State *ms, lua_State *L,
                       ਅਚਲ ਅੱਖਰ *s, size_t ls, ਅਚਲ ਅੱਖਰ *p, size_t lp) {
  ms->L = L;
  ms->src_init = s;
  ms->src_ਸਿਰਾ = s + ls;
  ms->p_ਸਿਰਾ = p + lp;
}


/*
** (Re)prepare state for a match, setting fields that change during
** each match.
*/
ਸਥਿਰ ਖਾਲੀ reprepstate (ਮੇਲ_State *ms) {
  ms->matchdepth = MAXCCALLS;
  ms->level = 0;
}


ਸਥਿਰ ਪੂਰਨ str_ਲੱਭੋ_aux (lua_State *L, ਪੂਰਨ ਲੱਭੋ) {
  size_t ls, lp;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &ls);
  ਅਚਲ ਅੱਖਰ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  ਜੇ (init > ls) {  /* start after string's end? */
    luaL_pushfail(L);  /* cannot find anything */
    ਵਾਪਸ 1;
  }
  /* explicit request or no special characters? */
  ਜੇ (ਲੱਭੋ && (lua_toboolean(L, 4) || nospecials(p, lp))) {
    /* do a plain search */
    ਅਚਲ ਅੱਖਰ *s2 = lmemfind(s + init, ls - init, p, lp);
    ਜੇ (s2) {
      lua_pushinteger(L, ct_diff2S(s2 - s) + 1);
      lua_pushinteger(L, cast_st2S(ct_diff2sz(s2 - s) + lp));
      ਵਾਪਸ 2;
    }
  }
  ਨਹੀਂਤਾਂ {
    ਮੇਲ_State ms;
    ਅਚਲ ਅੱਖਰ *s1 = s + init;
    ਪੂਰਨ anchor = (*p == '^');
    ਜੇ (anchor) {
      p++; lp--;  /* skip anchor character */
    }
    prepstate(&ms, L, s, ls, p, lp);
    ਕਰੋ {
      ਅਚਲ ਅੱਖਰ *res;
      reprepstate(&ms);
      ਜੇ ((res=ਮੇਲ(&ms, s1, p)) != ਨਿਰਾ) {
        ਜੇ (ਲੱਭੋ) {
          lua_pushinteger(L, ct_diff2S(s1 - s) + 1);  /*ਸ਼ੁਰੂ*/
          lua_pushinteger(L, ct_diff2S(res - s));   /*ਸਿਰਾ*/
          ਵਾਪਸ ਧੱਕੋ_captures(&ms, ਨਿਰਾ, 0) + 2;
        }
        ਨਹੀਂਤਾਂ
          ਵਾਪਸ ਧੱਕੋ_captures(&ms, s1, res);
      }
    } ਜਦੋਂ (s1++ < ms.src_ਸਿਰਾ && !anchor);
  }
  luaL_pushfail(L);  /* not found */
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ str_ਲੱਭੋ (lua_State *L) {
  ਵਾਪਸ str_ਲੱਭੋ_aux(L, 1);
}


ਸਥਿਰ ਪੂਰਨ str_ਮੇਲ (lua_State *L) {
  ਵਾਪਸ str_ਲੱਭੋ_aux(L, 0);
}


/* state for 'gmatch' */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ GMatchState {
  ਅਚਲ ਅੱਖਰ *src;  /* current position */
  ਅਚਲ ਅੱਖਰ *p;  /*ਨਮੂਨਾ*/
  ਅਚਲ ਅੱਖਰ *lastmatch;  /* end of last match */
  ਮੇਲ_State ms;  /* match state */
} GMatchState;


ਸਥਿਰ ਪੂਰਨ gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, lua_upvalueindex(3));
  ਅਚਲ ਅੱਖਰ *src;
  gm->ms.L = L;
  ਲਈ (src = gm->src; src <= gm->ms.src_ਸਿਰਾ; src++) {
    ਅਚਲ ਅੱਖਰ *e;
    reprepstate(&gm->ms);
    ਜੇ ((e = ਮੇਲ(&gm->ms, src, gm->p)) != ਨਿਰਾ && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      ਵਾਪਸ ਧੱਕੋ_captures(&gm->ms, src, e);
    }
  }
  ਵਾਪਸ 0;  /* not found */
}


ਸਥਿਰ ਪੂਰਨ gmatch (lua_State *L) {
  size_t ls, lp;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &ls);
  ਅਚਲ ਅੱਖਰ *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);  /* keep strings on closure to avoid being collected */
  gm = (GMatchState *)lua_newuserdatauv(L, ਆਕਾਰ(GMatchState), 0);
  ਜੇ (init > ls)  /* start after string's end? */
    init = ls + 1;  /* avoid overflows in 's + init' */
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = ਨਿਰਾ;
  lua_pushcclosure(L, gmatch_aux, 3);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_s (ਮੇਲ_State *ms, lua_L_ਬਫਰ *b, ਅਚਲ ਅੱਖਰ *s,
                                                   ਅਚਲ ਅੱਖਰ *e) {
  size_t l;
  lua_State *L = ms->L;
  ਅਚਲ ਅੱਖਰ *news = lua_tolstring(L, 3, &l);
  ਅਚਲ ਅੱਖਰ *p;
  ਜਦੋਂ ((p = (ਅੱਖਰ *)memchr(news, L_ESC, l)) != ਨਿਰਾ) {
    luaL_addlstring(b, news, ct_diff2sz(p - news));
    p++;  /* skip ESC */
    ਜੇ (*p == L_ESC)  /* '%%' */
      luaL_addchar(b, *p);
    ਨਹੀਂਤਾਂ ਜੇ (*p == '0')  /* '%0' */
        luaL_addlstring(b, s, ct_diff2sz(e - s));
    ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕ_ਹੈ(cast_uchar(*p))) {  /* '%n' */
      ਅਚਲ ਅੱਖਰ *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      ਜੇ (resl == CAP_POSITION)
        luaL_addvalue(b);  /* add position to accumulated result */
      ਨਹੀਂਤਾਂ
        luaL_addlstring(b, cap, cast_sizet(resl));
    }
    ਨਹੀਂਤਾਂ
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
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_ਮੁੱਲ (ਮੇਲ_State *ms, lua_L_ਬਫਰ *b, ਅਚਲ ਅੱਖਰ *s,
                                      ਅਚਲ ਅੱਖਰ *e, ਪੂਰਨ tr) {
  lua_State *L = ms->L;
  ਚੋਣ (tr) {
    ਹਾਲਤ LUA_TFUNCTION: {  /* call the function */
      ਪੂਰਨ n;
      lua_pushvalue(L, 3);  /* push the function */
      n = ਧੱਕੋ_captures(ms, s, e);  /* all captures as arguments */
      lua_call(L, n, 1);  /* call it */
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_TTABLE: {  /* index the table */
      ਧੱਕੋ_onecapture(ms, 0, s, e);  /* first capture is the index */
      lua_gettable(L, 3);
      ਰੋਕੋ;
    }
    ਮੂਲ: {  /* LUA_TNUMBER or LUA_TSTRING */
      ਜੋੜੋ_s(ms, b, s, e);  /* add value to the buffer */
      ਵਾਪਸ 1;  /* something changed */
    }
  }
  ਜੇ (!lua_toboolean(L, -1)) {  /* nil or false? */
    lua_ਕੱਢੋ(L, 1);  /* remove value */
    luaL_addlstring(b, s, ct_diff2sz(e - s));  /* keep original text */
    ਵਾਪਸ 0;  /* no changes */
  }
  ਨਹੀਂਤਾਂ ਜੇ (l_unlikely(!lua_isstring(L, -1)))
    ਵਾਪਸ luaL_error(L, "invalid replacement value (a %s)",
                         luaL_typename(L, -1));
  ਨਹੀਂਤਾਂ {
    luaL_addvalue(b);  /* add result to accumulator */
    ਵਾਪਸ 1;  /* something changed */
  }
}


ਸਥਿਰ ਪੂਰਨ str_gsub (lua_State *L) {
  size_t srcl, lp;
  ਅਚਲ ਅੱਖਰ *src = luaL_checklstring(L, 1, &srcl);  /* subject */
  ਅਚਲ ਅੱਖਰ *p = luaL_checklstring(L, 2, &lp);  /*ਨਮੂਨਾ*/
  ਅਚਲ ਅੱਖਰ *lastmatch = ਨਿਰਾ;  /* end of last match */
  ਪੂਰਨ tr = lua_type(L, 3);  /* replacement type */
  /* max replacements */
  lua_Integer max_s = luaL_optinteger(L, 4, cast_st2S(srcl) + 1);
  ਪੂਰਨ anchor = (*p == '^');
  lua_Integer n = 0;  /* replacement count */
  ਪੂਰਨ ਬਦਲਿਆ = 0;  /* change flag */
  ਮੇਲ_State ms;
  lua_L_ਬਫਰ b;
  luaL_argexpected(L, tr == LUA_TNUMBER || tr == LUA_TSTRING ||
                   tr == LUA_TFUNCTION || tr == LUA_TTABLE, 3,
                      "string/function/table");
  luaL_buffinit(L, &b);
  ਜੇ (anchor) {
    p++; lp--;  /* skip anchor character */
  }
  prepstate(&ms, L, src, srcl, p, lp);
  ਜਦੋਂ (n < max_s) {
    ਅਚਲ ਅੱਖਰ *e;
    reprepstate(&ms);  /* (re)prepare state for new match */
    ਜੇ ((e = ਮੇਲ(&ms, src, p)) != ਨਿਰਾ && e != lastmatch) {  /* match? */
      n++;
      ਬਦਲਿਆ = ਜੋੜੋ_ਮੁੱਲ(&ms, &b, src, e, tr) || ਬਦਲਿਆ;
      src = lastmatch = e;
    }
    ਨਹੀਂਤਾਂ ਜੇ (src < ms.src_ਸਿਰਾ)  /* otherwise, skip one character */
      luaL_addchar(&b, *src++);
    ਨਹੀਂਤਾਂ ਰੋਕੋ;  /* end of subject */
    ਜੇ (anchor) ਰੋਕੋ;
  }
  ਜੇ (!ਬਦਲਿਆ)  /* no changes? */
    lua_pushvalue(L, 1);  /* return original string */
  ਨਹੀਂਤਾਂ {  /* something changed */
    luaL_addlstring(&b, src, ct_diff2sz(ms.src_ਸਿਰਾ - src));
    luaL_pushresult(&b);  /* create and return new string */
  }
  lua_pushinteger(L, n);  /* number of substitutions */
  ਵਾਪਸ 2;
}

/* }====================================================== */



/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

#ਜੇ !defined(lua_number2strx)	/* { */

/*
** Hexadecimal floating-point formatter
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIZELENMOD	(ਆਕਾਰ(LUA_ਸੰਖਿਆ_FRMLEN)/ਆਕਾਰ(ਅੱਖਰ))


/*
** Number of bits that goes into the first digit. It can be any value
** between 1 and 4; the following definition tries to align the number
** to nibble boundaries by making what is left after that first digit a
** multiple of 4.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_NBFD		((l_floatatt(MANT_DIG) - 1)%4 + 1)


/*
** Add integer part of 'x' to buffer and return new 'x'
*/
ਸਥਿਰ lua_ਸੰਖਿਆ adddigit (ਅੱਖਰ *buff, ਬਿਨਾਨਿਸ਼ਾਨੀ n, lua_ਸੰਖਿਆ x) {
  lua_ਸੰਖਿਆ dd = l_mathop(floor)(x);  /* get integer part from 'x' */
  ਪੂਰਨ d = (ਪੂਰਨ)dd;
  buff[n] = cast_char(d < 10 ? d + '0' : d - 10 + 'a');  /* add to buffer */
  ਵਾਪਸ x - dd;  /* return what is left */
}


ਸਥਿਰ ਪੂਰਨ num2straux (ਅੱਖਰ *buff, ਬਿਨਾਨਿਸ਼ਾਨੀ sz, lua_ਸੰਖਿਆ x) {
  /* if 'inf' or 'NaN', format it like '%g' */
  ਜੇ (x != x || x == (lua_ਸੰਖਿਆ)HUGE_VAL || x == -(lua_ਸੰਖਿਆ)HUGE_VAL)
    ਵਾਪਸ l_sprintf(buff, sz, LUA_ਸੰਖਿਆ_FMT, (LUAI_UACNUMBER)x);
  ਨਹੀਂਤਾਂ ਜੇ (x == 0) {  /* can be -0... */
    /* create "0" or "-0" followed by exponent */
    ਵਾਪਸ l_sprintf(buff, sz, LUA_ਸੰਖਿਆ_FMT "x0p+0", (LUAI_UACNUMBER)x);
  }
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ e;
    lua_ਸੰਖਿਆ m = l_mathop(frexp)(x, &e);  /* 'x' fraction and exponent */
    ਬਿਨਾਨਿਸ਼ਾਨੀ n = 0;  /* character count */
    ਜੇ (m < 0) {  /* is number negative? */
      buff[n++] = '-';  /* add sign */
      m = -m;  /* make it positive */
    }
    buff[n++] = '0'; buff[n++] = 'x';  /* add "0x" */
    m = adddigit(buff, n++, m * (1 << L_NBFD));  /* add first digit */
    e -= L_NBFD;  /* this digit goes before the radix point */
    ਜੇ (m > 0) {  /* more digits? */
      buff[n++] = lua_getlocaledecpoint();  /* add radix point */
      ਕਰੋ {  /* add as many digits as needed */
        m = adddigit(buff, n++, m * 16);
      } ਜਦੋਂ (m > 0);
    }
    n += cast_uint(l_sprintf(buff + n, sz - n, "p%+d", e));  /* add exponent */
    lua_assert(n < sz);
    ਵਾਪਸ cast_int(n);
  }
}


ਸਥਿਰ ਪੂਰਨ lua_number2strx (lua_State *L, ਅੱਖਰ *buff, ਬਿਨਾਨਿਸ਼ਾਨੀ sz,
                            ਅਚਲ ਅੱਖਰ *fmt, lua_ਸੰਖਿਆ x) {
  ਪੂਰਨ n = num2straux(buff, sz, x);
  ਜੇ (fmt[SIZELENMOD] == 'A') {
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < n; i++)
      buff[i] = cast_char(ਵੱਡਾ_ਅੱਖਰ(cast_uchar(buff[i])));
  }
  ਨਹੀਂਤਾਂ ਜੇ (l_unlikely(fmt[SIZELENMOD] != 'a'))
    ਵਾਪਸ luaL_error(L, "modifiers for format '%%a'/'%%A' not implemented");
  ਵਾਪਸ n;
}

#ਜੇ_ਅੰਤ				/* } */


/*
** Maximum size for items formatted with '%f'. This size is produced
** by format('%.99f', -maxfloat), and is equal to 99 + 3 ('-', '.',
** and '\0') + number of decimal digits to represent maxfloat (which
** is maximum exponent + 1). (99+3+1, adding some extra, 110)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ITEMF	(110 + l_floatatt(MAX_10_EXP))


/*
** All formats except '%f' do not need that large limit.  The other
** float formats use exponents, so that they fit in the 99 limit for
** significant digits; 's' for large strings and 'q' add items directly
** to the buffer; all integer formats also fit in the 99 limit.  The
** worst case are floats: they may need 99 significant digits, plus
** '0x', '-', '.', 'e+XXXX', and '\0'. Adding some extra, 120.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_ਵਸਤੂ	120


/* valid flags in a format specification */
#ਜੇ !defined(L_FMTFLAGSF)

/* valid flags for a, A, e, E, f, F, g, and G conversions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_FMTFLAGSF	"-+#0 "

/* valid flags for o, x, and X conversions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_FMTFLAGSX	"-#0"

/* valid flags for d and i conversions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_FMTFLAGSI	"-+0 "

/* valid flags for u conversions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_FMTFLAGSU	"-0"

/* valid flags for c, p, and s conversions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ L_FMTFLAGSC	"-"

#ਜੇ_ਅੰਤ


/*
** Maximum size of each format specification (such as "%-099.99d"):
** Initial '%', flags (up to 5), width (2), period, precision (2),
** length modifier (8), conversion specifier, and final '\0', plus some
** extra.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_FORMAT	32


ਸਥਿਰ ਖਾਲੀ addquoted (lua_L_ਬਫਰ *b, ਅਚਲ ਅੱਖਰ *s, size_t len) {
  luaL_addchar(b, '"');
  ਜਦੋਂ (len--) {
    ਜੇ (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    ਨਹੀਂਤਾਂ ਜੇ (iscntrl(cast_uchar(*s))) {
      ਅੱਖਰ buff[10];
      ਜੇ (!ਅੰਕ_ਹੈ(cast_uchar(*(s+1))))
        l_sprintf(buff, ਆਕਾਰ(buff), "\\%d", (ਪੂਰਨ)cast_uchar(*s));
      ਨਹੀਂਤਾਂ
        l_sprintf(buff, ਆਕਾਰ(buff), "\\%03d", (ਪੂਰਨ)cast_uchar(*s));
      luaL_addstring(b, buff);
    }
    ਨਹੀਂਤਾਂ
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
ਸਥਿਰ ਪੂਰਨ quotefloat (lua_State *L, ਅੱਖਰ *buff, lua_ਸੰਖਿਆ n) {
  ਅਚਲ ਅੱਖਰ *s;  /* for the fixed representations */
  ਜੇ (n == (lua_ਸੰਖਿਆ)HUGE_VAL)  /* inf? */
    s = "1e9999";
  ਨਹੀਂਤਾਂ ਜੇ (n == -(lua_ਸੰਖਿਆ)HUGE_VAL)  /* -inf? */
    s = "-1e9999";
  ਨਹੀਂਤਾਂ ਜੇ (n != n)  /* NaN? */
    s = "(0/0)";
  ਨਹੀਂਤਾਂ {  /* format number as hexadecimal */
    ਪੂਰਨ  nb = lua_number2strx(L, buff, MAX_ਵਸਤੂ,
                                 "%" LUA_ਸੰਖਿਆ_FRMLEN "a", n);
    /* ensures that 'buff' string uses a dot as the radix character */
    ਜੇ (memchr(buff, '.', cast_uint(nb)) == ਨਿਰਾ) {  /* no dot? */
      ਅੱਖਰ ਬਿੰਦੂ = lua_getlocaledecpoint();  /* try locale point */
      ਅੱਖਰ *ppoint = (ਅੱਖਰ *)memchr(buff, ਬਿੰਦੂ, cast_uint(nb));
      ਜੇ (ppoint) *ppoint = '.';  /* change it to a dot */
    }
    ਵਾਪਸ nb;
  }
  /* for the fixed representations */
  ਵਾਪਸ l_sprintf(buff, MAX_ਵਸਤੂ, "%s", s);
}


ਸਥਿਰ ਖਾਲੀ addliteral (lua_State *L, lua_L_ਬਫਰ *b, ਪੂਰਨ arg) {
  ਚੋਣ (lua_type(L, arg)) {
    ਹਾਲਤ LUA_TSTRING: {
      size_t len;
      ਅਚਲ ਅੱਖਰ *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_TNUMBER: {
      ਅੱਖਰ *buff = luaL_prepbuffsize(b, MAX_ਵਸਤੂ);
      ਪੂਰਨ nb;
      ਜੇ (!lua_isinteger(L, arg))  /* float? */
        nb = quotefloat(L, buff, lua_tonumber(L, arg));
      ਨਹੀਂਤਾਂ {  /* integers */
        lua_Integer n = lua_tointeger(L, arg);
        ਅਚਲ ਅੱਖਰ *format = (n == LUA_MININTEGER)  /* corner case? */
                           ? "0x%" LUA_INTEGER_FRMLEN "x"  /* use hex */
                           : LUA_INTEGER_FMT;  /* else use default format */
        nb = l_sprintf(buff, MAX_ਵਸਤੂ, format, (LUAI_UACINT)n);
      }
      luaL_addsize(b, cast_uint(nb));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_TNIL: ਹਾਲਤ LUA_TBOOLEAN: {
      luaL_tolstring(L, arg, ਨਿਰਾ);
      luaL_addvalue(b);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *get2digits (ਅਚਲ ਅੱਖਰ *s) {
  ਜੇ (ਅੰਕ_ਹੈ(cast_uchar(*s))) {
    s++;
    ਜੇ (ਅੰਕ_ਹੈ(cast_uchar(*s))) s++;  /* (2 digits at most) */
  }
  ਵਾਪਸ s;
}


/*
** Check whether a conversion specification is valid. When called,
** first character in 'form' must be '%' and last character must
** be a valid conversion specifier. 'flags' are the accepted flags;
** 'precision' signals whether to accept a precision.
*/
ਸਥਿਰ ਖਾਲੀ checkformat (lua_State *L, ਅਚਲ ਅੱਖਰ *form, ਅਚਲ ਅੱਖਰ *flags,
                                       ਪੂਰਨ precision) {
  ਅਚਲ ਅੱਖਰ *spec = form + 1;  /* skip '%' */
  spec += strspn(spec, flags);  /* skip flags */
  ਜੇ (*spec != '0') {  /* a width cannot start with '0' */
    spec = get2digits(spec);  /* skip width */
    ਜੇ (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);  /* skip precision */
    }
  }
  ਜੇ (!ਅੱਖਰ_ਹੈ(cast_uchar(*spec)))  /* did not go to the end? */
    luaL_error(L, "invalid conversion specification: '%s'", form);
}


/*
** Get a conversion specification and copy it to 'form'.
** Return the address of its last character.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getformat (lua_State *L, ਅਚਲ ਅੱਖਰ *strfrmt,
                                            ਅੱਖਰ *form) {
  /* spans flags, width, and precision ('0' is included as a flag) */
  size_t len = strspn(strfrmt, L_FMTFLAGSF "123456789.");
  len++;  /* adds following character (should be the specifier) */
  /* still needs space for '%', '\0', plus a length modifier */
  ਜੇ (len >= MAX_FORMAT - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(form, strfrmt, len * ਆਕਾਰ(ਅੱਖਰ));
  *(form + len) = '\0';
  ਵਾਪਸ strfrmt + len - 1;
}


/*
** add length modifier into formats
*/
ਸਥਿਰ ਖਾਲੀ addlenmod (ਅੱਖਰ *form, ਅਚਲ ਅੱਖਰ *lenmod) {
  size_t l = ਲੜੀ_ਲੰਬਾਈ(form);
  size_t lm = ਲੜੀ_ਲੰਬਾਈ(lenmod);
  ਅੱਖਰ spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


ਸਥਿਰ ਪੂਰਨ str_format (lua_State *L) {
  ਪੂਰਨ ਸਿਖਰ = lua_gettop(L);
  ਪੂਰਨ arg = 1;
  size_t sfl;
  ਅਚਲ ਅੱਖਰ *strfrmt = luaL_checklstring(L, arg, &sfl);
  ਅਚਲ ਅੱਖਰ *strfrmt_ਸਿਰਾ = strfrmt+sfl;
  ਅਚਲ ਅੱਖਰ *flags;
  lua_L_ਬਫਰ b;
  luaL_buffinit(L, &b);
  ਜਦੋਂ (strfrmt < strfrmt_ਸਿਰਾ) {
    ਜੇ (*strfrmt != L_ESC)
      luaL_addchar(&b, *strfrmt++);
    ਨਹੀਂਤਾਂ ਜੇ (*++strfrmt == L_ESC)
      luaL_addchar(&b, *strfrmt++);  /* %% */
    ਨਹੀਂਤਾਂ { /* format item */
      ਅੱਖਰ form[MAX_FORMAT];  /* to store the format ('%...') */
      ਬਿਨਾਨਿਸ਼ਾਨੀ maxitem = MAX_ਵਸਤੂ;  /* maximum length for the result */
      ਅੱਖਰ *buff = luaL_prepbuffsize(&b, maxitem);  /* to put result */
      ਪੂਰਨ nb = 0;  /* number of bytes in result */
      ਜੇ (++arg > ਸਿਖਰ)
        ਵਾਪਸ luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      ਚੋਣ (*strfrmt++) {
        ਹਾਲਤ 'c': {
          checkformat(L, form, L_FMTFLAGSC, 0);
          nb = l_sprintf(buff, maxitem, form, (ਪੂਰਨ)luaL_checkinteger(L, arg));
          ਰੋਕੋ;
        }
        ਹਾਲਤ 'd': ਹਾਲਤ 'i':
          flags = L_FMTFLAGSI;
          ਜਾਓ intcase;
        ਹਾਲਤ 'u':
          flags = L_FMTFLAGSU;
          ਜਾਓ intcase;
        ਹਾਲਤ 'o': ਹਾਲਤ 'x': ਹਾਲਤ 'X':
          flags = L_FMTFLAGSX;
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, LUA_INTEGER_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACINT)n);
          ਰੋਕੋ;
        }
        ਹਾਲਤ 'a': ਹਾਲਤ 'A':
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ਸੰਖਿਆ_FRMLEN);
          nb = lua_number2strx(L, buff, maxitem, form,
                                  luaL_checknumber(L, arg));
          ਰੋਕੋ;
        ਹਾਲਤ 'f':
          maxitem = MAX_ITEMF;  /* extra space for '%f' */
          buff = luaL_prepbuffsize(&b, maxitem);
          /* FALLTHROUGH */
        ਹਾਲਤ 'e': ਹਾਲਤ 'E': ਹਾਲਤ 'g': ਹਾਲਤ 'G': {
          lua_ਸੰਖਿਆ n = luaL_checknumber(L, arg);
          checkformat(L, form, L_FMTFLAGSF, 1);
          addlenmod(form, LUA_ਸੰਖਿਆ_FRMLEN);
          nb = l_sprintf(buff, maxitem, form, (LUAI_UACNUMBER)n);
          ਰੋਕੋ;
        }
        ਹਾਲਤ 'p': {
          ਅਚਲ ਖਾਲੀ *p = lua_topointer(L, arg);
          checkformat(L, form, L_FMTFLAGSC, 0);
          ਜੇ (p == ਨਿਰਾ) {  /* avoid calling 'printf' with argument NULL */
            p = "(null)";  /*ਨਤੀਜਾ*/
            form[ਲੜੀ_ਲੰਬਾਈ(form) - 1] = 's';  /* format it as a string */
          }
          nb = l_sprintf(buff, maxitem, form, p);
          ਰੋਕੋ;
        }
        ਹਾਲਤ 'q': {
          ਜੇ (form[2] != '\0')  /* modifiers? */
            ਵਾਪਸ luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          ਰੋਕੋ;
        }
        ਹਾਲਤ 's': {
          size_t l;
          ਅਚਲ ਅੱਖਰ *s = luaL_tolstring(L, arg, &l);
          ਜੇ (form[2] == '\0')  /* no modifiers? */
            luaL_addvalue(&b);  /* keep entire string */
          ਨਹੀਂਤਾਂ {
            luaL_argcheck(L, l == ਲੜੀ_ਲੰਬਾਈ(s), arg, "string contains zeros");
            checkformat(L, form, L_FMTFLAGSC, 1);
            ਜੇ (strchr(form, '.') == ਨਿਰਾ && l >= 100) {
              /* no precision and string is too long to be formatted */
              luaL_addvalue(&b);  /* keep entire string */
            }
            ਨਹੀਂਤਾਂ {  /* format the string into 'buff' */
              nb = l_sprintf(buff, maxitem, form, s);
              lua_ਕੱਢੋ(L, 1);  /* remove result from 'luaL_tolstring' */
            }
          }
          ਰੋਕੋ;
        }
        ਮੂਲ: {  /* also treat cases 'pnLlh' */
          ਵਾਪਸ luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      lua_assert(cast_uint(nb) < maxitem);
      luaL_addsize(&b, cast_uint(nb));
    }
  }
  luaL_pushresult(&b);
  ਵਾਪਸ 1;
}

/* }====================================================== */


/*
** {======================================================
** PACK/UNPACK
** =======================================================
*/


/* value used for padding */
#ਜੇ !defined(LUAL_PACKPADBYTE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAL_PACKPADBYTE		0x00
#ਜੇ_ਅੰਤ

/* maximum size for the binary representation of an integer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXINTSIZE	16

/* number of bits in a character */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NB	CHAR_BIT

/* mask for one character (NB 1's) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MC	((1 << NB) - 1)

/* size of a lua_Integer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZINT	((ਪੂਰਨ)ਆਕਾਰ(lua_Integer))


/* dummy union to get native endianness */
ਸਥਿਰ ਅਚਲ ਸੰਘ {
  ਪੂਰਨ dummy;
  ਅੱਖਰ little;  /* true iff machine is little endian */
} nativeendian = {1};


/*
** information to pack/unpack stuff
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸਿਰਲੇਖ {
  lua_State *L;
  ਪੂਰਨ islittle;
  ਬਿਨਾਨਿਸ਼ਾਨੀ maxalign;
} ਸਿਰਲੇਖ;


/*
** options for pack/unpack
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ KOption {
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
ਸਥਿਰ ਪੂਰਨ digit (ਪੂਰਨ c) { ਵਾਪਸ '0' <= c && c <= '9'; }

ਸਥਿਰ size_t getnum (ਅਚਲ ਅੱਖਰ **fmt, size_t df) {
  ਜੇ (!digit(**fmt))  /* no number? */
    ਵਾਪਸ df;  /* return default value */
  ਨਹੀਂਤਾਂ {
    size_t a = 0;
    ਕਰੋ {
      a = a*10 + cast_uint(*((*fmt)++) - '0');
    } ਜਦੋਂ (digit(**fmt) && a <= (MAX_ਮਾਪ - 9)/10);
    ਵਾਪਸ a;
  }
}


/*
** Read an integer numeral and raises an error if it is larger
** than the maximum size of integers.
*/
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ getnumlimit (ਸਿਰਲੇਖ *h, ਅਚਲ ਅੱਖਰ **fmt, size_t df) {
  size_t sz = getnum(fmt, df);
  ਜੇ (l_unlikely((sz - 1u) >= MAXINTSIZE))
    ਵਾਪਸ cast_uint(luaL_error(h->L,
               "integral size (%d) out of limits [1,%d]", sz, MAXINTSIZE));
  ਵਾਪਸ cast_uint(sz);
}


/*
** Initialize Header
*/
ਸਥਿਰ ਖਾਲੀ initheader (lua_State *L, ਸਿਰਲੇਖ *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}


/*
** Read and classify next option. 'size' is filled with option's size.
*/
ਸਥਿਰ KOption getoption (ਸਿਰਲੇਖ *h, ਅਚਲ ਅੱਖਰ **fmt, size_t *ਮਾਪ) {
  /* dummy structure to get native alignment requirements */
  ਬਣਤਰ cD { ਅੱਖਰ c; ਸੰਘ { LUAI_MAXALIGN; } u; };
  ਪੂਰਨ opt = *((*fmt)++);
  *ਮਾਪ = 0;  /* default */
  ਚੋਣ (opt) {
    ਹਾਲਤ 'b': *ਮਾਪ = ਆਕਾਰ(ਅੱਖਰ); ਵਾਪਸ Kint;
    ਹਾਲਤ 'B': *ਮਾਪ = ਆਕਾਰ(ਅੱਖਰ); ਵਾਪਸ Kuint;
    ਹਾਲਤ 'h': *ਮਾਪ = ਆਕਾਰ(ਛੋਟਾ); ਵਾਪਸ Kint;
    ਹਾਲਤ 'H': *ਮਾਪ = ਆਕਾਰ(ਛੋਟਾ); ਵਾਪਸ Kuint;
    ਹਾਲਤ 'l': *ਮਾਪ = ਆਕਾਰ(ਲੰਮਾ); ਵਾਪਸ Kint;
    ਹਾਲਤ 'L': *ਮਾਪ = ਆਕਾਰ(ਲੰਮਾ); ਵਾਪਸ Kuint;
    ਹਾਲਤ 'j': *ਮਾਪ = ਆਕਾਰ(lua_Integer); ਵਾਪਸ Kint;
    ਹਾਲਤ 'J': *ਮਾਪ = ਆਕਾਰ(lua_Integer); ਵਾਪਸ Kuint;
    ਹਾਲਤ 'T': *ਮਾਪ = ਆਕਾਰ(size_t); ਵਾਪਸ Kuint;
    ਹਾਲਤ 'f': *ਮਾਪ = ਆਕਾਰ(ਦਸ਼ਮਲਵ); ਵਾਪਸ Kfloat;
    ਹਾਲਤ 'n': *ਮਾਪ = ਆਕਾਰ(lua_ਸੰਖਿਆ); ਵਾਪਸ Knumber;
    ਹਾਲਤ 'd': *ਮਾਪ = ਆਕਾਰ(ਦੁੱਗਣਾ); ਵਾਪਸ Kdouble;
    ਹਾਲਤ 'i': *ਮਾਪ = getnumlimit(h, fmt, ਆਕਾਰ(ਪੂਰਨ)); ਵਾਪਸ Kint;
    ਹਾਲਤ 'I': *ਮਾਪ = getnumlimit(h, fmt, ਆਕਾਰ(ਪੂਰਨ)); ਵਾਪਸ Kuint;
    ਹਾਲਤ 's': *ਮਾਪ = getnumlimit(h, fmt, ਆਕਾਰ(size_t)); ਵਾਪਸ Kstring;
    ਹਾਲਤ 'c':
      *ਮਾਪ = getnum(fmt, cast_sizet(-1));
      ਜੇ (l_unlikely(*ਮਾਪ == cast_sizet(-1)))
        luaL_error(h->L, "missing size for format option 'c'");
      ਵਾਪਸ Kchar;
    ਹਾਲਤ 'z': ਵਾਪਸ Kzstr;
    ਹਾਲਤ 'x': *ਮਾਪ = 1; ਵਾਪਸ Kpadding;
    ਹਾਲਤ 'X': ਵਾਪਸ Kpaddalign;
    ਹਾਲਤ ' ': ਰੋਕੋ;
    ਹਾਲਤ '<': h->islittle = 1; ਰੋਕੋ;
    ਹਾਲਤ '>': h->islittle = 0; ਰੋਕੋ;
    ਹਾਲਤ '=': h->islittle = nativeendian.little; ਰੋਕੋ;
    ਹਾਲਤ '!': {
      ਅਚਲ size_t maxalign = offsetof(ਬਣਤਰ cD, u);
      h->maxalign = getnumlimit(h, fmt, maxalign);
      ਰੋਕੋ;
    }
    ਮੂਲ: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  ਵਾਪਸ Knop;
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
ਸਥਿਰ KOption getdetails (ਸਿਰਲੇਖ *h, size_t totalsize, ਅਚਲ ਅੱਖਰ **fmt,
                           size_t *psize, ਬਿਨਾਨਿਸ਼ਾਨੀ *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  size_t align = *psize;  /* usually, alignment follows size */
  ਜੇ (opt == Kpaddalign) {  /* 'X' gets alignment from following option */
    ਜੇ (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  ਜੇ (align <= 1 || opt == Kchar)  /* need no alignment? */
    *ntoalign = 0;
  ਨਹੀਂਤਾਂ {
    ਜੇ (align > h->maxalign)  /* enforce maximum alignment */
      align = h->maxalign;
    ਜੇ (l_unlikely(!ispow2(align))) {  /* not a power of 2? */
      *ntoalign = 0;  /* to avoid warnings */
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    }
    ਨਹੀਂਤਾਂ {
      /* 'szmoda' = totalsize % align */
      ਬਿਨਾਨਿਸ਼ਾਨੀ szmoda = cast_uint(totalsize & (align - 1));
      *ntoalign = cast_uint((align - szmoda) & (align - 1));
    }
  }
  ਵਾਪਸ opt;
}


/*
** Pack integer 'n' with 'size' bytes and 'islittle' endianness.
** The final 'if' handles the case when 'size' is larger than
** the size of a Lua integer, correcting the extra sign-extension
** bytes if necessary (by default they would be zeros).
*/
ਸਥਿਰ ਖਾਲੀ packint (lua_L_ਬਫਰ *b, lua_Unsigned n,
                     ਪੂਰਨ islittle, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ, ਪੂਰਨ neg) {
  ਅੱਖਰ *buff = luaL_prepbuffsize(b, ਮਾਪ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  buff[islittle ? 0 : ਮਾਪ - 1] = (ਅੱਖਰ)(n & MC);  /* first byte */
  ਲਈ (i = 1; i < ਮਾਪ; i++) {
    n >>= NB;
    buff[islittle ? i : ਮਾਪ - 1 - i] = (ਅੱਖਰ)(n & MC);
  }
  ਜੇ (neg && ਮਾਪ > SZINT) {  /* negative number need sign extension? */
    ਲਈ (i = SZINT; i < ਮਾਪ; i++)  /* correct extra bytes */
      buff[islittle ? i : ਮਾਪ - 1 - i] = (ਅੱਖਰ)MC;
  }
  luaL_addsize(b, ਮਾਪ);  /* add result to buffer */
}


/*
** Copy 'size' bytes from 'src' to 'dest', correcting endianness if
** given 'islittle' is different from native endianness.
*/
ਸਥਿਰ ਖਾਲੀ copywithendian (ਅੱਖਰ *dest, ਅਚਲ ਅੱਖਰ *src,
                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ, ਪੂਰਨ islittle) {
  ਜੇ (islittle == nativeendian.little)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, src, ਮਾਪ);
  ਨਹੀਂਤਾਂ {
    dest += ਮਾਪ - 1;
    ਜਦੋਂ (ਮਾਪ-- != 0)
      *(dest--) = *(src++);
  }
}


ਸਥਿਰ ਪੂਰਨ str_pack (lua_State *L) {
  lua_L_ਬਫਰ b;
  ਸਿਰਲੇਖ h;
  ਅਚਲ ਅੱਖਰ *fmt = luaL_checkstring(L, 1);  /* format string */
  ਪੂਰਨ arg = 1;  /* current argument to pack */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  lua_pushnil(L);  /* mark to separate arguments from string buffer */
  luaL_buffinit(L, &b);
  ਜਦੋਂ (*fmt != '\0') {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ntoalign;
    size_t ਮਾਪ;
    KOption opt = getdetails(&h, totalsize, &fmt, &ਮਾਪ, &ntoalign);
    luaL_argcheck(L, ਮਾਪ + ntoalign <= MAX_ਮਾਪ - totalsize, arg,
                     "result too long");
    totalsize += ntoalign + ਮਾਪ;
    ਜਦੋਂ (ntoalign-- > 0)
     luaL_addchar(&b, LUAL_PACKPADBYTE);  /* fill alignment */
    arg++;
    ਚੋਣ (opt) {
      ਹਾਲਤ Kint: {  /* signed integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ਜੇ (ਮਾਪ < SZINT) {  /* need overflow check? */
          lua_Integer lim = (lua_Integer)1 << ((ਮਾਪ * NB) - 1);
          luaL_argcheck(L, -lim <= n && n < lim, arg, "integer overflow");
        }
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(ਮਾਪ), (n < 0));
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kuint: {  /* unsigned integers */
        lua_Integer n = luaL_checkinteger(L, arg);
        ਜੇ (ਮਾਪ < SZINT)  /* need overflow check? */
          luaL_argcheck(L, (lua_Unsigned)n < ((lua_Unsigned)1 << (ਮਾਪ * NB)),
                           arg, "unsigned overflow");
        packint(&b, (lua_Unsigned)n, h.islittle, cast_uint(ਮਾਪ), 0);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kfloat: {  /* C float */
        ਦਸ਼ਮਲਵ f = (ਦਸ਼ਮਲਵ)luaL_checknumber(L, arg);  /* get argument */
        ਅੱਖਰ *buff = luaL_prepbuffsize(&b, ਆਕਾਰ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ਅੱਖਰ *)&f, ਆਕਾਰ(f), h.islittle);
        luaL_addsize(&b, ਮਾਪ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Knumber: {  /* Lua float */
        lua_ਸੰਖਿਆ f = luaL_checknumber(L, arg);  /* get argument */
        ਅੱਖਰ *buff = luaL_prepbuffsize(&b, ਆਕਾਰ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ਅੱਖਰ *)&f, ਆਕਾਰ(f), h.islittle);
        luaL_addsize(&b, ਮਾਪ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kdouble: {  /* C double */
        ਦੁੱਗਣਾ f = (ਦੁੱਗਣਾ)luaL_checknumber(L, arg);  /* get argument */
        ਅੱਖਰ *buff = luaL_prepbuffsize(&b, ਆਕਾਰ(f));
        /* move 'f' to final result, correcting endianness if needed */
        copywithendian(buff, (ਅੱਖਰ *)&f, ਆਕਾਰ(f), h.islittle);
        luaL_addsize(&b, ਮਾਪ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kchar: {  /* fixed-size string */
        size_t len;
        ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, len <= ਮਾਪ, arg, "string longer than given size");
        luaL_addlstring(&b, s, len);  /* add string */
        ਜੇ (len < ਮਾਪ) {  /* does it need padding? */
          size_t psize = ਮਾਪ - len;  /* pad size */
          ਅੱਖਰ *buff = luaL_prepbuffsize(&b, psize);
          memset(buff, LUAL_PACKPADBYTE, psize);
          luaL_addsize(&b, psize);
        }
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kstring: {  /* strings with length count */
        size_t len;
        ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, ਮਾਪ >= ਆਕਾਰ(lua_Unsigned) ||
                         len < ((lua_Unsigned)1 << (ਮਾਪ * NB)),
                         arg, "string length does not fit in given size");
        /* pack length */
        packint(&b, (lua_Unsigned)len, h.islittle, cast_uint(ਮਾਪ), 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kzstr: {  /* zero-terminated string */
        size_t len;
        ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, arg, &len);
        luaL_argcheck(L, ਲੜੀ_ਲੰਬਾਈ(s) == len, arg, "string contains zeros");
        luaL_addlstring(&b, s, len);
        luaL_addchar(&b, '\0');  /* add zero at the end */
        totalsize += len + 1;
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kpadding: luaL_addchar(&b, LUAL_PACKPADBYTE);  /* FALLTHROUGH */
      ਹਾਲਤ Kpaddalign: ਹਾਲਤ Knop:
        arg--;  /* undo increment */
        ਰੋਕੋ;
    }
  }
  luaL_pushresult(&b);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ str_packsize (lua_State *L) {
  ਸਿਰਲੇਖ h;
  ਅਚਲ ਅੱਖਰ *fmt = luaL_checkstring(L, 1);  /* format string */
  size_t totalsize = 0;  /* accumulate total size of result */
  initheader(L, &h);
  ਜਦੋਂ (*fmt != '\0') {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ntoalign;
    size_t ਮਾਪ;
    KOption opt = getdetails(&h, totalsize, &fmt, &ਮਾਪ, &ntoalign);
    luaL_argcheck(L, opt != Kstring && opt != Kzstr, 1,
                     "variable-length format");
    ਮਾਪ += ntoalign;  /* total space used by option */
    luaL_argcheck(L, totalsize <= MAX_ਮਾਪ - ਮਾਪ,
                     1, "format result too large");
    totalsize += ਮਾਪ;
  }
  lua_pushinteger(L, cast_st2S(totalsize));
  ਵਾਪਸ 1;
}


/*
** Unpack an integer with 'size' bytes and 'islittle' endianness.
** If size is smaller than the size of a Lua integer and integer
** is signed, must do sign extension (propagating the sign to the
** higher bits); if size is larger than the size of a Lua integer,
** it must check the unread bytes to see whether they do not cause an
** overflow.
*/
ਸਥਿਰ lua_Integer unpackint (lua_State *L, ਅਚਲ ਅੱਖਰ *str,
                              ਪੂਰਨ islittle, ਪੂਰਨ ਮਾਪ, ਪੂਰਨ issigned) {
  lua_Unsigned res = 0;
  ਪੂਰਨ i;
  ਪੂਰਨ ਹੱਦਬੰਦੀ = (ਮਾਪ  <= SZINT) ? ਮਾਪ : SZINT;
  ਲਈ (i = ਹੱਦਬੰਦੀ - 1; i >= 0; i--) {
    res <<= NB;
    res |= (lua_Unsigned)(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)str[islittle ? i : ਮਾਪ - 1 - i];
  }
  ਜੇ (ਮਾਪ < SZINT) {  /* real size smaller than lua_Integer? */
    ਜੇ (issigned) {  /* needs sign extension? */
      lua_Unsigned ਨਕਾਬ_2 = (lua_Unsigned)1 << (ਮਾਪ*NB - 1);
      res = ((res ^ ਨਕਾਬ_2) - ਨਕਾਬ_2);  /* do sign extension */
    }
  }
  ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ > SZINT) {  /* must check unread bytes */
    ਪੂਰਨ ਨਕਾਬ_2 = (!issigned || (lua_Integer)res >= 0) ? 0 : MC;
    ਲਈ (i = ਹੱਦਬੰਦੀ; i < ਮਾਪ; i++) {
      ਜੇ (l_unlikely((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)str[islittle ? i : ਮਾਪ - 1 - i] != ਨਕਾਬ_2))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", ਮਾਪ);
    }
  }
  ਵਾਪਸ (lua_Integer)res;
}


ਸਥਿਰ ਪੂਰਨ str_unpack (lua_State *L) {
  ਸਿਰਲੇਖ h;
  ਅਚਲ ਅੱਖਰ *fmt = luaL_checkstring(L, 1);
  size_t ld;
  ਅਚਲ ਅੱਖਰ *ਅੰਕੜੇ = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  ਪੂਰਨ n = 0;  /* number of results */
  luaL_argcheck(L, pos <= ld, 3, "initial position out of string");
  initheader(L, &h);
  ਜਦੋਂ (*fmt != '\0') {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ntoalign;
    size_t ਮਾਪ;
    KOption opt = getdetails(&h, pos, &fmt, &ਮਾਪ, &ntoalign);
    luaL_argcheck(L, ntoalign + ਮਾਪ <= ld - pos, 2,
                    "data string too short");
    pos += ntoalign;  /* skip alignment */
    /* stack space for item + next position */
    luaL_checkstack(L, 2, "too many results");
    n++;
    ਚੋਣ (opt) {
      ਹਾਲਤ Kint:
      ਹਾਲਤ Kuint: {
        lua_Integer res = unpackint(L, ਅੰਕੜੇ + pos, h.islittle,
                                       cast_int(ਮਾਪ), (opt == Kint));
        lua_pushinteger(L, res);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kfloat: {
        ਦਸ਼ਮਲਵ f;
        copywithendian((ਅੱਖਰ *)&f, ਅੰਕੜੇ + pos, ਆਕਾਰ(f), h.islittle);
        lua_pushnumber(L, (lua_ਸੰਖਿਆ)f);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Knumber: {
        lua_ਸੰਖਿਆ f;
        copywithendian((ਅੱਖਰ *)&f, ਅੰਕੜੇ + pos, ਆਕਾਰ(f), h.islittle);
        lua_pushnumber(L, f);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kdouble: {
        ਦੁੱਗਣਾ f;
        copywithendian((ਅੱਖਰ *)&f, ਅੰਕੜੇ + pos, ਆਕਾਰ(f), h.islittle);
        lua_pushnumber(L, (lua_ਸੰਖਿਆ)f);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kchar: {
        lua_pushlstring(L, ਅੰਕੜੇ + pos, ਮਾਪ);
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kstring: {
        lua_Unsigned len = (lua_Unsigned)unpackint(L, ਅੰਕੜੇ + pos,
                                          h.islittle, cast_int(ਮਾਪ), 0);
        luaL_argcheck(L, len <= ld - pos - ਮਾਪ, 2, "data string too short");
        lua_pushlstring(L, ਅੰਕੜੇ + pos + ਮਾਪ, cast_sizet(len));
        pos += cast_sizet(len);  /* skip string */
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kzstr: {
        size_t len = ਲੜੀ_ਲੰਬਾਈ(ਅੰਕੜੇ + pos);
        luaL_argcheck(L, pos + len < ld, 2,
                         "unfinished string for format 'z'");
        lua_pushlstring(L, ਅੰਕੜੇ + pos, len);
        pos += len + 1;  /* skip string plus final '\0' */
        ਰੋਕੋ;
      }
      ਹਾਲਤ Kpaddalign: ਹਾਲਤ Kpadding: ਹਾਲਤ Knop:
        n--;  /* undo increment */
        ਰੋਕੋ;
    }
    pos += ਮਾਪ;
  }
  lua_pushinteger(L, cast_st2S(pos) + 1);  /* next position */
  ਵਾਪਸ n + 1;
}

/* }====================================================== */


ਸਥਿਰ ਅਚਲ luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_ਲੱਭੋ},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_ਹੇਠਲਾ},
  {"match", str_ਮੇਲ},
  {"rep", str_rep},
  {"reverse", str_ਉਲਟ},
  {"sub", str_sub},
  {"upper", str_ਉੱਤਲਾ},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {ਨਿਰਾ, ਨਿਰਾ}
};


ਸਥਿਰ ਖਾਲੀ createmetatable (lua_State *L) {
  /* table to be metatable for strings */
  luaL_newlibtable(L, stringmetamethods);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushliteral(L, "");  /* dummy string */
  lua_pushvalue(L, -2);  /* copy table */
  lua_setmetatable(L, -2);  /* set table as metatable for strings */
  lua_ਕੱਢੋ(L, 1);  /* pop dummy string */
  lua_pushvalue(L, -2);  /* get string library */
  lua_setfield(L, -2, "__index");  /* metatable.__index = string */
  lua_ਕੱਢੋ(L, 1);  /* pop metatable */
}


/*
** Open string library
*/
LUAMOD_API ਪੂਰਨ luaopen_string (lua_State *L) {
  luaL_newlib(L, strlib);
  createmetatable(L);
  ਵਾਪਸ 1;
}

