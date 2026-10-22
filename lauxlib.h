/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#شامل <stddef.h>
#شامل <stdio.h>

#شامل "luaconf.h"
#شامل "lua.h"


/* global table */
#define LUA_GNAME	"_G"


تعریفنوع ساختار luaL_Buffer luaL_Buffer;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


تعریفنوع ساختار luaL_Reg {
  ثابت نویسه *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(اندازه(lua_Integer)*16 + اندازه(lua_عدد))

LUALIB_API تهی (luaL_checkversion_) (lua_State *L, lua_عدد ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_رقم, LUAL_NUMSIZES)

LUALIB_API صحیح (luaL_getmetafield) (lua_State *L, صحیح obj, ثابت نویسه *e);
LUALIB_API صحیح (luaL_callmeta) (lua_State *L, صحیح obj, ثابت نویسه *e);
LUALIB_API ثابت نویسه *(luaL_tolstring) (lua_State *L, صحیح idx, size_t *len);
LUALIB_API صحیح (luaL_argerror) (lua_State *L, صحیح arg, ثابت نویسه *extramsg);
LUALIB_API صحیح (luaL_typeerror) (lua_State *L, صحیح arg, ثابت نویسه *tname);
LUALIB_API ثابت نویسه *(luaL_checklstring) (lua_State *L, صحیح arg,
                                                          size_t *l);
LUALIB_API ثابت نویسه *(luaL_optlstring) (lua_State *L, صحیح arg,
                                          ثابت نویسه *def, size_t *l);
LUALIB_API lua_عدد (luaL_checknumber) (lua_State *L, صحیح arg);
LUALIB_API lua_عدد (luaL_optnumber) (lua_State *L, صحیح arg, lua_عدد def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, صحیح arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, صحیح arg,
                                          lua_Integer def);

LUALIB_API تهی (luaL_checkstack) (lua_State *L, صحیح sz, ثابت نویسه *پیام);
LUALIB_API تهی (luaL_checktype) (lua_State *L, صحیح arg, صحیح t);
LUALIB_API تهی (luaL_checkany) (lua_State *L, صحیح arg);

LUALIB_API صحیح   (luaL_newmetatable) (lua_State *L, ثابت نویسه *tname);
LUALIB_API تهی  (luaL_setmetatable) (lua_State *L, ثابت نویسه *tname);
LUALIB_API تهی *(luaL_testudata) (lua_State *L, صحیح ud, ثابت نویسه *tname);
LUALIB_API تهی *(luaL_checkudata) (lua_State *L, صحیح ud, ثابت نویسه *tname);

LUALIB_API تهی (luaL_where) (lua_State *L, صحیح lvl);
LUALIB_API صحیح (luaL_error) (lua_State *L, ثابت نویسه *fmt, ...);

LUALIB_API صحیح (luaL_checkoption) (lua_State *L, صحیح arg, ثابت نویسه *def,
                                   ثابت نویسه *ثابت lst[]);

LUALIB_API صحیح (luaL_fileresult) (lua_State *L, صحیح stat, ثابت نویسه *fname);
LUALIB_API صحیح (luaL_execresult) (lua_State *L, صحیح stat);

LUALIB_API تهی *(luaL_alloc) (تهی *ud, تهی *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API صحیح (luaL_ref) (lua_State *L, صحیح t);
LUALIB_API تهی (luaL_unref) (lua_State *L, صحیح t, صحیح ref);

LUALIB_API صحیح (luaL_loadfilex) (lua_State *L, ثابت نویسه *filename,
                                               ثابت نویسه *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,پوچ)

LUALIB_API صحیح (luaL_loadbufferx) (lua_State *L, ثابت نویسه *buff, size_t sz,
                                   ثابت نویسه *name, ثابت نویسه *mode);
LUALIB_API صحیح (luaL_loadstring) (lua_State *L, ثابت نویسه *s);

LUALIB_API lua_State *(luaL_newstate) (تهی);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, صحیح idx);

LUALIB_API تهی (luaL_addgsub) (luaL_Buffer *b, ثابت نویسه *s,
                                     ثابت نویسه *p, ثابت نویسه *r);
LUALIB_API ثابت نویسه *(luaL_gsub) (lua_State *L, ثابت نویسه *s,
                                    ثابت نویسه *p, ثابت نویسه *r);

LUALIB_API تهی (luaL_setfuncs) (lua_State *L, ثابت luaL_Reg *l, صحیح nup);

LUALIB_API صحیح (luaL_getsubtable) (lua_State *L, صحیح idx, ثابت نویسه *fname);

LUALIB_API تهی (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ثابت نویسه *پیام, صحیح level);

LUALIB_API تهی (luaL_requiref) (lua_State *L, ثابت نویسه *modname,
                                 lua_CFunction openf, صحیح glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, اندازه(l)/اندازه((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((تهی)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((تهی)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), پوچ))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), پوچ))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,پوچ)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#اگر defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#وگرنه
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

ساختار luaL_Buffer {
  نویسه *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  اجتماع {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    نویسه b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((تهی)((B)->n < (B)->size || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API تهی (luaL_buffinit) (lua_State *L, luaL_Buffer *B);
LUALIB_API نویسه *(luaL_prepbuffsize) (luaL_Buffer *B, size_t sz);
LUALIB_API تهی (luaL_addlstring) (luaL_Buffer *B, ثابت نویسه *s, size_t l);
LUALIB_API تهی (luaL_addstring) (luaL_Buffer *B, ثابت نویسه *s);
LUALIB_API تهی (luaL_addvalue) (luaL_Buffer *B);
LUALIB_API تهی (luaL_pushresult) (luaL_Buffer *B);
LUALIB_API تهی (luaL_pushresultsize) (luaL_Buffer *B, size_t sz);
LUALIB_API نویسه *(luaL_buffinitsize) (lua_State *L, luaL_Buffer *B, size_t sz);

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


تعریفنوع ساختار luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#اگر defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((صحیح)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((صحیح)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


