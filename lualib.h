/*
** $Id: lualib.h $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#अगर_अघोषित lualib_h
#घोषणा lualib_h

#समावेश "lua.h"


/* version suffix for environment variable names */
#घोषणा LUA_VERSUFFIX          "_" LUA_VERSION_MAJOR "_" LUA_VERSION_MINOR

#घोषणा LUA_GLIBK		1
LUAMOD_API पूर्णांक (luaopen_base) (lua_State *L);

#घोषणा LUA_LOADLIBNAME	"package"
#घोषणा LUA_LOADLIBK	(LUA_GLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_package) (lua_State *L);


#घोषणा LUA_COLIBNAME	"coroutine"
#घोषणा LUA_COLIBK	(LUA_LOADLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_coroutine) (lua_State *L);

#घोषणा LUA_DBLIBNAME	"debug"
#घोषणा LUA_DBLIBK	(LUA_COLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_debug) (lua_State *L);

#घोषणा LUA_IOLIBNAME	"io"
#घोषणा LUA_IOLIBK	(LUA_DBLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_io) (lua_State *L);

#घोषणा LUA_MATHLIBNAME	"math"
#घोषणा LUA_MATHLIBK	(LUA_IOLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_math) (lua_State *L);

#घोषणा LUA_OSLIBNAME	"os"
#घोषणा LUA_OSLIBK	(LUA_MATHLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_os) (lua_State *L);

#घोषणा LUA_STRLIBNAME	"string"
#घोषणा LUA_STRLIBK	(LUA_OSLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_string) (lua_State *L);

#घोषणा LUA_TABLIBNAME	"table"
#घोषणा LUA_TABLIBK	(LUA_STRLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_table) (lua_State *L);

#घोषणा LUA_UTF8LIBNAME	"utf8"
#घोषणा LUA_UTF8LIBK	(LUA_TABLIBK << 1)
LUAMOD_API पूर्णांक (luaopen_utf8) (lua_State *L);


/* open selected libraries */
LUALIB_API व्योम (luaL_openselectedlibs) (lua_State *L, पूर्णांक load, पूर्णांक preload);

/* open all libraries */
#घोषणा luaL_openlibs(L)	luaL_openselectedlibs(L, ~0, 0)


#पूर्ण_अगर
