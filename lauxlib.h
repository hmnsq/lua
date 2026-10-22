/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#यदि_अपरिभाषितम् lauxlib_h
#परिभाषय lauxlib_h


#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>

#अन्तर्भाव "luaconf.h"
#अन्तर्भाव "lua.h"


/* global table */
#परिभाषय LUA_GNAME	"_G"


प्रकारनिर्देश संरचना lua_L_बफर lua_L_बफर;


/* extra error code for 'luaL_loadfilex' */
#परिभाषय LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#परिभाषय LUA_LOADED_सारणी	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#परिभाषय LUA_PRELOAD_सारणी	"_PRELOAD"


प्रकारनिर्देश संरचना luaL_Reg {
  ध्रुव वर्ण *name;
  lua_CFunction func;
} luaL_Reg;


#परिभाषय LUAL_NUMSIZES	(परिमाण(lua_Integer)*16 + परिमाण(lua_सङ्ख्या))

LUALIB_API शून्य (luaL_checkversion_) (lua_State *L, lua_सङ्ख्या ver, size_t sz);
#परिभाषय luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_संस्करणम्_अङ्कः, LUAL_NUMSIZES)

LUALIB_API पूर्णाङ्क (luaL_getmetafield) (lua_State *L, पूर्णाङ्क obj, ध्रुव वर्ण *e);
LUALIB_API पूर्णाङ्क (luaL_callmeta) (lua_State *L, पूर्णाङ्क obj, ध्रुव वर्ण *e);
LUALIB_API ध्रुव वर्ण *(luaL_tolstring) (lua_State *L, पूर्णाङ्क idx, size_t *len);
LUALIB_API पूर्णाङ्क (luaL_argerror) (lua_State *L, पूर्णाङ्क arg, ध्रुव वर्ण *extramsg);
LUALIB_API पूर्णाङ्क (luaL_typeerror) (lua_State *L, पूर्णाङ्क arg, ध्रुव वर्ण *tname);
LUALIB_API ध्रुव वर्ण *(luaL_checklstring) (lua_State *L, पूर्णाङ्क arg,
                                                          size_t *l);
LUALIB_API ध्रुव वर्ण *(luaL_optlstring) (lua_State *L, पूर्णाङ्क arg,
                                          ध्रुव वर्ण *def, size_t *l);
LUALIB_API lua_सङ्ख्या (luaL_checknumber) (lua_State *L, पूर्णाङ्क arg);
LUALIB_API lua_सङ्ख्या (luaL_optnumber) (lua_State *L, पूर्णाङ्क arg, lua_सङ्ख्या def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, पूर्णाङ्क arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, पूर्णाङ्क arg,
                                          lua_Integer def);

LUALIB_API शून्य (luaL_checkstack) (lua_State *L, पूर्णाङ्क sz, ध्रुव वर्ण *सन्देशः);
LUALIB_API शून्य (luaL_checktype) (lua_State *L, पूर्णाङ्क arg, पूर्णाङ्क t);
LUALIB_API शून्य (luaL_checkany) (lua_State *L, पूर्णाङ्क arg);

LUALIB_API पूर्णाङ्क   (luaL_newmetatable) (lua_State *L, ध्रुव वर्ण *tname);
LUALIB_API शून्य  (luaL_setmetatable) (lua_State *L, ध्रुव वर्ण *tname);
LUALIB_API शून्य *(luaL_testudata) (lua_State *L, पूर्णाङ्क ud, ध्रुव वर्ण *tname);
LUALIB_API शून्य *(luaL_checkudata) (lua_State *L, पूर्णाङ्क ud, ध्रुव वर्ण *tname);

LUALIB_API शून्य (luaL_where) (lua_State *L, पूर्णाङ्क lvl);
LUALIB_API पूर्णाङ्क (luaL_error) (lua_State *L, ध्रुव वर्ण *fmt, ...);

LUALIB_API पूर्णाङ्क (luaL_checkoption) (lua_State *L, पूर्णाङ्क arg, ध्रुव वर्ण *def,
                                   ध्रुव वर्ण *ध्रुव lst[]);

LUALIB_API पूर्णाङ्क (luaL_fileresult) (lua_State *L, पूर्णाङ्क stat, ध्रुव वर्ण *fname);
LUALIB_API पूर्णाङ्क (luaL_execresult) (lua_State *L, पूर्णाङ्क stat);

