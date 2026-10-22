#అన్తర్భావ "lua.h"
#అన్తర్భావ "lauxlib.h"

స్థిర పూర్ణాంకం id (lua_State *L) {
  తిరిగి lua_gettop(L);
}


స్థిర స్థిరరాశి నిర్మాణం luaL_Reg funcs[] = {
  {"id", id},
  {ఖాళీ, ఖాళీ}
};


/* function used by lib11.c */
LUAMOD_API పూర్ణాంకం lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  తిరిగి 1;
}


LUAMOD_API పూర్ణాంకం onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  తిరిగి 2;
}


LUAMOD_API పూర్ణాంకం anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (పూర్ణాంకం)lua_tointeger(L, 1),
                                 (పూర్ణాంకం)lua_tointeger(L, 2));
  తిరిగి 1;
} 


LUAMOD_API పూర్ణాంకం luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  తిరిగి 1;
}

