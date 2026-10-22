/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ldebug_h
#ವ್ಯಾಖ್ಯಾನಿಸು ldebug_h


#ಅನ್ತರ್ಭಾವ "lstate.h"


#ವ್ಯಾಖ್ಯಾನಿಸು pcRel(pc, p)	(cast_int((pc) - (p)->ಸಂಕೇತ_2) - 1)


/* Active Lua function (given call info) */
#ವ್ಯಾಖ್ಯಾನಿಸು ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#ವ್ಯಾಖ್ಯಾನಿಸು resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#ಆದರೆ !defined(MAXIWTHABS)
#ವ್ಯಾಖ್ಯಾನಿಸು MAXIWTHABS	128
#ಆದರೆ_ಅಂತ್ಯ


LUAI_FUNC ಪೂರ್ಣಾಂಕ luaG_getfuncline (ಸ್ಥಿರಾಂಕ Proto *f, ಪೂರ್ಣಾಂಕ pc);
LUAI_FUNC ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaG_findlocal (lua_State *L, CallInfo *ci, ಪೂರ್ಣಾಂಕ n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o,
                                                ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o,
                                               ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1,
                                                  ಸ್ಥಿರಾಂಕ TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1,
                                                 ಸ್ಥಿರಾಂಕ TValue *p2,
                                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1,
                                                 ಸ್ಥಿರಾಂಕ TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1,
                                                 ಸ್ಥಿರಾಂಕ TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, ಪೂರ್ಣಾಂಕ k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...);
LUAI_FUNC ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaG_addinfo (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ,
                                                  TString *src, ಪೂರ್ಣಾಂಕ ಸಾಲು);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaG_traceexec (lua_State *L, ಸ್ಥಿರಾಂಕ Instruction *pc);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaG_tracecall (lua_State *L);


#ಆದರೆ_ಅಂತ್ಯ
