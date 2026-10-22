/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#यदि_अपरिभाषितम् lua_h
#परिभाषय lua_h

#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stddef.h>


#परिभाषय LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#परिभाषय LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#परिभाषय LUA_संस्करणम्_MAJOR_N	5
#परिभाषय LUA_संस्करणम्_MINOR_N	5
#परिभाषय LUA_संस्करणम्_RELEASE_N	1

#परिभाषय LUA_संस्करणम्_अङ्कः  (LUA_संस्करणम्_MAJOR_N * 100 + LUA_संस्करणम्_MINOR_N)
#परिभाषय LUA_संस्करणम्_RELEASE_अङ्कः  (LUA_संस्करणम्_अङ्कः * 100 + LUA_संस्करणम्_RELEASE_N)


#अन्तर्भाव "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#परिभाषय LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#परिभाषय LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#परिभाषय LUA_REGISTRYINDEX	(-(अधिकतम_पूर्णाङ्कः/2 + 1000))
#परिभाषय lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#परिभाषय LUA_OK		0
#परिभाषय LUA_YIELD	1
#परिभाषय LUA_ERRRUN	2
#परिभाषय LUA_ERRSYNTAX	3
#परिभाषय LUA_ERRMEM	4
#परिभाषय LUA_ERRERR	5


प्रकारनिर्देश संरचना lua_State lua_State;


/*
** basic types
*/
#परिभाषय LUA_TNONE		(-1)

#परिभाषय LUA_TNIL		0
#परिभाषय LUA_TBOOLEAN		1
#परिभाषय LUA_TLIGHTUSERDATA	2
#परिभाषय LUA_TNUMBER		3
#परिभाषय LUA_TSTRING		4
#परिभाषय LUA_TTABLE		5
#परिभाषय LUA_TFUNCTION		6
#परिभाषय LUA_TUSERDATA		7
#परिभाषय LUA_TTHREAD		8

#परिभाषय LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#परिभाषय LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#परिभाषय LUA_RIDX_GLOBALS	2
#परिभाषय LUA_RIDX_MAINTHREAD	3
#परिभाषय LUA_RIDX_चरमः		3


/* type of numbers in Lua */
प्रकारनिर्देश LUA_सङ्ख्या lua_सङ्ख्या;


/* type for integer functions */
प्रकारनिर्देश LUA_INTEGER lua_Integer;

