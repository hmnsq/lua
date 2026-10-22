/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ltm_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

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


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ udatatypename[] = "userdata";

LUAI_DDEF ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


ಶೂನ್ಯ luaT_init (lua_State *L) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ಸ್ಥಿರಾಂಕ TValue *luaT_gettm (ಕೋಷ್ಟಕ *events, TMS event, TString *ename) {
  ಸ್ಥಿರಾಂಕ TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  ಆದರೆ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ tm;
}


ಸ್ಥಿರಾಂಕ TValue *luaT_gettmbyobj (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o, TMS event) {
  ಕೋಷ್ಟಕ *mt;
  ಆಯ್ಕೆ (ttype(o)) {
    ಸ್ಥಿತಿ LUA_TTABLE:
      mt = hvalue(o)->metatable;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      mt = G(L)->mt[ttype(o)];
  }
  ಹಿಂತಿರುಗಿ (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaT_objtypename (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o) {
  ಕೋಷ್ಟಕ *mt;
  ಆದರೆ ((ttistable(o) && (mt = hvalue(o)->metatable) != ಖಾಲಿ) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ಖಾಲಿ)) {
    ಸ್ಥಿರಾಂಕ TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    ಆದರೆ (ttisstring(name))  /* is '__name' a string? */
      ಹಿಂತಿರುಗಿ getstr(tsvalue(name));  /* use it as type name */
  }
  ಹಿಂತಿರುಗಿ ttypename(ttype(o));  /* else use standard type name */
}


ಶೂನ್ಯ luaT_callTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *f, ಸ್ಥಿರಾಂಕ TValue *p1,
                  ಸ್ಥಿರಾಂಕ TValue *p2, ಸ್ಥಿರಾಂಕ TValue *p3) {
  StkId func = L->ತುದಿ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->ತುದಿ.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  ಆದರೆ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  ಇಲ್ಲದಿದ್ದರೆ
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *f, ಸ್ಥಿರಾಂಕ TValue *p1,
                        ಸ್ಥಿರಾಂಕ TValue *p2, StkId res) {
  ptrdiff_t ಫಲಿತಾಂಶ = savestack(L, res);
  StkId func = L->ತುದಿ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->ತುದಿ.p += 3;
  /* metamethod may yield only when called from Lua code */
  ಆದರೆ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  ಇಲ್ಲದಿದ್ದರೆ
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, ಫಲಿತಾಂಶ);
  setobjs2s(L, res, --L->ತುದಿ.p);  /* move result to its place */
  ಹಿಂತಿರುಗಿ ttypetag(s2v(res));  /* return tag of the result */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ callbinTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2,
                      StkId res, TMS event) {
  ಸ್ಥಿರಾಂಕ TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  ಆದರೆ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  ಆದರೆ (notm(tm))
    ಹಿಂತಿರುಗಿ -1;  /* tag method not found */
  ಇಲ್ಲದಿದ್ದರೆ  /* call tag method and return the tag of the result */
    ಹಿಂತಿರುಗಿ luaT_callTMres(L, tm, p1, p2, res);
}


