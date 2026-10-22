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

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#എങ്കിൽ !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#എങ്കിൽ !defined(LUA_LSUBSEP)
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
സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((ശൂന്യം)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
സ്ഥിതം ശൂന്യം lsys_unloadlib (ശൂന്യം *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
സ്ഥിതം ശൂന്യം *lsys_load (lua_State *L, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
സ്ഥിതം lua_CFunction lsys_sym (lua_State *L, ശൂന്യം *lib, സ്ഥിരം അക്ഷരം *sym);




#എങ്കിൽ defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#അന്തര്ഭാവ <dlfcn.h>


സ്ഥിതം ശൂന്യം lsys_unloadlib (ശൂന്യം *lib) {
  dlclose(lib);
}


സ്ഥിതം ശൂന്യം *lsys_load (lua_State *L, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ seeglb) {
  ശൂന്യം *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  എങ്കിൽ (l_unlikely(lib == ശൂന്യ))
    lua_pushstring(L, dlerror());
  തിരികെ lib;
}


സ്ഥിതം lua_CFunction lsys_sym (lua_State *L, ശൂന്യം *lib, സ്ഥിരം അക്ഷരം *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  എങ്കിൽ (l_unlikely(f == ശൂന്യ))
    lua_pushstring(L, dlerror());
  തിരികെ f;
}

/* }====================================================== */



#അല്ലെങ്കിലെങ്കിൽ defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#അന്തര്ഭാവ <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#എങ്കിൽ !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
സ്ഥിതം ശൂന്യം setprogdir (lua_State *L) {
  അക്ഷരം buff[MAX_PATH + 1];
  അക്ഷരം *lb;
  DWORD nsize = വലുപ്പം(buff)/വലുപ്പം(അക്ഷരം);
  DWORD n = GetModuleFileNameA(ശൂന്യ, buff, nsize);  /* get exec. name */
  എങ്കിൽ (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == ശൂന്യ)
    luaL_error(L, "unable to get ModuleFileName");
  അല്ലെങ്കിൽ {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




സ്ഥിതം ശൂന്യം pusherror (lua_State *L) {
  പൂർണ്ണസംഖ്യ error = GetLastError();
  അക്ഷരം buffer[128];
  എങ്കിൽ (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      ശൂന്യ, error, 0, buffer, വലുപ്പം(buffer)/വലുപ്പം(അക്ഷരം), ശൂന്യ))
    lua_pushstring(L, buffer);
  അല്ലെങ്കിൽ
    lua_pushfstring(L, "system error %d\n", error);
}

സ്ഥിതം ശൂന്യം lsys_unloadlib (ശൂന്യം *lib) {
  FreeLibrary((HMODULE)lib);
}


സ്ഥിതം ശൂന്യം *lsys_load (lua_State *L, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ seeglb) {
  HMODULE lib = LoadLibraryExA(path, ശൂന്യ, LUA_LLE_FLAGS);
  (ശൂന്യം)(seeglb);  /* not used: symbols are 'global' by default */
  എങ്കിൽ (lib == ശൂന്യ) pusherror(L);
  തിരികെ lib;
}


സ്ഥിതം lua_CFunction lsys_sym (lua_State *L, ശൂന്യം *lib, സ്ഥിരം അക്ഷരം *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  എങ്കിൽ (f == ശൂന്യ) pusherror(L);
  തിരികെ f;
}

/* }====================================================== */


#അല്ലെങ്കിൽ				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


സ്ഥിതം ശൂന്യം lsys_unloadlib (ശൂന്യം *lib) {
  (ശൂന്യം)(lib);  /* not used */
}


സ്ഥിതം ശൂന്യം *lsys_load (lua_State *L, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ seeglb) {
  (ശൂന്യം)(path); (ശൂന്യം)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  തിരികെ ശൂന്യ;
}


സ്ഥിതം lua_CFunction lsys_sym (lua_State *L, ശൂന്യം *lib, സ്ഥിരം അക്ഷരം *sym) {
  (ശൂന്യം)(lib); (ശൂന്യം)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  തിരികെ ശൂന്യ;
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
#എങ്കിൽ !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#എങ്കിൽ !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ noenv (lua_State *L) {
  പൂർണ്ണസംഖ്യ b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_എടുക്കുക(L, 1);  /* remove value */
  തിരികെ b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
സ്ഥിതം ശൂന്യം setpath (lua_State *L, സ്ഥിരം അക്ഷരം *fieldname,
                                   സ്ഥിരം അക്ഷരം *envname,
                                   സ്ഥിരം അക്ഷരം *dft) {
  സ്ഥിരം അക്ഷരം *dftmark;
  സ്ഥിരം അക്ഷരം *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  സ്ഥിരം അക്ഷരം *path = getenv(nver);  /* try versioned name */
  എങ്കിൽ (path == ശൂന്യ)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  എങ്കിൽ (path == ശൂന്യ || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), ശൂന്യ, ശൂന്യ);  /* use default */
  അല്ലെങ്കിൽ എങ്കിൽ ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == ശൂന്യ)
    lua_pushstring(L, path);  /* nothing to change */
  അല്ലെങ്കിൽ {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    എങ്കിൽ (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    എങ്കിൽ (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_എടുക്കുക(L, 1);  /* pop versioned variable name ('nver') */
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
സ്ഥിതം ശൂന്യം *checkclib (lua_State *L, സ്ഥിരം അക്ഷരം *path) {
  ശൂന്യം *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_എടുക്കുക(L, 2);  /* pop CLIBS table and 'plib' */
  തിരികെ plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
സ്ഥിതം ശൂന്യം *freelib (ശൂന്യം *ud, ശൂന്യം *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (ശൂന്യം)ptr; (ശൂന്യം)osize; (ശൂന്യം)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  തിരികെ ശൂന്യ;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
സ്ഥിതം ശൂന്യം createlibstr (lua_State *L, ശൂന്യം *plib) {
  /* common content for all library strings */
  സ്ഥിതം സ്ഥിരം അക്ഷരം dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, വലുപ്പം(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
സ്ഥിതം ശൂന്യം addtoclib (lua_State *L, സ്ഥിരം അക്ഷരം *path, ശൂന്യം *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_എടുക്കുക(L, 1);  /* pop CLIBS table */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ lookforfunc (lua_State *L, സ്ഥിരം അക്ഷരം *path, സ്ഥിരം അക്ഷരം *sym) {
  ശൂന്യം *reg = checkclib(L, path);  /* check loaded C libraries */
  എങ്കിൽ (reg == ശൂന്യ) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    എങ്കിൽ (reg == ശൂന്യ) തിരികെ ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  എങ്കിൽ (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    തിരികെ 0;  /* no errors */
  }
  അല്ലെങ്കിൽ {
    lua_CFunction f = lsys_sym(L, reg, sym);
    എങ്കിൽ (f == ശൂന്യ)
      തിരികെ ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    തിരികെ 0;  /* no errors */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ ll_loadlib (lua_State *L) {
  സ്ഥിരം അക്ഷരം *path = luaL_checkstring(L, 1);
  സ്ഥിരം അക്ഷരം *init = luaL_checkstring(L, 2);
  പൂർണ്ണസംഖ്യ stat = lookforfunc(L, path, init);
  എങ്കിൽ (l_likely(stat == 0))  /* no errors? */
    തിരികെ 1;  /* return the loaded function */
  അല്ലെങ്കിൽ {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_തിരുകുക(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    തിരികെ 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


സ്ഥിതം പൂർണ്ണസംഖ്യ readable (സ്ഥിരം അക്ഷരം *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  എങ്കിൽ (f == ശൂന്യ) തിരികെ 0;  /* open failed */
  fclose(f);
  തിരികെ 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *getnextfilename (അക്ഷരം **path, അക്ഷരം *end) {
  അക്ഷരം *sep;
  അക്ഷരം *name = *path;
  എങ്കിൽ (name == end)
    തിരികെ ശൂന്യ;  /* no more names */
  അല്ലെങ്കിൽ എങ്കിൽ (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  എങ്കിൽ (sep == ശൂന്യ)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  തിരികെ name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
സ്ഥിതം ശൂന്യം pusherrornotfound (lua_State *L, സ്ഥിരം അക്ഷരം *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *searchpath (lua_State *L, സ്ഥിരം അക്ഷരം *name,
                                             സ്ഥിരം അക്ഷരം *path,
                                             സ്ഥിരം അക്ഷരം *sep,
                                             സ്ഥിരം അക്ഷരം *dirsep) {
  luaL_Buffer buff;
  അക്ഷരം *pathname;  /* path with name inserted */
  അക്ഷരം *endpathname;  /* its end */
  സ്ഥിരം അക്ഷരം *filename;
  /* separator is non-empty and appears in 'name'? */
  എങ്കിൽ (*sep != '\0' && strchr(name, *sep) != ശൂന്യ)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  ആകുമ്പോൾ ((filename = getnextfilename(&pathname, endpathname)) != ശൂന്യ) {
    എങ്കിൽ (readable(filename))  /* does file exist and is readable? */
      തിരികെ lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  തിരികെ ശൂന്യ;  /* not found */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ ll_searchpath (lua_State *L) {
  സ്ഥിരം അക്ഷരം *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  എങ്കിൽ (f != ശൂന്യ) തിരികെ 1;
  അല്ലെങ്കിൽ {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_തിരുകുക(L, -2);
    തിരികെ 2;  /* return fail + error message */
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *findfile (lua_State *L, സ്ഥിരം അക്ഷരം *name,
                                           സ്ഥിരം അക്ഷരം *pname,
                                           സ്ഥിരം അക്ഷരം *dirsep) {
  സ്ഥിരം അക്ഷരം *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  എങ്കിൽ (l_unlikely(path == ശൂന്യ))
    luaL_error(L, "'package.%s' must be a string", pname);
  തിരികെ searchpath(L, name, path, ".", dirsep);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ checkload (lua_State *L, പൂർണ്ണസംഖ്യ stat, സ്ഥിരം അക്ഷരം *filename) {
  എങ്കിൽ (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    തിരികെ 2;  /* return open function and file name */
  }
  അല്ലെങ്കിൽ
    തിരികെ luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ searcher_Lua (lua_State *L) {
  സ്ഥിരം അക്ഷരം *filename;
  സ്ഥിരം അക്ഷരം *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  എങ്കിൽ (filename == ശൂന്യ) തിരികെ 1;  /* module not found in this path */
  തിരികെ checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ loadfunc (lua_State *L, സ്ഥിരം അക്ഷരം *filename, സ്ഥിരം അക്ഷരം *modname) {
  സ്ഥിരം അക്ഷരം *openfunc;
  സ്ഥിരം അക്ഷരം *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  എങ്കിൽ (mark) {
    പൂർണ്ണസംഖ്യ stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    എങ്കിൽ (stat != ERRFUNC) തിരികെ stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  തിരികെ lookforfunc(L, filename, openfunc);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ searcher_C (lua_State *L) {
  സ്ഥിരം അക്ഷരം *name = luaL_checkstring(L, 1);
  സ്ഥിരം അക്ഷരം *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  എങ്കിൽ (filename == ശൂന്യ) തിരികെ 1;  /* module not found in this path */
  തിരികെ checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ searcher_Croot (lua_State *L) {
  സ്ഥിരം അക്ഷരം *filename;
  സ്ഥിരം അക്ഷരം *name = luaL_checkstring(L, 1);
  സ്ഥിരം അക്ഷരം *p = strchr(name, '.');
  പൂർണ്ണസംഖ്യ stat;
  എങ്കിൽ (p == ശൂന്യ) തിരികെ 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  എങ്കിൽ (filename == ശൂന്യ) തിരികെ 1;  /* root not found */
  എങ്കിൽ ((stat = loadfunc(L, filename, name)) != 0) {
    എങ്കിൽ (stat != ERRFUNC)
      തിരികെ checkload(L, 0, filename);  /* real error */
    അല്ലെങ്കിൽ {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      തിരികെ 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  തിരികെ 2;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ searcher_preload (lua_State *L) {
  സ്ഥിരം അക്ഷരം *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  എങ്കിൽ (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ {
    lua_pushliteral(L, ":preload:");
    തിരികെ 2;
  }
}


സ്ഥിതം ശൂന്യം findloader (lua_State *L, സ്ഥിരം അക്ഷരം *name) {
  പൂർണ്ണസംഖ്യ i;
  luaL_Buffer സന്ദേശം;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  എങ്കിൽ (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &സന്ദേശം);
  luaL_addstring(&സന്ദേശം, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  വേണ്ടി (i = 1; ; i++) {
    എങ്കിൽ (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_എടുക്കുക(L, 1);  /* remove nil */
      luaL_buffsub(&സന്ദേശം, 2);  /* remove last prefix */
      luaL_pushresult(&സന്ദേശം);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    എങ്കിൽ (lua_isfunction(L, -2))  /* did it find a loader? */
      തിരികെ;  /* module loader found */
    അല്ലെങ്കിൽ എങ്കിൽ (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_എടുക്കുക(L, 1);  /* remove extra return */
      luaL_addvalue(&സന്ദേശം);  /* concatenate error message */
      luaL_addstring(&സന്ദേശം, "\n\t");  /* prefix for next message */
    }
    അല്ലെങ്കിൽ  /* no error message */
      lua_എടുക്കുക(L, 2);  /* remove both returns */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ ll_require (lua_State *L) {
  സ്ഥിരം അക്ഷരം *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  എങ്കിൽ (lua_toboolean(L, -1))  /* is it there? */
    തിരികെ 1;  /* package is already loaded */
  /* else must load package */
  lua_എടുക്കുക(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  എങ്കിൽ (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  അല്ലെങ്കിൽ
    lua_എടുക്കുക(L, 1);  /* pop nil */
  എങ്കിൽ (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  തിരികെ 2;  /* return module result and loader data */
}

/* }====================================================== */




സ്ഥിതം സ്ഥിരം luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", ശൂന്യ},
  {"cpath", ശൂന്യ},
  {"path", ശൂന്യ},
  {"searchers", ശൂന്യ},
  {"loaded", ശൂന്യ},
  {ശൂന്യ, ശൂന്യ}
};


സ്ഥിതം സ്ഥിരം luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {ശൂന്യ, ശൂന്യ}
};


സ്ഥിതം ശൂന്യം createsearcherstable (lua_State *L) {
  സ്ഥിതം സ്ഥിരം lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    ശൂന്യ
  };
  പൂർണ്ണസംഖ്യ i;
  /* create 'searchers' table */
  lua_createtable(L, വലുപ്പം(searchers)/വലുപ്പം(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  വേണ്ടി (i=0; searchers[i] != ശൂന്യ; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_എടുക്കുക(L, 1);  /* will not use it now */
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
  lua_എടുക്കുക(L, 1);  /* pop global table */
  തിരികെ 1;  /* return 'package' table */
}

