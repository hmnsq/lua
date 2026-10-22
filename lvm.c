/*
** $Id: lvm.c $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lvm_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"

#ಅನ್ತರ್ಭಾವ <float.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"


/*
** By default, use jump tables in the main interpreter loop on gcc
** and compatible compilers.
*/
#ಆದರೆ !defined(LUA_USE_JUMPTABLE)
#ಆದರೆ defined(__GNUC__)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_JUMPTABLE	1
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_JUMPTABLE	0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ



/* limit for table tag-method chains (to avoid infinite loops) */
#ವ್ಯಾಖ್ಯಾನಿಸು MAXTAGLOOP	2000


/*
** 'l_intfitsf' checks whether a given integer is in the range that
** can be converted to a float without rounding. Used in comparisons.
*/

/* number of bits in the mantissa of a float */
#ವ್ಯಾಖ್ಯಾನಿಸು NBM		(l_floatatt(MANT_DIG))

/*
** Check whether some integers may not fit in a float, testing whether
** (maxinteger >> NBM) > 0. (That implies (1 << NBM) <= maxinteger.)
** (The shifts are done in parts, to avoid shifting by more than the size
** of an integer. In a worst case, NBM == 113 for long double and
** sizeof(long) == 32.)
*/
#ಆದರೆ ((((LUA_MAXINTEGER >> (NBM / 4)) >> (NBM / 4)) >> (NBM / 4)) \
	>> (NBM - (3 * (NBM / 4))))  >  0

/* limit for integers that fit in a float */
#ವ್ಯಾಖ್ಯಾನಿಸು MAXINTFITSF	((lua_Unsigned)1 << NBM)

/* check whether 'i' is in the interval [-MAXINTFITSF, MAXINTFITSF] */
#ವ್ಯಾಖ್ಯಾನಿಸು l_intfitsf(i)	((MAXINTFITSF + l_castS2U(i)) <= (2 * MAXINTFITSF))

#ಇಲ್ಲದಿದ್ದರೆ  /* all integers fit in a float precisely */

#ವ್ಯಾಖ್ಯಾನಿಸು l_intfitsf(i)	1

#ಆದರೆ_ಅಂತ್ಯ


/*
** Try to convert a value from string to a number value.
** If the value is not a string or is a string not representing
** a valid numeral (or if coercions from strings to numbers
** are disabled via macro 'cvt2num'), do not modify 'result'
** and return 0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ l_strton (ಸ್ಥಿರಾಂಕ TValue *obj, TValue *ಫಲಿತಾಂಶ) {
  lua_assert(obj != ಫಲಿತಾಂಶ);
  ಆದರೆ (!cvt2num(obj))  /* is object not a string? */
    ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ {
    TString *st = tsvalue(obj);
    size_t stlen;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getlstr(st, stlen);
    ಹಿಂತಿರುಗಿ (luaO_str2num(s, ಫಲಿತಾಂಶ) == stlen + 1);
  }
}


