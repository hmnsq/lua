/*
** $Id: lapi.c $
** Lua API
** See Copyright Notice in lua.h
*/

#परिभाषय lapi_c
#परिभाषय LUA_CORE

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



ध्रुव वर्ण lua_ident[] =
  "$LuaVersion: " LUA_COPYRIGHT " $"
  "$LuaAuthors: " LUA_AUTHORS " $";



/*
** Test for a valid index (one that is not the 'nilvalue').
*/
#परिभाषय isvalid(L, o)	((o) != &G(L)->nilvalue)


/* test for pseudo index */
#परिभाषय ispseudo(i)		((i) <= LUA_REGISTRYINDEX)

/* test for upvalue */
#परिभाषय isupvalue(i)		((i) < LUA_REGISTRYINDEX)


/*
** Convert an acceptable index to a pointer to its respective value.
** Non-valid indices return the special nil value 'G(L)->nilvalue'.
*/
स्थिर TValue *index2value (lua_State *L, पूर्णाङ्क idx) {
  CallInfo *ci = L->ci;
  यदि (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, idx <= ci->शिखरम्.p - (ci->func.p + 1), "unacceptable index");
    यदि (o >= L->शिखरम्.p) प्रत्यागम &G(L)->nilvalue;
    अन्यथा प्रत्यागम s2v(o);
  }
  अन्यथा यदि (!ispseudo(idx)) {  /* negative index */
    api_check(L, idx != 0 && -idx <= L->शिखरम्.p - (ci->func.p + 1),
                 "invalid index");
    प्रत्यागम s2v(L->शिखरम्.p + idx);
  }
  अन्यथा यदि (idx == LUA_REGISTRYINDEX)
    प्रत्यागम &G(L)->l_registry;
  अन्यथा {  /* upvalues */
    idx = LUA_REGISTRYINDEX - idx;
    api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
    यदि (ttisCclosure(s2v(ci->func.p))) {  /* C closure? */
      CClosure *func = clCvalue(s2v(ci->func.p));
      प्रत्यागम (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &G(L)->nilvalue;
    }
    अन्यथा {  /* light C function or Lua function (through a hook)?) */
      api_check(L, ttislcf(s2v(ci->func.p)), "caller not a C function");
      प्रत्यागम &G(L)->nilvalue;  /* no upvalues */
    }
  }
}



/*
** Convert a valid actual index (not a pseudo-index) to its address.
*/
स्थिर StkId index2stack (lua_State *L, पूर्णाङ्क idx) {
  CallInfo *ci = L->ci;
  यदि (idx > 0) {
    StkId o = ci->func.p + idx;
    api_check(L, o < L->शिखरम्.p, "invalid index");
    प्रत्यागम o;
  }
  अन्यथा {    /* non-positive index */
    api_check(L, idx != 0 && -idx <= L->शिखरम्.p - (ci->func.p + 1),
                 "invalid index");
    api_check(L, !ispseudo(idx), "invalid index");
    प्रत्यागम L->शिखरम्.p + idx;
  }
}


LUA_API पूर्णाङ्क lua_checkstack (lua_State *L, पूर्णाङ्क n) {
  पूर्णाङ्क res;
  CallInfo *ci;
  lua_तालकम्(L);
  ci = L->ci;
  api_check(L, n >= 0, "negative 'n'");
  यदि (L->राशिः_चरमः.p - L->शिखरम्.p > n)  /* stack large enough? */
    res = 1;  /* yes; check is OK */
  अन्यथा  /* need to grow stack */
    res = luaD_growstack(L, n, 0);
  यदि (res && ci->शिखरम्.p < L->शिखरम्.p + n)
    ci->शिखरम्.p = L->शिखरम्.p + n;  /* adjust frame top */
  lua_unlock(L);
  प्रत्यागम res;
}


LUA_API शून्य lua_xmove (lua_State *from, lua_State *गन्तव्यम्, पूर्णाङ्क n) {
  पूर्णाङ्क i;
  यदि (from == गन्तव्यम्) प्रत्यागम;
  lua_तालकम्(गन्तव्यम्);
  api_checkpop(from, n);
  api_check(from, G(from) == G(गन्तव्यम्), "moving among independent states");
  api_check(from, गन्तव्यम्->ci->शिखरम्.p - गन्तव्यम्->शिखरम्.p >= n, "stack overflow");
  from->शिखरम्.p -= n;
  प्रति (i = 0; i < n; i++) {
    setobjs2s(गन्तव्यम्, गन्तव्यम्->शिखरम्.p, from->शिखरम्.p + i);
    गन्तव्यम्->शिखरम्.p++;  /* stack already checked by previous 'api_check' */
  }
  lua_unlock(गन्तव्यम्);
}


LUA_API lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  lua_तालकम्(L);
  old = G(L)->panic;
  G(L)->panic = panicf;
  lua_unlock(L);
  प्रत्यागम old;
}


LUA_API lua_सङ्ख्या lua_संस्करणम् (lua_State *L) {
  अप्रयुक्तम्(L);
  प्रत्यागम LUA_संस्करणम्_अङ्कः;
}



/*
** basic stack manipulation
*/


/*
** convert an acceptable stack index into an absolute index
*/
LUA_API पूर्णाङ्क lua_absindex (lua_State *L, पूर्णाङ्क idx) {
  प्रत्यागम (idx > 0 || ispseudo(idx))
         ? idx
         : cast_int(L->शिखरम्.p - L->ci->func.p) + idx;
}


LUA_API पूर्णाङ्क lua_gettop (lua_State *L) {
  प्रत्यागम cast_int(L->शिखरम्.p - (L->ci->func.p + 1));
}


LUA_API शून्य lua_settop (lua_State *L, पूर्णाङ्क idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;  /* difference for new top */
  lua_तालकम्(L);
  ci = L->ci;
  func = ci->func.p;
  यदि (idx >= 0) {
    api_check(L, idx <= ci->शिखरम्.p - (func + 1), "new top too large");
    diff = ((func + 1) + idx) - L->शिखरम्.p;
    प्रति (; diff > 0; diff--)
      setnilvalue2s(L->शिखरम्.p++);  /* clear new slots */
  }
  अन्यथा {
    api_check(L, -(idx+1) <= (L->शिखरम्.p - (func + 1)), "invalid new top");
    diff = idx + 1;  /* will "subtract" index (as it is negative) */
  }
  newtop = L->शिखरम्.p + diff;
  यदि (diff < 0 && L->tbclist.p >= newtop) {
    lua_assert(ci->callstatus & CIST_TBC);
    newtop = luaF_close(L, newtop, CLOSEKTOP, 0);
  }
  L->शिखरम्.p = newtop;  /* correct top only after closing any upvalue */
  lua_unlock(L);
}


