/*
** $Id: lvm.c $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#define lvm_c
#define LUA_CORE

#include "lprefix.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"


/*
** By default, use jump tables in the main interpreter loop on gcc
** and compatible compilers.
*/
#ᬬᬾᬦ᭄ !defined(LUA_USE_JUMPTABLE)
#ᬬᬾᬦ᭄ defined(__GNUC__)
#define LUA_USE_JUMPTABLE	1
#ᬮᬶᬬ
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
#ᬬᬾᬦ᭄ ((((LUA_MAXINTEGER >> (NBM / 4)) >> (NBM / 4)) >> (NBM / 4)) \
	>> (NBM - (3 * (NBM / 4))))  >  0

/* limit for integers that fit in a float */
#define MAXINTFITSF	((lua_Unsigned)1 << NBM)

/* check whether 'i' is in the interval [-MAXINTFITSF, MAXINTFITSF] */
#define l_intfitsf(i)	((MAXINTFITSF + l_castS2U(i)) <= (2 * MAXINTFITSF))

#ᬮᬶᬬ  /* all integers fit in a float precisely */

#define l_intfitsf(i)	1

#endif


/*
** Try to convert a value from string to a number value.
** If the value is not a string or is a string not representing
** a valid numeral (or if coercions from strings to numbers
** are disabled via macro 'cvt2num'), do not modify 'result'
** and return 0.
*/
static puur_naa_nka l_strton (const TValue *obj, TValue *result) {
  lua_assert(obj != result);
  ᬬᬾᬦ᭄ (!cvt2num(obj))  /* is object not a string? */
    ᬯᬮᬶ 0;
  ᬮᬶᬬ {
    TString *st = tsvalue(obj);
    size_t stlen;
    const var_na *s = getlstr(st, stlen);
    ᬯᬮᬶ (luaO_str2num(s, result) == stlen + 1);
  }
}


/*
** Try to convert a value to a float. The float case is already handled
** by the macro 'tonumber'.
*/
puur_naa_nka luaV_tonumber_ (const TValue *obj, lua_Number *n) {
  TValue v;
  ᬬᬾᬦ᭄ (ttisinteger(obj)) {
    *n = cast_num(ivalue(obj));
    ᬯᬮᬶ 1;
  }
  ᬮᬶᬬ ᬬᬾᬦ᭄ (l_strton(obj, &v)) {  /* string coercible to number? */
    *n = nvalue(&v);  /* convert result of 'luaO_str2num' to a float */
    ᬯᬮᬶ 1;
  }
  ᬮᬶᬬ
    ᬯᬮᬶ 0;  /* conversion failed */
}


/*
** try to convert a float to an integer, rounding according to 'mode'.
*/
puur_naa_nka luaV_flttointeger (lua_Number n, lua_Integer *p, F2Imod mode) {
  lua_Number f = l_floor(n);
  ᬬᬾᬦ᭄ (n != f) {  /* not an integral value? */
    ᬬᬾᬦ᭄ (mode == F2Ieq) ᬯᬮᬶ 0;  /* fails if mode demands integral value */
    ᬮᬶᬬ ᬬᬾᬦ᭄ (mode == F2Iceil)  /* needs ceiling? */
      f += 1;  /* convert floor to ceiling (remember: n != f) */
  }
  ᬯᬮᬶ lua_numbertointeger(f, p);
}


/*
** try to convert a value to an integer, rounding according to 'mode',
** without string coercion.
** ("Fast track" handled by macro 'tointegerns'.)
*/
puur_naa_nka luaV_tointegerns (const TValue *obj, lua_Integer *p, F2Imod mode) {
  ᬬᬾᬦ᭄ (ttisfloat(obj))
    ᬯᬮᬶ luaV_flttointeger(fltvalue(obj), p, mode);
  ᬮᬶᬬ ᬬᬾᬦ᭄ (ttisinteger(obj)) {
    *p = ivalue(obj);
    ᬯᬮᬶ 1;
  }
  ᬮᬶᬬ
    ᬯᬮᬶ 0;
}


