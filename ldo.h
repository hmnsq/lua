/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ldo_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ldo_h


#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#ਜੇ !defined(HARDSTACKTESTS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ condmovestack(L,pre,pos)	((ਖਾਲੀ)0)
#ਨਹੀਂਤਾਂ
/* realloc stack keeping its size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ condmovestack(L,pre,pos)  \
  { ਪੂਰਨ sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaD_checkstackaux(L,n,pre,pos)  \
	ਜੇ (l_unlikely(L->ਢੇਰ_ਆਖਰੀ.p - L->ਸਿਖਰ.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	ਨਹੀਂਤਾਂ { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(ਖਾਲੀ)0,(ਖਾਲੀ)0)



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ savestack(L,pt)		(cast_charp(pt) - cast_charp(L->ਢੇਰ.p))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ restorestack(L,n)	cast(StkId, cast_charp(L->ਢੇਰ.p) + (n))


/* macro to check stack size, preserving 'p' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/*
** Maximum depth for nested C calls, syntactical nested non-terminals,
** and other features implemented through recursion in C. (Value must
** fit in a 16-bit unsigned integer. It must also be compatible with
** the size of the C stack.)
*/
#ਜੇ !defined(LUAI_MAXCCALLS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_MAXCCALLS		200
#ਜੇ_ਅੰਤ


/* type of protected functions, to be ran by 'runprotected' */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*Pfunc) (lua_State *L, ਖਾਲੀ *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  ਅਚਲ ਅੱਖਰ *name,
                                                  ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ);
LUAI_FUNC ਖਾਲੀ luaD_hook (lua_State *L, ਪੂਰਨ event, ਪੂਰਨ ਸਤਰ,
                                        ਪੂਰਨ fTransfer, ਪੂਰਨ nTransfer);
LUAI_FUNC ਖਾਲੀ luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC ਪੂਰਨ luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              ਪੂਰਨ narg1, ਪੂਰਨ delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, ਪੂਰਨ nResults);
LUAI_FUNC ਖਾਲੀ luaD_call (lua_State *L, StkId func, ਪੂਰਨ nResults);
LUAI_FUNC ਖਾਲੀ luaD_callnoyield (lua_State *L, StkId func, ਪੂਰਨ nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus ਸਥਿਤੀ);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, ਖਾਲੀ *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC ਖਾਲੀ luaD_poscall (lua_State *L, CallInfo *ci, ਪੂਰਨ nres);
LUAI_FUNC ਪੂਰਨ luaD_reallocstack (lua_State *L, ਪੂਰਨ newsize, ਪੂਰਨ raiseerror);
LUAI_FUNC ਪੂਰਨ luaD_growstack (lua_State *L, ਪੂਰਨ n, ਪੂਰਨ raiseerror);
LUAI_FUNC ਖਾਲੀ luaD_shrinkstack (lua_State *L);
LUAI_FUNC ਪੂਰਨ luaD_checkminstack (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaD_anchorobj (lua_State *L, ਸਾਰਣੀ *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, ਖਾਲੀ *ud);

#ਜੇ_ਅੰਤ

