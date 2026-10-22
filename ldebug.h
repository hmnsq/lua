/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ifndef ldebug_h
#define ldebug_h


#അന്തര്ഭാവ "lstate.h"


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
#എങ്കിൽ !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LUAI_FUNC പൂർണ്ണസംഖ്യ luaG_getfuncline (സ്ഥിരം Proto *f, പൂർണ്ണസംഖ്യ pc);
LUAI_FUNC സ്ഥിരം അക്ഷരം *luaG_findlocal (lua_State *L, CallInfo *ci, പൂർണ്ണസംഖ്യ n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, സ്ഥിരം TValue *o,
                                                സ്ഥിരം അക്ഷരം *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, സ്ഥിരം TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, സ്ഥിരം TValue *o,
                                               സ്ഥിരം അക്ഷരം *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, സ്ഥിരം TValue *p1,
                                                  സ്ഥിരം TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, സ്ഥിരം TValue *p1,
                                                 സ്ഥിരം TValue *p2,
                                                 സ്ഥിരം അക്ഷരം *സന്ദേശം);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, സ്ഥിരം TValue *p1,
                                                 സ്ഥിരം TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, സ്ഥിരം TValue *p1,
                                                 സ്ഥിരം TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, പൂർണ്ണസംഖ്യ k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, ...);
LUAI_FUNC സ്ഥിരം അക്ഷരം *luaG_addinfo (lua_State *L, സ്ഥിരം അക്ഷരം *സന്ദേശം,
                                                  TString *src, പൂർണ്ണസംഖ്യ വരി);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaG_traceexec (lua_State *L, സ്ഥിരം Instruction *pc);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaG_tracecall (lua_State *L);


#endif
