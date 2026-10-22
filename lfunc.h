/*
** $Id: lfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lfunc_h
#घोषणा lfunc_h


#समावेश "lobject.h"


#घोषणा आकार_Cclosure(n)  \
	(दुरत्व(CClosure, upvalue) + माप(TValue) * cast_uint(n))

#घोषणा आकार_Lclosure(n)  \
	(दुरत्व(LClosure, upvals) + माप(UpVal *) * cast_uint(n))


/* test whether thread is in 'twups' list */
#घोषणा isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#घोषणा MAXUPVAL	255


#घोषणा upisopen(up)	((up)->v.p != &(up)->u.मूल्य)


#घोषणा uplevel(up)	check_exp(upisopen(up), cast(StkId, (up)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#घोषणा MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#घोषणा CLOSEKTOP	(LUA_ERRERR + 1)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, पूर्णांक nupvals);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, पूर्णांक nupvals);
LUAI_FUNC व्योम luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC UpVal *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC व्योम luaF_newtbcupval (lua_State *L, StkId level);
LUAI_FUNC व्योम luaF_closeupval (lua_State *L, StkId level);
LUAI_FUNC StkId luaF_close (lua_State *L, StkId level, TStatus status, पूर्णांक yy);
LUAI_FUNC व्योम luaF_unlinkupval (UpVal *uv);
LUAI_FUNC lu_mem luaF_protosize (Proto *p);
LUAI_FUNC व्योम luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC स्थिर अक्षर *luaF_getlocalname (स्थिर Proto *func,
                                         अचिन्हित local_संख्या, पूर्णांक pc);


#पूर्ण_अगर
