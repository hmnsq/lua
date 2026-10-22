/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#جے_بنا_تعریف lapi_h
#تعریف_کرو lapi_h


#ضمن "llimits.h"
#ضمن "lstate.h"


#إذا defined(LUA_USE_APICHECK)
#ضمن <assert.h>
#تعریف_کرو api_check(l,e,پیغام)	أكد(e)
#وإلا	/* for testing */
#تعریف_کرو api_check(l,e,پیغام)	((فراغ)(l), lua_assert((e) && پیغام))
#جے_انت



/* Increments 'L->top.p', checking for stack overflows */
#تعریف_کرو api_incr_چوٹی(L)  \
    (L->چوٹی.p++, api_check(L, L->چوٹی.p <= L->ci->چوٹی.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#إذا !defined(lua_جندرا)
#تعریف_کرو lua_جندرا(L)	((فراغ) 0)
#تعریف_کرو lua_unlock(L)	((فراغ) 0)
#جے_انت



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#تعریف_کرو adjustresults(L,nres) \
    { إذا ((nres) <= LUA_MULTRET && L->ci->چوٹی.p < L->چوٹی.p) \
	L->ci->چوٹی.p = L->چوٹی.p; }


/* Ensure the stack has at least 'n' elements */
#تعریف_کرو api_checknelems(L,n) \
       api_check(L, (n) < (L->چوٹی.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#تعریف_کرو api_checkpop(L,n) \
	api_check(L, (n) < L->چوٹی.p - L->ci->func.p &&  \
                     L->tbclist.p < L->چوٹی.p - (n), \
			  "not enough free elements in the stack")

#جے_انت
