/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ifndef lfunc_h
#define lfunc_h


#ଅନ୍ତର୍ଭାଵ "lobject.h"


#define sizeCclosure(n)  \
	(offsetof(CClosure, upvalue) + ଆକାର(TValue) * cast_uint(n))

#define sizeLclosure(n)  \
	(offsetof(LClosure, upvals) + ଆକାର(UpVal *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#define isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#define MAXUPVAL	255


#define upisopen(up)	((up)->v.p != &(up)->u.ମୂଲ୍ୟ)


#define uplevel(up)	check_exp(upisopen(up), cast(StkId, (up)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#define MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#define CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nupvals);
LUAI_FUNC ଶୂନ୍ୟ luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC UpVal *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC ଶୂନ୍ୟ luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC ଶୂନ୍ୟ luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus status, ପୂର୍ଣ୍ଣସଂଖ୍ୟା yy);
LUAI_FUNC ଶୂନ୍ୟ luaF_unlinkupval (UpVal *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC ଶୂନ୍ୟ luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ଅଚଳ ଅକ୍ଷର *luaF_getlocalname (ଅଚଳ Proto *func,
                                         unsigned local_ସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc);


#endif
