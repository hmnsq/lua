/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#अगर_अघोषित lauxlib_h
#घोषणा lauxlib_h


#समावेश <stddef.h>
#समावेश <stdio.h>

#समावेश "luaconf.h"
#समावेश "lua.h"


/* global table */
#घोषणा LUA_GNAME	"_G"


प्रकार काष्ठा lua_L_बफर lua_L_बफर;


/* extra error code for 'luaL_loadfilex' */
#घोषणा LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#घोषणा LUA_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#घोषणा LUA_PRELOAD_TABLE	"_PRELOAD"


प्रकार काष्ठा luaL_Reg {
  स्थिर अक्षर *name;
  lua_CFunction func;
} luaL_Reg;


#घोषणा LUAL_NUMSIZES	(माप(lua_Integer)*16 + माप(lua_संख्या))

LUALIB_API व्योम (luaL_checkversion_) (lua_State *L, lua_संख्या ver, माप_प्रकार sz);
#घोषणा luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_VERSION_अंक, LUAL_NUMSIZES)

LUALIB_API पूर्णांक (luaL_getmetafield) (lua_State *L, पूर्णांक obj, स्थिर अक्षर *e);
LUALIB_API पूर्णांक (luaL_callmeta) (lua_State *L, पूर्णांक obj, स्थिर अक्षर *e);
LUALIB_API स्थिर अक्षर *(luaL_tolstring) (lua_State *L, पूर्णांक idx, माप_प्रकार *len);
LUALIB_API पूर्णांक (luaL_argerror) (lua_State *L, पूर्णांक arg, स्थिर अक्षर *extramsg);
LUALIB_API पूर्णांक (luaL_typeerror) (lua_State *L, पूर्णांक arg, स्थिर अक्षर *tname);
LUALIB_API स्थिर अक्षर *(luaL_checklstring) (lua_State *L, पूर्णांक arg,
                                                          माप_प्रकार *l);
LUALIB_API स्थिर अक्षर *(luaL_optlstring) (lua_State *L, पूर्णांक arg,
                                          स्थिर अक्षर *def, माप_प्रकार *l);
LUALIB_API lua_संख्या (luaL_checknumber) (lua_State *L, पूर्णांक arg);
LUALIB_API lua_संख्या (luaL_optnumber) (lua_State *L, पूर्णांक arg, lua_संख्या def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, पूर्णांक arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, पूर्णांक arg,
                                          lua_Integer def);

LUALIB_API व्योम (luaL_checkstack) (lua_State *L, पूर्णांक sz, स्थिर अक्षर *संदेश);
LUALIB_API व्योम (luaL_checktype) (lua_State *L, पूर्णांक arg, पूर्णांक t);
LUALIB_API व्योम (luaL_checkany) (lua_State *L, पूर्णांक arg);

LUALIB_API पूर्णांक   (luaL_newmetatable) (lua_State *L, स्थिर अक्षर *tname);
LUALIB_API व्योम  (luaL_setmetatable) (lua_State *L, स्थिर अक्षर *tname);
LUALIB_API व्योम *(luaL_testudata) (lua_State *L, पूर्णांक ud, स्थिर अक्षर *tname);
LUALIB_API व्योम *(luaL_checkudata) (lua_State *L, पूर्णांक ud, स्थिर अक्षर *tname);

LUALIB_API व्योम (luaL_where) (lua_State *L, पूर्णांक lvl);
LUALIB_API पूर्णांक (luaL_error) (lua_State *L, स्थिर अक्षर *fmt, ...);

LUALIB_API पूर्णांक (luaL_checkoption) (lua_State *L, पूर्णांक arg, स्थिर अक्षर *def,
                                   स्थिर अक्षर *स्थिर lst[]);

LUALIB_API पूर्णांक (luaL_fileresult) (lua_State *L, पूर्णांक stat, स्थिर अक्षर *fname);
LUALIB_API पूर्णांक (luaL_execresult) (lua_State *L, पूर्णांक stat);

LUALIB_API व्योम *(luaL_alloc) (व्योम *ud, व्योम *ptr, माप_प्रकार osize,
                                                    माप_प्रकार nsize);


/* predefined references */
#घोषणा LUA_NOREF       (-2)
#घोषणा LUA_REFNIL      (-1)

LUALIB_API पूर्णांक (luaL_ref) (lua_State *L, पूर्णांक t);
LUALIB_API व्योम (luaL_unref) (lua_State *L, पूर्णांक t, पूर्णांक ref);

LUALIB_API पूर्णांक (luaL_loadfilex) (lua_State *L, स्थिर अक्षर *filename,
                                               स्थिर अक्षर *mode);

#घोषणा luaL_loadfile(L,f)	luaL_loadfilex(L,f,शून्य)

