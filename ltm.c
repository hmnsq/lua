/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#నిర్వచించు ltm_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "ltm.h"
#అన్తర్భావ "lvm.h"


స్థిర స్థిరరాశి అక్షరం udatatypename[] = "userdata";

LUAI_DDEF స్థిరరాశి అక్షరం *స్థిరరాశి luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


శూన్య luaT_init (lua_State *L) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  పూర్ణాంకం i;
  కొరకు (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
స్థిరరాశి TValue *luaT_gettm (పట్టిక *events, TMS event, TString *ename) {
  స్థిరరాశి TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  ఒకవేళ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    తిరిగి ఖాళీ;
  }
  లేకపోతే తిరిగి tm;
}


స్థిరరాశి TValue *luaT_gettmbyobj (lua_State *L, స్థిరరాశి TValue *o, TMS event) {
  పట్టిక *mt;
  ఎంపిక (ttype(o)) {
    స్థితి LUA_TTABLE:
      mt = hvalue(o)->metatable;
      ఆపు;
    స్థితి LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      ఆపు;
    అప్రమేయ:
      mt = G(L)->mt[ttype(o)];
  }
  తిరిగి (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
స్థిరరాశి అక్షరం *luaT_objtypename (lua_State *L, స్థిరరాశి TValue *o) {
  పట్టిక *mt;
  ఒకవేళ ((ttistable(o) && (mt = hvalue(o)->metatable) != ఖాళీ) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ఖాళీ)) {
    స్థిరరాశి TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    ఒకవేళ (ttisstring(name))  /* is '__name' a string? */
      తిరిగి getstr(tsvalue(name));  /* use it as type name */
  }
  తిరిగి ttypename(ttype(o));  /* else use standard type name */
}


శూన్య luaT_callTM (lua_State *L, స్థిరరాశి TValue *f, స్థిరరాశి TValue *p1,
                  స్థిరరాశి TValue *p2, స్థిరరాశి TValue *p3) {
  StkId func = L->శిఖరం.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->శిఖరం.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  ఒకవేళ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  లేకపోతే
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, స్థిరరాశి TValue *f, స్థిరరాశి TValue *p1,
                        స్థిరరాశి TValue *p2, StkId res) {
  ptrdiff_t ఫలితం = savestack(L, res);
  StkId func = L->శిఖరం.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->శిఖరం.p += 3;
  /* metamethod may yield only when called from Lua code */
  ఒకవేళ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  లేకపోతే
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, ఫలితం);
  setobjs2s(L, res, --L->శిఖరం.p);  /* move result to its place */
  తిరిగి ttypetag(s2v(res));  /* return tag of the result */
}


స్థిర పూర్ణాంకం callbinTM (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2,
                      StkId res, TMS event) {
  స్థిరరాశి TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  ఒకవేళ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  ఒకవేళ (notm(tm))
    తిరిగి -1;  /* tag method not found */
  లేకపోతే  /* call tag method and return the tag of the result */
    తిరిగి luaT_callTMres(L, tm, p1, p2, res);
}


