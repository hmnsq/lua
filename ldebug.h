/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#ಅನ್ತರ್ಭಾವ "lstate.h"


#define pcRel(pc, p)	(cast_int((pc) - (p)->code) - 1)


/* Active Lua function (given call info) */
#define ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#define resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#define ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#ಯದಿ !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaG_getfuncline (ಧ್ರುವ Proto *f, ಪೂರ್ಣಾಙ್ಕ pc);
LUAI_FUNC ಧ್ರುವ ವರ್ಣ *luaG_findlocal (lua_State *L, CallInfo *ci, ಪೂರ್ಣಾಙ್ಕ n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, ಧ್ರುವ TValue *o,
                                                ಧ್ರುವ ವರ್ಣ *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, ಧ್ರುವ TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, ಧ್ರುವ TValue *o,
                                               ಧ್ರುವ ವರ್ಣ *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, ಧ್ರುವ TValue *p1,
                                                  ಧ್ರುವ TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, ಧ್ರುವ TValue *p1,
                                                 ಧ್ರುವ TValue *p2,
                                                 ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, ಧ್ರುವ TValue *p1,
                                                 ಧ್ರುವ TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, ಧ್ರುವ TValue *p1,
                                                 ಧ್ರುವ TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, ಪೂರ್ಣಾಙ್ಕ k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...);
LUAI_FUNC ಧ್ರುವ ವರ್ಣ *luaG_addinfo (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ,
                                                  TString *src, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaG_traceexec (lua_State *L, ಧ್ರುವ Instruction *pc);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaG_tracecall (lua_State *L);


#endif
