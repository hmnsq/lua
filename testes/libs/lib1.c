#include "lua.h"
#include "lauxlib.h"

static цяло id (lua_State *L) {
  върни lua_gettop(L);
}


static const struct luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API цяло lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  върни 1;
}


LUAMOD_API цяло onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  върни 2;
}


LUAMOD_API цяло anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (цяло)lua_tointeger(L, 1),
                                 (цяло)lua_tointeger(L, 2));
  върни 1;
} 


LUAMOD_API цяло luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  върни 1;
}

