/*
** $Id: lua.h $
** Lua - A Scripting Language
** Lua.org, PUC-Rio, Brazil (www.lua.org)
** See Copyright Notice at the end of this file
*/


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lua_h
#ವ್ಯಾಖ್ಯಾನಿಸು lua_h

#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>


#ವ್ಯಾಖ್ಯಾನಿಸು LUA_COPYRIGHT	LUA_RELEASE "  Copyright (C) 1994-2026 Lua.org, PUC-Rio"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo, W. Celes"


#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_MAJOR_N	5
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_MINOR_N	5
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_RELEASE_N	1

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_ಅಂಕೆ  (LUA_ಆವೃತ್ತಿ_MAJOR_N * 100 + LUA_ಆವೃತ್ತಿ_MINOR_N)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_RELEASE_ಅಂಕೆ  (LUA_ಆವೃತ್ತಿ_ಅಂಕೆ * 100 + LUA_ಆವೃತ್ತಿ_RELEASE_N)


#ಅನ್ತರ್ಭಾವ "luaconf.h"


/* mark for precompiled code ('<esc>Lua') */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_SIGNATURE	"\x1bLua"

/* option for multiple returns in 'lua_pcall' and 'lua_call' */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (The stack size is limited to INT_MAX/2; we keep some free empty
** space after that to help overflow detection.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_REGISTRYINDEX	(-(ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ/2 + 1000))
#ವ್ಯಾಖ್ಯಾನಿಸು lua_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OK		0
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_YIELD	1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ERRRUN	2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ERRSYNTAX	3
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ERRMEM	4
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ERRERR	5


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ lua_State lua_State;


/*
** basic types
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TNONE		(-1)

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TNIL		0
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TBOOLEAN		1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TLIGHTUSERDATA	2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TNUMBER		3
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TSTRING		4
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TTABLE		5
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TFUNCTION		6
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TUSERDATA		7
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TTHREAD		8

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_NUMTYPES		9



/* minimum Lua stack available to a C function */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MINSTACK	20


/* predefined values in the registry */
/* index 1 is reserved for the reference mechanism */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_RIDX_GLOBALS	2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_RIDX_MAINTHREAD	3
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_RIDX_ಕೊನೆಯ		3


/* type of numbers in Lua */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUA_ಸಂಖ್ಯೆ lua_ಸಂಖ್ಯೆ;


/* type for integer functions */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUA_INTEGER lua_Integer;

/* unsigned integer type */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUA_UNSIGNED lua_Unsigned;

/* type for continuation-function contexts */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ LUA_KCONTEXT lua_KContext;


/*
** Type for C functions registered with Lua
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*lua_CFunction) (lua_State *L);

/*
** Type for continuation functions
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*lua_KFunction) (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, lua_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping Lua chunks
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * (*lua_Reader) (lua_State *L, ಶೂನ್ಯ *ud, size_t *sz);

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*lua_Writer) (lua_State *L, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p, size_t sz, ಶೂನ್ಯ *ud);


/*
** Type for memory-allocation functions
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ * (*lua_Alloc) (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*lua_WarnFunction) (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ tocont);


/*
** Type used by the debug API to collect debug information
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ lua_Debug lua_Debug;


/*
** Functions to be called by the debugger in specific events
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*lua_Hook) (lua_State *L, lua_Debug *ar);


/*
** generic extra include file
*/
#ಆದರೆ defined(LUA_USER_H)
#ಅನ್ತರ್ಭಾವ LUA_USER_H
#ಆದರೆ_ಅಂತ್ಯ


/*
** RCS ident string
*/
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ lua_ident[];


/*
** state manipulation
*/
LUA_API lua_State *(lua_newstate) (lua_Alloc f, ಶೂನ್ಯ *ud, ಚಿಹ್ನೆರಹಿತ ಬೀಜ);
LUA_API ಶೂನ್ಯ       (lua_close) (lua_State *L);
LUA_API lua_State *(lua_newthread) (lua_State *L);
LUA_API ಪೂರ್ಣಾಂಕ        (lua_closethread) (lua_State *L, lua_State *from);

LUA_API lua_CFunction (lua_atpanic) (lua_State *L, lua_CFunction panicf);


LUA_API lua_ಸಂಖ್ಯೆ (lua_ಆವೃತ್ತಿ) (lua_State *L);


/*
** basic stack manipulation
*/
LUA_API ಪೂರ್ಣಾಂಕ   (lua_absindex) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ   (lua_gettop) (lua_State *L);
LUA_API ಶೂನ್ಯ  (lua_settop) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಶೂನ್ಯ  (lua_pushvalue) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಶೂನ್ಯ  (lua_rotate) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ n);
LUA_API ಶೂನ್ಯ  (lua_copy) (lua_State *L, ಪೂರ್ಣಾಂಕ fromidx, ಪೂರ್ಣಾಂಕ toidx);
LUA_API ಪೂರ್ಣಾಂಕ   (lua_checkstack) (lua_State *L, ಪೂರ್ಣಾಂಕ n);

