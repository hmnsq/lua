/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lundump_h
#ವ್ಯಾಖ್ಯಾನಿಸು lundump_h

#ಅನ್ತರ್ಭಾವ <limits.h>

#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


/* data to catch conversion errors */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAC_ದತ್ತಾಂಶ	"\x19\x93\r\n\x1a\n"

#ವ್ಯಾಖ್ಯಾನಿಸು LUAC_INT	-0x5678
#ವ್ಯಾಖ್ಯಾನಿಸು LUAC_INST	0x12345678
#ವ್ಯಾಖ್ಯಾನಿಸು LUAC_ಅಂಕೆ	cast_ಅಂಕೆ(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUAC_ಆವೃತ್ತಿ	(LUA_ಆವೃತ್ತಿ_MAJOR_N*16+LUA_ಆವೃತ್ತಿ_MINOR_N)

#ವ್ಯಾಖ್ಯಾನಿಸು LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, ಕೋಷ್ಟಕ *anchor,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* name, ಪೂರ್ಣಾಂಕ fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaU_dump (lua_State* L, ಸ್ಥಿರಾಂಕ Proto* f, lua_Writer w,
                         ಶೂನ್ಯ* ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ strip);

#ಆದರೆ_ಅಂತ್ಯ
