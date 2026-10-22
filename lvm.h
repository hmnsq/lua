/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lvm_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lvm_h


#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"


#ਜੇ !defined(LUA_NOCVTN2S)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cvt2str(o)	ttisnumber(o)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cvt2str(o)	0	/* no conversion from numbers to strings */
#ਜੇ_ਅੰਤ


#ਜੇ !defined(LUA_NOCVTS2N)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cvt2num(o)	ttisstring(o)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cvt2num(o)	0	/* no conversion from strings to numbers */
#ਜੇ_ਅੰਤ


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#ਜੇ !defined(LUA_FLOORN2I)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_FLOORN2I		F2Ieq
#ਜੇ_ਅੰਤ


/*
** Rounding modes for float->integer coercion
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceiling of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_ਅੰਕ(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaV_rawequalobj(t1,t2)		luaV_equalobj(ਨਿਰਾ,t1,t2)


/*
** fast track for 'gettable'
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaV_fastget(t,k,res,f, ਟੈਗ) \
  (ਟੈਗ = (!ttistable(t) ? LUA_VNOTABLE : f(hvalue(t), k, res)))


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaV_fastgeti(t,k,res,ਟੈਗ) \
  ਜੇ (!ttistable(t)) ਟੈਗ = LUA_VNOTABLE; \
  ਨਹੀਂਤਾਂ { luaH_fastgeti(hvalue(t), k, res, ਟੈਗ); }


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaV_fastseti(t,k,val,hres) \
  ਜੇ (!ttistable(t)) hres = HNOTATABLE; \
  ਨਹੀਂਤਾਂ { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC ਪੂਰਨ luaV_equalobj (lua_State *L, ਅਚਲ TValue *t1, ਅਚਲ TValue *t2);
LUAI_FUNC ਪੂਰਨ luaV_lessthan (lua_State *L, ਅਚਲ TValue *l, ਅਚਲ TValue *r);
LUAI_FUNC ਪੂਰਨ luaV_lessequal (lua_State *L, ਅਚਲ TValue *l, ਅਚਲ TValue *r);
LUAI_FUNC ਪੂਰਨ luaV_tonumber_ (ਅਚਲ TValue *obj, lua_ਸੰਖਿਆ *n);
LUAI_FUNC ਪੂਰਨ luaV_tointeger (ਅਚਲ TValue *obj, lua_Integer *p, F2Imod ਬਹੁਲਕ);
LUAI_FUNC ਪੂਰਨ luaV_tointegerns (ਅਚਲ TValue *obj, lua_Integer *p,
                                F2Imod ਬਹੁਲਕ);
LUAI_FUNC ਪੂਰਨ luaV_flttointeger (lua_ਸੰਖਿਆ n, lua_Integer *p, F2Imod ਬਹੁਲਕ);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, ਅਚਲ TValue *t, TValue *ਕੁੰਜੀ,
                                                StkId val, lu_byte ਟੈਗ);
LUAI_FUNC ਖਾਲੀ luaV_finishset (lua_State *L, ਅਚਲ TValue *t, TValue *ਕੁੰਜੀ,
                                             TValue *val, ਪੂਰਨ aux);
LUAI_FUNC ਖਾਲੀ lua_V_ਅੰਤ_Op (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC ਖਾਲੀ luaV_concat (lua_State *L, ਪੂਰਨ ਕੁੱਲ);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_ਸੰਖਿਆ luaV_modf (lua_State *L, lua_ਸੰਖਿਆ x, lua_ਸੰਖਿਆ y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC ਖਾਲੀ luaV_objlen (lua_State *L, StkId ra, ਅਚਲ TValue *rb);

#ਜੇ_ਅੰਤ
