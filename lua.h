/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stddef.h>


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	1

#define LUA_VERSION_இலக்கம்  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_இலக்கம்  (LUA_VERSION_இலக்கம் * 100 + LUA_VERSION_RELEASE_N)


#அந்தர்பாவ "luaconf.h"


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


வகைவரை கட்டமைப்பு lua_State lua_State;


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
வகைவரை LUA_எண் lua_எண்;


/* type for integer functions */
வகைவரை LUA_INTEGER lua_Integer;

/* unsigned integer type */
வகைவரை LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
வகைவரை LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
வகைவரை முழு (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
வகைவரை முழு (*lua_KFunction) (lua_State *L, முழு status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
வகைவரை மாறிலி எழுத்து * (*lua_Reader) (lua_State *L, வெறுமை *ud, size_t *sz);

வகைவரை முழு (*lua_Writer) (lua_State *L, மாறிலி வெறுமை *p, size_t sz, வெறுமை *ud);


/*
** Type for memory-allocation functions
*/
வகைவரை வெறுமை * (*lua_Alloc) (வெறுமை *ud, வெறுமை *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
வகைவரை வெறுமை (*lua_WarnFunction) (வெறுமை *ud, மாறிலி எழுத்து *செய்தி, முழு tocont);


/*
** Type used by the debug API to collect debug information
*/
வகைவரை கட்டமைப்பு lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
வகைவரை வெறுமை (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#எனில் defined(LUA_USER_H)
#அந்தர்பாவ LUA_USER_H
#endif


/*
** RCS ident string
*/
வெளி மாறிலி எழுத்து lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, வெறுமை *ud, unsigned seed);
LUA_API வெறுமை       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API முழு        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_எண் (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API முழு   (lua_absindex) (lua_State *L, முழு idx);
LUA_API முழு   (lua_gettop) (lua_State *L);
LUA_API வெறுமை  (lua_settop) (lua_State *L, முழு idx);
LUA_API வெறுமை  (lua_pushvalue) (lua_State *L, முழு idx);
LUA_API வெறுமை  (lua_rotate) (lua_State *L, முழு idx, முழு n);
LUA_API வெறுமை  (lua_copy) (lua_State *L, முழு fromidx, முழு toidx);
LUA_API முழு   (lua_checkstack) (lua_State *L, முழு n);

LUA_API வெறுமை  (lua_xmove) (lua_State *from, lua_State *to, முழு n);


/*
** access functions (stack -> C)
*/

LUA_API முழு             (lua_isnumber) (lua_State *L, முழு idx);
LUA_API முழு             (lua_isstring) (lua_State *L, முழு idx);
LUA_API முழு             (lua_iscfunction) (lua_State *L, முழு idx);
LUA_API முழு             (lua_isinteger) (lua_State *L, முழு idx);
LUA_API முழு             (lua_isuserdata) (lua_State *L, முழு idx);
LUA_API முழு             (lua_type) (lua_State *L, முழு idx);
LUA_API மாறிலி எழுத்து     *(lua_typename) (lua_State *L, முழு tp);

LUA_API lua_எண்      (lua_tonumberx) (lua_State *L, முழு idx, முழு *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, முழு idx, முழு *isnum);
LUA_API முழு             (lua_toboolean) (lua_State *L, முழு idx);
LUA_API மாறிலி எழுத்து     *(lua_tolstring) (lua_State *L, முழு idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, முழு idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, முழு idx);
LUA_API வெறுமை	       *(lua_touserdata) (lua_State *L, முழு idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, முழு idx);
LUA_API மாறிலி வெறுமை     *(lua_topointer) (lua_State *L, முழு idx);


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

LUA_API வெறுமை  (lua_arith) (lua_State *L, முழு op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API முழு   (lua_rawequal) (lua_State *L, முழு idx1, முழு idx2);
LUA_API முழு   (lua_compare) (lua_State *L, முழு idx1, முழு idx2, முழு op);


/*
** push functions (C -> stack)
*/
LUA_API வெறுமை        (lua_pushnil) (lua_State *L);
LUA_API வெறுமை        (lua_pushnumber) (lua_State *L, lua_எண் n);
LUA_API வெறுமை        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API மாறிலி எழுத்து *(lua_pushlstring) (lua_State *L, மாறிலி எழுத்து *s, size_t len);
LUA_API மாறிலி எழுத்து *(lua_pushexternalstring) (lua_State *L,
		மாறிலி எழுத்து *s, size_t len, lua_Alloc falloc, வெறுமை *ud);
LUA_API மாறிலி எழுத்து *(lua_pushstring) (lua_State *L, மாறிலி எழுத்து *s);
LUA_API மாறிலி எழுத்து *(lua_pushvfstring) (lua_State *L, மாறிலி எழுத்து *fmt,
                                                      va_list argp);
LUA_API மாறிலி எழுத்து *(lua_pushfstring) (lua_State *L, மாறிலி எழுத்து *fmt, ...);
LUA_API வெறுமை  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, முழு n);
LUA_API வெறுமை  (lua_pushboolean) (lua_State *L, முழு b);
LUA_API வெறுமை  (lua_pushlightuserdata) (lua_State *L, வெறுமை *p);
LUA_API முழு   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API முழு (lua_getglobal) (lua_State *L, மாறிலி எழுத்து *name);
LUA_API முழு (lua_gettable) (lua_State *L, முழு idx);
LUA_API முழு (lua_getfield) (lua_State *L, முழு idx, மாறிலி எழுத்து *k);
LUA_API முழு (lua_geti) (lua_State *L, முழு idx, lua_Integer n);
LUA_API முழு (lua_rawget) (lua_State *L, முழு idx);
LUA_API முழு (lua_rawgeti) (lua_State *L, முழு idx, lua_Integer n);
LUA_API முழு (lua_rawgetp) (lua_State *L, முழு idx, மாறிலி வெறுமை *p);

LUA_API வெறுமை  (lua_createtable) (lua_State *L, முழு narr, முழு nrec);
LUA_API வெறுமை *(lua_newuserdatauv) (lua_State *L, size_t sz, முழு nuvalue);
LUA_API முழு   (lua_getmetatable) (lua_State *L, முழு objindex);
LUA_API முழு  (lua_getiuservalue) (lua_State *L, முழு idx, முழு n);


/*
** set functions (stack -> Lua)
*/
LUA_API வெறுமை  (lua_setglobal) (lua_State *L, மாறிலி எழுத்து *name);
LUA_API வெறுமை  (lua_settable) (lua_State *L, முழு idx);
LUA_API வெறுமை  (lua_setfield) (lua_State *L, முழு idx, மாறிலி எழுத்து *k);
LUA_API வெறுமை  (lua_seti) (lua_State *L, முழு idx, lua_Integer n);
LUA_API வெறுமை  (lua_rawset) (lua_State *L, முழு idx);
LUA_API வெறுமை  (lua_rawseti) (lua_State *L, முழு idx, lua_Integer n);
LUA_API வெறுமை  (lua_rawsetp) (lua_State *L, முழு idx, மாறிலி வெறுமை *p);
LUA_API முழு   (lua_setmetatable) (lua_State *L, முழு objindex);
LUA_API முழு   (lua_setiuservalue) (lua_State *L, முழு idx, முழு n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API வெறுமை  (lua_callk) (lua_State *L, முழு nargs, முழு nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, வெற்று)

LUA_API முழு   (lua_pcallk) (lua_State *L, முழு nargs, முழு nresults, முழு errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, வெற்று)

LUA_API முழு   (lua_load) (lua_State *L, lua_Reader reader, வெறுமை *dt,
                          மாறிலி எழுத்து *chunkname, மாறிலி எழுத்து *mode);

LUA_API முழு (lua_dump) (lua_State *L, lua_Writer writer, வெறுமை *data, முழு strip);


/*
** coroutine functions
*/
LUA_API முழு  (lua_yieldk)     (lua_State *L, முழு nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API முழு  (lua_resume)     (lua_State *L, lua_State *from, முழு narg,
                               முழு *nres);
LUA_API முழு  (lua_status)     (lua_State *L);
LUA_API முழு (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, வெற்று)


/*
** Warning-related functions
*/
LUA_API வெறுமை (lua_setwarnf) (lua_State *L, lua_WarnFunction f, வெறுமை *ud);
LUA_API வெறுமை (lua_warning)  (lua_State *L, மாறிலி எழுத்து *செய்தி, முழு tocont);


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


LUA_API முழு (lua_gc) (lua_State *L, முழு what, ...);


/*
** miscellaneous functions
*/

LUA_API முழு   (lua_error) (lua_State *L);

LUA_API முழு   (lua_next) (lua_State *L, முழு idx);

LUA_API வெறுமை  (lua_concat) (lua_State *L, முழு n);
LUA_API வெறுமை  (lua_len)    (lua_State *L, முழு idx);

#define LUA_N2SBUFFSZ	64
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, முழு idx, எழுத்து *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, மாறிலி எழுத்து *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, வெறுமை **ud);
LUA_API வெறுமை      (lua_setallocf) (lua_State *L, lua_Alloc f, வெறுமை *ud);

LUA_API வெறுமை (lua_toclose) (lua_State *L, முழு idx);
LUA_API வெறுமை (lua_closeslot) (lua_State *L, முழு idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((வெறுமை *)((எழுத்து *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),வெற்று)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),வெற்று)

#define lua_எடு(L,n)		lua_settop(L, -(n)-1)

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
	((வெறுமை)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), வெற்று)


#define lua_செருகு(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_எடு(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_எடு(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,வெற்று)

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


LUA_API முழு (lua_getstack) (lua_State *L, முழு level, lua_Debug *ar);
LUA_API முழு (lua_getinfo) (lua_State *L, மாறிலி எழுத்து *what, lua_Debug *ar);
LUA_API மாறிலி எழுத்து *(lua_getlocal) (lua_State *L, மாறிலி lua_Debug *ar, முழு n);
LUA_API மாறிலி எழுத்து *(lua_setlocal) (lua_State *L, மாறிலி lua_Debug *ar, முழு n);
LUA_API மாறிலி எழுத்து *(lua_getupvalue) (lua_State *L, முழு funcindex, முழு n);
LUA_API மாறிலி எழுத்து *(lua_setupvalue) (lua_State *L, முழு funcindex, முழு n);

LUA_API வெறுமை *(lua_upvalueid) (lua_State *L, முழு fidx, முழு n);
LUA_API வெறுமை  (lua_upvaluejoin) (lua_State *L, முழு fidx1, முழு n1,
                                               முழு fidx2, முழு n2);

LUA_API வெறுமை (lua_sethook) (lua_State *L, lua_Hook func, முழு mask, முழு மொத்தம்);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API முழு (lua_gethookmask) (lua_State *L);
LUA_API முழு (lua_gethookcount) (lua_State *L);


கட்டமைப்பு lua_Debug {
  முழு event;
  மாறிலி எழுத்து *name;	/* (n) */
  மாறிலி எழுத்து *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  மாறிலி எழுத்து *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  மாறிலி எழுத்து *source;	/* (S) */
  size_t srclen;	/* (S) */
  முழு currentline;	/* (l) */
  முழு linedefined;	/* (S) */
  முழு lastlinedefined;	/* (S) */
  unsigned எழுத்து nups;	/* (u) number of upvalues */
  unsigned எழுத்து nparams;/* (u) number of parameters */
  எழுத்து isvararg;        /* (u) */
  unsigned எழுத்து extraargs;  /* (t) number of extra arguments */
  எழுத்து istailcall;	/* (t) */
  முழு ftransfer;   /* (r) index of first value transferred */
  முழு ntransfer;   /* (r) number of transferred values */
  எழுத்து short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  கட்டமைப்பு CallInfo *i_ci;  /* active function */
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
