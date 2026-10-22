#شامل "lua.h"
#شامل "lauxlib.h"

ایستا صحیح id (lua_State *L) {
  بازگشت lua_gettop(L);
}


ایستا ثابت ساختار luaL_Reg funcs[] = {
  {"id", id},
  {پوچ, پوچ}
};


LUAMOD_API صحیح luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  بازگشت 1;
}


