/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lualib_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lualib_h

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"


/* version suffix for environment variable names */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_VERSUFFIX          "_" LUA_ਸੰਸਕਰਣ_MAJOR "_" LUA_ਸੰਸਕਰਣ_MINOR

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_GLIBK		1
LUAMOD_API ਪੂਰਨ (luaopen_ਅਧਾਰ) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LOADLIBNAME	"package"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_package) (lua_State *L);


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_COLIBNAME	"coroutine"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_coroutine) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_DBLIBNAME	"debug"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_debug) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_IOLIBNAME	"io"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_io) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MATHLIBNAME	"math"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_math) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OSLIBNAME	"os"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_os) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_STRLIBNAME	"string"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_string) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TABLIBNAME	"table"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_ਸਾਰਣੀ) (lua_State *L);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_UTF8LIBNAME	"utf8"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API ਪੂਰਨ (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API ਖਾਲੀ (luaL_openselectedlibs) (lua_State *L, ਪੂਰਨ ਲੱਦੋ, ਪੂਰਨ preload);

/* open all libraries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#ਜੇ_ਅੰਤ
