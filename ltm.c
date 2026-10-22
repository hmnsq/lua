/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltm_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"


ਸਥਿਰ ਅਚਲ ਅੱਖਰ udatatypename[] = "userdata";

LUAI_DDEF ਅਚਲ ਅੱਖਰ *ਅਚਲ luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


ਖਾਲੀ luaT_init (lua_State *L) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  ਪੂਰਨ i;
  ਲਈ (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ਅਚਲ TValue *luaT_gettm (ਸਾਰਣੀ *events, TMS event, TString *ename) {
  ਅਚਲ TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  ਜੇ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    ਵਾਪਸ ਨਿਰਾ;
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ tm;
}


ਅਚਲ TValue *luaT_gettmbyobj (lua_State *L, ਅਚਲ TValue *o, TMS event) {
  ਸਾਰਣੀ *mt;
  ਚੋਣ (ttype(o)) {
    ਹਾਲਤ LUA_TTABLE:
      mt = hvalue(o)->metatable;
      ਰੋਕੋ;
    ਹਾਲਤ LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      ਰੋਕੋ;
    ਮੂਲ:
      mt = G(L)->mt[ttype(o)];
  }
  ਵਾਪਸ (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ਅਚਲ ਅੱਖਰ *luaT_objtypename (lua_State *L, ਅਚਲ TValue *o) {
  ਸਾਰਣੀ *mt;
  ਜੇ ((ttistable(o) && (mt = hvalue(o)->metatable) != ਨਿਰਾ) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ਨਿਰਾ)) {
    ਅਚਲ TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    ਜੇ (ttisstring(name))  /* is '__name' a string? */
      ਵਾਪਸ getstr(tsvalue(name));  /* use it as type name */
  }
  ਵਾਪਸ ttypename(ttype(o));  /* else use standard type name */
}


ਖਾਲੀ luaT_callTM (lua_State *L, ਅਚਲ TValue *f, ਅਚਲ TValue *p1,
                  ਅਚਲ TValue *p2, ਅਚਲ TValue *p3) {
  StkId func = L->ਸਿਖਰ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->ਸਿਖਰ.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  ਜੇ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  ਨਹੀਂਤਾਂ
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ਅਚਲ TValue *f, ਅਚਲ TValue *p1,
                        ਅਚਲ TValue *p2, StkId res) {
  ptrdiff_t ਨਤੀਜਾ = savestack(L, res);
  StkId func = L->ਸਿਖਰ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->ਸਿਖਰ.p += 3;
  /* metamethod may yield only when called from Lua code */
  ਜੇ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  ਨਹੀਂਤਾਂ
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, ਨਤੀਜਾ);
  setobjs2s(L, res, --L->ਸਿਖਰ.p);  /* move result to its place */
  ਵਾਪਸ ttypetag(s2v(res));  /* return tag of the result */
}


ਸਥਿਰ ਪੂਰਨ callbinTM (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2,
                      StkId res, TMS event) {
  ਅਚਲ TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  ਜੇ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  ਜੇ (notm(tm))
    ਵਾਪਸ -1;  /* tag method not found */
  ਨਹੀਂਤਾਂ  /* call tag method and return the tag of the result */
    ਵਾਪਸ luaT_callTMres(L, tm, p1, p2, res);
}


ਖਾਲੀ luaT_trybinTM (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2,
                    StkId res, TMS event) {
  ਜੇ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    ਚੋਣ (event) {
      ਹਾਲਤ TM_BAND: ਹਾਲਤ TM_BOR: ਹਾਲਤ TM_BXOR:
      ਹਾਲਤ TM_SHL: ਹਾਲਤ TM_SHR: ਹਾਲਤ TM_BNOT: {
        ਜੇ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        ਨਹੀਂਤਾਂ
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      ਮੂਲ:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
ਖਾਲੀ luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->ਸਿਖਰ.p - 2;  /* first argument */
  ਜੇ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


ਖਾਲੀ luaT_trybinassocTM (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2,
                                       ਪੂਰਨ flip, StkId res, TMS event) {
  ਜੇ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  ਨਹੀਂਤਾਂ
    luaT_trybinTM(L, p1, p2, res, event);
}


ਖਾਲੀ luaT_trybiniTM (lua_State *L, ਅਚਲ TValue *p1, lua_Integer i2,
                                   ਪੂਰਨ flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
ਪੂਰਨ luaT_callorderTM (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2,
                      TMS event) {
  ਪੂਰਨ ਟੈਗ = callbinTM(L, p1, p2, L->ਸਿਖਰ.p, event);  /* try original event */
  ਜੇ (ਟੈਗ >= 0)  /* found tag method? */
    ਵਾਪਸ !tagisfalse(ਟੈਗ);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  ਵਾਪਸ 0;  /* to avoid warnings */
}


ਪੂਰਨ luaT_callorderiTM (lua_State *L, ਅਚਲ TValue *p1, ਪੂਰਨ v2,
                       ਪੂਰਨ flip, ਪੂਰਨ isfloat, TMS event) {
  TValue aux; ਅਚਲ TValue *p2;
  ਜੇ (isfloat) {
    setfltvalue(&aux, cast_ਅੰਕ(v2));
  }
  ਨਹੀਂਤਾਂ
    setivalue(&aux, v2);
  ਜੇ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  ਨਹੀਂਤਾਂ
    p2 = &aux;
  ਵਾਪਸ luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
ਸਥਿਰ ਖਾਲੀ createvarargtab (lua_State *L, StkId f, ਪੂਰਨ n) {
  ਪੂਰਨ i;
  TValue ਕੁੰਜੀ, ਮੁੱਲ;
  ਸਾਰਣੀ *t = luaH_new(L);
  sethvalue(L, s2v(L->ਸਿਖਰ.p), t);
  L->ਸਿਖਰ.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &ਕੁੰਜੀ, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&ਮੁੱਲ, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  lua_H_ਸੈੱਟ(L, t, &ਕੁੰਜੀ, &ਮੁੱਲ);  /* t.n = n */
  ਲਈ (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
ਸਥਿਰ ਖਾਲੀ buildhiddenargs (lua_State *L, CallInfo *ci, ਅਚਲ Proto *p,
                             ਪੂਰਨ totalargs, ਪੂਰਨ nfixparams, ਪੂਰਨ nextra) {
  ਪੂਰਨ i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->ਸਿਖਰ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  ਲਈ (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->ਸਿਖਰ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->ਸਿਖਰ.p += totalargs + 1;
}


ਖਾਲੀ luaT_adjustvarargs (lua_State *L, CallInfo *ci, ਅਚਲ Proto *p) {
  ਪੂਰਨ totalargs = cast_int(L->ਸਿਖਰ.p - ci->func.p) - 1;
  ਪੂਰਨ nfixparams = p->numparams;
  ਪੂਰਨ nextra = totalargs - nfixparams;  /* number of extra arguments */
  ਜੇ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->ਸਿਖਰ.p - 1);
  }
  ਨਹੀਂਤਾਂ {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->ਸਿਖਰ.p <= ci->ਸਿਖਰ.p && ci->ਸਿਖਰ.p <= L->ਢੇਰ_ਆਖਰੀ.p);
  }
}


ਖਾਲੀ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  ਪੂਰਨ nextra = ci->u.l.nextraargs;
  lua_Integer n;
  ਜੇ (tointegerns(rc, &n)) {  /* integral value? */
    ਜੇ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId ਥਾਂ = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ਨਿਰਾ), ra, ਥਾਂ);
      ਵਾਪਸ;
    }
  }
  ਨਹੀਂਤਾਂ ਜੇ (ttisstring(rc)) {  /* string value? */
    size_t len;
    ਅਚਲ ਅੱਖਰ *s = getlstr(tsvalue(rc), len);
    ਜੇ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      ਵਾਪਸ;
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
ਸਥਿਰ ਪੂਰਨ getnumargs (lua_State *L, CallInfo *ci, ਸਾਰਣੀ *h) {
  ਜੇ (h == ਨਿਰਾ)  /* no vararg table? */
    ਵਾਪਸ ci->u.l.nextraargs;
  ਨਹੀਂਤਾਂ {
    TValue res;
    ਜੇ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    ਵਾਪਸ cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
ਖਾਲੀ luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, ਪੂਰਨ wanted,
                                    ਪੂਰਨ vatab) {
  ਸਾਰਣੀ *h = (vatab < 0) ? ਨਿਰਾ : hvalue(s2v(ci->func.p + vatab + 1));
  ਪੂਰਨ nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  ਪੂਰਨ i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  ਜੇ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->ਸਿਖਰ.p = where + nargs;  /* next instruction will need top */
  }
  ਨਹੀਂਤਾਂ
    touse = (nargs > wanted) ? wanted : nargs;
  ਜੇ (h == ਨਿਰਾ) {  /* no vararg table? */
    ਲਈ (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  ਨਹੀਂਤਾਂ {  /* get vararg values from vararg table */
    ਲਈ (i = 0; i < touse; i++) {
      lu_byte ਟੈਗ = luaH_getint(h, i + 1, s2v(where + i));
      ਜੇ (tagisempty(ਟੈਗ))
       setnilvalue2s(where + i);
    }
  }
  ਲਈ (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

