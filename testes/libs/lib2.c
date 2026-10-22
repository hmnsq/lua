#include "lua.h"
#include "lauxlib.h"

static ចំនួនគត់ id (lua_State *L) {
  ត្រឡប់ lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


LUAMOD_API ចំនួនគត់ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ត្រឡប់ 1;
}


