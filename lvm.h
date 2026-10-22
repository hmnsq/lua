/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lvm_h
#وصف_ڪريو lvm_h


#ضمن "ldo.h"
#ضمن "lobject.h"
#ضمن "ltm.h"


#إذا !defined(LUA_NOCVTN2S)
#وصف_ڪريو cvt2str(o)	ttisnumber(o)
#وإلا
#وصف_ڪريو cvt2str(o)	0	/* no conversion from numbers to strings */
#جيڪڏهن_پورو


#إذا !defined(LUA_NOCVTS2N)
#وصف_ڪريو cvt2num(o)	ttisstring(o)
#وإلا
#وصف_ڪريو cvt2num(o)	0	/* no conversion from strings to numbers */
#جيڪڏهن_پورو


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#إذا !defined(LUA_FLOORN2I)
#وصف_ڪريو LUA_FLOORN2I		F2Ieq
#جيڪڏهن_پورو


/*
** Rounding modes for float->integer coercion
 */
تعريفنوع تعداد {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceiling of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#وصف_ڪريو tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#وصف_ڪريو tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_انگ(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#وصف_ڪريو tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#وصف_ڪريو tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#وصف_ڪريو intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#وصف_ڪريو luaV_rawequalobj(t1,t2)		luaV_equalobj(عدم,t1,t2)


/*
** fast track for 'gettable'
*/
#وصف_ڪريو luaV_fastget(t,k,res,f, ٽيگ) \
  (ٽيگ = (!ttistable(t) ? LUA_VNOTABLE : f(hvalue(t), k, res)))


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#وصف_ڪريو luaV_fastgeti(t,k,res,ٽيگ) \
  إذا (!ttistable(t)) ٽيگ = LUA_VNOTABLE; \
  وإلا { luaH_fastgeti(hvalue(t), k, res, ٽيگ); }


#وصف_ڪريو luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#وصف_ڪريو luaV_fastseti(t,k,val,hres) \
  إذا (!ttistable(t)) hres = HNOTATABLE; \
  وإلا { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#وصف_ڪريو luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#وصف_ڪريو luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC صحيح luaV_equalobj (lua_State *L, ثابت TValue *t1, ثابت TValue *t2);
LUAI_FUNC صحيح luaV_lessthan (lua_State *L, ثابت TValue *l, ثابت TValue *r);
LUAI_FUNC صحيح luaV_lessequal (lua_State *L, ثابت TValue *l, ثابت TValue *r);
LUAI_FUNC صحيح luaV_tonumber_ (ثابت TValue *obj, lua_عدد *n);
LUAI_FUNC صحيح luaV_tointeger (ثابت TValue *obj, lua_Integer *p, F2Imod گهڻائي);
LUAI_FUNC صحيح luaV_tointegerns (ثابت TValue *obj, lua_Integer *p,
                                F2Imod گهڻائي);
LUAI_FUNC صحيح luaV_flttointeger (lua_عدد n, lua_Integer *p, F2Imod گهڻائي);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, ثابت TValue *t, TValue *ڪنجي,
                                                StkId val, lu_byte ٽيگ);
LUAI_FUNC فراغ luaV_finishset (lua_State *L, ثابت TValue *t, TValue *ڪنجي,
                                             TValue *val, صحيح aux);
LUAI_FUNC فراغ lua_V_پڄاڻي_Op (lua_State *L);
LUAI_FUNC فراغ luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC فراغ luaV_concat (lua_State *L, صحيح ڪل);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_عدد luaV_modf (lua_State *L, lua_عدد x, lua_عدد y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC فراغ luaV_objlen (lua_State *L, StkId ra, ثابت TValue *rb);

#جيڪڏهن_پورو
