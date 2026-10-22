/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی lundump_h
#تعریف_کړه lundump_h

#ضمن <limits.h>

#ضمن "llimits.h"
#ضمن "lobject.h"
#ضمن "lzio.h"


/* data to catch conversion errors */
#تعریف_کړه LUAC_معلومات	"\x19\x93\r\n\x1a\n"

#تعریف_کړه LUAC_INT	-0x5678
#تعریف_کړه LUAC_INST	0x12345678
#تعریف_کړه LUAC_عدد	cast_عدد(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#تعریف_کړه LUAC_نسخه	(LUA_نسخه_MAJOR_N*16+LUA_نسخه_MINOR_N)

#تعریف_کړه LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, جدول *anchor,
                                 ثابت حرف* name, صحيح fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC صحيح luaU_dump (lua_State* L, ثابت Proto* f, lua_Writer w,
                         فراغ* معلومات, صحيح strip);

#د_که_پای
