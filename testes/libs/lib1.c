#include "lua.h"
#include "lauxlib.h"

static цео id (lua_State *L) {
  врати lua_gettop(L);
}


static const struct luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API цео lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  врати 1;
}


LUAMOD_API цео onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  врати 2;
}


LUAMOD_API цео anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (цео)lua_tointeger(L, 1),
                                 (цео)lua_tointeger(L, 2));
  врати 1;
} 


LUAMOD_API цео luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  врати 1;
}

