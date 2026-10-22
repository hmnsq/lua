/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#اگر_تعریف_نشده lualib_h
#تعریف_کن lualib_h

#ضمن "lua.h"


/* version suffix for environment variable names */
#تعریف_کن LUA_VERSUFFIX          "_" LUA_نسخه_MAJOR "_" LUA_نسخه_MINOR

#تعریف_کن LUA_GLIBK		1
LUAMOD_API صحيح (luaopen_پایه) (lua_State *L);

#تعریف_کن LUA_LOADLIBNAME	"package"
#تعریف_کن LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API صحيح (luaopen_package) (lua_State *L);


#تعریف_کن LUA_COLIBNAME	"coroutine"
#تعریف_کن LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API صحيح (luaopen_coroutine) (lua_State *L);

#تعریف_کن LUA_DBLIBNAME	"debug"
#تعریف_کن LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API صحيح (luaopen_debug) (lua_State *L);

#تعریف_کن LUA_IOLIBNAME	"io"
#تعریف_کن LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API صحيح (luaopen_io) (lua_State *L);

#تعریف_کن LUA_MATHLIBNAME	"math"
#تعریف_کن LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API صحيح (luaopen_math) (lua_State *L);

#تعریف_کن LUA_OSLIBNAME	"os"
#تعریف_کن LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API صحيح (luaopen_os) (lua_State *L);

#تعریف_کن LUA_STRLIBNAME	"string"
#تعریف_کن LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API صحيح (luaopen_string) (lua_State *L);

#تعریف_کن LUA_TABLIBNAME	"table"
#تعریف_کن LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API صحيح (luaopen_جدول) (lua_State *L);

#تعریف_کن LUA_UTF8LIBNAME	"utf8"
#تعریف_کن LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API صحيح (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API فراغ (luaL_openselectedlibs) (lua_State *L, صحيح بارگیری, صحيح preload);

/* open all libraries */
#تعریف_کن luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#پایان_اگر
