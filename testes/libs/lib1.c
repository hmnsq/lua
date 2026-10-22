#ضمن "lua.h"
#ضمن "lauxlib.h"

ساكن صحيح id (lua_State *L) {
  أرجع lua_gettop(L);
}


ساكن ثابت بنية luaL_Reg funcs[] = {
  {"id", id},
  {عدم, عدم}
};


/* function used by lib11.c */
LUAMOD_API صحيح lib1_export (lua_State *L) {
  lua_pushstring(L, "exported");
  أرجع 1;
}


LUAMOD_API صحيح onefunction (lua_State *L) {
  luaL_checkversion(L);
  lua_settop(L, 2);
  lua_pushvalue(L, 1);
  أرجع 2;
}


LUAMOD_API صحيح anotherfunc (lua_State *L) {
  luaL_checkversion(L);
  lua_pushfstring(L, "%d%%%d\n", (صحيح)lua_tointeger(L, 1),
                                 (صحيح)lua_tointeger(L, 2));
  أرجع 1;
} 


LUAMOD_API صحيح luaopen_lib1_sub (lua_State *L) {
  lua_setglobal(L, "y");  /* 2nd arg: extra value (file name) */
  lua_setglobal(L, "x");  /* 1st arg: module name */
  luaL_newlib(L, funcs);
  أرجع 1;
}

