/*
** $Id: lauxlib.h $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/


#నిర్వచితం_కాకపోతే lauxlib_h
#నిర్వచించు lauxlib_h


#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>

#అన్తర్భావ "luaconf.h"
#అన్తర్భావ "lua.h"


/* global table */
#నిర్వచించు LUA_GNAME	"_G"


రకనిర్వచనం నిర్మాణం lua_L_బఫరు lua_L_బఫరు;


/* extra error code for 'luaL_loadfilex' */
#నిర్వచించు LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#నిర్వచించు LUA_LOADED_పట్టిక	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#నిర్వచించు LUA_PRELOAD_పట్టిక	"_PRELOAD"


రకనిర్వచనం నిర్మాణం luaL_Reg {
  స్థిరరాశి అక్షరం *name;
  lua_CFunction func;
} luaL_Reg;


#నిర్వచించు LUAL_NUMSIZES	(పరిమాణం(lua_Integer)*16 + పరిమాణం(lua_సంఖ్య))

LUALIB_API శూన్య (luaL_checkversion_) (lua_State *L, lua_సంఖ్య ver, size_t sz);
#నిర్వచించు luaL_checkversion(L)  \
	  luaL_checkversion_(L, LUA_సంస్కరణ_అంకె, LUAL_NUMSIZES)

LUALIB_API పూర్ణాంకం (luaL_getmetafield) (lua_State *L, పూర్ణాంకం obj, స్థిరరాశి అక్షరం *e);
LUALIB_API పూర్ణాంకం (luaL_callmeta) (lua_State *L, పూర్ణాంకం obj, స్థిరరాశి అక్షరం *e);
LUALIB_API స్థిరరాశి అక్షరం *(luaL_tolstring) (lua_State *L, పూర్ణాంకం idx, size_t *len);
LUALIB_API పూర్ణాంకం (luaL_argerror) (lua_State *L, పూర్ణాంకం arg, స్థిరరాశి అక్షరం *extramsg);
LUALIB_API పూర్ణాంకం (luaL_typeerror) (lua_State *L, పూర్ణాంకం arg, స్థిరరాశి అక్షరం *tname);
LUALIB_API స్థిరరాశి అక్షరం *(luaL_checklstring) (lua_State *L, పూర్ణాంకం arg,
                                                          size_t *l);
LUALIB_API స్థిరరాశి అక్షరం *(luaL_optlstring) (lua_State *L, పూర్ణాంకం arg,
                                          స్థిరరాశి అక్షరం *def, size_t *l);
LUALIB_API lua_సంఖ్య (luaL_checknumber) (lua_State *L, పూర్ణాంకం arg);
LUALIB_API lua_సంఖ్య (luaL_optnumber) (lua_State *L, పూర్ణాంకం arg, lua_సంఖ్య def);

LUALIB_API lua_Integer (luaL_checkinteger) (lua_State *L, పూర్ణాంకం arg);
LUALIB_API lua_Integer (luaL_optinteger) (lua_State *L, పూర్ణాంకం arg,
                                          lua_Integer def);

LUALIB_API శూన్య (luaL_checkstack) (lua_State *L, పూర్ణాంకం sz, స్థిరరాశి అక్షరం *సందేశం);
LUALIB_API శూన్య (luaL_checktype) (lua_State *L, పూర్ణాంకం arg, పూర్ణాంకం t);
LUALIB_API శూన్య (luaL_checkany) (lua_State *L, పూర్ణాంకం arg);

LUALIB_API పూర్ణాంకం   (luaL_newmetatable) (lua_State *L, స్థిరరాశి అక్షరం *tname);
LUALIB_API శూన్య  (luaL_setmetatable) (lua_State *L, స్థిరరాశి అక్షరం *tname);
LUALIB_API శూన్య *(luaL_testudata) (lua_State *L, పూర్ణాంకం ud, స్థిరరాశి అక్షరం *tname);
LUALIB_API శూన్య *(luaL_checkudata) (lua_State *L, పూర్ణాంకం ud, స్థిరరాశి అక్షరం *tname);

