/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#اگر_غیر_تعریف_شدہ lauxlib_h
#تعریف_کرو lauxlib_h


#شامل <stddef.h>
#شامل <stdio.h>

#شامل "luaconf.h"
#شامل "lua.h"


/* global table */
#تعریف_کرو LUA_GNAME	"_G"


قسمتعریف ساخت lua_L_بفر lua_L_بفر;


/* extra error code for 'luaL_loadfilex' */
#تعریف_کرو LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#تعریف_کرو LUA_LOADED_جدول	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#تعریف_کرو LUA_PRELOAD_جدول	"_PRELOAD"


قسمتعریف ساخت luaL_Reg {
  ثابت حرف *name;
  lua_CFunction func;
} luaL_Reg;


#تعریف_کرو LUAL_NUMSIZES	(حجم(lua_Integer)*16 + حجم(lua_نمبر))

LUALIB_API خالی (luaL_checkversion_) (lua_State *L, lua_نمبر ver, size_t sz);
#تعریف_کرو luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_نسخہ_عدد, LUAL_NUMSIZES)

LUALIB_API صحیح (luaL_getmetafield) (lua_State *L, صحیح obj, ثابت حرف *e);
LUALIB_API صحیح (luaL_callmeta) (lua_State *L, صحیح obj, ثابت حرف *e);
LUALIB_API ثابت حرف *(luaL_tolstring) (lua_State *L, صحیح idx, size_t *len);
LUALIB_API صحیح (luaL_argerror) (lua_State *L, صحیح arg, ثابت حرف *extramsg);
LUALIB_API صحیح (luaL_typeerror) (lua_State *L, صحیح arg, ثابت حرف *tname);
LUALIB_API ثابت حرف *(luaL_checklstring) (lua_State *L, صحیح arg,
                                                          size_t *l);
LUALIB_API ثابت حرف *(luaL_optlstring) (lua_State *L, صحیح arg,
                                          ثابت حرف *def, size_t *l);
LUALIB_API lua_نمبر (luaL_checknumber) (lua_State *L, صحیح arg);
LUALIB_API lua_نمبر (luaL_optnumber) (lua_State *L, صحیح arg, lua_نمبر def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, صحیح arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, صحیح arg,
                                          lua_Integer def);

LUALIB_API خالی (luaL_checkstack) (lua_State *L, صحیح sz, ثابت حرف *پیغام);
LUALIB_API خالی (luaL_checktype) (lua_State *L, صحیح arg, صحیح t);
LUALIB_API خالی (luaL_checkany) (lua_State *L, صحیح arg);

LUALIB_API صحیح   (luaL_newmetatable) (lua_State *L, ثابت حرف *tname);
LUALIB_API خالی  (luaL_setmetatable) (lua_State *L, ثابت حرف *tname);
LUALIB_API خالی *(luaL_testudata) (lua_State *L, صحیح ud, ثابت حرف *tname);
LUALIB_API خالی *(luaL_checkudata) (lua_State *L, صحیح ud, ثابت حرف *tname);

LUALIB_API خالی (luaL_where) (lua_State *L, صحیح lvl);
LUALIB_API صحیح (luaL_error) (lua_State *L, ثابت حرف *fmt, ...);

LUALIB_API صحیح (luaL_checkoption) (lua_State *L, صحیح arg, ثابت حرف *def,
                                   ثابت حرف *ثابت lst[]);

LUALIB_API صحیح (luaL_fileresult) (lua_State *L, صحیح stat, ثابت حرف *fname);
LUALIB_API صحیح (luaL_execresult) (lua_State *L, صحیح stat);

