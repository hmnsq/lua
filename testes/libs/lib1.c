#അന്തര്ഭാവ "lua.h"
#അന്തര്ഭാവ "lauxlib.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ id (lua_State *L) {
  തിരികെ lua_gettop(L);
}


സ്ഥിതം സ്ഥിരം ഘടന luaL_Reg funcs[] = {
  {"id", id},
  {ശൂന്യ, ശൂന്യ}
};


/* function used by lib11.c */
LUAMOD_API പൂർണ്ണസംഖ്യ lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  തിരികെ 1;
}


LUAMOD_API പൂർണ്ണസംഖ്യ onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  തിരികെ 2;
}


LUAMOD_API പൂർണ്ണസംഖ്യ anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (പൂർണ്ണസംഖ്യ)lua_tointeger(L, 1),
                                 (പൂർണ്ണസംഖ്യ)lua_tointeger(L, 2));
  തിരികെ 1;
} 


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  തിരികെ 1;
}

