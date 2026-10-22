#include "lua.h"

/* function from lib1.c */
LUAMOD_API ціле lib1_export (lua_State *L);

LUAMOD_API ціле luaopen_lib11 (lua_State *L) {
  повернути lib1_export(L);
}


