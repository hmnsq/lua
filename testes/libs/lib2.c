#அந்தர்பாவ "lua.h"
#அந்தர்பாவ "lauxlib.h"

நிலையான முழு id (lua_State *L) {
  திரும்பு lua_gettop(L);
}


நிலையான மாறிலி கட்டமைப்பு luaL_Reg funcs[] = {
  {"id", id},
  {வெற்று, வெற்று}
};


LUAMOD_API முழு luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  திரும்பு 1;
}


