/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lua_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_h

#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_MAJOR_N	5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_MINOR_N	5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_RELEASE_N	1

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_ਅੰਕ  (LUA_ਸੰਸਕਰਣ_MAJOR_N * 100 + LUA_ਸੰਸਕਰਣ_MINOR_N)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_RELEASE_ਅੰਕ  (LUA_ਸੰਸਕਰਣ_ਅੰਕ * 100 + LUA_ਸੰਸਕਰਣ_RELEASE_N)


#ਅਨ੍ਤਰ੍ਭਾਵ "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_REGISTRYINDEX	(-(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ/2 + 1000))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OK		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_YIELD	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ERRRUN	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ERRSYNTAX	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ERRMEM	4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ERRERR	5


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ lua_State lua_State;


/*
** basic types
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TNONE		(-1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TNIL		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TBOOLEAN		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TLIGHTUSERDATA	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TNUMBER		3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TSTRING		4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TTABLE		5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TFUNCTION		6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TUSERDATA		7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TTHREAD		8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_RIDX_GLOBALS	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_RIDX_MAINTHREAD	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_RIDX_ਆਖਰੀ		3


/* type of numbers in Lua */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUA_ਸੰਖਿਆ lua_ਸੰਖਿਆ;


/* type for integer functions */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUA_INTEGER lua_Integer;

/* unsigned integer type */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*lua_KFunction) (lua_State *L, ਪੂਰਨ ਸਥਿਤੀ, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਚਲ ਅੱਖਰ * (*lua_Reader) (lua_State *L, ਖਾਲੀ *ud, size_t *sz);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*lua_Writer) (lua_State *L, ਅਚਲ ਖਾਲੀ *p, size_t sz, ਖਾਲੀ *ud);


/*
** Type for memory-allocation functions
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ * (*lua_Alloc) (ਖਾਲੀ *ud, ਖਾਲੀ *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*lua_WarnFunction) (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ tocont);


/*
** Type used by the debug API to collect debug information
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#ਜੇ defined(LUA_USER_H)
#ਅਨ੍ਤਰ੍ਭਾਵ LUA_USER_H
#ਜੇ_ਅੰਤ


/*
** RCS ident string
*/
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, ਖਾਲੀ *ud, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬੀਜ);
LUA_API ਖਾਲੀ       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API ਪੂਰਨ        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_ਸੰਖਿਆ (lua_ਸੰਸਕਰਣ) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API ਪੂਰਨ   (lua_absindex) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ   (lua_gettop) (lua_State *L);
LUA_API ਖਾਲੀ  (lua_settop) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਖਾਲੀ  (lua_pushvalue) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਖਾਲੀ  (lua_rotate) (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ n);
LUA_API ਖਾਲੀ  (lua_copy) (lua_State *L, ਪੂਰਨ fromidx, ਪੂਰਨ toidx);
LUA_API ਪੂਰਨ   (lua_checkstack) (lua_State *L, ਪੂਰਨ n);

LUA_API ਖਾਲੀ  (lua_xmove) (lua_State *from, lua_State *ਮੰਜ਼ਿਲ, ਪੂਰਨ n);


/*
** access functions (stack -> C)
*/

LUA_API ਪੂਰਨ             (lua_isnumber) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ             (lua_isstring) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ             (lua_iscfunction) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ             (lua_isinteger) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ             (lua_isuserdata) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ             (lua_type) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਅਚਲ ਅੱਖਰ     *(lua_typename) (lua_State *L, ਪੂਰਨ tp);

