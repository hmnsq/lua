#समावेश "lua.h"
#समावेश "lauxlib.h"

अटल पूर्णांक id (lua_State *L) {
  वापस lua_gettop(L);
}


अटल स्थिर काष्ठा luaL_Reg funcs[] = {
  {"id", id},
  {शून्य, शून्य}
};


LUAMOD_API पूर्णांक luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  वापस 1;
}


