/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#تعریف_کرو lapi_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <stdarg.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "ltm.h"
#شامل "lundump.h"
#شامل "lvm.h"



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
ساکن TValue *index2value (lua_State *L, صحیح idx) {
  CallInfo *ci = L->ci;
  اگر (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->چوٹی.p - (ci->func.p + 1), "unacceptable index");
    اگر (o >= L->چوٹی.p) واپس &G(L)->nilvalue;
    ورنہ واپس s2v(o);
  }
  ورنہ اگر (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->چوٹی.p - (ci->func.p + 1),
                 "invalid index");
    واپس s2v(L->چوٹی.p + idx);
  }
  ورنہ اگر (idx == LUA_REGISTRYINDEX)
    واپس &G(L)->l_registry;
  ورنہ {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    اگر (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      واپس (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    ورنہ {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      واپس &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
ساکن StkId index2stack (lua_State *L, صحیح idx) {
  CallInfo *ci = L->ci;
  اگر (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->چوٹی.p, "invalid index");
    واپس o;
  }
  ورنہ {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->چوٹی.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    واپس L->چوٹی.p + idx;
  }
}


LUA_API صحیح lua_checkstack (lua_State *L, صحیح n) {
  صحیح res;
  CallInfo *ci;
  lua_تالا(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  اگر (L->ڈھیر_آخری.p - L->چوٹی.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  ورنہ  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  اگر (res && ci->چوٹی.p < L->چوٹی.p + n)
    ci->چوٹی.p = L->چوٹی.p + n;  /* adjust frame top */
  lua_unlock(L);
  واپس res;
}


LUA_API خالی lua_xmove (lua_State *from, lua_State *مقصد, صحیح n) {
  صحیح i;
  اگر (from == مقصد) واپس;
  lua_تالا(مقصد);
  api_checkpop(from, n);
  api_check(from, G(from) == G(مقصد), "moving among independent states");
  api_check(from, مقصد->ci->چوٹی.p - مقصد->چوٹی.p >= n, "stack overflow");
  from->چوٹی.p -= n;
  برائے (i = 0; i < n; i++) {
    setobjs2s(مقصد, مقصد->چوٹی.p, from->چوٹی.p + i);
    مقصد->چوٹی.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(مقصد);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_تالا(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  واپس old;
}


LUA_API lua_نمبر lua_نسخہ (lua_State *L) {
  غیر_مستعمل(L);
  واپس LUA_نسخہ_عدد;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API صحیح lua_absindex (lua_State *L, صحیح idx) {
  واپس (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->چوٹی.p - L->ci->func.p) + idx;
}


LUA_API صحیح lua_gettop (lua_State *L) {
  واپس cast_int(L->چوٹی.p - (L->ci->func.p + 1));
}


LUA_API خالی lua_settop (lua_State *L, صحیح idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_تالا(L);
  ci = L->ci;
  func = ci->func.p;
  اگر (idx >= 0) {
    api_check(L, idx <= ci->چوٹی.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->چوٹی.p;
    برائے (; diff > 0; diff--)
      setnilvalue2s(L->چوٹی.p++);  /* clear new slots */
  }
  ورنہ {
    api_check(L, -(idx+1) <= (L->چوٹی.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->چوٹی.p + diff;
  اگر (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->چوٹی.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API خالی lua_closeslot (lua_State *L, صحیح idx) {
  StkId level;
  lua_تالا(L);
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
ساکن خالی الٹ (lua_State *L, StkId from, StkId مقصد) {
  برائے (; from < مقصد; from++, مقصد--) {
    TValue عارضی;
    setobj(L, &عارضی, s2v(from));
    setobjs2s(L, from, مقصد);
    setobj2s(L, مقصد, &عارضی);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API خالی lua_rotate (lua_State *L, صحیح idx, صحیح n) {
  StkId p, t, m;
  lua_تالا(L);
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


LUA_API خالی lua_copy (lua_State *L, صحیح fromidx, صحیح toidx) {
  TValue *fr, *مقصد;
  lua_تالا(L);
  fr = index2value(L, fromidx);
  مقصد = index2value(L, toidx);
  api_check(L, isvalid(L, مقصد), "invalid index");
  setobj(L, مقصد, fr);
  اگر (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API خالی lua_pushvalue (lua_State *L, صحیح idx) {
  lua_تالا(L);
  setobj2s(L, L->چوٹی.p, index2value(L, idx));
  api_incr_چوٹی(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API صحیح lua_type (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ثابت حرف *lua_typename (lua_State *L, صحیح t) {
  غیر_مستعمل(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  واپس ttypename(t);
}


LUA_API صحیح lua_iscfunction (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API صحیح lua_isinteger (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس ttisinteger(o);
}


LUA_API صحیح lua_isnumber (lua_State *L, صحیح idx) {
  lua_نمبر n;
  ثابت TValue *o = index2value(L, idx);
  واپس tonumber(o, &n);
}


LUA_API صحیح lua_isstring (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس (ttisstring(o) || cvt2str(o));
}


LUA_API صحیح lua_isuserdata (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API صحیح lua_rawequal (lua_State *L, صحیح index1, صحیح index2) {
  ثابت TValue *o1 = index2value(L, index1);
  ثابت TValue *o2 = index2value(L, index2);
  واپس (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API خالی lua_arith (lua_State *L, صحیح op) {
  lua_تالا(L);
  اگر (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  ورنہ {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->چوٹی.p, L->چوٹی.p - 1);
    api_incr_چوٹی(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->چوٹی.p - 2), s2v(L->چوٹی.p - 1), L->چوٹی.p - 2);
  L->چوٹی.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API صحیح lua_compare (lua_State *L, صحیح index1, صحیح index2, صحیح op) {
  ثابت TValue *o1;
  ثابت TValue *o2;
  صحیح i = 0;
  lua_تالا(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  اگر (isvalid(L, o1) && isvalid(L, o2)) {
    انتخاب (op) {
      حالت LUA_OPEQ: i = luaV_equalobj(L, o1, o2); توڑو;
      حالت LUA_OPLT: i = luaV_lessthan(L, o1, o2); توڑو;
      حالت LUA_OPLE: i = luaV_lessequal(L, o1, o2); توڑو;
      طےشدہ: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  واپس i;
}


LUA_API غیرعلامتی lua_numbertocstring (lua_State *L, صحیح idx, حرف *buff) {
  ثابت TValue *o = index2value(L, idx);
  اگر (ttisnumber(o)) {
    غیرعلامتی len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    واپس len;
  }
  ورنہ
    واپس 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ثابت حرف *s) {
  size_t sz = luaO_str2num(s, s2v(L->چوٹی.p));
  اگر (sz != 0)
    api_incr_چوٹی(L);
  واپس sz;
}


LUA_API lua_نمبر lua_tonumberx (lua_State *L, صحیح idx, صحیح *pisnum) {
  lua_نمبر n = 0;
  ثابت TValue *o = index2value(L, idx);
  صحیح isnum = tonumber(o, &n);
  اگر (pisnum)
    *pisnum = isnum;
  واپس n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, صحیح idx, صحیح *pisnum) {
  lua_Integer res = 0;
  ثابت TValue *o = index2value(L, idx);
  صحیح isnum = tointeger(o, &res);
  اگر (pisnum)
    *pisnum = isnum;
  واپس res;
}


LUA_API صحیح lua_toboolean (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس !l_isfalse(o);
}


LUA_API ثابت حرف *lua_tolstring (lua_State *L, صحیح idx, size_t *len) {
  TValue *o;
  lua_تالا(L);
  o = index2value(L, idx);
  اگر (!ttisstring(o)) {
    اگر (!cvt2str(o)) {  /* not convertible? */
      اگر (len != تہی) *len = 0;
      lua_unlock(L);
      واپس تہی;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  اگر (len != تہی)
    واپس getlstr(tsvalue(o), *len);
  ورنہ
    واپس getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  انتخاب (ttypetag(o)) {
    حالت LUA_VSHRSTR: واپس cast(lua_Unsigned, tsvalue(o)->shrlen);
    حالت LUA_VLNGSTR: واپس cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    حالت LUA_VUSERDATA: واپس cast(lua_Unsigned, uvalue(o)->len);
    حالت LUA_VTABLE: {
      lua_Unsigned res;
      lua_تالا(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      واپس res;
    }
    طےشدہ: واپس 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  اگر (ttislcf(o)) واپس fvalue(o);
  ورنہ اگر (ttisCclosure(o))
    واپس clCvalue(o)->f;
  ورنہ واپس تہی;  /* not a C function */
}


l_sinline خالی *touserdata (ثابت TValue *o) {
  انتخاب (ttype(o)) {
    حالت LUA_TUSERDATA: واپس getudatamem(uvalue(o));
    حالت LUA_TLIGHTUSERDATA: واپس pvalue(o);
    طےشدہ: واپس تہی;
  }
}


LUA_API خالی *lua_touserdata (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  واپس (!ttisthread(o)) ? تہی : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ثابت خالی *lua_topointer (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  انتخاب (ttypetag(o)) {
    حالت LUA_VLCF: واپس cast_voidp(cast_sizet(fvalue(o)));
    حالت LUA_VUSERDATA: حالت LUA_VLIGHTUSERDATA:
      واپس touserdata(o);
    طےشدہ: {
      اگر (iscollectable(o))
        واپس gcvalue(o);
      ورنہ
        واپس تہی;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API خالی lua_pushnil (lua_State *L) {
  lua_تالا(L);
  setnilvalue2s(L->چوٹی.p);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API خالی lua_pushnumber (lua_State *L, lua_نمبر n) {
  lua_تالا(L);
  setfltvalue(s2v(L->چوٹی.p), n);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API خالی lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_تالا(L);
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
  lua_تالا(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->چوٹی.p, ts);
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  واپس getstr(ts);
}


LUA_API ثابت حرف *lua_pushexternalstring (lua_State *L,
	        ثابت حرف *s, size_t len, lua_Alloc falloc, خالی *ud) {
  TString *ts;
  lua_تالا(L);
  api_check(L, len <= MAX_سائز, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->چوٹی.p, ts);
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  واپس getstr(ts);
}


LUA_API ثابت حرف *lua_pushstring (lua_State *L, ثابت حرف *s) {
  lua_تالا(L);
  اگر (s == تہی)
    setnilvalue2s(L->چوٹی.p);
  ورنہ {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->چوٹی.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  واپس s;
}


LUA_API ثابت حرف *lua_pushvfstring (lua_State *L, ثابت حرف *fmt,
                                      va_list argp) {
  ثابت حرف *ret;
  lua_تالا(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  واپس ret;
}


LUA_API ثابت حرف *lua_pushfstring (lua_State *L, ثابت حرف *fmt, ...) {
  ثابت حرف *ret;
  va_list argp;
  lua_تالا(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  واپس ret;
}


LUA_API خالی lua_pushcclosure (lua_State *L, lua_CFunction fn, صحیح n) {
  lua_تالا(L);
  اگر (n == 0) {
    setfvalue(s2v(L->چوٹی.p), fn);
    api_incr_چوٹی(L);
  }
  ورنہ {
    صحیح i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    برائے (i = 0; i < n; i++) {
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


LUA_API خالی lua_pushboolean (lua_State *L, صحیح b) {
  lua_تالا(L);
  اگر (b)
    setbtvalue(s2v(L->چوٹی.p));
  ورنہ
    setbfvalue(s2v(L->چوٹی.p));
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API خالی lua_pushlightuserdata (lua_State *L, خالی *p) {
  lua_تالا(L);
  setpvalue(s2v(L->چوٹی.p), p);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API صحیح lua_pushthread (lua_State *L) {
  lua_تالا(L);
  setthvalue(L, s2v(L->چوٹی.p), L);
  api_incr_چوٹی(L);
  lua_unlock(L);
  واپس (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


ساکن صحیح auxgetstr (lua_State *L, ثابت TValue *t, ثابت حرف *k) {
  lu_byte لیبل;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->چوٹی.p), luaH_getstr, لیبل);
  اگر (!tagisempty(لیبل))
    api_incr_چوٹی(L);
  ورنہ {
    setsvalue2s(L, L->چوٹی.p, str);
    api_incr_چوٹی(L);
    لیبل = luaV_finishget(L, t, s2v(L->چوٹی.p - 1), L->چوٹی.p - 1, لیبل);
  }
  lua_unlock(L);
  واپس novariant(لیبل);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
ساکن خالی get_Global_جدول (lua_State *L, TValue *gt) {
  جدول *registry = hvalue(&G(L)->l_registry);
  lu_byte لیبل = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (خالی)لیبل;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(لیبل) == LUA_TTABLE, "global table must exist");
}


LUA_API صحیح lua_getglobal (lua_State *L, ثابت حرف *name) {
  TValue gt;
  lua_تالا(L);
  get_Global_جدول(L, &gt);
  واپس auxgetstr(L, &gt, name);
}


LUA_API صحیح lua_gettable (lua_State *L, صحیح idx) {
  lu_byte لیبل;
  TValue *t;
  lua_تالا(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->چوٹی.p - 1), s2v(L->چوٹی.p - 1), luaH_get, لیبل);
  اگر (tagisempty(لیبل))
    لیبل = luaV_finishget(L, t, s2v(L->چوٹی.p - 1), L->چوٹی.p - 1, لیبل);
  lua_unlock(L);
  واپس novariant(لیبل);
}


LUA_API صحیح lua_getfield (lua_State *L, صحیح idx, ثابت حرف *k) {
  lua_تالا(L);
  واپس auxgetstr(L, index2value(L, idx), k);
}


LUA_API صحیح lua_geti (lua_State *L, صحیح idx, lua_Integer n) {
  TValue *t;
  lu_byte لیبل;
  lua_تالا(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->چوٹی.p), لیبل);
  اگر (tagisempty(لیبل)) {
    TValue کلید;
    setivalue(&کلید, n);
    لیبل = luaV_finishget(L, t, &کلید, L->چوٹی.p, لیبل);
  }
  api_incr_چوٹی(L);
  lua_unlock(L);
  واپس novariant(لیبل);
}


ساکن صحیح finishrawget (lua_State *L, lu_byte لیبل) {
  اگر (tagisempty(لیبل))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->چوٹی.p);
  api_incr_چوٹی(L);
  lua_unlock(L);
  واپس novariant(لیبل);
}


l_sinline جدول *gettable (lua_State *L, صحیح idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  واپس hvalue(t);
}


LUA_API صحیح lua_rawget (lua_State *L, صحیح idx) {
  جدول *t;
  lu_byte لیبل;
  lua_تالا(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  لیبل = luaH_get(t, s2v(L->چوٹی.p - 1), s2v(L->چوٹی.p - 1));
  L->چوٹی.p--;  /* pop key */
  واپس finishrawget(L, لیبل);
}


LUA_API صحیح lua_rawgeti (lua_State *L, صحیح idx, lua_Integer n) {
  جدول *t;
  lu_byte لیبل;
  lua_تالا(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->چوٹی.p), لیبل);
  واپس finishrawget(L, لیبل);
}


LUA_API صحیح lua_rawgetp (lua_State *L, صحیح idx, ثابت خالی *p) {
  جدول *t;
  TValue k;
  lua_تالا(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  واپس finishrawget(L, luaH_get(t, &k, s2v(L->چوٹی.p)));
}


LUA_API خالی lua_createtable (lua_State *L, صحیح narray, صحیح nrec) {
  جدول *t;
  lua_تالا(L);
  t = luaH_new(L);
  sethvalue2s(L, L->چوٹی.p, t);
  api_incr_چوٹی(L);
  اگر (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API صحیح lua_getmetatable (lua_State *L, صحیح objindex) {
  ثابت TValue *obj;
  جدول *mt;
  صحیح res = 0;
  lua_تالا(L);
  obj = index2value(L, objindex);
  انتخاب (ttype(obj)) {
    حالت LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      توڑو;
    حالت LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      توڑو;
    طےشدہ:
      mt = G(L)->mt[ttype(obj)];
      توڑو;
  }
  اگر (mt != تہی) {
    sethvalue2s(L, L->چوٹی.p, mt);
    api_incr_چوٹی(L);
    res = 1;
  }
  lua_unlock(L);
  واپس res;
}


LUA_API صحیح lua_getiuservalue (lua_State *L, صحیح idx, صحیح n) {
  TValue *o;
  صحیح t;
  lua_تالا(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  اگر (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->چوٹی.p);
    t = LUA_TNONE;
  }
  ورنہ {
    setobj2s(L, L->چوٹی.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->چوٹی.p));
  }
  api_incr_چوٹی(L);
  lua_unlock(L);
  واپس t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
ساکن خالی auxsetstr (lua_State *L, ثابت TValue *t, ثابت حرف *k) {
  صحیح hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->چوٹی.p - 1), hres, luaH_psetstr);
  اگر (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->چوٹی.p - 1));
    L->چوٹی.p--;  /* pop value */
  }
  ورنہ {
    setsvalue2s(L, L->چوٹی.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_چوٹی(L);
    luaV_finishset(L, t, s2v(L->چوٹی.p - 1), s2v(L->چوٹی.p - 2), hres);
    L->چوٹی.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API خالی lua_setglobal (lua_State *L, ثابت حرف *name) {
  TValue gt;
  lua_تالا(L);  /* unlock done in 'auxsetstr' */
  get_Global_جدول(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API خالی lua_settable (lua_State *L, صحیح idx) {
  TValue *t;
  صحیح hres;
  lua_تالا(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->چوٹی.p - 2), s2v(L->چوٹی.p - 1), hres, luaH_pset);
  اگر (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->چوٹی.p - 1));
  ورنہ
    luaV_finishset(L, t, s2v(L->چوٹی.p - 2), s2v(L->چوٹی.p - 1), hres);
  L->چوٹی.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API خالی lua_setfield (lua_State *L, صحیح idx, ثابت حرف *k) {
  lua_تالا(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API خالی lua_seti (lua_State *L, صحیح idx, lua_Integer n) {
  TValue *t;
  صحیح hres;
  lua_تالا(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->چوٹی.p - 1), hres);
  اگر (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->چوٹی.p - 1));
  ورنہ {
    TValue عارضی;
    setivalue(&عارضی, n);
    luaV_finishset(L, t, &عارضی, s2v(L->چوٹی.p - 1), hres);
  }
  L->چوٹی.p--;  /* pop value */
  lua_unlock(L);
}


ساکن خالی aux_rawset (lua_State *L, صحیح idx, TValue *کلید, صحیح n) {
  جدول *t;
  lua_تالا(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  lua_H_سیٹ(L, t, کلید, s2v(L->چوٹی.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->چوٹی.p - 1));
  L->چوٹی.p -= n;
  lua_unlock(L);
}


LUA_API خالی lua_rawset (lua_State *L, صحیح idx) {
  aux_rawset(L, idx, s2v(L->چوٹی.p - 2), 2);
}


LUA_API خالی lua_rawsetp (lua_State *L, صحیح idx, ثابت خالی *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API خالی lua_rawseti (lua_State *L, صحیح idx, lua_Integer n) {
  جدول *t;
  lua_تالا(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->چوٹی.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->چوٹی.p - 1));
  L->چوٹی.p--;
  lua_unlock(L);
}


LUA_API صحیح lua_setmetatable (lua_State *L, صحیح objindex) {
  TValue *obj;
  جدول *mt;
  lua_تالا(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  اگر (ttisnil(s2v(L->چوٹی.p - 1)))
    mt = تہی;
  ورنہ {
    api_check(L, ttistable(s2v(L->چوٹی.p - 1)), "table expected");
    mt = hvalue(s2v(L->چوٹی.p - 1));
  }
  انتخاب (ttype(obj)) {
    حالت LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      اگر (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      توڑو;
    }
    حالت LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      اگر (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      توڑو;
    }
    طےشدہ: {
      G(L)->mt[ttype(obj)] = mt;
      توڑو;
    }
  }
  L->چوٹی.p--;
  lua_unlock(L);
  واپس 1;
}


LUA_API صحیح lua_setiuservalue (lua_State *L, صحیح idx, صحیح n) {
  TValue *o;
  صحیح res;
  lua_تالا(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  اگر (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  ورنہ {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->چوٹی.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->چوٹی.p - 1));
    res = 1;
  }
  L->چوٹی.p--;
  lua_unlock(L);
  واپس res;
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


LUA_API خالی lua_callk (lua_State *L, صحیح nargs, صحیح nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_تالا(L);
  api_check(L, k == تہی || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->حالت_2 == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->چوٹی.p - (nargs+1);
  اگر (k != تہی && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  ورنہ  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
ساخت CallS {  /* data to 'f_call' */
  StkId func;
  صحیح nresults;
};


ساکن خالی f_call (lua_State *L, خالی *ud) {
  ساخت CallS *c = cast(ساخت CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API صحیح lua_pcallk (lua_State *L, صحیح nargs, صحیح nresults, صحیح errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  ساخت CallS c;
  TStatus حالت_2;
  ptrdiff_t func;
  lua_تالا(L);
  api_check(L, k == تہی || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->حالت_2 == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  اگر (errfunc == 0)
    func = 0;
  ورنہ {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->چوٹی.p - (nargs+1);  /* function to be called */
  اگر (k == تہی || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    حالت_2 = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  ورنہ {  /* prepare continuation (call is already protected by 'resume') */
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
    حالت_2 = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  واپس APIstatus(حالت_2);
}


LUA_API صحیح lua_بھرو (lua_State *L, lua_Reader reader, خالی *ڈیٹا,
                      ثابت حرف *chunkname, ثابت حرف *منوال) {
  ZIO z;
  TStatus حالت_2;
  lua_تالا(L);
  luaC_checkGC(L);
  اگر (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, ڈیٹا);
  حالت_2 = luaD_protectedparser(L, &z, chunkname, منوال);
  اگر (حالت_2 == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->چوٹی.p - 1));  /* get new function */
    اگر (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      get_Global_جدول(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  واپس APIstatus(حالت_2);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API صحیح lua_dump (lua_State *L, lua_Writer writer, خالی *ڈیٹا, صحیح strip) {
  صحیح حالت_2;
  ptrdiff_t otop = savestack(L, L->چوٹی.p);  /* original top */
  TValue *f = s2v(L->چوٹی.p - 1);  /* function to be dumped */
  lua_تالا(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  حالت_2 = luaU_dump(L, clLvalue(f)->p, writer, ڈیٹا, strip);
  L->چوٹی.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  واپس حالت_2;
}


LUA_API صحیح lua_حالت (lua_State *L) {
  واپس APIstatus(L->حالت_2);
}


/*
** Garbage-collection function
*/
LUA_API صحیح lua_gc (lua_State *L, صحیح what, ...) {
  va_list argp;
  صحیح res = 0;
  global_State *g = G(L);
  اگر (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    واپس -1;  /* all options are invalid when stopped */
  lua_تالا(L);
  va_start(argp, what);
  انتخاب (what) {
    حالت LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      توڑو;
    }
    حالت LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      توڑو;
    }
    حالت LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      توڑو;
    }
    حالت LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      توڑو;
    }
    حالت LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      توڑو;
    }
    حالت LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      صحیح work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      اگر (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      ورنہ اگر (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      ورنہ  /*حد_سے_تجاوز*/
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (خالی)0, work = 1);
      اگر (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      توڑو;
    }
    حالت LUA_GCISRUNNING: {
      res = gcrunning(g);
      توڑو;
    }
    حالت LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      توڑو;
    }
    حالت LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      توڑو;
    }
    حالت LUA_GCPARAM: {
      صحیح param = va_arg(argp, صحیح);
      صحیح قدر = va_arg(argp, صحیح);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      اگر (قدر >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(قدر));
      توڑو;
    }
    طےشدہ: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  واپس res;
}



/*
** miscellaneous functions
*/


LUA_API صحیح lua_error (lua_State *L) {
  TValue *errobj;
  lua_تالا(L);
  errobj = s2v(L->چوٹی.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  اگر (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  ورنہ
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  واپس 0;  /* to avoid warnings */
}


LUA_API صحیح lua_اگلا (lua_State *L, صحیح idx) {
  جدول *t;
  صحیح مزید;
  lua_تالا(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  مزید = lua_H_اگلا(L, t, L->چوٹی.p - 1);
  اگر (مزید)
    api_incr_چوٹی(L);
  ورنہ  /* no more elements */
    L->چوٹی.p--;  /* pop key */
  lua_unlock(L);
  واپس مزید;
}


LUA_API خالی lua_toclose (lua_State *L, صحیح idx) {
  StkId o;
  lua_تالا(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API خالی lua_concat (lua_State *L, صحیح n) {
  lua_تالا(L);
  اگر (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  ورنہ {  /* nothing to concatenate */
    setsvalue2s(L, L->چوٹی.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_چوٹی(L);
  }
  lua_unlock(L);
}


LUA_API خالی lua_len (lua_State *L, صحیح idx) {
  TValue *t;
  lua_تالا(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->چوٹی.p, t);
  api_incr_چوٹی(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, خالی **ud) {
  lua_Alloc f;
  lua_تالا(L);
  اگر (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  واپس f;
}


LUA_API خالی lua_setallocf (lua_State *L, lua_Alloc f, خالی *ud) {
  lua_تالا(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


خالی lua_setwarnf (lua_State *L, lua_WarnFunction f, خالی *ud) {
  lua_تالا(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


خالی lua_warning (lua_State *L, ثابت حرف *پیغام, صحیح tocont) {
  lua_تالا(L);
  luaE_warning(L, پیغام, tocont);
  lua_unlock(L);
}



LUA_API خالی *lua_newuserdatauv (lua_State *L, size_t سائز, صحیح nuvalue) {
  Udata *u;
  lua_تالا(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, سائز, cast(غیرعلامتی مختصر, nuvalue));
  setuvalue(L, s2v(L->چوٹی.p), u);
  api_incr_چوٹی(L);
  luaC_checkGC(L);
  lua_unlock(L);
  واپس getudatamem(u);
}



ساکن ثابت حرف *aux_upvalue (TValue *fi, صحیح n, TValue **val,
                                GCObject **owner) {
  انتخاب (ttypetag(fi)) {
    حالت LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      اگر (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        واپس تہی;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      اگر (owner) *owner = obj2gco(f);
      واپس "";
    }
    حالت LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      اگر (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        واپس تہی;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      اگر (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      واپس (name == تہی) ? "(no name)" : getstr(name);
    }
    طےشدہ: واپس تہی;  /* not a closure */
  }
}


LUA_API ثابت حرف *lua_getupvalue (lua_State *L, صحیح funcindex, صحیح n) {
  ثابت حرف *name;
  TValue *val = تہی;  /* to avoid warnings */
  lua_تالا(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, تہی);
  اگر (name) {
    setobj2s(L, L->چوٹی.p, val);
    api_incr_چوٹی(L);
  }
  lua_unlock(L);
  واپس name;
}


LUA_API ثابت حرف *lua_setupvalue (lua_State *L, صحیح funcindex, صحیح n) {
  ثابت حرف *name;
  TValue *val = تہی;  /* to avoid warnings */
  GCObject *owner = تہی;  /* to avoid warnings */
  TValue *fi;
  lua_تالا(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  اگر (name) {
    L->چوٹی.p--;
    setobj(L, val, s2v(L->چوٹی.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  واپس name;
}


ساکن اوپر_Val **getupvalref (lua_State *L, صحیح fidx, صحیح n, LClosure **pf) {
  ساکن ثابت اوپر_Val *ثابت nullup = تہی;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  اگر (pf) *pf = f;
  اگر (1 <= n && n <= f->p->sizeupvalues)
    واپس &f->upvals[n - 1];  /* get its upvalue pointer */
  ورنہ
    واپس (اوپر_Val**)&nullup;
}


LUA_API خالی *lua_upvalueid (lua_State *L, صحیح fidx, صحیح n) {
  TValue *fi = index2value(L, fidx);
  انتخاب (ttypetag(fi)) {
    حالت LUA_VLCL: {  /* lua closure */
      واپس *getupvalref(L, fidx, n, تہی);
    }
    حالت LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      اگر (1 <= n && n <= f->nupvalues)
        واپس &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    حالت LUA_VLCF:
      واپس تہی;  /* light C functions have no upvalues */
    طےشدہ: {
      api_check(L, 0, "function expected");
      واپس تہی;
    }
  }
}


LUA_API خالی lua_upvaluejoin (lua_State *L, صحیح fidx1, صحیح n1,
                                            صحیح fidx2, صحیح n2) {
  LClosure *f1;
  اوپر_Val **up1 = getupvalref(L, fidx1, n1, &f1);
  اوپر_Val **up2 = getupvalref(L, fidx2, n2, تہی);
  api_check(L, *up1 != تہی && *up2 != تہی, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


