/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lapi_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

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



ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
*/
#ವ್ಯಾಖ್ಯಾನಿಸು isvalid(L, o)	((o) != &G(L)->nilvalue)


/* test for pseudo index */
#ವ್ಯಾಖ್ಯಾನಿಸು ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#ವ್ಯಾಖ್ಯಾನಿಸು isupvalue(i)		((i) < LUA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
ಸ್ಥಿರ TValue *index2value (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  CallInfo *ci = L->ci;
  ಆದರೆ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->ತುದಿ.p - (ci->func.p + 1), "unacceptable index");
    ಆದರೆ (o >= L->ತುದಿ.p) ಹಿಂತಿರುಗಿ &G(L)->nilvalue;
    ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ s2v(o);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->ತುದಿ.p - (ci->func.p + 1),
                 "invalid index");
    ಹಿಂತಿರುಗಿ s2v(L->ತುದಿ.p + idx);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (idx == LUA_REGISTRYINDEX)
    ಹಿಂತಿರುಗಿ &G(L)->l_registry;
  ಇಲ್ಲದಿದ್ದರೆ {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    ಆದರೆ (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      ಹಿಂತಿರುಗಿ (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      ಹಿಂತಿರುಗಿ &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
ಸ್ಥಿರ StkId index2stack (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  CallInfo *ci = L->ci;
  ಆದರೆ (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->ತುದಿ.p, "invalid index");
    ಹಿಂತಿರುಗಿ o;
  }
  ಇಲ್ಲದಿದ್ದರೆ {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->ತುದಿ.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    ಹಿಂತಿರುಗಿ L->ತುದಿ.p + idx;
  }
}


LUA_API ಪೂರ್ಣಾಂಕ lua_checkstack (lua_State *L, ಪೂರ್ಣಾಂಕ n) {
  ಪೂರ್ಣಾಂಕ res;
  CallInfo *ci;
  lua_ಬೀಗ(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  ಆದರೆ (L->ರಾಶಿ_ಕೊನೆಯ.p - L->ತುದಿ.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  ಇಲ್ಲದಿದ್ದರೆ  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  ಆದರೆ (res && ci->ತುದಿ.p < L->ತುದಿ.p + n)
    ci->ತುದಿ.p = L->ತುದಿ.p + n;  /* adjust frame top */
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ res;
}


LUA_API ಶೂನ್ಯ lua_xmove (lua_State *from, lua_State *ಗಮ್ಯ, ಪೂರ್ಣಾಂಕ n) {
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ (from == ಗಮ್ಯ) ಹಿಂತಿರುಗಿ;
  lua_ಬೀಗ(ಗಮ್ಯ);
  api_checkpop(from, n);
  api_check(from, G(from) == G(ಗಮ್ಯ), "moving among independent states");
  api_check(from, ಗಮ್ಯ->ci->ತುದಿ.p - ಗಮ್ಯ->ತುದಿ.p >= n, "stack overflow");
  from->ತುದಿ.p -= n;
  ಗಾಗಿ (i = 0; i < n; i++) {
    setobjs2s(ಗಮ್ಯ, ಗಮ್ಯ->ತುದಿ.p, from->ತುದಿ.p + i);
    ಗಮ್ಯ->ತುದಿ.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(ಗಮ್ಯ);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_ಬೀಗ(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ old;
}


LUA_API lua_ಸಂಖ್ಯೆ lua_ಆವೃತ್ತಿ (lua_State *L) {
  ಬಳಸದ(L);
  ಹಿಂತಿರುಗಿ LUA_ಆವೃತ್ತಿ_ಅಂಕೆ;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API ಪೂರ್ಣಾಂಕ lua_absindex (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಹಿಂತಿರುಗಿ (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->ತುದಿ.p - L->ci->func.p) + idx;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_gettop (lua_State *L) {
  ಹಿಂತಿರುಗಿ cast_int(L->ತುದಿ.p - (L->ci->func.p + 1));
}


LUA_API ಶೂನ್ಯ lua_settop (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_ಬೀಗ(L);
  ci = L->ci;
  func = ci->func.p;
  ಆದರೆ (idx >= 0) {
    api_check(L, idx <= ci->ತುದಿ.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->ತುದಿ.p;
    ಗಾಗಿ (; diff > 0; diff--)
      setnilvalue2s(L->ತುದಿ.p++);  /* clear new slots */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    api_check(L, -(idx+1) <= (L->ತುದಿ.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->ತುದಿ.p + diff;
  ಆದರೆ (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->ತುದಿ.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_closeslot (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  StkId level;
  lua_ಬೀಗ(L);
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
ಸ್ಥಿರ ಶೂನ್ಯ ಹಿಮ್ಮುಖ (lua_State *L, StkId from, StkId ಗಮ್ಯ) {
  ಗಾಗಿ (; from < ಗಮ್ಯ; from++, ಗಮ್ಯ--) {
    TValue ತಾತ್ಕಾಲಿಕ;
    setobj(L, &ತಾತ್ಕಾಲಿಕ, s2v(from));
    setobjs2s(L, from, ಗಮ್ಯ);
    setobj2s(L, ಗಮ್ಯ, &ತಾತ್ಕಾಲಿಕ);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API ಶೂನ್ಯ lua_rotate (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ n) {
  StkId p, t, m;
  lua_ಬೀಗ(L);
  t = L->ತುದಿ.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  ಹಿಮ್ಮುಖ(L, p, m);  /* reverse the prefix with length 'n' */
  ಹಿಮ್ಮುಖ(L, m + 1, t);  /* reverse the suffix */
  ಹಿಮ್ಮುಖ(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_copy (lua_State *L, ಪೂರ್ಣಾಂಕ fromidx, ಪೂರ್ಣಾಂಕ toidx) {
  TValue *fr, *ಗಮ್ಯ;
  lua_ಬೀಗ(L);
  fr = index2value(L, fromidx);
  ಗಮ್ಯ = index2value(L, toidx);
  api_check(L, isvalid(L, ಗಮ್ಯ), "invalid index");
  setobj(L, ಗಮ್ಯ, fr);
  ಆದರೆ (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushvalue (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  lua_ಬೀಗ(L);
  setobj2s(L, L->ತುದಿ.p, index2value(L, idx));
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API ಪೂರ್ಣಾಂಕ lua_type (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_typename (lua_State *L, ಪೂರ್ಣಾಂಕ t) {
  ಬಳಸದ(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  ಹಿಂತಿರುಗಿ ttypename(t);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_iscfunction (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API ಪೂರ್ಣಾಂಕ lua_isinteger (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ ttisinteger(o);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_isnumber (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  lua_ಸಂಖ್ಯೆ n;
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ tonumber(o, &n);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_isstring (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ (ttisstring(o) || cvt2str(o));
}


LUA_API ಪೂರ್ಣಾಂಕ lua_isuserdata (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API ಪೂರ್ಣಾಂಕ lua_rawequal (lua_State *L, ಪೂರ್ಣಾಂಕ index1, ಪೂರ್ಣಾಂಕ index2) {
  ಸ್ಥಿರಾಂಕ TValue *o1 = index2value(L, index1);
  ಸ್ಥಿರಾಂಕ TValue *o2 = index2value(L, index2);
  ಹಿಂತಿರುಗಿ (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API ಶೂನ್ಯ lua_arith (lua_State *L, ಪೂರ್ಣಾಂಕ op) {
  lua_ಬೀಗ(L);
  ಆದರೆ (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  ಇಲ್ಲದಿದ್ದರೆ {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->ತುದಿ.p, L->ತುದಿ.p - 1);
    api_incr_ತುದಿ(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->ತುದಿ.p - 2), s2v(L->ತುದಿ.p - 1), L->ತುದಿ.p - 2);
  L->ತುದಿ.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_compare (lua_State *L, ಪೂರ್ಣಾಂಕ index1, ಪೂರ್ಣಾಂಕ index2, ಪೂರ್ಣಾಂಕ op) {
  ಸ್ಥಿರಾಂಕ TValue *o1;
  ಸ್ಥಿರಾಂಕ TValue *o2;
  ಪೂರ್ಣಾಂಕ i = 0;
  lua_ಬೀಗ(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  ಆದರೆ (isvalid(L, o1) && isvalid(L, o2)) {
    ಆಯ್ಕೆ (op) {
      ಸ್ಥಿತಿ LUA_OPEQ: i = luaV_equalobj(L, o1, o2); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_OPLT: i = luaV_lessthan(L, o1, o2); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_OPLE: i = luaV_lessequal(L, o1, o2); ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ i;
}


LUA_API ಚಿಹ್ನೆರಹಿತ lua_numbertocstring (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಅಕ್ಷರ *buff) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಆದರೆ (ttisnumber(o)) {
    ಚಿಹ್ನೆರಹಿತ len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    ಹಿಂತಿರುಗಿ len;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s) {
  size_t sz = luaO_str2num(s, s2v(L->ತುದಿ.p));
  ಆದರೆ (sz != 0)
    api_incr_ತುದಿ(L);
  ಹಿಂತಿರುಗಿ sz;
}


LUA_API lua_ಸಂಖ್ಯೆ lua_tonumberx (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ *pisnum) {
  lua_ಸಂಖ್ಯೆ n = 0;
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಪೂರ್ಣಾಂಕ isnum = tonumber(o, &n);
  ಆದರೆ (pisnum)
    *pisnum = isnum;
  ಹಿಂತಿರುಗಿ n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ *pisnum) {
  lua_Integer res = 0;
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಪೂರ್ಣಾಂಕ isnum = tointeger(o, &res);
  ಆದರೆ (pisnum)
    *pisnum = isnum;
  ಹಿಂತಿರುಗಿ res;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_toboolean (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ !l_isfalse(o);
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_tolstring (lua_State *L, ಪೂರ್ಣಾಂಕ idx, size_t *len) {
  TValue *o;
  lua_ಬೀಗ(L);
  o = index2value(L, idx);
  ಆದರೆ (!ttisstring(o)) {
    ಆದರೆ (!cvt2str(o)) {  /* not convertible? */
      ಆದರೆ (len != ಖಾಲಿ) *len = 0;
      lua_unlock(L);
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  ಆದರೆ (len != ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ getlstr(tsvalue(o), *len);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಆಯ್ಕೆ (ttypetag(o)) {
    ಸ್ಥಿತಿ LUA_VSHRSTR: ಹಿಂತಿರುಗಿ cast(lua_Unsigned, tsvalue(o)->shrlen);
    ಸ್ಥಿತಿ LUA_VLNGSTR: ಹಿಂತಿರುಗಿ cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    ಸ್ಥಿತಿ LUA_VUSERDATA: ಹಿಂತಿರುಗಿ cast(lua_Unsigned, uvalue(o)->len);
    ಸ್ಥಿತಿ LUA_VTABLE: {
      lua_Unsigned res;
      lua_ಬೀಗ(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      ಹಿಂತಿರುಗಿ res;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಆದರೆ (ttislcf(o)) ಹಿಂತಿರುಗಿ fvalue(o);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ttisCclosure(o))
    ಹಿಂತಿರುಗಿ clCvalue(o)->f;
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* not a C function */
}


l_sinline ಶೂನ್ಯ *touserdata (ಸ್ಥಿರಾಂಕ TValue *o) {
  ಆಯ್ಕೆ (ttype(o)) {
    ಸ್ಥಿತಿ LUA_TUSERDATA: ಹಿಂತಿರುಗಿ getudatamem(uvalue(o));
    ಸ್ಥಿತಿ LUA_TLIGHTUSERDATA: ಹಿಂತಿರುಗಿ pvalue(o);
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
}


LUA_API ಶೂನ್ಯ *lua_touserdata (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಹಿಂತಿರುಗಿ (!ttisthread(o)) ? ಖಾಲಿ : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *lua_topointer (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಸ್ಥಿರಾಂಕ TValue *o = index2value(L, idx);
  ಆಯ್ಕೆ (ttypetag(o)) {
    ಸ್ಥಿತಿ LUA_VLCF: ಹಿಂತಿರುಗಿ cast_voidp(cast_sizet(fvalue(o)));
    ಸ್ಥಿತಿ LUA_VUSERDATA: ಸ್ಥಿತಿ LUA_VLIGHTUSERDATA:
      ಹಿಂತಿರುಗಿ touserdata(o);
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಆದರೆ (iscollectable(o))
        ಹಿಂತಿರುಗಿ gcvalue(o);
      ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API ಶೂನ್ಯ lua_pushnil (lua_State *L) {
  lua_ಬೀಗ(L);
  setnilvalue2s(L->ತುದಿ.p);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushnumber (lua_State *L, lua_ಸಂಖ್ಯೆ n) {
  lua_ಬೀಗ(L);
  setfltvalue(s2v(L->ತುದಿ.p), n);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_ಬೀಗ(L);
  setivalue(s2v(L->ತುದಿ.p), n);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_pushlstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t len) {
  TString *ts;
  lua_ಬೀಗ(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->ತುದಿ.p, ts);
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ getstr(ts);
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_pushexternalstring (lua_State *L,
	        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t len, lua_Alloc falloc, ಶೂನ್ಯ *ud) {
  TString *ts;
  lua_ಬೀಗ(L);
  api_check(L, len <= MAX_ಗಾತ್ರ, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->ತುದಿ.p, ts);
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ getstr(ts);
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_pushstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s) {
  lua_ಬೀಗ(L);
  ಆದರೆ (s == ಖಾಲಿ)
    setnilvalue2s(L->ತುದಿ.p);
  ಇಲ್ಲದಿದ್ದರೆ {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->ತುದಿ.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ s;
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_pushvfstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt,
                                      va_list argp) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret;
  lua_ಬೀಗ(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ ret;
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_pushfstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret;
  va_list argp;
  lua_ಬೀಗ(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ ret;
}


LUA_API ಶೂನ್ಯ lua_pushcclosure (lua_State *L, lua_CFunction fn, ಪೂರ್ಣಾಂಕ n) {
  lua_ಬೀಗ(L);
  ಆದರೆ (n == 0) {
    setfvalue(s2v(L->ತುದಿ.p), fn);
    api_incr_ತುದಿ(L);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    ಗಾಗಿ (i = 0; i < n; i++) {
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


LUA_API ಶೂನ್ಯ lua_pushboolean (lua_State *L, ಪೂರ್ಣಾಂಕ b) {
  lua_ಬೀಗ(L);
  ಆದರೆ (b)
    setbtvalue(s2v(L->ತುದಿ.p));
  ಇಲ್ಲದಿದ್ದರೆ
    setbfvalue(s2v(L->ತುದಿ.p));
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_pushlightuserdata (lua_State *L, ಶೂನ್ಯ *p) {
  lua_ಬೀಗ(L);
  setpvalue(s2v(L->ತುದಿ.p), p);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_pushthread (lua_State *L) {
  lua_ಬೀಗ(L);
  setthvalue(L, s2v(L->ತುದಿ.p), L);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ auxgetstr (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k) {
  lu_byte ಟ್ಯಾಗ್;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->ತುದಿ.p), luaH_getstr, ಟ್ಯಾಗ್);
  ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್))
    api_incr_ತುದಿ(L);
  ಇಲ್ಲದಿದ್ದರೆ {
    setsvalue2s(L, L->ತುದಿ.p, str);
    api_incr_ತುದಿ(L);
    ಟ್ಯಾಗ್ = luaV_finishget(L, t, s2v(L->ತುದಿ.p - 1), L->ತುದಿ.p - 1, ಟ್ಯಾಗ್);
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ novariant(ಟ್ಯಾಗ್);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
ಸ್ಥಿರ ಶೂನ್ಯ get_Global_ಕೋಷ್ಟಕ (lua_State *L, TValue *gt) {
  ಕೋಷ್ಟಕ *registry = hvalue(&G(L)->l_registry);
  lu_byte ಟ್ಯಾಗ್ = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (ಶೂನ್ಯ)ಟ್ಯಾಗ್;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(ಟ್ಯಾಗ್) == LUA_TTABLE, "global table must exist");
}


LUA_API ಪೂರ್ಣಾಂಕ lua_getglobal (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name) {
  TValue gt;
  lua_ಬೀಗ(L);
  get_Global_ಕೋಷ್ಟಕ(L, &gt);
  ಹಿಂತಿರುಗಿ auxgetstr(L, &gt, name);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_gettable (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  lu_byte ಟ್ಯಾಗ್;
  TValue *t;
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->ತುದಿ.p - 1), s2v(L->ತುದಿ.p - 1), luaH_get, ಟ್ಯಾಗ್);
  ಆದರೆ (tagisempty(ಟ್ಯಾಗ್))
    ಟ್ಯಾಗ್ = luaV_finishget(L, t, s2v(L->ತುದಿ.p - 1), L->ತುದಿ.p - 1, ಟ್ಯಾಗ್);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ novariant(ಟ್ಯಾಗ್);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_getfield (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k) {
  lua_ಬೀಗ(L);
  ಹಿಂತಿರುಗಿ auxgetstr(L, index2value(L, idx), k);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_geti (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n) {
  TValue *t;
  lu_byte ಟ್ಯಾಗ್;
  lua_ಬೀಗ(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->ತುದಿ.p), ಟ್ಯಾಗ್);
  ಆದರೆ (tagisempty(ಟ್ಯಾಗ್)) {
    TValue ಕೀಲಿ;
    setivalue(&ಕೀಲಿ, n);
    ಟ್ಯಾಗ್ = luaV_finishget(L, t, &ಕೀಲಿ, L->ತುದಿ.p, ಟ್ಯಾಗ್);
  }
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ novariant(ಟ್ಯಾಗ್);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ finishrawget (lua_State *L, lu_byte ಟ್ಯಾಗ್) {
  ಆದರೆ (tagisempty(ಟ್ಯಾಗ್))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->ತುದಿ.p);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ novariant(ಟ್ಯಾಗ್);
}


l_sinline ಕೋಷ್ಟಕ *gettable (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  ಹಿಂತಿರುಗಿ hvalue(t);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_rawget (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಕೋಷ್ಟಕ *t;
  lu_byte ಟ್ಯಾಗ್;
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  ಟ್ಯಾಗ್ = luaH_get(t, s2v(L->ತುದಿ.p - 1), s2v(L->ತುದಿ.p - 1));
  L->ತುದಿ.p--;  /* pop key */
  ಹಿಂತಿರುಗಿ finishrawget(L, ಟ್ಯಾಗ್);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_rawgeti (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n) {
  ಕೋಷ್ಟಕ *t;
  lu_byte ಟ್ಯಾಗ್;
  lua_ಬೀಗ(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->ತುದಿ.p), ಟ್ಯಾಗ್);
  ಹಿಂತಿರುಗಿ finishrawget(L, ಟ್ಯಾಗ್);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_rawgetp (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p) {
  ಕೋಷ್ಟಕ *t;
  TValue k;
  lua_ಬೀಗ(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  ಹಿಂತಿರುಗಿ finishrawget(L, luaH_get(t, &k, s2v(L->ತುದಿ.p)));
}


LUA_API ಶೂನ್ಯ lua_createtable (lua_State *L, ಪೂರ್ಣಾಂಕ narray, ಪೂರ್ಣಾಂಕ nrec) {
  ಕೋಷ್ಟಕ *t;
  lua_ಬೀಗ(L);
  t = luaH_new(L);
  sethvalue2s(L, L->ತುದಿ.p, t);
  api_incr_ತುದಿ(L);
  ಆದರೆ (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_getmetatable (lua_State *L, ಪೂರ್ಣಾಂಕ objindex) {
  ಸ್ಥಿರಾಂಕ TValue *obj;
  ಕೋಷ್ಟಕ *mt;
  ಪೂರ್ಣಾಂಕ res = 0;
  lua_ಬೀಗ(L);
  obj = index2value(L, objindex);
  ಆಯ್ಕೆ (ttype(obj)) {
    ಸ್ಥಿತಿ LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      mt = G(L)->mt[ttype(obj)];
      ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ (mt != ಖಾಲಿ) {
    sethvalue2s(L, L->ತುದಿ.p, mt);
    api_incr_ತುದಿ(L);
    res = 1;
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ res;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_getiuservalue (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ n) {
  TValue *o;
  ಪೂರ್ಣಾಂಕ t;
  lua_ಬೀಗ(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ಆದರೆ (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->ತುದಿ.p);
    t = LUA_TNONE;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    setobj2s(L, L->ತುದಿ.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->ತುದಿ.p));
  }
  api_incr_ತುದಿ(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
ಸ್ಥಿರ ಶೂನ್ಯ auxsetstr (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k) {
  ಪೂರ್ಣಾಂಕ hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->ತುದಿ.p - 1), hres, luaH_psetstr);
  ಆದರೆ (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->ತುದಿ.p - 1));
    L->ತುದಿ.p--;  /* pop value */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    setsvalue2s(L, L->ತುದಿ.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_ತುದಿ(L);
    luaV_finishset(L, t, s2v(L->ತುದಿ.p - 1), s2v(L->ತುದಿ.p - 2), hres);
    L->ತುದಿ.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API ಶೂನ್ಯ lua_setglobal (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name) {
  TValue gt;
  lua_ಬೀಗ(L);  /* unlock done in 'auxsetstr' */
  get_Global_ಕೋಷ್ಟಕ(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API ಶೂನ್ಯ lua_settable (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  TValue *t;
  ಪೂರ್ಣಾಂಕ hres;
  lua_ಬೀಗ(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->ತುದಿ.p - 2), s2v(L->ತುದಿ.p - 1), hres, luaH_pset);
  ಆದರೆ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ತುದಿ.p - 1));
  ಇಲ್ಲದಿದ್ದರೆ
    luaV_finishset(L, t, s2v(L->ತುದಿ.p - 2), s2v(L->ತುದಿ.p - 1), hres);
  L->ತುದಿ.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_setfield (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k) {
  lua_ಬೀಗ(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API ಶೂನ್ಯ lua_seti (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n) {
  TValue *t;
  ಪೂರ್ಣಾಂಕ hres;
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->ತುದಿ.p - 1), hres);
  ಆದರೆ (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->ತುದಿ.p - 1));
  ಇಲ್ಲದಿದ್ದರೆ {
    TValue ತಾತ್ಕಾಲಿಕ;
    setivalue(&ತಾತ್ಕಾಲಿಕ, n);
    luaV_finishset(L, t, &ತಾತ್ಕಾಲಿಕ, s2v(L->ತುದಿ.p - 1), hres);
  }
  L->ತುದಿ.p--;  /* pop value */
  lua_unlock(L);
}


ಸ್ಥಿರ ಶೂನ್ಯ aux_rawset (lua_State *L, ಪೂರ್ಣಾಂಕ idx, TValue *ಕೀಲಿ, ಪೂರ್ಣಾಂಕ n) {
  ಕೋಷ್ಟಕ *t;
  lua_ಬೀಗ(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  lua_H_ಗಣ(L, t, ಕೀಲಿ, s2v(L->ತುದಿ.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->ತುದಿ.p - 1));
  L->ತುದಿ.p -= n;
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_rawset (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  aux_rawset(L, idx, s2v(L->ತುದಿ.p - 2), 2);
}


LUA_API ಶೂನ್ಯ lua_rawsetp (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API ಶೂನ್ಯ lua_rawseti (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n) {
  ಕೋಷ್ಟಕ *t;
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->ತುದಿ.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->ತುದಿ.p - 1));
  L->ತುದಿ.p--;
  lua_unlock(L);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_setmetatable (lua_State *L, ಪೂರ್ಣಾಂಕ objindex) {
  TValue *obj;
  ಕೋಷ್ಟಕ *mt;
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  ಆದರೆ (ttisnil(s2v(L->ತುದಿ.p - 1)))
    mt = ಖಾಲಿ;
  ಇಲ್ಲದಿದ್ದರೆ {
    api_check(L, ttistable(s2v(L->ತುದಿ.p - 1)), "table expected");
    mt = hvalue(s2v(L->ತುದಿ.p - 1));
  }
  ಆಯ್ಕೆ (ttype(obj)) {
    ಸ್ಥಿತಿ LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      ಆದರೆ (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      ಆದರೆ (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      G(L)->mt[ttype(obj)] = mt;
      ನಿಲ್ಲಿಸು;
    }
  }
  L->ತುದಿ.p--;
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ 1;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_setiuservalue (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ n) {
  TValue *o;
  ಪೂರ್ಣಾಂಕ res;
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  ಆದರೆ (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  ಇಲ್ಲದಿದ್ದರೆ {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->ತುದಿ.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->ತುದಿ.p - 1));
    res = 1;
  }
  L->ತುದಿ.p--;
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#ವ್ಯಾಖ್ಯಾನಿಸು checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->ತುದಿ.p - L->ತುದಿ.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API ಶೂನ್ಯ lua_callk (lua_State *L, ಪೂರ್ಣಾಂಕ nargs, ಪೂರ್ಣಾಂಕ nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_ಬೀಗ(L);
  api_check(L, k == ಖಾಲಿ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->ಸ್ಥಿತಿ_2 == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->ತುದಿ.p - (nargs+1);
  ಆದರೆ (k != ಖಾಲಿ && yieldable(L)) {  /* need to prepare continuation? */
    L->ci->u.c.k = k;  /* save continuation */
    L->ci->u.c.ctx = ctx;  /* save context */
    luaD_call(L, func, nresults);  /* do the call */
  }
  ಇಲ್ಲದಿದ್ದರೆ  /* no continuation or no yieldable */
    luaD_callnoyield(L, func, nresults);  /* just do the call */
  adjustresults(L, nresults);
  lua_unlock(L);
}



/*
** Execute a protected call.
*/
ರಚನೆ CallS {  /* data to 'f_call' */
  StkId func;
  ಪೂರ್ಣಾಂಕ nresults;
};


ಸ್ಥಿರ ಶೂನ್ಯ f_call (lua_State *L, ಶೂನ್ಯ *ud) {
  ರಚನೆ CallS *c = cast(ರಚನೆ CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API ಪೂರ್ಣಾಂಕ lua_pcallk (lua_State *L, ಪೂರ್ಣಾಂಕ nargs, ಪೂರ್ಣಾಂಕ nresults, ಪೂರ್ಣಾಂಕ errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  ರಚನೆ CallS c;
  TStatus ಸ್ಥಿತಿ_2;
  ptrdiff_t func;
  lua_ಬೀಗ(L);
  api_check(L, k == ಖಾಲಿ || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->ಸ್ಥಿತಿ_2 == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  ಆದರೆ (errfunc == 0)
    func = 0;
  ಇಲ್ಲದಿದ್ದರೆ {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->ತುದಿ.p - (nargs+1);  /* function to be called */
  ಆದರೆ (k == ಖಾಲಿ || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    ಸ್ಥಿತಿ_2 = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* prepare continuation (call is already protected by 'resume') */
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
    ಸ್ಥಿತಿ_2 = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ APIstatus(ಸ್ಥಿತಿ_2);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_ತುಂಬು (lua_State *L, lua_Reader reader, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
                      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *chunkname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
  ZIO z;
  TStatus ಸ್ಥಿತಿ_2;
  lua_ಬೀಗ(L);
  luaC_checkGC(L);
  ಆದರೆ (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, ದತ್ತಾಂಶ);
  ಸ್ಥಿತಿ_2 = luaD_protectedparser(L, &z, chunkname, ಬಹುಲಕ);
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->ತುದಿ.p - 1));  /* get new function */
    ಆದರೆ (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      get_Global_ಕೋಷ್ಟಕ(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ APIstatus(ಸ್ಥಿತಿ_2);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API ಪೂರ್ಣಾಂಕ lua_dump (lua_State *L, lua_Writer writer, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ strip) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ptrdiff_t otop = savestack(L, L->ತುದಿ.p);  /* original top */
  TValue *f = s2v(L->ತುದಿ.p - 1);  /* function to be dumped */
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  ಸ್ಥಿತಿ_2 = luaU_dump(L, clLvalue(f)->p, writer, ದತ್ತಾಂಶ, strip);
  L->ತುದಿ.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_ಸ್ಥಿತಿ (lua_State *L) {
  ಹಿಂತಿರುಗಿ APIstatus(L->ಸ್ಥಿತಿ_2);
}


/*
** Garbage-collection function
*/
LUA_API ಪೂರ್ಣಾಂಕ lua_gc (lua_State *L, ಪೂರ್ಣಾಂಕ what, ...) {
  va_list argp;
  ಪೂರ್ಣಾಂಕ res = 0;
  global_State *g = G(L);
  ಆದರೆ (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    ಹಿಂತಿರುಗಿ -1;  /* all options are invalid when stopped */
  lua_ಬೀಗ(L);
  va_start(argp, what);
  ಆಯ್ಕೆ (what) {
    ಸ್ಥಿತಿ LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      ಪೂರ್ಣಾಂಕ work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      ಆದರೆ (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      ಇಲ್ಲದಿದ್ದರೆ  /*ಅತಿಪ್ರವಾಹ*/
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (ಶೂನ್ಯ)0, work = 1);
      ಆದರೆ (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCISRUNNING: {
      res = gcrunning(g);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_GCPARAM: {
      ಪೂರ್ಣಾಂಕ param = va_arg(argp, ಪೂರ್ಣಾಂಕ);
      ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ = va_arg(argp, ಪೂರ್ಣಾಂಕ);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      ಆದರೆ (ಮೌಲ್ಯ >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(ಮೌಲ್ಯ));
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ res;
}



/*
** miscellaneous functions
*/


LUA_API ಪೂರ್ಣಾಂಕ lua_error (lua_State *L) {
  TValue *errobj;
  lua_ಬೀಗ(L);
  errobj = s2v(L->ತುದಿ.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  ಆದರೆ (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  ಇಲ್ಲದಿದ್ದರೆ
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  ಹಿಂತಿರುಗಿ 0;  /* to avoid warnings */
}


LUA_API ಪೂರ್ಣಾಂಕ lua_ಮುಂದಿನ (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  ಕೋಷ್ಟಕ *t;
  ಪೂರ್ಣಾಂಕ ಇನ್ನಷ್ಟು;
  lua_ಬೀಗ(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  ಇನ್ನಷ್ಟು = lua_H_ಮುಂದಿನ(L, t, L->ತುದಿ.p - 1);
  ಆದರೆ (ಇನ್ನಷ್ಟು)
    api_incr_ತುದಿ(L);
  ಇಲ್ಲದಿದ್ದರೆ  /* no more elements */
    L->ತುದಿ.p--;  /* pop key */
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ ಇನ್ನಷ್ಟು;
}


LUA_API ಶೂನ್ಯ lua_toclose (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  StkId o;
  lua_ಬೀಗ(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_concat (lua_State *L, ಪೂರ್ಣಾಂಕ n) {
  lua_ಬೀಗ(L);
  ಆದರೆ (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* nothing to concatenate */
    setsvalue2s(L, L->ತುದಿ.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_ತುದಿ(L);
  }
  lua_unlock(L);
}


LUA_API ಶೂನ್ಯ lua_len (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  TValue *t;
  lua_ಬೀಗ(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->ತುದಿ.p, t);
  api_incr_ತುದಿ(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, ಶೂನ್ಯ **ud) {
  lua_Alloc f;
  lua_ಬೀಗ(L);
  ಆದರೆ (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ f;
}


LUA_API ಶೂನ್ಯ lua_setallocf (lua_State *L, lua_Alloc f, ಶೂನ್ಯ *ud) {
  lua_ಬೀಗ(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


ಶೂನ್ಯ lua_setwarnf (lua_State *L, lua_WarnFunction f, ಶೂನ್ಯ *ud) {
  lua_ಬೀಗ(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


ಶೂನ್ಯ lua_warning (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ tocont) {
  lua_ಬೀಗ(L);
  luaE_warning(L, ಸಂದೇಶ, tocont);
  lua_unlock(L);
}



LUA_API ಶೂನ್ಯ *lua_newuserdatauv (lua_State *L, size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ nuvalue) {
  Udata *u;
  lua_ಬೀಗ(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, ಗಾತ್ರ_2, cast(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ, nuvalue));
  setuvalue(L, s2v(L->ತುದಿ.p), u);
  api_incr_ತುದಿ(L);
  luaC_checkGC(L);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ getudatamem(u);
}



ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *aux_upvalue (TValue *fi, ಪೂರ್ಣಾಂಕ n, TValue **val,
                                GCObject **owner) {
  ಆಯ್ಕೆ (ttypetag(fi)) {
    ಸ್ಥಿತಿ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ಆದರೆ (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      ಆದರೆ (owner) *owner = obj2gco(f);
      ಹಿಂತಿರುಗಿ "";
    }
    ಸ್ಥಿತಿ LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      ಆದರೆ (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      ಆದರೆ (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      ಹಿಂತಿರುಗಿ (name == ಖಾಲಿ) ? "(no name)" : getstr(name);
    }
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* not a closure */
  }
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_getupvalue (lua_State *L, ಪೂರ್ಣಾಂಕ funcindex, ಪೂರ್ಣಾಂಕ n) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  TValue *val = ಖಾಲಿ;  /* to avoid warnings */
  lua_ಬೀಗ(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, ಖಾಲಿ);
  ಆದರೆ (name) {
    setobj2s(L, L->ತುದಿ.p, val);
    api_incr_ತುದಿ(L);
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ name;
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_setupvalue (lua_State *L, ಪೂರ್ಣಾಂಕ funcindex, ಪೂರ್ಣಾಂಕ n) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  TValue *val = ಖಾಲಿ;  /* to avoid warnings */
  GCObject *owner = ಖಾಲಿ;  /* to avoid warnings */
  TValue *fi;
  lua_ಬೀಗ(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  ಆದರೆ (name) {
    L->ತುದಿ.p--;
    setobj(L, val, s2v(L->ತುದಿ.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ name;
}


ಸ್ಥಿರ ಮೇಲೆ_Val **getupvalref (lua_State *L, ಪೂರ್ಣಾಂಕ fidx, ಪೂರ್ಣಾಂಕ n, LClosure **pf) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಮೇಲೆ_Val *ಸ್ಥಿರಾಂಕ nullup = ಖಾಲಿ;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  ಆದರೆ (pf) *pf = f;
  ಆದರೆ (1 <= n && n <= f->p->sizeupvalues)
    ಹಿಂತಿರುಗಿ &f->upvals[n - 1];  /* get its upvalue pointer */
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ (ಮೇಲೆ_Val**)&nullup;
}


LUA_API ಶೂನ್ಯ *lua_upvalueid (lua_State *L, ಪೂರ್ಣಾಂಕ fidx, ಪೂರ್ಣಾಂಕ n) {
  TValue *fi = index2value(L, fidx);
  ಆಯ್ಕೆ (ttypetag(fi)) {
    ಸ್ಥಿತಿ LUA_VLCL: {  /* lua closure */
      ಹಿಂತಿರುಗಿ *getupvalref(L, fidx, n, ಖಾಲಿ);
    }
    ಸ್ಥಿತಿ LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      ಆದರೆ (1 <= n && n <= f->nupvalues)
        ಹಿಂತಿರುಗಿ &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ LUA_VLCF:
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* light C functions have no upvalues */
    ಪೂರ್ವನಿಯೋಜಿತ: {
      api_check(L, 0, "function expected");
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
  }
}


LUA_API ಶೂನ್ಯ lua_upvaluejoin (lua_State *L, ಪೂರ್ಣಾಂಕ fidx1, ಪೂರ್ಣಾಂಕ n1,
                                            ಪೂರ್ಣಾಂಕ fidx2, ಪೂರ್ಣಾಂಕ n2) {
  LClosure *f1;
  ಮೇಲೆ_Val **up1 = getupvalref(L, fidx1, n1, &f1);
  ಮೇಲೆ_Val **up2 = getupvalref(L, fidx2, n2, ಖಾಲಿ);
  api_check(L, *up1 != ಖಾಲಿ && *up2 != ಖಾಲಿ, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


