/*
** $Id: lvm.c $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#define lvm_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"

#अन्तर्भाव <float.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "ltm.h"
#अन्तर्भाव "lvm.h"


/*
** By default, use jump tables in the main interpreter loop on gcc
** and compatible compilers.
*/
#यदि !defined(LUA_USE_JUMPTABLE)
#यदि defined(__GNUC__)
#define LUA_USE_JUMPTABLE	1
#अन्यथा
#define LUA_USE_JUMPTABLE	0
#endif
#endif



/* limit for table tag-method chains (to avoid infinite loops) */
#define MAXTAGLOOP	2000


/*
** 'l_intfitsf' checks whether a given integer is in the range that
** can be converted to a float without rounding. Used in comparisons.
*/

/* number of bits in the mantissa of a float */
#define NBM		(l_floatatt(MANT_DIG))

/*
** Check whether some integers may not fit in a float, testing whether
** (maxinteger >> NBM) > 0. (That implies (1 << NBM) <= maxinteger.)
** (The shifts are done in parts, to avoid shifting by more than the size
** of an integer. In a worst case, NBM == 113 for long double and
** sizeof(long) == 32.)
*/
#यदि ((((LUA_MAXINTEGER >> (NBM / 4)) >> (NBM / 4)) >> (NBM / 4)) \
	>> (NBM - (3 * (NBM / 4))))  >  0

/* limit for integers that fit in a float */
#define MAXINTFITSF	((lua_Unsigned)1 << NBM)

/* check whether 'i' is in the interval [-MAXINTFITSF, MAXINTFITSF] */
#define l_intfitsf(i)	((MAXINTFITSF + l_castS2U(i)) <= (2 * MAXINTFITSF))

#अन्यथा  /* all integers fit in a float precisely */

#define l_intfitsf(i)	1

#endif


/*
** Try to convert a value from string to a number value.
** If the value is not a string or is a string not representing
** a valid numeral (or if coercions from strings to numbers
** are disabled via macro 'cvt2num'), do not modify 'result'
** and return 0.
*/
स्थिर पूर्णाङ्क l_strton (ध्रुव TValue *obj, TValue *result) {
  lua_assert(obj != result);
  यदि (!cvt2num(obj))  /* is object not a string? */
    प्रत्यागम 0;
  अन्यथा {
    TString *st = tsvalue(obj);
    माप_प्रकार stlen;
    ध्रुव वर्ण *s = getlstr(st, stlen);
    प्रत्यागम (luaO_str2num(s, result) == stlen + 1);
  }
}


/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'tonumber'.
*/
पूर्णाङ्क luaV_tonumber_ (ध्रुव TValue *obj, lua_संख्या *n) {
  TValue v;
  यदि (ttisinteger(obj)) {
    *n = cast_अंक(ivalue(obj));
    प्रत्यागम 1;
  }
  अन्यथा यदि (l_strton(obj, &v)) {  /* string coercible to number? */
    *n = nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    प्रत्यागम 1;
  }
  अन्यथा
    प्रत्यागम 0;  /* conversion failed */
}


/*
** try to convert a float to an integer, rounding according to 'mode'.
*/
पूर्णाङ्क luaV_flttointeger (lua_संख्या n, lua_Integer *p, F2Imod mode) {
  lua_संख्या f = l_floor(n);
  यदि (n != f) {  /* not an integral value? */
    यदि (mode == F2Ieq) प्रत्यागम 0;  /* fails if mode demands integral value */
    अन्यथा यदि (mode == F2Iceil)  /* needs ceiling? */
      f += 1;  /* convert floor to ceiling (remember: n != f) */
  }
  प्रत्यागम lua_numbertointeger(f, p);
}


/*
** try to convert a value to an integer, rounding according to 'mode',
** without string coercion.
** ("Fast track" handled by macro 'tointegerns'.)
*/
पूर्णाङ्क luaV_tointegerns (ध्रुव TValue *obj, lua_Integer *p, F2Imod mode) {
  यदि (ttisfloat(obj))
    प्रत्यागम luaV_flttointeger(fltvalue(obj), p, mode);
  अन्यथा यदि (ttisinteger(obj)) {
    *p = ivalue(obj);
    प्रत्यागम 1;
  }
  अन्यथा
    प्रत्यागम 0;
}


