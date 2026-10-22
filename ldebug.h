/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل ldebug_h
#وصف_ڪريو ldebug_h


#ضمن "lstate.h"


#وصف_ڪريو pcRel(pc, p)	(cast_int((pc) - (p)->ڪوڊ) - 1)


/* Active Lua function (given call info) */
#وصف_ڪريو ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#وصف_ڪريو resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#وصف_ڪريو ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#إذا !defined(MAXIWTHABS)
#وصف_ڪريو MAXIWTHABS	128
#جيڪڏهن_پورو


LUAI_FUNC صحيح luaG_getfuncline (ثابت Proto *f, صحيح pc);
LUAI_FUNC ثابت حرف *luaG_findlocal (lua_State *L, CallInfo *ci, صحيح n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, ثابت TValue *o,
                                                ثابت حرف *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, ثابت TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, ثابت TValue *o,
                                               ثابت حرف *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, ثابت TValue *p1,
                                                  ثابت TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, ثابت TValue *p1,
                                                 ثابت TValue *p2,
                                                 ثابت حرف *نياپو);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, ثابت TValue *p1,
                                                 ثابت TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, ثابت TValue *p1,
                                                 ثابت TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, صحيح k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, ثابت حرف *fmt, ...);
LUAI_FUNC ثابت حرف *luaG_addinfo (lua_State *L, ثابت حرف *نياپو,
                                                  TString *src, صحيح سٽ);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC صحيح luaG_traceexec (lua_State *L, ثابت Instruction *pc);
LUAI_FUNC صحيح luaG_tracecall (lua_State *L);


#جيڪڏهن_پورو
