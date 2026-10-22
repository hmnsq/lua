/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ifndef lundump_h
#define lundump_h

#ଅନ୍ତର୍ଭାଵ <limits.h>

#ଅନ୍ତର୍ଭାଵ "llimits.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lzio.h"


/* data to catch conversion errors */
#define LUAC_DATA	"\x19\x93\r\n\x1a\n"

#define LUAC_INT	-0x5678
#define LUAC_INST	0x12345678
#define LUAC_ଅଙ୍କ	cast_ଅଙ୍କ(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#define LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 ଅଚଳ ଅକ୍ଷର* name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaU_dump (lua_State* L, ଅଚଳ Proto* f, lua_Writer w,
                         ଶୂନ୍ୟ* data, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strip);

#endif
