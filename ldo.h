/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#شامل "llimits.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#اگر !defined(HARDSTACKTESTS)
#define condmovestack(L,pre,pos)	((تهی)0)
#وگرنه
/* realloc stack keeping its size */
#define condmovestack(L,pre,pos)  \
  { صحیح sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#endif

#define luaD_checkstackaux(L,n,pre,pos)  \
	اگر (l_unlikely(L->پشته_last.p - L->بالاترین.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	وگرنه { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(تهی)0,(تهی)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->پشته.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->پشته.p) + (n))


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
#اگر !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS		200
#endif


/* type of protected functions, to be ran by 'runprotected' */
تعریفنوع تهی (*Pfunc) (lua_State *L, تهی *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC تهی luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  ثابت نویسه *name,
                                                  ثابت نویسه *mode);
LUAI_FUNC تهی luaD_hook (lua_State *L, صحیح event, صحیح سطر,
                                        صحیح fTransfer, صحیح nTransfer);
LUAI_FUNC تهی luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC صحیح luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              صحیح narg1, صحیح delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, صحیح nResults);
LUAI_FUNC تهی luaD_call (lua_State *L, StkId func, صحیح nResults);
LUAI_FUNC تهی luaD_callnoyield (lua_State *L, StkId func, صحیح nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus status);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, تهی *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC تهی luaD_poscall (lua_State *L, CallInfo *ci, صحیح nres);
LUAI_FUNC صحیح luaD_reallocstack (lua_State *L, صحیح newsize, صحیح raiseerror);
LUAI_FUNC صحیح luaD_growstack (lua_State *L, صحیح n, صحیح raiseerror);
LUAI_FUNC تهی luaD_shrinkstack (lua_State *L);
LUAI_FUNC صحیح luaD_checkminstack (lua_State *L);
LUAI_FUNC تهی luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, تهی *ud);

#endif

