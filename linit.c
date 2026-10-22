/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ linit_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB


#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
ਸਥਿਰ ਅਚਲ luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_ਅਧਾਰ},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_ਸਾਰਣੀ},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {ਨਿਰਾ, ਨਿਰਾ}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API ਖਾਲੀ luaL_openselectedlibs (lua_State *L, ਪੂਰਨ ਲੱਦੋ, ਪੂਰਨ preload) {
  ਪੂਰਨ ਨਕਾਬ_2;
  ਅਚਲ luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_ਸਾਰਣੀ);
  ਲਈ (lib = stdlibs, ਨਕਾਬ_2 = 1; lib->name != ਨਿਰਾ; lib++, ਨਕਾਬ_2 <<= 1) {
    ਜੇ (ਲੱਦੋ & ਨਕਾਬ_2) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_ਕੱਢੋ(L, 1);  /* remove result from the stack */
    }
    ਨਹੀਂਤਾਂ ਜੇ (preload & ਨਕਾਬ_2) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((ਨਕਾਬ_2 >> 1) == LUA_UTF8LIBK);
  lua_ਕੱਢੋ(L, 1);  /* remove PRELOAD table */
}

