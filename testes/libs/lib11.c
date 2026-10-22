#include "lua.h"

/* function from lib1.c */
LUAMOD_API จำนวนเต็ม lib1_export (lua_State *L);

LUAMOD_API จำนวนเต็ม luaopen_lib11 (lua_State *L) {
  คืน lib1_export(L);
}


