/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی lfunc_h
#تعریف_کړه lfunc_h


#ضمن "lobject.h"


#تعریف_کړه اندازه_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + حجم(TValue) * cast_uint(n))

#تعریف_کړه اندازه_Lclosure(n)  \
	(offsetof(LClosure, upvals) + حجم(پورته_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#تعریف_کړه isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#تعریف_کړه MAXUPVAL	255


#تعریف_کړه upisopen(پورته)	((پورته)->v.p != &(پورته)->u.ارزښت)


#تعریف_کړه uplevel(پورته)	check_exp(upisopen(پورته), cast(StkId, (پورته)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#تعریف_کړه MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#تعریف_کړه CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC فراغ luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC پورته_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus حالت, صحيح yy);
LUAI_FUNC فراغ luaF_unlinkupval (پورته_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC فراغ luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ثابت حرف *luaF_getlocalname (ثابت Proto *func,
                                         بینښه local_شمېره, صحيح pc);


#د_که_پای
