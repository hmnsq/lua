#અન્તર્ભાવ "lua.h"
#અન્તર્ભાવ "lauxlib.h"

સ્થિર પૂર્ણાંક id (lua_State *L) {
  પાછા lua_gettop(L);
}


સ્થિર અચલ સંરચના luaL_Reg funcs[] = {
  {"id", id},
  {ખાલી, ખાલી}
};


/* function used by lib11.c */
LUAMOD_API પૂર્ણાંક lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  પાછા 1;
}


LUAMOD_API પૂર્ણાંક onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  પાછા 2;
}


LUAMOD_API પૂર્ણાંક anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (પૂર્ણાંક)lua_tointeger(L, 1),
                                 (પૂર્ણાંક)lua_tointeger(L, 2));
  પાછા 1;
} 


LUAMOD_API પૂર્ણાંક luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  પાછા 1;
}