LUA_API ಶೂನ್ಯ  (lua_xmove) (lua_State *from, lua_State *ಗಮ್ಯ, ಪೂರ್ಣಾಂಕ n);


/*
** access functions (stack -> C)
*/

LUA_API ಪೂರ್ಣಾಂಕ             (lua_isnumber) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ             (lua_isstring) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ             (lua_iscfunction) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ             (lua_isinteger) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ             (lua_isuserdata) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ             (lua_type) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ     *(lua_typename) (lua_State *L, ಪೂರ್ಣಾಂಕ tp);

LUA_API lua_ಸಂಖ್ಯೆ      (lua_tonumberx) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ *isnum);
LUA_API lua_Integer     (lua_tointegerx) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ *isnum);
LUA_API ಪೂರ್ಣಾಂಕ             (lua_toboolean) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ     *(lua_tolstring) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, size_t *len);
LUA_API lua_Unsigned    (lua_rawlen) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API lua_CFunction   (lua_tocfunction) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಶೂನ್ಯ	       *(lua_touserdata) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API lua_State      *(lua_tothread) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ     *(lua_topointer) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);


/*
** Comparison and arithmetic functions
*/

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPADD	0	/* ORDER TM, ORDER OP */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPSUB	1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPMUL	2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPMOD	3
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPPOW	4
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPDIV	5
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPIDIV	6
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPBAND	7
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPBOR	8
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPBXOR	9
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPSHL	10
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPSHR	11
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPUNM	12
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPBNOT	13

LUA_API ಶೂನ್ಯ  (lua_arith) (lua_State *L, ಪೂರ್ಣಾಂಕ op);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPEQ	0
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPLT	1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OPLE	2

LUA_API ಪೂರ್ಣಾಂಕ   (lua_rawequal) (lua_State *L, ಪೂರ್ಣಾಂಕ idx1, ಪೂರ್ಣಾಂಕ idx2);
LUA_API ಪೂರ್ಣಾಂಕ   (lua_compare) (lua_State *L, ಪೂರ್ಣಾಂಕ idx1, ಪೂರ್ಣಾಂಕ idx2, ಪೂರ್ಣಾಂಕ op);


/*
** push functions (C -> stack)
*/
LUA_API ಶೂನ್ಯ        (lua_pushnil) (lua_State *L);
LUA_API ಶೂನ್ಯ        (lua_pushnumber) (lua_State *L, lua_ಸಂಖ್ಯೆ n);
LUA_API ಶೂನ್ಯ        (lua_pushinteger) (lua_State *L, lua_Integer n);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_pushlstring) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t len);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_pushexternalstring) (lua_State *L,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t len, lua_Alloc falloc, ಶೂನ್ಯ *ud);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_pushstring) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_pushvfstring) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt,
                                                      va_list argp);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_pushfstring) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...);
LUA_API ಶೂನ್ಯ  (lua_pushcclosure) (lua_State *L, lua_CFunction fn, ಪೂರ್ಣಾಂಕ n);
LUA_API ಶೂನ್ಯ  (lua_pushboolean) (lua_State *L, ಪೂರ್ಣಾಂಕ b);
LUA_API ಶೂನ್ಯ  (lua_pushlightuserdata) (lua_State *L, ಶೂನ್ಯ *p);
LUA_API ಪೂರ್ಣಾಂಕ   (lua_pushthread) (lua_State *L);