శూన్య luaT_trybinTM (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2,
                    StkId res, TMS event) {
  ఒకవేళ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    ఎంపిక (event) {
      స్థితి TM_BAND: స్థితి TM_BOR: స్థితి TM_BXOR:
      స్థితి TM_SHL: స్థితి TM_SHR: స్థితి TM_BNOT: {
        ఒకవేళ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        లేకపోతే
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      అప్రమేయ:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
శూన్య luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->శిఖరం.p - 2;  /* first argument */
  ఒకవేళ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


శూన్య luaT_trybinassocTM (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2,
                                       పూర్ణాంకం flip, StkId res, TMS event) {
  ఒకవేళ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  లేకపోతే
    luaT_trybinTM(L, p1, p2, res, event);
}


శూన్య luaT_trybiniTM (lua_State *L, స్థిరరాశి TValue *p1, lua_Integer i2,
                                   పూర్ణాంకం flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
పూర్ణాంకం luaT_callorderTM (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2,
                      TMS event) {
  పూర్ణాంకం ట్యాగ్ = callbinTM(L, p1, p2, L->శిఖరం.p, event);  /* try original event */
  ఒకవేళ (ట్యాగ్ >= 0)  /* found tag method? */
    తిరిగి !tagisfalse(ట్యాగ్);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  తిరిగి 0;  /* to avoid warnings */
}


పూర్ణాంకం luaT_callorderiTM (lua_State *L, స్థిరరాశి TValue *p1, పూర్ణాంకం v2,
                       పూర్ణాంకం flip, పూర్ణాంకం isfloat, TMS event) {
  TValue aux; స్థిరరాశి TValue *p2;
  ఒకవేళ (isfloat) {
    setfltvalue(&aux, cast_అంకె(v2));
  }
  లేకపోతే
    setivalue(&aux, v2);
  ఒకవేళ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  లేకపోతే
    p2 = &aux;
  తిరిగి luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
స్థిర శూన్య createvarargtab (lua_State *L, StkId f, పూర్ణాంకం n) {
  పూర్ణాంకం i;
  TValue కీలకం, విలువ;
  పట్టిక *t = luaH_new(L);
  sethvalue(L, s2v(L->శిఖరం.p), t);
  L->శిఖరం.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &కీలకం, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&విలువ, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  lua_H_సమితి(L, t, &కీలకం, &విలువ);  /* t.n = n */
  కొరకు (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
స్థిర శూన్య buildhiddenargs (lua_State *L, CallInfo *ci, స్థిరరాశి Proto *p,
                             పూర్ణాంకం totalargs, పూర్ణాంకం nfixparams, పూర్ణాంకం nextra) {
  పూర్ణాంకం i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->శిఖరం.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  కొరకు (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->శిఖరం.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->శిఖరం.p += totalargs + 1;
}


శూన్య luaT_adjustvarargs (lua_State *L, CallInfo *ci, స్థిరరాశి Proto *p) {
  పూర్ణాంకం totalargs = cast_int(L->శిఖరం.p - ci->func.p) - 1;
  పూర్ణాంకం nfixparams = p->numparams;
  పూర్ణాంకం nextra = totalargs - nfixparams;  /* number of extra arguments */
  ఒకవేళ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->శిఖరం.p - 1);
  }
  లేకపోతే {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->శిఖరం.p <= ci->శిఖరం.p && ci->శిఖరం.p <= L->దొంతర_చివరి.p);
  }
}


శూన్య luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  పూర్ణాంకం nextra = ci->u.l.nextraargs;
  lua_Integer n;
  ఒకవేళ (tointegerns(rc, &n)) {  /* integral value? */
    ఒకవేళ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId స్థానం = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ఖాళీ), ra, స్థానం);
      తిరిగి;
    }
  }
  లేకపోతే ఒకవేళ (ttisstring(rc)) {  /* string value? */
    size_t len;
    స్థిరరాశి అక్షరం *s = getlstr(tsvalue(rc), len);
    ఒకవేళ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      తిరిగి;
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
స్థిర పూర్ణాంకం getnumargs (lua_State *L, CallInfo *ci, పట్టిక *h) {
  ఒకవేళ (h == ఖాళీ)  /* no vararg table? */
    తిరిగి ci->u.l.nextraargs;
  లేకపోతే {
    TValue res;
    ఒకవేళ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(గరిష్ఠ_పూర్ణాంకం/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    తిరిగి cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
శూన్య luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, పూర్ణాంకం wanted,
                                    పూర్ణాంకం vatab) {
  పట్టిక *h = (vatab < 0) ? ఖాళీ : hvalue(s2v(ci->func.p + vatab + 1));
  పూర్ణాంకం nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  పూర్ణాంకం i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  ఒకవేళ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->శిఖరం.p = where + nargs;  /* next instruction will need top */
  }
  లేకపోతే
    touse = (nargs > wanted) ? wanted : nargs;
  ఒకవేళ (h == ఖాళీ) {  /* no vararg table? */
    కొరకు (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  లేకపోతే {  /* get vararg values from vararg table */
    కొరకు (i = 0; i < touse; i++) {
      lu_byte ట్యాగ్ = luaH_getint(h, i + 1, s2v(where + i));
      ఒకవేళ (tagisempty(ట్యాగ్))
       setnilvalue2s(where + i);
    }
  }
  కొరకు (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

