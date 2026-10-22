#include "lua.h"
#include "lauxlib.h"

static ціле id (lua_State *L) {
  повернути lua_gettop(L);
}


static const struct luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API ціле lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  повернути 1;
}


LUAMOD_API ціле onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  повернути 2;
}


LUAMOD_API ціле anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ціле)lua_tointeger(L, 1),
                                 (ціле)lua_tointeger(L, 2));
  повернути 1;
} 


LUAMOD_API ціле luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  повернути 1;
}

