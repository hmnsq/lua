/*
** $Id: ltm.c $
** Tag methods
** See Copyright Notice in lua.h
*/

#define ltm_c
#define LUA_CORE

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"


static const var_na udatatypename[] = "userdata";

LUAI_DDEF const var_na *const luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


void luaT_init (lua_State *L) {
  static const var_na *const luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  ຈຳນວນເຕັມ i;
  ສຳລັບ (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, obj2gco(G(L)->tmname[i]));  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
const TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  const TValue *tm = luaH_Hgetshortstr(events, ename);
  lua_assert(event <= TM_EQ);
  ຖ້າ (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    ສົ່ງຄືນ NULL;
  }
  ບໍ່ດັ່ງນັ້ນ ສົ່ງຄືນ tm;
}


const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o, TMS event) {
  Table *mt;
  switch (ttype(o)) {
    pak_sa LUA_TTABLE:
      mt = hvalue(o)->metatable;
      bha_nga;
    pak_sa LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      bha_nga;
    default:
      mt = G(L)->mt[ttype(o)];
  }
  ສົ່ງຄືນ (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
const var_na *luaT_objtypename (lua_State *L, const TValue *o) {
  Table *mt;
  ຖ້າ ((ttistable(o) && (mt = hvalue(o)->metatable) != NULL) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != NULL)) {
    const TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    ຖ້າ (ttisstring(name))  /* is '__name' a string? */
      ສົ່ງຄືນ getstr(tsvalue(name));  /* use it as type name */
  }
  ສົ່ງຄືນ ttypename(ttype(o));  /* else use standard type name */
}


void luaT_callTM (lua_State *L, const TValue *f, const TValue *p1,
                  const TValue *p2, const TValue *p3) {
  StkId func = L->top.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  setobj2s(L, func + 3, p3);  /* 3rd argument */
  L->top.p = func + 4;
  /* metamethod may yield only when called from Lua code */
  ຖ້າ (isLuacode(L->ci))
    luaD_call(L, func, 0);
  ບໍ່ດັ່ງນັ້ນ
    luaD_callnoyield(L, func, 0);
}


lu_byte luaT_callTMres (lua_State *L, const TValue *f, const TValue *p1,
                        const TValue *p2, StkId res) {
  ptrdiff_t result = savestack(L, res);
  StkId func = L->top.p;
  setobj2s(L, func, f);  /* push function (assume EXTRA_STACK) */
  setobj2s(L, func + 1, p1);  /* 1st argument */
  setobj2s(L, func + 2, p2);  /* 2nd argument */
  L->top.p += 3;
  /* metamethod may yield only when called from Lua code */
  ຖ້າ (isLuacode(L->ci))
    luaD_call(L, func, 1);
  ບໍ່ດັ່ງນັ້ນ
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->top.p);  /* move result to its place */
  ສົ່ງຄືນ ttypetag(s2v(res));  /* return tag of the result */
}


