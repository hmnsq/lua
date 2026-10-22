#include "lua.h"
#include "lauxlib.h"

static ကိန်းပြည့် id (lua_State *L) {
  ပြန်ပေး lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


LUAMOD_API ကိန်းပြည့် luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ပြန်ပေး 1;
}


