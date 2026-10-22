/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#घोषणा lapi_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <limits.h>
#समावेश <stdarg.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lapi.h"
#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lfunc.h"
#समावेश "lgc.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lstate.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "ltm.h"
#समावेश "lundump.h"
#समावेश "lvm.h"



स्थिर अक्षर lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
*/
#घोषणा isvalid(L, o)	((o) != &G(L)->nilvalue)


/* test for pseudo index */
#घोषणा ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#घोषणा isupvalue(i)		((i) < LUA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
अटल TValue *index2value (lua_State *L, पूर्णांक idx) {
  CallInfo *ci = L->ci;
  अगर (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->शीर्ष.p - (ci->func.p + 1), "unacceptable index");
    अगर (o >= L->शीर्ष.p) वापस &G(L)->nilvalue;
    अन्यथा वापस s2v(o);
  }
  अन्यथा अगर (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->शीर्ष.p - (ci->func.p + 1),
                 "invalid index");
    वापस s2v(L->शीर्ष.p + idx);
  }
  अन्यथा अगर (idx == LUA_REGISTRYINDEX)
    वापस &G(L)->l_registry;
  अन्यथा {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    अगर (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      वापस (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    अन्यथा {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      वापस &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
अटल StkId index2stack (lua_State *L, पूर्णांक idx) {
  CallInfo *ci = L->ci;
  अगर (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->शीर्ष.p, "invalid index");
    वापस o;
  }
  अन्यथा {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->शीर्ष.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    वापस L->शीर्ष.p + idx;
  }
}


LUA_API पूर्णांक lua_checkstack (lua_State *L, पूर्णांक n) {
  पूर्णांक res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  अगर (L->ढेर_last.p - L->शीर्ष.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  अन्यथा  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  अगर (res && ci->शीर्ष.p < L->शीर्ष.p + n)
    ci->शीर्ष.p = L->शीर्ष.p + n;  /* adjust frame top */
  lua_unlock(L);
  वापस res;
}


LUA_API व्योम lua_xmove (lua_State *from, lua_State *to, पूर्णांक n) {
  पूर्णांक i;
  अगर (from == to) वापस;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->शीर्ष.p - to->शीर्ष.p >= n, "stack overflow");
  from->शीर्ष.p -= n;
  क्रम (i = 0; i < n; i++) {
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
  वापस old;
}


LUA_API lua_संख्या lua_version (lua_State *L) {
  UNUSED(L);
  वापस LUA_VERSION_अंक;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API पूर्णांक lua_absindex (lua_State *L, पूर्णांक idx) {
  वापस (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->शीर्ष.p - L->ci->func.p) + idx;
}


LUA_API पूर्णांक lua_gettop (lua_State *L) {
  वापस cast_int(L->शीर्ष.p - (L->ci->func.p + 1));
}


LUA_API व्योम lua_settop (lua_State *L, पूर्णांक idx) {
  CallInfo *ci;
  StkId func, newtop;
  सूचक_भेद_प्रकार diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  अगर (idx >= 0) {
    api_check(L, idx <= ci->शीर्ष.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->शीर्ष.p;
    क्रम (; diff > 0; diff--)
      setnilvalue2s(L->शीर्ष.p++);  /* clear new slots */
  }
  अन्यथा {
    api_check(L, -(idx+1) <= (L->शीर्ष.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->शीर्ष.p + diff;
  अगर (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->शीर्ष.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API व्योम lua_closeslot (lua_State *L, पूर्णांक idx) {
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
अटल व्योम reverse (lua_State *L, StkId from, StkId to) {
  क्रम (; from < to; from++, to--) {
    TValue अस्थायी;
    setobj(L, &अस्थायी, s2v(from));
    setobjs2s(L, from, to);
    setobj2s(L, to, &अस्थायी);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API व्योम lua_rotate (lua_State *L, पूर्णांक idx, पूर्णांक n) {
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


LUA_API व्योम lua_copy (lua_State *L, पूर्णांक fromidx, पूर्णांक toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  अगर (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API व्योम lua_pushvalue (lua_State *L, पूर्णांक idx) {
  lua_lock(L);
  setobj2s(L, L->शीर्ष.p, index2value(L, idx));
  api_incr_शीर्ष(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API पूर्णांक lua_type (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API स्थिर अक्षर *lua_typename (lua_State *L, पूर्णांक t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  वापस ttypename(t);
}


LUA_API पूर्णांक lua_iscfunction (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API पूर्णांक lua_isinteger (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस ttisinteger(o);
}


LUA_API पूर्णांक lua_isnumber (lua_State *L, पूर्णांक idx) {
  lua_संख्या n;
  स्थिर TValue *o = index2value(L, idx);
  वापस tonumber(o, &n);
}


LUA_API पूर्णांक lua_isstring (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस (ttisstring(o) || cvt2str(o));
}


LUA_API पूर्णांक lua_isuserdata (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API पूर्णांक lua_rawequal (lua_State *L, पूर्णांक index1, पूर्णांक index2) {
  स्थिर TValue *o1 = index2value(L, index1);
  स्थिर TValue *o2 = index2value(L, index2);
  वापस (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API व्योम lua_arith (lua_State *L, पूर्णांक op) {
  lua_lock(L);
  अगर (op != LUA_OPUNM && op != LUA_OPBNOT)
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
  स्थिर TValue *o1;
  स्थिर TValue *o2;
  पूर्णांक i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  अगर (isvalid(L, o1) && isvalid(L, o2)) {
    चयन (op) {
      हाल LUA_OPEQ: i = luaV_equalobj(L, o1, o2); अवरोध;
      हाल LUA_OPLT: i = luaV_lessthan(L, o1, o2); अवरोध;
      हाल LUA_OPLE: i = luaV_lessequal(L, o1, o2); अवरोध;
      शेष: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  वापस i;
}


LUA_API अचिन्हित lua_numbertocstring (lua_State *L, पूर्णांक idx, अक्षर *buff) {
  स्थिर TValue *o = index2value(L, idx);
  अगर (ttisnumber(o)) {
    अचिन्हित len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    वापस len;
  }
  अन्यथा
    वापस 0;
}


LUA_API माप_प्रकार lua_stringtonumber (lua_State *L, स्थिर अक्षर *s) {
  माप_प्रकार sz = luaO_str2num(s, s2v(L->शीर्ष.p));
  अगर (sz != 0)
    api_incr_शीर्ष(L);
  वापस sz;
}


LUA_API lua_संख्या lua_tonumberx (lua_State *L, पूर्णांक idx, पूर्णांक *pisnum) {
  lua_संख्या n = 0;
  स्थिर TValue *o = index2value(L, idx);
  पूर्णांक isnum = tonumber(o, &n);
  अगर (pisnum)
    *pisnum = isnum;
  वापस n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, पूर्णांक idx, पूर्णांक *pisnum) {
  lua_Integer res = 0;
  स्थिर TValue *o = index2value(L, idx);
  पूर्णांक isnum = tointeger(o, &res);
  अगर (pisnum)
    *pisnum = isnum;
  वापस res;
}


LUA_API पूर्णांक lua_toboolean (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस !l_isfalse(o);
}


LUA_API स्थिर अक्षर *lua_tolstring (lua_State *L, पूर्णांक idx, माप_प्रकार *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  अगर (!ttisstring(o)) {
    अगर (!cvt2str(o)) {  /* not convertible? */
      अगर (len != शून्य) *len = 0;
      lua_unlock(L);
      वापस शून्य;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  अगर (len != शून्य)
    वापस getlstr(tsvalue(o), *len);
  अन्यथा
    वापस getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  चयन (ttypetag(o)) {
    हाल LUA_VSHRSTR: वापस cast(lua_Unsigned, tsvalue(o)->shrlen);
    हाल LUA_VLNGSTR: वापस cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    हाल LUA_VUSERDATA: वापस cast(lua_Unsigned, uvalue(o)->len);
    हाल LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      वापस res;
    }
    शेष: वापस 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  अगर (ttislcf(o)) वापस fvalue(o);
  अन्यथा अगर (ttisCclosure(o))
    वापस clCvalue(o)->f;
  अन्यथा वापस शून्य;  /* not a C function */
}


l_sinline व्योम *touserdata (स्थिर TValue *o) {
  चयन (ttype(o)) {
    हाल LUA_TUSERDATA: वापस getudatamem(uvalue(o));
    हाल LUA_TLIGHTUSERDATA: वापस pvalue(o);
    शेष: वापस शून्य;
  }
}


LUA_API व्योम *lua_touserdata (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  वापस (!ttisthread(o)) ? शून्य : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API स्थिर व्योम *lua_topointer (lua_State *L, पूर्णांक idx) {
  स्थिर TValue *o = index2value(L, idx);
  चयन (ttypetag(o)) {
    हाल LUA_VLCF: वापस cast_voidp(cast_sizet(fvalue(o)));
    हाल LUA_VUSERDATA: हाल LUA_VLIGHTUSERDATA:
      वापस touserdata(o);
    शेष: {
      अगर (iscollectable(o))
        वापस gcvalue(o);
      अन्यथा
        वापस शून्य;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API व्योम lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->शीर्ष.p);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API व्योम lua_pushnumber (lua_State *L, lua_संख्या n) {
  lua_lock(L);
  setfltvalue(s2v(L->शीर्ष.p), n);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API व्योम lua_pushinteger (lua_State *L, lua_Integer n) {
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
LUA_API स्थिर अक्षर *lua_pushlstring (lua_State *L, स्थिर अक्षर *s, माप_प्रकार len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->शीर्ष.p, ts);
  api_incr_शीर्ष(L);
  luaC_checkGC(L);
  lua_unlock(L);
  वापस getstr(ts);
}


LUA_API स्थिर अक्षर *lua_pushexternalstring (lua_State *L,
	        स्थिर अक्षर *s, माप_प्रकार len, lua_Alloc falloc, व्योम *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_आकार, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->शीर्ष.p, ts);
  api_incr_शीर्ष(L);
  luaC_checkGC(L);
  lua_unlock(L);
  वापस getstr(ts);
}


LUA_API स्थिर अक्षर *lua_pushstring (lua_State *L, स्थिर अक्षर *s) {
  lua_lock(L);
  अगर (s == शून्य)
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
  वापस s;
}


LUA_API स्थिर अक्षर *lua_pushvfstring (lua_State *L, स्थिर अक्षर *fmt,
                                      बहु_सूची argp) {
  स्थिर अक्षर *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  वापस ret;
}


LUA_API स्थिर अक्षर *lua_pushfstring (lua_State *L, स्थिर अक्षर *fmt, ...) {
  स्थिर अक्षर *ret;
  बहु_सूची argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  वापस ret;
}


LUA_API व्योम lua_pushcclosure (lua_State *L, lua_CFunction fn, पूर्णांक n) {
  lua_lock(L);
  अगर (n == 0) {
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
    क्रम (i = 0; i < n; i++) {
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


LUA_API व्योम lua_pushboolean (lua_State *L, पूर्णांक b) {
  lua_lock(L);
  अगर (b)
    setbtvalue(s2v(L->शीर्ष.p));
  अन्यथा
    setbfvalue(s2v(L->शीर्ष.p));
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API व्योम lua_pushlightuserdata (lua_State *L, व्योम *p) {
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
  वापस (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


अटल पूर्णांक auxgetstr (lua_State *L, स्थिर TValue *t, स्थिर अक्षर *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->शीर्ष.p), luaH_getstr, tag);
  अगर (!tagisempty(tag))
    api_incr_शीर्ष(L);
  अन्यथा {
    setsvalue2s(L, L->शीर्ष.p, str);
    api_incr_शीर्ष(L);
    tag = luaV_finishget(L, t, s2v(L->शीर्ष.p - 1), L->शीर्ष.p - 1, tag);
  }
  lua_unlock(L);
  वापस novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
अटल व्योम getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (व्योम)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API पूर्णांक lua_getglobal (lua_State *L, स्थिर अक्षर *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  वापस auxgetstr(L, &gt, name);
}


LUA_API पूर्णांक lua_gettable (lua_State *L, पूर्णांक idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->शीर्ष.p - 1), s2v(L->शीर्ष.p - 1), luaH_get, tag);
  अगर (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->शीर्ष.p - 1), L->शीर्ष.p - 1, tag);
  lua_unlock(L);
  वापस novariant(tag);
}


LUA_API पूर्णांक lua_getfield (lua_State *L, पूर्णांक idx, स्थिर अक्षर *k) {
  lua_lock(L);
  वापस auxgetstr(L, index2value(L, idx), k);
}


LUA_API पूर्णांक lua_geti (lua_State *L, पूर्णांक idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->शीर्ष.p), tag);
  अगर (tagisempty(tag)) {
    TValue कुंजी;
    setivalue(&कुंजी, n);
    tag = luaV_finishget(L, t, &कुंजी, L->शीर्ष.p, tag);
  }
  api_incr_शीर्ष(L);
  lua_unlock(L);
  वापस novariant(tag);
}


अटल पूर्णांक finishrawget (lua_State *L, lu_byte tag) {
  अगर (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->शीर्ष.p);
  api_incr_शीर्ष(L);
  lua_unlock(L);
  वापस novariant(tag);
}


l_sinline Table *gettable (lua_State *L, पूर्णांक idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  वापस hvalue(t);
}


LUA_API पूर्णांक lua_rawget (lua_State *L, पूर्णांक idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->शीर्ष.p - 1), s2v(L->शीर्ष.p - 1));
  L->शीर्ष.p--;  /* pop key */
  वापस finishrawget(L, tag);
}


LUA_API पूर्णांक lua_rawgeti (lua_State *L, पूर्णांक idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->शीर्ष.p), tag);
  वापस finishrawget(L, tag);
}


LUA_API पूर्णांक lua_rawgetp (lua_State *L, पूर्णांक idx, स्थिर व्योम *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  वापस finishrawget(L, luaH_get(t, &k, s2v(L->शीर्ष.p)));
}


LUA_API व्योम lua_createtable (lua_State *L, पूर्णांक narray, पूर्णांक nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->शीर्ष.p, t);
  api_incr_शीर्ष(L);
  अगर (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API पूर्णांक lua_getmetatable (lua_State *L, पूर्णांक objindex) {
  स्थिर TValue *obj;
  Table *mt;
  पूर्णांक res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  चयन (ttype(obj)) {
    हाल LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      अवरोध;
    हाल LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      अवरोध;
    शेष:
      mt = G(L)->mt[ttype(obj)];
      अवरोध;
  }
  अगर (mt != शून्य) {
    sethvalue2s(L, L->शीर्ष.p, mt);
    api_incr_शीर्ष(L);
    res = 1;
  }
  lua_unlock(L);
  वापस res;
}


LUA_API पूर्णांक lua_getiuservalue (lua_State *L, पूर्णांक idx, पूर्णांक n) {
  TValue *o;
  पूर्णांक t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  अगर (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->शीर्ष.p);
    t = LUA_TNONE;
  }
  अन्यथा {
    setobj2s(L, L->शीर्ष.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->शीर्ष.p));
  }
  api_incr_शीर्ष(L);
  lua_unlock(L);
  वापस t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
अटल व्योम auxsetstr (lua_State *L, स्थिर TValue *t, स्थिर अक्षर *k) {
  पूर्णांक hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->शीर्ष.p - 1), hres, luaH_psetstr);
  अगर (hres == HOK) {
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


LUA_API व्योम lua_setglobal (lua_State *L, स्थिर अक्षर *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API व्योम lua_settable (lua_State *L, पूर्णांक idx) {
  TValue *t;
  पूर्णांक hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->शीर्ष.p - 2), s2v(L->शीर्ष.p - 1), hres, luaH_pset);
  अगर (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->शीर्ष.p - 1));
  अन्यथा
    luaV_finishset(L, t, s2v(L->शीर्ष.p - 2), s2v(L->शीर्ष.p - 1), hres);
  L->शीर्ष.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API व्योम lua_setfield (lua_State *L, पूर्णांक idx, स्थिर अक्षर *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API व्योम lua_seti (lua_State *L, पूर्णांक idx, lua_Integer n) {
  TValue *t;
  पूर्णांक hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->शीर्ष.p - 1), hres);
  अगर (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->शीर्ष.p - 1));
  अन्यथा {
    TValue अस्थायी;
    setivalue(&अस्थायी, n);
    luaV_finishset(L, t, &अस्थायी, s2v(L->शीर्ष.p - 1), hres);
  }
  L->शीर्ष.p--;  /* pop value */
  lua_unlock(L);
}


अटल व्योम aux_rawset (lua_State *L, पूर्णांक idx, TValue *कुंजी, पूर्णांक n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, कुंजी, s2v(L->शीर्ष.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->शीर्ष.p - 1));
  L->शीर्ष.p -= n;
  lua_unlock(L);
}


LUA_API व्योम lua_rawset (lua_State *L, पूर्णांक idx) {
  aux_rawset(L, idx, s2v(L->शीर्ष.p - 2), 2);
}


LUA_API व्योम lua_rawsetp (lua_State *L, पूर्णांक idx, स्थिर व्योम *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API व्योम lua_rawseti (lua_State *L, पूर्णांक idx, lua_Integer n) {
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
  अगर (ttisnil(s2v(L->शीर्ष.p - 1)))
    mt = शून्य;
  अन्यथा {
    api_check(L, ttistable(s2v(L->शीर्ष.p - 1)), "table expected");
    mt = hvalue(s2v(L->शीर्ष.p - 1));
  }
  चयन (ttype(obj)) {
    हाल LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      अगर (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      अवरोध;
    }
    हाल LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      अगर (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      अवरोध;
    }
    शेष: {
      G(L)->mt[ttype(obj)] = mt;
      अवरोध;
    }
  }
  L->शीर्ष.p--;
  lua_unlock(L);
  वापस 1;
}


LUA_API पूर्णांक lua_setiuservalue (lua_State *L, पूर्णांक idx, पूर्णांक n) {
  TValue *o;
  पूर्णांक res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  अगर (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  अन्यथा {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->शीर्ष.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->शीर्ष.p - 1));
    res = 1;
  }
  L->शीर्ष.p--;
  lua_unlock(L);
  वापस res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#घोषणा checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->शीर्ष.p - L->शीर्ष.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API व्योम lua_callk (lua_State *L, पूर्णांक nargs, पूर्णांक nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == शून्य || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->शीर्ष.p - (nargs+1);
  अगर (k != शून्य && yieldable(L)) {  /* need to prepare continuation? */
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
काष्ठा CallS {  /* data to 'f_call' */
  StkId func;
  पूर्णांक nresults;
};


अटल व्योम f_call (lua_State *L, व्योम *ud) {
  काष्ठा CallS *c = cast(काष्ठा CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API पूर्णांक lua_pcallk (lua_State *L, पूर्णांक nargs, पूर्णांक nresults, पूर्णांक errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  काष्ठा CallS c;
  TStatus status;
  सूचक_भेद_प्रकार func;
  lua_lock(L);
  api_check(L, k == शून्य || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  अगर (errfunc == 0)
    func = 0;
  अन्यथा {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->शीर्ष.p - (nargs+1);  /* function to be called */
  अगर (k == शून्य || !yieldable(L)) {  /* no continuation or no yieldable? */
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
  वापस APIstatus(status);
}


LUA_API पूर्णांक lua_load (lua_State *L, lua_Reader reader, व्योम *तथ्य,
                      स्थिर अक्षर *chunkname, स्थिर अक्षर *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  अगर (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, तथ्य);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  अगर (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->शीर्ष.p - 1));  /* get new function */
    अगर (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  वापस APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API पूर्णांक lua_dump (lua_State *L, lua_Writer writer, व्योम *तथ्य, पूर्णांक strip) {
  पूर्णांक status;
  सूचक_भेद_प्रकार otop = savestack(L, L->शीर्ष.p);  /* original top */
  TValue *f = s2v(L->शीर्ष.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, तथ्य, strip);
  L->शीर्ष.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  वापस status;
}


LUA_API पूर्णांक lua_status (lua_State *L) {
  वापस APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API पूर्णांक lua_gc (lua_State *L, पूर्णांक what, ...) {
  बहु_सूची argp;
  पूर्णांक res = 0;
  global_State *g = G(L);
  अगर (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    वापस -1;  /* all options are invalid when stopped */
  lua_lock(L);
  बहु_शुरू(argp, what);
  चयन (what) {
    हाल LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      अवरोध;
    }
    हाल LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      अवरोध;
    }
    हाल LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      अवरोध;
    }
    हाल LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      अवरोध;
    }
    हाल LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      अवरोध;
    }
    हाल LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, बहु_तर्क(argp, माप_प्रकार));
      l_mem newdebt;
      पूर्णांक work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      अगर (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      अन्यथा अगर (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      अन्यथा  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (व्योम)0, work = 1);
      अगर (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      अवरोध;
    }
    हाल LUA_GCISRUNNING: {
      res = gcrunning(g);
      अवरोध;
    }
    हाल LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      अवरोध;
    }
    हाल LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      अवरोध;
    }
    हाल LUA_GCPARAM: {
      पूर्णांक param = बहु_तर्क(argp, पूर्णांक);
      पूर्णांक मूल्य = बहु_तर्क(argp, पूर्णांक);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      अगर (मूल्य >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(मूल्य));
      अवरोध;
    }
    शेष: res = -1;  /* invalid option */
  }
  बहु_पूर्ण(argp);
  lua_unlock(L);
  वापस res;
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
  अगर (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  अन्यथा
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  वापस 0;  /* to avoid warnings */
}


LUA_API पूर्णांक lua_अग्र (lua_State *L, पूर्णांक idx) {
  Table *t;
  पूर्णांक more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = lua_H_अग्र(L, t, L->शीर्ष.p - 1);
  अगर (more)
    api_incr_शीर्ष(L);
  अन्यथा  /* no more elements */
    L->शीर्ष.p--;  /* pop key */
  lua_unlock(L);
  वापस more;
}


LUA_API व्योम lua_toclose (lua_State *L, पूर्णांक idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API व्योम lua_concat (lua_State *L, पूर्णांक n) {
  lua_lock(L);
  अगर (n > 0) {
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


LUA_API व्योम lua_len (lua_State *L, पूर्णांक idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->शीर्ष.p, t);
  api_incr_शीर्ष(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, व्योम **ud) {
  lua_Alloc f;
  lua_lock(L);
  अगर (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  वापस f;
}


LUA_API व्योम lua_setallocf (lua_State *L, lua_Alloc f, व्योम *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


व्योम lua_setwarnf (lua_State *L, lua_WarnFunction f, व्योम *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


व्योम lua_warning (lua_State *L, स्थिर अक्षर *संदेश, पूर्णांक tocont) {
  lua_lock(L);
  luaE_warning(L, संदेश, tocont);
  lua_unlock(L);
}



LUA_API व्योम *lua_newuserdatauv (lua_State *L, माप_प्रकार आकार, पूर्णांक nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < लघु_उच्च, "invalid value");
  u = luaS_newudata(L, आकार, cast(अचिन्हित लघु, nuvalue));
  setuvalue(L, s2v(L->शीर्ष.p), u);
  api_incr_शीर्ष(L);
  luaC_checkGC(L);
  lua_unlock(L);
  वापस getudatamem(u);
}



अटल स्थिर अक्षर *aux_upvalue (TValue *fi, पूर्णांक n, TValue **val,
                                GCObject **owner) {
  चयन (ttypetag(fi)) {
    हाल LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      अगर (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        वापस शून्य;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      अगर (owner) *owner = obj2gco(f);
      वापस "";
    }
    हाल LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      अगर (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        वापस शून्य;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      अगर (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      वापस (name == शून्य) ? "(no name)" : getstr(name);
    }
    शेष: वापस शून्य;  /* not a closure */
  }
}


LUA_API स्थिर अक्षर *lua_getupvalue (lua_State *L, पूर्णांक funcindex, पूर्णांक n) {
  स्थिर अक्षर *name;
  TValue *val = शून्य;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, शून्य);
  अगर (name) {
    setobj2s(L, L->शीर्ष.p, val);
    api_incr_शीर्ष(L);
  }
  lua_unlock(L);
  वापस name;
}


LUA_API स्थिर अक्षर *lua_setupvalue (lua_State *L, पूर्णांक funcindex, पूर्णांक n) {
  स्थिर अक्षर *name;
  TValue *val = शून्य;  /* to avoid warnings */
  GCObject *owner = शून्य;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  अगर (name) {
    L->शीर्ष.p--;
    setobj(L, val, s2v(L->शीर्ष.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  वापस name;
}


अटल UpVal **getupvalref (lua_State *L, पूर्णांक fidx, पूर्णांक n, LClosure **pf) {
  अटल स्थिर UpVal *स्थिर nullup = शून्य;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  अगर (pf) *pf = f;
  अगर (1 <= n && n <= f->p->sizeupvalues)
    वापस &f->upvals[n - 1];  /* get its upvalue pointer */
  अन्यथा
    वापस (UpVal**)&nullup;
}


LUA_API व्योम *lua_upvalueid (lua_State *L, पूर्णांक fidx, पूर्णांक n) {
  TValue *fi = index2value(L, fidx);
  चयन (ttypetag(fi)) {
    हाल LUA_VLCL: {  /* lua closure */
      वापस *getupvalref(L, fidx, n, शून्य);
    }
    हाल LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      अगर (1 <= n && n <= f->nupvalues)
        वापस &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    हाल LUA_VLCF:
      वापस शून्य;  /* light C functions have no upvalues */
    शेष: {
      api_check(L, 0, "function expected");
      वापस शून्य;
    }
  }
}


LUA_API व्योम lua_upvaluejoin (lua_State *L, पूर्णांक fidx1, पूर्णांक n1,
                                            पूर्णांक fidx2, पूर्णांक n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, शून्य);
  api_check(L, *up1 != शून्य && *up2 != शून्य, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