static ຈຳນວນເຕັມ callbinTM (lua_State *L, const TValue *p1, const TValue *p2,
                      StkId res, TMS event) {
  const TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  ຖ້າ (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  ຖ້າ (notm(tm))
    ສົ່ງຄືນ -1;  /* tag method not found */
  ບໍ່ດັ່ງນັ້ນ  /* call tag method and return the tag of the result */
    ສົ່ງຄືນ luaT_callTMres(L, tm, p1, p2, res);
}


void luaT_trybinTM (lua_State *L, const TValue *p1, const TValue *p2,
                    StkId res, TMS event) {
  ຖ້າ (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    switch (event) {
      pak_sa TM_BAND: pak_sa TM_BOR: pak_sa TM_BXOR:
      pak_sa TM_SHL: pak_sa TM_SHR: pak_sa TM_BNOT: {
        ຖ້າ (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        ບໍ່ດັ່ງນັ້ນ
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }
      /* calls never return, but to avoid warnings: *//* FALLTHROUGH */
      default:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


/*
** The use of 'p1' after 'callbinTM' is safe because, when a tag
** method is not found, 'callbinTM' cannot change the stack.
*/
void luaT_tryconcatTM (lua_State *L) {
  StkId p1 = L->top.p - 2;  /* first argument */
  ຖ້າ (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


void luaT_trybinassocTM (lua_State *L, const TValue *p1, const TValue *p2,
                                       ຈຳນວນເຕັມ flip, StkId res, TMS event) {
  ຖ້າ (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  ບໍ່ດັ່ງນັ້ນ
    luaT_trybinTM(L, p1, p2, res, event);
}


void luaT_trybiniTM (lua_State *L, const TValue *p1, lua_Integer i2,
                                   ຈຳນວນເຕັມ flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
ຈຳນວນເຕັມ luaT_callorderTM (lua_State *L, const TValue *p1, const TValue *p2,
                      TMS event) {
  ຈຳນວນເຕັມ tag = callbinTM(L, p1, p2, L->top.p, event);  /* try original event */
  ຖ້າ (tag >= 0)  /* found tag method? */
    ສົ່ງຄືນ !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  ສົ່ງຄືນ 0;  /* to avoid warnings */
}


ຈຳນວນເຕັມ luaT_callorderiTM (lua_State *L, const TValue *p1, ຈຳນວນເຕັມ v2,
                       ຈຳນວນເຕັມ flip, ຈຳນວນເຕັມ isfloat, TMS event) {
  TValue aux; const TValue *p2;
  ຖ້າ (isfloat) {
    setfltvalue(&aux, cast_num(v2));
  }
  ບໍ່ດັ່ງນັ້ນ
    setivalue(&aux, v2);
  ຖ້າ (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  ບໍ່ດັ່ງນັ້ນ
    p2 = &aux;
  ສົ່ງຄືນ luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
static void createvarargtab (lua_State *L, StkId f, ຈຳນວນເຕັມ n) {
  ຈຳນວນເຕັມ i;
  TValue key, value;
  Table *t = luaH_new(L);
  sethvalue(L, s2v(L->top.p), t);
  L->top.p++;
  luaH_resize(L, t, cast_uint(n), 1);
  setsvalue(L, &key, luaS_new(L, "n"));  /* key is "n" */
  setivalue(&value, n);  /* value is n */
  /* No need to anchor the key: Due to the resize, the next operation
     cannot trigger a garbage collection */
  luaH_set(L, t, &key, &value);  /* t.n = n */
  ສຳລັບ (i = 0; i < n; i++)
    luaH_setint(L, t, i + 1, s2v(f + i));
  luaC_checkGC(L);
}


/*
** initial stack:  func arg1 ... argn extra1 ...
**                 ^ ci->func                    ^ L->top
** final stack: func nil ... nil extra1 ... func arg1 ... argn
**                                          ^ ci->func
*/
static void buildhiddenargs (lua_State *L, CallInfo *ci, const Proto *p,
                             ຈຳນວນເຕັມ totalargs, ຈຳນວນເຕັມ nfixparams, ຈຳນວນເຕັມ nextra) {
  ຈຳນວນເຕັມ i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->top.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  ສຳລັບ (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->top.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->top.p += totalargs + 1;
}


void luaT_adjustvarargs (lua_State *L, CallInfo *ci, const Proto *p) {
  ຈຳນວນເຕັມ totalargs = cast_int(L->top.p - ci->func.p) - 1;
  ຈຳນວນເຕັມ nfixparams = p->numparams;
  ຈຳນວນເຕັມ nextra = totalargs - nfixparams;  /* number of extra arguments */
  ຖ້າ (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->top.p - 1);
  }
  ບໍ່ດັ່ງນັ້ນ {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->top.p <= ci->top.p && ci->top.p <= L->stack_last.p);
  }
}


void luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  ຈຳນວນເຕັມ nextra = ci->u.l.nextraargs;
  lua_Integer n;
  ຖ້າ (tointegerns(rc, &n)) {  /* integral value? */
    ຖ້າ (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)NULL), ra, slot);
      ສົ່ງຄືນ;
    }
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (ttisstring(rc)) {  /* string value? */
    size_t len;
    const var_na *s = getlstr(tsvalue(rc), len);
    ຖ້າ (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      ສົ່ງຄືນ;
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
static ຈຳນວນເຕັມ getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  ຖ້າ (h == NULL)  /* no vararg table? */
    ສົ່ງຄືນ ci->u.l.nextraargs;
  ບໍ່ດັ່ງນັ້ນ {
    TValue res;
    ຖ້າ (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    ສົ່ງຄືນ cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
void luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, ຈຳນວນເຕັມ wanted,
                                    ຈຳນວນເຕັມ vatab) {
  Table *h = (vatab < 0) ? NULL : hvalue(s2v(ci->func.p + vatab + 1));
  ຈຳນວນເຕັມ nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  ຈຳນວນເຕັມ i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  ຖ້າ (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->top.p = where + nargs;  /* next instruction will need top */
  }
  ບໍ່ດັ່ງນັ້ນ
    touse = (nargs > wanted) ? wanted : nargs;
  ຖ້າ (h == NULL) {  /* no vararg table? */
    ສຳລັບ (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  ບໍ່ດັ່ງນັ້ນ {  /* get vararg values from vararg table */
    ສຳລັບ (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      ຖ້າ (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  ສຳລັບ (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

