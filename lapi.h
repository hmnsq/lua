/*
** $Id: lapi.h $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lapi_h
#घोषणा lapi_h


#समावेश "llimits.h"
#समावेश "lstate.h"


#अगर defined(LUA_USE_APICHECK)
#समावेश <assert.h>
#घोषणा api_check(l,e,संदेश)	निश्चित(e)
#अन्यथा	/* for testing */
#घोषणा api_check(l,e,संदेश)	((व्योम)(l), lua_assert((e) && संदेश))
#पूर्ण_अगर



/* Increments 'L->top.p', checking for stack overflows */
#घोषणा api_incr_शीर्ष(L)  \
    (L->शीर्ष.p++, api_check(L, L->शीर्ष.p <= L->ci->शीर्ष.p, "stack overflow"))


/*
** macros that are executed whenever program enters the Lua core
** ('lua_lock') and leaves the core ('lua_unlock')
*/
#अगर !defined(lua_lock)
#घोषणा lua_lock(L)	((व्योम) 0)
#घोषणा lua_unlock(L)	((व्योम) 0)
#पूर्ण_अगर



/*
** If a call returns too many multiple returns, the callee may not have
** stack space to accommodate all results. In this case, this macro
** increases its stack space ('L->ci->top.p').
*/
#घोषणा adjustresults(L,nres) \
    { अगर ((nres) <= LUA_MULTRET && L->ci->शीर्ष.p < L->शीर्ष.p) \
	L->ci->शीर्ष.p = L->शीर्ष.p; }


/* Ensure the stack has at least 'n' elements */
#घोषणा api_checknelems(L,n) \
       api_check(L, (n) < (L->शीर्ष.p - L->ci->func.p), \
                         "not enough elements in the stack")


/* Ensure the stack has at least 'n' elements to be popped. (Some
** functions only update a slot after checking it for popping, but that
** is only an optimization for a pop followed by a push.)
*/
#घोषणा api_checkpop(L,n) \
	api_check(L, (n) < L->शीर्ष.p - L->ci->func.p &&  \
                     L->tbclist.p < L->शीर्ष.p - (n), \
			  "not enough free elements in the stack")

#पूर्ण_अगर
