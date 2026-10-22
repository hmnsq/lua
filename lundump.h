/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#ಅನ್ತರ್ಭಾವ <limits.h>

#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	-0x5678
#define LUAC_INST	0x12345678
#define LUAC_ಅಂಕೆ	cast_ಅಂಕೆ(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#define LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 ಧ್ರುವ ವರ್ಣ* name, ಪೂರ್ಣಾಙ್ಕ fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaU_dump (lua_State* L, ಧ್ರುವ Proto* f, lua_Writer w,
                         ಶೂನ್ಯ* data, ಪೂರ್ಣಾಙ್ಕ strip);

#endif
