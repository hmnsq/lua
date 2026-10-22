/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#अगर_अघोषित lua_h
#घोषणा lua_h

#समावेश <stdarg.h>
#समावेश <stddef.h>


#घोषणा LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#घोषणा LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#घोषणा LUA_VERSION_MAJOR_N	5
#घोषणा LUA_VERSION_MINOR_N	5
#घोषणा LUA_VERSION_RELEASE_N	1

#घोषणा LUA_VERSION_अंक  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#घोषणा LUA_VERSION_RELEASE_अंक  (LUA_VERSION_अंक * 100 + LUA_VERSION_RELEASE_N)


#समावेश "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#घोषणा LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#घोषणा LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#घोषणा LUA_REGISTRYINDEX	(-(पूर्णांक_उच्च/2 + 1000))
#घोषणा lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#घोषणा LUA_OK		0
#घोषणा LUA_YIELD	1
#घोषणा LUA_ERRRUN	2
#घोषणा LUA_ERRSYNTAX	3
#घोषणा LUA_ERRMEM	4
#घोषणा LUA_ERRERR	5


प्रकार काष्ठा lua_State lua_State;


/*
** basic types
*/
#घोषणा LUA_TNONE		(-1)

#घोषणा LUA_TNIL		0
#घोषणा LUA_TBOOLEAN		1
#घोषणा LUA_TLIGHTUSERDATA	2
#घोषणा LUA_TNUMBER		3
#घोषणा LUA_TSTRING		4
#घोषणा LUA_TTABLE		5
#घोषणा LUA_TFUNCTION		6
#घोषणा LUA_TUSERDATA		7
#घोषणा LUA_TTHREAD		8

#घोषणा LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#घोषणा LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#घोषणा LUA_RIDX_GLOBALS	2
#घोषणा LUA_RIDX_MAINTHREAD	3
#घोषणा LUA_RIDX_LAST		3


/* type of numbers in Lua */
प्रकार LUA_संख्या lua_संख्या;


/* type for integer functions */
प्रकार LUA_INTEGER lua_Integer;

