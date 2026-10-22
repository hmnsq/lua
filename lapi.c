/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lapi.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "ltm.h"
#অন্তর্ভাব "lundump.h"
#অন্তর্ভাব "lvm.h"



ধ্রুবক অক্ষর lua_ident[] =
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
স্থির TValue *index2value (lua_State *L, পূর্ণসংখ্যা idx) {
  CallInfo *ci = L->ci;
  যদি (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->শীর্ষ.p - (ci->func.p + 1), "unacceptable index");
    যদি (o >= L->শীর্ষ.p) ফেরত &G(L)->nilvalue;
    অন্যথা ফেরত s2v(o);
  }
  অন্যথা যদি (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->শীর্ষ.p - (ci->func.p + 1),
                 "invalid index");
    ফেরত s2v(L->শীর্ষ.p + idx);
  }
  অন্যথা যদি (idx == LUA_REGISTRYINDEX)
    ফেরত &G(L)->l_registry;
  অন্যথা {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    যদি (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      ফেরত (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    অন্যথা {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      ফেরত &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
স্থির StkId index2stack (lua_State *L, পূর্ণসংখ্যা idx) {
  CallInfo *ci = L->ci;
  যদি (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->শীর্ষ.p, "invalid index");
    ফেরত o;
  }
  অন্যথা {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->শীর্ষ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    ফেরত L->শীর্ষ.p + idx;
  }
}


LUA_API পূর্ণসংখ্যা lua_checkstack (lua_State *L, পূর্ণসংখ্যা n) {
  পূর্ণসংখ্যা res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  যদি (L->স্তূপ_last.p - L->শীর্ষ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  অন্যথা  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  যদি (res && ci->শীর্ষ.p < L->শীর্ষ.p + n)
    ci->শীর্ষ.p = L->শীর্ষ.p + n;  /* adjust frame top */
  lua_unlock(L);
  ফেরত res;
}


LUA_API শূন্য lua_xmove (lua_State *from, lua_State *to, পূর্ণসংখ্যা n) {
  পূর্ণসংখ্যা i;
  যদি (from == to) ফেরত;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->শীর্ষ.p - to->শীর্ষ.p >= n, "stack overflow");
  from->শীর্ষ.p -= n;
  জন্য (i = 0; i < n; i++) {
    setobjs2s(to, to->শীর্ষ.p, from->শীর্ষ.p + i);
    to->শীর্ষ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  ফেরত old;
}


LUA_API lua_সংখ্যা lua_version (lua_State *L) {
  UNUSED(L);
  ফেরত LUA_VERSION_অঙ্ক;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API পূর্ণসংখ্যা lua_absindex (lua_State *L, পূর্ণসংখ্যা idx) {
  ফেরত (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->শীর্ষ.p - L->ci->func.p) + idx;
}


LUA_API পূর্ণসংখ্যা lua_gettop (lua_State *L) {
  ফেরত cast_int(L->শীর্ষ.p - (L->ci->func.p + 1));
}


LUA_API শূন্য lua_settop (lua_State *L, পূর্ণসংখ্যা idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  যদি (idx >= 0) {
    api_check(L, idx <= ci->শীর্ষ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->শীর্ষ.p;
    জন্য (; diff > 0; diff--)
      setnilvalue2s(L->শীর্ষ.p++);  /* clear new slots */
  }
  অন্যথা {
    api_check(L, -(idx+1) <= (L->শীর্ষ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->শীর্ষ.p + diff;
  যদি (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->শীর্ষ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API শূন্য lua_closeslot (lua_State *L, পূর্ণসংখ্যা idx) {
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
  জন্য (; from < to; from++, to--) {
    TValue অস্থায়ী;
    setobj(L, &অস্থায়ী, s2v(from));
    setobjs2s(L, from, to);
    setobj2s(L, to, &অস্থায়ী);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API শূন্য lua_rotate (lua_State *L, পূর্ণসংখ্যা idx, পূর্ণসংখ্যা n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->শীর্ষ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API শূন্য lua_copy (lua_State *L, পূর্ণসংখ্যা fromidx, পূর্ণসংখ্যা toidx) {
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


LUA_API শূন্য lua_pushvalue (lua_State *L, পূর্ণসংখ্যা idx) {
  lua_lock(L);
  setobj2s(L, L->শীর্ষ.p, index2value(L, idx));
  api_incr_শীর্ষ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API পূর্ণসংখ্যা lua_type (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ধ্রুবক অক্ষর *lua_typename (lua_State *L, পূর্ণসংখ্যা t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  ফেরত ttypename(t);
}


LUA_API পূর্ণসংখ্যা lua_iscfunction (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API পূর্ণসংখ্যা lua_isinteger (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত ttisinteger(o);
}


LUA_API পূর্ণসংখ্যা lua_isnumber (lua_State *L, পূর্ণসংখ্যা idx) {
  lua_সংখ্যা n;
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত tonumber(o, &n);
}


LUA_API পূর্ণসংখ্যা lua_isstring (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত (ttisstring(o) || cvt2str(o));
}


LUA_API পূর্ণসংখ্যা lua_isuserdata (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API পূর্ণসংখ্যা lua_rawequal (lua_State *L, পূর্ণসংখ্যা index1, পূর্ণসংখ্যা index2) {
  ধ্রুবক TValue *o1 = index2value(L, index1);
  ধ্রুবক TValue *o2 = index2value(L, index2);
  ফেরত (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API শূন্য lua_arith (lua_State *L, পূর্ণসংখ্যা op) {
  lua_lock(L);
  যদি (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  অন্যথা {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->শীর্ষ.p, L->শীর্ষ.p - 1);
    api_incr_শীর্ষ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->শীর্ষ.p - 2), s2v(L->শীর্ষ.p - 1), L->শীর্ষ.p - 2);
  L->শীর্ষ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API পূর্ণসংখ্যা lua_compare (lua_State *L, পূর্ণসংখ্যা index1, পূর্ণসংখ্যা index2, পূর্ণসংখ্যা op) {
  ধ্রুবক TValue *o1;
  ধ্রুবক TValue *o2;
  পূর্ণসংখ্যা i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  যদি (isvalid(L, o1) && isvalid(L, o2)) {
    নির্বাচন (op) {
      ক্ষেত্র LUA_OPEQ: i = luaV_equalobj(L, o1, o2); বিরতি;
      ক্ষেত্র LUA_OPLT: i = luaV_lessthan(L, o1, o2); বিরতি;
      ক্ষেত্র LUA_OPLE: i = luaV_lessequal(L, o1, o2); বিরতি;
      পূর্বনির্ধারিত: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  ফেরত i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, পূর্ণসংখ্যা idx, অক্ষর *buff) {
  ধ্রুবক TValue *o = index2value(L, idx);
  যদি (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    ফেরত len;
  }
  অন্যথা
    ফেরত 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ধ্রুবক অক্ষর *s) {
  size_t sz = luaO_str2num(s, s2v(L->শীর্ষ.p));
  যদি (sz != 0)
    api_incr_শীর্ষ(L);
  ফেরত sz;
}


LUA_API lua_সংখ্যা lua_tonumberx (lua_State *L, পূর্ণসংখ্যা idx, পূর্ণসংখ্যা *pisnum) {
  lua_সংখ্যা n = 0;
  ধ্রুবক TValue *o = index2value(L, idx);
  পূর্ণসংখ্যা isnum = tonumber(o, &n);
  যদি (pisnum)
    *pisnum = isnum;
  ফেরত n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, পূর্ণসংখ্যা idx, পূর্ণসংখ্যা *pisnum) {
  lua_Integer res = 0;
  ধ্রুবক TValue *o = index2value(L, idx);
  পূর্ণসংখ্যা isnum = tointeger(o, &res);
  যদি (pisnum)
    *pisnum = isnum;
  ফেরত res;
}


LUA_API পূর্ণসংখ্যা lua_toboolean (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত !l_isfalse(o);
}


LUA_API ধ্রুবক অক্ষর *lua_tolstring (lua_State *L, পূর্ণসংখ্যা idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  যদি (!ttisstring(o)) {
    যদি (!cvt2str(o)) {  /* not convertible? */
      যদি (len != ফাঁকা) *len = 0;
      lua_unlock(L);
      ফেরত ফাঁকা;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  যদি (len != ফাঁকা)
    ফেরত getlstr(tsvalue(o), *len);
  অন্যথা
    ফেরত getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  নির্বাচন (ttypetag(o)) {
    ক্ষেত্র LUA_VSHRSTR: ফেরত cast(lua_Unsigned, tsvalue(o)->shrlen);
    ক্ষেত্র LUA_VLNGSTR: ফেরত cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    ক্ষেত্র LUA_VUSERDATA: ফেরত cast(lua_Unsigned, uvalue(o)->len);
    ক্ষেত্র LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      ফেরত res;
    }
    পূর্বনির্ধারিত: ফেরত 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  যদি (ttislcf(o)) ফেরত fvalue(o);
  অন্যথা যদি (ttisCclosure(o))
    ফেরত clCvalue(o)->f;
  অন্যথা ফেরত ফাঁকা;  /* not a C function */
}


l_sinline শূন্য *touserdata (ধ্রুবক TValue *o) {
  নির্বাচন (ttype(o)) {
    ক্ষেত্র LUA_TUSERDATA: ফেরত getudatamem(uvalue(o));
    ক্ষেত্র LUA_TLIGHTUSERDATA: ফেরত pvalue(o);
    পূর্বনির্ধারিত: ফেরত ফাঁকা;
  }
}


LUA_API শূন্য *lua_touserdata (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  ফেরত (!ttisthread(o)) ? ফাঁকা : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ধ্রুবক শূন্য *lua_topointer (lua_State *L, পূর্ণসংখ্যা idx) {
  ধ্রুবক TValue *o = index2value(L, idx);
  নির্বাচন (ttypetag(o)) {
    ক্ষেত্র LUA_VLCF: ফেরত cast_voidp(cast_sizet(fvalue(o)));
    ক্ষেত্র LUA_VUSERDATA: ক্ষেত্র LUA_VLIGHTUSERDATA:
      ফেরত touserdata(o);
    পূর্বনির্ধারিত: {
      যদি (iscollectable(o))
        ফেরত gcvalue(o);
      অন্যথা
        ফেরত ফাঁকা;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API শূন্য lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->শীর্ষ.p);
  api_incr_শীর্ষ(L);
  lua_unlock(L);
}


LUA_API শূন্য lua_pushnumber (lua_State *L, lua_সংখ্যা n) {
  lua_lock(L);
  setfltvalue(s2v(L->শীর্ষ.p), n);
  api_incr_শীর্ষ(L);
  lua_unlock(L);
}


LUA_API শূন্য lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->শীর্ষ.p), n);
  api_incr_শীর্ষ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ধ্রুবক অক্ষর *lua_pushlstring (lua_State *L, ধ্রুবক অক্ষর *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->শীর্ষ.p, ts);
  api_incr_শীর্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ফেরত getstr(ts);
}


LUA_API ধ্রুবক অক্ষর *lua_pushexternalstring (lua_State *L,
	        ধ্রুবক অক্ষর *s, size_t len, lua_Alloc falloc, শূন্য *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_আকার, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->শীর্ষ.p, ts);
  api_incr_শীর্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ফেরত getstr(ts);
}


LUA_API ধ্রুবক অক্ষর *lua_pushstring (lua_State *L, ধ্রুবক অক্ষর *s) {
  lua_lock(L);
  যদি (s == ফাঁকা)
    setnilvalue2s(L->শীর্ষ.p);
  অন্যথা {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->শীর্ষ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_শীর্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ফেরত s;
}


LUA_API ধ্রুবক অক্ষর *lua_pushvfstring (lua_State *L, ধ্রুবক অক্ষর *fmt,
                                      va_list argp) {
  ধ্রুবক অক্ষর *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  ফেরত ret;
}


LUA_API ধ্রুবক অক্ষর *lua_pushfstring (lua_State *L, ধ্রুবক অক্ষর *fmt, ...) {
  ধ্রুবক অক্ষর *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  ফেরত ret;
}


LUA_API শূন্য lua_pushcclosure (lua_State *L, lua_CFunction fn, পূর্ণসংখ্যা n) {
  lua_lock(L);
  যদি (n == 0) {
    setfvalue(s2v(L->শীর্ষ.p), fn);
    api_incr_শীর্ষ(L);
  }
  অন্যথা {
    পূর্ণসংখ্যা i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    জন্য (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->শীর্ষ.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->শীর্ষ.p -= n;
    setclCvalue(L, s2v(L->শীর্ষ.p), cl);
    api_incr_শীর্ষ(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API শূন্য lua_pushboolean (lua_State *L, পূর্ণসংখ্যা b) {
  lua_lock(L);
  যদি (b)
    setbtvalue(s2v(L->শীর্ষ.p));
  অন্যথা
    setbfvalue(s2v(L->শীর্ষ.p));
  api_incr_শীর্ষ(L);
  lua_unlock(L);
}


LUA_API শূন্য lua_pushlightuserdata (lua_State *L, শূন্য *p) {
  lua_lock(L);
  setpvalue(s2v(L->শীর্ষ.p), p);
  api_incr_শীর্ষ(L);
  lua_unlock(L);
}


LUA_API পূর্ণসংখ্যা lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->শীর্ষ.p), L);
  api_incr_শীর্ষ(L);
  lua_unlock(L);
  ফেরত (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


স্থির পূর্ণসংখ্যা auxgetstr (lua_State *L, ধ্রুবক TValue *t, ধ্রুবক অক্ষর *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->শীর্ষ.p), luaH_getstr, tag);
  যদি (!tagisempty(tag))
    api_incr_শীর্ষ(L);
  অন্যথা {
    setsvalue2s(L, L->শীর্ষ.p, str);
    api_incr_শীর্ষ(L);
    tag = luaV_finishget(L, t, s2v(L->শীর্ষ.p - 1), L->শীর্ষ.p - 1, tag);
  }
  lua_unlock(L);
  ফেরত novariant(tag);
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


LUA_API পূর্ণসংখ্যা lua_getglobal (lua_State *L, ধ্রুবক অক্ষর *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  ফেরত auxgetstr(L, &gt, name);
}


LUA_API পূর্ণসংখ্যা lua_gettable (lua_State *L, পূর্ণসংখ্যা idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->শীর্ষ.p - 1), s2v(L->শীর্ষ.p - 1), luaH_get, tag);
  যদি (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->শীর্ষ.p - 1), L->শীর্ষ.p - 1, tag);
  lua_unlock(L);
  ফেরত novariant(tag);
}


LUA_API পূর্ণসংখ্যা lua_getfield (lua_State *L, পূর্ণসংখ্যা idx, ধ্রুবক অক্ষর *k) {
  lua_lock(L);
  ফেরত auxgetstr(L, index2value(L, idx), k);
}


LUA_API পূর্ণসংখ্যা lua_geti (lua_State *L, পূর্ণসংখ্যা idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->শীর্ষ.p), tag);
  যদি (tagisempty(tag)) {
    TValue চাবি;
    setivalue(&চাবি, n);
    tag = luaV_finishget(L, t, &চাবি, L->শীর্ষ.p, tag);
  }
  api_incr_শীর্ষ(L);
  lua_unlock(L);
  ফেরত novariant(tag);
}


স্থির পূর্ণসংখ্যা finishrawget (lua_State *L, lu_byte tag) {
  যদি (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->শীর্ষ.p);
  api_incr_শীর্ষ(L);
  lua_unlock(L);
  ফেরত novariant(tag);
}


l_sinline Table *gettable (lua_State *L, পূর্ণসংখ্যা idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  ফেরত hvalue(t);
}


LUA_API পূর্ণসংখ্যা lua_rawget (lua_State *L, পূর্ণসংখ্যা idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->শীর্ষ.p - 1), s2v(L->শীর্ষ.p - 1));
  L->শীর্ষ.p--;  /* pop key */
  ফেরত finishrawget(L, tag);
}


LUA_API পূর্ণসংখ্যা lua_rawgeti (lua_State *L, পূর্ণসংখ্যা idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->শীর্ষ.p), tag);
  ফেরত finishrawget(L, tag);
}


LUA_API পূর্ণসংখ্যা lua_rawgetp (lua_State *L, পূর্ণসংখ্যা idx, ধ্রুবক শূন্য *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  ফেরত finishrawget(L, luaH_get(t, &k, s2v(L->শীর্ষ.p)));
}


LUA_API শূন্য lua_createtable (lua_State *L, পূর্ণসংখ্যা narray, পূর্ণসংখ্যা nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->শীর্ষ.p, t);
  api_incr_শীর্ষ(L);
  যদি (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API পূর্ণসংখ্যা lua_getmetatable (lua_State *L, পূর্ণসংখ্যা objindex) {
  ধ্রুবক TValue *obj;
  Table *mt;
  পূর্ণসংখ্যা res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  নির্বাচন (ttype(obj)) {
    ক্ষেত্র LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      বিরতি;
    ক্ষেত্র LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      বিরতি;
    পূর্বনির্ধারিত:
      mt = G(L)->mt[ttype(obj)];
      বিরতি;
  }
  যদি (mt != ফাঁকা) {
    sethvalue2s(L, L->শীর্ষ.p, mt);
    api_incr_শীর্ষ(L);
    res = 1;
  }
  lua_unlock(L);
  ফেরত res;
}


LUA_API পূর্ণসংখ্যা lua_getiuservalue (lua_State *L, পূর্ণসংখ্যা idx, পূর্ণসংখ্যা n) {
  TValue *o;
  পূর্ণসংখ্যা t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  যদি (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->শীর্ষ.p);
    t = LUA_TNONE;
  }
  অন্যথা {
    setobj2s(L, L->শীর্ষ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->শীর্ষ.p));
  }
  api_incr_শীর্ষ(L);
  lua_unlock(L);
  ফেরত t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
স্থির শূন্য auxsetstr (lua_State *L, ধ্রুবক TValue *t, ধ্রুবক অক্ষর *k) {
  পূর্ণসংখ্যা hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->শীর্ষ.p - 1), hres, luaH_psetstr);
  যদি (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->শীর্ষ.p - 1));
    L->শীর্ষ.p--;  /* pop value */
  }
  অন্যথা {
    setsvalue2s(L, L->শীর্ষ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_শীর্ষ(L);
    luaV_finishset(L, t, s2v(L->শীর্ষ.p - 1), s2v(L->শীর্ষ.p - 2), hres);
    L->শীর্ষ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API শূন্য lua_setglobal (lua_State *L, ধ্রুবক অক্ষর *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API শূন্য lua_settable (lua_State *L, পূর্ণসংখ্যা idx) {
  TValue *t;
  পূর্ণসংখ্যা hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->শীর্ষ.p - 2), s2v(L->শীর্ষ.p - 1), hres, luaH_pset);
  যদি (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->শীর্ষ.p - 1));
  অন্যথা
    luaV_finishset(L, t, s2v(L->শীর্ষ.p - 2), s2v(L->শীর্ষ.p - 1), hres);
  L->শীর্ষ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API শূন্য lua_setfield (lua_State *L, পূর্ণসংখ্যা idx, ধ্রুবক অক্ষর *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API শূন্য lua_seti (lua_State *L, পূর্ণসংখ্যা idx, lua_Integer n) {
  TValue *t;
  পূর্ণসংখ্যা hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->শীর্ষ.p - 1), hres);
  যদি (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->শীর্ষ.p - 1));
  অন্যথা {
    TValue অস্থায়ী;
    setivalue(&অস্থায়ী, n);
    luaV_finishset(L, t, &অস্থায়ী, s2v(L->শীর্ষ.p - 1), hres);
  }
  L->শীর্ষ.p--;  /* pop value */
  lua_unlock(L);
}


স্থির শূন্য aux_rawset (lua_State *L, পূর্ণসংখ্যা idx, TValue *চাবি, পূর্ণসংখ্যা n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, চাবি, s2v(L->শীর্ষ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->শীর্ষ.p - 1));
  L->শীর্ষ.p -= n;
  lua_unlock(L);
}


LUA_API শূন্য lua_rawset (lua_State *L, পূর্ণসংখ্যা idx) {
  aux_rawset(L, idx, s2v(L->শীর্ষ.p - 2), 2);
}


LUA_API শূন্য lua_rawsetp (lua_State *L, পূর্ণসংখ্যা idx, ধ্রুবক শূন্য *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API শূন্য lua_rawseti (lua_State *L, পূর্ণসংখ্যা idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->শীর্ষ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->শীর্ষ.p - 1));
  L->শীর্ষ.p--;
  lua_unlock(L);
}


LUA_API পূর্ণসংখ্যা lua_setmetatable (lua_State *L, পূর্ণসংখ্যা objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  যদি (ttisnil(s2v(L->শীর্ষ.p - 1)))
    mt = ফাঁকা;
  অন্যথা {
    api_check(L, ttistable(s2v(L->শীর্ষ.p - 1)), "table expected");
    mt = hvalue(s2v(L->শীর্ষ.p - 1));
  }
  নির্বাচন (ttype(obj)) {
    ক্ষেত্র LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      যদি (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      বিরতি;
    }
    ক্ষেত্র LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      যদি (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      G(L)->mt[ttype(obj)] = mt;
      বিরতি;
    }
  }
  L->শীর্ষ.p--;
  lua_unlock(L);
  ফেরত 1;
}


LUA_API পূর্ণসংখ্যা lua_setiuservalue (lua_State *L, পূর্ণসংখ্যা idx, পূর্ণসংখ্যা n) {
  TValue *o;
  পূর্ণসংখ্যা res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  যদি (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  অন্যথা {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->শীর্ষ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->শীর্ষ.p - 1));
    res = 1;
  }
  L->শীর্ষ.p--;
  lua_unlock(L);
  ফেরত res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->শীর্ষ.p - L->শীর্ষ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API শূন্য lua_callk (lua_State *L, পূর্ণসংখ্যা nargs, পূর্ণসংখ্যা nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == ফাঁকা || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->শীর্ষ.p - (nargs+1);
  যদি (k != ফাঁকা && yieldable(L)) {  /* need to prepare continuation? */
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
কাঠামো CallS {  /* data to 'f_call' */
  StkId func;
  পূর্ণসংখ্যা nresults;
};


স্থির শূন্য f_call (lua_State *L, শূন্য *ud) {
  কাঠামো CallS *c = cast(কাঠামো CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API পূর্ণসংখ্যা lua_pcallk (lua_State *L, পূর্ণসংখ্যা nargs, পূর্ণসংখ্যা nresults, পূর্ণসংখ্যা errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  কাঠামো CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == ফাঁকা || !isLua(L->ci),
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
  c.func = L->শীর্ষ.p - (nargs+1);  /* function to be called */
  যদি (k == ফাঁকা || !yieldable(L)) {  /* no continuation or no yieldable? */
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
  ফেরত APIstatus(status);
}


LUA_API পূর্ণসংখ্যা lua_load (lua_State *L, lua_Reader reader, শূন্য *তথ্য,
                      ধ্রুবক অক্ষর *chunkname, ধ্রুবক অক্ষর *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  যদি (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, তথ্য);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  যদি (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->শীর্ষ.p - 1));  /* get new function */
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
  ফেরত APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API পূর্ণসংখ্যা lua_dump (lua_State *L, lua_Writer writer, শূন্য *তথ্য, পূর্ণসংখ্যা strip) {
  পূর্ণসংখ্যা status;
  ptrdiff_t otop = savestack(L, L->শীর্ষ.p);  /* original top */
  TValue *f = s2v(L->শীর্ষ.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, তথ্য, strip);
  L->শীর্ষ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  ফেরত status;
}


LUA_API পূর্ণসংখ্যা lua_status (lua_State *L) {
  ফেরত APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API পূর্ণসংখ্যা lua_gc (lua_State *L, পূর্ণসংখ্যা what, ...) {
  va_list argp;
  পূর্ণসংখ্যা res = 0;
  global_State *g = G(L);
  যদি (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    ফেরত -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  নির্বাচন (what) {
    ক্ষেত্র LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      বিরতি;
    }
    ক্ষেত্র LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      বিরতি;
    }
    ক্ষেত্র LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      বিরতি;
    }
    ক্ষেত্র LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      বিরতি;
    }
    ক্ষেত্র LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      বিরতি;
    }
    ক্ষেত্র LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      পূর্ণসংখ্যা work = 0;  /* true if GC did some work */
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
      বিরতি;
    }
    ক্ষেত্র LUA_GCISRUNNING: {
      res = gcrunning(g);
      বিরতি;
    }
    ক্ষেত্র LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      বিরতি;
    }
    ক্ষেত্র LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      বিরতি;
    }
    ক্ষেত্র LUA_GCPARAM: {
      পূর্ণসংখ্যা param = va_arg(argp, পূর্ণসংখ্যা);
      পূর্ণসংখ্যা মান = va_arg(argp, পূর্ণসংখ্যা);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      যদি (মান >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(মান));
      বিরতি;
    }
    পূর্বনির্ধারিত: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  ফেরত res;
}



/*
** miscellaneous functions
*/


LUA_API পূর্ণসংখ্যা lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->শীর্ষ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  যদি (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  অন্যথা
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  ফেরত 0;  /* to avoid warnings */
}


LUA_API পূর্ণসংখ্যা lua_পরবর্তী (lua_State *L, পূর্ণসংখ্যা idx) {
  Table *t;
  পূর্ণসংখ্যা more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = lua_H_পরবর্তী(L, t, L->শীর্ষ.p - 1);
  যদি (more)
    api_incr_শীর্ষ(L);
  অন্যথা  /* no more elements */
    L->শীর্ষ.p--;  /* pop key */
  lua_unlock(L);
  ফেরত more;
}


LUA_API শূন্য lua_toclose (lua_State *L, পূর্ণসংখ্যা idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API শূন্য lua_concat (lua_State *L, পূর্ণসংখ্যা n) {
  lua_lock(L);
  যদি (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  অন্যথা {  /* nothing to concatenate */
    setsvalue2s(L, L->শীর্ষ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_শীর্ষ(L);
  }
  lua_unlock(L);
}


LUA_API শূন্য lua_len (lua_State *L, পূর্ণসংখ্যা idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->শীর্ষ.p, t);
  api_incr_শীর্ষ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, শূন্য **ud) {
  lua_Alloc f;
  lua_lock(L);
  যদি (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  ফেরত f;
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


শূন্য lua_warning (lua_State *L, ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা tocont) {
  lua_lock(L);
  luaE_warning(L, বার্তা, tocont);
  lua_unlock(L);
}



LUA_API শূন্য *lua_newuserdatauv (lua_State *L, size_t আকার_2, পূর্ণসংখ্যা nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, আকার_2, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->শীর্ষ.p), u);
  api_incr_শীর্ষ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ফেরত getudatamem(u);
}



স্থির ধ্রুবক অক্ষর *aux_upvalue (TValue *fi, পূর্ণসংখ্যা n, TValue **val,
                                GCObject **owner) {
  নির্বাচন (ttypetag(fi)) {
    ক্ষেত্র LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      যদি (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        ফেরত ফাঁকা;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      যদি (owner) *owner = obj2gco(f);
      ফেরত "";
    }
    ক্ষেত্র LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      যদি (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        ফেরত ফাঁকা;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      যদি (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      ফেরত (name == ফাঁকা) ? "(no name)" : getstr(name);
    }
    পূর্বনির্ধারিত: ফেরত ফাঁকা;  /* not a closure */
  }
}


LUA_API ধ্রুবক অক্ষর *lua_getupvalue (lua_State *L, পূর্ণসংখ্যা funcindex, পূর্ণসংখ্যা n) {
  ধ্রুবক অক্ষর *name;
  TValue *val = ফাঁকা;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ফাঁকা);
  যদি (name) {
    setobj2s(L, L->শীর্ষ.p, val);
    api_incr_শীর্ষ(L);
  }
  lua_unlock(L);
  ফেরত name;
}


LUA_API ধ্রুবক অক্ষর *lua_setupvalue (lua_State *L, পূর্ণসংখ্যা funcindex, পূর্ণসংখ্যা n) {
  ধ্রুবক অক্ষর *name;
  TValue *val = ফাঁকা;  /* to avoid warnings */
  GCObject *owner = ফাঁকা;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  যদি (name) {
    L->শীর্ষ.p--;
    setobj(L, val, s2v(L->শীর্ষ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  ফেরত name;
}


স্থির UpVal **getupvalref (lua_State *L, পূর্ণসংখ্যা fidx, পূর্ণসংখ্যা n, LClosure **pf) {
  স্থির ধ্রুবক UpVal *ধ্রুবক nullup = ফাঁকা;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  যদি (pf) *pf = f;
  যদি (1 <= n && n <= f->p->sizeupvalues)
    ফেরত &f->upvals[n - 1];  /* get its upvalue pointer */
  অন্যথা
    ফেরত (UpVal**)&nullup;
}


LUA_API শূন্য *lua_upvalueid (lua_State *L, পূর্ণসংখ্যা fidx, পূর্ণসংখ্যা n) {
  TValue *fi = index2value(L, fidx);
  নির্বাচন (ttypetag(fi)) {
    ক্ষেত্র LUA_VLCL: {  /* lua closure */
      ফেরত *getupvalref(L, fidx, n, ফাঁকা);
    }
    ক্ষেত্র LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      যদি (1 <= n && n <= f->nupvalues)
        ফেরত &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    ক্ষেত্র LUA_VLCF:
      ফেরত ফাঁকা;  /* light C functions have no upvalues */
    পূর্বনির্ধারিত: {
      api_check(L, 0, "function expected");
      ফেরত ফাঁকা;
    }
  }
}


LUA_API শূন্য lua_upvaluejoin (lua_State *L, পূর্ণসংখ্যা fidx1, পূর্ণসংখ্যা n1,
                                            পূর্ণসংখ্যা fidx2, পূর্ণসংখ্যা n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, ফাঁকা);
  api_check(L, *up1 != ফাঁকা && *up2 != ফাঁকা, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


