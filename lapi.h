/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#અન્તર્ભાવ "llimits.h"
#અન્તર્ભાવ "lstate.h"


#જો defined(LUA_USE_APICHECK)
#અન્તર્ભાવ <assert.h>
#define api_check(l,e,સંદેશ)	પ્રતિજ્ઞા(e)
#નહીંતર	/* for testing */
#define api_check(l,e,સંદેશ)	((શૂન્ય)(l), lua_assert((e) && સંદેશ))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_ટોચ(L)  \
    (L->ટોચ.p++, api_check(L, L->ટોચ.p <= L->ci->ટોચ.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#જો !defined(lua_lock)
#define lua_lock(L)	((શૂન્ય) 0)
#define lua_unlock(L)	((શૂન્ય) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { જો ((nres) <= LUA_MULTRET && L->ci->ટોચ.p < L->ટોચ.p) \
	L->ci->ટોચ.p = L->ટોચ.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->ટોચ.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->ટોચ.p - L->ci->func.p &&  \
                     L->tbclist.p < L->ટોચ.p - (n), \
			  "not enough free elements in the stack")

#endif
