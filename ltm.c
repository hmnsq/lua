/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "ltm.h"
#അന്തര്ഭാവ "lvm.h"


സ്ഥിതം സ്ഥിരം അക്ഷരം udatatypename[] = "userdata";

LUAI_DDEF സ്ഥിരം അക്ഷരം *സ്ഥിരം luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


ശൂന്യം luaT_init (lua_State *L) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
സ്ഥിരം TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  സ്ഥിരം TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  എങ്കിൽ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    തിരികെ ശൂന്യ;
  }
  അല്ലെങ്കിൽ തിരികെ tm;
}


സ്ഥിരം TValue *luaT_gettmbyobj (lua_State *L, സ്ഥിരം TValue *o, TMS event) {
  Table *mt;
  തിരഞ്ഞെടുപ്പ് (ttype(o)) {
    അവസ്ഥ LUA_TTABLE:
      mt = hvalue(o)->metatable;
      നിർത്തുക;
    അവസ്ഥ LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      mt = G(L)->mt[ttype(o)];
  }
  തിരികെ (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
സ്ഥിരം അക്ഷരം *luaT_objtypename (lua_State *L, സ്ഥിരം TValue *o) {
  Table *mt;
  എങ്കിൽ ((ttistable(o) && (mt = hvalue(o)->metatable) != ശൂന്യ) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ശൂന്യ)) {
    സ്ഥിരം TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    എങ്കിൽ (ttisstring(name))  /* is '__name' a string? */
      തിരികെ getstr(tsvalue(name));  /* use it as type name */
  }
  തിരികെ ttypename(ttype(o));  /* else use standard type name */
}


ശൂന്യം luaT_callTM (lua_State *L, സ്ഥിരം TValue *f, സ്ഥിരം TValue *p1,
                  സ്ഥിരം TValue *p2, സ്ഥിരം TValue *p3) {
  StkId func = L->മുകൾ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->മുകൾ.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  എങ്കിൽ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  അല്ലെങ്കിൽ
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, സ്ഥിരം TValue *f, സ്ഥിരം TValue *p1,
                        സ്ഥിരം TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->മുകൾ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->മുകൾ.p += 3;
  /* metamethod may yield only when called from Lua code */
  എങ്കിൽ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  അല്ലെങ്കിൽ
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->മുകൾ.p);  /* move result to its place */
  തിരികെ ttypetag(s2v(res));  /* return tag of the result */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ callbinTM (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2,
                      StkId res, TMS event) {
  സ്ഥിരം TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  എങ്കിൽ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  എങ്കിൽ (notm(tm))
    തിരികെ -1;  /* tag method not found */
  അല്ലെങ്കിൽ  /* call tag method and return the tag of the result */
    തിരികെ luaT_callTMres(L, tm, p1, p2, res);
}


ശൂന്യം luaT_trybinTM (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2,
                    StkId res, TMS event) {
  എങ്കിൽ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    തിരഞ്ഞെടുപ്പ് (event) {
      അവസ്ഥ TM_BAND: അവസ്ഥ TM_BOR: അവസ്ഥ TM_BXOR:
      അവസ്ഥ TM_SHL: അവസ്ഥ TM_SHR: അവസ്ഥ TM_BNOT: {
        എങ്കിൽ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        അല്ലെങ്കിൽ
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      സ്ഥിരസ്ഥിതി:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
ശൂന്യം luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->മുകൾ.p - 2;  /* first argument */
  എങ്കിൽ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


ശൂന്യം luaT_trybinassocTM (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2,
                                       പൂർണ്ണസംഖ്യ flip, StkId res, TMS event) {
  എങ്കിൽ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  അല്ലെങ്കിൽ
    luaT_trybinTM(L, p1, p2, res, event);
}


ശൂന്യം luaT_trybiniTM (lua_State *L, സ്ഥിരം TValue *p1, lua_Integer i2,
                                   പൂർണ്ണസംഖ്യ flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
പൂർണ്ണസംഖ്യ luaT_callorderTM (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2,
                      TMS event) {
  പൂർണ്ണസംഖ്യ tag = callbinTM(L, p1, p2, L->മുകൾ.p, event);  /* try original event */
  എങ്കിൽ (tag >= 0)  /* found tag method? */
    തിരികെ !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  തിരികെ 0;  /* to avoid warnings */
}


പൂർണ്ണസംഖ്യ luaT_callorderiTM (lua_State *L, സ്ഥിരം TValue *p1, പൂർണ്ണസംഖ്യ v2,
                       പൂർണ്ണസംഖ്യ flip, പൂർണ്ണസംഖ്യ isfloat, TMS event) {
  TValue aux; സ്ഥിരം TValue *p2;
  എങ്കിൽ (isfloat) {
    setfltvalue(&aux, cast_അക്കം(v2));
  }
  അല്ലെങ്കിൽ
    setivalue(&aux, v2);
  എങ്കിൽ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  അല്ലെങ്കിൽ
    p2 = &aux;
  തിരികെ luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
സ്ഥിതം ശൂന്യം createvarargtab (lua_State *L, StkId f, പൂർണ്ണസംഖ്യ n) {
  പൂർണ്ണസംഖ്യ i;
  TValue താക്കോൽ, മൂല്യം;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->മുകൾ.p), t);
  L->മുകൾ.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &താക്കോൽ, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&മൂല്യം, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &താക്കോൽ, &മൂല്യം);  /* t.n = n */
  വേണ്ടി (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
സ്ഥിതം ശൂന്യം buildhiddenargs (lua_State *L, CallInfo *ci, സ്ഥിരം Proto *p,
                             പൂർണ്ണസംഖ്യ totalargs, പൂർണ്ണസംഖ്യ nfixparams, പൂർണ്ണസംഖ്യ nextra) {
  പൂർണ്ണസംഖ്യ i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->മുകൾ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  വേണ്ടി (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->മുകൾ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->മുകൾ.p += totalargs + 1;
}


ശൂന്യം luaT_adjustvarargs (lua_State *L, CallInfo *ci, സ്ഥിരം Proto *p) {
  പൂർണ്ണസംഖ്യ totalargs = cast_int(L->മുകൾ.p - ci->func.p) - 1;
  പൂർണ്ണസംഖ്യ nfixparams = p->numparams;
  പൂർണ്ണസംഖ്യ nextra = totalargs - nfixparams;  /* number of extra arguments */
  എങ്കിൽ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->മുകൾ.p - 1);
  }
  അല്ലെങ്കിൽ {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->മുകൾ.p <= ci->മുകൾ.p && ci->മുകൾ.p <= L->അട്ടി_last.p);
  }
}


ശൂന്യം luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  പൂർണ്ണസംഖ്യ nextra = ci->u.l.nextraargs;
  lua_Integer n;
  എങ്കിൽ (tointegerns(rc, &n)) {  /* integral value? */
    എങ്കിൽ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ശൂന്യ), ra, slot);
      തിരികെ;
    }
  }
  അല്ലെങ്കിൽ എങ്കിൽ (ttisstring(rc)) {  /* string value? */
    size_t len;
    സ്ഥിരം അക്ഷരം *s = getlstr(tsvalue(rc), len);
    എങ്കിൽ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      തിരികെ;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  എങ്കിൽ (h == ശൂന്യ)  /* no vararg table? */
    തിരികെ ci->u.l.nextraargs;
  അല്ലെങ്കിൽ {
    TValue res;
    എങ്കിൽ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    തിരികെ cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
ശൂന്യം luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, പൂർണ്ണസംഖ്യ wanted,
                                    പൂർണ്ണസംഖ്യ vatab) {
  Table *h = (vatab < 0) ? ശൂന്യ : hvalue(s2v(ci->func.p + vatab + 1));
  പൂർണ്ണസംഖ്യ nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  പൂർണ്ണസംഖ്യ i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  എങ്കിൽ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->മുകൾ.p = where + nargs;  /* next instruction will need top */
  }
  അല്ലെങ്കിൽ
    touse = (nargs > wanted) ? wanted : nargs;
  എങ്കിൽ (h == ശൂന്യ) {  /* no vararg table? */
    വേണ്ടി (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  അല്ലെങ്കിൽ {  /* get vararg values from vararg table */
    വേണ്ടി (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      എങ്കിൽ (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  വേണ്ടി (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

