#అన్తర్భావ "lua.h"
#అన్తర్భావ "lauxlib.h"

స్థిర పూర్ణాంకం id (lua_State *L) {
  తిరిగి lua_gettop(L);
}


స్థిర స్థిరరాశి నిర్మాణం luaL_Reg funcs[] = {
  {"id", id},
  {ఖాళీ, ఖాళీ}
};


LUAMOD_API పూర్ణాంకం luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  తిరిగి 1;
}


