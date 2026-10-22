#include "lua.h"


정수 luaopen_lib2 (lua_State *L);

LUAMOD_API 정수 luaopen_lib21 (lua_State *L) {
  반환 luaopen_lib2(L);
}


