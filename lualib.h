/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#నిర్వచితం_కాకపోతే lualib_h
#నిర్వచించు lualib_h

#అన్తర్భావ "lua.h"


/* version suffix for environment variable names */
#నిర్వచించు LUA_VERSUFFIX          "_" LUA_సంస్కరణ_MAJOR "_" LUA_సంస్కరణ_MINOR

#నిర్వచించు LUA_GLIBK		1
LUAMOD_API పూర్ణాంకం (luaopen_పునాది) (lua_State *L);

#నిర్వచించు LUA_LOADLIBNAME	"package"
#నిర్వచించు LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_package) (lua_State *L);


#నిర్వచించు LUA_COLIBNAME	"coroutine"
#నిర్వచించు LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_coroutine) (lua_State *L);

#నిర్వచించు LUA_DBLIBNAME	"debug"
#నిర్వచించు LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_debug) (lua_State *L);

#నిర్వచించు LUA_IOLIBNAME	"io"
#నిర్వచించు LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_io) (lua_State *L);

#నిర్వచించు LUA_MATHLIBNAME	"math"
#నిర్వచించు LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_math) (lua_State *L);

#నిర్వచించు LUA_OSLIBNAME	"os"
#నిర్వచించు LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_os) (lua_State *L);

#నిర్వచించు LUA_STRLIBNAME	"string"
#నిర్వచించు LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_string) (lua_State *L);

#నిర్వచించు LUA_TABLIBNAME	"table"
#నిర్వచించు LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_పట్టిక) (lua_State *L);

#నిర్వచించు LUA_UTF8LIBNAME	"utf8"
#నిర్వచించు LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API పూర్ణాంకం (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API శూన్య (luaL_openselectedlibs) (lua_State *L, పూర్ణాంకం ఎక్కించు, పూర్ణాంకం preload);

/* open all libraries */
#నిర్వచించు luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#అయితే_ముగింపు
