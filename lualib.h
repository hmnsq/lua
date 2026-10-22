/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#جيڪڏهن_وصف_نه_ٿيل lualib_h
#وصف_ڪريو lualib_h

#ضمن "lua.h"


/* version suffix for environment variable names */
#وصف_ڪريو LUA_VERSUFFIX          "_" LUA_نسخو_MAJOR "_" LUA_نسخو_MINOR

#وصف_ڪريو LUA_GLIBK		1
LUAMOD_API صحيح (luaopen_بنياد) (lua_State *L);

#وصف_ڪريو LUA_LOADLIBNAME	"package"
#وصف_ڪريو LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API صحيح (luaopen_package) (lua_State *L);


#وصف_ڪريو LUA_COLIBNAME	"coroutine"
#وصف_ڪريو LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API صحيح (luaopen_coroutine) (lua_State *L);

#وصف_ڪريو LUA_DBLIBNAME	"debug"
#وصف_ڪريو LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API صحيح (luaopen_debug) (lua_State *L);

#وصف_ڪريو LUA_IOLIBNAME	"io"
#وصف_ڪريو LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API صحيح (luaopen_io) (lua_State *L);

#وصف_ڪريو LUA_MATHLIBNAME	"math"
#وصف_ڪريو LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API صحيح (luaopen_math) (lua_State *L);

#وصف_ڪريو LUA_OSLIBNAME	"os"
#وصف_ڪريو LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API صحيح (luaopen_os) (lua_State *L);

#وصف_ڪريو LUA_STRLIBNAME	"string"
#وصف_ڪريو LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API صحيح (luaopen_string) (lua_State *L);

#وصف_ڪريو LUA_TABLIBNAME	"table"
#وصف_ڪريو LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API صحيح (luaopen_جدول) (lua_State *L);

#وصف_ڪريو LUA_UTF8LIBNAME	"utf8"
#وصف_ڪريو LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API صحيح (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API فراغ (luaL_openselectedlibs) (lua_State *L, صحيح ڀريو, صحيح preload);

/* open all libraries */
#وصف_ڪريو luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#جيڪڏهن_پورو
