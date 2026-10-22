/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#جے_بنا_تعریف lvm_h
#تعریف_کرو lvm_h


#ضمن "ldo.h"
#ضمن "lobject.h"
#ضمن "ltm.h"


#إذا !defined(LUA_NOCVTN2S)
#تعریف_کرو cvt2str(o)	ttisnumber(o)
#وإلا
#تعریف_کرو cvt2str(o)	0	/* no conversion from numbers to strings */
#جے_انت


#إذا !defined(LUA_NOCVTS2N)
#تعریف_کرو cvt2num(o)	ttisstring(o)
#وإلا
#تعریف_کرو cvt2num(o)	0	/* no conversion from strings to numbers */
#جے_انت


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#إذا !defined(LUA_FLOORN2I)
#تعریف_کرو LUA_FLOORN2I		F2Ieq
#جے_انت


/*
** Rounding modes for float->integer coercion
 */
تعريفنوع تعداد {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceiling of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#تعریف_کرو tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#تعریف_کرو tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_عدد(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#تعریف_کرو tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#تعریف_کرو tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#تعریف_کرو intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#تعریف_کرو luaV_rawequalobj(t1,t2)		luaV_equalobj(عدم,t1,t2)


/*
** fast track for 'gettable'
*/
#تعریف_کرو luaV_fastget(t,k,res,f, ٹیگ) \
  (ٹیگ = (!ttistable(t) ? LUA_VNOTABLE : f(hvalue(t), k, res)))


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#تعریف_کرو luaV_fastgeti(t,k,res,ٹیگ) \
  إذا (!ttistable(t)) ٹیگ = LUA_VNOTABLE; \
  وإلا { luaH_fastgeti(hvalue(t), k, res, ٹیگ); }


#تعریف_کرو luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#تعریف_کرو luaV_fastseti(t,k,val,hres) \
  إذا (!ttistable(t)) hres = HNOTATABLE; \
  وإلا { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#تعریف_کرو luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#تعریف_کرو luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC صحيح luaV_equalobj (lua_State *L, ثابت TValue *t1, ثابت TValue *t2);
LUAI_FUNC صحيح luaV_lessthan (lua_State *L, ثابت TValue *l, ثابت TValue *r);
LUAI_FUNC صحيح luaV_lessequal (lua_State *L, ثابت TValue *l, ثابت TValue *r);
LUAI_FUNC صحيح luaV_tonumber_ (ثابت TValue *obj, lua_نمبر *n);
LUAI_FUNC صحيح luaV_tointeger (ثابت TValue *obj, lua_Integer *p, F2Imod منوال);
LUAI_FUNC صحيح luaV_tointegerns (ثابت TValue *obj, lua_Integer *p,
                                F2Imod منوال);
LUAI_FUNC صحيح luaV_flttointeger (lua_نمبر n, lua_Integer *p, F2Imod منوال);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, ثابت TValue *t, TValue *کلید,
                                                StkId val, lu_byte ٹیگ);
LUAI_FUNC فراغ luaV_finishset (lua_State *L, ثابت TValue *t, TValue *کلید,
                                             TValue *val, صحيح aux);
LUAI_FUNC فراغ lua_V_انت_Op (lua_State *L);
LUAI_FUNC فراغ luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC فراغ luaV_concat (lua_State *L, صحيح کل);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_نمبر luaV_modf (lua_State *L, lua_نمبر x, lua_نمبر y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC فراغ luaV_objlen (lua_State *L, StkId ra, ثابت TValue *rb);

#جے_انت
