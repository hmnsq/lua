/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "ltm.h"
#अन्तर्भाव "lvm.h"


स्थिर स्थिरांक अक्षर udatatypename[] = "userdata";

LUAI_DDEF स्थिरांक अक्षर *स्थिरांक luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


शून्य luaT_init (lua_State *L) {
  स्थिर स्थिरांक अक्षर *स्थिरांक luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  पूर्णांक i;
  साठी (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
स्थिरांक TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  स्थिरांक TValue *पंचांग = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  जर (notm(पंचांग)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    परत रिक्त;
  }
  अन्यथा परत पंचांग;
}


स्थिरांक TValue *luaT_gettmbyobj (lua_State *L, स्थिरांक TValue *o, TMS event) {
  Table *mt;
  निवड (ttype(o)) {
    स्थिती LUA_TTABLE:
      mt = hvalue(o)->metatable;
      थांबा;
    स्थिती LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      थांबा;
    पूर्वनिर्धारित:
      mt = G(L)->mt[ttype(o)];
  }
  परत (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
स्थिरांक अक्षर *luaT_objtypename (lua_State *L, स्थिरांक TValue *o) {
  Table *mt;
  जर ((ttistable(o) && (mt = hvalue(o)->metatable) != रिक्त) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != रिक्त)) {
    स्थिरांक TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    जर (ttisstring(name))  /* is '__name' a string? */
      परत getstr(tsvalue(name));  /* use it as type name */
  }
  परत ttypename(ttype(o));  /* else use standard type name */
}


शून्य luaT_callTM (lua_State *L, स्थिरांक TValue *f, स्थिरांक TValue *p1,
                  स्थिरांक TValue *p2, स्थिरांक TValue *p3) {
  StkId func = L->शीर्ष.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->शीर्ष.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  जर (isLuacode(L->ci))
    luaD_call(L, func, 0);
  अन्यथा
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, स्थिरांक TValue *f, स्थिरांक TValue *p1,
                        स्थिरांक TValue *p2, StkId res) {
  सूचक_भेद_प्रकार result = savestack(L, res);
  StkId func = L->शीर्ष.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->शीर्ष.p += 3;
  /* metamethod may yield only when called from Lua code */
  जर (isLuacode(L->ci))
    luaD_call(L, func, 1);
  अन्यथा
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->शीर्ष.p);  /* move result to its place */
  परत ttypetag(s2v(res));  /* return tag of the result */
}


स्थिर पूर्णांक callbinTM (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2,
                      StkId res, TMS event) {
  स्थिरांक TValue *पंचांग = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  जर (notm(पंचांग))
    पंचांग = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  जर (notm(पंचांग))
    परत -1;  /* tag method not found */
  अन्यथा  /* call tag method and return the tag of the result */
    परत luaT_callTMres(L, पंचांग, p1, p2, res);
}


शून्य luaT_trybinTM (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2,
                    StkId res, TMS event) {
  जर (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    निवड (event) {
      स्थिती TM_BAND: स्थिती TM_BOR: स्थिती TM_BXOR:
      स्थिती TM_SHL: स्थिती TM_SHR: स्थिती TM_BNOT: {
        जर (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        अन्यथा
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      पूर्वनिर्धारित:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
शून्य luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->शीर्ष.p - 2;  /* first argument */
  जर (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


शून्य luaT_trybinassocTM (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2,
                                       पूर्णांक flip, StkId res, TMS event) {
  जर (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  अन्यथा
    luaT_trybinTM(L, p1, p2, res, event);
}


शून्य luaT_trybiniTM (lua_State *L, स्थिरांक TValue *p1, lua_Integer i2,
                                   पूर्णांक flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
पूर्णांक luaT_callorderTM (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2,
                      TMS event) {
  पूर्णांक tag = callbinTM(L, p1, p2, L->शीर्ष.p, event);  /* try original event */
  जर (tag >= 0)  /* found tag method? */
    परत !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  परत 0;  /* to avoid warnings */
}


पूर्णांक luaT_callorderiTM (lua_State *L, स्थिरांक TValue *p1, पूर्णांक v2,
                       पूर्णांक flip, पूर्णांक isfloat, TMS event) {
  TValue aux; स्थिरांक TValue *p2;
  जर (isfloat) {
    setfltvalue(&aux, cast_अंक(v2));
  }
  अन्यथा
    setivalue(&aux, v2);
  जर (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  अन्यथा
    p2 = &aux;
  परत luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
स्थिर शून्य createvarargtab (lua_State *L, StkId f, पूर्णांक n) {
  पूर्णांक i;
  TValue किल्ली, मूल्य;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->शीर्ष.p), t);
  L->शीर्ष.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &किल्ली, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&मूल्य, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &किल्ली, &मूल्य);  /* t.n = n */
  साठी (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
स्थिर शून्य buildhiddenargs (lua_State *L, CallInfo *ci, स्थिरांक Proto *p,
                             पूर्णांक totalargs, पूर्णांक nfixparams, पूर्णांक nextra) {
  पूर्णांक i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->शीर्ष.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  साठी (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->शीर्ष.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->शीर्ष.p += totalargs + 1;
}


शून्य luaT_adjustvarargs (lua_State *L, CallInfo *ci, स्थिरांक Proto *p) {
  पूर्णांक totalargs = cast_int(L->शीर्ष.p - ci->func.p) - 1;
  पूर्णांक nfixparams = p->numparams;
  पूर्णांक nextra = totalargs - nfixparams;  /* number of extra arguments */
  जर (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->शीर्ष.p - 1);
  }
  अन्यथा {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->शीर्ष.p <= ci->शीर्ष.p && ci->शीर्ष.p <= L->चळत_last.p);
  }
}


शून्य luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  पूर्णांक nextra = ci->u.l.nextraargs;
  lua_Integer n;
  जर (tointegerns(rc, &n)) {  /* integral value? */
    जर (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)रिक्त), ra, slot);
      परत;
    }
  }
  अन्यथा जर (ttisstring(rc)) {  /* string value? */
    माप_प्रकार len;
    स्थिरांक अक्षर *s = getlstr(tsvalue(rc), len);
    जर (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      परत;
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
स्थिर पूर्णांक getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  जर (h == रिक्त)  /* no vararg table? */
    परत ci->u.l.nextraargs;
  अन्यथा {
    TValue res;
    जर (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(पूर्णांक_उच्च/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    परत cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
शून्य luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, पूर्णांक wanted,
                                    पूर्णांक vatab) {
  Table *h = (vatab < 0) ? रिक्त : hvalue(s2v(ci->func.p + vatab + 1));
  पूर्णांक nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  पूर्णांक i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  जर (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->शीर्ष.p = where + nargs;  /* next instruction will need top */
  }
  अन्यथा
    touse = (nargs > wanted) ? wanted : nargs;
  जर (h == रिक्त) {  /* no vararg table? */
    साठी (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  अन्यथा {  /* get vararg values from vararg table */
    साठी (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      जर (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  साठी (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

