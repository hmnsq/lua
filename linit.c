/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#ವ್ಯಾಖ್ಯಾನಿಸು linit_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB


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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_ತಳ},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_ಕೋಷ್ಟಕ},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {ಖಾಲಿ, ಖಾಲಿ}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API ಶೂನ್ಯ luaL_openselectedlibs (lua_State *L, ಪೂರ್ಣಾಂಕ ತುಂಬು, ಪೂರ್ಣಾಂಕ preload) {
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2;
  ಸ್ಥಿರಾಂಕ luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_ಕೋಷ್ಟಕ);
  ಗಾಗಿ (lib = stdlibs, ಮುಖವಾಡ_2 = 1; lib->name != ಖಾಲಿ; lib++, ಮುಖವಾಡ_2 <<= 1) {
    ಆದರೆ (ತುಂಬು & ಮುಖವಾಡ_2) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_ತೆಗೆ(L, 1);  /* remove result from the stack */
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (preload & ಮುಖವಾಡ_2) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((ಮುಖವಾಡ_2 >> 1) == LUA_UTF8LIBK);
  lua_ತೆಗೆ(L, 1);  /* remove PRELOAD table */
}

