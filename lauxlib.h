/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lauxlib_h
#ವ್ಯಾಖ್ಯಾನಿಸು lauxlib_h


#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ಅನ್ತರ್ಭಾವ "luaconf.h"
#ಅನ್ತರ್ಭಾವ "lua.h"


/* global table */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GNAME	"_G"


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ lua_L_ಬಫರ್ lua_L_ಬಫರ್;


/* extra error code for 'luaL_loadfilex' */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LOADED_ಕೋಷ್ಟಕ	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_PRELOAD_ಕೋಷ್ಟಕ	"_PRELOAD"


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ luaL_Reg {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  lua_CFunction func;
} luaL_Reg;


#ವ್ಯಾಖ್ಯಾನಿಸು LUAL_NUMSIZES	(ಗಾತ್ರ(lua_Integer)*16 + ಗಾತ್ರ(lua_ಸಂಖ್ಯೆ))

LUALIB_API ಶೂನ್ಯ (luaL_checkversion_) (lua_State *L, lua_ಸಂಖ್ಯೆ ver, size_t sz);
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_ಆವೃತ್ತಿ_ಅಂಕೆ, LUAL_NUMSIZES)

LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_getmetafield) (lua_State *L, ಪೂರ್ಣಾಂಕ obj, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e);
LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_callmeta) (lua_State *L, ಪೂರ್ಣಾಂಕ obj, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e);
LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(luaL_tolstring) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, size_t *len);
LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_argerror) (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extramsg);
LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_typeerror) (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname);
LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(luaL_checklstring) (lua_State *L, ಪೂರ್ಣಾಂಕ arg,
                                                          size_t *l);
LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(luaL_optlstring) (lua_State *L, ಪೂರ್ಣಾಂಕ arg,
                                          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *def, size_t *l);
LUALIB_API lua_ಸಂಖ್ಯೆ (luaL_checknumber) (lua_State *L, ಪೂರ್ಣಾಂಕ arg);
LUALIB_API lua_ಸಂಖ್ಯೆ (luaL_optnumber) (lua_State *L, ಪೂರ್ಣಾಂಕ arg, lua_ಸಂಖ್ಯೆ def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, ಪೂರ್ಣಾಂಕ arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, ಪೂರ್ಣಾಂಕ arg,
                                          lua_Integer def);

LUALIB_API ಶೂನ್ಯ (luaL_checkstack) (lua_State *L, ಪೂರ್ಣಾಂಕ sz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ);
LUALIB_API ಶೂನ್ಯ (luaL_checktype) (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಪೂರ್ಣಾಂಕ t);
LUALIB_API ಶೂನ್ಯ (luaL_checkany) (lua_State *L, ಪೂರ್ಣಾಂಕ arg);

LUALIB_API ಪೂರ್ಣಾಂಕ   (luaL_newmetatable) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname);
LUALIB_API ಶೂನ್ಯ  (luaL_setmetatable) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname);
LUALIB_API ಶೂನ್ಯ *(luaL_testudata) (lua_State *L, ಪೂರ್ಣಾಂಕ ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname);
LUALIB_API ಶೂನ್ಯ *(luaL_checkudata) (lua_State *L, ಪೂರ್ಣಾಂಕ ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname);

LUALIB_API ಶೂನ್ಯ (luaL_where) (lua_State *L, ಪೂರ್ಣಾಂಕ lvl);
LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_error) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...);

LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_checkoption) (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *def,
                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ lst[]);

LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_fileresult) (lua_State *L, ಪೂರ್ಣಾಂಕ stat, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);
LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_execresult) (lua_State *L, ಪೂರ್ಣಾಂಕ stat);

LUALIB_API ಶೂನ್ಯ *(luaL_alloc) (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_NOREF       (-2)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_REFNIL      (-1)

LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_ref) (lua_State *L, ಪೂರ್ಣಾಂಕ t);
LUALIB_API ಶೂನ್ಯ (luaL_unref) (lua_State *L, ಪೂರ್ಣಾಂಕ t, ಪೂರ್ಣಾಂಕ ref);

LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_loadfilex) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
                                               ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ);

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_loadfile(L,f)	luaL_loadfilex(L,f,ಖಾಲಿ)

LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_loadbufferx) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buff, size_t sz,
                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ);
LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_loadstring) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);

LUALIB_API lua_State *(luaL_newstate) (ಶೂನ್ಯ);

LUALIB_API ಚಿಹ್ನೆರಹಿತ (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);

LUALIB_API ಶೂನ್ಯ (luaL_addgsub) (lua_L_ಬಫರ್ *b, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *r);
LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(luaL_gsub) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *r);

LUALIB_API ಶೂನ್ಯ (luaL_setfuncs) (lua_State *L, ಸ್ಥಿರಾಂಕ luaL_Reg *l, ಪೂರ್ಣಾಂಕ nup);

LUALIB_API ಪೂರ್ಣಾಂಕ (luaL_getsubtable) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname);

LUALIB_API ಶೂನ್ಯ (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ level);

LUALIB_API ಶೂನ್ಯ (luaL_requiref) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *modname,
                                 lua_CFunction openf, ಪೂರ್ಣಾಂಕ glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#ವ್ಯಾಖ್ಯಾನಿಸು luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, ಗಾತ್ರ(l)/ಗಾತ್ರ((l)[0]) - 1)

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_argcheck(L, cond,arg,extramsg)	\
	((ಶೂನ್ಯ)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_argexpected(L,cond,arg,tname)	\
	((ಶೂನ್ಯ)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ಖಾಲಿ))
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ಖಾಲಿ))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ಖಾಲಿ)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#ಆದರೆ defined(LUA_FAILISFALSE)
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_pushfail(L)	lua_pushboolean(L, 0)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_pushfail(L)	lua_pushnil(L)
#ಆದರೆ_ಅಂತ್ಯ



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

ರಚನೆ lua_L_ಬಫರ್ {
  ಅಕ್ಷರ *b;  /* buffer address */
  size_t ಗಾತ್ರ_2;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  ಒಕ್ಕೂಟ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    ಅಕ್ಷರ b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#ವ್ಯಾಖ್ಯಾನಿಸು luaL_bufflen(bf)	((bf)->n)
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_buffaddr(bf)	((bf)->b)


#ವ್ಯಾಖ್ಯಾನಿಸು luaL_addchar(B,c) \
  ((ಶೂನ್ಯ)((B)->n < (B)->ಗಾತ್ರ_2 || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_addsize(B,s)	((B)->n += (s))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API ಶೂನ್ಯ (luaL_buffinit) (lua_State *L, lua_L_ಬಫರ್ *B);
LUALIB_API ಅಕ್ಷರ *(luaL_prepbuffsize) (lua_L_ಬಫರ್ *B, size_t sz);
LUALIB_API ಶೂನ್ಯ (luaL_addlstring) (lua_L_ಬಫರ್ *B, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t l);
LUALIB_API ಶೂನ್ಯ (luaL_addstring) (lua_L_ಬಫರ್ *B, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
LUALIB_API ಶೂನ್ಯ (luaL_addvalue) (lua_L_ಬಫರ್ *B);
LUALIB_API ಶೂನ್ಯ (luaL_pushresult) (lua_L_ಬಫರ್ *B);
LUALIB_API ಶೂನ್ಯ (luaL_pushresultsize) (lua_L_ಬಫರ್ *B, size_t sz);
LUALIB_API ಅಕ್ಷರ *(luaL_buffinitsize) (lua_State *L, lua_L_ಬಫರ್ *B, size_t sz);

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

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

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_FILEHANDLE          "FILE*"


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ lua_L_ಹರಿವು {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} lua_L_ಹರಿವು;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#ಆದರೆ defined(LUA_COMPAT_APIINTCASTS)

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_checkint(L,n)	((ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, (n)))
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_optint(L,n,d)	((ಪೂರ್ಣಾಂಕ)luaL_optinteger(L, (n), (d)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaL_checklong(L,n)	((ದೀರ್ಘ)luaL_checkinteger(L, (n)))
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_optlong(L,n,d)	((ದೀರ್ಘ)luaL_optinteger(L, (n), (d)))

#ಆದರೆ_ಅಂತ್ಯ
/* }============================================================ */



#ಆದರೆ_ಅಂತ್ಯ


