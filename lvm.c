/*
** $Id: lvm.c $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#నిర్వచించు lvm_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"

#అన్తర్భావ <float.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <math.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lopcodes.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "ltm.h"
#అన్తర్భావ "lvm.h"


/*
** By default, use jump tables in the main interpreter loop on gcc
** and compatible compilers.
*/
#ఒకవేళ !defined(LUA_USE_JUMPTABLE)
#ఒకవేళ defined(__GNUC__)
#నిర్వచించు LUA_USE_JUMPTABLE	1
#లేకపోతే
#నిర్వచించు LUA_USE_JUMPTABLE	0
#అయితే_ముగింపు
#అయితే_ముగింపు



/* limit for table tag-method chains (to avoid infinite loops) */
#నిర్వచించు MAXTAGLOOP	2000


/*
** 'l_intfitsf' checks whether a given integer is in the range that
** can be converted to a float without rounding. Used in comparisons.
*/

/* number of bits in the mantissa of a float */
#నిర్వచించు NBM		(l_floatatt(MANT_DIG))

/*
** Check whether some integers may not fit in a float, testing whether
** (maxinteger >> NBM) > 0. (That implies (1 << NBM) <= maxinteger.)
** (The shifts are done in parts, to avoid shifting by more than the size
** of an integer. In a worst case, NBM == 113 for long double and
** sizeof(long) == 32.)
*/
#ఒకవేళ ((((LUA_MAXINTEGER >> (NBM / 4)) >> (NBM / 4)) >> (NBM / 4)) \
	>> (NBM - (3 * (NBM / 4))))  >  0

/* limit for integers that fit in a float */
#నిర్వచించు MAXINTFITSF	((lua_Unsigned)1 << NBM)

/* check whether 'i' is in the interval [-MAXINTFITSF, MAXINTFITSF] */
#నిర్వచించు l_intfitsf(i)	((MAXINTFITSF + l_castS2U(i)) <= (2 * MAXINTFITSF))

#లేకపోతే  /* all integers fit in a float precisely */

#నిర్వచించు l_intfitsf(i)	1

#అయితే_ముగింపు


/*
** Try to convert a value from string to a number value.
** If the value is not a string or is a string not representing
** a valid numeral (or if coercions from strings to numbers
** are disabled via macro 'cvt2num'), do not modify 'result'
** and return 0.
*/
స్థిర పూర్ణాంకం l_strton (స్థిరరాశి TValue *obj, TValue *ఫలితం) {
  lua_assert(obj != ఫలితం);
  ఒకవేళ (!cvt2num(obj))  /* is object not a string? */
    తిరిగి 0;
  లేకపోతే {
    TString *st = tsvalue(obj);
    size_t stlen;
    స్థిరరాశి అక్షరం *s = getlstr(st, stlen);
    తిరిగి (luaO_str2num(s, ఫలితం) == stlen + 1);
  }
}


/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'tonumber'.
*/
పూర్ణాంకం luaV_tonumber_ (స్థిరరాశి TValue *obj, lua_సంఖ్య *n) {
  TValue v;
  ఒకవేళ (ttisinteger(obj)) {
    *n = cast_అంకె(ivalue(obj));
    తిరిగి 1;
  }
  లేకపోతే ఒకవేళ (l_strton(obj, &v)) {  /* string coercible to number? */
    *n = nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    తిరిగి 1;
  }
  లేకపోతే
    తిరిగి 0;  /* conversion failed */
}


/*
** try to convert a float to an integer, rounding according to 'mode'.
*/
పూర్ణాంకం luaV_flttointeger (lua_సంఖ్య n, lua_Integer *p, F2Imod బహుళకం) {
  lua_సంఖ్య f = l_floor(n);
  ఒకవేళ (n != f) {  /* not an integral value? */
    ఒకవేళ (బహుళకం == F2Ieq) తిరిగి 0;  /* fails if mode demands integral value */
    లేకపోతే ఒకవేళ (బహుళకం == F2Iceil)  /* needs ceiling? */
      f += 1;  /* convert floor to ceiling (remember: n != f) */
  }
  తిరిగి lua_numbertointeger(f, p);
}


/*
** try to convert a value to an integer, rounding according to 'mode',
** without string coercion.
** ("Fast track" handled by macro 'tointegerns'.)
*/
పూర్ణాంకం luaV_tointegerns (స్థిరరాశి TValue *obj, lua_Integer *p, F2Imod బహుళకం) {
  ఒకవేళ (ttisfloat(obj))
    తిరిగి luaV_flttointeger(fltvalue(obj), p, బహుళకం);
  లేకపోతే ఒకవేళ (ttisinteger(obj)) {
    *p = ivalue(obj);
    తిరిగి 1;
  }
  లేకపోతే
    తిరిగి 0;
}


