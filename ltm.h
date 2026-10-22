/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే ltm_h
#నిర్వచించు ltm_h


#అన్తర్భావ "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
రకనిర్వచనం గణన {
  TM_సూచీ,
  TM_NEWINDEX,
  TM_GC,
  TM_బహుళకం,
  TM_LEN,
  TM_EQ,  /* last tag method with fast access */
  TM_కూడు,
  TM_SUB,
  TM_MUL,
  TM_MOD,
  TM_POW,
  TM_DIV,
  TM_IDIV,
  TM_BAND,
  TM_BOR,
  TM_BXOR,
  TM_SHL,
  TM_SHR,
  TM_UNM,
  TM_BNOT,
  TM_LT,
  TM_LE,
  TM_CONCAT,
  TM_CALL,
  TM_CLOSE,
  TM_N		/* number of elements in the enum */
} TMS;


/*
** Mask with 1 in all fast-access methods. A 1 in any of these bits
** in the flag of a (meta)table means the metatable does not have the
** corresponding metamethod field. (Bit 6 of the flag indicates that
** the table is using the dummy node.)
*/
#నిర్వచించు maskflags	cast_byte(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#నిర్వచించు notm(tm)	ttisnil(tm)

#నిర్వచించు checknoTM(mt,e)	((mt) == ఖాళీ || (mt)->flags & (1u<<(e)))

#నిర్వచించు gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? ఖాళీ : luaT_gettm(mt, e, (g)->tmname[e]))

#నిర్వచించు fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#నిర్వచించు ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(స్థిరరాశి అక్షరం *స్థిరరాశి luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC స్థిరరాశి అక్షరం *luaT_objtypename (lua_State *L, స్థిరరాశి TValue *o);

LUAI_FUNC స్థిరరాశి TValue *luaT_gettm (పట్టిక *events, TMS event, TString *ename);
LUAI_FUNC స్థిరరాశి TValue *luaT_gettmbyobj (lua_State *L, స్థిరరాశి TValue *o,
                                                       TMS event);
LUAI_FUNC శూన్య luaT_init (lua_State *L);

LUAI_FUNC శూన్య luaT_callTM (lua_State *L, స్థిరరాశి TValue *f, స్థిరరాశి TValue *p1,
                            స్థిరరాశి TValue *p2, స్థిరరాశి TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, స్థిరరాశి TValue *f,
                               స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2, StkId p3);
LUAI_FUNC శూన్య luaT_trybinTM (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC శూన్య luaT_tryconcatTM (lua_State *L);
LUAI_FUNC శూన్య luaT_trybinassocTM (lua_State *L, స్థిరరాశి TValue *p1,
       స్థిరరాశి TValue *p2, పూర్ణాంకం inv, StkId res, TMS event);
LUAI_FUNC శూన్య luaT_trybiniTM (lua_State *L, స్థిరరాశి TValue *p1, lua_Integer i2,
                               పూర్ణాంకం inv, StkId res, TMS event);
LUAI_FUNC పూర్ణాంకం luaT_callorderTM (lua_State *L, స్థిరరాశి TValue *p1,
                                స్థిరరాశి TValue *p2, TMS event);
LUAI_FUNC పూర్ణాంకం luaT_callorderiTM (lua_State *L, స్థిరరాశి TValue *p1, పూర్ణాంకం v2,
                                 పూర్ణాంకం inv, పూర్ణాంకం isfloat, TMS event);

LUAI_FUNC శూన్య luaT_adjustvarargs (lua_State *L, నిర్మాణం CallInfo *ci,
                                                 స్థిరరాశి Proto *p);
LUAI_FUNC శూన్య luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC శూన్య luaT_getvarargs (lua_State *L, నిర్మాణం CallInfo *ci, StkId where,
                                              పూర్ణాంకం wanted, పూర్ణాంకం vatab);


#అయితే_ముగింపు
