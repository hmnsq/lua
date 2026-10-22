/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ifndef lua_h
#define lua_h

#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>


#define LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#define LUA_VERSION_MAJOR_N	5
#define LUA_VERSION_MINOR_N	5
#define LUA_VERSION_RELEASE_N	1

#define LUA_VERSION_ಅಂಕೆ  (LUA_VERSION_MAJOR_N * 100 + LUA_VERSION_MINOR_N)
#define LUA_VERSION_RELEASE_ಅಂಕೆ  (LUA_VERSION_ಅಂಕೆ * 100 + LUA_VERSION_RELEASE_N)


#ಅನ್ತರ್ಭಾವ "luaconf.h"


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


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ lua_State lua_State;


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
ಪ್ರಕಾರನಿರ್ದೇಶ LUA_ಸಂಖ್ಯೆ lua_ಸಂಖ್ಯೆ;


/* type for integer functions */
ಪ್ರಕಾರನಿರ್ದೇಶ LUA_INTEGER lua_Integer;

/* unsigned integer type */
ಪ್ರಕಾರನಿರ್ದೇಶ LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
ಪ್ರಕಾರನಿರ್ದೇಶ LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ (*lua_KFunction) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಧ್ರುವ ವರ್ಣ * (*lua_Reader) (lua_State *L, ಶೂನ್ಯ *ud, size_t *sz);

ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ (*lua_Writer) (lua_State *L, ಧ್ರುವ ಶೂನ್ಯ *p, size_t sz, ಶೂನ್ಯ *ud);


/*
** Type for memory-allocation functions
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ * (*lua_Alloc) (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ (*lua_WarnFunction) (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ, ಪೂರ್ಣಾಙ್ಕ tocont);


/*
** Type used by the debug API to collect debug information
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#ಯದಿ defined(LUA_USER_H)
#ಅನ್ತರ್ಭಾವ LUA_USER_H
#endif


/*
** RCS ident string
*/
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, ಶೂನ್ಯ *ud, unsigned seed);
LUA_API ಶೂನ್ಯ       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API ಪೂರ್ಣಾಙ್ಕ        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_ಸಂಖ್ಯೆ (lua_version) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_absindex) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_gettop) (lua_State *L);
LUA_API ಶೂನ್ಯ  (lua_settop) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಶೂನ್ಯ  (lua_pushvalue) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಶೂನ್ಯ  (lua_rotate) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ n);
LUA_API ಶೂನ್ಯ  (lua_copy) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ fromidx, ಪೂರ್ಣಾಙ್ಕ toidx);
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_checkstack) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ n);

LUA_API ಶೂನ್ಯ  (lua_xmove) (lua_State *from, lua_State *to, ಪೂರ್ಣಾಙ್ಕ n);


/*
** access functions (stack -> C)
*/

LUA_API ಪೂರ್ಣಾಙ್ಕ             (lua_isnumber) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ             (lua_isstring) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ             (lua_iscfunction) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ             (lua_isinteger) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ             (lua_isuserdata) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ             (lua_type) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಧ್ರುವ ವರ್ಣ     *(lua_typename) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ tp);

LUA_API lua_ಸಂಖ್ಯೆ      (lua_tonumberx) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ *isnum);
LUA_API ಪೂರ್ಣಾಙ್ಕ             (lua_toboolean) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಧ್ರುವ ವರ್ಣ     *(lua_tolstring) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಶೂನ್ಯ	       *(lua_touserdata) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಧ್ರುವ ಶೂನ್ಯ     *(lua_topointer) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);


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

LUA_API ಶೂನ್ಯ  (lua_arith) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ op);

#define LUA_OPEQ	0
#define LUA_OPLT	1
#define LUA_OPLE	2

LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_rawequal) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx1, ಪೂರ್ಣಾಙ್ಕ idx2);
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_compare) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx1, ಪೂರ್ಣಾಙ್ಕ idx2, ಪೂರ್ಣಾಙ್ಕ op);


/*
** push functions (C -> stack)
*/
LUA_API ಶೂನ್ಯ        (lua_pushnil) (lua_State *L);
LUA_API ಶೂನ್ಯ        (lua_pushnumber) (lua_State *L, lua_ಸಂಖ್ಯೆ n);
LUA_API ಶೂನ್ಯ        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_pushlstring) (lua_State *L, ಧ್ರುವ ವರ್ಣ *s, size_t len);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_pushexternalstring) (lua_State *L,
		ಧ್ರುವ ವರ್ಣ *s, size_t len, lua_Alloc falloc, ಶೂನ್ಯ *ud);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_pushstring) (lua_State *L, ಧ್ರುವ ವರ್ಣ *s);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_pushvfstring) (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt,
                                                      va_list argp);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_pushfstring) (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...);
