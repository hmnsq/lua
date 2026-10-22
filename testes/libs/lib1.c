#समावेश "lua.h"
#समावेश "lauxlib.h"

अटल पूर्णांक id (lua_State *L) {
  वापस lua_gettop(L);
}


अटल स्थिर काष्ठा luaL_Reg funcs[] = {
  {"id", id},
  {शून्य, शून्य}
};


/* function used by lib11.c */
LUAMOD_API पूर्णांक lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  वापस 1;
}


LUAMOD_API पूर्णांक onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  वापस 2;
}


LUAMOD_API पूर्णांक anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (पूर्णांक)lua_tointeger(L, 1),
                                 (पूर्णांक)lua_tointeger(L, 2));
  वापस 1;
} 


LUAMOD_API पूर्णांक luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  वापस 1;
}

