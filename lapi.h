/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#অন্তর্ভাৱ "llimits.h"
#অন্তর্ভাৱ "lstate.h"


#যদি defined(LUA_USE_APICHECK)
#অন্তর্ভাৱ <assert.h>
#define api_check(l,e,বাৰ্তা)	প্রতিজ্ঞা(e)
#অন্যথা	/* for testing */
#define api_check(l,e,বাৰ্তা)	((শূন্য)(l), lua_assert((e) && বাৰ্তা))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_শীৰ্ষ(L)  \
    (L->শীৰ্ষ.p++, api_check(L, L->শীৰ্ষ.p <= L->ci->শীৰ্ষ.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#যদি !defined(lua_lock)
#define lua_lock(L)	((শূন্য) 0)
#define lua_unlock(L)	((শূন্য) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { যদি ((nres) <= LUA_MULTRET && L->ci->শীৰ্ষ.p < L->শীৰ্ষ.p) \
	L->ci->শীৰ্ষ.p = L->শীৰ্ষ.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->শীৰ্ষ.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->শীৰ্ষ.p - L->ci->func.p &&  \
                     L->tbclist.p < L->শীৰ্ষ.p - (n), \
			  "not enough free elements in the stack")

#endif
