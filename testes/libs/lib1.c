#अन्तर्भाव "lua.h"
#अन्तर्भाव "lauxlib.h"

स्थिर पूर्णांक id (lua_State *L) {
  परत lua_gettop(L);
}


स्थिर स्थिरांक संरचना luaL_Reg funcs[] = {
  {"id", id},
  {रिक्त, रिक्त}
};


/* function used by lib11.c */
LUAMOD_API पूर्णांक lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  परत 1;
}


LUAMOD_API पूर्णांक onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  परत 2;
}


LUAMOD_API पूर्णांक anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (पूर्णांक)lua_tointeger(L, 1),
                                 (पूर्णांक)lua_tointeger(L, 2));
  परत 1;
} 


LUAMOD_API पूर्णांक luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  परत 1;
}

