/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lfunc_h
#وصف_ڪريو lfunc_h


#ضمن "lobject.h"


#وصف_ڪريو سائيز_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + حجم(TValue) * cast_uint(n))

#وصف_ڪريو سائيز_Lclosure(n)  \
	(offsetof(LClosure, upvals) + حجم(مٿي_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#وصف_ڪريو isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#وصف_ڪريو MAXUPVAL	255


#وصف_ڪريو upisopen(مٿي)	((مٿي)->v.p != &(مٿي)->u.قدر)


#وصف_ڪريو uplevel(مٿي)	check_exp(upisopen(مٿي), cast(StkId, (مٿي)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#وصف_ڪريو MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#وصف_ڪريو CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC فراغ luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC مٿي_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus حالت, صحيح yy);
LUAI_FUNC فراغ luaF_unlinkupval (مٿي_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC فراغ luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ثابت حرف *luaF_getlocalname (ثابت Proto *func,
                                         بنانشان local_عدد, صحيح pc);


#جيڪڏهن_پورو
