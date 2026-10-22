/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>

#അന്തര്ഭാവ "luaconf.h"
#അന്തര്ഭാവ "lua.h"


/* global table */
#define LUA_GNAME	"_G"


തരനിർവചനം ഘടന luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


തരനിർവചനം ഘടന luaL_Reg {
  സ്ഥിരം അക്ഷരം *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(വലുപ്പം(lua_Integer)*16 + വലുപ്പം(lua_സംഖ്യ))

LUALIB_API ശൂന്യം (luaL_checkversion_) (lua_State *L, lua_സംഖ്യ ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_അക്കം, LUAL_NUMSIZES)

LUALIB_API പൂർണ്ണസംഖ്യ (luaL_getmetafield) (lua_State *L, പൂർണ്ണസംഖ്യ obj, സ്ഥിരം അക്ഷരം *e);
LUALIB_API പൂർണ്ണസംഖ്യ (luaL_callmeta) (lua_State *L, പൂർണ്ണസംഖ്യ obj, സ്ഥിരം അക്ഷരം *e);
LUALIB_API സ്ഥിരം അക്ഷരം *(luaL_tolstring) (lua_State *L, പൂർണ്ണസംഖ്യ idx, size_t *len);
LUALIB_API പൂർണ്ണസംഖ്യ (luaL_argerror) (lua_State *L, പൂർണ്ണസംഖ്യ arg, സ്ഥിരം അക്ഷരം *extramsg);
LUALIB_API പൂർണ്ണസംഖ്യ (luaL_typeerror) (lua_State *L, പൂർണ്ണസംഖ്യ arg, സ്ഥിരം അക്ഷരം *tname);
LUALIB_API സ്ഥിരം അക്ഷരം *(luaL_checklstring) (lua_State *L, പൂർണ്ണസംഖ്യ arg,
                                                          size_t *l);
LUALIB_API സ്ഥിരം അക്ഷരം *(luaL_optlstring) (lua_State *L, പൂർണ്ണസംഖ്യ arg,
                                          സ്ഥിരം അക്ഷരം *def, size_t *l);
LUALIB_API lua_സംഖ്യ (luaL_checknumber) (lua_State *L, പൂർണ്ണസംഖ്യ arg);
LUALIB_API lua_സംഖ്യ (luaL_optnumber) (lua_State *L, പൂർണ്ണസംഖ്യ arg, lua_സംഖ്യ def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, പൂർണ്ണസംഖ്യ arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, പൂർണ്ണസംഖ്യ arg,
                                          lua_Integer def);

LUALIB_API ശൂന്യം (luaL_checkstack) (lua_State *L, പൂർണ്ണസംഖ്യ sz, സ്ഥിരം അക്ഷരം *സന്ദേശം);
LUALIB_API ശൂന്യം (luaL_checktype) (lua_State *L, പൂർണ്ണസംഖ്യ arg, പൂർണ്ണസംഖ്യ t);
LUALIB_API ശൂന്യം (luaL_checkany) (lua_State *L, പൂർണ്ണസംഖ്യ arg);

LUALIB_API പൂർണ്ണസംഖ്യ   (luaL_newmetatable) (lua_State *L, സ്ഥിരം അക്ഷരം *tname);
LUALIB_API ശൂന്യം  (luaL_setmetatable) (lua_State *L, സ്ഥിരം അക്ഷരം *tname);
LUALIB_API ശൂന്യം *(luaL_testudata) (lua_State *L, പൂർണ്ണസംഖ്യ ud, സ്ഥിരം അക്ഷരം *tname);
LUALIB_API ശൂന്യം *(luaL_checkudata) (lua_State *L, പൂർണ്ണസംഖ്യ ud, സ്ഥിരം അക്ഷരം *tname);

LUALIB_API ശൂന്യം (luaL_where) (lua_State *L, പൂർണ്ണസംഖ്യ lvl);
LUALIB_API പൂർണ്ണസംഖ്യ (luaL_error) (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, ...);

LUALIB_API പൂർണ്ണസംഖ്യ (luaL_checkoption) (lua_State *L, പൂർണ്ണസംഖ്യ arg, സ്ഥിരം അക്ഷരം *def,
                                   സ്ഥിരം അക്ഷരം *സ്ഥിരം lst[]);

LUALIB_API പൂർണ്ണസംഖ്യ (luaL_fileresult) (lua_State *L, പൂർണ്ണസംഖ്യ stat, സ്ഥിരം അക്ഷരം *fname);
LUALIB_API പൂർണ്ണസംഖ്യ (luaL_execresult) (lua_State *L, പൂർണ്ണസംഖ്യ stat);

LUALIB_API ശൂന്യം *(luaL_alloc) (ശൂന്യം *ud, ശൂന്യം *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API പൂർണ്ണസംഖ്യ (luaL_ref) (lua_State *L, പൂർണ്ണസംഖ്യ t);
LUALIB_API ശൂന്യം (luaL_unref) (lua_State *L, പൂർണ്ണസംഖ്യ t, പൂർണ്ണസംഖ്യ ref);

LUALIB_API പൂർണ്ണസംഖ്യ (luaL_loadfilex) (lua_State *L, സ്ഥിരം അക്ഷരം *filename,
                                               സ്ഥിരം അക്ഷരം *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,ശൂന്യ)

LUALIB_API പൂർണ്ണസംഖ്യ (luaL_loadbufferx) (lua_State *L, സ്ഥിരം അക്ഷരം *buff, size_t sz,
                                   സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *mode);
LUALIB_API പൂർണ്ണസംഖ്യ (luaL_loadstring) (lua_State *L, സ്ഥിരം അക്ഷരം *s);

LUALIB_API lua_State *(luaL_newstate) (ശൂന്യം);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, പൂർണ്ണസംഖ്യ idx);

LUALIB_API ശൂന്യം (luaL_addgsub) (luaL_Buffer *b, സ്ഥിരം അക്ഷരം *s,
                                     സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *r);
LUALIB_API സ്ഥിരം അക്ഷരം *(luaL_gsub) (lua_State *L, സ്ഥിരം അക്ഷരം *s,
                                    സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *r);

LUALIB_API ശൂന്യം (luaL_setfuncs) (lua_State *L, സ്ഥിരം luaL_Reg *l, പൂർണ്ണസംഖ്യ nup);

LUALIB_API പൂർണ്ണസംഖ്യ (luaL_getsubtable) (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം അക്ഷരം *fname);

LUALIB_API ശൂന്യം (luaL_traceback) (lua_State *L, lua_State *L1,
                                  സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ level);

LUALIB_API ശൂന്യം (luaL_requiref) (lua_State *L, സ്ഥിരം അക്ഷരം *modname,
                                 lua_CFunction openf, പൂർണ്ണസംഖ്യ glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, വലുപ്പം(l)/വലുപ്പം((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((ശൂന്യം)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((ശൂന്യം)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ശൂന്യ))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ശൂന്യ))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ശൂന്യ)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#എങ്കിൽ defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#അല്ലെങ്കിൽ
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

ഘടന luaL_Buffer {
  അക്ഷരം *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  സംഘം {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    അക്ഷരം b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((ശൂന്യം)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API ശൂന്യം (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API അക്ഷരം *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API ശൂന്യം (luaL_addlstring) (luaL_Buffer *B, സ്ഥിരം അക്ഷരം *s, size_t l);
LUALIB_API ശൂന്യം (luaL_addstring) (luaL_Buffer *B, സ്ഥിരം അക്ഷരം *s);
LUALIB_API ശൂന്യം (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API ശൂന്യം (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API ശൂന്യം (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API അക്ഷരം *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

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


തരനിർവചനം ഘടന luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#എങ്കിൽ defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((പൂർണ്ണസംഖ്യ)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


