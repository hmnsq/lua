/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#ضمن "llimits.h"
#ضمن "lstate.h"


#إذا defined(LUA_USE_APICHECK)
#ضمن <assert.h>
#define api_check(l,e,پەیام)	أكد(e)
#وإلا	/* for testing */
#define api_check(l,e,پەیام)	((فراغ)(l), lua_assert((e) && پەیام))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_لووتکە(L)  \
    (L->لووتکە.p++, api_check(L, L->لووتکە.p <= L->ci->لووتکە.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#إذا !defined(lua_lock)
#define lua_lock(L)	((فراغ) 0)
#define lua_unlock(L)	((فراغ) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { إذا ((nres) <= LUA_MULTRET && L->ci->لووتکە.p < L->لووتکە.p) \
	L->ci->لووتکە.p = L->لووتکە.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->لووتکە.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->لووتکە.p - L->ci->func.p &&  \
                     L->tbclist.p < L->لووتکە.p - (n), \
			  "not enough free elements in the stack")

#endif
