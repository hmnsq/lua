/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् lfunc_h
#परिभाषय lfunc_h


#अन्तर्भाव "lobject.h"


#परिभाषय आकार_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + परिमाण(TValue) * cast_uint(n))

#परिभाषय आकार_Lclosure(n)  \
	(offsetof(LClosure, upvals) + परिमाण(ऊर्ध्वम्_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#परिभाषय isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#परिभाषय MAXUPVAL	255


#परिभाषय upisopen(ऊर्ध्वम्)	((ऊर्ध्वम्)->v.p != &(ऊर्ध्वम्)->u.मानम्)


#परिभाषय uplevel(ऊर्ध्वम्)	check_exp(upisopen(ऊर्ध्वम्), cast(StkId, (ऊर्ध्वम्)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#परिभाषय MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#परिभाषय CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, पूर्णाङ्क nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, पूर्णाङ्क nupvals);
LUAI_FUNC शून्य luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC ऊर्ध्वम्_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC शून्य luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC शून्य luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus स्थितिः, पूर्णाङ्क yy);
LUAI_FUNC शून्य luaF_unlinkupval (ऊर्ध्वम्_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC शून्य luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ध्रुव वर्ण *luaF_getlocalname (ध्रुव Proto *func,
                                         अचिह्नित local_सङ्ख्या, पूर्णाङ्क pc);


#यदि_अन्तः
