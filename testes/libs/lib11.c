#include "lua.h"

/* function from lib1.c */
LUAMOD_API ຈຳນວນເຕັມ lib1_export (lua_State *L);

LUAMOD_API ຈຳນວນເຕັມ luaopen_lib11 (lua_State *L) {
  ສົ່ງຄືນ lib1_export(L);
}


