/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#اگر_غیر_تعریف_شدہ lualib_h
#تعریف_کرو lualib_h

#شامل "lua.h"


/* version suffix for environment variable names */
#تعریف_کرو LUA_VERSUFFIX          "_" LUA_نسخہ_MAJOR "_" LUA_نسخہ_MINOR

#تعریف_کرو LUA_GLIBK		1
LUAMOD_API صحیح (luaopen_بنیاد) (lua_State *L);

#تعریف_کرو LUA_LOADLIBNAME	"package"
#تعریف_کرو LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API صحیح (luaopen_package) (lua_State *L);


#تعریف_کرو LUA_COLIBNAME	"coroutine"
#تعریف_کرو LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API صحیح (luaopen_coroutine) (lua_State *L);

#تعریف_کرو LUA_DBLIBNAME	"debug"
#تعریف_کرو LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API صحیح (luaopen_debug) (lua_State *L);

#تعریف_کرو LUA_IOLIBNAME	"io"
#تعریف_کرو LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API صحیح (luaopen_io) (lua_State *L);

#تعریف_کرو LUA_MATHLIBNAME	"math"
#تعریف_کرو LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API صحیح (luaopen_math) (lua_State *L);

#تعریف_کرو LUA_OSLIBNAME	"os"
#تعریف_کرو LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API صحیح (luaopen_os) (lua_State *L);

#تعریف_کرو LUA_STRLIBNAME	"string"
#تعریف_کرو LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API صحیح (luaopen_string) (lua_State *L);

#تعریف_کرو LUA_TABLIBNAME	"table"
#تعریف_کرو LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API صحیح (luaopen_جدول) (lua_State *L);

#تعریف_کرو LUA_UTF8LIBNAME	"utf8"
#تعریف_کرو LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API صحیح (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API خالی (luaL_openselectedlibs) (lua_State *L, صحیح بھرو, صحیح preload);

/* open all libraries */
#تعریف_کرو luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#اگر_ختم
