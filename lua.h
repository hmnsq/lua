/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#جيڪڏهن_وصف_نه_ٿيل lua_h
#وصف_ڪريو lua_h

#ضمن <stdarg.h>
#ضمن <stddef.h>


#وصف_ڪريو LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#وصف_ڪريو LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#وصف_ڪريو LUA_نسخو_MAJOR_N	5
#وصف_ڪريو LUA_نسخو_MINOR_N	5
#وصف_ڪريو LUA_نسخو_RELEASE_N	1

#وصف_ڪريو LUA_نسخو_انگ  (LUA_نسخو_MAJOR_N * 100 + LUA_نسخو_MINOR_N)
#وصف_ڪريو LUA_نسخو_RELEASE_انگ  (LUA_نسخو_انگ * 100 + LUA_نسخو_RELEASE_N)


#ضمن "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#وصف_ڪريو LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#وصف_ڪريو LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#وصف_ڪريو LUA_REGISTRYINDEX	(-(وڌ_کان_وڌ_عدد/2 + 1000))
#وصف_ڪريو lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#وصف_ڪريو LUA_OK		0
#وصف_ڪريو LUA_YIELD	1
#وصف_ڪريو LUA_ERRRUN	2
#وصف_ڪريو LUA_ERRSYNTAX	3
#وصف_ڪريو LUA_ERRMEM	4
#وصف_ڪريو LUA_ERRERR	5


تعريفنوع بنية lua_State lua_State;


/*
** basic types
*/
#وصف_ڪريو LUA_TNONE		(-1)

#وصف_ڪريو LUA_TNIL		0
#وصف_ڪريو LUA_TBOOLEAN		1
#وصف_ڪريو LUA_TLIGHTUSERDATA	2
#وصف_ڪريو LUA_TNUMBER		3
#وصف_ڪريو LUA_TSTRING		4
#وصف_ڪريو LUA_TTABLE		5
#وصف_ڪريو LUA_TFUNCTION		6
#وصف_ڪريو LUA_TUSERDATA		7
#وصف_ڪريو LUA_TTHREAD		8

#وصف_ڪريو LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#وصف_ڪريو LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#وصف_ڪريو LUA_RIDX_GLOBALS	2
#وصف_ڪريو LUA_RIDX_MAINTHREAD	3
#وصف_ڪريو LUA_RIDX_آخري		3


/* type of numbers in Lua */
تعريفنوع LUA_عدد lua_عدد;


/* type for integer functions */
تعريفنوع LUA_INTEGER lua_Integer;

