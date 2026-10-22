/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lapi_h
#ವ್ಯಾಖ್ಯಾನಿಸು lapi_h


#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"


#ಆದರೆ defined(LUA_USE_APICHECK)
#ಅನ್ತರ್ಭಾವ <assert.h>
#ವ್ಯಾಖ್ಯಾನಿಸು api_check(l,e,ಸಂದೇಶ)	ಪ್ರತಿಜ್ಞಾ(e)
#ಇಲ್ಲದಿದ್ದರೆ	/* for testing */
#ವ್ಯಾಖ್ಯಾನಿಸು api_check(l,e,ಸಂದೇಶ)	((ಶೂನ್ಯ)(l), lua_assert((e) && ಸಂದೇಶ))
#ಆದರೆ_ಅಂತ್ಯ



/* Increments 'L->top.p', checking for stack overflows */
#ವ್ಯಾಖ್ಯಾನಿಸು api_incr_ತುದಿ(L)  \
    (L->ತುದಿ.p++, api_check(L, L->ತುದಿ.p <= L->ci->ತುದಿ.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#ಆದರೆ !defined(lua_ಬೀಗ)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_ಬೀಗ(L)	((ಶೂನ್ಯ) 0)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_unlock(L)	((ಶೂನ್ಯ) 0)
#ಆದರೆ_ಅಂತ್ಯ



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#ವ್ಯಾಖ್ಯಾನಿಸು adjustresults(L,nres) \
    { ಆದರೆ ((nres) <= LUA_MULTRET && L->ci->ತುದಿ.p < L->ತುದಿ.p) \
	L->ci->ತುದಿ.p = L->ತುದಿ.p; }


/* Ensure the stack has at least 'n' elements */
#ವ್ಯಾಖ್ಯಾನಿಸು api_checknelems(L,n) \
       api_check(L, (n) < (L->ತುದಿ.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು api_checkpop(L,n) \
	api_check(L, (n) < L->ತುದಿ.p - L->ci->func.p &&  \
                     L->tbclist.p < L->ತುದಿ.p - (n), \
			  "not enough free elements in the stack")

#ಆದರೆ_ಅಂತ್ಯ
