/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#define lapi_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"
#ಅನ್ತರ್ಭಾವ "lundump.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"



ಧ್ರುವ ವರ್ಣ lua_ident[] =
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
ಸ್ಥಿರ TValue *index2value (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  CallInfo *ci = L->ci;
  ಯದಿ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->ತುದಿ.p - (ci->func.p + 1), "unacceptable index");
    ಯದಿ (o >= L->ತುದಿ.p) ಪ್ರತ್ಯಾಗಮ &G(L)->nilvalue;
    ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ s2v(o);
  }
  ಅನ್ಯಥಾ ಯದಿ (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->ತುದಿ.p - (ci->func.p + 1),
                 "invalid index");
    ಪ್ರತ್ಯಾಗಮ s2v(L->ತುದಿ.p + idx);
  }
  ಅನ್ಯಥಾ ಯದಿ (idx == LUA_REGISTRYINDEX)
    ಪ್ರತ್ಯಾಗಮ &G(L)->l_registry;
  ಅನ್ಯಥಾ {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    ಯದಿ (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      ಪ್ರತ್ಯಾಗಮ (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    ಅನ್ಯಥಾ {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      ಪ್ರತ್ಯಾಗಮ &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
ಸ್ಥಿರ StkId index2stack (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  CallInfo *ci = L->ci;
  ಯದಿ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->ತುದಿ.p, "invalid index");
    ಪ್ರತ್ಯಾಗಮ o;
  }
  ಅನ್ಯಥಾ {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->ತುದಿ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    ಪ್ರತ್ಯಾಗಮ L->ತುದಿ.p + idx;
  }
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_checkstack (lua_State *L, ಪೂರ್ಣಾಙ್ಕ n) {
  ಪೂರ್ಣಾಙ್ಕ res;
  CallInfo *ci;
  lua_lock(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  ಯದಿ (L->ರಾಶಿ_last.p - L->ತುದಿ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  ಅನ್ಯಥಾ  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  ಯದಿ (res && ci->ತುದಿ.p < L->ತುದಿ.p + n)
    ci->ತುದಿ.p = L->ತುದಿ.p + n;  /* adjust frame top */
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ res;
}


LUA_API ಶೂನ್ಯ lua_xmove (lua_State *from, lua_State *to, ಪೂರ್ಣಾಙ್ಕ n) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ (from == to) ಪ್ರತ್ಯಾಗಮ;
  lua_lock(to);
  api_checkpop(from, n);
  api_check(from, G(from) == G(to), "moving among independent states");
  api_check(from, to->ci->ತುದಿ.p - to->ತುದಿ.p >= n, "stack overflow");
  from->ತುದಿ.p -= n;
  ಪ್ರತಿ (i = 0; i < n; i++) {
    setobjs2s(to, to->ತುದಿ.p, from->ತುದಿ.p + i);
    to->ತುದಿ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(to);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_lock(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ old;
}


LUA_API lua_ಸಂಖ್ಯೆ lua_version (lua_State *L) {
  UNUSED(L);
  ಪ್ರತ್ಯಾಗಮ LUA_VERSION_ಅಂಕೆ;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API ಪೂರ್ಣಾಙ್ಕ lua_absindex (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಪ್ರತ್ಯಾಗಮ (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->ತುದಿ.p - L->ci->func.p) + idx;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_gettop (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ cast_int(L->ತುದಿ.p - (L->ci->func.p + 1));
}


LUA_API ಶೂನ್ಯ lua_settop (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_lock(L);
  ci = L->ci;
  func = ci->func.p;
  ಯದಿ (idx >= 0) {
    api_check(L, idx <= ci->ತುದಿ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->ತುದಿ.p;
    ಪ್ರತಿ (; diff > 0; diff--)
      setnilvalue2s(L->ತುದಿ.p++);  /* clear new slots */
  }
  ಅನ್ಯಥಾ {
    api_check(L, -(idx+1) <= (L->ತುದಿ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->ತುದಿ.p + diff;
  ಯದಿ (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->ತುದಿ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_closeslot (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
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
ಸ್ಥಿರ ಶೂನ್ಯ reverse (lua_State *L, StkId from, StkId to) {
  ಪ್ರತಿ (; from < to; from++, to--) {
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
LUA_API ಶೂನ್ಯ lua_rotate (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ n) {
  StkId p, t, m;
  lua_lock(L);
  t = L->ತುದಿ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_copy (lua_State *L, ಪೂರ್ಣಾಙ್ಕ fromidx, ಪೂರ್ಣಾಙ್ಕ toidx) {
  TValue *fr, *to;
  lua_lock(L);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  api_check(L, isvalid(L, to), "invalid index");
  setobj(L, to, fr);
  ಯದಿ (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushvalue (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  lua_lock(L);
  setobj2s(L, L->ತುದಿ.p, index2value(L, idx));
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_type (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_typename (lua_State *L, ಪೂರ್ಣಾಙ್ಕ t) {
  UNUSED(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  ಪ್ರತ್ಯಾಗಮ ttypename(t);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_iscfunction (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_isinteger (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ ttisinteger(o);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_isnumber (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  lua_ಸಂಖ್ಯೆ n;
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ tonumber(o, &n);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_isstring (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ (ttisstring(o) || cvt2str(o));
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_isuserdata (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_rawequal (lua_State *L, ಪೂರ್ಣಾಙ್ಕ index1, ಪೂರ್ಣಾಙ್ಕ index2) {
  ಧ್ರುವ TValue *o1 = index2value(L, index1);
  ಧ್ರುವ TValue *o2 = index2value(L, index2);
  ಪ್ರತ್ಯಾಗಮ (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API ಶೂನ್ಯ lua_arith (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op) {
  lua_lock(L);
  ಯದಿ (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  ಅನ್ಯಥಾ {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->ತುದಿ.p, L->ತುದಿ.p - 1);
    api_incr_ತುದಿ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->ತುದಿ.p - 2), s2v(L->ತುದಿ.p - 1), L->ತುದಿ.p - 2);
  L->ತುದಿ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_compare (lua_State *L, ಪೂರ್ಣಾಙ್ಕ index1, ಪೂರ್ಣಾಙ್ಕ index2, ಪೂರ್ಣಾಙ್ಕ op) {
  ಧ್ರುವ TValue *o1;
  ಧ್ರುವ TValue *o2;
  ಪೂರ್ಣಾಙ್ಕ i = 0;
  lua_lock(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  ಯದಿ (isvalid(L, o1) && isvalid(L, o2)) {
    ವಿಕಲ್ಪ (op) {
      ಪಕ್ಷ LUA_OPEQ: i = luaV_equalobj(L, o1, o2); ಭಙ್ಗ;
      ಪಕ್ಷ LUA_OPLT: i = luaV_lessthan(L, o1, o2); ಭಙ್ಗ;
      ಪಕ್ಷ LUA_OPLE: i = luaV_lessequal(L, o1, o2); ಭಙ್ಗ;
      ಸ್ವಭಾವ: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ i;
}


LUA_API unsigned lua_numbertocstring (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ವರ್ಣ *buff) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಯದಿ (ttisnumber(o)) {
    unsigned len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    ಪ್ರತ್ಯಾಗಮ len;
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ಧ್ರುವ ವರ್ಣ *s) {
  size_t sz = luaO_str2num(s, s2v(L->ತುದಿ.p));
  ಯದಿ (sz != 0)
    api_incr_ತುದಿ(L);
  ಪ್ರತ್ಯಾಗಮ sz;
}


LUA_API lua_ಸಂಖ್ಯೆ lua_tonumberx (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ *pisnum) {
  lua_ಸಂಖ್ಯೆ n = 0;
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪೂರ್ಣಾಙ್ಕ isnum = tonumber(o, &n);
  ಯದಿ (pisnum)
    *pisnum = isnum;
  ಪ್ರತ್ಯಾಗಮ n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ *pisnum) {
  lua_Integer res = 0;
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪೂರ್ಣಾಙ್ಕ isnum = tointeger(o, &res);
  ಯದಿ (pisnum)
    *pisnum = isnum;
  ಪ್ರತ್ಯಾಗಮ res;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_toboolean (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ !l_isfalse(o);
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_tolstring (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, size_t *len) {
  TValue *o;
  lua_lock(L);
  o = index2value(L, idx);
  ಯದಿ (!ttisstring(o)) {
    ಯದಿ (!cvt2str(o)) {  /* not convertible? */
      ಯದಿ (len != ರಿಕ್ತ) *len = 0;
      lua_unlock(L);
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  ಯದಿ (len != ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ getlstr(tsvalue(o), *len);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ವಿಕಲ್ಪ (ttypetag(o)) {
    ಪಕ್ಷ LUA_VSHRSTR: ಪ್ರತ್ಯಾಗಮ cast(lua_Unsigned, tsvalue(o)->shrlen);
    ಪಕ್ಷ LUA_VLNGSTR: ಪ್ರತ್ಯಾಗಮ cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    ಪಕ್ಷ LUA_VUSERDATA: ಪ್ರತ್ಯಾಗಮ cast(lua_Unsigned, uvalue(o)->len);
    ಪಕ್ಷ LUA_VTABLE: {
      lua_Unsigned res;
      lua_lock(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      ಪ್ರತ್ಯಾಗಮ res;
    }
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಯದಿ (ttislcf(o)) ಪ್ರತ್ಯಾಗಮ fvalue(o);
  ಅನ್ಯಥಾ ಯದಿ (ttisCclosure(o))
    ಪ್ರತ್ಯಾಗಮ clCvalue(o)->f;
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* not a C function */
}


l_sinline ಶೂನ್ಯ *touserdata (ಧ್ರುವ TValue *o) {
  ವಿಕಲ್ಪ (ttype(o)) {
    ಪಕ್ಷ LUA_TUSERDATA: ಪ್ರತ್ಯಾಗಮ getudatamem(uvalue(o));
    ಪಕ್ಷ LUA_TLIGHTUSERDATA: ಪ್ರತ್ಯಾಗಮ pvalue(o);
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
}


LUA_API ಶೂನ್ಯ *lua_touserdata (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ಪ್ರತ್ಯಾಗಮ (!ttisthread(o)) ? ರಿಕ್ತ : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ಧ್ರುವ ಶೂನ್ಯ *lua_topointer (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  ಧ್ರುವ TValue *o = index2value(L, idx);
  ವಿಕಲ್ಪ (ttypetag(o)) {
    ಪಕ್ಷ LUA_VLCF: ಪ್ರತ್ಯಾಗಮ cast_voidp(cast_sizet(fvalue(o)));
    ಪಕ್ಷ LUA_VUSERDATA: ಪಕ್ಷ LUA_VLIGHTUSERDATA:
      ಪ್ರತ್ಯಾಗಮ touserdata(o);
    ಸ್ವಭಾವ: {
      ಯದಿ (iscollectable(o))
        ಪ್ರತ್ಯಾಗಮ gcvalue(o);
      ಅನ್ಯಥಾ
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API ಶೂನ್ಯ lua_pushnil (lua_State *L) {
  lua_lock(L);
  setnilvalue2s(L->ತುದಿ.p);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushnumber (lua_State *L, lua_ಸಂಖ್ಯೆ n) {
  lua_lock(L);
  setfltvalue(s2v(L->ತುದಿ.p), n);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_lock(L);
  setivalue(s2v(L->ತುದಿ.p), n);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ಧ್ರುವ ವರ್ಣ *lua_pushlstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *s, size_t len) {
  TString *ts;
  lua_lock(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->ತುದಿ.p, ts);
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ getstr(ts);
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_pushexternalstring (lua_State *L,
	        ಧ್ರುವ ವರ್ಣ *s, size_t len, lua_Alloc falloc, ಶೂನ್ಯ *ud) {
  TString *ts;
  lua_lock(L);
  api_check(L, len <= MAX_SIZE, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->ತುದಿ.p, ts);
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ getstr(ts);
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_pushstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *s) {
  lua_lock(L);
  ಯದಿ (s == ರಿಕ್ತ)
    setnilvalue2s(L->ತುದಿ.p);
  ಅನ್ಯಥಾ {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->ತುದಿ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ s;
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_pushvfstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt,
                                      va_list argp) {
  ಧ್ರುವ ವರ್ಣ *ret;
  lua_lock(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ ret;
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_pushfstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...) {
  ಧ್ರುವ ವರ್ಣ *ret;
  va_list argp;
  lua_lock(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ ret;
}


LUA_API ಶೂನ್ಯ lua_pushcclosure (lua_State *L, lua_CFunction fn, ಪೂರ್ಣಾಙ್ಕ n) {
  lua_lock(L);
  ಯದಿ (n == 0) {
    setfvalue(s2v(L->ತುದಿ.p), fn);
    api_incr_ತುದಿ(L);
  }
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    ಪ್ರತಿ (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->ತುದಿ.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->ತುದಿ.p -= n;
    setclCvalue(L, s2v(L->ತುದಿ.p), cl);
    api_incr_ತುದಿ(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushboolean (lua_State *L, ಪೂರ್ಣಾಙ್ಕ b) {
  lua_lock(L);
  ಯದಿ (b)
    setbtvalue(s2v(L->ತುದಿ.p));
  ಅನ್ಯಥಾ
    setbfvalue(s2v(L->ತುದಿ.p));
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushlightuserdata (lua_State *L, ಶೂನ್ಯ *p) {
  lua_lock(L);
  setpvalue(s2v(L->ತುದಿ.p), p);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_pushthread (lua_State *L) {
  lua_lock(L);
  setthvalue(L, s2v(L->ತುದಿ.p), L);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ auxgetstr (lua_State *L, ಧ್ರುವ TValue *t, ಧ್ರುವ ವರ್ಣ *k) {
  lu_byte tag;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->ತುದಿ.p), luaH_getstr, tag);
  ಯದಿ (!tagisempty(tag))
    api_incr_ತುದಿ(L);
  ಅನ್ಯಥಾ {
    setsvalue2s(L, L->ತುದಿ.p, str);
    api_incr_ತುದಿ(L);
    tag = luaV_finishget(L, t, s2v(L->ತುದಿ.p - 1), L->ತುದಿ.p - 1, tag);
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ novariant(tag);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
ಸ್ಥಿರ ಶೂನ್ಯ getGlobalTable (lua_State *L, TValue *gt) {
  Table *registry = hvalue(&G(L)->l_registry);
  lu_byte tag = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (ಶೂನ್ಯ)tag;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(tag) == LUA_TTABLE, "global table must exist");
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_getglobal (lua_State *L, ಧ್ರುವ ವರ್ಣ *name) {
  TValue gt;
  lua_lock(L);
  getGlobalTable(L, &gt);
  ಪ್ರತ್ಯಾಗಮ auxgetstr(L, &gt, name);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_gettable (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  lu_byte tag;
  TValue *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->ತುದಿ.p - 1), s2v(L->ತುದಿ.p - 1), luaH_get, tag);
  ಯದಿ (tagisempty(tag))
    tag = luaV_finishget(L, t, s2v(L->ತುದಿ.p - 1), L->ತುದಿ.p - 1, tag);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ novariant(tag);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_getfield (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *k) {
  lua_lock(L);
  ಪ್ರತ್ಯಾಗಮ auxgetstr(L, index2value(L, idx), k);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_geti (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n) {
  TValue *t;
  lu_byte tag;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->ತುದಿ.p), tag);
  ಯದಿ (tagisempty(tag)) {
    TValue ಕೀಲಿ;
    setivalue(&ಕೀಲಿ, n);
    tag = luaV_finishget(L, t, &ಕೀಲಿ, L->ತುದಿ.p, tag);
  }
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ novariant(tag);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ finishrawget (lua_State *L, lu_byte tag) {
  ಯದಿ (tagisempty(tag))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->ತುದಿ.p);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ novariant(tag);
}


l_sinline Table *gettable (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  ಪ್ರತ್ಯಾಗಮ hvalue(t);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_rawget (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  tag = luaH_get(t, s2v(L->ತುದಿ.p - 1), s2v(L->ತುದಿ.p - 1));
  L->ತುದಿ.p--;  /* pop key */
  ಪ್ರತ್ಯಾಗಮ finishrawget(L, tag);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_rawgeti (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n) {
  Table *t;
  lu_byte tag;
  lua_lock(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->ತುದಿ.p), tag);
  ಪ್ರತ್ಯಾಗಮ finishrawget(L, tag);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_rawgetp (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ಶೂನ್ಯ *p) {
  Table *t;
  TValue k;
  lua_lock(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  ಪ್ರತ್ಯಾಗಮ finishrawget(L, luaH_get(t, &k, s2v(L->ತುದಿ.p)));
}


LUA_API ಶೂನ್ಯ lua_createtable (lua_State *L, ಪೂರ್ಣಾಙ್ಕ narray, ಪೂರ್ಣಾಙ್ಕ nrec) {
  Table *t;
  lua_lock(L);
  t = luaH_new(L);
  sethvalue2s(L, L->ತುದಿ.p, t);
  api_incr_ತುದಿ(L);
  ಯದಿ (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_getmetatable (lua_State *L, ಪೂರ್ಣಾಙ್ಕ objindex) {
  ಧ್ರುವ TValue *obj;
  Table *mt;
  ಪೂರ್ಣಾಙ್ಕ res = 0;
  lua_lock(L);
  obj = index2value(L, objindex);
  ವಿಕಲ್ಪ (ttype(obj)) {
    ಪಕ್ಷ LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      ಭಙ್ಗ;
    ಪಕ್ಷ LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      mt = G(L)->mt[ttype(obj)];
      ಭಙ್ಗ;
  }
  ಯದಿ (mt != ರಿಕ್ತ) {
    sethvalue2s(L, L->ತುದಿ.p, mt);
    api_incr_ತುದಿ(L);
    res = 1;
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ res;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_getiuservalue (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ n) {
  TValue *o;
  ಪೂರ್ಣಾಙ್ಕ t;
  lua_lock(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ಯದಿ (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->ತುದಿ.p);
    t = LUA_TNONE;
  }
  ಅನ್ಯಥಾ {
    setobj2s(L, L->ತುದಿ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->ತುದಿ.p));
  }
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
ಸ್ಥಿರ ಶೂನ್ಯ auxsetstr (lua_State *L, ಧ್ರುವ TValue *t, ಧ್ರುವ ವರ್ಣ *k) {
  ಪೂರ್ಣಾಙ್ಕ hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->ತುದಿ.p - 1), hres, luaH_psetstr);
  ಯದಿ (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->ತುದಿ.p - 1));
    L->ತುದಿ.p--;  /* pop value */
  }
  ಅನ್ಯಥಾ {
    setsvalue2s(L, L->ತುದಿ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_ತುದಿ(L);
    luaV_finishset(L, t, s2v(L->ತುದಿ.p - 1), s2v(L->ತುದಿ.p - 2), hres);
    L->ತುದಿ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API ಶೂನ್ಯ lua_setglobal (lua_State *L, ಧ್ರುವ ವರ್ಣ *name) {
  TValue gt;
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  getGlobalTable(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API ಶೂನ್ಯ lua_settable (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  TValue *t;
  ಪೂರ್ಣಾಙ್ಕ hres;
  lua_lock(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->ತುದಿ.p - 2), s2v(L->ತುದಿ.p - 1), hres, luaH_pset);
  ಯದಿ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ತುದಿ.p - 1));
  ಅನ್ಯಥಾ
    luaV_finishset(L, t, s2v(L->ತುದಿ.p - 2), s2v(L->ತುದಿ.p - 1), hres);
  L->ತುದಿ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_setfield (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *k) {
  lua_lock(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API ಶೂನ್ಯ lua_seti (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n) {
  TValue *t;
  ಪೂರ್ಣಾಙ್ಕ hres;
  lua_lock(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->ತುದಿ.p - 1), hres);
  ಯದಿ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ತುದಿ.p - 1));
  ಅನ್ಯಥಾ {
    TValue temp;
    setivalue(&temp, n);
    luaV_finishset(L, t, &temp, s2v(L->ತುದಿ.p - 1), hres);
  }
  L->ತುದಿ.p--;  /* pop value */
  lua_unlock(L);
}


ಸ್ಥಿರ ಶೂನ್ಯ aux_rawset (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, TValue *ಕೀಲಿ, ಪೂರ್ಣಾಙ್ಕ n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  luaH_set(L, t, ಕೀಲಿ, s2v(L->ತುದಿ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->ತುದಿ.p - 1));
  L->ತುದಿ.p -= n;
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_rawset (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  aux_rawset(L, idx, s2v(L->ತುದಿ.p - 2), 2);
}


LUA_API ಶೂನ್ಯ lua_rawsetp (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ಶೂನ್ಯ *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API ಶೂನ್ಯ lua_rawseti (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n) {
  Table *t;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->ತುದಿ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->ತುದಿ.p - 1));
  L->ತುದಿ.p--;
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_setmetatable (lua_State *L, ಪೂರ್ಣಾಙ್ಕ objindex) {
  TValue *obj;
  Table *mt;
  lua_lock(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  ಯದಿ (ttisnil(s2v(L->ತುದಿ.p - 1)))
    mt = ರಿಕ್ತ;
  ಅನ್ಯಥಾ {
    api_check(L, ttistable(s2v(L->ತುದಿ.p - 1)), "table expected");
    mt = hvalue(s2v(L->ತುದಿ.p - 1));
  }
  ವಿಕಲ್ಪ (ttype(obj)) {
    ಪಕ್ಷ LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      ಯದಿ (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      ಯದಿ (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      G(L)->mt[ttype(obj)] = mt;
      ಭಙ್ಗ;
    }
  }
  L->ತುದಿ.p--;
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ 1;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_setiuservalue (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ n) {
  TValue *o;
  ಪೂರ್ಣಾಙ್ಕ res;
  lua_lock(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ಯದಿ (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  ಅನ್ಯಥಾ {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->ತುದಿ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->ತುದಿ.p - 1));
    res = 1;
  }
  L->ತುದಿ.p--;
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#define checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->ತುದಿ.p - L->ತುದಿ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API ಶೂನ್ಯ lua_callk (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nargs, ಪೂರ್ಣಾಙ್ಕ nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_lock(L);
  api_check(L, k == ರಿಕ್ತ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->ತುದಿ.p - (nargs+1);
  ಯದಿ (k != ರಿಕ್ತ && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  ಅನ್ಯಥಾ  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
ಸಂರಚನಾ CallS {  /* data to 'f_call' */
  StkId func;
  ಪೂರ್ಣಾಙ್ಕ nresults;
};


ಸ್ಥಿರ ಶೂನ್ಯ f_call (lua_State *L, ಶೂನ್ಯ *ud) {
  ಸಂರಚನಾ CallS *c = cast(ಸಂರಚನಾ CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API ಪೂರ್ಣಾಙ್ಕ lua_pcallk (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nargs, ಪೂರ್ಣಾಙ್ಕ nresults, ಪೂರ್ಣಾಙ್ಕ errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  ಸಂರಚನಾ CallS c;
  TStatus status;
  ptrdiff_t func;
  lua_lock(L);
  api_check(L, k == ರಿಕ್ತ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->status == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  ಯದಿ (errfunc == 0)
    func = 0;
  ಅನ್ಯಥಾ {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->ತುದಿ.p - (nargs+1);  /* function to be called */
  ಯದಿ (k == ರಿಕ್ತ || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  ಅನ್ಯಥಾ {  /* prepare continuation (call is already protected by 'resume') */
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
  ಪ್ರತ್ಯಾಗಮ APIstatus(status);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_load (lua_State *L, lua_Reader reader, ಶೂನ್ಯ *data,
                      ಧ್ರುವ ವರ್ಣ *chunkname, ಧ್ರುವ ವರ್ಣ *mode) {
  ZIO z;
  TStatus status;
  lua_lock(L);
  luaC_checkGC(L);
  ಯದಿ (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  ಯದಿ (status == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->ತುದಿ.p - 1));  /* get new function */
    ಯದಿ (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      getGlobalTable(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ APIstatus(status);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API ಪೂರ್ಣಾಙ್ಕ lua_dump (lua_State *L, lua_Writer writer, ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ strip) {
  ಪೂರ್ಣಾಙ್ಕ status;
  ptrdiff_t otop = savestack(L, L->ತುದಿ.p);  /* original top */
  TValue *f = s2v(L->ತುದಿ.p - 1);  /* function to be dumped */
  lua_lock(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  status = luaU_dump(L, clLvalue(f)->p, writer, data, strip);
  L->ತುದಿ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ status;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_status (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ APIstatus(L->status);
}


/*
** Garbage-collection function
*/
LUA_API ಪೂರ್ಣಾಙ್ಕ lua_gc (lua_State *L, ಪೂರ್ಣಾಙ್ಕ what, ...) {
  va_list argp;
  ಪೂರ್ಣಾಙ್ಕ res = 0;
  global_State *g = G(L);
  ಯದಿ (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    ಪ್ರತ್ಯಾಗಮ -1;  /* all options are invalid when stopped */
  lua_lock(L);
  va_start(argp, what);
  ವಿಕಲ್ಪ (what) {
    ಪಕ್ಷ LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      ಪೂರ್ಣಾಙ್ಕ work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      ಯದಿ (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      ಅನ್ಯಥಾ ಯದಿ (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      ಅನ್ಯಥಾ  /* overflow */
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (ಶೂನ್ಯ)0, work = 1);
      ಯದಿ (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCISRUNNING: {
      res = gcrunning(g);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_GCPARAM: {
      ಪೂರ್ಣಾಙ್ಕ param = va_arg(argp, ಪೂರ್ಣಾಙ್ಕ);
      ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ = va_arg(argp, ಪೂರ್ಣಾಙ್ಕ);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      ಯದಿ (ಮೌಲ್ಯ >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(ಮೌಲ್ಯ));
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ res;
}



/*
** miscellaneous functions
*/


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_error (lua_State *L) {
  TValue *errobj;
  lua_lock(L);
  errobj = s2v(L->ತುದಿ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  ಯದಿ (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  ಅನ್ಯಥಾ
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  ಪ್ರತ್ಯಾಗಮ 0;  /* to avoid warnings */
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_next (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  Table *t;
  ಪೂರ್ಣಾಙ್ಕ more;
  lua_lock(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  more = luaH_next(L, t, L->ತುದಿ.p - 1);
  ಯದಿ (more)
    api_incr_ತುದಿ(L);
  ಅನ್ಯಥಾ  /* no more elements */
    L->ತುದಿ.p--;  /* pop key */
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ more;
}


LUA_API ಶೂನ್ಯ lua_toclose (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  StkId o;
  lua_lock(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_concat (lua_State *L, ಪೂರ್ಣಾಙ್ಕ n) {
  lua_lock(L);
  ಯದಿ (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  ಅನ್ಯಥಾ {  /* nothing to concatenate */
    setsvalue2s(L, L->ತುದಿ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_ತುದಿ(L);
  }
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_len (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  TValue *t;
  lua_lock(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->ತುದಿ.p, t);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, ಶೂನ್ಯ **ud) {
  lua_Alloc f;
  lua_lock(L);
  ಯದಿ (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ f;
}


LUA_API ಶೂನ್ಯ lua_setallocf (lua_State *L, lua_Alloc f, ಶೂನ್ಯ *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


ಶೂನ್ಯ lua_setwarnf (lua_State *L, lua_WarnFunction f, ಶೂನ್ಯ *ud) {
  lua_lock(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


ಶೂನ್ಯ lua_warning (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ, ಪೂರ್ಣಾಙ್ಕ tocont) {
  lua_lock(L);
  luaE_warning(L, ಸಂದೇಶ, tocont);
  lua_unlock(L);
}



LUA_API ಶೂನ್ಯ *lua_newuserdatauv (lua_State *L, size_t size, ಪೂರ್ಣಾಙ್ಕ nuvalue) {
  Udata *u;
  lua_lock(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, size, cast(unsigned short, nuvalue));
  setuvalue(L, s2v(L->ತುದಿ.p), u);
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ getudatamem(u);
}



ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *aux_upvalue (TValue *fi, ಪೂರ್ಣಾಙ್ಕ n, TValue **val,
                                GCObject **owner) {
  ವಿಕಲ್ಪ (ttypetag(fi)) {
    ಪಕ್ಷ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ಯದಿ (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      ಯದಿ (owner) *owner = obj2gco(f);
      ಪ್ರತ್ಯಾಗಮ "";
    }
    ಪಕ್ಷ LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      ಯದಿ (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      ಯದಿ (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      ಪ್ರತ್ಯಾಗಮ (name == ರಿಕ್ತ) ? "(no name)" : getstr(name);
    }
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* not a closure */
  }
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_getupvalue (lua_State *L, ಪೂರ್ಣಾಙ್ಕ funcindex, ಪೂರ್ಣಾಙ್ಕ n) {
  ಧ್ರುವ ವರ್ಣ *name;
  TValue *val = ರಿಕ್ತ;  /* to avoid warnings */
  lua_lock(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ರಿಕ್ತ);
  ಯದಿ (name) {
    setobj2s(L, L->ತುದಿ.p, val);
    api_incr_ತುದಿ(L);
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ name;
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_setupvalue (lua_State *L, ಪೂರ್ಣಾಙ್ಕ funcindex, ಪೂರ್ಣಾಙ್ಕ n) {
  ಧ್ರುವ ವರ್ಣ *name;
  TValue *val = ರಿಕ್ತ;  /* to avoid warnings */
  GCObject *owner = ರಿಕ್ತ;  /* to avoid warnings */
  TValue *fi;
  lua_lock(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  ಯದಿ (name) {
    L->ತುದಿ.p--;
    setobj(L, val, s2v(L->ತುದಿ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ name;
}


ಸ್ಥಿರ UpVal **getupvalref (lua_State *L, ಪೂರ್ಣಾಙ್ಕ fidx, ಪೂರ್ಣಾಙ್ಕ n, LClosure **pf) {
  ಸ್ಥಿರ ಧ್ರುವ UpVal *ಧ್ರುವ nullup = ರಿಕ್ತ;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  ಯದಿ (pf) *pf = f;
  ಯದಿ (1 <= n && n <= f->p->sizeupvalues)
    ಪ್ರತ್ಯಾಗಮ &f->upvals[n - 1];  /* get its upvalue pointer */
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ (UpVal**)&nullup;
}


LUA_API ಶೂನ್ಯ *lua_upvalueid (lua_State *L, ಪೂರ್ಣಾಙ್ಕ fidx, ಪೂರ್ಣಾಙ್ಕ n) {
  TValue *fi = index2value(L, fidx);
  ವಿಕಲ್ಪ (ttypetag(fi)) {
    ಪಕ್ಷ LUA_VLCL: {  /* lua closure */
      ಪ್ರತ್ಯಾಗಮ *getupvalref(L, fidx, n, ರಿಕ್ತ);
    }
    ಪಕ್ಷ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ಯದಿ (1 <= n && n <= f->nupvalues)
        ಪ್ರತ್ಯಾಗಮ &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    ಪಕ್ಷ LUA_VLCF:
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* light C functions have no upvalues */
    ಸ್ವಭಾವ: {
      api_check(L, 0, "function expected");
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
  }
}


LUA_API ಶೂನ್ಯ lua_upvaluejoin (lua_State *L, ಪೂರ್ಣಾಙ್ಕ fidx1, ಪೂರ್ಣಾಙ್ಕ n1,
                                            ಪೂರ್ಣಾಙ್ಕ fidx2, ಪೂರ್ಣಾಙ್ಕ n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, ರಿಕ್ತ);
  api_check(L, *up1 != ರಿಕ್ತ && *up2 != ರಿಕ್ತ, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


