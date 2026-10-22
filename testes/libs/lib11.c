#include "lua.h"

/* function from lib1.c */
LUAMOD_API 整数 lib1_export (lua_State *L);

LUAMOD_API 整数 luaopen_lib11 (lua_State *L) {
  戻る lib1_export(L);
}


