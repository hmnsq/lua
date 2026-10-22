#অন্তর্ভাব "lua.h"
#অন্তর্ভাব "lauxlib.h"

স্থির পূর্ণসংখ্যা id (lua_State *L) {
  ফেরত lua_gettop(L);
}


স্থির ধ্রুবক কাঠামো luaL_Reg funcs[] = {
  {"id", id},
  {ফাঁকা, ফাঁকা}
};


LUAMOD_API পূর্ণসংখ্যা luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ফেরত 1;
}


