#അന്തര്ഭാവ "lua.h"
#അന്തര്ഭാവ "lauxlib.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ id (lua_State *L) {
  തിരികെ lua_gettop(L);
}


സ്ഥിതം സ്ഥിരം ഘടന luaL_Reg funcs[] = {
  {"id", id},
  {ശൂന്യ, ശൂന്യ}
};


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  തിരികെ 1;
}


