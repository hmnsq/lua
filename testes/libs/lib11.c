#అన్తర్భావ "lua.h"

/* function from lib1.c */
LUAMOD_API పూర్ణాంకం lib1_export (lua_State *L);

LUAMOD_API పూర్ణాంకం luaopen_lib11 (lua_State *L) {
  తిరిగి lib1_export(L);
}


