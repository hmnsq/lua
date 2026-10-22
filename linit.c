/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB


#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <stddef.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
স্থির ধ্রুব luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_base},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {রিক্ত, রিক্ত}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API শূন্য luaL_openselectedlibs (lua_State *L, পূর্ণাঙ্ক load, পূর্ণাঙ্ক preload) {
  পূর্ণাঙ্ক mask;
  ধ্রুব luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  প্রতি (lib = stdlibs, mask = 1; lib->name != রিক্ত; lib++, mask <<= 1) {
    যদি (load & mask) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_বের_করো(L, 1);  /* remove result from the stack */
    }
    অন্যথা যদি (preload & mask) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((mask >> 1) == LUA_UTF8LIBK);
  lua_বের_করো(L, 1);  /* remove PRELOAD table */
}

