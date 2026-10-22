/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lapi_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lapi_h


#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"


#ਜੇ defined(LUA_USE_APICHECK)
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ api_check(l,e,ਸੁਨੇਹਾ)	ਪ੍ਰਤਿਜ੍ਞਾ(e)
#ਨਹੀਂਤਾਂ	/* for testing */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ api_check(l,e,ਸੁਨੇਹਾ)	((ਖਾਲੀ)(l), lua_assert((e) && ਸੁਨੇਹਾ))
#ਜੇ_ਅੰਤ



/* Increments 'L->top.p', checking for stack overflows */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ api_incr_ਸਿਖਰ(L)  \
    (L->ਸਿਖਰ.p++, api_check(L, L->ਸਿਖਰ.p <= L->ci->ਸਿਖਰ.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#ਜੇ !defined(lua_ਤਾਲਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_ਤਾਲਾ(L)	((ਖਾਲੀ) 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_unlock(L)	((ਖਾਲੀ) 0)
#ਜੇ_ਅੰਤ



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ adjustresults(L,nres) \
    { ਜੇ ((nres) <= LUA_MULTRET && L->ci->ਸਿਖਰ.p < L->ਸਿਖਰ.p) \
	L->ci->ਸਿਖਰ.p = L->ਸਿਖਰ.p; }


/* Ensure the stack has at least 'n' elements */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ api_checknelems(L,n) \
       api_check(L, (n) < (L->ਸਿਖਰ.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ api_checkpop(L,n) \
	api_check(L, (n) < L->ਸਿਖਰ.p - L->ci->func.p &&  \
                     L->tbclist.p < L->ਸਿਖਰ.p - (n), \
			  "not enough free elements in the stack")

#ਜੇ_ਅੰਤ
