/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

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



ثابت نویسه lua_ident[] =
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
ایستا TValue *index2value (lua_State *L, صحیح idx) {
  CallInfo *ci = L->ci;
  اگر (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->بالاترین.p - (ci->func.p + 1), "unacceptable index");
    اگر (o >= L->بالاترین.p) بازگشت &G(L)->nilvalue;
    وگرنه بازگشت s2v(o);
  }
  وگرنه اگر (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->بالاترین.p - (ci->func.p + 1),
                 "invalid index");
    بازگشت s2v(L->بالاترین.p + idx);
  }
  وگرنه اگر (idx == LUA_REGISTRYINDEX)
    بازگشت &G(L)->l_registry;
  وگرنه {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    اگر (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      بازگشت (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    وگرنه {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      بازگشت &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
ایستا StkId index2stack (lua_State *L, صحیح idx) {
  CallInfo *ci = L->ci;
  اگر (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->بالاترین.p, "invalid index");
    بازگشت o;
  }
  وگرنه {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->بالاترین.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    بازگشت L->بالاترین.p + idx;
  }
}


LUA_API صحیح lua_checkstack (lua_State *L, صحیح n) {
  صحیح res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  اگر (L->پشته_last.p - L->بالاترین.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  وگرنه  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  اگر (res && ci->بالاترین.p < L->بالاترین.p + n)
    ci->بالاترین.p = L->بالاترین.p + n;  /* adjust frame top */
  lua_unlock(L);
  بازگشت res;
}


LUA_API تهی lua_xmove (lua_State *from, lua_State *to, صحیح n) {
  صحیح i;
  اگر (from == to) بازگشت;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->بالاترین.p - to->بالاترین.p >= n, "stack overflow");
  from->بالاترین.p -= n;
  برای (i = 0; i < n; i++) {
    setobjs2s(to, to->بالاترین.p, from->بالاترین.p + i);
    to->بالاترین.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  بازگشت old;
}


LUA_API lua_عدد lua_version (lua_State *L) {
  UNUSED(L);
  بازگشت LUA_VERSION_رقم;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API صحیح lua_absindex (lua_State *L, صحیح idx) {
  بازگشت (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->بالاترین.p - L->ci->func.p) + idx;
}


LUA_API صحیح lua_gettop (lua_State *L) {
  بازگشت cast_int(L->بالاترین.p - (L->ci->func.p + 1));
}


LUA_API تهی lua_settop (lua_State *L, صحیح idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  اگر (idx >= 0) {
    api_check(L, idx <= ci->بالاترین.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->بالاترین.p;
    برای (; diff > 0; diff--)
      setnilvalue2s(L->بالاترین.p++);  /* clear new slots */
  }
  وگرنه {
    api_check(L, -(idx+1) <= (L->بالاترین.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->بالاترین.p + diff;
  اگر (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->بالاترین.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API تهی lua_closeslot (lua_State *L, صحیح idx) {
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
ایستا تهی reverse (lua_State *L, StkId from, StkId to) {
  برای (; from < to; from++, to--) {
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
LUA_API تهی lua_rotate (lua_State *L, صحیح idx, صحیح n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->بالاترین.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API تهی lua_copy (lua_State *L, صحیح fromidx, صحیح toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  اگر (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API تهی lua_pushvalue (lua_State *L, صحیح idx) {
  lua_lock(L);
  setobj2s(L, L->بالاترین.p, index2value(L, idx));
  api_incr_بالاترین(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API صحیح lua_type (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ثابت نویسه *lua_typename (lua_State *L, صحیح t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  بازگشت ttypename(t);
}


LUA_API صحیح lua_iscfunction (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API صحیح lua_isinteger (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت ttisinteger(o);
}


LUA_API صحیح lua_isnumber (lua_State *L, صحیح idx) {
  lua_عدد n;
  ثابت TValue *o = index2value(L, idx);
  بازگشت tonumber(o, &n);
}


LUA_API صحیح lua_isstring (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت (ttisstring(o) || cvt2str(o));
}


LUA_API صحیح lua_isuserdata (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API صحیح lua_rawequal (lua_State *L, صحیح index1, صحیح index2) {
  ثابت TValue *o1 = index2value(L, index1);
  ثابت TValue *o2 = index2value(L, index2);
  بازگشت (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API تهی lua_arith (lua_State *L, صحیح op) {
  lua_lock(L);
  اگر (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  وگرنه {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->بالاترین.p, L->بالاترین.p - 1);
    api_incr_بالاترین(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->بالاترین.p - 2), s2v(L->بالاترین.p - 1), L->بالاترین.p - 2);
  L->بالاترین.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API صحیح lua_compare (lua_State *L, صحیح index1, صحیح index2, صحیح op) {
  ثابت TValue *o1;
  ثابت TValue *o2;
  صحیح i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  اگر (isvalid(L, o1) && isvalid(L, o2)) {
    انتخاب (op) {
      حالت LUA_OPEQ: i = luaV_equalobj(L, o1, o2); بشکن;
      حالت LUA_OPLT: i = luaV_lessthan(L, o1, o2); بشکن;
      حالت LUA_OPLE: i = luaV_lessequal(L, o1, o2); بشکن;
      پیشفرض: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  بازگشت i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, صحیح idx, نویسه *buff) {
  ثابت TValue *o = index2value(L, idx);
  اگر (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    بازگشت len;
  }
  وگرنه
    بازگشت 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ثابت نویسه *s) {
  size_t sz = luaO_str2num(s, s2v(L->بالاترین.p));
  اگر (sz != 0)
    api_incr_بالاترین(L);
  بازگشت sz;
}


LUA_API lua_عدد lua_tonumberx (lua_State *L, صحیح idx, صحیح *pisnum) {
  lua_عدد n = 0;
  ثابت TValue *o = index2value(L, idx);
  صحیح isnum = tonumber(o, &n);
  اگر (pisnum)
    *pisnum = isnum;
  بازگشت n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, صحیح idx, صحیح *pisnum) {
  lua_Integer res = 0;
  ثابت TValue *o = index2value(L, idx);
  صحیح isnum = tointeger(o, &res);
  اگر (pisnum)
    *pisnum = isnum;
  بازگشت res;
}


LUA_API صحیح lua_toboolean (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت !l_isfalse(o);
}


LUA_API ثابت نویسه *lua_tolstring (lua_State *L, صحیح idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  اگر (!ttisstring(o)) {
    اگر (!cvt2str(o)) {  /* not convertible? */
      اگر (len != پوچ) *len = 0;
      lua_unlock(L);
      بازگشت پوچ;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  اگر (len != پوچ)
    بازگشت getlstr(tsvalue(o), *len);
  وگرنه
    بازگشت getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  انتخاب (ttypetag(o)) {
    حالت LUA_VSHRSTR: بازگشت cast(lua_Unsigned, tsvalue(o)->shrlen);
    حالت LUA_VLNGSTR: بازگشت cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    حالت LUA_VUSERDATA: بازگشت cast(lua_Unsigned, uvalue(o)->len);
    حالت LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      بازگشت res;
    }
    پیشفرض: بازگشت 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  اگر (ttislcf(o)) بازگشت fvalue(o);
  وگرنه اگر (ttisCclosure(o))
    بازگشت clCvalue(o)->f;
  وگرنه بازگشت پوچ;  /* not a C function */
}


l_sinline تهی *touserdata (ثابت TValue *o) {
  انتخاب (ttype(o)) {
    حالت LUA_TUSERDATA: بازگشت getudatamem(uvalue(o));
    حالت LUA_TLIGHTUSERDATA: بازگشت pvalue(o);
    پیشفرض: بازگشت پوچ;
  }
}


LUA_API تهی *lua_touserdata (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  بازگشت (!ttisthread(o)) ? پوچ : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ثابت تهی *lua_topointer (lua_State *L, صحیح idx) {
  ثابت TValue *o = index2value(L, idx);
  انتخاب (ttypetag(o)) {
    حالت LUA_VLCF: بازگشت cast_voidp(cast_sizet(fvalue(o)));
    حالت LUA_VUSERDATA: حالت LUA_VLIGHTUSERDATA:
      بازگشت touserdata(o);
    پیشفرض: {
      اگر (iscollectable(o))
        بازگشت gcvalue(o);
      وگرنه
        بازگشت پوچ;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API تهی lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->بالاترین.p);
  api_incr_بالاترین(L);
  lua_unlock(L);
}


LUA_API تهی lua_pushnumber (lua_State *L, lua_عدد n) {
  lua_lock(L);
  setfltvalue(s2v(L->بالاترین.p), n);
  api_incr_بالاترین(L);
  lua_unlock(L);
}


LUA_API تهی lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->بالاترین.p), n);
  api_incr_بالاترین(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ثابت نویسه *lua_pushlstring (lua_State *L, ثابت نویسه *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->بالاترین.p, ts);
  api_incr_بالاترین(L);
  luaC_checkGC(L);
  lua_unlock(L);
  بازگشت getstr(ts);
}


LUA_API ثابت نویسه *lua_pushexternalstring (lua_State *L,
	        ثابت نویسه *s, size_t len, lua_Alloc falloc, تهی *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->بالاترین.p, ts);
  api_incr_بالاترین(L);
  luaC_checkGC(L);
  lua_unlock(L);
  بازگشت getstr(ts);
}


LUA_API ثابت نویسه *lua_pushstring (lua_State *L, ثابت نویسه *s) {
  lua_lock(L);
  اگر (s == پوچ)
    setnilvalue2s(L->بالاترین.p);
  وگرنه {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->بالاترین.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_بالاترین(L);
  luaC_checkGC(L);
  lua_unlock(L);
  بازگشت s;
}


LUA_API ثابت نویسه *lua_pushvfstring (lua_State *L, ثابت نویسه *fmt,
                                      va_list argp) {
  ثابت نویسه *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  بازگشت ret;
}


LUA_API ثابت نویسه *lua_pushfstring (lua_State *L, ثابت نویسه *fmt, ...) {
  ثابت نویسه *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  بازگشت ret;
}


LUA_API تهی lua_pushcclosure (lua_State *L, lua_CFunction fn, صحیح n) {
  lua_lock(L);
  اگر (n == 0) {
    setfvalue(s2v(L->بالاترین.p), fn);
    api_incr_بالاترین(L);
  }
  وگرنه {
    صحیح i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    برای (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->بالاترین.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->بالاترین.p -= n;
    setclCvalue(L, s2v(L->بالاترین.p), cl);
    api_incr_بالاترین(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API تهی lua_pushboolean (lua_State *L, صحیح b) {
  lua_lock(L);
  اگر (b)
    setbtvalue(s2v(L->بالاترین.p));
  وگرنه
    setbfvalue(s2v(L->بالاترین.p));
  api_incr_بالاترین(L);
  lua_unlock(L);
}


LUA_API تهی lua_pushlightuserdata (lua_State *L, تهی *p) {
  lua_lock(L);
  setpvalue(s2v(L->بالاترین.p), p);
  api_incr_بالاترین(L);
  lua_unlock(L);
}


LUA_API صحیح lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->بالاترین.p), L);
  api_incr_بالاترین(L);
  lua_unlock(L);
  بازگشت (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


ایستا صحیح auxgetstr (lua_State *L, ثابت TValue *t, ثابت نویسه *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->بالاترین.p), luaH_getstr, tag);
  اگر (!tagisempty(tag))
    api_incr_بالاترین(L);
  وگرنه {
    setsvalue2s(L, L->بالاترین.p, str);
    api_incr_بالاترین(L);
    tag = luaV_finishget(L, t, s2v(L->بالاترین.p - 1), L->بالاترین.p - 1, tag);
  }
  lua_unlock(L);
  بازگشت novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
ایستا تهی getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (تهی)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API صحیح lua_getglobal (lua_State *L, ثابت نویسه *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  بازگشت auxgetstr(L, &gt, name);
}


LUA_API صحیح lua_gettable (lua_State *L, صحیح idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->بالاترین.p - 1), s2v(L->بالاترین.p - 1), luaH_get, tag);
  اگر (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->بالاترین.p - 1), L->بالاترین.p - 1, tag);
  lua_unlock(L);
  بازگشت novariant(tag);
}


LUA_API صحیح lua_getfield (lua_State *L, صحیح idx, ثابت نویسه *k) {
  lua_lock(L);
  بازگشت auxgetstr(L, index2value(L, idx), k);
}


LUA_API صحیح lua_geti (lua_State *L, صحیح idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->بالاترین.p), tag);
  اگر (tagisempty(tag)) {
    TValue کلید;
    setivalue(&کلید, n);
    tag = luaV_finishget(L, t, &کلید, L->بالاترین.p, tag);
  }
  api_incr_بالاترین(L);
  lua_unlock(L);
  بازگشت novariant(tag);
}


ایستا صحیح finishrawget (lua_State *L, lu_byte tag) {
  اگر (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->بالاترین.p);
  api_incr_بالاترین(L);
  lua_unlock(L);
  بازگشت novariant(tag);
}


l_sinline Table *gettable (lua_State *L, صحیح idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  بازگشت hvalue(t);
}


LUA_API صحیح lua_rawget (lua_State *L, صحیح idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->بالاترین.p - 1), s2v(L->بالاترین.p - 1));
  L->بالاترین.p--;  /* pop key */
  بازگشت finishrawget(L, tag);
}


LUA_API صحیح lua_rawgeti (lua_State *L, صحیح idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->بالاترین.p), tag);
  بازگشت finishrawget(L, tag);
}


LUA_API صحیح lua_rawgetp (lua_State *L, صحیح idx, ثابت تهی *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  بازگشت finishrawget(L, luaH_get(t, &k, s2v(L->بالاترین.p)));
}


LUA_API تهی lua_createtable (lua_State *L, صحیح narray, صحیح nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->بالاترین.p, t);
  api_incr_بالاترین(L);
  اگر (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API صحیح lua_getmetatable (lua_State *L, صحیح objindex) {
  ثابت TValue *obj;
  Table *mt;
  صحیح res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  انتخاب (ttype(obj)) {
    حالت LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      بشکن;
    حالت LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      بشکن;
    پیشفرض:
      mt = G(L)->mt[ttype(obj)];
      بشکن;
  }
  اگر (mt != پوچ) {
    sethvalue2s(L, L->بالاترین.p, mt);
    api_incr_بالاترین(L);
    res = 1;
  }
  lua_unlock(L);
  بازگشت res;
}


LUA_API صحیح lua_getiuservalue (lua_State *L, صحیح idx, صحیح n) {
  TValue *o;
  صحیح t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  اگر (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->بالاترین.p);
    t = LUA_TNONE;
  }
  وگرنه {
    setobj2s(L, L->بالاترین.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->بالاترین.p));
  }
  api_incr_بالاترین(L);
  lua_unlock(L);
  بازگشت t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
ایستا تهی auxsetstr (lua_State *L, ثابت TValue *t, ثابت نویسه *k) {
  صحیح hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->بالاترین.p - 1), hres, luaH_psetstr);
  اگر (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->بالاترین.p - 1));
    L->بالاترین.p--;  /* pop value */
  }
  وگرنه {
    setsvalue2s(L, L->بالاترین.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_بالاترین(L);
    luaV_finishset(L, t, s2v(L->بالاترین.p - 1), s2v(L->بالاترین.p - 2), hres);
    L->بالاترین.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API تهی lua_setglobal (lua_State *L, ثابت نویسه *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API تهی lua_settable (lua_State *L, صحیح idx) {
  TValue *t;
  صحیح hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->بالاترین.p - 2), s2v(L->بالاترین.p - 1), hres, luaH_pset);
  اگر (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->بالاترین.p - 1));
  وگرنه
    luaV_finishset(L, t, s2v(L->بالاترین.p - 2), s2v(L->بالاترین.p - 1), hres);
  L->بالاترین.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API تهی lua_setfield (lua_State *L, صحیح idx, ثابت نویسه *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API تهی lua_seti (lua_State *L, صحیح idx, lua_Integer n) {
  TValue *t;
  صحیح hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->بالاترین.p - 1), hres);
  اگر (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->بالاترین.p - 1));
  وگرنه {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->بالاترین.p - 1), hres);
  }
  L->بالاترین.p--;  /* pop value */
  lua_unlock(L);
}


ایستا تهی aux_rawset (lua_State *L, صحیح idx, TValue *کلید, صحیح n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, کلید, s2v(L->بالاترین.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->بالاترین.p - 1));
  L->بالاترین.p -= n;
  lua_unlock(L);
}


LUA_API تهی lua_rawset (lua_State *L, صحیح idx) {
  aux_rawset(L, idx, s2v(L->بالاترین.p - 2), 2);
}


LUA_API تهی lua_rawsetp (lua_State *L, صحیح idx, ثابت تهی *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API تهی lua_rawseti (lua_State *L, صحیح idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->بالاترین.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->بالاترین.p - 1));
  L->بالاترین.p--;
  lua_unlock(L);
}


LUA_API صحیح lua_setmetatable (lua_State *L, صحیح objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  اگر (ttisnil(s2v(L->بالاترین.p - 1)))
    mt = پوچ;
  وگرنه {
    api_check(L, ttistable(s2v(L->بالاترین.p - 1)), "table expected");
    mt = hvalue(s2v(L->بالاترین.p - 1));
  }
  انتخاب (ttype(obj)) {
    حالت LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      اگر (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      بشکن;
    }
    حالت LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      اگر (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      بشکن;
    }
    پیشفرض: {
      G(L)->mt[ttype(obj)] = mt;
      بشکن;
    }
  }
  L->بالاترین.p--;
  lua_unlock(L);
  بازگشت 1;
}


LUA_API صحیح lua_setiuservalue (lua_State *L, صحیح idx, صحیح n) {
  TValue *o;
  صحیح res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  اگر (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  وگرنه {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->بالاترین.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->بالاترین.p - 1));
    res = 1;
  }
  L->بالاترین.p--;
  lua_unlock(L);
  بازگشت res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->بالاترین.p - L->بالاترین.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API تهی lua_callk (lua_State *L, صحیح nargs, صحیح nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == پوچ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->بالاترین.p - (nargs+1);
  اگر (k != پوچ && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  وگرنه  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
ساختار CallS {  /* data to 'f_call' */
  StkId func;
  صحیح nresults;
};


ایستا تهی f_call (lua_State *L, تهی *ud) {
  ساختار CallS *c = cast(ساختار CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API صحیح lua_pcallk (lua_State *L, صحیح nargs, صحیح nresults, صحیح errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  ساختار CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == پوچ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  اگر (errfunc == 0)
    func = 0;
  وگرنه {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->بالاترین.p - (nargs+1);  /* function to be called */
  اگر (k == پوچ || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  وگرنه {  /* prepare continuation (call is already protected by 'resume') */
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
  بازگشت APIstatus(status);
}


LUA_API صحیح lua_load (lua_State *L, lua_Reader reader, تهی *data,
                      ثابت نویسه *chunkname, ثابت نویسه *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  اگر (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  اگر (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->بالاترین.p - 1));  /* get new function */
    اگر (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  بازگشت APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API صحیح lua_dump (lua_State *L, lua_Writer writer, تهی *data, صحیح strip) {
  صحیح status;
  ptrdiff_t otop = savestack(L, L->بالاترین.p);  /* original top */
  TValue *f = s2v(L->بالاترین.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->بالاترین.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  بازگشت status;
}


LUA_API صحیح lua_status (lua_State *L) {
  بازگشت APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API صحیح lua_gc (lua_State *L, صحیح what, ...) {
  va_list argp;
  صحیح res = 0;
  global_State *g = G(L);
  اگر (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    بازگشت -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  انتخاب (what) {
    حالت LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      بشکن;
    }
    حالت LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      بشکن;
    }
    حالت LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      بشکن;
    }
    حالت LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      بشکن;
    }
    حالت LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      بشکن;
    }
    حالت LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      صحیح work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      اگر (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      وگرنه اگر (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      وگرنه  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (تهی)0, work = 1);
      اگر (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      بشکن;
    }
    حالت LUA_GCISRUNNING: {
      res = gcrunning(g);
      بشکن;
    }
    حالت LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      بشکن;
    }
    حالت LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      بشکن;
    }
    حالت LUA_GCPARAM: {
      صحیح param = va_arg(argp, صحیح);
      صحیح مقدار = va_arg(argp, صحیح);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      اگر (مقدار >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(مقدار));
      بشکن;
    }
    پیشفرض: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  بازگشت res;
}



/*
** miscellaneous functions
*/


LUA_API صحیح lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->بالاترین.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  اگر (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  وگرنه
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  بازگشت 0;  /* to avoid warnings */
}


LUA_API صحیح lua_next (lua_State *L, صحیح idx) {
  Table *t;
  صحیح more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->بالاترین.p - 1);
  اگر (more)
    api_incr_بالاترین(L);
  وگرنه  /* no more elements */
    L->بالاترین.p--;  /* pop key */
  lua_unlock(L);
  بازگشت more;
}


LUA_API تهی lua_toclose (lua_State *L, صحیح idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API تهی lua_concat (lua_State *L, صحیح n) {
  lua_lock(L);
  اگر (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  وگرنه {  /* nothing to concatenate */
    setsvalue2s(L, L->بالاترین.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_بالاترین(L);
  }
  lua_unlock(L);
}


LUA_API تهی lua_len (lua_State *L, صحیح idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->بالاترین.p, t);
  api_incr_بالاترین(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, تهی **ud) {
  lua_Alloc f;
  lua_lock(L);
  اگر (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  بازگشت f;
}


LUA_API تهی lua_setallocf (lua_State *L, lua_Alloc f, تهی *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


تهی lua_setwarnf (lua_State *L, lua_WarnFunction f, تهی *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


تهی lua_warning (lua_State *L, ثابت نویسه *پیام, صحیح tocont) {
  lua_lock(L);
  luaE_warning(L, پیام, tocont);
  lua_unlock(L);
}



LUA_API تهی *lua_newuserdatauv (lua_State *L, size_t size, صحیح nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->بالاترین.p), u);
  api_incr_بالاترین(L);
  luaC_checkGC(L);
  lua_unlock(L);
  بازگشت getudatamem(u);
}



ایستا ثابت نویسه *aux_upvalue (TValue *fi, صحیح n, TValue **val,
                                GCObject **owner) {
  انتخاب (ttypetag(fi)) {
    حالت LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      اگر (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        بازگشت پوچ;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      اگر (owner) *owner = obj2gco(f);
      بازگشت "";
    }
    حالت LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      اگر (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        بازگشت پوچ;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      اگر (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      بازگشت (name == پوچ) ? "(no name)" : getstr(name);
    }
    پیشفرض: بازگشت پوچ;  /* not a closure */
  }
}


LUA_API ثابت نویسه *lua_getupvalue (lua_State *L, صحیح funcindex, صحیح n) {
  ثابت نویسه *name;
  TValue *val = پوچ;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, پوچ);
  اگر (name) {
    setobj2s(L, L->بالاترین.p, val);
    api_incr_بالاترین(L);
  }
  lua_unlock(L);
  بازگشت name;
}


LUA_API ثابت نویسه *lua_setupvalue (lua_State *L, صحیح funcindex, صحیح n) {
  ثابت نویسه *name;
  TValue *val = پوچ;  /* to avoid warnings */
  GCObject *owner = پوچ;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  اگر (name) {
    L->بالاترین.p--;
    setobj(L, val, s2v(L->بالاترین.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  بازگشت name;
}


ایستا UpVal **getupvalref (lua_State *L, صحیح fidx, صحیح n, LClosure **pf) {
  ایستا ثابت UpVal *ثابت nullup = پوچ;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  اگر (pf) *pf = f;
  اگر (1 <= n && n <= f->p->sizeupvalues)
    بازگشت &f->upvals[n - 1];  /* get its upvalue pointer */
  وگرنه
    بازگشت (UpVal**)&nullup;
}


LUA_API تهی *lua_upvalueid (lua_State *L, صحیح fidx, صحیح n) {
  TValue *fi = index2value(L, fidx);
  انتخاب (ttypetag(fi)) {
    حالت LUA_VLCL: {  /* lua closure */
      بازگشت *getupvalref(L, fidx, n, پوچ);
    }
    حالت LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      اگر (1 <= n && n <= f->nupvalues)
        بازگشت &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    حالت LUA_VLCF:
      بازگشت پوچ;  /* light C functions have no upvalues */
    پیشفرض: {
      api_check(L, 0, "function expected");
      بازگشت پوچ;
    }
  }
}


LUA_API تهی lua_upvaluejoin (lua_State *L, صحیح fidx1, صحیح n1,
                                            صحیح fidx2, صحیح n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, پوچ);
  api_check(L, *up1 != پوچ && *up2 != پوچ, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


