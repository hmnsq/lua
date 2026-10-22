/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lapi_h
#وصف_ڪريو lapi_h


#ضمن "llimits.h"
#ضمن "lstate.h"


#إذا defined(LUA_USE_APICHECK)
#ضمن <assert.h>
#وصف_ڪريو api_check(l,e,نياپو)	أكد(e)
#وإلا	/* for testing */
#وصف_ڪريو api_check(l,e,نياپو)	((فراغ)(l), lua_assert((e) && نياپو))
#جيڪڏهن_پورو



/* Increments 'L->top.p', checking for stack overflows */
#وصف_ڪريو api_incr_چوٽي(L)  \
    (L->چوٽي.p++, api_check(L, L->چوٽي.p <= L->ci->چوٽي.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#إذا !defined(lua_تالو)
#وصف_ڪريو lua_تالو(L)	((فراغ) 0)
#وصف_ڪريو lua_unlock(L)	((فراغ) 0)
#جيڪڏهن_پورو



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#وصف_ڪريو adjustresults(L,nres) \
    { إذا ((nres) <= LUA_MULTRET && L->ci->چوٽي.p < L->چوٽي.p) \
	L->ci->چوٽي.p = L->چوٽي.p; }


/* Ensure the stack has at least 'n' elements */
#وصف_ڪريو api_checknelems(L,n) \
       api_check(L, (n) < (L->چوٽي.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#وصف_ڪريو api_checkpop(L,n) \
	api_check(L, (n) < L->چوٽي.p - L->ci->func.p &&  \
                     L->tbclist.p < L->چوٽي.p - (n), \
			  "not enough free elements in the stack")

#جيڪڏهن_پورو
