#अन्तर्भाव "lua.h"
#अन्तर्भाव "lauxlib.h"

स्थिर पूर्णाङ्क id (lua_State *L) {
  पटिनिवत्त lua_gettop(L);
}


स्थिर ध्रुव संरचना luaL_Reg funcs[] = {
  {"id", id},
  {रिक्त, रिक्त}
};


/* function used by lib11.c */
LUAMOD_API पूर्णाङ्क lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  पटिनिवत्त 1;
}


LUAMOD_API पूर्णाङ्क onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  पटिनिवत्त 2;
}


LUAMOD_API पूर्णाङ्क anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (पूर्णाङ्क)lua_tointeger(L, 1),
                                 (पूर्णाङ्क)lua_tointeger(L, 2));
  पटिनिवत्त 1;
} 


LUAMOD_API पूर्णाङ्क luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  पटिनिवत्त 1;
}

