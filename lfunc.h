/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lfunc_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lfunc_h


#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + ਆਕਾਰ(TValue) * cast_uint(n))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_Lclosure(n)  \
	(offsetof(LClosure, upvals) + ਆਕਾਰ(ਉੱਪਰ_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXUPVAL	255


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ upisopen(ਉੱਪਰ)	((ਉੱਪਰ)->v.p != &(ਉੱਪਰ)->u.ਮੁੱਲ)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ uplevel(ਉੱਪਰ)	check_exp(upisopen(ਉੱਪਰ), cast(StkId, (ਉੱਪਰ)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, ਪੂਰਨ nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, ਪੂਰਨ nupvals);
LUAI_FUNC ਖਾਲੀ luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC ਉੱਪਰ_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC ਖਾਲੀ luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC ਖਾਲੀ luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus ਸਥਿਤੀ, ਪੂਰਨ yy);
LUAI_FUNC ਖਾਲੀ luaF_unlinkupval (ਉੱਪਰ_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC ਖਾਲੀ luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ਅਚਲ ਅੱਖਰ *luaF_getlocalname (ਅਚਲ Proto *func,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ local_ਸੰਖਿਆ, ਪੂਰਨ pc);


#ਜੇ_ਅੰਤ
