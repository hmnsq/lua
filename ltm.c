/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "ltm.h"
#ଅନ୍ତର୍ଭାଵ "lvm.h"


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର udatatypename[] = "userdata";

LUAI_DDEF ଅଚଳ ଅକ୍ଷର *ଅଚଳ luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


ଶୂନ୍ୟ luaT_init (lua_State *L) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ଅଚଳ TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  ଅଚଳ TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  ଯଦି (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    ଫେରନ୍ତୁ ଖାଲି;
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ tm;
}


ଅଚଳ TValue *luaT_gettmbyobj (lua_State *L, ଅଚଳ TValue *o, TMS event) {
  Table *mt;
  ବାଛ (ttype(o)) {
    ସ୍ଥିତି LUA_TTABLE:
      mt = hvalue(o)->metatable;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      mt = G(L)->mt[ttype(o)];
  }
  ଫେରନ୍ତୁ (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ଅଚଳ ଅକ୍ଷର *luaT_objtypename (lua_State *L, ଅଚଳ TValue *o) {
  Table *mt;
  ଯଦି ((ttistable(o) && (mt = hvalue(o)->metatable) != ଖାଲି) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != ଖାଲି)) {
    ଅଚଳ TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    ଯଦି (ttisstring(name))  /* is '__name' a string? */
      ଫେରନ୍ତୁ getstr(tsvalue(name));  /* use it as type name */
  }
  ଫେରନ୍ତୁ ttypename(ttype(o));  /* else use standard type name */
}


ଶୂନ୍ୟ luaT_callTM (lua_State *L, ଅଚଳ TValue *f, ଅଚଳ TValue *p1,
                  ଅଚଳ TValue *p2, ଅଚଳ TValue *p3) {
  StkId func = L->ଶୀର୍ଷ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->ଶୀର୍ଷ.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  ଯଦି (isLuacode(L->ci))
    luaD_call(L, func, 0);
  ଅନ୍ୟଥା
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ଅଚଳ TValue *f, ଅଚଳ TValue *p1,
                        ଅଚଳ TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->ଶୀର୍ଷ.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->ଶୀର୍ଷ.p += 3;
  /* metamethod may yield only when called from Lua code */
  ଯଦି (isLuacode(L->ci))
    luaD_call(L, func, 1);
  ଅନ୍ୟଥା
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->ଶୀର୍ଷ.p);  /* move result to its place */
  ଫେରନ୍ତୁ ttypetag(s2v(res));  /* return tag of the result */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା callbinTM (lua_State *L, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2,
                      StkId res, TMS event) {
  ଅଚଳ TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  ଯଦି (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  ଯଦି (notm(tm))
    ଫେରନ୍ତୁ -1;  /* tag method not found */
  ଅନ୍ୟଥା  /* call tag method and return the tag of the result */
    ଫେରନ୍ତୁ luaT_callTMres(L, tm, p1, p2, res);
}


ଶୂନ୍ୟ luaT_trybinTM (lua_State *L, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2,
                    StkId res, TMS event) {
  ଯଦି (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    ବାଛ (event) {
      ସ୍ଥିତି TM_BAND: ସ୍ଥିତି TM_BOR: ସ୍ଥିତି TM_BXOR:
      ସ୍ଥିତି TM_SHL: ସ୍ଥିତି TM_SHR: ସ୍ଥିତି TM_BNOT: {
        ଯଦି (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        ଅନ୍ୟଥା
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
ଶୂନ୍ୟ luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->ଶୀର୍ଷ.p - 2;  /* first argument */
  ଯଦି (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


ଶୂନ୍ୟ luaT_trybinassocTM (lua_State *L, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2,
                                       ପୂର୍ଣ୍ଣସଂଖ୍ୟା flip, StkId res, TMS event) {
  ଯଦି (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  ଅନ୍ୟଥା
    luaT_trybinTM(L, p1, p2, res, event);
}


ଶୂନ୍ୟ luaT_trybiniTM (lua_State *L, ଅଚଳ TValue *p1, lua_Integer i2,
                                   ପୂର୍ଣ୍ଣସଂଖ୍ୟା flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaT_callorderTM (lua_State *L, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2,
                      TMS event) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag = callbinTM(L, p1, p2, L->ଶୀର୍ଷ.p, event);  /* try original event */
  ଯଦି (tag >= 0)  /* found tag method? */
    ଫେରନ୍ତୁ !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  ଫେରନ୍ତୁ 0;  /* to avoid warnings */
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaT_callorderiTM (lua_State *L, ଅଚଳ TValue *p1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା v2,
                       ପୂର୍ଣ୍ଣସଂଖ୍ୟା flip, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isfloat, TMS event) {
  TValue aux; ଅଚଳ TValue *p2;
  ଯଦି (isfloat) {
    setfltvalue(&aux, cast_ଅଙ୍କ(v2));
  }
  ଅନ୍ୟଥା
    setivalue(&aux, v2);
  ଯଦି (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  ଅନ୍ୟଥା
    p2 = &aux;
  ଫେରନ୍ତୁ luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
ସ୍ଥିର ଶୂନ୍ୟ createvarargtab (lua_State *L, StkId f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  TValue ଚାବି, ମୂଲ୍ୟ;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->ଶୀର୍ଷ.p), t);
  L->ଶୀର୍ଷ.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &ଚାବି, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&ମୂଲ୍ୟ, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &ଚାବି, &ମୂଲ୍ୟ);  /* t.n = n */
  ପାଇଁ (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
ସ୍ଥିର ଶୂନ୍ୟ buildhiddenargs (lua_State *L, CallInfo *ci, ଅଚଳ Proto *p,
                             ପୂର୍ଣ୍ଣସଂଖ୍ୟା totalargs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixparams, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextra) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->ଶୀର୍ଷ.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  ପାଇଁ (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->ଶୀର୍ଷ.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->ଶୀର୍ଷ.p += totalargs + 1;
}


ଶୂନ୍ୟ luaT_adjustvarargs (lua_State *L, CallInfo *ci, ଅଚଳ Proto *p) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା totalargs = cast_int(L->ଶୀର୍ଷ.p - ci->func.p) - 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixparams = p->numparams;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextra = totalargs - nfixparams;  /* number of extra arguments */
  ଯଦି (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->ଶୀର୍ଷ.p - 1);
  }
  ଅନ୍ୟଥା {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->ଶୀର୍ଷ.p <= ci->ଶୀର୍ଷ.p && ci->ଶୀର୍ଷ.p <= L->ଗଦା_last.p);
  }
}


ଶୂନ୍ୟ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextra = ci->u.l.nextraargs;
  lua_Integer n;
  ଯଦି (tointegerns(rc, &n)) {  /* integral value? */
    ଯଦି (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)ଖାଲି), ra, slot);
      ଫେରନ୍ତୁ;
    }
  }
  ଅନ୍ୟଥା ଯଦି (ttisstring(rc)) {  /* string value? */
    size_t len;
    ଅଚଳ ଅକ୍ଷର *s = getlstr(tsvalue(rc), len);
    ଯଦି (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      ଫେରନ୍ତୁ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  ଯଦି (h == ଖାଲି)  /* no vararg table? */
    ଫେରନ୍ତୁ ci->u.l.nextraargs;
  ଅନ୍ୟଥା {
    TValue res;
    ଯଦି (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    ଫେରନ୍ତୁ cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
ଶୂନ୍ୟ luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, ପୂର୍ଣ୍ଣସଂଖ୍ୟା wanted,
                                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା vatab) {
  Table *h = (vatab < 0) ? ଖାଲି : hvalue(s2v(ci->func.p + vatab + 1));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  ଯଦି (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->ଶୀର୍ଷ.p = where + nargs;  /* next instruction will need top */
  }
  ଅନ୍ୟଥା
    touse = (nargs > wanted) ? wanted : nargs;
  ଯଦି (h == ଖାଲି) {  /* no vararg table? */
    ପାଇଁ (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  ଅନ୍ୟଥା {  /* get vararg values from vararg table */
    ପାଇଁ (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      ଯଦି (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  ପାଇଁ (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

