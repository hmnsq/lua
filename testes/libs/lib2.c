#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"

ਸਥਿਰ ਪੂਰਨ id (lua_State *L) {
  ਵਾਪਸ lua_gettop(L);
}


ਸਥਿਰ ਅਚਲ ਬਣਤਰ luaL_Reg funcs[] = {
  {"id", id},
  {ਨਿਰਾ, ਨਿਰਾ}
};


LUAMOD_API ਪੂਰਨ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ਵਾਪਸ 1;
}


