#அந்தர்பாவ "lua.h"

/* function from lib1.c */
LUAMOD_API முழு lib1_export (lua_State *L);

LUAMOD_API முழு luaopen_lib11 (lua_State *L) {
  திரும்பு lib1_export(L);
}


