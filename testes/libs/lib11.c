#include "lua.h"

/* function from lib1.c */
LUAMOD_API puur_naa_nka lib1_export (lua_State *L);

LUAMOD_API puur_naa_nka luaopen_lib11 (lua_State *L) {
  ᰀᰧᰌ lib1_export(L);
}


