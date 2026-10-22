/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ifndef lfunc_h
#define lfunc_h


#अन्तर्भाव "lobject.h"


#define sizeCclosure(n)  \
	(दुरत्व(CClosure, upvalue) + परिमाण(TValue) * cast_uint(n))

#define sizeLclosure(n)  \
	(दुरत्व(LClosure, upvals) + परिमाण(UpVal *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#define isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#define MAXUPVAL	255


#define upisopen(up)	((up)->v.p != &(up)->u.मूल्य)


#define uplevel(up)	check_exp(upisopen(up), cast(StkId, (up)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#define MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#define CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, पूर्णाङ्क nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, पूर्णाङ्क nupvals);
LUAI_FUNC शून्य luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC UpVal *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC शून्य luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC शून्य luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus status, पूर्णाङ्क yy);
LUAI_FUNC शून्य luaF_unlinkupval (UpVal *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC शून्य luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ध्रुव वर्ण *luaF_getlocalname (ध्रुव Proto *func,
                                         अचिन्हित local_संख्या, पूर्णाङ्क pc);


#endif
