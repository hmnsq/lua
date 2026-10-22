/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् lapi_h
#परिभाषय lapi_h


#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lstate.h"


#यदि defined(LUA_USE_APICHECK)
#अन्तर्भाव <assert.h>
#परिभाषय api_check(l,e,सन्देशः)	प्रतिज्ञा(e)
#अन्यथा	/* for testing */
#परिभाषय api_check(l,e,सन्देशः)	((शून्य)(l), lua_assert((e) && सन्देशः))
#यदि_अन्तः



/* Increments 'L->top.p', checking for stack overflows */
#परिभाषय api_incr_शिखरम्(L)  \
    (L->शिखरम्.p++, api_check(L, L->शिखरम्.p <= L->ci->शिखरम्.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#यदि !defined(lua_तालकम्)
#परिभाषय lua_तालकम्(L)	((शून्य) 0)
#परिभाषय lua_unlock(L)	((शून्य) 0)
#यदि_अन्तः



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#परिभाषय adjustresults(L,nres) \
    { यदि ((nres) <= LUA_MULTRET && L->ci->शिखरम्.p < L->शिखरम्.p) \
	L->ci->शिखरम्.p = L->शिखरम्.p; }


/* Ensure the stack has at least 'n' elements */
#परिभाषय api_checknelems(L,n) \
       api_check(L, (n) < (L->शिखरम्.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#परिभाषय api_checkpop(L,n) \
	api_check(L, (n) < L->शिखरम्.p - L->ci->func.p &&  \
                     L->tbclist.p < L->शिखरम्.p - (n), \
			  "not enough free elements in the stack")

#यदि_अन्तः
