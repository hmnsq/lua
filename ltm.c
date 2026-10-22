/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "ltm.h"
#அந்தர்பாவ "lvm.h"


நிலையான மாறிலி எழுத்து udatatypename[] = "userdata";

LUAI_DDEF மாறிலி எழுத்து *மாறிலி luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


வெறுமை luaT_init (lua_State *L) {
  நிலையான மாறிலி எழுத்து *மாறிலி luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  முழு i;
  க்கு (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
மாறிலி TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  மாறிலி TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  எனில் (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    திரும்பு வெற்று;
  }
  இல்லையெனில் திரும்பு tm;
}


மாறிலி TValue *luaT_gettmbyobj (lua_State *L, மாறிலி TValue *o, TMS event) {
  Table *mt;
  தேர்வு (ttype(o)) {
    நிலை LUA_TTABLE:
      mt = hvalue(o)->metatable;
      நிறுத்து;
    நிலை LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      நிறுத்து;
    இயல்பு:
      mt = G(L)->mt[ttype(o)];
  }
  திரும்பு (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
மாறிலி எழுத்து *luaT_objtypename (lua_State *L, மாறிலி TValue *o) {
  Table *mt;
  எனில் ((ttistable(o) && (mt = hvalue(o)->metatable) != வெற்று) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != வெற்று)) {
    மாறிலி TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    எனில் (ttisstring(name))  /* is '__name' a string? */
      திரும்பு getstr(tsvalue(name));  /* use it as type name */
  }
  திரும்பு ttypename(ttype(o));  /* else use standard type name */
}


வெறுமை luaT_callTM (lua_State *L, மாறிலி TValue *f, மாறிலி TValue *p1,
                  மாறிலி TValue *p2, மாறிலி TValue *p3) {
  StkId func = L->உச்சி.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->உச்சி.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  எனில் (isLuacode(L->ci))
    luaD_call(L, func, 0);
  இல்லையெனில்
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, மாறிலி TValue *f, மாறிலி TValue *p1,
                        மாறிலி TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->உச்சி.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->உச்சி.p += 3;
  /* metamethod may yield only when called from Lua code */
  எனில் (isLuacode(L->ci))
    luaD_call(L, func, 1);
  இல்லையெனில்
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->உச்சி.p);  /* move result to its place */
  திரும்பு ttypetag(s2v(res));  /* return tag of the result */
}


நிலையான முழு callbinTM (lua_State *L, மாறிலி TValue *p1, மாறிலி TValue *p2,
                      StkId res, TMS event) {
  மாறிலி TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  எனில் (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  எனில் (notm(tm))
    திரும்பு -1;  /* tag method not found */
  இல்லையெனில்  /* call tag method and return the tag of the result */
    திரும்பு luaT_callTMres(L, tm, p1, p2, res);
}


வெறுமை luaT_trybinTM (lua_State *L, மாறிலி TValue *p1, மாறிலி TValue *p2,
                    StkId res, TMS event) {
  எனில் (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    தேர்வு (event) {
      நிலை TM_BAND: நிலை TM_BOR: நிலை TM_BXOR:
      நிலை TM_SHL: நிலை TM_SHR: நிலை TM_BNOT: {
        எனில் (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        இல்லையெனில்
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      இயல்பு:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
வெறுமை luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->உச்சி.p - 2;  /* first argument */
  எனில் (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


வெறுமை luaT_trybinassocTM (lua_State *L, மாறிலி TValue *p1, மாறிலி TValue *p2,
                                       முழு flip, StkId res, TMS event) {
  எனில் (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  இல்லையெனில்
    luaT_trybinTM(L, p1, p2, res, event);
}


வெறுமை luaT_trybiniTM (lua_State *L, மாறிலி TValue *p1, lua_Integer i2,
                                   முழு flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
முழு luaT_callorderTM (lua_State *L, மாறிலி TValue *p1, மாறிலி TValue *p2,
                      TMS event) {
  முழு tag = callbinTM(L, p1, p2, L->உச்சி.p, event);  /* try original event */
  எனில் (tag >= 0)  /* found tag method? */
    திரும்பு !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  திரும்பு 0;  /* to avoid warnings */
}


முழு luaT_callorderiTM (lua_State *L, மாறிலி TValue *p1, முழு v2,
                       முழு flip, முழு isfloat, TMS event) {
  TValue aux; மாறிலி TValue *p2;
  எனில் (isfloat) {
    setfltvalue(&aux, cast_இலக்கம்(v2));
  }
  இல்லையெனில்
    setivalue(&aux, v2);
  எனில் (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  இல்லையெனில்
    p2 = &aux;
  திரும்பு luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
நிலையான வெறுமை createvarargtab (lua_State *L, StkId f, முழு n) {
  முழு i;
  TValue சாவி, மதிப்பு;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->உச்சி.p), t);
  L->உச்சி.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &சாவி, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&மதிப்பு, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &சாவி, &மதிப்பு);  /* t.n = n */
  க்கு (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
நிலையான வெறுமை buildhiddenargs (lua_State *L, CallInfo *ci, மாறிலி Proto *p,
                             முழு totalargs, முழு nfixparams, முழு nextra) {
  முழு i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->உச்சி.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  க்கு (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->உச்சி.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->உச்சி.p += totalargs + 1;
}


வெறுமை luaT_adjustvarargs (lua_State *L, CallInfo *ci, மாறிலி Proto *p) {
  முழு totalargs = cast_int(L->உச்சி.p - ci->func.p) - 1;
  முழு nfixparams = p->numparams;
  முழு nextra = totalargs - nfixparams;  /* number of extra arguments */
  எனில் (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->உச்சி.p - 1);
  }
  இல்லையெனில் {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->உச்சி.p <= ci->உச்சி.p && ci->உச்சி.p <= L->அடுக்கு_last.p);
  }
}


வெறுமை luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  முழு nextra = ci->u.l.nextraargs;
  lua_Integer n;
  எனில் (tointegerns(rc, &n)) {  /* integral value? */
    எனில் (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)வெற்று), ra, slot);
      திரும்பு;
    }
  }
  இல்லையெனில் எனில் (ttisstring(rc)) {  /* string value? */
    size_t len;
    மாறிலி எழுத்து *s = getlstr(tsvalue(rc), len);
    எனில் (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      திரும்பு;
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
நிலையான முழு getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  எனில் (h == வெற்று)  /* no vararg table? */
    திரும்பு ci->u.l.nextraargs;
  இல்லையெனில் {
    TValue res;
    எனில் (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    திரும்பு cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
வெறுமை luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, முழு wanted,
                                    முழு vatab) {
  Table *h = (vatab < 0) ? வெற்று : hvalue(s2v(ci->func.p + vatab + 1));
  முழு nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  முழு i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  எனில் (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->உச்சி.p = where + nargs;  /* next instruction will need top */
  }
  இல்லையெனில்
    touse = (nargs > wanted) ? wanted : nargs;
  எனில் (h == வெற்று) {  /* no vararg table? */
    க்கு (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  இல்லையெனில் {  /* get vararg values from vararg table */
    க்கு (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      எனில் (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  க்கு (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

