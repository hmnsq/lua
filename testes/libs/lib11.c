#include "lua.h"

/* function from lib1.c */
LUAMOD_API ကိန်းပြည့် lib1_export (lua_State *L);

LUAMOD_API ကိန်းပြည့် luaopen_lib11 (lua_State *L) {
  ပြန်ပေး lib1_export(L);
}


