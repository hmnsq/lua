/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ಅನ್ತರ್ಭಾವ "luaconf.h"
#ಅನ್ತರ್ಭಾವ "lua.h"


/* global table */
#define LUA_GNAME	"_G"


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ luaL_Reg {
  ಧ್ರುವ ವರ್ಣ *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(ಪರಿಮಾಣ(lua_Integer)*16 + ಪರಿಮಾಣ(lua_ಸಂಖ್ಯೆ))

LUALIB_API ಶೂನ್ಯ (luaL_checkversion_) (lua_State *L, lua_ಸಂಖ್ಯೆ ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_ಅಂಕೆ, LUAL_NUMSIZES)

LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_getmetafield) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ obj, ಧ್ರುವ ವರ್ಣ *e);
LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_callmeta) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ obj, ಧ್ರುವ ವರ್ಣ *e);
LUALIB_API ಧ್ರುವ ವರ್ಣ *(luaL_tolstring) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, size_t *len);
LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_argerror) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಧ್ರುವ ವರ್ಣ *extramsg);
LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_typeerror) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಧ್ರುವ ವರ್ಣ *tname);
LUALIB_API ಧ್ರುವ ವರ್ಣ *(luaL_checklstring) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg,
                                                          size_t *l);
LUALIB_API ಧ್ರುವ ವರ್ಣ *(luaL_optlstring) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg,
                                          ಧ್ರುವ ವರ್ಣ *def, size_t *l);
LUALIB_API lua_ಸಂಖ್ಯೆ (luaL_checknumber) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg);
LUALIB_API lua_ಸಂಖ್ಯೆ (luaL_optnumber) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, lua_ಸಂಖ್ಯೆ def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg,
                                          lua_Integer def);

LUALIB_API ಶೂನ್ಯ (luaL_checkstack) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ sz, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ);
LUALIB_API ಶೂನ್ಯ (luaL_checktype) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಪೂರ್ಣಾಙ್ಕ t);
LUALIB_API ಶೂನ್ಯ (luaL_checkany) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg);

LUALIB_API ಪೂರ್ಣಾಙ್ಕ   (luaL_newmetatable) (lua_State *L, ಧ್ರುವ ವರ್ಣ *tname);
LUALIB_API ಶೂನ್ಯ  (luaL_setmetatable) (lua_State *L, ಧ್ರುವ ವರ್ಣ *tname);
LUALIB_API ಶೂನ್ಯ *(luaL_testudata) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ ud, ಧ್ರುವ ವರ್ಣ *tname);
LUALIB_API ಶೂನ್ಯ *(luaL_checkudata) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ ud, ಧ್ರುವ ವರ್ಣ *tname);

LUALIB_API ಶೂನ್ಯ (luaL_where) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ lvl);
LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_error) (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...);

LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_checkoption) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಧ್ರುವ ವರ್ಣ *def,
                                   ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ lst[]);

LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_fileresult) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ stat, ಧ್ರುವ ವರ್ಣ *fname);
LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_execresult) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ stat);

LUALIB_API ಶೂನ್ಯ *(luaL_alloc) (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_ref) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ t);
LUALIB_API ಶೂನ್ಯ (luaL_unref) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ t, ಪೂರ್ಣಾಙ್ಕ ref);

LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_loadfilex) (lua_State *L, ಧ್ರುವ ವರ್ಣ *filename,
                                               ಧ್ರುವ ವರ್ಣ *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,ರಿಕ್ತ)

LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_loadbufferx) (lua_State *L, ಧ್ರುವ ವರ್ಣ *buff, size_t sz,
                                   ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *mode);
LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_loadstring) (lua_State *L, ಧ್ರುವ ವರ್ಣ *s);

LUALIB_API lua_State *(luaL_newstate) (ಶೂನ್ಯ);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);

LUALIB_API ಶೂನ್ಯ (luaL_addgsub) (luaL_Buffer *b, ಧ್ರುವ ವರ್ಣ *s,
                                     ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *r);
LUALIB_API ಧ್ರುವ ವರ್ಣ *(luaL_gsub) (lua_State *L, ಧ್ರುವ ವರ್ಣ *s,
                                    ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *r);

LUALIB_API ಶೂನ್ಯ (luaL_setfuncs) (lua_State *L, ಧ್ರುವ luaL_Reg *l, ಪೂರ್ಣಾಙ್ಕ nup);

LUALIB_API ಪೂರ್ಣಾಙ್ಕ (luaL_getsubtable) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *fname);

LUALIB_API ಶೂನ್ಯ (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ, ಪೂರ್ಣಾಙ್ಕ level);

LUALIB_API ಶೂನ್ಯ (luaL_requiref) (lua_State *L, ಧ್ರುವ ವರ್ಣ *modname,
                                 lua_CFunction openf, ಪೂರ್ಣಾಙ್ಕ glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, ಪರಿಮಾಣ(l)/ಪರಿಮಾಣ((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((ಶೂನ್ಯ)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((ಶೂನ್ಯ)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ರಿಕ್ತ))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ರಿಕ್ತ))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ರಿಕ್ತ)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#ಯದಿ defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#ಅನ್ಯಥಾ
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

ಸಂರಚನಾ luaL_Buffer {
  ವರ್ಣ *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  ಸಙ್ಘ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    ವರ್ಣ b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((ಶೂನ್ಯ)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API ಶೂನ್ಯ (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API ವರ್ಣ *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API ಶೂನ್ಯ (luaL_addlstring) (luaL_Buffer *B, ಧ್ರುವ ವರ್ಣ *s, size_t l);
LUALIB_API ಶೂನ್ಯ (luaL_addstring) (luaL_Buffer *B, ಧ್ರುವ ವರ್ಣ *s);
LUALIB_API ಶೂನ್ಯ (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API ಶೂನ್ಯ (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API ಶೂನ್ಯ (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API ವರ್ಣ *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

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


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#ಯದಿ defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((ಪೂರ್ಣಾಙ್ಕ)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


