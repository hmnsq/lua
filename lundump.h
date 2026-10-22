/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده lundump_h
#تعریف_کن lundump_h

#ضمن <limits.h>

#ضمن "llimits.h"
#ضمن "lobject.h"
#ضمن "lzio.h"


/* data to catch conversion errors */
#تعریف_کن LUAC_داده	"\x19\x93\r\n\x1a\n"

#تعریف_کن LUAC_INT	-0x5678
#تعریف_کن LUAC_INST	0x12345678
#تعریف_کن LUAC_رقم	cast_رقم(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#تعریف_کن LUAC_نسخه	(LUA_نسخه_MAJOR_N*16+LUA_نسخه_MINOR_N)

#تعریف_کن LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, جدول *anchor,
                                 ثابت حرف* name, صحيح fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC صحيح luaU_dump (lua_State* L, ثابت Proto* f, lua_Writer w,
                         فراغ* داده, صحيح strip);

#پایان_اگر
