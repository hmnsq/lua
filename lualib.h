/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#جے_بنا_تعریف lualib_h
#تعریف_کرو lualib_h

#ضمن "lua.h"


/* version suffix for environment variable names */
#تعریف_کرو LUA_VERSUFFIX          "_" LUA_نسخہ_MAJOR "_" LUA_نسخہ_MINOR

#تعریف_کرو LUA_GLIBK		1
LUAMOD_API صحيح (luaopen_ادھار) (lua_State *L);

#تعریف_کرو LUA_LOADLIBNAME	"package"
#تعریف_کرو LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API صحيح (luaopen_package) (lua_State *L);


#تعریف_کرو LUA_COLIBNAME	"coroutine"
#تعریف_کرو LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API صحيح (luaopen_coroutine) (lua_State *L);

#تعریف_کرو LUA_DBLIBNAME	"debug"
#تعریف_کرو LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API صحيح (luaopen_debug) (lua_State *L);

#تعریف_کرو LUA_IOLIBNAME	"io"
#تعریف_کرو LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API صحيح (luaopen_io) (lua_State *L);

#تعریف_کرو LUA_MATHLIBNAME	"math"
#تعریف_کرو LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API صحيح (luaopen_math) (lua_State *L);

#تعریف_کرو LUA_OSLIBNAME	"os"
#تعریف_کرو LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API صحيح (luaopen_os) (lua_State *L);

#تعریف_کرو LUA_STRLIBNAME	"string"
#تعریف_کرو LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API صحيح (luaopen_string) (lua_State *L);

#تعریف_کرو LUA_TABLIBNAME	"table"
#تعریف_کرو LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API صحيح (luaopen_جدول) (lua_State *L);

#تعریف_کرو LUA_UTF8LIBNAME	"utf8"
#تعریف_کرو LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API صحيح (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API فراغ (luaL_openselectedlibs) (lua_State *L, صحيح لدو, صحيح preload);

/* open all libraries */
#تعریف_کرو luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#جے_انت
