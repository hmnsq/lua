/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#اگر_غیر_تعریف_شدہ lua_h
#تعریف_کرو lua_h

#شامل <stdarg.h>
#شامل <stddef.h>


#تعریف_کرو LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#تعریف_کرو LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#تعریف_کرو LUA_نسخہ_MAJOR_N	5
#تعریف_کرو LUA_نسخہ_MINOR_N	5
#تعریف_کرو LUA_نسخہ_RELEASE_N	1

#تعریف_کرو LUA_نسخہ_عدد  (LUA_نسخہ_MAJOR_N * 100 + LUA_نسخہ_MINOR_N)
#تعریف_کرو LUA_نسخہ_RELEASE_عدد  (LUA_نسخہ_عدد * 100 + LUA_نسخہ_RELEASE_N)


#شامل "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#تعریف_کرو LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#تعریف_کرو LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#تعریف_کرو LUA_REGISTRYINDEX	(-(زیادہ_سے_زیادہ_عدد/2 + 1000))
#تعریف_کرو lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#تعریف_کرو LUA_OK		0
#تعریف_کرو LUA_YIELD	1
#تعریف_کرو LUA_ERRRUN	2
#تعریف_کرو LUA_ERRSYNTAX	3
#تعریف_کرو LUA_ERRMEM	4
#تعریف_کرو LUA_ERRERR	5


قسمتعریف ساخت lua_State lua_State;


/*
** basic types
*/
#تعریف_کرو LUA_TNONE		(-1)

#تعریف_کرو LUA_TNIL		0
#تعریف_کرو LUA_TBOOLEAN		1
#تعریف_کرو LUA_TLIGHTUSERDATA	2
#تعریف_کرو LUA_TNUMBER		3
#تعریف_کرو LUA_TSTRING		4
#تعریف_کرو LUA_TTABLE		5
#تعریف_کرو LUA_TFUNCTION		6
#تعریف_کرو LUA_TUSERDATA		7
#تعریف_کرو LUA_TTHREAD		8

#تعریف_کرو LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#تعریف_کرو LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#تعریف_کرو LUA_RIDX_GLOBALS	2
#تعریف_کرو LUA_RIDX_MAINTHREAD	3
#تعریف_کرو LUA_RIDX_آخری		3


/* type of numbers in Lua */
قسمتعریف LUA_نمبر lua_نمبر;


/* type for integer functions */
قسمتعریف LUA_INTEGER lua_Integer;

/* unsigned integer type */
قسمتعریف LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
قسمتعریف LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
قسمتعریف صحیح (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
قسمتعریف صحیح (*lua_KFunction) (lua_State *L, صحیح حالت_2, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
قسمتعریف ثابت حرف * (*lua_Reader) (lua_State *L, خالی *ud, size_t *sz);

قسمتعریف صحیح (*lua_Writer) (lua_State *L, ثابت خالی *p, size_t sz, خالی *ud);


/*
** Type for memory-allocation functions
*/
قسمتعریف خالی * (*lua_Alloc) (خالی *ud, خالی *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
قسمتعریف خالی (*lua_WarnFunction) (خالی *ud, ثابت حرف *پیغام, صحیح tocont);


/*
** Type used by the debug API to collect debug information
*/
قسمتعریف ساخت lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
قسمتعریف خالی (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#اگر defined(LUA_USER_H)
#شامل LUA_USER_H
#اگر_ختم


/*
** RCS ident string
*/
خارجی ثابت حرف lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, خالی *ud, غیرعلامتی بیج);
LUA_API خالی       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API صحیح        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_نمبر (lua_نسخہ) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API صحیح   (lua_absindex) (lua_State *L, صحیح idx);
LUA_API صحیح   (lua_gettop) (lua_State *L);
LUA_API خالی  (lua_settop) (lua_State *L, صحیح idx);
LUA_API خالی  (lua_pushvalue) (lua_State *L, صحیح idx);
LUA_API خالی  (lua_rotate) (lua_State *L, صحیح idx, صحیح n);
LUA_API خالی  (lua_copy) (lua_State *L, صحیح fromidx, صحیح toidx);
LUA_API صحیح   (lua_checkstack) (lua_State *L, صحیح n);

LUA_API خالی  (lua_xmove) (lua_State *from, lua_State *مقصد, صحیح n);


/*
** access functions (stack -> C)
*/

LUA_API صحیح             (lua_isnumber) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_isstring) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_iscfunction) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_isinteger) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_isuserdata) (lua_State *L, صحیح idx);
LUA_API صحیح             (lua_type) (lua_State *L, صحیح idx);
LUA_API ثابت حرف     *(lua_typename) (lua_State *L, صحیح tp);

