/*
** $Id: lvm.c $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#define lvm_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"

#அந்தர்பாவ <float.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <math.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lapi.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lopcodes.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "ltm.h"
#அந்தர்பாவ "lvm.h"


/*
** By default, use jump tables in the main interpreter loop on gcc
** and compatible compilers.
*/
#எனில் !defined(LUA_USE_JUMPTABLE)
#எனில் defined(__GNUC__)
#define LUA_USE_JUMPTABLE	1
#இல்லையெனில்
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
#எனில் ((((LUA_MAXINTEGER >> (NBM / 4)) >> (NBM / 4)) >> (NBM / 4)) \
	>> (NBM - (3 * (NBM / 4))))  >  0

/* limit for integers that fit in a float */
#define MAXINTFITSF	((lua_Unsigned)1 << NBM)

/* check whether 'i' is in the interval [-MAXINTFITSF, MAXINTFITSF] */
#define l_intfitsf(i)	((MAXINTFITSF + l_castS2U(i)) <= (2 * MAXINTFITSF))

#இல்லையெனில்  /* all integers fit in a float precisely */

#define l_intfitsf(i)	1

#endif


/*
** Try to convert a value from string to a number value.
** If the value is not a string or is a string not representing
** a valid numeral (or if coercions from strings to numbers
** are disabled via macro 'cvt2num'), do not modify 'result'
** and return 0.
*/
நிலையான முழு l_strton (மாறிலி TValue *obj, TValue *result) {
  lua_assert(obj != result);
  எனில் (!cvt2num(obj))  /* is object not a string? */
    திரும்பு 0;
  இல்லையெனில் {
    TString *st = tsvalue(obj);
    size_t stlen;
    மாறிலி எழுத்து *s = getlstr(st, stlen);
    திரும்பு (luaO_str2num(s, result) == stlen + 1);
  }
}


/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'tonumber'.
*/
முழு luaV_tonumber_ (மாறிலி TValue *obj, lua_எண் *n) {
  TValue v;
  எனில் (ttisinteger(obj)) {
    *n = cast_இலக்கம்(ivalue(obj));
    திரும்பு 1;
  }
  இல்லையெனில் எனில் (l_strton(obj, &v)) {  /* string coercible to number? */
    *n = nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    திரும்பு 1;
  }
  இல்லையெனில்
    திரும்பு 0;  /* conversion failed */
}


/*
** try to convert a float to an integer, rounding according to 'mode'.
*/
முழு luaV_flttointeger (lua_எண் n, lua_Integer *p, F2Imod mode) {
  lua_எண் f = l_floor(n);
  எனில் (n != f) {  /* not an integral value? */
    எனில் (mode == F2Ieq) திரும்பு 0;  /* fails if mode demands integral value */
    இல்லையெனில் எனில் (mode == F2Iceil)  /* needs ceiling? */
      f += 1;  /* convert floor to ceiling (remember: n != f) */
  }
  திரும்பு lua_numbertointeger(f, p);
}


/*
** try to convert a value to an integer, rounding according to 'mode',
** without string coercion.
** ("Fast track" handled by macro 'tointegerns'.)
*/
முழு luaV_tointegerns (மாறிலி TValue *obj, lua_Integer *p, F2Imod mode) {
  எனில் (ttisfloat(obj))
    திரும்பு luaV_flttointeger(fltvalue(obj), p, mode);
  இல்லையெனில் எனில் (ttisinteger(obj)) {
    *p = ivalue(obj);
    திரும்பு 1;
  }
  இல்லையெனில்
    திரும்பு 0;
}


