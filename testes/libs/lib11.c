#include "lua.h"

/* function from lib1.c */
LUAMOD_API ចំនួនគត់ lib1_export (lua_State *L);

LUAMOD_API ចំនួនគត់ luaopen_lib11 (lua_State *L) {
  ត្រឡប់ lib1_export(L);
}


