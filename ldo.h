/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#अगर_अघोषित ldo_h
#घोषणा ldo_h


#समावेश "llimits.h"
#समावेश "lobject.h"
#समावेश "lstate.h"
#समावेश "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#अगर !defined(HARDSTACKTESTS)
#घोषणा condmovestack(L,pre,pos)	((व्योम)0)
#अन्यथा
/* realloc stack keeping its size */
#घोषणा condmovestack(L,pre,pos)  \
  { पूर्णांक sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#पूर्ण_अगर

#घोषणा luaD_checkstackaux(L,n,pre,pos)  \
	अगर (l_unlikely(L->ढेर_last.p - L->शीर्ष.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	अन्यथा { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#घोषणा luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(व्योम)0,(व्योम)0)



#घोषणा savestack(L,pt)		(cast_charp(pt) - cast_charp(L->ढेर.p))
#घोषणा restorestack(L,n)	cast(StkId, cast_charp(L->ढेर.p) + (n))


/* macro to check stack size, preserving 'p' */
#घोषणा checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    सूचक_भेद_प्रकार t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/*
** Maximum depth for nested C calls, syntactical nested non-terminals,
** and other features implemented through recursion in C. (Value must
** fit in a 16-bit unsigned integer. It must also be compatible with
** the size of the C stack.)
*/
#अगर !defined(LUAI_MAXCCALLS)
#घोषणा LUAI_MAXCCALLS		200
#पूर्ण_अगर


/* type of protected functions, to be ran by 'runprotected' */
प्रकार व्योम (*Pfunc) (lua_State *L, व्योम *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC व्योम luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  स्थिर अक्षर *name,
                                                  स्थिर अक्षर *mode);
LUAI_FUNC व्योम luaD_hook (lua_State *L, पूर्णांक event, पूर्णांक पंक्ति,
                                        पूर्णांक fTransfer, पूर्णांक nTransfer);
LUAI_FUNC व्योम luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC पूर्णांक luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              पूर्णांक narg1, पूर्णांक delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, पूर्णांक nResults);
LUAI_FUNC व्योम luaD_call (lua_State *L, StkId func, पूर्णांक nResults);
LUAI_FUNC व्योम luaD_callnoyield (lua_State *L, StkId func, पूर्णांक nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, सूचक_भेद_प्रकार level,
                                                     TStatus status);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, व्योम *u,
                                        सूचक_भेद_प्रकार oldtop, सूचक_भेद_प्रकार ef);
LUAI_FUNC व्योम luaD_poscall (lua_State *L, CallInfo *ci, पूर्णांक nres);
LUAI_FUNC पूर्णांक luaD_reallocstack (lua_State *L, पूर्णांक newsize, पूर्णांक raiseerror);
LUAI_FUNC पूर्णांक luaD_growstack (lua_State *L, पूर्णांक n, पूर्णांक raiseerror);
LUAI_FUNC व्योम luaD_shrinkstack (lua_State *L);
LUAI_FUNC पूर्णांक luaD_checkminstack (lua_State *L);
LUAI_FUNC व्योम luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, व्योम *ud);

#पूर्ण_अगर

