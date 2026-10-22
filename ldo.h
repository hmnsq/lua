/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् ldo_h
#परिभाषय ldo_h


#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#यदि !defined(HARDSTACKTESTS)
#परिभाषय condmovestack(L,pre,pos)	((शून्य)0)
#अन्यथा
/* realloc stack keeping its size */
#परिभाषय condmovestack(L,pre,pos)  \
  { पूर्णाङ्क sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#यदि_अन्तः

#परिभाषय luaD_checkstackaux(L,n,pre,pos)  \
	यदि (l_unlikely(L->राशिः_चरमः.p - L->शिखरम्.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	अन्यथा { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#परिभाषय luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(शून्य)0,(शून्य)0)



#परिभाषय savestack(L,pt)		(cast_charp(pt) - cast_charp(L->राशिः.p))
#परिभाषय restorestack(L,n)	cast(StkId, cast_charp(L->राशिः.p) + (n))


/* macro to check stack size, preserving 'p' */
#परिभाषय checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/*
** Maximum depth for nested C calls, syntactical nested non-terminals,
** and other features implemented through recursion in C. (Value must
** fit in a 16-bit unsigned integer. It must also be compatible with
** the size of the C stack.)
*/
#यदि !defined(LUAI_MAXCCALLS)
#परिभाषय LUAI_MAXCCALLS		200
#यदि_अन्तः


/* type of protected functions, to be ran by 'runprotected' */
प्रकारनिर्देश शून्य (*Pfunc) (lua_State *L, शून्य *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC शून्य luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  ध्रुव वर्ण *name,
                                                  ध्रुव वर्ण *बहुलकम्);
LUAI_FUNC शून्य luaD_hook (lua_State *L, पूर्णाङ्क event, पूर्णाङ्क पङ्क्तिः,
                                        पूर्णाङ्क fTransfer, पूर्णाङ्क nTransfer);
LUAI_FUNC शून्य luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC पूर्णाङ्क luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              पूर्णाङ्क narg1, पूर्णाङ्क delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, पूर्णाङ्क nResults);
LUAI_FUNC शून्य luaD_call (lua_State *L, StkId func, पूर्णाङ्क nResults);
LUAI_FUNC शून्य luaD_callnoyield (lua_State *L, StkId func, पूर्णाङ्क nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus स्थितिः);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, शून्य *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC शून्य luaD_poscall (lua_State *L, CallInfo *ci, पूर्णाङ्क nres);
LUAI_FUNC पूर्णाङ्क luaD_reallocstack (lua_State *L, पूर्णाङ्क newsize, पूर्णाङ्क raiseerror);
LUAI_FUNC पूर्णाङ्क luaD_growstack (lua_State *L, पूर्णाङ्क n, पूर्णाङ्क raiseerror);
LUAI_FUNC शून्य luaD_shrinkstack (lua_State *L);
LUAI_FUNC पूर्णाङ्क luaD_checkminstack (lua_State *L);
LUAI_FUNC शून्य luaD_anchorobj (lua_State *L, सारणी *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, शून्य *ud);

#यदि_अन्तः

