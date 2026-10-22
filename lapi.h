/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lapi_h
#تعریف_کرو lapi_h


#شامل "llimits.h"
#شامل "lstate.h"


#اگر defined(LUA_USE_APICHECK)
#شامل <assert.h>
#تعریف_کرو api_check(l,e,پیغام)	أكد(e)
#ورنہ	/* for testing */
#تعریف_کرو api_check(l,e,پیغام)	((خالی)(l), lua_assert((e) && پیغام))
#اگر_ختم



/* Increments 'L->top.p', checking for stack overflows */
#تعریف_کرو api_incr_چوٹی(L)  \
    (L->چوٹی.p++, api_check(L, L->چوٹی.p <= L->ci->چوٹی.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#اگر !defined(lua_تالا)
#تعریف_کرو lua_تالا(L)	((خالی) 0)
#تعریف_کرو lua_unlock(L)	((خالی) 0)
#اگر_ختم



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#تعریف_کرو adjustresults(L,nres) \
    { اگر ((nres) <= LUA_MULTRET && L->ci->چوٹی.p < L->چوٹی.p) \
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

#اگر_ختم
