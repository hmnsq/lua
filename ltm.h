/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#ಅನ್ತರ್ಭಾವ "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
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

#define checknoTM(mt,e)	((mt) == ರಿಕ್ತ || (mt)->flags & (1u<<(e)))

#define gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? ರಿಕ್ತ : luaT_gettm(mt, e, (g)->tmname[e]))

#define fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#define ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC ಧ್ರುವ ವರ್ಣ *luaT_objtypename (lua_State *L, ಧ್ರುವ TValue *o);

LUAI_FUNC ಧ್ರುವ TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC ಧ್ರುವ TValue *luaT_gettmbyobj (lua_State *L, ಧ್ರುವ TValue *o,
                                                       TMS event);
LUAI_FUNC ಶೂನ್ಯ luaT_init (lua_State *L);

LUAI_FUNC ಶೂನ್ಯ luaT_callTM (lua_State *L, ಧ್ರುವ TValue *f, ಧ್ರುವ TValue *p1,
                            ಧ್ರುವ TValue *p2, ಧ್ರುವ TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, ಧ್ರುವ TValue *f,
                               ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2, StkId p3);
LUAI_FUNC ಶೂನ್ಯ luaT_trybinTM (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC ಶೂನ್ಯ luaT_tryconcatTM (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaT_trybinassocTM (lua_State *L, ಧ್ರುವ TValue *p1,
       ಧ್ರುವ TValue *p2, ಪೂರ್ಣಾಙ್ಕ inv, StkId res, TMS event);
LUAI_FUNC ಶೂನ್ಯ luaT_trybiniTM (lua_State *L, ಧ್ರುವ TValue *p1, lua_Integer i2,
                               ಪೂರ್ಣಾಙ್ಕ inv, StkId res, TMS event);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaT_callorderTM (lua_State *L, ಧ್ರುವ TValue *p1,
                                ಧ್ರುವ TValue *p2, TMS event);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaT_callorderiTM (lua_State *L, ಧ್ರುವ TValue *p1, ಪೂರ್ಣಾಙ್ಕ v2,
                                 ಪೂರ್ಣಾಙ್ಕ inv, ಪೂರ್ಣಾಙ್ಕ isfloat, TMS event);

LUAI_FUNC ಶೂನ್ಯ luaT_adjustvarargs (lua_State *L, ಸಂರಚನಾ CallInfo *ci,
                                                 ಧ್ರುವ Proto *p);
LUAI_FUNC ಶೂನ್ಯ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC ಶೂನ್ಯ luaT_getvarargs (lua_State *L, ಸಂರಚನಾ CallInfo *ci, StkId where,
                                              ಪೂರ್ಣಾಙ್ಕ wanted, ಪೂರ್ಣಾಙ್ಕ vatab);


#endif
