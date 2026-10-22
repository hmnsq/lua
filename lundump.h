/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#അന്തര്ഭാവ <limits.h>

#അന്തര്ഭാവ "llimits.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lzio.h"


/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	-0x5678
#define LUAC_INST	0x12345678
#define LUAC_അക്കം	cast_അക്കം(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#define LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 സ്ഥിരം അക്ഷരം* name, പൂർണ്ണസംഖ്യ fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC പൂർണ്ണസംഖ്യ luaU_dump (lua_State* L, സ്ഥിരം Proto* f, lua_Writer w,
                         ശൂന്യം* data, പൂർണ്ണസംഖ്യ strip);

#endif