/* unsigned integer type */
प्रकारनिर्देश LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
प्रकारनिर्देश LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
प्रकारनिर्देश पूर्णाङ्क (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
प्रकारनिर्देश पूर्णाङ्क (*lua_KFunction) (lua_State *L, पूर्णाङ्क स्थितिः, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
प्रकारनिर्देश ध्रुव वर्ण * (*lua_Reader) (lua_State *L, शून्य *ud, size_t *sz);

प्रकारनिर्देश पूर्णाङ्क (*lua_Writer) (lua_State *L, ध्रुव शून्य *p, size_t sz, शून्य *ud);


/*
** Type for memory-allocation functions
*/
प्रकारनिर्देश शून्य * (*lua_Alloc) (शून्य *ud, शून्य *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
प्रकारनिर्देश शून्य (*lua_WarnFunction) (शून्य *ud, ध्रुव वर्ण *सन्देशः, पूर्णाङ्क tocont);


/*
** Type used by the debug API to collect debug information
*/
प्रकारनिर्देश संरचना lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
प्रकारनिर्देश शून्य (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#यदि defined(LUA_USER_H)
#अन्तर्भाव LUA_USER_H
#यदि_अन्तः


/*
** RCS ident string
*/
बाह्य ध्रुव वर्ण lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, शून्य *ud, अचिह्नित बीजम्);
LUA_API शून्य       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API पूर्णाङ्क        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_सङ्ख्या (lua_संस्करणम्) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API पूर्णाङ्क   (lua_absindex) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क   (lua_gettop) (lua_State *L);
LUA_API शून्य  (lua_settop) (lua_State *L, पूर्णाङ्क idx);
LUA_API शून्य  (lua_pushvalue) (lua_State *L, पूर्णाङ्क idx);
LUA_API शून्य  (lua_rotate) (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क n);
LUA_API शून्य  (lua_copy) (lua_State *L, पूर्णाङ्क fromidx, पूर्णाङ्क toidx);
LUA_API पूर्णाङ्क   (lua_checkstack) (lua_State *L, पूर्णाङ्क n);

LUA_API शून्य  (lua_xmove) (lua_State *from, lua_State *गन्तव्यम्, पूर्णाङ्क n);


/*
** access functions (stack -> C)
*/

LUA_API पूर्णाङ्क             (lua_isnumber) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क             (lua_isstring) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क             (lua_iscfunction) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क             (lua_isinteger) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क             (lua_isuserdata) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क             (lua_type) (lua_State *L, पूर्णाङ्क idx);
LUA_API ध्रुव वर्ण     *(lua_typename) (lua_State *L, पूर्णाङ्क tp);

LUA_API lua_सङ्ख्या      (lua_tonumberx) (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क *isnum);
LUA_API पूर्णाङ्क             (lua_toboolean) (lua_State *L, पूर्णाङ्क idx);
LUA_API ध्रुव वर्ण     *(lua_tolstring) (lua_State *L, पूर्णाङ्क idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, पूर्णाङ्क idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, पूर्णाङ्क idx);
LUA_API शून्य	       *(lua_touserdata) (lua_State *L, पूर्णाङ्क idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, पूर्णाङ्क idx);
LUA_API ध्रुव शून्य     *(lua_topointer) (lua_State *L, पूर्णाङ्क idx);


/*
** Comparison and arithmetic functions
*/

#परिभाषय LUA_OPADD	0	/* ORDER TM, ORDER OP */
#परिभाषय LUA_OPSUB	1
#परिभाषय LUA_OPMUL	2
#परिभाषय LUA_OPMOD	3
#परिभाषय LUA_OPPOW	4
#परिभाषय LUA_OPDIV	5
#परिभाषय LUA_OPIDIV	6
#परिभाषय LUA_OPBAND	7
#परिभाषय LUA_OPBOR	8
#परिभाषय LUA_OPBXOR	9
#परिभाषय LUA_OPSHL	10
#परिभाषय LUA_OPSHR	11
#परिभाषय LUA_OPUNM	12
#परिभाषय LUA_OPBNOT	13

LUA_API शून्य  (lua_arith) (lua_State *L, पूर्णाङ्क op);

#परिभाषय LUA_OPEQ	0
#परिभाषय LUA_OPLT	1
#परिभाषय LUA_OPLE	2

LUA_API पूर्णाङ्क   (lua_rawequal) (lua_State *L, पूर्णाङ्क idx1, पूर्णाङ्क idx2);
LUA_API पूर्णाङ्क   (lua_compare) (lua_State *L, पूर्णाङ्क idx1, पूर्णाङ्क idx2, पूर्णाङ्क op);


/*
** push functions (C -> stack)
*/
LUA_API शून्य        (lua_pushnil) (lua_State *L);
LUA_API शून्य        (lua_pushnumber) (lua_State *L, lua_सङ्ख्या n);
LUA_API शून्य        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ध्रुव वर्ण *(lua_pushlstring) (lua_State *L, ध्रुव वर्ण *s, size_t len);
LUA_API ध्रुव वर्ण *(lua_pushexternalstring) (lua_State *L,
		ध्रुव वर्ण *s, size_t len, lua_Alloc falloc, शून्य *ud);
LUA_API ध्रुव वर्ण *(lua_pushstring) (lua_State *L, ध्रुव वर्ण *s);
LUA_API ध्रुव वर्ण *(lua_pushvfstring) (lua_State *L, ध्रुव वर्ण *fmt,
                                                      va_list argp);
LUA_API ध्रुव वर्ण *(lua_pushfstring) (lua_State *L, ध्रुव वर्ण *fmt, ...);
LUA_API शून्य  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, पूर्णाङ्क n);
LUA_API शून्य  (lua_pushboolean) (lua_State *L, पूर्णाङ्क b);
LUA_API शून्य  (lua_pushlightuserdata) (lua_State *L, शून्य *p);
LUA_API पूर्णाङ्क   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API पूर्णाङ्क (lua_getglobal) (lua_State *L, ध्रुव वर्ण *name);
LUA_API पूर्णाङ्क (lua_gettable) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क (lua_getfield) (lua_State *L, पूर्णाङ्क idx, ध्रुव वर्ण *k);
LUA_API पूर्णाङ्क (lua_geti) (lua_State *L, पूर्णाङ्क idx, lua_Integer n);
LUA_API पूर्णाङ्क (lua_rawget) (lua_State *L, पूर्णाङ्क idx);
LUA_API पूर्णाङ्क (lua_rawgeti) (lua_State *L, पूर्णाङ्क idx, lua_Integer n);
LUA_API पूर्णाङ्क (lua_rawgetp) (lua_State *L, पूर्णाङ्क idx, ध्रुव शून्य *p);

LUA_API शून्य  (lua_createtable) (lua_State *L, पूर्णाङ्क narr, पूर्णाङ्क nrec);
LUA_API शून्य *(lua_newuserdatauv) (lua_State *L, size_t sz, पूर्णाङ्क nuvalue);
LUA_API पूर्णाङ्क   (lua_getmetatable) (lua_State *L, पूर्णाङ्क objindex);
LUA_API पूर्णाङ्क  (lua_getiuservalue) (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क n);


/*
** set functions (stack -> Lua)
*/
LUA_API शून्य  (lua_setglobal) (lua_State *L, ध्रुव वर्ण *name);
LUA_API शून्य  (lua_settable) (lua_State *L, पूर्णाङ्क idx);
LUA_API शून्य  (lua_setfield) (lua_State *L, पूर्णाङ्क idx, ध्रुव वर्ण *k);
LUA_API शून्य  (lua_seti) (lua_State *L, पूर्णाङ्क idx, lua_Integer n);
LUA_API शून्य  (lua_rawset) (lua_State *L, पूर्णाङ्क idx);
LUA_API शून्य  (lua_rawseti) (lua_State *L, पूर्णाङ्क idx, lua_Integer n);
LUA_API शून्य  (lua_rawsetp) (lua_State *L, पूर्णाङ्क idx, ध्रुव शून्य *p);
LUA_API पूर्णाङ्क   (lua_setmetatable) (lua_State *L, पूर्णाङ्क objindex);
LUA_API पूर्णाङ्क   (lua_setiuservalue) (lua_State *L, पूर्णाङ्क idx, पूर्णाङ्क n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API शून्य  (lua_callk) (lua_State *L, पूर्णाङ्क nargs, पूर्णाङ्क nresults,
                           lua_KContext ctx, lua_KFunction k);
#परिभाषय lua_call(L,n,r)		lua_callk(L, (n), (r), 0, रिक्त)

LUA_API पूर्णाङ्क   (lua_pcallk) (lua_State *L, पूर्णाङ्क nargs, पूर्णाङ्क nresults, पूर्णाङ्क errfunc,
                            lua_KContext ctx, lua_KFunction k);
#परिभाषय lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, रिक्त)

LUA_API पूर्णाङ्क   (lua_आरोपय) (lua_State *L, lua_Reader reader, शून्य *dt,
                          ध्रुव वर्ण *chunkname, ध्रुव वर्ण *बहुलकम्);

LUA_API पूर्णाङ्क (lua_dump) (lua_State *L, lua_Writer writer, शून्य *दत्त, पूर्णाङ्क strip);


/*
** coroutine functions
*/
LUA_API पूर्णाङ्क  (lua_yieldk)     (lua_State *L, पूर्णाङ्क nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API पूर्णाङ्क  (lua_resume)     (lua_State *L, lua_State *from, पूर्णाङ्क narg,
                               पूर्णाङ्क *nres);
LUA_API पूर्णाङ्क  (lua_स्थितिः)     (lua_State *L);
LUA_API पूर्णाङ्क (lua_isyieldable) (lua_State *L);

#परिभाषय lua_yield(L,n)		lua_yieldk(L, (n), 0, रिक्त)


/*
** Warning-related functions
*/
LUA_API शून्य (lua_setwarnf) (lua_State *L, lua_WarnFunction f, शून्य *ud);
LUA_API शून्य (lua_warning)  (lua_State *L, ध्रुव वर्ण *सन्देशः, पूर्णाङ्क tocont);


/*
** garbage-collection options
*/

#परिभाषय LUA_GCSTOP		0
#परिभाषय LUA_GCRESTART		1
#परिभाषय LUA_GCCOLLECT		2
#परिभाषय LUA_GCCOUNT		3
#परिभाषय LUA_GCCOUNTB		4
#परिभाषय LUA_GCSTEP		5
#परिभाषय LUA_GCISRUNNING		6
#परिभाषय LUA_GCGEN		7
#परिभाषय LUA_GCINC		8
#परिभाषय LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#परिभाषय LUA_GCPMINORMUL		0  /* control minor collections */
#परिभाषय LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#परिभाषय LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#परिभाषय LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#परिभाषय LUA_GCPSTEPMUL		4  /* GC "speed" */
#परिभाषय LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#परिभाषय LUA_GCPN		6


LUA_API पूर्णाङ्क (lua_gc) (lua_State *L, पूर्णाङ्क what, ...);


/*
** miscellaneous functions
*/

LUA_API पूर्णाङ्क   (lua_error) (lua_State *L);

LUA_API पूर्णाङ्क   (lua_अग्र) (lua_State *L, पूर्णाङ्क idx);

LUA_API शून्य  (lua_concat) (lua_State *L, पूर्णाङ्क n);
LUA_API शून्य  (lua_len)    (lua_State *L, पूर्णाङ्क idx);

#परिभाषय LUA_N2SBUFFSZ	64
LUA_API अचिह्नित  (lua_numbertocstring) (lua_State *L, पूर्णाङ्क idx, वर्ण *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ध्रुव वर्ण *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, शून्य **ud);
LUA_API शून्य      (lua_setallocf) (lua_State *L, lua_Alloc f, शून्य *ud);

LUA_API शून्य (lua_toclose) (lua_State *L, पूर्णाङ्क idx);
LUA_API शून्य (lua_closeslot) (lua_State *L, पूर्णाङ्क idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#परिभाषय lua_getextraspace(L)	((शून्य *)((वर्ण *)(L) - LUA_EXTRASPACE))

#परिभाषय lua_tonumber(L,i)	lua_tonumberx(L,(i),रिक्त)
#परिभाषय lua_tointeger(L,i)	lua_tointegerx(L,(i),रिक्त)

#परिभाषय lua_निष्कासय(L,n)		lua_settop(L, -(n)-1)

#परिभाषय lua_newtable(L)		lua_createtable(L, 0, 0)

#परिभाषय lua_पञ्जिकाकोशः(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#परिभाषय lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#परिभाषय lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#परिभाषय lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#परिभाषय lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#परिभाषय lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#परिभाषय lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#परिभाषय lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#परिभाषय lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#परिभाषय lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#परिभाषय lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#परिभाषय lua_pushglobaltable(L)  \
	((शून्य)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#परिभाषय lua_tostring(L,i)	lua_tolstring(L, (i), रिक्त)


#परिभाषय lua_निवेशय(L,idx)	lua_rotate(L, (idx), 1)

#परिभाषय lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_निष्कासय(L, 1))

#परिभाषय lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_निष्कासय(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#परिभाषय lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#परिभाषय lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#परिभाषय lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#परिभाषय lua_resetthread(L)	lua_closethread(L,रिक्त)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#परिभाषय LUA_HOOKCALL	0
#परिभाषय LUA_HOOKRET	1
#परिभाषय LUA_HOOKLINE	2
#परिभाषय LUA_HOOKCOUNT	3
#परिभाषय LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#परिभाषय LUA_MASKCALL	(1 << LUA_HOOKCALL)
#परिभाषय LUA_MASKRET	(1 << LUA_HOOKRET)
#परिभाषय LUA_MASKLINE	(1 << LUA_HOOKLINE)
#परिभाषय LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API पूर्णाङ्क (lua_getstack) (lua_State *L, पूर्णाङ्क level, lua_Debug *ar);
LUA_API पूर्णाङ्क (lua_getinfo) (lua_State *L, ध्रुव वर्ण *what, lua_Debug *ar);
LUA_API ध्रुव वर्ण *(lua_getlocal) (lua_State *L, ध्रुव lua_Debug *ar, पूर्णाङ्क n);
LUA_API ध्रुव वर्ण *(lua_setlocal) (lua_State *L, ध्रुव lua_Debug *ar, पूर्णाङ्क n);
LUA_API ध्रुव वर्ण *(lua_getupvalue) (lua_State *L, पूर्णाङ्क funcindex, पूर्णाङ्क n);
LUA_API ध्रुव वर्ण *(lua_setupvalue) (lua_State *L, पूर्णाङ्क funcindex, पूर्णाङ्क n);

LUA_API शून्य *(lua_upvalueid) (lua_State *L, पूर्णाङ्क fidx, पूर्णाङ्क n);
LUA_API शून्य  (lua_upvaluejoin) (lua_State *L, पूर्णाङ्क fidx1, पूर्णाङ्क n1,
                                               पूर्णाङ्क fidx2, पूर्णाङ्क n2);

LUA_API शून्य (lua_sethook) (lua_State *L, lua_Hook func, पूर्णाङ्क आवरणम्_2, पूर्णाङ्क गणनम्);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API पूर्णाङ्क (lua_gethookmask) (lua_State *L);
LUA_API पूर्णाङ्क (lua_gethookcount) (lua_State *L);


संरचना lua_Debug {
  पूर्णाङ्क event;
  ध्रुव वर्ण *name;	/* (n) */
  ध्रुव वर्ण *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ध्रुव वर्ण *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ध्रुव वर्ण *source;	/* (S) */
  size_t srclen;	/* (S) */
  पूर्णाङ्क currentline;	/* (l) */
  पूर्णाङ्क linedefined;	/* (S) */
  पूर्णाङ्क lastlinedefined;	/* (S) */
  अचिह्नित वर्ण nups;	/* (u) number of upvalues */
  अचिह्नित वर्ण nparams;/* (u) number of parameters */
  वर्ण isvararg;        /* (u) */
  अचिह्नित वर्ण extraargs;  /* (t) number of extra arguments */
  वर्ण istailcall;	/* (t) */
  पूर्णाङ्क ftransfer;   /* (r) index of first value transferred */
  पूर्णाङ्क ntransfer;   /* (r) number of transferred values */
  वर्ण short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  संरचना CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#परिभाषय LUAI_TOSTRAUX(x)	#x
#परिभाषय LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#परिभाषय LUA_संस्करणम्_MAJOR	LUAI_TOSTR(LUA_संस्करणम्_MAJOR_N)
#परिभाषय LUA_संस्करणम्_MINOR	LUAI_TOSTR(LUA_संस्करणम्_MINOR_N)
#परिभाषय LUA_संस्करणम्_RELEASE	LUAI_TOSTR(LUA_संस्करणम्_RELEASE_N)

#परिभाषय LUA_संस्करणम्	"Lua " LUA_संस्करणम्_MAJOR "." LUA_संस्करणम्_MINOR
#परिभाषय LUA_RELEASE	LUA_संस्करणम् "." LUA_संस्करणम्_RELEASE


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


#यदि_अन्तः
