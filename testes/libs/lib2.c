#অন্তর্ভাৱ "lua.h"
#অন্তর্ভাৱ "lauxlib.h"

স্থির পূর্ণাঙ্ক id (lua_State *L) {
  ঘূৰাই lua_gettop(L);
}


স্থির ধ্রুৱ সংরচনা luaL_Reg funcs[] = {
  {"id", id},
  {রিক্ত, রিক্ত}
};


LUAMOD_API পূর্ণাঙ্ক luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ঘূৰাই 1;
}


