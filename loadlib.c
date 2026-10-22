/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#define loadlib_c
#define LUA_LIB

#include "lprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#만약 !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#만약 !defined(LUA_LSUBSEP)
#define LUA_LSUBSEP		LUA_DIRSEP
#endif


/* prefix for open functions in C libraries */
#define LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#define LUA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
static const char *const CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((void)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
static void lsys_unloadlib (void *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
static void *lsys_load (lua_State *L, const char *path, 정수 seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
static lua_CFunction lsys_sym (lua_State *L, void *lib, const char *sym);




#만약 defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#include <dlfcn.h>


static void lsys_unloadlib (void *lib) {
  dlclose(lib);
}


static void *lsys_load (lua_State *L, const char *path, 정수 seeglb) {
  void *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  만약 (l_unlikely(lib == NULL))
    lua_pushstring(L, dlerror());
  반환 lib;
}


static lua_CFunction lsys_sym (lua_State *L, void *lib, const char *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  만약 (l_unlikely(f == NULL))
    lua_pushstring(L, dlerror());
  반환 f;
}

/* }====================================================== */



#elif defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#include <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#만약 !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
static void setprogdir (lua_State *L) {
  char buff[MAX_PATH + 1];
  char *lb;
  DWORD nsize = sizeof(buff)/sizeof(char);
  DWORD n = GetModuleFileNameA(NULL, buff, nsize);  /* get exec. name */
  만약 (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == NULL)
    luaL_error(L, "unable to get ModuleFileName");
  아니면 {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




static void pusherror (lua_State *L) {
  정수 error = GetLastError();
  char buffer[128];
  만약 (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer)/sizeof(char), NULL))
    lua_pushstring(L, buffer);
  아니면
    lua_pushfstring(L, "system error %d\n", error);
}

static void lsys_unloadlib (void *lib) {
  FreeLibrary((HMODULE)lib);
}


static void *lsys_load (lua_State *L, const char *path, 정수 seeglb) {
  HMODULE lib = LoadLibraryExA(path, NULL, LUA_LLE_FLAGS);
  (void)(seeglb);  /* not used: symbols are 'global' by default */
  만약 (lib == NULL) pusherror(L);
  반환 lib;
}


static lua_CFunction lsys_sym (lua_State *L, void *lib, const char *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  만약 (f == NULL) pusherror(L);
  반환 f;
}

/* }====================================================== */


#아니면				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


static void lsys_unloadlib (void *lib) {
  (void)(lib);  /* not used */
}


static void *lsys_load (lua_State *L, const char *path, 정수 seeglb) {
  (void)(path); (void)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  반환 NULL;
}


static lua_CFunction lsys_sym (lua_State *L, void *lib, const char *sym) {
  (void)(lib); (void)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  반환 NULL;
}

/* }====================================================== */
#endif				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#만약 !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#만약 !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
static 정수 noenv (lua_State *L) {
  정수 b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_pop(L, 1);  /* remove value */
  반환 b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
static void setpath (lua_State *L, const char *fieldname,
                                   const char *envname,
                                   const char *dft) {
  const char *dftmark;
  const char *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  const char *path = getenv(nver);  /* try versioned name */
  만약 (path == NULL)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  만약 (path == NULL || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), NULL, NULL);  /* use default */
  아니면 만약 ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == NULL)
    lua_pushstring(L, path);  /* nothing to change */
  아니면 {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    만약 (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    만약 (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_pop(L, 1);  /* pop versioned variable name ('nver') */
}

/* }================================================================== */


/*
** External strings created by DLLs may need the DLL code to be
** deallocated. This implies that a DLL can only be unloaded after all
** its strings were deallocated. To ensure that, we create a 'library
** string' to represent each DLL, and when this string is deallocated
** it closes its corresponding DLL.
** (The string itself is irrelevant; its userdata is the DLL pointer.)
*/


/*
** return registry.CLIBS[path]
*/
static void *checkclib (lua_State *L, const char *path) {
  void *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_pop(L, 2);  /* pop CLIBS table and 'plib' */
  반환 plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
static void *freelib (void *ud, void *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (void)ptr; (void)osize; (void)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  반환 NULL;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
static void createlibstr (lua_State *L, void *plib) {
  /* common content for all library strings */
  static const char dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, sizeof(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
static void addtoclib (lua_State *L, const char *path, void *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_pop(L, 1);  /* pop CLIBS table */
}


/* error codes for 'lookforfunc' */
#define ERRLIB		1
#define ERRFUNC		2

/*
** Look for a C function named 'sym' in a dynamically loaded library
** 'path'.
** First, check whether the library is already loaded; if not, try
** to load it.
** Then, if 'sym' is '*', return true (as library has been loaded).
** Otherwise, look for symbol 'sym' in the library and push a
** C function with that symbol.
** Return 0 with 'true' or a function in the stack; in case of
** errors, return an error code with an error message in the stack.
*/
static 정수 lookforfunc (lua_State *L, const char *path, const char *sym) {
  void *reg = checkclib(L, path);  /* check loaded C libraries */
  만약 (reg == NULL) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    만약 (reg == NULL) 반환 ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  만약 (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    반환 0;  /* no errors */
  }
  아니면 {
    lua_CFunction f = lsys_sym(L, reg, sym);
    만약 (f == NULL)
      반환 ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    반환 0;  /* no errors */
  }
}


static 정수 ll_loadlib (lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *init = luaL_checkstring(L, 2);
  정수 stat = lookforfunc(L, path, init);
  만약 (l_likely(stat == 0))  /* no errors? */
    반환 1;  /* return the loaded function */
  아니면 {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_insert(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    반환 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


static 정수 readable (const char *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  만약 (f == NULL) 반환 0;  /* open failed */
  fclose(f);
  반환 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
static const char *getnextfilename (char **path, char *end) {
  char *sep;
  char *name = *path;
  만약 (name == end)
    반환 NULL;  /* no more names */
  아니면 만약 (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  만약 (sep == NULL)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  반환 name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
static void pusherrornotfound (lua_State *L, const char *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


static const char *searchpath (lua_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  luaL_Buffer buff;
  char *pathname;  /* path with name inserted */
  char *endpathname;  /* its end */
  const char *filename;
  /* separator is non-empty and appears in 'name'? */
  만약 (*sep != '\0' && strchr(name, *sep) != NULL)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  동안 ((filename = getnextfilename(&pathname, endpathname)) != NULL) {
    만약 (readable(filename))  /* does file exist and is readable? */
      반환 lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  반환 NULL;  /* not found */
}


static 정수 ll_searchpath (lua_State *L) {
  const char *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  만약 (f != NULL) 반환 1;
  아니면 {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_insert(L, -2);
    반환 2;  /* return fail + error message */
  }
}


static const char *findfile (lua_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  만약 (l_unlikely(path == NULL))
    luaL_error(L, "'package.%s' must be a string", pname);
  반환 searchpath(L, name, path, ".", dirsep);
}


static 정수 checkload (lua_State *L, 정수 stat, const char *filename) {
  만약 (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    반환 2;  /* return open function and file name */
  }
  아니면
    반환 luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


static 정수 searcher_Lua (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  만약 (filename == NULL) 반환 1;  /* module not found in this path */
  반환 checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
static 정수 loadfunc (lua_State *L, const char *filename, const char *modname) {
  const char *openfunc;
  const char *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  만약 (mark) {
    정수 stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    만약 (stat != ERRFUNC) 반환 stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  반환 lookforfunc(L, filename, openfunc);
}


static 정수 searcher_C (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  const char *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  만약 (filename == NULL) 반환 1;  /* module not found in this path */
  반환 checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


static 정수 searcher_Croot (lua_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  const char *p = strchr(name, '.');
  정수 stat;
  만약 (p == NULL) 반환 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  만약 (filename == NULL) 반환 1;  /* root not found */
  만약 ((stat = loadfunc(L, filename, name)) != 0) {
    만약 (stat != ERRFUNC)
      반환 checkload(L, 0, filename);  /* real error */
    아니면 {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      반환 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  반환 2;
}


static 정수 searcher_preload (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  만약 (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    반환 1;
  }
  아니면 {
    lua_pushliteral(L, ":preload:");
    반환 2;
  }
}


static void findloader (lua_State *L, const char *name) {
  정수 i;
  luaL_Buffer msg;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  만약 (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &msg);
  luaL_addstring(&msg, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  위해 (i = 1; ; i++) {
    만약 (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_pop(L, 1);  /* remove nil */
      luaL_buffsub(&msg, 2);  /* remove last prefix */
      luaL_pushresult(&msg);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    만약 (lua_isfunction(L, -2))  /* did it find a loader? */
      반환;  /* module loader found */
    아니면 만약 (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_pop(L, 1);  /* remove extra return */
      luaL_addvalue(&msg);  /* concatenate error message */
      luaL_addstring(&msg, "\n\t");  /* prefix for next message */
    }
    아니면  /* no error message */
      lua_pop(L, 2);  /* remove both returns */
  }
}


static 정수 ll_require (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  만약 (lua_toboolean(L, -1))  /* is it there? */
    반환 1;  /* package is already loaded */
  /* else must load package */
  lua_pop(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  만약 (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  아니면
    lua_pop(L, 1);  /* pop nil */
  만약 (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  반환 2;  /* return module result and loader data */
}

/* }====================================================== */




static const luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", NULL},
  {"cpath", NULL},
  {"path", NULL},
  {"searchers", NULL},
  {"loaded", NULL},
  {NULL, NULL}
};


static const luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {NULL, NULL}
};


static void createsearcherstable (lua_State *L) {
  static const lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    NULL
  };
  정수 i;
  /* create 'searchers' table */
  lua_createtable(L, sizeof(searchers)/sizeof(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  위해 (i=0; searchers[i] != NULL; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API 정수 luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_pop(L, 1);  /* will not use it now */
  luaL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LUA_PATH_VAR, LUA_PATH_DEFAULT);
  setpath(L, "cpath", LUA_CPATH_VAR, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_PATH_SEP "\n" LUA_PATH_MARK "\n"
                     LUA_EXEC_DIR "\n" LUA_IGMARK "\n");
  lua_setfield(L, -2, "config");
  /* set field 'loaded' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_setfield(L, -2, "loaded");
  /* set field 'preload' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  lua_setfield(L, -2, "preload");
  lua_pushglobaltable(L);
  lua_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  luaL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  lua_pop(L, 1);  /* pop global table */
  반환 1;  /* return 'package' table */
}

