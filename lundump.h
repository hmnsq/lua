/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lundump_h
#تعریف_کرو lundump_h

#شامل <limits.h>

#شامل "llimits.h"
#شامل "lobject.h"
#شامل "lzio.h"


/* data to catch conversion errors */
#تعریف_کرو LUAC_ڈیٹا	"\x19\x93\r\n\x1a\n"

#تعریف_کرو LUAC_INT	-0x5678
#تعریف_کرو LUAC_INST	0x12345678
#تعریف_کرو LUAC_عدد	cast_عدد(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#تعریف_کرو LUAC_نسخہ	(LUA_نسخہ_MAJOR_N*16+LUA_نسخہ_MINOR_N)

#تعریف_کرو LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, جدول *anchor,
                                 ثابت حرف* name, صحیح fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC صحیح luaU_dump (lua_State* L, ثابت Proto* f, lua_Writer w,
                         خالی* ڈیٹا, صحیح strip);

#اگر_ختم
