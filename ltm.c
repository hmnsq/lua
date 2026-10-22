/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#घोषणा ltm_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <string.h>

#समावेश "lua.h"

#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lgc.h"
#समावेश "lobject.h"
#समावेश "lstate.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "ltm.h"
#समावेश "lvm.h"


अटल स्थिर अक्षर udatatypename[] = "userdata";

LUAI_DDEF स्थिर अक्षर *स्थिर luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


व्योम luaT_init (lua_State *L) {
  अटल स्थिर अक्षर *स्थिर luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  पूर्णांक i;
  क्रम (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
स्थिर TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  स्थिर TValue *पंचांग = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  अगर (notm(पंचांग)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    वापस शून्य;
  }
  अन्यथा वापस पंचांग;
}


स्थिर TValue *luaT_gettmbyobj (lua_State *L, स्थिर TValue *o, TMS event) {
  Table *mt;
  चयन (ttype(o)) {
    हाल LUA_TTABLE:
      mt = hvalue(o)->metatable;
      अवरोध;
    हाल LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      अवरोध;
    शेष:
      mt = G(L)->mt[ttype(o)];
  }
  वापस (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
स्थिर अक्षर *luaT_objtypename (lua_State *L, स्थिर TValue *o) {
  Table *mt;
  अगर ((ttistable(o) && (mt = hvalue(o)->metatable) != शून्य) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != शून्य)) {
    स्थिर TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    अगर (ttisstring(name))  /* is '__name' a string? */
      वापस getstr(tsvalue(name));  /* use it as type name */
  }
  वापस ttypename(ttype(o));  /* else use standard type name */
}


व्योम luaT_callTM (lua_State *L, स्थिर TValue *f, स्थिर TValue *p1,
                  स्थिर TValue *p2, स्थिर TValue *p3) {
  StkId func = L->शीर्ष.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->शीर्ष.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  अगर (isLuacode(L->ci))
    luaD_call(L, func, 0);
  अन्यथा
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, स्थिर TValue *f, स्थिर TValue *p1,
                        स्थिर TValue *p2, StkId res) {
  सूचक_भेद_प्रकार परिणाम = savestack(L, res);
  StkId func = L->शीर्ष.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->शीर्ष.p += 3;
  /* metamethod may yield only when called from Lua code */
  अगर (isLuacode(L->ci))
    luaD_call(L, func, 1);
  अन्यथा
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, परिणाम);
  setobjs2s(L, res, --L->शीर्ष.p);  /* move result to its place */
  वापस ttypetag(s2v(res));  /* return tag of the result */
}


अटल पूर्णांक callbinTM (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2,
                      StkId res, TMS event) {
  स्थिर TValue *पंचांग = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  अगर (notm(पंचांग))
    पंचांग = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  अगर (notm(पंचांग))
    वापस -1;  /* tag method not found */
  अन्यथा  /* call tag method and return the tag of the result */
    वापस luaT_callTMres(L, पंचांग, p1, p2, res);
}


व्योम luaT_trybinTM (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2,
                    StkId res, TMS event) {
  अगर (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    चयन (event) {
      हाल TM_BAND: हाल TM_BOR: हाल TM_BXOR:
      हाल TM_SHL: हाल TM_SHR: हाल TM_BNOT: {
        अगर (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        अन्यथा
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      शेष:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
व्योम luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->शीर्ष.p - 2;  /* first argument */
  अगर (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


व्योम luaT_trybinassocTM (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2,
                                       पूर्णांक flip, StkId res, TMS event) {
  अगर (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  अन्यथा
    luaT_trybinTM(L, p1, p2, res, event);
}


व्योम luaT_trybiniTM (lua_State *L, स्थिर TValue *p1, lua_Integer i2,
                                   पूर्णांक flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
पूर्णांक luaT_callorderTM (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2,
                      TMS event) {
  पूर्णांक tag = callbinTM(L, p1, p2, L->शीर्ष.p, event);  /* try original event */
  अगर (tag >= 0)  /* found tag method? */
    वापस !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  वापस 0;  /* to avoid warnings */
}


पूर्णांक luaT_callorderiTM (lua_State *L, स्थिर TValue *p1, पूर्णांक v2,
                       पूर्णांक flip, पूर्णांक isfloat, TMS event) {
  TValue aux; स्थिर TValue *p2;
  अगर (isfloat) {
    setfltvalue(&aux, cast_अंक(v2));
  }
  अन्यथा
    setivalue(&aux, v2);
  अगर (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  अन्यथा
    p2 = &aux;
  वापस luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
अटल व्योम createvarargtab (lua_State *L, StkId f, पूर्णांक n) {
  पूर्णांक i;
  TValue कुंजी, मूल्य;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->शीर्ष.p), t);
  L->शीर्ष.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &कुंजी, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&मूल्य, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &कुंजी, &मूल्य);  /* t.n = n */
  क्रम (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
अटल व्योम buildhiddenargs (lua_State *L, CallInfo *ci, स्थिर Proto *p,
                             पूर्णांक totalargs, पूर्णांक nfixparams, पूर्णांक nextra) {
  पूर्णांक i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->शीर्ष.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  क्रम (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->शीर्ष.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->शीर्ष.p += totalargs + 1;
}


व्योम luaT_adjustvarargs (lua_State *L, CallInfo *ci, स्थिर Proto *p) {
  पूर्णांक totalargs = cast_int(L->शीर्ष.p - ci->func.p) - 1;
  पूर्णांक nfixparams = p->numparams;
  पूर्णांक nextra = totalargs - nfixparams;  /* number of extra arguments */
  अगर (p->flag & PF_VATAB) {  /* does it need a vararg table? */
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
    lua_assert(L->शीर्ष.p <= ci->शीर्ष.p && ci->शीर्ष.p <= L->ढेर_last.p);
  }
}


व्योम luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  पूर्णांक nextra = ci->u.l.nextraargs;
  lua_Integer n;
  अगर (tointegerns(rc, &n)) {  /* integral value? */
    अगर (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)शून्य), ra, slot);
      वापस;
    }
  }
  अन्यथा अगर (ttisstring(rc)) {  /* string value? */
    माप_प्रकार len;
    स्थिर अक्षर *s = getlstr(tsvalue(rc), len);
    अगर (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      वापस;
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
अटल पूर्णांक getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  अगर (h == शून्य)  /* no vararg table? */
    वापस ci->u.l.nextraargs;
  अन्यथा {
    TValue res;
    अगर (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(पूर्णांक_उच्च/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    वापस cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
व्योम luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, पूर्णांक wanted,
                                    पूर्णांक vatab) {
  Table *h = (vatab < 0) ? शून्य : hvalue(s2v(ci->func.p + vatab + 1));
  पूर्णांक nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  पूर्णांक i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  अगर (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->शीर्ष.p = where + nargs;  /* next instruction will need top */
  }
  अन्यथा
    touse = (nargs > wanted) ? wanted : nargs;
  अगर (h == शून्य) {  /* no vararg table? */
    क्रम (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  अन्यथा {  /* get vararg values from vararg table */
    क्रम (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      अगर (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  क्रम (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

