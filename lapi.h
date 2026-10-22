/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی lapi_h
#تعریف_کړه lapi_h


#ضمن "llimits.h"
#ضمن "lstate.h"


#إذا defined(LUA_USE_APICHECK)
#ضمن <assert.h>
#تعریف_کړه api_check(l,e,پیغام)	أكد(e)
#وإلا	/* for testing */
#تعریف_کړه api_check(l,e,پیغام)	((فراغ)(l), lua_assert((e) && پیغام))
#د_که_پای



/* Increments 'L->top.p', checking for stack overflows */
#تعریف_کړه api_incr_سر(L)  \
    (L->سر.p++, api_check(L, L->سر.p <= L->ci->سر.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#إذا !defined(lua_قلف)
#تعریف_کړه lua_قلف(L)	((فراغ) 0)
#تعریف_کړه lua_unlock(L)	((فراغ) 0)
#د_که_پای



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#تعریف_کړه adjustresults(L,nres) \
    { إذا ((nres) <= LUA_MULTRET && L->ci->سر.p < L->سر.p) \
	L->ci->سر.p = L->سر.p; }


/* Ensure the stack has at least 'n' elements */
#تعریف_کړه api_checknelems(L,n) \
       api_check(L, (n) < (L->سر.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#تعریف_کړه api_checkpop(L,n) \
	api_check(L, (n) < L->سر.p - L->ci->func.p &&  \
                     L->tbclist.p < L->سر.p - (n), \
			  "not enough free elements in the stack")

#د_که_پای