LUA_API lua_ਸੰਖਿਆ      (lua_tonumberx) (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ *isnum);
LUA_API ਪੂਰਨ             (lua_toboolean) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਅਚਲ ਅੱਖਰ     *(lua_tolstring) (lua_State *L, ਪੂਰਨ idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, ਪੂਰਨ idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਖਾਲੀ	       *(lua_touserdata) (lua_State *L, ਪੂਰਨ idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਅਚਲ ਖਾਲੀ     *(lua_topointer) (lua_State *L, ਪੂਰਨ idx);


/*
** Comparison and arithmetic functions
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPADD	0	/* ORDER TM, ORDER OP */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPSUB	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPMUL	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPMOD	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPPOW	4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPDIV	5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPIDIV	6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPBAND	7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPBOR	8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPBXOR	9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPSHL	10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPSHR	11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPUNM	12
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPBNOT	13

LUA_API ਖਾਲੀ  (lua_arith) (lua_State *L, ਪੂਰਨ op);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPEQ	0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPLT	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OPLE	2

LUA_API ਪੂਰਨ   (lua_rawequal) (lua_State *L, ਪੂਰਨ idx1, ਪੂਰਨ idx2);
LUA_API ਪੂਰਨ   (lua_compare) (lua_State *L, ਪੂਰਨ idx1, ਪੂਰਨ idx2, ਪੂਰਨ op);


/*
** push functions (C -> stack)
*/
LUA_API ਖਾਲੀ        (lua_pushnil) (lua_State *L);
LUA_API ਖਾਲੀ        (lua_pushnumber) (lua_State *L, lua_ਸੰਖਿਆ n);
LUA_API ਖਾਲੀ        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_pushlstring) (lua_State *L, ਅਚਲ ਅੱਖਰ *s, size_t len);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_pushexternalstring) (lua_State *L,
		ਅਚਲ ਅੱਖਰ *s, size_t len, lua_Alloc falloc, ਖਾਲੀ *ud);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_pushstring) (lua_State *L, ਅਚਲ ਅੱਖਰ *s);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_pushvfstring) (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt,
                                                      va_list argp);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_pushfstring) (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...);
LUA_API ਖਾਲੀ  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, ਪੂਰਨ n);
LUA_API ਖਾਲੀ  (lua_pushboolean) (lua_State *L, ਪੂਰਨ b);
LUA_API ਖਾਲੀ  (lua_pushlightuserdata) (lua_State *L, ਖਾਲੀ *p);
LUA_API ਪੂਰਨ   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API ਪੂਰਨ (lua_getglobal) (lua_State *L, ਅਚਲ ਅੱਖਰ *name);
LUA_API ਪੂਰਨ (lua_gettable) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ (lua_getfield) (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *k);
LUA_API ਪੂਰਨ (lua_geti) (lua_State *L, ਪੂਰਨ idx, lua_Integer n);
LUA_API ਪੂਰਨ (lua_rawget) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਪੂਰਨ (lua_rawgeti) (lua_State *L, ਪੂਰਨ idx, lua_Integer n);
LUA_API ਪੂਰਨ (lua_rawgetp) (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਖਾਲੀ *p);

LUA_API ਖਾਲੀ  (lua_createtable) (lua_State *L, ਪੂਰਨ narr, ਪੂਰਨ nrec);
LUA_API ਖਾਲੀ *(lua_newuserdatauv) (lua_State *L, size_t sz, ਪੂਰਨ nuvalue);
LUA_API ਪੂਰਨ   (lua_getmetatable) (lua_State *L, ਪੂਰਨ objindex);
LUA_API ਪੂਰਨ  (lua_getiuservalue) (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ n);


/*
** set functions (stack -> Lua)
*/
LUA_API ਖਾਲੀ  (lua_setglobal) (lua_State *L, ਅਚਲ ਅੱਖਰ *name);
LUA_API ਖਾਲੀ  (lua_settable) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਖਾਲੀ  (lua_setfield) (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *k);
LUA_API ਖਾਲੀ  (lua_seti) (lua_State *L, ਪੂਰਨ idx, lua_Integer n);
LUA_API ਖਾਲੀ  (lua_rawset) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਖਾਲੀ  (lua_rawseti) (lua_State *L, ਪੂਰਨ idx, lua_Integer n);
LUA_API ਖਾਲੀ  (lua_rawsetp) (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਖਾਲੀ *p);
LUA_API ਪੂਰਨ   (lua_setmetatable) (lua_State *L, ਪੂਰਨ objindex);
LUA_API ਪੂਰਨ   (lua_setiuservalue) (lua_State *L, ਪੂਰਨ idx, ਪੂਰਨ n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API ਖਾਲੀ  (lua_callk) (lua_State *L, ਪੂਰਨ nargs, ਪੂਰਨ nresults,
                           lua_KContext ctx, lua_KFunction k);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_call(L,n,r)		lua_callk(L, (n), (r), 0, ਨਿਰਾ)

LUA_API ਪੂਰਨ   (lua_pcallk) (lua_State *L, ਪੂਰਨ nargs, ਪੂਰਨ nresults, ਪੂਰਨ errfunc,
                            lua_KContext ctx, lua_KFunction k);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, ਨਿਰਾ)

