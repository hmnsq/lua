#شامل "lua.h"
#شامل "lauxlib.h"

ساکن صحیح id (lua_State *L) {
  واپس lua_gettop(L);
}


ساکن ثابت ساخت luaL_Reg funcs[] = {
  {"id", id},
  {تہی, تہی}
};


/* function used by lib11.c */
LUAMOD_API صحیح lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  واپس 1;
}


LUAMOD_API صحیح onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  واپس 2;
}


LUAMOD_API صحیح anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (صحیح)lua_tointeger(L, 1),
                                 (صحیح)lua_tointeger(L, 2));
  واپس 1;
} 


LUAMOD_API صحیح luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  واپس 1;
}

