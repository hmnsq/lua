/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#അന്തര്ഭാവ "llimits.h"
#അന്തര്ഭാവ "lstate.h"


#എങ്കിൽ defined(LUA_USE_APICHECK)
#അന്തര്ഭാവ <assert.h>
#define api_check(l,e,സന്ദേശം)	പ്രതിജ്ഞാ(e)
#അല്ലെങ്കിൽ	/* for testing */
#define api_check(l,e,സന്ദേശം)	((ശൂന്യം)(l), lua_assert((e) && സന്ദേശം))
#endif



/* Increments 'L->top.p', checking for stack overflows */
#define api_incr_മുകൾ(L)  \
    (L->മുകൾ.p++, api_check(L, L->മുകൾ.p <= L->ci->മുകൾ.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#എങ്കിൽ !defined(lua_lock)
#define lua_lock(L)	((ശൂന്യം) 0)
#define lua_unlock(L)	((ശൂന്യം) 0)
#endif



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#define adjustresults(L,nres) \
    { എങ്കിൽ ((nres) <= LUA_MULTRET && L->ci->മുകൾ.p < L->മുകൾ.p) \
	L->ci->മുകൾ.p = L->മുകൾ.p; }


/* Ensure the stack has at least 'n' elements */
#define api_checknelems(L,n) \
       api_check(L, (n) < (L->മുകൾ.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#define api_checkpop(L,n) \
	api_check(L, (n) < L->മുകൾ.p - L->ci->func.p &&  \
                     L->tbclist.p < L->മുകൾ.p - (n), \
			  "not enough free elements in the stack")

#endif
