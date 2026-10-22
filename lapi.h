/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#அந்தர்பாவ "llimits.h"
#அந்தர்பாவ "lstate.h"


#எனில் defined(LUA_USE_APICHECK)
#அந்தர்பாவ <assert.h>
#define api_check(l,e,செய்தி)	ப்ரதிஜ்ஞா(e)
#இல்லையெனில்	/* for testing */
#define api_check(l,e,செய்தி)	((வெறுமை)(l), lua_assert((e) && செய்தி))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_உச்சி(L)  \
    (L->உச்சி.p++, api_check(L, L->உச்சி.p <= L->ci->உச்சி.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#எனில் !defined(lua_lock)
#define lua_lock(L)	((வெறுமை) 0)
#define lua_unlock(L)	((வெறுமை) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { எனில் ((nres) <= LUA_MULTRET && L->ci->உச்சி.p < L->உச்சி.p) \
	L->ci->உச்சி.p = L->உச்சி.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->உச்சி.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->உச்சி.p - L->ci->func.p &&  \
                     L->tbclist.p < L->உச்சி.p - (n), \
			  "not enough free elements in the stack")

#endif