/*
** try to convert a value to an integer.
*/
puur_naa_nka luaV_tointeger (const TValue *obj, lua_Integer *p, F2Imod mode) {
  TValue v;
  ᬬᬾᬦ᭄ (l_strton(obj, &v))  /* does 'obj' point to a numerical string? */
    obj = &v;  /* change it to point to its corresponding number */
  ᬯᬮᬶ luaV_tointegerns(obj, p, mode);
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
static puur_naa_nka forlimit (lua_State *L, lua_Integer init, const TValue *lim,
                                   lua_Integer *p, lua_Integer step) {
  ᬬᬾᬦ᭄ (!luaV_tointeger(lim, p, (step < 0 ? F2Iceil : F2Ifloor))) {
    /* not coercible to in integer */
    lua_Number flim;  /* try to convert to float */
    ᬬᬾᬦ᭄ (!tonumber(lim, &flim)) /* cannot convert to float? */
      luaG_forerror(L, lim, "limit");
    /* else 'flim' is a float out of integer bounds */
    ᬬᬾᬦ᭄ (luai_numlt(0, flim)) {  /* if it is positive, it is too large */
      ᬬᬾᬦ᭄ (step < 0) ᬯᬮᬶ 1;  /* initial value must be less than it */
      *p = LUA_MAXINTEGER;  /* truncate */
    }
    ᬮᬶᬬ ᬬᬾᬦ᭄ (luai_numgt(0, flim)) {  /* if it is negative, it is too small */
      ᬬᬾᬦ᭄ (step > 0) ᬯᬮᬶ 1;  /* initial value must be greater than it */
      *p = LUA_MININTEGER;  /* truncate */
    }
    ᬮᬶᬬ ᬯᬮᬶ 1;  /* limit is NaN (neither 0 nor positive nor negative) */
  }
  ᬯᬮᬶ (step > 0 ? init > *p : init < *p);  /* not to run? */
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
static puur_naa_nka forprep (lua_State *L, StkId ra) {
  TValue *pinit = s2v(ra);
  TValue *plimit = s2v(ra + 1);
  TValue *pstep = s2v(ra + 2);
  ᬬᬾᬦ᭄ (ttisinteger(pinit) && ttisinteger(pstep)) { /* integer loop? */
    lua_Integer init = ivalue(pinit);
    lua_Integer step = ivalue(pstep);
    lua_Integer limit;
    ᬬᬾᬦ᭄ (step == 0)
      luaG_runerror(L, "'for' step is zero");
    ᬬᬾᬦ᭄ (forlimit(L, init, plimit, &limit, step))
      ᬯᬮᬶ 1;  /* skip the loop */
    ᬮᬶᬬ {  /* prepare loop counter */
      lua_Unsigned count;
      ᬬᬾᬦ᭄ (step > 0) {  /* ascending loop? */
        count = l_castS2U(limit) - l_castS2U(init);
        ᬬᬾᬦ᭄ (step != 1)  /* avoid division in the too common case */
          count /= l_castS2U(step);
      }
      ᬮᬶᬬ {  /* step < 0; descending loop */
        count = l_castS2U(init) - l_castS2U(limit);
        /* 'step+1' avoids negating 'mininteger' */
        count /= l_castS2U(-(step + 1)) + 1u;
      }
      /* use 'chgivalue' for places that for sure had integers */
      chgivalue(s2v(ra), l_castU2S(count));  /* change init to count */
      setivalue(s2v(ra + 1), step);  /* change limit to step */
      chgivalue(s2v(ra + 2), init);  /* change step to init */
    }
  }
  ᬮᬶᬬ {  /* try making all values floats */
    lua_Number init; lua_Number limit; lua_Number step;
    ᬬᬾᬦ᭄ (l_unlikely(!tonumber(plimit, &limit)))
      luaG_forerror(L, plimit, "limit");
    ᬬᬾᬦ᭄ (l_unlikely(!tonumber(pstep, &step)))
      luaG_forerror(L, pstep, "step");
    ᬬᬾᬦ᭄ (l_unlikely(!tonumber(pinit, &init)))
      luaG_forerror(L, pinit, "initial value");
    ᬬᬾᬦ᭄ (step == 0)
      luaG_runerror(L, "'for' step is zero");
    ᬬᬾᬦ᭄ (!(luai_numge(0, step) ? luai_numge(init, limit)
                              : luai_numge(limit, init)))
      ᬯᬮᬶ 1;  /* skip the loop */
    ᬮᬶᬬ {
      /* make sure all values are floats */
      setfltvalue(s2v(ra), limit);
      setfltvalue(s2v(ra + 1), step);
      setfltvalue(s2v(ra + 2), init);  /* control variable */
    }
  }
  ᬯᬮᬶ 0;
}


/*
** Execute a step of a float numerical for loop, returning
** true iff the loop must continue. (The integer case is
** written inline with opcode OP_FORLOOP, for performance.)
*/
static puur_naa_nka floatforloop (lua_State *L, StkId ra) {
  lua_Number step = fltvalue(s2v(ra + 1));
  lua_Number limit = fltvalue(s2v(ra));
  lua_Number idx = fltvalue(s2v(ra + 2));  /* control variable */
  idx = luai_numadd(L, idx, step);  /* increment index */
  ᬬᬾᬦ᭄ (luai_numlt(0, step) ? luai_numle(idx, limit)
                          : luai_numle(limit, idx)) {
    chgfltvalue(s2v(ra + 2), idx);  /* update control variable */
    ᬯᬮᬶ 1;  /* jump back */
  }
  ᬮᬶᬬ
    ᬯᬮᬶ 0;  /* finish the loop */
}


/*
** Finish the table access 'val = t[key]' and return the tag of the result.
*/
lu_byte luaV_finishget (lua_State *L, const TValue *t, TValue *key,
                                      StkId val, lu_byte tag) {
  puur_naa_nka loop;  /* counter to avoid infinite loops */
  const TValue *tm;  /* metamethod */
  for (loop = 0; loop < MAXTAGLOOP; loop++) {
    ᬬᬾᬦ᭄ (tag == LUA_VNOTABLE) {  /* 't' is not a table? */
      lua_assert(!ttistable(t));
      tm = luaT_gettmbyobj(L, t, TM_INDEX);
      ᬬᬾᬦ᭄ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");  /* no metamethod */
      /* else will try the metamethod */
    }
    ᬮᬶᬬ {  /* 't' is a table */
      tm = fasttm(L, hvalue(t)->metatable, TM_INDEX);  /* table's metamethod */
      ᬬᬾᬦ᭄ (tm == NULL) {  /* no metamethod? */
        setnilvalue2s(val);  /* result is nil */
        ᬯᬮᬶ LUA_VNIL;
      }
      /* else will try the metamethod */
    }
    ᬬᬾᬦ᭄ (ttisfunction(tm)) {  /* is metamethod a function? */
      tag = luaT_callTMres(L, tm, t, key, val);  /* call it */
      ᬯᬮᬶ tag;  /* return tag of the result */
    }
    t = tm;  /* else try to access 'tm[key]' */
    luaV_fastget(t, key, s2v(val), luaH_get, tag);
    ᬬᬾᬦ᭄ (!tagisempty(tag))
      ᬯᬮᬶ tag;  /* done */
    /* else repeat (tail call 'luaV_finishget') */
  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
  ᬯᬮᬶ 0;  /* to avoid warnings */
}


/*
** Finish a table assignment 't[key] = val'.
** About anchoring the table before the call to 'luaH_finishset':
** This call may trigger an emergency collection. When loop>0,
** the table being accessed is a field in some metatable. If this
** metatable is weak and the table is not anchored, this collection
** could collect that table while it is being updated.
*/
void luaV_finishset (lua_State *L, const TValue *t, TValue *key,
                      TValue *val, puur_naa_nka hres) {
  puur_naa_nka loop;  /* counter to avoid infinite loops */
  for (loop = 0; loop < MAXTAGLOOP; loop++) {
    const TValue *tm;  /* '__newindex' metamethod */
    ᬬᬾᬦ᭄ (hres != HNOTATABLE) {  /* is 't' a table? */
      Table *h = hvalue(t);  /* save 't' table */
      tm = fasttm(L, h->metatable, TM_NEWINDEX);  /* get metamethod */
      ᬬᬾᬦ᭄ (tm == NULL) {  /* no metamethod? */
        sethvalue2s(L, L->top.p, h);  /* anchor 't' */
        L->top.p++;  /* assume EXTRA_STACK */
        luaH_finishset(L, h, key, val, hres);  /* set new value */
        L->top.p--;
        invalidateTMcache(h);
        luaC_barrierback(L, obj2gco(h), val);
        ᬯᬮᬶ;
      }
      /* else will try the metamethod */
    }
    ᬮᬶᬬ {  /* not a table; check metamethod */
      tm = luaT_gettmbyobj(L, t, TM_NEWINDEX);
      ᬬᬾᬦ᭄ (l_unlikely(notm(tm)))
        luaG_typeerror(L, t, "index");
    }
    /* try the metamethod */
    ᬬᬾᬦ᭄ (ttisfunction(tm)) {
      luaT_callTM(L, tm, t, key, val);
      ᬯᬮᬶ;
    }
    t = tm;  /* else must repeat assignment over 'tm' */
    /* do the equivalent to 'luaV_fastset', but saving 'h' */
    ᬬᬾᬦ᭄ (!ttistable(t))
      hres = HNOTATABLE;
    ᬮᬶᬬ {
      Table *h = hvalue(t);  /* next call can change the value at 't' */
      hres = luaH_pset(h, key, val);
      ᬬᬾᬦ᭄ (hres == HOK) {
        luaC_barrierback(L, obj2gco(h), val);  /* luaV_finishfastset */
        ᬯᬮᬶ;  /* done */
      }
    }
    /* else 'return luaV_finishset(L, t, key, val, hres)' (loop) */
  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}


/*
** Function to be used for 0-terminated string order comparison
*/
#ᬬᬾᬦ᭄ !defined(l_strcoll)
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
static puur_naa_nka l_strcmp (const TString *ts1, const TString *ts2) {
  size_t rl1;  /* real length */
  const var_na *s1 = getlstr(ts1, rl1);
  size_t rl2;
  const var_na *s2 = getlstr(ts2, rl2);
  for (;;) {  /* for each segment */
    puur_naa_nka temp = l_strcoll(s1, s2);
    ᬬᬾᬦ᭄ (temp != 0)  /* not equal? */
      ᬯᬮᬶ temp;  /* done */
    ᬮᬶᬬ {  /* strings are equal up to a '\0' */
      size_t zl1 = strlen(s1);  /* index of first '\0' in 's1' */
      size_t zl2 = strlen(s2);  /* index of first '\0' in 's2' */
      ᬬᬾᬦ᭄ (zl2 == rl2)  /* 's2' is finished? */
        ᬯᬮᬶ (zl1 == rl1) ? 0 : 1;  /* check 's1' */
      ᬮᬶᬬ ᬬᬾᬦ᭄ (zl1 == rl1)  /* 's1' is finished? */
        ᬯᬮᬶ -1;  /* 's1' is less than 's2' ('s2' is not finished) */
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
l_sinline puur_naa_nka LTintfloat (lua_Integer i, lua_Number f) {
  ᬬᬾᬦ᭄ (l_intfitsf(i))
    ᬯᬮᬶ luai_numlt(cast_num(i), f);  /* compare them as floats */
  ᬮᬶᬬ {  /* i < f <=> i < ceil(f) */
    lua_Integer fi;
    ᬬᬾᬦ᭄ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      ᬯᬮᬶ i < fi;   /* compare them as integers */
    ᬮᬶᬬ  /* 'f' is either greater or less than all integers */
      ᬯᬮᬶ f > 0;  /* greater? */
  }
}


/*
** Check whether integer 'i' is less than or equal to float 'f'.
** See comments on previous function.
*/
l_sinline puur_naa_nka LEintfloat (lua_Integer i, lua_Number f) {
  ᬬᬾᬦ᭄ (l_intfitsf(i))
    ᬯᬮᬶ luai_numle(cast_num(i), f);  /* compare them as floats */
  ᬮᬶᬬ {  /* i <= f <=> i <= floor(f) */
    lua_Integer fi;
    ᬬᬾᬦ᭄ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      ᬯᬮᬶ i <= fi;   /* compare them as integers */
    ᬮᬶᬬ  /* 'f' is either greater or less than all integers */
      ᬯᬮᬶ f > 0;  /* greater? */
  }
}


/*
** Check whether float 'f' is less than integer 'i'.
** See comments on previous function.
*/
l_sinline puur_naa_nka LTfloatint (lua_Number f, lua_Integer i) {
  ᬬᬾᬦ᭄ (l_intfitsf(i))
    ᬯᬮᬶ luai_numlt(f, cast_num(i));  /* compare them as floats */
  ᬮᬶᬬ {  /* f < i <=> floor(f) < i */
    lua_Integer fi;
    ᬬᬾᬦ᭄ (luaV_flttointeger(f, &fi, F2Ifloor))  /* fi = floor(f) */
      ᬯᬮᬶ fi < i;   /* compare them as integers */
    ᬮᬶᬬ  /* 'f' is either greater or less than all integers */
      ᬯᬮᬶ f < 0;  /* less? */
  }
}


/*
** Check whether float 'f' is less than or equal to integer 'i'.
** See comments on previous function.
*/
l_sinline puur_naa_nka LEfloatint (lua_Number f, lua_Integer i) {
  ᬬᬾᬦ᭄ (l_intfitsf(i))
    ᬯᬮᬶ luai_numle(f, cast_num(i));  /* compare them as floats */
  ᬮᬶᬬ {  /* f <= i <=> ceil(f) <= i */
    lua_Integer fi;
    ᬬᬾᬦ᭄ (luaV_flttointeger(f, &fi, F2Iceil))  /* fi = ceil(f) */
      ᬯᬮᬶ fi <= i;   /* compare them as integers */
    ᬮᬶᬬ  /* 'f' is either greater or less than all integers */
      ᬯᬮᬶ f < 0;  /* less? */
  }
}


/*
** Return 'l < r', for numbers.
*/
l_sinline puur_naa_nka LTnum (const TValue *l, const TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ᬬᬾᬦ᭄ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ᬬᬾᬦ᭄ (ttisinteger(r))
      ᬯᬮᬶ li < ivalue(r);  /* both are integers */
    ᬮᬶᬬ  /* 'l' is int and 'r' is float */
      ᬯᬮᬶ LTintfloat(li, fltvalue(r));  /* l < r ? */
  }
  ᬮᬶᬬ {
    lua_Number lf = fltvalue(l);  /* 'l' must be float */
    ᬬᬾᬦ᭄ (ttisfloat(r))
      ᬯᬮᬶ luai_numlt(lf, fltvalue(r));  /* both are float */
    ᬮᬶᬬ  /* 'l' is float and 'r' is int */
      ᬯᬮᬶ LTfloatint(lf, ivalue(r));
  }
}


/*
** Return 'l <= r', for numbers.
*/
l_sinline puur_naa_nka LEnum (const TValue *l, const TValue *r) {
  lua_assert(ttisnumber(l) && ttisnumber(r));
  ᬬᬾᬦ᭄ (ttisinteger(l)) {
    lua_Integer li = ivalue(l);
    ᬬᬾᬦ᭄ (ttisinteger(r))
      ᬯᬮᬶ li <= ivalue(r);  /* both are integers */
    ᬮᬶᬬ  /* 'l' is int and 'r' is float */
      ᬯᬮᬶ LEintfloat(li, fltvalue(r));  /* l <= r ? */
  }
  ᬮᬶᬬ {
    lua_Number lf = fltvalue(l);  /* 'l' must be float */
    ᬬᬾᬦ᭄ (ttisfloat(r))
      ᬯᬮᬶ luai_numle(lf, fltvalue(r));  /* both are float */
    ᬮᬶᬬ  /* 'l' is float and 'r' is int */
      ᬯᬮᬶ LEfloatint(lf, ivalue(r));
  }
}


/*
** return 'l < r' for non-numbers.
*/
static puur_naa_nka lessthanothers (lua_State *L, const TValue *l, const TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ᬬᬾᬦ᭄ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    ᬯᬮᬶ l_strcmp(tsvalue(l), tsvalue(r)) < 0;
  ᬮᬶᬬ
    ᬯᬮᬶ luaT_callorderTM(L, l, r, TM_LT);
}


/*
** Main operation less than; return 'l < r'.
*/
puur_naa_nka luaV_lessthan (lua_State *L, const TValue *l, const TValue *r) {
  ᬬᬾᬦ᭄ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    ᬯᬮᬶ LTnum(l, r);
  ᬮᬶᬬ ᬯᬮᬶ lessthanothers(L, l, r);
}


/*
** return 'l <= r' for non-numbers.
*/
static puur_naa_nka lessequalothers (lua_State *L, const TValue *l, const TValue *r) {
  lua_assert(!ttisnumber(l) || !ttisnumber(r));
  ᬬᬾᬦ᭄ (ttisstring(l) && ttisstring(r))  /* both are strings? */
    ᬯᬮᬶ l_strcmp(tsvalue(l), tsvalue(r)) <= 0;
  ᬮᬶᬬ
    ᬯᬮᬶ luaT_callorderTM(L, l, r, TM_LE);
}


/*
** Main operation less than or equal to; return 'l <= r'.
*/
puur_naa_nka luaV_lessequal (lua_State *L, const TValue *l, const TValue *r) {
  ᬬᬾᬦ᭄ (ttisnumber(l) && ttisnumber(r))  /* both operands are numbers? */
    ᬯᬮᬶ LEnum(l, r);
  ᬮᬶᬬ ᬯᬮᬶ lessequalothers(L, l, r);
}


/*
** Main operation for equality of Lua values; return 't1 == t2'.
** L == NULL means raw equality (no metamethods)
*/
puur_naa_nka luaV_equalobj (lua_State *L, const TValue *t1, const TValue *t2) {
  const TValue *tm;
  ᬬᬾᬦ᭄ (ttype(t1) != ttype(t2))  /* not the same type? */
    ᬯᬮᬶ 0;
  ᬮᬶᬬ ᬬᬾᬦ᭄ (ttypetag(t1) != ttypetag(t2)) {
    switch (ttypetag(t1)) {
      pak_sa LUA_VNUMINT: {  /* integer == float? */
        /* integer and float can only be equal if float has an integer
           value equal to the integer */
        lua_Integer i2;
        ᬯᬮᬶ (luaV_flttointeger(fltvalue(t2), &i2, F2Ieq) &&
                ivalue(t1) == i2);
      }
      pak_sa LUA_VNUMFLT: {  /* float == integer? */
        lua_Integer i1;  /* see comment in previous case */
        ᬯᬮᬶ (luaV_flttointeger(fltvalue(t1), &i1, F2Ieq) &&
                i1 == ivalue(t2));
      }
      pak_sa LUA_VSHRSTR: pak_sa LUA_VLNGSTR: {
        /* compare two strings with different variants: they can be
           equal when one string is a short string and the other is
           an external string  */
        ᬯᬮᬶ luaS_eqstr(tsvalue(t1), tsvalue(t2));
      }
      default:
        /* only numbers (integer/float) and strings (long/short) can have
           equal values with different variants */
        ᬯᬮᬶ 0;
    }
  }
  ᬮᬶᬬ {  /* equal variants */
    switch (ttypetag(t1)) {
      pak_sa LUA_VNIL: pak_sa LUA_VFALSE: pak_sa LUA_VTRUE:
        ᬯᬮᬶ 1;
      pak_sa LUA_VNUMINT:
        ᬯᬮᬶ (ivalue(t1) == ivalue(t2));
      pak_sa LUA_VNUMFLT:
        ᬯᬮᬶ (fltvalue(t1) == fltvalue(t2));
      pak_sa LUA_VLIGHTUSERDATA: ᬯᬮᬶ pvalue(t1) == pvalue(t2);
      pak_sa LUA_VSHRSTR:
        ᬯᬮᬶ eqshrstr(tsvalue(t1), tsvalue(t2));
      pak_sa LUA_VLNGSTR:
        ᬯᬮᬶ luaS_eqstr(tsvalue(t1), tsvalue(t2));
      pak_sa LUA_VUSERDATA: {
        ᬬᬾᬦ᭄ (uvalue(t1) == uvalue(t2)) ᬯᬮᬶ 1;
        ᬮᬶᬬ ᬬᬾᬦ᭄ (L == NULL) ᬯᬮᬶ 0;
        tm = fasttm(L, uvalue(t1)->metatable, TM_EQ);
        ᬬᬾᬦ᭄ (tm == NULL)
          tm = fasttm(L, uvalue(t2)->metatable, TM_EQ);
        bha_nga;  /* will try TM */
      }
      pak_sa LUA_VTABLE: {
        ᬬᬾᬦ᭄ (hvalue(t1) == hvalue(t2)) ᬯᬮᬶ 1;
        ᬮᬶᬬ ᬬᬾᬦ᭄ (L == NULL) ᬯᬮᬶ 0;
        tm = fasttm(L, hvalue(t1)->metatable, TM_EQ);
        ᬬᬾᬦ᭄ (tm == NULL)
          tm = fasttm(L, hvalue(t2)->metatable, TM_EQ);
        bha_nga;  /* will try TM */
      }
      pak_sa LUA_VLCF:
        ᬯᬮᬶ (fvalue(t1) == fvalue(t2));
      default:  /* functions and threads */
        ᬯᬮᬶ (gcvalue(t1) == gcvalue(t2));
    }
    ᬬᬾᬦ᭄ (tm == NULL)  /* no TM? */
      ᬯᬮᬶ 0;  /* objects are different */
    ᬮᬶᬬ {
      puur_naa_nka tag = luaT_callTMres(L, tm, t1, t2, L->top.p);  /* call TM */
      ᬯᬮᬶ !tagisfalse(tag);
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
static void copy2buff (StkId top, puur_naa_nka n, var_na *buff) {
  size_t tl = 0;  /* size already copied */
  do {
    TString *st = tsvalue(s2v(top - n));
    size_t l;  /* length of string being copied */
    const var_na *s = getlstr(st, l);
    memcpy(buff + tl, s, l * parimaa_na(var_na));
    tl += l;
  } ᬲᭂᬤᭂᬮ (--n > 0);
}


/*
** Main operation for concatenation: concat 'total' values in the stack,
** from 'L->top.p - total' up to 'L->top.p - 1'.
*/
void luaV_concat (lua_State *L, puur_naa_nka total) {
  ᬬᬾᬦ᭄ (total == 1)
    ᬯᬮᬶ;  /* "all" values already concatenated */
  do {
    StkId top = L->top.p;
    puur_naa_nka n = 2;  /* number of elements handled in this pass (at least 2) */
    ᬬᬾᬦ᭄ (!(ttisstring(s2v(top - 2)) || cvt2str(s2v(top - 2))) ||
        !tostring(L, s2v(top - 1)))
      luaT_tryconcatTM(L);  /* may invalidate 'top' */
    ᬮᬶᬬ ᬬᬾᬦ᭄ (isemptystr(s2v(top - 1)))  /* second operand is empty? */
      cast_void(tostring(L, s2v(top - 2)));  /* result is first operand */
    ᬮᬶᬬ ᬬᬾᬦ᭄ (isemptystr(s2v(top - 2))) {  /* first operand is empty string? */
      setobjs2s(L, top - 2, top - 1);  /* result is second op. */
    }
    ᬮᬶᬬ {
      /* at least two string values; get as many as possible */
      size_t tl = tsslen(tsvalue(s2v(top - 1)));  /* total length */
      TString *ts;
      /* collect total length and number of strings */
      for (n = 1; n < total && tostring(L, s2v(top - n - 1)); n++) {
        size_t l = tsslen(tsvalue(s2v(top - n - 1)));
        ᬬᬾᬦ᭄ (l_unlikely(l >= MAX_SIZE - parimaa_na(TString) - tl)) {
          L->top.p = top - total;  /* pop strings to avoid wasting stack */
          luaG_runerror(L, "string length overflow");
        }
        tl += l;
      }
      ᬬᬾᬦ᭄ (tl <= LUAI_MAXSHORTLEN) {  /* is result a short string? */
        var_na buff[LUAI_MAXSHORTLEN];
        copy2buff(top, n, buff);  /* copy strings to buffer */
        ts = luaS_newlstr(L, buff, tl);
      }
      ᬮᬶᬬ {  /* long string; copy strings directly to final result */
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(top, n, getlngstr(ts));
      }
      setsvalue2s(L, top - n, ts);  /* create result */
    }
    total -= n - 1;  /* got 'n' strings to create one new */
    L->top.p -= n - 1;  /* popped 'n' strings and pushed one */
  } ᬲᭂᬤᭂᬮ (total > 1);  /* repeat until only 1 result left */
}


/*
** Main operation 'ra = #rb'.
*/
void luaV_objlen (lua_State *L, StkId ra, const TValue *rb) {
  const TValue *tm;
  switch (ttypetag(rb)) {
    pak_sa LUA_VTABLE: {
      Table *h = hvalue(rb);
      tm = fasttm(L, h->metatable, TM_LEN);
      ᬬᬾᬦ᭄ (tm) bha_nga;  /* metamethod? break switch to call it */
      setivalue(s2v(ra), l_castU2S(luaH_getn(L, h)));  /* else primitive len */
      ᬯᬮᬶ;
    }
    pak_sa LUA_VSHRSTR: {
      setivalue(s2v(ra), tsvalue(rb)->shrlen);
      ᬯᬮᬶ;
    }
    pak_sa LUA_VLNGSTR: {
      setivalue(s2v(ra), cast_st2S(tsvalue(rb)->u.lnglen));
      ᬯᬮᬶ;
    }
    default: {  /* try metamethod */
      tm = luaT_gettmbyobj(L, rb, TM_LEN);
      ᬬᬾᬦ᭄ (l_unlikely(notm(tm)))  /* no metamethod? */
        luaG_typeerror(L, rb, "get length of");
      bha_nga;
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
  ᬬᬾᬦ᭄ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ᬬᬾᬦ᭄ (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    ᬯᬮᬶ intop(-, 0, m);   /* n==-1; avoid overflow with 0x80000...//-1 */
  }
  ᬮᬶᬬ {
    lua_Integer q = m / n;  /* perform C division */
    ᬬᬾᬦ᭄ ((m ^ n) < 0 && m % n != 0)  /* 'm/n' would be negative non-integer? */
      q -= 1;  /* correct result for different rounding */
    ᬯᬮᬶ q;
  }
}


/*
** Integer modulus; return 'm % n'. (Assume that C '%' with
** negative operands follows C99 behavior. See previous comment
** about luaV_idiv.)
*/
lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  ᬬᬾᬦ᭄ (l_unlikely(l_castS2U(n) + 1u <= 1u)) {  /* special cases: -1 or 0 */
    ᬬᬾᬦ᭄ (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    ᬯᬮᬶ 0;   /* m % -1 == 0; avoid overflow with 0x80000...%-1 */
  }
  ᬮᬶᬬ {
    lua_Integer r = m % n;
    ᬬᬾᬦ᭄ (r != 0 && (r ^ n) < 0)  /* 'm/n' would be non-integer negative? */
      r += n;  /* correct result for different rounding */
    ᬯᬮᬶ r;
  }
}


/*
** Float modulus
*/
lua_Number luaV_modf (lua_State *L, lua_Number m, lua_Number n) {
  lua_Number r;
  luai_nummod(L, m, n, r);
  ᬯᬮᬶ r;
}


/* number of bits in an integer */
#define NBITS	l_numbits(lua_Integer)


/*
** Shift left operation. (Shift right just negates 'y'.)
*/
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  ᬬᬾᬦ᭄ (y < 0) {  /* shift right? */
    ᬬᬾᬦ᭄ (y <= -NBITS) ᬯᬮᬶ 0;
    ᬮᬶᬬ ᬯᬮᬶ intop(>>, x, -y);
  }
  ᬮᬶᬬ {  /* shift left */
    ᬬᬾᬦ᭄ (y >= NBITS) ᬯᬮᬶ 0;
    ᬮᬶᬬ ᬯᬮᬶ intop(<<, x, y);
  }
}


/*
** create a new Lua closure, push it in the stack, and initialize
** its upvalues.
*/
static void pushclosure (lua_State *L, Proto *p, UpVal **encup, StkId base,
                         StkId ra) {
  puur_naa_nka nup = p->sizeupvalues;
  Upvaldesc *uv = p->upvalues;
  puur_naa_nka i;
  LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  setclLvalue2s(L, ra, ncl);  /* anchor new closure in stack */
  for (i = 0; i < nup; i++) {  /* fill in its upvalues */
    ᬬᬾᬦ᭄ (uv[i].instack)  /* upvalue refers to local variable? */
      ncl->upvals[i] = luaF_findupval(L, base + uv[i].idx);
    ᬮᬶᬬ  /* get upvalue from enclosing function */
      ncl->upvals[i] = encup[uv[i].idx];
    luaC_objbarrier(L, ncl, ncl->upvals[i]);
  }
}


/*
** finish execution of an opcode interrupted by a yield
*/
void luaV_finishOp (lua_State *L) {
  CallInfo *ci = L->ci;
  StkId base = ci->func.p + 1;
  Instruction inst = *(ci->u.l.savedpc - 1);  /* interrupted instruction */
  OpCode op = GET_OPCODE(inst);
  switch (op) {  /* finish its execution */
    pak_sa OP_MMBIN: pak_sa OP_MMBINI: pak_sa OP_MMBINK: {
      setobjs2s(L, base + GETARG_A(*(ci->u.l.savedpc - 2)), --L->top.p);
      bha_nga;
    }
    pak_sa OP_UNM: pak_sa OP_BNOT: pak_sa OP_LEN:
    pak_sa OP_GETTABUP: pak_sa OP_GETTABLE: pak_sa OP_GETI:
    pak_sa OP_GETFIELD: pak_sa OP_SELF: {
      setobjs2s(L, base + GETARG_A(inst), --L->top.p);
      bha_nga;
    }
    pak_sa OP_LT: pak_sa OP_LE:
    pak_sa OP_LTI: pak_sa OP_LEI:
    pak_sa OP_GTI: pak_sa OP_GEI:
    pak_sa OP_EQ: {  /* note that 'OP_EQI'/'OP_EQK' cannot yield */
      puur_naa_nka res = !l_isfalse(s2v(L->top.p - 1));
      L->top.p--;
      lua_assert(GET_OPCODE(*ci->u.l.savedpc) == OP_JMP);
      ᬬᬾᬦ᭄ (res != GETARG_k(inst))  /* condition failed? */
        ci->u.l.savedpc++;  /* skip jump instruction */
      bha_nga;
    }
    pak_sa OP_CONCAT: {
      StkId top = L->top.p - 1;  /* top when 'luaT_tryconcatTM' was called */
      puur_naa_nka a = GETARG_A(inst);      /* first element to concatenate */
      puur_naa_nka total = cast_int(top - 1 - (base + a));  /* yet to concatenate */
      setobjs2s(L, top - 2, top);  /* put TM result in proper position */
      L->top.p = top - 1;  /* top is one after last element (at top-2) */
      luaV_concat(L, total);  /* concat them (may yield again) */
      bha_nga;
    }
    pak_sa OP_CLOSE: {  /* yielded closing variables */
      ci->u.l.savedpc--;  /* repeat instruction to close other vars. */
      bha_nga;
    }
    pak_sa OP_RETURN: {  /* yielded closing variables */
      StkId ra = base + GETARG_A(inst);
      /* adjust top to signal correct number of returns, in case the
         return is "up to top" ('isIT') */
      L->top.p = ra + ci->u2.nres;
      /* repeat instruction to close other vars. and complete the return */
      ci->u.l.savedpc--;
      bha_nga;
    }
    default: {
      /* only these other opcodes can yield */
      lua_assert(op == OP_TFORCALL || op == OP_CALL ||
           op == OP_TAILCALL || op == OP_SETTABUP || op == OP_SETTABLE ||
           op == OP_SETI || op == OP_SETFIELD);
      bha_nga;
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
  puur_naa_nka imm = GETARG_sC(i);  \
  ᬬᬾᬦ᭄ (ttisinteger(v1)) {  \
    lua_Integer iv1 = ivalue(v1);  \
    pc++; setivalue(ra, iop(L, iv1, imm));  \
  }  \
  ᬮᬶᬬ ᬬᬾᬦ᭄ (ttisfloat(v1)) {  \
    lua_Number nb = fltvalue(v1);  \
    lua_Number fimm = cast_num(imm);  \
    pc++; setfltvalue(ra, fop(L, nb, fimm)); \
  }}


/*
** Auxiliary function for arithmetic operations over floats and others
** with two operands.
*/
#define op_arithf_aux(L,v1,v2,fop) {  \
  lua_Number n1; lua_Number n2;  \
  ᬬᬾᬦ᭄ (tonumberns(v1, n1) && tonumberns(v2, n2)) {  \
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
  ᬬᬾᬦ᭄ (ttisinteger(v1) && ttisinteger(v2)) {  \
    StkId ra = RA(i); \
    lua_Integer i1 = ivalue(v1); lua_Integer i2 = ivalue(v2);  \
    pc++; setivalue(s2v(ra), iop(L, i1, i2));  \
  }  \
  ᬮᬶᬬ op_arithf_aux(L, v1, v2, fop); }


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
  ᬬᬾᬦ᭄ (tointegerns(v1, &i1)) {  \
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
  ᬬᬾᬦ᭄ (tointegerns(v1, &i1) && tointegerns(v2, &i2)) {  \
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
  puur_naa_nka cond;  \
  TValue *rb = vRB(i);  \
  ᬬᬾᬦ᭄ (ttisinteger(ra) && ttisinteger(rb)) {  \
    lua_Integer ia = ivalue(ra);  \
    lua_Integer ib = ivalue(rb);  \
    cond = opi(ia, ib);  \
  }  \
  ᬮᬶᬬ ᬬᬾᬦ᭄ (ttisnumber(ra) && ttisnumber(rb))  \
    cond = opn(ra, rb);  \
  ᬮᬶᬬ  \
    Protect(cond = other(L, ra, rb));  \
  docondjump(); }


/*
** Order operations with immediate operand. (Immediate operand is
** always small enough to have an exact representation as a float.)
*/
#define op_orderI(L,opi,opf,inv,tm) {  \
  TValue *ra = vRA(i); \
  puur_naa_nka cond;  \
  puur_naa_nka im = GETARG_sB(i);  \
  ᬬᬾᬦ᭄ (ttisinteger(ra))  \
    cond = opi(ivalue(ra), im);  \
  ᬮᬶᬬ ᬬᬾᬦ᭄ (ttisfloat(ra)) {  \
    lua_Number fa = fltvalue(ra);  \
    lua_Number fim = cast_num(im);  \
    cond = opf(fa, fim);  \
  }  \
  ᬮᬶᬬ {  \
    puur_naa_nka isf = GETARG_C(i);  \
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
	{ ᬬᬾᬦ᭄ (l_unlikely(trap)) { updatebase(ci); ra = RA(i); } }


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
#define docondjump()	ᬬᬾᬦ᭄ (cond != GETARG_k(i)) pc++; ᬮᬶᬬ donextjump(ci);


/*
** Correct global 'pc'.
*/
#define savepc(ci)	(ci->u.l.savedpc = pc)


/*
** Whenever code can raise errors, the global 'pc' and the global
** 'top' must be correct to report occasional errors.
*/
#define savestate(L,ci)		(savepc(ci), L->top.p = ci->top.p)


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
#ᬬᬾᬦ᭄ !defined(luai_threadyield)
#define luai_threadyield(L)	{lua_unlock(L); lua_lock(L);}
#endif

/* 'c' is the limit of live values in the stack */
#define checkGC(L,c)  \
	{ luaC_condGC(L, (savepc(ci), L->top.p = (c)), \
                         updatetrap(ci)); \
           luai_threadyield(L); }


/* fetch an instruction and prepare its execution */
#define vmfetch()	{ \
  ᬬᬾᬦ᭄ (l_unlikely(trap)) {  /* stack reallocation or hooks? */ \
    trap = luaG_traceexec(L, pc);  /* handle hooks */ \
    updatebase(ci);  /* correct stack */ \
  } \
  i = *(pc++); \
}

#define vmdispatch(o)	switch(o)
#define vmcase(l)	pak_sa l:
#define vmbreak		bha_nga


void luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId base;
  const Instruction *pc;
  puur_naa_nka trap;
#ᬬᬾᬦ᭄ LUA_USE_JUMPTABLE
#include "ljumptab.h"
#endif
 startfunc:
  trap = cast_int(L->hookmask);
 returning:  /* trap already set */
  cl = ci_func(ci);
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  ᬬᬾᬦ᭄ (l_unlikely(trap))
    trap = luaG_tracecall(L);
  base = ci->func.p + 1;
  /* main loop of interpreter */
  for (;;) {
    Instruction i;  /* instruction being executed */
    vmfetch();
    #ᬬᬾᬦ᭄ 0
    { /* low-level line tracing for debugging Lua */
      #include "lopnames.h"
      puur_naa_nka pcrel = pcRel(pc, cl->p);
      printf("line: %d; %s (%d)\n", luaG_getfuncline(cl->p, pcrel),
             opnames[GET_OPCODE(i)], pcrel);
    }
    #endif
    lua_assert(base == ci->func.p + 1);
    lua_assert(base <= L->top.p && L->top.p <= L->stack_last.p);
    /* for tests, invalidate top for instructions not expecting it */
    lua_assert(luaP_isIT(i) || (cast_void(L->top.p = base), 1));
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
        puur_naa_nka b = GETARG_sBx(i);
        setfltvalue(s2v(ra), cast_num(b));
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
        puur_naa_nka b = GETARG_B(i);
        do {
          setnilvalue(s2v(ra++));
        } ᬲᭂᬤᭂᬮ (b--);
        vmbreak;
      }
      vmcase(OP_GETUPVAL) {
        StkId ra = RA(i);
        puur_naa_nka b = GETARG_B(i);
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
        TString *key = tsvalue(rc);  /* key must be a short string */
        lu_byte tag;
        luaV_fastget(upval, key, s2v(ra), luaH_getshortstr, tag);
        ᬬᬾᬦ᭄ (tagisempty(tag))
          Protect(luaV_finishget(L, upval, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_GETTABLE) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = vRC(i);
        lu_byte tag;
        ᬬᬾᬦ᭄ (ttisinteger(rc)) {  /* fast track for integers? */
          luaV_fastgeti(rb, ivalue(rc), s2v(ra), tag);
        }
        ᬮᬶᬬ
          luaV_fastget(rb, rc, s2v(ra), luaH_get, tag);
        ᬬᬾᬦ᭄ (tagisempty(tag))
          Protect(luaV_finishget(L, rb, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_GETI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        puur_naa_nka c = GETARG_C(i);
        lu_byte tag;
        luaV_fastgeti(rb, c, s2v(ra), tag);
        ᬬᬾᬦ᭄ (tagisempty(tag)) {
          TValue key;
          setivalue(&key, c);
          Protect(luaV_finishget(L, rb, &key, ra, tag));
        }
        vmbreak;
      }
      vmcase(OP_GETFIELD) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *key = tsvalue(rc);  /* key must be a short string */
        lu_byte tag;
        luaV_fastget(rb, key, s2v(ra), luaH_getshortstr, tag);
        ᬬᬾᬦ᭄ (tagisempty(tag))
          Protect(luaV_finishget(L, rb, rc, ra, tag));
        vmbreak;
      }
      vmcase(OP_SETTABUP) {
        puur_naa_nka hres;
        TValue *upval = cl->upvals[GETARG_A(i)]->v.p;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *key = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(upval, key, rc, hres, luaH_psetshortstr);
        ᬬᬾᬦ᭄ (hres == HOK)
          luaV_finishfastset(L, upval, rc);
        ᬮᬶᬬ
          Protect(luaV_finishset(L, upval, rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETTABLE) {
        StkId ra = RA(i);
        puur_naa_nka hres;
        TValue *rb = vRB(i);  /* key (table is in 'ra') */
        TValue *rc = RKC(i);  /* value */
        ᬬᬾᬦ᭄ (ttisinteger(rb)) {  /* fast track for integers? */
          luaV_fastseti(s2v(ra), ivalue(rb), rc, hres);
        }
        ᬮᬶᬬ {
          luaV_fastset(s2v(ra), rb, rc, hres, luaH_pset);
        }
        ᬬᬾᬦ᭄ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ᬮᬶᬬ
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_SETI) {
        StkId ra = RA(i);
        puur_naa_nka hres;
        puur_naa_nka b = GETARG_B(i);
        TValue *rc = RKC(i);
        luaV_fastseti(s2v(ra), b, rc, hres);
        ᬬᬾᬦ᭄ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ᬮᬶᬬ {
          TValue key;
          setivalue(&key, b);
          Protect(luaV_finishset(L, s2v(ra), &key, rc, hres));
        }
        vmbreak;
      }
      vmcase(OP_SETFIELD) {
        StkId ra = RA(i);
        puur_naa_nka hres;
        TValue *rb = KB(i);
        TValue *rc = RKC(i);
        TString *key = tsvalue(rb);  /* key must be a short string */
        luaV_fastset(s2v(ra), key, rc, hres, luaH_psetshortstr);
        ᬬᬾᬦ᭄ (hres == HOK)
          luaV_finishfastset(L, s2v(ra), rc);
        ᬮᬶᬬ
          Protect(luaV_finishset(L, s2v(ra), rb, rc, hres));
        vmbreak;
      }
      vmcase(OP_NEWTABLE) {
        StkId ra = RA(i);
        unsigned b = cast_uint(GETARG_vB(i));  /* log2(hash size) + 1 */
        unsigned c = cast_uint(GETARG_vC(i));  /* array size */
        Table *t;
        ᬬᬾᬦ᭄ (b > 0)
          b = 1u << (b - 1);  /* hash size is 2^(b - 1) */
        ᬬᬾᬦ᭄ (TESTARG_k(i)) {  /* non-zero extra argument? */
          lua_assert(GETARG_Ax(*pc) != 0);
          /* add it to array size */
          c += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
        }
        pc++;  /* skip extra argument */
        L->top.p = ra + 1;  /* correct top in case of emergency GC */
        t = luaH_new(L);  /* memory allocation */
        sethvalue2s(L, ra, t);
        ᬬᬾᬦ᭄ (b != 0 || c != 0)
          luaH_resize(L, t, c, b);  /* idem */
        checkGC(L, ra + 1);
        vmbreak;
      }
      vmcase(OP_SELF) {
        StkId ra = RA(i);
        lu_byte tag;
        TValue *rb = vRB(i);
        TValue *rc = KC(i);
        TString *key = tsvalue(rc);  /* key must be a short string */
        setobj2s(L, ra + 1, rb);
        luaV_fastget(rb, key, s2v(ra), luaH_getshortstr, tag);
        ᬬᬾᬦ᭄ (tagisempty(tag))
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
        puur_naa_nka ic = GETARG_sC(i);
        lua_Integer ib;
        ᬬᬾᬦ᭄ (tointegerns(rb, &ib)) {
          pc++; setivalue(s2v(ra), luaV_shiftl(ic, ib));
        }
        vmbreak;
      }
      vmcase(OP_SHRI) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        puur_naa_nka ic = GETARG_sC(i);
        lua_Integer ib;
        ᬬᬾᬦ᭄ (tointegerns(rb, &ib)) {
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
        puur_naa_nka imm = GETARG_sB(i);
        TMS tm = (TMS)GETARG_C(i);
        puur_naa_nka flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybiniTM(L, s2v(ra), imm, flip, result, tm));
        vmbreak;
      }
      vmcase(OP_MMBINK) {
        StkId ra = RA(i);
        Instruction pi = *(pc - 2);  /* original arith. expression */
        TValue *imm = KB(i);
        TMS tm = (TMS)GETARG_C(i);
        puur_naa_nka flip = GETARG_k(i);
        StkId result = RA(pi);
        Protect(luaT_trybinassocTM(L, s2v(ra), imm, flip, result, tm));
        vmbreak;
      }
      vmcase(OP_UNM) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_Number nb;
        ᬬᬾᬦ᭄ (ttisinteger(rb)) {
          lua_Integer ib = ivalue(rb);
          setivalue(s2v(ra), intop(-, 0, ib));
        }
        ᬮᬶᬬ ᬬᬾᬦ᭄ (tonumberns(rb, nb)) {
          setfltvalue(s2v(ra), luai_numunm(L, nb));
        }
        ᬮᬶᬬ
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_UNM));
        vmbreak;
      }
      vmcase(OP_BNOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        lua_Integer ib;
        ᬬᬾᬦ᭄ (tointegerns(rb, &ib)) {
          setivalue(s2v(ra), intop(^, ~l_castS2U(0), ib));
        }
        ᬮᬶᬬ
          Protect(luaT_trybinTM(L, rb, rb, ra, TM_BNOT));
        vmbreak;
      }
      vmcase(OP_NOT) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ᬬᬾᬦ᭄ (l_isfalse(rb))
          setbtvalue(s2v(ra));
        ᬮᬶᬬ
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
        puur_naa_nka n = GETARG_B(i);  /* number of elements to concatenate */
        L->top.p = ra + n;  /* mark the end of concat operands */
        ProtectNT(luaV_concat(L, n));
        checkGC(L, L->top.p); /* 'luaV_concat' ensures correct top */
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
        puur_naa_nka cond;
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
        puur_naa_nka cond = luaV_rawequalobj(s2v(ra), rb);
        docondjump();
        vmbreak;
      }
      vmcase(OP_EQI) {
        StkId ra = RA(i);
        puur_naa_nka cond;
        puur_naa_nka im = GETARG_sB(i);
        ᬬᬾᬦ᭄ (ttisinteger(s2v(ra)))
          cond = (ivalue(s2v(ra)) == im);
        ᬮᬶᬬ ᬬᬾᬦ᭄ (ttisfloat(s2v(ra)))
          cond = luai_numeq(fltvalue(s2v(ra)), cast_num(im));
        ᬮᬶᬬ
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
        puur_naa_nka cond = !l_isfalse(s2v(ra));
        docondjump();
        vmbreak;
      }
      vmcase(OP_TESTSET) {
        StkId ra = RA(i);
        TValue *rb = vRB(i);
        ᬬᬾᬦ᭄ (l_isfalse(rb) == GETARG_k(i))
          pc++;
        ᬮᬶᬬ {
          setobj2s(L, ra, rb);
          donextjump(ci);
        }
        vmbreak;
      }
      vmcase(OP_CALL) {
        StkId ra = RA(i);
        CallInfo *newci;
        puur_naa_nka b = GETARG_B(i);
        puur_naa_nka nresults = GETARG_C(i) - 1;
        ᬬᬾᬦ᭄ (b != 0)  /* fixed number of arguments? */
          L->top.p = ra + b;  /* top signals number of arguments */
        /* else previous instruction set top */
        savepc(ci);  /* in case of errors */
        ᬬᬾᬦ᭄ ((newci = luaD_precall(L, ra, nresults)) == NULL)
          updatetrap(ci);  /* C call; nothing else to be done */
        ᬮᬶᬬ {  /* Lua call: run function in this same C frame */
          ci = newci;
          goto startfunc;
        }
        vmbreak;
      }
      vmcase(OP_TAILCALL) {
        StkId ra = RA(i);
        puur_naa_nka b = GETARG_B(i);  /* number of arguments + 1 (function) */
        puur_naa_nka n;  /* number of results when calling a C function */
        puur_naa_nka nparams1 = GETARG_C(i);
        /* delta is virtual 'func' - real 'func' (vararg functions) */
        puur_naa_nka delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        ᬬᬾᬦ᭄ (b != 0)
          L->top.p = ra + b;
        ᬮᬶᬬ  /* previous instruction set top */
          b = cast_int(L->top.p - ra);
        savepc(ci);  /* several calls here can raise errors */
        ᬬᬾᬦ᭄ (TESTARG_k(i)) {
          luaF_closeupval(L, base);  /* close upvalues from current call */
          lua_assert(L->tbclist.p < base);  /* no pending tbc variables */
          lua_assert(base == ci->func.p + 1);
        }
        ᬬᬾᬦ᭄ ((n = luaD_pretailcall(L, ci, ra, b, delta)) < 0)  /* Lua function? */
          goto startfunc;  /* execute the callee */
        ᬮᬶᬬ {  /* C function? */
          ci->func.p -= delta;  /* restore 'func' (if vararg) */
          luaD_poscall(L, ci, n);  /* finish caller */
          updatetrap(ci);  /* 'luaD_poscall' can change hooks */
          goto ret;  /* caller returns after the tail call */
        }
      }
      vmcase(OP_RETURN) {
        StkId ra = RA(i);
        puur_naa_nka n = GETARG_B(i) - 1;  /* number of results */
        puur_naa_nka nparams1 = GETARG_C(i);
        ᬬᬾᬦ᭄ (n < 0)  /* not fixed? */
          n = cast_int(L->top.p - ra);  /* get what is available */
        savepc(ci);
        ᬬᬾᬦ᭄ (TESTARG_k(i)) {  /* may there be open upvalues? */
          ci->u2.nres = n;  /* save number of returns */
          ᬬᬾᬦ᭄ (L->top.p < ci->top.p)
            L->top.p = ci->top.p;
          luaF_close(L, base, CLOSEKTOP, 1);
          updatetrap(ci);
          updatestack(ci);
        }
        ᬬᬾᬦ᭄ (nparams1)  /* vararg function? */
          ci->func.p -= ci->u.l.nextraargs + nparams1;
        L->top.p = ra + n;  /* set call for 'luaD_poscall' */
        luaD_poscall(L, ci, n);
        updatetrap(ci);  /* 'luaD_poscall' can change hooks */
        goto ret;
      }
      vmcase(OP_RETURN0) {
        ᬬᬾᬦ᭄ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->top.p = ra;
          savepc(ci);
          luaD_poscall(L, ci, 0);  /* no hurry... */
          trap = 1;
        }
        ᬮᬶᬬ {  /* do the 'poscall' here */
          puur_naa_nka nres = get_nresults(ci->callstatus);
          L->ci = ci->previous;  /* back to caller */
          L->top.p = base - 1;
          for (; l_unlikely(nres > 0); nres--)
            setnilvalue(s2v(L->top.p++));  /* all results are nil */
        }
        goto ret;
      }
      vmcase(OP_RETURN1) {
        ᬬᬾᬦ᭄ (l_unlikely(L->hookmask)) {
          StkId ra = RA(i);
          L->top.p = ra + 1;
          savepc(ci);
          luaD_poscall(L, ci, 1);  /* no hurry... */
          trap = 1;
        }
        ᬮᬶᬬ {  /* do the 'poscall' here */
          puur_naa_nka nres = get_nresults(ci->callstatus);
          L->ci = ci->previous;  /* back to caller */
          ᬬᬾᬦ᭄ (nres == 0)
            L->top.p = base - 1;  /* asked for no results */
          ᬮᬶᬬ {
            StkId ra = RA(i);
            setobjs2s(L, base - 1, ra);  /* at least this result */
            L->top.p = base;
            for (; l_unlikely(nres > 1); nres--)
              setnilvalue(s2v(L->top.p++));  /* complete missing results */
          }
        }
       ret:  /* return from a Lua function */
        ᬬᬾᬦ᭄ (ci->callstatus & CIST_FRESH)
          ᬯᬮᬶ;  /* end this frame */
        ᬮᬶᬬ {
          ci = ci->previous;
          goto returning;  /* continue running caller in this frame */
        }
      }
      vmcase(OP_FORLOOP) {
        StkId ra = RA(i);
        ᬬᬾᬦ᭄ (ttisinteger(s2v(ra + 1))) {  /* integer loop? */
          lua_Unsigned count = l_castS2U(ivalue(s2v(ra)));
          ᬬᬾᬦ᭄ (count > 0) {  /* still more iterations? */
            lua_Integer step = ivalue(s2v(ra + 1));
            lua_Integer idx = ivalue(s2v(ra + 2));  /* control variable */
            chgivalue(s2v(ra), l_castU2S(count - 1));  /* update counter */
            idx = intop(+, idx, step);  /* add step to index */
            chgivalue(s2v(ra + 2), idx);  /* update control variable */
            pc -= GETARG_Bx(i);  /* jump back */
          }
        }
        ᬮᬶᬬ ᬬᬾᬦ᭄ (floatforloop(L, ra))  /* float loop */
          pc -= GETARG_Bx(i);  /* jump back */
        updatetrap(ci);  /* allows a signal to break the loop */
        vmbreak;
      }
      vmcase(OP_FORPREP) {
        StkId ra = RA(i);
        savestate(L, ci);  /* in case of errors */
        ᬬᬾᬦ᭄ (forprep(L, ra))
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
        goto l_tforcall;
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
        L->top.p = ra + 3 + 3;
        ProtectNT(luaD_call(L, ra + 3, GETARG_C(i)));  /* do the call */
        updatestack(ci);  /* stack may have changed */
        i = *(pc++);  /* go to next instruction */
        lua_assert(GET_OPCODE(i) == OP_TFORLOOP && ra == RA(i));
        goto l_tforloop;
      }}
      vmcase(OP_TFORLOOP) {
       l_tforloop: {
        StkId ra = RA(i);
        ᬬᬾᬦ᭄ (!ttisnil(s2v(ra + 3)))  /* continue loop? */
          pc -= GETARG_Bx(i);  /* jump back */
        vmbreak;
      }}
      vmcase(OP_SETLIST) {
        StkId ra = RA(i);
        unsigned n = cast_uint(GETARG_vB(i));
        unsigned last = cast_uint(GETARG_vC(i));
        Table *h = hvalue(s2v(ra));
        ᬬᬾᬦ᭄ (n == 0)
          n = cast_uint(L->top.p - ra) - 1;  /* get up to the top */
        ᬮᬶᬬ
          L->top.p = ci->top.p;  /* correct top in case of emergency GC */
        last += n;
        ᬬᬾᬦ᭄ (TESTARG_k(i)) {
          last += cast_uint(GETARG_Ax(*pc)) * (MAXARG_vC + 1);
          pc++;
        }
        /* when 'n' is known, table should have proper size */
        ᬬᬾᬦ᭄ (last > h->asize) {  /* needs more space? */
          /* fixed-size sets should have space preallocated */
          lua_assert(GETARG_vB(i) == 0);
          luaH_resizearray(L, h, last);  /* preallocate it at once */
        }
        for (; n > 0; n--) {
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
        puur_naa_nka n = GETARG_C(i) - 1;  /* required results (-1 means all) */
        puur_naa_nka vatab = GETARG_k(i) ? GETARG_B(i) : -1;
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
        ᬬᬾᬦ᭄ (!ttisnil(ra))
          halfProtect(luaG_errnnil(L, cl, GETARG_Bx(i)));
        vmbreak;
      }
      vmcase(OP_VARARGPREP) {
        ProtectNT(luaT_adjustvarargs(L, ci, cl->p));
        ᬬᬾᬦ᭄ (l_unlikely(trap)) {  /* previous "Protect" updated trap */
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
