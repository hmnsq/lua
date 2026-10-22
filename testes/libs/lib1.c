#include "lua.h"
#include "lauxlib.h"

static 整数 id (lua_State *L) {
  返回 lua_gettop(L);
}


static const struct luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API 整数 lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  返回 1;
}


LUAMOD_API 整数 onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  返回 2;
}


LUAMOD_API 整数 anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (整数)lua_tointeger(L, 1),
                                 (整数)lua_tointeger(L, 2));
  返回 1;
} 


LUAMOD_API 整数 luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  返回 1;
}

