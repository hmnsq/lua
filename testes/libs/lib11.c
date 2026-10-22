#include "lua.h"

/* function from lib1.c */
LUAMOD_API ཧྲིལ lib1_export (lua_State *L);

LUAMOD_API ཧྲིལ luaopen_lib11 (lua_State *L) {
  ཕྱིར lib1_export(L);
}


