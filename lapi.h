/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#شامل "llimits.h"
#شامل "lstate.h"


#اگر defined(LUA_USE_APICHECK)
#شامل <assert.h>
#define api_check(l,e,پیام)	أكد(e)
#وگرنه	/* for testing */
#define api_check(l,e,پیام)	((تهی)(l), lua_assert((e) && پیام))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_بالاترین(L)  \
    (L->بالاترین.p++, api_check(L, L->بالاترین.p <= L->ci->بالاترین.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#اگر !defined(lua_lock)
#define lua_lock(L)	((تهی) 0)
#define lua_unlock(L)	((تهی) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { اگر ((nres) <= LUA_MULTRET && L->ci->بالاترین.p < L->بالاترین.p) \
	L->ci->بالاترین.p = L->بالاترین.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->بالاترین.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->بالاترین.p - L->ci->func.p &&  \
                     L->tbclist.p < L->بالاترین.p - (n), \
			  "not enough free elements in the stack")

#endif
