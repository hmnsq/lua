#include "lua.h"
#include "lauxlib.h"

static ཧྲིལ id (lua_State *L) {
  ལོག lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API ཧྲིལ lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ལོག 1;
}


LUAMOD_API ཧྲིལ onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ལོག 2;
}


LUAMOD_API ཧྲིལ anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ཧྲིལ)lua_tointeger(L, 1),
                                 (ཧྲིལ)lua_tointeger(L, 2));
  ལོག 1;
} 


LUAMOD_API ཧྲིལ luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ལོག 1;
}

