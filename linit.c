/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB


#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stddef.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lualib.h"
#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
स्थिर ध्रुव luaL_Reg stdlibs[] = {
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
  {रिक्त, रिक्त}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API शून्य luaL_openselectedlibs (lua_State *L, पूर्णाङ्क load, पूर्णाङ्क preload) {
  पूर्णाङ्क mask;
  ध्रुव luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  प्रति (lib = stdlibs, mask = 1; lib->name != रिक्त; lib++, mask <<= 1) {
    यदि (load & mask) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_काढा(L, 1);  /* remove result from the stack */
    }
    अन्यथा यदि (preload & mask) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((mask >> 1) == LUA_UTF8LIBK);
  lua_काढा(L, 1);  /* remove PRELOAD table */
}

