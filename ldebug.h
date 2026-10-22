/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#অন্তর্ভাব "lstate.h"


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
#যদি !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC পূর্ণসংখ্যা luaG_getfuncline (ধ্রুবক Proto *f, পূর্ণসংখ্যা pc);
LUAI_FUNC ধ্রুবক অক্ষর *luaG_findlocal (lua_State *L, CallInfo *ci, পূর্ণসংখ্যা n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, ধ্রুবক TValue *o,
                                                ধ্রুবক অক্ষর *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, ধ্রুবক TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, ধ্রুবক TValue *o,
                                               ধ্রুবক অক্ষর *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, ধ্রুবক TValue *p1,
                                                  ধ্রুবক TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, ধ্রুবক TValue *p1,
                                                 ধ্রুবক TValue *p2,
                                                 ধ্রুবক অক্ষর *বার্তা);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, ধ্রুবক TValue *p1,
                                                 ধ্রুবক TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, ধ্রুবক TValue *p1,
                                                 ধ্রুবক TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, পূর্ণসংখ্যা k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, ধ্রুবক অক্ষর *fmt, ...);
LUAI_FUNC ধ্রুবক অক্ষর *luaG_addinfo (lua_State *L, ধ্রুবক অক্ষর *বার্তা,
                                                  TString *src, পূর্ণসংখ্যা সারি);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC পূর্ণসংখ্যা luaG_traceexec (lua_State *L, ধ্রুবক Instruction *pc);
LUAI_FUNC পূর্ণসংখ্যা luaG_tracecall (lua_State *L);


#endif
