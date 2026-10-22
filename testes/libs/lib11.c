#include "lua.h"

/* function from lib1.c */
LUAMOD_API පූර්ණ_සංඛ්‍යා lib1_export (lua_State *L);

LUAMOD_API පූර්ණ_සංඛ්‍යා luaopen_lib11 (lua_State *L) {
  ආපසු lib1_export(L);
}


