/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#यदि_अपरिभाषितम् lualib_h
#परिभाषय lualib_h

#अन्तर्भाव "lua.h"


/* version suffix for environment variable names */
#परिभाषय LUA_VERSUFFIX          "_" LUA_संस्करणम्_MAJOR "_" LUA_संस्करणम्_MINOR

#परिभाषय LUA_GLIBK		1
LUAMOD_API पूर्णाङ्क (luaopen_आधारः) (lua_State *L);

#परिभाषय LUA_LOADLIBNAME	"package"
#परिभाषय LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_package) (lua_State *L);


#परिभाषय LUA_COLIBNAME	"coroutine"
#परिभाषय LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_coroutine) (lua_State *L);

#परिभाषय LUA_DBLIBNAME	"debug"
#परिभाषय LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_debug) (lua_State *L);

#परिभाषय LUA_IOLIBNAME	"io"
#परिभाषय LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_io) (lua_State *L);

#परिभाषय LUA_MATHLIBNAME	"math"
#परिभाषय LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_math) (lua_State *L);

#परिभाषय LUA_OSLIBNAME	"os"
#परिभाषय LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_os) (lua_State *L);

#परिभाषय LUA_STRLIBNAME	"string"
#परिभाषय LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_string) (lua_State *L);

#परिभाषय LUA_TABLIBNAME	"table"
#परिभाषय LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_सारणी) (lua_State *L);

#परिभाषय LUA_UTF8LIBNAME	"utf8"
#परिभाषय LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API पूर्णाङ्क (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API शून्य (luaL_openselectedlibs) (lua_State *L, पूर्णाङ्क आरोपय, पूर्णाङ्क preload);

/* open all libraries */
#परिभाषय luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#यदि_अन्तः
