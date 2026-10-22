#ಅನ್ತರ್ಭಾವ "lua.h"
#ಅನ್ತರ್ಭಾವ "lauxlib.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ id (lua_State *L) {
  ಹಿಂತಿರುಗಿ lua_gettop(L);
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ luaL_Reg funcs[] = {
  {"id", id},
  {ಖಾಲಿ, ಖಾಲಿ}
};


/* function used by lib11.c */
LUAMOD_API ಪೂರ್ಣಾಂಕ lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ಹಿಂತಿರುಗಿ 1;
}


LUAMOD_API ಪೂರ್ಣಾಂಕ onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ಹಿಂತಿರುಗಿ 2;
}


LUAMOD_API ಪೂರ್ಣಾಂಕ anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ಪೂರ್ಣಾಂಕ)lua_tointeger(L, 1),
                                 (ಪೂರ್ಣಾಂಕ)lua_tointeger(L, 2));
  ಹಿಂತಿರುಗಿ 1;
} 


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ಹಿಂತಿರುಗಿ 1;
}

