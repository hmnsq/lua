#include "lua.h"
#include "lauxlib.h"

static ຈຳນວນເຕັມ id (lua_State *L) {
  ສົ່ງຄືນ lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API ຈຳນວນເຕັມ lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ສົ່ງຄືນ 1;
}


LUAMOD_API ຈຳນວນເຕັມ onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ສົ່ງຄືນ 2;
}


LUAMOD_API ຈຳນວນເຕັມ anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ຈຳນວນເຕັມ)lua_tointeger(L, 1),
                                 (ຈຳນວນເຕັມ)lua_tointeger(L, 2));
  ສົ່ງຄືນ 1;
} 


LUAMOD_API ຈຳນວນເຕັມ luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ສົ່ງຄືນ 1;
}

