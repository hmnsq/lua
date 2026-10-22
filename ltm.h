/*
** $Id: ltm.h $
** Tag methods
** See Copyright Notice in lua.h
*/

#अगर_अघोषित ltm_h
#घोषणा ltm_h


#समावेश "lobject.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
प्रकार क्रमागत {
  TM_सूचकांक,
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
#घोषणा maskflags	cast_byte(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#घोषणा notm(पंचांग)	ttisnil(पंचांग)

#घोषणा checknoTM(mt,e)	((mt) == शून्य || (mt)->flags & (1u<<(e)))

#घोषणा gfasttm(g,mt,e)  \
  (checknoTM(mt, e) ? शून्य : luaT_gettm(mt, e, (g)->tmname[e]))

#घोषणा fasttm(l,mt,e)	gfasttm(G(l), mt, e)

#घोषणा ttypename(x)	luaT_typenames_[(x) + 1]

LUAI_DDEC(स्थिर अक्षर *स्थिर luaT_typenames_[LUA_TOTALTYPES];)


LUAI_FUNC स्थिर अक्षर *luaT_objtypename (lua_State *L, स्थिर TValue *o);

LUAI_FUNC स्थिर TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC स्थिर TValue *luaT_gettmbyobj (lua_State *L, स्थिर TValue *o,
                                                       TMS event);
LUAI_FUNC व्योम luaT_init (lua_State *L);

LUAI_FUNC व्योम luaT_callTM (lua_State *L, स्थिर TValue *f, स्थिर TValue *p1,
                            स्थिर TValue *p2, स्थिर TValue *p3);
LUAI_FUNC lu_byte luaT_callTMres (lua_State *L, स्थिर TValue *f,
                               स्थिर TValue *p1, स्थिर TValue *p2, StkId p3);
LUAI_FUNC व्योम luaT_trybinTM (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2,
                              StkId res, TMS event);
LUAI_FUNC व्योम luaT_tryconcatTM (lua_State *L);
LUAI_FUNC व्योम luaT_trybinassocTM (lua_State *L, स्थिर TValue *p1,
       स्थिर TValue *p2, पूर्णांक inv, StkId res, TMS event);
LUAI_FUNC व्योम luaT_trybiniTM (lua_State *L, स्थिर TValue *p1, lua_Integer i2,
                               पूर्णांक inv, StkId res, TMS event);
LUAI_FUNC पूर्णांक luaT_callorderTM (lua_State *L, स्थिर TValue *p1,
                                स्थिर TValue *p2, TMS event);
LUAI_FUNC पूर्णांक luaT_callorderiTM (lua_State *L, स्थिर TValue *p1, पूर्णांक v2,
                                 पूर्णांक inv, पूर्णांक isfloat, TMS event);

LUAI_FUNC व्योम luaT_adjustvarargs (lua_State *L, काष्ठा CallInfo *ci,
                                                 स्थिर Proto *p);
LUAI_FUNC व्योम luaT_getvararg (CallInfo *ci, StkId ra, TValue *rc);
LUAI_FUNC व्योम luaT_getvarargs (lua_State *L, काष्ठा CallInfo *ci, StkId where,
                                              पूर्णांक wanted, पूर्णांक vatab);


#पूर्ण_अगर
