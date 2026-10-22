/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#अगर_अघोषित ldebug_h
#घोषणा ldebug_h


#समावेश "lstate.h"


#घोषणा pcRel(pc, p)	(cast_int((pc) - (p)->code) - 1)


/* Active Lua function (given call info) */
#घोषणा ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#घोषणा resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#घोषणा ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#अगर !defined(MAXIWTHABS)
#घोषणा MAXIWTHABS	128
#पूर्ण_अगर


LUAI_FUNC पूर्णांक luaG_getfuncline (स्थिर Proto *f, पूर्णांक pc);
LUAI_FUNC स्थिर अक्षर *luaG_findlocal (lua_State *L, CallInfo *ci, पूर्णांक n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, स्थिर TValue *o,
                                                स्थिर अक्षर *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, स्थिर TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, स्थिर TValue *o,
                                               स्थिर अक्षर *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, स्थिर TValue *p1,
                                                  स्थिर TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, स्थिर TValue *p1,
                                                 स्थिर TValue *p2,
                                                 स्थिर अक्षर *संदेश);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, स्थिर TValue *p1,
                                                 स्थिर TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, स्थिर TValue *p1,
                                                 स्थिर TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, पूर्णांक k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, स्थिर अक्षर *fmt, ...);
LUAI_FUNC स्थिर अक्षर *luaG_addinfo (lua_State *L, स्थिर अक्षर *संदेश,
                                                  TString *src, पूर्णांक पंक्ति);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC पूर्णांक luaG_traceexec (lua_State *L, स्थिर Instruction *pc);
LUAI_FUNC पूर्णांक luaG_tracecall (lua_State *L);


#पूर्ण_अगर