/*
** get functions (Lua -> stack)
*/
LUA_API ಪೂರ್ಣಾಂಕ (lua_getglobal) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name);
LUA_API ಪೂರ್ಣಾಂಕ (lua_gettable) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ (lua_getfield) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k);
LUA_API ಪೂರ್ಣಾಂಕ (lua_geti) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n);
LUA_API ಪೂರ್ಣಾಂಕ (lua_rawget) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಪೂರ್ಣಾಂಕ (lua_rawgeti) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n);
LUA_API ಪೂರ್ಣಾಂಕ (lua_rawgetp) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p);

LUA_API ಶೂನ್ಯ  (lua_createtable) (lua_State *L, ಪೂರ್ಣಾಂಕ narr, ಪೂರ್ಣಾಂಕ nrec);
LUA_API ಶೂನ್ಯ *(lua_newuserdatauv) (lua_State *L, size_t sz, ಪೂರ್ಣಾಂಕ nuvalue);
LUA_API ಪೂರ್ಣಾಂಕ   (lua_getmetatable) (lua_State *L, ಪೂರ್ಣಾಂಕ objindex);
LUA_API ಪೂರ್ಣಾಂಕ  (lua_getiuservalue) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ n);


/*
** set functions (stack -> Lua)
*/
LUA_API ಶೂನ್ಯ  (lua_setglobal) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name);
LUA_API ಶೂನ್ಯ  (lua_settable) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಶೂನ್ಯ  (lua_setfield) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *k);
LUA_API ಶೂನ್ಯ  (lua_seti) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n);
LUA_API ಶೂನ್ಯ  (lua_rawset) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಶೂನ್ಯ  (lua_rawseti) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, lua_Integer n);
LUA_API ಶೂನ್ಯ  (lua_rawsetp) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p);
LUA_API ಪೂರ್ಣಾಂಕ   (lua_setmetatable) (lua_State *L, ಪೂರ್ಣಾಂಕ objindex);
LUA_API ಪೂರ್ಣಾಂಕ   (lua_setiuservalue) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ n);


/*
** 'load' and 'call' functions (load and run Lua code)
*/
LUA_API ಶೂನ್ಯ  (lua_callk) (lua_State *L, ಪೂರ್ಣಾಂಕ nargs, ಪೂರ್ಣಾಂಕ nresults,
                           lua_KContext ctx, lua_KFunction k);
#ವ್ಯಾಖ್ಯಾನಿಸು lua_call(L,n,r)		lua_callk(L, (n), (r), 0, ಖಾಲಿ)

LUA_API ಪೂರ್ಣಾಂಕ   (lua_pcallk) (lua_State *L, ಪೂರ್ಣಾಂಕ nargs, ಪೂರ್ಣಾಂಕ nresults, ಪೂರ್ಣಾಂಕ errfunc,
                            lua_KContext ctx, lua_KFunction k);
#ವ್ಯಾಖ್ಯಾನಿಸು lua_pcall(L,n,r,f)	lua_pcallk(L, (n), (r), (f), 0, ಖಾಲಿ)

LUA_API ಪೂರ್ಣಾಂಕ   (lua_ತುಂಬು) (lua_State *L, lua_Reader reader, ಶೂನ್ಯ *dt,
                          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *chunkname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ);

LUA_API ಪೂರ್ಣಾಂಕ (lua_dump) (lua_State *L, lua_Writer writer, ಶೂನ್ಯ *ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ strip);


/*
** coroutine functions
*/
LUA_API ಪೂರ್ಣಾಂಕ  (lua_yieldk)     (lua_State *L, ಪೂರ್ಣಾಂಕ nresults, lua_KContext ctx,
                               lua_KFunction k);
LUA_API ಪೂರ್ಣಾಂಕ  (lua_resume)     (lua_State *L, lua_State *from, ಪೂರ್ಣಾಂಕ narg,
                               ಪೂರ್ಣಾಂಕ *nres);
LUA_API ಪೂರ್ಣಾಂಕ  (lua_ಸ್ಥಿತಿ)     (lua_State *L);
LUA_API ಪೂರ್ಣಾಂಕ (lua_isyieldable) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು lua_yield(L,n)		lua_yieldk(L, (n), 0, ಖಾಲಿ)


/*
** Warning-related functions
*/
LUA_API ಶೂನ್ಯ (lua_setwarnf) (lua_State *L, lua_WarnFunction f, ಶೂನ್ಯ *ud);
LUA_API ಶೂನ್ಯ (lua_warning)  (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ tocont);


