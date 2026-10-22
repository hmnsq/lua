/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#include "lstate.h"


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
#ᯀᯮᯝ !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC puur_naa_nka luaG_getfuncline (const Proto *f, puur_naa_nka pc);
LUAI_FUNC const var_na *luaG_findlocal (lua_State *L, CallInfo *ci, puur_naa_nka n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, const TValue *o,
                                                const var_na *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, const TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, const TValue *o,
                                               const var_na *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, const TValue *p1,
                                                  const TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, const TValue *p1,
                                                 const TValue *p2,
                                                 const var_na *msg);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, const TValue *p1,
                                                 const TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, const TValue *p1,
                                                 const TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, puur_naa_nka k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, const var_na *fmt, ...);
LUAI_FUNC const var_na *luaG_addinfo (lua_State *L, const var_na *msg,
                                                  TString *src, puur_naa_nka line);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC puur_naa_nka luaG_traceexec (lua_State *L, const Instruction *pc);
LUAI_FUNC puur_naa_nka luaG_tracecall (lua_State *L);


#endif
