#include "lua.h"

/* function from lib1.c */
LUAMOD_API цяло lib1_export (lua_State *L);

LUAMOD_API цяло luaopen_lib11 (lua_State *L) {
  върни lib1_export(L);
}