LUA_API शून्य lua_closeslot (lua_State *L, पूर्णाङ्क idx) {
  StkId level;
  lua_तालकम्(L);
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
स्थिर शून्य reverse (lua_State *L, StkId from, StkId गन्तव्यम्) {
  प्रति (; from < गन्तव्यम्; from++, गन्तव्यम्--) {
    TValue अस्थायी;
    setobj(L, &अस्थायी, s2v(from));
    setobjs2s(L, from, गन्तव्यम्);
    setobj2s(L, गन्तव्यम्, &अस्थायी);
  }
}


/*
** Let x = AB, where A is a prefix of length 'n'. Then,
** rotate x n == BA. But BA == (A^r . B^r)^r.
*/
LUA_API शून्य lua_rotate (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क n) {
  StkId p, t, m;
  lua_तालकम्(L);
  t = L->शिखरम्.p - 1;  /* end of stack segment being rotated */
  p = index2stack(L, idx);  /* start of segment */
  api_check(L, L->tbclist.p < p, "moving a to-be-closed slot");
  api_check(L, (n >= 0 ? n : -n) <= (t - p + 1), "invalid 'n'");
  m = (n >= 0 ? t - n : p - n - 1);  /* end of prefix */
  reverse(L, p, m);  /* reverse the prefix with length 'n' */
  reverse(L, m + 1, t);  /* reverse the suffix */
  reverse(L, p, t);  /* reverse the entire segment */
  lua_unlock(L);
}


LUA_API शून्य lua_copy (lua_State *L, पूर्णाङ्क fromidx, पूर्णाङ्क toidx) {
  TValue *fr, *गन्तव्यम्;
  lua_तालकम्(L);
  fr = index2value(L, fromidx);
  गन्तव्यम् = index2value(L, toidx);
  api_check(L, isvalid(L, गन्तव्यम्), "invalid index");
  setobj(L, गन्तव्यम्, fr);
  यदि (isupvalue(toidx))  /* function upvalue? */
    luaC_barrier(L, clCvalue(s2v(L->ci->func.p)), fr);
  /* LUA_REGISTRYINDEX does not need gc barrier
     (collector revisits it before finishing collection) */
  lua_unlock(L);
}


LUA_API शून्य lua_pushvalue (lua_State *L, पूर्णाङ्क idx) {
  lua_तालकम्(L);
  setobj2s(L, L->शिखरम्.p, index2value(L, idx));
  api_incr_शिखरम्(L);
  lua_unlock(L);
}



/*
** access functions (stack -> C)
*/


LUA_API पूर्णाङ्क lua_type (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम (isvalid(L, o) ? ttype(o) : LUA_TNONE);
}


LUA_API ध्रुव वर्ण *lua_typename (lua_State *L, पूर्णाङ्क t) {
  अप्रयुक्तम्(L);
  api_check(L, LUA_TNONE <= t && t < LUA_NUMTYPES, "invalid type");
  प्रत्यागम ttypename(t);
}


LUA_API पूर्णाङ्क lua_iscfunction (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम (ttislcf(o) || (ttisCclosure(o)));
}


LUA_API पूर्णाङ्क lua_isinteger (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम ttisinteger(o);
}


LUA_API पूर्णाङ्क lua_isnumber (lua_State *L, पूर्णाङ्क idx) {
  lua_सङ्ख्या n;
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम tonumber(o, &n);
}


LUA_API पूर्णाङ्क lua_isstring (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम (ttisstring(o) || cvt2str(o));
}


LUA_API पूर्णाङ्क lua_isuserdata (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम (ttisfulluserdata(o) || ttislightuserdata(o));
}


LUA_API पूर्णाङ्क lua_rawequal (lua_State *L, पूर्णाङ्क index1, पूर्णाङ्क index2) {
  ध्रुव TValue *o1 = index2value(L, index1);
  ध्रुव TValue *o2 = index2value(L, index2);
  प्रत्यागम (isvalid(L, o1) && isvalid(L, o2)) ? luaV_rawequalobj(o1, o2) : 0;
}


LUA_API शून्य lua_arith (lua_State *L, पूर्णाङ्क op) {
  lua_तालकम्(L);
  यदि (op != LUA_OPUNM && op != LUA_OPBNOT)
    api_checkpop(L, 2);  /* all other operations expect two operands */
  अन्यथा {  /* for unary operations, add fake 2nd operand */
    api_checkpop(L, 1);
    setobjs2s(L, L->शिखरम्.p, L->शिखरम्.p - 1);
    api_incr_शिखरम्(L);
  }
  /* first operand at top - 2, second at top - 1; result go to top - 2 */
  luaO_arith(L, op, s2v(L->शिखरम्.p - 2), s2v(L->शिखरम्.p - 1), L->शिखरम्.p - 2);
  L->शिखरम्.p--;  /* pop second operand */
  lua_unlock(L);
}


LUA_API पूर्णाङ्क lua_compare (lua_State *L, पूर्णाङ्क index1, पूर्णाङ्क index2, पूर्णाङ्क op) {
  ध्रुव TValue *o1;
  ध्रुव TValue *o2;
  पूर्णाङ्क i = 0;
  lua_तालकम्(L);  /* may call tag method */
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  यदि (isvalid(L, o1) && isvalid(L, o2)) {
    विकल्प (op) {
      पक्ष LUA_OPEQ: i = luaV_equalobj(L, o1, o2); भङ्ग;
      पक्ष LUA_OPLT: i = luaV_lessthan(L, o1, o2); भङ्ग;
      पक्ष LUA_OPLE: i = luaV_lessequal(L, o1, o2); भङ्ग;
      स्वभाव: api_check(L, 0, "invalid option");
    }
  }
  lua_unlock(L);
  प्रत्यागम i;
}


LUA_API अचिह्नित lua_numbertocstring (lua_State *L, पूर्णाङ्क idx, वर्ण *buff) {
  ध्रुव TValue *o = index2value(L, idx);
  यदि (ttisnumber(o)) {
    अचिह्नित len = luaO_tostringbuff(o, buff);
    buff[len++] = '\0';  /* add final zero */
    प्रत्यागम len;
  }
  अन्यथा
    प्रत्यागम 0;
}


LUA_API size_t lua_stringtonumber (lua_State *L, ध्रुव वर्ण *s) {
  size_t sz = luaO_str2num(s, s2v(L->शिखरम्.p));
  यदि (sz != 0)
    api_incr_शिखरम्(L);
  प्रत्यागम sz;
}


LUA_API lua_सङ्ख्या lua_tonumberx (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क *pisnum) {
  lua_सङ्ख्या n = 0;
  ध्रुव TValue *o = index2value(L, idx);
  पूर्णाङ्क isnum = tonumber(o, &n);
  यदि (pisnum)
    *pisnum = isnum;
  प्रत्यागम n;
}


LUA_API lua_Integer lua_tointegerx (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क *pisnum) {
  lua_Integer res = 0;
  ध्रुव TValue *o = index2value(L, idx);
  पूर्णाङ्क isnum = tointeger(o, &res);
  यदि (pisnum)
    *pisnum = isnum;
  प्रत्यागम res;
}


LUA_API पूर्णाङ्क lua_toboolean (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम !l_isfalse(o);
}


LUA_API ध्रुव वर्ण *lua_tolstring (lua_State *L, पूर्णाङ्क idx, size_t *len) {
  TValue *o;
  lua_तालकम्(L);
  o = index2value(L, idx);
  यदि (!ttisstring(o)) {
    यदि (!cvt2str(o)) {  /* not convertible? */
      यदि (len != रिक्त) *len = 0;
      lua_unlock(L);
      प्रत्यागम रिक्त;
    }
    luaO_tostring(L, o);
    luaC_checkGC(L);
    o = index2value(L, idx);  /* previous call may reallocate the stack */
  }
  lua_unlock(L);
  यदि (len != रिक्त)
    प्रत्यागम getlstr(tsvalue(o), *len);
  अन्यथा
    प्रत्यागम getstr(tsvalue(o));
}


LUA_API lua_Unsigned lua_rawlen (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  विकल्प (ttypetag(o)) {
    पक्ष LUA_VSHRSTR: प्रत्यागम cast(lua_Unsigned, tsvalue(o)->shrlen);
    पक्ष LUA_VLNGSTR: प्रत्यागम cast(lua_Unsigned, tsvalue(o)->u.lnglen);
    पक्ष LUA_VUSERDATA: प्रत्यागम cast(lua_Unsigned, uvalue(o)->len);
    पक्ष LUA_VTABLE: {
      lua_Unsigned res;
      lua_तालकम्(L);
      res = luaH_getn(L, hvalue(o));
      lua_unlock(L);
      प्रत्यागम res;
    }
    स्वभाव: प्रत्यागम 0;
  }
}


LUA_API lua_CFunction lua_tocfunction (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  यदि (ttislcf(o)) प्रत्यागम fvalue(o);
  अन्यथा यदि (ttisCclosure(o))
    प्रत्यागम clCvalue(o)->f;
  अन्यथा प्रत्यागम रिक्त;  /* not a C function */
}


l_sinline शून्य *touserdata (ध्रुव TValue *o) {
  विकल्प (ttype(o)) {
    पक्ष LUA_TUSERDATA: प्रत्यागम getudatamem(uvalue(o));
    पक्ष LUA_TLIGHTUSERDATA: प्रत्यागम pvalue(o);
    स्वभाव: प्रत्यागम रिक्त;
  }
}


LUA_API शून्य *lua_touserdata (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम touserdata(o);
}


LUA_API lua_State *lua_tothread (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  प्रत्यागम (!ttisthread(o)) ? रिक्त : thvalue(o);
}


/*
** Returns a pointer to the internal representation of an object.
** Note that ISO C does not allow the conversion of a pointer to
** function to a 'void*', so the conversion here goes through
** a 'size_t'. (As the returned pointer is only informative, this
** conversion should not be a problem.)
*/
LUA_API ध्रुव शून्य *lua_topointer (lua_State *L, पूर्णाङ्क idx) {
  ध्रुव TValue *o = index2value(L, idx);
  विकल्प (ttypetag(o)) {
    पक्ष LUA_VLCF: प्रत्यागम cast_voidp(cast_sizet(fvalue(o)));
    पक्ष LUA_VUSERDATA: पक्ष LUA_VLIGHTUSERDATA:
      प्रत्यागम touserdata(o);
    स्वभाव: {
      यदि (iscollectable(o))
        प्रत्यागम gcvalue(o);
      अन्यथा
        प्रत्यागम रिक्त;
    }
  }
}



/*
** push functions (C -> stack)
*/


LUA_API शून्य lua_pushnil (lua_State *L) {
  lua_तालकम्(L);
  setnilvalue2s(L->शिखरम्.p);
  api_incr_शिखरम्(L);
  lua_unlock(L);
}


LUA_API शून्य lua_pushnumber (lua_State *L, lua_सङ्ख्या n) {
  lua_तालकम्(L);
  setfltvalue(s2v(L->शिखरम्.p), n);
  api_incr_शिखरम्(L);
  lua_unlock(L);
}


LUA_API शून्य lua_pushinteger (lua_State *L, lua_Integer n) {
  lua_तालकम्(L);
  setivalue(s2v(L->शिखरम्.p), n);
  api_incr_शिखरम्(L);
  lua_unlock(L);
}


/*
** Pushes on the stack a string with given length. Avoid using 's' when
** 'len' == 0 (as 's' can be NULL in that case), due to later use of
** 'memcmp' and 'memcpy'.
*/
LUA_API ध्रुव वर्ण *lua_pushlstring (lua_State *L, ध्रुव वर्ण *s, size_t len) {
  TString *ts;
  lua_तालकम्(L);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  setsvalue2s(L, L->शिखरम्.p, ts);
  api_incr_शिखरम्(L);
  luaC_checkGC(L);
  lua_unlock(L);
  प्रत्यागम getstr(ts);
}


LUA_API ध्रुव वर्ण *lua_pushexternalstring (lua_State *L,
	        ध्रुव वर्ण *s, size_t len, lua_Alloc falloc, शून्य *ud) {
  TString *ts;
  lua_तालकम्(L);
  api_check(L, len <= MAX_आकार, "string too large");
  api_check(L, s[len] == '\0', "string not ending with zero");
  ts = luaS_newextlstr (L, s, len, falloc, ud);
  setsvalue2s(L, L->शिखरम्.p, ts);
  api_incr_शिखरम्(L);
  luaC_checkGC(L);
  lua_unlock(L);
  प्रत्यागम getstr(ts);
}


LUA_API ध्रुव वर्ण *lua_pushstring (lua_State *L, ध्रुव वर्ण *s) {
  lua_तालकम्(L);
  यदि (s == रिक्त)
    setnilvalue2s(L->शिखरम्.p);
  अन्यथा {
    TString *ts;
    ts = luaS_new(L, s);
    setsvalue2s(L, L->शिखरम्.p, ts);
    s = getstr(ts);  /* internal copy's address */
  }
  api_incr_शिखरम्(L);
  luaC_checkGC(L);
  lua_unlock(L);
  प्रत्यागम s;
}


LUA_API ध्रुव वर्ण *lua_pushvfstring (lua_State *L, ध्रुव वर्ण *fmt,
                                      va_list argp) {
  ध्रुव वर्ण *ret;
  lua_तालकम्(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  luaC_checkGC(L);
  lua_unlock(L);
  प्रत्यागम ret;
}


LUA_API ध्रुव वर्ण *lua_pushfstring (lua_State *L, ध्रुव वर्ण *fmt, ...) {
  ध्रुव वर्ण *ret;
  va_list argp;
  lua_तालकम्(L);
  pushvfstring(L, argp, fmt, ret);
  luaC_checkGC(L);
  lua_unlock(L);
  प्रत्यागम ret;
}


LUA_API शून्य lua_pushcclosure (lua_State *L, lua_CFunction fn, पूर्णाङ्क n) {
  lua_तालकम्(L);
  यदि (n == 0) {
    setfvalue(s2v(L->शिखरम्.p), fn);
    api_incr_शिखरम्(L);
  }
  अन्यथा {
    पूर्णाङ्क i;
    CClosure *cl;
    api_checkpop(L, n);
    api_check(L, n <= MAXUPVAL, "upvalue index too large");
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    प्रति (i = 0; i < n; i++) {
      setobj2n(L, &cl->upvalue[i], s2v(L->शिखरम्.p - n + i));
      /* does not need barrier because closure is white */
      lua_assert(iswhite(cl));
    }
    L->शिखरम्.p -= n;
    setclCvalue(L, s2v(L->शिखरम्.p), cl);
    api_incr_शिखरम्(L);
    luaC_checkGC(L);
  }
  lua_unlock(L);
}


LUA_API शून्य lua_pushboolean (lua_State *L, पूर्णाङ्क b) {
  lua_तालकम्(L);
  यदि (b)
    setbtvalue(s2v(L->शिखरम्.p));
  अन्यथा
    setbfvalue(s2v(L->शिखरम्.p));
  api_incr_शिखरम्(L);
  lua_unlock(L);
}


LUA_API शून्य lua_pushlightuserdata (lua_State *L, शून्य *p) {
  lua_तालकम्(L);
  setpvalue(s2v(L->शिखरम्.p), p);
  api_incr_शिखरम्(L);
  lua_unlock(L);
}


LUA_API पूर्णाङ्क lua_pushthread (lua_State *L) {
  lua_तालकम्(L);
  setthvalue(L, s2v(L->शिखरम्.p), L);
  api_incr_शिखरम्(L);
  lua_unlock(L);
  प्रत्यागम (mainthread(G(L)) == L);
}



/*
** get functions (Lua -> stack)
*/


स्थिर पूर्णाङ्क auxgetstr (lua_State *L, ध्रुव TValue *t, ध्रुव वर्ण *k) {
  lu_byte चिह्नकम्;
  TString *str = luaS_new(L, k);
  luaV_fastget(t, str, s2v(L->शिखरम्.p), luaH_getstr, चिह्नकम्);
  यदि (!tagisempty(चिह्नकम्))
    api_incr_शिखरम्(L);
  अन्यथा {
    setsvalue2s(L, L->शिखरम्.p, str);
    api_incr_शिखरम्(L);
    चिह्नकम् = luaV_finishget(L, t, s2v(L->शिखरम्.p - 1), L->शिखरम्.p - 1, चिह्नकम्);
  }
  lua_unlock(L);
  प्रत्यागम novariant(चिह्नकम्);
}


/*
** The following function assumes that the registry cannot be a weak
** table; so, an emergency collection while using the global table
** cannot collect it.
*/
स्थिर शून्य get_Global_सारणी (lua_State *L, TValue *gt) {
  सारणी *registry = hvalue(&G(L)->l_registry);
  lu_byte चिह्नकम् = luaH_getint(registry, LUA_RIDX_GLOBALS, gt);
  (शून्य)चिह्नकम्;  /* avoid not-used warnings when checks are off */
  api_check(L, novariant(चिह्नकम्) == LUA_TTABLE, "global table must exist");
}


LUA_API पूर्णाङ्क lua_getglobal (lua_State *L, ध्रुव वर्ण *name) {
  TValue gt;
  lua_तालकम्(L);
  get_Global_सारणी(L, &gt);
  प्रत्यागम auxgetstr(L, &gt, name);
}


LUA_API पूर्णाङ्क lua_gettable (lua_State *L, पूर्णाङ्क idx) {
  lu_byte चिह्नकम्;
  TValue *t;
  lua_तालकम्(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastget(t, s2v(L->शिखरम्.p - 1), s2v(L->शिखरम्.p - 1), luaH_get, चिह्नकम्);
  यदि (tagisempty(चिह्नकम्))
    चिह्नकम् = luaV_finishget(L, t, s2v(L->शिखरम्.p - 1), L->शिखरम्.p - 1, चिह्नकम्);
  lua_unlock(L);
  प्रत्यागम novariant(चिह्नकम्);
}


LUA_API पूर्णाङ्क lua_getfield (lua_State *L, पूर्णाङ्क idx, ध्रुव वर्ण *k) {
  lua_तालकम्(L);
  प्रत्यागम auxgetstr(L, index2value(L, idx), k);
}


LUA_API पूर्णाङ्क lua_geti (lua_State *L, पूर्णाङ्क idx, lua_Integer n) {
  TValue *t;
  lu_byte चिह्नकम्;
  lua_तालकम्(L);
  t = index2value(L, idx);
  luaV_fastgeti(t, n, s2v(L->शिखरम्.p), चिह्नकम्);
  यदि (tagisempty(चिह्नकम्)) {
    TValue कुञ्चिका;
    setivalue(&कुञ्चिका, n);
    चिह्नकम् = luaV_finishget(L, t, &कुञ्चिका, L->शिखरम्.p, चिह्नकम्);
  }
  api_incr_शिखरम्(L);
  lua_unlock(L);
  प्रत्यागम novariant(चिह्नकम्);
}


स्थिर पूर्णाङ्क finishrawget (lua_State *L, lu_byte चिह्नकम्) {
  यदि (tagisempty(चिह्नकम्))  /* avoid copying empty items to the stack */
    setnilvalue2s(L->शिखरम्.p);
  api_incr_शिखरम्(L);
  lua_unlock(L);
  प्रत्यागम novariant(चिह्नकम्);
}


l_sinline सारणी *gettable (lua_State *L, पूर्णाङ्क idx) {
  TValue *t = index2value(L, idx);
  api_check(L, ttistable(t), "table expected");
  प्रत्यागम hvalue(t);
}


LUA_API पूर्णाङ्क lua_rawget (lua_State *L, पूर्णाङ्क idx) {
  सारणी *t;
  lu_byte चिह्नकम्;
  lua_तालकम्(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  चिह्नकम् = luaH_get(t, s2v(L->शिखरम्.p - 1), s2v(L->शिखरम्.p - 1));
  L->शिखरम्.p--;  /* pop key */
  प्रत्यागम finishrawget(L, चिह्नकम्);
}


LUA_API पूर्णाङ्क lua_rawgeti (lua_State *L, पूर्णाङ्क idx, lua_Integer n) {
  सारणी *t;
  lu_byte चिह्नकम्;
  lua_तालकम्(L);
  t = gettable(L, idx);
  luaH_fastgeti(t, n, s2v(L->शिखरम्.p), चिह्नकम्);
  प्रत्यागम finishrawget(L, चिह्नकम्);
}


LUA_API पूर्णाङ्क lua_rawgetp (lua_State *L, पूर्णाङ्क idx, ध्रुव शून्य *p) {
  सारणी *t;
  TValue k;
  lua_तालकम्(L);
  t = gettable(L, idx);
  setpvalue(&k, cast_voidp(p));
  प्रत्यागम finishrawget(L, luaH_get(t, &k, s2v(L->शिखरम्.p)));
}


LUA_API शून्य lua_createtable (lua_State *L, पूर्णाङ्क narray, पूर्णाङ्क nrec) {
  सारणी *t;
  lua_तालकम्(L);
  t = luaH_new(L);
  sethvalue2s(L, L->शिखरम्.p, t);
  api_incr_शिखरम्(L);
  यदि (narray > 0 || nrec > 0)
    luaH_resize(L, t, cast_uint(narray), cast_uint(nrec));
  luaC_checkGC(L);
  lua_unlock(L);
}


LUA_API पूर्णाङ्क lua_getmetatable (lua_State *L, पूर्णाङ्क objindex) {
  ध्रुव TValue *obj;
  सारणी *mt;
  पूर्णाङ्क res = 0;
  lua_तालकम्(L);
  obj = index2value(L, objindex);
  विकल्प (ttype(obj)) {
    पक्ष LUA_TTABLE:
      mt = hvalue(obj)->metatable;
      भङ्ग;
    पक्ष LUA_TUSERDATA:
      mt = uvalue(obj)->metatable;
      भङ्ग;
    स्वभाव:
      mt = G(L)->mt[ttype(obj)];
      भङ्ग;
  }
  यदि (mt != रिक्त) {
    sethvalue2s(L, L->शिखरम्.p, mt);
    api_incr_शिखरम्(L);
    res = 1;
  }
  lua_unlock(L);
  प्रत्यागम res;
}


LUA_API पूर्णाङ्क lua_getiuservalue (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क n) {
  TValue *o;
  पूर्णाङ्क t;
  lua_तालकम्(L);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  यदि (n <= 0 || n > uvalue(o)->nuvalue) {
    setnilvalue2s(L->शिखरम्.p);
    t = LUA_TNONE;
  }
  अन्यथा {
    setobj2s(L, L->शिखरम्.p, &uvalue(o)->uv[n - 1].uv);
    t = ttype(s2v(L->शिखरम्.p));
  }
  api_incr_शिखरम्(L);
  lua_unlock(L);
  प्रत्यागम t;
}


/*
** set functions (stack -> Lua)
*/

/*
** t[k] = value at the top of the stack (where 'k' is a string)
*/
स्थिर शून्य auxsetstr (lua_State *L, ध्रुव TValue *t, ध्रुव वर्ण *k) {
  पूर्णाङ्क hres;
  TString *str = luaS_new(L, k);
  api_checkpop(L, 1);
  luaV_fastset(t, str, s2v(L->शिखरम्.p - 1), hres, luaH_psetstr);
  यदि (hres == HOK) {
    luaV_finishfastset(L, t, s2v(L->शिखरम्.p - 1));
    L->शिखरम्.p--;  /* pop value */
  }
  अन्यथा {
    setsvalue2s(L, L->शिखरम्.p, str);  /* push 'str' (to make it a TValue) */
    api_incr_शिखरम्(L);
    luaV_finishset(L, t, s2v(L->शिखरम्.p - 1), s2v(L->शिखरम्.p - 2), hres);
    L->शिखरम्.p -= 2;  /* pop value and key */
  }
  lua_unlock(L);  /* lock done by caller */
}


LUA_API शून्य lua_setglobal (lua_State *L, ध्रुव वर्ण *name) {
  TValue gt;
  lua_तालकम्(L);  /* unlock done in 'auxsetstr' */
  get_Global_सारणी(L, &gt);
  auxsetstr(L, &gt, name);
}


LUA_API शून्य lua_settable (lua_State *L, पूर्णाङ्क idx) {
  TValue *t;
  पूर्णाङ्क hres;
  lua_तालकम्(L);
  api_checkpop(L, 2);
  t = index2value(L, idx);
  luaV_fastset(t, s2v(L->शिखरम्.p - 2), s2v(L->शिखरम्.p - 1), hres, luaH_pset);
  यदि (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->शिखरम्.p - 1));
  अन्यथा
    luaV_finishset(L, t, s2v(L->शिखरम्.p - 2), s2v(L->शिखरम्.p - 1), hres);
  L->शिखरम्.p -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API शून्य lua_setfield (lua_State *L, पूर्णाङ्क idx, ध्रुव वर्ण *k) {
  lua_तालकम्(L);  /* unlock done in 'auxsetstr' */
  auxsetstr(L, index2value(L, idx), k);
}


LUA_API शून्य lua_seti (lua_State *L, पूर्णाङ्क idx, lua_Integer n) {
  TValue *t;
  पूर्णाङ्क hres;
  lua_तालकम्(L);
  api_checkpop(L, 1);
  t = index2value(L, idx);
  luaV_fastseti(t, n, s2v(L->शिखरम्.p - 1), hres);
  यदि (hres == HOK)
    luaV_finishfastset(L, t, s2v(L->शिखरम्.p - 1));
  अन्यथा {
    TValue अस्थायी;
    setivalue(&अस्थायी, n);
    luaV_finishset(L, t, &अस्थायी, s2v(L->शिखरम्.p - 1), hres);
  }
  L->शिखरम्.p--;  /* pop value */
  lua_unlock(L);
}


स्थिर शून्य aux_rawset (lua_State *L, पूर्णाङ्क idx, TValue *कुञ्चिका, पूर्णाङ्क n) {
  सारणी *t;
  lua_तालकम्(L);
  api_checkpop(L, n);
  t = gettable(L, idx);
  lua_H_समुच्चयः(L, t, कुञ्चिका, s2v(L->शिखरम्.p - 1));
  invalidateTMcache(t);
  luaC_barrierback(L, obj2gco(t), s2v(L->शिखरम्.p - 1));
  L->शिखरम्.p -= n;
  lua_unlock(L);
}


LUA_API शून्य lua_rawset (lua_State *L, पूर्णाङ्क idx) {
  aux_rawset(L, idx, s2v(L->शिखरम्.p - 2), 2);
}


LUA_API शून्य lua_rawsetp (lua_State *L, पूर्णाङ्क idx, ध्रुव शून्य *p) {
  TValue k;
  setpvalue(&k, cast_voidp(p));
  aux_rawset(L, idx, &k, 1);
}


LUA_API शून्य lua_rawseti (lua_State *L, पूर्णाङ्क idx, lua_Integer n) {
  सारणी *t;
  lua_तालकम्(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  luaH_setint(L, t, n, s2v(L->शिखरम्.p - 1));
  luaC_barrierback(L, obj2gco(t), s2v(L->शिखरम्.p - 1));
  L->शिखरम्.p--;
  lua_unlock(L);
}


LUA_API पूर्णाङ्क lua_setmetatable (lua_State *L, पूर्णाङ्क objindex) {
  TValue *obj;
  सारणी *mt;
  lua_तालकम्(L);
  api_checkpop(L, 1);
  obj = index2value(L, objindex);
  यदि (ttisnil(s2v(L->शिखरम्.p - 1)))
    mt = रिक्त;
  अन्यथा {
    api_check(L, ttistable(s2v(L->शिखरम्.p - 1)), "table expected");
    mt = hvalue(s2v(L->शिखरम्.p - 1));
  }
  विकल्प (ttype(obj)) {
    पक्ष LUA_TTABLE: {
      hvalue(obj)->metatable = mt;
      यदि (mt) {
        luaC_objbarrier(L, gcvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      भङ्ग;
    }
    पक्ष LUA_TUSERDATA: {
      uvalue(obj)->metatable = mt;
      यदि (mt) {
        luaC_objbarrier(L, uvalue(obj), mt);
        luaC_checkfinalizer(L, gcvalue(obj), mt);
      }
      भङ्ग;
    }
    स्वभाव: {
      G(L)->mt[ttype(obj)] = mt;
      भङ्ग;
    }
  }
  L->शिखरम्.p--;
  lua_unlock(L);
  प्रत्यागम 1;
}


LUA_API पूर्णाङ्क lua_setiuservalue (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क n) {
  TValue *o;
  पूर्णाङ्क res;
  lua_तालकम्(L);
  api_checkpop(L, 1);
  o = index2value(L, idx);
  api_check(L, ttisfulluserdata(o), "full userdata expected");
  यदि (!(cast_uint(n) - 1u < cast_uint(uvalue(o)->nuvalue)))
    res = 0;  /* 'n' not in [1, uvalue(o)->nuvalue] */
  अन्यथा {
    setobj(L, &uvalue(o)->uv[n - 1].uv, s2v(L->शिखरम्.p - 1));
    luaC_barrierback(L, gcvalue(o), s2v(L->शिखरम्.p - 1));
    res = 1;
  }
  L->शिखरम्.p--;
  lua_unlock(L);
  प्रत्यागम res;
}


/*
** 'load' and 'call' functions (run Lua code)
*/


#परिभाषय checkresults(L,na,nr) \
     (api_check(L, (nr) == LUA_MULTRET \
               || (L->ci->शिखरम्.p - L->शिखरम्.p >= (nr) - (na)), \
	"results from function overflow current stack size"), \
      api_check(L, LUA_MULTRET <= (nr) && (nr) <= MAXRESULTS,  \
                   "invalid number of results"))


LUA_API शून्य lua_callk (lua_State *L, पूर्णाङ्क nargs, पूर्णाङ्क nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  lua_तालकम्(L);
  api_check(L, k == रिक्त || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->स्थितिः == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  func = L->शिखरम्.p - (nargs+1);
  यदि (k != रिक्त && yieldable(L)) {  /* need to prepare continuation? */
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
  पूर्णाङ्क nresults;
};


स्थिर शून्य f_call (lua_State *L, शून्य *ud) {
  संरचना CallS *c = cast(संरचना CallS *, ud);
  luaD_callnoyield(L, c->func, c->nresults);
}



LUA_API पूर्णाङ्क lua_pcallk (lua_State *L, पूर्णाङ्क nargs, पूर्णाङ्क nresults, पूर्णाङ्क errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  संरचना CallS c;
  TStatus स्थितिः;
  ptrdiff_t func;
  lua_तालकम्(L);
  api_check(L, k == रिक्त || !isLua(L->ci),
    "cannot use continuations inside hooks");
  api_checkpop(L, nargs + 1);
  api_check(L, L->स्थितिः == LUA_OK, "cannot do calls on non-normal thread");
  checkresults(L, nargs, nresults);
  यदि (errfunc == 0)
    func = 0;
  अन्यथा {
    StkId o = index2stack(L, errfunc);
    api_check(L, ttisfunction(s2v(o)), "error handler must be a function");
    func = savestack(L, o);
  }
  c.func = L->शिखरम्.p - (nargs+1);  /* function to be called */
  यदि (k == रिक्त || !yieldable(L)) {  /* no continuation or no yieldable? */
    c.nresults = nresults;  /* do a 'conventional' protected call */
    स्थितिः = luaD_pcall(L, f_call, &c, savestack(L, c.func), func);
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
    स्थितिः = LUA_OK;  /* if it is here, there were no errors */
  }
  adjustresults(L, nresults);
  lua_unlock(L);
  प्रत्यागम APIstatus(स्थितिः);
}


LUA_API पूर्णाङ्क lua_आरोपय (lua_State *L, lua_Reader reader, शून्य *दत्त,
                      ध्रुव वर्ण *chunkname, ध्रुव वर्ण *बहुलकम्) {
  ZIO z;
  TStatus स्थितिः;
  lua_तालकम्(L);
  luaC_checkGC(L);
  यदि (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, दत्त);
  स्थितिः = luaD_protectedparser(L, &z, chunkname, बहुलकम्);
  यदि (स्थितिः == LUA_OK) {  /* no errors? */
    LClosure *f = clLvalue(s2v(L->शिखरम्.p - 1));  /* get new function */
    यदि (f->nupvalues >= 1) {  /* does it have an upvalue? */
      /* get global table from registry */
      TValue gt;
      get_Global_सारणी(L, &gt);
      /* set global table as 1st upvalue of 'f' (may be LUA_ENV) */
      setobj(L, f->upvals[0]->v.p, &gt);
      luaC_barrier(L, f->upvals[0], &gt);
    }
  }
  lua_unlock(L);
  प्रत्यागम APIstatus(स्थितिः);
}


/*
** Dump a Lua function, calling 'writer' to write its parts. Ensure
** the stack returns with its original size.
*/
LUA_API पूर्णाङ्क lua_dump (lua_State *L, lua_Writer writer, शून्य *दत्त, पूर्णाङ्क strip) {
  पूर्णाङ्क स्थितिः;
  ptrdiff_t otop = savestack(L, L->शिखरम्.p);  /* original top */
  TValue *f = s2v(L->शिखरम्.p - 1);  /* function to be dumped */
  lua_तालकम्(L);
  api_checkpop(L, 1);
  api_check(L, isLfunction(f), "Lua function expected");
  स्थितिः = luaU_dump(L, clLvalue(f)->p, writer, दत्त, strip);
  L->शिखरम्.p = restorestack(L, otop);  /* restore top */
  lua_unlock(L);
  प्रत्यागम स्थितिः;
}


LUA_API पूर्णाङ्क lua_स्थितिः (lua_State *L) {
  प्रत्यागम APIstatus(L->स्थितिः);
}


/*
** Garbage-collection function
*/
LUA_API पूर्णाङ्क lua_gc (lua_State *L, पूर्णाङ्क what, ...) {
  va_list argp;
  पूर्णाङ्क res = 0;
  global_State *g = G(L);
  यदि (g->gcstp & (GCSTPGC | GCSTPCLS))  /* internal stop? */
    प्रत्यागम -1;  /* all options are invalid when stopped */
  lua_तालकम्(L);
  va_start(argp, what);
  विकल्प (what) {
    पक्ष LUA_GCSTOP: {
      g->gcstp = GCSTPUSR;  /* stopped by the user */
      भङ्ग;
    }
    पक्ष LUA_GCRESTART: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;  /* (other bits must be zero here) */
      भङ्ग;
    }
    पक्ष LUA_GCCOLLECT: {
      luaC_fullgc(L, 0);
      भङ्ग;
    }
    पक्ष LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(gettotalbytes(g) >> 10);
      भङ्ग;
    }
    पक्ष LUA_GCCOUNTB: {
      res = cast_int(gettotalbytes(g) & 0x3ff);
      भङ्ग;
    }
    पक्ष LUA_GCSTEP: {
      lu_byte oldstp = g->gcstp;
      l_mem n = cast(l_mem, va_arg(argp, size_t));
      l_mem newdebt;
      पूर्णाङ्क work = 0;  /* true if GC did some work */
      g->gcstp = 0;  /* allow GC to run (other bits must be zero here) */
      यदि (n <= 0)
        newdebt = 0;  /* force to run one basic step */
      अन्यथा यदि (g->GCdebt >= n - MAX_LMEM)  /* no overflow? */
        newdebt = g->GCdebt - n;
      अन्यथा  /*अतिप्रवाहः*/
        newdebt = -MAX_LMEM;  /* set debt to mininum value */
      luaE_setdebt(g, newdebt);
      luaC_condGC(L, (शून्य)0, work = 1);
      यदि (work && g->gcstate == GCSpause)  /* end of cycle? */
        res = 1;  /* signal it */
      g->gcstp = oldstp;  /* restore previous state */
      भङ्ग;
    }
    पक्ष LUA_GCISRUNNING: {
      res = gcrunning(g);
      भङ्ग;
    }
    पक्ष LUA_GCGEN: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_GENMINOR);
      भङ्ग;
    }
    पक्ष LUA_GCINC: {
      res = (g->gckind == KGC_INC) ? LUA_GCINC : LUA_GCGEN;
      luaC_changemode(L, KGC_INC);
      भङ्ग;
    }
    पक्ष LUA_GCPARAM: {
      पूर्णाङ्क param = va_arg(argp, पूर्णाङ्क);
      पूर्णाङ्क मानम् = va_arg(argp, पूर्णाङ्क);
      api_check(L, 0 <= param && param < LUA_GCPN, "invalid parameter");
      res = cast_int(luaO_applyparam(g->gcparams[param], 100));
      यदि (मानम् >= 0)
        g->gcparams[param] = luaO_codeparam(cast_uint(मानम्));
      भङ्ग;
    }
    स्वभाव: res = -1;  /* invalid option */
  }
  va_end(argp);
  lua_unlock(L);
  प्रत्यागम res;
}



/*
** miscellaneous functions
*/


LUA_API पूर्णाङ्क lua_error (lua_State *L) {
  TValue *errobj;
  lua_तालकम्(L);
  errobj = s2v(L->शिखरम्.p - 1);
  api_checkpop(L, 1);
  /* error object is the memory error message? */
  यदि (ttisshrstring(errobj) && eqshrstr(tsvalue(errobj), G(L)->memerrmsg))
    luaM_error(L);  /* raise a memory error */
  अन्यथा
    luaG_errormsg(L);  /* raise a regular error */
  /* code unreachable; will unlock when control actually leaves the kernel */
  प्रत्यागम 0;  /* to avoid warnings */
}


LUA_API पूर्णाङ्क lua_अग्र (lua_State *L, पूर्णाङ्क idx) {
  सारणी *t;
  पूर्णाङ्क अधिकम्;
  lua_तालकम्(L);
  api_checkpop(L, 1);
  t = gettable(L, idx);
  अधिकम् = lua_H_अग्र(L, t, L->शिखरम्.p - 1);
  यदि (अधिकम्)
    api_incr_शिखरम्(L);
  अन्यथा  /* no more elements */
    L->शिखरम्.p--;  /* pop key */
  lua_unlock(L);
  प्रत्यागम अधिकम्;
}


LUA_API शून्य lua_toclose (lua_State *L, पूर्णाङ्क idx) {
  StkId o;
  lua_तालकम्(L);
  o = index2stack(L, idx);
  api_check(L, L->tbclist.p < o, "given index below or equal a marked one");
  luaF_newtbcupval(L, o);  /* create new to-be-closed upvalue */
  L->ci->callstatus |= CIST_TBC;  /* mark that function has TBC slots */
  lua_unlock(L);
}


LUA_API शून्य lua_concat (lua_State *L, पूर्णाङ्क n) {
  lua_तालकम्(L);
  यदि (n > 0) {
    api_checkpop(L, n);
    luaV_concat(L, n);
    luaC_checkGC(L);
  }
  अन्यथा {  /* nothing to concatenate */
    setsvalue2s(L, L->शिखरम्.p, luaS_newlstr(L, "", 0));  /* push empty string */
    api_incr_शिखरम्(L);
  }
  lua_unlock(L);
}


LUA_API शून्य lua_len (lua_State *L, पूर्णाङ्क idx) {
  TValue *t;
  lua_तालकम्(L);
  t = index2value(L, idx);
  luaV_objlen(L, L->शिखरम्.p, t);
  api_incr_शिखरम्(L);
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, शून्य **ud) {
  lua_Alloc f;
  lua_तालकम्(L);
  यदि (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  प्रत्यागम f;
}


LUA_API शून्य lua_setallocf (lua_State *L, lua_Alloc f, शून्य *ud) {
  lua_तालकम्(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


शून्य lua_setwarnf (lua_State *L, lua_WarnFunction f, शून्य *ud) {
  lua_तालकम्(L);
  G(L)->ud_warn = ud;
  G(L)->warnf = f;
  lua_unlock(L);
}


शून्य lua_warning (lua_State *L, ध्रुव वर्ण *सन्देशः, पूर्णाङ्क tocont) {
  lua_तालकम्(L);
  luaE_warning(L, सन्देशः, tocont);
  lua_unlock(L);
}



LUA_API शून्य *lua_newuserdatauv (lua_State *L, size_t आकार, पूर्णाङ्क nuvalue) {
  Udata *u;
  lua_तालकम्(L);
  api_check(L, 0 <= nuvalue && nuvalue < SHRT_MAX, "invalid value");
  u = luaS_newudata(L, आकार, cast(अचिह्नित ह्रस्व, nuvalue));
  setuvalue(L, s2v(L->शिखरम्.p), u);
  api_incr_शिखरम्(L);
  luaC_checkGC(L);
  lua_unlock(L);
  प्रत्यागम getudatamem(u);
}



स्थिर ध्रुव वर्ण *aux_upvalue (TValue *fi, पूर्णाङ्क n, TValue **val,
                                GCObject **owner) {
  विकल्प (ttypetag(fi)) {
    पक्ष LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      यदि (!(cast_uint(n) - 1u < cast_uint(f->nupvalues)))
        प्रत्यागम रिक्त;  /* 'n' not in [1, f->nupvalues] */
      *val = &f->upvalue[n-1];
      यदि (owner) *owner = obj2gco(f);
      प्रत्यागम "";
    }
    पक्ष LUA_VLCL: {  /* Lua closure */
      LClosure *f = clLvalue(fi);
      TString *name;
      Proto *p = f->p;
      यदि (!(cast_uint(n) - 1u  < cast_uint(p->sizeupvalues)))
        प्रत्यागम रिक्त;  /* 'n' not in [1, p->sizeupvalues] */
      *val = f->upvals[n-1]->v.p;
      यदि (owner) *owner = obj2gco(f->upvals[n - 1]);
      name = p->upvalues[n-1].name;
      प्रत्यागम (name == रिक्त) ? "(no name)" : getstr(name);
    }
    स्वभाव: प्रत्यागम रिक्त;  /* not a closure */
  }
}


LUA_API ध्रुव वर्ण *lua_getupvalue (lua_State *L, पूर्णाङ्क funcindex, पूर्णाङ्क n) {
  ध्रुव वर्ण *name;
  TValue *val = रिक्त;  /* to avoid warnings */
  lua_तालकम्(L);
  name = aux_upvalue(index2value(L, funcindex), n, &val, रिक्त);
  यदि (name) {
    setobj2s(L, L->शिखरम्.p, val);
    api_incr_शिखरम्(L);
  }
  lua_unlock(L);
  प्रत्यागम name;
}


LUA_API ध्रुव वर्ण *lua_setupvalue (lua_State *L, पूर्णाङ्क funcindex, पूर्णाङ्क n) {
  ध्रुव वर्ण *name;
  TValue *val = रिक्त;  /* to avoid warnings */
  GCObject *owner = रिक्त;  /* to avoid warnings */
  TValue *fi;
  lua_तालकम्(L);
  fi = index2value(L, funcindex);
  api_checkpop(L, 1);
  name = aux_upvalue(fi, n, &val, &owner);
  यदि (name) {
    L->शिखरम्.p--;
    setobj(L, val, s2v(L->शिखरम्.p));
    luaC_barrier(L, owner, val);
  }
  lua_unlock(L);
  प्रत्यागम name;
}


स्थिर ऊर्ध्वम्_Val **getupvalref (lua_State *L, पूर्णाङ्क fidx, पूर्णाङ्क n, LClosure **pf) {
  स्थिर ध्रुव ऊर्ध्वम्_Val *ध्रुव nullup = रिक्त;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  api_check(L, ttisLclosure(fi), "Lua function expected");
  f = clLvalue(fi);
  यदि (pf) *pf = f;
  यदि (1 <= n && n <= f->p->sizeupvalues)
    प्रत्यागम &f->upvals[n - 1];  /* get its upvalue pointer */
  अन्यथा
    प्रत्यागम (ऊर्ध्वम्_Val**)&nullup;
}


LUA_API शून्य *lua_upvalueid (lua_State *L, पूर्णाङ्क fidx, पूर्णाङ्क n) {
  TValue *fi = index2value(L, fidx);
  विकल्प (ttypetag(fi)) {
    पक्ष LUA_VLCL: {  /* lua closure */
      प्रत्यागम *getupvalref(L, fidx, n, रिक्त);
    }
    पक्ष LUA_VCCL: {  /* C closure */
      CClosure *f = clCvalue(fi);
      यदि (1 <= n && n <= f->nupvalues)
        प्रत्यागम &f->upvalue[n - 1];
      /* else */
    }  /* FALLTHROUGH */
    पक्ष LUA_VLCF:
      प्रत्यागम रिक्त;  /* light C functions have no upvalues */
    स्वभाव: {
      api_check(L, 0, "function expected");
      प्रत्यागम रिक्त;
    }
  }
}


LUA_API शून्य lua_upvaluejoin (lua_State *L, पूर्णाङ्क fidx1, पूर्णाङ्क n1,
                                            पूर्णाङ्क fidx2, पूर्णाङ्क n2) {
  LClosure *f1;
  ऊर्ध्वम्_Val **up1 = getupvalref(L, fidx1, n1, &f1);
  ऊर्ध्वम्_Val **up2 = getupvalref(L, fidx2, n2, रिक्त);
  api_check(L, *up1 != रिक्त && *up2 != रिक्त, "invalid upvalue index");
  *up1 = *up2;
  luaC_objbarrier(L, f1, *up1);
}


