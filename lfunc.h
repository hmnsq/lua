/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lfunc_h
#ವ್ಯಾಖ್ಯಾನಿಸು lfunc_h


#ಅನ್ತರ್ಭಾವ "lobject.h"


#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_Cclosure(n)  \
	(offsetof(CClosure, upvalue) + ಗಾತ್ರ(TValue) * cast_uint(n))

#ವ್ಯಾಖ್ಯಾನಿಸು ಗಾತ್ರ_Lclosure(n)  \
	(offsetof(LClosure, upvals) + ಗಾತ್ರ(ಮೇಲೆ_Val *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#ವ್ಯಾಖ್ಯಾನಿಸು isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXUPVAL	255


#ವ್ಯಾಖ್ಯಾನಿಸು upisopen(ಮೇಲೆ)	((ಮೇಲೆ)->v.p != &(ಮೇಲೆ)->u.ಮೌಲ್ಯ)


#ವ್ಯಾಖ್ಯಾನಿಸು uplevel(ಮೇಲೆ)	check_exp(upisopen(ಮೇಲೆ), cast(StkId, (ಮೇಲೆ)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#ವ್ಯಾಖ್ಯಾನಿಸು CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, ಪೂರ್ಣಾಂಕ nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, ಪೂರ್ಣಾಂಕ nupvals);
LUAI_FUNC ಶೂನ್ಯ luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC ಮೇಲೆ_Val *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC ಶೂನ್ಯ luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC ಶೂನ್ಯ luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus ಸ್ಥಿತಿ_2, ಪೂರ್ಣಾಂಕ yy);
LUAI_FUNC ಶೂನ್ಯ luaF_unlinkupval (ಮೇಲೆ_Val *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC ಶೂನ್ಯ luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaF_getlocalname (ಸ್ಥಿರಾಂಕ Proto *func,
                                         ಚಿಹ್ನೆರಹಿತ local_ಸಂಖ್ಯೆ, ಪೂರ್ಣಾಂಕ pc);


#ಆದರೆ_ಅಂತ್ಯ
