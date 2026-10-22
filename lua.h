/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stddef.h>


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	1

#define LUA_VERSION_അക്കം  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_അക്കം  (LUA_VERSION_അക്കം * 100 + LUA_VERSION_RELEASE_N)


#അന്തര്ഭാവ "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#define LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#define LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#define LUA_REGISTRYINDEX	(-(INT_MAX/2 + 1000))
#define lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


തരനിർവചനം ഘടന lua_State lua_State;


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#define LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#define LUA_RIDX_GLOBALS	2
#define LUA_RIDX_MAINTHREAD	3
#define LUA_RIDX_LAST		3


/* type of numbers in Lua */
തരനിർവചനം LUA_സംഖ്യ lua_സംഖ്യ;


/* type for integer functions */
തരനിർവചനം LUA_INTEGER lua_Integer;

/* unsigned integer type */
തരനിർവചനം LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
തരനിർവചനം LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
തരനിർവചനം പൂർണ്ണസംഖ്യ (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
തരനിർവചനം പൂർണ്ണസംഖ്യ (*lua_KFunction) (lua_State *L, പൂർണ്ണസംഖ്യ status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
തരനിർവചനം സ്ഥിരം അക്ഷരം * (*lua_Reader) (lua_State *L, ശൂന്യം *ud, size_t *sz);

തരനിർവചനം പൂർണ്ണസംഖ്യ (*lua_Writer) (lua_State *L, സ്ഥിരം ശൂന്യം *p, size_t sz, ശൂന്യം *ud);


/*
** Type for memory-allocation functions
*/
തരനിർവചനം ശൂന്യം * (*lua_Alloc) (ശൂന്യം *ud, ശൂന്യം *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
തരനിർവചനം ശൂന്യം (*lua_WarnFunction) (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ tocont);


/*
** Type used by the debug API to collect debug information
*/
തരനിർവചനം ഘടന lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
തരനിർവചനം ശൂന്യം (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#എങ്കിൽ defined(LUA_USER_H)
#അന്തര്ഭാവ LUA_USER_H
#endif


/*
** RCS ident string
*/
ബാഹ്യ സ്ഥിരം അക്ഷരം lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, ശൂന്യം *ud, unsigned seed);
LUA_API ശൂന്യം       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API പൂർണ്ണസംഖ്യ        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_സംഖ്യ (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API പൂർണ്ണസംഖ്യ   (lua_absindex) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ   (lua_gettop) (lua_State *L);
LUA_API ശൂന്യം  (lua_settop) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API ശൂന്യം  (lua_pushvalue) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API ശൂന്യം  (lua_rotate) (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ n);
LUA_API ശൂന്യം  (lua_copy) (lua_State *L, പൂർണ്ണസംഖ്യ fromidx, പൂർണ്ണസംഖ്യ toidx);
LUA_API പൂർണ്ണസംഖ്യ   (lua_checkstack) (lua_State *L, പൂർണ്ണസംഖ്യ n);

LUA_API ശൂന്യം  (lua_xmove) (lua_State *from, lua_State *to, പൂർണ്ണസംഖ്യ n);


/*
** access functions (stack -> C)
*/

LUA_API പൂർണ്ണസംഖ്യ             (lua_isnumber) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ             (lua_isstring) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ             (lua_iscfunction) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ             (lua_isinteger) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ             (lua_isuserdata) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ             (lua_type) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API സ്ഥിരം അക്ഷരം     *(lua_typename) (lua_State *L, പൂർണ്ണസംഖ്യ tp);

LUA_API lua_സംഖ്യ      (lua_tonumberx) (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ *isnum);
LUA_API പൂർണ്ണസംഖ്യ             (lua_toboolean) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API സ്ഥിരം അക്ഷരം     *(lua_tolstring) (lua_State *L, പൂർണ്ണസംഖ്യ idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API ശൂന്യം	       *(lua_touserdata) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API സ്ഥിരം ശൂന്യം     *(lua_topointer) (lua_State *L, പൂർണ്ണസംഖ്യ idx);


/*
** Comparison and arithmetic functions
*/

#define LUA_OPADD	0	/* ORDER TM, ORDER OP */
#define LUA_OPSUB	1
#define LUA_OPMUL	2
#define LUA_OPMOD	3
#define LUA_OPPOW	4
#define LUA_OPDIV	5
#define LUA_OPIDIV	6
#define LUA_OPBAND	7
#define LUA_OPBOR	8
#define LUA_OPBXOR	9
#define LUA_OPSHL	10
#define LUA_OPSHR	11
#define LUA_OPUNM	12
#define LUA_OPBNOT	13

LUA_API ശൂന്യം  (lua_arith) (lua_State *L, പൂർണ്ണസംഖ്യ op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API പൂർണ്ണസംഖ്യ   (lua_rawequal) (lua_State *L, പൂർണ്ണസംഖ്യ idx1, പൂർണ്ണസംഖ്യ idx2);
LUA_API പൂർണ്ണസംഖ്യ   (lua_compare) (lua_State *L, പൂർണ്ണസംഖ്യ idx1, പൂർണ്ണസംഖ്യ idx2, പൂർണ്ണസംഖ്യ op);


/*
** push functions (C -> stack)
*/
LUA_API ശൂന്യം        (lua_pushnil) (lua_State *L);
LUA_API ശൂന്യം        (lua_pushnumber) (lua_State *L, lua_സംഖ്യ n);
LUA_API ശൂന്യം        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API സ്ഥിരം അക്ഷരം *(lua_pushlstring) (lua_State *L, സ്ഥിരം അക്ഷരം *s, size_t len);
LUA_API സ്ഥിരം അക്ഷരം *(lua_pushexternalstring) (lua_State *L,
		സ്ഥിരം അക്ഷരം *s, size_t len, lua_Alloc falloc, ശൂന്യം *ud);
LUA_API സ്ഥിരം അക്ഷരം *(lua_pushstring) (lua_State *L, സ്ഥിരം അക്ഷരം *s);
LUA_API സ്ഥിരം അക്ഷരം *(lua_pushvfstring) (lua_State *L, സ്ഥിരം അക്ഷരം *fmt,
                                                      va_list argp);
LUA_API സ്ഥിരം അക്ഷരം *(lua_pushfstring) (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, ...);
LUA_API ശൂന്യം  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, പൂർണ്ണസംഖ്യ n);
LUA_API ശൂന്യം  (lua_pushboolean) (lua_State *L, പൂർണ്ണസംഖ്യ b);
LUA_API ശൂന്യം  (lua_pushlightuserdata) (lua_State *L, ശൂന്യം *p);
LUA_API പൂർണ്ണസംഖ്യ   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API പൂർണ്ണസംഖ്യ (lua_getglobal) (lua_State *L, സ്ഥിരം അക്ഷരം *name);
LUA_API പൂർണ്ണസംഖ്യ (lua_gettable) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ (lua_getfield) (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം അക്ഷരം *k);
LUA_API പൂർണ്ണസംഖ്യ (lua_geti) (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n);
LUA_API പൂർണ്ണസംഖ്യ (lua_rawget) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API പൂർണ്ണസംഖ്യ (lua_rawgeti) (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n);
LUA_API പൂർണ്ണസംഖ്യ (lua_rawgetp) (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം ശൂന്യം *p);

LUA_API ശൂന്യം  (lua_createtable) (lua_State *L, പൂർണ്ണസംഖ്യ narr, പൂർണ്ണസംഖ്യ nrec);
LUA_API ശൂന്യം *(lua_newuserdatauv) (lua_State *L, size_t sz, പൂർണ്ണസംഖ്യ nuvalue);
LUA_API പൂർണ്ണസംഖ്യ   (lua_getmetatable) (lua_State *L, പൂർണ്ണസംഖ്യ objindex);
LUA_API പൂർണ്ണസംഖ്യ  (lua_getiuservalue) (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ n);


/*
** set functions (stack -> Lua)
*/
LUA_API ശൂന്യം  (lua_setglobal) (lua_State *L, സ്ഥിരം അക്ഷരം *name);
LUA_API ശൂന്യം  (lua_settable) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API ശൂന്യം  (lua_setfield) (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം അക്ഷരം *k);
LUA_API ശൂന്യം  (lua_seti) (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n);
LUA_API ശൂന്യം  (lua_rawset) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API ശൂന്യം  (lua_rawseti) (lua_State *L, പൂർണ്ണസംഖ്യ idx, lua_Integer n);
LUA_API ശൂന്യം  (lua_rawsetp) (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം ശൂന്യം *p);
LUA_API പൂർണ്ണസംഖ്യ   (lua_setmetatable) (lua_State *L, പൂർണ്ണസംഖ്യ objindex);
LUA_API പൂർണ്ണസംഖ്യ   (lua_setiuservalue) (lua_State *L, പൂർണ്ണസംഖ്യ idx, പൂർണ്ണസംഖ്യ n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API ശൂന്യം  (lua_callk) (lua_State *L, പൂർണ്ണസംഖ്യ nargs, പൂർണ്ണസംഖ്യ nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, ശൂന്യ)

LUA_API പൂർണ്ണസംഖ്യ   (lua_pcallk) (lua_State *L, പൂർണ്ണസംഖ്യ nargs, പൂർണ്ണസംഖ്യ nresults, പൂർണ്ണസംഖ്യ errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, ശൂന്യ)

LUA_API പൂർണ്ണസംഖ്യ   (lua_load) (lua_State *L, lua_Reader reader, ശൂന്യം *dt,
                          സ്ഥിരം അക്ഷരം *chunkname, സ്ഥിരം അക്ഷരം *mode);

LUA_API പൂർണ്ണസംഖ്യ (lua_dump) (lua_State *L, lua_Writer writer, ശൂന്യം *data, പൂർണ്ണസംഖ്യ strip);


/*
** coroutine functions
*/
LUA_API പൂർണ്ണസംഖ്യ  (lua_yieldk)     (lua_State *L, പൂർണ്ണസംഖ്യ nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API പൂർണ്ണസംഖ്യ  (lua_resume)     (lua_State *L, lua_State *from, പൂർണ്ണസംഖ്യ narg,
                               പൂർണ്ണസംഖ്യ *nres);
LUA_API പൂർണ്ണസംഖ്യ  (lua_status)     (lua_State *L);
LUA_API പൂർണ്ണസംഖ്യ (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, ശൂന്യ)


/*
** Warning-related functions
*/
LUA_API ശൂന്യം (lua_setwarnf) (lua_State *L, lua_WarnFunction f, ശൂന്യം *ud);
LUA_API ശൂന്യം (lua_warning)  (lua_State *L, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ tocont);


/*
** garbage-collection options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCISRUNNING		6
#define LUA_GCGEN		7
#define LUA_GCINC		8
#define LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#define LUA_GCPMINORMUL		0  /* control minor collections */
#define LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#define LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#define LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#define LUA_GCPSTEPMUL		4  /* GC "speed" */
#define LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#define LUA_GCPN		6


LUA_API പൂർണ്ണസംഖ്യ (lua_gc) (lua_State *L, പൂർണ്ണസംഖ്യ what, ...);


/*
** miscellaneous functions
*/

LUA_API പൂർണ്ണസംഖ്യ   (lua_error) (lua_State *L);

LUA_API പൂർണ്ണസംഖ്യ   (lua_next) (lua_State *L, പൂർണ്ണസംഖ്യ idx);

LUA_API ശൂന്യം  (lua_concat) (lua_State *L, പൂർണ്ണസംഖ്യ n);
LUA_API ശൂന്യം  (lua_len)    (lua_State *L, പൂർണ്ണസംഖ്യ idx);

#define LUA_N2SBUFFSZ	64
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, പൂർണ്ണസംഖ്യ idx, അക്ഷരം *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, സ്ഥിരം അക്ഷരം *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, ശൂന്യം **ud);
LUA_API ശൂന്യം      (lua_setallocf) (lua_State *L, lua_Alloc f, ശൂന്യം *ud);

LUA_API ശൂന്യം (lua_toclose) (lua_State *L, പൂർണ്ണസംഖ്യ idx);
LUA_API ശൂന്യം (lua_closeslot) (lua_State *L, പൂർണ്ണസംഖ്യ idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((ശൂന്യം *)((അക്ഷരം *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),ശൂന്യ)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),ശൂന്യ)

#define lua_എടുക്കുക(L,n)		lua_settop(L, -(n)-1)

#define lua_newtable(L)		lua_createtable(L, 0, 0)

#define lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#define lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#define lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#define lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#define lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#define lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#define lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#define lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#define lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#define lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#define lua_pushglobaltable(L)  \
	((ശൂന്യം)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), ശൂന്യ)


#define lua_തിരുകുക(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_എടുക്കുക(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_എടുക്കുക(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,ശൂന്യ)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API പൂർണ്ണസംഖ്യ (lua_getstack) (lua_State *L, പൂർണ്ണസംഖ്യ level, lua_Debug *ar);
LUA_API പൂർണ്ണസംഖ്യ (lua_getinfo) (lua_State *L, സ്ഥിരം അക്ഷരം *what, lua_Debug *ar);
LUA_API സ്ഥിരം അക്ഷരം *(lua_getlocal) (lua_State *L, സ്ഥിരം lua_Debug *ar, പൂർണ്ണസംഖ്യ n);
LUA_API സ്ഥിരം അക്ഷരം *(lua_setlocal) (lua_State *L, സ്ഥിരം lua_Debug *ar, പൂർണ്ണസംഖ്യ n);
LUA_API സ്ഥിരം അക്ഷരം *(lua_getupvalue) (lua_State *L, പൂർണ്ണസംഖ്യ funcindex, പൂർണ്ണസംഖ്യ n);
LUA_API സ്ഥിരം അക്ഷരം *(lua_setupvalue) (lua_State *L, പൂർണ്ണസംഖ്യ funcindex, പൂർണ്ണസംഖ്യ n);

LUA_API ശൂന്യം *(lua_upvalueid) (lua_State *L, പൂർണ്ണസംഖ്യ fidx, പൂർണ്ണസംഖ്യ n);
LUA_API ശൂന്യം  (lua_upvaluejoin) (lua_State *L, പൂർണ്ണസംഖ്യ fidx1, പൂർണ്ണസംഖ്യ n1,
                                               പൂർണ്ണസംഖ്യ fidx2, പൂർണ്ണസംഖ്യ n2);

LUA_API ശൂന്യം (lua_sethook) (lua_State *L, lua_Hook func, പൂർണ്ണസംഖ്യ mask, പൂർണ്ണസംഖ്യ എണ്ണം);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API പൂർണ്ണസംഖ്യ (lua_gethookmask) (lua_State *L);
LUA_API പൂർണ്ണസംഖ്യ (lua_gethookcount) (lua_State *L);


ഘടന lua_Debug {
  പൂർണ്ണസംഖ്യ event;
  സ്ഥിരം അക്ഷരം *name;	/* (n) */
  സ്ഥിരം അക്ഷരം *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  സ്ഥിരം അക്ഷരം *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  സ്ഥിരം അക്ഷരം *source;	/* (S) */
  size_t srclen;	/* (S) */
  പൂർണ്ണസംഖ്യ currentline;	/* (l) */
  പൂർണ്ണസംഖ്യ linedefined;	/* (S) */
  പൂർണ്ണസംഖ്യ lastlinedefined;	/* (S) */
  unsigned അക്ഷരം nups;	/* (u) number of upvalues */
  unsigned അക്ഷരം nparams;/* (u) number of parameters */
  അക്ഷരം isvararg;        /* (u) */
  unsigned അക്ഷരം extraargs;  /* (t) number of extra arguments */
  അക്ഷരം istailcall;	/* (t) */
  പൂർണ്ണസംഖ്യ ftransfer;   /* (r) index of first value transferred */
  പൂർണ്ണസംഖ്യ ntransfer;   /* (r) number of transferred values */
  അക്ഷരം short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  ഘടന CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#define LUAI_TOSTRAUX(x)	#x
#define LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#define LUA_VERSION_MAJOR	LUAI_TOSTR(LUA_VERSION_MAJOR_N)
#define LUA_VERSION_MINOR	LUAI_TOSTR(LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE	LUAI_TOSTR(LUA_VERSION_RELEASE_N)

#define LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#define LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE


/******************************************************************************
* Copyright (C) 1994-2026 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
