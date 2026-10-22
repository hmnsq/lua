/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#شامل <stdarg.h>
#شامل <stddef.h>


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	1

#define LUA_VERSION_رقم  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_رقم  (LUA_VERSION_رقم * 100 + LUA_VERSION_RELEASE_N)


#شامل "luaconf.h"


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


تعریفنوع ساختار lua_State lua_State;


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
تعریفنوع LUA_عدد lua_عدد;


/* type for integer functions */
تعریفنوع LUA_INTEGER lua_Integer;

/* unsigned integer type */
تعریفنوع LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
تعریفنوع LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
تعریفنوع صحیح (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
تعریفنوع صحیح (*lua_KFunction) (lua_State *L, صحیح status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
تعریفنوع ثابت نویسه * (*lua_Reader) (lua_State *L, تهی *ud, size_t *sz);

تعریفنوع صحیح (*lua_Writer) (lua_State *L, ثابت تهی *p, size_t sz, تهی *ud);


/*
** Type for memory-allocation functions
*/
تعریفنوع تهی * (*lua_Alloc) (تهی *ud, تهی *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
تعریفنوع تهی (*lua_WarnFunction) (تهی *ud, ثابت نویسه *پیام, صحیح tocont);


/*
** Type used by the debug API to collect debug information
*/
تعریفنوع ساختار lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
تعریفنوع تهی (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#اگر defined(LUA_USER_H)
#شامل LUA_USER_H
#endif


/*
** RCS ident string
*/
بیرونی ثابت نویسه lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, تهی *ud, unsigned seed);
LUA_API تهی       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API صحیح        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_عدد (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API صحیح   (lua_absindex) (lua_State *L, صحیح idx);
LUA_API صحیح   (lua_gettop) (lua_State *L);
LUA_API تهی  (lua_settop) (lua_State *L, صحیح idx);
LUA_API تهی  (lua_pushvalue) (lua_State *L, صحیح idx);
LUA_API تهی  (lua_rotate) (lua_State *L, صحیح idx, صحیح n);
LUA_API تهی  (lua_copy) (lua_State *L, صحیح fromidx, صحیح toidx);
LUA_API صحیح   (lua_checkstack) (lua_State *L, صحیح n);

LUA_API تهی  (lua_xmove) (lua_State *from, lua_State *to, صحیح n);


/*
** access functions (stack -> C)
*/

LUA_API صحیح             (lua_isnumber) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_isstring) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_iscfunction) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_isinteger) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_isuserdata) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_type) (lua_State *L, صحیح idx);
LUA_API ثابت نویسه     *(lua_typename) (lua_State *L, صحیح tp);

LUA_API lua_عدد      (lua_tonumberx) (lua_State *L, صحیح idx, صحیح *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, صحیح idx, صحیح *isnum);
LUA_API صحیح             (lua_toboolean) (lua_State *L, صحیح idx);
LUA_API ثابت نویسه     *(lua_tolstring) (lua_State *L, صحیح idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, صحیح idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, صحیح idx);
LUA_API تهی	       *(lua_touserdata) (lua_State *L, صحیح idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, صحیح idx);
LUA_API ثابت تهی     *(lua_topointer) (lua_State *L, صحیح idx);


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

