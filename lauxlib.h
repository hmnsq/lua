/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>

#অন্তর্ভাব "luaconf.h"
#অন্তর্ভাব "lua.h"


/* global table */
#define LUA_GNAME	"_G"


প্রকারলেখ কাঠামো lua_L_বাফার lua_L_বাফার;


/* extra error code for 'luaL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LUA_PRELOAD_TABLE	"_PRELOAD"


প্রকারলেখ কাঠামো luaL_Reg {
  ধ্রুবক অক্ষর *name;
  lua_CFunction func;
} luaL_Reg;


#define LUAL_NUMSIZES	(আকার(lua_Integer)*16 + আকার(lua_সংখ্যা))

LUALIB_API শূন্য (luaL_checkversion_) (lua_State *L, lua_সংখ্যা ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_অঙ্ক, LUAL_NUMSIZES)

LUALIB_API পূর্ণসংখ্যা (luaL_getmetafield) (lua_State *L, পূর্ণসংখ্যা obj, ধ্রুবক অক্ষর *e);
LUALIB_API পূর্ণসংখ্যা (luaL_callmeta) (lua_State *L, পূর্ণসংখ্যা obj, ধ্রুবক অক্ষর *e);
LUALIB_API ধ্রুবক অক্ষর *(luaL_tolstring) (lua_State *L, পূর্ণসংখ্যা idx, size_t *len);
LUALIB_API পূর্ণসংখ্যা (luaL_argerror) (lua_State *L, পূর্ণসংখ্যা arg, ধ্রুবক অক্ষর *extramsg);
LUALIB_API পূর্ণসংখ্যা (luaL_typeerror) (lua_State *L, পূর্ণসংখ্যা arg, ধ্রুবক অক্ষর *tname);
LUALIB_API ধ্রুবক অক্ষর *(luaL_checklstring) (lua_State *L, পূর্ণসংখ্যা arg,
                                                          size_t *l);
LUALIB_API ধ্রুবক অক্ষর *(luaL_optlstring) (lua_State *L, পূর্ণসংখ্যা arg,
                                          ধ্রুবক অক্ষর *def, size_t *l);
LUALIB_API lua_সংখ্যা (luaL_checknumber) (lua_State *L, পূর্ণসংখ্যা arg);
LUALIB_API lua_সংখ্যা (luaL_optnumber) (lua_State *L, পূর্ণসংখ্যা arg, lua_সংখ্যা def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, পূর্ণসংখ্যা arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, পূর্ণসংখ্যা arg,
                                          lua_Integer def);

LUALIB_API শূন্য (luaL_checkstack) (lua_State *L, পূর্ণসংখ্যা sz, ধ্রুবক অক্ষর *বার্তা);
LUALIB_API শূন্য (luaL_checktype) (lua_State *L, পূর্ণসংখ্যা arg, পূর্ণসংখ্যা t);
LUALIB_API শূন্য (luaL_checkany) (lua_State *L, পূর্ণসংখ্যা arg);

LUALIB_API পূর্ণসংখ্যা   (luaL_newmetatable) (lua_State *L, ধ্রুবক অক্ষর *tname);
LUALIB_API শূন্য  (luaL_setmetatable) (lua_State *L, ধ্রুবক অক্ষর *tname);
LUALIB_API শূন্য *(luaL_testudata) (lua_State *L, পূর্ণসংখ্যা ud, ধ্রুবক অক্ষর *tname);
LUALIB_API শূন্য *(luaL_checkudata) (lua_State *L, পূর্ণসংখ্যা ud, ধ্রুবক অক্ষর *tname);

LUALIB_API শূন্য (luaL_where) (lua_State *L, পূর্ণসংখ্যা lvl);
LUALIB_API পূর্ণসংখ্যা (luaL_error) (lua_State *L, ধ্রুবক অক্ষর *fmt, ...);

LUALIB_API পূর্ণসংখ্যা (luaL_checkoption) (lua_State *L, পূর্ণসংখ্যা arg, ধ্রুবক অক্ষর *def,
                                   ধ্রুবক অক্ষর *ধ্রুবক lst[]);

LUALIB_API পূর্ণসংখ্যা (luaL_fileresult) (lua_State *L, পূর্ণসংখ্যা stat, ধ্রুবক অক্ষর *fname);
LUALIB_API পূর্ণসংখ্যা (luaL_execresult) (lua_State *L, পূর্ণসংখ্যা stat);

LUALIB_API শূন্য *(luaL_alloc) (শূন্য *ud, শূন্য *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#define LUA_NOREF       (-2)
#define LUA_REFNIL      (-1)

LUALIB_API পূর্ণসংখ্যা (luaL_ref) (lua_State *L, পূর্ণসংখ্যা t);
LUALIB_API শূন্য (luaL_unref) (lua_State *L, পূর্ণসংখ্যা t, পূর্ণসংখ্যা ref);

LUALIB_API পূর্ণসংখ্যা (luaL_loadfilex) (lua_State *L, ধ্রুবক অক্ষর *filename,
                                               ধ্রুবক অক্ষর *mode);

#define luaL_loadfile(L,f)	luaL_loadfilex(L,f,ফাঁকা)

LUALIB_API পূর্ণসংখ্যা (luaL_loadbufferx) (lua_State *L, ধ্রুবক অক্ষর *buff, size_t sz,
                                   ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *mode);
LUALIB_API পূর্ণসংখ্যা (luaL_loadstring) (lua_State *L, ধ্রুবক অক্ষর *s);

LUALIB_API lua_State *(luaL_newstate) (শূন্য);

LUALIB_API unsigned (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, পূর্ণসংখ্যা idx);

LUALIB_API শূন্য (luaL_addgsub) (lua_L_বাফার *b, ধ্রুবক অক্ষর *s,
                                     ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *r);
LUALIB_API ধ্রুবক অক্ষর *(luaL_gsub) (lua_State *L, ধ্রুবক অক্ষর *s,
                                    ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *r);

LUALIB_API শূন্য (luaL_setfuncs) (lua_State *L, ধ্রুবক luaL_Reg *l, পূর্ণসংখ্যা nup);

LUALIB_API পূর্ণসংখ্যা (luaL_getsubtable) (lua_State *L, পূর্ণসংখ্যা idx, ধ্রুবক অক্ষর *fname);

LUALIB_API শূন্য (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা level);

LUALIB_API শূন্য (luaL_requiref) (lua_State *L, ধ্রুবক অক্ষর *modname,
                                 lua_CFunction openf, পূর্ণসংখ্যা glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, আকার(l)/আকার((l)[0]) - 1)

#define luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((শূন্য)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((শূন্য)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ফাঁকা))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ফাঁকা))

#define luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#define luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#define luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ফাঁকা)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#define luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#যদি defined(LUA_FAILISFALSE)
#define luaL_pushfail(L)	lua_pushboolean(L, 0)
#অন্যথা
#define luaL_pushfail(L)	lua_pushnil(L)
#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

কাঠামো lua_L_বাফার {
  অক্ষর *b;  /* buffer address */
  size_t আকার_2;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  সংঘ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    অক্ষর b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define luaL_bufflen(bf)	((bf)->n)