LUALIB_API శూన్య (luaL_where) (lua_State *L, పూర్ణాంకం lvl);
LUALIB_API పూర్ణాంకం (luaL_error) (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...);

LUALIB_API పూర్ణాంకం (luaL_checkoption) (lua_State *L, పూర్ణాంకం arg, స్థిరరాశి అక్షరం *def,
                                   స్థిరరాశి అక్షరం *స్థిరరాశి lst[]);

LUALIB_API పూర్ణాంకం (luaL_fileresult) (lua_State *L, పూర్ణాంకం stat, స్థిరరాశి అక్షరం *fname);
LUALIB_API పూర్ణాంకం (luaL_execresult) (lua_State *L, పూర్ణాంకం stat);

LUALIB_API శూన్య *(luaL_alloc) (శూన్య *ud, శూన్య *ptr, size_t osize,
                                                    size_t nsize);


/* predefined references */
#నిర్వచించు LUA_NOREF       (-2)
#నిర్వచించు LUA_REFNIL      (-1)

LUALIB_API పూర్ణాంకం (luaL_ref) (lua_State *L, పూర్ణాంకం t);
LUALIB_API శూన్య (luaL_unref) (lua_State *L, పూర్ణాంకం t, పూర్ణాంకం ref);

LUALIB_API పూర్ణాంకం (luaL_loadfilex) (lua_State *L, స్థిరరాశి అక్షరం *filename,
                                               స్థిరరాశి అక్షరం *బహుళకం);

#నిర్వచించు luaL_loadfile(L,f)	luaL_loadfilex(L,f,ఖాళీ)

LUALIB_API పూర్ణాంకం (luaL_loadbufferx) (lua_State *L, స్థిరరాశి అక్షరం *buff, size_t sz,
                                   స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *బహుళకం);
LUALIB_API పూర్ణాంకం (luaL_loadstring) (lua_State *L, స్థిరరాశి అక్షరం *s);

LUALIB_API lua_State *(luaL_newstate) (శూన్య);

LUALIB_API గుర్తులేని (luaL_makeseed) (lua_State *L);

LUALIB_API lua_Integer (luaL_len) (lua_State *L, పూర్ణాంకం idx);

LUALIB_API శూన్య (luaL_addgsub) (lua_L_బఫరు *b, స్థిరరాశి అక్షరం *s,
                                     స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *r);
LUALIB_API స్థిరరాశి అక్షరం *(luaL_gsub) (lua_State *L, స్థిరరాశి అక్షరం *s,
                                    స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *r);

LUALIB_API శూన్య (luaL_setfuncs) (lua_State *L, స్థిరరాశి luaL_Reg *l, పూర్ణాంకం nup);

LUALIB_API పూర్ణాంకం (luaL_getsubtable) (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి అక్షరం *fname);

LUALIB_API శూన్య (luaL_traceback) (lua_State *L, lua_State *L1,
                                  స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం level);

