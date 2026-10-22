/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ifndef lvm_h
#define lvm_h


#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"


#ಯದಿ !defined(LUA_NOCVTN2S)
#define cvt2str(o)	ttisnumber(o)
#ಅನ್ಯಥಾ
#define cvt2str(o)	0	/* no conversion from numbers to strings */
#endif


#ಯದಿ !defined(LUA_NOCVTS2N)
#define cvt2num(o)	ttisstring(o)
#ಅನ್ಯಥಾ
#define cvt2num(o)	0	/* no conversion from strings to numbers */
#endif


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#ಯದಿ !defined(LUA_FLOORN2I)
#define LUA_FLOORN2I		F2Ieq
#endif


/*
** Rounding modes for float->integer coercion
 */
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceiling of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#define tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#define tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_ಅಂಕೆ(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#define tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#define tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#define intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#define luaV_rawequalobj(t1,t2)		luaV_equalobj(ರಿಕ್ತ,t1,t2)


/*
** fast track for 'gettable'
*/
#define luaV_fastget(t,k,res,f, tag) \
  (tag = (!ttistable(t) ? LUA_VNOTABLE : f(hvalue(t), k, res)))


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#define luaV_fastgeti(t,k,res,tag) \
  ಯದಿ (!ttistable(t)) tag = LUA_VNOTABLE; \
  ಅನ್ಯಥಾ { luaH_fastgeti(hvalue(t), k, res, tag); }


#define luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#define luaV_fastseti(t,k,val,hres) \
  ಯದಿ (!ttistable(t)) hres = HNOTATABLE; \
  ಅನ್ಯಥಾ { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#define luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#define luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaV_equalobj (lua_State *L, ಧ್ರುವ TValue *t1, ಧ್ರುವ TValue *t2);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaV_lessthan (lua_State *L, ಧ್ರುವ TValue *l, ಧ್ರುವ TValue *r);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaV_lessequal (lua_State *L, ಧ್ರುವ TValue *l, ಧ್ರುವ TValue *r);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaV_tonumber_ (ಧ್ರುವ TValue *obj, lua_ಸಂಖ್ಯೆ *n);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaV_tointeger (ಧ್ರುವ TValue *obj, lua_Integer *p, F2Imod mode);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaV_tointegerns (ಧ್ರುವ TValue *obj, lua_Integer *p,
                                F2Imod mode);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaV_flttointeger (lua_ಸಂಖ್ಯೆ n, lua_Integer *p, F2Imod mode);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, ಧ್ರುವ TValue *t, TValue *ಕೀಲಿ,
                                                StkId val, lu_byte tag);
LUAI_FUNC ಶೂನ್ಯ luaV_finishset (lua_State *L, ಧ್ರುವ TValue *t, TValue *ಕೀಲಿ,
                                             TValue *val, ಪೂರ್ಣಾಙ್ಕ aux);
LUAI_FUNC ಶೂನ್ಯ luaV_finishOp (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC ಶೂನ್ಯ luaV_concat (lua_State *L, ಪೂರ್ಣಾಙ್ಕ total);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_ಸಂಖ್ಯೆ luaV_modf (lua_State *L, lua_ಸಂಖ್ಯೆ x, lua_ಸಂಖ್ಯೆ y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC ಶೂನ್ಯ luaV_objlen (lua_State *L, StkId ra, ಧ್ರುವ TValue *rb);

#endif
