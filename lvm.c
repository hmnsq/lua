/*
** $Id: lvm.c $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lvm_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <float.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"


/*
** By default, use jump tables in the main interpreter loop on gcc
** and compatible compilers.
*/
#ਜੇ !defined(LUA_USE_JUMPTABLE)
#ਜੇ defined(__GNUC__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_JUMPTABLE	1
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_JUMPTABLE	0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ



/* limit for table tag-method chains (to avoid infinite loops) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXTAGLOOP	2000


/*
** 'l_intfitsf' checks whether a given integer is in the range that
** can be converted to a float without rounding. Used in comparisons.
*/

/* number of bits in the mantissa of a float */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NBM		(l_floatatt(MANT_DIG))

/*
** Check whether some integers may not fit in a float, testing whether
** (maxinteger >> NBM) > 0. (That implies (1 << NBM) <= maxinteger.)
** (The shifts are done in parts, to avoid shifting by more than the size
** of an integer. In a worst case, NBM == 113 for long double and
** sizeof(long) == 32.)
*/
#ਜੇ ((((LUA_MAXINTEGER >> (NBM / 4)) >> (NBM / 4)) >> (NBM / 4)) \
	>> (NBM - (3 * (NBM / 4))))  >  0

/* limit for integers that fit in a float */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXINTFITSF	((lua_Unsigned)1 << NBM)

/* check whether 'i' is in the interval [-MAXINTFITSF, MAXINTFITSF] */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_intfitsf(i)	((MAXINTFITSF + l_castS2U(i)) <= (2 * MAXINTFITSF))

#ਨਹੀਂਤਾਂ  /* all integers fit in a float precisely */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_intfitsf(i)	1

#ਜੇ_ਅੰਤ


/*
** Try to convert a value from string to a number value.
** If the value is not a string or is a string not representing
** a valid numeral (or if coercions from strings to numbers
** are disabled via macro 'cvt2num'), do not modify 'result'
** and return 0.
*/
ਸਥਿਰ ਪੂਰਨ l_strton (ਅਚਲ TValue *obj, TValue *ਨਤੀਜਾ) {
  lua_assert(obj != ਨਤੀਜਾ);
  ਜੇ (!cvt2num(obj))  /* is object not a string? */
    ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ {
    TString *st = tsvalue(obj);
    size_t stlen;
    ਅਚਲ ਅੱਖਰ *s = getlstr(st, stlen);
    ਵਾਪਸ (luaO_str2num(s, ਨਤੀਜਾ) == stlen + 1);
  }
}