LUALIB_API خالی *(luaL_alloc) (خالی *ud, خالی *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#تعریف_کرو LUA_NOREF       (-2)
#تعریف_کرو LUA_REFNIL      (-1)

LUALIB_API صحیح (luaL_ref) (lua_State *L, صحیح t);
LUALIB_API خالی (luaL_unref) (lua_State *L, صحیح t, صحیح ref);

LUALIB_API صحیح (luaL_loadfilex) (lua_State *L, ثابت حرف *filename,
                                               ثابت حرف *منوال);

#تعریف_کرو luaL_loadfile(L,f)	luaL_loadfilex(L,f,تہی)

LUALIB_API صحیح (luaL_loadbufferx) (lua_State *L, ثابت حرف *buff, size_t sz,
                                   ثابت حرف *name, ثابت حرف *منوال);
LUALIB_API صحیح (luaL_loadstring) (lua_State *L, ثابت حرف *s);

LUALIB_API lua_State *(luaL_newstate) (خالی);

LUALIB_API غیرعلامتی (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, صحیح idx);

LUALIB_API خالی (luaL_addgsub) (lua_L_بفر *b, ثابت حرف *s,
                                     ثابت حرف *p, ثابت حرف *r);
LUALIB_API ثابت حرف *(luaL_gsub) (lua_State *L, ثابت حرف *s,
                                    ثابت حرف *p, ثابت حرف *r);

LUALIB_API خالی (luaL_setfuncs) (lua_State *L, ثابت luaL_Reg *l, صحیح nup);

LUALIB_API صحیح (luaL_getsubtable) (lua_State *L, صحیح idx, ثابت حرف *fname);

LUALIB_API خالی (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ثابت حرف *پیغام, صحیح level);

LUALIB_API خالی (luaL_requiref) (lua_State *L, ثابت حرف *modname,
                                 lua_CFunction openf, صحیح glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#تعریف_کرو luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, حجم(l)/حجم((l)[0]) - 1)

#تعریف_کرو luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#تعریف_کرو luaL_argcheck(L, cond,arg,extramsg)	\
	((خالی)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#تعریف_کرو luaL_argexpected(L,cond,arg,tname)	\
	((خالی)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#تعریف_کرو luaL_checkstring(L,n)	(luaL_checklstring(L, (n), تہی))
#تعریف_کرو luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), تہی))

#تعریف_کرو luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#تعریف_کرو luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#تعریف_کرو luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#تعریف_کرو luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#تعریف_کرو luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#تعریف_کرو luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,تہی)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#تعریف_کرو luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#اگر defined(LUA_FAILISFALSE)
#تعریف_کرو luaL_pushfail(L)	lua_pushboolean(L, 0)
#ورنہ
#تعریف_کرو luaL_pushfail(L)	lua_pushnil(L)
#اگر_ختم



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

ساخت lua_L_بفر {
  حرف *b;  /* buffer address */
  size_t سائز;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  اتحاد {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    حرف b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#تعریف_کرو luaL_bufflen(bf)	((bf)->n)
#تعریف_کرو luaL_buffaddr(bf)	((bf)->b)


#تعریف_کرو luaL_addchar(B,c) \
  ((خالی)((B)->n < (B)->سائز || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#تعریف_کرو luaL_addsize(B,s)	((B)->n += (s))

#تعریف_کرو luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API خالی (luaL_buffinit) (lua_State *L, lua_L_بفر *B);
LUALIB_API حرف *(luaL_prepbuffsize) (lua_L_بفر *B, size_t sz);
LUALIB_API خالی (luaL_addlstring) (lua_L_بفر *B, ثابت حرف *s, size_t l);
LUALIB_API خالی (luaL_addstring) (lua_L_بفر *B, ثابت حرف *s);
LUALIB_API خالی (luaL_addvalue) (lua_L_بفر *B);
LUALIB_API خالی (luaL_pushresult) (lua_L_بفر *B);
LUALIB_API خالی (luaL_pushresultsize) (lua_L_بفر *B, size_t sz);
LUALIB_API حرف *(luaL_buffinitsize) (lua_State *L, lua_L_بفر *B, size_t sz);

#تعریف_کرو luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

/* }====================================================== */



/*
** {======================================================
** File handles for IO library
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LUA_FILEHANDLE' and
** initial structure 'luaL_Stream' (it may contain other fields
** after that initial structure).
*/

#تعریف_کرو LUA_FILEHANDLE          "FILE*"


قسمتعریف ساخت lua_L_دھارا {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} lua_L_دھارا;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#اگر defined(LUA_COMPAT_APIINTCASTS)

#تعریف_کرو luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#تعریف_کرو luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#تعریف_کرو luaL_checkint(L,n)	((صحیح)luaL_checkinteger(L, (n)))
#تعریف_کرو luaL_optint(L,n,d)	((صحیح)luaL_optinteger(L, (n), (d)))

#تعریف_کرو luaL_checklong(L,n)	((طویل)luaL_checkinteger(L, (n)))
#تعریف_کرو luaL_optlong(L,n,d)	((طویل)luaL_optinteger(L, (n), (d)))

#اگر_ختم
/* }============================================================ */



#اگر_ختم


