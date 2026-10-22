#include "lua.h"
#include "lauxlib.h"

static පූර්ණ_සංඛ්‍යා id (lua_State *L) {
  ආපසු lua_gettop(L);
}


static const sa_nracanaa luaL_Reg funcs[] = {
  {"id", id},
  {NULL, NULL}
};


/* function used by lib11.c */
LUAMOD_API පූර්ණ_සංඛ්‍යා lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ආපසු 1;
}


LUAMOD_API පූර්ණ_සංඛ්‍යා onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ආපසු 2;
}


LUAMOD_API පූර්ණ_සංඛ්‍යා anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (පූර්ණ_සංඛ්‍යා)lua_tointeger(L, 1),
                                 (පූර්ණ_සංඛ්‍යා)lua_tointeger(L, 2));
  ආපසු 1;
} 


LUAMOD_API පූර්ණ_සංඛ්‍යා luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ආපසු 1;
}

