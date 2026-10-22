/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#ಅನ್ತರ್ಭಾವ "lua.h"


/* version suffix for environment variable names */
#define LUA_VERSUFFIX          "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

#define LUA_GLIBK		1
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_base) (lua_State *L);

#define LUA_LOADLIBNAME	"package"
#define LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_package) (lua_State *L);


#define LUA_COLIBNAME	"coroutine"
#define LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_coroutine) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
#define LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_debug) (lua_State *L);

#define LUA_IOLIBNAME	"io"
#define LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_io) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
#define LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_math) (lua_State *L);

#define LUA_OSLIBNAME	"os"
#define LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_os) (lua_State *L);

#define LUA_STRLIBNAME	"string"
#define LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_string) (lua_State *L);

#define LUA_TABLIBNAME	"table"
#define LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_table) (lua_State *L);

#define LUA_UTF8LIBNAME	"utf8"
#define LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಙ್ಕ (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API ಶೂನ್ಯ (luaL_openselectedlibs) (lua_State *L, ಪೂರ್ಣಾಙ್ಕ load, ಪೂರ್ಣಾಙ್ಕ preload);

/* open all libraries */
#define luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#endif
