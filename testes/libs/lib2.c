#અન્તર્ભાવ "lua.h"
#અન્તર્ભાવ "lauxlib.h"

સ્થિર પૂર્ણાંક id (lua_State *L) {
  પાછા lua_gettop(L);
}


સ્થિર અચલ સંરચના luaL_Reg funcs[] = {
  {"id", id},
  {ખાલી, ખાલી}
};


LUAMOD_API પૂર્ણાંક luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  પાછા 1;
}