LUA_API ಶೂನ್ಯ  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, ಪೂರ್ಣಾಙ್ಕ n);
LUA_API ಶೂನ್ಯ  (lua_pushboolean) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ b);
LUA_API ಶೂನ್ಯ  (lua_pushlightuserdata) (lua_State *L, ಶೂನ್ಯ *p);
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_getglobal) (lua_State *L, ಧ್ರುವ ವರ್ಣ *name);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_gettable) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_getfield) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *k);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_geti) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_rawget) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_rawgeti) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_rawgetp) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ಶೂನ್ಯ *p);

LUA_API ಶೂನ್ಯ  (lua_createtable) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ narr, ಪೂರ್ಣಾಙ್ಕ nrec);
LUA_API ಶೂನ್ಯ *(lua_newuserdatauv) (lua_State *L, size_t sz, ಪೂರ್ಣಾಙ್ಕ nuvalue);
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_getmetatable) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ objindex);
LUA_API ಪೂರ್ಣಾಙ್ಕ  (lua_getiuservalue) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ n);


/*
** set functions (stack -> Lua)
*/
LUA_API ಶೂನ್ಯ  (lua_setglobal) (lua_State *L, ಧ್ರುವ ವರ್ಣ *name);
LUA_API ಶೂನ್ಯ  (lua_settable) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಶೂನ್ಯ  (lua_setfield) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *k);
LUA_API ಶೂನ್ಯ  (lua_seti) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n);
LUA_API ಶೂನ್ಯ  (lua_rawset) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಶೂನ್ಯ  (lua_rawseti) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, lua_Integer n);
LUA_API ಶೂನ್ಯ  (lua_rawsetp) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ಶೂನ್ಯ *p);
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_setmetatable) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ objindex);
LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_setiuservalue) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಪೂರ್ಣಾಙ್ಕ n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API ಶೂನ್ಯ  (lua_callk) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nargs, ಪೂರ್ಣಾಙ್ಕ nresults,
                           lua_KContext ctx, lua_KFunction k);
#define lua_call(L,n,r)		lua_callk(L, (n), (r), 0, ರಿಕ್ತ)

LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_pcallk) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nargs, ಪೂರ್ಣಾಙ್ಕ nresults, ಪೂರ್ಣಾಙ್ಕ errfunc,
                            lua_KContext ctx, lua_KFunction k);
#define lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, ರಿಕ್ತ)

LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_load) (lua_State *L, lua_Reader reader, ಶೂನ್ಯ *dt,
                          ಧ್ರುವ ವರ್ಣ *chunkname, ಧ್ರುವ ವರ್ಣ *mode);

LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_dump) (lua_State *L, lua_Writer writer, ಶೂನ್ಯ *data, ಪೂರ್ಣಾಙ್ಕ strip);


/*
** coroutine functions
*/
LUA_API ಪೂರ್ಣಾಙ್ಕ  (lua_yieldk)     (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API ಪೂರ್ಣಾಙ್ಕ  (lua_resume)     (lua_State *L, lua_State *from, ಪೂರ್ಣಾಙ್ಕ narg,
                               ಪೂರ್ಣಾಙ್ಕ *nres);
LUA_API ಪೂರ್ಣಾಙ್ಕ  (lua_status)     (lua_State *L);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_isyieldable) (lua_State *L);

#define lua_yield(L,n)		lua_yieldk(L, (n), 0, ರಿಕ್ತ)


/*
** Warning-related functions
*/
LUA_API ಶೂನ್ಯ (lua_setwarnf) (lua_State *L, lua_WarnFunction f, ಶೂನ್ಯ *ud);
LUA_API ಶೂನ್ಯ (lua_warning)  (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ, ಪೂರ್ಣಾಙ್ಕ tocont);


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


LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_gc) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ what, ...);


/*
** miscellaneous functions
*/

LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_error) (lua_State *L);

LUA_API ಪೂರ್ಣಾಙ್ಕ   (lua_next) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);