/* unsigned integer type */
प्रकार LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
प्रकार LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
प्रकार पूर्णांक (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
प्रकार पूर्णांक (*lua_KFunction) (lua_State *L, पूर्णांक status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
प्रकार स्थिर अक्षर * (*lua_Reader) (lua_State *L, व्योम *ud, माप_प्रकार *sz);

प्रकार पूर्णांक (*lua_Writer) (lua_State *L, स्थिर व्योम *p, माप_प्रकार sz, व्योम *ud);


/*
** Type for memory-allocation functions
*/
प्रकार व्योम * (*lua_Alloc) (व्योम *ud, व्योम *ptr, माप_प्रकार osize, माप_प्रकार nsize);


/*
** Type for warning functions
*/
प्रकार व्योम (*lua_WarnFunction) (व्योम *ud, स्थिर अक्षर *संदेश, पूर्णांक tocont);


/*
** Type used by the debug API to collect debug information
*/
प्रकार काष्ठा lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
प्रकार व्योम (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#अगर defined(LUA_USER_H)
#समावेश LUA_USER_H
#पूर्ण_अगर


/*
** RCS ident string
*/
बाह्य स्थिर अक्षर lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, व्योम *ud, अचिन्हित seed);
LUA_API व्योम       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API पूर्णांक        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_संख्या (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API पूर्णांक   (lua_absindex) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक   (lua_gettop) (lua_State *L);
LUA_API व्योम  (lua_settop) (lua_State *L, पूर्णांक idx);
LUA_API व्योम  (lua_pushvalue) (lua_State *L, पूर्णांक idx);
LUA_API व्योम  (lua_rotate) (lua_State *L, पूर्णांक idx, पूर्णांक n);
LUA_API व्योम  (lua_copy) (lua_State *L, पूर्णांक fromidx, पूर्णांक toidx);
LUA_API पूर्णांक   (lua_checkstack) (lua_State *L, पूर्णांक n);

LUA_API व्योम  (lua_xmove) (lua_State *from, lua_State *to, पूर्णांक n);


/*
** access functions (stack -> C)
*/

LUA_API पूर्णांक             (lua_isnumber) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक             (lua_isstring) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक             (lua_iscfunction) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक             (lua_isinteger) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक             (lua_isuserdata) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक             (lua_type) (lua_State *L, पूर्णांक idx);
LUA_API स्थिर अक्षर     *(lua_typename) (lua_State *L, पूर्णांक tp);

LUA_API lua_संख्या      (lua_tonumberx) (lua_State *L, पूर्णांक idx, पूर्णांक *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, पूर्णांक idx, पूर्णांक *isnum);
LUA_API पूर्णांक             (lua_toboolean) (lua_State *L, पूर्णांक idx);
LUA_API स्थिर अक्षर     *(lua_tolstring) (lua_State *L, पूर्णांक idx, माप_प्रकार *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, पूर्णांक idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, पूर्णांक idx);
LUA_API व्योम	       *(lua_touserdata) (lua_State *L, पूर्णांक idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, पूर्णांक idx);
LUA_API स्थिर व्योम     *(lua_topointer) (lua_State *L, पूर्णांक idx);


/*
** Comparison and arithmetic functions
*/

#घोषणा LUA_OPADD	0	/* ORDER TM, ORDER OP */
#घोषणा LUA_OPSUB	1
#घोषणा LUA_OPMUL	2
#घोषणा LUA_OPMOD	3
#घोषणा LUA_OPPOW	4
#घोषणा LUA_OPDIV	5
#घोषणा LUA_OPIDIV	6
#घोषणा LUA_OPBAND	7
#घोषणा LUA_OPBOR	8
#घोषणा LUA_OPBXOR	9
#घोषणा LUA_OPSHL	10
#घोषणा LUA_OPSHR	11
#घोषणा LUA_OPUNM	12
#घोषणा LUA_OPBNOT	13

LUA_API व्योम  (lua_arith) (lua_State *L, पूर्णांक op);

#घोषणा LUA_OPEQ	0
#घोषणा LUA_OPLT	1
#घोषणा LUA_OPLE	2

LUA_API पूर्णांक   (lua_rawequal) (lua_State *L, पूर्णांक idx1, पूर्णांक idx2);
LUA_API पूर्णांक   (lua_compare) (lua_State *L, पूर्णांक idx1, पूर्णांक idx2, पूर्णांक op);


/*
** push functions (C -> stack)
*/
LUA_API व्योम        (lua_pushnil) (lua_State *L);
LUA_API व्योम        (lua_pushnumber) (lua_State *L, lua_संख्या n);
LUA_API व्योम        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API स्थिर अक्षर *(lua_pushlstring) (lua_State *L, स्थिर अक्षर *s, माप_प्रकार len);
LUA_API स्थिर अक्षर *(lua_pushexternalstring) (lua_State *L,
		स्थिर अक्षर *s, माप_प्रकार len, lua_Alloc falloc, व्योम *ud);
LUA_API स्थिर अक्षर *(lua_pushstring) (lua_State *L, स्थिर अक्षर *s);
LUA_API स्थिर अक्षर *(lua_pushvfstring) (lua_State *L, स्थिर अक्षर *fmt,
                                                      बहु_सूची argp);
LUA_API स्थिर अक्षर *(lua_pushfstring) (lua_State *L, स्थिर अक्षर *fmt, ...);
LUA_API व्योम  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, पूर्णांक n);
LUA_API व्योम  (lua_pushboolean) (lua_State *L, पूर्णांक b);
LUA_API व्योम  (lua_pushlightuserdata) (lua_State *L, व्योम *p);
LUA_API पूर्णांक   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API पूर्णांक (lua_getglobal) (lua_State *L, स्थिर अक्षर *name);
LUA_API पूर्णांक (lua_gettable) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक (lua_getfield) (lua_State *L, पूर्णांक idx, स्थिर अक्षर *k);
LUA_API पूर्णांक (lua_geti) (lua_State *L, पूर्णांक idx, lua_Integer n);
LUA_API पूर्णांक (lua_rawget) (lua_State *L, पूर्णांक idx);
LUA_API पूर्णांक (lua_rawgeti) (lua_State *L, पूर्णांक idx, lua_Integer n);
LUA_API पूर्णांक (lua_rawgetp) (lua_State *L, पूर्णांक idx, स्थिर व्योम *p);

LUA_API व्योम  (lua_createtable) (lua_State *L, पूर्णांक narr, पूर्णांक nrec);
LUA_API व्योम *(lua_newuserdatauv) (lua_State *L, माप_प्रकार sz, पूर्णांक nuvalue);
LUA_API पूर्णांक   (lua_getmetatable) (lua_State *L, पूर्णांक objindex);
LUA_API पूर्णांक  (lua_getiuservalue) (lua_State *L, पूर्णांक idx, पूर्णांक n);


/*
** set functions (stack -> Lua)
*/
LUA_API व्योम  (lua_setglobal) (lua_State *L, स्थिर अक्षर *name);
LUA_API व्योम  (lua_settable) (lua_State *L, पूर्णांक idx);
LUA_API व्योम  (lua_setfield) (lua_State *L, पूर्णांक idx, स्थिर अक्षर *k);
LUA_API व्योम  (lua_seti) (lua_State *L, पूर्णांक idx, lua_Integer n);
LUA_API व्योम  (lua_rawset) (lua_State *L, पूर्णांक idx);
LUA_API व्योम  (lua_rawseti) (lua_State *L, पूर्णांक idx, lua_Integer n);
LUA_API व्योम  (lua_rawsetp) (lua_State *L, पूर्णांक idx, स्थिर व्योम *p);
LUA_API पूर्णांक   (lua_setmetatable) (lua_State *L, पूर्णांक objindex);
LUA_API पूर्णांक   (lua_setiuservalue) (lua_State *L, पूर्णांक idx, पूर्णांक n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API व्योम  (lua_callk) (lua_State *L, पूर्णांक nargs, पूर्णांक nresults,
                           lua_KContext ctx, lua_KFunction k);
#घोषणा lua_call(L,n,r)		lua_callk(L, (n), (r), 0, शून्य)

LUA_API पूर्णांक   (lua_pcallk) (lua_State *L, पूर्णांक nargs, पूर्णांक nresults, पूर्णांक errfunc,
                            lua_KContext ctx, lua_KFunction k);
#घोषणा lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, शून्य)

LUA_API पूर्णांक   (lua_load) (lua_State *L, lua_Reader reader, व्योम *dt,
                          स्थिर अक्षर *chunkname, स्थिर अक्षर *mode);

LUA_API पूर्णांक (lua_dump) (lua_State *L, lua_Writer writer, व्योम *तथ्य, पूर्णांक strip);


/*
** coroutine functions
*/
LUA_API पूर्णांक  (lua_yieldk)     (lua_State *L, पूर्णांक nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API पूर्णांक  (lua_resume)     (lua_State *L, lua_State *from, पूर्णांक narg,
                               पूर्णांक *nres);
LUA_API पूर्णांक  (lua_status)     (lua_State *L);
LUA_API पूर्णांक (lua_isyieldable) (lua_State *L);

#घोषणा lua_yield(L,n)		lua_yieldk(L, (n), 0, शून्य)


/*
** Warning-related functions
*/
LUA_API व्योम (lua_setwarnf) (lua_State *L, lua_WarnFunction f, व्योम *ud);
LUA_API व्योम (lua_warning)  (lua_State *L, स्थिर अक्षर *संदेश, पूर्णांक tocont);


/*
** garbage-collection options
*/

#घोषणा LUA_GCSTOP		0
#घोषणा LUA_GCRESTART		1
#घोषणा LUA_GCCOLLECT		2
#घोषणा LUA_GCCOUNT		3
#घोषणा LUA_GCCOUNTB		4
#घोषणा LUA_GCSTEP		5
#घोषणा LUA_GCISRUNNING		6
#घोषणा LUA_GCGEN		7
#घोषणा LUA_GCINC		8
#घोषणा LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#घोषणा LUA_GCPMINORMUL		0  /* control minor collections */
#घोषणा LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#घोषणा LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#घोषणा LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#घोषणा LUA_GCPSTEPMUL		4  /* GC "speed" */
#घोषणा LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#घोषणा LUA_GCPN		6


LUA_API पूर्णांक (lua_gc) (lua_State *L, पूर्णांक what, ...);


/*
** miscellaneous functions
*/

LUA_API पूर्णांक   (lua_error) (lua_State *L);

LUA_API पूर्णांक   (lua_अग्र) (lua_State *L, पूर्णांक idx);

LUA_API व्योम  (lua_concat) (lua_State *L, पूर्णांक n);
LUA_API व्योम  (lua_len)    (lua_State *L, पूर्णांक idx);

#घोषणा LUA_N2SBUFFSZ	64
LUA_API अचिन्हित  (lua_numbertocstring) (lua_State *L, पूर्णांक idx, अक्षर *buff);
LUA_API माप_प्रकार  (lua_stringtonumber) (lua_State *L, स्थिर अक्षर *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, व्योम **ud);
LUA_API व्योम      (lua_setallocf) (lua_State *L, lua_Alloc f, व्योम *ud);

LUA_API व्योम (lua_toclose) (lua_State *L, पूर्णांक idx);
LUA_API व्योम (lua_closeslot) (lua_State *L, पूर्णांक idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#घोषणा lua_getextraspace(L)	((व्योम *)((अक्षर *)(L) - LUA_EXTRASPACE))

#घोषणा lua_tonumber(L,i)	lua_tonumberx(L,(i),शून्य)
#घोषणा lua_tointeger(L,i)	lua_tointegerx(L,(i),शून्य)

#घोषणा lua_निकालो(L,n)		lua_settop(L, -(n)-1)

#घोषणा lua_newtable(L)		lua_createtable(L, 0, 0)

#घोषणा lua_register(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#घोषणा lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#घोषणा lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#घोषणा lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#घोषणा lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#घोषणा lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#घोषणा lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#घोषणा lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#घोषणा lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#घोषणा lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#घोषणा lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#घोषणा lua_pushglobaltable(L)  \
	((व्योम)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#घोषणा lua_tostring(L,i)	lua_tolstring(L, (i), शून्य)


#घोषणा lua_जोड़ो(L,idx)	lua_rotate(L, (idx), 1)

#घोषणा lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_निकालो(L, 1))

#घोषणा lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_निकालो(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#घोषणा lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#घोषणा lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#घोषणा lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#घोषणा lua_resetthread(L)	lua_closethread(L,शून्य)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#घोषणा LUA_HOOKCALL	0
#घोषणा LUA_HOOKRET	1
#घोषणा LUA_HOOKLINE	2
#घोषणा LUA_HOOKCOUNT	3
#घोषणा LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#घोषणा LUA_MASKCALL	(1 << LUA_HOOKCALL)
#घोषणा LUA_MASKRET	(1 << LUA_HOOKRET)
#घोषणा LUA_MASKLINE	(1 << LUA_HOOKLINE)
#घोषणा LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API पूर्णांक (lua_getstack) (lua_State *L, पूर्णांक level, lua_Debug *ar);
LUA_API पूर्णांक (lua_getinfo) (lua_State *L, स्थिर अक्षर *what, lua_Debug *ar);
LUA_API स्थिर अक्षर *(lua_getlocal) (lua_State *L, स्थिर lua_Debug *ar, पूर्णांक n);
LUA_API स्थिर अक्षर *(lua_setlocal) (lua_State *L, स्थिर lua_Debug *ar, पूर्णांक n);
LUA_API स्थिर अक्षर *(lua_getupvalue) (lua_State *L, पूर्णांक funcindex, पूर्णांक n);
LUA_API स्थिर अक्षर *(lua_setupvalue) (lua_State *L, पूर्णांक funcindex, पूर्णांक n);

LUA_API व्योम *(lua_upvalueid) (lua_State *L, पूर्णांक fidx, पूर्णांक n);
LUA_API व्योम  (lua_upvaluejoin) (lua_State *L, पूर्णांक fidx1, पूर्णांक n1,
                                               पूर्णांक fidx2, पूर्णांक n2);

LUA_API व्योम (lua_sethook) (lua_State *L, lua_Hook func, पूर्णांक mask, पूर्णांक गिनती);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API पूर्णांक (lua_gethookmask) (lua_State *L);
LUA_API पूर्णांक (lua_gethookcount) (lua_State *L);


काष्ठा lua_Debug {
  पूर्णांक event;
  स्थिर अक्षर *name;	/* (n) */
  स्थिर अक्षर *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  स्थिर अक्षर *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  स्थिर अक्षर *source;	/* (S) */
  माप_प्रकार srclen;	/* (S) */
  पूर्णांक currentline;	/* (l) */
  पूर्णांक linedefined;	/* (S) */
  पूर्णांक lastlinedefined;	/* (S) */
  अचिन्हित अक्षर nups;	/* (u) number of upvalues */
  अचिन्हित अक्षर nparams;/* (u) number of parameters */
  अक्षर isvararg;        /* (u) */
  अचिन्हित अक्षर extraargs;  /* (t) number of extra arguments */
  अक्षर istailcall;	/* (t) */
  पूर्णांक ftransfer;   /* (r) index of first value transferred */
  पूर्णांक ntransfer;   /* (r) number of transferred values */
  अक्षर short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  काष्ठा CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#घोषणा LUAI_TOSTRAUX(x)	#x
#घोषणा LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#घोषणा LUA_VERSION_MAJOR	LUAI_TOSTR(LUA_VERSION_MAJOR_N)
#घोषणा LUA_VERSION_MINOR	LUAI_TOSTR(LUA_VERSION_MINOR_N)
#घोषणा LUA_VERSION_RELEASE	LUAI_TOSTR(LUA_VERSION_RELEASE_N)

#घोषणा LUA_VERSION	"Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#घोषणा LUA_RELEASE	LUA_VERSION "." LUA_VERSION_RELEASE


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


#पूर्ण_अगर
