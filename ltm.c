/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#تعریف_کرو ltm_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <string.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lgc.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "ltm.h"
#شامل "lvm.h"


ساکن ثابت حرف udatatypename[] = "userdata";

LUAI_DDEF ثابت حرف *ثابت luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


خالی luaT_init (lua_State *L) {
  ساکن ثابت حرف *ثابت luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  صحیح i;
  برائے (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ثابت TValue *luaT_gettm (جدول *events, TMS event, TString *ename) {
  ثابت TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  اگر (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    واپس تہی;
  }
  ورنہ واپس tm;
}


ثابت TValue *luaT_gettmbyobj (lua_State *L, ثابت TValue *o, TMS event) {
  جدول *mt;
  انتخاب (ttype(o)) {
    حالت LUA_TTABLE:
      mt = hvalue(o)->metatable;
      توڑو;
    حالت LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      توڑو;
    طےشدہ:
      mt = G(L)->mt[ttype(o)];
  }
  واپس (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ثابت حرف *luaT_objtypename (lua_State *L, ثابت TValue *o) {
  جدول *mt;
  اگر ((ttistable(o) && (mt = hvalue(o)->metatable) != تہی) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != تہی)) {
    ثابت TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    اگر (ttisstring(name))  /* is '__name' a string? */
      واپس getstr(tsvalue(name));  /* use it as type name */
  }
  واپس ttypename(ttype(o));  /* else use standard type name */
}


خالی luaT_callTM (lua_State *L, ثابت TValue *f, ثابت TValue *p1,
                  ثابت TValue *p2, ثابت TValue *p3) {
  StkId func = L->چوٹی.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->چوٹی.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  اگر (isLuacode(L->ci))
    luaD_call(L, func, 0);
  ورنہ
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ثابت TValue *f, ثابت TValue *p1,
                        ثابت TValue *p2, StkId res) {
  ptrdiff_t نتیجہ = savestack(L, res);
  StkId func = L->چوٹی.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->چوٹی.p += 3;
  /* metamethod may yield only when called from Lua code */
  اگر (isLuacode(L->ci))
    luaD_call(L, func, 1);
  ورنہ
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, نتیجہ);
  setobjs2s(L, res, --L->چوٹی.p);  /* move result to its place */
  واپس ttypetag(s2v(res));  /* return tag of the result */
}


ساکن صحیح callbinTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                      StkId res, TMS event) {
  ثابت TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  اگر (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  اگر (notm(tm))
    واپس -1;  /* tag method not found */
  ورنہ  /* call tag method and return the tag of the result */
    واپس luaT_callTMres(L, tm, p1, p2, res);
}


خالی luaT_trybinTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                    StkId res, TMS event) {
  اگر (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    انتخاب (event) {
      حالت TM_BAND: حالت TM_BOR: حالت TM_BXOR:
      حالت TM_SHL: حالت TM_SHR: حالت TM_BNOT: {
        اگر (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        ورنہ
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      طےشدہ:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
خالی luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->چوٹی.p - 2;  /* first argument */
  اگر (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


خالی luaT_trybinassocTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                                       صحیح flip, StkId res, TMS event) {
  اگر (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  ورنہ
    luaT_trybinTM(L, p1, p2, res, event);
}


خالی luaT_trybiniTM (lua_State *L, ثابت TValue *p1, lua_Integer i2,
                                   صحیح flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
صحیح luaT_callorderTM (lua_State *L, ثابت TValue *p1, ثابت TValue *p2,
                      TMS event) {
  صحیح لیبل = callbinTM(L, p1, p2, L->چوٹی.p, event);  /* try original event */
  اگر (لیبل >= 0)  /* found tag method? */
    واپس !tagisfalse(لیبل);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  واپس 0;  /* to avoid warnings */
}


صحیح luaT_callorderiTM (lua_State *L, ثابت TValue *p1, صحیح v2,
                       صحیح flip, صحیح isfloat, TMS event) {
  TValue aux; ثابت TValue *p2;
  اگر (isfloat) {
    setfltvalue(&aux, cast_عدد(v2));
  }
  ورنہ
    setivalue(&aux, v2);
  اگر (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  ورنہ
    p2 = &aux;
  واپس luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
ساکن خالی createvarargtab (lua_State *L, StkId f, صحیح n) {
  صحیح i;
  TValue کلید, قدر;
  جدول *t = luaH_new(L);
  sethvalue(L, s2v(L->چوٹی.p), t);
  L->چوٹی.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &کلید, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&قدر, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  lua_H_سیٹ(L, t, &کلید, &قدر);  /* t.n = n */
  برائے (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
ساکن خالی buildhiddenargs (lua_State *L, CallInfo *ci, ثابت Proto *p,
                             صحیح totalargs, صحیح nfixparams, صحیح nextra) {
  صحیح i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->چوٹی.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  برائے (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->چوٹی.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->چوٹی.p += totalargs + 1;
}


خالی luaT_adjustvarargs (lua_State *L, CallInfo *ci, ثابت Proto *p) {
  صحیح totalargs = cast_int(L->چوٹی.p - ci->func.p) - 1;
  صحیح nfixparams = p->numparams;
  صحیح nextra = totalargs - nfixparams;  /* number of extra arguments */
  اگر (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->چوٹی.p - 1);
  }
  ورنہ {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->چوٹی.p <= ci->چوٹی.p && ci->چوٹی.p <= L->ڈھیر_آخری.p);
  }
}


خالی luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  صحیح nextra = ci->u.l.nextraargs;
  lua_Integer n;
  اگر (tointegerns(rc, &n)) {  /* integral value? */
    اگر (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId خانہ = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)تہی), ra, خانہ);
      واپس;
    }
  }
  ورنہ اگر (ttisstring(rc)) {  /* string value? */
    size_t len;
    ثابت حرف *s = getlstr(tsvalue(rc), len);
    اگر (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      واپس;
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
ساکن صحیح getnumargs (lua_State *L, CallInfo *ci, جدول *h) {
  اگر (h == تہی)  /* no vararg table? */
    واپس ci->u.l.nextraargs;
  ورنہ {
    TValue res;
    اگر (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(زیادہ_سے_زیادہ_عدد/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    واپس cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
خالی luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, صحیح wanted,
                                    صحیح vatab) {
  جدول *h = (vatab < 0) ? تہی : hvalue(s2v(ci->func.p + vatab + 1));
  صحیح nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  صحیح i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  اگر (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->چوٹی.p = where + nargs;  /* next instruction will need top */
  }
  ورنہ
    touse = (nargs > wanted) ? wanted : nargs;
  اگر (h == تہی) {  /* no vararg table? */
    برائے (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  ورنہ {  /* get vararg values from vararg table */
    برائے (i = 0; i < touse; i++) {
      lu_byte لیبل = luaH_getint(h, i + 1, s2v(where + i));
      اگر (tagisempty(لیبل))
       setnilvalue2s(where + i);
    }
  }
  برائے (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

