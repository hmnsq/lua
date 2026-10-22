/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stddef.h>


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	1

#define LUA_VERSION_अङ्कः  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_अङ्कः  (LUA_VERSION_अङ्कः * 100 + LUA_VERSION_RELEASE_N)


#अन्तर्भाव "luaconf.h"


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


प्रकारनिर्देश संरचना lua_State lua_State;


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
प्रकारनिर्देश पूर्णाङ्क (*lua_KFunction) (lua_State *L, पूर्णाङ्क status, lua_KContext ctx);


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
#जइ defined(LUA_USER_H)
#अन्तर्भाव LUA_USER_H
#endif


/*
** RCS ident string
*/
बाह्य ध्रुव वर्ण lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, शून्य *ud, unsigned seed);
LUA_API शून्य       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API पूर्णाङ्क        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_सङ्ख्या (lua_version) (lua_State *L);


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

LUA_API शून्य  (lua_xmove) (lua_State *from, lua_State *to, पूर्णाङ्क n);


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

LUA_API शून्य  (lua_arith) (lua_State *L, पूर्णाङ्क op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

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
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, रिक्त)

LUA_API पूर्णाङ्क   (lua_pcallk) (lua_State *L, पूर्णाङ्क nargs, पूर्णाङ्क nresults, पूर्णाङ्क errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, रिक्त)

LUA_API पूर्णाङ्क   (lua_load) (lua_State *L, lua_Reader reader, शून्य *dt,
                          ध्रुव वर्ण *chunkname, ध्रुव वर्ण *mode);

LUA_API पूर्णाङ्क (lua_dump) (lua_State *L, lua_Writer writer, शून्य *data, पूर्णाङ्क strip);


/*
** coroutine functions
*/
LUA_API पूर्णाङ्क  (lua_yieldk)     (lua_State *L, पूर्णाङ्क nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API पूर्णाङ्क  (lua_resume)     (lua_State *L, lua_State *from, पूर्णाङ्क narg,
                               पूर्णाङ्क *nres);
LUA_API पूर्णाङ्क  (lua_status)     (lua_State *L);
LUA_API पूर्णाङ्क (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, रिक्त)


/*
** Warning-related functions
*/
LUA_API शून्य (lua_setwarnf) (lua_State *L, lua_WarnFunction f, शून्य *ud);
LUA_API शून्य (lua_warning)  (lua_State *L, ध्रुव वर्ण *सन्देशः, पूर्णाङ्क tocont);


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


LUA_API पूर्णाङ्क (lua_gc) (lua_State *L, पूर्णाङ्क what, ...);


/*
** miscellaneous functions
*/

LUA_API पूर्णाङ्क   (lua_error) (lua_State *L);

LUA_API पूर्णाङ्क   (lua_next) (lua_State *L, पूर्णाङ्क idx);

LUA_API शून्य  (lua_concat) (lua_State *L, पूर्णाङ्क n);
LUA_API शून्य  (lua_len)    (lua_State *L, पूर्णाङ्क idx);

#define LUA_N2SBUFFSZ	64
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, पूर्णाङ्क idx, वर्ण *buff);
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

#define lua_getextraspace(L)	((शून्य *)((वर्ण *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),रिक्त)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),रिक्त)

#define lua_निष्कासय(L,n)		lua_settop(L, -(n)-1)

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
	((शून्य)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), रिक्त)


#define lua_निवेशय(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_निष्कासय(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_निष्कासय(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,रिक्त)

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


LUA_API पूर्णाङ्क (lua_getstack) (lua_State *L, पूर्णाङ्क level, lua_Debug *ar);
LUA_API पूर्णाङ्क (lua_getinfo) (lua_State *L, ध्रुव वर्ण *what, lua_Debug *ar);
LUA_API ध्रुव वर्ण *(lua_getlocal) (lua_State *L, ध्रुव lua_Debug *ar, पूर्णाङ्क n);
LUA_API ध्रुव वर्ण *(lua_setlocal) (lua_State *L, ध्रुव lua_Debug *ar, पूर्णाङ्क n);
LUA_API ध्रुव वर्ण *(lua_getupvalue) (lua_State *L, पूर्णाङ्क funcindex, पूर्णाङ्क n);
LUA_API ध्रुव वर्ण *(lua_setupvalue) (lua_State *L, पूर्णाङ्क funcindex, पूर्णाङ्क n);

LUA_API शून्य *(lua_upvalueid) (lua_State *L, पूर्णाङ्क fidx, पूर्णाङ्क n);
LUA_API शून्य  (lua_upvaluejoin) (lua_State *L, पूर्णाङ्क fidx1, पूर्णाङ्क n1,
                                               पूर्णाङ्क fidx2, पूर्णाङ्क n2);

LUA_API शून्य (lua_sethook) (lua_State *L, lua_Hook func, पूर्णाङ्क mask, पूर्णाङ्क गणनम्);
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
  unsigned वर्ण nups;	/* (u) number of upvalues */
  unsigned वर्ण nparams;/* (u) number of parameters */
  वर्ण isvararg;        /* (u) */
  unsigned वर्ण extraargs;  /* (t) number of extra arguments */
  वर्ण istailcall;	/* (t) */
  पूर्णाङ्क ftransfer;   /* (r) index of first value transferred */
  पूर्णाङ्क ntransfer;   /* (r) number of transferred values */
  वर्ण short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  संरचना CallInfo *i_ci;  /* active function */
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
