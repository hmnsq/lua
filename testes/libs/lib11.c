#include "lua.h"

/* function from lib1.c */
LUAMOD_API целое lib1_export (lua_State *L);

LUAMOD_API целое luaopen_lib11 (lua_State *L) {
  вернуть lib1_export(L);
}


