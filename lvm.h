/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lvm_h
#ವ್ಯಾಖ್ಯಾನಿಸು lvm_h


#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"


#ಆದರೆ !defined(LUA_NOCVTN2S)
#ವ್ಯಾಖ್ಯಾನಿಸು cvt2str(o)	ttisnumber(o)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು cvt2str(o)	0	/* no conversion from numbers to strings */
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ !defined(LUA_NOCVTS2N)
#ವ್ಯಾಖ್ಯಾನಿಸು cvt2num(o)	ttisstring(o)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು cvt2num(o)	0	/* no conversion from strings to numbers */
#ಆದರೆ_ಅಂತ್ಯ


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#ಆದರೆ !defined(LUA_FLOORN2I)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FLOORN2I		F2Ieq
#ಆದರೆ_ಅಂತ್ಯ


/*
** Rounding modes for float->integer coercion
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceiling of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#ವ್ಯಾಖ್ಯಾನಿಸು tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#ವ್ಯಾಖ್ಯಾನಿಸು tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_ಅಂಕೆ(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#ವ್ಯಾಖ್ಯಾನಿಸು tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#ವ್ಯಾಖ್ಯಾನಿಸು tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#ವ್ಯಾಖ್ಯಾನಿಸು intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#ವ್ಯಾಖ್ಯಾನಿಸು luaV_rawequalobj(t1,t2)		luaV_equalobj(ಖಾಲಿ,t1,t2)


/*
** fast track for 'gettable'
*/
#ವ್ಯಾಖ್ಯಾನಿಸು luaV_fastget(t,k,res,f, ಟ್ಯಾಗ್) \
  (ಟ್ಯಾಗ್ = (!ttistable(t) ? LUA_VNOTABLE : f(hvalue(t), k, res)))


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು luaV_fastgeti(t,k,res,ಟ್ಯಾಗ್) \
  ಆದರೆ (!ttistable(t)) ಟ್ಯಾಗ್ = LUA_VNOTABLE; \
  ಇಲ್ಲದಿದ್ದರೆ { luaH_fastgeti(hvalue(t), k, res, ಟ್ಯಾಗ್); }


#ವ್ಯಾಖ್ಯಾನಿಸು luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaV_fastseti(t,k,val,hres) \
  ಆದರೆ (!ttistable(t)) hres = HNOTATABLE; \
  ಇಲ್ಲದಿದ್ದರೆ { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#ವ್ಯಾಖ್ಯಾನಿಸು luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#ವ್ಯಾಖ್ಯಾನಿಸು luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC ಪೂರ್ಣಾಂಕ luaV_equalobj (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t1, ಸ್ಥಿರಾಂಕ TValue *t2);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaV_lessthan (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaV_lessequal (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *l, ಸ್ಥಿರಾಂಕ TValue *r);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaV_tonumber_ (ಸ್ಥಿರಾಂಕ TValue *obj, lua_ಸಂಖ್ಯೆ *n);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaV_tointeger (ಸ್ಥಿರಾಂಕ TValue *obj, lua_Integer *p, F2Imod ಬಹುಲಕ);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaV_tointegerns (ಸ್ಥಿರಾಂಕ TValue *obj, lua_Integer *p,
                                F2Imod ಬಹುಲಕ);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaV_flttointeger (lua_ಸಂಖ್ಯೆ n, lua_Integer *p, F2Imod ಬಹುಲಕ);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t, TValue *ಕೀಲಿ,
                                                StkId val, lu_byte ಟ್ಯಾಗ್);
LUAI_FUNC ಶೂನ್ಯ luaV_finishset (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *t, TValue *ಕೀಲಿ,
                                             TValue *val, ಪೂರ್ಣಾಂಕ aux);
LUAI_FUNC ಶೂನ್ಯ lua_V_ಮುಕ್ತಾಯ_Op (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC ಶೂನ್ಯ luaV_concat (lua_State *L, ಪೂರ್ಣಾಂಕ ಒಟ್ಟು);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_ಸಂಖ್ಯೆ luaV_modf (lua_State *L, lua_ಸಂಖ್ಯೆ x, lua_ಸಂಖ್ಯೆ y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC ಶೂನ್ಯ luaV_objlen (lua_State *L, StkId ra, ಸ್ಥಿರಾಂಕ TValue *rb);

#ಆದರೆ_ಅಂತ್ಯ