LUA_API lua_نمبر      (lua_tonumberx) (lua_State *L, صحیح idx, صحیح *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, صحیح idx, صحیح *isnum);
LUA_API صحیح             (lua_toboolean) (lua_State *L, صحیح idx);
LUA_API ثابت حرف     *(lua_tolstring) (lua_State *L, صحیح idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, صحیح idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, صحیح idx);
LUA_API خالی	       *(lua_touserdata) (lua_State *L, صحیح idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, صحیح idx);
LUA_API ثابت خالی     *(lua_topointer) (lua_State *L, صحیح idx);


/*
** Comparison and arithmetic functions
*/

#تعریف_کرو LUA_OPADD	0	/* ORDER TM, ORDER OP */
#تعریف_کرو LUA_OPSUB	1
#تعریف_کرو LUA_OPMUL	2
#تعریف_کرو LUA_OPMOD	3
#تعریف_کرو LUA_OPPOW	4
#تعریف_کرو LUA_OPDIV	5
#تعریف_کرو LUA_OPIDIV	6
#تعریف_کرو LUA_OPBAND	7
#تعریف_کرو LUA_OPBOR	8
#تعریف_کرو LUA_OPBXOR	9
#تعریف_کرو LUA_OPSHL	10
#تعریف_کرو LUA_OPSHR	11
#تعریف_کرو LUA_OPUNM	12
#تعریف_کرو LUA_OPBNOT	13

LUA_API خالی  (lua_arith) (lua_State *L, صحیح op);

#تعریف_کرو LUA_OPEQ	0
#تعریف_کرو LUA_OPLT	1
#تعریف_کرو LUA_OPLE	2

LUA_API صحیح   (lua_rawequal) (lua_State *L, صحیح idx1, صحیح idx2);
LUA_API صحیح   (lua_compare) (lua_State *L, صحیح idx1, صحیح idx2, صحیح op);


/*
** push functions (C -> stack)
*/
LUA_API خالی        (lua_pushnil) (lua_State *L);
LUA_API خالی        (lua_pushnumber) (lua_State *L, lua_نمبر n);
LUA_API خالی        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ثابت حرف *(lua_pushlstring) (lua_State *L, ثابت حرف *s, size_t len);
LUA_API ثابت حرف *(lua_pushexternalstring) (lua_State *L,
		ثابت حرف *s, size_t len, lua_Alloc falloc, خالی *ud);
LUA_API ثابت حرف *(lua_pushstring) (lua_State *L, ثابت حرف *s);
LUA_API ثابت حرف *(lua_pushvfstring) (lua_State *L, ثابت حرف *fmt,
                                                      va_list argp);
