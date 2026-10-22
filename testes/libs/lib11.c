#अन्तर्भाव "lua.h"

/* function from lib1.c */
LUAMOD_API पूर्णाङ्क lib1_export (lua_State *L);

LUAMOD_API पूर्णाङ्क luaopen_lib11 (lua_State *L) {
  फर्काऊ lib1_export(L);
}


