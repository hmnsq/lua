/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#அந்தர்பாவ "lstate.h"


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
#எனில் !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC முழு luaG_getfuncline (மாறிலி Proto *f, முழு pc);
LUAI_FUNC மாறிலி எழுத்து *luaG_findlocal (lua_State *L, CallInfo *ci, முழு n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, மாறிலி TValue *o,
                                                மாறிலி எழுத்து *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, மாறிலி TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, மாறிலி TValue *o,
                                               மாறிலி எழுத்து *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, மாறிலி TValue *p1,
                                                  மாறிலி TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, மாறிலி TValue *p1,
                                                 மாறிலி TValue *p2,
                                                 மாறிலி எழுத்து *செய்தி);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, மாறிலி TValue *p1,
                                                 மாறிலி TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, மாறிலி TValue *p1,
                                                 மாறிலி TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, முழு k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, மாறிலி எழுத்து *fmt, ...);
LUAI_FUNC மாறிலி எழுத்து *luaG_addinfo (lua_State *L, மாறிலி எழுத்து *செய்தி,
                                                  TString *src, முழு வரி);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC முழு luaG_traceexec (lua_State *L, மாறிலி Instruction *pc);
LUAI_FUNC முழு luaG_tracecall (lua_State *L);


#endif
