/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#ଅନ୍ତର୍ଭାଵ "lstate.h"


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
#ଯଦି !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaG_getfuncline (ଅଚଳ Proto *f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc);
LUAI_FUNC ଅଚଳ ଅକ୍ଷର *luaG_findlocal (lua_State *L, CallInfo *ci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, ଅଚଳ TValue *o,
                                                ଅଚଳ ଅକ୍ଷର *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, ଅଚଳ TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, ଅଚଳ TValue *o,
                                               ଅଚଳ ଅକ୍ଷର *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, ଅଚଳ TValue *p1,
                                                  ଅଚଳ TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, ଅଚଳ TValue *p1,
                                                 ଅଚଳ TValue *p2,
                                                 ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, ଅଚଳ TValue *p1,
                                                 ଅଚଳ TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, ଅଚଳ TValue *p1,
                                                 ଅଚଳ TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, ପୂର୍ଣ୍ଣସଂଖ୍ୟା k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt, ...);
LUAI_FUNC ଅଚଳ ଅକ୍ଷର *luaG_addinfo (lua_State *L, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ,
                                                  TString *src, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaG_traceexec (lua_State *L, ଅଚଳ Instruction *pc);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaG_tracecall (lua_State *L);


#endif
