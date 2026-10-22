/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#অন্তর্ভাৱ "llimits.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lstate.h"
#অন্তর্ভাৱ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#যদি !defined(HARDSTACKTESTS)
#define condmovestack(L,pre,pos)	((শূন্য)0)
#অন্যথা
/* realloc stack keeping its size */
#define condmovestack(L,pre,pos)  \
  { পূর্ণাঙ্ক sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#endif

#define luaD_checkstackaux(L,n,pre,pos)  \
	যদি (l_unlikely(L->স্তূপ_last.p - L->শীৰ্ষ.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	অন্যথা { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(শূন্য)0,(শূন্য)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->স্তূপ.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->স্তূপ.p) + (n))


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
#যদি !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS		200
#endif


/* type of protected functions, to be ran by 'runprotected' */
প্রকারনির্দেশ শূন্য (*Pfunc) (lua_State *L, শূন্য *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC শূন্য luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  ধ্রুৱ ৱর্ণ *name,
                                                  ধ্রুৱ ৱর্ণ *mode);
LUAI_FUNC শূন্য luaD_hook (lua_State *L, পূর্ণাঙ্ক event, পূর্ণাঙ্ক সাৰি,
                                        পূর্ণাঙ্ক fTransfer, পূর্ণাঙ্ক nTransfer);
LUAI_FUNC শূন্য luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC পূর্ণাঙ্ক luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              পূর্ণাঙ্ক narg1, পূর্ণাঙ্ক delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, পূর্ণাঙ্ক nResults);
LUAI_FUNC শূন্য luaD_call (lua_State *L, StkId func, পূর্ণাঙ্ক nResults);
LUAI_FUNC শূন্য luaD_callnoyield (lua_State *L, StkId func, পূর্ণাঙ্ক nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus status);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, শূন্য *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC শূন্য luaD_poscall (lua_State *L, CallInfo *ci, পূর্ণাঙ্ক nres);
LUAI_FUNC পূর্ণাঙ্ক luaD_reallocstack (lua_State *L, পূর্ণাঙ্ক newsize, পূর্ণাঙ্ক raiseerror);
LUAI_FUNC পূর্ণাঙ্ক luaD_growstack (lua_State *L, পূর্ণাঙ্ক n, পূর্ণাঙ্ক raiseerror);
LUAI_FUNC শূন্য luaD_shrinkstack (lua_State *L);
LUAI_FUNC পূর্ণাঙ্ক luaD_checkminstack (lua_State *L);
LUAI_FUNC শূন্য luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, শূন্য *ud);

#endif

