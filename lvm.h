/*
** $Id: lvm.h $
** Lua virtual machine
** See Copyright Notice in lua.h
*/

#ifndef lvm_h
#define lvm_h


#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "ltm.h"


#എങ്കിൽ !defined(LUA_NOCVTN2S)
#define cvt2str(o)	ttisnumber(o)
#അല്ലെങ്കിൽ
#define cvt2str(o)	0	/* no conversion from numbers to strings */
#endif


#എങ്കിൽ !defined(LUA_NOCVTS2N)
#define cvt2num(o)	ttisstring(o)
#അല്ലെങ്കിൽ
#define cvt2num(o)	0	/* no conversion from strings to numbers */
#endif


/*
** You can define LUA_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#എങ്കിൽ !defined(LUA_FLOORN2I)
#define LUA_FLOORN2I		F2Ieq
#endif


/*
** Rounding modes for float->integer coercion
 */
തരനിർവചനം എണ്ണൽ {
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
	(ttisinteger(o) ? ((n) = cast_അക്കം(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#define tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LUA_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#define tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LUA_FLOORN2I))


#define intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#define luaV_rawequalobj(t1,t2)		luaV_equalobj(ശൂന്യ,t1,t2)


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
  എങ്കിൽ (!ttistable(t)) tag = LUA_VNOTABLE; \
  അല്ലെങ്കിൽ { luaH_fastgeti(hvalue(t), k, res, tag); }


#define luaV_fastset(t,k,val,hres,f) \
  (hres = (!ttistable(t) ? HNOTATABLE : f(hvalue(t), k, val)))

#define luaV_fastseti(t,k,val,hres) \
  എങ്കിൽ (!ttistable(t)) hres = HNOTATABLE; \
  അല്ലെങ്കിൽ { luaH_fastseti(hvalue(t), k, val, hres); }


/*
** Finish a fast set operation (when fast set succeeds).
*/
#define luaV_finishfastset(L,t,v)	luaC_barrierback(L, gcvalue(t), v)


/*
** Shift right is the same as shift left with a negative 'y'
*/
#define luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LUAI_FUNC പൂർണ്ണസംഖ്യ luaV_equalobj (lua_State *L, സ്ഥിരം TValue *t1, സ്ഥിരം TValue *t2);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaV_lessthan (lua_State *L, സ്ഥിരം TValue *l, സ്ഥിരം TValue *r);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaV_lessequal (lua_State *L, സ്ഥിരം TValue *l, സ്ഥിരം TValue *r);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaV_tonumber_ (സ്ഥിരം TValue *obj, lua_സംഖ്യ *n);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaV_tointeger (സ്ഥിരം TValue *obj, lua_Integer *p, F2Imod mode);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaV_tointegerns (സ്ഥിരം TValue *obj, lua_Integer *p,
                                F2Imod mode);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaV_flttointeger (lua_സംഖ്യ n, lua_Integer *p, F2Imod mode);
LUAI_FUNC lu_byte luaV_finishget (lua_State *L, സ്ഥിരം TValue *t, TValue *താക്കോൽ,
                                                StkId val, lu_byte tag);
LUAI_FUNC ശൂന്യം luaV_finishset (lua_State *L, സ്ഥിരം TValue *t, TValue *താക്കോൽ,
                                             TValue *val, പൂർണ്ണസംഖ്യ aux);
LUAI_FUNC ശൂന്യം luaV_finishOp (lua_State *L);
LUAI_FUNC ശൂന്യം luaV_execute (lua_State *L, CallInfo *ci);
LUAI_FUNC ശൂന്യം luaV_concat (lua_State *L, പൂർണ്ണസംഖ്യ total);
LUAI_FUNC lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
LUAI_FUNC lua_സംഖ്യ luaV_modf (lua_State *L, lua_സംഖ്യ x, lua_സംഖ്യ y);
LUAI_FUNC lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
LUAI_FUNC ശൂന്യം luaV_objlen (lua_State *L, StkId ra, സ്ഥിരം TValue *rb);

#endif
