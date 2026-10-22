/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ଅନ୍ତର୍ଭାଵ "luaconf.h"
#ଅନ୍ତର୍ଭାଵ "lua.h"


/* global table */
#define LUA_GNAME	"_G"


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା luaL_Reg {
  ଅଚଳ ଅକ୍ଷର *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(ଆକାର(lua_Integer)*16 + ଆକାର(lua_ସଂଖ୍ୟା))

LUALIB_API ଶୂନ୍ୟ (luaL_checkversion_) (lua_State *L, lua_ସଂଖ୍ୟା ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_ଅଙ୍କ, LUAL_NUMSIZES)

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_getmetafield) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା obj, ଅଚଳ ଅକ୍ଷର *e);
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_callmeta) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା obj, ଅଚଳ ଅକ୍ଷର *e);
LUALIB_API ଅଚଳ ଅକ୍ଷର *(luaL_tolstring) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, size_t *len);
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_argerror) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ଅଚଳ ଅକ୍ଷର *extramsg);
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_typeerror) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ଅଚଳ ଅକ୍ଷର *tname);
LUALIB_API ଅଚଳ ଅକ୍ଷର *(luaL_checklstring) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg,
                                                          size_t *l);
LUALIB_API ଅଚଳ ଅକ୍ଷର *(luaL_optlstring) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg,
                                          ଅଚଳ ଅକ୍ଷର *def, size_t *l);
LUALIB_API lua_ସଂଖ୍ୟା (luaL_checknumber) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg);
LUALIB_API lua_ସଂଖ୍ୟା (luaL_optnumber) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, lua_ସଂଖ୍ୟା def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg,
                                          lua_Integer def);

LUALIB_API ଶୂନ୍ୟ (luaL_checkstack) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ);
LUALIB_API ଶୂନ୍ୟ (luaL_checktype) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା t);
LUALIB_API ଶୂନ୍ୟ (luaL_checkany) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg);

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା   (luaL_newmetatable) (lua_State *L, ଅଚଳ ଅକ୍ଷର *tname);
LUALIB_API ଶୂନ୍ୟ  (luaL_setmetatable) (lua_State *L, ଅଚଳ ଅକ୍ଷର *tname);
LUALIB_API ଶୂନ୍ୟ *(luaL_testudata) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ud, ଅଚଳ ଅକ୍ଷର *tname);
LUALIB_API ଶୂନ୍ୟ *(luaL_checkudata) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ud, ଅଚଳ ଅକ୍ଷର *tname);

LUALIB_API ଶୂନ୍ୟ (luaL_where) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lvl);
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_error) (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt, ...);

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_checkoption) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ଅଚଳ ଅକ୍ଷର *def,
                                   ଅଚଳ ଅକ୍ଷର *ଅଚଳ lst[]);

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_fileresult) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat, ଅଚଳ ଅକ୍ଷର *fname);
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_execresult) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat);

LUALIB_API ଶୂନ୍ୟ *(luaL_alloc) (ଶୂନ୍ୟ *ud, ଶୂନ୍ୟ *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_ref) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା t);
LUALIB_API ଶୂନ୍ୟ (luaL_unref) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ref);

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_loadfilex) (lua_State *L, ଅଚଳ ଅକ୍ଷର *filename,
                                               ଅଚଳ ଅକ୍ଷର *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,ଖାଲି)

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_loadbufferx) (lua_State *L, ଅଚଳ ଅକ୍ଷର *buff, size_t sz,
                                   ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *mode);
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_loadstring) (lua_State *L, ଅଚଳ ଅକ୍ଷର *s);

LUALIB_API lua_State *(luaL_newstate) (ଶୂନ୍ୟ);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx);

LUALIB_API ଶୂନ୍ୟ (luaL_addgsub) (luaL_Buffer *b, ଅଚଳ ଅକ୍ଷର *s,
                                     ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *r);
LUALIB_API ଅଚଳ ଅକ୍ଷର *(luaL_gsub) (lua_State *L, ଅଚଳ ଅକ୍ଷର *s,
                                    ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *r);

LUALIB_API ଶୂନ୍ୟ (luaL_setfuncs) (lua_State *L, ଅଚଳ luaL_Reg *l, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nup);

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା (luaL_getsubtable) (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅଚଳ ଅକ୍ଷର *fname);

LUALIB_API ଶୂନ୍ୟ (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level);

LUALIB_API ଶୂନ୍ୟ (luaL_requiref) (lua_State *L, ଅଚଳ ଅକ୍ଷର *modname,
                                 lua_CFunction openf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, ଆକାର(l)/ଆକାର((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((ଶୂନ୍ୟ)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((ଶୂନ୍ୟ)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ଖାଲି))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ଖାଲି))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ଖାଲି)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#ଯଦି defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#ଅନ୍ୟଥା
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

ସଂରଚନା luaL_Buffer {
  ଅକ୍ଷର *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  ସଂଘ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    ଅକ୍ଷର b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((ଶୂନ୍ୟ)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API ଶୂନ୍ୟ (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API ଅକ୍ଷର *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API ଶୂନ୍ୟ (luaL_addlstring) (luaL_Buffer *B, ଅଚଳ ଅକ୍ଷର *s, size_t l);
LUALIB_API ଶୂନ୍ୟ (luaL_addstring) (luaL_Buffer *B, ଅଚଳ ଅକ୍ଷର *s);
LUALIB_API ଶୂନ୍ୟ (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API ଶୂନ୍ୟ (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API ଶୂନ୍ୟ (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API ଅକ୍ଷର *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

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


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#ଯଦି defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


