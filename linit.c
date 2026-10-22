/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#تعریف_کن linit_c
#تعریف_کن LUA_LIB


#ضمن "lprefix.h"


#ضمن <stddef.h>

#ضمن "lua.h"

#ضمن "lualib.h"
#ضمن "lauxlib.h"
#ضمن "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
ساكن ثابت luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_پایه},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_جدول},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {عدم, عدم}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API فراغ luaL_openselectedlibs (lua_State *L, صحيح بارگیری, صحيح preload) {
  صحيح ماسک_2;
  ثابت luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_جدول);
  لكل (lib = stdlibs, ماسک_2 = 1; lib->name != عدم; lib++, ماسک_2 <<= 1) {
    إذا (بارگیری & ماسک_2) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_برداشت(L, 1);  /* remove result from the stack */
    }
    وإلا إذا (preload & ماسک_2) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((ماسک_2 >> 1) == LUA_UTF8LIBK);
  lua_برداشت(L, 1);  /* remove PRELOAD table */
}

