/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lualib_h
#ವ್ಯಾಖ್ಯಾನಿಸು lualib_h

#ಅನ್ತರ್ಭಾವ "lua.h"


/* version suffix for environment variable names */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_VERSUFFIX          "_" LUA_ಆವೃತ್ತಿ_MAJOR "_" LUA_ಆವೃತ್ತಿ_MINOR

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_GLIBK		1
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_ತಳ) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LOADLIBNAME	"package"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_package) (lua_State *L);


#ವ್ಯಾಖ್ಯಾನಿಸು LUA_COLIBNAME	"coroutine"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_coroutine) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_DBLIBNAME	"debug"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_debug) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_IOLIBNAME	"io"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_io) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MATHLIBNAME	"math"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_math) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OSLIBNAME	"os"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_os) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_STRLIBNAME	"string"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_string) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TABLIBNAME	"table"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_ಕೋಷ್ಟಕ) (lua_State *L);

#ವ್ಯಾಖ್ಯಾನಿಸು LUA_UTF8LIBNAME	"utf8"
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API ಪೂರ್ಣಾಂಕ (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API ಶೂನ್ಯ (luaL_openselectedlibs) (lua_State *L, ಪೂರ್ಣಾಂಕ ತುಂಬು, ಪೂರ್ಣಾಂಕ preload);

/* open all libraries */
#ವ್ಯಾಖ್ಯಾನಿಸು luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#ಆದರೆ_ಅಂತ್ಯ
