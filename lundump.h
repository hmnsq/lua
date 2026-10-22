/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lundump_h
#وصف_ڪريو lundump_h

#ضمن <limits.h>

#ضمن "llimits.h"
#ضمن "lobject.h"
#ضمن "lzio.h"


/* data to catch conversion errors */
#وصف_ڪريو LUAC_معلومات	"\x19\x93\r\n\x1a\n"

#وصف_ڪريو LUAC_INT	-0x5678
#وصف_ڪريو LUAC_INST	0x12345678
#وصف_ڪريو LUAC_انگ	cast_انگ(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#وصف_ڪريو LUAC_نسخو	(LUA_نسخو_MAJOR_N*16+LUA_نسخو_MINOR_N)

#وصف_ڪريو LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, جدول *anchor,
                                 ثابت حرف* name, صحيح fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC صحيح luaU_dump (lua_State* L, ثابت Proto* f, lua_Writer w,
                         فراغ* معلومات, صحيح strip);

#جيڪڏهن_پورو
