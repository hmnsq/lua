/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef ldo_h
#define ldo_h


#അന്തര്ഭാവ "llimits.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#എങ്കിൽ !defined(HARDSTACKTESTS)
#define condmovestack(L,pre,pos)	((ശൂന്യം)0)
#അല്ലെങ്കിൽ
/* realloc stack keeping its size */
#define condmovestack(L,pre,pos)  \
  { പൂർണ്ണസംഖ്യ sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#endif

#define luaD_checkstackaux(L,n,pre,pos)  \
	എങ്കിൽ (l_unlikely(L->അട്ടി_last.p - L->മുകൾ.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	അല്ലെങ്കിൽ { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(ശൂന്യം)0,(ശൂന്യം)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->അട്ടി.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->അട്ടി.p) + (n))


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
#എങ്കിൽ !defined(LUAI_MAXCCALLS)
#define LUAI_MAXCCALLS		200
#endif


/* type of protected functions, to be ran by 'runprotected' */
തരനിർവചനം ശൂന്യം (*Pfunc) (lua_State *L, ശൂന്യം *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC ശൂന്യം luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  സ്ഥിരം അക്ഷരം *name,
                                                  സ്ഥിരം അക്ഷരം *mode);
LUAI_FUNC ശൂന്യം luaD_hook (lua_State *L, പൂർണ്ണസംഖ്യ event, പൂർണ്ണസംഖ്യ വരി,
                                        പൂർണ്ണസംഖ്യ fTransfer, പൂർണ്ണസംഖ്യ nTransfer);
LUAI_FUNC ശൂന്യം luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              പൂർണ്ണസംഖ്യ narg1, പൂർണ്ണസംഖ്യ delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, പൂർണ്ണസംഖ്യ nResults);
LUAI_FUNC ശൂന്യം luaD_call (lua_State *L, StkId func, പൂർണ്ണസംഖ്യ nResults);
LUAI_FUNC ശൂന്യം luaD_callnoyield (lua_State *L, StkId func, പൂർണ്ണസംഖ്യ nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus status);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, ശൂന്യം *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC ശൂന്യം luaD_poscall (lua_State *L, CallInfo *ci, പൂർണ്ണസംഖ്യ nres);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaD_reallocstack (lua_State *L, പൂർണ്ണസംഖ്യ newsize, പൂർണ്ണസംഖ്യ raiseerror);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaD_growstack (lua_State *L, പൂർണ്ണസംഖ്യ n, പൂർണ്ണസംഖ്യ raiseerror);
LUAI_FUNC ശൂന്യം luaD_shrinkstack (lua_State *L);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaD_checkminstack (lua_State *L);
LUAI_FUNC ശൂന്യം luaD_anchorobj (lua_State *L, Table *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, ശൂന്യം *ud);

#endif

