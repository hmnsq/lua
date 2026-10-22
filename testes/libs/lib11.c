#ଅନ୍ତର୍ଭାଵ "lua.h"

/* function from lib1.c */
LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lib1_export (lua_State *L);

LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_lib11 (lua_State *L) {
  ଫେରନ୍ତୁ lib1_export(L);
}


