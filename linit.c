/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB


#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stddef.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
ಸ್ಥಿರ ಧ್ರುವ luaL_Reg stdlibs[] = {
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
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API ಶೂನ್ಯ luaL_openselectedlibs (lua_State *L, ಪೂರ್ಣಾಙ್ಕ load, ಪೂರ್ಣಾಙ್ಕ preload) {
  ಪೂರ್ಣಾಙ್ಕ mask;
  ಧ್ರುವ luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  ಪ್ರತಿ (lib = stdlibs, mask = 1; lib->name != ರಿಕ್ತ; lib++, mask <<= 1) {
    ಯದಿ (load & mask) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_ತೆಗೆ(L, 1);  /* remove result from the stack */
    }
    ಅನ್ಯಥಾ ಯದಿ (preload & mask) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((mask >> 1) == LUA_UTF8LIBK);
  lua_ತೆಗೆ(L, 1);  /* remove PRELOAD table */
}

