#ଅନ୍ତର୍ଭାଵ "lua.h"
#ଅନ୍ତର୍ଭାଵ "lauxlib.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା id (lua_State *L) {
  ଫେରନ୍ତୁ lua_gettop(L);
}


ସ୍ଥିର ଅଚଳ ସଂରଚନା luaL_Reg funcs[] = {
  {"id", id},
  {ଖାଲି, ଖାଲି}
};


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ଫେରନ୍ତୁ 1;
}


