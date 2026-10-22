#ಅನ್ತರ್ಭಾವ "lua.h"
#ಅನ್ತರ್ಭಾವ "lauxlib.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ id (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ lua_gettop(L);
}


ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ luaL_Reg funcs[] = {
  {"id", id},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ಪ್ರತ್ಯಾಗಮ 1;
}