LUALIB_API पूर्णांक (luaL_loadbufferx) (lua_State *L, स्थिर अक्षर *buff, माप_प्रकार sz,
                                   स्थिर अक्षर *name, स्थिर अक्षर *mode);
LUALIB_API पूर्णांक (luaL_loadstring) (lua_State *L, स्थिर अक्षर *s);

LUALIB_API lua_State *(luaL_newstate) (व्योम);

LUALIB_API अचिन्हित (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, पूर्णांक idx);

LUALIB_API व्योम (luaL_addgsub) (lua_L_बफर *b, स्थिर अक्षर *s,
                                     स्थिर अक्षर *p, स्थिर अक्षर *r);
LUALIB_API स्थिर अक्षर *(luaL_gsub) (lua_State *L, स्थिर अक्षर *s,
                                    स्थिर अक्षर *p, स्थिर अक्षर *r);

LUALIB_API व्योम (luaL_setfuncs) (lua_State *L, स्थिर luaL_Reg *l, पूर्णांक nup);

LUALIB_API पूर्णांक (luaL_getsubtable) (lua_State *L, पूर्णांक idx, स्थिर अक्षर *fname);

LUALIB_API व्योम (luaL_traceback) (lua_State *L, lua_State *L1,
                                  स्थिर अक्षर *संदेश, पूर्णांक level);

LUALIB_API व्योम (luaL_requiref) (lua_State *L, स्थिर अक्षर *modname,
                                 lua_CFunction openf, पूर्णांक glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#घोषणा luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, माप(l)/माप((l)[0]) - 1)

#घोषणा luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#घोषणा luaL_argcheck(L, cond,arg,extramsg)	\
	((व्योम)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#घोषणा luaL_argexpected(L,cond,arg,tname)	\
	((व्योम)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#घोषणा luaL_checkstring(L,n)	(luaL_checklstring(L, (n), शून्य))
#घोषणा luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), शून्य))

#घोषणा luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#घोषणा luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#घोषणा luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#घोषणा luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#घोषणा luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#घोषणा luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,शून्य)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#घोषणा luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#अगर defined(LUA_FAILISFALSE)
#घोषणा luaL_pushfail(L)	lua_pushboolean(L, 0)
#अन्यथा
#घोषणा luaL_pushfail(L)	lua_pushnil(L)
#पूर्ण_अगर



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

काष्ठा lua_L_बफर {
  अक्षर *b;  /* buffer address */
  माप_प्रकार आकार;  /* buffer size */
  माप_प्रकार n;  /* number of characters in buffer */
  lua_State *L;
  जोड़ {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    अक्षर b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#घोषणा luaL_bufflen(bf)	((bf)->n)
#घोषणा luaL_buffaddr(bf)	((bf)->b)


#घोषणा luaL_addchar(B,c) \
  ((व्योम)((B)->n < (B)->आकार || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#घोषणा luaL_addsize(B,s)	((B)->n += (s))

#घोषणा luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API व्योम (luaL_buffinit) (lua_State *L, lua_L_बफर *B);
LUALIB_API अक्षर *(luaL_prepbuffsize) (lua_L_बफर *B, माप_प्रकार sz);
LUALIB_API व्योम (luaL_addlstring) (lua_L_बफर *B, स्थिर अक्षर *s, माप_प्रकार l);
LUALIB_API व्योम (luaL_addstring) (lua_L_बफर *B, स्थिर अक्षर *s);
LUALIB_API व्योम (luaL_addvalue) (lua_L_बफर *B);
LUALIB_API व्योम (luaL_pushresult) (lua_L_बफर *B);
LUALIB_API व्योम (luaL_pushresultsize) (lua_L_बफर *B, माप_प्रकार sz);
LUALIB_API अक्षर *(luaL_buffinitsize) (lua_State *L, lua_L_बफर *B, माप_प्रकार sz);

#घोषणा luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

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

#घोषणा LUA_FILEHANDLE          "FILE*"


प्रकार काष्ठा luaL_Stream {
  खाता *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} luaL_Stream;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#अगर defined(LUA_COMPAT_APIINTCASTS)

#घोषणा luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#घोषणा luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#घोषणा luaL_checkint(L,n)	((पूर्णांक)luaL_checkinteger(L, (n)))
#घोषणा luaL_optint(L,n,d)	((पूर्णांक)luaL_optinteger(L, (n), (d)))

#घोषणा luaL_checklong(L,n)	((दीर्घ)luaL_checkinteger(L, (n)))
#घोषणा luaL_optlong(L,n,d)	((दीर्घ)luaL_optinteger(L, (n), (d)))

#पूर्ण_अगर
/* }============================================================ */



#पूर्ण_अगर