LUALIB_API శూన్య (luaL_requiref) (lua_State *L, స్థిరరాశి అక్షరం *modname,
                                 lua_CFunction openf, పూర్ణాంకం glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#నిర్వచించు luaL_newlibtable(L,l)	\
  lua_createtable(L, 0, పరిమాణం(l)/పరిమాణం((l)[0]) - 1)

#నిర్వచించు luaL_newlib(L,l)  \
  (luaL_checkversion(L), luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

#నిర్వచించు luaL_argcheck(L, cond,arg,extramsg)	\
	((శూన్య)(luai_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#నిర్వచించు luaL_argexpected(L,cond,arg,tname)	\
	((శూన్య)(luai_likely(cond) || luaL_typeerror(L, (arg), (tname))))

#నిర్వచించు luaL_checkstring(L,n)	(luaL_checklstring(L, (n), ఖాళీ))
#నిర్వచించు luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), ఖాళీ))

#నిర్వచించు luaL_typename(L,i)	lua_typename(L, lua_type(L,(i)))

#నిర్వచించు luaL_dofile(L, fn) \
	(luaL_loadfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#నిర్వచించు luaL_dostring(L, s) \
	(luaL_loadstring(L, s) || lua_pcall(L, 0, LUA_MULTRET, 0))

#నిర్వచించు luaL_getmetatable(L,n)	(lua_getfield(L, LUA_REGISTRYINDEX, (n)))

#నిర్వచించు luaL_opt(L,f,n,d)	(lua_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#నిర్వచించు luaL_loadbuffer(L,s,sz,n)	luaL_loadbufferx(L,s,sz,n,ఖాళీ)


/*
** Perform arithmetic operations on lua_Integer values with wrap-around
** semantics, as the Lua core does.
*/
#నిర్వచించు luaL_intop(op,v1,v2)  \
	((lua_Integer)((lua_Unsigned)(v1) op (lua_Unsigned)(v2)))


/* push the value used to represent failure/error */
#ఒకవేళ defined(LUA_FAILISFALSE)
#నిర్వచించు luaL_pushfail(L)	lua_pushboolean(L, 0)
#లేకపోతే
#నిర్వచించు luaL_pushfail(L)	lua_pushnil(L)
#అయితే_ముగింపు



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

నిర్మాణం lua_L_బఫరు {
  అక్షరం *b;  /* buffer address */
  size_t కొలత;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  lua_State *L;
  సంఘం {
    LUAI_MAXALIGN;  /* ensure maximum alignment for buffer */
    అక్షరం b[LUAL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#నిర్వచించు luaL_bufflen(bf)	((bf)->n)
#నిర్వచించు luaL_buffaddr(bf)	((bf)->b)


#నిర్వచించు luaL_addchar(B,c) \
  ((శూన్య)((B)->n < (B)->కొలత || luaL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#నిర్వచించు luaL_addsize(B,s)	((B)->n += (s))

#నిర్వచించు luaL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API శూన్య (luaL_buffinit) (lua_State *L, lua_L_బఫరు *B);
LUALIB_API అక్షరం *(luaL_prepbuffsize) (lua_L_బఫరు *B, size_t sz);
LUALIB_API శూన్య (luaL_addlstring) (lua_L_బఫరు *B, స్థిరరాశి అక్షరం *s, size_t l);
LUALIB_API శూన్య (luaL_addstring) (lua_L_బఫరు *B, స్థిరరాశి అక్షరం *s);
LUALIB_API శూన్య (luaL_addvalue) (lua_L_బఫరు *B);
LUALIB_API శూన్య (luaL_pushresult) (lua_L_బఫరు *B);
LUALIB_API శూన్య (luaL_pushresultsize) (lua_L_బఫరు *B, size_t sz);
LUALIB_API అక్షరం *(luaL_buffinitsize) (lua_State *L, lua_L_బఫరు *B, size_t sz);

#నిర్వచించు luaL_prepbuffer(B)	luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

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

#నిర్వచించు LUA_FILEHANDLE          "FILE*"


రకనిర్వచనం నిర్మాణం lua_L_ప్రవాహం {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} lua_L_ప్రవాహం;

/* }====================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#ఒకవేళ defined(LUA_COMPAT_APIINTCASTS)

#నిర్వచించు luaL_checkunsigned(L,a)	((lua_Unsigned)luaL_checkinteger(L,a))
#నిర్వచించు luaL_optunsigned(L,a,d)	\
	((lua_Unsigned)luaL_optinteger(L,a,(lua_Integer)(d)))

#నిర్వచించు luaL_checkint(L,n)	((పూర్ణాంకం)luaL_checkinteger(L, (n)))
#నిర్వచించు luaL_optint(L,n,d)	((పూర్ణాంకం)luaL_optinteger(L, (n), (d)))

#నిర్వచించు luaL_checklong(L,n)	((దీర్ఘ)luaL_checkinteger(L, (n)))
#నిర్వచించు luaL_optlong(L,n,d)	((దీర్ఘ)luaL_optinteger(L, (n), (d)))

#అయితే_ముగింపు
/* }============================================================ */



#అయితే_ముగింపు


