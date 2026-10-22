/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ltm_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltm_h


#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
  TM_ਸੂਚਕਾਂਕ,
  TM_NEWINDEX,
  TM_GC,
  TM_ਬਹੁਲਕ,
  TM_LEN,
  TM_EQ,  /* last tag method with fast access */
  TM_ਜੋੜੋ,
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ maskflags	cast_byte(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ notm(tm)	ttisnil(tm)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checknoTM(mt,e)	((mt) == ਨਿਰਾ || (mt)->flags & (1u<<(e)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? ਨਿਰਾ : luaT_gettm(mt, e, (g)->tmname[e]))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(ਅਚਲ ਅੱਖਰ *ਅਚਲ luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC ਅਚਲ ਅੱਖਰ *luaT_objtypename (lua_State *L, ਅਚਲ TValue *o);

LUAI_FUNC ਅਚਲ TValue *luaT_gettm (ਸਾਰਣੀ *events, TMS event, TString *ename);
LUAI_FUNC ਅਚਲ TValue *luaT_gettmbyobj (lua_State *L, ਅਚਲ TValue *o,
                                                       TMS event);
LUAI_FUNC ਖਾਲੀ luaT_init (lua_State *L);

LUAI_FUNC ਖਾਲੀ luaT_callTM (lua_State *L, ਅਚਲ TValue *f, ਅਚਲ TValue *p1,
                            ਅਚਲ TValue *p2, ਅਚਲ TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, ਅਚਲ TValue *f,
                               ਅਚਲ TValue *p1, ਅਚਲ TValue *p2, StkId p3);
LUAI_FUNC ਖਾਲੀ luaT_trybinTM (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC ਖਾਲੀ luaT_tryconcatTM (lua_State *L);
LUAI_FUNC ਖਾਲੀ luaT_trybinassocTM (lua_State *L, ਅਚਲ TValue *p1,
       ਅਚਲ TValue *p2, ਪੂਰਨ inv, StkId res, TMS event);
LUAI_FUNC ਖਾਲੀ luaT_trybiniTM (lua_State *L, ਅਚਲ TValue *p1, lua_Integer i2,
                               ਪੂਰਨ inv, StkId res, TMS event);
LUAI_FUNC ਪੂਰਨ luaT_callorderTM (lua_State *L, ਅਚਲ TValue *p1,
                                ਅਚਲ TValue *p2, TMS event);
LUAI_FUNC ਪੂਰਨ luaT_callorderiTM (lua_State *L, ਅਚਲ TValue *p1, ਪੂਰਨ v2,
                                 ਪੂਰਨ inv, ਪੂਰਨ isfloat, TMS event);

LUAI_FUNC ਖਾਲੀ luaT_adjustvarargs (lua_State *L, ਬਣਤਰ CallInfo *ci,
                                                 ਅਚਲ Proto *p);
LUAI_FUNC ਖਾਲੀ luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC ਖਾਲੀ luaT_getvarargs (lua_State *L, ਬਣਤਰ CallInfo *ci, StkId where,
                                              ਪੂਰਨ wanted, ਪੂਰਨ vatab);


#ਜੇ_ਅੰਤ