/* unsigned integer type */
تعريفنوع LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
تعريفنوع LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
تعريفنوع صحيح (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
تعريفنوع صحيح (*lua_KFunction) (lua_State *L, صحيح حالت, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
تعريفنوع ثابت حرف * (*lua_Reader) (lua_State *L, فراغ *ud, size_t *sz);

تعريفنوع صحيح (*lua_Writer) (lua_State *L, ثابت فراغ *p, size_t sz, فراغ *ud);


/*
** Type for memory-allocation functions
*/
تعريفنوع فراغ * (*lua_Alloc) (فراغ *ud, فراغ *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
تعريفنوع فراغ (*lua_WarnFunction) (فراغ *ud, ثابت حرف *نياپو, صحيح tocont);


/*
** Type used by the debug API to collect debug information
*/
تعريفنوع بنية lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
تعريفنوع فراغ (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#إذا defined(LUA_USER_H)
#ضمن LUA_USER_H
#جيڪڏهن_پورو


/*
** RCS ident string
*/
خارجي ثابت حرف lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, فراغ *ud, بنانشان ٻج);
LUA_API فراغ       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API صحيح        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_عدد (lua_نسخو) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API صحيح   (lua_absindex) (lua_State *L, صحيح idx);
LUA_API صحيح   (lua_gettop) (lua_State *L);
LUA_API فراغ  (lua_settop) (lua_State *L, صحيح idx);
LUA_API فراغ  (lua_pushvalue) (lua_State *L, صحيح idx);
LUA_API فراغ  (lua_rotate) (lua_State *L, صحيح idx, صحيح n);
LUA_API فراغ  (lua_copy) (lua_State *L, صحيح fromidx, صحيح toidx);
LUA_API صحيح   (lua_checkstack) (lua_State *L, صحيح n);

LUA_API فراغ  (lua_xmove) (lua_State *from, lua_State *منزل, صحيح n);


/*
** access functions (stack -> C)
*/

LUA_API صحيح             (lua_isnumber) (lua_State *L, صحيح idx);
LUA_API صحيح             (lua_isstring) (lua_State *L, صحيح idx);
LUA_API صحيح             (lua_iscfunction) (lua_State *L, صحيح idx);
LUA_API صحيح             (lua_isinteger) (lua_State *L, صحيح idx);
LUA_API صحيح             (lua_isuserdata) (lua_State *L, صحيح idx);
LUA_API صحيح             (lua_type) (lua_State *L, صحيح idx);
LUA_API ثابت حرف     *(lua_typename) (lua_State *L, صحيح tp);

LUA_API lua_عدد      (lua_tonumberx) (lua_State *L, صحيح idx, صحيح *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, صحيح idx, صحيح *isnum);
LUA_API صحيح             (lua_toboolean) (lua_State *L, صحيح idx);
LUA_API ثابت حرف     *(lua_tolstring) (lua_State *L, صحيح idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, صحيح idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, صحيح idx);
LUA_API فراغ	       *(lua_touserdata) (lua_State *L, صحيح idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, صحيح idx);
LUA_API ثابت فراغ     *(lua_topointer) (lua_State *L, صحيح idx);


/*
** Comparison and arithmetic functions
*/

#وصف_ڪريو LUA_OPADD	0	/* ORDER TM, ORDER OP */
#وصف_ڪريو LUA_OPSUB	1
#وصف_ڪريو LUA_OPMUL	2
#وصف_ڪريو LUA_OPMOD	3
#وصف_ڪريو LUA_OPPOW	4
#وصف_ڪريو LUA_OPDIV	5
#وصف_ڪريو LUA_OPIDIV	6
#وصف_ڪريو LUA_OPBAND	7
#وصف_ڪريو LUA_OPBOR	8
#وصف_ڪريو LUA_OPBXOR	9
#وصف_ڪريو LUA_OPSHL	10
#وصف_ڪريو LUA_OPSHR	11
#وصف_ڪريو LUA_OPUNM	12
#وصف_ڪريو LUA_OPBNOT	13

LUA_API فراغ  (lua_arith) (lua_State *L, صحيح op);

#وصف_ڪريو LUA_OPEQ	0
#وصف_ڪريو LUA_OPLT	1
#وصف_ڪريو LUA_OPLE	2

LUA_API صحيح   (lua_rawequal) (lua_State *L, صحيح idx1, صحيح idx2);
LUA_API صحيح   (lua_compare) (lua_State *L, صحيح idx1, صحيح idx2, صحيح op);


/*
** push functions (C -> stack)
*/
LUA_API فراغ        (lua_pushnil) (lua_State *L);
LUA_API فراغ        (lua_pushnumber) (lua_State *L, lua_عدد n);
LUA_API فراغ        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ثابت حرف *(lua_pushlstring) (lua_State *L, ثابت حرف *s, size_t len);
LUA_API ثابت حرف *(lua_pushexternalstring) (lua_State *L,
		ثابت حرف *s, size_t len, lua_Alloc falloc, فراغ *ud);
LUA_API ثابت حرف *(lua_pushstring) (lua_State *L, ثابت حرف *s);
LUA_API ثابت حرف *(lua_pushvfstring) (lua_State *L, ثابت حرف *fmt,
                                                      va_list argp);
LUA_API ثابت حرف *(lua_pushfstring) (lua_State *L, ثابت حرف *fmt, ...);
LUA_API فراغ  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, صحيح n);
LUA_API فراغ  (lua_pushboolean) (lua_State *L, صحيح b);
LUA_API فراغ  (lua_pushlightuserdata) (lua_State *L, فراغ *p);
LUA_API صحيح   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API صحيح (lua_getglobal) (lua_State *L, ثابت حرف *name);
LUA_API صحيح (lua_gettable) (lua_State *L, صحيح idx);
LUA_API صحيح (lua_getfield) (lua_State *L, صحيح idx, ثابت حرف *k);
LUA_API صحيح (lua_geti) (lua_State *L, صحيح idx, lua_Integer n);
LUA_API صحيح (lua_rawget) (lua_State *L, صحيح idx);
LUA_API صحيح (lua_rawgeti) (lua_State *L, صحيح idx, lua_Integer n);
LUA_API صحيح (lua_rawgetp) (lua_State *L, صحيح idx, ثابت فراغ *p);

LUA_API فراغ  (lua_createtable) (lua_State *L, صحيح narr, صحيح nrec);
LUA_API فراغ *(lua_newuserdatauv) (lua_State *L, size_t sz, صحيح nuvalue);
LUA_API صحيح   (lua_getmetatable) (lua_State *L, صحيح objindex);
LUA_API صحيح  (lua_getiuservalue) (lua_State *L, صحيح idx, صحيح n);


/*
** set functions (stack -> Lua)
*/
LUA_API فراغ  (lua_setglobal) (lua_State *L, ثابت حرف *name);
LUA_API فراغ  (lua_settable) (lua_State *L, صحيح idx);
LUA_API فراغ  (lua_setfield) (lua_State *L, صحيح idx, ثابت حرف *k);
LUA_API فراغ  (lua_seti) (lua_State *L, صحيح idx, lua_Integer n);
LUA_API فراغ  (lua_rawset) (lua_State *L, صحيح idx);
LUA_API فراغ  (lua_rawseti) (lua_State *L, صحيح idx, lua_Integer n);
LUA_API فراغ  (lua_rawsetp) (lua_State *L, صحيح idx, ثابت فراغ *p);
LUA_API صحيح   (lua_setmetatable) (lua_State *L, صحيح objindex);
LUA_API صحيح   (lua_setiuservalue) (lua_State *L, صحيح idx, صحيح n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API فراغ  (lua_callk) (lua_State *L, صحيح nargs, صحيح nresults,
                           lua_KContext ctx, lua_KFunction k);
#وصف_ڪريو lua_call(L,n,r)		lua_callk(L, (n), (r), 0, عدم)

LUA_API صحيح   (lua_pcallk) (lua_State *L, صحيح nargs, صحيح nresults, صحيح errfunc,
                            lua_KContext ctx, lua_KFunction k);
#وصف_ڪريو lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, عدم)

LUA_API صحيح   (lua_ڀريو) (lua_State *L, lua_Reader reader, فراغ *dt,
                          ثابت حرف *chunkname, ثابت حرف *گهڻائي);

LUA_API صحيح (lua_dump) (lua_State *L, lua_Writer writer, فراغ *معلومات, صحيح strip);


/*
** coroutine functions
*/
LUA_API صحيح  (lua_yieldk)     (lua_State *L, صحيح nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API صحيح  (lua_resume)     (lua_State *L, lua_State *from, صحيح narg,
                               صحيح *nres);
LUA_API صحيح  (lua_حالت)     (lua_State *L);
LUA_API صحيح (lua_isyieldable) (lua_State *L);

#وصف_ڪريو lua_yield(L,n)		lua_yieldk(L, (n), 0, عدم)


/*
** Warning-related functions
*/
LUA_API فراغ (lua_setwarnf) (lua_State *L, lua_WarnFunction f, فراغ *ud);
LUA_API فراغ (lua_warning)  (lua_State *L, ثابت حرف *نياپو, صحيح tocont);


/*
** garbage-collection options
*/

#وصف_ڪريو LUA_GCSTOP		0
#وصف_ڪريو LUA_GCRESTART		1
#وصف_ڪريو LUA_GCCOLLECT		2
#وصف_ڪريو LUA_GCCOUNT		3
#وصف_ڪريو LUA_GCCOUNTB		4
#وصف_ڪريو LUA_GCSTEP		5
#وصف_ڪريو LUA_GCISRUNNING		6
#وصف_ڪريو LUA_GCGEN		7
#وصف_ڪريو LUA_GCINC		8
#وصف_ڪريو LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#وصف_ڪريو LUA_GCPMINORMUL		0  /* control minor collections */
#وصف_ڪريو LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#وصف_ڪريو LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#وصف_ڪريو LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#وصف_ڪريو LUA_GCPSTEPMUL		4  /* GC "speed" */
#وصف_ڪريو LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#وصف_ڪريو LUA_GCPN		6


LUA_API صحيح (lua_gc) (lua_State *L, صحيح what, ...);


/*
** miscellaneous functions
*/

LUA_API صحيح   (lua_error) (lua_State *L);

LUA_API صحيح   (lua_ايندڙ) (lua_State *L, صحيح idx);

LUA_API فراغ  (lua_concat) (lua_State *L, صحيح n);
LUA_API فراغ  (lua_len)    (lua_State *L, صحيح idx);

#وصف_ڪريو LUA_N2SBUFFSZ	64
LUA_API بنانشان  (lua_numbertocstring) (lua_State *L, صحيح idx, حرف *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ثابت حرف *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, فراغ **ud);
LUA_API فراغ      (lua_setallocf) (lua_State *L, lua_Alloc f, فراغ *ud);

LUA_API فراغ (lua_toclose) (lua_State *L, صحيح idx);
LUA_API فراغ (lua_closeslot) (lua_State *L, صحيح idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#وصف_ڪريو lua_getextraspace(L)	((فراغ *)((حرف *)(L) - LUA_EXTRASPACE))

#وصف_ڪريو lua_tonumber(L,i)	lua_tonumberx(L,(i),عدم)
#وصف_ڪريو lua_tointeger(L,i)	lua_tointegerx(L,(i),عدم)

#وصف_ڪريو lua_ڪڍو(L,n)		lua_settop(L, -(n)-1)

#وصف_ڪريو lua_newtable(L)		lua_createtable(L, 0, 0)

#وصف_ڪريو lua_رجسٽر(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#وصف_ڪريو lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#وصف_ڪريو lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#وصف_ڪريو lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#وصف_ڪريو lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#وصف_ڪريو lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#وصف_ڪريو lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#وصف_ڪريو lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#وصف_ڪريو lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#وصف_ڪريو lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#وصف_ڪريو lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#وصف_ڪريو lua_pushglobaltable(L)  \
	((فراغ)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#وصف_ڪريو lua_tostring(L,i)	lua_tolstring(L, (i), عدم)


#وصف_ڪريو lua_وجهو(L,idx)	lua_rotate(L, (idx), 1)

#وصف_ڪريو lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_ڪڍو(L, 1))

#وصف_ڪريو lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_ڪڍو(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#وصف_ڪريو lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#وصف_ڪريو lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#وصف_ڪريو lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#وصف_ڪريو lua_resetthread(L)	lua_closethread(L,عدم)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#وصف_ڪريو LUA_HOOKCALL	0
#وصف_ڪريو LUA_HOOKRET	1
#وصف_ڪريو LUA_HOOKLINE	2
#وصف_ڪريو LUA_HOOKCOUNT	3
#وصف_ڪريو LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#وصف_ڪريو LUA_MASKCALL	(1 << LUA_HOOKCALL)
#وصف_ڪريو LUA_MASKRET	(1 << LUA_HOOKRET)
#وصف_ڪريو LUA_MASKLINE	(1 << LUA_HOOKLINE)
#وصف_ڪريو LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API صحيح (lua_getstack) (lua_State *L, صحيح level, lua_Debug *ar);
LUA_API صحيح (lua_getinfo) (lua_State *L, ثابت حرف *what, lua_Debug *ar);
LUA_API ثابت حرف *(lua_getlocal) (lua_State *L, ثابت lua_Debug *ar, صحيح n);
LUA_API ثابت حرف *(lua_setlocal) (lua_State *L, ثابت lua_Debug *ar, صحيح n);
LUA_API ثابت حرف *(lua_getupvalue) (lua_State *L, صحيح funcindex, صحيح n);
LUA_API ثابت حرف *(lua_setupvalue) (lua_State *L, صحيح funcindex, صحيح n);

LUA_API فراغ *(lua_upvalueid) (lua_State *L, صحيح fidx, صحيح n);
LUA_API فراغ  (lua_upvaluejoin) (lua_State *L, صحيح fidx1, صحيح n1,
                                               صحيح fidx2, صحيح n2);

LUA_API فراغ (lua_sethook) (lua_State *L, lua_Hook func, صحيح نقاب_2, صحيح ڳڻپ);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API صحيح (lua_gethookmask) (lua_State *L);
LUA_API صحيح (lua_gethookcount) (lua_State *L);


بنية lua_Debug {
  صحيح event;
  ثابت حرف *name;	/* (n) */
  ثابت حرف *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ثابت حرف *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ثابت حرف *ذريعو;	/* (S) */
  size_t srclen;	/* (S) */
  صحيح currentline;	/* (l) */
  صحيح linedefined;	/* (S) */
  صحيح lastlinedefined;	/* (S) */
  بنانشان حرف nups;	/* (u) number of upvalues */
  بنانشان حرف nparams;/* (u) number of parameters */
  حرف isvararg;        /* (u) */
  بنانشان حرف extraargs;  /* (t) number of extra arguments */
  حرف istailcall;	/* (t) */
  صحيح ftransfer;   /* (r) index of first value transferred */
  صحيح ntransfer;   /* (r) number of transferred values */
  حرف short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  بنية CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#وصف_ڪريو LUAI_TOSTRAUX(x)	#x
#وصف_ڪريو LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#وصف_ڪريو LUA_نسخو_MAJOR	LUAI_TOSTR(LUA_نسخو_MAJOR_N)
#وصف_ڪريو LUA_نسخو_MINOR	LUAI_TOSTR(LUA_نسخو_MINOR_N)
#وصف_ڪريو LUA_نسخو_RELEASE	LUAI_TOSTR(LUA_نسخو_RELEASE_N)

#وصف_ڪريو LUA_نسخو	"Lua " LUA_نسخو_MAJOR "." LUA_نسخو_MINOR
#وصف_ڪريو LUA_RELEASE	LUA_نسخو "." LUA_نسخو_RELEASE


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


#جيڪڏهن_پورو