/*
** try to convert a value to an integer.
*/
முழு luaV_tointeger (மாறிலி TValue *obj, lua_Integer *p, F2Imod mode) {
  TValue v;
  எனில் (l_strton(obj, &v))  /* does 'obj' point to a numerical string? */
    obj = &v;  /* change it to point to its corresponding number */
  திரும்பு luaV_tointegerns(obj, p, mode);
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
நிலையான முழு forlimit (lua_State *L, lua_Integer init, மாறிலி TValue *lim,
                                   lua_Integer *p, lua_Integer step) {
  எனில் (!luaV_tointeger(lim, p, (step < 0 ? F2Iceil : F2Ifloor))) {
    /* not coercible to in integer */
    lua_எண் flim;  /* try to convert to float */
    எனில் (!tonumber(lim, &flim)) /* cannot convert to float? */
      luaG_forerror(L, lim, "limit");
    /* else 'flim' is a float out of integer bounds */
    எனில் (luai_numlt(0, flim)) {  /* if it is positive, it is too large */
      எனில் (step < 0) திரும்பு 1;  /* initial value must be less than it */
      *p = LUA_MAXINTEGER;  /* truncate */
    }
    இல்லையெனில் எனில் (luai_numgt(0, flim)) {  /* if it is negative, it is too small */
      எனில் (step > 0) திரும்பு 1;  /* initial value must be greater than it */
      *p = LUA_MININTEGER;  /* truncate */
    }
    இல்லையெனில் திரும்பு 1;  /* limit is NaN (neither 0 nor positive nor negative) */
  }
  திரும்பு (step > 0 ? init > *p : init < *p);  /* not to run? */
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
நிலையான முழு forprep (lua_State *L, StkId ra) {
  TValue *pinit = s2v(ra);
  TValue *plimit = s2v(ra + 1);
  TValue *pstep = s2v(ra + 2);
  எனில் (ttisinteger(pinit) && ttisinteger(pstep)) { /* integer loop? */
    lua_Integer init = ivalue(pinit);
    lua_Integer step = ivalue(pstep);
    lua_Integer limit;
    எனில் (step == 0)
      luaG_runerror(L, "'for' step is zero");
    எனில் (forlimit(L, init, plimit, &limit, step))
      திரும்பு 1;  /* skip the loop */
    இல்லையெனில் {  /* prepare loop counter */
      lua_Unsigned மொத்தம்;
      எனில் (step > 0) {  /* ascending loop? */
        மொத்தம் = l_castS2U(limit) - l_castS2U(init);
        எனில் (step != 1)  /* avoid division in the too common case */
          மொத்தம் /= l_castS2U(step);
      }
      இல்லையெனில் {  /* step < 0; descending loop */
        மொத்தம் = l_castS2U(init) - l_castS2U(limit);
        /* 'step+1' avoids negating 'mininteger' */
        மொத்தம் /= l_castS2U(-(step + 1)) + 1u;
      }
      /* use 'chgivalue' for places that for sure had integers */
      chgivalue(s2v(ra), l_castU2S(மொத்தம்));  /* change init to count */
      setivalue(s2v(ra + 1), step);  /* change limit to step */
      chgivalue(s2v(ra + 2), init);  /* change step to init */
    }
  }
  இல்லையெனில் {  /* try making all values floats */
    lua_எண் init; lua_எண் limit; lua_எண் step;
    எனில் (l_unlikely(!tonumber(plimit, &limit)))
      luaG_forerror(L, plimit, "limit");
    எனில் (l_unlikely(!tonumber(pstep, &step)))
      luaG_forerror(L, pstep, "step");
    எனில் (l_unlikely(!tonumber(pinit, &init)))
      luaG_forerror(L, pinit, "initial value");
    எனில் (step == 0)
      luaG_runerror(L, "'for' step is zero");
    எனில் (!(luai_numge(0, step) ? luai_numge(init, limit)
                              : luai_numge(limit, init)))
      திரும்பு 1;  /* skip the loop */
    இல்லையெனில் {
      /* make sure all values are floats */
      setfltvalue(s2v(ra), limit);
      setfltvalue(s2v(ra + 1), step);
      setfltvalue(s2v(ra + 2), init);  /* control variable */
    }
  }
  திரும்பு 0;
}


/*
** Execute a step of a float numerical for loop, returning
** true iff the loop must continue. (The integer case is
** written inline with opcode OP_FORLOOP, for performance.)
*/
நிலையான முழு floatforloop (lua_State *L, StkId ra) {
  lua_எண் step = fltvalue(s2v(ra + 1));
  lua_எண் limit = fltvalue(s2v(ra));
  lua_எண் idx = fltvalue(s2v(ra + 2));  /* control variable */
  idx = luai_numadd(L, idx, step);  /* increment index */
  எனில் (luai_numlt(0, step) ? luai_numle(idx, limit)
                          : luai_numle(limit, idx)) {
    chgfltvalue(s2v(ra + 2), idx);  /* update control variable */
    திரும்பு 1;  /* jump back */
  }
  இல்லையெனில்
    திரும்பு 0;  /* finish the loop */
}


/*
** Finish the table access 'val = t[key]' and return the tag of the result.
*/
lu_byte luaV_finishget (lua_State *L, மாறிலி TValue *t, TValue *சாவி,
                                      StkId val, lu_byte tag) {
  முழு loop;  /* counter to avoid infinite loops */
  மாறிலி TValue *tm;  /* metamethod */
  க்கு (loop = 0; loop < MAXTAGLOOP; loop++) {
    எனில் (tag == LUA_VNOTABLE) {  /* 't' is not a table? */
      lua_assert(!ttistable(t));
      tm = luaT_gettmbyobj(L, t, TM_INDEX);
      எனில் (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    இல்லையெனில் {  /* 't' is a table */
      tm = fasttm(L, hvalue(t)->metatable, TM_INDEX);  /* table's metamethod */
      எனில் (tm == வெற்று) {  /* no metamethod? */
        setnilvalue2s(val);  /* result is nil */
        திரும்பு LUA_VNIL;
      }
      /* else will try the metamethod */
    }
    எனில் (ttisfunction(tm)) {  /* is metamethod a function? */
      tag = luaT_callTMres(L, tm, t, சாவி, val);  /* call it */
      திரும்பு tag;  /* return tag of the result */
    }
    t = tm;  /* else try to access 'tm[key]' */
    luaV_fastget(t, சாவி, s2v(val), luaH_get, tag);
    எனில் (!tagisempty(tag))
      திரும்பு tag;  /* done */
    /* else repeat (tail call 'luaV_finishget') */
  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
  திரும்பு 0;  /* to avoid warnings */
}


/*
** Finish a table assignment 't[key] = val'.
** About anchoring the table before the call to 'luaH_finishset':
** This call may trigger an emergency collection. When loop>0,
** the table being accessed is a field in some metatable. If this
** metatable is weak and the table is not anchored, this collection
** could collect that table while it is being updated.
*/
வெறுமை luaV_finishset (lua_State *L, மாறிலி TValue *t, TValue *சாவி,
                      TValue *val, முழு hres) {
  முழு loop;  /* counter to avoid infinite loops */
  க்கு (loop = 0; loop < MAXTAGLOOP; loop++) {
    மாறிலி TValue *tm;  /* '__newindex' metamethod */
    எனில் (hres != HNOTATABLE) {  /* is 't' a table? */
      Table *h = hvalue(t);  /* save 't' table */
      tm = fasttm(L, h->metatable, TM_NEWINDEX);  /* get metamethod */
      எனில் (tm == வெற்று) {  /* no metamethod? */
        sethvalue2s(L, L->உச்சி.p, h);  /* anchor 't' */
        L->உச்சி.p++;  /* assume EXTRA_STACK */
        luaH_finishset(L, h, சாவி, val, hres);  /* set new value */
        L->உச்சி.p--;
        invalidateTMcache(h);
        luaC_barrierback(L, obj2gco(h), val);
        திரும்பு;
      }
      /* else will try the metamethod */
    }
    இல்லையெனில் {  /* not a table; check metamethod */
      tm = luaT_gettmbyobj(L, t, TM_NEWINDEX);
      எனில் (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");
    }
    /* try the metamethod */
    எனில் (ttisfunction(tm)) {
      luaT_callTM(L, tm, t, சாவி, val);
      திரும்பு;
    }
    t = tm;  /* else must repeat assignment over 'tm' */
    /* do the equivalent to 'luaV_fastset', but saving 'h' */
    எனில் (!ttistable(t))
      hres = HNOTATABLE;
    இல்லையெனில் {
      Table *h = hvalue(t);  /* next call can change the value at 't' */
      hres = luaH_pset(h, சாவி, val);
      எனில் (hres == HOK) {
        luaC_barrierback(L, obj2gco(h), val);  /* luaV_finishfastset */
        திரும்பு;  /* done */
      }
    }
    /* else 'return luaV_finishset(L, t, key, val, hres)' (loop) */
  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}


/*
** Function to be used for 0-terminated string order comparison
*/
#எனில் !defined(l_strcoll)
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
நிலையான முழு l_strcmp (மாறிலி TString *ts1, மாறிலி TString *ts2) {
  size_t rl1;  /* real length */
  மாறிலி எழுத்து *s1 = getlstr(ts1, rl1);
  size_t rl2;
  மாறிலி எழுத்து *s2 = getlstr(ts2, rl2);
  க்கு (;;) {  /* for each segment */
    முழு temp = l_strcoll(s1, s2);
    எனில் (temp != 0)  /* not equal? */
      திரும்பு temp;  /* done */
    இல்லையெனில் {  /* strings are equal up to a '\0' */
      size_t zl1 = strlen(s1);  /* index of first '\0' in 's1' */
      size_t zl2 = strlen(s2);  /* index of first '\0' in 's2' */
      எனில் (zl2 == rl2)  /* 's2' is finished? */
        திரும்பு (zl1 == rl1) ? 0 : 1;  /* check 's1' */
      இல்லையெனில் எனில் (zl1 == rl1)  /* 's1' is finished? */
        திரும்பு -1;  /* 's1' is less than 's2' ('s2' is not finished) */
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
l_sinline முழு LTintfloat (lua_Integer i, lua_எண் f) {
  எனில் (l_intfitsf(i))
    திரும்பு luai_numlt(cast_இலக்கம்(i), f);  /* compare them as floats */
  இல்லையெனில் {  /* i < f <=> i < ceil(f) */
    lua_Integer fi;
    எனில் (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      திரும்பு i < fi;   /* compare them as integers */
    இல்லையெனில்  /* 'f' is either greater or less than all integers */
      திரும்பு f > 0;  /* greater? */
  }
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
l_sinline முழு LEintfloat (lua_Integer i, lua_எண் f) {
  எனில் (l_intfitsf(i))
    திரும்பு luai_numle(cast_இலக்கம்(i), f);  /* compare them as floats */
  இல்லையெனில் {  /* i <= f <=> i <= floor(f) */
    lua_Integer fi;
    எனில் (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      திரும்பு i <= fi;   /* compare them as integers */
    இல்லையெனில்  /* 'f' is either greater or less than all integers */
      திரும்பு f > 0;  /* greater? */
  }
}


/*
** Check whether float 'f' is less than integer 'i'.
** See comments on previous function.
*/
l_sinline முழு LTfloatint (lua_எண் f, lua_Integer i) {
  எனில் (l_intfitsf(i))
    திரும்பு luai_numlt(f, cast_இலக்கம்(i));  /* compare them as floats */
  இல்லையெனில் {  /* f < i <=> floor(f) < i */
    lua_Integer fi;
    எனில் (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      திரும்பு fi < i;   /* compare them as integers */
    இல்லையெனில்  /* 'f' is either greater or less than all integers */
      திரும்பு f < 0;  /* less? */
  }
}


/*
** Check whether float 'f' is less than or equal to integer 'i'.
** See comments on previous function.
*/
l_sinline முழு LEfloatint (lua_எண் f, lua_Integer i) {
  எனில் (l_intfitsf(i))
    திரும்பு luai_numle(f, cast_இலக்கம்(i));  /* compare them as floats */
  இல்லையெனில் {  /* f <= i <=> ceil(f) <= i */
    lua_Integer fi;
    எனில் (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      திரும்பு fi <= i;   /* compare them as integers */
    இல்லையெனில்  /* 'f' is either greater or less than all integers */
      திரும்பு f < 0;  /* less? */
  }
}


/*
** Return 'l < r', for numbers.
*/
l_sinline முழு LTnum (மாறிலி TValue *l, மாறிலி TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  எனில் (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    எனில் (ttisinteger(r))
      திரும்பு li < ivalue(r);  /* both are integers */
    இல்லையெனில்  /* 'l' is int and 'r' is float */
      திரும்பு LTintfloat(li, fltvalue(r));  /* l < r ? */
  }
  இல்லையெனில் {
    lua_எண் lf = fltvalue(l);  /* 'l' must be float */
    எனில் (ttisfloat(r))
      திரும்பு luai_numlt(lf, fltvalue(r));  /* both are float */
    இல்லையெனில்  /* 'l' is float and 'r' is int */
      திரும்பு LTfloatint(lf, ivalue(r));
  }
}


/*
** Return 'l <= r', for numbers.
*/
l_sinline முழு LEnum (மாறிலி TValue *l, மாறிலி TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  எனில் (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    எனில் (ttisinteger(r))
      திரும்பு li <= ivalue(r);  /* both are integers */
    இல்லையெனில்  /* 'l' is int and 'r' is float */
      திரும்பு LEintfloat(li, fltvalue(r));  /* l <= r ? */
  }
  இல்லையெனில் {
    lua_எண் lf = fltvalue(l);  /* 'l' must be float */
    எனில் (ttisfloat(r))
      திரும்பு luai_numle(lf, fltvalue(r));  /* both are float */
    இல்லையெனில்  /* 'l' is float and 'r' is int */
      திரும்பு LEfloatint(lf, ivalue(r));
  }
}


/*
** return 'l < r' for non-numbers.
*/
நிலையான முழு lessthanothers (lua_State *L, மாறிலி TValue *l, மாறிலி TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  எனில் (ttisstring(l) && ttisstring(r))  /* both are strings? */
    திரும்பு l_strcmp(tsvalue(l), tsvalue(r)) < 0;
  இல்லையெனில்
    திரும்பு luaT_callorderTM(L, l, r, TM_LT);
}


/*
** Main operation less than; return 'l < r'.
*/
முழு luaV_lessthan (lua_State *L, மாறிலி TValue *l, மாறிலி TValue *r) {
  எனில் (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    திரும்பு LTnum(l, r);
  இல்லையெனில் திரும்பு lessthanothers(L, l, r);
}


/*
** return 'l <= r' for non-numbers.
*/
நிலையான முழு lessequalothers (lua_State *L, மாறிலி TValue *l, மாறிலி TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  எனில் (ttisstring(l) && ttisstring(r))  /* both are strings? */
    திரும்பு l_strcmp(tsvalue(l), tsvalue(r)) <= 0;
  இல்லையெனில்
    திரும்பு luaT_callorderTM(L, l, r, TM_LE);
}


/*
** Main operation less than or equal to; return 'l <= r'.
*/
முழு luaV_lessequal (lua_State *L, மாறிலி TValue *l, மாறிலி TValue *r) {
  எனில் (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    திரும்பு LEnum(l, r);
  இல்லையெனில் திரும்பு lessequalothers(L, l, r);
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
முழு luaV_equalobj (lua_State *L, மாறிலி TValue *t1, மாறிலி TValue *t2) {
  மாறிலி TValue *tm;
  எனில் (ttype(t1) != ttype(t2))  /* not the same type? */
    திரும்பு 0;
  இல்லையெனில் எனில் (ttypetag(t1) != ttypetag(t2)) {
    தேர்வு (ttypetag(t1)) {
      நிலை LUA_VNUMINT: {  /* integer == float? */
        /* integer and float can only be equal if float has an integer
           value equal to the integer */
        lua_Integer i2;
        திரும்பு (luaV_flttointeger(fltvalue(t2), &i2, F2Ieq) &&
                ivalue(t1) == i2);
      }
      நிலை LUA_VNUMFLT: {  /* float == integer? */
        lua_Integer i1;  /* see comment in previous case */
        திரும்பு (luaV_flttointeger(fltvalue(t1), &i1, F2Ieq) &&
                i1 == ivalue(t2));
      }
      நிலை LUA_VSHRSTR: நிலை LUA_VLNGSTR: {
        /* compare two strings with different variants: they can be
           equal when one string is a short string and the other is
           an external string  */
        திரும்பு luaS_eqstr(tsvalue(t1), tsvalue(t2));
      }
      இயல்பு:
        /* only numbers (integer/float) and strings (long/short) can have
           equal values with different variants */
        திரும்பு 0;
    }
  }
  இல்லையெனில் {  /* equal variants */
    தேர்வு (ttypetag(t1)) {
      நிலை LUA_VNIL: நிலை LUA_VFALSE: நிலை LUA_VTRUE:
        திரும்பு 1;
      நிலை LUA_VNUMINT:
        திரும்பு (ivalue(t1) == ivalue(t2));
      நிலை LUA_VNUMFLT:
        திரும்பு (fltvalue(t1) == fltvalue(t2));
      நிலை LUA_VLIGHTUSERDATA: திரும்பு pvalue(t1) == pvalue(t2);
      நிலை LUA_VSHRSTR:
        திரும்பு eqshrstr(tsvalue(t1), tsvalue(t2));
      நிலை LUA_VLNGSTR:
        திரும்பு luaS_eqstr(tsvalue(t1), tsvalue(t2));
      நிலை LUA_VUSERDATA: {
        எனில் (uvalue(t1) == uvalue(t2)) திரும்பு 1;
        இல்லையெனில் எனில் (L == வெற்று) திரும்பு 0;
        tm = fasttm(L, uvalue(t1)->metatable, TM_EQ);
        எனில் (tm == வெற்று)
          tm = fasttm(L, uvalue(t2)->metatable, TM_EQ);
        நிறுத்து;  /* will try TM */
      }
      நிலை LUA_VTABLE: {
        எனில் (hvalue(t1) == hvalue(t2)) திரும்பு 1;
        இல்லையெனில் எனில் (L == வெற்று) திரும்பு 0;
        tm = fasttm(L, hvalue(t1)->metatable, TM_EQ);
        எனில் (tm == வெற்று)
          tm = fasttm(L, hvalue(t2)->metatable, TM_EQ);
        நிறுத்து;  /* will try TM */
      }
      நிலை LUA_VLCF:
        திரும்பு (fvalue(t1) == fvalue(t2));
      இயல்பு:  /* functions and threads */
        திரும்பு (gcvalue(t1) == gcvalue(t2));
    }
    எனில் (tm == வெற்று)  /* no TM? */
      திரும்பு 0;  /* objects are different */
    இல்லையெனில் {
      முழு tag = luaT_callTMres(L, tm, t1, t2, L->உச்சி.p);  /* call TM */
      திரும்பு !tagisfalse(tag);
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
நிலையான வெறுமை copy2buff (StkId உச்சி, முழு n, எழுத்து *buff) {
  size_t tl = 0;  /* size already copied */
  செய் {
    TString *st = tsvalue(s2v(உச்சி - n));
    size_t l;  /* length of string being copied */
    மாறிலி எழுத்து *s = getlstr(st, l);
    memcpy(buff + tl, s, l * அளவு(எழுத்து));
    tl += l;
  } போது (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top.p - total' up to 'L->top.p - 1'.
*/
வெறுமை luaV_concat (lua_State *L, முழு total) {
  எனில் (total == 1)
    திரும்பு;  /* "all" values already concatenated */
  செய் {
    StkId உச்சி = L->உச்சி.p;
    முழு n = 2;  /* number of elements handled in this pass (at least 2) */
    எனில் (!(ttisstring(s2v(உச்சி - 2)) || cvt2str(s2v(உச்சி - 2))) ||
        !tostring(L, s2v(உச்சி - 1)))
      luaT_tryconcatTM(L);  /* may invalidate 'top' */
    இல்லையெனில் எனில் (isemptystr(s2v(உச்சி - 1)))  /* second operand is empty? */
      cast_void(tostring(L, s2v(உச்சி - 2)));  /* result is first operand */
    இல்லையெனில் எனில் (isemptystr(s2v(உச்சி - 2))) {  /* first operand is empty string? */
      setobjs2s(L, உச்சி - 2, உச்சி - 1);  /* result is second op. */
    }
    இல்லையெனில் {
      /* at least two string values; get as many as possible */
      size_t tl = tsslen(tsvalue(s2v(உச்சி - 1)));  /* total length */
      TString *ts;
      /* collect total length and number of strings */
      க்கு (n = 1; n < total && tostring(L, s2v(உச்சி - n - 1)); n++) {
        size_t l = tsslen(tsvalue(s2v(உச்சி - n - 1)));
        எனில் (l_unlikely(l >= MAX_SIZE - அளவு(TString) - tl)) {
          L->உச்சி.p = உச்சி - total;  /* pop strings to avoid wasting stack */
          luaG_runerror(L, "string length overflow");
        }
        tl += l;
      }
      எனில் (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        எழுத்து buff[LUAI_MAXSHORTLEN];
        copy2buff(உச்சி, n, buff);  /* copy strings to buffer */
        ts = luaS_newlstr(L, buff, tl);
      }
      இல்லையெனில் {  /* long string; copy strings directly to final result */
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(உச்சி, n, getlngstr(ts));
      }
      setsvalue2s(L, உச்சி - n, ts);  /* create result */
    }
    total -= n - 1;  /* got 'n' strings to create one new */
    L->உச்சி.p -= n - 1;  /* popped 'n' strings and pushed one */
  } போது (total > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra = #rb'.
*/
வெறுமை luaV_objlen (lua_State *L, StkId ra, மாறிலி TValue *rb) {
  மாறிலி TValue *tm;
  தேர்வு (ttypetag(rb)) {
    நிலை LUA_VTABLE: {
      Table *h = hvalue(rb);
      tm = fasttm(L, h->metatable, TM_LEN);
      எனில் (tm) நிறுத்து;  /* metamethod? break switch to call it */
      setivalue(s2v(ra), l_castU2S(luaH_getn(L, h)));  /* else primitive len */
      திரும்பு;
    }
    நிலை LUA_VSHRSTR: {
      setivalue(s2v(ra), tsvalue(rb)->shrlen);
      திரும்பு;
    }
    நிலை LUA_VLNGSTR: {
      setivalue(s2v(ra), cast_st2S(tsvalue(rb)->u.lnglen));
      திரும்பு;
    }
    இயல்பு: {  /* try metamethod */
      tm = luaT_gettmbyobj(L, rb, TM_LEN);
      எனில் (l_unlikely(notm(tm)))  /* no metamethod? */
        luaG_typeerror(L, rb, "get length of");
      நிறுத்து;
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
  எனில் (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    எனில் (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    திரும்பு intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  இல்லையெனில் {
    lua_Integer q = m / n;  /* perform C division */
    எனில் ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    திரும்பு q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_idiv.)
*/
lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  எனில் (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    எனில் (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    திரும்பு 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  இல்லையெனில் {
    lua_Integer r = m % n;
    எனில் (r != 0 && (r ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    திரும்பு r;
  }
}


/*
** Float modulus
*/
lua_எண் luaV_modf (lua_State *L, lua_எண் m, lua_எண் n) {
  lua_எண் r;
  luai_nummod(L, m, n, r);
  திரும்பு r;
}


/* number of bits in an integer */
#define NBITS	l_numbits(lua_Integer)


/*
** Shift left operation. (Shift right just negates 'y'.)
*/
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  எனில் (y < 0) {  /* shift right? */
    எனில் (y <= -NBITS) திரும்பு 0;
    இல்லையெனில் திரும்பு intop(>>, x, -y);
  }
  இல்லையெனில் {  /* shift left */
    எனில் (y >= NBITS) திரும்பு 0;
    இல்லையெனில் திரும்பு intop(<<, x, y);
  }
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues.
*/
நிலையான வெறுமை pushclosure (lua_State *L, Proto *p, UpVal **encup, StkId base,
                         StkId ra) {
  முழு nup = p->sizeupvalues;
  Upvaldesc *uv = p->upvalues;
  முழு i;
  LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  setclLvalue2s(L, ra, ncl);  /* anchor new closure in stack */
  க்கு (i = 0; i < nup; i++) {  /* fill in its upvalues */
    எனில் (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = luaF_findupval(L, base + uv[i].idx);
    இல்லையெனில்  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    luaC_objbarrier(L, ncl, ncl->upvals[i]);
  }
}


/*
** finish execution of an opcode interrupted by a yield
*/
வெறுமை luaV_finishOp (lua_State *L) {
  CallInfo *ci = L->ci;
  StkId base = ci->func.p + 1;
  Instruction inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  OpCode op = GET_OPCODE(inst);
  தேர்வு (op) {  /* finish its execution */
    நிலை OP_MMBIN: நிலை OP_MMBINI: நிலை OP_MMBINK: {
      setobjs2s(L, base + GETARG_A(*(ci->u.l.savedpc - 2)), --L->உச்சி.p);
      நிறுத்து;
    }
    நிலை OP_UNM: நிலை OP_BNOT: நிலை OP_LEN:
    நிலை OP_GETTABUP: நிலை OP_GETTABLE: நிலை OP_GETI:
    நிலை OP_GETFIELD: நிலை OP_SELF: {
      setobjs2s(L, base + GETARG_A(inst), --L->உச்சி.p);
      நிறுத்து;
    }
    நிலை OP_LT: நிலை OP_LE:
    நிலை OP_LTI: நிலை OP_LEI:
    நிலை OP_GTI: நிலை OP_GEI:
    நிலை OP_EQ: {  /* note that 'OP_EQI'/'OP_EQK' cannot yield */
      முழு res = !l_isfalse(s2v(L->உச்சி.p - 1));
      L->உச்சி.p--;
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_JMP);
      எனில் (res != GETARG_k(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      நிறுத்து;
    }
    நிலை OP_CONCAT: {
      StkId உச்சி = L->உச்சி.p - 1;  /* top when 'luaT_tryconcatTM' was called */
      முழு a = GETARG_A(inst);      /* first element to concatenate */
      முழு total = cast_int(உச்சி - 1 - (base + a));  /* yet to concatenate */
      setobjs2s(L, உச்சி - 2, உச்சி);  /* put TM result in proper position */
      L->உச்சி.p = உச்சி - 1;  /* top is one after last element (at top-2) */
      luaV_concat(L, total);  /* concat them (may yield again) */
      நிறுத்து;
    }
    நிலை OP_CLOSE: {  /* yielded closing variables */
      ci->u.l.savedpc--;  /* repeat instruction to close other vars. */
      நிறுத்து;
    }
    நிலை OP_RETURN: {  /* yielded closing variables */
      StkId ra = base + GETARG_A(inst);
      /* adjust top to signal correct number of returns, in case the
         return is "up to top" ('isIT') */
      L->உச்சி.p = ra + ci->u2.nres;
      /* repeat instruction to close other vars. and complete the return */
      ci->u.l.savedpc--;
      நிறுத்து;
    }
    இயல்பு: {
      /* only these other opcodes can yield */
      lua_assert(op == OP_TFORCALL || op == OP_CALL ||
           op == OP_TAILCALL || op == OP_SETTABUP || op == OP_SETTABLE ||
           op == OP_SETI || op == OP_SETFIELD);
      நிறுத்து;
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
  முழு imm = GETARG_sC(i);  \
  எனில் (ttisinteger(v1)) {  \
    lua_Integer iv1 = ivalue(v1);  \
    pc++; setivalue(ra, iop(L, iv1, imm));  \
  }  \
  இல்லையெனில் எனில் (ttisfloat(v1)) {  \
    lua_எண் nb = fltvalue(v1);  \
    lua_எண் fimm = cast_இலக்கம்(imm);  \
    pc++; setfltvalue(ra, fop(L, nb, fimm)); \
  }}


/*
** Auxiliary function for arithmetic operations over floats and others
** with two operands.
*/
#define op_arithf_aux(L,v1,v2,fop) {  \
  lua_எண் n1; lua_எண் n2;  \
  எனில் (tonumberns(v1, n1) && tonumberns(v2, n2)) {  \
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
  எனில் (ttisinteger(v1) && ttisinteger(v2)) {  \
    StkId ra = RA(i); \
    lua_Integer i1 = ivalue(v1); lua_Integer i2 = ivalue(v2);  \
    pc++; setivalue(s2v(ra), iop(L, i1, i2));  \
  }  \
  இல்லையெனில் op_arithf_aux(L, v1, v2, fop); }


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
  எனில் (tointegerns(v1, &i1)) {  \
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
  எனில் (tointegerns(v1, &i1) && tointegerns(v2, &i2)) {  \
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
  முழு cond;  \
  TValue *rb = vRB(i);  \
  எனில் (ttisinteger(ra) && ttisinteger(rb)) {  \
    lua_Integer ia = ivalue(ra);  \
    lua_Integer ib = ivalue(rb);  \
    cond = opi(ia, ib);  \
  }  \
  இல்லையெனில் எனில் (ttisnumber(ra) && ttisnumber(rb))  \
    cond = opn(ra, rb);  \
  இல்லையெனில்  \
    Protect(cond = other(L, ra, rb));  \
  docondjump(); }


/*
** Order operations with immediate operand. (Immediate operand is
** always small enough to have an exact representation as a float.)
*/
#define op_orderI(L,opi,opf,inv,tm) {  \
  TValue *ra = vRA(i); \
  முழு cond;  \
  முழு im = GETARG_sB(i);  \
  எனில் (ttisinteger(ra))  \
    cond = opi(ivalue(ra), im);  \
  இல்லையெனில் எனில் (ttisfloat(ra)) {  \
    lua_எண் fa = fltvalue(ra);  \
    lua_எண் fim = cast_இலக்கம்(im);  \
    cond = opf(fa, fim);  \
  }  \
  இல்லையெனில் {  \
    முழு isf = GETARG_C(i);  \
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
	{ எனில் (l_unlikely(trap)) { updatebase(ci); ra = RA(i); } }


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
#define docondjump()	எனில் (cond != GETARG_k(i)) pc++; இல்லையெனில் donextjump(ci);


/*
** Correct global 'pc'.
*/
#define savepc(ci)	(ci->u.l.savedpc = pc)


/*
** Whenever code can raise errors, the global 'pc' and the global
** 'top' must be correct to report occasional errors.
*/
#define savestate(L,ci)		(savepc(ci), L->உச்சி.p = ci->உச்சி.p)


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
#எனில் !defined(luai_threadyield)
#define luai_threadyield(L)	{lua_unlock(L); lua_lock(L);}
#endif

/* 'c' is the limit of live values in the stack */
#define checkGC(L,c)  \
	{ luaC_condGC(L, (savepc(ci), L->உச்சி.p = (c)), \
                         updatetrap(ci)); \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#define vmfetch()	{ \
  எனில் (l_unlikely(trap)) {  /* stack reallocation or hooks? */ \
    trap = luaG_traceexec(L, pc);  /* handle hooks */ \
    updatebase(ci);  /* correct stack */ \
  } \
  i = *(pc++); \
}

#define vmdispatch(o)	தேர்வு(o)
#define vmcase(l)	நிலை l:
#define vmbreak		நிறுத்து


வெறுமை luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId base;
  மாறிலி Instruction *pc;
  முழு trap;
#எனில் LUA_USE_JUMPTABLE
#அந்தர்பாவ "ljumptab.h"
#endif
 startfunc:
  trap = cast_int(L->hookmask);
 returning:  /* trap already set */
  cl = ci_func(ci);
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  எனில் (l_unlikely(trap))
    trap = luaG_tracecall(L);
  base = ci->func.p + 1;
  /* main loop of interpreter */
  க்கு (;;) {
    Instruction i;  /* instruction being executed */
    vmfetch();
    #எனில் 0
    { /* low-level line tracing for debugging Lua */
      #அந்தர்பாவ "lopnames.h"
      முழு pcrel = pcRel(pc, cl->p);
      printf("line: %d; %s (%d)\n", luaG_getfuncline(cl->p, pcrel),
             opnames[GET_OPCODE(i)], pcrel);
    }
    #endif
    lua_assert(base == ci->func.p + 1);
    lua_assert(base <= L->உச்சி.p && L->உச்சி.p <= L->அடுக்கு_last.p);
    /* for tests, invalidate top for instructions not expecting it */
    lua_assert(luaP_isIT(i) || (cast_void(L->உச்சி.p = base), 1));
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
        முழு b = GETARG_sBx(i);
        setfltvalue(s2v(ra), cast_இலக்கம்(b));
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
        முழு b = GETARG_B(i);
        செய் {
          setnilvalue(s2v(ra++));
        } போது (b--);
        vmbreak;
      }
      vmcase(OP_GETUPVAL) {
        StkId ra = RA(i);
        முழு b = GETARG_B(i);
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
        TString *சாவி = tsvalue(rc);  /* key must be a short string */
        lu_byte tag;
        luaV_fastget(upval, சாவி, s2v(ra), luaH_getshortstr, tag);
        எனில் (tagisempty(tag))
          Protect(luaV_finishget(L, upval, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_GETTABLE) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = vRC(i);
        lu_byte tag;
        எனில் (ttisinteger(rc)) {  /* fast track for integers? */
          luaV_fastgeti(rb, ivalue(rc), s2v(ra), tag);
        }
        இல்லையெனில்
          luaV_fastget(rb, rc, s2v(ra), luaH_get, tag);
        எனில் (tagisempty(tag))
          Protect(luaV_finishget(L, rb, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_GETI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        முழு c = GETARG_C(i);
        lu_byte tag;
        luaV_fastgeti(rb, c, s2v(ra), tag);
        எனில் (tagisempty(tag)) {
          TValue சாவி;
          setivalue(&சாவி, c);
          Protect(luaV_finishget(L, rb, &சாவி, ra, tag));
        }
        vmbreak;
      }
      vmcase(OP_GETFIELD) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *சாவி = tsvalue(rc);  /* key must be a short string */
        lu_byte tag;
        luaV_fastget(rb, சாவி, s2v(ra), luaH_getshortstr, tag);
        எனில் (tagisempty(tag))
          Protect(luaV_finishget(L, rb, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_SETTABUP) {
        முழு hres;
        TValue *upval = cl->upvals[GETARG_A(i)]->v.p;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *சாவி = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(upval, சாவி, rc, hres, luaH_psetshortstr);
        எனில் (hres == HOK)
          luaV_finishfastset(L, upval, rc);
        இல்லையெனில்
          Protect(luaV_finishset(L, upval, rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETTABLE) {
        StkId ra = RA(i);
        முழு hres;
        TValue *rb = vRB(i);  /* key (table is in 'ra') */
        TValue *rc = RKC(i);  /*மதிப்பு*/
        எனில் (ttisinteger(rb)) {  /* fast track for integers? */
          luaV_fastseti(s2v(ra), ivalue(rb), rc, hres);
        }
        இல்லையெனில் {
          luaV_fastset(s2v(ra), rb, rc, hres, luaH_pset);
        }
        எனில் (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        இல்லையெனில்
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETI) {
        StkId ra = RA(i);
        முழு hres;
        முழு b = GETARG_B(i);
        TValue *rc = RKC(i);
        luaV_fastseti(s2v(ra), b, rc, hres);
        எனில் (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        இல்லையெனில் {
          TValue சாவி;
          setivalue(&சாவி, b);
          Protect(luaV_finishset(L, s2v(ra), &சாவி, rc, hres));
        }
        vmbreak;
      }
      vmcase(OP_SETFIELD) {
        StkId ra = RA(i);
        முழு hres;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *சாவி = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(s2v(ra), சாவி, rc, hres, luaH_psetshortstr);
        எனில் (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        இல்லையெனில்
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_NEWTABLE) {
        StkId ra = RA(i);
        unsigned b = cast_uint(GETARG_vB(i));  /* log2(hash size) + 1 */
        unsigned c = cast_uint(GETARG_vC(i));  /* array size */
        Table *t;
        எனில் (b > 0)
          b = 1u << (b - 1);  /* hash size is 2^(b - 1) */
        எனில் (TESTARG_k(i)) {  /* non-zero extra argument? */
          lua_assert(GETARG_Ax(*pc) != 0);
          /* add it to array size */
          c += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
        }
        pc++;  /* skip extra argument */
        L->உச்சி.p = ra + 1;  /* correct top in case of emergency GC */
        t = luaH_new(L);  /* memory allocation */
        sethvalue2s(L, ra, t);
        எனில் (b != 0 || c != 0)
          luaH_resize(L, t, c, b);  /* idem */
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_SELF) {
        StkId ra = RA(i);
        lu_byte tag;
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *சாவி = tsvalue(rc);  /* key must be a short string */
        setobj2s(L, ra + 1, rb);
        luaV_fastget(rb, சாவி, s2v(ra), luaH_getshortstr, tag);
        எனில் (tagisempty(tag))
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
        முழு ic = GETARG_sC(i);
        lua_Integer ib;
        எனில் (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ic, ib));
        }
        vmbreak;
      }
      vmcase(OP_SHRI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        முழு ic = GETARG_sC(i);
        lua_Integer ib;
        எனில் (tointegerns(rb, &ib)) {
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
        TMS tm = (TMS)GETARG_C(i);
        StkId result = RA(pi);
        lua_assert(OP_ADD <= GET_OPCODE(pi) && GET_OPCODE(pi) <= OP_SHR);
        Protect(luaT_trybinTM(L, s2v(ra), rb, result, tm));
        vmbreak;
      }
      vmcase(OP_MMBINI) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        முழு imm = GETARG_sB(i);
        TMS tm = (TMS)GETARG_C(i);
        முழு flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, result, tm));
        vmbreak;
      }
      vmcase(OP_MMBINK) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        TValue *imm = KB(i);
        TMS tm = (TMS)GETARG_C(i);
        முழு flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, result, tm));
        vmbreak;
      }
      vmcase(OP_UNM) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_எண் nb;
        எனில் (ttisinteger(rb)) {
          lua_Integer ib = ivalue(rb);
          setivalue(s2v(ra), intop(-, 0, ib));
        }
        இல்லையெனில் எனில் (tonumberns(rb, nb)) {
          setfltvalue(s2v(ra), luai_numunm(L, nb));
        }
        இல்லையெனில்
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
        vmbreak;
      }
      vmcase(OP_BNOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_Integer ib;
        எனில் (tointegerns(rb, &ib)) {
          setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
        }
        இல்லையெனில்
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
        vmbreak;
      }
      vmcase(OP_NOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        எனில் (l_isfalse(rb))
          setbtvalue(s2v(ra));
        இல்லையெனில்
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
        முழு n = GETARG_B(i);  /* number of elements to concatenate */
        L->உச்சி.p = ra + n;  /* mark the end of concat operands */
        ProtectNT(luaV_concat(L, n));
        checkGC(L, L->உச்சி.p); /* 'luaV_concat' ensures correct top */
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
        முழு cond;
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
        முழு cond = luaV_rawequalobj(s2v(ra), rb);
        docondjump();
        vmbreak;
      }
      vmcase(OP_EQI) {
        StkId ra = RA(i);
        முழு cond;
        முழு im = GETARG_sB(i);
        எனில் (ttisinteger(s2v(ra)))
          cond = (ivalue(s2v(ra)) == im);
        இல்லையெனில் எனில் (ttisfloat(s2v(ra)))
          cond = luai_numeq(fltvalue(s2v(ra)), cast_இலக்கம்(im));
        இல்லையெனில்
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
        முழு cond = !l_isfalse(s2v(ra));
        docondjump();
        vmbreak;
      }
      vmcase(OP_TESTSET) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        எனில் (l_isfalse(rb) == GETARG_k(i))
          pc++;
        இல்லையெனில் {
          setobj2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(OP_CALL) {
        StkId ra = RA(i);
        CallInfo *newci;
        முழு b = GETARG_B(i);
        முழு nresults = GETARG_C(i) - 1;
        எனில் (b != 0)  /* fixed number of arguments? */
          L->உச்சி.p = ra + b;  /* top signals number of arguments */
        /* else previous instruction set top */
        savepc(ci);  /* in case of errors */
        எனில் ((newci = luaD_precall(L, ra, nresults)) == வெற்று)
          updatetrap(ci);  /* C call; nothing else to be done */
        இல்லையெனில் {  /* Lua call: run function in this same C frame */
          ci = newci;
          செல் startfunc;
        }
        vmbreak;
      }
      vmcase(OP_TAILCALL) {
        StkId ra = RA(i);
        முழு b = GETARG_B(i);  /* number of arguments + 1 (function) */
        முழு n;  /* number of results when calling a C function */
        முழு nparams1 = GETARG_C(i);
        /* delta is virtual 'func' - real 'func' (vararg functions) */
        முழு delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        எனில் (b != 0)
          L->உச்சி.p = ra + b;
        இல்லையெனில்  /* previous instruction set top */
          b = cast_int(L->உச்சி.p - ra);
        savepc(ci);  /* several calls here can raise errors */
        எனில் (TESTARG_k(i)) {
          luaF_closeupval(L, base);  /* close upvalues from current call */
          lua_assert(L->tbclist.p < base);  /* no pending tbc variables */
          lua_assert(base == ci->func.p + 1);
        }
        எனில் ((n = luaD_pretailcall(L, ci, ra, b, delta)) < 0)  /* Lua function? */
          செல் startfunc;  /* execute the callee */
        இல்லையெனில் {  /* C function? */
          ci->func.p -= delta;  /* restore 'func' (if vararg) */
          luaD_poscall(L, ci, n);  /* finish caller */
          updatetrap(ci);  /* 'luaD_poscall' can change hooks */
          செல் ret;  /* caller returns after the tail call */
        }
      }
      vmcase(OP_RETURN) {
        StkId ra = RA(i);
        முழு n = GETARG_B(i) - 1;  /* number of results */
        முழு nparams1 = GETARG_C(i);
        எனில் (n < 0)  /* not fixed? */
          n = cast_int(L->உச்சி.p - ra);  /* get what is available */
        savepc(ci);
        எனில் (TESTARG_k(i)) {  /* may there be open upvalues? */
          ci->u2.nres = n;  /* save number of returns */
          எனில் (L->உச்சி.p < ci->உச்சி.p)
            L->உச்சி.p = ci->உச்சி.p;
          luaF_close(L, base, CLOSEKTOP, 1);
          updatetrap(ci);
          updatestack(ci);
        }
        எனில் (nparams1)  /* vararg function? */
          ci->func.p -= ci->u.l.nextraargs + nparams1;
        L->உச்சி.p = ra + n;  /* set call for 'luaD_poscall' */
        luaD_poscall(L, ci, n);
        updatetrap(ci);  /* 'luaD_poscall' can change hooks */
        செல் ret;
      }
      vmcase(OP_RETURN0) {
        எனில் (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->உச்சி.p = ra;
          savepc(ci);
          luaD_poscall(L, ci, 0);  /* no hurry... */
          trap = 1;
        }
        இல்லையெனில் {  /* do the 'poscall' here */
          முழு nres = get_nresults(ci->callstatus);
          L->ci = ci->previous;  /* back to caller */
          L->உச்சி.p = base - 1;
          க்கு (; l_unlikely(nres > 0); nres--)
            setnilvalue(s2v(L->உச்சி.p++));  /* all results are nil */
        }
        செல் ret;
      }
      vmcase(OP_RETURN1) {
        எனில் (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->உச்சி.p = ra + 1;
          savepc(ci);
          luaD_poscall(L, ci, 1);  /* no hurry... */
          trap = 1;
        }
        இல்லையெனில் {  /* do the 'poscall' here */
          முழு nres = get_nresults(ci->callstatus);
          L->ci = ci->previous;  /* back to caller */
          எனில் (nres == 0)
            L->உச்சி.p = base - 1;  /* asked for no results */
          இல்லையெனில் {
            StkId ra = RA(i);
            setobjs2s(L, base - 1, ra);  /* at least this result */
            L->உச்சி.p = base;
            க்கு (; l_unlikely(nres > 1); nres--)
              setnilvalue(s2v(L->உச்சி.p++));  /* complete missing results */
          }
        }
       ret:  /* return from a Lua function */
        எனில் (ci->callstatus & CIST_FRESH)
          திரும்பு;  /* end this frame */
        இல்லையெனில் {
          ci = ci->previous;
          செல் returning;  /* continue running caller in this frame */
        }
      }
      vmcase(OP_FORLOOP) {
        StkId ra = RA(i);
        எனில் (ttisinteger(s2v(ra + 1))) {  /* integer loop? */
          lua_Unsigned மொத்தம் = l_castS2U(ivalue(s2v(ra)));
          எனில் (மொத்தம் > 0) {  /* still more iterations? */
            lua_Integer step = ivalue(s2v(ra + 1));
            lua_Integer idx = ivalue(s2v(ra + 2));  /* control variable */
            chgivalue(s2v(ra), l_castU2S(மொத்தம் - 1));  /* update counter */
            idx = intop(+, idx, step);  /* add step to index */
            chgivalue(s2v(ra + 2), idx);  /* update control variable */
            pc -= GETARG_Bx(i);  /* jump back */
          }
        }
        இல்லையெனில் எனில் (floatforloop(L, ra))  /* float loop */
          pc -= GETARG_Bx(i);  /* jump back */
        updatetrap(ci);  /* allows a signal to break the loop */
        vmbreak;
      }
      vmcase(OP_FORPREP) {
        StkId ra = RA(i);
        savestate(L, ci);  /* in case of errors */
        எனில் (forprep(L, ra))
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
        செல் l_tforcall;
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
        L->உச்சி.p = ra + 3 + 3;
        ProtectNT(luaD_call(L, ra + 3, GETARG_C(i)));  /* do the call */
        updatestack(ci);  /* stack may have changed */
        i = *(pc++);  /* go to next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
        செல் l_tforloop;
      }}
      vmcase(OP_TFORLOOP) {
       l_tforloop: {
        StkId ra = RA(i);
        எனில் (!ttisnil(s2v(ra + 3)))  /* continue loop? */
          pc -= GETARG_Bx(i);  /* jump back */
        vmbreak;
      }}
      vmcase(OP_SETLIST) {
        StkId ra = RA(i);
        unsigned n = cast_uint(GETARG_vB(i));
        unsigned last = cast_uint(GETARG_vC(i));
        Table *h = hvalue(s2v(ra));
        எனில் (n == 0)
          n = cast_uint(L->உச்சி.p - ra) - 1;  /* get up to the top */
        இல்லையெனில்
          L->உச்சி.p = ci->உச்சி.p;  /* correct top in case of emergency GC */
        last += n;
        எனில் (TESTARG_k(i)) {
          last += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
          pc++;
        }
        /* when 'n' is known, table should have proper size */
        எனில் (last > h->asize) {  /* needs more space? */
          /* fixed-size sets should have space preallocated */
          lua_assert(GETARG_vB(i) == 0);
          luaH_resizearray(L, h, last);  /* preallocate it at once */
        }
        க்கு (; n > 0; n--) {
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
        முழு n = GETARG_C(i) - 1;  /* required results (-1 means all) */
        முழு vatab = GETARG_k(i) ? GETARG_B(i) : -1;
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
        எனில் (!ttisnil(ra))
          halfProtect(luaG_errnnil(L, cl, GETARG_Bx(i)));
        vmbreak;
      }
      vmcase(OP_VARARGPREP) {
        ProtectNT(luaT_adjustvarargs(L, ci, cl->p));
        எனில் (l_unlikely(trap)) {  /* previous "Protect" updated trap */
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
