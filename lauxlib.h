/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>

#அந்தர்பாவ "luaconf.h"
#அந்தர்பாவ "lua.h"


/* global table */
#define LUA_GNAME	"_G"


வகைவரை கட்டமைப்பு luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


வகைவரை கட்டமைப்பு luaL_Reg {
  மாறிலி எழுத்து *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(அளவு(lua_Integer)*16 + அளவு(lua_எண்))

LUALIB_API வெறுமை (luaL_checkversion_) (lua_State *L, lua_எண் ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_இலக்கம், LUAL_NUMSIZES)

LUALIB_API முழு (luaL_getmetafield) (lua_State *L, முழு obj, மாறிலி எழுத்து *e);
LUALIB_API முழு (luaL_callmeta) (lua_State *L, முழு obj, மாறிலி எழுத்து *e);
LUALIB_API மாறிலி எழுத்து *(luaL_tolstring) (lua_State *L, முழு idx, size_t *len);
LUALIB_API முழு (luaL_argerror) (lua_State *L, முழு arg, மாறிலி எழுத்து *extramsg);
LUALIB_API முழு (luaL_typeerror) (lua_State *L, முழு arg, மாறிலி எழுத்து *tname);
LUALIB_API மாறிலி எழுத்து *(luaL_checklstring) (lua_State *L, முழு arg,
                                                          size_t *l);
LUALIB_API மாறிலி எழுத்து *(luaL_optlstring) (lua_State *L, முழு arg,
                                          மாறிலி எழுத்து *def, size_t *l);
LUALIB_API lua_எண் (luaL_checknumber) (lua_State *L, முழு arg);
LUALIB_API lua_எண் (luaL_optnumber) (lua_State *L, முழு arg, lua_எண் def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, முழு arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, முழு arg,
                                          lua_Integer def);

LUALIB_API வெறுமை (luaL_checkstack) (lua_State *L, முழு sz, மாறிலி எழுத்து *செய்தி);
LUALIB_API வெறுமை (luaL_checktype) (lua_State *L, முழு arg, முழு t);
LUALIB_API வெறுமை (luaL_checkany) (lua_State *L, முழு arg);

LUALIB_API முழு   (luaL_newmetatable) (lua_State *L, மாறிலி எழுத்து *tname);
LUALIB_API வெறுமை  (luaL_setmetatable) (lua_State *L, மாறிலி எழுத்து *tname);
LUALIB_API வெறுமை *(luaL_testudata) (lua_State *L, முழு ud, மாறிலி எழுத்து *tname);
LUALIB_API வெறுமை *(luaL_checkudata) (lua_State *L, முழு ud, மாறிலி எழுத்து *tname);

LUALIB_API வெறுமை (luaL_where) (lua_State *L, முழு lvl);
LUALIB_API முழு (luaL_error) (lua_State *L, மாறிலி எழுத்து *fmt, ...);

LUALIB_API முழு (luaL_checkoption) (lua_State *L, முழு arg, மாறிலி எழுத்து *def,
                                   மாறிலி எழுத்து *மாறிலி lst[]);

LUALIB_API முழு (luaL_fileresult) (lua_State *L, முழு stat, மாறிலி எழுத்து *fname);
LUALIB_API முழு (luaL_execresult) (lua_State *L, முழு stat);

LUALIB_API வெறுமை *(luaL_alloc) (வெறுமை *ud, வெறுமை *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API முழு (luaL_ref) (lua_State *L, முழு t);
LUALIB_API வெறுமை (luaL_unref) (lua_State *L, முழு t, முழு ref);

LUALIB_API முழு (luaL_loadfilex) (lua_State *L, மாறிலி எழுத்து *filename,
                                               மாறிலி எழுத்து *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,வெற்று)

LUALIB_API முழு (luaL_loadbufferx) (lua_State *L, மாறிலி எழுத்து *buff, size_t sz,
                                   மாறிலி எழுத்து *name, மாறிலி எழுத்து *mode);
LUALIB_API முழு (luaL_loadstring) (lua_State *L, மாறிலி எழுத்து *s);

LUALIB_API lua_State *(luaL_newstate) (வெறுமை);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, முழு idx);

LUALIB_API வெறுமை (luaL_addgsub) (luaL_Buffer *b, மாறிலி எழுத்து *s,
                                     மாறிலி எழுத்து *p, மாறிலி எழுத்து *r);
LUALIB_API மாறிலி எழுத்து *(luaL_gsub) (lua_State *L, மாறிலி எழுத்து *s,
                                    மாறிலி எழுத்து *p, மாறிலி எழுத்து *r);

LUALIB_API வெறுமை (luaL_setfuncs) (lua_State *L, மாறிலி luaL_Reg *l, முழு nup);

LUALIB_API முழு (luaL_getsubtable) (lua_State *L, முழு idx, மாறிலி எழுத்து *fname);

LUALIB_API வெறுமை (luaL_traceback) (lua_State *L, lua_State *L1,
                                  மாறிலி எழுத்து *செய்தி, முழு level);

LUALIB_API வெறுமை (luaL_requiref) (lua_State *L, மாறிலி எழுத்து *modname,
                                 lua_CFunction openf, முழு glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, அளவு(l)/அளவு((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((வெறுமை)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((வெறுமை)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), வெற்று))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), வெற்று))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,வெற்று)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#எனில் defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#இல்லையெனில்
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

கட்டமைப்பு luaL_Buffer {
  எழுத்து *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  ஒன்றிப்பு {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    எழுத்து b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((வெறுமை)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API வெறுமை (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API எழுத்து *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API வெறுமை (luaL_addlstring) (luaL_Buffer *B, மாறிலி எழுத்து *s, size_t l);
LUALIB_API வெறுமை (luaL_addstring) (luaL_Buffer *B, மாறிலி எழுத்து *s);
LUALIB_API வெறுமை (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API வெறுமை (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API வெறுமை (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API எழுத்து *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

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


வகைவரை கட்டமைப்பு luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#எனில் defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((முழு)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((முழு)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


