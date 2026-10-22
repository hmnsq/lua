/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lvm_h
#నిర్వచించు lvm_h


#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "ltm.h"


#ఒకవేళ !defined(LUA_NOCVTN2S)
#నిర్వచించు cvt2str(o)	ttisnumber(o)
#లేకపోతే
#నిర్వచించు cvt2str(o)	0	/* no conversion from numbers to strings */
#అయితే_ముగింపు


#ఒకవేళ !defined(LUA_NOCVTS2N)
#నిర్వచించు cvt2num(o)	ttisstring(o)
#లేకపోతే
#నిర్వచించు cvt2num(o)	0	/* no conversion from strings to numbers */
#అయితే_ముగింపు


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#ఒకవేళ !defined(LUA_FLOORN2I)
#నిర్వచించు LUA_FLOORN2I		F2Ieq
#అయితే_ముగింపు


/*
** Rounding modes for float->integer coercion
 */
రకనిర్వచనం గణన {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceiling of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#నిర్వచించు tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#నిర్వచించు tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_అంకె(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#నిర్వచించు tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#నిర్వచించు tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#నిర్వచించు intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#నిర్వచించు luaV_rawequalobj(t1,t2)		luaV_equalobj(ఖాళీ,t1,t2)


/*
** fast track for 'gettable'
*/
#నిర్వచించు luaV_fastget(t,k,res,f, ట్యాగ్) \
  (ట్యాగ్ = (!ttistable(t) ? LUA_VNOTABLE : f(hvalue(t), k, res)))


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#నిర్వచించు luaV_fastgeti(t,k,res,ట్యాగ్) \
  ఒకవేళ (!ttistable(t)) ట్యాగ్ = LUA_VNOTABLE; \
  లేకపోతే { luaH_fastgeti(hvalue(t), k, res, ట్యాగ్); }


#నిర్వచించు luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#నిర్వచించు luaV_fastseti(t,k,val,hres) \
  ఒకవేళ (!ttistable(t)) hres = HNOTATABLE; \
  లేకపోతే { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#నిర్వచించు luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#నిర్వచించు luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC పూర్ణాంకం luaV_equalobj (lua_State *L, స్థిరరాశి TValue *t1, స్థిరరాశి TValue *t2);
LUAI_FUNC పూర్ణాంకం luaV_lessthan (lua_State *L, స్థిరరాశి TValue *l, స్థిరరాశి TValue *r);
LUAI_FUNC పూర్ణాంకం luaV_lessequal (lua_State *L, స్థిరరాశి TValue *l, స్థిరరాశి TValue *r);
LUAI_FUNC పూర్ణాంకం luaV_tonumber_ (స్థిరరాశి TValue *obj, lua_సంఖ్య *n);
LUAI_FUNC పూర్ణాంకం luaV_tointeger (స్థిరరాశి TValue *obj, lua_Integer *p, F2Imod బహుళకం);
LUAI_FUNC పూర్ణాంకం luaV_tointegerns (స్థిరరాశి TValue *obj, lua_Integer *p,
                                F2Imod బహుళకం);
LUAI_FUNC పూర్ణాంకం luaV_flttointeger (lua_సంఖ్య n, lua_Integer *p, F2Imod బహుళకం);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, స్థిరరాశి TValue *t, TValue *కీలకం,
                                                StkId val, lu_byte ట్యాగ్);
LUAI_FUNC శూన్య luaV_finishset (lua_State *L, స్థిరరాశి TValue *t, TValue *కీలకం,
                                             TValue *val, పూర్ణాంకం aux);
LUAI_FUNC శూన్య lua_V_ముగింపు_Op (lua_State *L);
LUAI_FUNC శూన్య luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC శూన్య luaV_concat (lua_State *L, పూర్ణాంకం మొత్తం);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_సంఖ్య luaV_modf (lua_State *L, lua_సంఖ్య x, lua_సంఖ్య y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC శూన్య luaV_objlen (lua_State *L, StkId ra, స్థిరరాశి TValue *rb);

#అయితే_ముగింపు
