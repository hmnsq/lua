/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#جے_بنا_تعریف lauxlib_h
#تعریف_کرو lauxlib_h


#ضمن <stddef.h>
#ضمن <stdio.h>

#ضمن "luaconf.h"
#ضمن "lua.h"


/* global table */
#تعریف_کرو LUA_GNAME	"_G"


تعريفنوع بنية lua_L_بفر lua_L_بفر;


/* extra error code for 'luaL_loadfilex' */
#تعریف_کرو LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#تعریف_کرو LUA_LOADED_جدول	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#تعریف_کرو LUA_PRELOAD_جدول	"_PRELOAD"


تعريفنوع بنية luaL_Reg {
  ثابت حرف *name;
  lua_CFunction func;
} luaL_Reg;


#تعریف_کرو LUAL_NUMSIZES	(حجم(lua_Integer)*16 + حجم(lua_نمبر))

LUALIB_API فراغ (luaL_checkversion_) (lua_State *L, lua_نمبر ver, size_t sz);
#تعریف_کرو luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_نسخہ_عدد, LUAL_NUMSIZES)

LUALIB_API صحيح (luaL_getmetafield) (lua_State *L, صحيح obj, ثابت حرف *e);
LUALIB_API صحيح (luaL_callmeta) (lua_State *L, صحيح obj, ثابت حرف *e);
LUALIB_API ثابت حرف *(luaL_tolstring) (lua_State *L, صحيح idx, size_t *len);
LUALIB_API صحيح (luaL_argerror) (lua_State *L, صحيح arg, ثابت حرف *extramsg);
LUALIB_API صحيح (luaL_typeerror) (lua_State *L, صحيح arg, ثابت حرف *tname);
LUALIB_API ثابت حرف *(luaL_checklstring) (lua_State *L, صحيح arg,
                                                          size_t *l);
LUALIB_API ثابت حرف *(luaL_optlstring) (lua_State *L, صحيح arg,
                                          ثابت حرف *def, size_t *l);
LUALIB_API lua_نمبر (luaL_checknumber) (lua_State *L, صحيح arg);
LUALIB_API lua_نمبر (luaL_optnumber) (lua_State *L, صحيح arg, lua_نمبر def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, صحيح arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, صحيح arg,
                                          lua_Integer def);

LUALIB_API فراغ (luaL_checkstack) (lua_State *L, صحيح sz, ثابت حرف *پیغام);
LUALIB_API فراغ (luaL_checktype) (lua_State *L, صحيح arg, صحيح t);
LUALIB_API فراغ (luaL_checkany) (lua_State *L, صحيح arg);

LUALIB_API صحيح   (luaL_newmetatable) (lua_State *L, ثابت حرف *tname);
LUALIB_API فراغ  (luaL_setmetatable) (lua_State *L, ثابت حرف *tname);
LUALIB_API فراغ *(luaL_testudata) (lua_State *L, صحيح ud, ثابت حرف *tname);
LUALIB_API فراغ *(luaL_checkudata) (lua_State *L, صحيح ud, ثابت حرف *tname);

LUALIB_API فراغ (luaL_where) (lua_State *L, صحيح lvl);
LUALIB_API صحيح (luaL_error) (lua_State *L, ثابت حرف *fmt, ...);

LUALIB_API صحيح (luaL_checkoption) (lua_State *L, صحيح arg, ثابت حرف *def,
                                   ثابت حرف *ثابت lst[]);

LUALIB_API صحيح (luaL_fileresult) (lua_State *L, صحيح stat, ثابت حرف *fname);
LUALIB_API صحيح (luaL_execresult) (lua_State *L, صحيح stat);

