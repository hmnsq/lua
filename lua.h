/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <stddef.h>


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	1

#define LUA_VERSION_অঙ্ক  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_অঙ্ক  (LUA_VERSION_অঙ্ক * 100 + LUA_VERSION_RELEASE_N)


#অন্তর্ভাৱ "luaconf.h"


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


প্রকারনির্দেশ সংরচনা lua_State lua_State;


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
প্রকারনির্দেশ LUA_সংখ্যা lua_সংখ্যা;


/* type for integer functions */
প্রকারনির্দেশ LUA_INTEGER lua_Integer;

/* unsigned integer type */
প্রকারনির্দেশ LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
প্রকারনির্দেশ LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
প্রকারনির্দেশ পূর্ণাঙ্ক (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
প্রকারনির্দেশ পূর্ণাঙ্ক (*lua_KFunction) (lua_State *L, পূর্ণাঙ্ক status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
প্রকারনির্দেশ ধ্রুৱ ৱর্ণ * (*lua_Reader) (lua_State *L, শূন্য *ud, size_t *sz);

প্রকারনির্দেশ পূর্ণাঙ্ক (*lua_Writer) (lua_State *L, ধ্রুৱ শূন্য *p, size_t sz, শূন্য *ud);


/*
** Type for memory-allocation functions
*/
প্রকারনির্দেশ শূন্য * (*lua_Alloc) (শূন্য *ud, শূন্য *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
প্রকারনির্দেশ শূন্য (*lua_WarnFunction) (শূন্য *ud, ধ্রুৱ ৱর্ণ *বাৰ্তা, পূর্ণাঙ্ক tocont);


/*
** Type used by the debug API to collect debug information
*/
প্রকারনির্দেশ সংরচনা lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
প্রকারনির্দেশ শূন্য (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#যদি defined(LUA_USER_H)
#অন্তর্ভাৱ LUA_USER_H
#endif


/*
** RCS ident string
*/
বাহ্য ধ্রুৱ ৱর্ণ lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, শূন্য *ud, unsigned seed);
LUA_API শূন্য       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API পূর্ণাঙ্ক        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_সংখ্যা (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API পূর্ণাঙ্ক   (lua_absindex) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক   (lua_gettop) (lua_State *L);
LUA_API শূন্য  (lua_settop) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API শূন্য  (lua_pushvalue) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API শূন্য  (lua_rotate) (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক n);
LUA_API শূন্য  (lua_copy) (lua_State *L, পূর্ণাঙ্ক fromidx, পূর্ণাঙ্ক toidx);
LUA_API পূর্ণাঙ্ক   (lua_checkstack) (lua_State *L, পূর্ণাঙ্ক n);

LUA_API শূন্য  (lua_xmove) (lua_State *from, lua_State *to, পূর্ণাঙ্ক n);


/*
** access functions (stack -> C)
*/

LUA_API পূর্ণাঙ্ক             (lua_isnumber) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক             (lua_isstring) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক             (lua_iscfunction) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক             (lua_isinteger) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক             (lua_isuserdata) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক             (lua_type) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API ধ্রুৱ ৱর্ণ     *(lua_typename) (lua_State *L, পূর্ণাঙ্ক tp);

LUA_API lua_সংখ্যা      (lua_tonumberx) (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক *isnum);
LUA_API পূর্ণাঙ্ক             (lua_toboolean) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API ধ্রুৱ ৱর্ণ     *(lua_tolstring) (lua_State *L, পূর্ণাঙ্ক idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API শূন্য	       *(lua_touserdata) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API ধ্রুৱ শূন্য     *(lua_topointer) (lua_State *L, পূর্ণাঙ্ক idx);


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

LUA_API শূন্য  (lua_arith) (lua_State *L, পূর্ণাঙ্ক op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API পূর্ণাঙ্ক   (lua_rawequal) (lua_State *L, পূর্ণাঙ্ক idx1, পূর্ণাঙ্ক idx2);
LUA_API পূর্ণাঙ্ক   (lua_compare) (lua_State *L, পূর্ণাঙ্ক idx1, পূর্ণাঙ্ক idx2, পূর্ণাঙ্ক op);


/*
** push functions (C -> stack)
*/
LUA_API শূন্য        (lua_pushnil) (lua_State *L);
LUA_API শূন্য        (lua_pushnumber) (lua_State *L, lua_সংখ্যা n);
LUA_API শূন্য        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ধ্রুৱ ৱর্ণ *(lua_pushlstring) (lua_State *L, ধ্রুৱ ৱর্ণ *s, size_t len);
LUA_API ধ্রুৱ ৱর্ণ *(lua_pushexternalstring) (lua_State *L,
		ধ্রুৱ ৱর্ণ *s, size_t len, lua_Alloc falloc, শূন্য *ud);
LUA_API ধ্রুৱ ৱর্ণ *(lua_pushstring) (lua_State *L, ধ্রুৱ ৱর্ণ *s);
LUA_API ধ্রুৱ ৱর্ণ *(lua_pushvfstring) (lua_State *L, ধ্রুৱ ৱর্ণ *fmt,
                                                      va_list argp);
LUA_API ধ্রুৱ ৱর্ণ *(lua_pushfstring) (lua_State *L, ধ্রুৱ ৱর্ণ *fmt, ...);
LUA_API শূন্য  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, পূর্ণাঙ্ক n);
LUA_API শূন্য  (lua_pushboolean) (lua_State *L, পূর্ণাঙ্ক b);
LUA_API শূন্য  (lua_pushlightuserdata) (lua_State *L, শূন্য *p);
LUA_API পূর্ণাঙ্ক   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API পূর্ণাঙ্ক (lua_getglobal) (lua_State *L, ধ্রুৱ ৱর্ণ *name);
LUA_API পূর্ণাঙ্ক (lua_gettable) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক (lua_getfield) (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ ৱর্ণ *k);
LUA_API পূর্ণাঙ্ক (lua_geti) (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n);
LUA_API পূর্ণাঙ্ক (lua_rawget) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API পূর্ণাঙ্ক (lua_rawgeti) (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n);
LUA_API পূর্ণাঙ্ক (lua_rawgetp) (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ শূন্য *p);

LUA_API শূন্য  (lua_createtable) (lua_State *L, পূর্ণাঙ্ক narr, পূর্ণাঙ্ক nrec);
LUA_API শূন্য *(lua_newuserdatauv) (lua_State *L, size_t sz, পূর্ণাঙ্ক nuvalue);
LUA_API পূর্ণাঙ্ক   (lua_getmetatable) (lua_State *L, পূর্ণাঙ্ক objindex);
LUA_API পূর্ণাঙ্ক  (lua_getiuservalue) (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক n);


/*
** set functions (stack -> Lua)
*/
LUA_API শূন্য  (lua_setglobal) (lua_State *L, ধ্রুৱ ৱর্ণ *name);
LUA_API শূন্য  (lua_settable) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API শূন্য  (lua_setfield) (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ ৱর্ণ *k);
LUA_API শূন্য  (lua_seti) (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n);
LUA_API শূন্য  (lua_rawset) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API শূন্য  (lua_rawseti) (lua_State *L, পূর্ণাঙ্ক idx, lua_Integer n);
LUA_API শূন্য  (lua_rawsetp) (lua_State *L, পূর্ণাঙ্ক idx, ধ্রুৱ শূন্য *p);
LUA_API পূর্ণাঙ্ক   (lua_setmetatable) (lua_State *L, পূর্ণাঙ্ক objindex);
LUA_API পূর্ণাঙ্ক   (lua_setiuservalue) (lua_State *L, পূর্ণাঙ্ক idx, পূর্ণাঙ্ক n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API শূন্য  (lua_callk) (lua_State *L, পূর্ণাঙ্ক nargs, পূর্ণাঙ্ক nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, রিক্ত)

LUA_API পূর্ণাঙ্ক   (lua_pcallk) (lua_State *L, পূর্ণাঙ্ক nargs, পূর্ণাঙ্ক nresults, পূর্ণাঙ্ক errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, রিক্ত)

LUA_API পূর্ণাঙ্ক   (lua_load) (lua_State *L, lua_Reader reader, শূন্য *dt,
                          ধ্রুৱ ৱর্ণ *chunkname, ধ্রুৱ ৱর্ণ *mode);

LUA_API পূর্ণাঙ্ক (lua_dump) (lua_State *L, lua_Writer writer, শূন্য *data, পূর্ণাঙ্ক strip);


/*
** coroutine functions
*/
LUA_API পূর্ণাঙ্ক  (lua_yieldk)     (lua_State *L, পূর্ণাঙ্ক nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API পূর্ণাঙ্ক  (lua_resume)     (lua_State *L, lua_State *from, পূর্ণাঙ্ক narg,
                               পূর্ণাঙ্ক *nres);
LUA_API পূর্ণাঙ্ক  (lua_status)     (lua_State *L);
LUA_API পূর্ণাঙ্ক (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, রিক্ত)


/*
** Warning-related functions
*/
LUA_API শূন্য (lua_setwarnf) (lua_State *L, lua_WarnFunction f, শূন্য *ud);
LUA_API শূন্য (lua_warning)  (lua_State *L, ধ্রুৱ ৱর্ণ *বাৰ্তা, পূর্ণাঙ্ক tocont);


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


LUA_API পূর্ণাঙ্ক (lua_gc) (lua_State *L, পূর্ণাঙ্ক what, ...);


/*
** miscellaneous functions
*/

LUA_API পূর্ণাঙ্ক   (lua_error) (lua_State *L);

LUA_API পূর্ণাঙ্ক   (lua_next) (lua_State *L, পূর্ণাঙ্ক idx);

LUA_API শূন্য  (lua_concat) (lua_State *L, পূর্ণাঙ্ক n);
LUA_API শূন্য  (lua_len)    (lua_State *L, পূর্ণাঙ্ক idx);

#define LUA_N2SBUFFSZ	64
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, পূর্ণাঙ্ক idx, ৱর্ণ *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ধ্রুৱ ৱর্ণ *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, শূন্য **ud);
LUA_API শূন্য      (lua_setallocf) (lua_State *L, lua_Alloc f, শূন্য *ud);

LUA_API শূন্য (lua_toclose) (lua_State *L, পূর্ণাঙ্ক idx);
LUA_API শূন্য (lua_closeslot) (lua_State *L, পূর্ণাঙ্ক idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((শূন্য *)((ৱর্ণ *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),রিক্ত)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),রিক্ত)

#define lua_বেৰ_কৰো(L,n)		lua_settop(L, -(n)-1)

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
	((শূন্য)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), রিক্ত)


#define lua_ঢোকাও(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_বেৰ_কৰো(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_বেৰ_কৰো(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,রিক্ত)

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


LUA_API পূর্ণাঙ্ক (lua_getstack) (lua_State *L, পূর্ণাঙ্ক level, lua_Debug *ar);
LUA_API পূর্ণাঙ্ক (lua_getinfo) (lua_State *L, ধ্রুৱ ৱর্ণ *what, lua_Debug *ar);
LUA_API ধ্রুৱ ৱর্ণ *(lua_getlocal) (lua_State *L, ধ্রুৱ lua_Debug *ar, পূর্ণাঙ্ক n);
LUA_API ধ্রুৱ ৱর্ণ *(lua_setlocal) (lua_State *L, ধ্রুৱ lua_Debug *ar, পূর্ণাঙ্ক n);
LUA_API ধ্রুৱ ৱর্ণ *(lua_getupvalue) (lua_State *L, পূর্ণাঙ্ক funcindex, পূর্ণাঙ্ক n);
LUA_API ধ্রুৱ ৱর্ণ *(lua_setupvalue) (lua_State *L, পূর্ণাঙ্ক funcindex, পূর্ণাঙ্ক n);

LUA_API শূন্য *(lua_upvalueid) (lua_State *L, পূর্ণাঙ্ক fidx, পূর্ণাঙ্ক n);
LUA_API শূন্য  (lua_upvaluejoin) (lua_State *L, পূর্ণাঙ্ক fidx1, পূর্ণাঙ্ক n1,
                                               পূর্ণাঙ্ক fidx2, পূর্ণাঙ্ক n2);

LUA_API শূন্য (lua_sethook) (lua_State *L, lua_Hook func, পূর্ণাঙ্ক mask, পূর্ণাঙ্ক গুনতি);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API পূর্ণাঙ্ক (lua_gethookmask) (lua_State *L);
LUA_API পূর্ণাঙ্ক (lua_gethookcount) (lua_State *L);


সংরচনা lua_Debug {
  পূর্ণাঙ্ক event;
  ধ্রুৱ ৱর্ণ *name;	/* (n) */
  ধ্রুৱ ৱর্ণ *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ধ্রুৱ ৱর্ণ *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ধ্রুৱ ৱর্ণ *source;	/* (S) */
  size_t srclen;	/* (S) */
  পূর্ণাঙ্ক currentline;	/* (l) */
  পূর্ণাঙ্ক linedefined;	/* (S) */
  পূর্ণাঙ্ক lastlinedefined;	/* (S) */
  unsigned ৱর্ণ nups;	/* (u) number of upvalues */
  unsigned ৱর্ণ nparams;/* (u) number of parameters */
  ৱর্ণ isvararg;        /* (u) */
  unsigned ৱর্ণ extraargs;  /* (t) number of extra arguments */
  ৱর্ণ istailcall;	/* (t) */
  পূর্ণাঙ্ক ftransfer;   /* (r) index of first value transferred */
  পূর্ণাঙ্ক ntransfer;   /* (r) number of transferred values */
  ৱর্ণ short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  সংরচনা CallInfo *i_ci;  /* active function */
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
