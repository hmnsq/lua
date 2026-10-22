/*
** $Id: lundump.h $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lundump_h
#నిర్వచించు lundump_h

#అన్తర్భావ <limits.h>

#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lzio.h"


/* data to catch conversion errors */
#నిర్వచించు LUAC_దత్తాంశం	"\x19\x93\r\n\x1a\n"

#నిర్వచించు LUAC_INT	-0x5678
#నిర్వచించు LUAC_INST	0x12345678
#నిర్వచించు LUAC_అంకె	cast_అంకె(-370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#నిర్వచించు LUAC_సంస్కరణ	(LUA_సంస్కరణ_MAJOR_N*16+LUA_సంస్కరణ_MINOR_N)

#నిర్వచించు LUAC_FORMAT	0	/* this is the official format */


/* load one chunk; from lundump.c */
LUAI_FUNC LClosure* luaU_undump (lua_State* L, ZIO* Z, పట్టిక *anchor,
                                 స్థిరరాశి అక్షరం* name, పూర్ణాంకం fixed);

/* dump one chunk; from ldump.c */
LUAI_FUNC పూర్ణాంకం luaU_dump (lua_State* L, స్థిరరాశి Proto* f, lua_Writer w,
                         శూన్య* దత్తాంశం, పూర్ణాంకం strip);

#అయితే_ముగింపు
