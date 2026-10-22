#ಅನ್ತರ್ಭಾವ "lua.h"
#ಅನ್ತರ್ಭಾವ "lauxlib.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ id (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ lua_gettop(L);
}


ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ luaL_Reg funcs[] = {
  {"id", id},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


/* function used by lib11.c */
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  ಪ್ರತ್ಯಾಗಮ 1;
}


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  ಪ್ರತ್ಯಾಗಮ 2;
}


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (ಪೂರ್ಣಾಙ್ಕ)lua_tointeger(L, 1),
                                 (ಪೂರ್ಣಾಙ್ಕ)lua_tointeger(L, 2));
  ಪ್ರತ್ಯಾಗಮ 1;
} 


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  ಪ್ರತ್ಯಾಗಮ 1;
}

