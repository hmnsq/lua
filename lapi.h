/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"


#ಯದಿ defined(LUA_USE_APICHECK)
#ಅನ್ತರ್ಭಾವ <assert.h>
#define api_check(l,e,ಸಂದೇಶ)	ಪ್ರತಿಜ್ಞಾ(e)
#ಅನ್ಯಥಾ	/* for testing */
#define api_check(l,e,ಸಂದೇಶ)	((ಶೂನ್ಯ)(l), lua_assert((e) && ಸಂದೇಶ))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_ತುದಿ(L)  \
    (L->ತುದಿ.p++, api_check(L, L->ತುದಿ.p <= L->ci->ತುದಿ.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#ಯದಿ !defined(lua_lock)
#define lua_lock(L)	((ಶೂನ್ಯ) 0)
#define lua_unlock(L)	((ಶೂನ್ಯ) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { ಯದಿ ((nres) <= LUA_MULTRET && L->ci->ತುದಿ.p < L->ತುದಿ.p) \
	L->ci->ತುದಿ.p = L->ತುದಿ.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->ತುದಿ.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->ತುದಿ.p - L->ci->func.p &&  \
                     L->tbclist.p < L->ತುದಿ.p - (n), \
			  "not enough free elements in the stack")

#endif