/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'tonumber'.
*/
ਪੂਰਨ luaV_tonumber_ (ਅਚਲ TValue *obj, lua_ਸੰਖਿਆ *n) {
  TValue v;
  ਜੇ (ttisinteger(obj)) {
    *n = cast_ਅੰਕ(ivalue(obj));
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ ਜੇ (l_strton(obj, &v)) {  /* string coercible to number? */
    *n = nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;  /* conversion failed */
}


/*
** try to convert a float to an integer, rounding according to 'mode'.
*/
ਪੂਰਨ luaV_flttointeger (lua_ਸੰਖਿਆ n, lua_Integer *p, F2Imod ਬਹੁਲਕ) {
  lua_ਸੰਖਿਆ f = l_floor(n);
  ਜੇ (n != f) {  /* not an integral value? */
    ਜੇ (ਬਹੁਲਕ == F2Ieq) ਵਾਪਸ 0;  /* fails if mode demands integral value */
    ਨਹੀਂਤਾਂ ਜੇ (ਬਹੁਲਕ == F2Iceil)  /* needs ceiling? */
      f += 1;  /* convert floor to ceiling (remember: n != f) */
  }
  ਵਾਪਸ lua_numbertointeger(f, p);
}


/*
** try to convert a value to an integer, rounding according to 'mode',
** without string coercion.
** ("Fast track" handled by macro 'tointegerns'.)
*/
ਪੂਰਨ luaV_tointegerns (ਅਚਲ TValue *obj, lua_Integer *p, F2Imod ਬਹੁਲਕ) {
  ਜੇ (ttisfloat(obj))
    ਵਾਪਸ luaV_flttointeger(fltvalue(obj), p, ਬਹੁਲਕ);
  ਨਹੀਂਤਾਂ ਜੇ (ttisinteger(obj)) {
    *p = ivalue(obj);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;
}


/*
** try to convert a value to an integer.
*/
ਪੂਰਨ luaV_tointeger (ਅਚਲ TValue *obj, lua_Integer *p, F2Imod ਬਹੁਲਕ) {
  TValue v;
  ਜੇ (l_strton(obj, &v))  /* does 'obj' point to a numerical string? */
    obj = &v;  /* change it to point to its corresponding number */
  ਵਾਪਸ luaV_tointegerns(obj, p, ਬਹੁਲਕ);
}


/*
** Try to convert a 'for' limit to an integer, preserving the semantics
** of the loop. Return true if the loop must not run; otherwise, '*p'
** gets the integer limit.
** (The following explanation assumes a positive step; it is valid for
** negative steps mutatis mutandis.)
** If the limit is an integer or can be converted to an integer,
** rounding down, that is the limit.
** Otherwise, check whether the limit can be converted to a float. If
** the float is too large, clip it to LUA_MAXINTEGER.  If the float
** is too negative, the loop should not run, because any initial
** integer value is greater than such limit; so, the function returns
** true to signal that. (For this latter case, no integer limit would be
** correct; even a limit of LUA_MININTEGER would run the loop once for
** an initial value equal to LUA_MININTEGER.)
*/
ਸਥਿਰ ਪੂਰਨ forlimit (lua_State *L, lua_Integer init, ਅਚਲ TValue *lim,
                                   lua_Integer *p, lua_Integer ਕਦਮ) {
  ਜੇ (!luaV_tointeger(lim, p, (ਕਦਮ < 0 ? F2Iceil : F2Ifloor))) {
    /* not coercible to in integer */
    lua_ਸੰਖਿਆ flim;  /* try to convert to float */
    ਜੇ (!tonumber(lim, &flim)) /* cannot convert to float? */
      luaG_forerror(L, lim, "limit");
    /* else 'flim' is a float out of integer bounds */
    ਜੇ (luai_numlt(0, flim)) {  /* if it is positive, it is too large */
      ਜੇ (ਕਦਮ < 0) ਵਾਪਸ 1;  /* initial value must be less than it */
      *p = LUA_MAXINTEGER;  /* truncate */
    }
    ਨਹੀਂਤਾਂ ਜੇ (luai_numgt(0, flim)) {  /* if it is negative, it is too small */
      ਜੇ (ਕਦਮ > 0) ਵਾਪਸ 1;  /* initial value must be greater than it */
      *p = LUA_MININTEGER;  /* truncate */
    }
    ਨਹੀਂਤਾਂ ਵਾਪਸ 1;  /* limit is NaN (neither 0 nor positive nor negative) */
  }
  ਵਾਪਸ (ਕਦਮ > 0 ? init > *p : init < *p);  /* not to run? */
}


/*
** Prepare a numerical for loop (opcode OP_FORPREP).
** Before execution, stack is as follows:
**   ra     : initial value
**   ra + 1 : limit
**   ra + 2 : step
** Return true to skip the loop. Otherwise,
** after preparation, stack will be as follows:
**   ra     : loop counter (integer loops) or limit (float loops)
**   ra + 1 : step
**   ra + 2 : control variable
*/
ਸਥਿਰ ਪੂਰਨ forprep (lua_State *L, StkId ra) {
  TValue *pinit = s2v(ra);
  TValue *plimit = s2v(ra + 1);
  TValue *pstep = s2v(ra + 2);
  ਜੇ (ttisinteger(pinit) && ttisinteger(pstep)) { /* integer loop? */
    lua_Integer init = ivalue(pinit);
    lua_Integer ਕਦਮ = ivalue(pstep);
    lua_Integer ਹੱਦਬੰਦੀ;
    ਜੇ (ਕਦਮ == 0)
      luaG_runerror(L, "'for' step is zero");
    ਜੇ (forlimit(L, init, plimit, &ਹੱਦਬੰਦੀ, ਕਦਮ))
      ਵਾਪਸ 1;  /* skip the loop */
    ਨਹੀਂਤਾਂ {  /* prepare loop counter */
      lua_Unsigned ਗਿਣਤੀ_2;
      ਜੇ (ਕਦਮ > 0) {  /* ascending loop? */
        ਗਿਣਤੀ_2 = l_castS2U(ਹੱਦਬੰਦੀ) - l_castS2U(init);
        ਜੇ (ਕਦਮ != 1)  /* avoid division in the too common case */
          ਗਿਣਤੀ_2 /= l_castS2U(ਕਦਮ);
      }
      ਨਹੀਂਤਾਂ {  /* step < 0; descending loop */
        ਗਿਣਤੀ_2 = l_castS2U(init) - l_castS2U(ਹੱਦਬੰਦੀ);
        /* 'step+1' avoids negating 'mininteger' */
        ਗਿਣਤੀ_2 /= l_castS2U(-(ਕਦਮ + 1)) + 1u;
      }
      /* use 'chgivalue' for places that for sure had integers */
      chgivalue(s2v(ra), l_castU2S(ਗਿਣਤੀ_2));  /* change init to count */
      setivalue(s2v(ra + 1), ਕਦਮ);  /* change limit to step */
      chgivalue(s2v(ra + 2), init);  /* change step to init */
    }
  }
  ਨਹੀਂਤਾਂ {  /* try making all values floats */
    lua_ਸੰਖਿਆ init; lua_ਸੰਖਿਆ ਹੱਦਬੰਦੀ; lua_ਸੰਖਿਆ ਕਦਮ;
    ਜੇ (l_unlikely(!tonumber(plimit, &ਹੱਦਬੰਦੀ)))
      luaG_forerror(L, plimit, "limit");
    ਜੇ (l_unlikely(!tonumber(pstep, &ਕਦਮ)))
      luaG_forerror(L, pstep, "step");
    ਜੇ (l_unlikely(!tonumber(pinit, &init)))
      luaG_forerror(L, pinit, "initial value");
    ਜੇ (ਕਦਮ == 0)
      luaG_runerror(L, "'for' step is zero");
    ਜੇ (!(luai_numge(0, ਕਦਮ) ? luai_numge(init, ਹੱਦਬੰਦੀ)
                              : luai_numge(ਹੱਦਬੰਦੀ, init)))
      ਵਾਪਸ 1;  /* skip the loop */
    ਨਹੀਂਤਾਂ {
      /* make sure all values are floats */
      setfltvalue(s2v(ra), ਹੱਦਬੰਦੀ);
      setfltvalue(s2v(ra + 1), ਕਦਮ);
      setfltvalue(s2v(ra + 2), init);  /* control variable */
    }
  }
  ਵਾਪਸ 0;
}


/*
** Execute a step of a float numerical for loop, returning
** true iff the loop must continue. (The integer case is
** written inline with opcode OP_FORLOOP, for performance.)
*/
ਸਥਿਰ ਪੂਰਨ floatforloop (lua_State *L, StkId ra) {
  lua_ਸੰਖਿਆ ਕਦਮ = fltvalue(s2v(ra + 1));
  lua_ਸੰਖਿਆ ਹੱਦਬੰਦੀ = fltvalue(s2v(ra));
  lua_ਸੰਖਿਆ idx = fltvalue(s2v(ra + 2));  /* control variable */
  idx = luai_numadd(L, idx, ਕਦਮ);  /* increment index */
  ਜੇ (luai_numlt(0, ਕਦਮ) ? luai_numle(idx, ਹੱਦਬੰਦੀ)
                          : luai_numle(ਹੱਦਬੰਦੀ, idx)) {
    chgfltvalue(s2v(ra + 2), idx);  /* update control variable */
    ਵਾਪਸ 1;  /* jump back */
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;  /* finish the loop */
}


/*
** Finish the table access 'val = t[key]' and return the tag of the result.
*/
lu_byte luaV_finishget (lua_State *L, ਅਚਲ TValue *t, TValue *ਕੁੰਜੀ,
                                      StkId val, lu_byte ਟੈਗ) {
  ਪੂਰਨ loop;  /* counter to avoid infinite loops */
  ਅਚਲ TValue *tm;  /* metamethod */
  ਲਈ (loop = 0; loop < MAXTAGLOOP; loop++) {
    ਜੇ (ਟੈਗ == LUA_VNOTABLE) {  /* 't' is not a table? */
      lua_assert(!ttistable(t));
      tm = luaT_gettmbyobj(L, t, TM_ਸੂਚਕਾਂਕ);
      ਜੇ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    ਨਹੀਂਤਾਂ {  /* 't' is a table */
      tm = fasttm(L, hvalue(t)->metatable, TM_ਸੂਚਕਾਂਕ);  /* table's metamethod */
      ਜੇ (tm == ਨਿਰਾ) {  /* no metamethod? */
        setnilvalue2s(val);  /* result is nil */
        ਵਾਪਸ LUA_VNIL;
      }
      /* else will try the metamethod */
    }
    ਜੇ (ttisfunction(tm)) {  /* is metamethod a function? */
      ਟੈਗ = luaT_callTMres(L, tm, t, ਕੁੰਜੀ, val);  /* call it */
      ਵਾਪਸ ਟੈਗ;  /* return tag of the result */
    }
    t = tm;  /* else try to access 'tm[key]' */
    luaV_fastget(t, ਕੁੰਜੀ, s2v(val), luaH_get, ਟੈਗ);
    ਜੇ (!tagisempty(ਟੈਗ))
      ਵਾਪਸ ਟੈਗ;  /*ਪੂਰਾ*/
    /* else repeat (tail call 'luaV_finishget') */
  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
  ਵਾਪਸ 0;  /* to avoid warnings */
}


/*
** Finish a table assignment 't[key] = val'.
** About anchoring the table before the call to 'luaH_finishset':
** This call may trigger an emergency collection. When loop>0,
** the table being accessed is a field in some metatable. If this
** metatable is weak and the table is not anchored, this collection
** could collect that table while it is being updated.
*/
ਖਾਲੀ luaV_finishset (lua_State *L, ਅਚਲ TValue *t, TValue *ਕੁੰਜੀ,
                      TValue *val, ਪੂਰਨ hres) {
  ਪੂਰਨ loop;  /* counter to avoid infinite loops */
  ਲਈ (loop = 0; loop < MAXTAGLOOP; loop++) {
    ਅਚਲ TValue *tm;  /* '__newindex' metamethod */
    ਜੇ (hres != HNOTATABLE) {  /* is 't' a table? */
      ਸਾਰਣੀ *h = hvalue(t);  /* save 't' table */
      tm = fasttm(L, h->metatable, TM_NEWINDEX);  /* get metamethod */
      ਜੇ (tm == ਨਿਰਾ) {  /* no metamethod? */
        sethvalue2s(L, L->ਸਿਖਰ.p, h);  /* anchor 't' */
        L->ਸਿਖਰ.p++;  /* assume EXTRA_STACK */
        luaH_finishset(L, h, ਕੁੰਜੀ, val, hres);  /* set new value */
        L->ਸਿਖਰ.p--;
        invalidateTMcache(h);
        luaC_barrierback(L, obj2gco(h), val);
        ਵਾਪਸ;
      }
      /* else will try the metamethod */
    }
    ਨਹੀਂਤਾਂ {  /* not a table; check metamethod */
      tm = luaT_gettmbyobj(L, t, TM_NEWINDEX);
      ਜੇ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");
    }
    /* try the metamethod */
    ਜੇ (ttisfunction(tm)) {
      luaT_callTM(L, tm, t, ਕੁੰਜੀ, val);
      ਵਾਪਸ;
    }
    t = tm;  /* else must repeat assignment over 'tm' */
    /* do the equivalent to 'luaV_fastset', but saving 'h' */
    ਜੇ (!ttistable(t))
      hres = HNOTATABLE;
    ਨਹੀਂਤਾਂ {
      ਸਾਰਣੀ *h = hvalue(t);  /* next call can change the value at 't' */
      hres = luaH_pset(h, ਕੁੰਜੀ, val);
      ਜੇ (hres == HOK) {
        luaC_barrierback(L, obj2gco(h), val);  /* luaV_finishfastset */
        ਵਾਪਸ;  /*ਪੂਰਾ*/
      }
    }
    /* else 'return luaV_finishset(L, t, key, val, hres)' (loop) */
  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}


/*
** Function to be used for 0-terminated string order comparison
*/
#ਜੇ !defined(l_strcoll)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_strcoll	strcoll
#ਜੇ_ਅੰਤ


/*
** Compare two strings 'ts1' x 'ts2', returning an integer less-equal-
** -greater than zero if 'ts1' is less-equal-greater than 'ts2'.
** The code is a little tricky because it allows '\0' in the strings
** and it uses 'strcoll' (to respect locales) for each segment
** of the strings. Note that segments can compare equal but still
** have different lengths.
*/
ਸਥਿਰ ਪੂਰਨ l_strcmp (ਅਚਲ TString *ts1, ਅਚਲ TString *ts2) {
  size_t rl1;  /* real length */
  ਅਚਲ ਅੱਖਰ *s1 = getlstr(ts1, rl1);
  size_t rl2;
  ਅਚਲ ਅੱਖਰ *s2 = getlstr(ts2, rl2);
  ਲਈ (;;) {  /* for each segment */
    ਪੂਰਨ ਆਰਜ਼ੀ = l_strcoll(s1, s2);
    ਜੇ (ਆਰਜ਼ੀ != 0)  /* not equal? */
      ਵਾਪਸ ਆਰਜ਼ੀ;  /*ਪੂਰਾ*/
    ਨਹੀਂਤਾਂ {  /* strings are equal up to a '\0' */
      size_t zl1 = ਲੜੀ_ਲੰਬਾਈ(s1);  /* index of first '\0' in 's1' */
      size_t zl2 = ਲੜੀ_ਲੰਬਾਈ(s2);  /* index of first '\0' in 's2' */
      ਜੇ (zl2 == rl2)  /* 's2' is finished? */
        ਵਾਪਸ (zl1 == rl1) ? 0 : 1;  /* check 's1' */
      ਨਹੀਂਤਾਂ ਜੇ (zl1 == rl1)  /* 's1' is finished? */
        ਵਾਪਸ -1;  /* 's1' is less than 's2' ('s2' is not finished) */
      /* both strings longer than 'zl'; go on comparing after the '\0' */
      zl1++; zl2++;
      s1 += zl1; rl1 -= zl1; s2 += zl2; rl2 -= zl2;
    }
  }
}


/*
** Check whether integer 'i' is less than float 'f'. If 'i' has an
** exact representation as a float ('l_intfitsf'), compare numbers as
** floats. Otherwise, use the equivalence 'i < f <=> i < ceil(f)'.
** If 'ceil(f)' is out of integer range, either 'f' is greater than
** all integers or less than all integers.
** (The test with 'l_intfitsf' is only for performance; the else
** case is correct for all values, but it is slow due to the conversion
** from float to int.)
** When 'f' is NaN, comparisons must result in false.
*/
l_sinline ਪੂਰਨ LTintfloat (lua_Integer i, lua_ਸੰਖਿਆ f) {
  ਜੇ (l_intfitsf(i))
    ਵਾਪਸ luai_numlt(cast_ਅੰਕ(i), f);  /* compare them as floats */
  ਨਹੀਂਤਾਂ {  /* i < f <=> i < ceil(f) */
    lua_Integer fi;
    ਜੇ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      ਵਾਪਸ i < fi;   /* compare them as integers */
    ਨਹੀਂਤਾਂ  /* 'f' is either greater or less than all integers */
      ਵਾਪਸ f > 0;  /* greater? */
  }
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
l_sinline ਪੂਰਨ LEintfloat (lua_Integer i, lua_ਸੰਖਿਆ f) {
  ਜੇ (l_intfitsf(i))
    ਵਾਪਸ luai_numle(cast_ਅੰਕ(i), f);  /* compare them as floats */
  ਨਹੀਂਤਾਂ {  /* i <= f <=> i <= floor(f) */
    lua_Integer fi;
    ਜੇ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      ਵਾਪਸ i <= fi;   /* compare them as integers */
    ਨਹੀਂਤਾਂ  /* 'f' is either greater or less than all integers */
      ਵਾਪਸ f > 0;  /* greater? */
  }
}


/*
** Check whether float 'f' is less than integer 'i'.
** See comments on previous function.
*/
l_sinline ਪੂਰਨ LTfloatint (lua_ਸੰਖਿਆ f, lua_Integer i) {
  ਜੇ (l_intfitsf(i))
    ਵਾਪਸ luai_numlt(f, cast_ਅੰਕ(i));  /* compare them as floats */
  ਨਹੀਂਤਾਂ {  /* f < i <=> floor(f) < i */
    lua_Integer fi;
    ਜੇ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      ਵਾਪਸ fi < i;   /* compare them as integers */
    ਨਹੀਂਤਾਂ  /* 'f' is either greater or less than all integers */
      ਵਾਪਸ f < 0;  /* less? */
  }
}


/*
** Check whether float 'f' is less than or equal to integer 'i'.
** See comments on previous function.
*/
l_sinline ਪੂਰਨ LEfloatint (lua_ਸੰਖਿਆ f, lua_Integer i) {
  ਜੇ (l_intfitsf(i))
    ਵਾਪਸ luai_numle(f, cast_ਅੰਕ(i));  /* compare them as floats */
  ਨਹੀਂਤਾਂ {  /* f <= i <=> ceil(f) <= i */
    lua_Integer fi;
    ਜੇ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      ਵਾਪਸ fi <= i;   /* compare them as integers */
    ਨਹੀਂਤਾਂ  /* 'f' is either greater or less than all integers */
      ਵਾਪਸ f < 0;  /* less? */
  }
}


/*
** Return 'l < r', for numbers.
*/
l_sinline ਪੂਰਨ LTnum (ਅਚਲ TValue *l, ਅਚਲ TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ਜੇ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ਜੇ (ttisinteger(r))
      ਵਾਪਸ li < ivalue(r);  /* both are integers */
    ਨਹੀਂਤਾਂ  /* 'l' is int and 'r' is float */
      ਵਾਪਸ LTintfloat(li, fltvalue(r));  /* l < r ? */
  }
  ਨਹੀਂਤਾਂ {
    lua_ਸੰਖਿਆ lf = fltvalue(l);  /* 'l' must be float */
    ਜੇ (ttisfloat(r))
      ਵਾਪਸ luai_numlt(lf, fltvalue(r));  /* both are float */
    ਨਹੀਂਤਾਂ  /* 'l' is float and 'r' is int */
      ਵਾਪਸ LTfloatint(lf, ivalue(r));
  }
}


/*
** Return 'l <= r', for numbers.
*/
l_sinline ਪੂਰਨ LEnum (ਅਚਲ TValue *l, ਅਚਲ TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ਜੇ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ਜੇ (ttisinteger(r))
      ਵਾਪਸ li <= ivalue(r);  /* both are integers */
    ਨਹੀਂਤਾਂ  /* 'l' is int and 'r' is float */
      ਵਾਪਸ LEintfloat(li, fltvalue(r));  /* l <= r ? */
  }
  ਨਹੀਂਤਾਂ {
    lua_ਸੰਖਿਆ lf = fltvalue(l);  /* 'l' must be float */
    ਜੇ (ttisfloat(r))
      ਵਾਪਸ luai_numle(lf, fltvalue(r));  /* both are float */
    ਨਹੀਂਤਾਂ  /* 'l' is float and 'r' is int */
      ਵਾਪਸ LEfloatint(lf, ivalue(r));
  }
}


/*
** return 'l < r' for non-numbers.
*/
ਸਥਿਰ ਪੂਰਨ lessthanothers (lua_State *L, ਅਚਲ TValue *l, ਅਚਲ TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ਜੇ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    ਵਾਪਸ l_strcmp(tsvalue(l), tsvalue(r)) < 0;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ luaT_callorderTM(L, l, r, TM_LT);
}


/*
** Main operation less than; return 'l < r'.
*/
ਪੂਰਨ luaV_lessthan (lua_State *L, ਅਚਲ TValue *l, ਅਚਲ TValue *r) {
  ਜੇ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    ਵਾਪਸ LTnum(l, r);
  ਨਹੀਂਤਾਂ ਵਾਪਸ lessthanothers(L, l, r);
}


/*
** return 'l <= r' for non-numbers.
*/
ਸਥਿਰ ਪੂਰਨ lessequalothers (lua_State *L, ਅਚਲ TValue *l, ਅਚਲ TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ਜੇ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    ਵਾਪਸ l_strcmp(tsvalue(l), tsvalue(r)) <= 0;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ luaT_callorderTM(L, l, r, TM_LE);
}


/*
** Main operation less than or equal to; return 'l <= r'.
*/
ਪੂਰਨ luaV_lessequal (lua_State *L, ਅਚਲ TValue *l, ਅਚਲ TValue *r) {
  ਜੇ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    ਵਾਪਸ LEnum(l, r);
  ਨਹੀਂਤਾਂ ਵਾਪਸ lessequalothers(L, l, r);
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
ਪੂਰਨ luaV_equalobj (lua_State *L, ਅਚਲ TValue *t1, ਅਚਲ TValue *t2) {
  ਅਚਲ TValue *tm;
  ਜੇ (ttype(t1) != ttype(t2))  /* not the same type? */
    ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ ਜੇ (ttypetag(t1) != ttypetag(t2)) {
    ਚੋਣ (ttypetag(t1)) {
      ਹਾਲਤ LUA_VNUMINT: {  /* integer == float? */
        /* integer and float can only be equal if float has an integer
           value equal to the integer */
        lua_Integer i2;
        ਵਾਪਸ (luaV_flttointeger(fltvalue(t2), &i2, F2Ieq) &&
                ivalue(t1) == i2);
      }
      ਹਾਲਤ LUA_VNUMFLT: {  /* float == integer? */
        lua_Integer i1;  /* see comment in previous case */
        ਵਾਪਸ (luaV_flttointeger(fltvalue(t1), &i1, F2Ieq) &&
                i1 == ivalue(t2));
      }
      ਹਾਲਤ LUA_VSHRSTR: ਹਾਲਤ LUA_VLNGSTR: {
        /* compare two strings with different variants: they can be
           equal when one string is a short string and the other is
           an external string  */
        ਵਾਪਸ luaS_eqstr(tsvalue(t1), tsvalue(t2));
      }
      ਮੂਲ:
        /* only numbers (integer/float) and strings (long/short) can have
           equal values with different variants */
        ਵਾਪਸ 0;
    }
  }
  ਨਹੀਂਤਾਂ {  /* equal variants */
    ਚੋਣ (ttypetag(t1)) {
      ਹਾਲਤ LUA_VNIL: ਹਾਲਤ LUA_VFALSE: ਹਾਲਤ LUA_VTRUE:
        ਵਾਪਸ 1;
      ਹਾਲਤ LUA_VNUMINT:
        ਵਾਪਸ (ivalue(t1) == ivalue(t2));
      ਹਾਲਤ LUA_VNUMFLT:
        ਵਾਪਸ (fltvalue(t1) == fltvalue(t2));
      ਹਾਲਤ LUA_VLIGHTUSERDATA: ਵਾਪਸ pvalue(t1) == pvalue(t2);
      ਹਾਲਤ LUA_VSHRSTR:
        ਵਾਪਸ eqshrstr(tsvalue(t1), tsvalue(t2));
      ਹਾਲਤ LUA_VLNGSTR:
        ਵਾਪਸ luaS_eqstr(tsvalue(t1), tsvalue(t2));
      ਹਾਲਤ LUA_VUSERDATA: {
        ਜੇ (uvalue(t1) == uvalue(t2)) ਵਾਪਸ 1;
        ਨਹੀਂਤਾਂ ਜੇ (L == ਨਿਰਾ) ਵਾਪਸ 0;
        tm = fasttm(L, uvalue(t1)->metatable, TM_EQ);
        ਜੇ (tm == ਨਿਰਾ)
          tm = fasttm(L, uvalue(t2)->metatable, TM_EQ);
        ਰੋਕੋ;  /* will try TM */
      }
      ਹਾਲਤ LUA_VTABLE: {
        ਜੇ (hvalue(t1) == hvalue(t2)) ਵਾਪਸ 1;
        ਨਹੀਂਤਾਂ ਜੇ (L == ਨਿਰਾ) ਵਾਪਸ 0;
        tm = fasttm(L, hvalue(t1)->metatable, TM_EQ);
        ਜੇ (tm == ਨਿਰਾ)
          tm = fasttm(L, hvalue(t2)->metatable, TM_EQ);
        ਰੋਕੋ;  /* will try TM */
      }
      ਹਾਲਤ LUA_VLCF:
        ਵਾਪਸ (fvalue(t1) == fvalue(t2));
      ਮੂਲ:  /* functions and threads */
        ਵਾਪਸ (gcvalue(t1) == gcvalue(t2));
    }
    ਜੇ (tm == ਨਿਰਾ)  /* no TM? */
      ਵਾਪਸ 0;  /* objects are different */
    ਨਹੀਂਤਾਂ {
      ਪੂਰਨ ਟੈਗ = luaT_callTMres(L, tm, t1, t2, L->ਸਿਖਰ.p);  /* call TM */
      ਵਾਪਸ !tagisfalse(ਟੈਗ);
    }
  }
}


/* macro used by 'luaV_concat' to ensure that element at 'o' is a string */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tostring(L,o)  \
	(ttisstring(o) || (cvt2str(o) && (luaO_tostring(L, o), 1)))

/*
** Check whether object is a short empty string to optimize concatenation.
** (External strings can be empty too; they will be concatenated like
** non-empty ones.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isemptystr(o)	(ttisshrstring(o) && tsvalue(o)->shrlen == 0)

/* copy strings in stack from top - n up to top - 1 to buffer */
ਸਥਿਰ ਖਾਲੀ copy2buff (StkId ਸਿਖਰ, ਪੂਰਨ n, ਅੱਖਰ *buff) {
  size_t tl = 0;  /* size already copied */
  ਕਰੋ {
    TString *st = tsvalue(s2v(ਸਿਖਰ - n));
    size_t l;  /* length of string being copied */
    ਅਚਲ ਅੱਖਰ *s = getlstr(st, l);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(buff + tl, s, l * ਆਕਾਰ(ਅੱਖਰ));
    tl += l;
  } ਜਦੋਂ (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top.p - total' up to 'L->top.p - 1'.
*/
ਖਾਲੀ luaV_concat (lua_State *L, ਪੂਰਨ ਕੁੱਲ) {
  ਜੇ (ਕੁੱਲ == 1)
    ਵਾਪਸ;  /* "all" values already concatenated */
  ਕਰੋ {
    StkId ਸਿਖਰ = L->ਸਿਖਰ.p;
    ਪੂਰਨ n = 2;  /* number of elements handled in this pass (at least 2) */
    ਜੇ (!(ttisstring(s2v(ਸਿਖਰ - 2)) || cvt2str(s2v(ਸਿਖਰ - 2))) ||
        !tostring(L, s2v(ਸਿਖਰ - 1)))
      luaT_tryconcatTM(L);  /* may invalidate 'top' */
    ਨਹੀਂਤਾਂ ਜੇ (isemptystr(s2v(ਸਿਖਰ - 1)))  /* second operand is empty? */
      cast_void(tostring(L, s2v(ਸਿਖਰ - 2)));  /* result is first operand */
    ਨਹੀਂਤਾਂ ਜੇ (isemptystr(s2v(ਸਿਖਰ - 2))) {  /* first operand is empty string? */
      setobjs2s(L, ਸਿਖਰ - 2, ਸਿਖਰ - 1);  /* result is second op. */
    }
    ਨਹੀਂਤਾਂ {
      /* at least two string values; get as many as possible */
      size_t tl = tsslen(tsvalue(s2v(ਸਿਖਰ - 1)));  /* total length */
      TString *ts;
      /* collect total length and number of strings */
      ਲਈ (n = 1; n < ਕੁੱਲ && tostring(L, s2v(ਸਿਖਰ - n - 1)); n++) {
        size_t l = tsslen(tsvalue(s2v(ਸਿਖਰ - n - 1)));
        ਜੇ (l_unlikely(l >= MAX_ਮਾਪ - ਆਕਾਰ(TString) - tl)) {
          L->ਸਿਖਰ.p = ਸਿਖਰ - ਕੁੱਲ;  /* pop strings to avoid wasting stack */
          luaG_runerror(L, "string length overflow");
        }
        tl += l;
      }
      ਜੇ (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        ਅੱਖਰ buff[LUAI_MAXSHORTLEN];
        copy2buff(ਸਿਖਰ, n, buff);  /* copy strings to buffer */
        ts = luaS_newlstr(L, buff, tl);
      }
      ਨਹੀਂਤਾਂ {  /* long string; copy strings directly to final result */
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(ਸਿਖਰ, n, getlngstr(ts));
      }
      setsvalue2s(L, ਸਿਖਰ - n, ts);  /* create result */
    }
    ਕੁੱਲ -= n - 1;  /* got 'n' strings to create one new */
    L->ਸਿਖਰ.p -= n - 1;  /* popped 'n' strings and pushed one */
  } ਜਦੋਂ (ਕੁੱਲ > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra = #rb'.
*/
ਖਾਲੀ luaV_objlen (lua_State *L, StkId ra, ਅਚਲ TValue *rb) {
  ਅਚਲ TValue *tm;
  ਚੋਣ (ttypetag(rb)) {
    ਹਾਲਤ LUA_VTABLE: {
      ਸਾਰਣੀ *h = hvalue(rb);
      tm = fasttm(L, h->metatable, TM_LEN);
      ਜੇ (tm) ਰੋਕੋ;  /* metamethod? break switch to call it */
      setivalue(s2v(ra), l_castU2S(luaH_getn(L, h)));  /* else primitive len */
      ਵਾਪਸ;
    }
    ਹਾਲਤ LUA_VSHRSTR: {
      setivalue(s2v(ra), tsvalue(rb)->shrlen);
      ਵਾਪਸ;
    }
    ਹਾਲਤ LUA_VLNGSTR: {
      setivalue(s2v(ra), cast_st2S(tsvalue(rb)->u.lnglen));
      ਵਾਪਸ;
    }
    ਮੂਲ: {  /* try metamethod */
      tm = luaT_gettmbyobj(L, rb, TM_LEN);
      ਜੇ (l_unlikely(notm(tm)))  /* no metamethod? */
        luaG_typeerror(L, rb, "get length of");
      ਰੋਕੋ;
    }
  }
  luaT_callTMres(L, tm, rb, rb, ra);
}


/*
** Integer division; return 'm // n', that is, floor(m/n).
** C division truncates its result (rounds towards zero).
** 'floor(q) == trunc(q)' when 'q >= 0' or when 'q' is integer,
** otherwise 'floor(q) == trunc(q) - 1'.
*/
lua_Integer luaV_idiv (lua_State *L, lua_Integer m, lua_Integer n) {
  ਜੇ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ਜੇ (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    ਵਾਪਸ intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  ਨਹੀਂਤਾਂ {
    lua_Integer q = m / n;  /* perform C division */
    ਜੇ ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    ਵਾਪਸ q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_idiv.)
*/
lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  ਜੇ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ਜੇ (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    ਵਾਪਸ 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  ਨਹੀਂਤਾਂ {
    lua_Integer r = m % n;
    ਜੇ (r != 0 && (r ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    ਵਾਪਸ r;
  }
}


/*
** Float modulus
*/
lua_ਸੰਖਿਆ luaV_modf (lua_State *L, lua_ਸੰਖਿਆ m, lua_ਸੰਖਿਆ n) {
  lua_ਸੰਖਿਆ r;
  luai_nummod(L, m, n, r);
  ਵਾਪਸ r;
}


/* number of bits in an integer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NBITS	l_numbits(lua_Integer)


/*
** Shift left operation. (Shift right just negates 'y'.)
*/
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  ਜੇ (y < 0) {  /* shift right? */
    ਜੇ (y <= -NBITS) ਵਾਪਸ 0;
    ਨਹੀਂਤਾਂ ਵਾਪਸ intop(>>, x, -y);
  }
  ਨਹੀਂਤਾਂ {  /* shift left */
    ਜੇ (y >= NBITS) ਵਾਪਸ 0;
    ਨਹੀਂਤਾਂ ਵਾਪਸ intop(<<, x, y);
  }
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues.
*/
ਸਥਿਰ ਖਾਲੀ pushclosure (lua_State *L, Proto *p, ਉੱਪਰ_Val **encup, StkId ਅਧਾਰ,
                         StkId ra) {
  ਪੂਰਨ nup = p->sizeupvalues;
  Upvaldesc *uv = p->upvalues;
  ਪੂਰਨ i;
  LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  setclLvalue2s(L, ra, ncl);  /* anchor new closure in stack */
  ਲਈ (i = 0; i < nup; i++) {  /* fill in its upvalues */
    ਜੇ (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = luaF_findupval(L, ਅਧਾਰ + uv[i].idx);
    ਨਹੀਂਤਾਂ  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    luaC_objbarrier(L, ncl, ncl->upvals[i]);
  }
}


/*
** finish execution of an opcode interrupted by a yield
*/
ਖਾਲੀ lua_V_ਅੰਤ_Op (lua_State *L) {
  CallInfo *ci = L->ci;
  StkId ਅਧਾਰ = ci->func.p + 1;
  Instruction inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  Op_ਕੋਡ op = GET_OPCODE(inst);
  ਚੋਣ (op) {  /* finish its execution */
    ਹਾਲਤ OP_MMBIN: ਹਾਲਤ OP_MMBINI: ਹਾਲਤ OP_MMBINK: {
      setobjs2s(L, ਅਧਾਰ + GETARG_A(*(ci->u.l.savedpc - 2)), --L->ਸਿਖਰ.p);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OP_UNM: ਹਾਲਤ OP_BNOT: ਹਾਲਤ OP_LEN:
    ਹਾਲਤ OP_GETTABUP: ਹਾਲਤ OP_GETTABLE: ਹਾਲਤ OP_GETI:
    ਹਾਲਤ OP_GETFIELD: ਹਾਲਤ OP_SELF: {
      setobjs2s(L, ਅਧਾਰ + GETARG_A(inst), --L->ਸਿਖਰ.p);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OP_LT: ਹਾਲਤ OP_LE:
    ਹਾਲਤ OP_LTI: ਹਾਲਤ OP_LEI:
    ਹਾਲਤ OP_GTI: ਹਾਲਤ OP_GEI:
    ਹਾਲਤ OP_EQ: {  /* note that 'OP_EQI'/'OP_EQK' cannot yield */
      ਪੂਰਨ res = !l_isfalse(s2v(L->ਸਿਖਰ.p - 1));
      L->ਸਿਖਰ.p--;
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_JMP);
      ਜੇ (res != GETARG_k(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      ਰੋਕੋ;
    }
    ਹਾਲਤ OP_CONCAT: {
      StkId ਸਿਖਰ = L->ਸਿਖਰ.p - 1;  /* top when 'luaT_tryconcatTM' was called */
      ਪੂਰਨ a = GETARG_A(inst);      /* first element to concatenate */
      ਪੂਰਨ ਕੁੱਲ = cast_int(ਸਿਖਰ - 1 - (ਅਧਾਰ + a));  /* yet to concatenate */
      setobjs2s(L, ਸਿਖਰ - 2, ਸਿਖਰ);  /* put TM result in proper position */
      L->ਸਿਖਰ.p = ਸਿਖਰ - 1;  /* top is one after last element (at top-2) */
      luaV_concat(L, ਕੁੱਲ);  /* concat them (may yield again) */
      ਰੋਕੋ;
    }
    ਹਾਲਤ OP_CLOSE: {  /* yielded closing variables */
      ci->u.l.savedpc--;  /* repeat instruction to close other vars. */
      ਰੋਕੋ;
    }
    ਹਾਲਤ OP_RETURN: {  /* yielded closing variables */
      StkId ra = ਅਧਾਰ + GETARG_A(inst);
      /* adjust top to signal correct number of returns, in case the
         return is "up to top" ('isIT') */
      L->ਸਿਖਰ.p = ra + ci->u2.nres;
      /* repeat instruction to close other vars. and complete the return */
      ci->u.l.savedpc--;
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      /* only these other opcodes can yield */
      lua_assert(op == OP_TFORCALL || op == OP_CALL ||
           op == OP_TAILCALL || op == OP_SETTABUP || op == OP_SETTABLE ||
           op == OP_SETI || op == OP_SETFIELD);
      ਰੋਕੋ;
    }
  }
}




/*
** {==================================================================
** Macros for arithmetic/bitwise/comparison opcodes in 'luaV_execute'
**
** All these macros are to be used exclusively inside the main
** interpreter loop (function luaV_execute) and may access directly
** the local variables of that function (L, i, pc, ci, etc.).
** ===================================================================
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_addi(L,a,b)	intop(+, a, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_subi(L,a,b)	intop(-, a, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_muli(L,a,b)	intop(*, a, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_band(a,b)	intop(&, a, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_bor(a,b)	intop(|, a, b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_bxor(a,b)	intop(^, a, b)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_lti(a,b)	(a < b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_lei(a,b)	(a <= b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_gti(a,b)	(a > b)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_gei(a,b)	(a >= b)


/*
** Arithmetic operations with immediate operands. 'iop' is the integer
** operation, 'fop' is the float operation.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_arithI(L,iop,fop) {  \
  TValue *ra = vRA(i); \
  TValue *v1 = vRB(i);  \
  ਪੂਰਨ imm = GETARG_sC(i);  \
  ਜੇ (ttisinteger(v1)) {  \
    lua_Integer iv1 = ivalue(v1);  \
    pc++; setivalue(ra, iop(L, iv1, imm));  \
  }  \
  ਨਹੀਂਤਾਂ ਜੇ (ttisfloat(v1)) {  \
    lua_ਸੰਖਿਆ nb = fltvalue(v1);  \
    lua_ਸੰਖਿਆ fimm = cast_ਅੰਕ(imm);  \
    pc++; setfltvalue(ra, fop(L, nb, fimm)); \
  }}


/*
** Auxiliary function for arithmetic operations over floats and others
** with two operands.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_arithf_aux(L,v1,v2,fop) {  \
  lua_ਸੰਖਿਆ n1; lua_ਸੰਖਿਆ n2;  \
  ਜੇ (tonumberns(v1, n1) && tonumberns(v2, n2)) {  \
    StkId ra = RA(i);  \
    pc++; setfltvalue(s2v(ra), fop(L, n1, n2));  \
  }}


/*
** Arithmetic operations over floats and others with register operands.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_arithf(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with K operands for floats.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_arithfK(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations over integers and floats.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_arith_aux(L,v1,v2,iop,fop) {  \
  ਜੇ (ttisinteger(v1) && ttisinteger(v2)) {  \
    StkId ra = RA(i); \
    lua_Integer i1 = ivalue(v1); lua_Integer i2 = ivalue(v2);  \
    pc++; setivalue(s2v(ra), iop(L, i1, i2));  \
  }  \
  ਨਹੀਂਤਾਂ op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with register operands.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_arith(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Arithmetic operations with K operands.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_arithK(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Bitwise operations with constant operand.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_bitwiseK(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i);  \
  lua_Integer i1;  \
  lua_Integer i2 = ivalue(v2);  \
  ਜੇ (tointegerns(v1, &i1)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Bitwise operations with register operands.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_bitwise(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  lua_Integer i1; lua_Integer i2;  \
  ਜੇ (tointegerns(v1, &i1) && tointegerns(v2, &i2)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Order operations with register operands. 'opn' actually works
** for all numbers, but the fast track improves performance for
** integers.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_ਕ੍ਰਮ(L,opi,opn,other) {  \
  TValue *ra = vRA(i); \
  ਪੂਰਨ cond;  \
  TValue *rb = vRB(i);  \
  ਜੇ (ttisinteger(ra) && ttisinteger(rb)) {  \
    lua_Integer ia = ivalue(ra);  \
    lua_Integer ib = ivalue(rb);  \
    cond = opi(ia, ib);  \
  }  \
  ਨਹੀਂਤਾਂ ਜੇ (ttisnumber(ra) && ttisnumber(rb))  \
    cond = opn(ra, rb);  \
  ਨਹੀਂਤਾਂ  \
    Protect(cond = other(L, ra, rb));  \
  docondjump(); }


/*
** Order operations with immediate operand. (Immediate operand is
** always small enough to have an exact representation as a float.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ op_ਕ੍ਰਮ_I(L,opi,opf,inv,tm) {  \
  TValue *ra = vRA(i); \
  ਪੂਰਨ cond;  \
  ਪੂਰਨ im = GETARG_sB(i);  \
  ਜੇ (ttisinteger(ra))  \
    cond = opi(ivalue(ra), im);  \
  ਨਹੀਂਤਾਂ ਜੇ (ttisfloat(ra)) {  \
    lua_ਸੰਖਿਆ fa = fltvalue(ra);  \
    lua_ਸੰਖਿਆ fim = cast_ਅੰਕ(im);  \
    cond = opf(fa, fim);  \
  }  \
  ਨਹੀਂਤਾਂ {  \
    ਪੂਰਨ isf = GETARG_C(i);  \
    Protect(cond = luaT_callorderiTM(L, ra, im, inv, isf, tm));  \
  }  \
  docondjump(); }

/* }================================================================== */


/*
** {==================================================================
** Function 'luaV_execute': main interpreter loop
** ===================================================================
*/

/*
** some macros for common tasks in 'luaV_execute'
*/


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RA(i)	(ਅਧਾਰ+GETARG_A(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vRA(i)	s2v(RA(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RB(i)	(ਅਧਾਰ+GETARG_B(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vRB(i)	s2v(RB(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KB(i)	(k+GETARG_B(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC(i)	(ਅਧਾਰ+GETARG_C(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vRC(i)	s2v(RC(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KC(i)	(k+GETARG_C(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RKC(i)	((TESTARG_k(i)) ? k + GETARG_C(i) : s2v(ਅਧਾਰ + GETARG_C(i)))



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ updatetrap(ci)  (trap = cast_int(ci->u.l.trap))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ updatebase(ci)	(ਅਧਾਰ = ci->func.p + 1)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ updatestack(ci)  \
	{ ਜੇ (l_unlikely(trap)) { updatebase(ci); ra = RA(i); } }


/*
** Execute a jump instruction. The 'updatetrap' allows signals to stop
** tight loops. (Without it, the local copy of 'trap' could never change.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dojump(ci,i,e)	{ pc += GETARG_sJ(i) + e; updatetrap(ci); }


/* for test instructions, execute the jump instruction that follows it */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ donextjump(ci)	{ Instruction ni = *pc; dojump(ci, ni, 1); }

/*
** do a conditional jump: skip next instruction if 'cond' is not what
** was expected (parameter 'k'), else do next instruction, which must
** be a jump.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ docondjump()	ਜੇ (cond != GETARG_k(i)) pc++; ਨਹੀਂਤਾਂ donextjump(ci);


/*
** Correct global 'pc'.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ savepc(ci)	(ci->u.l.savedpc = pc)


/*
** Whenever code can raise errors, the global 'pc' and the global
** 'top' must be correct to report occasional errors.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ savestate(L,ci)		(savepc(ci), L->ਸਿਖਰ.p = ci->ਸਿਖਰ.p)


/*
** Protect code that, in general, can raise errors, reallocate the
** stack, and change the hooks.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Protect(ਘਾਤਾਂਕ)  (savestate(L,ci), (ਘਾਤਾਂਕ), updatetrap(ci))

/* special version that does not change the top */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ProtectNT(ਘਾਤਾਂਕ)  (savepc(ci), (ਘਾਤਾਂਕ), updatetrap(ci))

/*
** Protect code that can only raise errors. (That is, it cannot change
** the stack or hooks.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ halfProtect(ਘਾਤਾਂਕ)  (savestate(L,ci), (ਘਾਤਾਂਕ))

/*
** macro executed during Lua functions at points where the
** function can yield.
*/
#ਜੇ !defined(luai_threadyield)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_threadyield(L)	{lua_unlock(L); lua_ਤਾਲਾ(L);}
#ਜੇ_ਅੰਤ

/* 'c' is the limit of live values in the stack */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checkGC(L,c)  \
	{ luaC_condGC(L, (savepc(ci), L->ਸਿਖਰ.p = (c)), \
                         updatetrap(ci)); \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vmfetch()	{ \
  ਜੇ (l_unlikely(trap)) {  /* stack reallocation or hooks? */ \
    trap = luaG_traceexec(L, pc);  /* handle hooks */ \
    updatebase(ci);  /* correct stack */ \
  } \
  i = *(pc++); \
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vmdispatch(o)	ਚੋਣ(o)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vmcase(l)	ਹਾਲਤ l:
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vmbreak		ਰੋਕੋ


ਖਾਲੀ luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId ਅਧਾਰ;
  ਅਚਲ Instruction *pc;
  ਪੂਰਨ trap;
#ਜੇ LUA_USE_JUMPTABLE
#ਅਨ੍ਤਰ੍ਭਾਵ "ljumptab.h"
#ਜੇ_ਅੰਤ
 startfunc:
  trap = cast_int(L->hookmask);
 returning:  /* trap already set */
  cl = ci_func(ci);
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  ਜੇ (l_unlikely(trap))
    trap = luaG_tracecall(L);
  ਅਧਾਰ = ci->func.p + 1;
  /* main loop of interpreter */
  ਲਈ (;;) {
    Instruction i;  /* instruction being executed */
    vmfetch();
    #ਜੇ 0
    { /* low-level line tracing for debugging Lua */
      #ਅਨ੍ਤਰ੍ਭਾਵ "lopnames.h"
      ਪੂਰਨ pcrel = pcRel(pc, cl->p);
      ਲਿਖੋ("line: %d; %s (%d)\n", luaG_getfuncline(cl->p, pcrel),
             opnames[GET_OPCODE(i)], pcrel);
    }
    #ਜੇ_ਅੰਤ
    lua_assert(ਅਧਾਰ == ci->func.p + 1);
    lua_assert(ਅਧਾਰ <= L->ਸਿਖਰ.p && L->ਸਿਖਰ.p <= L->ਢੇਰ_ਆਖਰੀ.p);
    /* for tests, invalidate top for instructions not expecting it */
    lua_assert(luaP_isIT(i) || (cast_void(L->ਸਿਖਰ.p = ਅਧਾਰ), 1));
    vmdispatch (GET_OPCODE(i)) {
      vmcase(OP_MOVE) {
        StkId ra = RA(i);
        setobjs2s(L, ra, RB(i));
        vmbreak;
      }
      vmcase(OP_LOADI) {
        StkId ra = RA(i);
        lua_Integer b = GETARG_sBx(i);
        setivalue(s2v(ra), b);
        vmbreak;
      }
      vmcase(OP_LOADF) {
        StkId ra = RA(i);
        ਪੂਰਨ b = GETARG_sBx(i);
        setfltvalue(s2v(ra), cast_ਅੰਕ(b));
        vmbreak;
      }
      vmcase(OP_LOADK) {
        StkId ra = RA(i);
        TValue *rb = k + GETARG_Bx(i);
        setobj2s(L, ra, rb);
        vmbreak;
      }
      vmcase(OP_LOADKX) {
        StkId ra = RA(i);
        TValue *rb;
        rb = k + GETARG_Ax(*pc); pc++;
        setobj2s(L, ra, rb);
        vmbreak;
      }
      vmcase(OP_LOADFALSE) {
        StkId ra = RA(i);
        setbfvalue(s2v(ra));
        vmbreak;
      }
      vmcase(OP_LFALSESKIP) {
        StkId ra = RA(i);
        setbfvalue(s2v(ra));
        pc++;  /* skip next instruction */
        vmbreak;
      }
      vmcase(OP_LOADTRUE) {
        StkId ra = RA(i);
        setbtvalue(s2v(ra));
        vmbreak;
      }
      vmcase(OP_LOADNIL) {
        StkId ra = RA(i);
        ਪੂਰਨ b = GETARG_B(i);
        ਕਰੋ {
          setnilvalue(s2v(ra++));
        } ਜਦੋਂ (b--);
        vmbreak;
      }
      vmcase(OP_GETUPVAL) {
        StkId ra = RA(i);
        ਪੂਰਨ b = GETARG_B(i);
        setobj2s(L, ra, cl->upvals[b]->v.p);
        vmbreak;
      }
      vmcase(OP_SETUPVAL) {
        StkId ra = RA(i);
        ਉੱਪਰ_Val *uv = cl->upvals[GETARG_B(i)];
        setobj(L, uv->v.p, s2v(ra));
        luaC_barrier(L, uv, s2v(ra));
        vmbreak;
      }
      vmcase(OP_GETTABUP) {
        StkId ra = RA(i);
        TValue *upval = cl->upvals[GETARG_B(i)]->v.p;
        TValue *rc = KC(i);
        TString *ਕੁੰਜੀ = tsvalue(rc);  /* key must be a short string */
        lu_byte ਟੈਗ;
        luaV_fastget(upval, ਕੁੰਜੀ, s2v(ra), luaH_getshortstr, ਟੈਗ);
        ਜੇ (tagisempty(ਟੈਗ))
          Protect(luaV_finishget(L, upval, rc, ra, ਟੈਗ));
        vmbreak;
      }
      vmcase(OP_GETTABLE) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = vRC(i);
        lu_byte ਟੈਗ;
        ਜੇ (ttisinteger(rc)) {  /* fast track for integers? */
          luaV_fastgeti(rb, ivalue(rc), s2v(ra), ਟੈਗ);
        }
        ਨਹੀਂਤਾਂ
          luaV_fastget(rb, rc, s2v(ra), luaH_get, ਟੈਗ);
        ਜੇ (tagisempty(ਟੈਗ))
          Protect(luaV_finishget(L, rb, rc, ra, ਟੈਗ));
        vmbreak;
      }
      vmcase(OP_GETI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ਪੂਰਨ c = GETARG_C(i);
        lu_byte ਟੈਗ;
        luaV_fastgeti(rb, c, s2v(ra), ਟੈਗ);
        ਜੇ (tagisempty(ਟੈਗ)) {
          TValue ਕੁੰਜੀ;
          setivalue(&ਕੁੰਜੀ, c);
          Protect(luaV_finishget(L, rb, &ਕੁੰਜੀ, ra, ਟੈਗ));
        }
        vmbreak;
      }
      vmcase(OP_GETFIELD) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *ਕੁੰਜੀ = tsvalue(rc);  /* key must be a short string */
        lu_byte ਟੈਗ;
        luaV_fastget(rb, ਕੁੰਜੀ, s2v(ra), luaH_getshortstr, ਟੈਗ);
        ਜੇ (tagisempty(ਟੈਗ))
          Protect(luaV_finishget(L, rb, rc, ra, ਟੈਗ));
        vmbreak;
      }
      vmcase(OP_SETTABUP) {
        ਪੂਰਨ hres;
        TValue *upval = cl->upvals[GETARG_A(i)]->v.p;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *ਕੁੰਜੀ = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(upval, ਕੁੰਜੀ, rc, hres, luaH_psetshortstr);
        ਜੇ (hres == HOK)
          luaV_finishfastset(L, upval, rc);
        ਨਹੀਂਤਾਂ
          Protect(luaV_finishset(L, upval, rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETTABLE) {
        StkId ra = RA(i);
        ਪੂਰਨ hres;
        TValue *rb = vRB(i);  /* key (table is in 'ra') */
        TValue *rc = RKC(i);  /*ਮੁੱਲ*/
        ਜੇ (ttisinteger(rb)) {  /* fast track for integers? */
          luaV_fastseti(s2v(ra), ivalue(rb), rc, hres);
        }
        ਨਹੀਂਤਾਂ {
          luaV_fastset(s2v(ra), rb, rc, hres, luaH_pset);
        }
        ਜੇ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ਨਹੀਂਤਾਂ
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETI) {
        StkId ra = RA(i);
        ਪੂਰਨ hres;
        ਪੂਰਨ b = GETARG_B(i);
        TValue *rc = RKC(i);
        luaV_fastseti(s2v(ra), b, rc, hres);
        ਜੇ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ਨਹੀਂਤਾਂ {
          TValue ਕੁੰਜੀ;
          setivalue(&ਕੁੰਜੀ, b);
          Protect(luaV_finishset(L, s2v(ra), &ਕੁੰਜੀ, rc, hres));
        }
        vmbreak;
      }
      vmcase(OP_SETFIELD) {
        StkId ra = RA(i);
        ਪੂਰਨ hres;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *ਕੁੰਜੀ = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(s2v(ra), ਕੁੰਜੀ, rc, hres, luaH_psetshortstr);
        ਜੇ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ਨਹੀਂਤਾਂ
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_NEWTABLE) {
        StkId ra = RA(i);
        ਬਿਨਾਨਿਸ਼ਾਨੀ b = cast_uint(GETARG_vB(i));  /* log2(hash size) + 1 */
        ਬਿਨਾਨਿਸ਼ਾਨੀ c = cast_uint(GETARG_vC(i));  /* array size */
        ਸਾਰਣੀ *t;
        ਜੇ (b > 0)
          b = 1u << (b - 1);  /* hash size is 2^(b - 1) */
        ਜੇ (TESTARG_k(i)) {  /* non-zero extra argument? */
          lua_assert(GETARG_Ax(*pc) != 0);
          /* add it to array size */
          c += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
        }
        pc++;  /* skip extra argument */
        L->ਸਿਖਰ.p = ra + 1;  /* correct top in case of emergency GC */
        t = luaH_new(L);  /* memory allocation */
        sethvalue2s(L, ra, t);
        ਜੇ (b != 0 || c != 0)
          luaH_resize(L, t, c, b);  /* idem */
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_SELF) {
        StkId ra = RA(i);
        lu_byte ਟੈਗ;
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *ਕੁੰਜੀ = tsvalue(rc);  /* key must be a short string */
        setobj2s(L, ra + 1, rb);
        luaV_fastget(rb, ਕੁੰਜੀ, s2v(ra), luaH_getshortstr, ਟੈਗ);
        ਜੇ (tagisempty(ਟੈਗ))
          Protect(luaV_finishget(L, rb, rc, ra, ਟੈਗ));
        vmbreak;
      }
      vmcase(OP_ADDI) {
        op_arithI(L, l_addi, luai_numadd);
        vmbreak;
      }
      vmcase(OP_ADDK) {
        op_arithK(L, l_addi, luai_numadd);
        vmbreak;
      }
      vmcase(OP_SUBK) {
        op_arithK(L, l_subi, luai_numsub);
        vmbreak;
      }
      vmcase(OP_MULK) {
        op_arithK(L, l_muli, luai_nummul);
        vmbreak;
      }
      vmcase(OP_MODK) {
        savestate(L, ci);  /* in case of division by 0 */
        op_arithK(L, luaV_mod, luaV_modf);
        vmbreak;
      }
      vmcase(OP_POWK) {
        op_arithfK(L, luai_numpow);
        vmbreak;
      }
      vmcase(OP_DIVK) {
        op_arithfK(L, luai_numdiv);
        vmbreak;
      }
      vmcase(OP_IDIVK) {
        savestate(L, ci);  /* in case of division by 0 */
        op_arithK(L, luaV_idiv, luai_numidiv);
        vmbreak;
      }
      vmcase(OP_BANDK) {
        op_bitwiseK(L, l_band);
        vmbreak;
      }
      vmcase(OP_BORK) {
        op_bitwiseK(L, l_bor);
        vmbreak;
      }
      vmcase(OP_BXORK) {
        op_bitwiseK(L, l_bxor);
        vmbreak;
      }
      vmcase(OP_SHLI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ਪੂਰਨ ic = GETARG_sC(i);
        lua_Integer ib;
        ਜੇ (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ic, ib));
        }
        vmbreak;
      }
      vmcase(OP_SHRI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ਪੂਰਨ ic = GETARG_sC(i);
        lua_Integer ib;
        ਜੇ (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ib, -ic));
        }
        vmbreak;
      }
      vmcase(OP_ਜੋੜੋ) {
        op_arith(L, l_addi, luai_numadd);
        vmbreak;
      }
      vmcase(OP_SUB) {
        op_arith(L, l_subi, luai_numsub);
        vmbreak;
      }
      vmcase(OP_MUL) {
        op_arith(L, l_muli, luai_nummul);
        vmbreak;
      }
      vmcase(OP_MOD) {
        savestate(L, ci);  /* in case of division by 0 */
        op_arith(L, luaV_mod, luaV_modf);
        vmbreak;
      }
      vmcase(OP_POW) {
        op_arithf(L, luai_numpow);
        vmbreak;
      }
      vmcase(OP_DIV) {  /* float division (always with floats) */
        op_arithf(L, luai_numdiv);
        vmbreak;
      }
      vmcase(OP_IDIV) {  /* floor division */
        savestate(L, ci);  /* in case of division by 0 */
        op_arith(L, luaV_idiv, luai_numidiv);
        vmbreak;
      }
      vmcase(OP_BAND) {
        op_bitwise(L, l_band);
        vmbreak;
      }
      vmcase(OP_BOR) {
        op_bitwise(L, l_bor);
        vmbreak;
      }
      vmcase(OP_BXOR) {
        op_bitwise(L, l_bxor);
        vmbreak;
      }
      vmcase(OP_SHL) {
        op_bitwise(L, luaV_shiftl);
        vmbreak;
      }
      vmcase(OP_SHR) {
        op_bitwise(L, luaV_shiftr);
        vmbreak;
      }
      vmcase(OP_MMBIN) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        TValue *rb = vRB(i);
        TMS tm = (TMS)GETARG_C(i);
        StkId ਨਤੀਜਾ = RA(pi);
        lua_assert(OP_ਜੋੜੋ <= GET_OPCODE(pi) && GET_OPCODE(pi) <= OP_SHR);
        Protect(luaT_trybinTM(L, s2v(ra), rb, ਨਤੀਜਾ, tm));
        vmbreak;
      }
      vmcase(OP_MMBINI) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        ਪੂਰਨ imm = GETARG_sB(i);
        TMS tm = (TMS)GETARG_C(i);
        ਪੂਰਨ flip = GETARG_k(i);
        StkId ਨਤੀਜਾ = RA(pi);
        Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, ਨਤੀਜਾ, tm));
        vmbreak;
      }
      vmcase(OP_MMBINK) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        TValue *imm = KB(i);
        TMS tm = (TMS)GETARG_C(i);
        ਪੂਰਨ flip = GETARG_k(i);
        StkId ਨਤੀਜਾ = RA(pi);
        Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, ਨਤੀਜਾ, tm));
        vmbreak;
      }
      vmcase(OP_UNM) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_ਸੰਖਿਆ nb;
        ਜੇ (ttisinteger(rb)) {
          lua_Integer ib = ivalue(rb);
          setivalue(s2v(ra), intop(-, 0, ib));
        }
        ਨਹੀਂਤਾਂ ਜੇ (tonumberns(rb, nb)) {
          setfltvalue(s2v(ra), luai_numunm(L, nb));
        }
        ਨਹੀਂਤਾਂ
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
        vmbreak;
      }
      vmcase(OP_BNOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_Integer ib;
        ਜੇ (tointegerns(rb, &ib)) {
          setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
        }
        ਨਹੀਂਤਾਂ
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
        vmbreak;
      }
      vmcase(OP_NOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ਜੇ (l_isfalse(rb))
          setbtvalue(s2v(ra));
        ਨਹੀਂਤਾਂ
          setbfvalue(s2v(ra));
        vmbreak;
      }
      vmcase(OP_LEN) {
        StkId ra = RA(i);
        Protect(luaV_objlen(L, ra, vRB(i)));
        vmbreak;
      }
      vmcase(OP_CONCAT) {
        StkId ra = RA(i);
        ਪੂਰਨ n = GETARG_B(i);  /* number of elements to concatenate */
        L->ਸਿਖਰ.p = ra + n;  /* mark the end of concat operands */
        ProtectNT(luaV_concat(L, n));
        checkGC(L, L->ਸਿਖਰ.p); /* 'luaV_concat' ensures correct top */
        vmbreak;
      }
      vmcase(OP_CLOSE) {
        StkId ra = RA(i);
        lua_assert(!GETARG_B(i));  /* 'close must be alive */
        Protect(luaF_close(L, ra, LUA_OK, 1));
        vmbreak;
      }
      vmcase(OP_TBC) {
        StkId ra = RA(i);
        /* create new to-be-closed upvalue */
        halfProtect(luaF_newtbcupval(L, ra));
        vmbreak;
      }
      vmcase(OP_JMP) {
        dojump(ci, i, 0);
        vmbreak;
      }
      vmcase(OP_EQ) {
        StkId ra = RA(i);
        ਪੂਰਨ cond;
        TValue *rb = vRB(i);
        Protect(cond = luaV_equalobj(L, s2v(ra), rb));
        docondjump();
        vmbreak;
      }
      vmcase(OP_LT) {
        op_ਕ੍ਰਮ(L, l_lti, LTnum, lessthanothers);
        vmbreak;
      }
      vmcase(OP_LE) {
        op_ਕ੍ਰਮ(L, l_lei, LEnum, lessequalothers);
        vmbreak;
      }
      vmcase(OP_EQK) {
        StkId ra = RA(i);
        TValue *rb = KB(i);
        /* basic types do not use '__eq'; we can use raw equality */
        ਪੂਰਨ cond = luaV_rawequalobj(s2v(ra), rb);
        docondjump();
        vmbreak;
      }
      vmcase(OP_EQI) {
        StkId ra = RA(i);
        ਪੂਰਨ cond;
        ਪੂਰਨ im = GETARG_sB(i);
        ਜੇ (ttisinteger(s2v(ra)))
          cond = (ivalue(s2v(ra)) == im);
        ਨਹੀਂਤਾਂ ਜੇ (ttisfloat(s2v(ra)))
          cond = luai_numeq(fltvalue(s2v(ra)), cast_ਅੰਕ(im));
        ਨਹੀਂਤਾਂ
          cond = 0;  /* other types cannot be equal to a number */
        docondjump();
        vmbreak;
      }
      vmcase(OP_LTI) {
        op_ਕ੍ਰਮ_I(L, l_lti, luai_numlt, 0, TM_LT);
        vmbreak;
      }
      vmcase(OP_LEI) {
        op_ਕ੍ਰਮ_I(L, l_lei, luai_numle, 0, TM_LE);
        vmbreak;
      }
      vmcase(OP_GTI) {
        op_ਕ੍ਰਮ_I(L, l_gti, luai_numgt, 1, TM_LT);
        vmbreak;
      }
      vmcase(OP_GEI) {
        op_ਕ੍ਰਮ_I(L, l_gei, luai_numge, 1, TM_LE);
        vmbreak;
      }
      vmcase(OP_TEST) {
        StkId ra = RA(i);
        ਪੂਰਨ cond = !l_isfalse(s2v(ra));
        docondjump();
        vmbreak;
      }
      vmcase(OP_TESTSET) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ਜੇ (l_isfalse(rb) == GETARG_k(i))
          pc++;
        ਨਹੀਂਤਾਂ {
          setobj2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(OP_CALL) {
        StkId ra = RA(i);
        CallInfo *newci;
        ਪੂਰਨ b = GETARG_B(i);
        ਪੂਰਨ nresults = GETARG_C(i) - 1;
        ਜੇ (b != 0)  /* fixed number of arguments? */
          L->ਸਿਖਰ.p = ra + b;  /* top signals number of arguments */
        /* else previous instruction set top */
        savepc(ci);  /* in case of errors */
        ਜੇ ((newci = luaD_precall(L, ra, nresults)) == ਨਿਰਾ)
          updatetrap(ci);  /* C call; nothing else to be done */
        ਨਹੀਂਤਾਂ {  /* Lua call: run function in this same C frame */
          ci = newci;
          ਜਾਓ startfunc;
        }
        vmbreak;
      }
      vmcase(OP_TAILCALL) {
        StkId ra = RA(i);
        ਪੂਰਨ b = GETARG_B(i);  /* number of arguments + 1 (function) */
        ਪੂਰਨ n;  /* number of results when calling a C function */
        ਪੂਰਨ nparams1 = GETARG_C(i);
        /* delta is virtual 'func' - real 'func' (vararg functions) */
        ਪੂਰਨ delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        ਜੇ (b != 0)
          L->ਸਿਖਰ.p = ra + b;
        ਨਹੀਂਤਾਂ  /* previous instruction set top */
          b = cast_int(L->ਸਿਖਰ.p - ra);
        savepc(ci);  /* several calls here can raise errors */
        ਜੇ (TESTARG_k(i)) {
          luaF_closeupval(L, ਅਧਾਰ);  /* close upvalues from current call */
          lua_assert(L->tbclist.p < ਅਧਾਰ);  /* no pending tbc variables */
          lua_assert(ਅਧਾਰ == ci->func.p + 1);
        }
        ਜੇ ((n = luaD_pretailcall(L, ci, ra, b, delta)) < 0)  /* Lua function? */
          ਜਾਓ startfunc;  /* execute the callee */
        ਨਹੀਂਤਾਂ {  /* C function? */
          ci->func.p -= delta;  /* restore 'func' (if vararg) */
          luaD_poscall(L, ci, n);  /* finish caller */
          updatetrap(ci);  /* 'luaD_poscall' can change hooks */
          ਜਾਓ ret;  /* caller returns after the tail call */
        }
      }
      vmcase(OP_RETURN) {
        StkId ra = RA(i);
        ਪੂਰਨ n = GETARG_B(i) - 1;  /* number of results */
        ਪੂਰਨ nparams1 = GETARG_C(i);
        ਜੇ (n < 0)  /* not fixed? */
          n = cast_int(L->ਸਿਖਰ.p - ra);  /* get what is available */
        savepc(ci);
        ਜੇ (TESTARG_k(i)) {  /* may there be open upvalues? */
          ci->u2.nres = n;  /* save number of returns */
          ਜੇ (L->ਸਿਖਰ.p < ci->ਸਿਖਰ.p)
            L->ਸਿਖਰ.p = ci->ਸਿਖਰ.p;
          luaF_close(L, ਅਧਾਰ, CLOSEKTOP, 1);
          updatetrap(ci);
          updatestack(ci);
        }
        ਜੇ (nparams1)  /* vararg function? */
          ci->func.p -= ci->u.l.nextraargs + nparams1;
        L->ਸਿਖਰ.p = ra + n;  /* set call for 'luaD_poscall' */
        luaD_poscall(L, ci, n);
        updatetrap(ci);  /* 'luaD_poscall' can change hooks */
        ਜਾਓ ret;
      }
      vmcase(OP_RETURN0) {
        ਜੇ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->ਸਿਖਰ.p = ra;
          savepc(ci);
          luaD_poscall(L, ci, 0);  /* no hurry... */
          trap = 1;
        }
        ਨਹੀਂਤਾਂ {  /* do the 'poscall' here */
          ਪੂਰਨ nres = get_nresults(ci->callstatus);
          L->ci = ci->ਪਿਛਲਾ;  /* back to caller */
          L->ਸਿਖਰ.p = ਅਧਾਰ - 1;
          ਲਈ (; l_unlikely(nres > 0); nres--)
            setnilvalue(s2v(L->ਸਿਖਰ.p++));  /* all results are nil */
        }
        ਜਾਓ ret;
      }
      vmcase(OP_RETURN1) {
        ਜੇ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->ਸਿਖਰ.p = ra + 1;
          savepc(ci);
          luaD_poscall(L, ci, 1);  /* no hurry... */
          trap = 1;
        }
        ਨਹੀਂਤਾਂ {  /* do the 'poscall' here */
          ਪੂਰਨ nres = get_nresults(ci->callstatus);
          L->ci = ci->ਪਿਛਲਾ;  /* back to caller */
          ਜੇ (nres == 0)
            L->ਸਿਖਰ.p = ਅਧਾਰ - 1;  /* asked for no results */
          ਨਹੀਂਤਾਂ {
            StkId ra = RA(i);
            setobjs2s(L, ਅਧਾਰ - 1, ra);  /* at least this result */
            L->ਸਿਖਰ.p = ਅਧਾਰ;
            ਲਈ (; l_unlikely(nres > 1); nres--)
              setnilvalue(s2v(L->ਸਿਖਰ.p++));  /* complete missing results */
          }
        }
       ret:  /* return from a Lua function */
        ਜੇ (ci->callstatus & CIST_FRESH)
          ਵਾਪਸ;  /* end this frame */
        ਨਹੀਂਤਾਂ {
          ci = ci->ਪਿਛਲਾ;
          ਜਾਓ returning;  /* continue running caller in this frame */
        }
      }
      vmcase(OP_FORLOOP) {
        StkId ra = RA(i);
        ਜੇ (ttisinteger(s2v(ra + 1))) {  /* integer loop? */
          lua_Unsigned ਗਿਣਤੀ_2 = l_castS2U(ivalue(s2v(ra)));
          ਜੇ (ਗਿਣਤੀ_2 > 0) {  /* still more iterations? */
            lua_Integer ਕਦਮ = ivalue(s2v(ra + 1));
            lua_Integer idx = ivalue(s2v(ra + 2));  /* control variable */
            chgivalue(s2v(ra), l_castU2S(ਗਿਣਤੀ_2 - 1));  /* update counter */
            idx = intop(+, idx, ਕਦਮ);  /* add step to index */
            chgivalue(s2v(ra + 2), idx);  /* update control variable */
            pc -= GETARG_Bx(i);  /* jump back */
          }
        }
        ਨਹੀਂਤਾਂ ਜੇ (floatforloop(L, ra))  /* float loop */
          pc -= GETARG_Bx(i);  /* jump back */
        updatetrap(ci);  /* allows a signal to break the loop */
        vmbreak;
      }
      vmcase(OP_FORPREP) {
        StkId ra = RA(i);
        savestate(L, ci);  /* in case of errors */
        ਜੇ (forprep(L, ra))
          pc += GETARG_Bx(i) + 1;  /* skip the loop */
        vmbreak;
      }
      vmcase(OP_TFORPREP) {
       /* before: 'ra' has the iterator function, 'ra + 1' has the state,
          'ra + 2' has the initial value for the control variable, and
          'ra + 3' has the closing variable. This opcode then swaps the
          control and the closing variables and marks the closing variable
          as to-be-closed.
       */
       StkId ra = RA(i);
       TValue ਆਰਜ਼ੀ;  /* to swap control and closing variables */
       setobj(L, &ਆਰਜ਼ੀ, s2v(ra + 3));
       setobjs2s(L, ra + 3, ra + 2);
       setobj2s(L, ra + 2, &ਆਰਜ਼ੀ);
        /* create to-be-closed upvalue (if closing var. is not nil) */
        halfProtect(luaF_newtbcupval(L, ra + 2));
        pc += GETARG_Bx(i);  /* go to end of the loop */
        i = *(pc++);  /* fetch next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORCALL && ra == RA(i));
        ਜਾਓ l_tforcall;
      }
      vmcase(OP_TFORCALL) {
       l_tforcall: {
        /* 'ra' has the iterator function, 'ra + 1' has the state,
           'ra + 2' has the closing variable, and 'ra + 3' has the control
           variable. The call will use the stack starting at 'ra + 3',
           so that it preserves the first three values, and the first
           return will be the new value for the control variable.
        */
        StkId ra = RA(i);
        setobjs2s(L, ra + 5, ra + 3);  /* copy the control variable */
        setobjs2s(L, ra + 4, ra + 1);  /* copy state */
        setobjs2s(L, ra + 3, ra);  /* copy function */
        L->ਸਿਖਰ.p = ra + 3 + 3;
        ProtectNT(luaD_call(L, ra + 3, GETARG_C(i)));  /* do the call */
        updatestack(ci);  /* stack may have changed */
        i = *(pc++);  /* go to next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
        ਜਾਓ l_tforloop;
      }}
      vmcase(OP_TFORLOOP) {
       l_tforloop: {
        StkId ra = RA(i);
        ਜੇ (!ttisnil(s2v(ra + 3)))  /* continue loop? */
          pc -= GETARG_Bx(i);  /* jump back */
        vmbreak;
      }}
      vmcase(OP_SETLIST) {
        StkId ra = RA(i);
        ਬਿਨਾਨਿਸ਼ਾਨੀ n = cast_uint(GETARG_vB(i));
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ = cast_uint(GETARG_vC(i));
        ਸਾਰਣੀ *h = hvalue(s2v(ra));
        ਜੇ (n == 0)
          n = cast_uint(L->ਸਿਖਰ.p - ra) - 1;  /* get up to the top */
        ਨਹੀਂਤਾਂ
          L->ਸਿਖਰ.p = ci->ਸਿਖਰ.p;  /* correct top in case of emergency GC */
        ਆਖਰੀ += n;
        ਜੇ (TESTARG_k(i)) {
          ਆਖਰੀ += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
          pc++;
        }
        /* when 'n' is known, table should have proper size */
        ਜੇ (ਆਖਰੀ > h->asize) {  /* needs more space? */
          /* fixed-size sets should have space preallocated */
          lua_assert(GETARG_vB(i) == 0);
          luaH_resizearray(L, h, ਆਖਰੀ);  /* preallocate it at once */
        }
        ਲਈ (; n > 0; n--) {
          TValue *val = s2v(ra + n);
          obj2arr(h, ਆਖਰੀ - 1, val);
          ਆਖਰੀ--;
          luaC_barrierback(L, obj2gco(h), val);
        }
        vmbreak;
      }
      vmcase(OP_ਘੇਰਾ) {
        StkId ra = RA(i);
        Proto *p = cl->p->p[GETARG_Bx(i)];
        halfProtect(pushclosure(L, p, cl->upvals, ਅਧਾਰ, ra));
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_VARARG) {
        StkId ra = RA(i);
        ਪੂਰਨ n = GETARG_C(i) - 1;  /* required results (-1 means all) */
        ਪੂਰਨ vatab = GETARG_k(i) ? GETARG_B(i) : -1;
        Protect(luaT_getvarargs(L, ci, ra, n, vatab));
        vmbreak;
      }
      vmcase(OP_GETVARG) {
        StkId ra = RA(i);
        TValue *rc = vRC(i);
        luaT_getvararg(ci, ra, rc);
        vmbreak;
      }
      vmcase(OP_ERRNNIL) {
        TValue *ra = vRA(i);
        ਜੇ (!ttisnil(ra))
          halfProtect(luaG_errnnil(L, cl, GETARG_Bx(i)));
        vmbreak;
      }
      vmcase(OP_VARARGPREP) {
        ProtectNT(luaT_adjustvarargs(L, ci, cl->p));
        ਜੇ (l_unlikely(trap)) {  /* previous "Protect" updated trap */
          luaD_hookcall(L, ci);
          L->oldpc = 1;  /* next opcode will be seen as a "new" line */
        }
        updatebase(ci);  /* function has new base after adjustment */
        vmbreak;
      }
      vmcase(OP_EXTRAARG) {
        lua_assert(0);
        vmbreak;
      }
    }
  }
}

/* }================================================================== */
