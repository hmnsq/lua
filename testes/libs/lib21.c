#include "lua.h"


целое luaopen_lib2 (lua_State *L);

LUAMOD_API целое luaopen_lib21 (lua_State *L) {
  вернуть luaopen_lib2(L);
}


