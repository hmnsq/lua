/*
** $Id: ldebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ldebug_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ldebug_h


#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pcRel(pc, p)	(cast_int((pc) - (p)->ਕੋਡ) - 1)


/* Active Lua function (given call info) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#ਜੇ !defined(MAXIWTHABS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXIWTHABS	128
#ਜੇ_ਅੰਤ


LUAI_FUNC ਪੂਰਨ luaG_getfuncline (ਅਚਲ Proto *f, ਪੂਰਨ pc);
LUAI_FUNC ਅਚਲ ਅੱਖਰ *luaG_findlocal (lua_State *L, CallInfo *ci, ਪੂਰਨ n,
                                                    StkId *pos);
LUAI_FUNC l_noret luaG_typeerror (lua_State *L, ਅਚਲ TValue *o,
                                                ਅਚਲ ਅੱਖਰ *opname);
LUAI_FUNC l_noret luaG_callerror (lua_State *L, ਅਚਲ TValue *o);
LUAI_FUNC l_noret luaG_forerror (lua_State *L, ਅਚਲ TValue *o,
                                               ਅਚਲ ਅੱਖਰ *what);
LUAI_FUNC l_noret luaG_concaterror (lua_State *L, ਅਚਲ TValue *p1,
                                                  ਅਚਲ TValue *p2);
LUAI_FUNC l_noret luaG_opinterror (lua_State *L, ਅਚਲ TValue *p1,
                                                 ਅਚਲ TValue *p2,
                                                 ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ);
LUAI_FUNC l_noret luaG_tointerror (lua_State *L, ਅਚਲ TValue *p1,
                                                 ਅਚਲ TValue *p2);
LUAI_FUNC l_noret luaG_ordererror (lua_State *L, ਅਚਲ TValue *p1,
                                                 ਅਚਲ TValue *p2);
LUAI_FUNC l_noret luaG_errnnil (lua_State *L, LClosure *cl, ਪੂਰਨ k);
LUAI_FUNC l_noret luaG_runerror (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...);
LUAI_FUNC ਅਚਲ ਅੱਖਰ *luaG_addinfo (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ,
                                                  TString *src, ਪੂਰਨ ਸਤਰ);
LUAI_FUNC l_noret luaG_errormsg (lua_State *L);
LUAI_FUNC ਪੂਰਨ luaG_traceexec (lua_State *L, ਅਚਲ Instruction *pc);
LUAI_FUNC ਪੂਰਨ luaG_tracecall (lua_State *L);


#ਜੇ_ਅੰਤ
