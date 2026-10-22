/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#अन्तर्भाव "lstate.h"


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
#यदि !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC पूर्णाङ्क luaG_getfuncline (अचर Proto *f, पूर्णाङ्क pc);
LUAI_FUNC अचर वर्ण *luaG_findlocal (lua_State *L, CallInfo *ci, पूर्णाङ्क n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, अचर TValue *o,
                                                अचर वर्ण *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, अचर TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, अचर TValue *o,
                                               अचर वर्ण *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, अचर TValue *p1,
                                                  अचर TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, अचर TValue *p1,
                                                 अचर TValue *p2,
                                                 अचर वर्ण *सन्देश);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, अचर TValue *p1,
                                                 अचर TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, अचर TValue *p1,
                                                 अचर TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, पूर्णाङ्क k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, अचर वर्ण *fmt, ...);
LUAI_FUNC अचर वर्ण *luaG_addinfo (lua_State *L, अचर वर्ण *सन्देश,
                                                  TString *src, पूर्णाङ्क पङ्क्ति);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC पूर्णाङ्क luaG_traceexec (lua_State *L, अचर Instruction *pc);
LUAI_FUNC पूर्णाङ्क luaG_tracecall (lua_State *L);


#endif
