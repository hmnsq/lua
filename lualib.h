/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#که_نه_تعریف_شوی lualib_h
#تعریف_کړه lualib_h

#ضمن "lua.h"


/* version suffix for environment variable names */
#تعریف_کړه LUA_VERSUFFIX          "_" LUA_نسخه_MAJOR "_" LUA_نسخه_MINOR

#تعریف_کړه LUA_GLIBK		1
LUAMOD_API صحيح (luaopen_بنسټ) (lua_State *L);

#تعریف_کړه LUA_LOADLIBNAME	"package"
#تعریف_کړه LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API صحيح (luaopen_package) (lua_State *L);


#تعریف_کړه LUA_COLIBNAME	"coroutine"
#تعریف_کړه LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API صحيح (luaopen_coroutine) (lua_State *L);

#تعریف_کړه LUA_DBLIBNAME	"debug"
#تعریف_کړه LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API صحيح (luaopen_debug) (lua_State *L);

#تعریف_کړه LUA_IOLIBNAME	"io"
#تعریف_کړه LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API صحيح (luaopen_io) (lua_State *L);

#تعریف_کړه LUA_MATHLIBNAME	"math"
#تعریف_کړه LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API صحيح (luaopen_math) (lua_State *L);

#تعریف_کړه LUA_OSLIBNAME	"os"
#تعریف_کړه LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API صحيح (luaopen_os) (lua_State *L);

#تعریف_کړه LUA_STRLIBNAME	"string"
#تعریف_کړه LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API صحيح (luaopen_string) (lua_State *L);

#تعریف_کړه LUA_TABLIBNAME	"table"
#تعریف_کړه LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API صحيح (luaopen_جدول) (lua_State *L);

#تعریف_کړه LUA_UTF8LIBNAME	"utf8"
#تعریف_کړه LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API صحيح (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API فراغ (luaL_openselectedlibs) (lua_State *L, صحيح بارول, صحيح preload);

/* open all libraries */
#تعریف_کړه luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#د_که_پای
