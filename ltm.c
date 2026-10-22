/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "ltm.h"
#অন্তর্ভাব "lvm.h"


স্থির ধ্রুবক অক্ষর udatatypename[] = "userdata";

LUAI_DDEF ধ্রুবক অক্ষর *ধ্রুবক luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


শূন্য luaT_init (lua_State *L) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  পূর্ণসংখ্যা i;
  জন্য (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ধ্রুবক TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  ধ্রুবক TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  যদি (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    ফেরত ফাঁকা;
  }
  অন্যথা ফেরত tm;
}


ধ্রুবক TValue *luaT_gettmbyobj (lua_State *L, ধ্রুবক TValue *o, TMS event) {
  Table *mt;
  নির্বাচন (ttype(o)) {
    ক্ষেত্র LUA_TTABLE:
      mt = hvalue(o)->metatable;
      বিরতি;
    ক্ষেত্র LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      বিরতি;
    পূর্বনির্ধারিত:
      mt = G(L)->mt[ttype(o)];
  }
  ফেরত (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ধ্রুবক অক্ষর *luaT_objtypename (lua_State *L, ধ্রুবক TValue *o) {
  Table *mt;
  যদি ((ttistable(o) && (mt = hvalue(o)->metatable) != ফাঁকা) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ফাঁকা)) {
    ধ্রুবক TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    যদি (ttisstring(name))  /* is '__name' a string? */
      ফেরত getstr(tsvalue(name));  /* use it as type name */
  }
  ফেরত ttypename(ttype(o));  /* else use standard type name */
}


শূন্য luaT_callTM (lua_State *L, ধ্রুবক TValue *f, ধ্রুবক TValue *p1,
                  ধ্রুবক TValue *p2, ধ্রুবক TValue *p3) {
  StkId func = L->শীর্ষ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->শীর্ষ.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  যদি (isLuacode(L->ci))
    luaD_call(L, func, 0);
  অন্যথা
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ধ্রুবক TValue *f, ধ্রুবক TValue *p1,
                        ধ্রুবক TValue *p2, StkId res) {
  ptrdiff_t ফলাফল = savestack(L, res);
  StkId func = L->শীর্ষ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->শীর্ষ.p += 3;
  /* metamethod may yield only when called from Lua code */
  যদি (isLuacode(L->ci))
    luaD_call(L, func, 1);
  অন্যথা
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, ফলাফল);
  setobjs2s(L, res, --L->শীর্ষ.p);  /* move result to its place */
  ফেরত ttypetag(s2v(res));  /* return tag of the result */
}


