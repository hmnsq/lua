#ଅନ୍ତର୍ଭାଵ "lua.h"
#ଅନ୍ତର୍ଭାଵ "lauxlib.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା id (lua_State *L) {
  ଫେରନ୍ତୁ lua_gettop(L);
}


ସ୍ଥିର ଅଚଳ ସଂରଚନା luaL_Reg funcs[] = {
  {"id", id},
  {ଖାଲି, ଖାଲି}
};


/* function used by lib11.c */
LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ଫେରନ୍ତୁ 1;
}


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ଫେରନ୍ତୁ 2;
}


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)lua_tointeger(L, 1),
                                 (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)lua_tointeger(L, 2));
  ଫେରନ୍ତୁ 1;
} 


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ଫେରନ୍ତୁ 1;
}

