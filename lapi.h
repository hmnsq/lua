/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lapi_h
#నిర్వచించు lapi_h


#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lstate.h"


#ఒకవేళ defined(LUA_USE_APICHECK)
#అన్తర్భావ <assert.h>
#నిర్వచించు api_check(l,e,సందేశం)	ప్రతిజ్ఞా(e)
#లేకపోతే	/* for testing */
#నిర్వచించు api_check(l,e,సందేశం)	((శూన్య)(l), lua_assert((e) && సందేశం))
#అయితే_ముగింపు



/* Increments 'L->top.p', checking for stack overflows */
#నిర్వచించు api_incr_శిఖరం(L)  \
    (L->శిఖరం.p++, api_check(L, L->శిఖరం.p <= L->ci->శిఖరం.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#ఒకవేళ !defined(lua_తాళం)
#నిర్వచించు lua_తాళం(L)	((శూన్య) 0)
#నిర్వచించు lua_unlock(L)	((శూన్య) 0)
#అయితే_ముగింపు



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#నిర్వచించు adjustresults(L,nres) \
    { ఒకవేళ ((nres) <= LUA_MULTRET && L->ci->శిఖరం.p < L->శిఖరం.p) \
	L->ci->శిఖరం.p = L->శిఖరం.p; }


/* Ensure the stack has at least 'n' elements */
#నిర్వచించు api_checknelems(L,n) \
       api_check(L, (n) < (L->శిఖరం.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#నిర్వచించు api_checkpop(L,n) \
	api_check(L, (n) < L->శిఖరం.p - L->ci->func.p &&  \
                     L->tbclist.p < L->శిఖరం.p - (n), \
			  "not enough free elements in the stack")

#అయితే_ముగింపు