#define luaL_buffaddr(bf)	((bf)->b)


#define luaL_addchar(B,c) \
  ((শূন্য)((B)->n < (B)->আকার_2 || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define luaL_addsize(B,s)	((B)->n += (s))

#define luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API শূন্য (luaL_buffinit) (lua_State *L, lua_L_বাফার *B);
LUALIB_API অক্ষর *(luaL_prepbuffsize) (lua_L_বাফার *B, size_t sz);
LUALIB_API শূন্য (luaL_addlstring) (lua_L_বাফার *B, ধ্রুবক অক্ষর *s, size_t l);
LUALIB_API শূন্য (luaL_addstring) (lua_L_বাফার *B, ধ্রুবক অক্ষর *s);
LUALIB_API শূন্য (luaL_addvalue) (lua_L_বাফার *B);
LUALIB_API শূন্য (luaL_pushresult) (lua_L_বাফার *B);
LUALIB_API শূন্য (luaL_pushresultsize) (lua_L_বাফার *B, size_t sz);
LUALIB_API অক্ষর *(luaL_buffinitsize) (lua_State *L, lua_L_বাফার *B, size_t sz);

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


প্রকারলেখ কাঠামো luaL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#যদি defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#define luaL_checkint(L,n)	((পূর্ণসংখ্যা)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((পূর্ণসংখ্যা)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


