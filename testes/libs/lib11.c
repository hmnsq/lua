#समावेश "lua.h"

/* function from lib1.c */
LUAMOD_API पूर्णांक lib1_export (lua_State *L);

LUAMOD_API पूर्णांक luaopen_lib11 (lua_State *L) {
  वापस lib1_export(L);
}