/*
** try to convert a value to an integer.
*/
పూర్ణాంకం luaV_tointeger (స్థిరరాశి TValue *obj, lua_Integer *p, F2Imod బహుళకం) {
  TValue v;
  ఒకవేళ (l_strton(obj, &v))  /* does 'obj' point to a numerical string? */
    obj = &v;  /* change it to point to its corresponding number */
  తిరిగి luaV_tointegerns(obj, p, బహుళకం);
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
స్థిర పూర్ణాంకం forlimit (lua_State *L, lua_Integer init, స్థిరరాశి TValue *lim,
                                   lua_Integer *p, lua_Integer అడుగు) {
  ఒకవేళ (!luaV_tointeger(lim, p, (అడుగు < 0 ? F2Iceil : F2Ifloor))) {
    /* not coercible to in integer */
    lua_సంఖ్య flim;  /* try to convert to float */
    ఒకవేళ (!tonumber(lim, &flim)) /* cannot convert to float? */
      luaG_forerror(L, lim, "limit");
    /* else 'flim' is a float out of integer bounds */
    ఒకవేళ (luai_numlt(0, flim)) {  /* if it is positive, it is too large */
      ఒకవేళ (అడుగు < 0) తిరిగి 1;  /* initial value must be less than it */
      *p = LUA_MAXINTEGER;  /* truncate */
    }
    లేకపోతే ఒకవేళ (luai_numgt(0, flim)) {  /* if it is negative, it is too small */
      ఒకవేళ (అడుగు > 0) తిరిగి 1;  /* initial value must be greater than it */
      *p = LUA_MININTEGER;  /* truncate */
    }
    లేకపోతే తిరిగి 1;  /* limit is NaN (neither 0 nor positive nor negative) */
  }
  తిరిగి (అడుగు > 0 ? init > *p : init < *p);  /* not to run? */
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
స్థిర పూర్ణాంకం forprep (lua_State *L, StkId ra) {
  TValue *pinit = s2v(ra);
  TValue *plimit = s2v(ra + 1);
  TValue *pstep = s2v(ra + 2);
  ఒకవేళ (ttisinteger(pinit) && ttisinteger(pstep)) { /* integer loop? */
    lua_Integer init = ivalue(pinit);
    lua_Integer అడుగు = ivalue(pstep);
    lua_Integer హద్దు;
    ఒకవేళ (అడుగు == 0)
      luaG_runerror(L, "'for' step is zero");
    ఒకవేళ (forlimit(L, init, plimit, &హద్దు, అడుగు))
      తిరిగి 1;  /* skip the loop */
    లేకపోతే {  /* prepare loop counter */
      lua_Unsigned లెక్క;
      ఒకవేళ (అడుగు > 0) {  /* ascending loop? */
        లెక్క = l_castS2U(హద్దు) - l_castS2U(init);
        ఒకవేళ (అడుగు != 1)  /* avoid division in the too common case */
          లెక్క /= l_castS2U(అడుగు);
      }
      లేకపోతే {  /* step < 0; descending loop */
        లెక్క = l_castS2U(init) - l_castS2U(హద్దు);
        /* 'step+1' avoids negating 'mininteger' */
        లెక్క /= l_castS2U(-(అడుగు + 1)) + 1u;
      }
      /* use 'chgivalue' for places that for sure had integers */
      chgivalue(s2v(ra), l_castU2S(లెక్క));  /* change init to count */
      setivalue(s2v(ra + 1), అడుగు);  /* change limit to step */
      chgivalue(s2v(ra + 2), init);  /* change step to init */
    }
  }
  లేకపోతే {  /* try making all values floats */
    lua_సంఖ్య init; lua_సంఖ్య హద్దు; lua_సంఖ్య అడుగు;
    ఒకవేళ (l_unlikely(!tonumber(plimit, &హద్దు)))
      luaG_forerror(L, plimit, "limit");
    ఒకవేళ (l_unlikely(!tonumber(pstep, &అడుగు)))
      luaG_forerror(L, pstep, "step");
    ఒకవేళ (l_unlikely(!tonumber(pinit, &init)))
      luaG_forerror(L, pinit, "initial value");
    ఒకవేళ (అడుగు == 0)
      luaG_runerror(L, "'for' step is zero");
    ఒకవేళ (!(luai_numge(0, అడుగు) ? luai_numge(init, హద్దు)
                              : luai_numge(హద్దు, init)))
      తిరిగి 1;  /* skip the loop */
    లేకపోతే {
      /* make sure all values are floats */
      setfltvalue(s2v(ra), హద్దు);
      setfltvalue(s2v(ra + 1), అడుగు);
      setfltvalue(s2v(ra + 2), init);  /* control variable */
    }
  }
  తిరిగి 0;
}


/*
** Execute a step of a float numerical for loop, returning
** true iff the loop must continue. (The integer case is
** written inline with opcode OP_FORLOOP, for performance.)
*/
స్థిర పూర్ణాంకం floatforloop (lua_State *L, StkId ra) {
  lua_సంఖ్య అడుగు = fltvalue(s2v(ra + 1));
  lua_సంఖ్య హద్దు = fltvalue(s2v(ra));
  lua_సంఖ్య idx = fltvalue(s2v(ra + 2));  /* control variable */
  idx = luai_numadd(L, idx, అడుగు);  /* increment index */
  ఒకవేళ (luai_numlt(0, అడుగు) ? luai_numle(idx, హద్దు)
                          : luai_numle(హద్దు, idx)) {
    chgfltvalue(s2v(ra + 2), idx);  /* update control variable */
    తిరిగి 1;  /* jump back */
  }
  లేకపోతే
    తిరిగి 0;  /* finish the loop */
}


/*
** Finish the table access 'val = t[key]' and return the tag of the result.
*/
lu_byte luaV_finishget (lua_State *L, స్థిరరాశి TValue *t, TValue *కీలకం,
                                      StkId val, lu_byte ట్యాగ్) {
  పూర్ణాంకం loop;  /* counter to avoid infinite loops */
  స్థిరరాశి TValue *tm;  /* metamethod */
  కొరకు (loop = 0; loop < MAXTAGLOOP; loop++) {
    ఒకవేళ (ట్యాగ్ == LUA_VNOTABLE) {  /* 't' is not a table? */
      lua_assert(!ttistable(t));
      tm = luaT_gettmbyobj(L, t, TM_సూచీ);
      ఒకవేళ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    లేకపోతే {  /* 't' is a table */
      tm = fasttm(L, hvalue(t)->metatable, TM_సూచీ);  /* table's metamethod */
      ఒకవేళ (tm == ఖాళీ) {  /* no metamethod? */
        setnilvalue2s(val);  /* result is nil */
        తిరిగి LUA_VNIL;
      }
      /* else will try the metamethod */
    }
    ఒకవేళ (ttisfunction(tm)) {  /* is metamethod a function? */
      ట్యాగ్ = luaT_callTMres(L, tm, t, కీలకం, val);  /* call it */
      తిరిగి ట్యాగ్;  /* return tag of the result */
    }
    t = tm;  /* else try to access 'tm[key]' */
    luaV_fastget(t, కీలకం, s2v(val), luaH_get, ట్యాగ్);
    ఒకవేళ (!tagisempty(ట్యాగ్))
      తిరిగి ట్యాగ్;  /*పూర్తి*/
    /* else repeat (tail call 'luaV_finishget') */
  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
  తిరిగి 0;  /* to avoid warnings */
}


/*
** Finish a table assignment 't[key] = val'.
** About anchoring the table before the call to 'luaH_finishset':
** This call may trigger an emergency collection. When loop>0,
** the table being accessed is a field in some metatable. If this
** metatable is weak and the table is not anchored, this collection
** could collect that table while it is being updated.
*/
శూన్య luaV_finishset (lua_State *L, స్థిరరాశి TValue *t, TValue *కీలకం,
                      TValue *val, పూర్ణాంకం hres) {
  పూర్ణాంకం loop;  /* counter to avoid infinite loops */
  కొరకు (loop = 0; loop < MAXTAGLOOP; loop++) {
    స్థిరరాశి TValue *tm;  /* '__newindex' metamethod */
    ఒకవేళ (hres != HNOTATABLE) {  /* is 't' a table? */
      పట్టిక *h = hvalue(t);  /* save 't' table */
      tm = fasttm(L, h->metatable, TM_NEWINDEX);  /* get metamethod */
      ఒకవేళ (tm == ఖాళీ) {  /* no metamethod? */
        sethvalue2s(L, L->శిఖరం.p, h);  /* anchor 't' */
        L->శిఖరం.p++;  /* assume EXTRA_STACK */
        luaH_finishset(L, h, కీలకం, val, hres);  /* set new value */
        L->శిఖరం.p--;
        invalidateTMcache(h);
        luaC_barrierback(L, obj2gco(h), val);
        తిరిగి;
      }
      /* else will try the metamethod */
    }
    లేకపోతే {  /* not a table; check metamethod */
      tm = luaT_gettmbyobj(L, t, TM_NEWINDEX);
      ఒకవేళ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");
    }
    /* try the metamethod */
    ఒకవేళ (ttisfunction(tm)) {
      luaT_callTM(L, tm, t, కీలకం, val);
      తిరిగి;
    }
    t = tm;  /* else must repeat assignment over 'tm' */
    /* do the equivalent to 'luaV_fastset', but saving 'h' */
    ఒకవేళ (!ttistable(t))
      hres = HNOTATABLE;
    లేకపోతే {
      పట్టిక *h = hvalue(t);  /* next call can change the value at 't' */
      hres = luaH_pset(h, కీలకం, val);
      ఒకవేళ (hres == HOK) {
        luaC_barrierback(L, obj2gco(h), val);  /* luaV_finishfastset */
        తిరిగి;  /*పూర్తి*/
      }
    }
    /* else 'return luaV_finishset(L, t, key, val, hres)' (loop) */
  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}


/*
** Function to be used for 0-terminated string order comparison
*/
#ఒకవేళ !defined(l_strcoll)
#నిర్వచించు l_strcoll	strcoll
#అయితే_ముగింపు


/*
** Compare two strings 'ts1' x 'ts2', returning an integer less-equal-
** -greater than zero if 'ts1' is less-equal-greater than 'ts2'.
** The code is a little tricky because it allows '\0' in the strings
** and it uses 'strcoll' (to respect locales) for each segment
** of the strings. Note that segments can compare equal but still
** have different lengths.
*/
స్థిర పూర్ణాంకం l_strcmp (స్థిరరాశి TString *ts1, స్థిరరాశి TString *ts2) {
  size_t rl1;  /* real length */
  స్థిరరాశి అక్షరం *s1 = getlstr(ts1, rl1);
  size_t rl2;
  స్థిరరాశి అక్షరం *s2 = getlstr(ts2, rl2);
  కొరకు (;;) {  /* for each segment */
    పూర్ణాంకం తాత్కాలిక = l_strcoll(s1, s2);
    ఒకవేళ (తాత్కాలిక != 0)  /* not equal? */
      తిరిగి తాత్కాలిక;  /*పూర్తి*/
    లేకపోతే {  /* strings are equal up to a '\0' */
      size_t zl1 = అక్షరశ్రేణి_పొడవు(s1);  /* index of first '\0' in 's1' */
      size_t zl2 = అక్షరశ్రేణి_పొడవు(s2);  /* index of first '\0' in 's2' */
      ఒకవేళ (zl2 == rl2)  /* 's2' is finished? */
        తిరిగి (zl1 == rl1) ? 0 : 1;  /* check 's1' */
      లేకపోతే ఒకవేళ (zl1 == rl1)  /* 's1' is finished? */
        తిరిగి -1;  /* 's1' is less than 's2' ('s2' is not finished) */
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
l_sinline పూర్ణాంకం LTintfloat (lua_Integer i, lua_సంఖ్య f) {
  ఒకవేళ (l_intfitsf(i))
    తిరిగి luai_numlt(cast_అంకె(i), f);  /* compare them as floats */
  లేకపోతే {  /* i < f <=> i < ceil(f) */
    lua_Integer fi;
    ఒకవేళ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      తిరిగి i < fi;   /* compare them as integers */
    లేకపోతే  /* 'f' is either greater or less than all integers */
      తిరిగి f > 0;  /* greater? */
  }
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
l_sinline పూర్ణాంకం LEintfloat (lua_Integer i, lua_సంఖ్య f) {
  ఒకవేళ (l_intfitsf(i))
    తిరిగి luai_numle(cast_అంకె(i), f);  /* compare them as floats */
  లేకపోతే {  /* i <= f <=> i <= floor(f) */
    lua_Integer fi;
    ఒకవేళ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      తిరిగి i <= fi;   /* compare them as integers */
    లేకపోతే  /* 'f' is either greater or less than all integers */
      తిరిగి f > 0;  /* greater? */
  }
}


/*
** Check whether float 'f' is less than integer 'i'.
** See comments on previous function.
*/
l_sinline పూర్ణాంకం LTfloatint (lua_సంఖ్య f, lua_Integer i) {
  ఒకవేళ (l_intfitsf(i))
    తిరిగి luai_numlt(f, cast_అంకె(i));  /* compare them as floats */
  లేకపోతే {  /* f < i <=> floor(f) < i */
    lua_Integer fi;
    ఒకవేళ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      తిరిగి fi < i;   /* compare them as integers */
    లేకపోతే  /* 'f' is either greater or less than all integers */
      తిరిగి f < 0;  /* less? */
  }
}


/*
** Check whether float 'f' is less than or equal to integer 'i'.
** See comments on previous function.
*/
l_sinline పూర్ణాంకం LEfloatint (lua_సంఖ్య f, lua_Integer i) {
  ఒకవేళ (l_intfitsf(i))
    తిరిగి luai_numle(f, cast_అంకె(i));  /* compare them as floats */
  లేకపోతే {  /* f <= i <=> ceil(f) <= i */
    lua_Integer fi;
    ఒకవేళ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      తిరిగి fi <= i;   /* compare them as integers */
    లేకపోతే  /* 'f' is either greater or less than all integers */
      తిరిగి f < 0;  /* less? */
  }
}


/*
** Return 'l < r', for numbers.
*/
l_sinline పూర్ణాంకం LTnum (స్థిరరాశి TValue *l, స్థిరరాశి TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ఒకవేళ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ఒకవేళ (ttisinteger(r))
      తిరిగి li < ivalue(r);  /* both are integers */
    లేకపోతే  /* 'l' is int and 'r' is float */
      తిరిగి LTintfloat(li, fltvalue(r));  /* l < r ? */
  }
  లేకపోతే {
    lua_సంఖ్య lf = fltvalue(l);  /* 'l' must be float */
    ఒకవేళ (ttisfloat(r))
      తిరిగి luai_numlt(lf, fltvalue(r));  /* both are float */
    లేకపోతే  /* 'l' is float and 'r' is int */
      తిరిగి LTfloatint(lf, ivalue(r));
  }
}


/*
** Return 'l <= r', for numbers.
*/
l_sinline పూర్ణాంకం LEnum (స్థిరరాశి TValue *l, స్థిరరాశి TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ఒకవేళ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ఒకవేళ (ttisinteger(r))
      తిరిగి li <= ivalue(r);  /* both are integers */
    లేకపోతే  /* 'l' is int and 'r' is float */
      తిరిగి LEintfloat(li, fltvalue(r));  /* l <= r ? */
  }
  లేకపోతే {
    lua_సంఖ్య lf = fltvalue(l);  /* 'l' must be float */
    ఒకవేళ (ttisfloat(r))
      తిరిగి luai_numle(lf, fltvalue(r));  /* both are float */
    లేకపోతే  /* 'l' is float and 'r' is int */
      తిరిగి LEfloatint(lf, ivalue(r));
  }
}


/*
** return 'l < r' for non-numbers.
*/
స్థిర పూర్ణాంకం lessthanothers (lua_State *L, స్థిరరాశి TValue *l, స్థిరరాశి TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ఒకవేళ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    తిరిగి l_strcmp(tsvalue(l), tsvalue(r)) < 0;
  లేకపోతే
    తిరిగి luaT_callorderTM(L, l, r, TM_LT);
}


/*
** Main operation less than; return 'l < r'.
*/
పూర్ణాంకం luaV_lessthan (lua_State *L, స్థిరరాశి TValue *l, స్థిరరాశి TValue *r) {
  ఒకవేళ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    తిరిగి LTnum(l, r);
  లేకపోతే తిరిగి lessthanothers(L, l, r);
}


/*
** return 'l <= r' for non-numbers.
*/
స్థిర పూర్ణాంకం lessequalothers (lua_State *L, స్థిరరాశి TValue *l, స్థిరరాశి TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ఒకవేళ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    తిరిగి l_strcmp(tsvalue(l), tsvalue(r)) <= 0;
  లేకపోతే
    తిరిగి luaT_callorderTM(L, l, r, TM_LE);
}


/*
** Main operation less than or equal to; return 'l <= r'.
*/
పూర్ణాంకం luaV_lessequal (lua_State *L, స్థిరరాశి TValue *l, స్థిరరాశి TValue *r) {
  ఒకవేళ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    తిరిగి LEnum(l, r);
  లేకపోతే తిరిగి lessequalothers(L, l, r);
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
పూర్ణాంకం luaV_equalobj (lua_State *L, స్థిరరాశి TValue *t1, స్థిరరాశి TValue *t2) {
  స్థిరరాశి TValue *tm;
  ఒకవేళ (ttype(t1) != ttype(t2))  /* not the same type? */
    తిరిగి 0;
  లేకపోతే ఒకవేళ (ttypetag(t1) != ttypetag(t2)) {
    ఎంపిక (ttypetag(t1)) {
      స్థితి LUA_VNUMINT: {  /* integer == float? */
        /* integer and float can only be equal if float has an integer
           value equal to the integer */
        lua_Integer i2;
        తిరిగి (luaV_flttointeger(fltvalue(t2), &i2, F2Ieq) &&
                ivalue(t1) == i2);
      }
      స్థితి LUA_VNUMFLT: {  /* float == integer? */
        lua_Integer i1;  /* see comment in previous case */
        తిరిగి (luaV_flttointeger(fltvalue(t1), &i1, F2Ieq) &&
                i1 == ivalue(t2));
      }
      స్థితి LUA_VSHRSTR: స్థితి LUA_VLNGSTR: {
        /* compare two strings with different variants: they can be
           equal when one string is a short string and the other is
           an external string  */
        తిరిగి luaS_eqstr(tsvalue(t1), tsvalue(t2));
      }
      అప్రమేయ:
        /* only numbers (integer/float) and strings (long/short) can have
           equal values with different variants */
        తిరిగి 0;
    }
  }
  లేకపోతే {  /* equal variants */
    ఎంపిక (ttypetag(t1)) {
      స్థితి LUA_VNIL: స్థితి LUA_VFALSE: స్థితి LUA_VTRUE:
        తిరిగి 1;
      స్థితి LUA_VNUMINT:
        తిరిగి (ivalue(t1) == ivalue(t2));
      స్థితి LUA_VNUMFLT:
        తిరిగి (fltvalue(t1) == fltvalue(t2));
      స్థితి LUA_VLIGHTUSERDATA: తిరిగి pvalue(t1) == pvalue(t2);
      స్థితి LUA_VSHRSTR:
        తిరిగి eqshrstr(tsvalue(t1), tsvalue(t2));
      స్థితి LUA_VLNGSTR:
        తిరిగి luaS_eqstr(tsvalue(t1), tsvalue(t2));
      స్థితి LUA_VUSERDATA: {
        ఒకవేళ (uvalue(t1) == uvalue(t2)) తిరిగి 1;
        లేకపోతే ఒకవేళ (L == ఖాళీ) తిరిగి 0;
        tm = fasttm(L, uvalue(t1)->metatable, TM_EQ);
        ఒకవేళ (tm == ఖాళీ)
          tm = fasttm(L, uvalue(t2)->metatable, TM_EQ);
        ఆపు;  /* will try TM */
      }
      స్థితి LUA_VTABLE: {
        ఒకవేళ (hvalue(t1) == hvalue(t2)) తిరిగి 1;
        లేకపోతే ఒకవేళ (L == ఖాళీ) తిరిగి 0;
        tm = fasttm(L, hvalue(t1)->metatable, TM_EQ);
        ఒకవేళ (tm == ఖాళీ)
          tm = fasttm(L, hvalue(t2)->metatable, TM_EQ);
        ఆపు;  /* will try TM */
      }
      స్థితి LUA_VLCF:
        తిరిగి (fvalue(t1) == fvalue(t2));
      అప్రమేయ:  /* functions and threads */
        తిరిగి (gcvalue(t1) == gcvalue(t2));
    }
    ఒకవేళ (tm == ఖాళీ)  /* no TM? */
      తిరిగి 0;  /* objects are different */
    లేకపోతే {
      పూర్ణాంకం ట్యాగ్ = luaT_callTMres(L, tm, t1, t2, L->శిఖరం.p);  /* call TM */
      తిరిగి !tagisfalse(ట్యాగ్);
    }
  }
}


/* macro used by 'luaV_concat' to ensure that element at 'o' is a string */
#నిర్వచించు tostring(L,o)  \
	(ttisstring(o) || (cvt2str(o) && (luaO_tostring(L, o), 1)))

/*
** Check whether object is a short empty string to optimize concatenation.
** (External strings can be empty too; they will be concatenated like
** non-empty ones.)
*/
#నిర్వచించు isemptystr(o)	(ttisshrstring(o) && tsvalue(o)->shrlen == 0)

/* copy strings in stack from top - n up to top - 1 to buffer */
స్థిర శూన్య copy2buff (StkId శిఖరం, పూర్ణాంకం n, అక్షరం *buff) {
  size_t tl = 0;  /* size already copied */
  చేయి {
    TString *st = tsvalue(s2v(శిఖరం - n));
    size_t l;  /* length of string being copied */
    స్థిరరాశి అక్షరం *s = getlstr(st, l);
    జ్ఞాపకం_నకలు(buff + tl, s, l * పరిమాణం(అక్షరం));
    tl += l;
  } అయితే (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top.p - total' up to 'L->top.p - 1'.
*/
శూన్య luaV_concat (lua_State *L, పూర్ణాంకం మొత్తం) {
  ఒకవేళ (మొత్తం == 1)
    తిరిగి;  /* "all" values already concatenated */
  చేయి {
    StkId శిఖరం = L->శిఖరం.p;
    పూర్ణాంకం n = 2;  /* number of elements handled in this pass (at least 2) */
    ఒకవేళ (!(ttisstring(s2v(శిఖరం - 2)) || cvt2str(s2v(శిఖరం - 2))) ||
        !tostring(L, s2v(శిఖరం - 1)))
      luaT_tryconcatTM(L);  /* may invalidate 'top' */
    లేకపోతే ఒకవేళ (isemptystr(s2v(శిఖరం - 1)))  /* second operand is empty? */
      cast_void(tostring(L, s2v(శిఖరం - 2)));  /* result is first operand */
    లేకపోతే ఒకవేళ (isemptystr(s2v(శిఖరం - 2))) {  /* first operand is empty string? */
      setobjs2s(L, శిఖరం - 2, శిఖరం - 1);  /* result is second op. */
    }
    లేకపోతే {
      /* at least two string values; get as many as possible */
      size_t tl = tsslen(tsvalue(s2v(శిఖరం - 1)));  /* total length */
      TString *ts;
      /* collect total length and number of strings */
      కొరకు (n = 1; n < మొత్తం && tostring(L, s2v(శిఖరం - n - 1)); n++) {
        size_t l = tsslen(tsvalue(s2v(శిఖరం - n - 1)));
        ఒకవేళ (l_unlikely(l >= MAX_కొలత - పరిమాణం(TString) - tl)) {
          L->శిఖరం.p = శిఖరం - మొత్తం;  /* pop strings to avoid wasting stack */
          luaG_runerror(L, "string length overflow");
        }
        tl += l;
      }
      ఒకవేళ (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        అక్షరం buff[LUAI_MAXSHORTLEN];
        copy2buff(శిఖరం, n, buff);  /* copy strings to buffer */
        ts = luaS_newlstr(L, buff, tl);
      }
      లేకపోతే {  /* long string; copy strings directly to final result */
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(శిఖరం, n, getlngstr(ts));
      }
      setsvalue2s(L, శిఖరం - n, ts);  /* create result */
    }
    మొత్తం -= n - 1;  /* got 'n' strings to create one new */
    L->శిఖరం.p -= n - 1;  /* popped 'n' strings and pushed one */
  } అయితే (మొత్తం > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra = #rb'.
*/
శూన్య luaV_objlen (lua_State *L, StkId ra, స్థిరరాశి TValue *rb) {
  స్థిరరాశి TValue *tm;
  ఎంపిక (ttypetag(rb)) {
    స్థితి LUA_VTABLE: {
      పట్టిక *h = hvalue(rb);
      tm = fasttm(L, h->metatable, TM_LEN);
      ఒకవేళ (tm) ఆపు;  /* metamethod? break switch to call it */
      setivalue(s2v(ra), l_castU2S(luaH_getn(L, h)));  /* else primitive len */
      తిరిగి;
    }
    స్థితి LUA_VSHRSTR: {
      setivalue(s2v(ra), tsvalue(rb)->shrlen);
      తిరిగి;
    }
    స్థితి LUA_VLNGSTR: {
      setivalue(s2v(ra), cast_st2S(tsvalue(rb)->u.lnglen));
      తిరిగి;
    }
    అప్రమేయ: {  /* try metamethod */
      tm = luaT_gettmbyobj(L, rb, TM_LEN);
      ఒకవేళ (l_unlikely(notm(tm)))  /* no metamethod? */
        luaG_typeerror(L, rb, "get length of");
      ఆపు;
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
  ఒకవేళ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ఒకవేళ (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    తిరిగి intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  లేకపోతే {
    lua_Integer q = m / n;  /* perform C division */
    ఒకవేళ ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    తిరిగి q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_idiv.)
*/
lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  ఒకవేళ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ఒకవేళ (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    తిరిగి 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  లేకపోతే {
    lua_Integer r = m % n;
    ఒకవేళ (r != 0 && (r ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    తిరిగి r;
  }
}


/*
** Float modulus
*/
lua_సంఖ్య luaV_modf (lua_State *L, lua_సంఖ్య m, lua_సంఖ్య n) {
  lua_సంఖ్య r;
  luai_nummod(L, m, n, r);
  తిరిగి r;
}


/* number of bits in an integer */
#నిర్వచించు NBITS	l_numbits(lua_Integer)


/*
** Shift left operation. (Shift right just negates 'y'.)
*/
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  ఒకవేళ (y < 0) {  /* shift right? */
    ఒకవేళ (y <= -NBITS) తిరిగి 0;
    లేకపోతే తిరిగి intop(>>, x, -y);
  }
  లేకపోతే {  /* shift left */
    ఒకవేళ (y >= NBITS) తిరిగి 0;
    లేకపోతే తిరిగి intop(<<, x, y);
  }
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues.
*/
స్థిర శూన్య pushclosure (lua_State *L, Proto *p, పైకి_Val **encup, StkId పునాది,
                         StkId ra) {
  పూర్ణాంకం nup = p->sizeupvalues;
  Upvaldesc *uv = p->upvalues;
  పూర్ణాంకం i;
  LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  setclLvalue2s(L, ra, ncl);  /* anchor new closure in stack */
  కొరకు (i = 0; i < nup; i++) {  /* fill in its upvalues */
    ఒకవేళ (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = luaF_findupval(L, పునాది + uv[i].idx);
    లేకపోతే  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    luaC_objbarrier(L, ncl, ncl->upvals[i]);
  }
}


/*
** finish execution of an opcode interrupted by a yield
*/
శూన్య lua_V_ముగింపు_Op (lua_State *L) {
  CallInfo *ci = L->ci;
  StkId పునాది = ci->func.p + 1;
  Instruction inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  Op_సంకేతం op = GET_OPCODE(inst);
  ఎంపిక (op) {  /* finish its execution */
    స్థితి OP_MMBIN: స్థితి OP_MMBINI: స్థితి OP_MMBINK: {
      setobjs2s(L, పునాది + GETARG_A(*(ci->u.l.savedpc - 2)), --L->శిఖరం.p);
      ఆపు;
    }
    స్థితి OP_UNM: స్థితి OP_BNOT: స్థితి OP_LEN:
    స్థితి OP_GETTABUP: స్థితి OP_GETTABLE: స్థితి OP_GETI:
    స్థితి OP_GETFIELD: స్థితి OP_SELF: {
      setobjs2s(L, పునాది + GETARG_A(inst), --L->శిఖరం.p);
      ఆపు;
    }
    స్థితి OP_LT: స్థితి OP_LE:
    స్థితి OP_LTI: స్థితి OP_LEI:
    స్థితి OP_GTI: స్థితి OP_GEI:
    స్థితి OP_EQ: {  /* note that 'OP_EQI'/'OP_EQK' cannot yield */
      పూర్ణాంకం res = !l_isfalse(s2v(L->శిఖరం.p - 1));
      L->శిఖరం.p--;
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_JMP);
      ఒకవేళ (res != GETARG_k(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      ఆపు;
    }
    స్థితి OP_CONCAT: {
      StkId శిఖరం = L->శిఖరం.p - 1;  /* top when 'luaT_tryconcatTM' was called */
      పూర్ణాంకం a = GETARG_A(inst);      /* first element to concatenate */
      పూర్ణాంకం మొత్తం = cast_int(శిఖరం - 1 - (పునాది + a));  /* yet to concatenate */
      setobjs2s(L, శిఖరం - 2, శిఖరం);  /* put TM result in proper position */
      L->శిఖరం.p = శిఖరం - 1;  /* top is one after last element (at top-2) */
      luaV_concat(L, మొత్తం);  /* concat them (may yield again) */
      ఆపు;
    }
    స్థితి OP_CLOSE: {  /* yielded closing variables */
      ci->u.l.savedpc--;  /* repeat instruction to close other vars. */
      ఆపు;
    }
    స్థితి OP_RETURN: {  /* yielded closing variables */
      StkId ra = పునాది + GETARG_A(inst);
      /* adjust top to signal correct number of returns, in case the
         return is "up to top" ('isIT') */
      L->శిఖరం.p = ra + ci->u2.nres;
      /* repeat instruction to close other vars. and complete the return */
      ci->u.l.savedpc--;
      ఆపు;
    }
    అప్రమేయ: {
      /* only these other opcodes can yield */
      lua_assert(op == OP_TFORCALL || op == OP_CALL ||
           op == OP_TAILCALL || op == OP_SETTABUP || op == OP_SETTABLE ||
           op == OP_SETI || op == OP_SETFIELD);
      ఆపు;
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

#నిర్వచించు l_addi(L,a,b)	intop(+, a, b)
#నిర్వచించు l_subi(L,a,b)	intop(-, a, b)
#నిర్వచించు l_muli(L,a,b)	intop(*, a, b)
#నిర్వచించు l_band(a,b)	intop(&, a, b)
#నిర్వచించు l_bor(a,b)	intop(|, a, b)
#నిర్వచించు l_bxor(a,b)	intop(^, a, b)

#నిర్వచించు l_lti(a,b)	(a < b)
#నిర్వచించు l_lei(a,b)	(a <= b)
#నిర్వచించు l_gti(a,b)	(a > b)
#నిర్వచించు l_gei(a,b)	(a >= b)


/*
** Arithmetic operations with immediate operands. 'iop' is the integer
** operation, 'fop' is the float operation.
*/
#నిర్వచించు op_arithI(L,iop,fop) {  \
  TValue *ra = vRA(i); \
  TValue *v1 = vRB(i);  \
  పూర్ణాంకం imm = GETARG_sC(i);  \
  ఒకవేళ (ttisinteger(v1)) {  \
    lua_Integer iv1 = ivalue(v1);  \
    pc++; setivalue(ra, iop(L, iv1, imm));  \
  }  \
  లేకపోతే ఒకవేళ (ttisfloat(v1)) {  \
    lua_సంఖ్య nb = fltvalue(v1);  \
    lua_సంఖ్య fimm = cast_అంకె(imm);  \
    pc++; setfltvalue(ra, fop(L, nb, fimm)); \
  }}


/*
** Auxiliary function for arithmetic operations over floats and others
** with two operands.
*/
#నిర్వచించు op_arithf_aux(L,v1,v2,fop) {  \
  lua_సంఖ్య n1; lua_సంఖ్య n2;  \
  ఒకవేళ (tonumberns(v1, n1) && tonumberns(v2, n2)) {  \
    StkId ra = RA(i);  \
    pc++; setfltvalue(s2v(ra), fop(L, n1, n2));  \
  }}


/*
** Arithmetic operations over floats and others with register operands.
*/
#నిర్వచించు op_arithf(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with K operands for floats.
*/
#నిర్వచించు op_arithfK(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations over integers and floats.
*/
#నిర్వచించు op_arith_aux(L,v1,v2,iop,fop) {  \
  ఒకవేళ (ttisinteger(v1) && ttisinteger(v2)) {  \
    StkId ra = RA(i); \
    lua_Integer i1 = ivalue(v1); lua_Integer i2 = ivalue(v2);  \
    pc++; setivalue(s2v(ra), iop(L, i1, i2));  \
  }  \
  లేకపోతే op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with register operands.
*/
#నిర్వచించు op_arith(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Arithmetic operations with K operands.
*/
#నిర్వచించు op_arithK(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Bitwise operations with constant operand.
*/
#నిర్వచించు op_bitwiseK(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i);  \
  lua_Integer i1;  \
  lua_Integer i2 = ivalue(v2);  \
  ఒకవేళ (tointegerns(v1, &i1)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Bitwise operations with register operands.
*/
#నిర్వచించు op_bitwise(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  lua_Integer i1; lua_Integer i2;  \
  ఒకవేళ (tointegerns(v1, &i1) && tointegerns(v2, &i2)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Order operations with register operands. 'opn' actually works
** for all numbers, but the fast track improves performance for
** integers.
*/
#నిర్వచించు op_క్రమం(L,opi,opn,other) {  \
  TValue *ra = vRA(i); \
  పూర్ణాంకం cond;  \
  TValue *rb = vRB(i);  \
  ఒకవేళ (ttisinteger(ra) && ttisinteger(rb)) {  \
    lua_Integer ia = ivalue(ra);  \
    lua_Integer ib = ivalue(rb);  \
    cond = opi(ia, ib);  \
  }  \
  లేకపోతే ఒకవేళ (ttisnumber(ra) && ttisnumber(rb))  \
    cond = opn(ra, rb);  \
  లేకపోతే  \
    Protect(cond = other(L, ra, rb));  \
  docondjump(); }


/*
** Order operations with immediate operand. (Immediate operand is
** always small enough to have an exact representation as a float.)
*/
#నిర్వచించు op_క్రమం_I(L,opi,opf,inv,tm) {  \
  TValue *ra = vRA(i); \
  పూర్ణాంకం cond;  \
  పూర్ణాంకం im = GETARG_sB(i);  \
  ఒకవేళ (ttisinteger(ra))  \
    cond = opi(ivalue(ra), im);  \
  లేకపోతే ఒకవేళ (ttisfloat(ra)) {  \
    lua_సంఖ్య fa = fltvalue(ra);  \
    lua_సంఖ్య fim = cast_అంకె(im);  \
    cond = opf(fa, fim);  \
  }  \
  లేకపోతే {  \
    పూర్ణాంకం isf = GETARG_C(i);  \
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


#నిర్వచించు RA(i)	(పునాది+GETARG_A(i))
#నిర్వచించు vRA(i)	s2v(RA(i))
#నిర్వచించు RB(i)	(పునాది+GETARG_B(i))
#నిర్వచించు vRB(i)	s2v(RB(i))
#నిర్వచించు KB(i)	(k+GETARG_B(i))
#నిర్వచించు RC(i)	(పునాది+GETARG_C(i))
#నిర్వచించు vRC(i)	s2v(RC(i))
#నిర్వచించు KC(i)	(k+GETARG_C(i))
#నిర్వచించు RKC(i)	((TESTARG_k(i)) ? k + GETARG_C(i) : s2v(పునాది + GETARG_C(i)))



#నిర్వచించు updatetrap(ci)  (trap = cast_int(ci->u.l.trap))

#నిర్వచించు updatebase(ci)	(పునాది = ci->func.p + 1)


#నిర్వచించు updatestack(ci)  \
	{ ఒకవేళ (l_unlikely(trap)) { updatebase(ci); ra = RA(i); } }


/*
** Execute a jump instruction. The 'updatetrap' allows signals to stop
** tight loops. (Without it, the local copy of 'trap' could never change.)
*/
#నిర్వచించు dojump(ci,i,e)	{ pc += GETARG_sJ(i) + e; updatetrap(ci); }


/* for test instructions, execute the jump instruction that follows it */
#నిర్వచించు donextjump(ci)	{ Instruction ni = *pc; dojump(ci, ni, 1); }

/*
** do a conditional jump: skip next instruction if 'cond' is not what
** was expected (parameter 'k'), else do next instruction, which must
** be a jump.
*/
#నిర్వచించు docondjump()	ఒకవేళ (cond != GETARG_k(i)) pc++; లేకపోతే donextjump(ci);


/*
** Correct global 'pc'.
*/
#నిర్వచించు savepc(ci)	(ci->u.l.savedpc = pc)


/*
** Whenever code can raise errors, the global 'pc' and the global
** 'top' must be correct to report occasional errors.
*/
#నిర్వచించు savestate(L,ci)		(savepc(ci), L->శిఖరం.p = ci->శిఖరం.p)


/*
** Protect code that, in general, can raise errors, reallocate the
** stack, and change the hooks.
*/
#నిర్వచించు Protect(ఘాతాంకం)  (savestate(L,ci), (ఘాతాంకం), updatetrap(ci))

/* special version that does not change the top */
#నిర్వచించు ProtectNT(ఘాతాంకం)  (savepc(ci), (ఘాతాంకం), updatetrap(ci))

/*
** Protect code that can only raise errors. (That is, it cannot change
** the stack or hooks.)
*/
#నిర్వచించు halfProtect(ఘాతాంకం)  (savestate(L,ci), (ఘాతాంకం))

/*
** macro executed during Lua functions at points where the
** function can yield.
*/
#ఒకవేళ !defined(luai_threadyield)
#నిర్వచించు luai_threadyield(L)	{lua_unlock(L); lua_తాళం(L);}
#అయితే_ముగింపు

/* 'c' is the limit of live values in the stack */
#నిర్వచించు checkGC(L,c)  \
	{ luaC_condGC(L, (savepc(ci), L->శిఖరం.p = (c)), \
                         updatetrap(ci)); \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#నిర్వచించు vmfetch()	{ \
  ఒకవేళ (l_unlikely(trap)) {  /* stack reallocation or hooks? */ \
    trap = luaG_traceexec(L, pc);  /* handle hooks */ \
    updatebase(ci);  /* correct stack */ \
  } \
  i = *(pc++); \
}

#నిర్వచించు vmdispatch(o)	ఎంపిక(o)
#నిర్వచించు vmcase(l)	స్థితి l:
#నిర్వచించు vmbreak		ఆపు


శూన్య luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId పునాది;
  స్థిరరాశి Instruction *pc;
  పూర్ణాంకం trap;
#ఒకవేళ LUA_USE_JUMPTABLE
#అన్తర్భావ "ljumptab.h"
#అయితే_ముగింపు
 startfunc:
  trap = cast_int(L->hookmask);
 returning:  /* trap already set */
  cl = ci_func(ci);
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  ఒకవేళ (l_unlikely(trap))
    trap = luaG_tracecall(L);
  పునాది = ci->func.p + 1;
  /* main loop of interpreter */
  కొరకు (;;) {
    Instruction i;  /* instruction being executed */
    vmfetch();
    #ఒకవేళ 0
    { /* low-level line tracing for debugging Lua */
      #అన్తర్భావ "lopnames.h"
      పూర్ణాంకం pcrel = pcRel(pc, cl->p);
      రాయి("line: %d; %s (%d)\n", luaG_getfuncline(cl->p, pcrel),
             opnames[GET_OPCODE(i)], pcrel);
    }
    #అయితే_ముగింపు
    lua_assert(పునాది == ci->func.p + 1);
    lua_assert(పునాది <= L->శిఖరం.p && L->శిఖరం.p <= L->దొంతర_చివరి.p);
    /* for tests, invalidate top for instructions not expecting it */
    lua_assert(luaP_isIT(i) || (cast_void(L->శిఖరం.p = పునాది), 1));
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
        పూర్ణాంకం b = GETARG_sBx(i);
        setfltvalue(s2v(ra), cast_అంకె(b));
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
        పూర్ణాంకం b = GETARG_B(i);
        చేయి {
          setnilvalue(s2v(ra++));
        } అయితే (b--);
        vmbreak;
      }
      vmcase(OP_GETUPVAL) {
        StkId ra = RA(i);
        పూర్ణాంకం b = GETARG_B(i);
        setobj2s(L, ra, cl->upvals[b]->v.p);
        vmbreak;
      }
      vmcase(OP_SETUPVAL) {
        StkId ra = RA(i);
        పైకి_Val *uv = cl->upvals[GETARG_B(i)];
        setobj(L, uv->v.p, s2v(ra));
        luaC_barrier(L, uv, s2v(ra));
        vmbreak;
      }
      vmcase(OP_GETTABUP) {
        StkId ra = RA(i);
        TValue *upval = cl->upvals[GETARG_B(i)]->v.p;
        TValue *rc = KC(i);
        TString *కీలకం = tsvalue(rc);  /* key must be a short string */
        lu_byte ట్యాగ్;
        luaV_fastget(upval, కీలకం, s2v(ra), luaH_getshortstr, ట్యాగ్);
        ఒకవేళ (tagisempty(ట్యాగ్))
          Protect(luaV_finishget(L, upval, rc, ra, ట్యాగ్));
        vmbreak;
      }
      vmcase(OP_GETTABLE) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = vRC(i);
        lu_byte ట్యాగ్;
        ఒకవేళ (ttisinteger(rc)) {  /* fast track for integers? */
          luaV_fastgeti(rb, ivalue(rc), s2v(ra), ట్యాగ్);
        }
        లేకపోతే
          luaV_fastget(rb, rc, s2v(ra), luaH_get, ట్యాగ్);
        ఒకవేళ (tagisempty(ట్యాగ్))
          Protect(luaV_finishget(L, rb, rc, ra, ట్యాగ్));
        vmbreak;
      }
      vmcase(OP_GETI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        పూర్ణాంకం c = GETARG_C(i);
        lu_byte ట్యాగ్;
        luaV_fastgeti(rb, c, s2v(ra), ట్యాగ్);
        ఒకవేళ (tagisempty(ట్యాగ్)) {
          TValue కీలకం;
          setivalue(&కీలకం, c);
          Protect(luaV_finishget(L, rb, &కీలకం, ra, ట్యాగ్));
        }
        vmbreak;
      }
      vmcase(OP_GETFIELD) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *కీలకం = tsvalue(rc);  /* key must be a short string */
        lu_byte ట్యాగ్;
        luaV_fastget(rb, కీలకం, s2v(ra), luaH_getshortstr, ట్యాగ్);
        ఒకవేళ (tagisempty(ట్యాగ్))
          Protect(luaV_finishget(L, rb, rc, ra, ట్యాగ్));
        vmbreak;
      }
      vmcase(OP_SETTABUP) {
        పూర్ణాంకం hres;
        TValue *upval = cl->upvals[GETARG_A(i)]->v.p;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *కీలకం = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(upval, కీలకం, rc, hres, luaH_psetshortstr);
        ఒకవేళ (hres == HOK)
          luaV_finishfastset(L, upval, rc);
        లేకపోతే
          Protect(luaV_finishset(L, upval, rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETTABLE) {
        StkId ra = RA(i);
        పూర్ణాంకం hres;
        TValue *rb = vRB(i);  /* key (table is in 'ra') */
        TValue *rc = RKC(i);  /*విలువ*/
        ఒకవేళ (ttisinteger(rb)) {  /* fast track for integers? */
          luaV_fastseti(s2v(ra), ivalue(rb), rc, hres);
        }
        లేకపోతే {
          luaV_fastset(s2v(ra), rb, rc, hres, luaH_pset);
        }
        ఒకవేళ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        లేకపోతే
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETI) {
        StkId ra = RA(i);
        పూర్ణాంకం hres;
        పూర్ణాంకం b = GETARG_B(i);
        TValue *rc = RKC(i);
        luaV_fastseti(s2v(ra), b, rc, hres);
        ఒకవేళ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        లేకపోతే {
          TValue కీలకం;
          setivalue(&కీలకం, b);
          Protect(luaV_finishset(L, s2v(ra), &కీలకం, rc, hres));
        }
        vmbreak;
      }
      vmcase(OP_SETFIELD) {
        StkId ra = RA(i);
        పూర్ణాంకం hres;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *కీలకం = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(s2v(ra), కీలకం, rc, hres, luaH_psetshortstr);
        ఒకవేళ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        లేకపోతే
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_NEWTABLE) {
        StkId ra = RA(i);
        గుర్తులేని b = cast_uint(GETARG_vB(i));  /* log2(hash size) + 1 */
        గుర్తులేని c = cast_uint(GETARG_vC(i));  /* array size */
        పట్టిక *t;
        ఒకవేళ (b > 0)
          b = 1u << (b - 1);  /* hash size is 2^(b - 1) */
        ఒకవేళ (TESTARG_k(i)) {  /* non-zero extra argument? */
          lua_assert(GETARG_Ax(*pc) != 0);
          /* add it to array size */
          c += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
        }
        pc++;  /* skip extra argument */
        L->శిఖరం.p = ra + 1;  /* correct top in case of emergency GC */
        t = luaH_new(L);  /* memory allocation */
        sethvalue2s(L, ra, t);
        ఒకవేళ (b != 0 || c != 0)
          luaH_resize(L, t, c, b);  /* idem */
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_SELF) {
        StkId ra = RA(i);
        lu_byte ట్యాగ్;
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *కీలకం = tsvalue(rc);  /* key must be a short string */
        setobj2s(L, ra + 1, rb);
        luaV_fastget(rb, కీలకం, s2v(ra), luaH_getshortstr, ట్యాగ్);
        ఒకవేళ (tagisempty(ట్యాగ్))
          Protect(luaV_finishget(L, rb, rc, ra, ట్యాగ్));
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
        పూర్ణాంకం ic = GETARG_sC(i);
        lua_Integer ib;
        ఒకవేళ (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ic, ib));
        }
        vmbreak;
      }
      vmcase(OP_SHRI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        పూర్ణాంకం ic = GETARG_sC(i);
        lua_Integer ib;
        ఒకవేళ (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ib, -ic));
        }
        vmbreak;
      }
      vmcase(OP_కూడు) {
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
        StkId ఫలితం = RA(pi);
        lua_assert(OP_కూడు <= GET_OPCODE(pi) && GET_OPCODE(pi) <= OP_SHR);
        Protect(luaT_trybinTM(L, s2v(ra), rb, ఫలితం, tm));
        vmbreak;
      }
      vmcase(OP_MMBINI) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        పూర్ణాంకం imm = GETARG_sB(i);
        TMS tm = (TMS)GETARG_C(i);
        పూర్ణాంకం flip = GETARG_k(i);
        StkId ఫలితం = RA(pi);
        Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, ఫలితం, tm));
        vmbreak;
      }
      vmcase(OP_MMBINK) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        TValue *imm = KB(i);
        TMS tm = (TMS)GETARG_C(i);
        పూర్ణాంకం flip = GETARG_k(i);
        StkId ఫలితం = RA(pi);
        Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, ఫలితం, tm));
        vmbreak;
      }
      vmcase(OP_UNM) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_సంఖ్య nb;
        ఒకవేళ (ttisinteger(rb)) {
          lua_Integer ib = ivalue(rb);
          setivalue(s2v(ra), intop(-, 0, ib));
        }
        లేకపోతే ఒకవేళ (tonumberns(rb, nb)) {
          setfltvalue(s2v(ra), luai_numunm(L, nb));
        }
        లేకపోతే
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
        vmbreak;
      }
      vmcase(OP_BNOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_Integer ib;
        ఒకవేళ (tointegerns(rb, &ib)) {
          setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
        }
        లేకపోతే
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
        vmbreak;
      }
      vmcase(OP_NOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ఒకవేళ (l_isfalse(rb))
          setbtvalue(s2v(ra));
        లేకపోతే
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
        పూర్ణాంకం n = GETARG_B(i);  /* number of elements to concatenate */
        L->శిఖరం.p = ra + n;  /* mark the end of concat operands */
        ProtectNT(luaV_concat(L, n));
        checkGC(L, L->శిఖరం.p); /* 'luaV_concat' ensures correct top */
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
        పూర్ణాంకం cond;
        TValue *rb = vRB(i);
        Protect(cond = luaV_equalobj(L, s2v(ra), rb));
        docondjump();
        vmbreak;
      }
      vmcase(OP_LT) {
        op_క్రమం(L, l_lti, LTnum, lessthanothers);
        vmbreak;
      }
      vmcase(OP_LE) {
        op_క్రమం(L, l_lei, LEnum, lessequalothers);
        vmbreak;
      }
      vmcase(OP_EQK) {
        StkId ra = RA(i);
        TValue *rb = KB(i);
        /* basic types do not use '__eq'; we can use raw equality */
        పూర్ణాంకం cond = luaV_rawequalobj(s2v(ra), rb);
        docondjump();
        vmbreak;
      }
      vmcase(OP_EQI) {
        StkId ra = RA(i);
        పూర్ణాంకం cond;
        పూర్ణాంకం im = GETARG_sB(i);
        ఒకవేళ (ttisinteger(s2v(ra)))
          cond = (ivalue(s2v(ra)) == im);
        లేకపోతే ఒకవేళ (ttisfloat(s2v(ra)))
          cond = luai_numeq(fltvalue(s2v(ra)), cast_అంకె(im));
        లేకపోతే
          cond = 0;  /* other types cannot be equal to a number */
        docondjump();
        vmbreak;
      }
      vmcase(OP_LTI) {
        op_క్రమం_I(L, l_lti, luai_numlt, 0, TM_LT);
        vmbreak;
      }
      vmcase(OP_LEI) {
        op_క్రమం_I(L, l_lei, luai_numle, 0, TM_LE);
        vmbreak;
      }
      vmcase(OP_GTI) {
        op_క్రమం_I(L, l_gti, luai_numgt, 1, TM_LT);
        vmbreak;
      }
      vmcase(OP_GEI) {
        op_క్రమం_I(L, l_gei, luai_numge, 1, TM_LE);
        vmbreak;
      }
      vmcase(OP_TEST) {
        StkId ra = RA(i);
        పూర్ణాంకం cond = !l_isfalse(s2v(ra));
        docondjump();
        vmbreak;
      }
      vmcase(OP_TESTSET) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ఒకవేళ (l_isfalse(rb) == GETARG_k(i))
          pc++;
        లేకపోతే {
          setobj2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(OP_CALL) {
        StkId ra = RA(i);
        CallInfo *newci;
        పూర్ణాంకం b = GETARG_B(i);
        పూర్ణాంకం nresults = GETARG_C(i) - 1;
        ఒకవేళ (b != 0)  /* fixed number of arguments? */
          L->శిఖరం.p = ra + b;  /* top signals number of arguments */
        /* else previous instruction set top */
        savepc(ci);  /* in case of errors */
        ఒకవేళ ((newci = luaD_precall(L, ra, nresults)) == ఖాళీ)
          updatetrap(ci);  /* C call; nothing else to be done */
        లేకపోతే {  /* Lua call: run function in this same C frame */
          ci = newci;
          వెళ్ళు startfunc;
        }
        vmbreak;
      }
      vmcase(OP_TAILCALL) {
        StkId ra = RA(i);
        పూర్ణాంకం b = GETARG_B(i);  /* number of arguments + 1 (function) */
        పూర్ణాంకం n;  /* number of results when calling a C function */
        పూర్ణాంకం nparams1 = GETARG_C(i);
        /* delta is virtual 'func' - real 'func' (vararg functions) */
        పూర్ణాంకం delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        ఒకవేళ (b != 0)
          L->శిఖరం.p = ra + b;
        లేకపోతే  /* previous instruction set top */
          b = cast_int(L->శిఖరం.p - ra);
        savepc(ci);  /* several calls here can raise errors */
        ఒకవేళ (TESTARG_k(i)) {
          luaF_closeupval(L, పునాది);  /* close upvalues from current call */
          lua_assert(L->tbclist.p < పునాది);  /* no pending tbc variables */
          lua_assert(పునాది == ci->func.p + 1);
        }
        ఒకవేళ ((n = luaD_pretailcall(L, ci, ra, b, delta)) < 0)  /* Lua function? */
          వెళ్ళు startfunc;  /* execute the callee */
        లేకపోతే {  /* C function? */
          ci->func.p -= delta;  /* restore 'func' (if vararg) */
          luaD_poscall(L, ci, n);  /* finish caller */
          updatetrap(ci);  /* 'luaD_poscall' can change hooks */
          వెళ్ళు ret;  /* caller returns after the tail call */
        }
      }
      vmcase(OP_RETURN) {
        StkId ra = RA(i);
        పూర్ణాంకం n = GETARG_B(i) - 1;  /* number of results */
        పూర్ణాంకం nparams1 = GETARG_C(i);
        ఒకవేళ (n < 0)  /* not fixed? */
          n = cast_int(L->శిఖరం.p - ra);  /* get what is available */
        savepc(ci);
        ఒకవేళ (TESTARG_k(i)) {  /* may there be open upvalues? */
          ci->u2.nres = n;  /* save number of returns */
          ఒకవేళ (L->శిఖరం.p < ci->శిఖరం.p)
            L->శిఖరం.p = ci->శిఖరం.p;
          luaF_close(L, పునాది, CLOSEKTOP, 1);
          updatetrap(ci);
          updatestack(ci);
        }
        ఒకవేళ (nparams1)  /* vararg function? */
          ci->func.p -= ci->u.l.nextraargs + nparams1;
        L->శిఖరం.p = ra + n;  /* set call for 'luaD_poscall' */
        luaD_poscall(L, ci, n);
        updatetrap(ci);  /* 'luaD_poscall' can change hooks */
        వెళ్ళు ret;
      }
      vmcase(OP_RETURN0) {
        ఒకవేళ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->శిఖరం.p = ra;
          savepc(ci);
          luaD_poscall(L, ci, 0);  /* no hurry... */
          trap = 1;
        }
        లేకపోతే {  /* do the 'poscall' here */
          పూర్ణాంకం nres = get_nresults(ci->callstatus);
          L->ci = ci->మునుపటి;  /* back to caller */
          L->శిఖరం.p = పునాది - 1;
          కొరకు (; l_unlikely(nres > 0); nres--)
            setnilvalue(s2v(L->శిఖరం.p++));  /* all results are nil */
        }
        వెళ్ళు ret;
      }
      vmcase(OP_RETURN1) {
        ఒకవేళ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->శిఖరం.p = ra + 1;
          savepc(ci);
          luaD_poscall(L, ci, 1);  /* no hurry... */
          trap = 1;
        }
        లేకపోతే {  /* do the 'poscall' here */
          పూర్ణాంకం nres = get_nresults(ci->callstatus);
          L->ci = ci->మునుపటి;  /* back to caller */
          ఒకవేళ (nres == 0)
            L->శిఖరం.p = పునాది - 1;  /* asked for no results */
          లేకపోతే {
            StkId ra = RA(i);
            setobjs2s(L, పునాది - 1, ra);  /* at least this result */
            L->శిఖరం.p = పునాది;
            కొరకు (; l_unlikely(nres > 1); nres--)
              setnilvalue(s2v(L->శిఖరం.p++));  /* complete missing results */
          }
        }
       ret:  /* return from a Lua function */
        ఒకవేళ (ci->callstatus & CIST_FRESH)
          తిరిగి;  /* end this frame */
        లేకపోతే {
          ci = ci->మునుపటి;
          వెళ్ళు returning;  /* continue running caller in this frame */
        }
      }
      vmcase(OP_FORLOOP) {
        StkId ra = RA(i);
        ఒకవేళ (ttisinteger(s2v(ra + 1))) {  /* integer loop? */
          lua_Unsigned లెక్క = l_castS2U(ivalue(s2v(ra)));
          ఒకవేళ (లెక్క > 0) {  /* still more iterations? */
            lua_Integer అడుగు = ivalue(s2v(ra + 1));
            lua_Integer idx = ivalue(s2v(ra + 2));  /* control variable */
            chgivalue(s2v(ra), l_castU2S(లెక్క - 1));  /* update counter */
            idx = intop(+, idx, అడుగు);  /* add step to index */
            chgivalue(s2v(ra + 2), idx);  /* update control variable */
            pc -= GETARG_Bx(i);  /* jump back */
          }
        }
        లేకపోతే ఒకవేళ (floatforloop(L, ra))  /* float loop */
          pc -= GETARG_Bx(i);  /* jump back */
        updatetrap(ci);  /* allows a signal to break the loop */
        vmbreak;
      }
      vmcase(OP_FORPREP) {
        StkId ra = RA(i);
        savestate(L, ci);  /* in case of errors */
        ఒకవేళ (forprep(L, ra))
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
       TValue తాత్కాలిక;  /* to swap control and closing variables */
       setobj(L, &తాత్కాలిక, s2v(ra + 3));
       setobjs2s(L, ra + 3, ra + 2);
       setobj2s(L, ra + 2, &తాత్కాలిక);
        /* create to-be-closed upvalue (if closing var. is not nil) */
        halfProtect(luaF_newtbcupval(L, ra + 2));
        pc += GETARG_Bx(i);  /* go to end of the loop */
        i = *(pc++);  /* fetch next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORCALL && ra == RA(i));
        వెళ్ళు l_tforcall;
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
        L->శిఖరం.p = ra + 3 + 3;
        ProtectNT(luaD_call(L, ra + 3, GETARG_C(i)));  /* do the call */
        updatestack(ci);  /* stack may have changed */
        i = *(pc++);  /* go to next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
        వెళ్ళు l_tforloop;
      }}
      vmcase(OP_TFORLOOP) {
       l_tforloop: {
        StkId ra = RA(i);
        ఒకవేళ (!ttisnil(s2v(ra + 3)))  /* continue loop? */
          pc -= GETARG_Bx(i);  /* jump back */
        vmbreak;
      }}
      vmcase(OP_SETLIST) {
        StkId ra = RA(i);
        గుర్తులేని n = cast_uint(GETARG_vB(i));
        గుర్తులేని చివరి = cast_uint(GETARG_vC(i));
        పట్టిక *h = hvalue(s2v(ra));
        ఒకవేళ (n == 0)
          n = cast_uint(L->శిఖరం.p - ra) - 1;  /* get up to the top */
        లేకపోతే
          L->శిఖరం.p = ci->శిఖరం.p;  /* correct top in case of emergency GC */
        చివరి += n;
        ఒకవేళ (TESTARG_k(i)) {
          చివరి += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
          pc++;
        }
        /* when 'n' is known, table should have proper size */
        ఒకవేళ (చివరి > h->asize) {  /* needs more space? */
          /* fixed-size sets should have space preallocated */
          lua_assert(GETARG_vB(i) == 0);
          luaH_resizearray(L, h, చివరి);  /* preallocate it at once */
        }
        కొరకు (; n > 0; n--) {
          TValue *val = s2v(ra + n);
          obj2arr(h, చివరి - 1, val);
          చివరి--;
          luaC_barrierback(L, obj2gco(h), val);
        }
        vmbreak;
      }
      vmcase(OP_సంవృతం) {
        StkId ra = RA(i);
        Proto *p = cl->p->p[GETARG_Bx(i)];
        halfProtect(pushclosure(L, p, cl->upvals, పునాది, ra));
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_VARARG) {
        StkId ra = RA(i);
        పూర్ణాంకం n = GETARG_C(i) - 1;  /* required results (-1 means all) */
        పూర్ణాంకం vatab = GETARG_k(i) ? GETARG_B(i) : -1;
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
        ఒకవేళ (!ttisnil(ra))
          halfProtect(luaG_errnnil(L, cl, GETARG_Bx(i)));
        vmbreak;
      }
      vmcase(OP_VARARGPREP) {
        ProtectNT(luaT_adjustvarargs(L, ci, cl->p));
        ఒకవేళ (l_unlikely(trap)) {  /* previous "Protect" updated trap */
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
