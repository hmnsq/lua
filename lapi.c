/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#تعریف_کرو lapi_c
#تعریف_کرو LUA_CORE

#ضمن "lprefix.h"


#ضمن <limits.h>
#ضمن <stdarg.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "lgc.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "ltm.h"
#ضمن "lundump.h"
#ضمن "lvm.h"



ثابت حرف lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
*/
#تعریف_کرو isvalid(L, o)	((o) != &G(L)->nilvalue)


/* test for pseudo index */
#تعریف_کرو ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#تعریف_کرو isupvalue(i)		((i) < LUA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
ساكن TValue *index2value (lua_State *L, صحيح idx) {
  CallInfo *ci = L->ci;
  إذا (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->چوٹی.p - (ci->func.p + 1), "unacceptable index");
    إذا (o >= L->چوٹی.p) أرجع &G(L)->nilvalue;
    وإلا أرجع s2v(o);
  }
  وإلا إذا (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->چوٹی.p - (ci->func.p + 1),
                 "invalid index");
    أرجع s2v(L->چوٹی.p + idx);
  }
  وإلا إذا (idx == LUA_REGISTRYINDEX)
    أرجع &G(L)->l_registry;
  وإلا {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    إذا (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      أرجع (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    وإلا {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      أرجع &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
ساكن StkId index2stack (lua_State *L, صحيح idx) {
  CallInfo *ci = L->ci;
  إذا (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->چوٹی.p, "invalid index");
    أرجع o;
  }
  وإلا {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->چوٹی.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    أرجع L->چوٹی.p + idx;
  }
}


LUA_API صحيح lua_checkstack (lua_State *L, صحيح n) {
  صحيح res;
  CallInfo *ci;
  lua_جندرا(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  إذا (L->ڈھیر_آخری.p - L->چوٹی.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  وإلا  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  إذا (res && ci->چوٹی.p < L->چوٹی.p + n)
    ci->چوٹی.p = L->چوٹی.p + n;  /* adjust frame top */
  lua_unlock(L);
  أرجع res;
}


LUA_API فراغ lua_xmove (lua_State *from, lua_State *منزل, صحيح n) {
  صحيح i;
  إذا (from == منزل) أرجع;
  lua_جندرا(منزل);
  api_checkpop(from, n);
  api_check(from, G(from) == G(منزل), "moving among independent states");
  api_check(from, منزل->ci->چوٹی.p - منزل->چوٹی.p >= n, "stack overflow");
  from->چوٹی.p -= n;
  لكل (i = 0; i < n; i++) {
    setobjs2s(منزل, منزل->چوٹی.p, from->چوٹی.p + i);
    منزل->چوٹی.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(منزل);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_جندرا(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  أرجع old;
}


LUA_API lua_نمبر lua_نسخہ (lua_State *L) {
  بن_ورتیا(L);
  أرجع LUA_نسخہ_عدد;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API صحيح lua_absindex (lua_State *L, صحيح idx) {
  أرجع (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->چوٹی.p - L->ci->func.p) + idx;
}


LUA_API صحيح lua_gettop (lua_State *L) {
  أرجع cast_int(L->چوٹی.p - (L->ci->func.p + 1));
}


LUA_API فراغ lua_settop (lua_State *L, صحيح idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_جندرا(L);
  ci = L->ci;
  func = ci->func.p;
  إذا (idx >= 0) {
    api_check(L, idx <= ci->چوٹی.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->چوٹی.p;
    لكل (; diff > 0; diff--)
      setnilvalue2s(L->چوٹی.p++);  /* clear new slots */
  }
  وإلا {
    api_check(L, -(idx+1) <= (L->چوٹی.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->چوٹی.p + diff;
  إذا (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->چوٹی.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API فراغ lua_closeslot (lua_State *L, صحيح idx) {
  StkId level;
  lua_جندرا(L);
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
ساكن فراغ الٹ (lua_State *L, StkId from, StkId منزل) {
  لكل (; from < منزل; from++, منزل--) {
    TValue عارضی;
    setobj(L, &عارضی, s2v(from));
    setobjs2s(L, from, منزل);
    setobj2s(L, منزل, &عارضی);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API فراغ lua_rotate (lua_State *L, صحيح idx, صحيح n) {
  StkId p, t, m;
  lua_جندرا(L);
  t = L->چوٹی.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  الٹ(L, p, m);  /* reverse the prefix with length 'n' */
  الٹ(L, m + 1, t);  /* reverse the suffix */
  الٹ(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API فراغ lua_copy (lua_State *L, صحيح fromidx, صحيح toidx) {
  TValue *fr, *منزل;
  lua_جندرا(L);
  fr = index2value(L, fromidx);
  منزل = index2value(L, toidx);
  api_check(L, isvalid(L, منزل), "invalid index");
  setobj(L, منزل, fr);
  إذا (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API فراغ lua_pushvalue (lua_State *L, صحيح idx) {
  lua_جندرا(L);
  setobj2s(L, L->چوٹی.p, index2value(L, idx));
  api_incr_چوٹی(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API صحيح lua_type (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ثابت حرف *lua_typename (lua_State *L, صحيح t) {
  بن_ورتیا(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  أرجع ttypename(t);
}


LUA_API صحيح lua_iscfunction (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API صحيح lua_isinteger (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع ttisinteger(o);
}


LUA_API صحيح lua_isnumber (lua_State *L, صحيح idx) {
  lua_نمبر n;
  ثابت TValue *o = index2value(L, idx);
  أرجع tonumber(o, &n);
}


LUA_API صحيح lua_isstring (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع (ttisstring(o) || cvt2str(o));
}


LUA_API صحيح lua_isuserdata (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API صحيح lua_rawequal (lua_State *L, صحيح index1, صحيح index2) {
  ثابت TValue *o1 = index2value(L, index1);
  ثابت TValue *o2 = index2value(L, index2);
  أرجع (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API فراغ lua_arith (lua_State *L, صحيح op) {
  lua_جندرا(L);
  إذا (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  وإلا {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->چوٹی.p, L->چوٹی.p - 1);
    api_incr_چوٹی(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->چوٹی.p - 2), s2v(L->چوٹی.p - 1), L->چوٹی.p - 2);
  L->چوٹی.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API صحيح lua_compare (lua_State *L, صحيح index1, صحيح index2, صحيح op) {
  ثابت TValue *o1;
  ثابت TValue *o2;
  صحيح i = 0;
  lua_جندرا(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  إذا (isvalid(L, o1) && isvalid(L, o2)) {
    بدل (op) {
      حالة LUA_OPEQ: i = luaV_equalobj(L, o1, o2); اكسر;
      حالة LUA_OPLT: i = luaV_lessthan(L, o1, o2); اكسر;
      حالة LUA_OPLE: i = luaV_lessequal(L, o1, o2); اكسر;
      افتراضي: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  أرجع i;
}


LUA_API بنانشانی lua_numbertocstring (lua_State *L, صحيح idx, حرف *buff) {
  ثابت TValue *o = index2value(L, idx);
  إذا (ttisnumber(o)) {
    بنانشانی len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    أرجع len;
  }
  وإلا
    أرجع 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ثابت حرف *s) {
  size_t sz = luaO_str2num(s, s2v(L->چوٹی.p));
  إذا (sz != 0)
    api_incr_چوٹی(L);
  أرجع sz;
}


LUA_API lua_نمبر lua_tonumberx (lua_State *L, صحيح idx, صحيح *pisnum) {
  lua_نمبر n = 0;
  ثابت TValue *o = index2value(L, idx);
  صحيح isnum = tonumber(o, &n);
  إذا (pisnum)
    *pisnum = isnum;
  أرجع n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, صحيح idx, صحيح *pisnum) {
  lua_Integer res = 0;
  ثابت TValue *o = index2value(L, idx);
  صحيح isnum = tointeger(o, &res);
  إذا (pisnum)
    *pisnum = isnum;
  أرجع res;
}


LUA_API صحيح lua_toboolean (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع !l_isfalse(o);
}


LUA_API ثابت حرف *lua_tolstring (lua_State *L, صحيح idx, size_t *len) {
  TValue *o;
  lua_جندرا(L);
  o = index2value(L, idx);
  إذا (!ttisstring(o)) {
    إذا (!cvt2str(o)) {  /* not convertible? */
      إذا (len != عدم) *len = 0;
      lua_unlock(L);
      أرجع عدم;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  إذا (len != عدم)
    أرجع getlstr(tsvalue(o), *len);
  وإلا
    أرجع getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  بدل (ttypetag(o)) {
    حالة LUA_VSHRSTR: أرجع cast(lua_Unsigned, tsvalue(o)->shrlen);
    حالة LUA_VLNGSTR: أرجع cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    حالة LUA_VUSERDATA: أرجع cast(lua_Unsigned, uvalue(o)->len);
    حالة LUA_VTABLE: {
      lua_Unsigned res;
      lua_جندرا(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      أرجع res;
    }
    افتراضي: أرجع 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  إذا (ttislcf(o)) أرجع fvalue(o);
  وإلا إذا (ttisCclosure(o))
    أرجع clCvalue(o)->f;
  وإلا أرجع عدم;  /* not a C function */
}


l_sinline فراغ *touserdata (ثابت TValue *o) {
  بدل (ttype(o)) {
    حالة LUA_TUSERDATA: أرجع getudatamem(uvalue(o));
    حالة LUA_TLIGHTUSERDATA: أرجع pvalue(o);
    افتراضي: أرجع عدم;
  }
}


LUA_API فراغ *lua_touserdata (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  أرجع (!ttisthread(o)) ? عدم : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ثابت فراغ *lua_topointer (lua_State *L, صحيح idx) {
  ثابت TValue *o = index2value(L, idx);
  بدل (ttypetag(o)) {
    حالة LUA_VLCF: أرجع cast_voidp(cast_sizet(fvalue(o)));
    حالة LUA_VUSERDATA: حالة LUA_VLIGHTUSERDATA:
      أرجع touserdata(o);
    افتراضي: {
      إذا (iscollectable(o))
        أرجع gcvalue(o);
      وإلا
        أرجع عدم;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API فراغ lua_pushnil (lua_State *L) {
  lua_جندرا(L);
  setnilvalue2s(L->چوٹی.p);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API فراغ lua_pushnumber (lua_State *L, lua_نمبر n) {
  lua_جندرا(L);
  setfltvalue(s2v(L->چوٹی.p), n);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API فراغ lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_جندرا(L);
  setivalue(s2v(L->چوٹی.p), n);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ثابت حرف *lua_pushlstring (lua_State *L, ثابت حرف *s, size_t len) {
  TString *ts;
  lua_جندرا(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->چوٹی.p, ts);
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  أرجع getstr(ts);
}


LUA_API ثابت حرف *lua_pushexternalstring (lua_State *L,
	        ثابت حرف *s, size_t len, lua_Alloc falloc, فراغ *ud) {
  TString *ts;
  lua_جندرا(L);
  api_check(L, len <= MAX_ناپ, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->چوٹی.p, ts);
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  أرجع getstr(ts);
}


LUA_API ثابت حرف *lua_pushstring (lua_State *L, ثابت حرف *s) {
  lua_جندرا(L);
  إذا (s == عدم)
    setnilvalue2s(L->چوٹی.p);
  وإلا {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->چوٹی.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  أرجع s;
}


LUA_API ثابت حرف *lua_pushvfstring (lua_State *L, ثابت حرف *fmt,
                                      va_list argp) {
  ثابت حرف *ret;
  lua_جندرا(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  أرجع ret;
}


LUA_API ثابت حرف *lua_pushfstring (lua_State *L, ثابت حرف *fmt, ...) {
  ثابت حرف *ret;
  va_list argp;
  lua_جندرا(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  أرجع ret;
}


LUA_API فراغ lua_pushcclosure (lua_State *L, lua_CFunction fn, صحيح n) {
  lua_جندرا(L);
  إذا (n == 0) {
    setfvalue(s2v(L->چوٹی.p), fn);
    api_incr_چوٹی(L);
  }
  وإلا {
    صحيح i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    لكل (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->چوٹی.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->چوٹی.p -= n;
    setclCvalue(L, s2v(L->چوٹی.p), cl);
    api_incr_چوٹی(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API فراغ lua_pushboolean (lua_State *L, صحيح b) {
  lua_جندرا(L);
  إذا (b)
    setbtvalue(s2v(L->چوٹی.p));
  وإلا
    setbfvalue(s2v(L->چوٹی.p));
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API فراغ lua_pushlightuserdata (lua_State *L, فراغ *p) {
  lua_جندرا(L);
  setpvalue(s2v(L->چوٹی.p), p);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API صحيح lua_pushthread (lua_State *L) {
  lua_جندرا(L);
  setthvalue(L, s2v(L->چوٹی.p), L);
  api_incr_چوٹی(L);
  lua_unlock(L);
  أرجع (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


ساكن صحيح auxgetstr (lua_State *L, ثابت TValue *t, ثابت حرف *k) {
  lu_byte ٹیگ;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->چوٹی.p), luaH_getstr, ٹیگ);
  إذا (!tagisempty(ٹیگ))
    api_incr_چوٹی(L);
  وإلا {
    setsvalue2s(L, L->چوٹی.p, str);
    api_incr_چوٹی(L);
    ٹیگ = luaV_finishget(L, t, s2v(L->چوٹی.p - 1), L->چوٹی.p - 1, ٹیگ);
  }
  lua_unlock(L);
  أرجع novariant(ٹیگ);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
ساكن فراغ get_Global_جدول (lua_State *L, TValue *gt) {
  جدول *registry = hvalue(&G(L)->l_registry);
  lu_byte ٹیگ = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (فراغ)ٹیگ;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(ٹیگ) == LUA_TTABLE, "global table must exist");
}


LUA_API صحيح lua_getglobal (lua_State *L, ثابت حرف *name) {
  TValue gt;
  lua_جندرا(L);
  get_Global_جدول(L, &gt);
  أرجع auxgetstr(L, &gt, name);
}


LUA_API صحيح lua_gettable (lua_State *L, صحيح idx) {
  lu_byte ٹیگ;
  TValue *t;
  lua_جندرا(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->چوٹی.p - 1), s2v(L->چوٹی.p - 1), luaH_get, ٹیگ);
  إذا (tagisempty(ٹیگ))
    ٹیگ = luaV_finishget(L, t, s2v(L->چوٹی.p - 1), L->چوٹی.p - 1, ٹیگ);
  lua_unlock(L);
  أرجع novariant(ٹیگ);
}


LUA_API صحيح lua_getfield (lua_State *L, صحيح idx, ثابت حرف *k) {
  lua_جندرا(L);
  أرجع auxgetstr(L, index2value(L, idx), k);
}


LUA_API صحيح lua_geti (lua_State *L, صحيح idx, lua_Integer n) {
  TValue *t;
  lu_byte ٹیگ;
  lua_جندرا(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->چوٹی.p), ٹیگ);
  إذا (tagisempty(ٹیگ)) {
    TValue کلید;
    setivalue(&کلید, n);
    ٹیگ = luaV_finishget(L, t, &کلید, L->چوٹی.p, ٹیگ);
  }
  api_incr_چوٹی(L);
  lua_unlock(L);
  أرجع novariant(ٹیگ);
}


ساكن صحيح finishrawget (lua_State *L, lu_byte ٹیگ) {
  إذا (tagisempty(ٹیگ))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->چوٹی.p);
  api_incr_چوٹی(L);
  lua_unlock(L);
  أرجع novariant(ٹیگ);
}


l_sinline جدول *gettable (lua_State *L, صحيح idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  أرجع hvalue(t);
}


LUA_API صحيح lua_rawget (lua_State *L, صحيح idx) {
  جدول *t;
  lu_byte ٹیگ;
  lua_جندرا(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  ٹیگ = luaH_get(t, s2v(L->چوٹی.p - 1), s2v(L->چوٹی.p - 1));
  L->چوٹی.p--;  /* pop key */
  أرجع finishrawget(L, ٹیگ);
}


LUA_API صحيح lua_rawgeti (lua_State *L, صحيح idx, lua_Integer n) {
  جدول *t;
  lu_byte ٹیگ;
  lua_جندرا(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->چوٹی.p), ٹیگ);
  أرجع finishrawget(L, ٹیگ);
}


LUA_API صحيح lua_rawgetp (lua_State *L, صحيح idx, ثابت فراغ *p) {
  جدول *t;
  TValue k;
  lua_جندرا(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  أرجع finishrawget(L, luaH_get(t, &k, s2v(L->چوٹی.p)));
}


LUA_API فراغ lua_createtable (lua_State *L, صحيح narray, صحيح nrec) {
  جدول *t;
  lua_جندرا(L);
  t = luaH_new(L);
  sethvalue2s(L, L->چوٹی.p, t);
  api_incr_چوٹی(L);
  إذا (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API صحيح lua_getmetatable (lua_State *L, صحيح objindex) {
  ثابت TValue *obj;
  جدول *mt;
  صحيح res = 0;
  lua_جندرا(L);
  obj = index2value(L, objindex);
  بدل (ttype(obj)) {
    حالة LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      اكسر;
    حالة LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      اكسر;
    افتراضي:
      mt = G(L)->mt[ttype(obj)];
      اكسر;
  }
  إذا (mt != عدم) {
    sethvalue2s(L, L->چوٹی.p, mt);
    api_incr_چوٹی(L);
    res = 1;
  }
  lua_unlock(L);
  أرجع res;
}


LUA_API صحيح lua_getiuservalue (lua_State *L, صحيح idx, صحيح n) {
  TValue *o;
  صحيح t;
  lua_جندرا(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  إذا (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->چوٹی.p);
    t = LUA_TNONE;
  }
  وإلا {
    setobj2s(L, L->چوٹی.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->چوٹی.p));
  }
  api_incr_چوٹی(L);
  lua_unlock(L);
  أرجع t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
ساكن فراغ auxsetstr (lua_State *L, ثابت TValue *t, ثابت حرف *k) {
  صحيح hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->چوٹی.p - 1), hres, luaH_psetstr);
  إذا (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->چوٹی.p - 1));
    L->چوٹی.p--;  /* pop value */
  }
  وإلا {
    setsvalue2s(L, L->چوٹی.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_چوٹی(L);
    luaV_finishset(L, t, s2v(L->چوٹی.p - 1), s2v(L->چوٹی.p - 2), hres);
    L->چوٹی.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API فراغ lua_setglobal (lua_State *L, ثابت حرف *name) {
  TValue gt;
  lua_جندرا(L);  /* unlock done in 'auxsetstr' */
  get_Global_جدول(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API فراغ lua_settable (lua_State *L, صحيح idx) {
  TValue *t;
  صحيح hres;
  lua_جندرا(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->چوٹی.p - 2), s2v(L->چوٹی.p - 1), hres, luaH_pset);
  إذا (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->چوٹی.p - 1));
  وإلا
    luaV_finishset(L, t, s2v(L->چوٹی.p - 2), s2v(L->چوٹی.p - 1), hres);
  L->چوٹی.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API فراغ lua_setfield (lua_State *L, صحيح idx, ثابت حرف *k) {
  lua_جندرا(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API فراغ lua_seti (lua_State *L, صحيح idx, lua_Integer n) {
  TValue *t;
  صحيح hres;
  lua_جندرا(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->چوٹی.p - 1), hres);
  إذا (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->چوٹی.p - 1));
  وإلا {
    TValue عارضی;
    setivalue(&عارضی, n);
    luaV_finishset(L, t, &عارضی, s2v(L->چوٹی.p - 1), hres);
  }
  L->چوٹی.p--;  /* pop value */
  lua_unlock(L);
}


ساكن فراغ aux_rawset (lua_State *L, صحيح idx, TValue *کلید, صحيح n) {
  جدول *t;
  lua_جندرا(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  lua_H_سیٹ(L, t, کلید, s2v(L->چوٹی.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->چوٹی.p - 1));
  L->چوٹی.p -= n;
  lua_unlock(L);
}


LUA_API فراغ lua_rawset (lua_State *L, صحيح idx) {
  aux_rawset(L, idx, s2v(L->چوٹی.p - 2), 2);
}


LUA_API فراغ lua_rawsetp (lua_State *L, صحيح idx, ثابت فراغ *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API فراغ lua_rawseti (lua_State *L, صحيح idx, lua_Integer n) {
  جدول *t;
  lua_جندرا(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->چوٹی.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->چوٹی.p - 1));
  L->چوٹی.p--;
  lua_unlock(L);
}


LUA_API صحيح lua_setmetatable (lua_State *L, صحيح objindex) {
  TValue *obj;
  جدول *mt;
  lua_جندرا(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  إذا (ttisnil(s2v(L->چوٹی.p - 1)))
    mt = عدم;
  وإلا {
    api_check(L, ttistable(s2v(L->چوٹی.p - 1)), "table expected");
    mt = hvalue(s2v(L->چوٹی.p - 1));
  }
  بدل (ttype(obj)) {
    حالة LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      إذا (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      اكسر;
    }
    حالة LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      إذا (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      اكسر;
    }
    افتراضي: {
      G(L)->mt[ttype(obj)] = mt;
      اكسر;
    }
  }
  L->چوٹی.p--;
  lua_unlock(L);
  أرجع 1;
}


LUA_API صحيح lua_setiuservalue (lua_State *L, صحيح idx, صحيح n) {
  TValue *o;
  صحيح res;
  lua_جندرا(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  إذا (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  وإلا {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->چوٹی.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->چوٹی.p - 1));
    res = 1;
  }
  L->چوٹی.p--;
  lua_unlock(L);
  أرجع res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#تعریف_کرو checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->چوٹی.p - L->چوٹی.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API فراغ lua_callk (lua_State *L, صحيح nargs, صحيح nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_جندرا(L);
  api_check(L, k == عدم || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->حالت == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->چوٹی.p - (nargs+1);
  إذا (k != عدم && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  وإلا  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
بنية CallS {  /* data to 'f_call' */
  StkId func;
  صحيح nresults;
};


ساكن فراغ f_call (lua_State *L, فراغ *ud) {
  بنية CallS *c = cast(بنية CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API صحيح lua_pcallk (lua_State *L, صحيح nargs, صحيح nresults, صحيح errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  بنية CallS c;
  TStatus حالت;
  ptrdiff_t func;
  lua_جندرا(L);
  api_check(L, k == عدم || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->حالت == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  إذا (errfunc == 0)
    func = 0;
  وإلا {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->چوٹی.p - (nargs+1);  /* function to be called */
  إذا (k == عدم || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    حالت = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  وإلا {  /* prepare continuation (call is already protected by 'resume') */
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
    حالت = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  أرجع APIstatus(حالت);
}


LUA_API صحيح lua_لدو (lua_State *L, lua_Reader reader, فراغ *انکڑے,
                      ثابت حرف *chunkname, ثابت حرف *منوال) {
  ZIO z;
  TStatus حالت;
  lua_جندرا(L);
  luaC_checkGC(L);
  إذا (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, انکڑے);
  حالت = luaD_protectedparser(L, &z, chunkname, منوال);
  إذا (حالت == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->چوٹی.p - 1));  /* get new function */
    إذا (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      get_Global_جدول(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  أرجع APIstatus(حالت);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API صحيح lua_dump (lua_State *L, lua_Writer writer, فراغ *انکڑے, صحيح strip) {
  صحيح حالت;
  ptrdiff_t otop = savestack(L, L->چوٹی.p);  /* original top */
  TValue *f = s2v(L->چوٹی.p - 1);  /* function to be dumped */
  lua_جندرا(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  حالت = luaU_dump(L, clLvalue(f)->p, writer, انکڑے, strip);
  L->چوٹی.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  أرجع حالت;
}


LUA_API صحيح lua_حالت (lua_State *L) {
  أرجع APIstatus(L->حالت);
}


/*
** Garbage-collection function
*/
LUA_API صحيح lua_gc (lua_State *L, صحيح what, ...) {
  va_list argp;
  صحيح res = 0;
  global_State *g = G(L);
  إذا (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    أرجع -1;  /* all options are invalid when stopped */
  lua_جندرا(L);
  va_start(argp, what);
  بدل (what) {
    حالة LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      اكسر;
    }
    حالة LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      اكسر;
    }
    حالة LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      اكسر;
    }
    حالة LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      اكسر;
    }
    حالة LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      اكسر;
    }
    حالة LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      صحيح work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      إذا (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      وإلا إذا (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      وإلا  /*حد_توں_ودھ*/
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (فراغ)0, work = 1);
      إذا (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      اكسر;
    }
    حالة LUA_GCISRUNNING: {
      res = gcrunning(g);
      اكسر;
    }
    حالة LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      اكسر;
    }
    حالة LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      اكسر;
    }
    حالة LUA_GCPARAM: {
      صحيح param = va_arg(argp, صحيح);
      صحيح قدر = va_arg(argp, صحيح);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      إذا (قدر >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(قدر));
      اكسر;
    }
    افتراضي: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  أرجع res;
}



/*
** miscellaneous functions
*/


LUA_API صحيح lua_error (lua_State *L) {
  TValue *errobj;
  lua_جندرا(L);
  errobj = s2v(L->چوٹی.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  إذا (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  وإلا
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  أرجع 0;  /* to avoid warnings */
}


LUA_API صحيح lua_اگلا (lua_State *L, صحيح idx) {
  جدول *t;
  صحيح ہور;
  lua_جندرا(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  ہور = lua_H_اگلا(L, t, L->چوٹی.p - 1);
  إذا (ہور)
    api_incr_چوٹی(L);
  وإلا  /* no more elements */
    L->چوٹی.p--;  /* pop key */
  lua_unlock(L);
  أرجع ہور;
}


LUA_API فراغ lua_toclose (lua_State *L, صحيح idx) {
  StkId o;
  lua_جندرا(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API فراغ lua_concat (lua_State *L, صحيح n) {
  lua_جندرا(L);
  إذا (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  وإلا {  /* nothing to concatenate */
    setsvalue2s(L, L->چوٹی.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_چوٹی(L);
  }
  lua_unlock(L);
}


LUA_API فراغ lua_len (lua_State *L, صحيح idx) {
  TValue *t;
  lua_جندرا(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->چوٹی.p, t);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, فراغ **ud) {
  lua_Alloc f;
  lua_جندرا(L);
  إذا (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  أرجع f;
}


LUA_API فراغ lua_setallocf (lua_State *L, lua_Alloc f, فراغ *ud) {
  lua_جندرا(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


فراغ lua_setwarnf (lua_State *L, lua_WarnFunction f, فراغ *ud) {
  lua_جندرا(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


فراغ lua_warning (lua_State *L, ثابت حرف *پیغام, صحيح tocont) {
  lua_جندرا(L);
  luaE_warning(L, پیغام, tocont);
  lua_unlock(L);
}



LUA_API فراغ *lua_newuserdatauv (lua_State *L, size_t ناپ, صحيح nuvalue) {
  Udata *u;
  lua_جندرا(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, ناپ, cast(بنانشانی چھوٹا, nuvalue));
  setuvalue(L, s2v(L->چوٹی.p), u);
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  أرجع getudatamem(u);
}



ساكن ثابت حرف *aux_upvalue (TValue *fi, صحيح n, TValue **val,
                                GCObject **owner) {
  بدل (ttypetag(fi)) {
    حالة LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      إذا (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        أرجع عدم;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      إذا (owner) *owner = obj2gco(f);
      أرجع "";
    }
    حالة LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      إذا (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        أرجع عدم;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      إذا (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      أرجع (name == عدم) ? "(no name)" : getstr(name);
    }
    افتراضي: أرجع عدم;  /* not a closure */
  }
}


LUA_API ثابت حرف *lua_getupvalue (lua_State *L, صحيح funcindex, صحيح n) {
  ثابت حرف *name;
  TValue *val = عدم;  /* to avoid warnings */
  lua_جندرا(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, عدم);
  إذا (name) {
    setobj2s(L, L->چوٹی.p, val);
    api_incr_چوٹی(L);
  }
  lua_unlock(L);
  أرجع name;
}


LUA_API ثابت حرف *lua_setupvalue (lua_State *L, صحيح funcindex, صحيح n) {
  ثابت حرف *name;
  TValue *val = عدم;  /* to avoid warnings */
  GCObject *owner = عدم;  /* to avoid warnings */
  TValue *fi;
  lua_جندرا(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  إذا (name) {
    L->چوٹی.p--;
    setobj(L, val, s2v(L->چوٹی.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  أرجع name;
}


ساكن اتے_Val **getupvalref (lua_State *L, صحيح fidx, صحيح n, LClosure **pf) {
  ساكن ثابت اتے_Val *ثابت nullup = عدم;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  إذا (pf) *pf = f;
  إذا (1 <= n && n <= f->p->sizeupvalues)
    أرجع &f->upvals[n - 1];  /* get its upvalue pointer */
  وإلا
    أرجع (اتے_Val**)&nullup;
}


LUA_API فراغ *lua_upvalueid (lua_State *L, صحيح fidx, صحيح n) {
  TValue *fi = index2value(L, fidx);
  بدل (ttypetag(fi)) {
    حالة LUA_VLCL: {  /* lua closure */
      أرجع *getupvalref(L, fidx, n, عدم);
    }
    حالة LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      إذا (1 <= n && n <= f->nupvalues)
        أرجع &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    حالة LUA_VLCF:
      أرجع عدم;  /* light C functions have no upvalues */
    افتراضي: {
      api_check(L, 0, "function expected");
      أرجع عدم;
    }
  }
}


LUA_API فراغ lua_upvaluejoin (lua_State *L, صحيح fidx1, صحيح n1,
                                            صحيح fidx2, صحيح n2) {
  LClosure *f1;
  اتے_Val **up1 = getupvalref(L, fidx1, n1, &f1);
  اتے_Val **up2 = getupvalref(L, fidx2, n2, عدم);
  api_check(L, *up1 != عدم && *up2 != عدم, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


