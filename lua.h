/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stddef.h>


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	1

#define LUA_VERSION_અંક  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_અંક  (LUA_VERSION_અંક * 100 + LUA_VERSION_RELEASE_N)


#અન્તર્ભાવ "luaconf.h"


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


પ્રકારવ્યાખ્યા સંરચના lua_State lua_State;


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
પ્રકારવ્યાખ્યા LUA_સંખ્યા lua_સંખ્યા;


/* type for integer functions */
પ્રકારવ્યાખ્યા LUA_INTEGER lua_Integer;

/* unsigned integer type */
પ્રકારવ્યાખ્યા LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
પ્રકારવ્યાખ્યા LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
પ્રકારવ્યાખ્યા પૂર્ણાંક (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
પ્રકારવ્યાખ્યા પૂર્ણાંક (*lua_KFunction) (lua_State *L, પૂર્ણાંક status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
પ્રકારવ્યાખ્યા અચલ અક્ષર * (*lua_Reader) (lua_State *L, શૂન્ય *ud, size_t *sz);

પ્રકારવ્યાખ્યા પૂર્ણાંક (*lua_Writer) (lua_State *L, અચલ શૂન્ય *p, size_t sz, શૂન્ય *ud);


/*
** Type for memory-allocation functions
*/
પ્રકારવ્યાખ્યા શૂન્ય * (*lua_Alloc) (શૂન્ય *ud, શૂન્ય *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
પ્રકારવ્યાખ્યા શૂન્ય (*lua_WarnFunction) (શૂન્ય *ud, અચલ અક્ષર *સંદેશ, પૂર્ણાંક tocont);


/*
** Type used by the debug API to collect debug information
*/
પ્રકારવ્યાખ્યા સંરચના lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
પ્રકારવ્યાખ્યા શૂન્ય (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#જો defined(LUA_USER_H)
#અન્તર્ભાવ LUA_USER_H
#endif


/*
** RCS ident string
*/
બાહ્ય અચલ અક્ષર lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, શૂન્ય *ud, unsigned seed);
LUA_API શૂન્ય       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API પૂર્ણાંક        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_સંખ્યા (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API પૂર્ણાંક   (lua_absindex) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક   (lua_gettop) (lua_State *L);
LUA_API શૂન્ય  (lua_settop) (lua_State *L, પૂર્ણાંક idx);
LUA_API શૂન્ય  (lua_pushvalue) (lua_State *L, પૂર્ણાંક idx);
LUA_API શૂન્ય  (lua_rotate) (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક n);
LUA_API શૂન્ય  (lua_copy) (lua_State *L, પૂર્ણાંક fromidx, પૂર્ણાંક toidx);
LUA_API પૂર્ણાંક   (lua_checkstack) (lua_State *L, પૂર્ણાંક n);

LUA_API શૂન્ય  (lua_xmove) (lua_State *from, lua_State *to, પૂર્ણાંક n);


/*
** access functions (stack -> C)
*/

LUA_API પૂર્ણાંક             (lua_isnumber) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક             (lua_isstring) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક             (lua_iscfunction) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક             (lua_isinteger) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક             (lua_isuserdata) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક             (lua_type) (lua_State *L, પૂર્ણાંક idx);
LUA_API અચલ અક્ષર     *(lua_typename) (lua_State *L, પૂર્ણાંક tp);

LUA_API lua_સંખ્યા      (lua_tonumberx) (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક *isnum);
LUA_API પૂર્ણાંક             (lua_toboolean) (lua_State *L, પૂર્ણાંક idx);
LUA_API અચલ અક્ષર     *(lua_tolstring) (lua_State *L, પૂર્ણાંક idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, પૂર્ણાંક idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, પૂર્ણાંક idx);
LUA_API શૂન્ય	       *(lua_touserdata) (lua_State *L, પૂર્ણાંક idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, પૂર્ણાંક idx);
LUA_API અચલ શૂન્ય     *(lua_topointer) (lua_State *L, પૂર્ણાંક idx);


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

LUA_API શૂન્ય  (lua_arith) (lua_State *L, પૂર્ણાંક op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API પૂર્ણાંક   (lua_rawequal) (lua_State *L, પૂર્ણાંક idx1, પૂર્ણાંક idx2);
LUA_API પૂર્ણાંક   (lua_compare) (lua_State *L, પૂર્ણાંક idx1, પૂર્ણાંક idx2, પૂર્ણાંક op);


/*
** push functions (C -> stack)
*/
LUA_API શૂન્ય        (lua_pushnil) (lua_State *L);
LUA_API શૂન્ય        (lua_pushnumber) (lua_State *L, lua_સંખ્યા n);
LUA_API શૂન્ય        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API અચલ અક્ષર *(lua_pushlstring) (lua_State *L, અચલ અક્ષર *s, size_t len);
LUA_API અચલ અક્ષર *(lua_pushexternalstring) (lua_State *L,
		અચલ અક્ષર *s, size_t len, lua_Alloc falloc, શૂન્ય *ud);
LUA_API અચલ અક્ષર *(lua_pushstring) (lua_State *L, અચલ અક્ષર *s);
LUA_API અચલ અક્ષર *(lua_pushvfstring) (lua_State *L, અચલ અક્ષર *fmt,
                                                      va_list argp);
LUA_API અચલ અક્ષર *(lua_pushfstring) (lua_State *L, અચલ અક્ષર *fmt, ...);
LUA_API શૂન્ય  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, પૂર્ણાંક n);
LUA_API શૂન્ય  (lua_pushboolean) (lua_State *L, પૂર્ણાંક b);
LUA_API શૂન્ય  (lua_pushlightuserdata) (lua_State *L, શૂન્ય *p);
LUA_API પૂર્ણાંક   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API પૂર્ણાંક (lua_getglobal) (lua_State *L, અચલ અક્ષર *name);
LUA_API પૂર્ણાંક (lua_gettable) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક (lua_getfield) (lua_State *L, પૂર્ણાંક idx, અચલ અક્ષર *k);
LUA_API પૂર્ણાંક (lua_geti) (lua_State *L, પૂર્ણાંક idx, lua_Integer n);
LUA_API પૂર્ણાંક (lua_rawget) (lua_State *L, પૂર્ણાંક idx);
LUA_API પૂર્ણાંક (lua_rawgeti) (lua_State *L, પૂર્ણાંક idx, lua_Integer n);
LUA_API પૂર્ણાંક (lua_rawgetp) (lua_State *L, પૂર્ણાંક idx, અચલ શૂન્ય *p);

LUA_API શૂન્ય  (lua_createtable) (lua_State *L, પૂર્ણાંક narr, પૂર્ણાંક nrec);
LUA_API શૂન્ય *(lua_newuserdatauv) (lua_State *L, size_t sz, પૂર્ણાંક nuvalue);
LUA_API પૂર્ણાંક   (lua_getmetatable) (lua_State *L, પૂર્ણાંક objindex);
LUA_API પૂર્ણાંક  (lua_getiuservalue) (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક n);


/*
** set functions (stack -> Lua)
*/
LUA_API શૂન્ય  (lua_setglobal) (lua_State *L, અચલ અક્ષર *name);
LUA_API શૂન્ય  (lua_settable) (lua_State *L, પૂર્ણાંક idx);
LUA_API શૂન્ય  (lua_setfield) (lua_State *L, પૂર્ણાંક idx, અચલ અક્ષર *k);
LUA_API શૂન્ય  (lua_seti) (lua_State *L, પૂર્ણાંક idx, lua_Integer n);
LUA_API શૂન્ય  (lua_rawset) (lua_State *L, પૂર્ણાંક idx);
LUA_API શૂન્ય  (lua_rawseti) (lua_State *L, પૂર્ણાંક idx, lua_Integer n);
LUA_API શૂન્ય  (lua_rawsetp) (lua_State *L, પૂર્ણાંક idx, અચલ શૂન્ય *p);
LUA_API પૂર્ણાંક   (lua_setmetatable) (lua_State *L, પૂર્ણાંક objindex);
LUA_API પૂર્ણાંક   (lua_setiuservalue) (lua_State *L, પૂર્ણાંક idx, પૂર્ણાંક n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API શૂન્ય  (lua_callk) (lua_State *L, પૂર્ણાંક nargs, પૂર્ણાંક nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, ખાલી)

LUA_API પૂર્ણાંક   (lua_pcallk) (lua_State *L, પૂર્ણાંક nargs, પૂર્ણાંક nresults, પૂર્ણાંક errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, ખાલી)

LUA_API પૂર્ણાંક   (lua_load) (lua_State *L, lua_Reader reader, શૂન્ય *dt,
                          અચલ અક્ષર *chunkname, અચલ અક્ષર *mode);

LUA_API પૂર્ણાંક (lua_dump) (lua_State *L, lua_Writer writer, શૂન્ય *data, પૂર્ણાંક strip);


/*
** coroutine functions
*/
LUA_API પૂર્ણાંક  (lua_yieldk)     (lua_State *L, પૂર્ણાંક nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API પૂર્ણાંક  (lua_resume)     (lua_State *L, lua_State *from, પૂર્ણાંક narg,
                               પૂર્ણાંક *nres);
LUA_API પૂર્ણાંક  (lua_status)     (lua_State *L);
LUA_API પૂર્ણાંક (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, ખાલી)


/*
** Warning-related functions
*/
LUA_API શૂન્ય (lua_setwarnf) (lua_State *L, lua_WarnFunction f, શૂન્ય *ud);
LUA_API શૂન્ય (lua_warning)  (lua_State *L, અચલ અક્ષર *સંદેશ, પૂર્ણાંક tocont);


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


LUA_API પૂર્ણાંક (lua_gc) (lua_State *L, પૂર્ણાંક what, ...);


/*
** miscellaneous functions
*/

LUA_API પૂર્ણાંક   (lua_error) (lua_State *L);

LUA_API પૂર્ણાંક   (lua_next) (lua_State *L, પૂર્ણાંક idx);

LUA_API શૂન્ય  (lua_concat) (lua_State *L, પૂર્ણાંક n);
LUA_API શૂન્ય  (lua_len)    (lua_State *L, પૂર્ણાંક idx);

#define LUA_N2SBUFFSZ	64
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, પૂર્ણાંક idx, અક્ષર *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, અચલ અક્ષર *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, શૂન્ય **ud);
LUA_API શૂન્ય      (lua_setallocf) (lua_State *L, lua_Alloc f, શૂન્ય *ud);

LUA_API શૂન્ય (lua_toclose) (lua_State *L, પૂર્ણાંક idx);
LUA_API શૂન્ય (lua_closeslot) (lua_State *L, પૂર્ણાંક idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((શૂન્ય *)((અક્ષર *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),ખાલી)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),ખાલી)

#define lua_કાઢો(L,n)		lua_settop(L, -(n)-1)

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
	((શૂન્ય)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), ખાલી)


#define lua_ઉમેરો(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_કાઢો(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_કાઢો(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,ખાલી)

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


LUA_API પૂર્ણાંક (lua_getstack) (lua_State *L, પૂર્ણાંક level, lua_Debug *ar);
LUA_API પૂર્ણાંક (lua_getinfo) (lua_State *L, અચલ અક્ષર *what, lua_Debug *ar);
LUA_API અચલ અક્ષર *(lua_getlocal) (lua_State *L, અચલ lua_Debug *ar, પૂર્ણાંક n);
LUA_API અચલ અક્ષર *(lua_setlocal) (lua_State *L, અચલ lua_Debug *ar, પૂર્ણાંક n);
LUA_API અચલ અક્ષર *(lua_getupvalue) (lua_State *L, પૂર્ણાંક funcindex, પૂર્ણાંક n);
LUA_API અચલ અક્ષર *(lua_setupvalue) (lua_State *L, પૂર્ણાંક funcindex, પૂર્ણાંક n);

LUA_API શૂન્ય *(lua_upvalueid) (lua_State *L, પૂર્ણાંક fidx, પૂર્ણાંક n);
LUA_API શૂન્ય  (lua_upvaluejoin) (lua_State *L, પૂર્ણાંક fidx1, પૂર્ણાંક n1,
                                               પૂર્ણાંક fidx2, પૂર્ણાંક n2);

LUA_API શૂન્ય (lua_sethook) (lua_State *L, lua_Hook func, પૂર્ણાંક mask, પૂર્ણાંક ગણતરી);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API પૂર્ણાંક (lua_gethookmask) (lua_State *L);
LUA_API પૂર્ણાંક (lua_gethookcount) (lua_State *L);


સંરચના lua_Debug {
  પૂર્ણાંક event;
  અચલ અક્ષર *name;	/* (n) */
  અચલ અક્ષર *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  અચલ અક્ષર *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  અચલ અક્ષર *source;	/* (S) */
  size_t srclen;	/* (S) */
  પૂર્ણાંક currentline;	/* (l) */
  પૂર્ણાંક linedefined;	/* (S) */
  પૂર્ણાંક lastlinedefined;	/* (S) */
  unsigned અક્ષર nups;	/* (u) number of upvalues */
  unsigned અક્ષર nparams;/* (u) number of parameters */
  અક્ષર isvararg;        /* (u) */
  unsigned અક્ષર extraargs;  /* (t) number of extra arguments */
  અક્ષર istailcall;	/* (t) */
  પૂર્ણાંક ftransfer;   /* (r) index of first value transferred */
  પૂર્ણાંક ntransfer;   /* (r) number of transferred values */
  અક્ષર short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  સંરચના CallInfo *i_ci;  /* active function */
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
