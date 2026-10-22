/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <string.h>

#ضمن "lua.h"

#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lgc.h"
#ضمن "lobject.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "ltm.h"
#ضمن "lvm.h"


ساكن ثابت حرف udatatypename[] = "userdata";

LUAI_DDEF ثابت حرف *ثابت luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


فراغ luaT_init (lua_State *L) {
  ساكن ثابت حرف *ثابت luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  صحيح i;
  لكل (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ثابت TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  ثابت TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  ܐܢ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    ܦܢܐ عدم;
  }
  ܐܚܪܢܐ ܦܢܐ tm;
}


ثابت TValue *luaT_gettmbyobj (lua_State *L, ثابت TValue *o, TMS event) {
  Table *mt;
  بدل (ttype(o)) {
    حالة LUA_TTABLE:
      mt = hvalue(o)->metatable;
      اكسر;
    حالة LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      اكسر;
    افتراضي:
      mt = G(L)->mt[ttype(o)];
  }
  ܦܢܐ (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ثابت حرف *luaT_objtypename (lua_State *L, ثابت TValue *o) {
  Table *mt;
  ܐܢ ((ttistable(o) && (mt = hvalue(o)->metatable) != عدم) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != عدم)) {
    ثابت TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    ܐܢ (ttisstring(name))  /* is '__name' a string? */
      ܦܢܐ getstr(tsvalue(name));  /* use it as type name */
  }
  ܦܢܐ ttypename(ttype(o));  /* else use standard type name */
}


فراغ luaT_callTM (lua_State *L, ثابت TValue *f, ثابت TValue *p1,
                  ثابت TValue *p2, ثابت TValue *p3) {
  StkId func = L->top.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->top.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  ܐܢ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  ܐܚܪܢܐ
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ثابت TValue *f, ثابت TValue *p1,
                        ثابت TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->top.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->top.p += 3;
  /* metamethod may yield only when called from Lua code */
  ܐܢ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  ܐܚܪܢܐ
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->top.p);  /* move result to its place */
  ܦܢܐ ttypetag(s2v(res));  /* return tag of the result */
}


ساكن صحيح callbinTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                      StkId res, TMS event) {
  ثابت TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  ܐܢ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  ܐܢ (notm(tm))
    ܦܢܐ -1;  /* tag method not found */
  ܐܚܪܢܐ  /* call tag method and return the tag of the result */
    ܦܢܐ luaT_callTMres(L, tm, p1, p2, res);
}


فراغ luaT_trybinTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                    StkId res, TMS event) {
  ܐܢ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    بدل (event) {
      حالة TM_BAND: حالة TM_BOR: حالة TM_BXOR:
      حالة TM_SHL: حالة TM_SHR: حالة TM_BNOT: {
        ܐܢ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        ܐܚܪܢܐ
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      افتراضي:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
فراغ luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->top.p - 2;  /* first argument */
  ܐܢ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


فراغ luaT_trybinassocTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                                       صحيح flip, StkId res, TMS event) {
  ܐܢ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  ܐܚܪܢܐ
    luaT_trybinTM(L, p1, p2, res, event);
}


فراغ luaT_trybiniTM (lua_State *L, ثابت TValue *p1, lua_Integer i2,
                                   صحيح flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
صحيح luaT_callorderTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                      TMS event) {
  صحيح tag = callbinTM(L, p1, p2, L->top.p, event);  /* try original event */
  ܐܢ (tag >= 0)  /* found tag method? */
    ܦܢܐ !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  ܦܢܐ 0;  /* to avoid warnings */
}


صحيح luaT_callorderiTM (lua_State *L, ثابت TValue *p1, صحيح v2,
                       صحيح flip, صحيح isfloat, TMS event) {
  TValue aux; ثابت TValue *p2;
  ܐܢ (isfloat) {
    setfltvalue(&aux, cast_num(v2));
  }
  ܐܚܪܢܐ
    setivalue(&aux, v2);
  ܐܢ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  ܐܚܪܢܐ
    p2 = &aux;
  ܦܢܐ luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
ساكن فراغ createvarargtab (lua_State *L, StkId f, صحيح n) {
  صحيح i;
  TValue key, value;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->top.p), t);
  L->top.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &key, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&value, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &key, &value);  /* t.n = n */
  لكل (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
ساكن فراغ buildhiddenargs (lua_State *L, CallInfo *ci, ثابت Proto *p,
                             صحيح totalargs, صحيح nfixparams, صحيح nextra) {
  صحيح i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->top.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  لكل (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->top.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->top.p += totalargs + 1;
}


فراغ luaT_adjustvarargs (lua_State *L, CallInfo *ci, ثابت Proto *p) {
  صحيح totalargs = cast_int(L->top.p - ci->func.p) - 1;
  صحيح nfixparams = p->numparams;
  صحيح nextra = totalargs - nfixparams;  /* number of extra arguments */
  ܐܢ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->top.p - 1);
  }
  ܐܚܪܢܐ {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->top.p <= ci->top.p && ci->top.p <= L->stack_last.p);
  }
}


فراغ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  صحيح nextra = ci->u.l.nextraargs;
  lua_Integer n;
  ܐܢ (tointegerns(rc, &n)) {  /* integral value? */
    ܐܢ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)عدم), ra, slot);
      ܦܢܐ;
    }
  }
  ܐܚܪܢܐ ܐܢ (ttisstring(rc)) {  /* string value? */
    size_t len;
    ثابت حرف *s = getlstr(tsvalue(rc), len);
    ܐܢ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      ܦܢܐ;
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
ساكن صحيح getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  ܐܢ (h == عدم)  /* no vararg table? */
    ܦܢܐ ci->u.l.nextraargs;
  ܐܚܪܢܐ {
    TValue res;
    ܐܢ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    ܦܢܐ cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
فراغ luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, صحيح wanted,
                                    صحيح vatab) {
  Table *h = (vatab < 0) ? عدم : hvalue(s2v(ci->func.p + vatab + 1));
  صحيح nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  صحيح i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  ܐܢ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->top.p = where + nargs;  /* next instruction will need top */
  }
  ܐܚܪܢܐ
    touse = (nargs > wanted) ? wanted : nargs;
  ܐܢ (h == عدم) {  /* no vararg table? */
    لكل (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  ܐܚܪܢܐ {  /* get vararg values from vararg table */
    لكل (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      ܐܢ (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  لكل (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

