#অন্তর্ভাব "lua.h"
#অন্তর্ভাব "lauxlib.h"

স্থির পূর্ণসংখ্যা id (lua_State *L) {
  ফেরত lua_gettop(L);
}


স্থির ধ্রুবক কাঠামো luaL_Reg funcs[] = {
  {"id", id},
  {ফাঁকা, ফাঁকা}
};


/* function used by lib11.c */
LUAMOD_API পূর্ণসংখ্যা lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ফেরত 1;
}


LUAMOD_API পূর্ণসংখ্যা onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ফেরত 2;
}


LUAMOD_API পূর্ণসংখ্যা anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (পূর্ণসংখ্যা)lua_tointeger(L, 1),
                                 (পূর্ণসংখ্যা)lua_tointeger(L, 2));
  ফেরত 1;
} 


LUAMOD_API পূর্ণসংখ্যা luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ফেরত 1;
}