LUA_API ثابت حرف *(lua_pushfstring) (lua_State *L, ثابت حرف *fmt, ...);
LUA_API خالی  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, صحیح n);
LUA_API خالی  (lua_pushboolean) (lua_State *L, صحیح b);
LUA_API خالی  (lua_pushlightuserdata) (lua_State *L, خالی *p);
LUA_API صحیح   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API صحیح (lua_getglobal) (lua_State *L, ثابت حرف *name);
LUA_API صحیح (lua_gettable) (lua_State *L, صحیح idx);
LUA_API صحیح (lua_getfield) (lua_State *L, صحیح idx, ثابت حرف *k);
LUA_API صحیح (lua_geti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API صحیح (lua_rawget) (lua_State *L, صحیح idx);
LUA_API صحیح (lua_rawgeti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API صحیح (lua_rawgetp) (lua_State *L, صحیح idx, ثابت خالی *p);

LUA_API خالی  (lua_createtable) (lua_State *L, صحیح narr, صحیح nrec);
LUA_API خالی *(lua_newuserdatauv) (lua_State *L, size_t sz, صحیح nuvalue);
LUA_API صحیح   (lua_getmetatable) (lua_State *L, صحیح objindex);
LUA_API صحیح  (lua_getiuservalue) (lua_State *L, صحیح idx, صحیح n);


/*
** set functions (stack -> Lua)
*/
LUA_API خالی  (lua_setglobal) (lua_State *L, ثابت حرف *name);
LUA_API خالی  (lua_settable) (lua_State *L, صحیح idx);
LUA_API خالی  (lua_setfield) (lua_State *L, صحیح idx, ثابت حرف *k);
LUA_API خالی  (lua_seti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API خالی  (lua_rawset) (lua_State *L, صحیح idx);
LUA_API خالی  (lua_rawseti) (lua_State *L, صحیح idx, lua_Integer n);
LUA_API خالی  (lua_rawsetp) (lua_State *L, صحیح idx, ثابت خالی *p);
LUA_API صحیح   (lua_setmetatable) (lua_State *L, صحیح objindex);
LUA_API صحیح   (lua_setiuservalue) (lua_State *L, صحیح idx, صحیح n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API خالی  (lua_callk) (lua_State *L, صحیح nargs, صحیح nresults,
                           lua_KContext ctx, lua_KFunction k);
#تعریف_کرو lua_call(L,n,r)		lua_callk(L, (n), (r), 0, تہی)

LUA_API صحیح   (lua_pcallk) (lua_State *L, صحیح nargs, صحیح nresults, صحیح errfunc,
                            lua_KContext ctx, lua_KFunction k);
#تعریف_کرو lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, تہی)

LUA_API صحیح   (lua_بھرو) (lua_State *L, lua_Reader reader, خالی *dt,
                          ثابت حرف *chunkname, ثابت حرف *منوال);

LUA_API صحیح (lua_dump) (lua_State *L, lua_Writer writer, خالی *ڈیٹا, صحیح strip);


/*
** coroutine functions
*/
LUA_API صحیح  (lua_yieldk)     (lua_State *L, صحیح nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API صحیح  (lua_resume)     (lua_State *L, lua_State *from, صحیح narg,
                               صحیح *nres);
LUA_API صحیح  (lua_حالت)     (lua_State *L);
LUA_API صحیح (lua_isyieldable) (lua_State *L);

#تعریف_کرو lua_yield(L,n)		lua_yieldk(L, (n), 0, تہی)


/*
** Warning-related functions
*/
LUA_API خالی (lua_setwarnf) (lua_State *L, lua_WarnFunction f, خالی *ud);
LUA_API خالی (lua_warning)  (lua_State *L, ثابت حرف *پیغام, صحیح tocont);


/*
** garbage-collection options
*/

#تعریف_کرو LUA_GCSTOP		0
#تعریف_کرو LUA_GCRESTART		1
#تعریف_کرو LUA_GCCOLLECT		2
#تعریف_کرو LUA_GCCOUNT		3
#تعریف_کرو LUA_GCCOUNTB		4
#تعریف_کرو LUA_GCSTEP		5
#تعریف_کرو LUA_GCISRUNNING		6
#تعریف_کرو LUA_GCGEN		7
#تعریف_کرو LUA_GCINC		8
#تعریف_کرو LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#تعریف_کرو LUA_GCPMINORMUL		0  /* control minor collections */
#تعریف_کرو LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#تعریف_کرو LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#تعریف_کرو LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#تعریف_کرو LUA_GCPSTEPMUL		4  /* GC "speed" */
#تعریف_کرو LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#تعریف_کرو LUA_GCPN		6


LUA_API صحیح (lua_gc) (lua_State *L, صحیح what, ...);


/*
** miscellaneous functions
*/

LUA_API صحیح   (lua_error) (lua_State *L);

LUA_API صحیح   (lua_اگلا) (lua_State *L, صحیح idx);

LUA_API خالی  (lua_concat) (lua_State *L, صحیح n);
LUA_API خالی  (lua_len)    (lua_State *L, صحیح idx);

#تعریف_کرو LUA_N2SBUFFSZ	64
LUA_API غیرعلامتی  (lua_numbertocstring) (lua_State *L, صحیح idx, حرف *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ثابت حرف *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, خالی **ud);
LUA_API خالی      (lua_setallocf) (lua_State *L, lua_Alloc f, خالی *ud);

LUA_API خالی (lua_toclose) (lua_State *L, صحیح idx);
LUA_API خالی (lua_closeslot) (lua_State *L, صحیح idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#تعریف_کرو lua_getextraspace(L)	((خالی *)((حرف *)(L) - LUA_EXTRASPACE))

#تعریف_کرو lua_tonumber(L,i)	lua_tonumberx(L,(i),تہی)
#تعریف_کرو lua_tointeger(L,i)	lua_tointegerx(L,(i),تہی)

#تعریف_کرو lua_نکالو(L,n)		lua_settop(L, -(n)-1)

#تعریف_کرو lua_newtable(L)		lua_createtable(L, 0, 0)

#تعریف_کرو lua_رجسٹر(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#تعریف_کرو lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#تعریف_کرو lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#تعریف_کرو lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#تعریف_کرو lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#تعریف_کرو lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#تعریف_کرو lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#تعریف_کرو lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#تعریف_کرو lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#تعریف_کرو lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#تعریف_کرو lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#تعریف_کرو lua_pushglobaltable(L)  \
	((خالی)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#تعریف_کرو lua_tostring(L,i)	lua_tolstring(L, (i), تہی)


#تعریف_کرو lua_داخل_کرو(L,idx)	lua_rotate(L, (idx), 1)

#تعریف_کرو lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_نکالو(L, 1))

#تعریف_کرو lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_نکالو(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#تعریف_کرو lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#تعریف_کرو lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#تعریف_کرو lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#تعریف_کرو lua_resetthread(L)	lua_closethread(L,تہی)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#تعریف_کرو LUA_HOOKCALL	0
#تعریف_کرو LUA_HOOKRET	1
#تعریف_کرو LUA_HOOKLINE	2
#تعریف_کرو LUA_HOOKCOUNT	3
#تعریف_کرو LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#تعریف_کرو LUA_MASKCALL	(1 << LUA_HOOKCALL)
#تعریف_کرو LUA_MASKRET	(1 << LUA_HOOKRET)
#تعریف_کرو LUA_MASKLINE	(1 << LUA_HOOKLINE)
#تعریف_کرو LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API صحیح (lua_getstack) (lua_State *L, صحیح level, lua_Debug *ar);
LUA_API صحیح (lua_getinfo) (lua_State *L, ثابت حرف *what, lua_Debug *ar);
LUA_API ثابت حرف *(lua_getlocal) (lua_State *L, ثابت lua_Debug *ar, صحیح n);
LUA_API ثابت حرف *(lua_setlocal) (lua_State *L, ثابت lua_Debug *ar, صحیح n);
LUA_API ثابت حرف *(lua_getupvalue) (lua_State *L, صحیح funcindex, صحیح n);
LUA_API ثابت حرف *(lua_setupvalue) (lua_State *L, صحیح funcindex, صحیح n);

LUA_API خالی *(lua_upvalueid) (lua_State *L, صحیح fidx, صحیح n);
LUA_API خالی  (lua_upvaluejoin) (lua_State *L, صحیح fidx1, صحیح n1,
                                               صحیح fidx2, صحیح n2);

LUA_API خالی (lua_sethook) (lua_State *L, lua_Hook func, صحیح نقاب_2, صحیح گنتی);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API صحیح (lua_gethookmask) (lua_State *L);
LUA_API صحیح (lua_gethookcount) (lua_State *L);


ساخت lua_Debug {
  صحیح event;
  ثابت حرف *name;	/* (n) */
  ثابت حرف *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ثابت حرف *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ثابت حرف *ماخذ;	/* (S) */
  size_t srclen;	/* (S) */
  صحیح currentline;	/* (l) */
  صحیح linedefined;	/* (S) */
  صحیح lastlinedefined;	/* (S) */
  غیرعلامتی حرف nups;	/* (u) number of upvalues */
  غیرعلامتی حرف nparams;/* (u) number of parameters */
  حرف isvararg;        /* (u) */
  غیرعلامتی حرف extraargs;  /* (t) number of extra arguments */
  حرف istailcall;	/* (t) */
  صحیح ftransfer;   /* (r) index of first value transferred */
  صحیح ntransfer;   /* (r) number of transferred values */
  حرف short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  ساخت CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#تعریف_کرو LUAI_TOSTRAUX(x)	#x
#تعریف_کرو LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#تعریف_کرو LUA_نسخہ_MAJOR	LUAI_TOSTR(LUA_نسخہ_MAJOR_N)
#تعریف_کرو LUA_نسخہ_MINOR	LUAI_TOSTR(LUA_نسخہ_MINOR_N)
#تعریف_کرو LUA_نسخہ_RELEASE	LUAI_TOSTR(LUA_نسخہ_RELEASE_N)

#تعریف_کرو LUA_نسخہ	"Lua " LUA_نسخہ_MAJOR "." LUA_نسخہ_MINOR
#تعریف_کرو LUA_RELEASE	LUA_نسخہ "." LUA_نسخہ_RELEASE


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


#اگر_ختم
