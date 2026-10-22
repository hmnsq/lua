/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#నిర్వచితం_కాకపోతే lua_h
#నిర్వచించు lua_h

#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stddef.h>


#నిర్వచించు LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#నిర్వచించు LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#నిర్వచించు LUA_సంస్కరణ_MAJOR_N	5
#నిర్వచించు LUA_సంస్కరణ_MINOR_N	5
#నిర్వచించు LUA_సంస్కరణ_RELEASE_N	1

#నిర్వచించు LUA_సంస్కరణ_అంకె  (LUA_సంస్కరణ_MAJOR_N * 100 + LUA_సంస్కరణ_MINOR_N)
#నిర్వచించు LUA_సంస్కరణ_RELEASE_అంకె  (LUA_సంస్కరణ_అంకె * 100 + LUA_సంస్కరణ_RELEASE_N)


#అన్తర్భావ "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#నిర్వచించు LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#నిర్వచించు LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#నిర్వచించు LUA_REGISTRYINDEX	(-(గరిష్ఠ_పూర్ణాంకం/2 + 1000))
#నిర్వచించు lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#నిర్వచించు LUA_OK		0
#నిర్వచించు LUA_YIELD	1
#నిర్వచించు LUA_ERRRUN	2
#నిర్వచించు LUA_ERRSYNTAX	3
#నిర్వచించు LUA_ERRMEM	4
#నిర్వచించు LUA_ERRERR	5


రకనిర్వచనం నిర్మాణం lua_State lua_State;


/*
** basic types
*/
#నిర్వచించు LUA_TNONE		(-1)

#నిర్వచించు LUA_TNIL		0
#నిర్వచించు LUA_TBOOLEAN		1
#నిర్వచించు LUA_TLIGHTUSERDATA	2
#నిర్వచించు LUA_TNUMBER		3
#నిర్వచించు LUA_TSTRING		4
#నిర్వచించు LUA_TTABLE		5
#నిర్వచించు LUA_TFUNCTION		6
#నిర్వచించు LUA_TUSERDATA		7
#నిర్వచించు LUA_TTHREAD		8

#నిర్వచించు LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#నిర్వచించు LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#నిర్వచించు LUA_RIDX_GLOBALS	2
#నిర్వచించు LUA_RIDX_MAINTHREAD	3
#నిర్వచించు LUA_RIDX_చివరి		3


/* type of numbers in Lua */
రకనిర్వచనం LUA_సంఖ్య lua_సంఖ్య;


/* type for integer functions */
రకనిర్వచనం LUA_INTEGER lua_Integer;