ಶೂನ್ಯ luaT_trybinTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2,
                    StkId res, TMS event) {
  ಆದರೆ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    ಆಯ್ಕೆ (event) {
      ಸ್ಥಿತಿ TM_BAND: ಸ್ಥಿತಿ TM_BOR: ಸ್ಥಿತಿ TM_BXOR:
      ಸ್ಥಿತಿ TM_SHL: ಸ್ಥಿತಿ TM_SHR: ಸ್ಥಿತಿ TM_BNOT: {
        ಆದರೆ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        ಇಲ್ಲದಿದ್ದರೆ
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      ಪೂರ್ವನಿಯೋಜಿತ:
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
  ಆದರೆ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


ಶೂನ್ಯ luaT_trybinassocTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2,
                                       ಪೂರ್ಣಾಂಕ flip, StkId res, TMS event) {
  ಆದರೆ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  ಇಲ್ಲದಿದ್ದರೆ
    luaT_trybinTM(L, p1, p2, res, event);
}


ಶೂನ್ಯ luaT_trybiniTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, lua_Integer i2,
                                   ಪೂರ್ಣಾಂಕ flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
ಪೂರ್ಣಾಂಕ luaT_callorderTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2,
                      TMS event) {
  ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್ = callbinTM(L, p1, p2, L->ತುದಿ.p, event);  /* try original event */
  ಆದರೆ (ಟ್ಯಾಗ್ >= 0)  /* found tag method? */
    ಹಿಂತಿರುಗಿ !tagisfalse(ಟ್ಯಾಗ್);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  ಹಿಂತಿರುಗಿ 0;  /* to avoid warnings */
}


ಪೂರ್ಣಾಂಕ luaT_callorderiTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಪೂರ್ಣಾಂಕ v2,
                       ಪೂರ್ಣಾಂಕ flip, ಪೂರ್ಣಾಂಕ isfloat, TMS event) {
  TValue aux; ಸ್ಥಿರಾಂಕ TValue *p2;
  ಆದರೆ (isfloat) {
    setfltvalue(&aux, cast_ಅಂಕೆ(v2));
  }
  ಇಲ್ಲದಿದ್ದರೆ
    setivalue(&aux, v2);
  ಆದರೆ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    p2 = &aux;
  ಹಿಂತಿರುಗಿ luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ createvarargtab (lua_State *L, StkId f, ಪೂರ್ಣಾಂಕ n) {
  ಪೂರ್ಣಾಂಕ i;
  TValue ಕೀಲಿ, ಮೌಲ್ಯ;
  ಕೋಷ್ಟಕ *t = luaH_new(L);
  sethvalue(L, s2v(L->ತುದಿ.p), t);
  L->ತುದಿ.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &ಕೀಲಿ, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&ಮೌಲ್ಯ, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  lua_H_ಗಣ(L, t, &ಕೀಲಿ, &ಮೌಲ್ಯ);  /* t.n = n */
  ಗಾಗಿ (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
ಸ್ಥಿರ ಶೂನ್ಯ buildhiddenargs (lua_State *L, CallInfo *ci, ಸ್ಥಿರಾಂಕ Proto *p,
                             ಪೂರ್ಣಾಂಕ totalargs, ಪೂರ್ಣಾಂಕ nfixparams, ಪೂರ್ಣಾಂಕ nextra) {
  ಪೂರ್ಣಾಂಕ i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->ತುದಿ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  ಗಾಗಿ (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->ತುದಿ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->ತುದಿ.p += totalargs + 1;
}


ಶೂನ್ಯ luaT_adjustvarargs (lua_State *L, CallInfo *ci, ಸ್ಥಿರಾಂಕ Proto *p) {
  ಪೂರ್ಣಾಂಕ totalargs = cast_int(L->ತುದಿ.p - ci->func.p) - 1;
  ಪೂರ್ಣಾಂಕ nfixparams = p->numparams;
  ಪೂರ್ಣಾಂಕ nextra = totalargs - nfixparams;  /* number of extra arguments */
  ಆದರೆ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->ತುದಿ.p - 1);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->ತುದಿ.p <= ci->ತುದಿ.p && ci->ತುದಿ.p <= L->ರಾಶಿ_ಕೊನೆಯ.p);
  }
}


ಶೂನ್ಯ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  ಪೂರ್ಣಾಂಕ nextra = ci->u.l.nextraargs;
  lua_Integer n;
  ಆದರೆ (tointegerns(rc, &n)) {  /* integral value? */
    ಆದರೆ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId ಸ್ಥಾನ = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ಖಾಲಿ), ra, ಸ್ಥಾನ);
      ಹಿಂತಿರುಗಿ;
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisstring(rc)) {  /* string value? */
    size_t len;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getlstr(tsvalue(rc), len);
    ಆದರೆ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      ಹಿಂತಿರುಗಿ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getnumargs (lua_State *L, CallInfo *ci, ಕೋಷ್ಟಕ *h) {
  ಆದರೆ (h == ಖಾಲಿ)  /* no vararg table? */
    ಹಿಂತಿರುಗಿ ci->u.l.nextraargs;
  ಇಲ್ಲದಿದ್ದರೆ {
    TValue res;
    ಆದರೆ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    ಹಿಂತಿರುಗಿ cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
ಶೂನ್ಯ luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, ಪೂರ್ಣಾಂಕ wanted,
                                    ಪೂರ್ಣಾಂಕ vatab) {
  ಕೋಷ್ಟಕ *h = (vatab < 0) ? ಖಾಲಿ : hvalue(s2v(ci->func.p + vatab + 1));
  ಪೂರ್ಣಾಂಕ nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  ಪೂರ್ಣಾಂಕ i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  ಆದರೆ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->ತುದಿ.p = where + nargs;  /* next instruction will need top */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    touse = (nargs > wanted) ? wanted : nargs;
  ಆದರೆ (h == ಖಾಲಿ) {  /* no vararg table? */
    ಗಾಗಿ (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* get vararg values from vararg table */
    ಗಾಗಿ (i = 0; i < touse; i++) {
      lu_byte ಟ್ಯಾಗ್ = luaH_getint(h, i + 1, s2v(where + i));
      ಆದರೆ (tagisempty(ಟ್ಯಾಗ್))
       setnilvalue2s(where + i);
    }
  }
  ಗಾಗಿ (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

