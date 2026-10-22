#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"

ਸਥਿਰ ਪੂਰਨ id (lua_State *L) {
  ਵਾਪਸ lua_gettop(L);
}


ਸਥਿਰ ਅਚਲ ਬਣਤਰ luaL_Reg funcs[] = {
  {"id", id},
  {ਨਿਰਾ, ਨਿਰਾ}
};


/* function used by lib11.c */
LUAMOD_API ਪੂਰਨ lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ਵਾਪਸ 1;
}


LUAMOD_API ਪੂਰਨ onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ਵਾਪਸ 2;
}


LUAMOD_API ਪੂਰਨ anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ਪੂਰਨ)lua_tointeger(L, 1),
                                 (ਪੂਰਨ)lua_tointeger(L, 2));
  ਵਾਪਸ 1;
} 


LUAMOD_API ਪੂਰਨ luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ਵਾਪਸ 1;
}

