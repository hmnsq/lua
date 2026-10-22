#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

/* function from lib1.c */
LUAMOD_API ਪੂਰਨ lib1_export (lua_State *L);

LUAMOD_API ਪੂਰਨ luaopen_lib11 (lua_State *L) {
  ਵਾਪਸ lib1_export(L);
}


