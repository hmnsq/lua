#அந்தர்பாவ "lua.h"
#அந்தர்பாவ "lauxlib.h"

நிலையான முழு id (lua_State *L) {
  திரும்பு lua_gettop(L);
}


நிலையான மாறிலி கட்டமைப்பு luaL_Reg funcs[] = {
  {"id", id},
  {வெற்று, வெற்று}
};


/* function used by lib11.c */
LUAMOD_API முழு lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  திரும்பு 1;
}


LUAMOD_API முழு onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  திரும்பு 2;
}


LUAMOD_API முழு anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (முழு)lua_tointeger(L, 1),
                                 (முழு)lua_tointeger(L, 2));
  திரும்பு 1;
} 


LUAMOD_API முழு luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  திரும்பு 1;
}