LUALIB_API فراغ *(luaL_alloc) (فراغ *ud, فراغ *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#تعریف_کرو LUA_NOREF       (-2)
#تعریف_کرو LUA_REFNIL      (-1)

LUALIB_API صحيح (luaL_ref) (lua_State *L, صحيح t);
LUALIB_API فراغ (luaL_unref) (lua_State *L, صحيح t, صحيح ref);

LUALIB_API صحيح (luaL_loadfilex) (lua_State *L, ثابت حرف *filename,
                                               ثابت حرف *منوال);

#تعریف_کرو luaL_loadfile(L,f)	luaL_loadfilex(L,f,عدم)

LUALIB_API صحيح (luaL_loadbufferx) (lua_State *L, ثابت حرف *buff, size_t sz,
                                   ثابت حرف *name, ثابت حرف *منوال);
LUALIB_API صحيح (luaL_loadstring) (lua_State *L, ثابت حرف *s);

LUALIB_API lua_State *(luaL_newstate) (فراغ);

LUALIB_API بنانشانی (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, صحيح idx);

LUALIB_API فراغ (luaL_addgsub) (lua_L_بفر *b, ثابت حرف *s,
                                     ثابت حرف *p, ثابت حرف *r);
LUALIB_API ثابت حرف *(luaL_gsub) (lua_State *L, ثابت حرف *s,
                                    ثابت حرف *p, ثابت حرف *r);

LUALIB_API فراغ (luaL_setfuncs) (lua_State *L, ثابت luaL_Reg *l, صحيح nup);

LUALIB_API صحيح (luaL_getsubtable) (lua_State *L, صحيح idx, ثابت حرف *fname);

LUALIB_API فراغ (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ثابت حرف *پیغام, صحيح level);

LUALIB_API فراغ (luaL_requiref) (lua_State *L, ثابت حرف *modname,
                                 lua_CFunction openf, صحيح glb);

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
	((فراغ)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#تعریف_کرو luaL_argexpected(L,cond,arg,tname)	\
	((فراغ)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#تعریف_کرو luaL_checkstring(L,n)	(luaL_checklstring(L, (n), عدم))
#تعریف_کرو luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), عدم))

#تعریف_کرو luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#تعریف_کرو luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#تعریف_کرو luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#تعریف_کرو luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#تعریف_کرو luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#تعریف_کرو luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,عدم)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#تعریف_کرو luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#إذا defined(LUA_FAILISFALSE)
#تعریف_کرو luaL_pushfail(L)	lua_pushboolean(L, 0)
#وإلا
#تعریف_کرو luaL_pushfail(L)	lua_pushnil(L)
#جے_انت



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

بنية lua_L_بفر {
  حرف *b;  /* buffer address */
  size_t ناپ;  /* buffer size */
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
  ((فراغ)((B)->n < (B)->ناپ || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#تعریف_کرو luaL_addsize(B,s)	((B)->n += (s))

#تعریف_کرو luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API فراغ (luaL_buffinit) (lua_State *L, lua_L_بفر *B);
LUALIB_API حرف *(luaL_prepbuffsize) (lua_L_بفر *B, size_t sz);
LUALIB_API فراغ (luaL_addlstring) (lua_L_بفر *B, ثابت حرف *s, size_t l);
LUALIB_API فراغ (luaL_addstring) (lua_L_بفر *B, ثابت حرف *s);
LUALIB_API فراغ (luaL_addvalue) (lua_L_بفر *B);
LUALIB_API فراغ (luaL_pushresult) (lua_L_بفر *B);
LUALIB_API فراغ (luaL_pushresultsize) (lua_L_بفر *B, size_t sz);
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


تعريفنوع بنية lua_L_دھارا {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} lua_L_دھارا;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#إذا defined(LUA_COMPAT_APIINTCASTS)

#تعریف_کرو luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#تعریف_کرو luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#تعریف_کرو luaL_checkint(L,n)	((صحيح)luaL_checkinteger(L, (n)))
#تعریف_کرو luaL_optint(L,n,d)	((صحيح)luaL_optinteger(L, (n), (d)))

#تعریف_کرو luaL_checklong(L,n)	((لما)luaL_checkinteger(L, (n)))
#تعریف_کرو luaL_optlong(L,n,d)	((لما)luaL_optinteger(L, (n), (d)))

#جے_انت
/* }============================================================ */



#جے_انت


