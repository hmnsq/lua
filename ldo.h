/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ ldo_h
#تعریف_کرو ldo_h


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
#تعریف_کرو condmovestack(L,pre,pos)	((خالی)0)
#ورنہ
/* realloc stack keeping its size */
#تعریف_کرو condmovestack(L,pre,pos)  \
  { صحیح sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#اگر_ختم

#تعریف_کرو luaD_checkstackaux(L,n,pre,pos)  \
	اگر (l_unlikely(L->ڈھیر_آخری.p - L->چوٹی.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	ورنہ { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#تعریف_کرو luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(خالی)0,(خالی)0)



#تعریف_کرو savestack(L,pt)		(cast_charp(pt) - cast_charp(L->ڈھیر.p))
#تعریف_کرو restorestack(L,n)	cast(StkId, cast_charp(L->ڈھیر.p) + (n))


/* macro to check stack size, preserving 'p' */
#تعریف_کرو checkstackp(L,n,p)  \
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
#تعریف_کرو LUAI_MAXCCALLS		200
#اگر_ختم


/* type of protected functions, to be ran by 'runprotected' */
قسمتعریف خالی (*Pfunc) (lua_State *L, خالی *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC خالی luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  ثابت حرف *name,
                                                  ثابت حرف *منوال);
LUAI_FUNC خالی luaD_hook (lua_State *L, صحیح event, صحیح سطر,
                                        صحیح fTransfer, صحیح nTransfer);
LUAI_FUNC خالی luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC صحیح luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              صحیح narg1, صحیح delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, صحیح nResults);
LUAI_FUNC خالی luaD_call (lua_State *L, StkId func, صحیح nResults);
LUAI_FUNC خالی luaD_callnoyield (lua_State *L, StkId func, صحیح nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus حالت_2);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, خالی *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC خالی luaD_poscall (lua_State *L, CallInfo *ci, صحیح nres);
LUAI_FUNC صحیح luaD_reallocstack (lua_State *L, صحیح newsize, صحیح raiseerror);
LUAI_FUNC صحیح luaD_growstack (lua_State *L, صحیح n, صحیح raiseerror);
LUAI_FUNC خالی luaD_shrinkstack (lua_State *L);
LUAI_FUNC صحیح luaD_checkminstack (lua_State *L);
LUAI_FUNC خالی luaD_anchorobj (lua_State *L, جدول *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, خالی *ud);

#اگر_ختم

