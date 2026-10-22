#include "lua.h"


ចំនួនគត់ luaopen_lib2 (lua_State *L);

LUAMOD_API ចំនួនគត់ luaopen_lib21 (lua_State *L) {
  ត្រឡប់ luaopen_lib2(L);
}


