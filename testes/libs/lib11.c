#অন্তর্ভাব "lua.h"

/* function from lib1.c */
LUAMOD_API পূর্ণসংখ্যা lib1_export (lua_State *L);

LUAMOD_API পূর্ণসংখ্যা luaopen_lib11 (lua_State *L) {
  ফেরত lib1_export(L);
}


