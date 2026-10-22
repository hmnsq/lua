/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lfunc_h
#నిర్వచించు lfunc_h


#అన్తర్భావ "lobject.h"


#నిర్వచించు కొలత_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + పరిమాణం(TValue) * cast_uint(n))

#నిర్వచించు కొలత_Lclosure(n)  \
	(offsetof(LClosure, upvals) + పరిమాణం(పైకి_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#నిర్వచించు isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#నిర్వచించు MAXUPVAL	255


#నిర్వచించు upisopen(పైకి)	((పైకి)->v.p != &(పైకి)->u.విలువ)


#నిర్వచించు uplevel(పైకి)	check_exp(upisopen(పైకి), cast(StkId, (పైకి)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#నిర్వచించు MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#నిర్వచించు CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, పూర్ణాంకం nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, పూర్ణాంకం nupvals);
LUAI_FUNC శూన్య luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC పైకి_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC శూన్య luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC శూన్య luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus స్థితి_2, పూర్ణాంకం yy);
LUAI_FUNC శూన్య luaF_unlinkupval (పైకి_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC శూన్య luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC స్థిరరాశి అక్షరం *luaF_getlocalname (స్థిరరాశి Proto *func,
                                         గుర్తులేని local_సంఖ్య, పూర్ణాంకం pc);


#అయితే_ముగింపు
