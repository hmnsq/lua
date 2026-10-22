#ضمن "lua.h"
#ضمن "lauxlib.h"

ساكن صحيح id (lua_State *L) {
  أرجع lua_gettop(L);
}


ساكن ثابت بنية luaL_Reg funcs[] = {
  {"id", id},
  {عدم, عدم}
};


LUAMOD_API صحيح luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  أرجع 1;
}


