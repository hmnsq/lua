#অন্তর্ভাৱ "lua.h"
#অন্তর্ভাৱ "lauxlib.h"

স্থির পূর্ণাঙ্ক id (lua_State *L) {
  ঘূৰাই lua_gettop(L);
}


স্থির ধ্রুৱ সংরচনা luaL_Reg funcs[] = {
  {"id", id},
  {রিক্ত, রিক্ত}
};


/* function used by lib11.c */
LUAMOD_API পূর্ণাঙ্ক lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ঘূৰাই 1;
}


LUAMOD_API পূর্ণাঙ্ক onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ঘূৰাই 2;
}


LUAMOD_API পূর্ণাঙ্ক anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (পূর্ণাঙ্ক)lua_tointeger(L, 1),
                                 (পূর্ণাঙ্ক)lua_tointeger(L, 2));
  ঘূৰাই 1;
} 


LUAMOD_API পূর্ণাঙ্ক luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ঘূৰাই 1;
}

