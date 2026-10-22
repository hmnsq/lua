/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#अन्तर्भाव "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
प्रकारव्याख्या गणना {
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
#define notm(पंचांग)	ttisnil(पंचांग)

#define checknoTM(mt,e)	((mt) == रिक्त || (mt)->flags & (1u<<(e)))

#define gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? रिक्त : luaT_gettm(mt, e, (g)->tmname[e]))

#define fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#define ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(स्थिरांक अक्षर *स्थिरांक luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC स्थिरांक अक्षर *luaT_objtypename (lua_State *L, स्थिरांक TValue *o);

LUAI_FUNC स्थिरांक TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC स्थिरांक TValue *luaT_gettmbyobj (lua_State *L, स्थिरांक TValue *o,
                                                       TMS event);
LUAI_FUNC शून्य luaT_init (lua_State *L);

LUAI_FUNC शून्य luaT_callTM (lua_State *L, स्थिरांक TValue *f, स्थिरांक TValue *p1,
                            स्थिरांक TValue *p2, स्थिरांक TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, स्थिरांक TValue *f,
                               स्थिरांक TValue *p1, स्थिरांक TValue *p2, StkId p3);
LUAI_FUNC शून्य luaT_trybinTM (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC शून्य luaT_tryconcatTM (lua_State *L);
LUAI_FUNC शून्य luaT_trybinassocTM (lua_State *L, स्थिरांक TValue *p1,
       स्थिरांक TValue *p2, पूर्णांक inv, StkId res, TMS event);
LUAI_FUNC शून्य luaT_trybiniTM (lua_State *L, स्थिरांक TValue *p1, lua_Integer i2,
                               पूर्णांक inv, StkId res, TMS event);
LUAI_FUNC पूर्णांक luaT_callorderTM (lua_State *L, स्थिरांक TValue *p1,
                                स्थिरांक TValue *p2, TMS event);
LUAI_FUNC पूर्णांक luaT_callorderiTM (lua_State *L, स्थिरांक TValue *p1, पूर्णांक v2,
                                 पूर्णांक inv, पूर्णांक isfloat, TMS event);

LUAI_FUNC शून्य luaT_adjustvarargs (lua_State *L, संरचना CallInfo *ci,
                                                 स्थिरांक Proto *p);
LUAI_FUNC शून्य luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC शून्य luaT_getvarargs (lua_State *L, संरचना CallInfo *ci, StkId where,
                                              पूर्णांक wanted, पूर्णांक vatab);


#endif
