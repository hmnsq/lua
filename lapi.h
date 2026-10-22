/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#ଅନ୍ତର୍ଭାଵ "llimits.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"


#ଯଦି defined(LUA_USE_APICHECK)
#ଅନ୍ତର୍ଭାଵ <assert.h>
#define api_check(l,e,ସନ୍ଦେଶ)	ପ୍ରତିଜ୍ଞା(e)
#ଅନ୍ୟଥା	/* for testing */
#define api_check(l,e,ସନ୍ଦେଶ)	((ଶୂନ୍ୟ)(l), lua_assert((e) && ସନ୍ଦେଶ))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_ଶୀର୍ଷ(L)  \
    (L->ଶୀର୍ଷ.p++, api_check(L, L->ଶୀର୍ଷ.p <= L->ci->ଶୀର୍ଷ.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#ଯଦି !defined(lua_lock)
#define lua_lock(L)	((ଶୂନ୍ୟ) 0)
#define lua_unlock(L)	((ଶୂନ୍ୟ) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { ଯଦି ((nres) <= LUA_MULTRET && L->ci->ଶୀର୍ଷ.p < L->ଶୀର୍ଷ.p) \
	L->ci->ଶୀର୍ଷ.p = L->ଶୀର୍ଷ.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->ଶୀର୍ଷ.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->ଶୀର୍ଷ.p - L->ci->func.p &&  \
                     L->tbclist.p < L->ଶୀର୍ଷ.p - (n), \
			  "not enough free elements in the stack")

#endif
