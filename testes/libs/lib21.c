#include "lua.h"


ကိန်းပြည့် luaopen_lib2 (lua_State *L);

LUAMOD_API ကိန်းပြည့် luaopen_lib21 (lua_State *L) {
  ပြန်ပေး luaopen_lib2(L);
}