/*
** garbage-collection options
*/

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCSTOP		0
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCRESTART		1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCCOLLECT		2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCCOUNT		3
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCCOUNTB		4
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCSTEP		5
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCISRUNNING		6
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCGEN		7
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCINC		8
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPARAM		9


/*
** garbage-collection parameters
*/
/* parameters for generational mode */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPMINORMUL		0  /* control minor collections */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPMAJORMINOR	1  /* control shift major->minor */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPMINORMAJOR	2  /* control shift minor->major */

/* parameters for incremental mode */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPPAUSE		3  /* size of pause between successive GCs */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPSTEPMUL		4  /* GC "speed" */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPSTEPSIZE		5  /* GC granularity */

/* number of parameters */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GCPN		6


LUA_API ಪೂರ್ಣಾಂಕ (lua_gc) (lua_State *L, ಪೂರ್ಣಾಂಕ what, ...);


/*
** miscellaneous functions
*/

LUA_API ಪೂರ್ಣಾಂಕ   (lua_error) (lua_State *L);

LUA_API ಪೂರ್ಣಾಂಕ   (lua_ಮುಂದಿನ) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);

LUA_API ಶೂನ್ಯ  (lua_concat) (lua_State *L, ಪೂರ್ಣಾಂಕ n);
LUA_API ಶೂನ್ಯ  (lua_len)    (lua_State *L, ಪೂರ್ಣಾಂಕ idx);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_N2SBUFFSZ	64
LUA_API ಚಿಹ್ನೆರಹಿತ  (lua_numbertocstring) (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಅಕ್ಷರ *buff);
LUA_API size_t  (lua_stringtonumber) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);

LUA_API lua_Alloc (lua_getallocf) (lua_State *L, ಶೂನ್ಯ **ud);
LUA_API ಶೂನ್ಯ      (lua_setallocf) (lua_State *L, lua_Alloc f, ಶೂನ್ಯ *ud);

LUA_API ಶೂನ್ಯ (lua_toclose) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);
LUA_API ಶೂನ್ಯ (lua_closeslot) (lua_State *L, ಪೂರ್ಣಾಂಕ idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lua_getextraspace(L)	((ಶೂನ್ಯ *)((ಅಕ್ಷರ *)(L) - LUA_EXTRASPACE))

#ವ್ಯಾಖ್ಯಾನಿಸು lua_tonumber(L,i)	lua_tonumberx(L,(i),ಖಾಲಿ)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_tointeger(L,i)	lua_tointegerx(L,(i),ಖಾಲಿ)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_ತೆಗೆ(L,n)		lua_settop(L, -(n)-1)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_newtable(L)		lua_createtable(L, 0, 0)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_ರಿಜಿಸ್ಟರ್(L,n,f) (lua_pushcfunction(L, (f)), lua_setglobal(L, (n)))

#ವ್ಯಾಖ್ಯಾನಿಸು lua_pushcfunction(L,f)	lua_pushcclosure(L, (f), 0)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_isfunction(L,n)	(lua_type(L, (n)) == LUA_TFUNCTION)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_istable(L,n)	(lua_type(L, (n)) == LUA_TTABLE)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_islightuserdata(L,n)	(lua_type(L, (n)) == LUA_TLIGHTUSERDATA)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_isnil(L,n)		(lua_type(L, (n)) == LUA_TNIL)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_isboolean(L,n)	(lua_type(L, (n)) == LUA_TBOOLEAN)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_isthread(L,n)	(lua_type(L, (n)) == LUA_TTHREAD)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_isnone(L,n)		(lua_type(L, (n)) == LUA_TNONE)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_isnoneornil(L, n)	(lua_type(L, (n)) <= 0)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_pushliteral(L, s)	lua_pushstring(L, "" s)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_pushglobaltable(L)  \
	((ಶೂನ್ಯ)lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#ವ್ಯಾಖ್ಯಾನಿಸು lua_tostring(L,i)	lua_tolstring(L, (i), ಖಾಲಿ)


#ವ್ಯಾಖ್ಯಾನಿಸು lua_ಸೇರಿಸು(L,idx)	lua_rotate(L, (idx), 1)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_remove(L,idx)	(lua_rotate(L, (idx), -1), lua_ತೆಗೆ(L, 1))

#ವ್ಯಾಖ್ಯಾನಿಸು lua_replace(L,idx)	(lua_copy(L, -1, (idx)), lua_ತೆಗೆ(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lua_newuserdata(L,s)	lua_newuserdatauv(L,s,1)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_getuservalue(L,idx)	lua_getiuservalue(L,idx,1)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_setuservalue(L,idx)	lua_setiuservalue(L,idx,1)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_resetthread(L)	lua_closethread(L,ಖಾಲಿ)

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_HOOKCALL	0
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_HOOKRET	1
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_HOOKLINE	2
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_HOOKCOUNT	3
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MASKCALL	(1 << LUA_HOOKCALL)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MASKRET	(1 << LUA_HOOKRET)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MASKLINE	(1 << LUA_HOOKLINE)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API ಪೂರ್ಣಾಂಕ (lua_getstack) (lua_State *L, ಪೂರ್ಣಾಂಕ level, lua_Debug *ar);
LUA_API ಪೂರ್ಣಾಂಕ (lua_getinfo) (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what, lua_Debug *ar);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_getlocal) (lua_State *L, ಸ್ಥಿರಾಂಕ lua_Debug *ar, ಪೂರ್ಣಾಂಕ n);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_setlocal) (lua_State *L, ಸ್ಥಿರಾಂಕ lua_Debug *ar, ಪೂರ್ಣಾಂಕ n);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_getupvalue) (lua_State *L, ಪೂರ್ಣಾಂಕ funcindex, ಪೂರ್ಣಾಂಕ n);
LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(lua_setupvalue) (lua_State *L, ಪೂರ್ಣಾಂಕ funcindex, ಪೂರ್ಣಾಂಕ n);

