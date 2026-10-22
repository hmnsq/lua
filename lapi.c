/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lapi.h"
#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lfunc.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "ltm.h"
#ଅନ୍ତର୍ଭାଵ "lundump.h"
#ଅନ୍ତର୍ଭାଵ "lvm.h"



ଅଚଳ ଅକ୍ଷର lua_ident[] =
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
ସ୍ଥିର TValue *index2value (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  CallInfo *ci = L->ci;
  ଯଦି (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->ଶୀର୍ଷ.p - (ci->func.p + 1), "unacceptable index");
    ଯଦି (o >= L->ଶୀର୍ଷ.p) ଫେରନ୍ତୁ &G(L)->nilvalue;
    ଅନ୍ୟଥା ଫେରନ୍ତୁ s2v(o);
  }
  ଅନ୍ୟଥା ଯଦି (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->ଶୀର୍ଷ.p - (ci->func.p + 1),
                 "invalid index");
    ଫେରନ୍ତୁ s2v(L->ଶୀର୍ଷ.p + idx);
  }
  ଅନ୍ୟଥା ଯଦି (idx == LUA_REGISTRYINDEX)
    ଫେରନ୍ତୁ &G(L)->l_registry;
  ଅନ୍ୟଥା {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    ଯଦି (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      ଫେରନ୍ତୁ (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    ଅନ୍ୟଥା {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      ଫେରନ୍ତୁ &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
ସ୍ଥିର StkId index2stack (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  CallInfo *ci = L->ci;
  ଯଦି (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->ଶୀର୍ଷ.p, "invalid index");
    ଫେରନ୍ତୁ o;
  }
  ଅନ୍ୟଥା {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->ଶୀର୍ଷ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    ଫେରନ୍ତୁ L->ଶୀର୍ଷ.p + idx;
  }
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_checkstack (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  ଯଦି (L->ଗଦା_last.p - L->ଶୀର୍ଷ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  ଅନ୍ୟଥା  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  ଯଦି (res && ci->ଶୀର୍ଷ.p < L->ଶୀର୍ଷ.p + n)
    ci->ଶୀର୍ଷ.p = L->ଶୀର୍ଷ.p + n;  /* adjust frame top */
  lua_unlock(L);
  ଫେରନ୍ତୁ res;
}


LUA_API ଶୂନ୍ୟ lua_xmove (lua_State *from, lua_State *to, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି (from == to) ଫେରନ୍ତୁ;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->ଶୀର୍ଷ.p - to->ଶୀର୍ଷ.p >= n, "stack overflow");
  from->ଶୀର୍ଷ.p -= n;
  ପାଇଁ (i = 0; i < n; i++) {
    setobjs2s(to, to->ଶୀର୍ଷ.p, from->ଶୀର୍ଷ.p + i);
    to->ଶୀର୍ଷ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  ଫେରନ୍ତୁ old;
}


LUA_API lua_ସଂଖ୍ୟା lua_version (lua_State *L) {
  UNUSED(L);
  ଫେରନ୍ତୁ LUA_VERSION_ଅଙ୍କ;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_absindex (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଫେରନ୍ତୁ (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->ଶୀର୍ଷ.p - L->ci->func.p) + idx;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_gettop (lua_State *L) {
  ଫେରନ୍ତୁ cast_int(L->ଶୀର୍ଷ.p - (L->ci->func.p + 1));
}


LUA_API ଶୂନ୍ୟ lua_settop (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  ଯଦି (idx >= 0) {
    api_check(L, idx <= ci->ଶୀର୍ଷ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->ଶୀର୍ଷ.p;
    ପାଇଁ (; diff > 0; diff--)
      setnilvalue2s(L->ଶୀର୍ଷ.p++);  /* clear new slots */
  }
  ଅନ୍ୟଥା {
    api_check(L, -(idx+1) <= (L->ଶୀର୍ଷ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->ଶୀର୍ଷ.p + diff;
  ଯଦି (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->ଶୀର୍ଷ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_closeslot (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
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
ସ୍ଥିର ଶୂନ୍ୟ reverse (lua_State *L, StkId from, StkId to) {
  ପାଇଁ (; from < to; from++, to--) {
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
LUA_API ଶୂନ୍ୟ lua_rotate (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->ଶୀର୍ଷ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_copy (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fromidx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  ଯଦି (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_pushvalue (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  lua_lock(L);
  setobj2s(L, L->ଶୀର୍ଷ.p, index2value(L, idx));
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_type (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_typename (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  ଫେରନ୍ତୁ ttypename(t);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_iscfunction (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_isinteger (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ ttisinteger(o);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_isnumber (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  lua_ସଂଖ୍ୟା n;
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ tonumber(o, &n);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_isstring (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ (ttisstring(o) || cvt2str(o));
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_isuserdata (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_rawequal (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index2) {
  ଅଚଳ TValue *o1 = index2value(L, index1);
  ଅଚଳ TValue *o2 = index2value(L, index2);
  ଫେରନ୍ତୁ (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API ଶୂନ୍ୟ lua_arith (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op) {
  lua_lock(L);
  ଯଦି (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  ଅନ୍ୟଥା {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->ଶୀର୍ଷ.p, L->ଶୀର୍ଷ.p - 1);
    api_incr_ଶୀର୍ଷ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->ଶୀର୍ଷ.p - 2), s2v(L->ଶୀର୍ଷ.p - 1), L->ଶୀର୍ଷ.p - 2);
  L->ଶୀର୍ଷ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_compare (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op) {
  ଅଚଳ TValue *o1;
  ଅଚଳ TValue *o2;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  ଯଦି (isvalid(L, o1) && isvalid(L, o2)) {
    ବାଛ (op) {
      ସ୍ଥିତି LUA_OPEQ: i = luaV_equalobj(L, o1, o2); ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_OPLT: i = luaV_lessthan(L, o1, o2); ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_OPLE: i = luaV_lessequal(L, o1, o2); ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅକ୍ଷର *buff) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଯଦି (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    ଫେରନ୍ତୁ len;
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ଅଚଳ ଅକ୍ଷର *s) {
  size_t sz = luaO_str2num(s, s2v(L->ଶୀର୍ଷ.p));
  ଯଦି (sz != 0)
    api_incr_ଶୀର୍ଷ(L);
  ଫେରନ୍ତୁ sz;
}


LUA_API lua_ସଂଖ୍ୟା lua_tonumberx (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pisnum) {
  lua_ସଂଖ୍ୟା n = 0;
  ଅଚଳ TValue *o = index2value(L, idx);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isnum = tonumber(o, &n);
  ଯଦି (pisnum)
    *pisnum = isnum;
  ଫେରନ୍ତୁ n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pisnum) {
  lua_Integer res = 0;
  ଅଚଳ TValue *o = index2value(L, idx);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isnum = tointeger(o, &res);
  ଯଦି (pisnum)
    *pisnum = isnum;
  ଫେରନ୍ତୁ res;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_toboolean (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ !l_isfalse(o);
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_tolstring (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  ଯଦି (!ttisstring(o)) {
    ଯଦି (!cvt2str(o)) {  /* not convertible? */
      ଯଦି (len != ଖାଲି) *len = 0;
      lua_unlock(L);
      ଫେରନ୍ତୁ ଖାଲି;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  ଯଦି (len != ଖାଲି)
    ଫେରନ୍ତୁ getlstr(tsvalue(o), *len);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ବାଛ (ttypetag(o)) {
    ସ୍ଥିତି LUA_VSHRSTR: ଫେରନ୍ତୁ cast(lua_Unsigned, tsvalue(o)->shrlen);
    ସ୍ଥିତି LUA_VLNGSTR: ଫେରନ୍ତୁ cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    ସ୍ଥିତି LUA_VUSERDATA: ଫେରନ୍ତୁ cast(lua_Unsigned, uvalue(o)->len);
    ସ୍ଥିତି LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      ଫେରନ୍ତୁ res;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଯଦି (ttislcf(o)) ଫେରନ୍ତୁ fvalue(o);
  ଅନ୍ୟଥା ଯଦି (ttisCclosure(o))
    ଫେରନ୍ତୁ clCvalue(o)->f;
  ଅନ୍ୟଥା ଫେରନ୍ତୁ ଖାଲି;  /* not a C function */
}


l_sinline ଶୂନ୍ୟ *touserdata (ଅଚଳ TValue *o) {
  ବାଛ (ttype(o)) {
    ସ୍ଥିତି LUA_TUSERDATA: ଫେରନ୍ତୁ getudatamem(uvalue(o));
    ସ୍ଥିତି LUA_TLIGHTUSERDATA: ଫେରନ୍ତୁ pvalue(o);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ ଖାଲି;
  }
}


LUA_API ଶୂନ୍ୟ *lua_touserdata (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ଫେରନ୍ତୁ (!ttisthread(o)) ? ଖାଲି : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ଅଚଳ ଶୂନ୍ୟ *lua_topointer (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  ଅଚଳ TValue *o = index2value(L, idx);
  ବାଛ (ttypetag(o)) {
    ସ୍ଥିତି LUA_VLCF: ଫେରନ୍ତୁ cast_voidp(cast_sizet(fvalue(o)));
    ସ୍ଥିତି LUA_VUSERDATA: ସ୍ଥିତି LUA_VLIGHTUSERDATA:
      ଫେରନ୍ତୁ touserdata(o);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ଯଦି (iscollectable(o))
        ଫେରନ୍ତୁ gcvalue(o);
      ଅନ୍ୟଥା
        ଫେରନ୍ତୁ ଖାଲି;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API ଶୂନ୍ୟ lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->ଶୀର୍ଷ.p);
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_pushnumber (lua_State *L, lua_ସଂଖ୍ୟା n) {
  lua_lock(L);
  setfltvalue(s2v(L->ଶୀର୍ଷ.p), n);
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->ଶୀର୍ଷ.p), n);
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ଅଚଳ ଅକ୍ଷର *lua_pushlstring (lua_State *L, ଅଚଳ ଅକ୍ଷର *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->ଶୀର୍ଷ.p, ts);
  api_incr_ଶୀର୍ଷ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ getstr(ts);
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_pushexternalstring (lua_State *L,
	        ଅଚଳ ଅକ୍ଷର *s, size_t len, lua_Alloc falloc, ଶୂନ୍ୟ *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->ଶୀର୍ଷ.p, ts);
  api_incr_ଶୀର୍ଷ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ getstr(ts);
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_pushstring (lua_State *L, ଅଚଳ ଅକ୍ଷର *s) {
  lua_lock(L);
  ଯଦି (s == ଖାଲି)
    setnilvalue2s(L->ଶୀର୍ଷ.p);
  ଅନ୍ୟଥା {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->ଶୀର୍ଷ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_ଶୀର୍ଷ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ s;
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_pushvfstring (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt,
                                      va_list argp) {
  ଅଚଳ ଅକ୍ଷର *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ ret;
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_pushfstring (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt, ...) {
  ଅଚଳ ଅକ୍ଷର *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ ret;
}


LUA_API ଶୂନ୍ୟ lua_pushcclosure (lua_State *L, lua_CFunction fn, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  lua_lock(L);
  ଯଦି (n == 0) {
    setfvalue(s2v(L->ଶୀର୍ଷ.p), fn);
    api_incr_ଶୀର୍ଷ(L);
  }
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    ପାଇଁ (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->ଶୀର୍ଷ.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->ଶୀର୍ଷ.p -= n;
    setclCvalue(L, s2v(L->ଶୀର୍ଷ.p), cl);
    api_incr_ଶୀର୍ଷ(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_pushboolean (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା b) {
  lua_lock(L);
  ଯଦି (b)
    setbtvalue(s2v(L->ଶୀର୍ଷ.p));
  ଅନ୍ୟଥା
    setbfvalue(s2v(L->ଶୀର୍ଷ.p));
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_pushlightuserdata (lua_State *L, ଶୂନ୍ୟ *p) {
  lua_lock(L);
  setpvalue(s2v(L->ଶୀର୍ଷ.p), p);
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->ଶୀର୍ଷ.p), L);
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା auxgetstr (lua_State *L, ଅଚଳ TValue *t, ଅଚଳ ଅକ୍ଷର *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->ଶୀର୍ଷ.p), luaH_getstr, tag);
  ଯଦି (!tagisempty(tag))
    api_incr_ଶୀର୍ଷ(L);
  ଅନ୍ୟଥା {
    setsvalue2s(L, L->ଶୀର୍ଷ.p, str);
    api_incr_ଶୀର୍ଷ(L);
    tag = luaV_finishget(L, t, s2v(L->ଶୀର୍ଷ.p - 1), L->ଶୀର୍ଷ.p - 1, tag);
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
ସ୍ଥିର ଶୂନ୍ୟ getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (ଶୂନ୍ୟ)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_getglobal (lua_State *L, ଅଚଳ ଅକ୍ଷର *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  ଫେରନ୍ତୁ auxgetstr(L, &gt, name);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_gettable (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->ଶୀର୍ଷ.p - 1), s2v(L->ଶୀର୍ଷ.p - 1), luaH_get, tag);
  ଯଦି (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->ଶୀର୍ଷ.p - 1), L->ଶୀର୍ଷ.p - 1, tag);
  lua_unlock(L);
  ଫେରନ୍ତୁ novariant(tag);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_getfield (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅଚଳ ଅକ୍ଷର *k) {
  lua_lock(L);
  ଫେରନ୍ତୁ auxgetstr(L, index2value(L, idx), k);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_geti (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->ଶୀର୍ଷ.p), tag);
  ଯଦି (tagisempty(tag)) {
    TValue ଚାବି;
    setivalue(&ଚାବି, n);
    tag = luaV_finishget(L, t, &ଚାବି, L->ଶୀର୍ଷ.p, tag);
  }
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ novariant(tag);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା finishrawget (lua_State *L, lu_byte tag) {
  ଯଦି (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->ଶୀର୍ଷ.p);
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ novariant(tag);
}


l_sinline Table *gettable (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  ଫେରନ୍ତୁ hvalue(t);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_rawget (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->ଶୀର୍ଷ.p - 1), s2v(L->ଶୀର୍ଷ.p - 1));
  L->ଶୀର୍ଷ.p--;  /* pop key */
  ଫେରନ୍ତୁ finishrawget(L, tag);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_rawgeti (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->ଶୀର୍ଷ.p), tag);
  ଫେରନ୍ତୁ finishrawget(L, tag);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_rawgetp (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅଚଳ ଶୂନ୍ୟ *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  ଫେରନ୍ତୁ finishrawget(L, luaH_get(t, &k, s2v(L->ଶୀର୍ଷ.p)));
}


LUA_API ଶୂନ୍ୟ lua_createtable (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା narray, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->ଶୀର୍ଷ.p, t);
  api_incr_ଶୀର୍ଷ(L);
  ଯଦି (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_getmetatable (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା objindex) {
  ଅଚଳ TValue *obj;
  Table *mt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  ବାଛ (ttype(obj)) {
    ସ୍ଥିତି LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      mt = G(L)->mt[ttype(obj)];
      ଭାଙ୍ଗ;
  }
  ଯଦି (mt != ଖାଲି) {
    sethvalue2s(L, L->ଶୀର୍ଷ.p, mt);
    api_incr_ଶୀର୍ଷ(L);
    res = 1;
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ res;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_getiuservalue (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  TValue *o;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ଯଦି (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->ଶୀର୍ଷ.p);
    t = LUA_TNONE;
  }
  ଅନ୍ୟଥା {
    setobj2s(L, L->ଶୀର୍ଷ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->ଶୀର୍ଷ.p));
  }
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
ସ୍ଥିର ଶୂନ୍ୟ auxsetstr (lua_State *L, ଅଚଳ TValue *t, ଅଚଳ ଅକ୍ଷର *k) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->ଶୀର୍ଷ.p - 1), hres, luaH_psetstr);
  ଯଦି (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->ଶୀର୍ଷ.p - 1));
    L->ଶୀର୍ଷ.p--;  /* pop value */
  }
  ଅନ୍ୟଥା {
    setsvalue2s(L, L->ଶୀର୍ଷ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_ଶୀର୍ଷ(L);
    luaV_finishset(L, t, s2v(L->ଶୀର୍ଷ.p - 1), s2v(L->ଶୀର୍ଷ.p - 2), hres);
    L->ଶୀର୍ଷ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API ଶୂନ୍ୟ lua_setglobal (lua_State *L, ଅଚଳ ଅକ୍ଷର *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API ଶୂନ୍ୟ lua_settable (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  TValue *t;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->ଶୀର୍ଷ.p - 2), s2v(L->ଶୀର୍ଷ.p - 1), hres, luaH_pset);
  ଯଦି (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ଶୀର୍ଷ.p - 1));
  ଅନ୍ୟଥା
    luaV_finishset(L, t, s2v(L->ଶୀର୍ଷ.p - 2), s2v(L->ଶୀର୍ଷ.p - 1), hres);
  L->ଶୀର୍ଷ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_setfield (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅଚଳ ଅକ୍ଷର *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API ଶୂନ୍ୟ lua_seti (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, lua_Integer n) {
  TValue *t;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->ଶୀର୍ଷ.p - 1), hres);
  ଯଦି (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ଶୀର୍ଷ.p - 1));
  ଅନ୍ୟଥା {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->ଶୀର୍ଷ.p - 1), hres);
  }
  L->ଶୀର୍ଷ.p--;  /* pop value */
  lua_unlock(L);
}


ସ୍ଥିର ଶୂନ୍ୟ aux_rawset (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, TValue *ଚାବି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, ଚାବି, s2v(L->ଶୀର୍ଷ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->ଶୀର୍ଷ.p - 1));
  L->ଶୀର୍ଷ.p -= n;
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_rawset (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  aux_rawset(L, idx, s2v(L->ଶୀର୍ଷ.p - 2), 2);
}


LUA_API ଶୂନ୍ୟ lua_rawsetp (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅଚଳ ଶୂନ୍ୟ *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API ଶୂନ୍ୟ lua_rawseti (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->ଶୀର୍ଷ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->ଶୀର୍ଷ.p - 1));
  L->ଶୀର୍ଷ.p--;
  lua_unlock(L);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_setmetatable (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  ଯଦି (ttisnil(s2v(L->ଶୀର୍ଷ.p - 1)))
    mt = ଖାଲି;
  ଅନ୍ୟଥା {
    api_check(L, ttistable(s2v(L->ଶୀର୍ଷ.p - 1)), "table expected");
    mt = hvalue(s2v(L->ଶୀର୍ଷ.p - 1));
  }
  ବାଛ (ttype(obj)) {
    ସ୍ଥିତି LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      ଯଦି (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      ଯଦି (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      G(L)->mt[ttype(obj)] = mt;
      ଭାଙ୍ଗ;
    }
  }
  L->ଶୀର୍ଷ.p--;
  lua_unlock(L);
  ଫେରନ୍ତୁ 1;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_setiuservalue (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  TValue *o;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ଯଦି (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  ଅନ୍ୟଥା {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->ଶୀର୍ଷ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->ଶୀର୍ଷ.p - 1));
    res = 1;
  }
  L->ଶୀର୍ଷ.p--;
  lua_unlock(L);
  ଫେରନ୍ତୁ res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->ଶୀର୍ଷ.p - L->ଶୀର୍ଷ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API ଶୂନ୍ୟ lua_callk (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == ଖାଲି || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->ଶୀର୍ଷ.p - (nargs+1);
  ଯଦି (k != ଖାଲି && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  ଅନ୍ୟଥା  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
ସଂରଚନା CallS {  /* data to 'f_call' */
  StkId func;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nresults;
};


ସ୍ଥିର ଶୂନ୍ୟ f_call (lua_State *L, ଶୂନ୍ୟ *ud) {
  ସଂରଚନା CallS *c = cast(ସଂରଚନା CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_pcallk (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nresults, ପୂର୍ଣ୍ଣସଂଖ୍ୟା errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  ସଂରଚନା CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == ଖାଲି || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  ଯଦି (errfunc == 0)
    func = 0;
  ଅନ୍ୟଥା {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->ଶୀର୍ଷ.p - (nargs+1);  /* function to be called */
  ଯଦି (k == ଖାଲି || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  ଅନ୍ୟଥା {  /* prepare continuation (call is already protected by 'resume') */
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
  ଫେରନ୍ତୁ APIstatus(status);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_load (lua_State *L, lua_Reader reader, ଶୂନ୍ୟ *data,
                      ଅଚଳ ଅକ୍ଷର *chunkname, ଅଚଳ ଅକ୍ଷର *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  ଯଦି (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  ଯଦି (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->ଶୀର୍ଷ.p - 1));  /* get new function */
    ଯଦି (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_dump (lua_State *L, lua_Writer writer, ଶୂନ୍ୟ *data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strip) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  ptrdiff_t otop = savestack(L, L->ଶୀର୍ଷ.p);  /* original top */
  TValue *f = s2v(L->ଶୀର୍ଷ.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->ଶୀର୍ଷ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  ଫେରନ୍ତୁ status;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_status (lua_State *L) {
  ଫେରନ୍ତୁ APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_gc (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା what, ...) {
  va_list argp;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
  global_State *g = G(L);
  ଯଦି (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    ଫେରନ୍ତୁ -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  ବାଛ (what) {
    ସ୍ଥିତି LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      ଯଦି (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      ଅନ୍ୟଥା ଯଦି (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      ଅନ୍ୟଥା  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (ଶୂନ୍ୟ)0, work = 1);
      ଯଦି (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCISRUNNING: {
      res = gcrunning(g);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_GCPARAM: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା param = va_arg(argp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ = va_arg(argp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      ଯଦି (ମୂଲ୍ୟ >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(ମୂଲ୍ୟ));
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  ଫେରନ୍ତୁ res;
}



/*
** miscellaneous functions
*/


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->ଶୀର୍ଷ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  ଯଦି (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  ଅନ୍ୟଥା
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  ଫେରନ୍ତୁ 0;  /* to avoid warnings */
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_next (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  Table *t;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->ଶୀର୍ଷ.p - 1);
  ଯଦି (more)
    api_incr_ଶୀର୍ଷ(L);
  ଅନ୍ୟଥା  /* no more elements */
    L->ଶୀର୍ଷ.p--;  /* pop key */
  lua_unlock(L);
  ଫେରନ୍ତୁ more;
}


LUA_API ଶୂନ୍ୟ lua_toclose (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_concat (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  lua_lock(L);
  ଯଦି (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  ଅନ୍ୟଥା {  /* nothing to concatenate */
    setsvalue2s(L, L->ଶୀର୍ଷ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_ଶୀର୍ଷ(L);
  }
  lua_unlock(L);
}


LUA_API ଶୂନ୍ୟ lua_len (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->ଶୀର୍ଷ.p, t);
  api_incr_ଶୀର୍ଷ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, ଶୂନ୍ୟ **ud) {
  lua_Alloc f;
  lua_lock(L);
  ଯଦି (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  ଫେରନ୍ତୁ f;
}


LUA_API ଶୂନ୍ୟ lua_setallocf (lua_State *L, lua_Alloc f, ଶୂନ୍ୟ *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


ଶୂନ୍ୟ lua_setwarnf (lua_State *L, lua_WarnFunction f, ଶୂନ୍ୟ *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


ଶୂନ୍ୟ lua_warning (lua_State *L, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont) {
  lua_lock(L);
  luaE_warning(L, ସନ୍ଦେଶ, tocont);
  lua_unlock(L);
}



LUA_API ଶୂନ୍ୟ *lua_newuserdatauv (lua_State *L, size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->ଶୀର୍ଷ.p), u);
  api_incr_ଶୀର୍ଷ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ଫେରନ୍ତୁ getudatamem(u);
}



ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *aux_upvalue (TValue *fi, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, TValue **val,
                                GCObject **owner) {
  ବାଛ (ttypetag(fi)) {
    ସ୍ଥିତି LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ଯଦି (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        ଫେରନ୍ତୁ ଖାଲି;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      ଯଦି (owner) *owner = obj2gco(f);
      ଫେରନ୍ତୁ "";
    }
    ସ୍ଥିତି LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      ଯଦି (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        ଫେରନ୍ତୁ ଖାଲି;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      ଯଦି (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      ଫେରନ୍ତୁ (name == ଖାଲି) ? "(no name)" : getstr(name);
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ ଖାଲି;  /* not a closure */
  }
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_getupvalue (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା funcindex, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ଅଚଳ ଅକ୍ଷର *name;
  TValue *val = ଖାଲି;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ଖାଲି);
  ଯଦି (name) {
    setobj2s(L, L->ଶୀର୍ଷ.p, val);
    api_incr_ଶୀର୍ଷ(L);
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ name;
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_setupvalue (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା funcindex, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ଅଚଳ ଅକ୍ଷର *name;
  TValue *val = ଖାଲି;  /* to avoid warnings */
  GCObject *owner = ଖାଲି;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  ଯଦି (name) {
    L->ଶୀର୍ଷ.p--;
    setobj(L, val, s2v(L->ଶୀର୍ଷ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ name;
}


ସ୍ଥିର UpVal **getupvalref (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fidx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, LClosure **pf) {
  ସ୍ଥିର ଅଚଳ UpVal *ଅଚଳ nullup = ଖାଲି;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  ଯଦି (pf) *pf = f;
  ଯଦି (1 <= n && n <= f->p->sizeupvalues)
    ଫେରନ୍ତୁ &f->upvals[n - 1];  /* get its upvalue pointer */
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ (UpVal**)&nullup;
}


LUA_API ଶୂନ୍ୟ *lua_upvalueid (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fidx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  TValue *fi = index2value(L, fidx);
  ବାଛ (ttypetag(fi)) {
    ସ୍ଥିତି LUA_VLCL: {  /* lua closure */
      ଫେରନ୍ତୁ *getupvalref(L, fidx, n, ଖାଲି);
    }
    ସ୍ଥିତି LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ଯଦି (1 <= n && n <= f->nupvalues)
        ଫେରନ୍ତୁ &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    ସ୍ଥିତି LUA_VLCF:
      ଫେରନ୍ତୁ ଖାଲି;  /* light C functions have no upvalues */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      api_check(L, 0, "function expected");
      ଫେରନ୍ତୁ ଖାଲି;
    }
  }
}


LUA_API ଶୂନ୍ୟ lua_upvaluejoin (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fidx1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n1,
                                            ପୂର୍ଣ୍ଣସଂଖ୍ୟା fidx2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, ଖାଲି);
  api_check(L, *up1 != ଖାଲି && *up2 != ଖାଲି, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