/*
** try to convert a value to an integer.
*/
पूर्णाङ्क luaV_tointeger (ध्रुव TValue *obj, lua_Integer *p, F2Imod mode) {
  TValue v;
  यदि (l_strton(obj, &v))  /* does 'obj' point to a numerical string? */
    obj = &v;  /* change it to point to its corresponding number */
  प्रत्यागम luaV_tointegerns(obj, p, mode);
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
स्थिर पूर्णाङ्क forlimit (lua_State *L, lua_Integer init, ध्रुव TValue *lim,
                                   lua_Integer *p, lua_Integer step) {
  यदि (!luaV_tointeger(lim, p, (step < 0 ? F2Iceil : F2Ifloor))) {
    /* not coercible to in integer */
    lua_संख्या flim;  /* try to convert to float */
    यदि (!tonumber(lim, &flim)) /* cannot convert to float? */
      luaG_forerror(L, lim, "limit");
    /* else 'flim' is a float out of integer bounds */
    यदि (luai_numlt(0, flim)) {  /* if it is positive, it is too large */
      यदि (step < 0) प्रत्यागम 1;  /* initial value must be less than it */
      *p = LUA_MAXINTEGER;  /* truncate */
    }
    अन्यथा यदि (luai_numgt(0, flim)) {  /* if it is negative, it is too small */
      यदि (step > 0) प्रत्यागम 1;  /* initial value must be greater than it */
      *p = LUA_MININTEGER;  /* truncate */
    }
    अन्यथा प्रत्यागम 1;  /* limit is NaN (neither 0 nor positive nor negative) */
  }
  प्रत्यागम (step > 0 ? init > *p : init < *p);  /* not to run? */
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
स्थिर पूर्णाङ्क forprep (lua_State *L, StkId ra) {
  TValue *pinit = s2v(ra);
  TValue *plimit = s2v(ra + 1);
  TValue *pstep = s2v(ra + 2);
  यदि (ttisinteger(pinit) && ttisinteger(pstep)) { /* integer loop? */
    lua_Integer init = ivalue(pinit);
    lua_Integer step = ivalue(pstep);
    lua_Integer limit;
    यदि (step == 0)
      luaG_runerror(L, "'for' step is zero");
    यदि (forlimit(L, init, plimit, &limit, step))
      प्रत्यागम 1;  /* skip the loop */
    अन्यथा {  /* prepare loop counter */
      lua_Unsigned गिनती;
      यदि (step > 0) {  /* ascending loop? */
        गिनती = l_castS2U(limit) - l_castS2U(init);
        यदि (step != 1)  /* avoid division in the too common case */
          गिनती /= l_castS2U(step);
      }
      अन्यथा {  /* step < 0; descending loop */
        गिनती = l_castS2U(init) - l_castS2U(limit);
        /* 'step+1' avoids negating 'mininteger' */
        गिनती /= l_castS2U(-(step + 1)) + 1u;
      }
      /* use 'chgivalue' for places that for sure had integers */
      chgivalue(s2v(ra), l_castU2S(गिनती));  /* change init to count */
      setivalue(s2v(ra + 1), step);  /* change limit to step */
      chgivalue(s2v(ra + 2), init);  /* change step to init */
    }
  }
  अन्यथा {  /* try making all values floats */
    lua_संख्या init; lua_संख्या limit; lua_संख्या step;
    यदि (l_unlikely(!tonumber(plimit, &limit)))
      luaG_forerror(L, plimit, "limit");
    यदि (l_unlikely(!tonumber(pstep, &step)))
      luaG_forerror(L, pstep, "step");
    यदि (l_unlikely(!tonumber(pinit, &init)))
      luaG_forerror(L, pinit, "initial value");
    यदि (step == 0)
      luaG_runerror(L, "'for' step is zero");
    यदि (!(luai_numge(0, step) ? luai_numge(init, limit)
                              : luai_numge(limit, init)))
      प्रत्यागम 1;  /* skip the loop */
    अन्यथा {
      /* make sure all values are floats */
      setfltvalue(s2v(ra), limit);
      setfltvalue(s2v(ra + 1), step);
      setfltvalue(s2v(ra + 2), init);  /* control variable */
    }
  }
  प्रत्यागम 0;
}


/*
** Execute a step of a float numerical for loop, returning
** true iff the loop must continue. (The integer case is
** written inline with opcode OP_FORLOOP, for performance.)
*/
स्थिर पूर्णाङ्क floatforloop (lua_State *L, StkId ra) {
  lua_संख्या step = fltvalue(s2v(ra + 1));
  lua_संख्या limit = fltvalue(s2v(ra));
  lua_संख्या idx = fltvalue(s2v(ra + 2));  /* control variable */
  idx = luai_numadd(L, idx, step);  /* increment index */
  यदि (luai_numlt(0, step) ? luai_numle(idx, limit)
                          : luai_numle(limit, idx)) {
    chgfltvalue(s2v(ra + 2), idx);  /* update control variable */
    प्रत्यागम 1;  /* jump back */
  }
  अन्यथा
    प्रत्यागम 0;  /* finish the loop */
}


/*
** Finish the table access 'val = t[key]' and return the tag of the result.
*/
lu_byte luaV_finishget (lua_State *L, ध्रुव TValue *t, TValue *कुंजी,
                                      StkId val, lu_byte tag) {
  पूर्णाङ्क loop;  /* counter to avoid infinite loops */
  ध्रुव TValue *पंचांग;  /* metamethod */
  प्रति (loop = 0; loop < MAXTAGLOOP; loop++) {
    यदि (tag == LUA_VNOTABLE) {  /* 't' is not a table? */
      lua_assert(!ttistable(t));
      पंचांग = luaT_gettmbyobj(L, t, TM_INDEX);
      यदि (l_unlikely(notm(पंचांग)))
        luaG_typeerror(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    अन्यथा {  /* 't' is a table */
      पंचांग = fasttm(L, hvalue(t)->metatable, TM_INDEX);  /* table's metamethod */
      यदि (पंचांग == रिक्त) {  /* no metamethod? */
        setnilvalue2s(val);  /* result is nil */
        प्रत्यागम LUA_VNIL;
      }
      /* else will try the metamethod */
    }
    यदि (ttisfunction(पंचांग)) {  /* is metamethod a function? */
      tag = luaT_callTMres(L, पंचांग, t, कुंजी, val);  /* call it */
      प्रत्यागम tag;  /* return tag of the result */
    }
    t = पंचांग;  /* else try to access 'tm[key]' */
    luaV_fastget(t, कुंजी, s2v(val), luaH_get, tag);
    यदि (!tagisempty(tag))
      प्रत्यागम tag;  /* done */
    /* else repeat (tail call 'luaV_finishget') */
  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
  प्रत्यागम 0;  /* to avoid warnings */
}


/*
** Finish a table assignment 't[key] = val'.
** About anchoring the table before the call to 'luaH_finishset':
** This call may trigger an emergency collection. When loop>0,
** the table being accessed is a field in some metatable. If this
** metatable is weak and the table is not anchored, this collection
** could collect that table while it is being updated.
*/
शून्य luaV_finishset (lua_State *L, ध्रुव TValue *t, TValue *कुंजी,
                      TValue *val, पूर्णाङ्क hres) {
  पूर्णाङ्क loop;  /* counter to avoid infinite loops */
  प्रति (loop = 0; loop < MAXTAGLOOP; loop++) {
    ध्रुव TValue *पंचांग;  /* '__newindex' metamethod */
    यदि (hres != HNOTATABLE) {  /* is 't' a table? */
      Table *h = hvalue(t);  /* save 't' table */
      पंचांग = fasttm(L, h->metatable, TM_NEWINDEX);  /* get metamethod */
      यदि (पंचांग == रिक्त) {  /* no metamethod? */
        sethvalue2s(L, L->शीर्ष.p, h);  /* anchor 't' */
        L->शीर्ष.p++;  /* assume EXTRA_STACK */
        luaH_finishset(L, h, कुंजी, val, hres);  /* set new value */
        L->शीर्ष.p--;
        invalidateTMcache(h);
        luaC_barrierback(L, obj2gco(h), val);
        प्रत्यागम;
      }
      /* else will try the metamethod */
    }
    अन्यथा {  /* not a table; check metamethod */
      पंचांग = luaT_gettmbyobj(L, t, TM_NEWINDEX);
      यदि (l_unlikely(notm(पंचांग)))
        luaG_typeerror(L, t, "index");
    }
    /* try the metamethod */
    यदि (ttisfunction(पंचांग)) {
      luaT_callTM(L, पंचांग, t, कुंजी, val);
      प्रत्यागम;
    }
    t = पंचांग;  /* else must repeat assignment over 'tm' */
    /* do the equivalent to 'luaV_fastset', but saving 'h' */
    यदि (!ttistable(t))
      hres = HNOTATABLE;
    अन्यथा {
      Table *h = hvalue(t);  /* next call can change the value at 't' */
      hres = luaH_pset(h, कुंजी, val);
      यदि (hres == HOK) {
        luaC_barrierback(L, obj2gco(h), val);  /* luaV_finishfastset */
        प्रत्यागम;  /* done */
      }
    }
    /* else 'return luaV_finishset(L, t, key, val, hres)' (loop) */
  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}


/*
** Function to be used for 0-terminated string order comparison
*/
#यदि !defined(l_strcoll)
#define l_strcoll	strcoll
#endif


/*
** Compare two strings 'ts1' x 'ts2', returning an integer less-equal-
** -greater than zero if 'ts1' is less-equal-greater than 'ts2'.
** The code is a little tricky because it allows '\0' in the strings
** and it uses 'strcoll' (to respect locales) for each segment
** of the strings. Note that segments can compare equal but still
** have different lengths.
*/
स्थिर पूर्णाङ्क l_strcmp (ध्रुव TString *ts1, ध्रुव TString *ts2) {
  माप_प्रकार rl1;  /* real length */
  ध्रुव वर्ण *s1 = getlstr(ts1, rl1);
  माप_प्रकार rl2;
  ध्रुव वर्ण *s2 = getlstr(ts2, rl2);
  प्रति (;;) {  /* for each segment */
    पूर्णाङ्क temp = l_strcoll(s1, s2);
    यदि (temp != 0)  /* not equal? */
      प्रत्यागम temp;  /* done */
    अन्यथा {  /* strings are equal up to a '\0' */
      माप_प्रकार zl1 = म_माप(s1);  /* index of first '\0' in 's1' */
      माप_प्रकार zl2 = म_माप(s2);  /* index of first '\0' in 's2' */
      यदि (zl2 == rl2)  /* 's2' is finished? */
        प्रत्यागम (zl1 == rl1) ? 0 : 1;  /* check 's1' */
      अन्यथा यदि (zl1 == rl1)  /* 's1' is finished? */
        प्रत्यागम -1;  /* 's1' is less than 's2' ('s2' is not finished) */
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
l_sinline पूर्णाङ्क LTintfloat (lua_Integer i, lua_संख्या f) {
  यदि (l_intfitsf(i))
    प्रत्यागम luai_numlt(cast_अंक(i), f);  /* compare them as floats */
  अन्यथा {  /* i < f <=> i < ceil(f) */
    lua_Integer fi;
    यदि (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      प्रत्यागम i < fi;   /* compare them as integers */
    अन्यथा  /* 'f' is either greater or less than all integers */
      प्रत्यागम f > 0;  /* greater? */
  }
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
l_sinline पूर्णाङ्क LEintfloat (lua_Integer i, lua_संख्या f) {
  यदि (l_intfitsf(i))
    प्रत्यागम luai_numle(cast_अंक(i), f);  /* compare them as floats */
  अन्यथा {  /* i <= f <=> i <= floor(f) */
    lua_Integer fi;
    यदि (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      प्रत्यागम i <= fi;   /* compare them as integers */
    अन्यथा  /* 'f' is either greater or less than all integers */
      प्रत्यागम f > 0;  /* greater? */
  }
}


/*
** Check whether float 'f' is less than integer 'i'.
** See comments on previous function.
*/
l_sinline पूर्णाङ्क LTfloatint (lua_संख्या f, lua_Integer i) {
  यदि (l_intfitsf(i))
    प्रत्यागम luai_numlt(f, cast_अंक(i));  /* compare them as floats */
  अन्यथा {  /* f < i <=> floor(f) < i */
    lua_Integer fi;
    यदि (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      प्रत्यागम fi < i;   /* compare them as integers */
    अन्यथा  /* 'f' is either greater or less than all integers */
      प्रत्यागम f < 0;  /* less? */
  }
}


/*
** Check whether float 'f' is less than or equal to integer 'i'.
** See comments on previous function.
*/
l_sinline पूर्णाङ्क LEfloatint (lua_संख्या f, lua_Integer i) {
  यदि (l_intfitsf(i))
    प्रत्यागम luai_numle(f, cast_अंक(i));  /* compare them as floats */
  अन्यथा {  /* f <= i <=> ceil(f) <= i */
    lua_Integer fi;
    यदि (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      प्रत्यागम fi <= i;   /* compare them as integers */
    अन्यथा  /* 'f' is either greater or less than all integers */
      प्रत्यागम f < 0;  /* less? */
  }
}


/*
** Return 'l < r', for numbers.
*/
l_sinline पूर्णाङ्क LTnum (ध्रुव TValue *l, ध्रुव TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  यदि (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    यदि (ttisinteger(r))
      प्रत्यागम li < ivalue(r);  /* both are integers */
    अन्यथा  /* 'l' is int and 'r' is float */
      प्रत्यागम LTintfloat(li, fltvalue(r));  /* l < r ? */
  }
  अन्यथा {
    lua_संख्या lf = fltvalue(l);  /* 'l' must be float */
    यदि (ttisfloat(r))
      प्रत्यागम luai_numlt(lf, fltvalue(r));  /* both are float */
    अन्यथा  /* 'l' is float and 'r' is int */
      प्रत्यागम LTfloatint(lf, ivalue(r));
  }
}


/*
** Return 'l <= r', for numbers.
*/
l_sinline पूर्णाङ्क LEnum (ध्रुव TValue *l, ध्रुव TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  यदि (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    यदि (ttisinteger(r))
      प्रत्यागम li <= ivalue(r);  /* both are integers */
    अन्यथा  /* 'l' is int and 'r' is float */
      प्रत्यागम LEintfloat(li, fltvalue(r));  /* l <= r ? */
  }
  अन्यथा {
    lua_संख्या lf = fltvalue(l);  /* 'l' must be float */
    यदि (ttisfloat(r))
      प्रत्यागम luai_numle(lf, fltvalue(r));  /* both are float */
    अन्यथा  /* 'l' is float and 'r' is int */
      प्रत्यागम LEfloatint(lf, ivalue(r));
  }
}


/*
** return 'l < r' for non-numbers.
*/
स्थिर पूर्णाङ्क lessthanothers (lua_State *L, ध्रुव TValue *l, ध्रुव TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  यदि (ttisstring(l) && ttisstring(r))  /* both are strings? */
    प्रत्यागम l_strcmp(tsvalue(l), tsvalue(r)) < 0;
  अन्यथा
    प्रत्यागम luaT_callorderTM(L, l, r, TM_LT);
}


/*
** Main operation less than; return 'l < r'.
*/
पूर्णाङ्क luaV_lessthan (lua_State *L, ध्रुव TValue *l, ध्रुव TValue *r) {
  यदि (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    प्रत्यागम LTnum(l, r);
  अन्यथा प्रत्यागम lessthanothers(L, l, r);
}


/*
** return 'l <= r' for non-numbers.
*/
स्थिर पूर्णाङ्क lessequalothers (lua_State *L, ध्रुव TValue *l, ध्रुव TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  यदि (ttisstring(l) && ttisstring(r))  /* both are strings? */
    प्रत्यागम l_strcmp(tsvalue(l), tsvalue(r)) <= 0;
  अन्यथा
    प्रत्यागम luaT_callorderTM(L, l, r, TM_LE);
}


/*
** Main operation less than or equal to; return 'l <= r'.
*/
पूर्णाङ्क luaV_lessequal (lua_State *L, ध्रुव TValue *l, ध्रुव TValue *r) {
  यदि (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    प्रत्यागम LEnum(l, r);
  अन्यथा प्रत्यागम lessequalothers(L, l, r);
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
पूर्णाङ्क luaV_equalobj (lua_State *L, ध्रुव TValue *t1, ध्रुव TValue *t2) {
  ध्रुव TValue *पंचांग;
  यदि (ttype(t1) != ttype(t2))  /* not the same type? */
    प्रत्यागम 0;
  अन्यथा यदि (ttypetag(t1) != ttypetag(t2)) {
    विकल्प (ttypetag(t1)) {
      पक्ष LUA_VNUMINT: {  /* integer == float? */
        /* integer and float can only be equal if float has an integer
           value equal to the integer */
        lua_Integer i2;
        प्रत्यागम (luaV_flttointeger(fltvalue(t2), &i2, F2Ieq) &&
                ivalue(t1) == i2);
      }
      पक्ष LUA_VNUMFLT: {  /* float == integer? */
        lua_Integer i1;  /* see comment in previous case */
        प्रत्यागम (luaV_flttointeger(fltvalue(t1), &i1, F2Ieq) &&
                i1 == ivalue(t2));
      }
      पक्ष LUA_VSHRSTR: पक्ष LUA_VLNGSTR: {
        /* compare two strings with different variants: they can be
           equal when one string is a short string and the other is
           an external string  */
        प्रत्यागम luaS_eqstr(tsvalue(t1), tsvalue(t2));
      }
      स्वभाव:
        /* only numbers (integer/float) and strings (long/short) can have
           equal values with different variants */
        प्रत्यागम 0;
    }
  }
  अन्यथा {  /* equal variants */
    विकल्प (ttypetag(t1)) {
      पक्ष LUA_VNIL: पक्ष LUA_VFALSE: पक्ष LUA_VTRUE:
        प्रत्यागम 1;
      पक्ष LUA_VNUMINT:
        प्रत्यागम (ivalue(t1) == ivalue(t2));
      पक्ष LUA_VNUMFLT:
        प्रत्यागम (fltvalue(t1) == fltvalue(t2));
      पक्ष LUA_VLIGHTUSERDATA: प्रत्यागम pvalue(t1) == pvalue(t2);
      पक्ष LUA_VSHRSTR:
        प्रत्यागम eqshrstr(tsvalue(t1), tsvalue(t2));
      पक्ष LUA_VLNGSTR:
        प्रत्यागम luaS_eqstr(tsvalue(t1), tsvalue(t2));
      पक्ष LUA_VUSERDATA: {
        यदि (uvalue(t1) == uvalue(t2)) प्रत्यागम 1;
        अन्यथा यदि (L == रिक्त) प्रत्यागम 0;
        पंचांग = fasttm(L, uvalue(t1)->metatable, TM_EQ);
        यदि (पंचांग == रिक्त)
          पंचांग = fasttm(L, uvalue(t2)->metatable, TM_EQ);
        भङ्ग;  /* will try TM */
      }
      पक्ष LUA_VTABLE: {
        यदि (hvalue(t1) == hvalue(t2)) प्रत्यागम 1;
        अन्यथा यदि (L == रिक्त) प्रत्यागम 0;
        पंचांग = fasttm(L, hvalue(t1)->metatable, TM_EQ);
        यदि (पंचांग == रिक्त)
          पंचांग = fasttm(L, hvalue(t2)->metatable, TM_EQ);
        भङ्ग;  /* will try TM */
      }
      पक्ष LUA_VLCF:
        प्रत्यागम (fvalue(t1) == fvalue(t2));
      स्वभाव:  /* functions and threads */
        प्रत्यागम (gcvalue(t1) == gcvalue(t2));
    }
    यदि (पंचांग == रिक्त)  /* no TM? */
      प्रत्यागम 0;  /* objects are different */
    अन्यथा {
      पूर्णाङ्क tag = luaT_callTMres(L, पंचांग, t1, t2, L->शीर्ष.p);  /* call TM */
      प्रत्यागम !tagisfalse(tag);
    }
  }
}


/* macro used by 'luaV_concat' to ensure that element at 'o' is a string */
#define tostring(L,o)  \
	(ttisstring(o) || (cvt2str(o) && (luaO_tostring(L, o), 1)))

/*
** Check whether object is a short empty string to optimize concatenation.
** (External strings can be empty too; they will be concatenated like
** non-empty ones.)
*/
#define isemptystr(o)	(ttisshrstring(o) && tsvalue(o)->shrlen == 0)

/* copy strings in stack from top - n up to top - 1 to buffer */
स्थिर शून्य copy2buff (StkId शीर्ष, पूर्णाङ्क n, वर्ण *buff) {
  माप_प्रकार tl = 0;  /* size already copied */
  कुरु {
    TString *st = tsvalue(s2v(शीर्ष - n));
    माप_प्रकार l;  /* length of string being copied */
    ध्रुव वर्ण *s = getlstr(st, l);
    स_नकल(buff + tl, s, l * परिमाण(वर्ण));
    tl += l;
  } यावत् (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top.p - total' up to 'L->top.p - 1'.
*/
शून्य luaV_concat (lua_State *L, पूर्णाङ्क total) {
  यदि (total == 1)
    प्रत्यागम;  /* "all" values already concatenated */
  कुरु {
    StkId शीर्ष = L->शीर्ष.p;
    पूर्णाङ्क n = 2;  /* number of elements handled in this pass (at least 2) */
    यदि (!(ttisstring(s2v(शीर्ष - 2)) || cvt2str(s2v(शीर्ष - 2))) ||
        !tostring(L, s2v(शीर्ष - 1)))
      luaT_tryconcatTM(L);  /* may invalidate 'top' */
    अन्यथा यदि (isemptystr(s2v(शीर्ष - 1)))  /* second operand is empty? */
      cast_void(tostring(L, s2v(शीर्ष - 2)));  /* result is first operand */
    अन्यथा यदि (isemptystr(s2v(शीर्ष - 2))) {  /* first operand is empty string? */
      setobjs2s(L, शीर्ष - 2, शीर्ष - 1);  /* result is second op. */
    }
    अन्यथा {
      /* at least two string values; get as many as possible */
      माप_प्रकार tl = tsslen(tsvalue(s2v(शीर्ष - 1)));  /* total length */
      TString *ts;
      /* collect total length and number of strings */
      प्रति (n = 1; n < total && tostring(L, s2v(शीर्ष - n - 1)); n++) {
        माप_प्रकार l = tsslen(tsvalue(s2v(शीर्ष - n - 1)));
        यदि (l_unlikely(l >= MAX_SIZE - परिमाण(TString) - tl)) {
          L->शीर्ष.p = शीर्ष - total;  /* pop strings to avoid wasting stack */
          luaG_runerror(L, "string length overflow");
        }
        tl += l;
      }
      यदि (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        वर्ण buff[LUAI_MAXSHORTLEN];
        copy2buff(शीर्ष, n, buff);  /* copy strings to buffer */
        ts = luaS_newlstr(L, buff, tl);
      }
      अन्यथा {  /* long string; copy strings directly to final result */
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(शीर्ष, n, getlngstr(ts));
      }
      setsvalue2s(L, शीर्ष - n, ts);  /* create result */
    }
    total -= n - 1;  /* got 'n' strings to create one new */
    L->शीर्ष.p -= n - 1;  /* popped 'n' strings and pushed one */
  } यावत् (total > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra = #rb'.
*/
शून्य luaV_objlen (lua_State *L, StkId ra, ध्रुव TValue *rb) {
  ध्रुव TValue *पंचांग;
  विकल्प (ttypetag(rb)) {
    पक्ष LUA_VTABLE: {
      Table *h = hvalue(rb);
      पंचांग = fasttm(L, h->metatable, TM_LEN);
      यदि (पंचांग) भङ्ग;  /* metamethod? break switch to call it */
      setivalue(s2v(ra), l_castU2S(luaH_getn(L, h)));  /* else primitive len */
      प्रत्यागम;
    }
    पक्ष LUA_VSHRSTR: {
      setivalue(s2v(ra), tsvalue(rb)->shrlen);
      प्रत्यागम;
    }
    पक्ष LUA_VLNGSTR: {
      setivalue(s2v(ra), cast_st2S(tsvalue(rb)->u.lnglen));
      प्रत्यागम;
    }
    स्वभाव: {  /* try metamethod */
      पंचांग = luaT_gettmbyobj(L, rb, TM_LEN);
      यदि (l_unlikely(notm(पंचांग)))  /* no metamethod? */
        luaG_typeerror(L, rb, "get length of");
      भङ्ग;
    }
  }
  luaT_callTMres(L, पंचांग, rb, rb, ra);
}


/*
** Integer division; return 'm // n', that is, floor(m/n).
** C division truncates its result (rounds towards zero).
** 'floor(q) == trunc(q)' when 'q >= 0' or when 'q' is integer,
** otherwise 'floor(q) == trunc(q) - 1'.
*/
lua_Integer luaV_idiv (lua_State *L, lua_Integer m, lua_Integer n) {
  यदि (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    यदि (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    प्रत्यागम intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  अन्यथा {
    lua_Integer q = m / n;  /* perform C division */
    यदि ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    प्रत्यागम q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_idiv.)
*/
lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  यदि (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    यदि (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    प्रत्यागम 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  अन्यथा {
    lua_Integer r = m % n;
    यदि (r != 0 && (r ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    प्रत्यागम r;
  }
}


/*
** Float modulus
*/
lua_संख्या luaV_modf (lua_State *L, lua_संख्या m, lua_संख्या n) {
  lua_संख्या r;
  luai_nummod(L, m, n, r);
  प्रत्यागम r;
}


/* number of bits in an integer */
#define NBITS	l_numbits(lua_Integer)


/*
** Shift left operation. (Shift right just negates 'y'.)
*/
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  यदि (y < 0) {  /* shift right? */
    यदि (y <= -NBITS) प्रत्यागम 0;
    अन्यथा प्रत्यागम intop(>>, x, -y);
  }
  अन्यथा {  /* shift left */
    यदि (y >= NBITS) प्रत्यागम 0;
    अन्यथा प्रत्यागम intop(<<, x, y);
  }
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues.
*/
स्थिर शून्य pushclosure (lua_State *L, Proto *p, UpVal **encup, StkId base,
                         StkId ra) {
  पूर्णाङ्क nup = p->sizeupvalues;
  Upvaldesc *uv = p->upvalues;
  पूर्णाङ्क i;
  LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  setclLvalue2s(L, ra, ncl);  /* anchor new closure in stack */
  प्रति (i = 0; i < nup; i++) {  /* fill in its upvalues */
    यदि (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = luaF_findupval(L, base + uv[i].idx);
    अन्यथा  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    luaC_objbarrier(L, ncl, ncl->upvals[i]);
  }
}


/*
** finish execution of an opcode interrupted by a yield
*/
शून्य luaV_finishOp (lua_State *L) {
  CallInfo *ci = L->ci;
  StkId base = ci->func.p + 1;
  Instruction inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  OpCode op = GET_OPCODE(inst);
  विकल्प (op) {  /* finish its execution */
    पक्ष OP_MMBIN: पक्ष OP_MMBINI: पक्ष OP_MMBINK: {
      setobjs2s(L, base + GETARG_A(*(ci->u.l.savedpc - 2)), --L->शीर्ष.p);
      भङ्ग;
    }
    पक्ष OP_UNM: पक्ष OP_BNOT: पक्ष OP_LEN:
    पक्ष OP_GETTABUP: पक्ष OP_GETTABLE: पक्ष OP_GETI:
    पक्ष OP_GETFIELD: पक्ष OP_SELF: {
      setobjs2s(L, base + GETARG_A(inst), --L->शीर्ष.p);
      भङ्ग;
    }
    पक्ष OP_LT: पक्ष OP_LE:
    पक्ष OP_LTI: पक्ष OP_LEI:
    पक्ष OP_GTI: पक्ष OP_GEI:
    पक्ष OP_EQ: {  /* note that 'OP_EQI'/'OP_EQK' cannot yield */
      पूर्णाङ्क res = !l_isfalse(s2v(L->शीर्ष.p - 1));
      L->शीर्ष.p--;
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_JMP);
      यदि (res != GETARG_k(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      भङ्ग;
    }
    पक्ष OP_CONCAT: {
      StkId शीर्ष = L->शीर्ष.p - 1;  /* top when 'luaT_tryconcatTM' was called */
      पूर्णाङ्क a = GETARG_A(inst);      /* first element to concatenate */
      पूर्णाङ्क total = cast_int(शीर्ष - 1 - (base + a));  /* yet to concatenate */
      setobjs2s(L, शीर्ष - 2, शीर्ष);  /* put TM result in proper position */
      L->शीर्ष.p = शीर्ष - 1;  /* top is one after last element (at top-2) */
      luaV_concat(L, total);  /* concat them (may yield again) */
      भङ्ग;
    }
    पक्ष OP_CLOSE: {  /* yielded closing variables */
      ci->u.l.savedpc--;  /* repeat instruction to close other vars. */
      भङ्ग;
    }
    पक्ष OP_RETURN: {  /* yielded closing variables */
      StkId ra = base + GETARG_A(inst);
      /* adjust top to signal correct number of returns, in case the
         return is "up to top" ('isIT') */
      L->शीर्ष.p = ra + ci->u2.nres;
      /* repeat instruction to close other vars. and complete the return */
      ci->u.l.savedpc--;
      भङ्ग;
    }
    स्वभाव: {
      /* only these other opcodes can yield */
      lua_assert(op == OP_TFORCALL || op == OP_CALL ||
           op == OP_TAILCALL || op == OP_SETTABUP || op == OP_SETTABLE ||
           op == OP_SETI || op == OP_SETFIELD);
      भङ्ग;
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

#define l_addi(L,a,b)	intop(+, a, b)
#define l_subi(L,a,b)	intop(-, a, b)
#define l_muli(L,a,b)	intop(*, a, b)
#define l_band(a,b)	intop(&, a, b)
#define l_bor(a,b)	intop(|, a, b)
#define l_bxor(a,b)	intop(^, a, b)

#define l_lti(a,b)	(a < b)
#define l_lei(a,b)	(a <= b)
#define l_gti(a,b)	(a > b)
#define l_gei(a,b)	(a >= b)


/*
** Arithmetic operations with immediate operands. 'iop' is the integer
** operation, 'fop' is the float operation.
*/
#define op_arithI(L,iop,fop) {  \
  TValue *ra = vRA(i); \
  TValue *v1 = vRB(i);  \
  पूर्णाङ्क imm = GETARG_sC(i);  \
  यदि (ttisinteger(v1)) {  \
    lua_Integer iv1 = ivalue(v1);  \
    pc++; setivalue(ra, iop(L, iv1, imm));  \
  }  \
  अन्यथा यदि (ttisfloat(v1)) {  \
    lua_संख्या nb = fltvalue(v1);  \
    lua_संख्या fimm = cast_अंक(imm);  \
    pc++; setfltvalue(ra, fop(L, nb, fimm)); \
  }}


/*
** Auxiliary function for arithmetic operations over floats and others
** with two operands.
*/
#define op_arithf_aux(L,v1,v2,fop) {  \
  lua_संख्या n1; lua_संख्या n2;  \
  यदि (tonumberns(v1, n1) && tonumberns(v2, n2)) {  \
    StkId ra = RA(i);  \
    pc++; setfltvalue(s2v(ra), fop(L, n1, n2));  \
  }}


/*
** Arithmetic operations over floats and others with register operands.
*/
#define op_arithf(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with K operands for floats.
*/
#define op_arithfK(L,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations over integers and floats.
*/
#define op_arith_aux(L,v1,v2,iop,fop) {  \
  यदि (ttisinteger(v1) && ttisinteger(v2)) {  \
    StkId ra = RA(i); \
    lua_Integer i1 = ivalue(v1); lua_Integer i2 = ivalue(v2);  \
    pc++; setivalue(s2v(ra), iop(L, i1, i2));  \
  }  \
  अन्यथा op_arithf_aux(L, v1, v2, fop); }


/*
** Arithmetic operations with register operands.
*/
#define op_arith(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Arithmetic operations with K operands.
*/
#define op_arithK(L,iop,fop) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i); lua_assert(ttisnumber(v2));  \
  op_arith_aux(L, v1, v2, iop, fop); }


/*
** Bitwise operations with constant operand.
*/
#define op_bitwiseK(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = KC(i);  \
  lua_Integer i1;  \
  lua_Integer i2 = ivalue(v2);  \
  यदि (tointegerns(v1, &i1)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Bitwise operations with register operands.
*/
#define op_bitwise(L,op) {  \
  TValue *v1 = vRB(i);  \
  TValue *v2 = vRC(i);  \
  lua_Integer i1; lua_Integer i2;  \
  यदि (tointegerns(v1, &i1) && tointegerns(v2, &i2)) {  \
    StkId ra = RA(i); \
    pc++; setivalue(s2v(ra), op(i1, i2));  \
  }}


/*
** Order operations with register operands. 'opn' actually works
** for all numbers, but the fast track improves performance for
** integers.
*/
#define op_order(L,opi,opn,other) {  \
  TValue *ra = vRA(i); \
  पूर्णाङ्क cond;  \
  TValue *rb = vRB(i);  \
  यदि (ttisinteger(ra) && ttisinteger(rb)) {  \
    lua_Integer ia = ivalue(ra);  \
    lua_Integer ib = ivalue(rb);  \
    cond = opi(ia, ib);  \
  }  \
  अन्यथा यदि (ttisnumber(ra) && ttisnumber(rb))  \
    cond = opn(ra, rb);  \
  अन्यथा  \
    Protect(cond = other(L, ra, rb));  \
  docondjump(); }


/*
** Order operations with immediate operand. (Immediate operand is
** always small enough to have an exact representation as a float.)
*/
#define op_orderI(L,opi,opf,inv,पंचांग) {  \
  TValue *ra = vRA(i); \
  पूर्णाङ्क cond;  \
  पूर्णाङ्क im = GETARG_sB(i);  \
  यदि (ttisinteger(ra))  \
    cond = opi(ivalue(ra), im);  \
  अन्यथा यदि (ttisfloat(ra)) {  \
    lua_संख्या fa = fltvalue(ra);  \
    lua_संख्या fim = cast_अंक(im);  \
    cond = opf(fa, fim);  \
  }  \
  अन्यथा {  \
    पूर्णाङ्क isf = GETARG_C(i);  \
    Protect(cond = luaT_callorderiTM(L, ra, im, inv, isf, पंचांग));  \
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


#define RA(i)	(base+GETARG_A(i))
#define vRA(i)	s2v(RA(i))
#define RB(i)	(base+GETARG_B(i))
#define vRB(i)	s2v(RB(i))
#define KB(i)	(k+GETARG_B(i))
#define RC(i)	(base+GETARG_C(i))
#define vRC(i)	s2v(RC(i))
#define KC(i)	(k+GETARG_C(i))
#define RKC(i)	((TESTARG_k(i)) ? k + GETARG_C(i) : s2v(base + GETARG_C(i)))



#define updatetrap(ci)  (trap = cast_int(ci->u.l.trap))

#define updatebase(ci)	(base = ci->func.p + 1)


#define updatestack(ci)  \
	{ यदि (l_unlikely(trap)) { updatebase(ci); ra = RA(i); } }


/*
** Execute a jump instruction. The 'updatetrap' allows signals to stop
** tight loops. (Without it, the local copy of 'trap' could never change.)
*/
#define dojump(ci,i,e)	{ pc += GETARG_sJ(i) + e; updatetrap(ci); }


/* for test instructions, execute the jump instruction that follows it */
#define donextjump(ci)	{ Instruction ni = *pc; dojump(ci, ni, 1); }

/*
** do a conditional jump: skip next instruction if 'cond' is not what
** was expected (parameter 'k'), else do next instruction, which must
** be a jump.
*/
#define docondjump()	यदि (cond != GETARG_k(i)) pc++; अन्यथा donextjump(ci);


/*
** Correct global 'pc'.
*/
#define savepc(ci)	(ci->u.l.savedpc = pc)


/*
** Whenever code can raise errors, the global 'pc' and the global
** 'top' must be correct to report occasional errors.
*/
#define savestate(L,ci)		(savepc(ci), L->शीर्ष.p = ci->शीर्ष.p)


/*
** Protect code that, in general, can raise errors, reallocate the
** stack, and change the hooks.
*/
#define Protect(exp)  (savestate(L,ci), (exp), updatetrap(ci))

/* special version that does not change the top */
#define ProtectNT(exp)  (savepc(ci), (exp), updatetrap(ci))

/*
** Protect code that can only raise errors. (That is, it cannot change
** the stack or hooks.)
*/
#define halfProtect(exp)  (savestate(L,ci), (exp))

/*
** macro executed during Lua functions at points where the
** function can yield.
*/
#यदि !defined(luai_threadyield)
#define luai_threadyield(L)	{lua_unlock(L); lua_lock(L);}
#endif

/* 'c' is the limit of live values in the stack */
#define checkGC(L,c)  \
	{ luaC_condGC(L, (savepc(ci), L->शीर्ष.p = (c)), \
                         updatetrap(ci)); \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#define vmfetch()	{ \
  यदि (l_unlikely(trap)) {  /* stack reallocation or hooks? */ \
    trap = luaG_traceexec(L, pc);  /* handle hooks */ \
    updatebase(ci);  /* correct stack */ \
  } \
  i = *(pc++); \
}

#define vmdispatch(o)	विकल्प(o)
#define vmcase(l)	पक्ष l:
#define vmbreak		भङ्ग


शून्य luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId base;
  ध्रुव Instruction *pc;
  पूर्णाङ्क trap;
#यदि LUA_USE_JUMPTABLE
#अन्तर्भाव "ljumptab.h"
#endif
 startfunc:
  trap = cast_int(L->hookmask);
 returning:  /* trap already set */
  cl = ci_func(ci);
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  यदि (l_unlikely(trap))
    trap = luaG_tracecall(L);
  base = ci->func.p + 1;
  /* main loop of interpreter */
  प्रति (;;) {
    Instruction i;  /* instruction being executed */
    vmfetch();
    #यदि 0
    { /* low-level line tracing for debugging Lua */
      #अन्तर्भाव "lopnames.h"
      पूर्णाङ्क pcrel = pcRel(pc, cl->p);
      म_लिखो("line: %d; %s (%d)\n", luaG_getfuncline(cl->p, pcrel),
             opnames[GET_OPCODE(i)], pcrel);
    }
    #endif
    lua_assert(base == ci->func.p + 1);
    lua_assert(base <= L->शीर्ष.p && L->शीर्ष.p <= L->ढेर_last.p);
    /* for tests, invalidate top for instructions not expecting it */
    lua_assert(luaP_isIT(i) || (cast_void(L->शीर्ष.p = base), 1));
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
        पूर्णाङ्क b = GETARG_sBx(i);
        setfltvalue(s2v(ra), cast_अंक(b));
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
        पूर्णाङ्क b = GETARG_B(i);
        कुरु {
          setnilvalue(s2v(ra++));
        } यावत् (b--);
        vmbreak;
      }
      vmcase(OP_GETUPVAL) {
        StkId ra = RA(i);
        पूर्णाङ्क b = GETARG_B(i);
        setobj2s(L, ra, cl->upvals[b]->v.p);
        vmbreak;
      }
      vmcase(OP_SETUPVAL) {
        StkId ra = RA(i);
        UpVal *uv = cl->upvals[GETARG_B(i)];
        setobj(L, uv->v.p, s2v(ra));
        luaC_barrier(L, uv, s2v(ra));
        vmbreak;
      }
      vmcase(OP_GETTABUP) {
        StkId ra = RA(i);
        TValue *upval = cl->upvals[GETARG_B(i)]->v.p;
        TValue *rc = KC(i);
        TString *कुंजी = tsvalue(rc);  /* key must be a short string */
        lu_byte tag;
        luaV_fastget(upval, कुंजी, s2v(ra), luaH_getshortstr, tag);
        यदि (tagisempty(tag))
          Protect(luaV_finishget(L, upval, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_GETTABLE) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = vRC(i);
        lu_byte tag;
        यदि (ttisinteger(rc)) {  /* fast track for integers? */
          luaV_fastgeti(rb, ivalue(rc), s2v(ra), tag);
        }
        अन्यथा
          luaV_fastget(rb, rc, s2v(ra), luaH_get, tag);
        यदि (tagisempty(tag))
          Protect(luaV_finishget(L, rb, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_GETI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        पूर्णाङ्क c = GETARG_C(i);
        lu_byte tag;
        luaV_fastgeti(rb, c, s2v(ra), tag);
        यदि (tagisempty(tag)) {
          TValue कुंजी;
          setivalue(&कुंजी, c);
          Protect(luaV_finishget(L, rb, &कुंजी, ra, tag));
        }
        vmbreak;
      }
      vmcase(OP_GETFIELD) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *कुंजी = tsvalue(rc);  /* key must be a short string */
        lu_byte tag;
        luaV_fastget(rb, कुंजी, s2v(ra), luaH_getshortstr, tag);
        यदि (tagisempty(tag))
          Protect(luaV_finishget(L, rb, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_SETTABUP) {
        पूर्णाङ्क hres;
        TValue *upval = cl->upvals[GETARG_A(i)]->v.p;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *कुंजी = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(upval, कुंजी, rc, hres, luaH_psetshortstr);
        यदि (hres == HOK)
          luaV_finishfastset(L, upval, rc);
        अन्यथा
          Protect(luaV_finishset(L, upval, rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETTABLE) {
        StkId ra = RA(i);
        पूर्णाङ्क hres;
        TValue *rb = vRB(i);  /* key (table is in 'ra') */
        TValue *rc = RKC(i);  /*मूल्य*/
        यदि (ttisinteger(rb)) {  /* fast track for integers? */
          luaV_fastseti(s2v(ra), ivalue(rb), rc, hres);
        }
        अन्यथा {
          luaV_fastset(s2v(ra), rb, rc, hres, luaH_pset);
        }
        यदि (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        अन्यथा
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETI) {
        StkId ra = RA(i);
        पूर्णाङ्क hres;
        पूर्णाङ्क b = GETARG_B(i);
        TValue *rc = RKC(i);
        luaV_fastseti(s2v(ra), b, rc, hres);
        यदि (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        अन्यथा {
          TValue कुंजी;
          setivalue(&कुंजी, b);
          Protect(luaV_finishset(L, s2v(ra), &कुंजी, rc, hres));
        }
        vmbreak;
      }
      vmcase(OP_SETFIELD) {
        StkId ra = RA(i);
        पूर्णाङ्क hres;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *कुंजी = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(s2v(ra), कुंजी, rc, hres, luaH_psetshortstr);
        यदि (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        अन्यथा
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_NEWTABLE) {
        StkId ra = RA(i);
        अचिन्हित b = cast_uint(GETARG_vB(i));  /* log2(hash size) + 1 */
        अचिन्हित c = cast_uint(GETARG_vC(i));  /* array size */
        Table *t;
        यदि (b > 0)
          b = 1u << (b - 1);  /* hash size is 2^(b - 1) */
        यदि (TESTARG_k(i)) {  /* non-zero extra argument? */
          lua_assert(GETARG_Ax(*pc) != 0);
          /* add it to array size */
          c += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
        }
        pc++;  /* skip extra argument */
        L->शीर्ष.p = ra + 1;  /* correct top in case of emergency GC */
        t = luaH_new(L);  /* memory allocation */
        sethvalue2s(L, ra, t);
        यदि (b != 0 || c != 0)
          luaH_resize(L, t, c, b);  /* idem */
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_SELF) {
        StkId ra = RA(i);
        lu_byte tag;
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *कुंजी = tsvalue(rc);  /* key must be a short string */
        setobj2s(L, ra + 1, rb);
        luaV_fastget(rb, कुंजी, s2v(ra), luaH_getshortstr, tag);
        यदि (tagisempty(tag))
          Protect(luaV_finishget(L, rb, rc, ra, tag));
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
        पूर्णाङ्क ic = GETARG_sC(i);
        lua_Integer ib;
        यदि (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ic, ib));
        }
        vmbreak;
      }
      vmcase(OP_SHRI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        पूर्णाङ्क ic = GETARG_sC(i);
        lua_Integer ib;
        यदि (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ib, -ic));
        }
        vmbreak;
      }
      vmcase(OP_ADD) {
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
        TMS पंचांग = (TMS)GETARG_C(i);
        StkId result = RA(pi);
        lua_assert(OP_ADD <= GET_OPCODE(pi) && GET_OPCODE(pi) <= OP_SHR);
        Protect(luaT_trybinTM(L, s2v(ra), rb, result, पंचांग));
        vmbreak;
      }
      vmcase(OP_MMBINI) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        पूर्णाङ्क imm = GETARG_sB(i);
        TMS पंचांग = (TMS)GETARG_C(i);
        पूर्णाङ्क flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, result, पंचांग));
        vmbreak;
      }
      vmcase(OP_MMBINK) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        TValue *imm = KB(i);
        TMS पंचांग = (TMS)GETARG_C(i);
        पूर्णाङ्क flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, result, पंचांग));
        vmbreak;
      }
      vmcase(OP_UNM) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_संख्या nb;
        यदि (ttisinteger(rb)) {
          lua_Integer ib = ivalue(rb);
          setivalue(s2v(ra), intop(-, 0, ib));
        }
        अन्यथा यदि (tonumberns(rb, nb)) {
          setfltvalue(s2v(ra), luai_numunm(L, nb));
        }
        अन्यथा
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
        vmbreak;
      }
      vmcase(OP_BNOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_Integer ib;
        यदि (tointegerns(rb, &ib)) {
          setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
        }
        अन्यथा
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
        vmbreak;
      }
      vmcase(OP_NOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        यदि (l_isfalse(rb))
          setbtvalue(s2v(ra));
        अन्यथा
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
        पूर्णाङ्क n = GETARG_B(i);  /* number of elements to concatenate */
        L->शीर्ष.p = ra + n;  /* mark the end of concat operands */
        ProtectNT(luaV_concat(L, n));
        checkGC(L, L->शीर्ष.p); /* 'luaV_concat' ensures correct top */
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
        पूर्णाङ्क cond;
        TValue *rb = vRB(i);
        Protect(cond = luaV_equalobj(L, s2v(ra), rb));
        docondjump();
        vmbreak;
      }
      vmcase(OP_LT) {
        op_order(L, l_lti, LTnum, lessthanothers);
        vmbreak;
      }
      vmcase(OP_LE) {
        op_order(L, l_lei, LEnum, lessequalothers);
        vmbreak;
      }
      vmcase(OP_EQK) {
        StkId ra = RA(i);
        TValue *rb = KB(i);
        /* basic types do not use '__eq'; we can use raw equality */
        पूर्णाङ्क cond = luaV_rawequalobj(s2v(ra), rb);
        docondjump();
        vmbreak;
      }
      vmcase(OP_EQI) {
        StkId ra = RA(i);
        पूर्णाङ्क cond;
        पूर्णाङ्क im = GETARG_sB(i);
        यदि (ttisinteger(s2v(ra)))
          cond = (ivalue(s2v(ra)) == im);
        अन्यथा यदि (ttisfloat(s2v(ra)))
          cond = luai_numeq(fltvalue(s2v(ra)), cast_अंक(im));
        अन्यथा
          cond = 0;  /* other types cannot be equal to a number */
        docondjump();
        vmbreak;
      }
      vmcase(OP_LTI) {
        op_orderI(L, l_lti, luai_numlt, 0, TM_LT);
        vmbreak;
      }
      vmcase(OP_LEI) {
        op_orderI(L, l_lei, luai_numle, 0, TM_LE);
        vmbreak;
      }
      vmcase(OP_GTI) {
        op_orderI(L, l_gti, luai_numgt, 1, TM_LT);
        vmbreak;
      }
      vmcase(OP_GEI) {
        op_orderI(L, l_gei, luai_numge, 1, TM_LE);
        vmbreak;
      }
      vmcase(OP_TEST) {
        StkId ra = RA(i);
        पूर्णाङ्क cond = !l_isfalse(s2v(ra));
        docondjump();
        vmbreak;
      }
      vmcase(OP_TESTSET) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        यदि (l_isfalse(rb) == GETARG_k(i))
          pc++;
        अन्यथा {
          setobj2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(OP_CALL) {
        StkId ra = RA(i);
        CallInfo *newci;
        पूर्णाङ्क b = GETARG_B(i);
        पूर्णाङ्क nresults = GETARG_C(i) - 1;
        यदि (b != 0)  /* fixed number of arguments? */
          L->शीर्ष.p = ra + b;  /* top signals number of arguments */
        /* else previous instruction set top */
        savepc(ci);  /* in case of errors */
        यदि ((newci = luaD_precall(L, ra, nresults)) == रिक्त)
          updatetrap(ci);  /* C call; nothing else to be done */
        अन्यथा {  /* Lua call: run function in this same C frame */
          ci = newci;
          गच्छ startfunc;
        }
        vmbreak;
      }
      vmcase(OP_TAILCALL) {
        StkId ra = RA(i);
        पूर्णाङ्क b = GETARG_B(i);  /* number of arguments + 1 (function) */
        पूर्णाङ्क n;  /* number of results when calling a C function */
        पूर्णाङ्क nparams1 = GETARG_C(i);
        /* delta is virtual 'func' - real 'func' (vararg functions) */
        पूर्णाङ्क delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        यदि (b != 0)
          L->शीर्ष.p = ra + b;
        अन्यथा  /* previous instruction set top */
          b = cast_int(L->शीर्ष.p - ra);
        savepc(ci);  /* several calls here can raise errors */
        यदि (TESTARG_k(i)) {
          luaF_closeupval(L, base);  /* close upvalues from current call */
          lua_assert(L->tbclist.p < base);  /* no pending tbc variables */
          lua_assert(base == ci->func.p + 1);
        }
        यदि ((n = luaD_pretailcall(L, ci, ra, b, delta)) < 0)  /* Lua function? */
          गच्छ startfunc;  /* execute the callee */
        अन्यथा {  /* C function? */
          ci->func.p -= delta;  /* restore 'func' (if vararg) */
          luaD_poscall(L, ci, n);  /* finish caller */
          updatetrap(ci);  /* 'luaD_poscall' can change hooks */
          गच्छ ret;  /* caller returns after the tail call */
        }
      }
      vmcase(OP_RETURN) {
        StkId ra = RA(i);
        पूर्णाङ्क n = GETARG_B(i) - 1;  /* number of results */
        पूर्णाङ्क nparams1 = GETARG_C(i);
        यदि (n < 0)  /* not fixed? */
          n = cast_int(L->शीर्ष.p - ra);  /* get what is available */
        savepc(ci);
        यदि (TESTARG_k(i)) {  /* may there be open upvalues? */
          ci->u2.nres = n;  /* save number of returns */
          यदि (L->शीर्ष.p < ci->शीर्ष.p)
            L->शीर्ष.p = ci->शीर्ष.p;
          luaF_close(L, base, CLOSEKTOP, 1);
          updatetrap(ci);
          updatestack(ci);
        }
        यदि (nparams1)  /* vararg function? */
          ci->func.p -= ci->u.l.nextraargs + nparams1;
        L->शीर्ष.p = ra + n;  /* set call for 'luaD_poscall' */
        luaD_poscall(L, ci, n);
        updatetrap(ci);  /* 'luaD_poscall' can change hooks */
        गच्छ ret;
      }
      vmcase(OP_RETURN0) {
        यदि (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->शीर्ष.p = ra;
          savepc(ci);
          luaD_poscall(L, ci, 0);  /* no hurry... */
          trap = 1;
        }
        अन्यथा {  /* do the 'poscall' here */
          पूर्णाङ्क nres = get_nresults(ci->callstatus);
          L->ci = ci->previous;  /* back to caller */
          L->शीर्ष.p = base - 1;
          प्रति (; l_unlikely(nres > 0); nres--)
            setnilvalue(s2v(L->शीर्ष.p++));  /* all results are nil */
        }
        गच्छ ret;
      }
      vmcase(OP_RETURN1) {
        यदि (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->शीर्ष.p = ra + 1;
          savepc(ci);
          luaD_poscall(L, ci, 1);  /* no hurry... */
          trap = 1;
        }
        अन्यथा {  /* do the 'poscall' here */
          पूर्णाङ्क nres = get_nresults(ci->callstatus);
          L->ci = ci->previous;  /* back to caller */
          यदि (nres == 0)
            L->शीर्ष.p = base - 1;  /* asked for no results */
          अन्यथा {
            StkId ra = RA(i);
            setobjs2s(L, base - 1, ra);  /* at least this result */
            L->शीर्ष.p = base;
            प्रति (; l_unlikely(nres > 1); nres--)
              setnilvalue(s2v(L->शीर्ष.p++));  /* complete missing results */
          }
        }
       ret:  /* return from a Lua function */
        यदि (ci->callstatus & CIST_FRESH)
          प्रत्यागम;  /* end this frame */
        अन्यथा {
          ci = ci->previous;
          गच्छ returning;  /* continue running caller in this frame */
        }
      }
      vmcase(OP_FORLOOP) {
        StkId ra = RA(i);
        यदि (ttisinteger(s2v(ra + 1))) {  /* integer loop? */
          lua_Unsigned गिनती = l_castS2U(ivalue(s2v(ra)));
          यदि (गिनती > 0) {  /* still more iterations? */
            lua_Integer step = ivalue(s2v(ra + 1));
            lua_Integer idx = ivalue(s2v(ra + 2));  /* control variable */
            chgivalue(s2v(ra), l_castU2S(गिनती - 1));  /* update counter */
            idx = intop(+, idx, step);  /* add step to index */
            chgivalue(s2v(ra + 2), idx);  /* update control variable */
            pc -= GETARG_Bx(i);  /* jump back */
          }
        }
        अन्यथा यदि (floatforloop(L, ra))  /* float loop */
          pc -= GETARG_Bx(i);  /* jump back */
        updatetrap(ci);  /* allows a signal to break the loop */
        vmbreak;
      }
      vmcase(OP_FORPREP) {
        StkId ra = RA(i);
        savestate(L, ci);  /* in case of errors */
        यदि (forprep(L, ra))
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
       TValue temp;  /* to swap control and closing variables */
       setobj(L, &temp, s2v(ra + 3));
       setobjs2s(L, ra + 3, ra + 2);
       setobj2s(L, ra + 2, &temp);
        /* create to-be-closed upvalue (if closing var. is not nil) */
        halfProtect(luaF_newtbcupval(L, ra + 2));
        pc += GETARG_Bx(i);  /* go to end of the loop */
        i = *(pc++);  /* fetch next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORCALL && ra == RA(i));
        गच्छ l_tforcall;
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
        L->शीर्ष.p = ra + 3 + 3;
        ProtectNT(luaD_call(L, ra + 3, GETARG_C(i)));  /* do the call */
        updatestack(ci);  /* stack may have changed */
        i = *(pc++);  /* go to next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
        गच्छ l_tforloop;
      }}
      vmcase(OP_TFORLOOP) {
       l_tforloop: {
        StkId ra = RA(i);
        यदि (!ttisnil(s2v(ra + 3)))  /* continue loop? */
          pc -= GETARG_Bx(i);  /* jump back */
        vmbreak;
      }}
      vmcase(OP_SETLIST) {
        StkId ra = RA(i);
        अचिन्हित n = cast_uint(GETARG_vB(i));
        अचिन्हित last = cast_uint(GETARG_vC(i));
        Table *h = hvalue(s2v(ra));
        यदि (n == 0)
          n = cast_uint(L->शीर्ष.p - ra) - 1;  /* get up to the top */
        अन्यथा
          L->शीर्ष.p = ci->शीर्ष.p;  /* correct top in case of emergency GC */
        last += n;
        यदि (TESTARG_k(i)) {
          last += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
          pc++;
        }
        /* when 'n' is known, table should have proper size */
        यदि (last > h->asize) {  /* needs more space? */
          /* fixed-size sets should have space preallocated */
          lua_assert(GETARG_vB(i) == 0);
          luaH_resizearray(L, h, last);  /* preallocate it at once */
        }
        प्रति (; n > 0; n--) {
          TValue *val = s2v(ra + n);
          obj2arr(h, last - 1, val);
          last--;
          luaC_barrierback(L, obj2gco(h), val);
        }
        vmbreak;
      }
      vmcase(OP_CLOSURE) {
        StkId ra = RA(i);
        Proto *p = cl->p->p[GETARG_Bx(i)];
        halfProtect(pushclosure(L, p, cl->upvals, base, ra));
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_VARARG) {
        StkId ra = RA(i);
        पूर्णाङ्क n = GETARG_C(i) - 1;  /* required results (-1 means all) */
        पूर्णाङ्क vatab = GETARG_k(i) ? GETARG_B(i) : -1;
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
        यदि (!ttisnil(ra))
          halfProtect(luaG_errnnil(L, cl, GETARG_Bx(i)));
        vmbreak;
      }
      vmcase(OP_VARARGPREP) {
        ProtectNT(luaT_adjustvarargs(L, ci, cl->p));
        यदि (l_unlikely(trap)) {  /* previous "Protect" updated trap */
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
