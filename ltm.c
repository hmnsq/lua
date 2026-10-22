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


static const char udatatypename[] = "userdata";

LUAI_DDEF const char *const luaT_typenames_[LUA_TOTALTYPES] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto" /* these last cases are used for tests only */
};


void luaT_init (lua_State *L) {
  static const char *const luaT_eventname[] = {  /* ORDER TM */
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  整数 i;
  为 (i=0; i<TM_N; i++) {
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
  如果 (notm(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    返回 NULL;
  }
  否则 返回 tm;
}


const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o, TMS event) {
  Table *mt;
  switch (ttype(o)) {
    case LUA_TTABLE:
      mt = hvalue(o)->metatable;
      break;
    case LUA_TUSERDATA:
      mt = uvalue(o)->metatable;
      break;
    default:
      mt = G(L)->mt[ttype(o)];
  }
  返回 (mt ? luaH_Hgetshortstr(mt, G(L)->tmname[event]) : &G(L)->nilvalue);
}


/*
** Return the name of the type of an object. For tables and userdata
** with metatable, use their '__name' metafield, if present.
*/
const char *luaT_objtypename (lua_State *L, const TValue *o) {
  Table *mt;
  如果 ((ttistable(o) && (mt = hvalue(o)->metatable) != NULL) ||
      (ttisfulluserdata(o) && (mt = uvalue(o)->metatable) != NULL)) {
    const TValue *name = luaH_Hgetshortstr(mt, luaS_new(L, "__name"));
    如果 (ttisstring(name))  /* is '__name' a string? */
      返回 getstr(tsvalue(name));  /* use it as type name */
  }
  返回 ttypename(ttype(o));  /* else use standard type name */
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
  如果 (isLuacode(L->ci))
    luaD_call(L, func, 0);
  否则
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
  如果 (isLuacode(L->ci))
    luaD_call(L, func, 1);
  否则
    luaD_callnoyield(L, func, 1);
  res = restorestack(L, result);
  setobjs2s(L, res, --L->top.p);  /* move result to its place */
  返回 ttypetag(s2v(res));  /* return tag of the result */
}


static 整数 callbinTM (lua_State *L, const TValue *p1, const TValue *p2,
                      StkId res, TMS event) {
  const TValue *tm = luaT_gettmbyobj(L, p1, event);  /* try first operand */
  如果 (notm(tm))
    tm = luaT_gettmbyobj(L, p2, event);  /* try second operand */
  如果 (notm(tm))
    返回 -1;  /* tag method not found */
  否则  /* call tag method and return the tag of the result */
    返回 luaT_callTMres(L, tm, p1, p2, res);
}


void luaT_trybinTM (lua_State *L, const TValue *p1, const TValue *p2,
                    StkId res, TMS event) {
  如果 (l_unlikely(callbinTM(L, p1, p2, res, event) < 0)) {
    switch (event) {
      case TM_BAND: case TM_BOR: case TM_BXOR:
      case TM_SHL: case TM_SHR: case TM_BNOT: {
        如果 (ttisnumber(p1) && ttisnumber(p2))
          luaG_tointerror(L, p1, p2);
        否则
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
  如果 (l_unlikely(callbinTM(L, s2v(p1), s2v(p1 + 1), p1, TM_CONCAT) < 0))
    luaG_concaterror(L, s2v(p1), s2v(p1 + 1));
}


void luaT_trybinassocTM (lua_State *L, const TValue *p1, const TValue *p2,
                                       整数 flip, StkId res, TMS event) {
  如果 (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  否则
    luaT_trybinTM(L, p1, p2, res, event);
}


void luaT_trybiniTM (lua_State *L, const TValue *p1, lua_Integer i2,
                                   整数 flip, StkId res, TMS event) {
  TValue aux;
  setivalue(&aux, i2);
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}


/*
** Calls an order tag method.
*/
整数 luaT_callorderTM (lua_State *L, const TValue *p1, const TValue *p2,
                      TMS event) {
  整数 tag = callbinTM(L, p1, p2, L->top.p, event);  /* try original event */
  如果 (tag >= 0)  /* found tag method? */
    返回 !tagisfalse(tag);
  luaG_ordererror(L, p1, p2);  /* no metamethod found */
  返回 0;  /* to avoid warnings */
}


整数 luaT_callorderiTM (lua_State *L, const TValue *p1, 整数 v2,
                       整数 flip, 整数 isfloat, TMS event) {
  TValue aux; const TValue *p2;
  如果 (isfloat) {
    setfltvalue(&aux, cast_num(v2));
  }
  否则
    setivalue(&aux, v2);
  如果 (flip) {  /* arguments were exchanged? */
    p2 = p1; p1 = &aux;  /* correct them */
  }
  否则
    p2 = &aux;
  返回 luaT_callorderTM(L, p1, p2, event);
}


/*
** Create a vararg table at the top of the stack, with 'n' elements
** starting at 'f'.
*/
static void createvarargtab (lua_State *L, StkId f, 整数 n) {
  整数 i;
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
  为 (i = 0; i < n; i++)
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
                             整数 totalargs, 整数 nfixparams, 整数 nextra) {
  整数 i;
  ci->u.l.nextraargs = nextra;
  luaD_checkstack(L, p->maxstacksize + 1);
  /* copy function to the top of the stack, after extra arguments */
  setobjs2s(L, L->top.p++, ci->func.p);
  /* move fixed parameters to after the copied function */
  为 (i = 1; i <= nfixparams; i++) {
    setobjs2s(L, L->top.p++, ci->func.p + i);
    setnilvalue2s(ci->func.p + i);  /* erase original parameter (for GC) */
  }
  ci->func.p += totalargs + 1;  /* 'func' now lives after hidden arguments */
  ci->top.p += totalargs + 1;
}


void luaT_adjustvarargs (lua_State *L, CallInfo *ci, const Proto *p) {
  整数 totalargs = cast_int(L->top.p - ci->func.p) - 1;
  整数 nfixparams = p->numparams;
  整数 nextra = totalargs - nfixparams;  /* number of extra arguments */
  如果 (p->flag & PF_VATAB) {  /* does it need a vararg table? */
    lua_assert(!(p->flag & PF_VAHID));
    createvarargtab(L, ci->func.p + nfixparams + 1, nextra);
    /* move table to proper place (last parameter) */
    setobjs2s(L, ci->func.p + nfixparams + 1, L->top.p - 1);
  }
  否则 {  /* no table */
    lua_assert(p->flag & PF_VAHID);
    buildhiddenargs(L, ci, p, totalargs, nfixparams, nextra);
    /* set vararg parameter to nil */
    setnilvalue2s(ci->func.p + nfixparams + 1);
    lua_assert(L->top.p <= ci->top.p && ci->top.p <= L->stack_last.p);
  }
}


void luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc) {
  整数 nextra = ci->u.l.nextraargs;
  lua_Integer n;
  如果 (tointegerns(rc, &n)) {  /* integral value? */
    如果 (l_castS2U(n) - 1 < cast_uint(nextra)) {
      StkId slot = ci->func.p - nextra + cast_int(n) - 1;
      setobjs2s(((lua_State*)NULL), ra, slot);
      返回;
    }
  }
  否则 如果 (ttisstring(rc)) {  /* string value? */
    size_t len;
    const char *s = getlstr(tsvalue(rc), len);
    如果 (len == 1 && s[0] == 'n') {  /* key is "n"? */
      setivalue(s2v(ra), nextra);
      返回;
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
static 整数 getnumargs (lua_State *L, CallInfo *ci, Table *h) {
  如果 (h == NULL)  /* no vararg table? */
    返回 ci->u.l.nextraargs;
  否则 {
    TValue res;
    如果 (luaH_getshortstr(h, luaS_new(L, "n"), &res) != LUA_VNUMINT ||
        l_castS2U(ivalue(&res)) > cast_uint(INT_MAX/2))
      luaG_runerror(L, "vararg table has no proper 'n'");
    返回 cast_int(ivalue(&res));
  }
}


/*
** Get 'wanted' vararg arguments and put them in 'where'. 'vatab' is
** the register of the vararg table or -1 if there is no vararg table.
*/
void luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, 整数 wanted,
                                    整数 vatab) {
  Table *h = (vatab < 0) ? NULL : hvalue(s2v(ci->func.p + vatab + 1));
  整数 nargs = getnumargs(L, ci, h);  /* number of available vararg args. */
  整数 i, touse;  /* 'touse' is minimum between 'wanted' and 'nargs' */
  如果 (wanted < 0) {
    touse = wanted = nargs;  /* get all extra arguments available */
    checkstackp(L, nargs, where);  /* ensure stack space */
    L->top.p = where + nargs;  /* next instruction will need top */
  }
  否则
    touse = (nargs > wanted) ? wanted : nargs;
  如果 (h == NULL) {  /* no vararg table? */
    为 (i = 0; i < touse; i++)  /* get vararg values from the stack */
      setobjs2s(L, where + i, ci->func.p - nargs + i);
  }
  否则 {  /* get vararg values from vararg table */
    为 (i = 0; i < touse; i++) {
      lu_byte tag = luaH_getint(h, i + 1, s2v(where + i));
      如果 (tagisempty(tag))
       setnilvalue2s(where + i);
    }
  }
  为 (; i < wanted; i++)   /* complete required results with nil */
    setnilvalue2s(where + i);
}