LUALIB_API शून्य *(luaL_alloc) (शून्य *ud, शून्य *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#परिभाषय LUA_NOREF       (-2)
#परिभाषय LUA_REFNIL      (-1)

LUALIB_API पूर्णाङ्क (luaL_ref) (lua_State *L, पूर्णाङ्क t);
LUALIB_API शून्य (luaL_unref) (lua_State *L, पूर्णाङ्क t, पूर्णाङ्क ref);

LUALIB_API पूर्णाङ्क (luaL_loadfilex) (lua_State *L, ध्रुव वर्ण *filename,
                                               ध्रुव वर्ण *बहुलकम्);

#परिभाषय luaL_loadfile(L,f)	luaL_loadfilex(L,f,रिक्त)

LUALIB_API पूर्णाङ्क (luaL_loadbufferx) (lua_State *L, ध्रुव वर्ण *buff, size_t sz,
                                   ध्रुव वर्ण *name, ध्रुव वर्ण *बहुलकम्);
LUALIB_API पूर्णाङ्क (luaL_loadstring) (lua_State *L, ध्रुव वर्ण *s);

LUALIB_API lua_State *(luaL_newstate) (शून्य);

LUALIB_API अचिह्नित (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, पूर्णाङ्क idx);

LUALIB_API शून्य (luaL_addgsub) (lua_L_बफर *b, ध्रुव वर्ण *s,
                                     ध्रुव वर्ण *p, ध्रुव वर्ण *r);
LUALIB_API ध्रुव वर्ण *(luaL_gsub) (lua_State *L, ध्रुव वर्ण *s,
                                    ध्रुव वर्ण *p, ध्रुव वर्ण *r);

LUALIB_API शून्य (luaL_setfuncs) (lua_State *L, ध्रुव luaL_Reg *l, पूर्णाङ्क nup);

LUALIB_API पूर्णाङ्क (luaL_getsubtable) (lua_State *L, पूर्णाङ्क idx, ध्रुव वर्ण *fname);

LUALIB_API शून्य (luaL_traceback) (lua_State *L, lua_State *L1,
                                  ध्रुव वर्ण *सन्देशः, पूर्णाङ्क level);

LUALIB_API शून्य (luaL_requiref) (lua_State *L, ध्रुव वर्ण *modname,
                                 lua_CFunction openf, पूर्णाङ्क glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#परिभाषय luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, परिमाण(l)/परिमाण((l)[0]) - 1)

#परिभाषय luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#परिभाषय luaL_argcheck(L, cond,arg,extramsg)	\
	((शून्य)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#परिभाषय luaL_argexpected(L,cond,arg,tname)	\
	((शून्य)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#परिभाषय luaL_checkstring(L,n)	(luaL_checklstring(L, (n), रिक्त))
#परिभाषय luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), रिक्त))

#परिभाषय luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#परिभाषय luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#परिभाषय luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#परिभाषय luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#परिभाषय luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#परिभाषय luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,रिक्त)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#परिभाषय luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#यदि defined(LUA_FAILISFALSE)
#परिभाषय luaL_pushfail(L)	lua_pushboolean(L, 0)
#अन्यथा
#परिभाषय luaL_pushfail(L)	lua_pushnil(L)
#यदि_अन्तः



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

संरचना lua_L_बफर {
  वर्ण *b;  /* buffer address */
  size_t आकार;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  सङ्घ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    वर्ण b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#परिभाषय luaL_bufflen(bf)	((bf)->n)
#परिभाषय luaL_buffaddr(bf)	((bf)->b)


#परिभाषय luaL_addchar(B,c) \
  ((शून्य)((B)->n < (B)->आकार || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#परिभाषय luaL_addsize(B,s)	((B)->n += (s))

#परिभाषय luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API शून्य (luaL_buffinit) (lua_State *L, lua_L_बफर *B);
LUALIB_API वर्ण *(luaL_prepbuffsize) (lua_L_बफर *B, size_t sz);
LUALIB_API शून्य (luaL_addlstring) (lua_L_बफर *B, ध्रुव वर्ण *s, size_t l);
LUALIB_API शून्य (luaL_addstring) (lua_L_बफर *B, ध्रुव वर्ण *s);
LUALIB_API शून्य (luaL_addvalue) (lua_L_बफर *B);
LUALIB_API शून्य (luaL_pushresult) (lua_L_बफर *B);
LUALIB_API शून्य (luaL_pushresultsize) (lua_L_बफर *B, size_t sz);
LUALIB_API वर्ण *(luaL_buffinitsize) (lua_State *L, lua_L_बफर *B, size_t sz);

#परिभाषय luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

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

#परिभाषय LUA_FILEHANDLE          "FILE*"


प्रकारनिर्देश संरचना lua_L_प्रवाहः {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} lua_L_प्रवाहः;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#यदि defined(LUA_COMPAT_APIINTCASTS)

#परिभाषय luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#परिभाषय luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#परिभाषय luaL_checkint(L,n)	((पूर्णाङ्क)luaL_checkinteger(L, (n)))
#परिभाषय luaL_optint(L,n,d)	((पूर्णाङ्क)luaL_optinteger(L, (n), (d)))

#परिभाषय luaL_checklong(L,n)	((दीर्घ)luaL_checkinteger(L, (n)))
#परिभाषय luaL_optlong(L,n,d)	((दीर्घ)luaL_optinteger(L, (n), (d)))

#यदि_अन्तः
/* }============================================================ */



#यदि_अन्तः