LUA_API ਪੂਰਨ   (lua_ਲੱਦੋ) (lua_State *L, lua_Reader reader, ਖਾਲੀ *dt,
                          ਅਚਲ ਅੱਖਰ *chunkname, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ);

LUA_API ਪੂਰਨ (lua_dump) (lua_State *L, lua_Writer writer, ਖਾਲੀ *ਅੰਕੜੇ, ਪੂਰਨ strip);


/*
** coroutine functions
*/
LUA_API ਪੂਰਨ  (lua_yieldk)     (lua_State *L, ਪੂਰਨ nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API ਪੂਰਨ  (lua_resume)     (lua_State *L, lua_State *from, ਪੂਰਨ narg,
                               ਪੂਰਨ *nres);
LUA_API ਪੂਰਨ  (lua_ਸਥਿਤੀ)     (lua_State *L);
LUA_API ਪੂਰਨ (lua_isyieldable) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_yield(L,n)		lua_yieldk(L, (n), 0, ਨਿਰਾ)


/*
** Warning-related functions
*/
LUA_API ਖਾਲੀ (lua_setwarnf) (lua_State *L, lua_WarnFunction f, ਖਾਲੀ *ud);
LUA_API ਖਾਲੀ (lua_warning)  (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ tocont);


/*
** garbage-collection options
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCSTOP		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCRESTART		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCCOLLECT		2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCCOUNT		3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCCOUNTB		4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCSTEP		5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCISRUNNING		6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCGEN		7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCINC		8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPMINORMUL		0  /* control minor collections */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPSTEPMUL		4  /* GC "speed" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GCPN		6


LUA_API ਪੂਰਨ (lua_gc) (lua_State *L, ਪੂਰਨ what, ...);


/*
** miscellaneous functions
*/

LUA_API ਪੂਰਨ   (lua_error) (lua_State *L);

LUA_API ਪੂਰਨ   (lua_ਅਗਲਾ) (lua_State *L, ਪੂਰਨ idx);

LUA_API ਖਾਲੀ  (lua_concat) (lua_State *L, ਪੂਰਨ n);
LUA_API ਖਾਲੀ  (lua_len)    (lua_State *L, ਪੂਰਨ idx);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_N2SBUFFSZ	64
LUA_API ਬਿਨਾਨਿਸ਼ਾਨੀ  (lua_numbertocstring) (lua_State *L, ਪੂਰਨ idx, ਅੱਖਰ *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ਅਚਲ ਅੱਖਰ *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, ਖਾਲੀ **ud);
LUA_API ਖਾਲੀ      (lua_setallocf) (lua_State *L, lua_Alloc f, ਖਾਲੀ *ud);

LUA_API ਖਾਲੀ (lua_toclose) (lua_State *L, ਪੂਰਨ idx);
LUA_API ਖਾਲੀ (lua_closeslot) (lua_State *L, ਪੂਰਨ idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_getextraspace(L)	((ਖਾਲੀ *)((ਅੱਖਰ *)(L) - LUA_EXTRASPACE))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_tonumber(L,i)	lua_tonumberx(L,(i),ਨਿਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_tointeger(L,i)	lua_tointegerx(L,(i),ਨਿਰਾ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_ਕੱਢੋ(L,n)		lua_settop(L, -(n)-1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_newtable(L)		lua_createtable(L, 0, 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_ਰਜਿਸਟਰ(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_pushglobaltable(L)  \
	((ਖਾਲੀ)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_tostring(L,i)	lua_tolstring(L, (i), ਨਿਰਾ)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_ਪਾਓ(L,idx)	lua_rotate(L, (idx), 1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_ਕੱਢੋ(L, 1))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_ਕੱਢੋ(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_resetthread(L)	lua_closethread(L,ਨਿਰਾ)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_HOOKCALL	0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_HOOKRET	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_HOOKLINE	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_HOOKCOUNT	3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MASKCALL	(1 << LUA_HOOKCALL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MASKRET	(1 << LUA_HOOKRET)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MASKLINE	(1 << LUA_HOOKLINE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API ਪੂਰਨ (lua_getstack) (lua_State *L, ਪੂਰਨ level, lua_Debug *ar);
LUA_API ਪੂਰਨ (lua_getinfo) (lua_State *L, ਅਚਲ ਅੱਖਰ *what, lua_Debug *ar);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_getlocal) (lua_State *L, ਅਚਲ lua_Debug *ar, ਪੂਰਨ n);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_setlocal) (lua_State *L, ਅਚਲ lua_Debug *ar, ਪੂਰਨ n);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_getupvalue) (lua_State *L, ਪੂਰਨ funcindex, ਪੂਰਨ n);
LUA_API ਅਚਲ ਅੱਖਰ *(lua_setupvalue) (lua_State *L, ਪੂਰਨ funcindex, ਪੂਰਨ n);

LUA_API ਖਾਲੀ *(lua_upvalueid) (lua_State *L, ਪੂਰਨ fidx, ਪੂਰਨ n);
LUA_API ਖਾਲੀ  (lua_upvaluejoin) (lua_State *L, ਪੂਰਨ fidx1, ਪੂਰਨ n1,
                                               ਪੂਰਨ fidx2, ਪੂਰਨ n2);

LUA_API ਖਾਲੀ (lua_sethook) (lua_State *L, lua_Hook func, ਪੂਰਨ ਨਕਾਬ_2, ਪੂਰਨ ਗਿਣਤੀ_2);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API ਪੂਰਨ (lua_gethookmask) (lua_State *L);
LUA_API ਪੂਰਨ (lua_gethookcount) (lua_State *L);


ਬਣਤਰ lua_Debug {
  ਪੂਰਨ event;
  ਅਚਲ ਅੱਖਰ *name;	/* (n) */
  ਅਚਲ ਅੱਖਰ *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ਅਚਲ ਅੱਖਰ *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ਅਚਲ ਅੱਖਰ *ਸਰੋਤ;	/* (S) */
  size_t srclen;	/* (S) */
  ਪੂਰਨ currentline;	/* (l) */
  ਪੂਰਨ linedefined;	/* (S) */
  ਪੂਰਨ lastlinedefined;	/* (S) */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ nups;	/* (u) number of upvalues */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ nparams;/* (u) number of parameters */
  ਅੱਖਰ isvararg;        /* (u) */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ extraargs;  /* (t) number of extra arguments */
  ਅੱਖਰ istailcall;	/* (t) */
  ਪੂਰਨ ftransfer;   /* (r) index of first value transferred */
  ਪੂਰਨ ntransfer;   /* (r) number of transferred values */
  ਅੱਖਰ short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  ਬਣਤਰ CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_TOSTRAUX(x)	#x
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_MAJOR	LUAI_TOSTR(LUA_ਸੰਸਕਰਣ_MAJOR_N)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_MINOR	LUAI_TOSTR(LUA_ਸੰਸਕਰਣ_MINOR_N)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ_RELEASE	LUAI_TOSTR(LUA_ਸੰਸਕਰਣ_RELEASE_N)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਸੰਸਕਰਣ	"Lua " LUA_ਸੰਸਕਰਣ_MAJOR "." LUA_ਸੰਸਕਰਣ_MINOR
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_RELEASE	LUA_ਸੰਸਕਰਣ "." LUA_ਸੰਸਕਰਣ_RELEASE


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


#ਜੇ_ਅੰਤ
