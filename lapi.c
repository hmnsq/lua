/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lapi.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "ltm.h"
#അന്തര്ഭാവ "lundump.h"
#അന്തര്ഭാവ "lvm.h"



സ്ഥിരം അക്ഷരം lua_ident[] =
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
സ്ഥിതം TValue *index2value (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  CallInfo *ci = L->ci;
  എങ്കിൽ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->മുകൾ.p - (ci->func.p + 1), "unacceptable index");
    എങ്കിൽ (o >= L->മുകൾ.p) തിരികെ &G(L)->nilvalue;
    അല്ലെങ്കിൽ തിരികെ s2v(o);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->മുകൾ.p - (ci->func.p + 1),
                 "invalid index");
    തിരികെ s2v(L->മുകൾ.p + idx);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (idx == LUA_REGISTRYINDEX)
    തിരികെ &G(L)->l_registry;
  അല്ലെങ്കിൽ {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    എങ്കിൽ (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      തിരികെ (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    അല്ലെങ്കിൽ {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      തിരികെ &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
സ്ഥിതം StkId index2stack (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  CallInfo *ci = L->ci;
  എങ്കിൽ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->മുകൾ.p, "invalid index");
    തിരികെ o;
  }
  അല്ലെങ്കിൽ {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->മുകൾ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    തിരികെ L->മുകൾ.p + idx;
  }
}


LUA_API പൂർണ്ണസംഖ്യ lua_checkstack (lua_State *L, പൂർണ്ണസംഖ്യ n) {
  പൂർണ്ണസംഖ്യ res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  എങ്കിൽ (L->അട്ടി_last.p - L->മുകൾ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  അല്ലെങ്കിൽ  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  എങ്കിൽ (res && ci->മുകൾ.p < L->മുകൾ.p + n)
    ci->മുകൾ.p = L->മുകൾ.p + n;  /* adjust frame top */
  lua_unlock(L);
  തിരികെ res;
}


LUA_API ശൂന്യം lua_xmove (lua_State *from, lua_State *to, പൂർണ്ണസംഖ്യ n) {
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ (from == to) തിരികെ;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->മുകൾ.p - to->മുകൾ.p >= n, "stack overflow");
  from->മുകൾ.p -= n;
  വേണ്ടി (i = 0; i < n; i++) {
    setobjs2s(to, to->മുകൾ.p, from->മുകൾ.p + i);
    to->മുകൾ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  തിരികെ old;
}


LUA_API lua_സംഖ്യ lua_version (lua_State *L) {
  UNUSED(L);
  തിരികെ LUA_VERSION_അക്കം;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API പൂർണ്ണസംഖ്യ lua_absindex (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  തിരികെ (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->മുകൾ.p - L->ci->func.p) + idx;
}


LUA_API പൂർണ്ണസംഖ്യ lua_gettop (lua_State *L) {
  തിരികെ cast_int(L->മുകൾ.p - (L->ci->func.p + 1));
}


LUA_API ശൂന്യം lua_settop (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  എങ്കിൽ (idx >= 0) {
    api_check(L, idx <= ci->മുകൾ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->മുകൾ.p;
    വേണ്ടി (; diff > 0; diff--)
      setnilvalue2s(L->മുകൾ.p++);  /* clear new slots */
  }
  അല്ലെങ്കിൽ {
    api_check(L, -(idx+1) <= (L->മുകൾ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->മുകൾ.p + diff;
  എങ്കിൽ (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->മുകൾ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_closeslot (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
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
സ്ഥിതം ശൂന്യം reverse (lua_State *L, StkId from, StkId to) {
  വേണ്ടി (; from < to; from++, to--) {
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
LUA_API ശൂന്യം lua_rotate (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->മുകൾ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_copy (lua_State *L, പൂർണ്ണസംഖ്യ fromidx, പൂർണ്ണസംഖ്യ toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  എങ്കിൽ (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_pushvalue (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  lua_lock(L);
  setobj2s(L, L->മുകൾ.p, index2value(L, idx));
  api_incr_മുകൾ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API പൂർണ്ണസംഖ്യ lua_type (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API സ്ഥിരം അക്ഷരം *lua_typename (lua_State *L, പൂർണ്ണസംഖ്യ t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  തിരികെ ttypename(t);
}


LUA_API പൂർണ്ണസംഖ്യ lua_iscfunction (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API പൂർണ്ണസംഖ്യ lua_isinteger (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ ttisinteger(o);
}


LUA_API പൂർണ്ണസംഖ്യ lua_isnumber (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  lua_സംഖ്യ n;
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ tonumber(o, &n);
}


LUA_API പൂർണ്ണസംഖ്യ lua_isstring (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ (ttisstring(o) || cvt2str(o));
}


LUA_API പൂർണ്ണസംഖ്യ lua_isuserdata (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API പൂർണ്ണസംഖ്യ lua_rawequal (lua_State *L, പൂർണ്ണസംഖ്യ index1, പൂർണ്ണസംഖ്യ index2) {
  സ്ഥിരം TValue *o1 = index2value(L, index1);
  സ്ഥിരം TValue *o2 = index2value(L, index2);
  തിരികെ (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API ശൂന്യം lua_arith (lua_State *L, പൂർണ്ണസംഖ്യ op) {
  lua_lock(L);
  എങ്കിൽ (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  അല്ലെങ്കിൽ {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->മുകൾ.p, L->മുകൾ.p - 1);
    api_incr_മുകൾ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->മുകൾ.p - 2), s2v(L->മുകൾ.p - 1), L->മുകൾ.p - 2);
  L->മുകൾ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API പൂർണ്ണസംഖ്യ lua_compare (lua_State *L, പൂർണ്ണസംഖ്യ index1, പൂർണ്ണസംഖ്യ index2, പൂർണ്ണസംഖ്യ op) {
  സ്ഥിരം TValue *o1;
  സ്ഥിരം TValue *o2;
  പൂർണ്ണസംഖ്യ i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  എങ്കിൽ (isvalid(L, o1) && isvalid(L, o2)) {
    തിരഞ്ഞെടുപ്പ് (op) {
      അവസ്ഥ LUA_OPEQ: i = luaV_equalobj(L, o1, o2); നിർത്തുക;
      അവസ്ഥ LUA_OPLT: i = luaV_lessthan(L, o1, o2); നിർത്തുക;
      അവസ്ഥ LUA_OPLE: i = luaV_lessequal(L, o1, o2); നിർത്തുക;
      സ്ഥിരസ്ഥിതി: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  തിരികെ i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, പൂർണ്ണസംഖ്യ idx, അക്ഷരം *buff) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  എങ്കിൽ (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    തിരികെ len;
  }
  അല്ലെങ്കിൽ
    തിരികെ 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, സ്ഥിരം അക്ഷരം *s) {
  size_t sz = luaO_str2num(s, s2v(L->മുകൾ.p));
  എങ്കിൽ (sz != 0)
    api_incr_മുകൾ(L);
  തിരികെ sz;
}


LUA_API lua_സംഖ്യ lua_tonumberx (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ *pisnum) {
  lua_സംഖ്യ n = 0;
  സ്ഥിരം TValue *o = index2value(L, idx);
  പൂർണ്ണസംഖ്യ isnum = tonumber(o, &n);
  എങ്കിൽ (pisnum)
    *pisnum = isnum;
  തിരികെ n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ *pisnum) {
  lua_Integer res = 0;
  സ്ഥിരം TValue *o = index2value(L, idx);
  പൂർണ്ണസംഖ്യ isnum = tointeger(o, &res);
  എങ്കിൽ (pisnum)
    *pisnum = isnum;
  തിരികെ res;
}


LUA_API പൂർണ്ണസംഖ്യ lua_toboolean (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ !l_isfalse(o);
}


LUA_API സ്ഥിരം അക്ഷരം *lua_tolstring (lua_State *L, പൂർണ്ണസംഖ്യ idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  എങ്കിൽ (!ttisstring(o)) {
    എങ്കിൽ (!cvt2str(o)) {  /* not convertible? */
      എങ്കിൽ (len != ശൂന്യ) *len = 0;
      lua_unlock(L);
      തിരികെ ശൂന്യ;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  എങ്കിൽ (len != ശൂന്യ)
    തിരികെ getlstr(tsvalue(o), *len);
  അല്ലെങ്കിൽ
    തിരികെ getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരഞ്ഞെടുപ്പ് (ttypetag(o)) {
    അവസ്ഥ LUA_VSHRSTR: തിരികെ cast(lua_Unsigned, tsvalue(o)->shrlen);
    അവസ്ഥ LUA_VLNGSTR: തിരികെ cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    അവസ്ഥ LUA_VUSERDATA: തിരികെ cast(lua_Unsigned, uvalue(o)->len);
    അവസ്ഥ LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      തിരികെ res;
    }
    സ്ഥിരസ്ഥിതി: തിരികെ 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  എങ്കിൽ (ttislcf(o)) തിരികെ fvalue(o);
  അല്ലെങ്കിൽ എങ്കിൽ (ttisCclosure(o))
    തിരികെ clCvalue(o)->f;
  അല്ലെങ്കിൽ തിരികെ ശൂന്യ;  /* not a C function */
}


l_sinline ശൂന്യം *touserdata (സ്ഥിരം TValue *o) {
  തിരഞ്ഞെടുപ്പ് (ttype(o)) {
    അവസ്ഥ LUA_TUSERDATA: തിരികെ getudatamem(uvalue(o));
    അവസ്ഥ LUA_TLIGHTUSERDATA: തിരികെ pvalue(o);
    സ്ഥിരസ്ഥിതി: തിരികെ ശൂന്യ;
  }
}


LUA_API ശൂന്യം *lua_touserdata (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരികെ (!ttisthread(o)) ? ശൂന്യ : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API സ്ഥിരം ശൂന്യം *lua_topointer (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  സ്ഥിരം TValue *o = index2value(L, idx);
  തിരഞ്ഞെടുപ്പ് (ttypetag(o)) {
    അവസ്ഥ LUA_VLCF: തിരികെ cast_voidp(cast_sizet(fvalue(o)));
    അവസ്ഥ LUA_VUSERDATA: അവസ്ഥ LUA_VLIGHTUSERDATA:
      തിരികെ touserdata(o);
    സ്ഥിരസ്ഥിതി: {
      എങ്കിൽ (iscollectable(o))
        തിരികെ gcvalue(o);
      അല്ലെങ്കിൽ
        തിരികെ ശൂന്യ;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API ശൂന്യം lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->മുകൾ.p);
  api_incr_മുകൾ(L);
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_pushnumber (lua_State *L, lua_സംഖ്യ n) {
  lua_lock(L);
  setfltvalue(s2v(L->മുകൾ.p), n);
  api_incr_മുകൾ(L);
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->മുകൾ.p), n);
  api_incr_മുകൾ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API സ്ഥിരം അക്ഷരം *lua_pushlstring (lua_State *L, സ്ഥിരം അക്ഷരം *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->മുകൾ.p, ts);
  api_incr_മുകൾ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  തിരികെ getstr(ts);
}


LUA_API സ്ഥിരം അക്ഷരം *lua_pushexternalstring (lua_State *L,
	        സ്ഥിരം അക്ഷരം *s, size_t len, lua_Alloc falloc, ശൂന്യം *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->മുകൾ.p, ts);
  api_incr_മുകൾ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  തിരികെ getstr(ts);
}


LUA_API സ്ഥിരം അക്ഷരം *lua_pushstring (lua_State *L, സ്ഥിരം അക്ഷരം *s) {
  lua_lock(L);
  എങ്കിൽ (s == ശൂന്യ)
    setnilvalue2s(L->മുകൾ.p);
  അല്ലെങ്കിൽ {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->മുകൾ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_മുകൾ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  തിരികെ s;
}


LUA_API സ്ഥിരം അക്ഷരം *lua_pushvfstring (lua_State *L, സ്ഥിരം അക്ഷരം *fmt,
                                      va_list argp) {
  സ്ഥിരം അക്ഷരം *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  തിരികെ ret;
}


LUA_API സ്ഥിരം അക്ഷരം *lua_pushfstring (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, ...) {
  സ്ഥിരം അക്ഷരം *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  തിരികെ ret;
}


LUA_API ശൂന്യം lua_pushcclosure (lua_State *L, lua_CFunction fn, പൂർണ്ണസംഖ്യ n) {
  lua_lock(L);
  എങ്കിൽ (n == 0) {
    setfvalue(s2v(L->മുകൾ.p), fn);
    api_incr_മുകൾ(L);
  }
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    വേണ്ടി (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->മുകൾ.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->മുകൾ.p -= n;
    setclCvalue(L, s2v(L->മുകൾ.p), cl);
    api_incr_മുകൾ(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_pushboolean (lua_State *L, പൂർണ്ണസംഖ്യ b) {
  lua_lock(L);
  എങ്കിൽ (b)
    setbtvalue(s2v(L->മുകൾ.p));
  അല്ലെങ്കിൽ
    setbfvalue(s2v(L->മുകൾ.p));
  api_incr_മുകൾ(L);
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_pushlightuserdata (lua_State *L, ശൂന്യം *p) {
  lua_lock(L);
  setpvalue(s2v(L->മുകൾ.p), p);
  api_incr_മുകൾ(L);
  lua_unlock(L);
}


LUA_API പൂർണ്ണസംഖ്യ lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->മുകൾ.p), L);
  api_incr_മുകൾ(L);
  lua_unlock(L);
  തിരികെ (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


സ്ഥിതം പൂർണ്ണസംഖ്യ auxgetstr (lua_State *L, സ്ഥിരം TValue *t, സ്ഥിരം അക്ഷരം *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->മുകൾ.p), luaH_getstr, tag);
  എങ്കിൽ (!tagisempty(tag))
    api_incr_മുകൾ(L);
  അല്ലെങ്കിൽ {
    setsvalue2s(L, L->മുകൾ.p, str);
    api_incr_മുകൾ(L);
    tag = luaV_finishget(L, t, s2v(L->മുകൾ.p - 1), L->മുകൾ.p - 1, tag);
  }
  lua_unlock(L);
  തിരികെ novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
സ്ഥിതം ശൂന്യം getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (ശൂന്യം)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API പൂർണ്ണസംഖ്യ lua_getglobal (lua_State *L, സ്ഥിരം അക്ഷരം *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  തിരികെ auxgetstr(L, &gt, name);
}


LUA_API പൂർണ്ണസംഖ്യ lua_gettable (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->മുകൾ.p - 1), s2v(L->മുകൾ.p - 1), luaH_get, tag);
  എങ്കിൽ (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->മുകൾ.p - 1), L->മുകൾ.p - 1, tag);
  lua_unlock(L);
  തിരികെ novariant(tag);
}


LUA_API പൂർണ്ണസംഖ്യ lua_getfield (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം അക്ഷരം *k) {
  lua_lock(L);
  തിരികെ auxgetstr(L, index2value(L, idx), k);
}


LUA_API പൂർണ്ണസംഖ്യ lua_geti (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->മുകൾ.p), tag);
  എങ്കിൽ (tagisempty(tag)) {
    TValue താക്കോൽ;
    setivalue(&താക്കോൽ, n);
    tag = luaV_finishget(L, t, &താക്കോൽ, L->മുകൾ.p, tag);
  }
  api_incr_മുകൾ(L);
  lua_unlock(L);
  തിരികെ novariant(tag);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ finishrawget (lua_State *L, lu_byte tag) {
  എങ്കിൽ (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->മുകൾ.p);
  api_incr_മുകൾ(L);
  lua_unlock(L);
  തിരികെ novariant(tag);
}


l_sinline Table *gettable (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  തിരികെ hvalue(t);
}


LUA_API പൂർണ്ണസംഖ്യ lua_rawget (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->മുകൾ.p - 1), s2v(L->മുകൾ.p - 1));
  L->മുകൾ.p--;  /* pop key */
  തിരികെ finishrawget(L, tag);
}


LUA_API പൂർണ്ണസംഖ്യ lua_rawgeti (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->മുകൾ.p), tag);
  തിരികെ finishrawget(L, tag);
}


LUA_API പൂർണ്ണസംഖ്യ lua_rawgetp (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം ശൂന്യം *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  തിരികെ finishrawget(L, luaH_get(t, &k, s2v(L->മുകൾ.p)));
}


LUA_API ശൂന്യം lua_createtable (lua_State *L, പൂർണ്ണസംഖ്യ narray, പൂർണ്ണസംഖ്യ nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->മുകൾ.p, t);
  api_incr_മുകൾ(L);
  എങ്കിൽ (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API പൂർണ്ണസംഖ്യ lua_getmetatable (lua_State *L, പൂർണ്ണസംഖ്യ objindex) {
  സ്ഥിരം TValue *obj;
  Table *mt;
  പൂർണ്ണസംഖ്യ res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  തിരഞ്ഞെടുപ്പ് (ttype(obj)) {
    അവസ്ഥ LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      നിർത്തുക;
    അവസ്ഥ LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      mt = G(L)->mt[ttype(obj)];
      നിർത്തുക;
  }
  എങ്കിൽ (mt != ശൂന്യ) {
    sethvalue2s(L, L->മുകൾ.p, mt);
    api_incr_മുകൾ(L);
    res = 1;
  }
  lua_unlock(L);
  തിരികെ res;
}


LUA_API പൂർണ്ണസംഖ്യ lua_getiuservalue (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ n) {
  TValue *o;
  പൂർണ്ണസംഖ്യ t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  എങ്കിൽ (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->മുകൾ.p);
    t = LUA_TNONE;
  }
  അല്ലെങ്കിൽ {
    setobj2s(L, L->മുകൾ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->മുകൾ.p));
  }
  api_incr_മുകൾ(L);
  lua_unlock(L);
  തിരികെ t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
സ്ഥിതം ശൂന്യം auxsetstr (lua_State *L, സ്ഥിരം TValue *t, സ്ഥിരം അക്ഷരം *k) {
  പൂർണ്ണസംഖ്യ hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->മുകൾ.p - 1), hres, luaH_psetstr);
  എങ്കിൽ (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->മുകൾ.p - 1));
    L->മുകൾ.p--;  /* pop value */
  }
  അല്ലെങ്കിൽ {
    setsvalue2s(L, L->മുകൾ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_മുകൾ(L);
    luaV_finishset(L, t, s2v(L->മുകൾ.p - 1), s2v(L->മുകൾ.p - 2), hres);
    L->മുകൾ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API ശൂന്യം lua_setglobal (lua_State *L, സ്ഥിരം അക്ഷരം *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API ശൂന്യം lua_settable (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  TValue *t;
  പൂർണ്ണസംഖ്യ hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->മുകൾ.p - 2), s2v(L->മുകൾ.p - 1), hres, luaH_pset);
  എങ്കിൽ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->മുകൾ.p - 1));
  അല്ലെങ്കിൽ
    luaV_finishset(L, t, s2v(L->മുകൾ.p - 2), s2v(L->മുകൾ.p - 1), hres);
  L->മുകൾ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_setfield (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം അക്ഷരം *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API ശൂന്യം lua_seti (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n) {
  TValue *t;
  പൂർണ്ണസംഖ്യ hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->മുകൾ.p - 1), hres);
  എങ്കിൽ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->മുകൾ.p - 1));
  അല്ലെങ്കിൽ {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->മുകൾ.p - 1), hres);
  }
  L->മുകൾ.p--;  /* pop value */
  lua_unlock(L);
}


സ്ഥിതം ശൂന്യം aux_rawset (lua_State *L, പൂർണ്ണസംഖ്യ idx, TValue *താക്കോൽ, പൂർണ്ണസംഖ്യ n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, താക്കോൽ, s2v(L->മുകൾ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->മുകൾ.p - 1));
  L->മുകൾ.p -= n;
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_rawset (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  aux_rawset(L, idx, s2v(L->മുകൾ.p - 2), 2);
}


LUA_API ശൂന്യം lua_rawsetp (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം ശൂന്യം *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API ശൂന്യം lua_rawseti (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->മുകൾ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->മുകൾ.p - 1));
  L->മുകൾ.p--;
  lua_unlock(L);
}


LUA_API പൂർണ്ണസംഖ്യ lua_setmetatable (lua_State *L, പൂർണ്ണസംഖ്യ objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  എങ്കിൽ (ttisnil(s2v(L->മുകൾ.p - 1)))
    mt = ശൂന്യ;
  അല്ലെങ്കിൽ {
    api_check(L, ttistable(s2v(L->മുകൾ.p - 1)), "table expected");
    mt = hvalue(s2v(L->മുകൾ.p - 1));
  }
  തിരഞ്ഞെടുപ്പ് (ttype(obj)) {
    അവസ്ഥ LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      എങ്കിൽ (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      നിർത്തുക;
    }
    അവസ്ഥ LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      എങ്കിൽ (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      G(L)->mt[ttype(obj)] = mt;
      നിർത്തുക;
    }
  }
  L->മുകൾ.p--;
  lua_unlock(L);
  തിരികെ 1;
}


LUA_API പൂർണ്ണസംഖ്യ lua_setiuservalue (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ n) {
  TValue *o;
  പൂർണ്ണസംഖ്യ res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  എങ്കിൽ (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  അല്ലെങ്കിൽ {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->മുകൾ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->മുകൾ.p - 1));
    res = 1;
  }
  L->മുകൾ.p--;
  lua_unlock(L);
  തിരികെ res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->മുകൾ.p - L->മുകൾ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API ശൂന്യം lua_callk (lua_State *L, പൂർണ്ണസംഖ്യ nargs, പൂർണ്ണസംഖ്യ nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == ശൂന്യ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->മുകൾ.p - (nargs+1);
  എങ്കിൽ (k != ശൂന്യ && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  അല്ലെങ്കിൽ  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
ഘടന CallS {  /* data to 'f_call' */
  StkId func;
  പൂർണ്ണസംഖ്യ nresults;
};


സ്ഥിതം ശൂന്യം f_call (lua_State *L, ശൂന്യം *ud) {
  ഘടന CallS *c = cast(ഘടന CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API പൂർണ്ണസംഖ്യ lua_pcallk (lua_State *L, പൂർണ്ണസംഖ്യ nargs, പൂർണ്ണസംഖ്യ nresults, പൂർണ്ണസംഖ്യ errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  ഘടന CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == ശൂന്യ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  എങ്കിൽ (errfunc == 0)
    func = 0;
  അല്ലെങ്കിൽ {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->മുകൾ.p - (nargs+1);  /* function to be called */
  എങ്കിൽ (k == ശൂന്യ || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  അല്ലെങ്കിൽ {  /* prepare continuation (call is already protected by 'resume') */
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
  തിരികെ APIstatus(status);
}


LUA_API പൂർണ്ണസംഖ്യ lua_load (lua_State *L, lua_Reader reader, ശൂന്യം *data,
                      സ്ഥിരം അക്ഷരം *chunkname, സ്ഥിരം അക്ഷരം *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  എങ്കിൽ (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  എങ്കിൽ (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->മുകൾ.p - 1));  /* get new function */
    എങ്കിൽ (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  തിരികെ APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API പൂർണ്ണസംഖ്യ lua_dump (lua_State *L, lua_Writer writer, ശൂന്യം *data, പൂർണ്ണസംഖ്യ strip) {
  പൂർണ്ണസംഖ്യ status;
  ptrdiff_t otop = savestack(L, L->മുകൾ.p);  /* original top */
  TValue *f = s2v(L->മുകൾ.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->മുകൾ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  തിരികെ status;
}


LUA_API പൂർണ്ണസംഖ്യ lua_status (lua_State *L) {
  തിരികെ APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API പൂർണ്ണസംഖ്യ lua_gc (lua_State *L, പൂർണ്ണസംഖ്യ what, ...) {
  va_list argp;
  പൂർണ്ണസംഖ്യ res = 0;
  global_State *g = G(L);
  എങ്കിൽ (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    തിരികെ -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  തിരഞ്ഞെടുപ്പ് (what) {
    അവസ്ഥ LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      പൂർണ്ണസംഖ്യ work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      എങ്കിൽ (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      അല്ലെങ്കിൽ എങ്കിൽ (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      അല്ലെങ്കിൽ  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (ശൂന്യം)0, work = 1);
      എങ്കിൽ (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCISRUNNING: {
      res = gcrunning(g);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_GCPARAM: {
      പൂർണ്ണസംഖ്യ param = va_arg(argp, പൂർണ്ണസംഖ്യ);
      പൂർണ്ണസംഖ്യ മൂല്യം = va_arg(argp, പൂർണ്ണസംഖ്യ);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      എങ്കിൽ (മൂല്യം >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(മൂല്യം));
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  തിരികെ res;
}



/*
** miscellaneous functions
*/


LUA_API പൂർണ്ണസംഖ്യ lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->മുകൾ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  എങ്കിൽ (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  അല്ലെങ്കിൽ
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  തിരികെ 0;  /* to avoid warnings */
}


LUA_API പൂർണ്ണസംഖ്യ lua_next (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  Table *t;
  പൂർണ്ണസംഖ്യ more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->മുകൾ.p - 1);
  എങ്കിൽ (more)
    api_incr_മുകൾ(L);
  അല്ലെങ്കിൽ  /* no more elements */
    L->മുകൾ.p--;  /* pop key */
  lua_unlock(L);
  തിരികെ more;
}


LUA_API ശൂന്യം lua_toclose (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_concat (lua_State *L, പൂർണ്ണസംഖ്യ n) {
  lua_lock(L);
  എങ്കിൽ (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  അല്ലെങ്കിൽ {  /* nothing to concatenate */
    setsvalue2s(L, L->മുകൾ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_മുകൾ(L);
  }
  lua_unlock(L);
}


LUA_API ശൂന്യം lua_len (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->മുകൾ.p, t);
  api_incr_മുകൾ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, ശൂന്യം **ud) {
  lua_Alloc f;
  lua_lock(L);
  എങ്കിൽ (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  തിരികെ f;
}


LUA_API ശൂന്യം lua_setallocf (lua_State *L, lua_Alloc f, ശൂന്യം *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


ശൂന്യം lua_setwarnf (lua_State *L, lua_WarnFunction f, ശൂന്യം *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


ശൂന്യം lua_warning (lua_State *L, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ tocont) {
  lua_lock(L);
  luaE_warning(L, സന്ദേശം, tocont);
  lua_unlock(L);
}



LUA_API ശൂന്യം *lua_newuserdatauv (lua_State *L, size_t size, പൂർണ്ണസംഖ്യ nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->മുകൾ.p), u);
  api_incr_മുകൾ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  തിരികെ getudatamem(u);
}



സ്ഥിതം സ്ഥിരം അക്ഷരം *aux_upvalue (TValue *fi, പൂർണ്ണസംഖ്യ n, TValue **val,
                                GCObject **owner) {
  തിരഞ്ഞെടുപ്പ് (ttypetag(fi)) {
    അവസ്ഥ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      എങ്കിൽ (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        തിരികെ ശൂന്യ;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      എങ്കിൽ (owner) *owner = obj2gco(f);
      തിരികെ "";
    }
    അവസ്ഥ LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      എങ്കിൽ (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        തിരികെ ശൂന്യ;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      എങ്കിൽ (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      തിരികെ (name == ശൂന്യ) ? "(no name)" : getstr(name);
    }
    സ്ഥിരസ്ഥിതി: തിരികെ ശൂന്യ;  /* not a closure */
  }
}


LUA_API സ്ഥിരം അക്ഷരം *lua_getupvalue (lua_State *L, പൂർണ്ണസംഖ്യ funcindex, പൂർണ്ണസംഖ്യ n) {
  സ്ഥിരം അക്ഷരം *name;
  TValue *val = ശൂന്യ;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ശൂന്യ);
  എങ്കിൽ (name) {
    setobj2s(L, L->മുകൾ.p, val);
    api_incr_മുകൾ(L);
  }
  lua_unlock(L);
  തിരികെ name;
}


LUA_API സ്ഥിരം അക്ഷരം *lua_setupvalue (lua_State *L, പൂർണ്ണസംഖ്യ funcindex, പൂർണ്ണസംഖ്യ n) {
  സ്ഥിരം അക്ഷരം *name;
  TValue *val = ശൂന്യ;  /* to avoid warnings */
  GCObject *owner = ശൂന്യ;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  എങ്കിൽ (name) {
    L->മുകൾ.p--;
    setobj(L, val, s2v(L->മുകൾ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  തിരികെ name;
}


സ്ഥിതം UpVal **getupvalref (lua_State *L, പൂർണ്ണസംഖ്യ fidx, പൂർണ്ണസംഖ്യ n, LClosure **pf) {
  സ്ഥിതം സ്ഥിരം UpVal *സ്ഥിരം nullup = ശൂന്യ;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  എങ്കിൽ (pf) *pf = f;
  എങ്കിൽ (1 <= n && n <= f->p->sizeupvalues)
    തിരികെ &f->upvals[n - 1];  /* get its upvalue pointer */
  അല്ലെങ്കിൽ
    തിരികെ (UpVal**)&nullup;
}


LUA_API ശൂന്യം *lua_upvalueid (lua_State *L, പൂർണ്ണസംഖ്യ fidx, പൂർണ്ണസംഖ്യ n) {
  TValue *fi = index2value(L, fidx);
  തിരഞ്ഞെടുപ്പ് (ttypetag(fi)) {
    അവസ്ഥ LUA_VLCL: {  /* lua closure */
      തിരികെ *getupvalref(L, fidx, n, ശൂന്യ);
    }
    അവസ്ഥ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      എങ്കിൽ (1 <= n && n <= f->nupvalues)
        തിരികെ &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    അവസ്ഥ LUA_VLCF:
      തിരികെ ശൂന്യ;  /* light C functions have no upvalues */
    സ്ഥിരസ്ഥിതി: {
      api_check(L, 0, "function expected");
      തിരികെ ശൂന്യ;
    }
  }
}


LUA_API ശൂന്യം lua_upvaluejoin (lua_State *L, പൂർണ്ണസംഖ്യ fidx1, പൂർണ്ണസംഖ്യ n1,
                                            പൂർണ്ണസംഖ്യ fidx2, പൂർണ്ണസംഖ്യ n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, ശൂന്യ);
  api_check(L, *up1 != ശൂന്യ && *up2 != ശൂന്യ, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


