#ಅನ್ತರ್ಭಾವ "lua.h"

/* function from lib1.c */
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ lib1_export (lua_State *L);

LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_lib11 (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ lib1_export(L);
}