LUA_API تهی  (lua_arith) (lua_State *L, صحیح op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API صحیح   (lua_rawequal) (lua_State *L, صحیح idx1, صحیح idx2);
LUA_API صحیح   (lua_compare) (lua_State *L, صحیح idx1, صحیح idx2, صحیح op);


/*
** push functions (C -> stack)
*/
LUA_API تهی        (lua_pushnil) (lua_State *L);
LUA_API تهی        (lua_pushnumber) (lua_State *L, lua_عدد n);
LUA_API تهی        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ثابت نویسه *(lua_pushlstring) (lua_State *L, ثابت نویسه *s, size_t len);
LUA_API ثابت نویسه *(lua_pushexternalstring) (lua_State *L,
		ثابت نویسه *s, size_t len, lua_Alloc falloc, تهی *ud);
LUA_API ثابت نویسه *(lua_pushstring) (lua_State *L, ثابت نویسه *s);
LUA_API ثابت نویسه *(lua_pushvfstring) (lua_State *L, ثابت نویسه *fmt,
                                                      va_list argp);
LUA_API ثابت نویسه *(lua_pushfstring) (lua_State *L, ثابت نویسه *fmt, ...);
LUA_API تهی  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, صحیح n);
LUA_API تهی  (lua_pushboolean) (lua_State *L, صحیح b);
LUA_API تهی  (lua_pushlightuserdata) (lua_State *L, تهی *p);
LUA_API صحیح   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API صحیح (lua_getglobal) (lua_State *L, ثابت نویسه *name);
LUA_API صحیح (lua_gettable) (lua_State *L, صحیح idx);
LUA_API صحیح (lua_getfield) (lua_State *L, صحیح idx, ثابت نویسه *k);
LUA_API صحیح (lua_geti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API صحیح (lua_rawget) (lua_State *L, صحیح idx);
LUA_API صحیح (lua_rawgeti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API صحیح (lua_rawgetp) (lua_State *L, صحیح idx, ثابت تهی *p);

LUA_API تهی  (lua_createtable) (lua_State *L, صحیح narr, صحیح nrec);
LUA_API تهی *(lua_newuserdatauv) (lua_State *L, size_t sz, صحیح nuvalue);
LUA_API صحیح   (lua_getmetatable) (lua_State *L, صحیح objindex);
LUA_API صحیح  (lua_getiuservalue) (lua_State *L, صحیح idx, صحیح n);


/*
** set functions (stack -> Lua)
*/
LUA_API تهی  (lua_setglobal) (lua_State *L, ثابت نویسه *name);
LUA_API تهی  (lua_settable) (lua_State *L, صحیح idx);
LUA_API تهی  (lua_setfield) (lua_State *L, صحیح idx, ثابت نویسه *k);
LUA_API تهی  (lua_seti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API تهی  (lua_rawset) (lua_State *L, صحیح idx);
LUA_API تهی  (lua_rawseti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API تهی  (lua_rawsetp) (lua_State *L, صحیح idx, ثابت تهی *p);
LUA_API صحیح   (lua_setmetatable) (lua_State *L, صحیح objindex);
LUA_API صحیح   (lua_setiuservalue) (lua_State *L, صحیح idx, صحیح n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API تهی  (lua_callk) (lua_State *L, صحیح nargs, صحیح nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, پوچ)

LUA_API صحیح   (lua_pcallk) (lua_State *L, صحیح nargs, صحیح nresults, صحیح errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, پوچ)

LUA_API صحیح   (lua_load) (lua_State *L, lua_Reader reader, تهی *dt,
                          ثابت نویسه *chunkname, ثابت نویسه *mode);

LUA_API صحیح (lua_dump) (lua_State *L, lua_Writer writer, تهی *data, صحیح strip);


/*
** coroutine functions
*/
LUA_API صحیح  (lua_yieldk)     (lua_State *L, صحیح nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API صحیح  (lua_resume)     (lua_State *L, lua_State *from, صحیح narg,
                               صحیح *nres);
LUA_API صحیح  (lua_status)     (lua_State *L);
LUA_API صحیح (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, پوچ)


/*
** Warning-related functions
*/
LUA_API تهی (lua_setwarnf) (lua_State *L, lua_WarnFunction f, تهی *ud);
LUA_API تهی (lua_warning)  (lua_State *L, ثابت نویسه *پیام, صحیح tocont);


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


LUA_API صحیح (lua_gc) (lua_State *L, صحیح what, ...);


/*
** miscellaneous functions
*/

LUA_API صحیح   (lua_error) (lua_State *L);

LUA_API صحیح   (lua_next) (lua_State *L, صحیح idx);

LUA_API تهی  (lua_concat) (lua_State *L, صحیح n);
LUA_API تهی  (lua_len)    (lua_State *L, صحیح idx);

#define LUA_N2SBUFFSZ	64
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, صحیح idx, نویسه *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ثابت نویسه *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, تهی **ud);
LUA_API تهی      (lua_setallocf) (lua_State *L, lua_Alloc f, تهی *ud);

LUA_API تهی (lua_toclose) (lua_State *L, صحیح idx);
LUA_API تهی (lua_closeslot) (lua_State *L, صحیح idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((تهی *)((نویسه *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),پوچ)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),پوچ)

#define lua_برداشت(L,n)		lua_settop(L, -(n)-1)

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
	((تهی)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), پوچ)


#define lua_درج(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_برداشت(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_برداشت(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,پوچ)

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


LUA_API صحیح (lua_getstack) (lua_State *L, صحیح level, lua_Debug *ar);
LUA_API صحیح (lua_getinfo) (lua_State *L, ثابت نویسه *what, lua_Debug *ar);
LUA_API ثابت نویسه *(lua_getlocal) (lua_State *L, ثابت lua_Debug *ar, صحیح n);
LUA_API ثابت نویسه *(lua_setlocal) (lua_State *L, ثابت lua_Debug *ar, صحیح n);
LUA_API ثابت نویسه *(lua_getupvalue) (lua_State *L, صحیح funcindex, صحیح n);
LUA_API ثابت نویسه *(lua_setupvalue) (lua_State *L, صحیح funcindex, صحیح n);

LUA_API تهی *(lua_upvalueid) (lua_State *L, صحیح fidx, صحیح n);
LUA_API تهی  (lua_upvaluejoin) (lua_State *L, صحیح fidx1, صحیح n1,
                                               صحیح fidx2, صحیح n2);

LUA_API تهی (lua_sethook) (lua_State *L, lua_Hook func, صحیح mask, صحیح شمار);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API صحیح (lua_gethookmask) (lua_State *L);
LUA_API صحیح (lua_gethookcount) (lua_State *L);


ساختار lua_Debug {
  صحیح event;
  ثابت نویسه *name;	/* (n) */
  ثابت نویسه *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ثابت نویسه *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ثابت نویسه *source;	/* (S) */
  size_t srclen;	/* (S) */
  صحیح currentline;	/* (l) */
  صحیح linedefined;	/* (S) */
  صحیح lastlinedefined;	/* (S) */
  unsigned نویسه nups;	/* (u) number of upvalues */
  unsigned نویسه nparams;/* (u) number of parameters */
  نویسه isvararg;        /* (u) */
  unsigned نویسه extraargs;  /* (t) number of extra arguments */
  نویسه istailcall;	/* (t) */
  صحیح ftransfer;   /* (r) index of first value transferred */
  صحیح ntransfer;   /* (r) number of transferred values */
  نویسه short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  ساختار CallInfo *i_ci;  /* active function */
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