/* unsigned integer type */
రకనిర్వచనం LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
రకనిర్వచనం LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
రకనిర్వచనం పూర్ణాంకం (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
రకనిర్వచనం పూర్ణాంకం (*lua_KFunction) (lua_State *L, పూర్ణాంకం స్థితి_2, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
రకనిర్వచనం స్థిరరాశి అక్షరం * (*lua_Reader) (lua_State *L, శూన్య *ud, size_t *sz);

రకనిర్వచనం పూర్ణాంకం (*lua_Writer) (lua_State *L, స్థిరరాశి శూన్య *p, size_t sz, శూన్య *ud);


/*
** Type for memory-allocation functions
*/
రకనిర్వచనం శూన్య * (*lua_Alloc) (శూన్య *ud, శూన్య *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
రకనిర్వచనం శూన్య (*lua_WarnFunction) (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం tocont);


/*
** Type used by the debug API to collect debug information
*/
రకనిర్వచనం నిర్మాణం lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
రకనిర్వచనం శూన్య (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#ఒకవేళ defined(LUA_USER_H)
#అన్తర్భావ LUA_USER_H
#అయితే_ముగింపు


/*
** RCS ident string
*/
బాహ్య స్థిరరాశి అక్షరం lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, శూన్య *ud, గుర్తులేని విత్తనం);
LUA_API శూన్య       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API పూర్ణాంకం        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_సంఖ్య (lua_సంస్కరణ) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API పూర్ణాంకం   (lua_absindex) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం   (lua_gettop) (lua_State *L);
LUA_API శూన్య  (lua_settop) (lua_State *L, పూర్ణాంకం idx);
LUA_API శూన్య  (lua_pushvalue) (lua_State *L, పూర్ణాంకం idx);
LUA_API శూన్య  (lua_rotate) (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం n);
LUA_API శూన్య  (lua_copy) (lua_State *L, పూర్ణాంకం fromidx, పూర్ణాంకం toidx);
LUA_API పూర్ణాంకం   (lua_checkstack) (lua_State *L, పూర్ణాంకం n);

LUA_API శూన్య  (lua_xmove) (lua_State *from, lua_State *గమ్యం, పూర్ణాంకం n);


/*
** access functions (stack -> C)
*/

LUA_API పూర్ణాంకం             (lua_isnumber) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం             (lua_isstring) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం             (lua_iscfunction) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం             (lua_isinteger) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం             (lua_isuserdata) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం             (lua_type) (lua_State *L, పూర్ణాంకం idx);
LUA_API స్థిరరాశి అక్షరం     *(lua_typename) (lua_State *L, పూర్ణాంకం tp);

LUA_API lua_సంఖ్య      (lua_tonumberx) (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం *isnum);
LUA_API పూర్ణాంకం             (lua_toboolean) (lua_State *L, పూర్ణాంకం idx);
LUA_API స్థిరరాశి అక్షరం     *(lua_tolstring) (lua_State *L, పూర్ణాంకం idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, పూర్ణాంకం idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, పూర్ణాంకం idx);
LUA_API శూన్య	       *(lua_touserdata) (lua_State *L, పూర్ణాంకం idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, పూర్ణాంకం idx);
LUA_API స్థిరరాశి శూన్య     *(lua_topointer) (lua_State *L, పూర్ణాంకం idx);


/*
** Comparison and arithmetic functions
*/

#నిర్వచించు LUA_OPADD	0	/* ORDER TM, ORDER OP */
#నిర్వచించు LUA_OPSUB	1
#నిర్వచించు LUA_OPMUL	2
#నిర్వచించు LUA_OPMOD	3
#నిర్వచించు LUA_OPPOW	4
#నిర్వచించు LUA_OPDIV	5
#నిర్వచించు LUA_OPIDIV	6
#నిర్వచించు LUA_OPBAND	7
#నిర్వచించు LUA_OPBOR	8
#నిర్వచించు LUA_OPBXOR	9
#నిర్వచించు LUA_OPSHL	10
#నిర్వచించు LUA_OPSHR	11
#నిర్వచించు LUA_OPUNM	12
#నిర్వచించు LUA_OPBNOT	13

LUA_API శూన్య  (lua_arith) (lua_State *L, పూర్ణాంకం op);

#నిర్వచించు LUA_OPEQ	0
#నిర్వచించు LUA_OPLT	1
#నిర్వచించు LUA_OPLE	2

LUA_API పూర్ణాంకం   (lua_rawequal) (lua_State *L, పూర్ణాంకం idx1, పూర్ణాంకం idx2);
LUA_API పూర్ణాంకం   (lua_compare) (lua_State *L, పూర్ణాంకం idx1, పూర్ణాంకం idx2, పూర్ణాంకం op);


/*
** push functions (C -> stack)
*/
LUA_API శూన్య        (lua_pushnil) (lua_State *L);
LUA_API శూన్య        (lua_pushnumber) (lua_State *L, lua_సంఖ్య n);
LUA_API శూన్య        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API స్థిరరాశి అక్షరం *(lua_pushlstring) (lua_State *L, స్థిరరాశి అక్షరం *s, size_t len);
LUA_API స్థిరరాశి అక్షరం *(lua_pushexternalstring) (lua_State *L,
		స్థిరరాశి అక్షరం *s, size_t len, lua_Alloc falloc, శూన్య *ud);
LUA_API స్థిరరాశి అక్షరం *(lua_pushstring) (lua_State *L, స్థిరరాశి అక్షరం *s);
LUA_API స్థిరరాశి అక్షరం *(lua_pushvfstring) (lua_State *L, స్థిరరాశి అక్షరం *fmt,
                                                      va_list argp);
LUA_API స్థిరరాశి అక్షరం *(lua_pushfstring) (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...);
LUA_API శూన్య  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, పూర్ణాంకం n);
LUA_API శూన్య  (lua_pushboolean) (lua_State *L, పూర్ణాంకం b);
LUA_API శూన్య  (lua_pushlightuserdata) (lua_State *L, శూన్య *p);
LUA_API పూర్ణాంకం   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API పూర్ణాంకం (lua_getglobal) (lua_State *L, స్థిరరాశి అక్షరం *name);
LUA_API పూర్ణాంకం (lua_gettable) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం (lua_getfield) (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి అక్షరం *k);
LUA_API పూర్ణాంకం (lua_geti) (lua_State *L, పూర్ణాంకం idx, lua_Integer n);
LUA_API పూర్ణాంకం (lua_rawget) (lua_State *L, పూర్ణాంకం idx);
LUA_API పూర్ణాంకం (lua_rawgeti) (lua_State *L, పూర్ణాంకం idx, lua_Integer n);
LUA_API పూర్ణాంకం (lua_rawgetp) (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి శూన్య *p);

LUA_API శూన్య  (lua_createtable) (lua_State *L, పూర్ణాంకం narr, పూర్ణాంకం nrec);
LUA_API శూన్య *(lua_newuserdatauv) (lua_State *L, size_t sz, పూర్ణాంకం nuvalue);
LUA_API పూర్ణాంకం   (lua_getmetatable) (lua_State *L, పూర్ణాంకం objindex);
LUA_API పూర్ణాంకం  (lua_getiuservalue) (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం n);


/*
** set functions (stack -> Lua)
*/
LUA_API శూన్య  (lua_setglobal) (lua_State *L, స్థిరరాశి అక్షరం *name);
LUA_API శూన్య  (lua_settable) (lua_State *L, పూర్ణాంకం idx);
LUA_API శూన్య  (lua_setfield) (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి అక్షరం *k);
LUA_API శూన్య  (lua_seti) (lua_State *L, పూర్ణాంకం idx, lua_Integer n);
LUA_API శూన్య  (lua_rawset) (lua_State *L, పూర్ణాంకం idx);
LUA_API శూన్య  (lua_rawseti) (lua_State *L, పూర్ణాంకం idx, lua_Integer n);
LUA_API శూన్య  (lua_rawsetp) (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి శూన్య *p);
LUA_API పూర్ణాంకం   (lua_setmetatable) (lua_State *L, పూర్ణాంకం objindex);
LUA_API పూర్ణాంకం   (lua_setiuservalue) (lua_State *L, పూర్ణాంకం idx, పూర్ణాంకం n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API శూన్య  (lua_callk) (lua_State *L, పూర్ణాంకం nargs, పూర్ణాంకం nresults,
                           lua_KContext ctx, lua_KFunction k);
#నిర్వచించు lua_call(L,n,r)		lua_callk(L, (n), (r), 0, ఖాళీ)

LUA_API పూర్ణాంకం   (lua_pcallk) (lua_State *L, పూర్ణాంకం nargs, పూర్ణాంకం nresults, పూర్ణాంకం errfunc,
                            lua_KContext ctx, lua_KFunction k);
#నిర్వచించు lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, ఖాళీ)

LUA_API పూర్ణాంకం   (lua_ఎక్కించు) (lua_State *L, lua_Reader reader, శూన్య *dt,
                          స్థిరరాశి అక్షరం *chunkname, స్థిరరాశి అక్షరం *బహుళకం);

LUA_API పూర్ణాంకం (lua_dump) (lua_State *L, lua_Writer writer, శూన్య *దత్తాంశం, పూర్ణాంకం strip);


/*
** coroutine functions
*/
LUA_API పూర్ణాంకం  (lua_yieldk)     (lua_State *L, పూర్ణాంకం nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API పూర్ణాంకం  (lua_resume)     (lua_State *L, lua_State *from, పూర్ణాంకం narg,
                               పూర్ణాంకం *nres);
LUA_API పూర్ణాంకం  (lua_స్థితి)     (lua_State *L);
LUA_API పూర్ణాంకం (lua_isyieldable) (lua_State *L);

#నిర్వచించు lua_yield(L,n)		lua_yieldk(L, (n), 0, ఖాళీ)


/*
** Warning-related functions
*/
LUA_API శూన్య (lua_setwarnf) (lua_State *L, lua_WarnFunction f, శూన్య *ud);
LUA_API శూన్య (lua_warning)  (lua_State *L, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం tocont);


/*
** garbage-collection options
*/

#నిర్వచించు LUA_GCSTOP		0
#నిర్వచించు LUA_GCRESTART		1
#నిర్వచించు LUA_GCCOLLECT		2
#నిర్వచించు LUA_GCCOUNT		3
#నిర్వచించు LUA_GCCOUNTB		4
#నిర్వచించు LUA_GCSTEP		5
#నిర్వచించు LUA_GCISRUNNING		6
#నిర్వచించు LUA_GCGEN		7
#నిర్వచించు LUA_GCINC		8
#నిర్వచించు LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#నిర్వచించు LUA_GCPMINORMUL		0  /* control minor collections */
#నిర్వచించు LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#నిర్వచించు LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#నిర్వచించు LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#నిర్వచించు LUA_GCPSTEPMUL		4  /* GC "speed" */
#నిర్వచించు LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#నిర్వచించు LUA_GCPN		6


LUA_API పూర్ణాంకం (lua_gc) (lua_State *L, పూర్ణాంకం what, ...);


/*
** miscellaneous functions
*/

LUA_API పూర్ణాంకం   (lua_error) (lua_State *L);

LUA_API పూర్ణాంకం   (lua_తదుపరి) (lua_State *L, పూర్ణాంకం idx);

LUA_API శూన్య  (lua_concat) (lua_State *L, పూర్ణాంకం n);
LUA_API శూన్య  (lua_len)    (lua_State *L, పూర్ణాంకం idx);

#నిర్వచించు LUA_N2SBUFFSZ	64
LUA_API గుర్తులేని  (lua_numbertocstring) (lua_State *L, పూర్ణాంకం idx, అక్షరం *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, స్థిరరాశి అక్షరం *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, శూన్య **ud);
LUA_API శూన్య      (lua_setallocf) (lua_State *L, lua_Alloc f, శూన్య *ud);

LUA_API శూన్య (lua_toclose) (lua_State *L, పూర్ణాంకం idx);
LUA_API శూన్య (lua_closeslot) (lua_State *L, పూర్ణాంకం idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#నిర్వచించు lua_getextraspace(L)	((శూన్య *)((అక్షరం *)(L) - LUA_EXTRASPACE))

#నిర్వచించు lua_tonumber(L,i)	lua_tonumberx(L,(i),ఖాళీ)
#నిర్వచించు lua_tointeger(L,i)	lua_tointegerx(L,(i),ఖాళీ)

#నిర్వచించు lua_తీయి(L,n)		lua_settop(L, -(n)-1)

#నిర్వచించు lua_newtable(L)		lua_createtable(L, 0, 0)

#నిర్వచించు lua_రిజిస్టరు(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#నిర్వచించు lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#నిర్వచించు lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#నిర్వచించు lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#నిర్వచించు lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#నిర్వచించు lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#నిర్వచించు lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#నిర్వచించు lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#నిర్వచించు lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#నిర్వచించు lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#నిర్వచించు lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#నిర్వచించు lua_pushglobaltable(L)  \
	((శూన్య)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#నిర్వచించు lua_tostring(L,i)	lua_tolstring(L, (i), ఖాళీ)


#నిర్వచించు lua_చొప్పించు(L,idx)	lua_rotate(L, (idx), 1)

#నిర్వచించు lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_తీయి(L, 1))

#నిర్వచించు lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_తీయి(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#నిర్వచించు lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#నిర్వచించు lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#నిర్వచించు lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#నిర్వచించు lua_resetthread(L)	lua_closethread(L,ఖాళీ)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#నిర్వచించు LUA_HOOKCALL	0
#నిర్వచించు LUA_HOOKRET	1
#నిర్వచించు LUA_HOOKLINE	2
#నిర్వచించు LUA_HOOKCOUNT	3
#నిర్వచించు LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#నిర్వచించు LUA_MASKCALL	(1 << LUA_HOOKCALL)
#నిర్వచించు LUA_MASKRET	(1 << LUA_HOOKRET)
#నిర్వచించు LUA_MASKLINE	(1 << LUA_HOOKLINE)
#నిర్వచించు LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API పూర్ణాంకం (lua_getstack) (lua_State *L, పూర్ణాంకం level, lua_Debug *ar);
LUA_API పూర్ణాంకం (lua_getinfo) (lua_State *L, స్థిరరాశి అక్షరం *what, lua_Debug *ar);
LUA_API స్థిరరాశి అక్షరం *(lua_getlocal) (lua_State *L, స్థిరరాశి lua_Debug *ar, పూర్ణాంకం n);
LUA_API స్థిరరాశి అక్షరం *(lua_setlocal) (lua_State *L, స్థిరరాశి lua_Debug *ar, పూర్ణాంకం n);
LUA_API స్థిరరాశి అక్షరం *(lua_getupvalue) (lua_State *L, పూర్ణాంకం funcindex, పూర్ణాంకం n);
LUA_API స్థిరరాశి అక్షరం *(lua_setupvalue) (lua_State *L, పూర్ణాంకం funcindex, పూర్ణాంకం n);

LUA_API శూన్య *(lua_upvalueid) (lua_State *L, పూర్ణాంకం fidx, పూర్ణాంకం n);
LUA_API శూన్య  (lua_upvaluejoin) (lua_State *L, పూర్ణాంకం fidx1, పూర్ణాంకం n1,
                                               పూర్ణాంకం fidx2, పూర్ణాంకం n2);

LUA_API శూన్య (lua_sethook) (lua_State *L, lua_Hook func, పూర్ణాంకం ముసుగు_2, పూర్ణాంకం లెక్క);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API పూర్ణాంకం (lua_gethookmask) (lua_State *L);
LUA_API పూర్ణాంకం (lua_gethookcount) (lua_State *L);


నిర్మాణం lua_Debug {
  పూర్ణాంకం event;
  స్థిరరాశి అక్షరం *name;	/* (n) */
  స్థిరరాశి అక్షరం *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  స్థిరరాశి అక్షరం *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  స్థిరరాశి అక్షరం *ఆధారం;	/* (S) */
  size_t srclen;	/* (S) */
  పూర్ణాంకం currentline;	/* (l) */
  పూర్ణాంకం linedefined;	/* (S) */
  పూర్ణాంకం lastlinedefined;	/* (S) */
  గుర్తులేని అక్షరం nups;	/* (u) number of upvalues */
  గుర్తులేని అక్షరం nparams;/* (u) number of parameters */
  అక్షరం isvararg;        /* (u) */
  గుర్తులేని అక్షరం extraargs;  /* (t) number of extra arguments */
  అక్షరం istailcall;	/* (t) */
  పూర్ణాంకం ftransfer;   /* (r) index of first value transferred */
  పూర్ణాంకం ntransfer;   /* (r) number of transferred values */
  అక్షరం short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  నిర్మాణం CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#నిర్వచించు LUAI_TOSTRAUX(x)	#x
#నిర్వచించు LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#నిర్వచించు LUA_సంస్కరణ_MAJOR	LUAI_TOSTR(LUA_సంస్కరణ_MAJOR_N)
#నిర్వచించు LUA_సంస్కరణ_MINOR	LUAI_TOSTR(LUA_సంస్కరణ_MINOR_N)
#నిర్వచించు LUA_సంస్కరణ_RELEASE	LUAI_TOSTR(LUA_సంస్కరణ_RELEASE_N)

#నిర్వచించు LUA_సంస్కరణ	"Lua " LUA_సంస్కరణ_MAJOR "." LUA_సంస్కరణ_MINOR
#నిర్వచించు LUA_RELEASE	LUA_సంస్కరణ "." LUA_సంస్కరణ_RELEASE


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


#అయితే_ముగింపు
