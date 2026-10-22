#અન્તર્ભાવ "lua.h"

/* function from lib1.c */
LUAMOD_API પૂર્ણાંક lib1_export (lua_State *L);

LUAMOD_API પૂર્ણાંક luaopen_lib11 (lua_State *L) {
  પાછા lib1_export(L);
}


