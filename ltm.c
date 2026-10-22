/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ udatatypename[] = "userdata";

LUAI_DDEF ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


ಶೂನ್ಯ luaT_init (lua_State *L) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ಧ್ರುವ TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  ಧ್ರುವ TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  ಯದಿ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ tm;
}


ಧ್ರುವ TValue *luaT_gettmbyobj (lua_State *L, ಧ್ರುವ TValue *o, TMS event) {
  Table *mt;
  ವಿಕಲ್ಪ (ttype(o)) {
    ಪಕ್ಷ LUA_TTABLE:
      mt = hvalue(o)->metatable;
      ಭಙ್ಗ;
    ಪಕ್ಷ LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      mt = G(L)->mt[ttype(o)];
  }
  ಪ್ರತ್ಯಾಗಮ (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ಧ್ರುವ ವರ್ಣ *luaT_objtypename (lua_State *L, ಧ್ರುವ TValue *o) {
  Table *mt;
  ಯದಿ ((ttistable(o) && (mt = hvalue(o)->metatable) != ರಿಕ್ತ) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ರಿಕ್ತ)) {
    ಧ್ರುವ TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    ಯದಿ (ttisstring(name))  /* is '__name' a string? */
      ಪ್ರತ್ಯಾಗಮ getstr(tsvalue(name));  /* use it as type name */
  }
  ಪ್ರತ್ಯಾಗಮ ttypename(ttype(o));  /* else use standard type name */
}


ಶೂನ್ಯ luaT_callTM (lua_State *L, ಧ್ರುವ TValue *f, ಧ್ರುವ TValue *p1,
                  ಧ್ರುವ TValue *p2, ಧ್ರುವ TValue *p3) {
  StkId func = L->ತುದಿ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->ತುದಿ.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  ಯದಿ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  ಅನ್ಯಥಾ
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ಧ್ರುವ TValue *f, ಧ್ರುವ TValue *p1,
                        ಧ್ರುವ TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->ತುದಿ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->ತುದಿ.p += 3;
  /* metamethod may yield only when called from Lua code */
  ಯದಿ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  ಅನ್ಯಥಾ
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->ತುದಿ.p);  /* move result to its place */
  ಪ್ರತ್ಯಾಗಮ ttypetag(s2v(res));  /* return tag of the result */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ callbinTM (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2,
                      StkId res, TMS event) {
  ಧ್ರುವ TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  ಯದಿ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  ಯದಿ (notm(tm))
    ಪ್ರತ್ಯಾಗಮ -1;  /* tag method not found */
  ಅನ್ಯಥಾ  /* call tag method and return the tag of the result */
    ಪ್ರತ್ಯಾಗಮ luaT_callTMres(L, tm, p1, p2, res);
}


ಶೂನ್ಯ luaT_trybinTM (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2,
                    StkId res, TMS event) {
  ಯದಿ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    ವಿಕಲ್ಪ (event) {
      ಪಕ್ಷ TM_BAND: ಪಕ್ಷ TM_BOR: ಪಕ್ಷ TM_BXOR:
      ಪಕ್ಷ TM_SHL: ಪಕ್ಷ TM_SHR: ಪಕ್ಷ TM_BNOT: {
        ಯದಿ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        ಅನ್ಯಥಾ
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      ಸ್ವಭಾವ:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
ಶೂನ್ಯ luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->ತುದಿ.p - 2;  /* first argument */
  ಯದಿ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


ಶೂನ್ಯ luaT_trybinassocTM (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2,
                                       ಪೂರ್ಣಾಙ್ಕ flip, StkId res, TMS event) {
  ಯದಿ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  ಅನ್ಯಥಾ
    luaT_trybinTM(L, p1, p2, res, event);
}


ಶೂನ್ಯ luaT_trybiniTM (lua_State *L, ಧ್ರುವ TValue *p1, lua_Integer i2,
                                   ಪೂರ್ಣಾಙ್ಕ flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
ಪೂರ್ಣಾಙ್ಕ luaT_callorderTM (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2,
                      TMS event) {
  ಪೂರ್ಣಾಙ್ಕ tag = callbinTM(L, p1, p2, L->ತುದಿ.p, event);  /* try original event */
  ಯದಿ (tag >= 0)  /* found tag method? */
    ಪ್ರತ್ಯಾಗಮ !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  ಪ್ರತ್ಯಾಗಮ 0;  /* to avoid warnings */
}


ಪೂರ್ಣಾಙ್ಕ luaT_callorderiTM (lua_State *L, ಧ್ರುವ TValue *p1, ಪೂರ್ಣಾಙ್ಕ v2,
                       ಪೂರ್ಣಾಙ್ಕ flip, ಪೂರ್ಣಾಙ್ಕ isfloat, TMS event) {
  TValue aux; ಧ್ರುವ TValue *p2;
  ಯದಿ (isfloat) {
    setfltvalue(&aux, cast_ಅಂಕೆ(v2));
  }
  ಅನ್ಯಥಾ
    setivalue(&aux, v2);
  ಯದಿ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  ಅನ್ಯಥಾ
    p2 = &aux;
  ಪ್ರತ್ಯಾಗಮ luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ createvarargtab (lua_State *L, StkId f, ಪೂರ್ಣಾಙ್ಕ n) {
  ಪೂರ್ಣಾಙ್ಕ i;
  TValue ಕೀಲಿ, ಮೌಲ್ಯ;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->ತುದಿ.p), t);
  L->ತುದಿ.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &ಕೀಲಿ, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&ಮೌಲ್ಯ, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &ಕೀಲಿ, &ಮೌಲ್ಯ);  /* t.n = n */
  ಪ್ರತಿ (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
ಸ್ಥಿರ ಶೂನ್ಯ buildhiddenargs (lua_State *L, CallInfo *ci, ಧ್ರುವ Proto *p,
                             ಪೂರ್ಣಾಙ್ಕ totalargs, ಪೂರ್ಣಾಙ್ಕ nfixparams, ಪೂರ್ಣಾಙ್ಕ nextra) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->ತುದಿ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  ಪ್ರತಿ (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->ತುದಿ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->ತುದಿ.p += totalargs + 1;
}


ಶೂನ್ಯ luaT_adjustvarargs (lua_State *L, CallInfo *ci, ಧ್ರುವ Proto *p) {
  ಪೂರ್ಣಾಙ್ಕ totalargs = cast_int(L->ತುದಿ.p - ci->func.p) - 1;
  ಪೂರ್ಣಾಙ್ಕ nfixparams = p->numparams;
  ಪೂರ್ಣಾಙ್ಕ nextra = totalargs - nfixparams;  /* number of extra arguments */
  ಯದಿ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->ತುದಿ.p - 1);
  }
  ಅನ್ಯಥಾ {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->ತುದಿ.p <= ci->ತುದಿ.p && ci->ತುದಿ.p <= L->ರಾಶಿ_last.p);
  }
}


ಶೂನ್ಯ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  ಪೂರ್ಣಾಙ್ಕ nextra = ci->u.l.nextraargs;
  lua_Integer n;
  ಯದಿ (tointegerns(rc, &n)) {  /* integral value? */
    ಯದಿ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ರಿಕ್ತ), ra, slot);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  ಅನ್ಯಥಾ ಯದಿ (ttisstring(rc)) {  /* string value? */
    size_t len;
    ಧ್ರುವ ವರ್ಣ *s = getlstr(tsvalue(rc), len);
    ಯದಿ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  ಯದಿ (h == ರಿಕ್ತ)  /* no vararg table? */
    ಪ್ರತ್ಯಾಗಮ ci->u.l.nextraargs;
  ಅನ್ಯಥಾ {
    TValue res;
    ಯದಿ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    ಪ್ರತ್ಯಾಗಮ cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
ಶೂನ್ಯ luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, ಪೂರ್ಣಾಙ್ಕ wanted,
                                    ಪೂರ್ಣಾಙ್ಕ vatab) {
  Table *h = (vatab < 0) ? ರಿಕ್ತ : hvalue(s2v(ci->func.p + vatab + 1));
  ಪೂರ್ಣಾಙ್ಕ nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  ಪೂರ್ಣಾಙ್ಕ i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  ಯದಿ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->ತುದಿ.p = where + nargs;  /* next instruction will need top */
  }
  ಅನ್ಯಥಾ
    touse = (nargs > wanted) ? wanted : nargs;
  ಯದಿ (h == ರಿಕ್ತ) {  /* no vararg table? */
    ಪ್ರತಿ (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  ಅನ್ಯಥಾ {  /* get vararg values from vararg table */
    ಪ್ರತಿ (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      ಯದಿ (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  ಪ್ರತಿ (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

