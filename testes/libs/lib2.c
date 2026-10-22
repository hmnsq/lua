#अन्तर्भाव "lua.h"
#अन्तर्भाव "lauxlib.h"

स्थिर पूर्णाङ्क id (lua_State *L) {
  प्रत्यागम lua_gettop(L);
}


स्थिर ध्रुव संरचना luaL_Reg funcs[] = {
  {"id", id},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णाङ्क luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  प्रत्यागम 1;
}


