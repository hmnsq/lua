/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ltm_h
#ವ್ಯಾಖ್ಯಾನಿಸು ltm_h


#ಅನ್ತರ್ಭಾವ "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
  TM_ಸೂಚ್ಯಂಕ,
  TM_NEWINDEX,
  TM_GC,
  TM_ಬಹುಲಕ,
  TM_LEN,
  TM_EQ,  /* last tag method with fast access */
  TM_ಕೂಡಿಸು,
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
#ವ್ಯಾಖ್ಯಾನಿಸು maskflags	cast_byte(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು notm(tm)	ttisnil(tm)

#ವ್ಯಾಖ್ಯಾನಿಸು checknoTM(mt,e)	((mt) == ಖಾಲಿ || (mt)->flags & (1u<<(e)))

#ವ್ಯಾಖ್ಯಾನಿಸು gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? ಖಾಲಿ : luaT_gettm(mt, e, (g)->tmname[e]))

#ವ್ಯಾಖ್ಯಾನಿಸು fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#ವ್ಯಾಖ್ಯಾನಿಸು ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaT_objtypename (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o);

LUAI_FUNC ಸ್ಥಿರಾಂಕ TValue *luaT_gettm (ಕೋಷ್ಟಕ *events, TMS event, TString *ename);
LUAI_FUNC ಸ್ಥಿರಾಂಕ TValue *luaT_gettmbyobj (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o,
                                                       TMS event);
LUAI_FUNC ಶೂನ್ಯ luaT_init (lua_State *L);

LUAI_FUNC ಶೂನ್ಯ luaT_callTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *f, ಸ್ಥಿರಾಂಕ TValue *p1,
                            ಸ್ಥಿರಾಂಕ TValue *p2, ಸ್ಥಿರಾಂಕ TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *f,
                               ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2, StkId p3);
LUAI_FUNC ಶೂನ್ಯ luaT_trybinTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC ಶೂನ್ಯ luaT_tryconcatTM (lua_State *L);
LUAI_FUNC ಶೂನ್ಯ luaT_trybinassocTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1,
       ಸ್ಥಿರಾಂಕ TValue *p2, ಪೂರ್ಣಾಂಕ inv, StkId res, TMS event);
LUAI_FUNC ಶೂನ್ಯ luaT_trybiniTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, lua_Integer i2,
                               ಪೂರ್ಣಾಂಕ inv, StkId res, TMS event);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaT_callorderTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1,
                                ಸ್ಥಿರಾಂಕ TValue *p2, TMS event);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaT_callorderiTM (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಪೂರ್ಣಾಂಕ v2,
                                 ಪೂರ್ಣಾಂಕ inv, ಪೂರ್ಣಾಂಕ isfloat, TMS event);

LUAI_FUNC ಶೂನ್ಯ luaT_adjustvarargs (lua_State *L, ರಚನೆ CallInfo *ci,
                                                 ಸ್ಥಿರಾಂಕ Proto *p);
LUAI_FUNC ಶೂನ್ಯ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC ಶೂನ್ಯ luaT_getvarargs (lua_State *L, ರಚನೆ CallInfo *ci, StkId where,
                                              ಪೂರ್ಣಾಂಕ wanted, ಪೂರ್ಣಾಂಕ vatab);


#ಆದರೆ_ಅಂತ್ಯ
