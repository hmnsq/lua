/*
** $Id: linit.c $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


#నిర్వచించు linit_c
#నిర్వచించు LUA_LIB


#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stddef.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lualib.h"
#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "llimits.h"


/*
** Standard Libraries. (Must be listed in the same ORDER of their
** respective constants LUA_<libname>K.)
*/
స్థిర స్థిరరాశి luaL_Reg stdlibs[] = {
  {LUA_GNAME, luaopen_పునాది},
  {LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_DBLIBNAME, luaopen_debug},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_TABLIBNAME, luaopen_పట్టిక},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {ఖాళీ, ఖాళీ}
};


/*
** require and preload selected standard libraries
*/
LUALIB_API శూన్య luaL_openselectedlibs (lua_State *L, పూర్ణాంకం ఎక్కించు, పూర్ణాంకం preload) {
  పూర్ణాంకం ముసుగు_2;
  స్థిరరాశి luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_పట్టిక);
  కొరకు (lib = stdlibs, ముసుగు_2 = 1; lib->name != ఖాళీ; lib++, ముసుగు_2 <<= 1) {
    ఒకవేళ (ఎక్కించు & ముసుగు_2) {  /* selected? */
      luaL_requiref(L, lib->name, lib->func, 1);  /* require library */
      lua_తీయి(L, 1);  /* remove result from the stack */
    }
    లేకపోతే ఒకవేళ (preload & ముసుగు_2) {  /* selected? */
      lua_pushcfunction(L, lib->func);
      lua_setfield(L, -2, lib->name);  /* add library to PRELOAD table */
    }
  }
  lua_assert((ముసుగు_2 >> 1) == LUA_UTF8LIBK);
  lua_తీయి(L, 1);  /* remove PRELOAD table */
}

