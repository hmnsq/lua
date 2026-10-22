/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lapi_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lundump.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"



ਅਚਲ ਅੱਖਰ lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isvalid(L, o)	((o) != &G(L)->nilvalue)


/* test for pseudo index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isupvalue(i)		((i) < LUA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
ਸਥਿਰ TValue *index2value (lua_State *L, ਪੂਰਨ idx) {
  CallInfo *ci = L->ci;
  ਜੇ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->ਸਿਖਰ.p - (ci->func.p + 1), "unacceptable index");
    ਜੇ (o >= L->ਸਿਖਰ.p) ਵਾਪਸ &G(L)->nilvalue;
    ਨਹੀਂਤਾਂ ਵਾਪਸ s2v(o);
  }
  ਨਹੀਂਤਾਂ ਜੇ (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->ਸਿਖਰ.p - (ci->func.p + 1),
                 "invalid index");
    ਵਾਪਸ s2v(L->ਸਿਖਰ.p + idx);
  }
  ਨਹੀਂਤਾਂ ਜੇ (idx == LUA_REGISTRYINDEX)
    ਵਾਪਸ &G(L)->l_registry;
  ਨਹੀਂਤਾਂ {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    ਜੇ (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      ਵਾਪਸ (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    ਨਹੀਂਤਾਂ {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      ਵਾਪਸ &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
ਸਥਿਰ StkId index2stack (lua_State *L, ਪੂਰਨ idx) {
  CallInfo *ci = L->ci;
  ਜੇ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->ਸਿਖਰ.p, "invalid index");
    ਵਾਪਸ o;
  }
  ਨਹੀਂਤਾਂ {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->ਸਿਖਰ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    ਵਾਪਸ L->ਸਿਖਰ.p + idx;
  }
}


LUA_API ਪੂਰਨ lua_checkstack (lua_State *L, ਪੂਰਨ n) {
  ਪੂਰਨ res;
  CallInfo *ci;
  lua_ਤਾਲਾ(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  ਜੇ (L->ਢੇਰ_ਆਖਰੀ.p - L->ਸਿਖਰ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  ਨਹੀਂਤਾਂ  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  ਜੇ (res && ci->ਸਿਖਰ.p < L->ਸਿਖਰ.p + n)
    ci->ਸਿਖਰ.p = L->ਸਿਖਰ.p + n;  /* adjust frame top */
  lua_unlock(L);
  ਵਾਪਸ res;
}


