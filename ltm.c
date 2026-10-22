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


स्थिर ध्रुव वर्ण udatatypename[] = "userdata";

LUAI_DDEF ध्रुव वर्ण *ध्रुव luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


शून्य luaT_init (lua_State *L) {
  स्थिर ध्रुव वर्ण *ध्रुव luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  पूर्णाङ्क i;
  प्रति (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
ध्रुव TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  ध्रुव TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  जइ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    प्रत्यागम रिक्त;
  }
  अण्णहा प्रत्यागम tm;
}


ध्रुव TValue *luaT_gettmbyobj (lua_State *L, ध्रुव TValue *o, TMS event) {
  Table *mt;
  विकल्प (ttype(o)) {
    पक्ष LUA_TTABLE:
      mt = hvalue(o)->metatable;
      भङ्ग;
    पक्ष LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      भङ्ग;
    स्वभाव:
      mt = G(L)->mt[ttype(o)];
  }
  प्रत्यागम (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
ध्रुव वर्ण *luaT_objtypename (lua_State *L, ध्रुव TValue *o) {
  Table *mt;
  जइ ((ttistable(o) && (mt = hvalue(o)->metatable) != रिक्त) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != रिक्त)) {
    ध्रुव TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    जइ (ttisstring(name))  /* is '__name' a string? */
      प्रत्यागम getstr(tsvalue(name));  /* use it as type name */
  }
  प्रत्यागम ttypename(ttype(o));  /* else use standard type name */
}


शून्य luaT_callTM (lua_State *L, ध्रुव TValue *f, ध्रुव TValue *p1,
                  ध्रुव TValue *p2, ध्रुव TValue *p3) {
  StkId func = L->शिखरम्.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->शिखरम्.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  जइ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  अण्णहा
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, ध्रुव TValue *f, ध्रुव TValue *p1,
                        ध्रुव TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->शिखरम्.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->शिखरम्.p += 3;
  /* metamethod may yield only when called from Lua code */
  जइ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  अण्णहा
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->शिखरम्.p);  /* move result to its place */
  प्रत्यागम ttypetag(s2v(res));  /* return tag of the result */
}


स्थिर पूर्णाङ्क callbinTM (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2,
                      StkId res, TMS event) {
  ध्रुव TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  जइ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  जइ (notm(tm))
    प्रत्यागम -1;  /* tag method not found */
  अण्णहा  /* call tag method and return the tag of the result */
    प्रत्यागम luaT_callTMres(L, tm, p1, p2, res);
}


शून्य luaT_trybinTM (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2,
                    StkId res, TMS event) {
  जइ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    विकल्प (event) {
      पक्ष TM_BAND: पक्ष TM_BOR: पक्ष TM_BXOR:
      पक्ष TM_SHL: पक्ष TM_SHR: पक्ष TM_BNOT: {
        जइ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        अण्णहा
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      स्वभाव:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
शून्य luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->शिखरम्.p - 2;  /* first argument */
  जइ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


शून्य luaT_trybinassocTM (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2,
                                       पूर्णाङ्क flip, StkId res, TMS event) {
  जइ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  अण्णहा
    luaT_trybinTM(L, p1, p2, res, event);
}


शून्य luaT_trybiniTM (lua_State *L, ध्रुव TValue *p1, lua_Integer i2,
                                   पूर्णाङ्क flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
पूर्णाङ्क luaT_callorderTM (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2,
                      TMS event) {
  पूर्णाङ्क tag = callbinTM(L, p1, p2, L->शिखरम्.p, event);  /* try original event */
  जइ (tag >= 0)  /* found tag method? */
    प्रत्यागम !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  प्रत्यागम 0;  /* to avoid warnings */
}


पूर्णाङ्क luaT_callorderiTM (lua_State *L, ध्रुव TValue *p1, पूर्णाङ्क v2,
                       पूर्णाङ्क flip, पूर्णाङ्क isfloat, TMS event) {
  TValue aux; ध्रुव TValue *p2;
  जइ (isfloat) {
    setfltvalue(&aux, cast_अङ्कः(v2));
  }
  अण्णहा
    setivalue(&aux, v2);
  जइ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  अण्णहा
    p2 = &aux;
  प्रत्यागम luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
स्थिर शून्य createvarargtab (lua_State *L, StkId f, पूर्णाङ्क n) {
  पूर्णाङ्क i;
  TValue कुञ्चिका, मानम्;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->शिखरम्.p), t);
  L->शिखरम्.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &कुञ्चिका, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&मानम्, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &कुञ्चिका, &मानम्);  /* t.n = n */
  प्रति (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
स्थिर शून्य buildhiddenargs (lua_State *L, CallInfo *ci, ध्रुव Proto *p,
                             पूर्णाङ्क totalargs, पूर्णाङ्क nfixparams, पूर्णाङ्क nextra) {
  पूर्णाङ्क i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->शिखरम्.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  प्रति (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->शिखरम्.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->शिखरम्.p += totalargs + 1;
}


शून्य luaT_adjustvarargs (lua_State *L, CallInfo *ci, ध्रुव Proto *p) {
  पूर्णाङ्क totalargs = cast_int(L->शिखरम्.p - ci->func.p) - 1;
  पूर्णाङ्क nfixparams = p->numparams;
  पूर्णाङ्क nextra = totalargs - nfixparams;  /* number of extra arguments */
  जइ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->शिखरम्.p - 1);
  }
  अण्णहा {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->शिखरम्.p <= ci->शिखरम्.p && ci->शिखरम्.p <= L->राशिः_last.p);
  }
}


शून्य luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  पूर्णाङ्क nextra = ci->u.l.nextraargs;
  lua_Integer n;
  जइ (tointegerns(rc, &n)) {  /* integral value? */
    जइ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)रिक्त), ra, slot);
      प्रत्यागम;
    }
  }
  अण्णहा जइ (ttisstring(rc)) {  /* string value? */
    size_t len;
    ध्रुव वर्ण *s = getlstr(tsvalue(rc), len);
    जइ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      प्रत्यागम;
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
स्थिर पूर्णाङ्क getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  जइ (h == रिक्त)  /* no vararg table? */
    प्रत्यागम ci->u.l.nextraargs;
  अण्णहा {
    TValue res;
    जइ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    प्रत्यागम cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
शून्य luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, पूर्णाङ्क wanted,
                                    पूर्णाङ्क vatab) {
  Table *h = (vatab < 0) ? रिक्त : hvalue(s2v(ci->func.p + vatab + 1));
  पूर्णाङ्क nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  पूर्णाङ्क i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  जइ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->शिखरम्.p = where + nargs;  /* next instruction will need top */
  }
  अण्णहा
    touse = (nargs > wanted) ? wanted : nargs;
  जइ (h == रिक्त) {  /* no vararg table? */
    प्रति (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  अण्णहा {  /* get vararg values from vararg table */
    प्रति (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      जइ (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  प्रति (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

