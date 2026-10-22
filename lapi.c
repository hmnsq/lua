/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lapi.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "ltm.h"
#அந்தர்பாவ "lundump.h"
#அந்தர்பாவ "lvm.h"



மாறிலி எழுத்து lua_ident[] =
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
நிலையான TValue *index2value (lua_State *L, முழு idx) {
  CallInfo *ci = L->ci;
  எனில் (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->உச்சி.p - (ci->func.p + 1), "unacceptable index");
    எனில் (o >= L->உச்சி.p) திரும்பு &G(L)->nilvalue;
    இல்லையெனில் திரும்பு s2v(o);
  }
  இல்லையெனில் எனில் (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->உச்சி.p - (ci->func.p + 1),
                 "invalid index");
    திரும்பு s2v(L->உச்சி.p + idx);
  }
  இல்லையெனில் எனில் (idx == LUA_REGISTRYINDEX)
    திரும்பு &G(L)->l_registry;
  இல்லையெனில் {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    எனில் (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      திரும்பு (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    இல்லையெனில் {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      திரும்பு &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
நிலையான StkId index2stack (lua_State *L, முழு idx) {
  CallInfo *ci = L->ci;
  எனில் (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->உச்சி.p, "invalid index");
    திரும்பு o;
  }
  இல்லையெனில் {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->உச்சி.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    திரும்பு L->உச்சி.p + idx;
  }
}


LUA_API முழு lua_checkstack (lua_State *L, முழு n) {
  முழு res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  எனில் (L->அடுக்கு_last.p - L->உச்சி.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  இல்லையெனில்  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  எனில் (res && ci->உச்சி.p < L->உச்சி.p + n)
    ci->உச்சி.p = L->உச்சி.p + n;  /* adjust frame top */
  lua_unlock(L);
  திரும்பு res;
}


LUA_API வெறுமை lua_xmove (lua_State *from, lua_State *to, முழு n) {
  முழு i;
  எனில் (from == to) திரும்பு;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->உச்சி.p - to->உச்சி.p >= n, "stack overflow");
  from->உச்சி.p -= n;
  க்கு (i = 0; i < n; i++) {
    setobjs2s(to, to->உச்சி.p, from->உச்சி.p + i);
    to->உச்சி.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  திரும்பு old;
}


LUA_API lua_எண் lua_version (lua_State *L) {
  UNUSED(L);
  திரும்பு LUA_VERSION_இலக்கம்;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API முழு lua_absindex (lua_State *L, முழு idx) {
  திரும்பு (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->உச்சி.p - L->ci->func.p) + idx;
}


LUA_API முழு lua_gettop (lua_State *L) {
  திரும்பு cast_int(L->உச்சி.p - (L->ci->func.p + 1));
}


LUA_API வெறுமை lua_settop (lua_State *L, முழு idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  எனில் (idx >= 0) {
    api_check(L, idx <= ci->உச்சி.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->உச்சி.p;
    க்கு (; diff > 0; diff--)
      setnilvalue2s(L->உச்சி.p++);  /* clear new slots */
  }
  இல்லையெனில் {
    api_check(L, -(idx+1) <= (L->உச்சி.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->உச்சி.p + diff;
  எனில் (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->உச்சி.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API வெறுமை lua_closeslot (lua_State *L, முழு idx) {
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
நிலையான வெறுமை reverse (lua_State *L, StkId from, StkId to) {
  க்கு (; from < to; from++, to--) {
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
LUA_API வெறுமை lua_rotate (lua_State *L, முழு idx, முழு n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->உச்சி.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API வெறுமை lua_copy (lua_State *L, முழு fromidx, முழு toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  எனில் (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API வெறுமை lua_pushvalue (lua_State *L, முழு idx) {
  lua_lock(L);
  setobj2s(L, L->உச்சி.p, index2value(L, idx));
  api_incr_உச்சி(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API முழு lua_type (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API மாறிலி எழுத்து *lua_typename (lua_State *L, முழு t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  திரும்பு ttypename(t);
}


LUA_API முழு lua_iscfunction (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API முழு lua_isinteger (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு ttisinteger(o);
}


LUA_API முழு lua_isnumber (lua_State *L, முழு idx) {
  lua_எண் n;
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு tonumber(o, &n);
}


LUA_API முழு lua_isstring (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு (ttisstring(o) || cvt2str(o));
}


LUA_API முழு lua_isuserdata (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API முழு lua_rawequal (lua_State *L, முழு index1, முழு index2) {
  மாறிலி TValue *o1 = index2value(L, index1);
  மாறிலி TValue *o2 = index2value(L, index2);
  திரும்பு (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API வெறுமை lua_arith (lua_State *L, முழு op) {
  lua_lock(L);
  எனில் (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  இல்லையெனில் {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->உச்சி.p, L->உச்சி.p - 1);
    api_incr_உச்சி(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->உச்சி.p - 2), s2v(L->உச்சி.p - 1), L->உச்சி.p - 2);
  L->உச்சி.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API முழு lua_compare (lua_State *L, முழு index1, முழு index2, முழு op) {
  மாறிலி TValue *o1;
  மாறிலி TValue *o2;
  முழு i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  எனில் (isvalid(L, o1) && isvalid(L, o2)) {
    தேர்வு (op) {
      நிலை LUA_OPEQ: i = luaV_equalobj(L, o1, o2); நிறுத்து;
      நிலை LUA_OPLT: i = luaV_lessthan(L, o1, o2); நிறுத்து;
      நிலை LUA_OPLE: i = luaV_lessequal(L, o1, o2); நிறுத்து;
      இயல்பு: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  திரும்பு i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, முழு idx, எழுத்து *buff) {
  மாறிலி TValue *o = index2value(L, idx);
  எனில் (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    திரும்பு len;
  }
  இல்லையெனில்
    திரும்பு 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, மாறிலி எழுத்து *s) {
  size_t sz = luaO_str2num(s, s2v(L->உச்சி.p));
  எனில் (sz != 0)
    api_incr_உச்சி(L);
  திரும்பு sz;
}


LUA_API lua_எண் lua_tonumberx (lua_State *L, முழு idx, முழு *pisnum) {
  lua_எண் n = 0;
  மாறிலி TValue *o = index2value(L, idx);
  முழு isnum = tonumber(o, &n);
  எனில் (pisnum)
    *pisnum = isnum;
  திரும்பு n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, முழு idx, முழு *pisnum) {
  lua_Integer res = 0;
  மாறிலி TValue *o = index2value(L, idx);
  முழு isnum = tointeger(o, &res);
  எனில் (pisnum)
    *pisnum = isnum;
  திரும்பு res;
}


LUA_API முழு lua_toboolean (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு !l_isfalse(o);
}


LUA_API மாறிலி எழுத்து *lua_tolstring (lua_State *L, முழு idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  எனில் (!ttisstring(o)) {
    எனில் (!cvt2str(o)) {  /* not convertible? */
      எனில் (len != வெற்று) *len = 0;
      lua_unlock(L);
      திரும்பு வெற்று;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  எனில் (len != வெற்று)
    திரும்பு getlstr(tsvalue(o), *len);
  இல்லையெனில்
    திரும்பு getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  தேர்வு (ttypetag(o)) {
    நிலை LUA_VSHRSTR: திரும்பு cast(lua_Unsigned, tsvalue(o)->shrlen);
    நிலை LUA_VLNGSTR: திரும்பு cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    நிலை LUA_VUSERDATA: திரும்பு cast(lua_Unsigned, uvalue(o)->len);
    நிலை LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      திரும்பு res;
    }
    இயல்பு: திரும்பு 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  எனில் (ttislcf(o)) திரும்பு fvalue(o);
  இல்லையெனில் எனில் (ttisCclosure(o))
    திரும்பு clCvalue(o)->f;
  இல்லையெனில் திரும்பு வெற்று;  /* not a C function */
}


l_sinline வெறுமை *touserdata (மாறிலி TValue *o) {
  தேர்வு (ttype(o)) {
    நிலை LUA_TUSERDATA: திரும்பு getudatamem(uvalue(o));
    நிலை LUA_TLIGHTUSERDATA: திரும்பு pvalue(o);
    இயல்பு: திரும்பு வெற்று;
  }
}


LUA_API வெறுமை *lua_touserdata (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  திரும்பு (!ttisthread(o)) ? வெற்று : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API மாறிலி வெறுமை *lua_topointer (lua_State *L, முழு idx) {
  மாறிலி TValue *o = index2value(L, idx);
  தேர்வு (ttypetag(o)) {
    நிலை LUA_VLCF: திரும்பு cast_voidp(cast_sizet(fvalue(o)));
    நிலை LUA_VUSERDATA: நிலை LUA_VLIGHTUSERDATA:
      திரும்பு touserdata(o);
    இயல்பு: {
      எனில் (iscollectable(o))
        திரும்பு gcvalue(o);
      இல்லையெனில்
        திரும்பு வெற்று;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API வெறுமை lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->உச்சி.p);
  api_incr_உச்சி(L);
  lua_unlock(L);
}


LUA_API வெறுமை lua_pushnumber (lua_State *L, lua_எண் n) {
  lua_lock(L);
  setfltvalue(s2v(L->உச்சி.p), n);
  api_incr_உச்சி(L);
  lua_unlock(L);
}


LUA_API வெறுமை lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->உச்சி.p), n);
  api_incr_உச்சி(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API மாறிலி எழுத்து *lua_pushlstring (lua_State *L, மாறிலி எழுத்து *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->உச்சி.p, ts);
  api_incr_உச்சி(L);
  luaC_checkGC(L);
  lua_unlock(L);
  திரும்பு getstr(ts);
}


LUA_API மாறிலி எழுத்து *lua_pushexternalstring (lua_State *L,
	        மாறிலி எழுத்து *s, size_t len, lua_Alloc falloc, வெறுமை *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->உச்சி.p, ts);
  api_incr_உச்சி(L);
  luaC_checkGC(L);
  lua_unlock(L);
  திரும்பு getstr(ts);
}


LUA_API மாறிலி எழுத்து *lua_pushstring (lua_State *L, மாறிலி எழுத்து *s) {
  lua_lock(L);
  எனில் (s == வெற்று)
    setnilvalue2s(L->உச்சி.p);
  இல்லையெனில் {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->உச்சி.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_உச்சி(L);
  luaC_checkGC(L);
  lua_unlock(L);
  திரும்பு s;
}


LUA_API மாறிலி எழுத்து *lua_pushvfstring (lua_State *L, மாறிலி எழுத்து *fmt,
                                      va_list argp) {
  மாறிலி எழுத்து *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  திரும்பு ret;
}


LUA_API மாறிலி எழுத்து *lua_pushfstring (lua_State *L, மாறிலி எழுத்து *fmt, ...) {
  மாறிலி எழுத்து *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  திரும்பு ret;
}


LUA_API வெறுமை lua_pushcclosure (lua_State *L, lua_CFunction fn, முழு n) {
  lua_lock(L);
  எனில் (n == 0) {
    setfvalue(s2v(L->உச்சி.p), fn);
    api_incr_உச்சி(L);
  }
  இல்லையெனில் {
    முழு i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    க்கு (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->உச்சி.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->உச்சி.p -= n;
    setclCvalue(L, s2v(L->உச்சி.p), cl);
    api_incr_உச்சி(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API வெறுமை lua_pushboolean (lua_State *L, முழு b) {
  lua_lock(L);
  எனில் (b)
    setbtvalue(s2v(L->உச்சி.p));
  இல்லையெனில்
    setbfvalue(s2v(L->உச்சி.p));
  api_incr_உச்சி(L);
  lua_unlock(L);
}


LUA_API வெறுமை lua_pushlightuserdata (lua_State *L, வெறுமை *p) {
  lua_lock(L);
  setpvalue(s2v(L->உச்சி.p), p);
  api_incr_உச்சி(L);
  lua_unlock(L);
}


LUA_API முழு lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->உச்சி.p), L);
  api_incr_உச்சி(L);
  lua_unlock(L);
  திரும்பு (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


நிலையான முழு auxgetstr (lua_State *L, மாறிலி TValue *t, மாறிலி எழுத்து *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->உச்சி.p), luaH_getstr, tag);
  எனில் (!tagisempty(tag))
    api_incr_உச்சி(L);
  இல்லையெனில் {
    setsvalue2s(L, L->உச்சி.p, str);
    api_incr_உச்சி(L);
    tag = luaV_finishget(L, t, s2v(L->உச்சி.p - 1), L->உச்சி.p - 1, tag);
  }
  lua_unlock(L);
  திரும்பு novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
நிலையான வெறுமை getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (வெறுமை)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API முழு lua_getglobal (lua_State *L, மாறிலி எழுத்து *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  திரும்பு auxgetstr(L, &gt, name);
}


LUA_API முழு lua_gettable (lua_State *L, முழு idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->உச்சி.p - 1), s2v(L->உச்சி.p - 1), luaH_get, tag);
  எனில் (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->உச்சி.p - 1), L->உச்சி.p - 1, tag);
  lua_unlock(L);
  திரும்பு novariant(tag);
}


LUA_API முழு lua_getfield (lua_State *L, முழு idx, மாறிலி எழுத்து *k) {
  lua_lock(L);
  திரும்பு auxgetstr(L, index2value(L, idx), k);
}


LUA_API முழு lua_geti (lua_State *L, முழு idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->உச்சி.p), tag);
  எனில் (tagisempty(tag)) {
    TValue சாவி;
    setivalue(&சாவி, n);
    tag = luaV_finishget(L, t, &சாவி, L->உச்சி.p, tag);
  }
  api_incr_உச்சி(L);
  lua_unlock(L);
  திரும்பு novariant(tag);
}


நிலையான முழு finishrawget (lua_State *L, lu_byte tag) {
  எனில் (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->உச்சி.p);
  api_incr_உச்சி(L);
  lua_unlock(L);
  திரும்பு novariant(tag);
}


l_sinline Table *gettable (lua_State *L, முழு idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  திரும்பு hvalue(t);
}


LUA_API முழு lua_rawget (lua_State *L, முழு idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->உச்சி.p - 1), s2v(L->உச்சி.p - 1));
  L->உச்சி.p--;  /* pop key */
  திரும்பு finishrawget(L, tag);
}


LUA_API முழு lua_rawgeti (lua_State *L, முழு idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->உச்சி.p), tag);
  திரும்பு finishrawget(L, tag);
}


LUA_API முழு lua_rawgetp (lua_State *L, முழு idx, மாறிலி வெறுமை *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  திரும்பு finishrawget(L, luaH_get(t, &k, s2v(L->உச்சி.p)));
}


LUA_API வெறுமை lua_createtable (lua_State *L, முழு narray, முழு nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->உச்சி.p, t);
  api_incr_உச்சி(L);
  எனில் (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API முழு lua_getmetatable (lua_State *L, முழு objindex) {
  மாறிலி TValue *obj;
  Table *mt;
  முழு res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  தேர்வு (ttype(obj)) {
    நிலை LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      நிறுத்து;
    நிலை LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      நிறுத்து;
    இயல்பு:
      mt = G(L)->mt[ttype(obj)];
      நிறுத்து;
  }
  எனில் (mt != வெற்று) {
    sethvalue2s(L, L->உச்சி.p, mt);
    api_incr_உச்சி(L);
    res = 1;
  }
  lua_unlock(L);
  திரும்பு res;
}


LUA_API முழு lua_getiuservalue (lua_State *L, முழு idx, முழு n) {
  TValue *o;
  முழு t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  எனில் (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->உச்சி.p);
    t = LUA_TNONE;
  }
  இல்லையெனில் {
    setobj2s(L, L->உச்சி.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->உச்சி.p));
  }
  api_incr_உச்சி(L);
  lua_unlock(L);
  திரும்பு t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
நிலையான வெறுமை auxsetstr (lua_State *L, மாறிலி TValue *t, மாறிலி எழுத்து *k) {
  முழு hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->உச்சி.p - 1), hres, luaH_psetstr);
  எனில் (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->உச்சி.p - 1));
    L->உச்சி.p--;  /* pop value */
  }
  இல்லையெனில் {
    setsvalue2s(L, L->உச்சி.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_உச்சி(L);
    luaV_finishset(L, t, s2v(L->உச்சி.p - 1), s2v(L->உச்சி.p - 2), hres);
    L->உச்சி.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API வெறுமை lua_setglobal (lua_State *L, மாறிலி எழுத்து *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API வெறுமை lua_settable (lua_State *L, முழு idx) {
  TValue *t;
  முழு hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->உச்சி.p - 2), s2v(L->உச்சி.p - 1), hres, luaH_pset);
  எனில் (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->உச்சி.p - 1));
  இல்லையெனில்
    luaV_finishset(L, t, s2v(L->உச்சி.p - 2), s2v(L->உச்சி.p - 1), hres);
  L->உச்சி.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API வெறுமை lua_setfield (lua_State *L, முழு idx, மாறிலி எழுத்து *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API வெறுமை lua_seti (lua_State *L, முழு idx, lua_Integer n) {
  TValue *t;
  முழு hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->உச்சி.p - 1), hres);
  எனில் (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->உச்சி.p - 1));
  இல்லையெனில் {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->உச்சி.p - 1), hres);
  }
  L->உச்சி.p--;  /* pop value */
  lua_unlock(L);
}


நிலையான வெறுமை aux_rawset (lua_State *L, முழு idx, TValue *சாவி, முழு n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, சாவி, s2v(L->உச்சி.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->உச்சி.p - 1));
  L->உச்சி.p -= n;
  lua_unlock(L);
}


LUA_API வெறுமை lua_rawset (lua_State *L, முழு idx) {
  aux_rawset(L, idx, s2v(L->உச்சி.p - 2), 2);
}


LUA_API வெறுமை lua_rawsetp (lua_State *L, முழு idx, மாறிலி வெறுமை *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API வெறுமை lua_rawseti (lua_State *L, முழு idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->உச்சி.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->உச்சி.p - 1));
  L->உச்சி.p--;
  lua_unlock(L);
}


LUA_API முழு lua_setmetatable (lua_State *L, முழு objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  எனில் (ttisnil(s2v(L->உச்சி.p - 1)))
    mt = வெற்று;
  இல்லையெனில் {
    api_check(L, ttistable(s2v(L->உச்சி.p - 1)), "table expected");
    mt = hvalue(s2v(L->உச்சி.p - 1));
  }
  தேர்வு (ttype(obj)) {
    நிலை LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      எனில் (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      நிறுத்து;
    }
    நிலை LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      எனில் (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      நிறுத்து;
    }
    இயல்பு: {
      G(L)->mt[ttype(obj)] = mt;
      நிறுத்து;
    }
  }
  L->உச்சி.p--;
  lua_unlock(L);
  திரும்பு 1;
}


LUA_API முழு lua_setiuservalue (lua_State *L, முழு idx, முழு n) {
  TValue *o;
  முழு res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  எனில் (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  இல்லையெனில் {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->உச்சி.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->உச்சி.p - 1));
    res = 1;
  }
  L->உச்சி.p--;
  lua_unlock(L);
  திரும்பு res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->உச்சி.p - L->உச்சி.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API வெறுமை lua_callk (lua_State *L, முழு nargs, முழு nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == வெற்று || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->உச்சி.p - (nargs+1);
  எனில் (k != வெற்று && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  இல்லையெனில்  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
கட்டமைப்பு CallS {  /* data to 'f_call' */
  StkId func;
  முழு nresults;
};


நிலையான வெறுமை f_call (lua_State *L, வெறுமை *ud) {
  கட்டமைப்பு CallS *c = cast(கட்டமைப்பு CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API முழு lua_pcallk (lua_State *L, முழு nargs, முழு nresults, முழு errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  கட்டமைப்பு CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == வெற்று || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  எனில் (errfunc == 0)
    func = 0;
  இல்லையெனில் {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->உச்சி.p - (nargs+1);  /* function to be called */
  எனில் (k == வெற்று || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  இல்லையெனில் {  /* prepare continuation (call is already protected by 'resume') */
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
  திரும்பு APIstatus(status);
}


LUA_API முழு lua_load (lua_State *L, lua_Reader reader, வெறுமை *data,
                      மாறிலி எழுத்து *chunkname, மாறிலி எழுத்து *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  எனில் (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  எனில் (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->உச்சி.p - 1));  /* get new function */
    எனில் (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  திரும்பு APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API முழு lua_dump (lua_State *L, lua_Writer writer, வெறுமை *data, முழு strip) {
  முழு status;
  ptrdiff_t otop = savestack(L, L->உச்சி.p);  /* original top */
  TValue *f = s2v(L->உச்சி.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->உச்சி.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  திரும்பு status;
}


LUA_API முழு lua_status (lua_State *L) {
  திரும்பு APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API முழு lua_gc (lua_State *L, முழு what, ...) {
  va_list argp;
  முழு res = 0;
  global_State *g = G(L);
  எனில் (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    திரும்பு -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  தேர்வு (what) {
    நிலை LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      நிறுத்து;
    }
    நிலை LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      நிறுத்து;
    }
    நிலை LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      நிறுத்து;
    }
    நிலை LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      நிறுத்து;
    }
    நிலை LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      நிறுத்து;
    }
    நிலை LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      முழு work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      எனில் (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      இல்லையெனில் எனில் (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      இல்லையெனில்  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (வெறுமை)0, work = 1);
      எனில் (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      நிறுத்து;
    }
    நிலை LUA_GCISRUNNING: {
      res = gcrunning(g);
      நிறுத்து;
    }
    நிலை LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      நிறுத்து;
    }
    நிலை LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      நிறுத்து;
    }
    நிலை LUA_GCPARAM: {
      முழு param = va_arg(argp, முழு);
      முழு மதிப்பு = va_arg(argp, முழு);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      எனில் (மதிப்பு >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(மதிப்பு));
      நிறுத்து;
    }
    இயல்பு: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  திரும்பு res;
}



/*
** miscellaneous functions
*/


LUA_API முழு lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->உச்சி.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  எனில் (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  இல்லையெனில்
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  திரும்பு 0;  /* to avoid warnings */
}


LUA_API முழு lua_next (lua_State *L, முழு idx) {
  Table *t;
  முழு more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->உச்சி.p - 1);
  எனில் (more)
    api_incr_உச்சி(L);
  இல்லையெனில்  /* no more elements */
    L->உச்சி.p--;  /* pop key */
  lua_unlock(L);
  திரும்பு more;
}


LUA_API வெறுமை lua_toclose (lua_State *L, முழு idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API வெறுமை lua_concat (lua_State *L, முழு n) {
  lua_lock(L);
  எனில் (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  இல்லையெனில் {  /* nothing to concatenate */
    setsvalue2s(L, L->உச்சி.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_உச்சி(L);
  }
  lua_unlock(L);
}


LUA_API வெறுமை lua_len (lua_State *L, முழு idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->உச்சி.p, t);
  api_incr_உச்சி(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, வெறுமை **ud) {
  lua_Alloc f;
  lua_lock(L);
  எனில் (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  திரும்பு f;
}


LUA_API வெறுமை lua_setallocf (lua_State *L, lua_Alloc f, வெறுமை *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


வெறுமை lua_setwarnf (lua_State *L, lua_WarnFunction f, வெறுமை *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


வெறுமை lua_warning (lua_State *L, மாறிலி எழுத்து *செய்தி, முழு tocont) {
  lua_lock(L);
  luaE_warning(L, செய்தி, tocont);
  lua_unlock(L);
}



LUA_API வெறுமை *lua_newuserdatauv (lua_State *L, size_t size, முழு nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->உச்சி.p), u);
  api_incr_உச்சி(L);
  luaC_checkGC(L);
  lua_unlock(L);
  திரும்பு getudatamem(u);
}



நிலையான மாறிலி எழுத்து *aux_upvalue (TValue *fi, முழு n, TValue **val,
                                GCObject **owner) {
  தேர்வு (ttypetag(fi)) {
    நிலை LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      எனில் (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        திரும்பு வெற்று;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      எனில் (owner) *owner = obj2gco(f);
      திரும்பு "";
    }
    நிலை LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      எனில் (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        திரும்பு வெற்று;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      எனில் (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      திரும்பு (name == வெற்று) ? "(no name)" : getstr(name);
    }
    இயல்பு: திரும்பு வெற்று;  /* not a closure */
  }
}


LUA_API மாறிலி எழுத்து *lua_getupvalue (lua_State *L, முழு funcindex, முழு n) {
  மாறிலி எழுத்து *name;
  TValue *val = வெற்று;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, வெற்று);
  எனில் (name) {
    setobj2s(L, L->உச்சி.p, val);
    api_incr_உச்சி(L);
  }
  lua_unlock(L);
  திரும்பு name;
}


LUA_API மாறிலி எழுத்து *lua_setupvalue (lua_State *L, முழு funcindex, முழு n) {
  மாறிலி எழுத்து *name;
  TValue *val = வெற்று;  /* to avoid warnings */
  GCObject *owner = வெற்று;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  எனில் (name) {
    L->உச்சி.p--;
    setobj(L, val, s2v(L->உச்சி.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  திரும்பு name;
}


நிலையான UpVal **getupvalref (lua_State *L, முழு fidx, முழு n, LClosure **pf) {
  நிலையான மாறிலி UpVal *மாறிலி nullup = வெற்று;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  எனில் (pf) *pf = f;
  எனில் (1 <= n && n <= f->p->sizeupvalues)
    திரும்பு &f->upvals[n - 1];  /* get its upvalue pointer */
  இல்லையெனில்
    திரும்பு (UpVal**)&nullup;
}


LUA_API வெறுமை *lua_upvalueid (lua_State *L, முழு fidx, முழு n) {
  TValue *fi = index2value(L, fidx);
  தேர்வு (ttypetag(fi)) {
    நிலை LUA_VLCL: {  /* lua closure */
      திரும்பு *getupvalref(L, fidx, n, வெற்று);
    }
    நிலை LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      எனில் (1 <= n && n <= f->nupvalues)
        திரும்பு &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    நிலை LUA_VLCF:
      திரும்பு வெற்று;  /* light C functions have no upvalues */
    இயல்பு: {
      api_check(L, 0, "function expected");
      திரும்பு வெற்று;
    }
  }
}


LUA_API வெறுமை lua_upvaluejoin (lua_State *L, முழு fidx1, முழு n1,
                                            முழு fidx2, முழு n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, வெற்று);
  api_check(L, *up1 != வெற்று && *up2 != வெற்று, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


