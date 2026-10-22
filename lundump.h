/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् lundump_h
#परिभाषय lundump_h

#अन्तर्भाव <limits.h>

#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lzio.h"


/* data to catch conversion errors */
#परिभाषय LUAC_दत्त	"\x19\x93\r\n\x1a\n"

#परिभाषय LUAC_INT	-0x5678
#परिभाषय LUAC_INST	0x12345678
#परिभाषय LUAC_अङ्कः	cast_अङ्कः(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#परिभाषय LUAC_संस्करणम्	(LUA_संस्करणम्_MAJOR_N*16+LUA_संस्करणम्_MINOR_N)

#परिभाषय LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, सारणी *anchor,
                                 ध्रुव वर्ण* name, पूर्णाङ्क fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC पूर्णाङ्क luaU_dump (lua_State* L, ध्रुव Proto* f, lua_Writer w,
                         शून्य* दत्त, पूर्णाङ्क strip);

#यदि_अन्तः
