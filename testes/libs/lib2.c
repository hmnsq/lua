#شامل "lua.h"
#شامل "lauxlib.h"

ساکن صحیح id (lua_State *L) {
  واپس lua_gettop(L);
}


ساکن ثابت ساخت luaL_Reg funcs[] = {
  {"id", id},
  {تہی, تہی}
};


LUAMOD_API صحیح luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  واپس 1;
}


