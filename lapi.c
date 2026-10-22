/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "ltm.h"
#अन्तर्भाव "lundump.h"
#अन्तर्भाव "lvm.h"



स्थिरांक अक्षर lua_ident[] =
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
स्थिर TValue *index2value (lua_State *L, पूर्णांक idx) {
  CallInfo *ci = L->ci;
  जर (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->शीर्ष.p - (ci->func.p + 1), "unacceptable index");
    जर (o >= L->शीर्ष.p) परत &G(L)->nilvalue;
    अन्यथा परत s2v(o);
  }
  अन्यथा जर (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->शीर्ष.p - (ci->func.p + 1),
                 "invalid index");
    परत s2v(L->शीर्ष.p + idx);
  }
  अन्यथा जर (idx == LUA_REGISTRYINDEX)
    परत &G(L)->l_registry;
  अन्यथा {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    जर (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      परत (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    अन्यथा {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      परत &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
स्थिर StkId index2stack (lua_State *L, पूर्णांक idx) {
  CallInfo *ci = L->ci;
  जर (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->शीर्ष.p, "invalid index");
    परत o;
  }
  अन्यथा {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->शीर्ष.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    परत L->शीर्ष.p + idx;
  }
}


LUA_API पूर्णांक lua_checkstack (lua_State *L, पूर्णांक n) {
  पूर्णांक res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  जर (L->चळत_last.p - L->शीर्ष.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  अन्यथा  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  जर (res && ci->शीर्ष.p < L->शीर्ष.p + n)
    ci->शीर्ष.p = L->शीर्ष.p + n;  /* adjust frame top */
  lua_unlock(L);
  परत res;
}


LUA_API शून्य lua_xmove (lua_State *from, lua_State *to, पूर्णांक n) {
  पूर्णांक i;
  जर (from == to) परत;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->शीर्ष.p - to->शीर्ष.p >= n, "stack overflow");
  from->शीर्ष.p -= n;
  साठी (i = 0; i < n; i++) {
    setobjs2s(to, to->शीर्ष.p, from->शीर्ष.p + i);
    to->शीर्ष.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  परत old;
}


LUA_API lua_संख्या lua_version (lua_State *L) {
  UNUSED(L);
  परत LUA_VERSION_अंक;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API पूर्णांक lua_absindex (lua_State *L, पूर्णांक idx) {
  परत (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->शीर्ष.p - L->ci->func.p) + idx;
}


LUA_API पूर्णांक lua_gettop (lua_State *L) {
  परत cast_int(L->शीर्ष.p - (L->ci->func.p + 1));
}


LUA_API शून्य lua_settop (lua_State *L, पूर्णांक idx) {
  CallInfo *ci;
  StkId func, newtop;
  सूचक_भेद_प्रकार diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  जर (idx >= 0) {
    api_check(L, idx <= ci->शीर्ष.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->शीर्ष.p;
    साठी (; diff > 0; diff--)
      setnilvalue2s(L->शीर्ष.p++);  /* clear new slots */
  }
  अन्यथा {
    api_check(L, -(idx+1) <= (L->शीर्ष.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->शीर्ष.p + diff;
  जर (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->शीर्ष.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API शून्य lua_closeslot (lua_State *L, पूर्णांक idx) {
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
स्थिर शून्य reverse (lua_State *L, StkId from, StkId to) {
  साठी (; from < to; from++, to--) {
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
LUA_API शून्य lua_rotate (lua_State *L, पूर्णांक idx, पूर्णांक n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->शीर्ष.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API शून्य lua_copy (lua_State *L, पूर्णांक fromidx, पूर्णांक toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  जर (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API शून्य lua_pushvalue (lua_State *L, पूर्णांक idx) {
  lua_lock(L);
  setobj2s(L, L->शीर्ष.p, index2value(L, idx));
  api_incr_शीर्ष(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API पूर्णांक lua_type (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API स्थिरांक अक्षर *lua_typename (lua_State *L, पूर्णांक t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  परत ttypename(t);
}


LUA_API पूर्णांक lua_iscfunction (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API पूर्णांक lua_isinteger (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत ttisinteger(o);
}


LUA_API पूर्णांक lua_isnumber (lua_State *L, पूर्णांक idx) {
  lua_संख्या n;
  स्थिरांक TValue *o = index2value(L, idx);
  परत tonumber(o, &n);
}


LUA_API पूर्णांक lua_isstring (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत (ttisstring(o) || cvt2str(o));
}


LUA_API पूर्णांक lua_isuserdata (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API पूर्णांक lua_rawequal (lua_State *L, पूर्णांक index1, पूर्णांक index2) {
  स्थिरांक TValue *o1 = index2value(L, index1);
  स्थिरांक TValue *o2 = index2value(L, index2);
  परत (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API शून्य lua_arith (lua_State *L, पूर्णांक op) {
  lua_lock(L);
  जर (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  अन्यथा {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->शीर्ष.p, L->शीर्ष.p - 1);
    api_incr_शीर्ष(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->शीर्ष.p - 2), s2v(L->शीर्ष.p - 1), L->शीर्ष.p - 2);
  L->शीर्ष.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API पूर्णांक lua_compare (lua_State *L, पूर्णांक index1, पूर्णांक index2, पूर्णांक op) {
  स्थिरांक TValue *o1;
  स्थिरांक TValue *o2;
  पूर्णांक i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  जर (isvalid(L, o1) && isvalid(L, o2)) {
    निवड (op) {
      स्थिती LUA_OPEQ: i = luaV_equalobj(L, o1, o2); थांबा;
      स्थिती LUA_OPLT: i = luaV_lessthan(L, o1, o2); थांबा;
      स्थिती LUA_OPLE: i = luaV_lessequal(L, o1, o2); थांबा;
      पूर्वनिर्धारित: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  परत i;
}


LUA_API अचिन्हित lua_numbertocstring (lua_State *L, पूर्णांक idx, अक्षर *buff) {
  स्थिरांक TValue *o = index2value(L, idx);
  जर (ttisnumber(o)) {
    अचिन्हित len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    परत len;
  }
  अन्यथा
    परत 0;
}


LUA_API माप_प्रकार lua_stringtonumber (lua_State *L, स्थिरांक अक्षर *s) {
  माप_प्रकार sz = luaO_str2num(s, s2v(L->शीर्ष.p));
  जर (sz != 0)
    api_incr_शीर्ष(L);
  परत sz;
}


LUA_API lua_संख्या lua_tonumberx (lua_State *L, पूर्णांक idx, पूर्णांक *pisnum) {
  lua_संख्या n = 0;
  स्थिरांक TValue *o = index2value(L, idx);
  पूर्णांक isnum = tonumber(o, &n);
  जर (pisnum)
    *pisnum = isnum;
  परत n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, पूर्णांक idx, पूर्णांक *pisnum) {
  lua_Integer res = 0;
  स्थिरांक TValue *o = index2value(L, idx);
  पूर्णांक isnum = tointeger(o, &res);
  जर (pisnum)
    *pisnum = isnum;
  परत res;
}


LUA_API पूर्णांक lua_toboolean (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत !l_isfalse(o);
}


LUA_API स्थिरांक अक्षर *lua_tolstring (lua_State *L, पूर्णांक idx, माप_प्रकार *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  जर (!ttisstring(o)) {
    जर (!cvt2str(o)) {  /* not convertible? */
      जर (len != रिक्त) *len = 0;
      lua_unlock(L);
      परत रिक्त;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  जर (len != रिक्त)
    परत getlstr(tsvalue(o), *len);
  अन्यथा
    परत getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  निवड (ttypetag(o)) {
    स्थिती LUA_VSHRSTR: परत cast(lua_Unsigned, tsvalue(o)->shrlen);
    स्थिती LUA_VLNGSTR: परत cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    स्थिती LUA_VUSERDATA: परत cast(lua_Unsigned, uvalue(o)->len);
    स्थिती LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      परत res;
    }
    पूर्वनिर्धारित: परत 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  जर (ttislcf(o)) परत fvalue(o);
  अन्यथा जर (ttisCclosure(o))
    परत clCvalue(o)->f;
  अन्यथा परत रिक्त;  /* not a C function */
}


l_sinline शून्य *touserdata (स्थिरांक TValue *o) {
  निवड (ttype(o)) {
    स्थिती LUA_TUSERDATA: परत getudatamem(uvalue(o));
    स्थिती LUA_TLIGHTUSERDATA: परत pvalue(o);
    पूर्वनिर्धारित: परत रिक्त;
  }
}


LUA_API शून्य *lua_touserdata (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  परत (!ttisthread(o)) ? रिक्त : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API स्थिरांक शून्य *lua_topointer (lua_State *L, पूर्णांक idx) {
  स्थिरांक TValue *o = index2value(L, idx);
  निवड (ttypetag(o)) {
    स्थिती LUA_VLCF: परत cast_voidp(cast_sizet(fvalue(o)));
    स्थिती LUA_VUSERDATA: स्थिती LUA_VLIGHTUSERDATA:
      परत touserdata(o);
    पूर्वनिर्धारित: {
      जर (iscollectable(o))
        परत gcvalue(o);
      अन्यथा
        परत रिक्त;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API शून्य lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->शीर्ष.p);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API शून्य lua_pushnumber (lua_State *L, lua_संख्या n) {
  lua_lock(L);
  setfltvalue(s2v(L->शीर्ष.p), n);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API शून्य lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->शीर्ष.p), n);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API स्थिरांक अक्षर *lua_pushlstring (lua_State *L, स्थिरांक अक्षर *s, माप_प्रकार len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->शीर्ष.p, ts);
  api_incr_शीर्ष(L);
  luaC_checkGC(L);
  lua_unlock(L);
  परत getstr(ts);
}


LUA_API स्थिरांक अक्षर *lua_pushexternalstring (lua_State *L,
	        स्थिरांक अक्षर *s, माप_प्रकार len, lua_Alloc falloc, शून्य *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->शीर्ष.p, ts);
  api_incr_शीर्ष(L);
  luaC_checkGC(L);
  lua_unlock(L);
  परत getstr(ts);
}


LUA_API स्थिरांक अक्षर *lua_pushstring (lua_State *L, स्थिरांक अक्षर *s) {
  lua_lock(L);
  जर (s == रिक्त)
    setnilvalue2s(L->शीर्ष.p);
  अन्यथा {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->शीर्ष.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_शीर्ष(L);
  luaC_checkGC(L);
  lua_unlock(L);
  परत s;
}


LUA_API स्थिरांक अक्षर *lua_pushvfstring (lua_State *L, स्थिरांक अक्षर *fmt,
                                      बहु_सूची argp) {
  स्थिरांक अक्षर *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  परत ret;
}


LUA_API स्थिरांक अक्षर *lua_pushfstring (lua_State *L, स्थिरांक अक्षर *fmt, ...) {
  स्थिरांक अक्षर *ret;
  बहु_सूची argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  परत ret;
}


LUA_API शून्य lua_pushcclosure (lua_State *L, lua_CFunction fn, पूर्णांक n) {
  lua_lock(L);
  जर (n == 0) {
    setfvalue(s2v(L->शीर्ष.p), fn);
    api_incr_शीर्ष(L);
  }
  अन्यथा {
    पूर्णांक i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    साठी (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->शीर्ष.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->शीर्ष.p -= n;
    setclCvalue(L, s2v(L->शीर्ष.p), cl);
    api_incr_शीर्ष(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API शून्य lua_pushboolean (lua_State *L, पूर्णांक b) {
  lua_lock(L);
  जर (b)
    setbtvalue(s2v(L->शीर्ष.p));
  अन्यथा
    setbfvalue(s2v(L->शीर्ष.p));
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API शून्य lua_pushlightuserdata (lua_State *L, शून्य *p) {
  lua_lock(L);
  setpvalue(s2v(L->शीर्ष.p), p);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API पूर्णांक lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->शीर्ष.p), L);
  api_incr_शीर्ष(L);
  lua_unlock(L);
  परत (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


स्थिर पूर्णांक auxgetstr (lua_State *L, स्थिरांक TValue *t, स्थिरांक अक्षर *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->शीर्ष.p), luaH_getstr, tag);
  जर (!tagisempty(tag))
    api_incr_शीर्ष(L);
  अन्यथा {
    setsvalue2s(L, L->शीर्ष.p, str);
    api_incr_शीर्ष(L);
    tag = luaV_finishget(L, t, s2v(L->शीर्ष.p - 1), L->शीर्ष.p - 1, tag);
  }
  lua_unlock(L);
  परत novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
स्थिर शून्य getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (शून्य)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API पूर्णांक lua_getglobal (lua_State *L, स्थिरांक अक्षर *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  परत auxgetstr(L, &gt, name);
}


LUA_API पूर्णांक lua_gettable (lua_State *L, पूर्णांक idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->शीर्ष.p - 1), s2v(L->शीर्ष.p - 1), luaH_get, tag);
  जर (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->शीर्ष.p - 1), L->शीर्ष.p - 1, tag);
  lua_unlock(L);
  परत novariant(tag);
}


LUA_API पूर्णांक lua_getfield (lua_State *L, पूर्णांक idx, स्थिरांक अक्षर *k) {
  lua_lock(L);
  परत auxgetstr(L, index2value(L, idx), k);
}


LUA_API पूर्णांक lua_geti (lua_State *L, पूर्णांक idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->शीर्ष.p), tag);
  जर (tagisempty(tag)) {
    TValue किल्ली;
    setivalue(&किल्ली, n);
    tag = luaV_finishget(L, t, &किल्ली, L->शीर्ष.p, tag);
  }
  api_incr_शीर्ष(L);
  lua_unlock(L);
  परत novariant(tag);
}


स्थिर पूर्णांक finishrawget (lua_State *L, lu_byte tag) {
  जर (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->शीर्ष.p);
  api_incr_शीर्ष(L);
  lua_unlock(L);
  परत novariant(tag);
}


l_sinline Table *gettable (lua_State *L, पूर्णांक idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  परत hvalue(t);
}


LUA_API पूर्णांक lua_rawget (lua_State *L, पूर्णांक idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->शीर्ष.p - 1), s2v(L->शीर्ष.p - 1));
  L->शीर्ष.p--;  /* pop key */
  परत finishrawget(L, tag);
}


LUA_API पूर्णांक lua_rawgeti (lua_State *L, पूर्णांक idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->शीर्ष.p), tag);
  परत finishrawget(L, tag);
}


LUA_API पूर्णांक lua_rawgetp (lua_State *L, पूर्णांक idx, स्थिरांक शून्य *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  परत finishrawget(L, luaH_get(t, &k, s2v(L->शीर्ष.p)));
}


LUA_API शून्य lua_createtable (lua_State *L, पूर्णांक narray, पूर्णांक nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->शीर्ष.p, t);
  api_incr_शीर्ष(L);
  जर (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API पूर्णांक lua_getmetatable (lua_State *L, पूर्णांक objindex) {
  स्थिरांक TValue *obj;
  Table *mt;
  पूर्णांक res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  निवड (ttype(obj)) {
    स्थिती LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      थांबा;
    स्थिती LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      थांबा;
    पूर्वनिर्धारित:
      mt = G(L)->mt[ttype(obj)];
      थांबा;
  }
  जर (mt != रिक्त) {
    sethvalue2s(L, L->शीर्ष.p, mt);
    api_incr_शीर्ष(L);
    res = 1;
  }
  lua_unlock(L);
  परत res;
}


LUA_API पूर्णांक lua_getiuservalue (lua_State *L, पूर्णांक idx, पूर्णांक n) {
  TValue *o;
  पूर्णांक t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  जर (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->शीर्ष.p);
    t = LUA_TNONE;
  }
  अन्यथा {
    setobj2s(L, L->शीर्ष.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->शीर्ष.p));
  }
  api_incr_शीर्ष(L);
  lua_unlock(L);
  परत t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
स्थिर शून्य auxsetstr (lua_State *L, स्थिरांक TValue *t, स्थिरांक अक्षर *k) {
  पूर्णांक hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->शीर्ष.p - 1), hres, luaH_psetstr);
  जर (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->शीर्ष.p - 1));
    L->शीर्ष.p--;  /* pop value */
  }
  अन्यथा {
    setsvalue2s(L, L->शीर्ष.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_शीर्ष(L);
    luaV_finishset(L, t, s2v(L->शीर्ष.p - 1), s2v(L->शीर्ष.p - 2), hres);
    L->शीर्ष.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API शून्य lua_setglobal (lua_State *L, स्थिरांक अक्षर *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API शून्य lua_settable (lua_State *L, पूर्णांक idx) {
  TValue *t;
  पूर्णांक hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->शीर्ष.p - 2), s2v(L->शीर्ष.p - 1), hres, luaH_pset);
  जर (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->शीर्ष.p - 1));
  अन्यथा
    luaV_finishset(L, t, s2v(L->शीर्ष.p - 2), s2v(L->शीर्ष.p - 1), hres);
  L->शीर्ष.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API शून्य lua_setfield (lua_State *L, पूर्णांक idx, स्थिरांक अक्षर *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API शून्य lua_seti (lua_State *L, पूर्णांक idx, lua_Integer n) {
  TValue *t;
  पूर्णांक hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->शीर्ष.p - 1), hres);
  जर (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->शीर्ष.p - 1));
  अन्यथा {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->शीर्ष.p - 1), hres);
  }
  L->शीर्ष.p--;  /* pop value */
  lua_unlock(L);
}


स्थिर शून्य aux_rawset (lua_State *L, पूर्णांक idx, TValue *किल्ली, पूर्णांक n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, किल्ली, s2v(L->शीर्ष.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->शीर्ष.p - 1));
  L->शीर्ष.p -= n;
  lua_unlock(L);
}


LUA_API शून्य lua_rawset (lua_State *L, पूर्णांक idx) {
  aux_rawset(L, idx, s2v(L->शीर्ष.p - 2), 2);
}


LUA_API शून्य lua_rawsetp (lua_State *L, पूर्णांक idx, स्थिरांक शून्य *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API शून्य lua_rawseti (lua_State *L, पूर्णांक idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->शीर्ष.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->शीर्ष.p - 1));
  L->शीर्ष.p--;
  lua_unlock(L);
}


LUA_API पूर्णांक lua_setmetatable (lua_State *L, पूर्णांक objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  जर (ttisnil(s2v(L->शीर्ष.p - 1)))
    mt = रिक्त;
  अन्यथा {
    api_check(L, ttistable(s2v(L->शीर्ष.p - 1)), "table expected");
    mt = hvalue(s2v(L->शीर्ष.p - 1));
  }
  निवड (ttype(obj)) {
    स्थिती LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      जर (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      थांबा;
    }
    स्थिती LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      जर (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      थांबा;
    }
    पूर्वनिर्धारित: {
      G(L)->mt[ttype(obj)] = mt;
      थांबा;
    }
  }
  L->शीर्ष.p--;
  lua_unlock(L);
  परत 1;
}


LUA_API पूर्णांक lua_setiuservalue (lua_State *L, पूर्णांक idx, पूर्णांक n) {
  TValue *o;
  पूर्णांक res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  जर (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  अन्यथा {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->शीर्ष.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->शीर्ष.p - 1));
    res = 1;
  }
  L->शीर्ष.p--;
  lua_unlock(L);
  परत res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->शीर्ष.p - L->शीर्ष.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API शून्य lua_callk (lua_State *L, पूर्णांक nargs, पूर्णांक nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == रिक्त || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->शीर्ष.p - (nargs+1);
  जर (k != रिक्त && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  अन्यथा  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
संरचना CallS {  /* data to 'f_call' */
  StkId func;
  पूर्णांक nresults;
};


स्थिर शून्य f_call (lua_State *L, शून्य *ud) {
  संरचना CallS *c = cast(संरचना CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API पूर्णांक lua_pcallk (lua_State *L, पूर्णांक nargs, पूर्णांक nresults, पूर्णांक errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  संरचना CallS c;
  TStatus status;
  सूचक_भेद_प्रकार func;
  lua_lock(L);
  api_check(L, k == रिक्त || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  जर (errfunc == 0)
    func = 0;
  अन्यथा {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->शीर्ष.p - (nargs+1);  /* function to be called */
  जर (k == रिक्त || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  अन्यथा {  /* prepare continuation (call is already protected by 'resume') */
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
  परत APIstatus(status);
}


LUA_API पूर्णांक lua_load (lua_State *L, lua_Reader reader, शून्य *data,
                      स्थिरांक अक्षर *chunkname, स्थिरांक अक्षर *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  जर (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  जर (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->शीर्ष.p - 1));  /* get new function */
    जर (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  परत APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API पूर्णांक lua_dump (lua_State *L, lua_Writer writer, शून्य *data, पूर्णांक strip) {
  पूर्णांक status;
  सूचक_भेद_प्रकार otop = savestack(L, L->शीर्ष.p);  /* original top */
  TValue *f = s2v(L->शीर्ष.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->शीर्ष.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  परत status;
}


LUA_API पूर्णांक lua_status (lua_State *L) {
  परत APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API पूर्णांक lua_gc (lua_State *L, पूर्णांक what, ...) {
  बहु_सूची argp;
  पूर्णांक res = 0;
  global_State *g = G(L);
  जर (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    परत -1;  /* all options are invalid when stopped */
  lua_lock(L);
  बहु_शुरू(argp, what);
  निवड (what) {
    स्थिती LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      थांबा;
    }
    स्थिती LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      थांबा;
    }
    स्थिती LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      थांबा;
    }
    स्थिती LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      थांबा;
    }
    स्थिती LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      थांबा;
    }
    स्थिती LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, बहु_तर्क(argp, माप_प्रकार));
      l_mem newdebt;
      पूर्णांक work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      जर (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      अन्यथा जर (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      अन्यथा  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (शून्य)0, work = 1);
      जर (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      थांबा;
    }
    स्थिती LUA_GCISRUNNING: {
      res = gcrunning(g);
      थांबा;
    }
    स्थिती LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      थांबा;
    }
    स्थिती LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      थांबा;
    }
    स्थिती LUA_GCPARAM: {
      पूर्णांक param = बहु_तर्क(argp, पूर्णांक);
      पूर्णांक मूल्य = बहु_तर्क(argp, पूर्णांक);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      जर (मूल्य >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(मूल्य));
      थांबा;
    }
    पूर्वनिर्धारित: res = -1;  /* invalid option */
  }
  बहु_पूर्ण(argp);
  lua_unlock(L);
  परत res;
}



/*
** miscellaneous functions
*/


LUA_API पूर्णांक lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->शीर्ष.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  जर (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  अन्यथा
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  परत 0;  /* to avoid warnings */
}


LUA_API पूर्णांक lua_next (lua_State *L, पूर्णांक idx) {
  Table *t;
  पूर्णांक more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->शीर्ष.p - 1);
  जर (more)
    api_incr_शीर्ष(L);
  अन्यथा  /* no more elements */
    L->शीर्ष.p--;  /* pop key */
  lua_unlock(L);
  परत more;
}


LUA_API शून्य lua_toclose (lua_State *L, पूर्णांक idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API शून्य lua_concat (lua_State *L, पूर्णांक n) {
  lua_lock(L);
  जर (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  अन्यथा {  /* nothing to concatenate */
    setsvalue2s(L, L->शीर्ष.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_शीर्ष(L);
  }
  lua_unlock(L);
}


LUA_API शून्य lua_len (lua_State *L, पूर्णांक idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->शीर्ष.p, t);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, शून्य **ud) {
  lua_Alloc f;
  lua_lock(L);
  जर (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  परत f;
}


LUA_API शून्य lua_setallocf (lua_State *L, lua_Alloc f, शून्य *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


शून्य lua_setwarnf (lua_State *L, lua_WarnFunction f, शून्य *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


शून्य lua_warning (lua_State *L, स्थिरांक अक्षर *संदेश, पूर्णांक tocont) {
  lua_lock(L);
  luaE_warning(L, संदेश, tocont);
  lua_unlock(L);
}



LUA_API शून्य *lua_newuserdatauv (lua_State *L, माप_प्रकार size, पूर्णांक nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < लघु_उच्च, "invalid value");
  u = luaS_newudata(L, size, cast(अचिन्हित लघु, nuvalue));
  setuvalue(L, s2v(L->शीर्ष.p), u);
  api_incr_शीर्ष(L);
  luaC_checkGC(L);
  lua_unlock(L);
  परत getudatamem(u);
}



स्थिर स्थिरांक अक्षर *aux_upvalue (TValue *fi, पूर्णांक n, TValue **val,
                                GCObject **owner) {
  निवड (ttypetag(fi)) {
    स्थिती LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      जर (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        परत रिक्त;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      जर (owner) *owner = obj2gco(f);
      परत "";
    }
    स्थिती LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      जर (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        परत रिक्त;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      जर (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      परत (name == रिक्त) ? "(no name)" : getstr(name);
    }
    पूर्वनिर्धारित: परत रिक्त;  /* not a closure */
  }
}


LUA_API स्थिरांक अक्षर *lua_getupvalue (lua_State *L, पूर्णांक funcindex, पूर्णांक n) {
  स्थिरांक अक्षर *name;
  TValue *val = रिक्त;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, रिक्त);
  जर (name) {
    setobj2s(L, L->शीर्ष.p, val);
    api_incr_शीर्ष(L);
  }
  lua_unlock(L);
  परत name;
}


LUA_API स्थिरांक अक्षर *lua_setupvalue (lua_State *L, पूर्णांक funcindex, पूर्णांक n) {
  स्थिरांक अक्षर *name;
  TValue *val = रिक्त;  /* to avoid warnings */
  GCObject *owner = रिक्त;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  जर (name) {
    L->शीर्ष.p--;
    setobj(L, val, s2v(L->शीर्ष.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  परत name;
}


स्थिर UpVal **getupvalref (lua_State *L, पूर्णांक fidx, पूर्णांक n, LClosure **pf) {
  स्थिर स्थिरांक UpVal *स्थिरांक nullup = रिक्त;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  जर (pf) *pf = f;
  जर (1 <= n && n <= f->p->sizeupvalues)
    परत &f->upvals[n - 1];  /* get its upvalue pointer */
  अन्यथा
    परत (UpVal**)&nullup;
}


LUA_API शून्य *lua_upvalueid (lua_State *L, पूर्णांक fidx, पूर्णांक n) {
  TValue *fi = index2value(L, fidx);
  निवड (ttypetag(fi)) {
    स्थिती LUA_VLCL: {  /* lua closure */
      परत *getupvalref(L, fidx, n, रिक्त);
    }
    स्थिती LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      जर (1 <= n && n <= f->nupvalues)
        परत &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    स्थिती LUA_VLCF:
      परत रिक्त;  /* light C functions have no upvalues */
    पूर्वनिर्धारित: {
      api_check(L, 0, "function expected");
      परत रिक्त;
    }
  }
}


LUA_API शून्य lua_upvaluejoin (lua_State *L, पूर्णांक fidx1, पूर्णांक n1,
                                            पूर्णांक fidx2, पूर्णांक n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, रिक्त);
  api_check(L, *up1 != रिक्त && *up2 != रिक्त, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


