/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#அந்தர்பாவ "llimits.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#எனில் !defined(HARDSTACKTESTS)
#define condmovestack(L,pre,pos)	((வெறுமை)0)
#இல்லையெனில்
/* realloc stack keeping its size */
#define condmovestack(L,pre,pos)  \
  { முழு sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#endif

#define luaD_checkstackaux(L,n,pre,pos)  \
	எனில் (l_unlikely(L->அடுக்கு_last.p - L->உச்சி.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	இல்லையெனில் { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(வெறுமை)0,(வெறுமை)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->அடுக்கு.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->அடுக்கு.p) + (n))


/* macro to check stack size, preserving 'p' */
#define checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/*
** Maximum depth for nested C calls, syntactical nested non-terminals,
** and other features implemented through recursion in C. (Value must
** fit in a 16-bit unsigned integer. It must also be compatible with
** the size of the C stack.)
*/
#எனில் !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS		200
#endif


/* type of protected functions, to be ran by 'runprotected' */
வகைவரை வெறுமை (*Pfunc) (lua_State *L, வெறுமை *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC வெறுமை luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  மாறிலி எழுத்து *name,
                                                  மாறிலி எழுத்து *mode);
LUAI_FUNC வெறுமை luaD_hook (lua_State *L, முழு event, முழு வரி,
                                        முழு fTransfer, முழு nTransfer);
LUAI_FUNC வெறுமை luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC முழு luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              முழு narg1, முழு delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, முழு nResults);
LUAI_FUNC வெறுமை luaD_call (lua_State *L, StkId func, முழு nResults);
LUAI_FUNC வெறுமை luaD_callnoyield (lua_State *L, StkId func, முழு nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus status);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, வெறுமை *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC வெறுமை luaD_poscall (lua_State *L, CallInfo *ci, முழு nres);
LUAI_FUNC முழு luaD_reallocstack (lua_State *L, முழு newsize, முழு raiseerror);
LUAI_FUNC முழு luaD_growstack (lua_State *L, முழு n, முழு raiseerror);
LUAI_FUNC வெறுமை luaD_shrinkstack (lua_State *L);
LUAI_FUNC முழு luaD_checkminstack (lua_State *L);
LUAI_FUNC வெறுமை luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, வெறுமை *ud);

#endif

