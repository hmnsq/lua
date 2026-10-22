#include "lua.h"
#include "lauxlib.h"

static ចំនួនគត់ id (lua_State *L) {
  ត្រឡប់ lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API ចំនួនគត់ lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ត្រឡប់ 1;
}


LUAMOD_API ចំនួនគត់ onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ត្រឡប់ 2;
}


LUAMOD_API ចំនួនគត់ anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ចំនួនគត់)lua_tointeger(L, 1),
                                 (ចំនួនគត់)lua_tointeger(L, 2));
  ត្រឡប់ 1;
} 


LUAMOD_API ចំនួនគត់ luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ត្រឡប់ 1;
}

