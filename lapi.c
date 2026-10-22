/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lapi.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lfunc.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "ltm.h"
#અન્તર્ભાવ "lundump.h"
#અન્તર્ભાવ "lvm.h"



અચલ અક્ષર lua_ident[] =
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
સ્થિર TValue *index2value (lua_State *L, પૂર્ણાંક idx) {
  CallInfo *ci = L->ci;
  જો (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->ટોચ.p - (ci->func.p + 1), "unacceptable index");
    જો (o >= L->ટોચ.p) પાછા &G(L)->nilvalue;
    નહીંતર પાછા s2v(o);
  }
  નહીંતર જો (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->ટોચ.p - (ci->func.p + 1),
                 "invalid index");
    પાછા s2v(L->ટોચ.p + idx);
  }
  નહીંતર જો (idx == LUA_REGISTRYINDEX)
    પાછા &G(L)->l_registry;
  નહીંતર {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    જો (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      પાછા (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    નહીંતર {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      પાછા &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
સ્થિર StkId index2stack (lua_State *L, પૂર્ણાંક idx) {
  CallInfo *ci = L->ci;
  જો (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->ટોચ.p, "invalid index");
    પાછા o;
  }
  નહીંતર {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->ટોચ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    પાછા L->ટોચ.p + idx;
  }
}


LUA_API પૂર્ણાંક lua_checkstack (lua_State *L, પૂર્ણાંક n) {
  પૂર્ણાંક res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  જો (L->ઢગલો_last.p - L->ટોચ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  નહીંતર  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  જો (res && ci->ટોચ.p < L->ટોચ.p + n)
    ci->ટોચ.p = L->ટોચ.p + n;  /* adjust frame top */
  lua_unlock(L);
  પાછા res;
}


LUA_API શૂન્ય lua_xmove (lua_State *from, lua_State *to, પૂર્ણાંક n) {
  પૂર્ણાંક i;
  જો (from == to) પાછા;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->ટોચ.p - to->ટોચ.p >= n, "stack overflow");
  from->ટોચ.p -= n;
  માટે (i = 0; i < n; i++) {
    setobjs2s(to, to->ટોચ.p, from->ટોચ.p + i);
    to->ટોચ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  પાછા old;
}


LUA_API lua_સંખ્યા lua_version (lua_State *L) {
  UNUSED(L);
  પાછા LUA_VERSION_અંક;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API પૂર્ણાંક lua_absindex (lua_State *L, પૂર્ણાંક idx) {
  પાછા (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->ટોચ.p - L->ci->func.p) + idx;
}


LUA_API પૂર્ણાંક lua_gettop (lua_State *L) {
  પાછા cast_int(L->ટોચ.p - (L->ci->func.p + 1));
}


LUA_API શૂન્ય lua_settop (lua_State *L, પૂર્ણાંક idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  જો (idx >= 0) {
    api_check(L, idx <= ci->ટોચ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->ટોચ.p;
    માટે (; diff > 0; diff--)
      setnilvalue2s(L->ટોચ.p++);  /* clear new slots */
  }
  નહીંતર {
    api_check(L, -(idx+1) <= (L->ટોચ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->ટોચ.p + diff;
  જો (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->ટોચ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API શૂન્ય lua_closeslot (lua_State *L, પૂર્ણાંક idx) {
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
સ્થિર શૂન્ય reverse (lua_State *L, StkId from, StkId to) {
  માટે (; from < to; from++, to--) {
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
LUA_API શૂન્ય lua_rotate (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->ટોચ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API શૂન્ય lua_copy (lua_State *L, પૂર્ણાંક fromidx, પૂર્ણાંક toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  જો (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API શૂન્ય lua_pushvalue (lua_State *L, પૂર્ણાંક idx) {
  lua_lock(L);
  setobj2s(L, L->ટોચ.p, index2value(L, idx));
  api_incr_ટોચ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API પૂર્ણાંક lua_type (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API અચલ અક્ષર *lua_typename (lua_State *L, પૂર્ણાંક t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  પાછા ttypename(t);
}


LUA_API પૂર્ણાંક lua_iscfunction (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API પૂર્ણાંક lua_isinteger (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા ttisinteger(o);
}


LUA_API પૂર્ણાંક lua_isnumber (lua_State *L, પૂર્ણાંક idx) {
  lua_સંખ્યા n;
  અચલ TValue *o = index2value(L, idx);
  પાછા tonumber(o, &n);
}


LUA_API પૂર્ણાંક lua_isstring (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા (ttisstring(o) || cvt2str(o));
}


LUA_API પૂર્ણાંક lua_isuserdata (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API પૂર્ણાંક lua_rawequal (lua_State *L, પૂર્ણાંક index1, પૂર્ણાંક index2) {
  અચલ TValue *o1 = index2value(L, index1);
  અચલ TValue *o2 = index2value(L, index2);
  પાછા (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API શૂન્ય lua_arith (lua_State *L, પૂર્ણાંક op) {
  lua_lock(L);
  જો (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  નહીંતર {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->ટોચ.p, L->ટોચ.p - 1);
    api_incr_ટોચ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->ટોચ.p - 2), s2v(L->ટોચ.p - 1), L->ટોચ.p - 2);
  L->ટોચ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API પૂર્ણાંક lua_compare (lua_State *L, પૂર્ણાંક index1, પૂર્ણાંક index2, પૂર્ણાંક op) {
  અચલ TValue *o1;
  અચલ TValue *o2;
  પૂર્ણાંક i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  જો (isvalid(L, o1) && isvalid(L, o2)) {
    પસંદગી (op) {
      સ્થિતિ LUA_OPEQ: i = luaV_equalobj(L, o1, o2); તોડો;
      સ્થિતિ LUA_OPLT: i = luaV_lessthan(L, o1, o2); તોડો;
      સ્થિતિ LUA_OPLE: i = luaV_lessequal(L, o1, o2); તોડો;
      મૂળભૂત: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  પાછા i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, પૂર્ણાંક idx, અક્ષર *buff) {
  અચલ TValue *o = index2value(L, idx);
  જો (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    પાછા len;
  }
  નહીંતર
    પાછા 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, અચલ અક્ષર *s) {
  size_t sz = luaO_str2num(s, s2v(L->ટોચ.p));
  જો (sz != 0)
    api_incr_ટોચ(L);
  પાછા sz;
}


LUA_API lua_સંખ્યા lua_tonumberx (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક *pisnum) {
  lua_સંખ્યા n = 0;
  અચલ TValue *o = index2value(L, idx);
  પૂર્ણાંક isnum = tonumber(o, &n);
  જો (pisnum)
    *pisnum = isnum;
  પાછા n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક *pisnum) {
  lua_Integer res = 0;
  અચલ TValue *o = index2value(L, idx);
  પૂર્ણાંક isnum = tointeger(o, &res);
  જો (pisnum)
    *pisnum = isnum;
  પાછા res;
}


LUA_API પૂર્ણાંક lua_toboolean (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા !l_isfalse(o);
}


LUA_API અચલ અક્ષર *lua_tolstring (lua_State *L, પૂર્ણાંક idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  જો (!ttisstring(o)) {
    જો (!cvt2str(o)) {  /* not convertible? */
      જો (len != ખાલી) *len = 0;
      lua_unlock(L);
      પાછા ખાલી;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  જો (len != ખાલી)
    પાછા getlstr(tsvalue(o), *len);
  નહીંતર
    પાછા getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પસંદગી (ttypetag(o)) {
    સ્થિતિ LUA_VSHRSTR: પાછા cast(lua_Unsigned, tsvalue(o)->shrlen);
    સ્થિતિ LUA_VLNGSTR: પાછા cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    સ્થિતિ LUA_VUSERDATA: પાછા cast(lua_Unsigned, uvalue(o)->len);
    સ્થિતિ LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      પાછા res;
    }
    મૂળભૂત: પાછા 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  જો (ttislcf(o)) પાછા fvalue(o);
  નહીંતર જો (ttisCclosure(o))
    પાછા clCvalue(o)->f;
  નહીંતર પાછા ખાલી;  /* not a C function */
}


l_sinline શૂન્ય *touserdata (અચલ TValue *o) {
  પસંદગી (ttype(o)) {
    સ્થિતિ LUA_TUSERDATA: પાછા getudatamem(uvalue(o));
    સ્થિતિ LUA_TLIGHTUSERDATA: પાછા pvalue(o);
    મૂળભૂત: પાછા ખાલી;
  }
}


LUA_API શૂન્ય *lua_touserdata (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પાછા (!ttisthread(o)) ? ખાલી : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API અચલ શૂન્ય *lua_topointer (lua_State *L, પૂર્ણાંક idx) {
  અચલ TValue *o = index2value(L, idx);
  પસંદગી (ttypetag(o)) {
    સ્થિતિ LUA_VLCF: પાછા cast_voidp(cast_sizet(fvalue(o)));
    સ્થિતિ LUA_VUSERDATA: સ્થિતિ LUA_VLIGHTUSERDATA:
      પાછા touserdata(o);
    મૂળભૂત: {
      જો (iscollectable(o))
        પાછા gcvalue(o);
      નહીંતર
        પાછા ખાલી;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API શૂન્ય lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->ટોચ.p);
  api_incr_ટોચ(L);
  lua_unlock(L);
}


LUA_API શૂન્ય lua_pushnumber (lua_State *L, lua_સંખ્યા n) {
  lua_lock(L);
  setfltvalue(s2v(L->ટોચ.p), n);
  api_incr_ટોચ(L);
  lua_unlock(L);
}


LUA_API શૂન્ય lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->ટોચ.p), n);
  api_incr_ટોચ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API અચલ અક્ષર *lua_pushlstring (lua_State *L, અચલ અક્ષર *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->ટોચ.p, ts);
  api_incr_ટોચ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  પાછા getstr(ts);
}


LUA_API અચલ અક્ષર *lua_pushexternalstring (lua_State *L,
	        અચલ અક્ષર *s, size_t len, lua_Alloc falloc, શૂન્ય *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->ટોચ.p, ts);
  api_incr_ટોચ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  પાછા getstr(ts);
}


LUA_API અચલ અક્ષર *lua_pushstring (lua_State *L, અચલ અક્ષર *s) {
  lua_lock(L);
  જો (s == ખાલી)
    setnilvalue2s(L->ટોચ.p);
  નહીંતર {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->ટોચ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_ટોચ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  પાછા s;
}


LUA_API અચલ અક્ષર *lua_pushvfstring (lua_State *L, અચલ અક્ષર *fmt,
                                      va_list argp) {
  અચલ અક્ષર *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  પાછા ret;
}


LUA_API અચલ અક્ષર *lua_pushfstring (lua_State *L, અચલ અક્ષર *fmt, ...) {
  અચલ અક્ષર *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  પાછા ret;
}


LUA_API શૂન્ય lua_pushcclosure (lua_State *L, lua_CFunction fn, પૂર્ણાંક n) {
  lua_lock(L);
  જો (n == 0) {
    setfvalue(s2v(L->ટોચ.p), fn);
    api_incr_ટોચ(L);
  }
  નહીંતર {
    પૂર્ણાંક i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    માટે (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->ટોચ.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->ટોચ.p -= n;
    setclCvalue(L, s2v(L->ટોચ.p), cl);
    api_incr_ટોચ(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API શૂન્ય lua_pushboolean (lua_State *L, પૂર્ણાંક b) {
  lua_lock(L);
  જો (b)
    setbtvalue(s2v(L->ટોચ.p));
  નહીંતર
    setbfvalue(s2v(L->ટોચ.p));
  api_incr_ટોચ(L);
  lua_unlock(L);
}


LUA_API શૂન્ય lua_pushlightuserdata (lua_State *L, શૂન્ય *p) {
  lua_lock(L);
  setpvalue(s2v(L->ટોચ.p), p);
  api_incr_ટોચ(L);
  lua_unlock(L);
}


LUA_API પૂર્ણાંક lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->ટોચ.p), L);
  api_incr_ટોચ(L);
  lua_unlock(L);
  પાછા (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


સ્થિર પૂર્ણાંક auxgetstr (lua_State *L, અચલ TValue *t, અચલ અક્ષર *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->ટોચ.p), luaH_getstr, tag);
  જો (!tagisempty(tag))
    api_incr_ટોચ(L);
  નહીંતર {
    setsvalue2s(L, L->ટોચ.p, str);
    api_incr_ટોચ(L);
    tag = luaV_finishget(L, t, s2v(L->ટોચ.p - 1), L->ટોચ.p - 1, tag);
  }
  lua_unlock(L);
  પાછા novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
સ્થિર શૂન્ય getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (શૂન્ય)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API પૂર્ણાંક lua_getglobal (lua_State *L, અચલ અક્ષર *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  પાછા auxgetstr(L, &gt, name);
}


LUA_API પૂર્ણાંક lua_gettable (lua_State *L, પૂર્ણાંક idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->ટોચ.p - 1), s2v(L->ટોચ.p - 1), luaH_get, tag);
  જો (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->ટોચ.p - 1), L->ટોચ.p - 1, tag);
  lua_unlock(L);
  પાછા novariant(tag);
}


LUA_API પૂર્ણાંક lua_getfield (lua_State *L, પૂર્ણાંક idx, અચલ અક્ષર *k) {
  lua_lock(L);
  પાછા auxgetstr(L, index2value(L, idx), k);
}


LUA_API પૂર્ણાંક lua_geti (lua_State *L, પૂર્ણાંક idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->ટોચ.p), tag);
  જો (tagisempty(tag)) {
    TValue ચાવી;
    setivalue(&ચાવી, n);
    tag = luaV_finishget(L, t, &ચાવી, L->ટોચ.p, tag);
  }
  api_incr_ટોચ(L);
  lua_unlock(L);
  પાછા novariant(tag);
}


સ્થિર પૂર્ણાંક finishrawget (lua_State *L, lu_byte tag) {
  જો (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->ટોચ.p);
  api_incr_ટોચ(L);
  lua_unlock(L);
  પાછા novariant(tag);
}


l_sinline Table *gettable (lua_State *L, પૂર્ણાંક idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  પાછા hvalue(t);
}


LUA_API પૂર્ણાંક lua_rawget (lua_State *L, પૂર્ણાંક idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->ટોચ.p - 1), s2v(L->ટોચ.p - 1));
  L->ટોચ.p--;  /* pop key */
  પાછા finishrawget(L, tag);
}


LUA_API પૂર્ણાંક lua_rawgeti (lua_State *L, પૂર્ણાંક idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->ટોચ.p), tag);
  પાછા finishrawget(L, tag);
}


LUA_API પૂર્ણાંક lua_rawgetp (lua_State *L, પૂર્ણાંક idx, અચલ શૂન્ય *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  પાછા finishrawget(L, luaH_get(t, &k, s2v(L->ટોચ.p)));
}


LUA_API શૂન્ય lua_createtable (lua_State *L, પૂર્ણાંક narray, પૂર્ણાંક nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->ટોચ.p, t);
  api_incr_ટોચ(L);
  જો (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API પૂર્ણાંક lua_getmetatable (lua_State *L, પૂર્ણાંક objindex) {
  અચલ TValue *obj;
  Table *mt;
  પૂર્ણાંક res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  પસંદગી (ttype(obj)) {
    સ્થિતિ LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      તોડો;
    સ્થિતિ LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      તોડો;
    મૂળભૂત:
      mt = G(L)->mt[ttype(obj)];
      તોડો;
  }
  જો (mt != ખાલી) {
    sethvalue2s(L, L->ટોચ.p, mt);
    api_incr_ટોચ(L);
    res = 1;
  }
  lua_unlock(L);
  પાછા res;
}


LUA_API પૂર્ણાંક lua_getiuservalue (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક n) {
  TValue *o;
  પૂર્ણાંક t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  જો (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->ટોચ.p);
    t = LUA_TNONE;
  }
  નહીંતર {
    setobj2s(L, L->ટોચ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->ટોચ.p));
  }
  api_incr_ટોચ(L);
  lua_unlock(L);
  પાછા t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
સ્થિર શૂન્ય auxsetstr (lua_State *L, અચલ TValue *t, અચલ અક્ષર *k) {
  પૂર્ણાંક hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->ટોચ.p - 1), hres, luaH_psetstr);
  જો (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->ટોચ.p - 1));
    L->ટોચ.p--;  /* pop value */
  }
  નહીંતર {
    setsvalue2s(L, L->ટોચ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_ટોચ(L);
    luaV_finishset(L, t, s2v(L->ટોચ.p - 1), s2v(L->ટોચ.p - 2), hres);
    L->ટોચ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API શૂન્ય lua_setglobal (lua_State *L, અચલ અક્ષર *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API શૂન્ય lua_settable (lua_State *L, પૂર્ણાંક idx) {
  TValue *t;
  પૂર્ણાંક hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->ટોચ.p - 2), s2v(L->ટોચ.p - 1), hres, luaH_pset);
  જો (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ટોચ.p - 1));
  નહીંતર
    luaV_finishset(L, t, s2v(L->ટોચ.p - 2), s2v(L->ટોચ.p - 1), hres);
  L->ટોચ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API શૂન્ય lua_setfield (lua_State *L, પૂર્ણાંક idx, અચલ અક્ષર *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API શૂન્ય lua_seti (lua_State *L, પૂર્ણાંક idx, lua_Integer n) {
  TValue *t;
  પૂર્ણાંક hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->ટોચ.p - 1), hres);
  જો (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ટોચ.p - 1));
  નહીંતર {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->ટોચ.p - 1), hres);
  }
  L->ટોચ.p--;  /* pop value */
  lua_unlock(L);
}


સ્થિર શૂન્ય aux_rawset (lua_State *L, પૂર્ણાંક idx, TValue *ચાવી, પૂર્ણાંક n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, ચાવી, s2v(L->ટોચ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->ટોચ.p - 1));
  L->ટોચ.p -= n;
  lua_unlock(L);
}


LUA_API શૂન્ય lua_rawset (lua_State *L, પૂર્ણાંક idx) {
  aux_rawset(L, idx, s2v(L->ટોચ.p - 2), 2);
}


LUA_API શૂન્ય lua_rawsetp (lua_State *L, પૂર્ણાંક idx, અચલ શૂન્ય *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API શૂન્ય lua_rawseti (lua_State *L, પૂર્ણાંક idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->ટોચ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->ટોચ.p - 1));
  L->ટોચ.p--;
  lua_unlock(L);
}


LUA_API પૂર્ણાંક lua_setmetatable (lua_State *L, પૂર્ણાંક objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  જો (ttisnil(s2v(L->ટોચ.p - 1)))
    mt = ખાલી;
  નહીંતર {
    api_check(L, ttistable(s2v(L->ટોચ.p - 1)), "table expected");
    mt = hvalue(s2v(L->ટોચ.p - 1));
  }
  પસંદગી (ttype(obj)) {
    સ્થિતિ LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      જો (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      તોડો;
    }
    સ્થિતિ LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      જો (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      તોડો;
    }
    મૂળભૂત: {
      G(L)->mt[ttype(obj)] = mt;
      તોડો;
    }
  }
  L->ટોચ.p--;
  lua_unlock(L);
  પાછા 1;
}


LUA_API પૂર્ણાંક lua_setiuservalue (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક n) {
  TValue *o;
  પૂર્ણાંક res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  જો (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  નહીંતર {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->ટોચ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->ટોચ.p - 1));
    res = 1;
  }
  L->ટોચ.p--;
  lua_unlock(L);
  પાછા res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->ટોચ.p - L->ટોચ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API શૂન્ય lua_callk (lua_State *L, પૂર્ણાંક nargs, પૂર્ણાંક nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == ખાલી || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->ટોચ.p - (nargs+1);
  જો (k != ખાલી && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  નહીંતર  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
સંરચના CallS {  /* data to 'f_call' */
  StkId func;
  પૂર્ણાંક nresults;
};


સ્થિર શૂન્ય f_call (lua_State *L, શૂન્ય *ud) {
  સંરચના CallS *c = cast(સંરચના CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API પૂર્ણાંક lua_pcallk (lua_State *L, પૂર્ણાંક nargs, પૂર્ણાંક nresults, પૂર્ણાંક errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  સંરચના CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == ખાલી || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  જો (errfunc == 0)
    func = 0;
  નહીંતર {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->ટોચ.p - (nargs+1);  /* function to be called */
  જો (k == ખાલી || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  નહીંતર {  /* prepare continuation (call is already protected by 'resume') */
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
  પાછા APIstatus(status);
}


LUA_API પૂર્ણાંક lua_load (lua_State *L, lua_Reader reader, શૂન્ય *data,
                      અચલ અક્ષર *chunkname, અચલ અક્ષર *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  જો (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  જો (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->ટોચ.p - 1));  /* get new function */
    જો (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  પાછા APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API પૂર્ણાંક lua_dump (lua_State *L, lua_Writer writer, શૂન્ય *data, પૂર્ણાંક strip) {
  પૂર્ણાંક status;
  ptrdiff_t otop = savestack(L, L->ટોચ.p);  /* original top */
  TValue *f = s2v(L->ટોચ.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->ટોચ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  પાછા status;
}


LUA_API પૂર્ણાંક lua_status (lua_State *L) {
  પાછા APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API પૂર્ણાંક lua_gc (lua_State *L, પૂર્ણાંક what, ...) {
  va_list argp;
  પૂર્ણાંક res = 0;
  global_State *g = G(L);
  જો (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    પાછા -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  પસંદગી (what) {
    સ્થિતિ LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      તોડો;
    }
    સ્થિતિ LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      તોડો;
    }
    સ્થિતિ LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      તોડો;
    }
    સ્થિતિ LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      તોડો;
    }
    સ્થિતિ LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      તોડો;
    }
    સ્થિતિ LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      પૂર્ણાંક work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      જો (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      નહીંતર જો (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      નહીંતર  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (શૂન્ય)0, work = 1);
      જો (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      તોડો;
    }
    સ્થિતિ LUA_GCISRUNNING: {
      res = gcrunning(g);
      તોડો;
    }
    સ્થિતિ LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      તોડો;
    }
    સ્થિતિ LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      તોડો;
    }
    સ્થિતિ LUA_GCPARAM: {
      પૂર્ણાંક param = va_arg(argp, પૂર્ણાંક);
      પૂર્ણાંક કિંમત = va_arg(argp, પૂર્ણાંક);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      જો (કિંમત >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(કિંમત));
      તોડો;
    }
    મૂળભૂત: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  પાછા res;
}



/*
** miscellaneous functions
*/


LUA_API પૂર્ણાંક lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->ટોચ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  જો (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  નહીંતર
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  પાછા 0;  /* to avoid warnings */
}


LUA_API પૂર્ણાંક lua_next (lua_State *L, પૂર્ણાંક idx) {
  Table *t;
  પૂર્ણાંક more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->ટોચ.p - 1);
  જો (more)
    api_incr_ટોચ(L);
  નહીંતર  /* no more elements */
    L->ટોચ.p--;  /* pop key */
  lua_unlock(L);
  પાછા more;
}


LUA_API શૂન્ય lua_toclose (lua_State *L, પૂર્ણાંક idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API શૂન્ય lua_concat (lua_State *L, પૂર્ણાંક n) {
  lua_lock(L);
  જો (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  નહીંતર {  /* nothing to concatenate */
    setsvalue2s(L, L->ટોચ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_ટોચ(L);
  }
  lua_unlock(L);
}


LUA_API શૂન્ય lua_len (lua_State *L, પૂર્ણાંક idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->ટોચ.p, t);
  api_incr_ટોચ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, શૂન્ય **ud) {
  lua_Alloc f;
  lua_lock(L);
  જો (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  પાછા f;
}


LUA_API શૂન્ય lua_setallocf (lua_State *L, lua_Alloc f, શૂન્ય *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


શૂન્ય lua_setwarnf (lua_State *L, lua_WarnFunction f, શૂન્ય *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


શૂન્ય lua_warning (lua_State *L, અચલ અક્ષર *સંદેશ, પૂર્ણાંક tocont) {
  lua_lock(L);
  luaE_warning(L, સંદેશ, tocont);
  lua_unlock(L);
}



LUA_API શૂન્ય *lua_newuserdatauv (lua_State *L, size_t size, પૂર્ણાંક nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->ટોચ.p), u);
  api_incr_ટોચ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  પાછા getudatamem(u);
}



સ્થિર અચલ અક્ષર *aux_upvalue (TValue *fi, પૂર્ણાંક n, TValue **val,
                                GCObject **owner) {
  પસંદગી (ttypetag(fi)) {
    સ્થિતિ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      જો (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        પાછા ખાલી;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      જો (owner) *owner = obj2gco(f);
      પાછા "";
    }
    સ્થિતિ LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      જો (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        પાછા ખાલી;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      જો (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      પાછા (name == ખાલી) ? "(no name)" : getstr(name);
    }
    મૂળભૂત: પાછા ખાલી;  /* not a closure */
  }
}


LUA_API અચલ અક્ષર *lua_getupvalue (lua_State *L, પૂર્ણાંક funcindex, પૂર્ણાંક n) {
  અચલ અક્ષર *name;
  TValue *val = ખાલી;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ખાલી);
  જો (name) {
    setobj2s(L, L->ટોચ.p, val);
    api_incr_ટોચ(L);
  }
  lua_unlock(L);
  પાછા name;
}


LUA_API અચલ અક્ષર *lua_setupvalue (lua_State *L, પૂર્ણાંક funcindex, પૂર્ણાંક n) {
  અચલ અક્ષર *name;
  TValue *val = ખાલી;  /* to avoid warnings */
  GCObject *owner = ખાલી;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  જો (name) {
    L->ટોચ.p--;
    setobj(L, val, s2v(L->ટોચ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  પાછા name;
}


સ્થિર UpVal **getupvalref (lua_State *L, પૂર્ણાંક fidx, પૂર્ણાંક n, LClosure **pf) {
  સ્થિર અચલ UpVal *અચલ nullup = ખાલી;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  જો (pf) *pf = f;
  જો (1 <= n && n <= f->p->sizeupvalues)
    પાછા &f->upvals[n - 1];  /* get its upvalue pointer */
  નહીંતર
    પાછા (UpVal**)&nullup;
}


LUA_API શૂન્ય *lua_upvalueid (lua_State *L, પૂર્ણાંક fidx, પૂર્ણાંક n) {
  TValue *fi = index2value(L, fidx);
  પસંદગી (ttypetag(fi)) {
    સ્થિતિ LUA_VLCL: {  /* lua closure */
      પાછા *getupvalref(L, fidx, n, ખાલી);
    }
    સ્થિતિ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      જો (1 <= n && n <= f->nupvalues)
        પાછા &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    સ્થિતિ LUA_VLCF:
      પાછા ખાલી;  /* light C functions have no upvalues */
    મૂળભૂત: {
      api_check(L, 0, "function expected");
      પાછા ખાલી;
    }
  }
}


LUA_API શૂન્ય lua_upvaluejoin (lua_State *L, પૂર્ણાંક fidx1, પૂર્ણાંક n1,
                                            પૂર્ણાંક fidx2, પૂર્ણાંક n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, ખાલી);
  api_check(L, *up1 != ખાલી && *up2 != ખાલી, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


