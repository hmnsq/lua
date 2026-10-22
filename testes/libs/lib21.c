#include "lua.h"


ຈຳນວນເຕັມ luaopen_lib2 (lua_State *L);

LUAMOD_API ຈຳນວນເຕັມ luaopen_lib21 (lua_State *L) {
  ສົ່ງຄືນ luaopen_lib2(L);
}


