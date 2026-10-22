/*
** $Id: ldo.h $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ldo_h
#ವ್ಯಾಖ್ಯಾನಿಸು ldo_h


#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/

#ಆದರೆ !defined(HARDSTACKTESTS)
#ವ್ಯಾಖ್ಯಾನಿಸು condmovestack(L,pre,pos)	((ಶೂನ್ಯ)0)
#ಇಲ್ಲದಿದ್ದರೆ
/* realloc stack keeping its size */
#ವ್ಯಾಖ್ಯಾನಿಸು condmovestack(L,pre,pos)  \
  { ಪೂರ್ಣಾಂಕ sz_ = stacksize(L); pre; luaD_reallocstack((L), sz_, 0); pos; }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು luaD_checkstackaux(L,n,pre,pos)  \
	ಆದರೆ (l_unlikely(L->ರಾಶಿ_ಕೊನೆಯ.p - L->ತುದಿ.p <= (n))) \
	  { pre; luaD_growstack(L, n, 1); pos; } \
	ಇಲ್ಲದಿದ್ದರೆ { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#ವ್ಯಾಖ್ಯಾನಿಸು luaD_checkstack(L,n)	luaD_checkstackaux(L,n,(ಶೂನ್ಯ)0,(ಶೂನ್ಯ)0)



#ವ್ಯಾಖ್ಯಾನಿಸು savestack(L,pt)		(cast_charp(pt) - cast_charp(L->ರಾಶಿ.p))
#ವ್ಯಾಖ್ಯಾನಿಸು restorestack(L,n)	cast(StkId, cast_charp(L->ರಾಶಿ.p) + (n))


/* macro to check stack size, preserving 'p' */
#ವ್ಯಾಖ್ಯಾನಿಸು checkstackp(L,n,p)  \
  luaD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/*
** Maximum depth for nested C calls, syntactical nested non-terminals,
** and other features implemented through recursion in C. (Value must
** fit in a 16-bit unsigned integer. It must also be compatible with
** the size of the C stack.)
*/
#ಆದರೆ !defined(LUAI_MAXCCALLS)
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXCCALLS		200
#ಆದರೆ_ಅಂತ್ಯ


/* type of protected functions, to be ran by 'runprotected' */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*Pfunc) (lua_State *L, ಶೂನ್ಯ *ud);

LUAI_FUNC l_noret luaD_errerr (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaD_seterrorobj (lua_State *L, TStatus errcode, StkId oldtop);
LUAI_FUNC TStatus luaD_protectedparser (lua_State *L, ZIO *z,
                                                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                                                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ);
LUAI_FUNC ಶೂನ್ಯ luaD_hook (lua_State *L, ಪೂರ್ಣಾಂಕ event, ಪೂರ್ಣಾಂಕ ಸಾಲು,
                                        ಪೂರ್ಣಾಂಕ fTransfer, ಪೂರ್ಣಾಂಕ nTransfer);
LUAI_FUNC ಶೂನ್ಯ luaD_hookcall (lua_State *L, CallInfo *ci);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              ಪೂರ್ಣಾಂಕ narg1, ಪೂರ್ಣಾಂಕ delta);
LUAI_FUNC CallInfo *luaD_precall (lua_State *L, StkId func, ಪೂರ್ಣಾಂಕ nResults);
LUAI_FUNC ಶೂನ್ಯ luaD_call (lua_State *L, StkId func, ಪೂರ್ಣಾಂಕ nResults);
LUAI_FUNC ಶೂನ್ಯ luaD_callnoyield (lua_State *L, StkId func, ಪೂರ್ಣಾಂಕ nResults);
LUAI_FUNC TStatus luaD_closeprotected (lua_State *L, ptrdiff_t level,
                                                     TStatus ಸ್ಥಿತಿ_2);
LUAI_FUNC TStatus luaD_pcall (lua_State *L, Pfunc func, ಶೂನ್ಯ *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC ಶೂನ್ಯ luaD_poscall (lua_State *L, CallInfo *ci, ಪೂರ್ಣಾಂಕ nres);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaD_reallocstack (lua_State *L, ಪೂರ್ಣಾಂಕ newsize, ಪೂರ್ಣಾಂಕ raiseerror);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaD_growstack (lua_State *L, ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ raiseerror);
LUAI_FUNC ಶೂನ್ಯ luaD_shrinkstack (lua_State *L);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaD_checkminstack (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaD_anchorobj (lua_State *L, ಕೋಷ್ಟಕ *anchor, GCObject *obj);

LUAI_FUNC l_noret luaD_throw (lua_State *L, TStatus errcode);
LUAI_FUNC l_noret luaD_throwbaselevel (lua_State *L, TStatus errcode);
LUAI_FUNC TStatus luaD_rawrunprotected (lua_State *L, Pfunc f, ಶೂನ್ಯ *ud);

#ಆದರೆ_ಅಂತ್ಯ

