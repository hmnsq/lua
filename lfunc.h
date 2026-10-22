/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#جے_بنا_تعریف lfunc_h
#تعریف_کرو lfunc_h


#ضمن "lobject.h"


#تعریف_کرو ناپ_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + حجم(TValue) * cast_uint(n))

#تعریف_کرو ناپ_Lclosure(n)  \
	(offsetof(LClosure, upvals) + حجم(اتے_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#تعریف_کرو isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#تعریف_کرو MAXUPVAL	255


#تعریف_کرو upisopen(اتے)	((اتے)->v.p != &(اتے)->u.قدر)


#تعریف_کرو uplevel(اتے)	check_exp(upisopen(اتے), cast(StkId, (اتے)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#تعریف_کرو MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#تعریف_کرو CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, صحيح nupvals);
LUAI_FUNC فراغ luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC اتے_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC فراغ luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus حالت, صحيح yy);
LUAI_FUNC فراغ luaF_unlinkupval (اتے_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC فراغ luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ثابت حرف *luaF_getlocalname (ثابت Proto *func,
                                         بنانشانی local_نمبر, صحيح pc);


#جے_انت
