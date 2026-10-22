#ಅನ್ತರ್ಭಾವ "lua.h"

/* function from lib1.c */
LUAMOD_API ಪೂರ್ಣಾಂಕ lib1_export (lua_State *L);

LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_lib11 (lua_State *L) {
  ಹಿಂತಿರುಗಿ lib1_export(L);
}