স্থির পূর্ণসংখ্যা callbinTM (lua_State *L, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2,
                      StkId res, TMS event) {
  ধ্রুবক TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  যদি (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  যদি (notm(tm))
    ফেরত -1;  /* tag method not found */
  অন্যথা  /* call tag method and return the tag of the result */
    ফেরত luaT_callTMres(L, tm, p1, p2, res);
}


শূন্য luaT_trybinTM (lua_State *L, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2,
                    StkId res, TMS event) {
  যদি (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    নির্বাচন (event) {
      ক্ষেত্র TM_BAND: ক্ষেত্র TM_BOR: ক্ষেত্র TM_BXOR:
      ক্ষেত্র TM_SHL: ক্ষেত্র TM_SHR: ক্ষেত্র TM_BNOT: {
        যদি (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        অন্যথা
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      পূর্বনির্ধারিত:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
শূন্য luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->শীর্ষ.p - 2;  /* first argument */
  যদি (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


শূন্য luaT_trybinassocTM (lua_State *L, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2,
                                       পূর্ণসংখ্যা flip, StkId res, TMS event) {
  যদি (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  অন্যথা
    luaT_trybinTM(L, p1, p2, res, event);
}


শূন্য luaT_trybiniTM (lua_State *L, ধ্রুবক TValue *p1, lua_Integer i2,
                                   পূর্ণসংখ্যা flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
পূর্ণসংখ্যা luaT_callorderTM (lua_State *L, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2,
                      TMS event) {
  পূর্ণসংখ্যা tag = callbinTM(L, p1, p2, L->শীর্ষ.p, event);  /* try original event */
  যদি (tag >= 0)  /* found tag method? */
    ফেরত !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  ফেরত 0;  /* to avoid warnings */
}


পূর্ণসংখ্যা luaT_callorderiTM (lua_State *L, ধ্রুবক TValue *p1, পূর্ণসংখ্যা v2,
                       পূর্ণসংখ্যা flip, পূর্ণসংখ্যা isfloat, TMS event) {
  TValue aux; ধ্রুবক TValue *p2;
  যদি (isfloat) {
    setfltvalue(&aux, cast_অঙ্ক(v2));
  }
  অন্যথা
    setivalue(&aux, v2);
  যদি (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  অন্যথা
    p2 = &aux;
  ফেরত luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
স্থির শূন্য createvarargtab (lua_State *L, StkId f, পূর্ণসংখ্যা n) {
  পূর্ণসংখ্যা i;
  TValue চাবি, মান;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->শীর্ষ.p), t);
  L->শীর্ষ.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &চাবি, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&মান, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &চাবি, &মান);  /* t.n = n */
  জন্য (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
স্থির শূন্য buildhiddenargs (lua_State *L, CallInfo *ci, ধ্রুবক Proto *p,
                             পূর্ণসংখ্যা totalargs, পূর্ণসংখ্যা nfixparams, পূর্ণসংখ্যা nextra) {
  পূর্ণসংখ্যা i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->শীর্ষ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  জন্য (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->শীর্ষ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->শীর্ষ.p += totalargs + 1;
}


শূন্য luaT_adjustvarargs (lua_State *L, CallInfo *ci, ধ্রুবক Proto *p) {
  পূর্ণসংখ্যা totalargs = cast_int(L->শীর্ষ.p - ci->func.p) - 1;
  পূর্ণসংখ্যা nfixparams = p->numparams;
  পূর্ণসংখ্যা nextra = totalargs - nfixparams;  /* number of extra arguments */
  যদি (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->শীর্ষ.p - 1);
  }
  অন্যথা {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->শীর্ষ.p <= ci->শীর্ষ.p && ci->শীর্ষ.p <= L->স্তূপ_last.p);
  }
}


শূন্য luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  পূর্ণসংখ্যা nextra = ci->u.l.nextraargs;
  lua_Integer n;
  যদি (tointegerns(rc, &n)) {  /* integral value? */
    যদি (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ফাঁকা), ra, slot);
      ফেরত;
    }
  }
  অন্যথা যদি (ttisstring(rc)) {  /* string value? */
    size_t len;
    ধ্রুবক অক্ষর *s = getlstr(tsvalue(rc), len);
    যদি (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      ফেরত;
    }
  }
  setnilvalue2s(ra);  /* else produce nil */
}


/*
** Get the number of extra arguments in a vararg function. If vararg
** table has been optimized away, that number is in the call info.
** Otherwise, get the field 'n' from the vararg table and check that it
** has a proper value (non-negative integer not larger than the stack
** limit).
*/
স্থির পূর্ণসংখ্যা getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  যদি (h == ফাঁকা)  /* no vararg table? */
    ফেরত ci->u.l.nextraargs;
  অন্যথা {
    TValue res;
    যদি (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    ফেরত cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
শূন্য luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, পূর্ণসংখ্যা wanted,
                                    পূর্ণসংখ্যা vatab) {
  Table *h = (vatab < 0) ? ফাঁকা : hvalue(s2v(ci->func.p + vatab + 1));
  পূর্ণসংখ্যা nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  পূর্ণসংখ্যা i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  যদি (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->শীর্ষ.p = where + nargs;  /* next instruction will need top */
  }
  অন্যথা
    touse = (nargs > wanted) ? wanted : nargs;
  যদি (h == ফাঁকা) {  /* no vararg table? */
    জন্য (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  অন্যথা {  /* get vararg values from vararg table */
    জন্য (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      যদি (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  জন্য (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

