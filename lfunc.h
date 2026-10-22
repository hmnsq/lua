/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده lfunc_h
#تعریف_کن lfunc_h


#ضمن "lobject.h"


#تعریف_کن اندازه_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + حجم(TValue) * cast_uint(n))

#تعریف_کن اندازه_Lclosure(n)  \
	(offsetof(LClosure, upvals) + حجم(به_بالا_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#تعریف_کن isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#تعریف_کن MAXUPVAL	255


#تعریف_کن upisopen(به_بالا)	((به_بالا)->v.p != &(به_بالا)->u.مقدار)


#تعریف_کن uplevel(به_بالا)	check_exp(upisopen(به_بالا), cast(StkId, (به_بالا)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#تعریف_کن MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#تعریف_کن CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC فراغ luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC به_بالا_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus وضعیت, صحيح yy);
LUAI_FUNC فراغ luaF_unlinkupval (به_بالا_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC فراغ luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ثابت حرف *luaF_getlocalname (ثابت Proto *func,
                                         بیعلامت local_عدد, صحيح pc);


#پایان_اگر
