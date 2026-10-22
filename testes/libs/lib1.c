#include "lua.h"
#include "lauxlib.h"

static puur_naa_nka id (lua_State *L) {
  ꦮꦭꦶ lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API puur_naa_nka lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ꦮꦭꦶ 1;
}


LUAMOD_API puur_naa_nka onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ꦮꦭꦶ 2;
}


LUAMOD_API puur_naa_nka anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (puur_naa_nka)lua_tointeger(L, 1),
                                 (puur_naa_nka)lua_tointeger(L, 2));
  ꦮꦭꦶ 1;
} 


LUAMOD_API puur_naa_nka luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ꦮꦭꦶ 1;
}

