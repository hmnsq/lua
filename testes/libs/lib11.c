#include "lua.h"

/* function from lib1.c */
LUAMOD_API số_nguyên lib1_export (lua_State *L);

LUAMOD_API số_nguyên luaopen_lib11 (lua_State *L) {
  trả_về lib1_export(L);
}