/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'tonumber'.
*/
ಪೂರ್ಣಾಂಕ luaV_tonumber_ (ಸ್ಥಿರಾಂಕ TValue *obj, lua_ಸಂಖ್ಯೆ *n) {
  TValue v;
  ಆದರೆ (ttisinteger(obj)) {
    *n = cast_ಅಂಕೆ(ivalue(obj));
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (l_strton(obj, &v)) {  /* string coercible to number? */
    *n = nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;  /* conversion failed */
}


/*
** try to convert a float to an integer, rounding according to 'mode'.
*/
ಪೂರ್ಣಾಂಕ luaV_flttointeger (lua_ಸಂಖ್ಯೆ n, lua_Integer *p, F2Imod ಬಹುಲಕ) {
  lua_ಸಂಖ್ಯೆ f = l_floor(n);
  ಆದರೆ (n != f) {  /* not an integral value? */
    ಆದರೆ (ಬಹುಲಕ == F2Ieq) ಹಿಂತಿರುಗಿ 0;  /* fails if mode demands integral value */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಹುಲಕ == F2Iceil)  /* needs ceiling? */
      f += 1;  /* convert floor to ceiling (remember: n != f) */
  }
  ಹಿಂತಿರುಗಿ lua_numbertointeger(f, p);
}


/*
** try to convert a value to an integer, rounding according to 'mode',
** without string coercion.
** ("Fast track" handled by macro 'tointegerns'.)
*/
ಪೂರ್ಣಾಂಕ luaV_tointegerns (ಸ್ಥಿರಾಂಕ TValue *obj, lua_Integer *p, F2Imod ಬಹುಲಕ) {
  ಆದರೆ (ttisfloat(obj))
    ಹಿಂತಿರುಗಿ luaV_flttointeger(fltvalue(obj), p, ಬಹುಲಕ);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisinteger(obj)) {
    *p = ivalue(obj);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;
}


/*
** try to convert a value to an integer.
*/
ಪೂರ್ಣಾಂಕ luaV_tointeger (ಸ್ಥಿರಾಂಕ TValue *obj, lua_Integer *p, F2Imod ಬಹುಲಕ) {
  TValue v;
  ಆದರೆ (l_strton(obj, &v))  /* does 'obj' point to a numerical string? */
    obj = &v;  /* change it to point to its corresponding number */
  ಹಿಂತಿರುಗಿ luaV_tointegerns(obj, p, ಬಹುಲಕ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ forlimit (lua_State *L, lua_Integer init, ಸ್ಥಿರಾಂಕ TValue *lim,
                                   lua_Integer *p, lua_Integer ಹೆಜ್ಜೆ) {
  ಆದರೆ (!luaV_tointeger(lim, p, (ಹೆಜ್ಜೆ < 0 ? F2Iceil : F2Ifloor))) {
    /* not coercible to in integer */
    lua_ಸಂಖ್ಯೆ flim;  /* try to convert to float */
    ಆದರೆ (!tonumber(lim, &flim)) /* cannot convert to float? */
      luaG_forerror(L, lim, "limit");
    /* else 'flim' is a float out of integer bounds */
    ಆದರೆ (luai_numlt(0, flim)) {  /* if it is positive, it is too large */
      ಆದರೆ (ಹೆಜ್ಜೆ < 0) ಹಿಂತಿರುಗಿ 1;  /* initial value must be less than it */
      *p = LUA_MAXINTEGER;  /* truncate */
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (luai_numgt(0, flim)) {  /* if it is negative, it is too small */
      ಆದರೆ (ಹೆಜ್ಜೆ > 0) ಹಿಂತಿರುಗಿ 1;  /* initial value must be greater than it */
      *p = LUA_MININTEGER;  /* truncate */
    }
    ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 1;  /* limit is NaN (neither 0 nor positive nor negative) */
  }
  ಹಿಂತಿರುಗಿ (ಹೆಜ್ಜೆ > 0 ? init > *p : init < *p);  /* not to run? */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ forprep (lua_State *L, StkId ra) {
  TValue *pinit = s2v(ra);
  TValue *plimit = s2v(ra + 1);
  TValue *pstep = s2v(ra + 2);
  ಆದರೆ (ttisinteger(pinit) && ttisinteger(pstep)) { /* integer loop? */
    lua_Integer init = ivalue(pinit);
    lua_Integer ಹೆಜ್ಜೆ = ivalue(pstep);
    lua_Integer ಎಲ್ಲೆ;
    ಆದರೆ (ಹೆಜ್ಜೆ == 0)
      luaG_runerror(L, "'for' step is zero");
    ಆದರೆ (forlimit(L, init, plimit, &ಎಲ್ಲೆ, ಹೆಜ್ಜೆ))
      ಹಿಂತಿರುಗಿ 1;  /* skip the loop */
    ಇಲ್ಲದಿದ್ದರೆ {  /* prepare loop counter */
      lua_Unsigned ಎಣಿಕೆ_2;
      ಆದರೆ (ಹೆಜ್ಜೆ > 0) {  /* ascending loop? */
        ಎಣಿಕೆ_2 = l_castS2U(ಎಲ್ಲೆ) - l_castS2U(init);
        ಆದರೆ (ಹೆಜ್ಜೆ != 1)  /* avoid division in the too common case */
          ಎಣಿಕೆ_2 /= l_castS2U(ಹೆಜ್ಜೆ);
      }
      ಇಲ್ಲದಿದ್ದರೆ {  /* step < 0; descending loop */
        ಎಣಿಕೆ_2 = l_castS2U(init) - l_castS2U(ಎಲ್ಲೆ);
        /* 'step+1' avoids negating 'mininteger' */
        ಎಣಿಕೆ_2 /= l_castS2U(-(ಹೆಜ್ಜೆ + 1)) + 1u;
      }
      /* use 'chgivalue' for places that for sure had integers */
      chgivalue(s2v(ra), l_castU2S(ಎಣಿಕೆ_2));  /* change init to count */
      setivalue(s2v(ra + 1), ಹೆಜ್ಜೆ);  /* change limit to step */
      chgivalue(s2v(ra + 2), init);  /* change step to init */
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* try making all values floats */
    lua_ಸಂಖ್ಯೆ init; lua_ಸಂಖ್ಯೆ ಎಲ್ಲೆ; lua_ಸಂಖ್ಯೆ ಹೆಜ್ಜೆ;
    ಆದರೆ (l_unlikely(!tonumber(plimit, &ಎಲ್ಲೆ)))
      luaG_forerror(L, plimit, "limit");
    ಆದರೆ (l_unlikely(!tonumber(pstep, &ಹೆಜ್ಜೆ)))
      luaG_forerror(L, pstep, "step");
    ಆದರೆ (l_unlikely(!tonumber(pinit, &init)))
      luaG_forerror(L, pinit, "initial value");
    ಆದರೆ (ಹೆಜ್ಜೆ == 0)
      luaG_runerror(L, "'for' step is zero");
    ಆದರೆ (!(luai_numge(0, ಹೆಜ್ಜೆ) ? luai_numge(init, ಎಲ್ಲೆ)
                              : luai_numge(ಎಲ್ಲೆ, init)))
      ಹಿಂತಿರುಗಿ 1;  /* skip the loop */
    ಇಲ್ಲದಿದ್ದರೆ {
      /* make sure all values are floats */
      setfltvalue(s2v(ra), ಎಲ್ಲೆ);
      setfltvalue(s2v(ra + 1), ಹೆಜ್ಜೆ);
      setfltvalue(s2v(ra + 2), init);  /* control variable */
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}


/*
** Execute a step of a float numerical for loop, returning
** true iff the loop must continue. (The integer case is
** written inline with opcode OP_FORLOOP, for performance.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ floatforloop (lua_State *L, StkId ra) {
  lua_ಸಂಖ್ಯೆ ಹೆಜ್ಜೆ = fltvalue(s2v(ra + 1));
  lua_ಸಂಖ್ಯೆ ಎಲ್ಲೆ = fltvalue(s2v(ra));
  lua_ಸಂಖ್ಯೆ idx = fltvalue(s2v(ra + 2));  /* control variable */
  idx = luai_numadd(L, idx, ಹೆಜ್ಜೆ);  /* increment index */
  ಆದರೆ (luai_numlt(0, ಹೆಜ್ಜೆ) ? luai_numle(idx, ಎಲ್ಲೆ)
                          : luai_numle(ಎಲ್ಲೆ, idx)) {
    chgfltvalue(s2v(ra + 2), idx);  /* update control variable */
    ಹಿಂತಿರುಗಿ 1;  /* jump back */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;  /* finish the loop */
}


/*
** Finish the table access 'val = t[key]' and return the tag of the result.
*/
lu_byte luaV_finishget (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t, TValue *ಕೀಲಿ,
                                      StkId val, lu_byte ಟ್ಯಾಗ್) {
  ಪೂರ್ಣಾಂಕ loop;  /* counter to avoid infinite loops */
  ಸ್ಥಿರಾಂಕ TValue *tm;  /* metamethod */
  ಗಾಗಿ (loop = 0; loop < MAXTAGLOOP; loop++) {
    ಆದರೆ (ಟ್ಯಾಗ್ == LUA_VNOTABLE) {  /* 't' is not a table? */
      lua_assert(!ttistable(t));
      tm = luaT_gettmbyobj(L, t, TM_ಸೂಚ್ಯಂಕ);
      ಆದರೆ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* 't' is a table */
      tm = fasttm(L, hvalue(t)->metatable, TM_ಸೂಚ್ಯಂಕ);  /* table's metamethod */
      ಆದರೆ (tm == ಖಾಲಿ) {  /* no metamethod? */
        setnilvalue2s(val);  /* result is nil */
        ಹಿಂತಿರುಗಿ LUA_VNIL;
      }
      /* else will try the metamethod */
    }
    ಆದರೆ (ttisfunction(tm)) {  /* is metamethod a function? */
      ಟ್ಯಾಗ್ = luaT_callTMres(L, tm, t, ಕೀಲಿ, val);  /* call it */
      ಹಿಂತಿರುಗಿ ಟ್ಯಾಗ್;  /* return tag of the result */
    }
    t = tm;  /* else try to access 'tm[key]' */
    luaV_fastget(t, ಕೀಲಿ, s2v(val), luaH_get, ಟ್ಯಾಗ್);
    ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್))
      ಹಿಂತಿರುಗಿ ಟ್ಯಾಗ್;  /*ಮುಗಿದ*/
    /* else repeat (tail call 'luaV_finishget') */
  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
  ಹಿಂತಿರುಗಿ 0;  /* to avoid warnings */
}


/*
** Finish a table assignment 't[key] = val'.
** About anchoring the table before the call to 'luaH_finishset':
** This call may trigger an emergency collection. When loop>0,
** the table being accessed is a field in some metatable. If this
** metatable is weak and the table is not anchored, this collection
** could collect that table while it is being updated.
*/
ಶೂನ್ಯ luaV_finishset (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t, TValue *ಕೀಲಿ,
                      TValue *val, ಪೂರ್ಣಾಂಕ hres) {
  ಪೂರ್ಣಾಂಕ loop;  /* counter to avoid infinite loops */
  ಗಾಗಿ (loop = 0; loop < MAXTAGLOOP; loop++) {
    ಸ್ಥಿರಾಂಕ TValue *tm;  /* '__newindex' metamethod */
    ಆದರೆ (hres != HNOTATABLE) {  /* is 't' a table? */
      ಕೋಷ್ಟಕ *h = hvalue(t);  /* save 't' table */
      tm = fasttm(L, h->metatable, TM_NEWINDEX);  /* get metamethod */
      ಆದರೆ (tm == ಖಾಲಿ) {  /* no metamethod? */
        sethvalue2s(L, L->ತುದಿ.p, h);  /* anchor 't' */
        L->ತುದಿ.p++;  /* assume EXTRA_STACK */
        luaH_finishset(L, h, ಕೀಲಿ, val, hres);  /* set new value */
        L->ತುದಿ.p--;
        invalidateTMcache(h);
        luaC_barrierback(L, obj2gco(h), val);
        ಹಿಂತಿರುಗಿ;
      }
      /* else will try the metamethod */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* not a table; check metamethod */
      tm = luaT_gettmbyobj(L, t, TM_NEWINDEX);
      ಆದರೆ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");
    }
    /* try the metamethod */
    ಆದರೆ (ttisfunction(tm)) {
      luaT_callTM(L, tm, t, ಕೀಲಿ, val);
      ಹಿಂತಿರುಗಿ;
    }
    t = tm;  /* else must repeat assignment over 'tm' */
    /* do the equivalent to 'luaV_fastset', but saving 'h' */
    ಆದರೆ (!ttistable(t))
      hres = HNOTATABLE;
    ಇಲ್ಲದಿದ್ದರೆ {
      ಕೋಷ್ಟಕ *h = hvalue(t);  /* next call can change the value at 't' */
      hres = luaH_pset(h, ಕೀಲಿ, val);
      ಆದರೆ (hres == HOK) {
        luaC_barrierback(L, obj2gco(h), val);  /* luaV_finishfastset */
        ಹಿಂತಿರುಗಿ;  /*ಮುಗಿದ*/
      }
    }
    /* else 'return luaV_finishset(L, t, key, val, hres)' (loop) */
  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}


/*
** Function to be used for 0-terminated string order comparison
*/
#ಆದರೆ !defined(l_strcoll)
#ವ್ಯಾಖ್ಯಾನಿಸು l_strcoll	strcoll
#ಆದರೆ_ಅಂತ್ಯ


/*
** Compare two strings 'ts1' x 'ts2', returning an integer less-equal-
** -greater than zero if 'ts1' is less-equal-greater than 'ts2'.
** The code is a little tricky because it allows '\0' in the strings
** and it uses 'strcoll' (to respect locales) for each segment
** of the strings. Note that segments can compare equal but still
** have different lengths.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ l_strcmp (ಸ್ಥಿರಾಂಕ TString *ts1, ಸ್ಥಿರಾಂಕ TString *ts2) {
  size_t rl1;  /* real length */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1 = getlstr(ts1, rl1);
  size_t rl2;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2 = getlstr(ts2, rl2);
  ಗಾಗಿ (;;) {  /* for each segment */
    ಪೂರ್ಣಾಂಕ ತಾತ್ಕಾಲಿಕ = l_strcoll(s1, s2);
    ಆದರೆ (ತಾತ್ಕಾಲಿಕ != 0)  /* not equal? */
      ಹಿಂತಿರುಗಿ ತಾತ್ಕಾಲಿಕ;  /*ಮುಗಿದ*/
    ಇಲ್ಲದಿದ್ದರೆ {  /* strings are equal up to a '\0' */
      size_t zl1 = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s1);  /* index of first '\0' in 's1' */
      size_t zl2 = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s2);  /* index of first '\0' in 's2' */
      ಆದರೆ (zl2 == rl2)  /* 's2' is finished? */
        ಹಿಂತಿರುಗಿ (zl1 == rl1) ? 0 : 1;  /* check 's1' */
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (zl1 == rl1)  /* 's1' is finished? */
        ಹಿಂತಿರುಗಿ -1;  /* 's1' is less than 's2' ('s2' is not finished) */
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
l_sinline ಪೂರ್ಣಾಂಕ LTintfloat (lua_Integer i, lua_ಸಂಖ್ಯೆ f) {
  ಆದರೆ (l_intfitsf(i))
    ಹಿಂತಿರುಗಿ luai_numlt(cast_ಅಂಕೆ(i), f);  /* compare them as floats */
  ಇಲ್ಲದಿದ್ದರೆ {  /* i < f <=> i < ceil(f) */
    lua_Integer fi;
    ಆದರೆ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      ಹಿಂತಿರುಗಿ i < fi;   /* compare them as integers */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'f' is either greater or less than all integers */
      ಹಿಂತಿರುಗಿ f > 0;  /* greater? */
  }
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
l_sinline ಪೂರ್ಣಾಂಕ LEintfloat (lua_Integer i, lua_ಸಂಖ್ಯೆ f) {
  ಆದರೆ (l_intfitsf(i))
    ಹಿಂತಿರುಗಿ luai_numle(cast_ಅಂಕೆ(i), f);  /* compare them as floats */
  ಇಲ್ಲದಿದ್ದರೆ {  /* i <= f <=> i <= floor(f) */
    lua_Integer fi;
    ಆದರೆ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      ಹಿಂತಿರುಗಿ i <= fi;   /* compare them as integers */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'f' is either greater or less than all integers */
      ಹಿಂತಿರುಗಿ f > 0;  /* greater? */
  }
}


/*
** Check whether float 'f' is less than integer 'i'.
** See comments on previous function.
*/
l_sinline ಪೂರ್ಣಾಂಕ LTfloatint (lua_ಸಂಖ್ಯೆ f, lua_Integer i) {
  ಆದರೆ (l_intfitsf(i))
    ಹಿಂತಿರುಗಿ luai_numlt(f, cast_ಅಂಕೆ(i));  /* compare them as floats */
  ಇಲ್ಲದಿದ್ದರೆ {  /* f < i <=> floor(f) < i */
    lua_Integer fi;
    ಆದರೆ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      ಹಿಂತಿರುಗಿ fi < i;   /* compare them as integers */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'f' is either greater or less than all integers */
      ಹಿಂತಿರುಗಿ f < 0;  /* less? */
  }
}


/*
** Check whether float 'f' is less than or equal to integer 'i'.
** See comments on previous function.
*/
l_sinline ಪೂರ್ಣಾಂಕ LEfloatint (lua_ಸಂಖ್ಯೆ f, lua_Integer i) {
  ಆದರೆ (l_intfitsf(i))
    ಹಿಂತಿರುಗಿ luai_numle(f, cast_ಅಂಕೆ(i));  /* compare them as floats */
  ಇಲ್ಲದಿದ್ದರೆ {  /* f <= i <=> ceil(f) <= i */
    lua_Integer fi;
    ಆದರೆ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      ಹಿಂತಿರುಗಿ fi <= i;   /* compare them as integers */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'f' is either greater or less than all integers */
      ಹಿಂತಿರುಗಿ f < 0;  /* less? */
  }
}


/*
** Return 'l < r', for numbers.
*/
l_sinline ಪೂರ್ಣಾಂಕ LTnum (ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ಆದರೆ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ಆದರೆ (ttisinteger(r))
      ಹಿಂತಿರುಗಿ li < ivalue(r);  /* both are integers */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'l' is int and 'r' is float */
      ಹಿಂತಿರುಗಿ LTintfloat(li, fltvalue(r));  /* l < r ? */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_ಸಂಖ್ಯೆ lf = fltvalue(l);  /* 'l' must be float */
    ಆದರೆ (ttisfloat(r))
      ಹಿಂತಿರುಗಿ luai_numlt(lf, fltvalue(r));  /* both are float */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'l' is float and 'r' is int */
      ಹಿಂತಿರುಗಿ LTfloatint(lf, ivalue(r));
  }
}


/*
** Return 'l <= r', for numbers.
*/
l_sinline ಪೂರ್ಣಾಂಕ LEnum (ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ಆದರೆ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ಆದರೆ (ttisinteger(r))
      ಹಿಂತಿರುಗಿ li <= ivalue(r);  /* both are integers */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'l' is int and 'r' is float */
      ಹಿಂತಿರುಗಿ LEintfloat(li, fltvalue(r));  /* l <= r ? */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_ಸಂಖ್ಯೆ lf = fltvalue(l);  /* 'l' must be float */
    ಆದರೆ (ttisfloat(r))
      ಹಿಂತಿರುಗಿ luai_numle(lf, fltvalue(r));  /* both are float */
    ಇಲ್ಲದಿದ್ದರೆ  /* 'l' is float and 'r' is int */
      ಹಿಂತಿರುಗಿ LEfloatint(lf, ivalue(r));
  }
}


/*
** return 'l < r' for non-numbers.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lessthanothers (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ಆದರೆ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    ಹಿಂತಿರುಗಿ l_strcmp(tsvalue(l), tsvalue(r)) < 0;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ luaT_callorderTM(L, l, r, TM_LT);
}


/*
** Main operation less than; return 'l < r'.
*/
ಪೂರ್ಣಾಂಕ luaV_lessthan (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r) {
  ಆದರೆ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    ಹಿಂತಿರುಗಿ LTnum(l, r);
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ lessthanothers(L, l, r);
}


/*
** return 'l <= r' for non-numbers.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lessequalothers (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ಆದರೆ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    ಹಿಂತಿರುಗಿ l_strcmp(tsvalue(l), tsvalue(r)) <= 0;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ luaT_callorderTM(L, l, r, TM_LE);
}


/*
** Main operation less than or equal to; return 'l <= r'.
*/
ಪೂರ್ಣಾಂಕ luaV_lessequal (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r) {
  ಆದರೆ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    ಹಿಂತಿರುಗಿ LEnum(l, r);
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ lessequalothers(L, l, r);
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
ಪೂರ್ಣಾಂಕ luaV_equalobj (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t1, ಸ್ಥಿರಾಂಕ TValue *t2) {
  ಸ್ಥಿರಾಂಕ TValue *tm;
  ಆದರೆ (ttype(t1) != ttype(t2))  /* not the same type? */
    ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttypetag(t1) != ttypetag(t2)) {
    ಆಯ್ಕೆ (ttypetag(t1)) {
      ಸ್ಥಿತಿ LUA_VNUMINT: {  /* integer == float? */
        /* integer and float can only be equal if float has an integer
           value equal to the integer */
        lua_Integer i2;
        ಹಿಂತಿರುಗಿ (luaV_flttointeger(fltvalue(t2), &i2, F2Ieq) &&
                ivalue(t1) == i2);
      }
      ಸ್ಥಿತಿ LUA_VNUMFLT: {  /* float == integer? */
        lua_Integer i1;  /* see comment in previous case */
        ಹಿಂತಿರುಗಿ (luaV_flttointeger(fltvalue(t1), &i1, F2Ieq) &&
                i1 == ivalue(t2));
      }
      ಸ್ಥಿತಿ LUA_VSHRSTR: ಸ್ಥಿತಿ LUA_VLNGSTR: {
        /* compare two strings with different variants: they can be
           equal when one string is a short string and the other is
           an external string  */
        ಹಿಂತಿರುಗಿ luaS_eqstr(tsvalue(t1), tsvalue(t2));
      }
      ಪೂರ್ವನಿಯೋಜಿತ:
        /* only numbers (integer/float) and strings (long/short) can have
           equal values with different variants */
        ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* equal variants */
    ಆಯ್ಕೆ (ttypetag(t1)) {
      ಸ್ಥಿತಿ LUA_VNIL: ಸ್ಥಿತಿ LUA_VFALSE: ಸ್ಥಿತಿ LUA_VTRUE:
        ಹಿಂತಿರುಗಿ 1;
      ಸ್ಥಿತಿ LUA_VNUMINT:
        ಹಿಂತಿರುಗಿ (ivalue(t1) == ivalue(t2));
      ಸ್ಥಿತಿ LUA_VNUMFLT:
        ಹಿಂತಿರುಗಿ (fltvalue(t1) == fltvalue(t2));
      ಸ್ಥಿತಿ LUA_VLIGHTUSERDATA: ಹಿಂತಿರುಗಿ pvalue(t1) == pvalue(t2);
      ಸ್ಥಿತಿ LUA_VSHRSTR:
        ಹಿಂತಿರುಗಿ eqshrstr(tsvalue(t1), tsvalue(t2));
      ಸ್ಥಿತಿ LUA_VLNGSTR:
        ಹಿಂತಿರುಗಿ luaS_eqstr(tsvalue(t1), tsvalue(t2));
      ಸ್ಥಿತಿ LUA_VUSERDATA: {
        ಆದರೆ (uvalue(t1) == uvalue(t2)) ಹಿಂತಿರುಗಿ 1;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (L == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 0;
        tm = fasttm(L, uvalue(t1)->metatable, TM_EQ);
        ಆದರೆ (tm == ಖಾಲಿ)
          tm = fasttm(L, uvalue(t2)->metatable, TM_EQ);
        ನಿಲ್ಲಿಸು;  /* will try TM */
      }
      ಸ್ಥಿತಿ LUA_VTABLE: {
        ಆದರೆ (hvalue(t1) == hvalue(t2)) ಹಿಂತಿರುಗಿ 1;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (L == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 0;
        tm = fasttm(L, hvalue(t1)->metatable, TM_EQ);
        ಆದರೆ (tm == ಖಾಲಿ)
          tm = fasttm(L, hvalue(t2)->metatable, TM_EQ);
        ನಿಲ್ಲಿಸು;  /* will try TM */
      }
      ಸ್ಥಿತಿ LUA_VLCF:
        ಹಿಂತಿರುಗಿ (fvalue(t1) == fvalue(t2));
      ಪೂರ್ವನಿಯೋಜಿತ:  /* functions and threads */
        ಹಿಂತಿರುಗಿ (gcvalue(t1) == gcvalue(t2));
    }
    ಆದರೆ (tm == ಖಾಲಿ)  /* no TM? */
      ಹಿಂತಿರುಗಿ 0;  /* objects are different */
    ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್ = luaT_callTMres(L, tm, t1, t2, L->ತುದಿ.p);  /* call TM */
      ಹಿಂತಿರುಗಿ !tagisfalse(ಟ್ಯಾಗ್);
    }
  }
}


/* macro used by 'luaV_concat' to ensure that element at 'o' is a string */
#ವ್ಯಾಖ್ಯಾನಿಸು tostring(L,o)  \
	(ttisstring(o) || (cvt2str(o) && (luaO_tostring(L, o), 1)))

/*
** Check whether object is a short empty string to optimize concatenation.
** (External strings can be empty too; they will be concatenated like
** non-empty ones.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು isemptystr(o)	(ttisshrstring(o) && tsvalue(o)->shrlen == 0)

/* copy strings in stack from top - n up to top - 1 to buffer */
ಸ್ಥಿರ ಶೂನ್ಯ copy2buff (StkId ತುದಿ, ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *buff) {
  size_t tl = 0;  /* size already copied */
  ಮಾಡು {
    TString *st = tsvalue(s2v(ತುದಿ - n));
    size_t l;  /* length of string being copied */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getlstr(st, l);
    ಸ್ಮರಣೆ_ನಕಲು(buff + tl, s, l * ಗಾತ್ರ(ಅಕ್ಷರ));
    tl += l;
  } ಆಗ (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top.p - total' up to 'L->top.p - 1'.
*/
ಶೂನ್ಯ luaV_concat (lua_State *L, ಪೂರ್ಣಾಂಕ ಒಟ್ಟು) {
  ಆದರೆ (ಒಟ್ಟು == 1)
    ಹಿಂತಿರುಗಿ;  /* "all" values already concatenated */
  ಮಾಡು {
    StkId ತುದಿ = L->ತುದಿ.p;
    ಪೂರ್ಣಾಂಕ n = 2;  /* number of elements handled in this pass (at least 2) */
    ಆದರೆ (!(ttisstring(s2v(ತುದಿ - 2)) || cvt2str(s2v(ತುದಿ - 2))) ||
        !tostring(L, s2v(ತುದಿ - 1)))
      luaT_tryconcatTM(L);  /* may invalidate 'top' */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isemptystr(s2v(ತುದಿ - 1)))  /* second operand is empty? */
      cast_void(tostring(L, s2v(ತುದಿ - 2)));  /* result is first operand */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isemptystr(s2v(ತುದಿ - 2))) {  /* first operand is empty string? */
      setobjs2s(L, ತುದಿ - 2, ತುದಿ - 1);  /* result is second op. */
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      /* at least two string values; get as many as possible */
      size_t tl = tsslen(tsvalue(s2v(ತುದಿ - 1)));  /* total length */
      TString *ts;
      /* collect total length and number of strings */
      ಗಾಗಿ (n = 1; n < ಒಟ್ಟು && tostring(L, s2v(ತುದಿ - n - 1)); n++) {
        size_t l = tsslen(tsvalue(s2v(ತುದಿ - n - 1)));
        ಆದರೆ (l_unlikely(l >= MAX_ಗಾತ್ರ - ಗಾತ್ರ(TString) - tl)) {
          L->ತುದಿ.p = ತುದಿ - ಒಟ್ಟು;  /* pop strings to avoid wasting stack */
          luaG_runerror(L, "string length overflow");
        }
        tl += l;
      }
      ಆದರೆ (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        ಅಕ್ಷರ buff[LUAI_MAXSHORTLEN];
        copy2buff(ತುದಿ, n, buff);  /* copy strings to buffer */
        ts = luaS_newlstr(L, buff, tl);
      }
      ಇಲ್ಲದಿದ್ದರೆ {  /* long string; copy strings directly to final result */
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(ತುದಿ, n, getlngstr(ts));
      }
      setsvalue2s(L, ತುದಿ - n, ts);  /* create result */
    }
    ಒಟ್ಟು -= n - 1;  /* got 'n' strings to create one new */
    L->ತುದಿ.p -= n - 1;  /* popped 'n' strings and pushed one */
  } ಆಗ (ಒಟ್ಟು > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra = #rb'.
*/
ಶೂನ್ಯ luaV_objlen (lua_State *L, StkId ra, ಸ್ಥಿರಾಂಕ TValue *rb) {
  ಸ್ಥಿರಾಂಕ TValue *tm;
  ಆಯ್ಕೆ (ttypetag(rb)) {
    ಸ್ಥಿತಿ LUA_VTABLE: {
      ಕೋಷ್ಟಕ *h = hvalue(rb);
      tm = fasttm(L, h->metatable, TM_LEN);
      ಆದರೆ (tm) ನಿಲ್ಲಿಸು;  /* metamethod? break switch to call it */
      setivalue(s2v(ra), l_castU2S(luaH_getn(L, h)));  /* else primitive len */
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ LUA_VSHRSTR: {
      setivalue(s2v(ra), tsvalue(rb)->shrlen);
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ LUA_VLNGSTR: {
      setivalue(s2v(ra), cast_st2S(tsvalue(rb)->u.lnglen));
      ಹಿಂತಿರುಗಿ;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {  /* try metamethod */
      tm = luaT_gettmbyobj(L, rb, TM_LEN);
      ಆದರೆ (l_unlikely(notm(tm)))  /* no metamethod? */
        luaG_typeerror(L, rb, "get length of");
      ನಿಲ್ಲಿಸು;
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
  ಆದರೆ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ಆದರೆ (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    ಹಿಂತಿರುಗಿ intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_Integer q = m / n;  /* perform C division */
    ಆದರೆ ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    ಹಿಂತಿರುಗಿ q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_idiv.)
*/
lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  ಆದರೆ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ಆದರೆ (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    ಹಿಂತಿರುಗಿ 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_Integer r = m % n;
    ಆದರೆ (r != 0 && (r ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    ಹಿಂತಿರುಗಿ r;
  }
}


/*
** Float modulus
*/
lua_ಸಂಖ್ಯೆ luaV_modf (lua_State *L, lua_ಸಂಖ್ಯೆ m, lua_ಸಂಖ್ಯೆ n) {
  lua_ಸಂಖ್ಯೆ r;
  luai_nummod(L, m, n, r);
  ಹಿಂತಿರುಗಿ r;
}


/* number of bits in an integer */
#ವ್ಯಾಖ್ಯಾನಿಸು NBITS	l_numbits(lua_Integer)


/*
** Shift left operation. (Shift right just negates 'y'.)
*/
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  ಆದರೆ (y < 0) {  /* shift right? */
    ಆದರೆ (y <= -NBITS) ಹಿಂತಿರುಗಿ 0;
    ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ intop(>>, x, -y);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* shift left */
    ಆದರೆ (y >= NBITS) ಹಿಂತಿರುಗಿ 0;
    ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ intop(<<, x, y);
  }
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pushclosure (lua_State *L, Proto *p, ಮೇಲೆ_Val **encup, StkId ತಳ,
                         StkId ra) {
  ಪೂರ್ಣಾಂಕ nup = p->sizeupvalues;
  Upvaldesc *uv = p->upvalues;
  ಪೂರ್ಣಾಂಕ i;
  LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  setclLvalue2s(L, ra, ncl);  /* anchor new closure in stack */
  ಗಾಗಿ (i = 0; i < nup; i++) {  /* fill in its upvalues */
    ಆದರೆ (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = luaF_findupval(L, ತಳ + uv[i].idx);
    ಇಲ್ಲದಿದ್ದರೆ  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    luaC_objbarrier(L, ncl, ncl->upvals[i]);
  }
}


/*
** finish execution of an opcode interrupted by a yield
*/
ಶೂನ್ಯ lua_V_ಮುಕ್ತಾಯ_Op (lua_State *L) {
  CallInfo *ci = L->ci;
  StkId ತಳ = ci->func.p + 1;
  Instruction inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  Op_ಸಂಕೇತ op = GET_OPCODE(inst);
  ಆಯ್ಕೆ (op) {  /* finish its execution */
    ಸ್ಥಿತಿ OP_MMBIN: ಸ್ಥಿತಿ OP_MMBINI: ಸ್ಥಿತಿ OP_MMBINK: {
      setobjs2s(L, ತಳ + GETARG_A(*(ci->u.l.savedpc - 2)), --L->ತುದಿ.p);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OP_UNM: ಸ್ಥಿತಿ OP_BNOT: ಸ್ಥಿತಿ OP_LEN:
    ಸ್ಥಿತಿ OP_GETTABUP: ಸ್ಥಿತಿ OP_GETTABLE: ಸ್ಥಿತಿ OP_GETI:
    ಸ್ಥಿತಿ OP_GETFIELD: ಸ್ಥಿತಿ OP_SELF: {
      setobjs2s(L, ತಳ + GETARG_A(inst), --L->ತುದಿ.p);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OP_LT: ಸ್ಥಿತಿ OP_LE:
    ಸ್ಥಿತಿ OP_LTI: ಸ್ಥಿತಿ OP_LEI:
    ಸ್ಥಿತಿ OP_GTI: ಸ್ಥಿತಿ OP_GEI:
    ಸ್ಥಿತಿ OP_EQ: {  /* note that 'OP_EQI'/'OP_EQK' cannot yield */
      ಪೂರ್ಣಾಂಕ res = !l_isfalse(s2v(L->ತುದಿ.p - 1));
      L->ತುದಿ.p--;
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_JMP);
      ಆದರೆ (res != GETARG_k(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OP_CONCAT: {
      StkId ತುದಿ = L->ತುದಿ.p - 1;  /* top when 'luaT_tryconcatTM' was called */
      ಪೂರ್ಣಾಂಕ a = GETARG_A(inst);      /* first element to concatenate */
      ಪೂರ್ಣಾಂಕ ಒಟ್ಟು = cast_int(ತುದಿ - 1 - (ತಳ + a));  /* yet to concatenate */
      setobjs2s(L, ತುದಿ - 2, ತುದಿ);  /* put TM result in proper position */
      L->ತುದಿ.p = ತುದಿ - 1;  /* top is one after last element (at top-2) */
      luaV_concat(L, ಒಟ್ಟು);  /* concat them (may yield again) */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OP_CLOSE: {  /* yielded closing variables */
      ci->u.l.savedpc--;  /* repeat instruction to close other vars. */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OP_RETURN: {  /* yielded closing variables */
      StkId ra = ತಳ + GETARG_A(inst);
      /* adjust top to signal correct number of returns, in case the
         return is "up to top" ('isIT') */
      L->ತುದಿ.p = ra + ci->u2.nres;
      /* repeat instruction to close other vars. and complete the return */
      ci->u.l.savedpc--;
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      /* only these other opcodes can yield */
      lua_assert(op == OP_TFORCALL || op == OP_CALL ||
           op == OP_TAILCALL || op == OP_SETTABUP || op == OP_SETTABLE ||
           op == OP_SETI || op == OP_SETFIELD);
      ನಿಲ್ಲಿಸು;
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

#ವ್ಯಾಖ್ಯಾನಿಸು l_addi(L,a,b)	intop(+, a, b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_subi(L,a,b)	intop(-, a, b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_muli(L,a,b)	intop(*, a, b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_band(a,b)	intop(&, a, b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_bor(a,b)	intop(|, a, b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_bxor(a,b)	intop(^, a, b)

#ವ್ಯಾಖ್ಯಾನಿಸು l_lti(a,b)	(a < b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_lei(a,b)	(a <= b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_gti(a,b)	(a > b)
#ವ್ಯಾಖ್ಯಾನಿಸು l_gei(a,b)	(a >= b)


/*
** Arithmetic operations with immediate operands. 'iop' is the integer
** operation, 'fop' is the float operation.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_arithI(L,iop,fop) {  \
  TValue *ra = vRA(i); \
  TValue *v1 = vRB(i);  \
  ಪೂರ್ಣಾಂಕ imm = GETARG_sC(i);  \
  ಆದರೆ (ttisinteger(v1)) {  \
    lua_Integer iv1 = ivalue(v1);  \
    pc++; setivalue(ra, iop(L, iv1, imm));  \
  }  \
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisfloat(v1)) {  \
    lua_ಸಂಖ್ಯೆ nb = fltvalue(v1);  \
    lua_ಸಂಖ್ಯೆ fimm = cast_ಅಂಕೆ(imm);  \
    pc++; setfltvalue(ra, fop(L, nb, fimm)); \
  }}


/*
** Auxiliary function for arithmetic operations over floats and others
** with two operands.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_arithf_aux(L,v1,v2,fop) {  \
  lua_ಸಂಖ್ಯೆ n1; lua_ಸಂಖ್ಯೆ n2;  \
  ಆದರೆ (tonumberns(v1, n1) && tonumberns(v2, n2)) {  \
    StkId ra = RA(i);  \
    pc++; setfltvalue(s2v(ra), fop(L, n1, n2));  \
  }}


/*
** Arithmetic operations over floats and others with register operands.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_arithf(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with K operands for floats.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_arithfK(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations over integers and floats.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_arith_aux(L,v1,v2,iop,fop) {  \
  ಆದರೆ (ttisinteger(v1) && ttisinteger(v2)) {  \
    StkId ra = RA(i); \
    lua_Integer i1 = ivalue(v1); lua_Integer i2 = ivalue(v2);  \
    pc++; setivalue(s2v(ra), iop(L, i1, i2));  \
  }  \
  ಇಲ್ಲದಿದ್ದರೆ op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with register operands.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_arith(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Arithmetic operations with K operands.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_arithK(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Bitwise operations with constant operand.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_bitwiseK(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i);  \
  lua_Integer i1;  \
  lua_Integer i2 = ivalue(v2);  \
  ಆದರೆ (tointegerns(v1, &i1)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Bitwise operations with register operands.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_bitwise(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  lua_Integer i1; lua_Integer i2;  \
  ಆದರೆ (tointegerns(v1, &i1) && tointegerns(v2, &i2)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Order operations with register operands. 'opn' actually works
** for all numbers, but the fast track improves performance for
** integers.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_ಕ್ರಮ(L,opi,opn,other) {  \
  TValue *ra = vRA(i); \
  ಪೂರ್ಣಾಂಕ cond;  \
  TValue *rb = vRB(i);  \
  ಆದರೆ (ttisinteger(ra) && ttisinteger(rb)) {  \
    lua_Integer ia = ivalue(ra);  \
    lua_Integer ib = ivalue(rb);  \
    cond = opi(ia, ib);  \
  }  \
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisnumber(ra) && ttisnumber(rb))  \
    cond = opn(ra, rb);  \
  ಇಲ್ಲದಿದ್ದರೆ  \
    Protect(cond = other(L, ra, rb));  \
  docondjump(); }


/*
** Order operations with immediate operand. (Immediate operand is
** always small enough to have an exact representation as a float.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು op_ಕ್ರಮ_I(L,opi,opf,inv,tm) {  \
  TValue *ra = vRA(i); \
  ಪೂರ್ಣಾಂಕ cond;  \
  ಪೂರ್ಣಾಂಕ im = GETARG_sB(i);  \
  ಆದರೆ (ttisinteger(ra))  \
    cond = opi(ivalue(ra), im);  \
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisfloat(ra)) {  \
    lua_ಸಂಖ್ಯೆ fa = fltvalue(ra);  \
    lua_ಸಂಖ್ಯೆ fim = cast_ಅಂಕೆ(im);  \
    cond = opf(fa, fim);  \
  }  \
  ಇಲ್ಲದಿದ್ದರೆ {  \
    ಪೂರ್ಣಾಂಕ isf = GETARG_C(i);  \
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


#ವ್ಯಾಖ್ಯಾನಿಸು RA(i)	(ತಳ+GETARG_A(i))
#ವ್ಯಾಖ್ಯಾನಿಸು vRA(i)	s2v(RA(i))
#ವ್ಯಾಖ್ಯಾನಿಸು RB(i)	(ತಳ+GETARG_B(i))
#ವ್ಯಾಖ್ಯಾನಿಸು vRB(i)	s2v(RB(i))
#ವ್ಯಾಖ್ಯಾನಿಸು KB(i)	(k+GETARG_B(i))
#ವ್ಯಾಖ್ಯಾನಿಸು RC(i)	(ತಳ+GETARG_C(i))
#ವ್ಯಾಖ್ಯಾನಿಸು vRC(i)	s2v(RC(i))
#ವ್ಯಾಖ್ಯಾನಿಸು KC(i)	(k+GETARG_C(i))
#ವ್ಯಾಖ್ಯಾನಿಸು RKC(i)	((TESTARG_k(i)) ? k + GETARG_C(i) : s2v(ತಳ + GETARG_C(i)))



#ವ್ಯಾಖ್ಯಾನಿಸು updatetrap(ci)  (trap = cast_int(ci->u.l.trap))

#ವ್ಯಾಖ್ಯಾನಿಸು updatebase(ci)	(ತಳ = ci->func.p + 1)


#ವ್ಯಾಖ್ಯಾನಿಸು updatestack(ci)  \
	{ ಆದರೆ (l_unlikely(trap)) { updatebase(ci); ra = RA(i); } }


/*
** Execute a jump instruction. The 'updatetrap' allows signals to stop
** tight loops. (Without it, the local copy of 'trap' could never change.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು dojump(ci,i,e)	{ pc += GETARG_sJ(i) + e; updatetrap(ci); }


/* for test instructions, execute the jump instruction that follows it */
#ವ್ಯಾಖ್ಯಾನಿಸು donextjump(ci)	{ Instruction ni = *pc; dojump(ci, ni, 1); }

/*
** do a conditional jump: skip next instruction if 'cond' is not what
** was expected (parameter 'k'), else do next instruction, which must
** be a jump.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು docondjump()	ಆದರೆ (cond != GETARG_k(i)) pc++; ಇಲ್ಲದಿದ್ದರೆ donextjump(ci);


/*
** Correct global 'pc'.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು savepc(ci)	(ci->u.l.savedpc = pc)


/*
** Whenever code can raise errors, the global 'pc' and the global
** 'top' must be correct to report occasional errors.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು savestate(L,ci)		(savepc(ci), L->ತುದಿ.p = ci->ತುದಿ.p)


/*
** Protect code that, in general, can raise errors, reallocate the
** stack, and change the hooks.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು Protect(ಘಾತಾಂಕ)  (savestate(L,ci), (ಘಾತಾಂಕ), updatetrap(ci))

/* special version that does not change the top */
#ವ್ಯಾಖ್ಯಾನಿಸು ProtectNT(ಘಾತಾಂಕ)  (savepc(ci), (ಘಾತಾಂಕ), updatetrap(ci))

/*
** Protect code that can only raise errors. (That is, it cannot change
** the stack or hooks.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು halfProtect(ಘಾತಾಂಕ)  (savestate(L,ci), (ಘಾತಾಂಕ))

/*
** macro executed during Lua functions at points where the
** function can yield.
*/
#ಆದರೆ !defined(luai_threadyield)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_threadyield(L)	{lua_unlock(L); lua_ಬೀಗ(L);}
#ಆದರೆ_ಅಂತ್ಯ

/* 'c' is the limit of live values in the stack */
#ವ್ಯಾಖ್ಯಾನಿಸು checkGC(L,c)  \
	{ luaC_condGC(L, (savepc(ci), L->ತುದಿ.p = (c)), \
                         updatetrap(ci)); \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#ವ್ಯಾಖ್ಯಾನಿಸು vmfetch()	{ \
  ಆದರೆ (l_unlikely(trap)) {  /* stack reallocation or hooks? */ \
    trap = luaG_traceexec(L, pc);  /* handle hooks */ \
    updatebase(ci);  /* correct stack */ \
  } \
  i = *(pc++); \
}

#ವ್ಯಾಖ್ಯಾನಿಸು vmdispatch(o)	ಆಯ್ಕೆ(o)
#ವ್ಯಾಖ್ಯಾನಿಸು vmcase(l)	ಸ್ಥಿತಿ l:
#ವ್ಯಾಖ್ಯಾನಿಸು vmbreak		ನಿಲ್ಲಿಸು


ಶೂನ್ಯ luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId ತಳ;
  ಸ್ಥಿರಾಂಕ Instruction *pc;
  ಪೂರ್ಣಾಂಕ trap;
#ಆದರೆ LUA_USE_JUMPTABLE
#ಅನ್ತರ್ಭಾವ "ljumptab.h"
#ಆದರೆ_ಅಂತ್ಯ
 startfunc:
  trap = cast_int(L->hookmask);
 returning:  /* trap already set */
  cl = ci_func(ci);
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  ಆದರೆ (l_unlikely(trap))
    trap = luaG_tracecall(L);
  ತಳ = ci->func.p + 1;
  /* main loop of interpreter */
  ಗಾಗಿ (;;) {
    Instruction i;  /* instruction being executed */
    vmfetch();
    #ಆದರೆ 0
    { /* low-level line tracing for debugging Lua */
      #ಅನ್ತರ್ಭಾವ "lopnames.h"
      ಪೂರ್ಣಾಂಕ pcrel = pcRel(pc, cl->p);
      ಬರೆ("line: %d; %s (%d)\n", luaG_getfuncline(cl->p, pcrel),
             opnames[GET_OPCODE(i)], pcrel);
    }
    #ಆದರೆ_ಅಂತ್ಯ
    lua_assert(ತಳ == ci->func.p + 1);
    lua_assert(ತಳ <= L->ತುದಿ.p && L->ತುದಿ.p <= L->ರಾಶಿ_ಕೊನೆಯ.p);
    /* for tests, invalidate top for instructions not expecting it */
    lua_assert(luaP_isIT(i) || (cast_void(L->ತುದಿ.p = ತಳ), 1));
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
        ಪೂರ್ಣಾಂಕ b = GETARG_sBx(i);
        setfltvalue(s2v(ra), cast_ಅಂಕೆ(b));
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
        ಪೂರ್ಣಾಂಕ b = GETARG_B(i);
        ಮಾಡು {
          setnilvalue(s2v(ra++));
        } ಆಗ (b--);
        vmbreak;
      }
      vmcase(OP_GETUPVAL) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ b = GETARG_B(i);
        setobj2s(L, ra, cl->upvals[b]->v.p);
        vmbreak;
      }
      vmcase(OP_SETUPVAL) {
        StkId ra = RA(i);
        ಮೇಲೆ_Val *uv = cl->upvals[GETARG_B(i)];
        setobj(L, uv->v.p, s2v(ra));
        luaC_barrier(L, uv, s2v(ra));
        vmbreak;
      }
      vmcase(OP_GETTABUP) {
        StkId ra = RA(i);
        TValue *upval = cl->upvals[GETARG_B(i)]->v.p;
        TValue *rc = KC(i);
        TString *ಕೀಲಿ = tsvalue(rc);  /* key must be a short string */
        lu_byte ಟ್ಯಾಗ್;
        luaV_fastget(upval, ಕೀಲಿ, s2v(ra), luaH_getshortstr, ಟ್ಯಾಗ್);
        ಆದರೆ (tagisempty(ಟ್ಯಾಗ್))
          Protect(luaV_finishget(L, upval, rc, ra, ಟ್ಯಾಗ್));
        vmbreak;
      }
      vmcase(OP_GETTABLE) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = vRC(i);
        lu_byte ಟ್ಯಾಗ್;
        ಆದರೆ (ttisinteger(rc)) {  /* fast track for integers? */
          luaV_fastgeti(rb, ivalue(rc), s2v(ra), ಟ್ಯಾಗ್);
        }
        ಇಲ್ಲದಿದ್ದರೆ
          luaV_fastget(rb, rc, s2v(ra), luaH_get, ಟ್ಯಾಗ್);
        ಆದರೆ (tagisempty(ಟ್ಯಾಗ್))
          Protect(luaV_finishget(L, rb, rc, ra, ಟ್ಯಾಗ್));
        vmbreak;
      }
      vmcase(OP_GETI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ಪೂರ್ಣಾಂಕ c = GETARG_C(i);
        lu_byte ಟ್ಯಾಗ್;
        luaV_fastgeti(rb, c, s2v(ra), ಟ್ಯಾಗ್);
        ಆದರೆ (tagisempty(ಟ್ಯಾಗ್)) {
          TValue ಕೀಲಿ;
          setivalue(&ಕೀಲಿ, c);
          Protect(luaV_finishget(L, rb, &ಕೀಲಿ, ra, ಟ್ಯಾಗ್));
        }
        vmbreak;
      }
      vmcase(OP_GETFIELD) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *ಕೀಲಿ = tsvalue(rc);  /* key must be a short string */
        lu_byte ಟ್ಯಾಗ್;
        luaV_fastget(rb, ಕೀಲಿ, s2v(ra), luaH_getshortstr, ಟ್ಯಾಗ್);
        ಆದರೆ (tagisempty(ಟ್ಯಾಗ್))
          Protect(luaV_finishget(L, rb, rc, ra, ಟ್ಯಾಗ್));
        vmbreak;
      }
      vmcase(OP_SETTABUP) {
        ಪೂರ್ಣಾಂಕ hres;
        TValue *upval = cl->upvals[GETARG_A(i)]->v.p;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *ಕೀಲಿ = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(upval, ಕೀಲಿ, rc, hres, luaH_psetshortstr);
        ಆದರೆ (hres == HOK)
          luaV_finishfastset(L, upval, rc);
        ಇಲ್ಲದಿದ್ದರೆ
          Protect(luaV_finishset(L, upval, rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETTABLE) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ hres;
        TValue *rb = vRB(i);  /* key (table is in 'ra') */
        TValue *rc = RKC(i);  /*ಮೌಲ್ಯ*/
        ಆದರೆ (ttisinteger(rb)) {  /* fast track for integers? */
          luaV_fastseti(s2v(ra), ivalue(rb), rc, hres);
        }
        ಇಲ್ಲದಿದ್ದರೆ {
          luaV_fastset(s2v(ra), rb, rc, hres, luaH_pset);
        }
        ಆದರೆ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ಇಲ್ಲದಿದ್ದರೆ
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETI) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ hres;
        ಪೂರ್ಣಾಂಕ b = GETARG_B(i);
        TValue *rc = RKC(i);
        luaV_fastseti(s2v(ra), b, rc, hres);
        ಆದರೆ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ಇಲ್ಲದಿದ್ದರೆ {
          TValue ಕೀಲಿ;
          setivalue(&ಕೀಲಿ, b);
          Protect(luaV_finishset(L, s2v(ra), &ಕೀಲಿ, rc, hres));
        }
        vmbreak;
      }
      vmcase(OP_SETFIELD) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ hres;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *ಕೀಲಿ = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(s2v(ra), ಕೀಲಿ, rc, hres, luaH_psetshortstr);
        ಆದರೆ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ಇಲ್ಲದಿದ್ದರೆ
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_NEWTABLE) {
        StkId ra = RA(i);
        ಚಿಹ್ನೆರಹಿತ b = cast_uint(GETARG_vB(i));  /* log2(hash size) + 1 */
        ಚಿಹ್ನೆರಹಿತ c = cast_uint(GETARG_vC(i));  /* array size */
        ಕೋಷ್ಟಕ *t;
        ಆದರೆ (b > 0)
          b = 1u << (b - 1);  /* hash size is 2^(b - 1) */
        ಆದರೆ (TESTARG_k(i)) {  /* non-zero extra argument? */
          lua_assert(GETARG_Ax(*pc) != 0);
          /* add it to array size */
          c += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
        }
        pc++;  /* skip extra argument */
        L->ತುದಿ.p = ra + 1;  /* correct top in case of emergency GC */
        t = luaH_new(L);  /* memory allocation */
        sethvalue2s(L, ra, t);
        ಆದರೆ (b != 0 || c != 0)
          luaH_resize(L, t, c, b);  /* idem */
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_SELF) {
        StkId ra = RA(i);
        lu_byte ಟ್ಯಾಗ್;
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *ಕೀಲಿ = tsvalue(rc);  /* key must be a short string */
        setobj2s(L, ra + 1, rb);
        luaV_fastget(rb, ಕೀಲಿ, s2v(ra), luaH_getshortstr, ಟ್ಯಾಗ್);
        ಆದರೆ (tagisempty(ಟ್ಯಾಗ್))
          Protect(luaV_finishget(L, rb, rc, ra, ಟ್ಯಾಗ್));
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
        ಪೂರ್ಣಾಂಕ ic = GETARG_sC(i);
        lua_Integer ib;
        ಆದರೆ (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ic, ib));
        }
        vmbreak;
      }
      vmcase(OP_SHRI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ಪೂರ್ಣಾಂಕ ic = GETARG_sC(i);
        lua_Integer ib;
        ಆದರೆ (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ib, -ic));
        }
        vmbreak;
      }
      vmcase(OP_ಕೂಡಿಸು) {
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
        StkId ಫಲಿತಾಂಶ = RA(pi);
        lua_assert(OP_ಕೂಡಿಸು <= GET_OPCODE(pi) && GET_OPCODE(pi) <= OP_SHR);
        Protect(luaT_trybinTM(L, s2v(ra), rb, ಫಲಿತಾಂಶ, tm));
        vmbreak;
      }
      vmcase(OP_MMBINI) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        ಪೂರ್ಣಾಂಕ imm = GETARG_sB(i);
        TMS tm = (TMS)GETARG_C(i);
        ಪೂರ್ಣಾಂಕ flip = GETARG_k(i);
        StkId ಫಲಿತಾಂಶ = RA(pi);
        Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, ಫಲಿತಾಂಶ, tm));
        vmbreak;
      }
      vmcase(OP_MMBINK) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        TValue *imm = KB(i);
        TMS tm = (TMS)GETARG_C(i);
        ಪೂರ್ಣಾಂಕ flip = GETARG_k(i);
        StkId ಫಲಿತಾಂಶ = RA(pi);
        Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, ಫಲಿತಾಂಶ, tm));
        vmbreak;
      }
      vmcase(OP_UNM) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_ಸಂಖ್ಯೆ nb;
        ಆದರೆ (ttisinteger(rb)) {
          lua_Integer ib = ivalue(rb);
          setivalue(s2v(ra), intop(-, 0, ib));
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (tonumberns(rb, nb)) {
          setfltvalue(s2v(ra), luai_numunm(L, nb));
        }
        ಇಲ್ಲದಿದ್ದರೆ
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
        vmbreak;
      }
      vmcase(OP_BNOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_Integer ib;
        ಆದರೆ (tointegerns(rb, &ib)) {
          setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
        }
        ಇಲ್ಲದಿದ್ದರೆ
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
        vmbreak;
      }
      vmcase(OP_NOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ಆದರೆ (l_isfalse(rb))
          setbtvalue(s2v(ra));
        ಇಲ್ಲದಿದ್ದರೆ
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
        ಪೂರ್ಣಾಂಕ n = GETARG_B(i);  /* number of elements to concatenate */
        L->ತುದಿ.p = ra + n;  /* mark the end of concat operands */
        ProtectNT(luaV_concat(L, n));
        checkGC(L, L->ತುದಿ.p); /* 'luaV_concat' ensures correct top */
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
        ಪೂರ್ಣಾಂಕ cond;
        TValue *rb = vRB(i);
        Protect(cond = luaV_equalobj(L, s2v(ra), rb));
        docondjump();
        vmbreak;
      }
      vmcase(OP_LT) {
        op_ಕ್ರಮ(L, l_lti, LTnum, lessthanothers);
        vmbreak;
      }
      vmcase(OP_LE) {
        op_ಕ್ರಮ(L, l_lei, LEnum, lessequalothers);
        vmbreak;
      }
      vmcase(OP_EQK) {
        StkId ra = RA(i);
        TValue *rb = KB(i);
        /* basic types do not use '__eq'; we can use raw equality */
        ಪೂರ್ಣಾಂಕ cond = luaV_rawequalobj(s2v(ra), rb);
        docondjump();
        vmbreak;
      }
      vmcase(OP_EQI) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ cond;
        ಪೂರ್ಣಾಂಕ im = GETARG_sB(i);
        ಆದರೆ (ttisinteger(s2v(ra)))
          cond = (ivalue(s2v(ra)) == im);
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisfloat(s2v(ra)))
          cond = luai_numeq(fltvalue(s2v(ra)), cast_ಅಂಕೆ(im));
        ಇಲ್ಲದಿದ್ದರೆ
          cond = 0;  /* other types cannot be equal to a number */
        docondjump();
        vmbreak;
      }
      vmcase(OP_LTI) {
        op_ಕ್ರಮ_I(L, l_lti, luai_numlt, 0, TM_LT);
        vmbreak;
      }
      vmcase(OP_LEI) {
        op_ಕ್ರಮ_I(L, l_lei, luai_numle, 0, TM_LE);
        vmbreak;
      }
      vmcase(OP_GTI) {
        op_ಕ್ರಮ_I(L, l_gti, luai_numgt, 1, TM_LT);
        vmbreak;
      }
      vmcase(OP_GEI) {
        op_ಕ್ರಮ_I(L, l_gei, luai_numge, 1, TM_LE);
        vmbreak;
      }
      vmcase(OP_TEST) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ cond = !l_isfalse(s2v(ra));
        docondjump();
        vmbreak;
      }
      vmcase(OP_TESTSET) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ಆದರೆ (l_isfalse(rb) == GETARG_k(i))
          pc++;
        ಇಲ್ಲದಿದ್ದರೆ {
          setobj2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(OP_CALL) {
        StkId ra = RA(i);
        CallInfo *newci;
        ಪೂರ್ಣಾಂಕ b = GETARG_B(i);
        ಪೂರ್ಣಾಂಕ nresults = GETARG_C(i) - 1;
        ಆದರೆ (b != 0)  /* fixed number of arguments? */
          L->ತುದಿ.p = ra + b;  /* top signals number of arguments */
        /* else previous instruction set top */
        savepc(ci);  /* in case of errors */
        ಆದರೆ ((newci = luaD_precall(L, ra, nresults)) == ಖಾಲಿ)
          updatetrap(ci);  /* C call; nothing else to be done */
        ಇಲ್ಲದಿದ್ದರೆ {  /* Lua call: run function in this same C frame */
          ci = newci;
          ಹೋಗು startfunc;
        }
        vmbreak;
      }
      vmcase(OP_TAILCALL) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ b = GETARG_B(i);  /* number of arguments + 1 (function) */
        ಪೂರ್ಣಾಂಕ n;  /* number of results when calling a C function */
        ಪೂರ್ಣಾಂಕ nparams1 = GETARG_C(i);
        /* delta is virtual 'func' - real 'func' (vararg functions) */
        ಪೂರ್ಣಾಂಕ delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        ಆದರೆ (b != 0)
          L->ತುದಿ.p = ra + b;
        ಇಲ್ಲದಿದ್ದರೆ  /* previous instruction set top */
          b = cast_int(L->ತುದಿ.p - ra);
        savepc(ci);  /* several calls here can raise errors */
        ಆದರೆ (TESTARG_k(i)) {
          luaF_closeupval(L, ತಳ);  /* close upvalues from current call */
          lua_assert(L->tbclist.p < ತಳ);  /* no pending tbc variables */
          lua_assert(ತಳ == ci->func.p + 1);
        }
        ಆದರೆ ((n = luaD_pretailcall(L, ci, ra, b, delta)) < 0)  /* Lua function? */
          ಹೋಗು startfunc;  /* execute the callee */
        ಇಲ್ಲದಿದ್ದರೆ {  /* C function? */
          ci->func.p -= delta;  /* restore 'func' (if vararg) */
          luaD_poscall(L, ci, n);  /* finish caller */
          updatetrap(ci);  /* 'luaD_poscall' can change hooks */
          ಹೋಗು ret;  /* caller returns after the tail call */
        }
      }
      vmcase(OP_RETURN) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ n = GETARG_B(i) - 1;  /* number of results */
        ಪೂರ್ಣಾಂಕ nparams1 = GETARG_C(i);
        ಆದರೆ (n < 0)  /* not fixed? */
          n = cast_int(L->ತುದಿ.p - ra);  /* get what is available */
        savepc(ci);
        ಆದರೆ (TESTARG_k(i)) {  /* may there be open upvalues? */
          ci->u2.nres = n;  /* save number of returns */
          ಆದರೆ (L->ತುದಿ.p < ci->ತುದಿ.p)
            L->ತುದಿ.p = ci->ತುದಿ.p;
          luaF_close(L, ತಳ, CLOSEKTOP, 1);
          updatetrap(ci);
          updatestack(ci);
        }
        ಆದರೆ (nparams1)  /* vararg function? */
          ci->func.p -= ci->u.l.nextraargs + nparams1;
        L->ತುದಿ.p = ra + n;  /* set call for 'luaD_poscall' */
        luaD_poscall(L, ci, n);
        updatetrap(ci);  /* 'luaD_poscall' can change hooks */
        ಹೋಗು ret;
      }
      vmcase(OP_RETURN0) {
        ಆದರೆ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->ತುದಿ.p = ra;
          savepc(ci);
          luaD_poscall(L, ci, 0);  /* no hurry... */
          trap = 1;
        }
        ಇಲ್ಲದಿದ್ದರೆ {  /* do the 'poscall' here */
          ಪೂರ್ಣಾಂಕ nres = get_nresults(ci->callstatus);
          L->ci = ci->ಹಿಂದಿನ;  /* back to caller */
          L->ತುದಿ.p = ತಳ - 1;
          ಗಾಗಿ (; l_unlikely(nres > 0); nres--)
            setnilvalue(s2v(L->ತುದಿ.p++));  /* all results are nil */
        }
        ಹೋಗು ret;
      }
      vmcase(OP_RETURN1) {
        ಆದರೆ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->ತುದಿ.p = ra + 1;
          savepc(ci);
          luaD_poscall(L, ci, 1);  /* no hurry... */
          trap = 1;
        }
        ಇಲ್ಲದಿದ್ದರೆ {  /* do the 'poscall' here */
          ಪೂರ್ಣಾಂಕ nres = get_nresults(ci->callstatus);
          L->ci = ci->ಹಿಂದಿನ;  /* back to caller */
          ಆದರೆ (nres == 0)
            L->ತುದಿ.p = ತಳ - 1;  /* asked for no results */
          ಇಲ್ಲದಿದ್ದರೆ {
            StkId ra = RA(i);
            setobjs2s(L, ತಳ - 1, ra);  /* at least this result */
            L->ತುದಿ.p = ತಳ;
            ಗಾಗಿ (; l_unlikely(nres > 1); nres--)
              setnilvalue(s2v(L->ತುದಿ.p++));  /* complete missing results */
          }
        }
       ret:  /* return from a Lua function */
        ಆದರೆ (ci->callstatus & CIST_FRESH)
          ಹಿಂತಿರುಗಿ;  /* end this frame */
        ಇಲ್ಲದಿದ್ದರೆ {
          ci = ci->ಹಿಂದಿನ;
          ಹೋಗು returning;  /* continue running caller in this frame */
        }
      }
      vmcase(OP_FORLOOP) {
        StkId ra = RA(i);
        ಆದರೆ (ttisinteger(s2v(ra + 1))) {  /* integer loop? */
          lua_Unsigned ಎಣಿಕೆ_2 = l_castS2U(ivalue(s2v(ra)));
          ಆದರೆ (ಎಣಿಕೆ_2 > 0) {  /* still more iterations? */
            lua_Integer ಹೆಜ್ಜೆ = ivalue(s2v(ra + 1));
            lua_Integer idx = ivalue(s2v(ra + 2));  /* control variable */
            chgivalue(s2v(ra), l_castU2S(ಎಣಿಕೆ_2 - 1));  /* update counter */
            idx = intop(+, idx, ಹೆಜ್ಜೆ);  /* add step to index */
            chgivalue(s2v(ra + 2), idx);  /* update control variable */
            pc -= GETARG_Bx(i);  /* jump back */
          }
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (floatforloop(L, ra))  /* float loop */
          pc -= GETARG_Bx(i);  /* jump back */
        updatetrap(ci);  /* allows a signal to break the loop */
        vmbreak;
      }
      vmcase(OP_FORPREP) {
        StkId ra = RA(i);
        savestate(L, ci);  /* in case of errors */
        ಆದರೆ (forprep(L, ra))
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
       TValue ತಾತ್ಕಾಲಿಕ;  /* to swap control and closing variables */
       setobj(L, &ತಾತ್ಕಾಲಿಕ, s2v(ra + 3));
       setobjs2s(L, ra + 3, ra + 2);
       setobj2s(L, ra + 2, &ತಾತ್ಕಾಲಿಕ);
        /* create to-be-closed upvalue (if closing var. is not nil) */
        halfProtect(luaF_newtbcupval(L, ra + 2));
        pc += GETARG_Bx(i);  /* go to end of the loop */
        i = *(pc++);  /* fetch next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORCALL && ra == RA(i));
        ಹೋಗು l_tforcall;
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
        L->ತುದಿ.p = ra + 3 + 3;
        ProtectNT(luaD_call(L, ra + 3, GETARG_C(i)));  /* do the call */
        updatestack(ci);  /* stack may have changed */
        i = *(pc++);  /* go to next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
        ಹೋಗು l_tforloop;
      }}
      vmcase(OP_TFORLOOP) {
       l_tforloop: {
        StkId ra = RA(i);
        ಆದರೆ (!ttisnil(s2v(ra + 3)))  /* continue loop? */
          pc -= GETARG_Bx(i);  /* jump back */
        vmbreak;
      }}
      vmcase(OP_SETLIST) {
        StkId ra = RA(i);
        ಚಿಹ್ನೆರಹಿತ n = cast_uint(GETARG_vB(i));
        ಚಿಹ್ನೆರಹಿತ ಕೊನೆಯ = cast_uint(GETARG_vC(i));
        ಕೋಷ್ಟಕ *h = hvalue(s2v(ra));
        ಆದರೆ (n == 0)
          n = cast_uint(L->ತುದಿ.p - ra) - 1;  /* get up to the top */
        ಇಲ್ಲದಿದ್ದರೆ
          L->ತುದಿ.p = ci->ತುದಿ.p;  /* correct top in case of emergency GC */
        ಕೊನೆಯ += n;
        ಆದರೆ (TESTARG_k(i)) {
          ಕೊನೆಯ += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
          pc++;
        }
        /* when 'n' is known, table should have proper size */
        ಆದರೆ (ಕೊನೆಯ > h->asize) {  /* needs more space? */
          /* fixed-size sets should have space preallocated */
          lua_assert(GETARG_vB(i) == 0);
          luaH_resizearray(L, h, ಕೊನೆಯ);  /* preallocate it at once */
        }
        ಗಾಗಿ (; n > 0; n--) {
          TValue *val = s2v(ra + n);
          obj2arr(h, ಕೊನೆಯ - 1, val);
          ಕೊನೆಯ--;
          luaC_barrierback(L, obj2gco(h), val);
        }
        vmbreak;
      }
      vmcase(OP_ಆವರಣ) {
        StkId ra = RA(i);
        Proto *p = cl->p->p[GETARG_Bx(i)];
        halfProtect(pushclosure(L, p, cl->upvals, ತಳ, ra));
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_VARARG) {
        StkId ra = RA(i);
        ಪೂರ್ಣಾಂಕ n = GETARG_C(i) - 1;  /* required results (-1 means all) */
        ಪೂರ್ಣಾಂಕ vatab = GETARG_k(i) ? GETARG_B(i) : -1;
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
        ಆದರೆ (!ttisnil(ra))
          halfProtect(luaG_errnnil(L, cl, GETARG_Bx(i)));
        vmbreak;
      }
      vmcase(OP_VARARGPREP) {
        ProtectNT(luaT_adjustvarargs(L, ci, cl->p));
        ಆದರೆ (l_unlikely(trap)) {  /* previous "Protect" updated trap */
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
