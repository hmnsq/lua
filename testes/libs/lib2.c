#अन्तर्भाव "lua.h"
#अन्तर्भाव "lauxlib.h"

स्थिर पूर्णांक id (lua_State *L) {
  परत lua_gettop(L);
}


स्थिर स्थिरांक संरचना luaL_Reg funcs[] = {
  {"id", id},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णांक luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  परत 1;
}


