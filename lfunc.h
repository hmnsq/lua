/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ifndef lfunc_h
#define lfunc_h


#அந்தர்பாவ "lobject.h"


#define sizeCclosure(n)  \
	(offsetof(CClosure, upvalue) + அளவு(TValue) * cast_uint(n))

#define sizeLclosure(n)  \
	(offsetof(LClosure, upvals) + அளவு(UpVal *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#define isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#define MAXUPVAL	255


#define upisopen(up)	((up)->v.p != &(up)->u.மதிப்பு)


#define uplevel(up)	check_exp(upisopen(up), cast(StkId, (up)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#define MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#define CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, முழு nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, முழு nupvals);
LUAI_FUNC வெறுமை luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC UpVal *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC வெறுமை luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC வெறுமை luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus status, முழு yy);
LUAI_FUNC வெறுமை luaF_unlinkupval (UpVal *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC வெறுமை luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC மாறிலி எழுத்து *luaF_getlocalname (மாறிலி Proto *func,
                                         unsigned local_எண், முழு pc);


#endif
