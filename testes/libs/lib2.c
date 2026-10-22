#ಅನ್ತರ್ಭಾವ "lua.h"
#ಅನ್ತರ್ಭಾವ "lauxlib.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ id (lua_State *L) {
  ಹಿಂತಿರುಗಿ lua_gettop(L);
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ luaL_Reg funcs[] = {
  {"id", id},
  {ಖಾಲಿ, ಖಾಲಿ}
};


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  luaL_newlib(L, funcs);
  ಹಿಂತಿರುಗಿ 1;
}


