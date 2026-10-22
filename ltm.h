/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#അന്തര്ഭാവ "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
തരനിർവചനം എണ്ണൽ {
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

#define checknoTM(mt,e)	((mt) == ശൂന്യ || (mt)->flags & (1u<<(e)))

#define gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? ശൂന്യ : luaT_gettm(mt, e, (g)->tmname[e]))

#define fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#define ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(സ്ഥിരം അക്ഷരം *സ്ഥിരം luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC സ്ഥിരം അക്ഷരം *luaT_objtypename (lua_State *L, സ്ഥിരം TValue *o);

LUAI_FUNC സ്ഥിരം TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC സ്ഥിരം TValue *luaT_gettmbyobj (lua_State *L, സ്ഥിരം TValue *o,
                                                       TMS event);
LUAI_FUNC ശൂന്യം luaT_init (lua_State *L);

LUAI_FUNC ശൂന്യം luaT_callTM (lua_State *L, സ്ഥിരം TValue *f, സ്ഥിരം TValue *p1,
                            സ്ഥിരം TValue *p2, സ്ഥിരം TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, സ്ഥിരം TValue *f,
                               സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2, StkId p3);
LUAI_FUNC ശൂന്യം luaT_trybinTM (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC ശൂന്യം luaT_tryconcatTM (lua_State *L);
LUAI_FUNC ശൂന്യം luaT_trybinassocTM (lua_State *L, സ്ഥിരം TValue *p1,
       സ്ഥിരം TValue *p2, പൂർണ്ണസംഖ്യ inv, StkId res, TMS event);
LUAI_FUNC ശൂന്യം luaT_trybiniTM (lua_State *L, സ്ഥിരം TValue *p1, lua_Integer i2,
                               പൂർണ്ണസംഖ്യ inv, StkId res, TMS event);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaT_callorderTM (lua_State *L, സ്ഥിരം TValue *p1,
                                സ്ഥിരം TValue *p2, TMS event);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaT_callorderiTM (lua_State *L, സ്ഥിരം TValue *p1, പൂർണ്ണസംഖ്യ v2,
                                 പൂർണ്ണസംഖ്യ inv, പൂർണ്ണസംഖ്യ isfloat, TMS event);

LUAI_FUNC ശൂന്യം luaT_adjustvarargs (lua_State *L, ഘടന CallInfo *ci,
                                                 സ്ഥിരം Proto *p);
LUAI_FUNC ശൂന്യം luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC ശൂന്യം luaT_getvarargs (lua_State *L, ഘടന CallInfo *ci, StkId where,
                                              പൂർണ്ണസംഖ്യ wanted, പൂർണ്ണസംഖ്യ vatab);


#endif
