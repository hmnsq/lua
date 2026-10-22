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

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#ಯದಿ !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#ಯದಿ !defined(LUA_LSUBSEP)
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
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((ಶೂನ್ಯ)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *lsys_load (lua_State *L, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಧ್ರುವ ವರ್ಣ *sym);




#ಯದಿ defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#ಅನ್ತರ್ಭಾವ <dlfcn.h>


ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib) {
  dlclose(lib);
}


ಸ್ಥಿರ ಶೂನ್ಯ *lsys_load (lua_State *L, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ seeglb) {
  ಶೂನ್ಯ *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  ಯದಿ (l_unlikely(lib == ರಿಕ್ತ))
    lua_pushstring(L, dlerror());
  ಪ್ರತ್ಯಾಗಮ lib;
}


ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಧ್ರುವ ವರ್ಣ *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  ಯದಿ (l_unlikely(f == ರಿಕ್ತ))
    lua_pushstring(L, dlerror());
  ಪ್ರತ್ಯಾಗಮ f;
}

/* }====================================================== */



#ನೋಚೇದ್ಯದಿ defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#ಅನ್ತರ್ಭಾವ <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#ಯದಿ !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setprogdir (lua_State *L) {
  ವರ್ಣ buff[MAX_PATH + 1];
  ವರ್ಣ *lb;
  DWORD nsize = ಪರಿಮಾಣ(buff)/ಪರಿಮಾಣ(ವರ್ಣ);
  DWORD n = GetModuleFileNameA(ರಿಕ್ತ, buff, nsize);  /* get exec. name */
  ಯದಿ (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == ರಿಕ್ತ)
    luaL_error(L, "unable to get ModuleFileName");
  ಅನ್ಯಥಾ {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




ಸ್ಥಿರ ಶೂನ್ಯ pusherror (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ error = GetLastError();
  ವರ್ಣ buffer[128];
  ಯದಿ (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      ರಿಕ್ತ, error, 0, buffer, ಪರಿಮಾಣ(buffer)/ಪರಿಮಾಣ(ವರ್ಣ), ರಿಕ್ತ))
    lua_pushstring(L, buffer);
  ಅನ್ಯಥಾ
    lua_pushfstring(L, "system error %d\n", error);
}

ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib) {
  FreeLibrary((HMODULE)lib);
}


ಸ್ಥಿರ ಶೂನ್ಯ *lsys_load (lua_State *L, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ seeglb) {
  HMODULE lib = LoadLibraryExA(path, ರಿಕ್ತ, LUA_LLE_FLAGS);
  (ಶೂನ್ಯ)(seeglb);  /* not used: symbols are 'global' by default */
  ಯದಿ (lib == ರಿಕ್ತ) pusherror(L);
  ಪ್ರತ್ಯಾಗಮ lib;
}


ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಧ್ರುವ ವರ್ಣ *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  ಯದಿ (f == ರಿಕ್ತ) pusherror(L);
  ಪ್ರತ್ಯಾಗಮ f;
}

/* }====================================================== */


#ಅನ್ಯಥಾ				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib) {
  (ಶೂನ್ಯ)(lib);  /* not used */
}


