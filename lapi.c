/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#నిర్వచించు lapi_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "ltm.h"
#అన్తర్భావ "lundump.h"
#అన్తర్భావ "lvm.h"



స్థిరరాశి అక్షరం lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
*/
#నిర్వచించు isvalid(L, o)	((o) != &G(L)->nilvalue)


/* test for pseudo index */
#నిర్వచించు ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#నిర్వచించు isupvalue(i)		((i) < LUA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
స్థిర TValue *index2value (lua_State *L, పూర్ణాంకం idx) {
  CallInfo *ci = L->ci;
  ఒకవేళ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->శిఖరం.p - (ci->func.p + 1), "unacceptable index");
    ఒకవేళ (o >= L->శిఖరం.p) తిరిగి &G(L)->nilvalue;
    లేకపోతే తిరిగి s2v(o);
  }
  లేకపోతే ఒకవేళ (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->శిఖరం.p - (ci->func.p + 1),
                 "invalid index");
    తిరిగి s2v(L->శిఖరం.p + idx);
  }
  లేకపోతే ఒకవేళ (idx == LUA_REGISTRYINDEX)
    తిరిగి &G(L)->l_registry;
  లేకపోతే {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    ఒకవేళ (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      తిరిగి (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    లేకపోతే {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      తిరిగి &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
స్థిర StkId index2stack (lua_State *L, పూర్ణాంకం idx) {
  CallInfo *ci = L->ci;
  ఒకవేళ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->శిఖరం.p, "invalid index");
    తిరిగి o;
  }
  లేకపోతే {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->శిఖరం.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    తిరిగి L->శిఖరం.p + idx;
  }
}


LUA_API పూర్ణాంకం lua_checkstack (lua_State *L, పూర్ణాంకం n) {
  పూర్ణాంకం res;
  CallInfo *ci;
  lua_తాళం(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  ఒకవేళ (L->దొంతర_చివరి.p - L->శిఖరం.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  లేకపోతే  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  ఒకవేళ (res && ci->శిఖరం.p < L->శిఖరం.p + n)
    ci->శిఖరం.p = L->శిఖరం.p + n;  /* adjust frame top */
  lua_unlock(L);
  తిరిగి res;
}


LUA_API శూన్య lua_xmove (lua_State *from, lua_State *గమ్యం, పూర్ణాంకం n) {
  పూర్ణాంకం i;
  ఒకవేళ (from == గమ్యం) తిరిగి;
  lua_తాళం(గమ్యం);
  api_checkpop(from, n);
  api_check(from, G(from) == G(గమ్యం), "moving among independent states");
  api_check(from, గమ్యం->ci->శిఖరం.p - గమ్యం->శిఖరం.p >= n, "stack overflow");
  from->శిఖరం.p -= n;
  కొరకు (i = 0; i < n; i++) {
    setobjs2s(గమ్యం, గమ్యం->శిఖరం.p, from->శిఖరం.p + i);
    గమ్యం->శిఖరం.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(గమ్యం);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_తాళం(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  తిరిగి old;
}


LUA_API lua_సంఖ్య lua_సంస్కరణ (lua_State *L) {
  వాడని(L);
  తిరిగి LUA_సంస్కరణ_అంకె;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API పూర్ణాంకం lua_absindex (lua_State *L, పూర్ణాంకం idx) {
  తిరిగి (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->శిఖరం.p - L->ci->func.p) + idx;
}


LUA_API పూర్ణాంకం lua_gettop (lua_State *L) {
  తిరిగి cast_int(L->శిఖరం.p - (L->ci->func.p + 1));
}


LUA_API శూన్య lua_settop (lua_State *L, పూర్ణాంకం idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_తాళం(L);
  ci = L->ci;
  func = ci->func.p;
  ఒకవేళ (idx >= 0) {
    api_check(L, idx <= ci->శిఖరం.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->శిఖరం.p;
    కొరకు (; diff > 0; diff--)
      setnilvalue2s(L->శిఖరం.p++);  /* clear new slots */
  }
  లేకపోతే {
    api_check(L, -(idx+1) <= (L->శిఖరం.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->శిఖరం.p + diff;
  ఒకవేళ (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->శిఖరం.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API శూన్య lua_closeslot (lua_State *L, పూర్ణాంకం idx) {
  StkId level;
  lua_తాళం(L);
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
స్థిర శూన్య తిరగేయి (lua_State *L, StkId from, StkId గమ్యం) {
  కొరకు (; from < గమ్యం; from++, గమ్యం--) {
    TValue తాత్కాలిక;
    setobj(L, &తాత్కాలిక, s2v(from));
    setobjs2s(L, from, గమ్యం);
    setobj2s(L, గమ్యం, &తాత్కాలిక);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API శూన్య lua_rotate (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం n) {
  StkId p, t, m;
  lua_తాళం(L);
  t = L->శిఖరం.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  తిరగేయి(L, p, m);  /* reverse the prefix with length 'n' */
  తిరగేయి(L, m + 1, t);  /* reverse the suffix */
  తిరగేయి(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API శూన్య lua_copy (lua_State *L, పూర్ణాంకం fromidx, పూర్ణాంకం toidx) {
  TValue *fr, *గమ్యం;
  lua_తాళం(L);
  fr = index2value(L, fromidx);
  గమ్యం = index2value(L, toidx);
  api_check(L, isvalid(L, గమ్యం), "invalid index");
  setobj(L, గమ్యం, fr);
  ఒకవేళ (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API శూన్య lua_pushvalue (lua_State *L, పూర్ణాంకం idx) {
  lua_తాళం(L);
  setobj2s(L, L->శిఖరం.p, index2value(L, idx));
  api_incr_శిఖరం(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API పూర్ణాంకం lua_type (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API స్థిరరాశి అక్షరం *lua_typename (lua_State *L, పూర్ణాంకం t) {
  వాడని(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  తిరిగి ttypename(t);
}


LUA_API పూర్ణాంకం lua_iscfunction (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API పూర్ణాంకం lua_isinteger (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి ttisinteger(o);
}


LUA_API పూర్ణాంకం lua_isnumber (lua_State *L, పూర్ణాంకం idx) {
  lua_సంఖ్య n;
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి tonumber(o, &n);
}


LUA_API పూర్ణాంకం lua_isstring (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి (ttisstring(o) || cvt2str(o));
}


LUA_API పూర్ణాంకం lua_isuserdata (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API పూర్ణాంకం lua_rawequal (lua_State *L, పూర్ణాంకం index1, పూర్ణాంకం index2) {
  స్థిరరాశి TValue *o1 = index2value(L, index1);
  స్థిరరాశి TValue *o2 = index2value(L, index2);
  తిరిగి (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API శూన్య lua_arith (lua_State *L, పూర్ణాంకం op) {
  lua_తాళం(L);
  ఒకవేళ (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  లేకపోతే {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->శిఖరం.p, L->శిఖరం.p - 1);
    api_incr_శిఖరం(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->శిఖరం.p - 2), s2v(L->శిఖరం.p - 1), L->శిఖరం.p - 2);
  L->శిఖరం.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API పూర్ణాంకం lua_compare (lua_State *L, పూర్ణాంకం index1, పూర్ణాంకం index2, పూర్ణాంకం op) {
  స్థిరరాశి TValue *o1;
  స్థిరరాశి TValue *o2;
  పూర్ణాంకం i = 0;
  lua_తాళం(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  ఒకవేళ (isvalid(L, o1) && isvalid(L, o2)) {
    ఎంపిక (op) {
      స్థితి LUA_OPEQ: i = luaV_equalobj(L, o1, o2); ఆపు;
      స్థితి LUA_OPLT: i = luaV_lessthan(L, o1, o2); ఆపు;
      స్థితి LUA_OPLE: i = luaV_lessequal(L, o1, o2); ఆపు;
      అప్రమేయ: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  తిరిగి i;
}


LUA_API గుర్తులేని lua_numbertocstring (lua_State *L, పూర్ణాంకం idx, అక్షరం *buff) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  ఒకవేళ (ttisnumber(o)) {
    గుర్తులేని len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    తిరిగి len;
  }
  లేకపోతే
    తిరిగి 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, స్థిరరాశి అక్షరం *s) {
  size_t sz = luaO_str2num(s, s2v(L->శిఖరం.p));
  ఒకవేళ (sz != 0)
    api_incr_శిఖరం(L);
  తిరిగి sz;
}


LUA_API lua_సంఖ్య lua_tonumberx (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం *pisnum) {
  lua_సంఖ్య n = 0;
  స్థిరరాశి TValue *o = index2value(L, idx);
  పూర్ణాంకం isnum = tonumber(o, &n);
  ఒకవేళ (pisnum)
    *pisnum = isnum;
  తిరిగి n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం *pisnum) {
  lua_Integer res = 0;
  స్థిరరాశి TValue *o = index2value(L, idx);
  పూర్ణాంకం isnum = tointeger(o, &res);
  ఒకవేళ (pisnum)
    *pisnum = isnum;
  తిరిగి res;
}


LUA_API పూర్ణాంకం lua_toboolean (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి !l_isfalse(o);
}


LUA_API స్థిరరాశి అక్షరం *lua_tolstring (lua_State *L, పూర్ణాంకం idx, size_t *len) {
  TValue *o;
  lua_తాళం(L);
  o = index2value(L, idx);
  ఒకవేళ (!ttisstring(o)) {
    ఒకవేళ (!cvt2str(o)) {  /* not convertible? */
      ఒకవేళ (len != ఖాళీ) *len = 0;
      lua_unlock(L);
      తిరిగి ఖాళీ;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  ఒకవేళ (len != ఖాళీ)
    తిరిగి getlstr(tsvalue(o), *len);
  లేకపోతే
    తిరిగి getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  ఎంపిక (ttypetag(o)) {
    స్థితి LUA_VSHRSTR: తిరిగి cast(lua_Unsigned, tsvalue(o)->shrlen);
    స్థితి LUA_VLNGSTR: తిరిగి cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    స్థితి LUA_VUSERDATA: తిరిగి cast(lua_Unsigned, uvalue(o)->len);
    స్థితి LUA_VTABLE: {
      lua_Unsigned res;
      lua_తాళం(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      తిరిగి res;
    }
    అప్రమేయ: తిరిగి 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  ఒకవేళ (ttislcf(o)) తిరిగి fvalue(o);
  లేకపోతే ఒకవేళ (ttisCclosure(o))
    తిరిగి clCvalue(o)->f;
  లేకపోతే తిరిగి ఖాళీ;  /* not a C function */
}


l_sinline శూన్య *touserdata (స్థిరరాశి TValue *o) {
  ఎంపిక (ttype(o)) {
    స్థితి LUA_TUSERDATA: తిరిగి getudatamem(uvalue(o));
    స్థితి LUA_TLIGHTUSERDATA: తిరిగి pvalue(o);
    అప్రమేయ: తిరిగి ఖాళీ;
  }
}


LUA_API శూన్య *lua_touserdata (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  తిరిగి (!ttisthread(o)) ? ఖాళీ : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API స్థిరరాశి శూన్య *lua_topointer (lua_State *L, పూర్ణాంకం idx) {
  స్థిరరాశి TValue *o = index2value(L, idx);
  ఎంపిక (ttypetag(o)) {
    స్థితి LUA_VLCF: తిరిగి cast_voidp(cast_sizet(fvalue(o)));
    స్థితి LUA_VUSERDATA: స్థితి LUA_VLIGHTUSERDATA:
      తిరిగి touserdata(o);
    అప్రమేయ: {
      ఒకవేళ (iscollectable(o))
        తిరిగి gcvalue(o);
      లేకపోతే
        తిరిగి ఖాళీ;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API శూన్య lua_pushnil (lua_State *L) {
  lua_తాళం(L);
  setnilvalue2s(L->శిఖరం.p);
  api_incr_శిఖరం(L);
  lua_unlock(L);
}


LUA_API శూన్య lua_pushnumber (lua_State *L, lua_సంఖ్య n) {
  lua_తాళం(L);
  setfltvalue(s2v(L->శిఖరం.p), n);
  api_incr_శిఖరం(L);
  lua_unlock(L);
}


LUA_API శూన్య lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_తాళం(L);
  setivalue(s2v(L->శిఖరం.p), n);
  api_incr_శిఖరం(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API స్థిరరాశి అక్షరం *lua_pushlstring (lua_State *L, స్థిరరాశి అక్షరం *s, size_t len) {
  TString *ts;
  lua_తాళం(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->శిఖరం.p, ts);
  api_incr_శిఖరం(L);
  luaC_checkGC(L);
  lua_unlock(L);
  తిరిగి getstr(ts);
}


LUA_API స్థిరరాశి అక్షరం *lua_pushexternalstring (lua_State *L,
	        స్థిరరాశి అక్షరం *s, size_t len, lua_Alloc falloc, శూన్య *ud) {
  TString *ts;
  lua_తాళం(L);
  api_check(L, len <= MAX_కొలత, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->శిఖరం.p, ts);
  api_incr_శిఖరం(L);
  luaC_checkGC(L);
  lua_unlock(L);
  తిరిగి getstr(ts);
}


LUA_API స్థిరరాశి అక్షరం *lua_pushstring (lua_State *L, స్థిరరాశి అక్షరం *s) {
  lua_తాళం(L);
  ఒకవేళ (s == ఖాళీ)
    setnilvalue2s(L->శిఖరం.p);
  లేకపోతే {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->శిఖరం.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_శిఖరం(L);
  luaC_checkGC(L);
  lua_unlock(L);
  తిరిగి s;
}


LUA_API స్థిరరాశి అక్షరం *lua_pushvfstring (lua_State *L, స్థిరరాశి అక్షరం *fmt,
                                      va_list argp) {
  స్థిరరాశి అక్షరం *ret;
  lua_తాళం(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  తిరిగి ret;
}


LUA_API స్థిరరాశి అక్షరం *lua_pushfstring (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...) {
  స్థిరరాశి అక్షరం *ret;
  va_list argp;
  lua_తాళం(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  తిరిగి ret;
}


LUA_API శూన్య lua_pushcclosure (lua_State *L, lua_CFunction fn, పూర్ణాంకం n) {
  lua_తాళం(L);
  ఒకవేళ (n == 0) {
    setfvalue(s2v(L->శిఖరం.p), fn);
    api_incr_శిఖరం(L);
  }
  లేకపోతే {
    పూర్ణాంకం i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    కొరకు (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->శిఖరం.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->శిఖరం.p -= n;
    setclCvalue(L, s2v(L->శిఖరం.p), cl);
    api_incr_శిఖరం(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API శూన్య lua_pushboolean (lua_State *L, పూర్ణాంకం b) {
  lua_తాళం(L);
  ఒకవేళ (b)
    setbtvalue(s2v(L->శిఖరం.p));
  లేకపోతే
    setbfvalue(s2v(L->శిఖరం.p));
  api_incr_శిఖరం(L);
  lua_unlock(L);
}


LUA_API శూన్య lua_pushlightuserdata (lua_State *L, శూన్య *p) {
  lua_తాళం(L);
  setpvalue(s2v(L->శిఖరం.p), p);
  api_incr_శిఖరం(L);
  lua_unlock(L);
}


LUA_API పూర్ణాంకం lua_pushthread (lua_State *L) {
  lua_తాళం(L);
  setthvalue(L, s2v(L->శిఖరం.p), L);
  api_incr_శిఖరం(L);
  lua_unlock(L);
  తిరిగి (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


స్థిర పూర్ణాంకం auxgetstr (lua_State *L, స్థిరరాశి TValue *t, స్థిరరాశి అక్షరం *k) {
  lu_byte ట్యాగ్;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->శిఖరం.p), luaH_getstr, ట్యాగ్);
  ఒకవేళ (!tagisempty(ట్యాగ్))
    api_incr_శిఖరం(L);
  లేకపోతే {
    setsvalue2s(L, L->శిఖరం.p, str);
    api_incr_శిఖరం(L);
    ట్యాగ్ = luaV_finishget(L, t, s2v(L->శిఖరం.p - 1), L->శిఖరం.p - 1, ట్యాగ్);
  }
  lua_unlock(L);
  తిరిగి novariant(ట్యాగ్);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
స్థిర శూన్య get_Global_పట్టిక (lua_State *L, TValue *gt) {
  పట్టిక *registry = hvalue(&G(L)->l_registry);
  lu_byte ట్యాగ్ = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (శూన్య)ట్యాగ్;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(ట్యాగ్) == LUA_TTABLE, "global table must exist");
}


LUA_API పూర్ణాంకం lua_getglobal (lua_State *L, స్థిరరాశి అక్షరం *name) {
  TValue gt;
  lua_తాళం(L);
  get_Global_పట్టిక(L, &gt);
  తిరిగి auxgetstr(L, &gt, name);
}


LUA_API పూర్ణాంకం lua_gettable (lua_State *L, పూర్ణాంకం idx) {
  lu_byte ట్యాగ్;
  TValue *t;
  lua_తాళం(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->శిఖరం.p - 1), s2v(L->శిఖరం.p - 1), luaH_get, ట్యాగ్);
  ఒకవేళ (tagisempty(ట్యాగ్))
    ట్యాగ్ = luaV_finishget(L, t, s2v(L->శిఖరం.p - 1), L->శిఖరం.p - 1, ట్యాగ్);
  lua_unlock(L);
  తిరిగి novariant(ట్యాగ్);
}


LUA_API పూర్ణాంకం lua_getfield (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి అక్షరం *k) {
  lua_తాళం(L);
  తిరిగి auxgetstr(L, index2value(L, idx), k);
}


LUA_API పూర్ణాంకం lua_geti (lua_State *L, పూర్ణాంకం idx, lua_Integer n) {
  TValue *t;
  lu_byte ట్యాగ్;
  lua_తాళం(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->శిఖరం.p), ట్యాగ్);
  ఒకవేళ (tagisempty(ట్యాగ్)) {
    TValue కీలకం;
    setivalue(&కీలకం, n);
    ట్యాగ్ = luaV_finishget(L, t, &కీలకం, L->శిఖరం.p, ట్యాగ్);
  }
  api_incr_శిఖరం(L);
  lua_unlock(L);
  తిరిగి novariant(ట్యాగ్);
}


స్థిర పూర్ణాంకం finishrawget (lua_State *L, lu_byte ట్యాగ్) {
  ఒకవేళ (tagisempty(ట్యాగ్))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->శిఖరం.p);
  api_incr_శిఖరం(L);
  lua_unlock(L);
  తిరిగి novariant(ట్యాగ్);
}


l_sinline పట్టిక *gettable (lua_State *L, పూర్ణాంకం idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  తిరిగి hvalue(t);
}


LUA_API పూర్ణాంకం lua_rawget (lua_State *L, పూర్ణాంకం idx) {
  పట్టిక *t;
  lu_byte ట్యాగ్;
  lua_తాళం(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  ట్యాగ్ = luaH_get(t, s2v(L->శిఖరం.p - 1), s2v(L->శిఖరం.p - 1));
  L->శిఖరం.p--;  /* pop key */
  తిరిగి finishrawget(L, ట్యాగ్);
}


LUA_API పూర్ణాంకం lua_rawgeti (lua_State *L, పూర్ణాంకం idx, lua_Integer n) {
  పట్టిక *t;
  lu_byte ట్యాగ్;
  lua_తాళం(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->శిఖరం.p), ట్యాగ్);
  తిరిగి finishrawget(L, ట్యాగ్);
}


LUA_API పూర్ణాంకం lua_rawgetp (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి శూన్య *p) {
  పట్టిక *t;
  TValue k;
  lua_తాళం(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  తిరిగి finishrawget(L, luaH_get(t, &k, s2v(L->శిఖరం.p)));
}


LUA_API శూన్య lua_createtable (lua_State *L, పూర్ణాంకం narray, పూర్ణాంకం nrec) {
  పట్టిక *t;
  lua_తాళం(L);
  t = luaH_new(L);
  sethvalue2s(L, L->శిఖరం.p, t);
  api_incr_శిఖరం(L);
  ఒకవేళ (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API పూర్ణాంకం lua_getmetatable (lua_State *L, పూర్ణాంకం objindex) {
  స్థిరరాశి TValue *obj;
  పట్టిక *mt;
  పూర్ణాంకం res = 0;
  lua_తాళం(L);
  obj = index2value(L, objindex);
  ఎంపిక (ttype(obj)) {
    స్థితి LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      ఆపు;
    స్థితి LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      ఆపు;
    అప్రమేయ:
      mt = G(L)->mt[ttype(obj)];
      ఆపు;
  }
  ఒకవేళ (mt != ఖాళీ) {
    sethvalue2s(L, L->శిఖరం.p, mt);
    api_incr_శిఖరం(L);
    res = 1;
  }
  lua_unlock(L);
  తిరిగి res;
}


LUA_API పూర్ణాంకం lua_getiuservalue (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం n) {
  TValue *o;
  పూర్ణాంకం t;
  lua_తాళం(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ఒకవేళ (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->శిఖరం.p);
    t = LUA_TNONE;
  }
  లేకపోతే {
    setobj2s(L, L->శిఖరం.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->శిఖరం.p));
  }
  api_incr_శిఖరం(L);
  lua_unlock(L);
  తిరిగి t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
స్థిర శూన్య auxsetstr (lua_State *L, స్థిరరాశి TValue *t, స్థిరరాశి అక్షరం *k) {
  పూర్ణాంకం hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->శిఖరం.p - 1), hres, luaH_psetstr);
  ఒకవేళ (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->శిఖరం.p - 1));
    L->శిఖరం.p--;  /* pop value */
  }
  లేకపోతే {
    setsvalue2s(L, L->శిఖరం.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_శిఖరం(L);
    luaV_finishset(L, t, s2v(L->శిఖరం.p - 1), s2v(L->శిఖరం.p - 2), hres);
    L->శిఖరం.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API శూన్య lua_setglobal (lua_State *L, స్థిరరాశి అక్షరం *name) {
  TValue gt;
  lua_తాళం(L);  /* unlock done in 'auxsetstr' */
  get_Global_పట్టిక(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API శూన్య lua_settable (lua_State *L, పూర్ణాంకం idx) {
  TValue *t;
  పూర్ణాంకం hres;
  lua_తాళం(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->శిఖరం.p - 2), s2v(L->శిఖరం.p - 1), hres, luaH_pset);
  ఒకవేళ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->శిఖరం.p - 1));
  లేకపోతే
    luaV_finishset(L, t, s2v(L->శిఖరం.p - 2), s2v(L->శిఖరం.p - 1), hres);
  L->శిఖరం.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API శూన్య lua_setfield (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి అక్షరం *k) {
  lua_తాళం(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API శూన్య lua_seti (lua_State *L, పూర్ణాంకం idx, lua_Integer n) {
  TValue *t;
  పూర్ణాంకం hres;
  lua_తాళం(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->శిఖరం.p - 1), hres);
  ఒకవేళ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->శిఖరం.p - 1));
  లేకపోతే {
    TValue తాత్కాలిక;
    setivalue(&తాత్కాలిక, n);
    luaV_finishset(L, t, &తాత్కాలిక, s2v(L->శిఖరం.p - 1), hres);
  }
  L->శిఖరం.p--;  /* pop value */
  lua_unlock(L);
}


స్థిర శూన్య aux_rawset (lua_State *L, పూర్ణాంకం idx, TValue *కీలకం, పూర్ణాంకం n) {
  పట్టిక *t;
  lua_తాళం(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  lua_H_సమితి(L, t, కీలకం, s2v(L->శిఖరం.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->శిఖరం.p - 1));
  L->శిఖరం.p -= n;
  lua_unlock(L);
}


LUA_API శూన్య lua_rawset (lua_State *L, పూర్ణాంకం idx) {
  aux_rawset(L, idx, s2v(L->శిఖరం.p - 2), 2);
}


LUA_API శూన్య lua_rawsetp (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి శూన్య *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API శూన్య lua_rawseti (lua_State *L, పూర్ణాంకం idx, lua_Integer n) {
  పట్టిక *t;
  lua_తాళం(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->శిఖరం.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->శిఖరం.p - 1));
  L->శిఖరం.p--;
  lua_unlock(L);
}


LUA_API పూర్ణాంకం lua_setmetatable (lua_State *L, పూర్ణాంకం objindex) {
  TValue *obj;
  పట్టిక *mt;
  lua_తాళం(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  ఒకవేళ (ttisnil(s2v(L->శిఖరం.p - 1)))
    mt = ఖాళీ;
  లేకపోతే {
    api_check(L, ttistable(s2v(L->శిఖరం.p - 1)), "table expected");
    mt = hvalue(s2v(L->శిఖరం.p - 1));
  }
  ఎంపిక (ttype(obj)) {
    స్థితి LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      ఒకవేళ (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ఆపు;
    }
    స్థితి LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      ఒకవేళ (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ఆపు;
    }
    అప్రమేయ: {
      G(L)->mt[ttype(obj)] = mt;
      ఆపు;
    }
  }
  L->శిఖరం.p--;
  lua_unlock(L);
  తిరిగి 1;
}


LUA_API పూర్ణాంకం lua_setiuservalue (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం n) {
  TValue *o;
  పూర్ణాంకం res;
  lua_తాళం(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ఒకవేళ (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  లేకపోతే {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->శిఖరం.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->శిఖరం.p - 1));
    res = 1;
  }
  L->శిఖరం.p--;
  lua_unlock(L);
  తిరిగి res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#నిర్వచించు checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->శిఖరం.p - L->శిఖరం.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API శూన్య lua_callk (lua_State *L, పూర్ణాంకం nargs, పూర్ణాంకం nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_తాళం(L);
  api_check(L, k == ఖాళీ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->స్థితి_2 == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->శిఖరం.p - (nargs+1);
  ఒకవేళ (k != ఖాళీ && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  లేకపోతే  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
నిర్మాణం CallS {  /* data to 'f_call' */
  StkId func;
  పూర్ణాంకం nresults;
};


స్థిర శూన్య f_call (lua_State *L, శూన్య *ud) {
  నిర్మాణం CallS *c = cast(నిర్మాణం CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API పూర్ణాంకం lua_pcallk (lua_State *L, పూర్ణాంకం nargs, పూర్ణాంకం nresults, పూర్ణాంకం errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  నిర్మాణం CallS c;
  TStatus స్థితి_2;
  ptrdiff_t func;
  lua_తాళం(L);
  api_check(L, k == ఖాళీ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->స్థితి_2 == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  ఒకవేళ (errfunc == 0)
    func = 0;
  లేకపోతే {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->శిఖరం.p - (nargs+1);  /* function to be called */
  ఒకవేళ (k == ఖాళీ || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    స్థితి_2 = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  లేకపోతే {  /* prepare continuation (call is already protected by 'resume') */
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
    స్థితి_2 = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  తిరిగి APIstatus(స్థితి_2);
}


LUA_API పూర్ణాంకం lua_ఎక్కించు (lua_State *L, lua_Reader reader, శూన్య *దత్తాంశం,
                      స్థిరరాశి అక్షరం *chunkname, స్థిరరాశి అక్షరం *బహుళకం) {
  ZIO z;
  TStatus స్థితి_2;
  lua_తాళం(L);
  luaC_checkGC(L);
  ఒకవేళ (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, దత్తాంశం);
  స్థితి_2 = luaD_protectedparser(L, &z, chunkname, బహుళకం);
  ఒకవేళ (స్థితి_2 == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->శిఖరం.p - 1));  /* get new function */
    ఒకవేళ (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      get_Global_పట్టిక(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  తిరిగి APIstatus(స్థితి_2);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API పూర్ణాంకం lua_dump (lua_State *L, lua_Writer writer, శూన్య *దత్తాంశం, పూర్ణాంకం strip) {
  పూర్ణాంకం స్థితి_2;
  ptrdiff_t otop = savestack(L, L->శిఖరం.p);  /* original top */
  TValue *f = s2v(L->శిఖరం.p - 1);  /* function to be dumped */
  lua_తాళం(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  స్థితి_2 = luaU_dump(L, clLvalue(f)->p, writer, దత్తాంశం, strip);
  L->శిఖరం.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  తిరిగి స్థితి_2;
}


LUA_API పూర్ణాంకం lua_స్థితి (lua_State *L) {
  తిరిగి APIstatus(L->స్థితి_2);
}


/*
** Garbage-collection function
*/
LUA_API పూర్ణాంకం lua_gc (lua_State *L, పూర్ణాంకం what, ...) {
  va_list argp;
  పూర్ణాంకం res = 0;
  global_State *g = G(L);
  ఒకవేళ (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    తిరిగి -1;  /* all options are invalid when stopped */
  lua_తాళం(L);
  va_start(argp, what);
  ఎంపిక (what) {
    స్థితి LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      ఆపు;
    }
    స్థితి LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      ఆపు;
    }
    స్థితి LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      ఆపు;
    }
    స్థితి LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      ఆపు;
    }
    స్థితి LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      ఆపు;
    }
    స్థితి LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      పూర్ణాంకం work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      ఒకవేళ (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      లేకపోతే ఒకవేళ (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      లేకపోతే  /*అధికప్రవాహం*/
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (శూన్య)0, work = 1);
      ఒకవేళ (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      ఆపు;
    }
    స్థితి LUA_GCISRUNNING: {
      res = gcrunning(g);
      ఆపు;
    }
    స్థితి LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      ఆపు;
    }
    స్థితి LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      ఆపు;
    }
    స్థితి LUA_GCPARAM: {
      పూర్ణాంకం param = va_arg(argp, పూర్ణాంకం);
      పూర్ణాంకం విలువ = va_arg(argp, పూర్ణాంకం);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      ఒకవేళ (విలువ >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(విలువ));
      ఆపు;
    }
    అప్రమేయ: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  తిరిగి res;
}



/*
** miscellaneous functions
*/


LUA_API పూర్ణాంకం lua_error (lua_State *L) {
  TValue *errobj;
  lua_తాళం(L);
  errobj = s2v(L->శిఖరం.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  ఒకవేళ (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  లేకపోతే
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  తిరిగి 0;  /* to avoid warnings */
}


LUA_API పూర్ణాంకం lua_తదుపరి (lua_State *L, పూర్ణాంకం idx) {
  పట్టిక *t;
  పూర్ణాంకం మరిన్ని;
  lua_తాళం(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  మరిన్ని = lua_H_తదుపరి(L, t, L->శిఖరం.p - 1);
  ఒకవేళ (మరిన్ని)
    api_incr_శిఖరం(L);
  లేకపోతే  /* no more elements */
    L->శిఖరం.p--;  /* pop key */
  lua_unlock(L);
  తిరిగి మరిన్ని;
}


LUA_API శూన్య lua_toclose (lua_State *L, పూర్ణాంకం idx) {
  StkId o;
  lua_తాళం(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API శూన్య lua_concat (lua_State *L, పూర్ణాంకం n) {
  lua_తాళం(L);
  ఒకవేళ (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  లేకపోతే {  /* nothing to concatenate */
    setsvalue2s(L, L->శిఖరం.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_శిఖరం(L);
  }
  lua_unlock(L);
}


LUA_API శూన్య lua_len (lua_State *L, పూర్ణాంకం idx) {
  TValue *t;
  lua_తాళం(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->శిఖరం.p, t);
  api_incr_శిఖరం(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, శూన్య **ud) {
  lua_Alloc f;
  lua_తాళం(L);
  ఒకవేళ (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  తిరిగి f;
}


LUA_API శూన్య lua_setallocf (lua_State *L, lua_Alloc f, శూన్య *ud) {
  lua_తాళం(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


శూన్య lua_setwarnf (lua_State *L, lua_WarnFunction f, శూన్య *ud) {
  lua_తాళం(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


శూన్య lua_warning (lua_State *L, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం tocont) {
  lua_తాళం(L);
  luaE_warning(L, సందేశం, tocont);
  lua_unlock(L);
}



LUA_API శూన్య *lua_newuserdatauv (lua_State *L, size_t కొలత, పూర్ణాంకం nuvalue) {
  Udata *u;
  lua_తాళం(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, కొలత, cast(గుర్తులేని పొట్టి, nuvalue));
  setuvalue(L, s2v(L->శిఖరం.p), u);
  api_incr_శిఖరం(L);
  luaC_checkGC(L);
  lua_unlock(L);
  తిరిగి getudatamem(u);
}



స్థిర స్థిరరాశి అక్షరం *aux_upvalue (TValue *fi, పూర్ణాంకం n, TValue **val,
                                GCObject **owner) {
  ఎంపిక (ttypetag(fi)) {
    స్థితి LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ఒకవేళ (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        తిరిగి ఖాళీ;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      ఒకవేళ (owner) *owner = obj2gco(f);
      తిరిగి "";
    }
    స్థితి LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      ఒకవేళ (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        తిరిగి ఖాళీ;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      ఒకవేళ (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      తిరిగి (name == ఖాళీ) ? "(no name)" : getstr(name);
    }
    అప్రమేయ: తిరిగి ఖాళీ;  /* not a closure */
  }
}


LUA_API స్థిరరాశి అక్షరం *lua_getupvalue (lua_State *L, పూర్ణాంకం funcindex, పూర్ణాంకం n) {
  స్థిరరాశి అక్షరం *name;
  TValue *val = ఖాళీ;  /* to avoid warnings */
  lua_తాళం(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ఖాళీ);
  ఒకవేళ (name) {
    setobj2s(L, L->శిఖరం.p, val);
    api_incr_శిఖరం(L);
  }
  lua_unlock(L);
  తిరిగి name;
}


LUA_API స్థిరరాశి అక్షరం *lua_setupvalue (lua_State *L, పూర్ణాంకం funcindex, పూర్ణాంకం n) {
  స్థిరరాశి అక్షరం *name;
  TValue *val = ఖాళీ;  /* to avoid warnings */
  GCObject *owner = ఖాళీ;  /* to avoid warnings */
  TValue *fi;
  lua_తాళం(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  ఒకవేళ (name) {
    L->శిఖరం.p--;
    setobj(L, val, s2v(L->శిఖరం.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  తిరిగి name;
}


స్థిర పైకి_Val **getupvalref (lua_State *L, పూర్ణాంకం fidx, పూర్ణాంకం n, LClosure **pf) {
  స్థిర స్థిరరాశి పైకి_Val *స్థిరరాశి nullup = ఖాళీ;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  ఒకవేళ (pf) *pf = f;
  ఒకవేళ (1 <= n && n <= f->p->sizeupvalues)
    తిరిగి &f->upvals[n - 1];  /* get its upvalue pointer */
  లేకపోతే
    తిరిగి (పైకి_Val**)&nullup;
}


LUA_API శూన్య *lua_upvalueid (lua_State *L, పూర్ణాంకం fidx, పూర్ణాంకం n) {
  TValue *fi = index2value(L, fidx);
  ఎంపిక (ttypetag(fi)) {
    స్థితి LUA_VLCL: {  /* lua closure */
      తిరిగి *getupvalref(L, fidx, n, ఖాళీ);
    }
    స్థితి LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ఒకవేళ (1 <= n && n <= f->nupvalues)
        తిరిగి &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    స్థితి LUA_VLCF:
      తిరిగి ఖాళీ;  /* light C functions have no upvalues */
    అప్రమేయ: {
      api_check(L, 0, "function expected");
      తిరిగి ఖాళీ;
    }
  }
}


LUA_API శూన్య lua_upvaluejoin (lua_State *L, పూర్ణాంకం fidx1, పూర్ణాంకం n1,
                                            పూర్ణాంకం fidx2, పూర్ణాంకం n2) {
  LClosure *f1;
  పైకి_Val **up1 = getupvalref(L, fidx1, n1, &f1);
  పైకి_Val **up2 = getupvalref(L, fidx2, n2, ఖాళీ);
  api_check(L, *up1 != ఖాళీ && *up2 != ఖాళీ, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


