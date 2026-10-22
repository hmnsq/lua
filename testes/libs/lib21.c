#include "lua.h"


整数 luaopen_lib2 (lua_State *L);

LUAMOD_API 整数 luaopen_lib21 (lua_State *L) {
  返回 luaopen_lib2(L);
}


