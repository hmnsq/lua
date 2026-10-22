/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#অন্তর্ভাৱ <limits.h>

#অন্তর্ভাৱ "llimits.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lzio.h"


/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	-0x5678
#define LUAC_INST	0x12345678
#define LUAC_অঙ্ক	cast_অঙ্ক(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#define LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 ধ্রুৱ ৱর্ণ* name, পূর্ণাঙ্ক fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC পূর্ণাঙ্ক luaU_dump (lua_State* L, ধ্রুৱ Proto* f, lua_Writer w,
                         শূন্য* data, পূর্ণাঙ্ক strip);

#endif
