/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#include "luaconf.h"
#include "lua.h"


/* global table */
#define LUA_GNAME	"_G"


typedef sa_nracanaa luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


typedef sa_nracanaa luaL_Reg {
  const var_na *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(parimaa_na(lua_Integer)*16 + parimaa_na(lua_Number))

LUALIB_API void (luaL_checkversion_) (lua_State *L, lua_Number ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_NUM, LUAL_NUMSIZES)

LUALIB_API puur_naa_nka (luaL_getmetafield) (lua_State *L, puur_naa_nka obj, const var_na *e);
LUALIB_API puur_naa_nka (luaL_callmeta) (lua_State *L, puur_naa_nka obj, const var_na *e);
LUALIB_API const var_na *(luaL_tolstring) (lua_State *L, puur_naa_nka idx, size_t *len);
LUALIB_API puur_naa_nka (luaL_argerror) (lua_State *L, puur_naa_nka arg, const var_na *extramsg);
LUALIB_API puur_naa_nka (luaL_typeerror) (lua_State *L, puur_naa_nka arg, const var_na *tname);
LUALIB_API const var_na *(luaL_checklstring) (lua_State *L, puur_naa_nka arg,
                                                          size_t *l);
LUALIB_API const var_na *(luaL_optlstring) (lua_State *L, puur_naa_nka arg,
                                          const var_na *def, size_t *l);
LUALIB_API lua_Number (luaL_checknumber) (lua_State *L, puur_naa_nka arg);
LUALIB_API lua_Number (luaL_optnumber) (lua_State *L, puur_naa_nka arg, lua_Number def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, puur_naa_nka arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, puur_naa_nka arg,
                                          lua_Integer def);

LUALIB_API void (luaL_checkstack) (lua_State *L, puur_naa_nka sz, const var_na *msg);
LUALIB_API void (luaL_checktype) (lua_State *L, puur_naa_nka arg, puur_naa_nka t);
LUALIB_API void (luaL_checkany) (lua_State *L, puur_naa_nka arg);

LUALIB_API puur_naa_nka   (luaL_newmetatable) (lua_State *L, const var_na *tname);
LUALIB_API void  (luaL_setmetatable) (lua_State *L, const var_na *tname);
LUALIB_API void *(luaL_testudata) (lua_State *L, puur_naa_nka ud, const var_na *tname);
LUALIB_API void *(luaL_checkudata) (lua_State *L, puur_naa_nka ud, const var_na *tname);

LUALIB_API void (luaL_where) (lua_State *L, puur_naa_nka lvl);
LUALIB_API puur_naa_nka (luaL_error) (lua_State *L, const var_na *fmt, ...);

LUALIB_API puur_naa_nka (luaL_checkoption) (lua_State *L, puur_naa_nka arg, const var_na *def,
                                   const var_na *const lst[]);

LUALIB_API puur_naa_nka (luaL_fileresult) (lua_State *L, puur_naa_nka stat, const var_na *fname);
LUALIB_API puur_naa_nka (luaL_execresult) (lua_State *L, puur_naa_nka stat);

LUALIB_API void *(luaL_alloc) (void *ud, void *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API puur_naa_nka (luaL_ref) (lua_State *L, puur_naa_nka t);
LUALIB_API void (luaL_unref) (lua_State *L, puur_naa_nka t, puur_naa_nka ref);

LUALIB_API puur_naa_nka (luaL_loadfilex) (lua_State *L, const var_na *filename,
                                               const var_na *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,NULL)

LUALIB_API puur_naa_nka (luaL_loadbufferx) (lua_State *L, const var_na *buff, size_t sz,
                                   const var_na *name, const var_na *mode);
LUALIB_API puur_naa_nka (luaL_loadstring) (lua_State *L, const var_na *s);

LUALIB_API lua_State *(luaL_newstate) (void);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, puur_naa_nka idx);

LUALIB_API void (luaL_addgsub) (luaL_Buffer *b, const var_na *s,
                                     const var_na *p, const var_na *r);
LUALIB_API const var_na *(luaL_gsub) (lua_State *L, const var_na *s,
                                    const var_na *p, const var_na *r);

LUALIB_API void (luaL_setfuncs) (lua_State *L, const luaL_Reg *l, puur_naa_nka nup);

LUALIB_API puur_naa_nka (luaL_getsubtable) (lua_State *L, puur_naa_nka idx, const var_na *fname);

LUALIB_API void (luaL_traceback) (lua_State *L, lua_State *L1,
                                  const var_na *msg, puur_naa_nka level);

LUALIB_API void (luaL_requiref) (lua_State *L, const var_na *modname,
                                 lua_CFunction openf, puur_naa_nka glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, parimaa_na(l)/parimaa_na((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((void)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((void)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,NULL)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#ᨀᨘ defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#ᨀᨘᨁ
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

sa_nracanaa luaL_Buffer {
  var_na *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  sa_ngha {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    var_na b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((void)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API void (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API var_na *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API void (luaL_addlstring) (luaL_Buffer *B, const var_na *s, size_t l);
LUALIB_API void (luaL_addstring) (luaL_Buffer *B, const var_na *s);
LUALIB_API void (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API void (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API void (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API var_na *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

#define luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

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

#define LUA_FILEHANDLE          "FILE*"


typedef sa_nracanaa luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#ᨀᨘ defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((puur_naa_nka)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((puur_naa_nka)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


