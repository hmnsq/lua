/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#અન્તર્ભાવ "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
પ્રકારવ્યાખ્યા ગણના {
  TM_INDEX,
  TM_NEWINDEX,
  TM_GC,
  TM_MODE,
  TM_LEN,
  TM_EQ,  /* last tag method with fast access */
  TM_ADD,
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
#define maskflags	cast_byte(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#define notm(tm)	ttisnil(tm)

#define checknoTM(mt,e)	((mt) == ખાલી || (mt)->flags & (1u<<(e)))

#define gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? ખાલી : luaT_gettm(mt, e, (g)->tmname[e]))

#define fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#define ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(અચલ અક્ષર *અચલ luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC અચલ અક્ષર *luaT_objtypename (lua_State *L, અચલ TValue *o);

LUAI_FUNC અચલ TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC અચલ TValue *luaT_gettmbyobj (lua_State *L, અચલ TValue *o,
                                                       TMS event);
LUAI_FUNC શૂન્ય luaT_init (lua_State *L);

LUAI_FUNC શૂન્ય luaT_callTM (lua_State *L, અચલ TValue *f, અચલ TValue *p1,
                            અચલ TValue *p2, અચલ TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, અચલ TValue *f,
                               અચલ TValue *p1, અચલ TValue *p2, StkId p3);
LUAI_FUNC શૂન્ય luaT_trybinTM (lua_State *L, અચલ TValue *p1, અચલ TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC શૂન્ય luaT_tryconcatTM (lua_State *L);
LUAI_FUNC શૂન્ય luaT_trybinassocTM (lua_State *L, અચલ TValue *p1,
       અચલ TValue *p2, પૂર્ણાંક inv, StkId res, TMS event);
LUAI_FUNC શૂન્ય luaT_trybiniTM (lua_State *L, અચલ TValue *p1, lua_Integer i2,
                               પૂર્ણાંક inv, StkId res, TMS event);
LUAI_FUNC પૂર્ણાંક luaT_callorderTM (lua_State *L, અચલ TValue *p1,
                                અચલ TValue *p2, TMS event);
LUAI_FUNC પૂર્ણાંક luaT_callorderiTM (lua_State *L, અચલ TValue *p1, પૂર્ણાંક v2,
                                 પૂર્ણાંક inv, પૂર્ણાંક isfloat, TMS event);

LUAI_FUNC શૂન્ય luaT_adjustvarargs (lua_State *L, સંરચના CallInfo *ci,
                                                 અચલ Proto *p);
LUAI_FUNC શૂન્ય luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC શૂન્ય luaT_getvarargs (lua_State *L, સંરચના CallInfo *ci, StkId where,
                                              પૂર્ણાંક wanted, પૂર્ણાંક vatab);


#endif
