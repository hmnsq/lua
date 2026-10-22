#include "lua.h"

/* function from lib1.c */
LUAMOD_API цео lib1_export (lua_State *L);

LUAMOD_API цео luaopen_lib11 (lua_State *L) {
  врати lib1_export(L);
}


