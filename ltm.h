/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् ltm_h
#परिभाषय ltm_h


#अन्तर्भाव "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
प्रकारनिर्देश गणना {
  TM_सूचकांक,
  TM_NEWINDEX,
  TM_GC,
  TM_बहुलकम्,
  TM_LEN,
  TM_EQ,  /* last tag method with fast access */
  TM_सङ्कलय,
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
#परिभाषय maskflags	cast_byte(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#परिभाषय notm(tm)	ttisnil(tm)

#परिभाषय checknoTM(mt,e)	((mt) == रिक्त || (mt)->flags & (1u<<(e)))

#परिभाषय gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? रिक्त : luaT_gettm(mt, e, (g)->tmname[e]))

#परिभाषय fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#परिभाषय ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(ध्रुव वर्ण *ध्रुव luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC ध्रुव वर्ण *luaT_objtypename (lua_State *L, ध्रुव TValue *o);

LUAI_FUNC ध्रुव TValue *luaT_gettm (सारणी *events, TMS event, TString *ename);
LUAI_FUNC ध्रुव TValue *luaT_gettmbyobj (lua_State *L, ध्रुव TValue *o,
                                                       TMS event);
LUAI_FUNC शून्य luaT_init (lua_State *L);

LUAI_FUNC शून्य luaT_callTM (lua_State *L, ध्रुव TValue *f, ध्रुव TValue *p1,
                            ध्रुव TValue *p2, ध्रुव TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, ध्रुव TValue *f,
                               ध्रुव TValue *p1, ध्रुव TValue *p2, StkId p3);
LUAI_FUNC शून्य luaT_trybinTM (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC शून्य luaT_tryconcatTM (lua_State *L);
LUAI_FUNC शून्य luaT_trybinassocTM (lua_State *L, ध्रुव TValue *p1,
       ध्रुव TValue *p2, पूर्णाङ्क inv, StkId res, TMS event);
LUAI_FUNC शून्य luaT_trybiniTM (lua_State *L, ध्रुव TValue *p1, lua_Integer i2,
                               पूर्णाङ्क inv, StkId res, TMS event);
LUAI_FUNC पूर्णाङ्क luaT_callorderTM (lua_State *L, ध्रुव TValue *p1,
                                ध्रुव TValue *p2, TMS event);
LUAI_FUNC पूर्णाङ्क luaT_callorderiTM (lua_State *L, ध्रुव TValue *p1, पूर्णाङ्क v2,
                                 पूर्णाङ्क inv, पूर्णाङ्क isfloat, TMS event);

LUAI_FUNC शून्य luaT_adjustvarargs (lua_State *L, संरचना CallInfo *ci,
                                                 ध्रुव Proto *p);
LUAI_FUNC शून्य luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC शून्य luaT_getvarargs (lua_State *L, संरचना CallInfo *ci, StkId where,
                                              पूर्णाङ्क wanted, पूर्णाङ्क vatab);


#यदि_अन्तः
