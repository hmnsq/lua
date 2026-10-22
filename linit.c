/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB


#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stddef.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
സ്ഥിതം സ്ഥിരം luaL_Reg stdlibs[] = {
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
  {ശൂന്യ, ശൂന്യ}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API ശൂന്യം luaL_openselectedlibs (lua_State *L, പൂർണ്ണസംഖ്യ load, പൂർണ്ണസംഖ്യ preload) {
  പൂർണ്ണസംഖ്യ mask;
  സ്ഥിരം luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  വേണ്ടി (lib = stdlibs, mask = 1; lib->name != ശൂന്യ; lib++, mask <<= 1) {
    എങ്കിൽ (load & mask) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_എടുക്കുക(L, 1);  /* remove result from the stack */
    }
    അല്ലെങ്കിൽ എങ്കിൽ (preload & mask) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((mask >> 1) == LUA_UTF8LIBK);
  lua_എടുക്കുക(L, 1);  /* remove PRELOAD table */
}

