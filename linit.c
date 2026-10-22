/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#घोषणा linit_c
#घोषणा LUA_LIB


#समावेश "lprefix.h"


#समावेश <stddef.h>

#समावेश "lua.h"

#समावेश "lualib.h"
#समावेश "lauxlib.h"
#समावेश "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
अटल स्थिर luaL_Reg stdlibs[] = {
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
  {शून्य, शून्य}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API व्योम luaL_openselectedlibs (lua_State *L, पूर्णांक load, पूर्णांक preload) {
  पूर्णांक mask;
  स्थिर luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  क्रम (lib = stdlibs, mask = 1; lib->name != शून्य; lib++, mask <<= 1) {
    अगर (load & mask) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_निकालो(L, 1);  /* remove result from the stack */
    }
    अन्यथा अगर (preload & mask) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((mask >> 1) == LUA_UTF8LIBK);
  lua_निकालो(L, 1);  /* remove PRELOAD table */
}