ಸ್ಥಿರ ಶೂನ್ಯ *lsys_load (lua_State *L, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ seeglb) {
  (ಶೂನ್ಯ)(path); (ಶೂನ್ಯ)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಧ್ರುವ ವರ್ಣ *sym) {
  (ಶೂನ್ಯ)(lib); (ಶೂನ್ಯ)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
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
#ಯದಿ !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#ಯದಿ !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ noenv (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_ತೆಗೆ(L, 1);  /* remove value */
  ಪ್ರತ್ಯಾಗಮ b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ setpath (lua_State *L, ಧ್ರುವ ವರ್ಣ *fieldname,
                                   ಧ್ರುವ ವರ್ಣ *envname,
                                   ಧ್ರುವ ವರ್ಣ *dft) {
  ಧ್ರುವ ವರ್ಣ *dftmark;
  ಧ್ರುವ ವರ್ಣ *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ಧ್ರುವ ವರ್ಣ *path = getenv(nver);  /* try versioned name */
  ಯದಿ (path == ರಿಕ್ತ)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  ಯದಿ (path == ರಿಕ್ತ || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), ರಿಕ್ತ, ರಿಕ್ತ);  /* use default */
  ಅನ್ಯಥಾ ಯದಿ ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == ರಿಕ್ತ)
    lua_pushstring(L, path);  /* nothing to change */
  ಅನ್ಯಥಾ {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    ಯದಿ (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    ಯದಿ (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_ತೆಗೆ(L, 1);  /* pop versioned variable name ('nver') */
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
ಸ್ಥಿರ ಶೂನ್ಯ *checkclib (lua_State *L, ಧ್ರುವ ವರ್ಣ *path) {
  ಶೂನ್ಯ *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_ತೆಗೆ(L, 2);  /* pop CLIBS table and 'plib' */
  ಪ್ರತ್ಯಾಗಮ plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *freelib (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (ಶೂನ್ಯ)ptr; (ಶೂನ್ಯ)osize; (ಶೂನ್ಯ)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
ಸ್ಥಿರ ಶೂನ್ಯ createlibstr (lua_State *L, ಶೂನ್ಯ *plib) {
  /* common content for all library strings */
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, ಪರಿಮಾಣ(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
ಸ್ಥಿರ ಶೂನ್ಯ addtoclib (lua_State *L, ಧ್ರುವ ವರ್ಣ *path, ಶೂನ್ಯ *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_ತೆಗೆ(L, 1);  /* pop CLIBS table */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ lookforfunc (lua_State *L, ಧ್ರುವ ವರ್ಣ *path, ಧ್ರುವ ವರ್ಣ *sym) {
  ಶೂನ್ಯ *reg = checkclib(L, path);  /* check loaded C libraries */
  ಯದಿ (reg == ರಿಕ್ತ) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    ಯದಿ (reg == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  ಯದಿ (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    ಪ್ರತ್ಯಾಗಮ 0;  /* no errors */
  }
  ಅನ್ಯಥಾ {
    lua_CFunction f = lsys_sym(L, reg, sym);
    ಯದಿ (f == ರಿಕ್ತ)
      ಪ್ರತ್ಯಾಗಮ ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    ಪ್ರತ್ಯಾಗಮ 0;  /* no errors */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ll_loadlib (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *path = luaL_checkstring(L, 1);
  ಧ್ರುವ ವರ್ಣ *init = luaL_checkstring(L, 2);
  ಪೂರ್ಣಾಙ್ಕ stat = lookforfunc(L, path, init);
  ಯದಿ (l_likely(stat == 0))  /* no errors? */
    ಪ್ರತ್ಯಾಗಮ 1;  /* return the loaded function */
  ಅನ್ಯಥಾ {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_ಸೇರಿಸು(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    ಪ್ರತ್ಯಾಗಮ 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ readable (ಧ್ರುವ ವರ್ಣ *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  ಯದಿ (f == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 0;  /* open failed */
  fclose(f);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getnextfilename (ವರ್ಣ **path, ವರ್ಣ *end) {
  ವರ್ಣ *sep;
  ವರ್ಣ *name = *path;
  ಯದಿ (name == end)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* no more names */
  ಅನ್ಯಥಾ ಯದಿ (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  ಯದಿ (sep == ರಿಕ್ತ)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  ಪ್ರತ್ಯಾಗಮ name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
ಸ್ಥಿರ ಶೂನ್ಯ pusherrornotfound (lua_State *L, ಧ್ರುವ ವರ್ಣ *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *searchpath (lua_State *L, ಧ್ರುವ ವರ್ಣ *name,
                                             ಧ್ರುವ ವರ್ಣ *path,
                                             ಧ್ರುವ ವರ್ಣ *sep,
                                             ಧ್ರುವ ವರ್ಣ *dirsep) {
  luaL_Buffer buff;
  ವರ್ಣ *pathname;  /* path with name inserted */
  ವರ್ಣ *endpathname;  /* its end */
  ಧ್ರುವ ವರ್ಣ *filename;
  /* separator is non-empty and appears in 'name'? */
  ಯದಿ (*sep != '\0' && strchr(name, *sep) != ರಿಕ್ತ)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  ಯಾವತ್ ((filename = getnextfilename(&pathname, endpathname)) != ರಿಕ್ತ) {
    ಯದಿ (readable(filename))  /* does file exist and is readable? */
      ಪ್ರತ್ಯಾಗಮ lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* not found */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ll_searchpath (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  ಯದಿ (f != ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 1;
  ಅನ್ಯಥಾ {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_ಸೇರಿಸು(L, -2);
    ಪ್ರತ್ಯಾಗಮ 2;  /* return fail + error message */
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *findfile (lua_State *L, ಧ್ರುವ ವರ್ಣ *name,
                                           ಧ್ರುವ ವರ್ಣ *pname,
                                           ಧ್ರುವ ವರ್ಣ *dirsep) {
  ಧ್ರುವ ವರ್ಣ *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  ಯದಿ (l_unlikely(path == ರಿಕ್ತ))
    luaL_error(L, "'package.%s' must be a string", pname);
  ಪ್ರತ್ಯಾಗಮ searchpath(L, name, path, ".", dirsep);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkload (lua_State *L, ಪೂರ್ಣಾಙ್ಕ stat, ಧ್ರುವ ವರ್ಣ *filename) {
  ಯದಿ (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    ಪ್ರತ್ಯಾಗಮ 2;  /* return open function and file name */
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ searcher_Lua (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *filename;
  ಧ್ರುವ ವರ್ಣ *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  ಯದಿ (filename == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 1;  /* module not found in this path */
  ಪ್ರತ್ಯಾಗಮ checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loadfunc (lua_State *L, ಧ್ರುವ ವರ್ಣ *filename, ಧ್ರುವ ವರ್ಣ *modname) {
  ಧ್ರುವ ವರ್ಣ *openfunc;
  ಧ್ರುವ ವರ್ಣ *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  ಯದಿ (mark) {
    ಪೂರ್ಣಾಙ್ಕ stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    ಯದಿ (stat != ERRFUNC) ಪ್ರತ್ಯಾಗಮ stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  ಪ್ರತ್ಯಾಗಮ lookforfunc(L, filename, openfunc);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ searcher_C (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *name = luaL_checkstring(L, 1);
  ಧ್ರುವ ವರ್ಣ *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  ಯದಿ (filename == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 1;  /* module not found in this path */
  ಪ್ರತ್ಯಾಗಮ checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ searcher_Croot (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *filename;
  ಧ್ರುವ ವರ್ಣ *name = luaL_checkstring(L, 1);
  ಧ್ರುವ ವರ್ಣ *p = strchr(name, '.');
  ಪೂರ್ಣಾಙ್ಕ stat;
  ಯದಿ (p == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  ಯದಿ (filename == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 1;  /* root not found */
  ಯದಿ ((stat = loadfunc(L, filename, name)) != 0) {
    ಯದಿ (stat != ERRFUNC)
      ಪ್ರತ್ಯಾಗಮ checkload(L, 0, filename);  /* real error */
    ಅನ್ಯಥಾ {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  ಪ್ರತ್ಯಾಗಮ 2;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ searcher_preload (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  ಯದಿ (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ {
    lua_pushliteral(L, ":preload:");
    ಪ್ರತ್ಯಾಗಮ 2;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ findloader (lua_State *L, ಧ್ರುವ ವರ್ಣ *name) {
  ಪೂರ್ಣಾಙ್ಕ i;
  luaL_Buffer ಸಂದೇಶ;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  ಯದಿ (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &ಸಂದೇಶ);
  luaL_addstring(&ಸಂದೇಶ, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  ಪ್ರತಿ (i = 1; ; i++) {
    ಯದಿ (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_ತೆಗೆ(L, 1);  /* remove nil */
      luaL_buffsub(&ಸಂದೇಶ, 2);  /* remove last prefix */
      luaL_pushresult(&ಸಂದೇಶ);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    ಯದಿ (lua_isfunction(L, -2))  /* did it find a loader? */
      ಪ್ರತ್ಯಾಗಮ;  /* module loader found */
    ಅನ್ಯಥಾ ಯದಿ (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_ತೆಗೆ(L, 1);  /* remove extra return */
      luaL_addvalue(&ಸಂದೇಶ);  /* concatenate error message */
      luaL_addstring(&ಸಂದೇಶ, "\n\t");  /* prefix for next message */
    }
    ಅನ್ಯಥಾ  /* no error message */
      lua_ತೆಗೆ(L, 2);  /* remove both returns */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ll_require (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  ಯದಿ (lua_toboolean(L, -1))  /* is it there? */
    ಪ್ರತ್ಯಾಗಮ 1;  /* package is already loaded */
  /* else must load package */
  lua_ತೆಗೆ(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  ಯದಿ (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  ಅನ್ಯಥಾ
    lua_ತೆಗೆ(L, 1);  /* pop nil */
  ಯದಿ (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  ಪ್ರತ್ಯಾಗಮ 2;  /* return module result and loader data */
}

/* }====================================================== */




ಸ್ಥಿರ ಧ್ರುವ luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", ರಿಕ್ತ},
  {"cpath", ರಿಕ್ತ},
  {"path", ರಿಕ್ತ},
  {"searchers", ರಿಕ್ತ},
  {"loaded", ರಿಕ್ತ},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


ಸ್ಥಿರ ಶೂನ್ಯ createsearcherstable (lua_State *L) {
  ಸ್ಥಿರ ಧ್ರುವ lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    ರಿಕ್ತ
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  /* create 'searchers' table */
  lua_createtable(L, ಪರಿಮಾಣ(searchers)/ಪರಿಮಾಣ(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  ಪ್ರತಿ (i=0; searchers[i] != ರಿಕ್ತ; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_ತೆಗೆ(L, 1);  /* will not use it now */
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
  lua_ತೆಗೆ(L, 1);  /* pop global table */
  ಪ್ರತ್ಯಾಗಮ 1;  /* return 'package' table */
}

