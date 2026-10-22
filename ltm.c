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


স্থির ধ্রুব বর্ণ udatatypename[] = "userdata";

LUAI_DDEF ধ্রুব বর্ণ *ধ্রুব luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


শূন্য luaT_init (lua_State *L) {
  স্থির ধ্রুব বর্ণ *ধ্রুব luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  পূর্ণাঙ্ক i;
  প্রতি (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ধ্রুব TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  ধ্রুব TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  যদি (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    প্রত্যাগম রিক্ত;
  }
  অন্যথা প্রত্যাগম tm;
}


ধ্রুব TValue *luaT_gettmbyobj (lua_State *L, ধ্রুব TValue *o, TMS event) {
  Table *mt;
  বিকল্প (ttype(o)) {
    পক্ষ LUA_TTABLE:
      mt = hvalue(o)->metatable;
      ভঙ্গ;
    পক্ষ LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      ভঙ্গ;
    স্বভাব:
      mt = G(L)->mt[ttype(o)];
  }
  প্রত্যাগম (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ধ্রুব বর্ণ *luaT_objtypename (lua_State *L, ধ্রুব TValue *o) {
  Table *mt;
  যদি ((ttistable(o) && (mt = hvalue(o)->metatable) != রিক্ত) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != রিক্ত)) {
    ধ্রুব TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    যদি (ttisstring(name))  /* is '__name' a string? */
      প্রত্যাগম getstr(tsvalue(name));  /* use it as type name */
  }
  প্রত্যাগম ttypename(ttype(o));  /* else use standard type name */
}


শূন্য luaT_callTM (lua_State *L, ধ্রুব TValue *f, ধ্রুব TValue *p1,
                  ধ্রুব TValue *p2, ধ্রুব TValue *p3) {
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


lu_byte luaT_callTMres (lua_State *L, ধ্রুব TValue *f, ধ্রুব TValue *p1,
                        ধ্রুব TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
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
  res = restorestack(L, result);
  setobjs2s(L, res, --L->শীর্ষ.p);  /* move result to its place */
  প্রত্যাগম ttypetag(s2v(res));  /* return tag of the result */
}


স্থির পূর্ণাঙ্ক callbinTM (lua_State *L, ধ্রুব TValue *p1, ধ্রুব TValue *p2,
                      StkId res, TMS event) {
  ধ্রুব TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  যদি (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  যদি (notm(tm))
    প্রত্যাগম -1;  /* tag method not found */
  অন্যথা  /* call tag method and return the tag of the result */
    প্রত্যাগম luaT_callTMres(L, tm, p1, p2, res);
}


শূন্য luaT_trybinTM (lua_State *L, ধ্রুব TValue *p1, ধ্রুব TValue *p2,
                    StkId res, TMS event) {
  যদি (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    বিকল্প (event) {
      পক্ষ TM_BAND: পক্ষ TM_BOR: পক্ষ TM_BXOR:
      পক্ষ TM_SHL: পক্ষ TM_SHR: পক্ষ TM_BNOT: {
        যদি (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        অন্যথা
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      স্বভাব:
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


শূন্য luaT_trybinassocTM (lua_State *L, ধ্রুব TValue *p1, ধ্রুব TValue *p2,
                                       পূর্ণাঙ্ক flip, StkId res, TMS event) {
  যদি (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  অন্যথা
    luaT_trybinTM(L, p1, p2, res, event);
}


শূন্য luaT_trybiniTM (lua_State *L, ধ্রুব TValue *p1, lua_Integer i2,
                                   পূর্ণাঙ্ক flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
পূর্ণাঙ্ক luaT_callorderTM (lua_State *L, ধ্রুব TValue *p1, ধ্রুব TValue *p2,
                      TMS event) {
  পূর্ণাঙ্ক tag = callbinTM(L, p1, p2, L->শীর্ষ.p, event);  /* try original event */
  যদি (tag >= 0)  /* found tag method? */
    প্রত্যাগম !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  প্রত্যাগম 0;  /* to avoid warnings */
}


পূর্ণাঙ্ক luaT_callorderiTM (lua_State *L, ধ্রুব TValue *p1, পূর্ণাঙ্ক v2,
                       পূর্ণাঙ্ক flip, পূর্ণাঙ্ক isfloat, TMS event) {
  TValue aux; ধ্রুব TValue *p2;
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
  প্রত্যাগম luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
স্থির শূন্য createvarargtab (lua_State *L, StkId f, পূর্ণাঙ্ক n) {
  পূর্ণাঙ্ক i;
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
  প্রতি (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
স্থির শূন্য buildhiddenargs (lua_State *L, CallInfo *ci, ধ্রুব Proto *p,
                             পূর্ণাঙ্ক totalargs, পূর্ণাঙ্ক nfixparams, পূর্ণাঙ্ক nextra) {
  পূর্ণাঙ্ক i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->শীর্ষ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  প্রতি (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->শীর্ষ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->শীর্ষ.p += totalargs + 1;
}


শূন্য luaT_adjustvarargs (lua_State *L, CallInfo *ci, ধ্রুব Proto *p) {
  পূর্ণাঙ্ক totalargs = cast_int(L->শীর্ষ.p - ci->func.p) - 1;
  পূর্ণাঙ্ক nfixparams = p->numparams;
  পূর্ণাঙ্ক nextra = totalargs - nfixparams;  /* number of extra arguments */
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
  পূর্ণাঙ্ক nextra = ci->u.l.nextraargs;
  lua_Integer n;
  যদি (tointegerns(rc, &n)) {  /* integral value? */
    যদি (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)রিক্ত), ra, slot);
      প্রত্যাগম;
    }
  }
  অন্যথা যদি (ttisstring(rc)) {  /* string value? */
    size_t len;
    ধ্রুব বর্ণ *s = getlstr(tsvalue(rc), len);
    যদি (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      প্রত্যাগম;
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
স্থির পূর্ণাঙ্ক getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  যদি (h == রিক্ত)  /* no vararg table? */
    প্রত্যাগম ci->u.l.nextraargs;
  অন্যথা {
    TValue res;
    যদি (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    প্রত্যাগম cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
শূন্য luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, পূর্ণাঙ্ক wanted,
                                    পূর্ণাঙ্ক vatab) {
  Table *h = (vatab < 0) ? রিক্ত : hvalue(s2v(ci->func.p + vatab + 1));
  পূর্ণাঙ্ক nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  পূর্ণাঙ্ক i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  যদি (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->শীর্ষ.p = where + nargs;  /* next instruction will need top */
  }
  অন্যথা
    touse = (nargs > wanted) ? wanted : nargs;
  যদি (h == রিক্ত) {  /* no vararg table? */
    প্রতি (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  অন্যথা {  /* get vararg values from vararg table */
    প্রতি (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      যদি (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  প্রতি (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

