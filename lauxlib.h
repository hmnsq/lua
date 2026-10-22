/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lauxlib_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lauxlib_h


#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "luaconf.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"


/* global table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GNAME	"_G"


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ lua_L_ਬਫਰ lua_L_ਬਫਰ;


/* extra error code for 'luaL_loadfilex' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LOADED_ਸਾਰਣੀ	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_PRELOAD_ਸਾਰਣੀ	"_PRELOAD"


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ luaL_Reg {
  ਅਚਲ ਅੱਖਰ *name;
  lua_CFunction func;
} luaL_Reg;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAL_NUMSIZES	(ਆਕਾਰ(lua_Integer)*16 + ਆਕਾਰ(lua_ਸੰਖਿਆ))

LUALIB_API ਖਾਲੀ (luaL_checkversion_) (lua_State *L, lua_ਸੰਖਿਆ ver, size_t sz);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_ਸੰਸਕਰਣ_ਅੰਕ, LUAL_NUMSIZES)

LUALIB_API ਪੂਰਨ (luaL_getmetafield) (lua_State *L, ਪੂਰਨ obj, ਅਚਲ ਅੱਖਰ *e);
LUALIB_API ਪੂਰਨ (luaL_callmeta) (lua_State *L, ਪੂਰਨ obj, ਅਚਲ ਅੱਖਰ *e);
LUALIB_API ਅਚਲ ਅੱਖਰ *(luaL_tolstring) (lua_State *L, ਪੂਰਨ idx, size_t *len);
LUALIB_API ਪੂਰਨ (luaL_argerror) (lua_State *L, ਪੂਰਨ arg, ਅਚਲ ਅੱਖਰ *extramsg);
LUALIB_API ਪੂਰਨ (luaL_typeerror) (lua_State *L, ਪੂਰਨ arg, ਅਚਲ ਅੱਖਰ *tname);
LUALIB_API ਅਚਲ ਅੱਖਰ *(luaL_checklstring) (lua_State *L, ਪੂਰਨ arg,
                                                          size_t *l);
LUALIB_API ਅਚਲ ਅੱਖਰ *(luaL_optlstring) (lua_State *L, ਪੂਰਨ arg,
                                          ਅਚਲ ਅੱਖਰ *def, size_t *l);
LUALIB_API lua_ਸੰਖਿਆ (luaL_checknumber) (lua_State *L, ਪੂਰਨ arg);
LUALIB_API lua_ਸੰਖਿਆ (luaL_optnumber) (lua_State *L, ਪੂਰਨ arg, lua_ਸੰਖਿਆ def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, ਪੂਰਨ arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, ਪੂਰਨ arg,
                                          lua_Integer def);

LUALIB_API ਖਾਲੀ (luaL_checkstack) (lua_State *L, ਪੂਰਨ sz, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ);
LUALIB_API ਖਾਲੀ (luaL_checktype) (lua_State *L, ਪੂਰਨ arg, ਪੂਰਨ t);
LUALIB_API ਖਾਲੀ (luaL_checkany) (lua_State *L, ਪੂਰਨ arg);

LUALIB_API ਪੂਰਨ   (luaL_newmetatable) (lua_State *L, ਅਚਲ ਅੱਖਰ *tname);
LUALIB_API ਖਾਲੀ  (luaL_setmetatable) (lua_State *L, ਅਚਲ ਅੱਖਰ *tname);
LUALIB_API ਖਾਲੀ *(luaL_testudata) (lua_State *L, ਪੂਰਨ ud, ਅਚਲ ਅੱਖਰ *tname);
LUALIB_API ਖਾਲੀ *(luaL_checkudata) (lua_State *L, ਪੂਰਨ ud, ਅਚਲ ਅੱਖਰ *tname);

LUALIB_API ਖਾਲੀ (luaL_where) (lua_State *L, ਪੂਰਨ lvl);
LUALIB_API ਪੂਰਨ (luaL_error) (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...);

LUALIB_API ਪੂਰਨ (luaL_checkoption) (lua_State *L, ਪੂਰਨ arg, ਅਚਲ ਅੱਖਰ *def,
                                   ਅਚਲ ਅੱਖਰ *ਅਚਲ lst[]);

LUALIB_API ਪੂਰਨ (luaL_fileresult) (lua_State *L, ਪੂਰਨ stat, ਅਚਲ ਅੱਖਰ *fname);
LUALIB_API ਪੂਰਨ (luaL_execresult) (lua_State *L, ਪੂਰਨ stat);

LUALIB_API ਖਾਲੀ *(luaL_alloc) (ਖਾਲੀ *ud, ਖਾਲੀ *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_NOREF       (-2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_REFNIL      (-1)

LUALIB_API ਪੂਰਨ (luaL_ref) (lua_State *L, ਪੂਰਨ t);
LUALIB_API ਖਾਲੀ (luaL_unref) (lua_State *L, ਪੂਰਨ t, ਪੂਰਨ ref);

LUALIB_API ਪੂਰਨ (luaL_loadfilex) (lua_State *L, ਅਚਲ ਅੱਖਰ *filename,
                                               ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_loadfile(L,f)	luaL_loadfilex(L,f,ਨਿਰਾ)

LUALIB_API ਪੂਰਨ (luaL_loadbufferx) (lua_State *L, ਅਚਲ ਅੱਖਰ *buff, size_t sz,
                                   ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ);
LUALIB_API ਪੂਰਨ (luaL_loadstring) (lua_State *L, ਅਚਲ ਅੱਖਰ *s);

LUALIB_API lua_State *(luaL_newstate) (ਖਾਲੀ);

LUALIB_API ਬਿਨਾਨਿਸ਼ਾਨੀ (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, ਪੂਰਨ idx);

LUALIB_API ਖਾਲੀ (luaL_addgsub) (lua_L_ਬਫਰ *b, ਅਚਲ ਅੱਖਰ *s,
                                     ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *r);
LUALIB_API ਅਚਲ ਅੱਖਰ *(luaL_gsub) (lua_State *L, ਅਚਲ ਅੱਖਰ *s,
                                    ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *r);

LUALIB_API ਖਾਲੀ (luaL_setfuncs) (lua_State *L, ਅਚਲ luaL_Reg *l, ਪੂਰਨ nup);

LUALIB_API ਪੂਰਨ (luaL_getsubtable) (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *fname);

LUALIB_API ਖਾਲੀ (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ level);

LUALIB_API ਖਾਲੀ (luaL_requiref) (lua_State *L, ਅਚਲ ਅੱਖਰ *modname,
                                 lua_CFunction openf, ਪੂਰਨ glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, ਆਕਾਰ(l)/ਆਕਾਰ((l)[0]) - 1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_argcheck(L, cond,arg,extramsg)	\
	((ਖਾਲੀ)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_argexpected(L,cond,arg,tname)	\
	((ਖਾਲੀ)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ਨਿਰਾ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ਨਿਰਾ))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ਨਿਰਾ)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#ਜੇ defined(LUA_FAILISFALSE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_pushfail(L)	lua_pushboolean(L, 0)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_pushfail(L)	lua_pushnil(L)
#ਜੇ_ਅੰਤ



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

ਬਣਤਰ lua_L_ਬਫਰ {
  ਅੱਖਰ *b;  /* buffer address */
  size_t ਮਾਪ;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  ਸੰਘ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    ਅੱਖਰ b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_bufflen(bf)	((bf)->n)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_buffaddr(bf)	((bf)->b)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_addchar(B,c) \
  ((ਖਾਲੀ)((B)->n < (B)->ਮਾਪ || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_addsize(B,s)	((B)->n += (s))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API ਖਾਲੀ (luaL_buffinit) (lua_State *L, lua_L_ਬਫਰ *B);
LUALIB_API ਅੱਖਰ *(luaL_prepbuffsize) (lua_L_ਬਫਰ *B, size_t sz);
LUALIB_API ਖਾਲੀ (luaL_addlstring) (lua_L_ਬਫਰ *B, ਅਚਲ ਅੱਖਰ *s, size_t l);
LUALIB_API ਖਾਲੀ (luaL_addstring) (lua_L_ਬਫਰ *B, ਅਚਲ ਅੱਖਰ *s);
LUALIB_API ਖਾਲੀ (luaL_addvalue) (lua_L_ਬਫਰ *B);
LUALIB_API ਖਾਲੀ (luaL_pushresult) (lua_L_ਬਫਰ *B);
LUALIB_API ਖਾਲੀ (luaL_pushresultsize) (lua_L_ਬਫਰ *B, size_t sz);
LUALIB_API ਅੱਖਰ *(luaL_buffinitsize) (lua_State *L, lua_L_ਬਫਰ *B, size_t sz);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_FILEHANDLE          "FILE*"


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ lua_L_ਧਾਰਾ {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} lua_L_ਧਾਰਾ;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#ਜੇ defined(LUA_COMPAT_APIINTCASTS)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_checkint(L,n)	((ਪੂਰਨ)luaL_checkinteger(L, (n)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_optint(L,n,d)	((ਪੂਰਨ)luaL_optinteger(L, (n), (d)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_checklong(L,n)	((ਲੰਮਾ)luaL_checkinteger(L, (n)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_optlong(L,n,d)	((ਲੰਮਾ)luaL_optinteger(L, (n), (d)))

#ਜੇ_ਅੰਤ
/* }============================================================ */



#ਜੇ_ਅੰਤ


