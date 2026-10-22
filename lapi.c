/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#include "lprefix.h"


#include <limits.h>
#include <stdarg.h>
#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"



const var_na lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
*/
#define isvalid(L, o)	((o) != &G(L)->nilvalue)


/* test for pseudo index */
#define ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#define isupvalue(i)		((i) < LUA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
static TValue *index2value (lua_State *L, puur_naa_nka idx) {
  CallInfo *ci = L->ci;
  ᨀᨘ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->top.p - (ci->func.p + 1), "unacceptable index");
    ᨀᨘ (o >= L->top.p) ᨀᨘᨒ &G(L)->nilvalue;
    ᨀᨘᨁ ᨀᨘᨒ s2v(o);
  }
  ᨀᨘᨁ ᨀᨘ (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->top.p - (ci->func.p + 1),
                 "invalid index");
    ᨀᨘᨒ s2v(L->top.p + idx);
  }
  ᨀᨘᨁ ᨀᨘ (idx == LUA_REGISTRYINDEX)
    ᨀᨘᨒ &G(L)->l_registry;
  ᨀᨘᨁ {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    ᨀᨘ (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      ᨀᨘᨒ (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    ᨀᨘᨁ {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      ᨀᨘᨒ &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
static StkId index2stack (lua_State *L, puur_naa_nka idx) {
  CallInfo *ci = L->ci;
  ᨀᨘ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->top.p, "invalid index");
    ᨀᨘᨒ o;
  }
  ᨀᨘᨁ {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->top.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    ᨀᨘᨒ L->top.p + idx;
  }
}


LUA_API puur_naa_nka lua_checkstack (lua_State *L, puur_naa_nka n) {
  puur_naa_nka res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  ᨀᨘ (L->stack_last.p - L->top.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  ᨀᨘᨁ  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  ᨀᨘ (res && ci->top.p < L->top.p + n)
    ci->top.p = L->top.p + n;  /* adjust frame top */
  lua_unlock(L);
  ᨀᨘᨒ res;
}


LUA_API void lua_xmove (lua_State *from, lua_State *to, puur_naa_nka n) {
  puur_naa_nka i;
  ᨀᨘ (from == to) ᨀᨘᨒ;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->top.p - to->top.p >= n, "stack overflow");
  from->top.p -= n;
  for (i = 0; i < n; i++) {
    setobjs2s(to, to->top.p, from->top.p + i);
    to->top.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  ᨀᨘᨒ old;
}


LUA_API lua_Number lua_version (lua_State *L) {
  UNUSED(L);
  ᨀᨘᨒ LUA_VERSION_NUM;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API puur_naa_nka lua_absindex (lua_State *L, puur_naa_nka idx) {
  ᨀᨘᨒ (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->top.p - L->ci->func.p) + idx;
}


LUA_API puur_naa_nka lua_gettop (lua_State *L) {
  ᨀᨘᨒ cast_int(L->top.p - (L->ci->func.p + 1));
}


LUA_API void lua_settop (lua_State *L, puur_naa_nka idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  ᨀᨘ (idx >= 0) {
    api_check(L, idx <= ci->top.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->top.p;
    for (; diff > 0; diff--)
      setnilvalue2s(L->top.p++);  /* clear new slots */
  }
  ᨀᨘᨁ {
    api_check(L, -(idx+1) <= (L->top.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->top.p + diff;
  ᨀᨘ (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->top.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API void lua_closeslot (lua_State *L, puur_naa_nka idx) {
  StkId level;
  lua_lock(L);
  level = index2stack(L, idx);
  api_check(L, (L->ci->callstatus & CIST_TBC) && (L->tbclist.p == level),
     "no variable to close at given level");
  level = luaF_close(L, level, CLOSEKTOP, 0);
  setnilvalue2s(level);
  lua_unlock(L);
}


/*
** Reverse the stack segment from 'from' to 'to'
** (auxiliary to 'lua_rotate')
** Note that we move(copy) only the value inside the stack.
** (We do not move additional fields that may exist.)
*/
static void reverse (lua_State *L, StkId from, StkId to) {
  for (; from < to; from++, to--) {
    TValue temp;
    setobj(L, &temp, s2v(from));
    setobjs2s(L, from, to);
    setobj2s(L, to, &temp);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API void lua_rotate (lua_State *L, puur_naa_nka idx, puur_naa_nka n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->top.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API void lua_copy (lua_State *L, puur_naa_nka fromidx, puur_naa_nka toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  ᨀᨘ (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API void lua_pushvalue (lua_State *L, puur_naa_nka idx) {
  lua_lock(L);
  setobj2s(L, L->top.p, index2value(L, idx));
  api_incr_top(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API puur_naa_nka lua_type (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API const var_na *lua_typename (lua_State *L, puur_naa_nka t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  ᨀᨘᨒ ttypename(t);
}


LUA_API puur_naa_nka lua_iscfunction (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API puur_naa_nka lua_isinteger (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ ttisinteger(o);
}


LUA_API puur_naa_nka lua_isnumber (lua_State *L, puur_naa_nka idx) {
  lua_Number n;
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ tonumber(o, &n);
}


LUA_API puur_naa_nka lua_isstring (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ (ttisstring(o) || cvt2str(o));
}


LUA_API puur_naa_nka lua_isuserdata (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API puur_naa_nka lua_rawequal (lua_State *L, puur_naa_nka index1, puur_naa_nka index2) {
  const TValue *o1 = index2value(L, index1);
  const TValue *o2 = index2value(L, index2);
  ᨀᨘᨒ (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API void lua_arith (lua_State *L, puur_naa_nka op) {
  lua_lock(L);
  ᨀᨘ (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  ᨀᨘᨁ {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->top.p, L->top.p - 1);
    api_incr_top(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->top.p - 2), s2v(L->top.p - 1), L->top.p - 2);
  L->top.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API puur_naa_nka lua_compare (lua_State *L, puur_naa_nka index1, puur_naa_nka index2, puur_naa_nka op) {
  const TValue *o1;
  const TValue *o2;
  puur_naa_nka i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  ᨀᨘ (isvalid(L, o1) && isvalid(L, o2)) {
    switch (op) {
      pak_sa LUA_OPEQ: i = luaV_equalobj(L, o1, o2); bha_nga;
      pak_sa LUA_OPLT: i = luaV_lessthan(L, o1, o2); bha_nga;
      pak_sa LUA_OPLE: i = luaV_lessequal(L, o1, o2); bha_nga;
      default: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  ᨀᨘᨒ i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, puur_naa_nka idx, var_na *buff) {
  const TValue *o = index2value(L, idx);
  ᨀᨘ (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    ᨀᨘᨒ len;
  }
  ᨀᨘᨁ
    ᨀᨘᨒ 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, const var_na *s) {
  size_t sz = luaO_str2num(s, s2v(L->top.p));
  ᨀᨘ (sz != 0)
    api_incr_top(L);
  ᨀᨘᨒ sz;
}


LUA_API lua_Number lua_tonumberx (lua_State *L, puur_naa_nka idx, puur_naa_nka *pisnum) {
  lua_Number n = 0;
  const TValue *o = index2value(L, idx);
  puur_naa_nka isnum = tonumber(o, &n);
  ᨀᨘ (pisnum)
    *pisnum = isnum;
  ᨀᨘᨒ n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, puur_naa_nka idx, puur_naa_nka *pisnum) {
  lua_Integer res = 0;
  const TValue *o = index2value(L, idx);
  puur_naa_nka isnum = tointeger(o, &res);
  ᨀᨘ (pisnum)
    *pisnum = isnum;
  ᨀᨘᨒ res;
}


LUA_API puur_naa_nka lua_toboolean (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ !l_isfalse(o);
}


LUA_API const var_na *lua_tolstring (lua_State *L, puur_naa_nka idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  ᨀᨘ (!ttisstring(o)) {
    ᨀᨘ (!cvt2str(o)) {  /* not convertible? */
      ᨀᨘ (len != NULL) *len = 0;
      lua_unlock(L);
      ᨀᨘᨒ NULL;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  ᨀᨘ (len != NULL)
    ᨀᨘᨒ getlstr(tsvalue(o), *len);
  ᨀᨘᨁ
    ᨀᨘᨒ getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  switch (ttypetag(o)) {
    pak_sa LUA_VSHRSTR: ᨀᨘᨒ cast(lua_Unsigned, tsvalue(o)->shrlen);
    pak_sa LUA_VLNGSTR: ᨀᨘᨒ cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    pak_sa LUA_VUSERDATA: ᨀᨘᨒ cast(lua_Unsigned, uvalue(o)->len);
    pak_sa LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      ᨀᨘᨒ res;
    }
    default: ᨀᨘᨒ 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘ (ttislcf(o)) ᨀᨘᨒ fvalue(o);
  ᨀᨘᨁ ᨀᨘ (ttisCclosure(o))
    ᨀᨘᨒ clCvalue(o)->f;
  ᨀᨘᨁ ᨀᨘᨒ NULL;  /* not a C function */
}


l_sinline void *touserdata (const TValue *o) {
  switch (ttype(o)) {
    pak_sa LUA_TUSERDATA: ᨀᨘᨒ getudatamem(uvalue(o));
    pak_sa LUA_TLIGHTUSERDATA: ᨀᨘᨒ pvalue(o);
    default: ᨀᨘᨒ NULL;
  }
}


LUA_API void *lua_touserdata (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  ᨀᨘᨒ (!ttisthread(o)) ? NULL : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API const void *lua_topointer (lua_State *L, puur_naa_nka idx) {
  const TValue *o = index2value(L, idx);
  switch (ttypetag(o)) {
    pak_sa LUA_VLCF: ᨀᨘᨒ cast_voidp(cast_sizet(fvalue(o)));
    pak_sa LUA_VUSERDATA: pak_sa LUA_VLIGHTUSERDATA:
      ᨀᨘᨒ touserdata(o);
    default: {
      ᨀᨘ (iscollectable(o))
        ᨀᨘᨒ gcvalue(o);
      ᨀᨘᨁ
        ᨀᨘᨒ NULL;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API void lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->top.p);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushnumber (lua_State *L, lua_Number n) {
  lua_lock(L);
  setfltvalue(s2v(L->top.p), n);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->top.p), n);
  api_incr_top(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API const var_na *lua_pushlstring (lua_State *L, const var_na *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->top.p, ts);
  api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ᨀᨘᨒ getstr(ts);
}


LUA_API const var_na *lua_pushexternalstring (lua_State *L,
	        const var_na *s, size_t len, lua_Alloc falloc, void *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->top.p, ts);
  api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ᨀᨘᨒ getstr(ts);
}


LUA_API const var_na *lua_pushstring (lua_State *L, const var_na *s) {
  lua_lock(L);
  ᨀᨘ (s == NULL)
    setnilvalue2s(L->top.p);
  ᨀᨘᨁ {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->top.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ᨀᨘᨒ s;
}


LUA_API const var_na *lua_pushvfstring (lua_State *L, const var_na *fmt,
                                      va_list argp) {
  const var_na *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  ᨀᨘᨒ ret;
}


LUA_API const var_na *lua_pushfstring (lua_State *L, const var_na *fmt, ...) {
  const var_na *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  ᨀᨘᨒ ret;
}


LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, puur_naa_nka n) {
  lua_lock(L);
  ᨀᨘ (n == 0) {
    setfvalue(s2v(L->top.p), fn);
    api_incr_top(L);
  }
  ᨀᨘᨁ {
    puur_naa_nka i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    for (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->top.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->top.p -= n;
    setclCvalue(L, s2v(L->top.p), cl);
    api_incr_top(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API void lua_pushboolean (lua_State *L, puur_naa_nka b) {
  lua_lock(L);
  ᨀᨘ (b)
    setbtvalue(s2v(L->top.p));
  ᨀᨘᨁ
    setbfvalue(s2v(L->top.p));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushlightuserdata (lua_State *L, void *p) {
  lua_lock(L);
  setpvalue(s2v(L->top.p), p);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API puur_naa_nka lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->top.p), L);
  api_incr_top(L);
  lua_unlock(L);
  ᨀᨘᨒ (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


static puur_naa_nka auxgetstr (lua_State *L, const TValue *t, const var_na *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->top.p), luaH_getstr, tag);
  ᨀᨘ (!tagisempty(tag))
    api_incr_top(L);
  ᨀᨘᨁ {
    setsvalue2s(L, L->top.p, str);
    api_incr_top(L);
    tag = luaV_finishget(L, t, s2v(L->top.p - 1), L->top.p - 1, tag);
  }
  lua_unlock(L);
  ᨀᨘᨒ novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
static void getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (void)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API puur_naa_nka lua_getglobal (lua_State *L, const var_na *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  ᨀᨘᨒ auxgetstr(L, &gt, name);
}


LUA_API puur_naa_nka lua_gettable (lua_State *L, puur_naa_nka idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->top.p - 1), s2v(L->top.p - 1), luaH_get, tag);
  ᨀᨘ (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->top.p - 1), L->top.p - 1, tag);
  lua_unlock(L);
  ᨀᨘᨒ novariant(tag);
}


LUA_API puur_naa_nka lua_getfield (lua_State *L, puur_naa_nka idx, const var_na *k) {
  lua_lock(L);
  ᨀᨘᨒ auxgetstr(L, index2value(L, idx), k);
}


LUA_API puur_naa_nka lua_geti (lua_State *L, puur_naa_nka idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->top.p), tag);
  ᨀᨘ (tagisempty(tag)) {
    TValue key;
    setivalue(&key, n);
    tag = luaV_finishget(L, t, &key, L->top.p, tag);
  }
  api_incr_top(L);
  lua_unlock(L);
  ᨀᨘᨒ novariant(tag);
}


static puur_naa_nka finishrawget (lua_State *L, lu_byte tag) {
  ᨀᨘ (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->top.p);
  api_incr_top(L);
  lua_unlock(L);
  ᨀᨘᨒ novariant(tag);
}


l_sinline Table *gettable (lua_State *L, puur_naa_nka idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  ᨀᨘᨒ hvalue(t);
}


LUA_API puur_naa_nka lua_rawget (lua_State *L, puur_naa_nka idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->top.p - 1), s2v(L->top.p - 1));
  L->top.p--;  /* pop key */
  ᨀᨘᨒ finishrawget(L, tag);
}


LUA_API puur_naa_nka lua_rawgeti (lua_State *L, puur_naa_nka idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->top.p), tag);
  ᨀᨘᨒ finishrawget(L, tag);
}


LUA_API puur_naa_nka lua_rawgetp (lua_State *L, puur_naa_nka idx, const void *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  ᨀᨘᨒ finishrawget(L, luaH_get(t, &k, s2v(L->top.p)));
}


LUA_API void lua_createtable (lua_State *L, puur_naa_nka narray, puur_naa_nka nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->top.p, t);
  api_incr_top(L);
  ᨀᨘ (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API puur_naa_nka lua_getmetatable (lua_State *L, puur_naa_nka objindex) {
  const TValue *obj;
  Table *mt;
  puur_naa_nka res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  switch (ttype(obj)) {
    pak_sa LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      bha_nga;
    pak_sa LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      bha_nga;
    default:
      mt = G(L)->mt[ttype(obj)];
      bha_nga;
  }
  ᨀᨘ (mt != NULL) {
    sethvalue2s(L, L->top.p, mt);
    api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  ᨀᨘᨒ res;
}


LUA_API puur_naa_nka lua_getiuservalue (lua_State *L, puur_naa_nka idx, puur_naa_nka n) {
  TValue *o;
  puur_naa_nka t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ᨀᨘ (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->top.p);
    t = LUA_TNONE;
  }
  ᨀᨘᨁ {
    setobj2s(L, L->top.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->top.p));
  }
  api_incr_top(L);
  lua_unlock(L);
  ᨀᨘᨒ t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
static void auxsetstr (lua_State *L, const TValue *t, const var_na *k) {
  puur_naa_nka hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->top.p - 1), hres, luaH_psetstr);
  ᨀᨘ (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->top.p - 1));
    L->top.p--;  /* pop value */
  }
  ᨀᨘᨁ {
    setsvalue2s(L, L->top.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_top(L);
    luaV_finishset(L, t, s2v(L->top.p - 1), s2v(L->top.p - 2), hres);
    L->top.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API void lua_setglobal (lua_State *L, const var_na *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API void lua_settable (lua_State *L, puur_naa_nka idx) {
  TValue *t;
  puur_naa_nka hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->top.p - 2), s2v(L->top.p - 1), hres, luaH_pset);
  ᨀᨘ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->top.p - 1));
  ᨀᨘᨁ
    luaV_finishset(L, t, s2v(L->top.p - 2), s2v(L->top.p - 1), hres);
  L->top.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API void lua_setfield (lua_State *L, puur_naa_nka idx, const var_na *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API void lua_seti (lua_State *L, puur_naa_nka idx, lua_Integer n) {
  TValue *t;
  puur_naa_nka hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->top.p - 1), hres);
  ᨀᨘ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->top.p - 1));
  ᨀᨘᨁ {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->top.p - 1), hres);
  }
  L->top.p--;  /* pop value */
  lua_unlock(L);
}


static void aux_rawset (lua_State *L, puur_naa_nka idx, TValue *key, puur_naa_nka n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, key, s2v(L->top.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->top.p - 1));
  L->top.p -= n;
  lua_unlock(L);
}


LUA_API void lua_rawset (lua_State *L, puur_naa_nka idx) {
  aux_rawset(L, idx, s2v(L->top.p - 2), 2);
}


LUA_API void lua_rawsetp (lua_State *L, puur_naa_nka idx, const void *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API void lua_rawseti (lua_State *L, puur_naa_nka idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->top.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->top.p - 1));
  L->top.p--;
  lua_unlock(L);
}


LUA_API puur_naa_nka lua_setmetatable (lua_State *L, puur_naa_nka objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  ᨀᨘ (ttisnil(s2v(L->top.p - 1)))
    mt = NULL;
  ᨀᨘᨁ {
    api_check(L, ttistable(s2v(L->top.p - 1)), "table expected");
    mt = hvalue(s2v(L->top.p - 1));
  }
  switch (ttype(obj)) {
    pak_sa LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      ᨀᨘ (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      bha_nga;
    }
    pak_sa LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      ᨀᨘ (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      bha_nga;
    }
    default: {
      G(L)->mt[ttype(obj)] = mt;
      bha_nga;
    }
  }
  L->top.p--;
  lua_unlock(L);
  ᨀᨘᨒ 1;
}


LUA_API puur_naa_nka lua_setiuservalue (lua_State *L, puur_naa_nka idx, puur_naa_nka n) {
  TValue *o;
  puur_naa_nka res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ᨀᨘ (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  ᨀᨘᨁ {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->top.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->top.p - 1));
    res = 1;
  }
  L->top.p--;
  lua_unlock(L);
  ᨀᨘᨒ res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->top.p - L->top.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API void lua_callk (lua_State *L, puur_naa_nka nargs, puur_naa_nka nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == NULL || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->top.p - (nargs+1);
  ᨀᨘ (k != NULL && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  ᨀᨘᨁ  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
sa_nracanaa CallS {  /* data to 'f_call' */
  StkId func;
  puur_naa_nka nresults;
};


static void f_call (lua_State *L, void *ud) {
  sa_nracanaa CallS *c = cast(sa_nracanaa CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API puur_naa_nka lua_pcallk (lua_State *L, puur_naa_nka nargs, puur_naa_nka nresults, puur_naa_nka errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  sa_nracanaa CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == NULL || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  ᨀᨘ (errfunc == 0)
    func = 0;
  ᨀᨘᨁ {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->top.p - (nargs+1);  /* function to be called */
  ᨀᨘ (k == NULL || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  ᨀᨘᨁ {  /* prepare continuation (call is already protected by 'resume') */
    CallInfo *ci = L->ci;
    ci->u.c.k = k;  /* save continuation */
    ci->u.c.ctx = ctx;  /* save context */
    /* save information for error recovery */
    ci->u2.funcidx = cast_int(savestack(L, c.func));
    ci->u.c.old_errfunc = L->errfunc;
    L->errfunc = func;
    setoah(ci, L->allowhook);  /* save value of 'allowhook' */
    ci->callstatus |= CIST_YPCALL;  /* function can do error recovery */
    luaD_call(L, c.func, nresults);  /* do the call */
    ci->callstatus &= ~CIST_YPCALL;
    L->errfunc = ci->u.c.old_errfunc;
    status = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  ᨀᨘᨒ APIstatus(status);
}


LUA_API puur_naa_nka lua_load (lua_State *L, lua_Reader reader, void *data,
                      const var_na *chunkname, const var_na *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  ᨀᨘ (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  ᨀᨘ (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->top.p - 1));  /* get new function */
    ᨀᨘ (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  ᨀᨘᨒ APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API puur_naa_nka lua_dump (lua_State *L, lua_Writer writer, void *data, puur_naa_nka strip) {
  puur_naa_nka status;
  ptrdiff_t otop = savestack(L, L->top.p);  /* original top */
  TValue *f = s2v(L->top.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->top.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  ᨀᨘᨒ status;
}


LUA_API puur_naa_nka lua_status (lua_State *L) {
  ᨀᨘᨒ APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API puur_naa_nka lua_gc (lua_State *L, puur_naa_nka what, ...) {
  va_list argp;
  puur_naa_nka res = 0;
  global_State *g = G(L);
  ᨀᨘ (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    ᨀᨘᨒ -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  switch (what) {
    pak_sa LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      bha_nga;
    }
    pak_sa LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      bha_nga;
    }
    pak_sa LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      bha_nga;
    }
    pak_sa LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      bha_nga;
    }
    pak_sa LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      bha_nga;
    }
    pak_sa LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      puur_naa_nka work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      ᨀᨘ (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      ᨀᨘᨁ ᨀᨘ (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      ᨀᨘᨁ  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (void)0, work = 1);
      ᨀᨘ (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      bha_nga;
    }
    pak_sa LUA_GCISRUNNING: {
      res = gcrunning(g);
      bha_nga;
    }
    pak_sa LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      bha_nga;
    }
    pak_sa LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      bha_nga;
    }
    pak_sa LUA_GCPARAM: {
      puur_naa_nka param = va_arg(argp, puur_naa_nka);
      puur_naa_nka value = va_arg(argp, puur_naa_nka);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      ᨀᨘ (value >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(value));
      bha_nga;
    }
    default: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  ᨀᨘᨒ res;
}



/*
** miscellaneous functions
*/


LUA_API puur_naa_nka lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->top.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  ᨀᨘ (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  ᨀᨘᨁ
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  ᨀᨘᨒ 0;  /* to avoid warnings */
}


LUA_API puur_naa_nka lua_next (lua_State *L, puur_naa_nka idx) {
  Table *t;
  puur_naa_nka more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->top.p - 1);
  ᨀᨘ (more)
    api_incr_top(L);
  ᨀᨘᨁ  /* no more elements */
    L->top.p--;  /* pop key */
  lua_unlock(L);
  ᨀᨘᨒ more;
}


LUA_API void lua_toclose (lua_State *L, puur_naa_nka idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API void lua_concat (lua_State *L, puur_naa_nka n) {
  lua_lock(L);
  ᨀᨘ (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  ᨀᨘᨁ {  /* nothing to concatenate */
    setsvalue2s(L, L->top.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_top(L);
  }
  lua_unlock(L);
}


LUA_API void lua_len (lua_State *L, puur_naa_nka idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->top.p, t);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  lua_lock(L);
  ᨀᨘ (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  ᨀᨘᨒ f;
}


LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


void lua_setwarnf (lua_State *L, lua_WarnFunction f, void *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


void lua_warning (lua_State *L, const var_na *msg, puur_naa_nka tocont) {
  lua_lock(L);
  luaE_warning(L, msg, tocont);
  lua_unlock(L);
}



LUA_API void *lua_newuserdatauv (lua_State *L, size_t size, puur_naa_nka nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->top.p), u);
  api_incr_top(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ᨀᨘᨒ getudatamem(u);
}



static const var_na *aux_upvalue (TValue *fi, puur_naa_nka n, TValue **val,
                                GCObject **owner) {
  switch (ttypetag(fi)) {
    pak_sa LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ᨀᨘ (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        ᨀᨘᨒ NULL;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      ᨀᨘ (owner) *owner = obj2gco(f);
      ᨀᨘᨒ "";
    }
    pak_sa LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      ᨀᨘ (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        ᨀᨘᨒ NULL;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      ᨀᨘ (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      ᨀᨘᨒ (name == NULL) ? "(no name)" : getstr(name);
    }
    default: ᨀᨘᨒ NULL;  /* not a closure */
  }
}


LUA_API const var_na *lua_getupvalue (lua_State *L, puur_naa_nka funcindex, puur_naa_nka n) {
  const var_na *name;
  TValue *val = NULL;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, NULL);
  ᨀᨘ (name) {
    setobj2s(L, L->top.p, val);
    api_incr_top(L);
  }
  lua_unlock(L);
  ᨀᨘᨒ name;
}


LUA_API const var_na *lua_setupvalue (lua_State *L, puur_naa_nka funcindex, puur_naa_nka n) {
  const var_na *name;
  TValue *val = NULL;  /* to avoid warnings */
  GCObject *owner = NULL;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  ᨀᨘ (name) {
    L->top.p--;
    setobj(L, val, s2v(L->top.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  ᨀᨘᨒ name;
}


static UpVal **getupvalref (lua_State *L, puur_naa_nka fidx, puur_naa_nka n, LClosure **pf) {
  static const UpVal *const nullup = NULL;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  ᨀᨘ (pf) *pf = f;
  ᨀᨘ (1 <= n && n <= f->p->sizeupvalues)
    ᨀᨘᨒ &f->upvals[n - 1];  /* get its upvalue pointer */
  ᨀᨘᨁ
    ᨀᨘᨒ (UpVal**)&nullup;
}


LUA_API void *lua_upvalueid (lua_State *L, puur_naa_nka fidx, puur_naa_nka n) {
  TValue *fi = index2value(L, fidx);
  switch (ttypetag(fi)) {
    pak_sa LUA_VLCL: {  /* lua closure */
      ᨀᨘᨒ *getupvalref(L, fidx, n, NULL);
    }
    pak_sa LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ᨀᨘ (1 <= n && n <= f->nupvalues)
        ᨀᨘᨒ &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    pak_sa LUA_VLCF:
      ᨀᨘᨒ NULL;  /* light C functions have no upvalues */
    default: {
      api_check(L, 0, "function expected");
      ᨀᨘᨒ NULL;
    }
  }
}


LUA_API void lua_upvaluejoin (lua_State *L, puur_naa_nka fidx1, puur_naa_nka n1,
                                            puur_naa_nka fidx2, puur_naa_nka n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, NULL);
  api_check(L, *up1 != NULL && *up2 != NULL, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


