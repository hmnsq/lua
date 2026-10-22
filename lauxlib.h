/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>

#અન્તર્ભાવ "luaconf.h"
#અન્તર્ભાવ "lua.h"


/* global table */
#define LUA_GNAME	"_G"


પ્રકારવ્યાખ્યા સંરચના luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


પ્રકારવ્યાખ્યા સંરચના luaL_Reg {
  અચલ અક્ષર *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(કદ(lua_Integer)*16 + કદ(lua_સંખ્યા))

LUALIB_API શૂન્ય (luaL_checkversion_) (lua_State *L, lua_સંખ્યા ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_અંક, LUAL_NUMSIZES)

LUALIB_API પૂર્ણાંક (luaL_getmetafield) (lua_State *L, પૂર્ણાંક obj, અચલ અક્ષર *e);
LUALIB_API પૂર્ણાંક (luaL_callmeta) (lua_State *L, પૂર્ણાંક obj, અચલ અક્ષર *e);
LUALIB_API અચલ અક્ષર *(luaL_tolstring) (lua_State *L, પૂર્ણાંક idx, size_t *len);
LUALIB_API પૂર્ણાંક (luaL_argerror) (lua_State *L, પૂર્ણાંક arg, અચલ અક્ષર *extramsg);
LUALIB_API પૂર્ણાંક (luaL_typeerror) (lua_State *L, પૂર્ણાંક arg, અચલ અક્ષર *tname);
LUALIB_API અચલ અક્ષર *(luaL_checklstring) (lua_State *L, પૂર્ણાંક arg,
                                                          size_t *l);
LUALIB_API અચલ અક્ષર *(luaL_optlstring) (lua_State *L, પૂર્ણાંક arg,
                                          અચલ અક્ષર *def, size_t *l);
LUALIB_API lua_સંખ્યા (luaL_checknumber) (lua_State *L, પૂર્ણાંક arg);
LUALIB_API lua_સંખ્યા (luaL_optnumber) (lua_State *L, પૂર્ણાંક arg, lua_સંખ્યા def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, પૂર્ણાંક arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, પૂર્ણાંક arg,
                                          lua_Integer def);

LUALIB_API શૂન્ય (luaL_checkstack) (lua_State *L, પૂર્ણાંક sz, અચલ અક્ષર *સંદેશ);
LUALIB_API શૂન્ય (luaL_checktype) (lua_State *L, પૂર્ણાંક arg, પૂર્ણાંક t);
LUALIB_API શૂન્ય (luaL_checkany) (lua_State *L, પૂર્ણાંક arg);

LUALIB_API પૂર્ણાંક   (luaL_newmetatable) (lua_State *L, અચલ અક્ષર *tname);
LUALIB_API શૂન્ય  (luaL_setmetatable) (lua_State *L, અચલ અક્ષર *tname);
LUALIB_API શૂન્ય *(luaL_testudata) (lua_State *L, પૂર્ણાંક ud, અચલ અક્ષર *tname);
LUALIB_API શૂન્ય *(luaL_checkudata) (lua_State *L, પૂર્ણાંક ud, અચલ અક્ષર *tname);

LUALIB_API શૂન્ય (luaL_where) (lua_State *L, પૂર્ણાંક lvl);
LUALIB_API પૂર્ણાંક (luaL_error) (lua_State *L, અચલ અક્ષર *fmt, ...);

LUALIB_API પૂર્ણાંક (luaL_checkoption) (lua_State *L, પૂર્ણાંક arg, અચલ અક્ષર *def,
                                   અચલ અક્ષર *અચલ lst[]);

LUALIB_API પૂર્ણાંક (luaL_fileresult) (lua_State *L, પૂર્ણાંક stat, અચલ અક્ષર *fname);
LUALIB_API પૂર્ણાંક (luaL_execresult) (lua_State *L, પૂર્ણાંક stat);

LUALIB_API શૂન્ય *(luaL_alloc) (શૂન્ય *ud, શૂન્ય *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API પૂર્ણાંક (luaL_ref) (lua_State *L, પૂર્ણાંક t);
LUALIB_API શૂન્ય (luaL_unref) (lua_State *L, પૂર્ણાંક t, પૂર્ણાંક ref);

LUALIB_API પૂર્ણાંક (luaL_loadfilex) (lua_State *L, અચલ અક્ષર *filename,
                                               અચલ અક્ષર *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,ખાલી)

LUALIB_API પૂર્ણાંક (luaL_loadbufferx) (lua_State *L, અચલ અક્ષર *buff, size_t sz,
                                   અચલ અક્ષર *name, અચલ અક્ષર *mode);
LUALIB_API પૂર્ણાંક (luaL_loadstring) (lua_State *L, અચલ અક્ષર *s);

LUALIB_API lua_State *(luaL_newstate) (શૂન્ય);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, પૂર્ણાંક idx);

LUALIB_API શૂન્ય (luaL_addgsub) (luaL_Buffer *b, અચલ અક્ષર *s,
                                     અચલ અક્ષર *p, અચલ અક્ષર *r);
LUALIB_API અચલ અક્ષર *(luaL_gsub) (lua_State *L, અચલ અક્ષર *s,
                                    અચલ અક્ષર *p, અચલ અક્ષર *r);

LUALIB_API શૂન્ય (luaL_setfuncs) (lua_State *L, અચલ luaL_Reg *l, પૂર્ણાંક nup);

LUALIB_API પૂર્ણાંક (luaL_getsubtable) (lua_State *L, પૂર્ણાંક idx, અચલ અક્ષર *fname);

LUALIB_API શૂન્ય (luaL_traceback) (lua_State *L, lua_State *L1,
                                  અચલ અક્ષર *સંદેશ, પૂર્ણાંક level);

LUALIB_API શૂન્ય (luaL_requiref) (lua_State *L, અચલ અક્ષર *modname,
                                 lua_CFunction openf, પૂર્ણાંક glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, કદ(l)/કદ((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((શૂન્ય)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((શૂન્ય)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ખાલી))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ખાલી))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ખાલી)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#જો defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#નહીંતર
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

સંરચના luaL_Buffer {
  અક્ષર *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  સંઘ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    અક્ષર b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((શૂન્ય)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API શૂન્ય (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API અક્ષર *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API શૂન્ય (luaL_addlstring) (luaL_Buffer *B, અચલ અક્ષર *s, size_t l);
LUALIB_API શૂન્ય (luaL_addstring) (luaL_Buffer *B, અચલ અક્ષર *s);
LUALIB_API શૂન્ય (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API શૂન્ય (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API શૂન્ય (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API અક્ષર *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

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


પ્રકારવ્યાખ્યા સંરચના luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#જો defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((પૂર્ણાંક)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((પૂર્ણાંક)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


