#include "lua.h"
#include "lauxlib.h"

static целое id (lua_State *L) {
  вернуть lua_gettop(L);
}


static const struct luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API целое lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  вернуть 1;
}


LUAMOD_API целое onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  вернуть 2;
}


LUAMOD_API целое anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (целое)lua_tointeger(L, 1),
                                 (целое)lua_tointeger(L, 2));
  вернуть 1;
} 


LUAMOD_API целое luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  вернуть 1;
}

