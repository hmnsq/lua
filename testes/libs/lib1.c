#include "lua.h"
#include "lauxlib.h"

static số_nguyên id (lua_State *L) {
  trả_về lua_gettop(L);
}


static const struct luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API số_nguyên lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  trả_về 1;
}


LUAMOD_API số_nguyên onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  trả_về 2;
}


LUAMOD_API số_nguyên anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (số_nguyên)lua_tointeger(L, 1),
                                 (số_nguyên)lua_tointeger(L, 2));
  trả_về 1;
} 


LUAMOD_API số_nguyên luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  trả_về 1;
}

