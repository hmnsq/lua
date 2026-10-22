/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "ltm.h"
#અન્તર્ભાવ "lvm.h"


સ્થિર અચલ અક્ષર udatatypename[] = "userdata";

LUAI_DDEF અચલ અક્ષર *અચલ luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


શૂન્ય luaT_init (lua_State *L) {
  સ્થિર અચલ અક્ષર *અચલ luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  પૂર્ણાંક i;
  માટે (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
અચલ TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  અચલ TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  જો (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    પાછા ખાલી;
  }
  નહીંતર પાછા tm;
}


અચલ TValue *luaT_gettmbyobj (lua_State *L, અચલ TValue *o, TMS event) {
  Table *mt;
  પસંદગી (ttype(o)) {
    સ્થિતિ LUA_TTABLE:
      mt = hvalue(o)->metatable;
      તોડો;
    સ્થિતિ LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      તોડો;
    મૂળભૂત:
      mt = G(L)->mt[ttype(o)];
  }
  પાછા (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
અચલ અક્ષર *luaT_objtypename (lua_State *L, અચલ TValue *o) {
  Table *mt;
  જો ((ttistable(o) && (mt = hvalue(o)->metatable) != ખાલી) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ખાલી)) {
    અચલ TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    જો (ttisstring(name))  /* is '__name' a string? */
      પાછા getstr(tsvalue(name));  /* use it as type name */
  }
  પાછા ttypename(ttype(o));  /* else use standard type name */
}


શૂન્ય luaT_callTM (lua_State *L, અચલ TValue *f, અચલ TValue *p1,
                  અચલ TValue *p2, અચલ TValue *p3) {
  StkId func = L->ટોચ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->ટોચ.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  જો (isLuacode(L->ci))
    luaD_call(L, func, 0);
  નહીંતર
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, અચલ TValue *f, અચલ TValue *p1,
                        અચલ TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->ટોચ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->ટોચ.p += 3;
  /* metamethod may yield only when called from Lua code */
  જો (isLuacode(L->ci))
    luaD_call(L, func, 1);
  નહીંતર
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->ટોચ.p);  /* move result to its place */
  પાછા ttypetag(s2v(res));  /* return tag of the result */
}


સ્થિર પૂર્ણાંક callbinTM (lua_State *L, અચલ TValue *p1, અચલ TValue *p2,
                      StkId res, TMS event) {
  અચલ TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  જો (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  જો (notm(tm))
    પાછા -1;  /* tag method not found */
  નહીંતર  /* call tag method and return the tag of the result */
    પાછા luaT_callTMres(L, tm, p1, p2, res);
}


શૂન્ય luaT_trybinTM (lua_State *L, અચલ TValue *p1, અચલ TValue *p2,
                    StkId res, TMS event) {
  જો (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    પસંદગી (event) {
      સ્થિતિ TM_BAND: સ્થિતિ TM_BOR: સ્થિતિ TM_BXOR:
      સ્થિતિ TM_SHL: સ્થિતિ TM_SHR: સ્થિતિ TM_BNOT: {
        જો (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        નહીંતર
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      મૂળભૂત:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
શૂન્ય luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->ટોચ.p - 2;  /* first argument */
  જો (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


શૂન્ય luaT_trybinassocTM (lua_State *L, અચલ TValue *p1, અચલ TValue *p2,
                                       પૂર્ણાંક flip, StkId res, TMS event) {
  જો (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  નહીંતર
    luaT_trybinTM(L, p1, p2, res, event);
}


શૂન્ય luaT_trybiniTM (lua_State *L, અચલ TValue *p1, lua_Integer i2,
                                   પૂર્ણાંક flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
પૂર્ણાંક luaT_callorderTM (lua_State *L, અચલ TValue *p1, અચલ TValue *p2,
                      TMS event) {
  પૂર્ણાંક tag = callbinTM(L, p1, p2, L->ટોચ.p, event);  /* try original event */
  જો (tag >= 0)  /* found tag method? */
    પાછા !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  પાછા 0;  /* to avoid warnings */
}


પૂર્ણાંક luaT_callorderiTM (lua_State *L, અચલ TValue *p1, પૂર્ણાંક v2,
                       પૂર્ણાંક flip, પૂર્ણાંક isfloat, TMS event) {
  TValue aux; અચલ TValue *p2;
  જો (isfloat) {
    setfltvalue(&aux, cast_અંક(v2));
  }
  નહીંતર
    setivalue(&aux, v2);
  જો (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  નહીંતર
    p2 = &aux;
  પાછા luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
સ્થિર શૂન્ય createvarargtab (lua_State *L, StkId f, પૂર્ણાંક n) {
  પૂર્ણાંક i;
  TValue ચાવી, કિંમત;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->ટોચ.p), t);
  L->ટોચ.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &ચાવી, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&કિંમત, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &ચાવી, &કિંમત);  /* t.n = n */
  માટે (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
સ્થિર શૂન્ય buildhiddenargs (lua_State *L, CallInfo *ci, અચલ Proto *p,
                             પૂર્ણાંક totalargs, પૂર્ણાંક nfixparams, પૂર્ણાંક nextra) {
  પૂર્ણાંક i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->ટોચ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  માટે (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->ટોચ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->ટોચ.p += totalargs + 1;
}


શૂન્ય luaT_adjustvarargs (lua_State *L, CallInfo *ci, અચલ Proto *p) {
  પૂર્ણાંક totalargs = cast_int(L->ટોચ.p - ci->func.p) - 1;
  પૂર્ણાંક nfixparams = p->numparams;
  પૂર્ણાંક nextra = totalargs - nfixparams;  /* number of extra arguments */
  જો (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->ટોચ.p - 1);
  }
  નહીંતર {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->ટોચ.p <= ci->ટોચ.p && ci->ટોચ.p <= L->ઢગલો_last.p);
  }
}


શૂન્ય luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  પૂર્ણાંક nextra = ci->u.l.nextraargs;
  lua_Integer n;
  જો (tointegerns(rc, &n)) {  /* integral value? */
    જો (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ખાલી), ra, slot);
      પાછા;
    }
  }
  નહીંતર જો (ttisstring(rc)) {  /* string value? */
    size_t len;
    અચલ અક્ષર *s = getlstr(tsvalue(rc), len);
    જો (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      પાછા;
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
સ્થિર પૂર્ણાંક getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  જો (h == ખાલી)  /* no vararg table? */
    પાછા ci->u.l.nextraargs;
  નહીંતર {
    TValue res;
    જો (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    પાછા cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
શૂન્ય luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, પૂર્ણાંક wanted,
                                    પૂર્ણાંક vatab) {
  Table *h = (vatab < 0) ? ખાલી : hvalue(s2v(ci->func.p + vatab + 1));
  પૂર્ણાંક nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  પૂર્ણાંક i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  જો (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->ટોચ.p = where + nargs;  /* next instruction will need top */
  }
  નહીંતર
    touse = (nargs > wanted) ? wanted : nargs;
  જો (h == ખાલી) {  /* no vararg table? */
    માટે (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  નહીંતર {  /* get vararg values from vararg table */
    માટે (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      જો (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  માટે (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

