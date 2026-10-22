/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lundump_h
#घोषणा lundump_h

#समावेश <limits.h>

#समावेश "llimits.h"
#समावेश "lobject.h"
#समावेश "lzio.h"


/* data to catch conversion errors */
#घोषणा LUAC_तथ्य	"\x19\x93\r\n\x1a\n"

#घोषणा LUAC_INT	-0x5678
#घोषणा LUAC_INST	0x12345678
#घोषणा LUAC_अंक	cast_अंक(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#घोषणा LUAC_VERSION	(LUA_VERSION_MAJOR_N*16+LUA_VERSION_MINOR_N)

#घोषणा LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, Table *anchor,
                                 स्थिर अक्षर* name, पूर्णांक fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC पूर्णांक luaU_dump (lua_State* L, स्थिर Proto* f, lua_Writer w,
                         व्योम* तथ्य, पूर्णांक strip);

#पूर्ण_अगर
