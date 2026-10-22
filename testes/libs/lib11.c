#include "lua.h"

/* function from lib1.c */
LUAMOD_API 정수 lib1_export (lua_State *L);

LUAMOD_API 정수 luaopen_lib11 (lua_State *L) {
  반환 lib1_export(L);
}


