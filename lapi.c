/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lapi.h"
#অন্তর্ভাৱ "ldebug.h"
#অন্তর্ভাৱ "ldo.h"
#অন্তর্ভাৱ "lfunc.h"
#অন্তর্ভাৱ "lgc.h"
#অন্তর্ভাৱ "lmem.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lstate.h"
#অন্তর্ভাৱ "lstring.h"
#অন্তর্ভাৱ "ltable.h"
#অন্তর্ভাৱ "ltm.h"
#অন্তর্ভাৱ "lundump.h"
#অন্তর্ভাৱ "lvm.h"



ধ্রুৱ ৱর্ণ lua_ident[] =
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
স্থির TValue *index2value (lua_State *L, পূর্ণাঙ্ক idx) {
  CallInfo *ci = L->ci;
  যদি (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->শীৰ্ষ.p - (ci->func.p + 1), "unacceptable index");
    যদি (o >= L->শীৰ্ষ.p) ঘূৰাই &G(L)->nilvalue;
    অন্যথা ঘূৰাই s2v(o);
  }
  অন্যথা যদি (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->শীৰ্ষ.p - (ci->func.p + 1),
                 "invalid index");
    ঘূৰাই s2v(L->শীৰ্ষ.p + idx);
  }
  অন্যথা যদি (idx == LUA_REGISTRYINDEX)
    ঘূৰাই &G(L)->l_registry;
  অন্যথা {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    যদি (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      ঘূৰাই (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    অন্যথা {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      ঘূৰাই &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
স্থির StkId index2stack (lua_State *L, পূর্ণাঙ্ক idx) {
  CallInfo *ci = L->ci;
  যদি (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->শীৰ্ষ.p, "invalid index");
    ঘূৰাই o;
  }
  অন্যথা {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->শীৰ্ষ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    ঘূৰাই L->শীৰ্ষ.p + idx;
  }
}


LUA_API পূর্ণাঙ্ক lua_checkstack (lua_State *L, পূর্ণাঙ্ক n) {
  পূর্ণাঙ্ক res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  যদি (L->স্তূপ_last.p - L->শীৰ্ষ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  অন্যথা  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  যদি (res && ci->শীৰ্ষ.p < L->শীৰ্ষ.p + n)
    ci->শীৰ্ষ.p = L->শীৰ্ষ.p + n;  /* adjust frame top */
  lua_unlock(L);
  ঘূৰাই res;
}


LUA_API শূন্য lua_xmove (lua_State *from, lua_State *to, পূর্ণাঙ্ক n) {
  পূর্ণাঙ্ক i;
  যদি (from == to) ঘূৰাই;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->শীৰ্ষ.p - to->শীৰ্ষ.p >= n, "stack overflow");
  from->শীৰ্ষ.p -= n;
  বাবে (i = 0; i < n; i++) {
    setobjs2s(to, to->শীৰ্ষ.p, from->শীৰ্ষ.p + i);
    to->শীৰ্ষ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  ঘূৰাই old;
}


LUA_API lua_সংখ্যা lua_version (lua_State *L) {
  UNUSED(L);
  ঘূৰাই LUA_VERSION_অঙ্ক;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API পূর্ণাঙ্ক lua_absindex (lua_State *L, পূর্ণাঙ্ক idx) {
  ঘূৰাই (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->শীৰ্ষ.p - L->ci->func.p) + idx;
}


LUA_API পূর্ণাঙ্ক lua_gettop (lua_State *L) {
  ঘূৰাই cast_int(L->শীৰ্ষ.p - (L->ci->func.p + 1));
}


LUA_API শূন্য lua_settop (lua_State *L, পূর্ণাঙ্ক idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  যদি (idx >= 0) {
    api_check(L, idx <= ci->শীৰ্ষ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->শীৰ্ষ.p;
    বাবে (; diff > 0; diff--)
      setnilvalue2s(L->শীৰ্ষ.p++);  /* clear new slots */
  }
  অন্যথা {
    api_check(L, -(idx+1) <= (L->শীৰ্ষ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->শীৰ্ষ.p + diff;
  যদি (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->শীৰ্ষ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API শূন্য lua_closeslot (lua_State *L, পূর্ণাঙ্ক idx) {
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
স্থির শূন্য reverse (lua_State *L, StkId from, StkId to) {
  বাবে (; from < to; from++, to--) {
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
LUA_API শূন্য lua_rotate (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->শীৰ্ষ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API শূন্য lua_copy (lua_State *L, পূর্ণাঙ্ক fromidx, পূর্ণাঙ্ক toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  যদি (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API শূন্য lua_pushvalue (lua_State *L, পূর্ণাঙ্ক idx) {
  lua_lock(L);
  setobj2s(L, L->শীৰ্ষ.p, index2value(L, idx));
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API পূর্ণাঙ্ক lua_type (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ধ্রুৱ ৱর্ণ *lua_typename (lua_State *L, পূর্ণাঙ্ক t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  ঘূৰাই ttypename(t);
}


LUA_API পূর্ণাঙ্ক lua_iscfunction (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API পূর্ণাঙ্ক lua_isinteger (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই ttisinteger(o);
}


LUA_API পূর্ণাঙ্ক lua_isnumber (lua_State *L, পূর্ণাঙ্ক idx) {
  lua_সংখ্যা n;
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই tonumber(o, &n);
}


LUA_API পূর্ণাঙ্ক lua_isstring (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই (ttisstring(o) || cvt2str(o));
}


LUA_API পূর্ণাঙ্ক lua_isuserdata (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API পূর্ণাঙ্ক lua_rawequal (lua_State *L, পূর্ণাঙ্ক index1, পূর্ণাঙ্ক index2) {
  ধ্রুৱ TValue *o1 = index2value(L, index1);
  ধ্রুৱ TValue *o2 = index2value(L, index2);
  ঘূৰাই (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API শূন্য lua_arith (lua_State *L, পূর্ণাঙ্ক op) {
  lua_lock(L);
  যদি (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  অন্যথা {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->শীৰ্ষ.p, L->শীৰ্ষ.p - 1);
    api_incr_শীৰ্ষ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->শীৰ্ষ.p - 2), s2v(L->শীৰ্ষ.p - 1), L->শীৰ্ষ.p - 2);
  L->শীৰ্ষ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API পূর্ণাঙ্ক lua_compare (lua_State *L, পূর্ণাঙ্ক index1, পূর্ণাঙ্ক index2, পূর্ণাঙ্ক op) {
  ধ্রুৱ TValue *o1;
  ধ্রুৱ TValue *o2;
  পূর্ণাঙ্ক i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  যদি (isvalid(L, o1) && isvalid(L, o2)) {
    ৱিকল্প (op) {
      পক্ষ LUA_OPEQ: i = luaV_equalobj(L, o1, o2); ভঙ্গ;
      পক্ষ LUA_OPLT: i = luaV_lessthan(L, o1, o2); ভঙ্গ;
      পক্ষ LUA_OPLE: i = luaV_lessequal(L, o1, o2); ভঙ্গ;
      স্ৱভাৱ: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  ঘূৰাই i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, পূর্ণাঙ্ক idx, ৱর্ণ *buff) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  যদি (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    ঘূৰাই len;
  }
  অন্যথা
    ঘূৰাই 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ধ্রুৱ ৱর্ণ *s) {
  size_t sz = luaO_str2num(s, s2v(L->শীৰ্ষ.p));
  যদি (sz != 0)
    api_incr_শীৰ্ষ(L);
  ঘূৰাই sz;
}


LUA_API lua_সংখ্যা lua_tonumberx (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক *pisnum) {
  lua_সংখ্যা n = 0;
  ধ্রুৱ TValue *o = index2value(L, idx);
  পূর্ণাঙ্ক isnum = tonumber(o, &n);
  যদি (pisnum)
    *pisnum = isnum;
  ঘূৰাই n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক *pisnum) {
  lua_Integer res = 0;
  ধ্রুৱ TValue *o = index2value(L, idx);
  পূর্ণাঙ্ক isnum = tointeger(o, &res);
  যদি (pisnum)
    *pisnum = isnum;
  ঘূৰাই res;
}


LUA_API পূর্ণাঙ্ক lua_toboolean (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই !l_isfalse(o);
}


LUA_API ধ্রুৱ ৱর্ণ *lua_tolstring (lua_State *L, পূর্ণাঙ্ক idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  যদি (!ttisstring(o)) {
    যদি (!cvt2str(o)) {  /* not convertible? */
      যদি (len != রিক্ত) *len = 0;
      lua_unlock(L);
      ঘূৰাই রিক্ত;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  যদি (len != রিক্ত)
    ঘূৰাই getlstr(tsvalue(o), *len);
  অন্যথা
    ঘূৰাই getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ৱিকল্প (ttypetag(o)) {
    পক্ষ LUA_VSHRSTR: ঘূৰাই cast(lua_Unsigned, tsvalue(o)->shrlen);
    পক্ষ LUA_VLNGSTR: ঘূৰাই cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    পক্ষ LUA_VUSERDATA: ঘূৰাই cast(lua_Unsigned, uvalue(o)->len);
    পক্ষ LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      ঘূৰাই res;
    }
    স্ৱভাৱ: ঘূৰাই 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  যদি (ttislcf(o)) ঘূৰাই fvalue(o);
  অন্যথা যদি (ttisCclosure(o))
    ঘূৰাই clCvalue(o)->f;
  অন্যথা ঘূৰাই রিক্ত;  /* not a C function */
}


l_sinline শূন্য *touserdata (ধ্রুৱ TValue *o) {
  ৱিকল্প (ttype(o)) {
    পক্ষ LUA_TUSERDATA: ঘূৰাই getudatamem(uvalue(o));
    পক্ষ LUA_TLIGHTUSERDATA: ঘূৰাই pvalue(o);
    স্ৱভাৱ: ঘূৰাই রিক্ত;
  }
}


LUA_API শূন্য *lua_touserdata (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ঘূৰাই (!ttisthread(o)) ? রিক্ত : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ধ্রুৱ শূন্য *lua_topointer (lua_State *L, পূর্ণাঙ্ক idx) {
  ধ্রুৱ TValue *o = index2value(L, idx);
  ৱিকল্প (ttypetag(o)) {
    পক্ষ LUA_VLCF: ঘূৰাই cast_voidp(cast_sizet(fvalue(o)));
    পক্ষ LUA_VUSERDATA: পক্ষ LUA_VLIGHTUSERDATA:
      ঘূৰাই touserdata(o);
    স্ৱভাৱ: {
      যদি (iscollectable(o))
        ঘূৰাই gcvalue(o);
      অন্যথা
        ঘূৰাই রিক্ত;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API শূন্য lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->শীৰ্ষ.p);
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
}


LUA_API শূন্য lua_pushnumber (lua_State *L, lua_সংখ্যা n) {
  lua_lock(L);
  setfltvalue(s2v(L->শীৰ্ষ.p), n);
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
}


LUA_API শূন্য lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->শীৰ্ষ.p), n);
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ধ্রুৱ ৱর্ণ *lua_pushlstring (lua_State *L, ধ্রুৱ ৱর্ণ *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->শীৰ্ষ.p, ts);
  api_incr_শীৰ্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ঘূৰাই getstr(ts);
}


LUA_API ধ্রুৱ ৱর্ণ *lua_pushexternalstring (lua_State *L,
	        ধ্রুৱ ৱর্ণ *s, size_t len, lua_Alloc falloc, শূন্য *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->শীৰ্ষ.p, ts);
  api_incr_শীৰ্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ঘূৰাই getstr(ts);
}


LUA_API ধ্রুৱ ৱর্ণ *lua_pushstring (lua_State *L, ধ্রুৱ ৱর্ণ *s) {
  lua_lock(L);
  যদি (s == রিক্ত)
    setnilvalue2s(L->শীৰ্ষ.p);
  অন্যথা {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->শীৰ্ষ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_শীৰ্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ঘূৰাই s;
}


LUA_API ধ্রুৱ ৱর্ণ *lua_pushvfstring (lua_State *L, ধ্রুৱ ৱর্ণ *fmt,
                                      va_list argp) {
  ধ্রুৱ ৱর্ণ *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  ঘূৰাই ret;
}


LUA_API ধ্রুৱ ৱর্ণ *lua_pushfstring (lua_State *L, ধ্রুৱ ৱর্ণ *fmt, ...) {
  ধ্রুৱ ৱর্ণ *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  ঘূৰাই ret;
}


LUA_API শূন্য lua_pushcclosure (lua_State *L, lua_CFunction fn, পূর্ণাঙ্ক n) {
  lua_lock(L);
  যদি (n == 0) {
    setfvalue(s2v(L->শীৰ্ষ.p), fn);
    api_incr_শীৰ্ষ(L);
  }
  অন্যথা {
    পূর্ণাঙ্ক i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    বাবে (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->শীৰ্ষ.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->শীৰ্ষ.p -= n;
    setclCvalue(L, s2v(L->শীৰ্ষ.p), cl);
    api_incr_শীৰ্ষ(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API শূন্য lua_pushboolean (lua_State *L, পূর্ণাঙ্ক b) {
  lua_lock(L);
  যদি (b)
    setbtvalue(s2v(L->শীৰ্ষ.p));
  অন্যথা
    setbfvalue(s2v(L->শীৰ্ষ.p));
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
}


LUA_API শূন্য lua_pushlightuserdata (lua_State *L, শূন্য *p) {
  lua_lock(L);
  setpvalue(s2v(L->শীৰ্ষ.p), p);
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
}


LUA_API পূর্ণাঙ্ক lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->শীৰ্ষ.p), L);
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
  ঘূৰাই (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


স্থির পূর্ণাঙ্ক auxgetstr (lua_State *L, ধ্রুৱ TValue *t, ধ্রুৱ ৱর্ণ *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->শীৰ্ষ.p), luaH_getstr, tag);
  যদি (!tagisempty(tag))
    api_incr_শীৰ্ষ(L);
  অন্যথা {
    setsvalue2s(L, L->শীৰ্ষ.p, str);
    api_incr_শীৰ্ষ(L);
    tag = luaV_finishget(L, t, s2v(L->শীৰ্ষ.p - 1), L->শীৰ্ষ.p - 1, tag);
  }
  lua_unlock(L);
  ঘূৰাই novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
স্থির শূন্য getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (শূন্য)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API পূর্ণাঙ্ক lua_getglobal (lua_State *L, ধ্রুৱ ৱর্ণ *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  ঘূৰাই auxgetstr(L, &gt, name);
}


LUA_API পূর্ণাঙ্ক lua_gettable (lua_State *L, পূর্ণাঙ্ক idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->শীৰ্ষ.p - 1), s2v(L->শীৰ্ষ.p - 1), luaH_get, tag);
  যদি (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->শীৰ্ষ.p - 1), L->শীৰ্ষ.p - 1, tag);
  lua_unlock(L);
  ঘূৰাই novariant(tag);
}


LUA_API পূর্ণাঙ্ক lua_getfield (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ ৱর্ণ *k) {
  lua_lock(L);
  ঘূৰাই auxgetstr(L, index2value(L, idx), k);
}


LUA_API পূর্ণাঙ্ক lua_geti (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->শীৰ্ষ.p), tag);
  যদি (tagisempty(tag)) {
    TValue চাবি;
    setivalue(&চাবি, n);
    tag = luaV_finishget(L, t, &চাবি, L->শীৰ্ষ.p, tag);
  }
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
  ঘূৰাই novariant(tag);
}


স্থির পূর্ণাঙ্ক finishrawget (lua_State *L, lu_byte tag) {
  যদি (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->শীৰ্ষ.p);
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
  ঘূৰাই novariant(tag);
}


l_sinline Table *gettable (lua_State *L, পূর্ণাঙ্ক idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  ঘূৰাই hvalue(t);
}


LUA_API পূর্ণাঙ্ক lua_rawget (lua_State *L, পূর্ণাঙ্ক idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->শীৰ্ষ.p - 1), s2v(L->শীৰ্ষ.p - 1));
  L->শীৰ্ষ.p--;  /* pop key */
  ঘূৰাই finishrawget(L, tag);
}


LUA_API পূর্ণাঙ্ক lua_rawgeti (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->শীৰ্ষ.p), tag);
  ঘূৰাই finishrawget(L, tag);
}


LUA_API পূর্ণাঙ্ক lua_rawgetp (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ শূন্য *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  ঘূৰাই finishrawget(L, luaH_get(t, &k, s2v(L->শীৰ্ষ.p)));
}


LUA_API শূন্য lua_createtable (lua_State *L, পূর্ণাঙ্ক narray, পূর্ণাঙ্ক nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->শীৰ্ষ.p, t);
  api_incr_শীৰ্ষ(L);
  যদি (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API পূর্ণাঙ্ক lua_getmetatable (lua_State *L, পূর্ণাঙ্ক objindex) {
  ধ্রুৱ TValue *obj;
  Table *mt;
  পূর্ণাঙ্ক res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  ৱিকল্প (ttype(obj)) {
    পক্ষ LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      ভঙ্গ;
    পক্ষ LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      ভঙ্গ;
    স্ৱভাৱ:
      mt = G(L)->mt[ttype(obj)];
      ভঙ্গ;
  }
  যদি (mt != রিক্ত) {
    sethvalue2s(L, L->শীৰ্ষ.p, mt);
    api_incr_শীৰ্ষ(L);
    res = 1;
  }
  lua_unlock(L);
  ঘূৰাই res;
}


LUA_API পূর্ণাঙ্ক lua_getiuservalue (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক n) {
  TValue *o;
  পূর্ণাঙ্ক t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  যদি (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->শীৰ্ষ.p);
    t = LUA_TNONE;
  }
  অন্যথা {
    setobj2s(L, L->শীৰ্ষ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->শীৰ্ষ.p));
  }
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
  ঘূৰাই t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
স্থির শূন্য auxsetstr (lua_State *L, ধ্রুৱ TValue *t, ধ্রুৱ ৱর্ণ *k) {
  পূর্ণাঙ্ক hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->শীৰ্ষ.p - 1), hres, luaH_psetstr);
  যদি (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->শীৰ্ষ.p - 1));
    L->শীৰ্ষ.p--;  /* pop value */
  }
  অন্যথা {
    setsvalue2s(L, L->শীৰ্ষ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_শীৰ্ষ(L);
    luaV_finishset(L, t, s2v(L->শীৰ্ষ.p - 1), s2v(L->শীৰ্ষ.p - 2), hres);
    L->শীৰ্ষ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API শূন্য lua_setglobal (lua_State *L, ধ্রুৱ ৱর্ণ *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API শূন্য lua_settable (lua_State *L, পূর্ণাঙ্ক idx) {
  TValue *t;
  পূর্ণাঙ্ক hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->শীৰ্ষ.p - 2), s2v(L->শীৰ্ষ.p - 1), hres, luaH_pset);
  যদি (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->শীৰ্ষ.p - 1));
  অন্যথা
    luaV_finishset(L, t, s2v(L->শীৰ্ষ.p - 2), s2v(L->শীৰ্ষ.p - 1), hres);
  L->শীৰ্ষ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API শূন্য lua_setfield (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ ৱর্ণ *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API শূন্য lua_seti (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n) {
  TValue *t;
  পূর্ণাঙ্ক hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->শীৰ্ষ.p - 1), hres);
  যদি (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->শীৰ্ষ.p - 1));
  অন্যথা {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->শীৰ্ষ.p - 1), hres);
  }
  L->শীৰ্ষ.p--;  /* pop value */
  lua_unlock(L);
}


স্থির শূন্য aux_rawset (lua_State *L, পূর্ণাঙ্ক idx, TValue *চাবি, পূর্ণাঙ্ক n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, চাবি, s2v(L->শীৰ্ষ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->শীৰ্ষ.p - 1));
  L->শীৰ্ষ.p -= n;
  lua_unlock(L);
}


LUA_API শূন্য lua_rawset (lua_State *L, পূর্ণাঙ্ক idx) {
  aux_rawset(L, idx, s2v(L->শীৰ্ষ.p - 2), 2);
}


LUA_API শূন্য lua_rawsetp (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ শূন্য *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API শূন্য lua_rawseti (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->শীৰ্ষ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->শীৰ্ষ.p - 1));
  L->শীৰ্ষ.p--;
  lua_unlock(L);
}


LUA_API পূর্ণাঙ্ক lua_setmetatable (lua_State *L, পূর্ণাঙ্ক objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  যদি (ttisnil(s2v(L->শীৰ্ষ.p - 1)))
    mt = রিক্ত;
  অন্যথা {
    api_check(L, ttistable(s2v(L->শীৰ্ষ.p - 1)), "table expected");
    mt = hvalue(s2v(L->শীৰ্ষ.p - 1));
  }
  ৱিকল্প (ttype(obj)) {
    পক্ষ LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      যদি (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ভঙ্গ;
    }
    পক্ষ LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      যদি (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ভঙ্গ;
    }
    স্ৱভাৱ: {
      G(L)->mt[ttype(obj)] = mt;
      ভঙ্গ;
    }
  }
  L->শীৰ্ষ.p--;
  lua_unlock(L);
  ঘূৰাই 1;
}


LUA_API পূর্ণাঙ্ক lua_setiuservalue (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক n) {
  TValue *o;
  পূর্ণাঙ্ক res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  যদি (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  অন্যথা {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->শীৰ্ষ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->শীৰ্ষ.p - 1));
    res = 1;
  }
  L->শীৰ্ষ.p--;
  lua_unlock(L);
  ঘূৰাই res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->শীৰ্ষ.p - L->শীৰ্ষ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API শূন্য lua_callk (lua_State *L, পূর্ণাঙ্ক nargs, পূর্ণাঙ্ক nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == রিক্ত || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->শীৰ্ষ.p - (nargs+1);
  যদি (k != রিক্ত && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  অন্যথা  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
সংরচনা CallS {  /* data to 'f_call' */
  StkId func;
  পূর্ণাঙ্ক nresults;
};


স্থির শূন্য f_call (lua_State *L, শূন্য *ud) {
  সংরচনা CallS *c = cast(সংরচনা CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API পূর্ণাঙ্ক lua_pcallk (lua_State *L, পূর্ণাঙ্ক nargs, পূর্ণাঙ্ক nresults, পূর্ণাঙ্ক errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  সংরচনা CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == রিক্ত || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  যদি (errfunc == 0)
    func = 0;
  অন্যথা {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->শীৰ্ষ.p - (nargs+1);  /* function to be called */
  যদি (k == রিক্ত || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  অন্যথা {  /* prepare continuation (call is already protected by 'resume') */
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
  ঘূৰাই APIstatus(status);
}


LUA_API পূর্ণাঙ্ক lua_load (lua_State *L, lua_Reader reader, শূন্য *data,
                      ধ্রুৱ ৱর্ণ *chunkname, ধ্রুৱ ৱর্ণ *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  যদি (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  যদি (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->শীৰ্ষ.p - 1));  /* get new function */
    যদি (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  ঘূৰাই APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API পূর্ণাঙ্ক lua_dump (lua_State *L, lua_Writer writer, শূন্য *data, পূর্ণাঙ্ক strip) {
  পূর্ণাঙ্ক status;
  ptrdiff_t otop = savestack(L, L->শীৰ্ষ.p);  /* original top */
  TValue *f = s2v(L->শীৰ্ষ.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->শীৰ্ষ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  ঘূৰাই status;
}


LUA_API পূর্ণাঙ্ক lua_status (lua_State *L) {
  ঘূৰাই APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API পূর্ণাঙ্ক lua_gc (lua_State *L, পূর্ণাঙ্ক what, ...) {
  va_list argp;
  পূর্ণাঙ্ক res = 0;
  global_State *g = G(L);
  যদি (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    ঘূৰাই -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  ৱিকল্প (what) {
    পক্ষ LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      ভঙ্গ;
    }
    পক্ষ LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      ভঙ্গ;
    }
    পক্ষ LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      ভঙ্গ;
    }
    পক্ষ LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      ভঙ্গ;
    }
    পক্ষ LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      ভঙ্গ;
    }
    পক্ষ LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      পূর্ণাঙ্ক work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      যদি (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      অন্যথা যদি (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      অন্যথা  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (শূন্য)0, work = 1);
      যদি (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      ভঙ্গ;
    }
    পক্ষ LUA_GCISRUNNING: {
      res = gcrunning(g);
      ভঙ্গ;
    }
    পক্ষ LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      ভঙ্গ;
    }
    পক্ষ LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      ভঙ্গ;
    }
    পক্ষ LUA_GCPARAM: {
      পূর্ণাঙ্ক param = va_arg(argp, পূর্ণাঙ্ক);
      পূর্ণাঙ্ক মান = va_arg(argp, পূর্ণাঙ্ক);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      যদি (মান >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(মান));
      ভঙ্গ;
    }
    স্ৱভাৱ: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  ঘূৰাই res;
}



/*
** miscellaneous functions
*/


LUA_API পূর্ণাঙ্ক lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->শীৰ্ষ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  যদি (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  অন্যথা
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  ঘূৰাই 0;  /* to avoid warnings */
}


LUA_API পূর্ণাঙ্ক lua_next (lua_State *L, পূর্ণাঙ্ক idx) {
  Table *t;
  পূর্ণাঙ্ক more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->শীৰ্ষ.p - 1);
  যদি (more)
    api_incr_শীৰ্ষ(L);
  অন্যথা  /* no more elements */
    L->শীৰ্ষ.p--;  /* pop key */
  lua_unlock(L);
  ঘূৰাই more;
}


LUA_API শূন্য lua_toclose (lua_State *L, পূর্ণাঙ্ক idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API শূন্য lua_concat (lua_State *L, পূর্ণাঙ্ক n) {
  lua_lock(L);
  যদি (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  অন্যথা {  /* nothing to concatenate */
    setsvalue2s(L, L->শীৰ্ষ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_শীৰ্ষ(L);
  }
  lua_unlock(L);
}


LUA_API শূন্য lua_len (lua_State *L, পূর্ণাঙ্ক idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->শীৰ্ষ.p, t);
  api_incr_শীৰ্ষ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, শূন্য **ud) {
  lua_Alloc f;
  lua_lock(L);
  যদি (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  ঘূৰাই f;
}


LUA_API শূন্য lua_setallocf (lua_State *L, lua_Alloc f, শূন্য *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


শূন্য lua_setwarnf (lua_State *L, lua_WarnFunction f, শূন্য *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


শূন্য lua_warning (lua_State *L, ধ্রুৱ ৱর্ণ *বাৰ্তা, পূর্ণাঙ্ক tocont) {
  lua_lock(L);
  luaE_warning(L, বাৰ্তা, tocont);
  lua_unlock(L);
}



LUA_API শূন্য *lua_newuserdatauv (lua_State *L, size_t size, পূর্ণাঙ্ক nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->শীৰ্ষ.p), u);
  api_incr_শীৰ্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ঘূৰাই getudatamem(u);
}



স্থির ধ্রুৱ ৱর্ণ *aux_upvalue (TValue *fi, পূর্ণাঙ্ক n, TValue **val,
                                GCObject **owner) {
  ৱিকল্প (ttypetag(fi)) {
    পক্ষ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      যদি (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        ঘূৰাই রিক্ত;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      যদি (owner) *owner = obj2gco(f);
      ঘূৰাই "";
    }
    পক্ষ LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      যদি (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        ঘূৰাই রিক্ত;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      যদি (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      ঘূৰাই (name == রিক্ত) ? "(no name)" : getstr(name);
    }
    স্ৱভাৱ: ঘূৰাই রিক্ত;  /* not a closure */
  }
}


LUA_API ধ্রুৱ ৱর্ণ *lua_getupvalue (lua_State *L, পূর্ণাঙ্ক funcindex, পূর্ণাঙ্ক n) {
  ধ্রুৱ ৱর্ণ *name;
  TValue *val = রিক্ত;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, রিক্ত);
  যদি (name) {
    setobj2s(L, L->শীৰ্ষ.p, val);
    api_incr_শীৰ্ষ(L);
  }
  lua_unlock(L);
  ঘূৰাই name;
}


LUA_API ধ্রুৱ ৱর্ণ *lua_setupvalue (lua_State *L, পূর্ণাঙ্ক funcindex, পূর্ণাঙ্ক n) {
  ধ্রুৱ ৱর্ণ *name;
  TValue *val = রিক্ত;  /* to avoid warnings */
  GCObject *owner = রিক্ত;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  যদি (name) {
    L->শীৰ্ষ.p--;
    setobj(L, val, s2v(L->শীৰ্ষ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  ঘূৰাই name;
}


স্থির UpVal **getupvalref (lua_State *L, পূর্ণাঙ্ক fidx, পূর্ণাঙ্ক n, LClosure **pf) {
  স্থির ধ্রুৱ UpVal *ধ্রুৱ nullup = রিক্ত;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  যদি (pf) *pf = f;
  যদি (1 <= n && n <= f->p->sizeupvalues)
    ঘূৰাই &f->upvals[n - 1];  /* get its upvalue pointer */
  অন্যথা
    ঘূৰাই (UpVal**)&nullup;
}


LUA_API শূন্য *lua_upvalueid (lua_State *L, পূর্ণাঙ্ক fidx, পূর্ণাঙ্ক n) {
  TValue *fi = index2value(L, fidx);
  ৱিকল্প (ttypetag(fi)) {
    পক্ষ LUA_VLCL: {  /* lua closure */
      ঘূৰাই *getupvalref(L, fidx, n, রিক্ত);
    }
    পক্ষ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      যদি (1 <= n && n <= f->nupvalues)
        ঘূৰাই &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    পক্ষ LUA_VLCF:
      ঘূৰাই রিক্ত;  /* light C functions have no upvalues */
    স্ৱভাৱ: {
      api_check(L, 0, "function expected");
      ঘূৰাই রিক্ত;
    }
  }
}


LUA_API শূন্য lua_upvaluejoin (lua_State *L, পূর্ণাঙ্ক fidx1, পূর্ণাঙ্ক n1,
                                            পূর্ণাঙ্ক fidx2, পূর্ণাঙ্ক n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, রিক্ত);
  api_check(L, *up1 != রিক্ত && *up2 != রিক্ত, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


