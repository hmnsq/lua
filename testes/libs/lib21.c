#include "lua.h"


จำนวนเต็ม luaopen_lib2 (lua_State *L);

LUAMOD_API จำนวนเต็ม luaopen_lib21 (lua_State *L) {
  คืน luaopen_lib2(L);
}