LUA_API ಶೂನ್ಯ *(lua_upvalueid) (lua_State *L, ಪೂರ್ಣಾಂಕ fidx, ಪೂರ್ಣಾಂಕ n);
LUA_API ಶೂನ್ಯ  (lua_upvaluejoin) (lua_State *L, ಪೂರ್ಣಾಂಕ fidx1, ಪೂರ್ಣಾಂಕ n1,
                                               ಪೂರ್ಣಾಂಕ fidx2, ಪೂರ್ಣಾಂಕ n2);

LUA_API ಶೂನ್ಯ (lua_sethook) (lua_State *L, lua_Hook func, ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2);
LUA_API lua_Hook (lua_gethook) (lua_State *L);
LUA_API ಪೂರ್ಣಾಂಕ (lua_gethookmask) (lua_State *L);
LUA_API ಪೂರ್ಣಾಂಕ (lua_gethookcount) (lua_State *L);


ರಚನೆ lua_Debug {
  ಪೂರ್ಣಾಂಕ event;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;	/* (n) */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what;	/* (S) 'Lua', 'C', 'main', 'tail' */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ;	/* (S) */
  size_t srclen;	/* (S) */
  ಪೂರ್ಣಾಂಕ currentline;	/* (l) */
  ಪೂರ್ಣಾಂಕ linedefined;	/* (S) */
  ಪೂರ್ಣಾಂಕ lastlinedefined;	/* (S) */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ nups;	/* (u) number of upvalues */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ nparams;/* (u) number of parameters */
  ಅಕ್ಷರ isvararg;        /* (u) */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ extraargs;  /* (t) number of extra arguments */
  ಅಕ್ಷರ istailcall;	/* (t) */
  ಪೂರ್ಣಾಂಕ ftransfer;   /* (r) index of first value transferred */
  ಪೂರ್ಣಾಂಕ ntransfer;   /* (r) number of transferred values */
  ಅಕ್ಷರ short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  ರಚನೆ CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_TOSTRAUX(x)	#x
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_TOSTR(x)		LUAI_TOSTRAUX(x)

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_MAJOR	LUAI_TOSTR(LUA_ಆವೃತ್ತಿ_MAJOR_N)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_MINOR	LUAI_TOSTR(LUA_ಆವೃತ್ತಿ_MINOR_N)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ_RELEASE	LUAI_TOSTR(LUA_ಆವೃತ್ತಿ_RELEASE_N)

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಆವೃತ್ತಿ	"Lua " LUA_ಆವೃತ್ತಿ_MAJOR "." LUA_ಆವೃತ್ತಿ_MINOR
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_RELEASE	LUA_ಆವೃತ್ತಿ "." LUA_ಆವೃತ್ತಿ_RELEASE


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


#ಆದರೆ_ಅಂತ್ಯ
