#include "lua.h"
#include "lauxlib.h"

static จำนวนเต็ม id (lua_State *L) {
  คืน lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API จำนวนเต็ม lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  คืน 1;
}


LUAMOD_API จำนวนเต็ม onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  คืน 2;
}


LUAMOD_API จำนวนเต็ม anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (จำนวนเต็ม)lua_tointeger(L, 1),
                                 (จำนวนเต็ม)lua_tointeger(L, 2));
  คืน 1;
} 


LUAMOD_API จำนวนเต็ม luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  คืน 1;
}

