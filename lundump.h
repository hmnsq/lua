/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#अन्तर्भाव <limits.h>

#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lzio.h"


/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	-0x5678
#define LUAC_INST	0x12345678
#define LUAC_अङ्क	cast_अङ्क(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#define LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 अचर वर्ण* name, पूर्णाङ्क fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC पूर्णाङ्क luaU_dump (lua_State* L, अचर Proto* f, lua_Writer w,
                         शून्य* data, पूर्णाङ्क strip);

#endif
