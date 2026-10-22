#include "lua.h"
#include "lauxlib.h"

static ကိန်းပြည့် id (lua_State *L) {
  ပြန်ပေး lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API ကိန်းပြည့် lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ပြန်ပေး 1;
}


LUAMOD_API ကိန်းပြည့် onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ပြန်ပေး 2;
}


LUAMOD_API ကိန်းပြည့် anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ကိန်းပြည့်)lua_tointeger(L, 1),
                                 (ကိန်းပြည့်)lua_tointeger(L, 2));
  ပြန်ပေး 1;
} 


LUAMOD_API ကိန်းပြည့် luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ပြန်ပေး 1;
}

