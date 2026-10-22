/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده lapi_h
#تعریف_کن lapi_h


#ضمن "llimits.h"
#ضمن "lstate.h"


#إذا defined(LUA_USE_APICHECK)
#ضمن <assert.h>
#تعریف_کن api_check(l,e,پیام)	أكد(e)
#وإلا	/* for testing */
#تعریف_کن api_check(l,e,پیام)	((فراغ)(l), lua_assert((e) && پیام))
#پایان_اگر



/* Increments 'L->top.p', checking for stack overflows */
#تعریف_کن api_incr_بالاترین(L)  \
    (L->بالاترین.p++, api_check(L, L->بالاترین.p <= L->ci->بالاترین.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#إذا !defined(lua_قفل)
#تعریف_کن lua_قفل(L)	((فراغ) 0)
#تعریف_کن lua_unlock(L)	((فراغ) 0)
#پایان_اگر



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#تعریف_کن adjustresults(L,nres) \
    { إذا ((nres) <= LUA_MULTRET && L->ci->بالاترین.p < L->بالاترین.p) \
	L->ci->بالاترین.p = L->بالاترین.p; }


/* Ensure the stack has at least 'n' elements */
#تعریف_کن api_checknelems(L,n) \
       api_check(L, (n) < (L->بالاترین.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#تعریف_کن api_checkpop(L,n) \
	api_check(L, (n) < L->بالاترین.p - L->ci->func.p &&  \
                     L->tbclist.p < L->بالاترین.p - (n), \
			  "not enough free elements in the stack")

#پایان_اگر
