/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#ଅନ୍ତର୍ଭାଵ "llimits.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#ଯଦି !defined(HARDSTACKTESTS)
#define condmovestack(L,pre,pos)	((ଶୂନ୍ୟ)0)
#ଅନ୍ୟଥା
/* realloc stack keeping its size */
#define condmovestack(L,pre,pos)  \
  { ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#endif

#define luaD_checkstackaux(L,n,pre,pos)  \
	ଯଦି (l_unlikely(L->ଗଦା_last.p - L->ଶୀର୍ଷ.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	ଅନ୍ୟଥା { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(ଶୂନ୍ୟ)0,(ଶୂନ୍ୟ)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->ଗଦା.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->ଗଦା.p) + (n))


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
#ଯଦି !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS		200
#endif


/* type of protected functions, to be ran by 'runprotected' */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ (*Pfunc) (lua_State *L, ଶୂନ୍ୟ *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC ଶୂନ୍ୟ luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  ଅଚଳ ଅକ୍ଷର *name,
                                                  ଅଚଳ ଅକ୍ଷର *mode);
LUAI_FUNC ଶୂନ୍ୟ luaD_hook (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା event, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି,
                                        ପୂର୍ଣ୍ଣସଂଖ୍ୟା fTransfer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nTransfer);
LUAI_FUNC ଶୂନ୍ୟ luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              ପୂର୍ଣ୍ଣସଂଖ୍ୟା narg1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nResults);
LUAI_FUNC ଶୂନ୍ୟ luaD_call (lua_State *L, StkId func, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nResults);
LUAI_FUNC ଶୂନ୍ୟ luaD_callnoyield (lua_State *L, StkId func, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus status);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, ଶୂନ୍ୟ *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC ଶୂନ୍ୟ luaD_poscall (lua_State *L, CallInfo *ci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaD_reallocstack (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା newsize, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raiseerror);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaD_growstack (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା raiseerror);
LUAI_FUNC ଶୂନ୍ୟ luaD_shrinkstack (lua_State *L);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaD_checkminstack (lua_State *L);
LUAI_FUNC ଶୂନ୍ୟ luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, ଶୂନ୍ୟ *ud);

#endif

