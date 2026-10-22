/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#ضمن <limits.h>

#ضمن "llimits.h"
#ضمن "lobject.h"
#ضمن "lzio.h"


/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	-0x5678
#define LUAC_INST	0x12345678
#define LUAC_لمب	cast_لمب(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#define LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 ثابت حرف* name, صحيح fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC صحيح luaU_dump (lua_State* L, ثابت Proto* f, lua_Writer w,
                         فراغ* data, صحيح strip);

#endif
