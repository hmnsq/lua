/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే ldo_h
#నిర్వచించు ldo_h


#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#ఒకవేళ !defined(HARDSTACKTESTS)
#నిర్వచించు condmovestack(L,pre,pos)	((శూన్య)0)
#లేకపోతే
/* realloc stack keeping its size */
#నిర్వచించు condmovestack(L,pre,pos)  \
  { పూర్ణాంకం sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#అయితే_ముగింపు

#నిర్వచించు luaD_checkstackaux(L,n,pre,pos)  \
	ఒకవేళ (l_unlikely(L->దొంతర_చివరి.p - L->శిఖరం.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	లేకపోతే { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#నిర్వచించు luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(శూన్య)0,(శూన్య)0)



#నిర్వచించు savestack(L,pt)		(cast_charp(pt) - cast_charp(L->దొంతర.p))
#నిర్వచించు restorestack(L,n)	cast(StkId, cast_charp(L->దొంతర.p) + (n))


/* macro to check stack size, preserving 'p' */
#నిర్వచించు checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/*
** Maximum depth for nested C calls, syntactical nested non-terminals,
** and other features implemented through recursion in C. (Value must
** fit in a 16-bit unsigned integer. It must also be compatible with
** the size of the C stack.)
*/
#ఒకవేళ !defined(LUAI_MAXCCALLS)
#నిర్వచించు LUAI_MAXCCALLS		200
#అయితే_ముగింపు


/* type of protected functions, to be ran by 'runprotected' */
రకనిర్వచనం శూన్య (*Pfunc) (lua_State *L, శూన్య *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC శూన్య luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  స్థిరరాశి అక్షరం *name,
                                                  స్థిరరాశి అక్షరం *బహుళకం);
LUAI_FUNC శూన్య luaD_hook (lua_State *L, పూర్ణాంకం event, పూర్ణాంకం పంక్తి,
                                        పూర్ణాంకం fTransfer, పూర్ణాంకం nTransfer);
LUAI_FUNC శూన్య luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC పూర్ణాంకం luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              పూర్ణాంకం narg1, పూర్ణాంకం delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, పూర్ణాంకం nResults);
LUAI_FUNC శూన్య luaD_call (lua_State *L, StkId func, పూర్ణాంకం nResults);
LUAI_FUNC శూన్య luaD_callnoyield (lua_State *L, StkId func, పూర్ణాంకం nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus స్థితి_2);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, శూన్య *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC శూన్య luaD_poscall (lua_State *L, CallInfo *ci, పూర్ణాంకం nres);
LUAI_FUNC పూర్ణాంకం luaD_reallocstack (lua_State *L, పూర్ణాంకం newsize, పూర్ణాంకం raiseerror);
LUAI_FUNC పూర్ణాంకం luaD_growstack (lua_State *L, పూర్ణాంకం n, పూర్ణాంకం raiseerror);
LUAI_FUNC శూన్య luaD_shrinkstack (lua_State *L);
LUAI_FUNC పూర్ణాంకం luaD_checkminstack (lua_State *L);
LUAI_FUNC శూన్య luaD_anchorobj (lua_State *L, పట్టిక *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, శూన్య *ud);

#అయితే_ముగింపు

