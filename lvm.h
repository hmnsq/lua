/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lvm_h
#घोषणा lvm_h


#समावेश "ldo.h"
#समावेश "lobject.h"
#समावेश "ltm.h"


#अगर !defined(LUA_NOCVTN2S)
#घोषणा cvt2str(o)	ttisnumber(o)
#अन्यथा
#घोषणा cvt2str(o)	0	/* no conversion from numbers to strings */
#पूर्ण_अगर


#अगर !defined(LUA_NOCVTS2N)
#घोषणा cvt2num(o)	ttisstring(o)
#अन्यथा
#घोषणा cvt2num(o)	0	/* no conversion from strings to numbers */
#पूर्ण_अगर


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#अगर !defined(LUA_FLOORN2I)
#घोषणा LUA_FLOORN2I		F2Ieq
#पूर्ण_अगर


/*
** Rounding modes for float->integer coercion
 */
प्रकार क्रमागत {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceiling of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#घोषणा tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#घोषणा tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_अंक(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#घोषणा tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#घोषणा tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#घोषणा intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#घोषणा luaV_rawequalobj(t1,t2)		luaV_equalobj(शून्य,t1,t2)


/*
** fast track for 'gettable'
*/
#घोषणा luaV_fastget(t,k,res,f, tag) \
  (tag = (!ttistable(t) ? LUA_VNOTABLE : f(hvalue(t), k, res)))


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#घोषणा luaV_fastgeti(t,k,res,tag) \
  अगर (!ttistable(t)) tag = LUA_VNOTABLE; \
  अन्यथा { luaH_fastgeti(hvalue(t), k, res, tag); }


#घोषणा luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#घोषणा luaV_fastseti(t,k,val,hres) \
  अगर (!ttistable(t)) hres = HNOTATABLE; \
  अन्यथा { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#घोषणा luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#घोषणा luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC पूर्णांक luaV_equalobj (lua_State *L, स्थिर TValue *t1, स्थिर TValue *t2);
LUAI_FUNC पूर्णांक luaV_lessthan (lua_State *L, स्थिर TValue *l, स्थिर TValue *r);
LUAI_FUNC पूर्णांक luaV_lessequal (lua_State *L, स्थिर TValue *l, स्थिर TValue *r);
LUAI_FUNC पूर्णांक luaV_tonumber_ (स्थिर TValue *obj, lua_संख्या *n);
LUAI_FUNC पूर्णांक luaV_tointeger (स्थिर TValue *obj, lua_Integer *p, F2Imod mode);
LUAI_FUNC पूर्णांक luaV_tointegerns (स्थिर TValue *obj, lua_Integer *p,
                                F2Imod mode);
LUAI_FUNC पूर्णांक luaV_flttointeger (lua_संख्या n, lua_Integer *p, F2Imod mode);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, स्थिर TValue *t, TValue *कुंजी,
                                                StkId val, lu_byte tag);
LUAI_FUNC व्योम luaV_finishset (lua_State *L, स्थिर TValue *t, TValue *कुंजी,
                                             TValue *val, पूर्णांक aux);
LUAI_FUNC व्योम luaV_finishOp (lua_State *L);
LUAI_FUNC व्योम luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC व्योम luaV_concat (lua_State *L, पूर्णांक योग);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_संख्या luaV_modf (lua_State *L, lua_संख्या x, lua_संख्या y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC व्योम luaV_objlen (lua_State *L, StkId ra, स्थिर TValue *rb);

#पूर्ण_अगर