LUA_API ਖਾਲੀ lua_xmove (lua_State *from, lua_State *ਮੰਜ਼ਿਲ, ਪੂਰਨ n) {
  ਪੂਰਨ i;
  ਜੇ (from == ਮੰਜ਼ਿਲ) ਵਾਪਸ;
  lua_ਤਾਲਾ(ਮੰਜ਼ਿਲ);
  api_checkpop(from, n);
  api_check(from, G(from) == G(ਮੰਜ਼ਿਲ), "moving among independent states");
  api_check(from, ਮੰਜ਼ਿਲ->ci->ਸਿਖਰ.p - ਮੰਜ਼ਿਲ->ਸਿਖਰ.p >= n, "stack overflow");
  from->ਸਿਖਰ.p -= n;
  ਲਈ (i = 0; i < n; i++) {
    setobjs2s(ਮੰਜ਼ਿਲ, ਮੰਜ਼ਿਲ->ਸਿਖਰ.p, from->ਸਿਖਰ.p + i);
    ਮੰਜ਼ਿਲ->ਸਿਖਰ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(ਮੰਜ਼ਿਲ);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_ਤਾਲਾ(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  ਵਾਪਸ old;
}


LUA_API lua_ਸੰਖਿਆ lua_ਸੰਸਕਰਣ (lua_State *L) {
  ਅਣਵਰਤਿਆ(L);
  ਵਾਪਸ LUA_ਸੰਸਕਰਣ_ਅੰਕ;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API ਪੂਰਨ lua_absindex (lua_State *L, ਪੂਰਨ idx) {
  ਵਾਪਸ (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->ਸਿਖਰ.p - L->ci->func.p) + idx;
}


LUA_API ਪੂਰਨ lua_gettop (lua_State *L) {
  ਵਾਪਸ cast_int(L->ਸਿਖਰ.p - (L->ci->func.p + 1));
}


LUA_API ਖਾਲੀ lua_settop (lua_State *L, ਪੂਰਨ idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_ਤਾਲਾ(L);
  ci = L->ci;
  func = ci->func.p;
  ਜੇ (idx >= 0) {
    api_check(L, idx <= ci->ਸਿਖਰ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->ਸਿਖਰ.p;
    ਲਈ (; diff > 0; diff--)
      setnilvalue2s(L->ਸਿਖਰ.p++);  /* clear new slots */
  }
  ਨਹੀਂਤਾਂ {
    api_check(L, -(idx+1) <= (L->ਸਿਖਰ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->ਸਿਖਰ.p + diff;
  ਜੇ (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->ਸਿਖਰ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_closeslot (lua_State *L, ਪੂਰਨ idx) {
  StkId level;
  lua_ਤਾਲਾ(L);
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
ਸਥਿਰ ਖਾਲੀ ਉਲਟ (lua_State *L, StkId from, StkId ਮੰਜ਼ਿਲ) {
  ਲਈ (; from < ਮੰਜ਼ਿਲ; from++, ਮੰਜ਼ਿਲ--) {
    TValue ਆਰਜ਼ੀ;
    setobj(L, &ਆਰਜ਼ੀ, s2v(from));
    setobjs2s(L, from, ਮੰਜ਼ਿਲ);
    setobj2s(L, ਮੰਜ਼ਿਲ, &ਆਰਜ਼ੀ);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API ਖਾਲੀ lua_rotate (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ n) {
  StkId p, t, m;
  lua_ਤਾਲਾ(L);
  t = L->ਸਿਖਰ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  ਉਲਟ(L, p, m);  /* reverse the prefix with length 'n' */
  ਉਲਟ(L, m + 1, t);  /* reverse the suffix */
  ਉਲਟ(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_copy (lua_State *L, ਪੂਰਨ fromidx, ਪੂਰਨ toidx) {
  TValue *fr, *ਮੰਜ਼ਿਲ;
  lua_ਤਾਲਾ(L);
  fr = index2value(L, fromidx);
  ਮੰਜ਼ਿਲ = index2value(L, toidx);
  api_check(L, isvalid(L, ਮੰਜ਼ਿਲ), "invalid index");
  setobj(L, ਮੰਜ਼ਿਲ, fr);
  ਜੇ (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_pushvalue (lua_State *L, ਪੂਰਨ idx) {
  lua_ਤਾਲਾ(L);
  setobj2s(L, L->ਸਿਖਰ.p, index2value(L, idx));
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API ਪੂਰਨ lua_type (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_typename (lua_State *L, ਪੂਰਨ t) {
  ਅਣਵਰਤਿਆ(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  ਵਾਪਸ ttypename(t);
}


LUA_API ਪੂਰਨ lua_iscfunction (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API ਪੂਰਨ lua_isinteger (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ ttisinteger(o);
}


LUA_API ਪੂਰਨ lua_isnumber (lua_State *L, ਪੂਰਨ idx) {
  lua_ਸੰਖਿਆ n;
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ tonumber(o, &n);
}


LUA_API ਪੂਰਨ lua_isstring (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ (ttisstring(o) || cvt2str(o));
}


LUA_API ਪੂਰਨ lua_isuserdata (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API ਪੂਰਨ lua_rawequal (lua_State *L, ਪੂਰਨ index1, ਪੂਰਨ index2) {
  ਅਚਲ TValue *o1 = index2value(L, index1);
  ਅਚਲ TValue *o2 = index2value(L, index2);
  ਵਾਪਸ (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API ਖਾਲੀ lua_arith (lua_State *L, ਪੂਰਨ op) {
  lua_ਤਾਲਾ(L);
  ਜੇ (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  ਨਹੀਂਤਾਂ {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->ਸਿਖਰ.p, L->ਸਿਖਰ.p - 1);
    api_incr_ਸਿਖਰ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->ਸਿਖਰ.p - 2), s2v(L->ਸਿਖਰ.p - 1), L->ਸਿਖਰ.p - 2);
  L->ਸਿਖਰ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API ਪੂਰਨ lua_compare (lua_State *L, ਪੂਰਨ index1, ਪੂਰਨ index2, ਪੂਰਨ op) {
  ਅਚਲ TValue *o1;
  ਅਚਲ TValue *o2;
  ਪੂਰਨ i = 0;
  lua_ਤਾਲਾ(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  ਜੇ (isvalid(L, o1) && isvalid(L, o2)) {
    ਚੋਣ (op) {
      ਹਾਲਤ LUA_OPEQ: i = luaV_equalobj(L, o1, o2); ਰੋਕੋ;
      ਹਾਲਤ LUA_OPLT: i = luaV_lessthan(L, o1, o2); ਰੋਕੋ;
      ਹਾਲਤ LUA_OPLE: i = luaV_lessequal(L, o1, o2); ਰੋਕੋ;
      ਮੂਲ: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  ਵਾਪਸ i;
}


LUA_API ਬਿਨਾਨਿਸ਼ਾਨੀ lua_numbertocstring (lua_State *L, ਪੂਰਨ idx, ਅੱਖਰ *buff) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਜੇ (ttisnumber(o)) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    ਵਾਪਸ len;
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ਅਚਲ ਅੱਖਰ *s) {
  size_t sz = luaO_str2num(s, s2v(L->ਸਿਖਰ.p));
  ਜੇ (sz != 0)
    api_incr_ਸਿਖਰ(L);
  ਵਾਪਸ sz;
}


LUA_API lua_ਸੰਖਿਆ lua_tonumberx (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ *pisnum) {
  lua_ਸੰਖਿਆ n = 0;
  ਅਚਲ TValue *o = index2value(L, idx);
  ਪੂਰਨ isnum = tonumber(o, &n);
  ਜੇ (pisnum)
    *pisnum = isnum;
  ਵਾਪਸ n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ *pisnum) {
  lua_Integer res = 0;
  ਅਚਲ TValue *o = index2value(L, idx);
  ਪੂਰਨ isnum = tointeger(o, &res);
  ਜੇ (pisnum)
    *pisnum = isnum;
  ਵਾਪਸ res;
}


LUA_API ਪੂਰਨ lua_toboolean (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ !l_isfalse(o);
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_tolstring (lua_State *L, ਪੂਰਨ idx, size_t *len) {
  TValue *o;
  lua_ਤਾਲਾ(L);
  o = index2value(L, idx);
  ਜੇ (!ttisstring(o)) {
    ਜੇ (!cvt2str(o)) {  /* not convertible? */
      ਜੇ (len != ਨਿਰਾ) *len = 0;
      lua_unlock(L);
      ਵਾਪਸ ਨਿਰਾ;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  ਜੇ (len != ਨਿਰਾ)
    ਵਾਪਸ getlstr(tsvalue(o), *len);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਚੋਣ (ttypetag(o)) {
    ਹਾਲਤ LUA_VSHRSTR: ਵਾਪਸ cast(lua_Unsigned, tsvalue(o)->shrlen);
    ਹਾਲਤ LUA_VLNGSTR: ਵਾਪਸ cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    ਹਾਲਤ LUA_VUSERDATA: ਵਾਪਸ cast(lua_Unsigned, uvalue(o)->len);
    ਹਾਲਤ LUA_VTABLE: {
      lua_Unsigned res;
      lua_ਤਾਲਾ(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      ਵਾਪਸ res;
    }
    ਮੂਲ: ਵਾਪਸ 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਜੇ (ttislcf(o)) ਵਾਪਸ fvalue(o);
  ਨਹੀਂਤਾਂ ਜੇ (ttisCclosure(o))
    ਵਾਪਸ clCvalue(o)->f;
  ਨਹੀਂਤਾਂ ਵਾਪਸ ਨਿਰਾ;  /* not a C function */
}


l_sinline ਖਾਲੀ *touserdata (ਅਚਲ TValue *o) {
  ਚੋਣ (ttype(o)) {
    ਹਾਲਤ LUA_TUSERDATA: ਵਾਪਸ getudatamem(uvalue(o));
    ਹਾਲਤ LUA_TLIGHTUSERDATA: ਵਾਪਸ pvalue(o);
    ਮੂਲ: ਵਾਪਸ ਨਿਰਾ;
  }
}


LUA_API ਖਾਲੀ *lua_touserdata (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਵਾਪਸ (!ttisthread(o)) ? ਨਿਰਾ : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ਅਚਲ ਖਾਲੀ *lua_topointer (lua_State *L, ਪੂਰਨ idx) {
  ਅਚਲ TValue *o = index2value(L, idx);
  ਚੋਣ (ttypetag(o)) {
    ਹਾਲਤ LUA_VLCF: ਵਾਪਸ cast_voidp(cast_sizet(fvalue(o)));
    ਹਾਲਤ LUA_VUSERDATA: ਹਾਲਤ LUA_VLIGHTUSERDATA:
      ਵਾਪਸ touserdata(o);
    ਮੂਲ: {
      ਜੇ (iscollectable(o))
        ਵਾਪਸ gcvalue(o);
      ਨਹੀਂਤਾਂ
        ਵਾਪਸ ਨਿਰਾ;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API ਖਾਲੀ lua_pushnil (lua_State *L) {
  lua_ਤਾਲਾ(L);
  setnilvalue2s(L->ਸਿਖਰ.p);
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_pushnumber (lua_State *L, lua_ਸੰਖਿਆ n) {
  lua_ਤਾਲਾ(L);
  setfltvalue(s2v(L->ਸਿਖਰ.p), n);
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_ਤਾਲਾ(L);
  setivalue(s2v(L->ਸਿਖਰ.p), n);
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ਅਚਲ ਅੱਖਰ *lua_pushlstring (lua_State *L, ਅਚਲ ਅੱਖਰ *s, size_t len) {
  TString *ts;
  lua_ਤਾਲਾ(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->ਸਿਖਰ.p, ts);
  api_incr_ਸਿਖਰ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ਵਾਪਸ getstr(ts);
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_pushexternalstring (lua_State *L,
	        ਅਚਲ ਅੱਖਰ *s, size_t len, lua_Alloc falloc, ਖਾਲੀ *ud) {
  TString *ts;
  lua_ਤਾਲਾ(L);
  api_check(L, len <= MAX_ਮਾਪ, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->ਸਿਖਰ.p, ts);
  api_incr_ਸਿਖਰ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ਵਾਪਸ getstr(ts);
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_pushstring (lua_State *L, ਅਚਲ ਅੱਖਰ *s) {
  lua_ਤਾਲਾ(L);
  ਜੇ (s == ਨਿਰਾ)
    setnilvalue2s(L->ਸਿਖਰ.p);
  ਨਹੀਂਤਾਂ {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->ਸਿਖਰ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_ਸਿਖਰ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ਵਾਪਸ s;
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_pushvfstring (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt,
                                      va_list argp) {
  ਅਚਲ ਅੱਖਰ *ret;
  lua_ਤਾਲਾ(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  ਵਾਪਸ ret;
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_pushfstring (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...) {
  ਅਚਲ ਅੱਖਰ *ret;
  va_list argp;
  lua_ਤਾਲਾ(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  ਵਾਪਸ ret;
}


LUA_API ਖਾਲੀ lua_pushcclosure (lua_State *L, lua_CFunction fn, ਪੂਰਨ n) {
  lua_ਤਾਲਾ(L);
  ਜੇ (n == 0) {
    setfvalue(s2v(L->ਸਿਖਰ.p), fn);
    api_incr_ਸਿਖਰ(L);
  }
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    ਲਈ (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->ਸਿਖਰ.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->ਸਿਖਰ.p -= n;
    setclCvalue(L, s2v(L->ਸਿਖਰ.p), cl);
    api_incr_ਸਿਖਰ(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_pushboolean (lua_State *L, ਪੂਰਨ b) {
  lua_ਤਾਲਾ(L);
  ਜੇ (b)
    setbtvalue(s2v(L->ਸਿਖਰ.p));
  ਨਹੀਂਤਾਂ
    setbfvalue(s2v(L->ਸਿਖਰ.p));
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_pushlightuserdata (lua_State *L, ਖਾਲੀ *p) {
  lua_ਤਾਲਾ(L);
  setpvalue(s2v(L->ਸਿਖਰ.p), p);
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
}


LUA_API ਪੂਰਨ lua_pushthread (lua_State *L) {
  lua_ਤਾਲਾ(L);
  setthvalue(L, s2v(L->ਸਿਖਰ.p), L);
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
  ਵਾਪਸ (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


ਸਥਿਰ ਪੂਰਨ auxgetstr (lua_State *L, ਅਚਲ TValue *t, ਅਚਲ ਅੱਖਰ *k) {
  lu_byte ਟੈਗ;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->ਸਿਖਰ.p), luaH_getstr, ਟੈਗ);
  ਜੇ (!tagisempty(ਟੈਗ))
    api_incr_ਸਿਖਰ(L);
  ਨਹੀਂਤਾਂ {
    setsvalue2s(L, L->ਸਿਖਰ.p, str);
    api_incr_ਸਿਖਰ(L);
    ਟੈਗ = luaV_finishget(L, t, s2v(L->ਸਿਖਰ.p - 1), L->ਸਿਖਰ.p - 1, ਟੈਗ);
  }
  lua_unlock(L);
  ਵਾਪਸ novariant(ਟੈਗ);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
ਸਥਿਰ ਖਾਲੀ get_Global_ਸਾਰਣੀ (lua_State *L, TValue *gt) {
  ਸਾਰਣੀ *registry = hvalue(&G(L)->l_registry);
  lu_byte ਟੈਗ = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (ਖਾਲੀ)ਟੈਗ;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(ਟੈਗ) == LUA_TTABLE, "global table must exist");
}


LUA_API ਪੂਰਨ lua_getglobal (lua_State *L, ਅਚਲ ਅੱਖਰ *name) {
  TValue gt;
  lua_ਤਾਲਾ(L);
  get_Global_ਸਾਰਣੀ(L, &gt);
  ਵਾਪਸ auxgetstr(L, &gt, name);
}


LUA_API ਪੂਰਨ lua_gettable (lua_State *L, ਪੂਰਨ idx) {
  lu_byte ਟੈਗ;
  TValue *t;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->ਸਿਖਰ.p - 1), s2v(L->ਸਿਖਰ.p - 1), luaH_get, ਟੈਗ);
  ਜੇ (tagisempty(ਟੈਗ))
    ਟੈਗ = luaV_finishget(L, t, s2v(L->ਸਿਖਰ.p - 1), L->ਸਿਖਰ.p - 1, ਟੈਗ);
  lua_unlock(L);
  ਵਾਪਸ novariant(ਟੈਗ);
}


LUA_API ਪੂਰਨ lua_getfield (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *k) {
  lua_ਤਾਲਾ(L);
  ਵਾਪਸ auxgetstr(L, index2value(L, idx), k);
}


LUA_API ਪੂਰਨ lua_geti (lua_State *L, ਪੂਰਨ idx, lua_Integer n) {
  TValue *t;
  lu_byte ਟੈਗ;
  lua_ਤਾਲਾ(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->ਸਿਖਰ.p), ਟੈਗ);
  ਜੇ (tagisempty(ਟੈਗ)) {
    TValue ਕੁੰਜੀ;
    setivalue(&ਕੁੰਜੀ, n);
    ਟੈਗ = luaV_finishget(L, t, &ਕੁੰਜੀ, L->ਸਿਖਰ.p, ਟੈਗ);
  }
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
  ਵਾਪਸ novariant(ਟੈਗ);
}


ਸਥਿਰ ਪੂਰਨ finishrawget (lua_State *L, lu_byte ਟੈਗ) {
  ਜੇ (tagisempty(ਟੈਗ))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->ਸਿਖਰ.p);
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
  ਵਾਪਸ novariant(ਟੈਗ);
}


l_sinline ਸਾਰਣੀ *gettable (lua_State *L, ਪੂਰਨ idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  ਵਾਪਸ hvalue(t);
}


LUA_API ਪੂਰਨ lua_rawget (lua_State *L, ਪੂਰਨ idx) {
  ਸਾਰਣੀ *t;
  lu_byte ਟੈਗ;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  ਟੈਗ = luaH_get(t, s2v(L->ਸਿਖਰ.p - 1), s2v(L->ਸਿਖਰ.p - 1));
  L->ਸਿਖਰ.p--;  /* pop key */
  ਵਾਪਸ finishrawget(L, ਟੈਗ);
}


LUA_API ਪੂਰਨ lua_rawgeti (lua_State *L, ਪੂਰਨ idx, lua_Integer n) {
  ਸਾਰਣੀ *t;
  lu_byte ਟੈਗ;
  lua_ਤਾਲਾ(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->ਸਿਖਰ.p), ਟੈਗ);
  ਵਾਪਸ finishrawget(L, ਟੈਗ);
}


LUA_API ਪੂਰਨ lua_rawgetp (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਖਾਲੀ *p) {
  ਸਾਰਣੀ *t;
  TValue k;
  lua_ਤਾਲਾ(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  ਵਾਪਸ finishrawget(L, luaH_get(t, &k, s2v(L->ਸਿਖਰ.p)));
}


LUA_API ਖਾਲੀ lua_createtable (lua_State *L, ਪੂਰਨ narray, ਪੂਰਨ nrec) {
  ਸਾਰਣੀ *t;
  lua_ਤਾਲਾ(L);
  t = luaH_new(L);
  sethvalue2s(L, L->ਸਿਖਰ.p, t);
  api_incr_ਸਿਖਰ(L);
  ਜੇ (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API ਪੂਰਨ lua_getmetatable (lua_State *L, ਪੂਰਨ objindex) {
  ਅਚਲ TValue *obj;
  ਸਾਰਣੀ *mt;
  ਪੂਰਨ res = 0;
  lua_ਤਾਲਾ(L);
  obj = index2value(L, objindex);
  ਚੋਣ (ttype(obj)) {
    ਹਾਲਤ LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      ਰੋਕੋ;
    ਹਾਲਤ LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      ਰੋਕੋ;
    ਮੂਲ:
      mt = G(L)->mt[ttype(obj)];
      ਰੋਕੋ;
  }
  ਜੇ (mt != ਨਿਰਾ) {
    sethvalue2s(L, L->ਸਿਖਰ.p, mt);
    api_incr_ਸਿਖਰ(L);
    res = 1;
  }
  lua_unlock(L);
  ਵਾਪਸ res;
}


LUA_API ਪੂਰਨ lua_getiuservalue (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ n) {
  TValue *o;
  ਪੂਰਨ t;
  lua_ਤਾਲਾ(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ਜੇ (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->ਸਿਖਰ.p);
    t = LUA_TNONE;
  }
  ਨਹੀਂਤਾਂ {
    setobj2s(L, L->ਸਿਖਰ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->ਸਿਖਰ.p));
  }
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
  ਵਾਪਸ t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
ਸਥਿਰ ਖਾਲੀ auxsetstr (lua_State *L, ਅਚਲ TValue *t, ਅਚਲ ਅੱਖਰ *k) {
  ਪੂਰਨ hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->ਸਿਖਰ.p - 1), hres, luaH_psetstr);
  ਜੇ (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->ਸਿਖਰ.p - 1));
    L->ਸਿਖਰ.p--;  /* pop value */
  }
  ਨਹੀਂਤਾਂ {
    setsvalue2s(L, L->ਸਿਖਰ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_ਸਿਖਰ(L);
    luaV_finishset(L, t, s2v(L->ਸਿਖਰ.p - 1), s2v(L->ਸਿਖਰ.p - 2), hres);
    L->ਸਿਖਰ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API ਖਾਲੀ lua_setglobal (lua_State *L, ਅਚਲ ਅੱਖਰ *name) {
  TValue gt;
  lua_ਤਾਲਾ(L);  /* unlock done in 'auxsetstr' */
  get_Global_ਸਾਰਣੀ(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API ਖਾਲੀ lua_settable (lua_State *L, ਪੂਰਨ idx) {
  TValue *t;
  ਪੂਰਨ hres;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->ਸਿਖਰ.p - 2), s2v(L->ਸਿਖਰ.p - 1), hres, luaH_pset);
  ਜੇ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ਸਿਖਰ.p - 1));
  ਨਹੀਂਤਾਂ
    luaV_finishset(L, t, s2v(L->ਸਿਖਰ.p - 2), s2v(L->ਸਿਖਰ.p - 1), hres);
  L->ਸਿਖਰ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_setfield (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *k) {
  lua_ਤਾਲਾ(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API ਖਾਲੀ lua_seti (lua_State *L, ਪੂਰਨ idx, lua_Integer n) {
  TValue *t;
  ਪੂਰਨ hres;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->ਸਿਖਰ.p - 1), hres);
  ਜੇ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ਸਿਖਰ.p - 1));
  ਨਹੀਂਤਾਂ {
    TValue ਆਰਜ਼ੀ;
    setivalue(&ਆਰਜ਼ੀ, n);
    luaV_finishset(L, t, &ਆਰਜ਼ੀ, s2v(L->ਸਿਖਰ.p - 1), hres);
  }
  L->ਸਿਖਰ.p--;  /* pop value */
  lua_unlock(L);
}


ਸਥਿਰ ਖਾਲੀ aux_rawset (lua_State *L, ਪੂਰਨ idx, TValue *ਕੁੰਜੀ, ਪੂਰਨ n) {
  ਸਾਰਣੀ *t;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  lua_H_ਸੈੱਟ(L, t, ਕੁੰਜੀ, s2v(L->ਸਿਖਰ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->ਸਿਖਰ.p - 1));
  L->ਸਿਖਰ.p -= n;
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_rawset (lua_State *L, ਪੂਰਨ idx) {
  aux_rawset(L, idx, s2v(L->ਸਿਖਰ.p - 2), 2);
}


LUA_API ਖਾਲੀ lua_rawsetp (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਖਾਲੀ *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API ਖਾਲੀ lua_rawseti (lua_State *L, ਪੂਰਨ idx, lua_Integer n) {
  ਸਾਰਣੀ *t;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->ਸਿਖਰ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->ਸਿਖਰ.p - 1));
  L->ਸਿਖਰ.p--;
  lua_unlock(L);
}


LUA_API ਪੂਰਨ lua_setmetatable (lua_State *L, ਪੂਰਨ objindex) {
  TValue *obj;
  ਸਾਰਣੀ *mt;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  ਜੇ (ttisnil(s2v(L->ਸਿਖਰ.p - 1)))
    mt = ਨਿਰਾ;
  ਨਹੀਂਤਾਂ {
    api_check(L, ttistable(s2v(L->ਸਿਖਰ.p - 1)), "table expected");
    mt = hvalue(s2v(L->ਸਿਖਰ.p - 1));
  }
  ਚੋਣ (ttype(obj)) {
    ਹਾਲਤ LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      ਜੇ (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      ਜੇ (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      G(L)->mt[ttype(obj)] = mt;
      ਰੋਕੋ;
    }
  }
  L->ਸਿਖਰ.p--;
  lua_unlock(L);
  ਵਾਪਸ 1;
}


LUA_API ਪੂਰਨ lua_setiuservalue (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ n) {
  TValue *o;
  ਪੂਰਨ res;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ਜੇ (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  ਨਹੀਂਤਾਂ {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->ਸਿਖਰ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->ਸਿਖਰ.p - 1));
    res = 1;
  }
  L->ਸਿਖਰ.p--;
  lua_unlock(L);
  ਵਾਪਸ res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->ਸਿਖਰ.p - L->ਸਿਖਰ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API ਖਾਲੀ lua_callk (lua_State *L, ਪੂਰਨ nargs, ਪੂਰਨ nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_ਤਾਲਾ(L);
  api_check(L, k == ਨਿਰਾ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->ਸਥਿਤੀ == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->ਸਿਖਰ.p - (nargs+1);
  ਜੇ (k != ਨਿਰਾ && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  ਨਹੀਂਤਾਂ  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
ਬਣਤਰ CallS {  /* data to 'f_call' */
  StkId func;
  ਪੂਰਨ nresults;
};


ਸਥਿਰ ਖਾਲੀ f_call (lua_State *L, ਖਾਲੀ *ud) {
  ਬਣਤਰ CallS *c = cast(ਬਣਤਰ CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API ਪੂਰਨ lua_pcallk (lua_State *L, ਪੂਰਨ nargs, ਪੂਰਨ nresults, ਪੂਰਨ errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  ਬਣਤਰ CallS c;
  TStatus ਸਥਿਤੀ;
  ptrdiff_t func;
  lua_ਤਾਲਾ(L);
  api_check(L, k == ਨਿਰਾ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->ਸਥਿਤੀ == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  ਜੇ (errfunc == 0)
    func = 0;
  ਨਹੀਂਤਾਂ {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->ਸਿਖਰ.p - (nargs+1);  /* function to be called */
  ਜੇ (k == ਨਿਰਾ || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    ਸਥਿਤੀ = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  ਨਹੀਂਤਾਂ {  /* prepare continuation (call is already protected by 'resume') */
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
    ਸਥਿਤੀ = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  ਵਾਪਸ APIstatus(ਸਥਿਤੀ);
}


LUA_API ਪੂਰਨ lua_ਲੱਦੋ (lua_State *L, lua_Reader reader, ਖਾਲੀ *ਅੰਕੜੇ,
                      ਅਚਲ ਅੱਖਰ *chunkname, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
  ZIO z;
  TStatus ਸਥਿਤੀ;
  lua_ਤਾਲਾ(L);
  luaC_checkGC(L);
  ਜੇ (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, ਅੰਕੜੇ);
  ਸਥਿਤੀ = luaD_protectedparser(L, &z, chunkname, ਬਹੁਲਕ);
  ਜੇ (ਸਥਿਤੀ == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->ਸਿਖਰ.p - 1));  /* get new function */
    ਜੇ (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      get_Global_ਸਾਰਣੀ(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  ਵਾਪਸ APIstatus(ਸਥਿਤੀ);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API ਪੂਰਨ lua_dump (lua_State *L, lua_Writer writer, ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ strip) {
  ਪੂਰਨ ਸਥਿਤੀ;
  ptrdiff_t otop = savestack(L, L->ਸਿਖਰ.p);  /* original top */
  TValue *f = s2v(L->ਸਿਖਰ.p - 1);  /* function to be dumped */
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  ਸਥਿਤੀ = luaU_dump(L, clLvalue(f)->p, writer, ਅੰਕੜੇ, strip);
  L->ਸਿਖਰ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  ਵਾਪਸ ਸਥਿਤੀ;
}


LUA_API ਪੂਰਨ lua_ਸਥਿਤੀ (lua_State *L) {
  ਵਾਪਸ APIstatus(L->ਸਥਿਤੀ);
}


/*
** Garbage-collection function
*/
LUA_API ਪੂਰਨ lua_gc (lua_State *L, ਪੂਰਨ what, ...) {
  va_list argp;
  ਪੂਰਨ res = 0;
  global_State *g = G(L);
  ਜੇ (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    ਵਾਪਸ -1;  /* all options are invalid when stopped */
  lua_ਤਾਲਾ(L);
  va_start(argp, what);
  ਚੋਣ (what) {
    ਹਾਲਤ LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      ਪੂਰਨ work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      ਜੇ (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      ਨਹੀਂਤਾਂ ਜੇ (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      ਨਹੀਂਤਾਂ  /*ਉਛਾਲ*/
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (ਖਾਲੀ)0, work = 1);
      ਜੇ (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCISRUNNING: {
      res = gcrunning(g);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_GCPARAM: {
      ਪੂਰਨ param = va_arg(argp, ਪੂਰਨ);
      ਪੂਰਨ ਮੁੱਲ = va_arg(argp, ਪੂਰਨ);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      ਜੇ (ਮੁੱਲ >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(ਮੁੱਲ));
      ਰੋਕੋ;
    }
    ਮੂਲ: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  ਵਾਪਸ res;
}



/*
** miscellaneous functions
*/


LUA_API ਪੂਰਨ lua_error (lua_State *L) {
  TValue *errobj;
  lua_ਤਾਲਾ(L);
  errobj = s2v(L->ਸਿਖਰ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  ਜੇ (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  ਨਹੀਂਤਾਂ
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  ਵਾਪਸ 0;  /* to avoid warnings */
}


LUA_API ਪੂਰਨ lua_ਅਗਲਾ (lua_State *L, ਪੂਰਨ idx) {
  ਸਾਰਣੀ *t;
  ਪੂਰਨ ਹੋਰ;
  lua_ਤਾਲਾ(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  ਹੋਰ = lua_H_ਅਗਲਾ(L, t, L->ਸਿਖਰ.p - 1);
  ਜੇ (ਹੋਰ)
    api_incr_ਸਿਖਰ(L);
  ਨਹੀਂਤਾਂ  /* no more elements */
    L->ਸਿਖਰ.p--;  /* pop key */
  lua_unlock(L);
  ਵਾਪਸ ਹੋਰ;
}


LUA_API ਖਾਲੀ lua_toclose (lua_State *L, ਪੂਰਨ idx) {
  StkId o;
  lua_ਤਾਲਾ(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_concat (lua_State *L, ਪੂਰਨ n) {
  lua_ਤਾਲਾ(L);
  ਜੇ (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  ਨਹੀਂਤਾਂ {  /* nothing to concatenate */
    setsvalue2s(L, L->ਸਿਖਰ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_ਸਿਖਰ(L);
  }
  lua_unlock(L);
}


LUA_API ਖਾਲੀ lua_len (lua_State *L, ਪੂਰਨ idx) {
  TValue *t;
  lua_ਤਾਲਾ(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->ਸਿਖਰ.p, t);
  api_incr_ਸਿਖਰ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, ਖਾਲੀ **ud) {
  lua_Alloc f;
  lua_ਤਾਲਾ(L);
  ਜੇ (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  ਵਾਪਸ f;
}


LUA_API ਖਾਲੀ lua_setallocf (lua_State *L, lua_Alloc f, ਖਾਲੀ *ud) {
  lua_ਤਾਲਾ(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


ਖਾਲੀ lua_setwarnf (lua_State *L, lua_WarnFunction f, ਖਾਲੀ *ud) {
  lua_ਤਾਲਾ(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


ਖਾਲੀ lua_warning (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ tocont) {
  lua_ਤਾਲਾ(L);
  luaE_warning(L, ਸੁਨੇਹਾ, tocont);
  lua_unlock(L);
}



LUA_API ਖਾਲੀ *lua_newuserdatauv (lua_State *L, size_t ਮਾਪ, ਪੂਰਨ nuvalue) {
  Udata *u;
  lua_ਤਾਲਾ(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, ਮਾਪ, cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ, nuvalue));
  setuvalue(L, s2v(L->ਸਿਖਰ.p), u);
  api_incr_ਸਿਖਰ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ਵਾਪਸ getudatamem(u);
}



ਸਥਿਰ ਅਚਲ ਅੱਖਰ *aux_upvalue (TValue *fi, ਪੂਰਨ n, TValue **val,
                                GCObject **owner) {
  ਚੋਣ (ttypetag(fi)) {
    ਹਾਲਤ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ਜੇ (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        ਵਾਪਸ ਨਿਰਾ;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      ਜੇ (owner) *owner = obj2gco(f);
      ਵਾਪਸ "";
    }
    ਹਾਲਤ LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      ਜੇ (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        ਵਾਪਸ ਨਿਰਾ;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      ਜੇ (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      ਵਾਪਸ (name == ਨਿਰਾ) ? "(no name)" : getstr(name);
    }
    ਮੂਲ: ਵਾਪਸ ਨਿਰਾ;  /* not a closure */
  }
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_getupvalue (lua_State *L, ਪੂਰਨ funcindex, ਪੂਰਨ n) {
  ਅਚਲ ਅੱਖਰ *name;
  TValue *val = ਨਿਰਾ;  /* to avoid warnings */
  lua_ਤਾਲਾ(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ਨਿਰਾ);
  ਜੇ (name) {
    setobj2s(L, L->ਸਿਖਰ.p, val);
    api_incr_ਸਿਖਰ(L);
  }
  lua_unlock(L);
  ਵਾਪਸ name;
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_setupvalue (lua_State *L, ਪੂਰਨ funcindex, ਪੂਰਨ n) {
  ਅਚਲ ਅੱਖਰ *name;
  TValue *val = ਨਿਰਾ;  /* to avoid warnings */
  GCObject *owner = ਨਿਰਾ;  /* to avoid warnings */
  TValue *fi;
  lua_ਤਾਲਾ(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  ਜੇ (name) {
    L->ਸਿਖਰ.p--;
    setobj(L, val, s2v(L->ਸਿਖਰ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  ਵਾਪਸ name;
}


ਸਥਿਰ ਉੱਪਰ_Val **getupvalref (lua_State *L, ਪੂਰਨ fidx, ਪੂਰਨ n, LClosure **pf) {
  ਸਥਿਰ ਅਚਲ ਉੱਪਰ_Val *ਅਚਲ nullup = ਨਿਰਾ;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  ਜੇ (pf) *pf = f;
  ਜੇ (1 <= n && n <= f->p->sizeupvalues)
    ਵਾਪਸ &f->upvals[n - 1];  /* get its upvalue pointer */
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ (ਉੱਪਰ_Val**)&nullup;
}


LUA_API ਖਾਲੀ *lua_upvalueid (lua_State *L, ਪੂਰਨ fidx, ਪੂਰਨ n) {
  TValue *fi = index2value(L, fidx);
  ਚੋਣ (ttypetag(fi)) {
    ਹਾਲਤ LUA_VLCL: {  /* lua closure */
      ਵਾਪਸ *getupvalref(L, fidx, n, ਨਿਰਾ);
    }
    ਹਾਲਤ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ਜੇ (1 <= n && n <= f->nupvalues)
        ਵਾਪਸ &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    ਹਾਲਤ LUA_VLCF:
      ਵਾਪਸ ਨਿਰਾ;  /* light C functions have no upvalues */
    ਮੂਲ: {
      api_check(L, 0, "function expected");
      ਵਾਪਸ ਨਿਰਾ;
    }
  }
}


LUA_API ਖਾਲੀ lua_upvaluejoin (lua_State *L, ਪੂਰਨ fidx1, ਪੂਰਨ n1,
                                            ਪੂਰਨ fidx2, ਪੂਰਨ n2) {
  LClosure *f1;
  ਉੱਪਰ_Val **up1 = getupvalref(L, fidx1, n1, &f1);
  ਉੱਪਰ_Val **up2 = getupvalref(L, fidx2, n2, ਨਿਰਾ);
  api_check(L, *up1 != ਨਿਰਾ && *up2 != ਨਿਰਾ, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


