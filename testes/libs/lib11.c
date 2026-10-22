#അന്തര്ഭാവ "lua.h"

/* function from lib1.c */
LUAMOD_API പൂർണ്ണസംഖ്യ lib1_export (lua_State *L);

LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_lib11 (lua_State *L) {
  തിരികെ lib1_export(L);
}


