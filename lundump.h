/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lundump_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lundump_h

#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"


/* data to catch conversion errors */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAC_ਅੰਕੜੇ	"\x19\x93\r\n\x1a\n"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAC_INT	-0x5678
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAC_INST	0x12345678
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAC_ਅੰਕ	cast_ਅੰਕ(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAC_ਸੰਸਕਰਣ	(LUA_ਸੰਸਕਰਣ_MAJOR_N*16+LUA_ਸੰਸਕਰਣ_MINOR_N)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, ਸਾਰਣੀ *anchor,
                                 ਅਚਲ ਅੱਖਰ* name, ਪੂਰਨ fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC ਪੂਰਨ luaU_dump (lua_State* L, ਅਚਲ Proto* f, lua_Writer w,
                         ਖਾਲੀ* ਅੰਕੜੇ, ਪੂਰਨ strip);

#ਜੇ_ਅੰਤ
