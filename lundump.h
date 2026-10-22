/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#અન્તર્ભાવ <limits.h>

#અન્તર્ભાવ "llimits.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lzio.h"


/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	-0x5678
#define LUAC_INST	0x12345678
#define LUAC_અંક	cast_અંક(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#define LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 અચલ અક્ષર* name, પૂર્ણાંક fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC પૂર્ણાંક luaU_dump (lua_State* L, અચલ Proto* f, lua_Writer w,
                         શૂન્ય* data, પૂર્ણાંક strip);

#endif
