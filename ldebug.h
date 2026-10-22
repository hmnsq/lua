/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#અન્તર્ભાવ "lstate.h"


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
#જો !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC પૂર્ણાંક luaG_getfuncline (અચલ Proto *f, પૂર્ણાંક pc);
LUAI_FUNC અચલ અક્ષર *luaG_findlocal (lua_State *L, CallInfo *ci, પૂર્ણાંક n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, અચલ TValue *o,
                                                અચલ અક્ષર *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, અચલ TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, અચલ TValue *o,
                                               અચલ અક્ષર *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, અચલ TValue *p1,
                                                  અચલ TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, અચલ TValue *p1,
                                                 અચલ TValue *p2,
                                                 અચલ અક્ષર *સંદેશ);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, અચલ TValue *p1,
                                                 અચલ TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, અચલ TValue *p1,
                                                 અચલ TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, પૂર્ણાંક k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, અચલ અક્ષર *fmt, ...);
LUAI_FUNC અચલ અક્ષર *luaG_addinfo (lua_State *L, અચલ અક્ષર *સંદેશ,
                                                  TString *src, પૂર્ણાંક લીટી);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC પૂર્ણાંક luaG_traceexec (lua_State *L, અચલ Instruction *pc);
LUAI_FUNC પૂર્ણાંક luaG_tracecall (lua_State *L);


#endif
