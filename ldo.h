/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#અન્તર્ભાવ "llimits.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#જો !defined(HARDSTACKTESTS)
#define condmovestack(L,pre,pos)	((શૂન્ય)0)
#નહીંતર
/* realloc stack keeping its size */
#define condmovestack(L,pre,pos)  \
  { પૂર્ણાંક sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#endif

#define luaD_checkstackaux(L,n,pre,pos)  \
	જો (l_unlikely(L->ઢગલો_last.p - L->ટોચ.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	નહીંતર { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(શૂન્ય)0,(શૂન્ય)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->ઢગલો.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->ઢગલો.p) + (n))


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
#જો !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS		200
#endif


/* type of protected functions, to be ran by 'runprotected' */
પ્રકારવ્યાખ્યા શૂન્ય (*Pfunc) (lua_State *L, શૂન્ય *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC શૂન્ય luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  અચલ અક્ષર *name,
                                                  અચલ અક્ષર *mode);
LUAI_FUNC શૂન્ય luaD_hook (lua_State *L, પૂર્ણાંક event, પૂર્ણાંક લીટી,
                                        પૂર્ણાંક fTransfer, પૂર્ણાંક nTransfer);
LUAI_FUNC શૂન્ય luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC પૂર્ણાંક luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              પૂર્ણાંક narg1, પૂર્ણાંક delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, પૂર્ણાંક nResults);
LUAI_FUNC શૂન્ય luaD_call (lua_State *L, StkId func, પૂર્ણાંક nResults);
LUAI_FUNC શૂન્ય luaD_callnoyield (lua_State *L, StkId func, પૂર્ણાંક nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus status);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, શૂન્ય *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC શૂન્ય luaD_poscall (lua_State *L, CallInfo *ci, પૂર્ણાંક nres);
LUAI_FUNC પૂર્ણાંક luaD_reallocstack (lua_State *L, પૂર્ણાંક newsize, પૂર્ણાંક raiseerror);
LUAI_FUNC પૂર્ણાંક luaD_growstack (lua_State *L, પૂર્ણાંક n, પૂર્ણાંક raiseerror);
LUAI_FUNC શૂન્ય luaD_shrinkstack (lua_State *L);
LUAI_FUNC પૂર્ણાંક luaD_checkminstack (lua_State *L);
LUAI_FUNC શૂન્ય luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, શૂન્ય *ud);

#endif

