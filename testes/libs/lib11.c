#ضمن "lua.h"

/* function from lib1.c */
LUAMOD_API صحيح lib1_export (lua_State *L);

LUAMOD_API صحيح luaopen_lib11 (lua_State *L) {
  إرجاع lib1_export(L);
}