LUA_API ಶೂನ್ಯ  (lua_concat) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ n);
LUA_API ಶೂನ್ಯ  (lua_len)    (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);

#define LUA_N2SBUFFSZ	64
LUA_API unsigned  (lua_numbertocstring) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ವರ್ಣ *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ಧ್ರುವ ವರ್ಣ *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, ಶೂನ್ಯ **ud);
LUA_API ಶೂನ್ಯ      (lua_setallocf) (lua_State *L, lua_Alloc f, ಶೂನ್ಯ *ud);

LUA_API ಶೂನ್ಯ (lua_toclose) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);
LUA_API ಶೂನ್ಯ (lua_closeslot) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define lua_getextraspace(L)	((ಶೂನ್ಯ *)((ವರ್ಣ *)(L) - LUA_EXTRASPACE))

#define lua_tonumber(L,i)	lua_tonumberx(L,(i),ರಿಕ್ತ)
#define lua_tointeger(L,i)	lua_tointegerx(L,(i),ರಿಕ್ತ)

#define lua_ತೆಗೆ(L,n)		lua_settop(L, -(n)-1)

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
	((ಶೂನ್ಯ)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define lua_tostring(L,i)	lua_tolstring(L, (i), ರಿಕ್ತ)


#define lua_ಸೇರಿಸು(L,idx)	lua_rotate(L, (idx), 1)

#define lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_ತೆಗೆ(L, 1))

#define lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_ತೆಗೆ(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#define lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#define lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#define lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#define lua_resetthread(L)	lua_closethread(L,ರಿಕ್ತ)

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


LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_getstack) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ level, lua_Debug *ar);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_getinfo) (lua_State *L, ಧ್ರುವ ವರ್ಣ *what, lua_Debug *ar);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_getlocal) (lua_State *L, ಧ್ರುವ lua_Debug *ar, ಪೂರ್ಣಾಙ್ಕ n);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_setlocal) (lua_State *L, ಧ್ರುವ lua_Debug *ar, ಪೂರ್ಣಾಙ್ಕ n);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_getupvalue) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ funcindex, ಪೂರ್ಣಾಙ್ಕ n);
LUA_API ಧ್ರುವ ವರ್ಣ *(lua_setupvalue) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ funcindex, ಪೂರ್ಣಾಙ್ಕ n);

LUA_API ಶೂನ್ಯ *(lua_upvalueid) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ fidx, ಪೂರ್ಣಾಙ್ಕ n);
LUA_API ಶೂನ್ಯ  (lua_upvaluejoin) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ fidx1, ಪೂರ್ಣಾಙ್ಕ n1,
                                               ಪೂರ್ಣಾಙ್ಕ fidx2, ಪೂರ್ಣಾಙ್ಕ n2);

LUA_API ಶೂನ್ಯ (lua_sethook) (lua_State *L, lua_Hook func, ಪೂರ್ಣಾಙ್ಕ mask, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_gethookmask) (lua_State *L);
LUA_API ಪೂರ್ಣಾಙ್ಕ (lua_gethookcount) (lua_State *L);


ಸಂರಚನಾ lua_Debug {
  ಪೂರ್ಣಾಙ್ಕ event;
  ಧ್ರುವ ವರ್ಣ *name;	/* (n) */
  ಧ್ರುವ ವರ್ಣ *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ಧ್ರುವ ವರ್ಣ *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ಧ್ರುವ ವರ್ಣ *source;	/* (S) */
  size_t srclen;	/* (S) */
  ಪೂರ್ಣಾಙ್ಕ currentline;	/* (l) */
  ಪೂರ್ಣಾಙ್ಕ linedefined;	/* (S) */
  ಪೂರ್ಣಾಙ್ಕ lastlinedefined;	/* (S) */
  unsigned ವರ್ಣ nups;	/* (u) number of upvalues */
  unsigned ವರ್ಣ nparams;/* (u) number of parameters */
  ವರ್ಣ isvararg;        /* (u) */
  unsigned ವರ್ಣ extraargs;  /* (t) number of extra arguments */
  ವರ್ಣ istailcall;	/* (t) */
  ಪೂರ್ಣಾಙ್ಕ ftransfer;   /* (r) index of first value transferred */
  ಪೂರ್ಣಾಙ್ಕ ntransfer;   /* (r) number of transferred values */
  ವರ್ಣ short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  ಸಂರಚನಾ CallInfo *i_ci;  /* active function */
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
