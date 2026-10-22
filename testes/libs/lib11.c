#شامل "lua.h"

/* function from lib1.c */
LUAMOD_API صحیح lib1_export (lua_State *L);

LUAMOD_API صحیح luaopen_lib11 (lua_State *L) {
  واپس lib1_export(L);
}


