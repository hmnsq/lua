#অন্তর্ভাব "lua.h"

/* function from lib1.c */
LUAMOD_API পূর্ণাঙ্ক lib1_export (lua_State *L);

LUAMOD_API পূর্ণাঙ্ক luaopen_lib11 (lua_State *L) {
  প্রত্যাগম lib1_export(L);
}


