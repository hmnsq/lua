#include "lua.h"


ཧྲིལ luaopen_lib2 (lua_State *L);

LUAMOD_API ཧྲིལ luaopen_lib21 (lua_State *L) {
  ཕྱིར luaopen_lib2(L);
}


