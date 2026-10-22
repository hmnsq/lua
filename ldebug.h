/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే ldebug_h
#నిర్వచించు ldebug_h


#అన్తర్భావ "lstate.h"


#నిర్వచించు pcRel(pc, p)	(cast_int((pc) - (p)->సంకేతం_2) - 1)


/* Active Lua function (given call info) */
#నిర్వచించు ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#నిర్వచించు resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#నిర్వచించు ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#ఒకవేళ !defined(MAXIWTHABS)
#నిర్వచించు MAXIWTHABS	128
#అయితే_ముగింపు


LUAI_FUNC పూర్ణాంకం luaG_getfuncline (స్థిరరాశి Proto *f, పూర్ణాంకం pc);
LUAI_FUNC స్థిరరాశి అక్షరం *luaG_findlocal (lua_State *L, CallInfo *ci, పూర్ణాంకం n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, స్థిరరాశి TValue *o,
                                                స్థిరరాశి అక్షరం *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, స్థిరరాశి TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, స్థిరరాశి TValue *o,
                                               స్థిరరాశి అక్షరం *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, స్థిరరాశి TValue *p1,
                                                  స్థిరరాశి TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, స్థిరరాశి TValue *p1,
                                                 స్థిరరాశి TValue *p2,
                                                 స్థిరరాశి అక్షరం *సందేశం);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, స్థిరరాశి TValue *p1,
                                                 స్థిరరాశి TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, స్థిరరాశి TValue *p1,
                                                 స్థిరరాశి TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, పూర్ణాంకం k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...);
LUAI_FUNC స్థిరరాశి అక్షరం *luaG_addinfo (lua_State *L, స్థిరరాశి అక్షరం *సందేశం,
                                                  TString *src, పూర్ణాంకం పంక్తి);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC పూర్ణాంకం luaG_traceexec (lua_State *L, స్థిరరాశి Instruction *pc);
LUAI_FUNC పూర్ణాంకం luaG_tracecall (lua_State *L);


#అయితే_ముగింపు
