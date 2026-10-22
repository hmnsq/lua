/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lfunc_h
#تعریف_کرو lfunc_h


#شامل "lobject.h"


#تعریف_کرو سائز_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + حجم(TValue) * cast_uint(n))

#تعریف_کرو سائز_Lclosure(n)  \
	(offsetof(LClosure, upvals) + حجم(اوپر_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#تعریف_کرو isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#تعریف_کرو MAXUPVAL	255


#تعریف_کرو upisopen(اوپر)	((اوپر)->v.p != &(اوپر)->u.قدر)


#تعریف_کرو uplevel(اوپر)	check_exp(upisopen(اوپر), cast(StkId, (اوپر)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#تعریف_کرو MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#تعریف_کرو CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, صحیح nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, صحیح nupvals);
LUAI_FUNC خالی luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC اوپر_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC خالی luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC خالی luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus حالت_2, صحیح yy);
LUAI_FUNC خالی luaF_unlinkupval (اوپر_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC خالی luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ثابت حرف *luaF_getlocalname (ثابت Proto *func,
                                         غیرعلامتی local_نمبر, صحیح pc);


#اگر_ختم
