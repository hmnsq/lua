/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#অন্তর্ভাৱ "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
প্রকারনির্দেশ গণনা {
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

#define checknoTM(mt,e)	((mt) == রিক্ত || (mt)->flags & (1u<<(e)))

#define gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? রিক্ত : luaT_gettm(mt, e, (g)->tmname[e]))

#define fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#define ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(ধ্রুৱ ৱর্ণ *ধ্রুৱ luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC ধ্রুৱ ৱর্ণ *luaT_objtypename (lua_State *L, ধ্রুৱ TValue *o);

LUAI_FUNC ধ্রুৱ TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC ধ্রুৱ TValue *luaT_gettmbyobj (lua_State *L, ধ্রুৱ TValue *o,
                                                       TMS event);
LUAI_FUNC শূন্য luaT_init (lua_State *L);

LUAI_FUNC শূন্য luaT_callTM (lua_State *L, ধ্রুৱ TValue *f, ধ্রুৱ TValue *p1,
                            ধ্রুৱ TValue *p2, ধ্রুৱ TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, ধ্রুৱ TValue *f,
                               ধ্রুৱ TValue *p1, ধ্রুৱ TValue *p2, StkId p3);
LUAI_FUNC শূন্য luaT_trybinTM (lua_State *L, ধ্রুৱ TValue *p1, ধ্রুৱ TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC শূন্য luaT_tryconcatTM (lua_State *L);
LUAI_FUNC শূন্য luaT_trybinassocTM (lua_State *L, ধ্রুৱ TValue *p1,
       ধ্রুৱ TValue *p2, পূর্ণাঙ্ক inv, StkId res, TMS event);
LUAI_FUNC শূন্য luaT_trybiniTM (lua_State *L, ধ্রুৱ TValue *p1, lua_Integer i2,
                               পূর্ণাঙ্ক inv, StkId res, TMS event);
LUAI_FUNC পূর্ণাঙ্ক luaT_callorderTM (lua_State *L, ধ্রুৱ TValue *p1,
                                ধ্রুৱ TValue *p2, TMS event);
LUAI_FUNC পূর্ণাঙ্ক luaT_callorderiTM (lua_State *L, ধ্রুৱ TValue *p1, পূর্ণাঙ্ক v2,
                                 পূর্ণাঙ্ক inv, পূর্ণাঙ্ক isfloat, TMS event);

LUAI_FUNC শূন্য luaT_adjustvarargs (lua_State *L, সংরচনা CallInfo *ci,
                                                 ধ্রুৱ Proto *p);
LUAI_FUNC শূন্য luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC শূন্য luaT_getvarargs (lua_State *L, সংরচনা CallInfo *ci, StkId where,
                                              পূর্ণাঙ্ক wanted, পূর্ণাঙ্ক vatab);


#endif
