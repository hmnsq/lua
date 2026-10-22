/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#تعریف_کرو linit_c
#تعریف_کرو LUA_LIB


#شامل "lprefix.h"


#شامل <stddef.h>

#شامل "lua.h"

#شامل "lualib.h"
#شامل "lauxlib.h"
#شامل "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
ساکن ثابت luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_بنیاد},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_جدول},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {تہی, تہی}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API خالی luaL_openselectedlibs (lua_State *L, صحیح بھرو, صحیح preload) {
  صحیح نقاب_2;
  ثابت luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_جدول);
  برائے (lib = stdlibs, نقاب_2 = 1; lib->name != تہی; lib++, نقاب_2 <<= 1) {
    اگر (بھرو & نقاب_2) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_نکالو(L, 1);  /* remove result from the stack */
    }
    ورنہ اگر (preload & نقاب_2) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((نقاب_2 >> 1) == LUA_UTF8LIBK);
  lua_نکالو(L, 1);  /* remove PRELOAD table */
}

