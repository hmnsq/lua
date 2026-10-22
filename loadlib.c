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

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#सचे !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#सचे !defined(LUA_LSUBSEP)
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
स्थिर ध्रुव वर्ण *ध्रुव CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((शून्य)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
स्थिर शून्य lsys_unloadlib (शून्य *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
स्थिर शून्य *lsys_load (lua_State *L, ध्रुव वर्ण *path, पूर्णाङ्क seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, ध्रुव वर्ण *sym);




#सचे defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#अन्तर्भाव <dlfcn.h>


स्थिर शून्य lsys_unloadlib (शून्य *lib) {
  dlclose(lib);
}


स्थिर शून्य *lsys_load (lua_State *L, ध्रुव वर्ण *path, पूर्णाङ्क seeglb) {
  शून्य *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  सचे (l_unlikely(lib == रिक्त))
    lua_pushstring(L, dlerror());
  पटिनिवत्त lib;
}


स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, ध्रुव वर्ण *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  सचे (l_unlikely(f == रिक्त))
    lua_pushstring(L, dlerror());
  पटिनिवत्त f;
}

/* }====================================================== */



#नोचेद्यदि defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#अन्तर्भाव <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#सचे !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
स्थिर शून्य setprogdir (lua_State *L) {
  वर्ण buff[MAX_PATH + 1];
  वर्ण *lb;
  DWORD nsize = परिमाण(buff)/परिमाण(वर्ण);
  DWORD n = GetModuleFileNameA(रिक्त, buff, nsize);  /* get exec. name */
  सचे (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == रिक्त)
    luaL_error(L, "unable to get ModuleFileName");
  अञ्ञथा {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




स्थिर शून्य pusherror (lua_State *L) {
  पूर्णाङ्क error = GetLastError();
  वर्ण buffer[128];
  सचे (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      रिक्त, error, 0, buffer, परिमाण(buffer)/परिमाण(वर्ण), रिक्त))
    lua_pushstring(L, buffer);
  अञ्ञथा
    lua_pushfstring(L, "system error %d\n", error);
}

स्थिर शून्य lsys_unloadlib (शून्य *lib) {
  FreeLibrary((HMODULE)lib);
}


स्थिर शून्य *lsys_load (lua_State *L, ध्रुव वर्ण *path, पूर्णाङ्क seeglb) {
  HMODULE lib = LoadLibraryExA(path, रिक्त, LUA_LLE_FLAGS);
  (शून्य)(seeglb);  /* not used: symbols are 'global' by default */
  सचे (lib == रिक्त) pusherror(L);
  पटिनिवत्त lib;
}


स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, ध्रुव वर्ण *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  सचे (f == रिक्त) pusherror(L);
  पटिनिवत्त f;
}

/* }====================================================== */


#अञ्ञथा				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


स्थिर शून्य lsys_unloadlib (शून्य *lib) {
  (शून्य)(lib);  /* not used */
}


स्थिर शून्य *lsys_load (lua_State *L, ध्रुव वर्ण *path, पूर्णाङ्क seeglb) {
  (शून्य)(path); (शून्य)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  पटिनिवत्त रिक्त;
}


स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, ध्रुव वर्ण *sym) {
  (शून्य)(lib); (शून्य)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  पटिनिवत्त रिक्त;
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
#सचे !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#सचे !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
स्थिर पूर्णाङ्क noenv (lua_State *L) {
  पूर्णाङ्क b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_निष्कासय(L, 1);  /* remove value */
  पटिनिवत्त b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
स्थिर शून्य setpath (lua_State *L, ध्रुव वर्ण *fieldname,
                                   ध्रुव वर्ण *envname,
                                   ध्रुव वर्ण *dft) {
  ध्रुव वर्ण *dftmark;
  ध्रुव वर्ण *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ध्रुव वर्ण *path = getenv(nver);  /* try versioned name */
  सचे (path == रिक्त)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  सचे (path == रिक्त || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), रिक्त, रिक्त);  /* use default */
  अञ्ञथा सचे ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == रिक्त)
    lua_pushstring(L, path);  /* nothing to change */
  अञ्ञथा {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    सचे (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    सचे (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_निष्कासय(L, 1);  /* pop versioned variable name ('nver') */
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
स्थिर शून्य *checkclib (lua_State *L, ध्रुव वर्ण *path) {
  शून्य *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_निष्कासय(L, 2);  /* pop CLIBS table and 'plib' */
  पटिनिवत्त plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
स्थिर शून्य *freelib (शून्य *ud, शून्य *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (शून्य)ptr; (शून्य)osize; (शून्य)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  पटिनिवत्त रिक्त;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
स्थिर शून्य createlibstr (lua_State *L, शून्य *plib) {
  /* common content for all library strings */
  स्थिर ध्रुव वर्ण dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, परिमाण(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
स्थिर शून्य addtoclib (lua_State *L, ध्रुव वर्ण *path, शून्य *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_निष्कासय(L, 1);  /* pop CLIBS table */
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
स्थिर पूर्णाङ्क lookforfunc (lua_State *L, ध्रुव वर्ण *path, ध्रुव वर्ण *sym) {
  शून्य *reg = checkclib(L, path);  /* check loaded C libraries */
  सचे (reg == रिक्त) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    सचे (reg == रिक्त) पटिनिवत्त ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  सचे (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    पटिनिवत्त 0;  /* no errors */
  }
  अञ्ञथा {
    lua_CFunction f = lsys_sym(L, reg, sym);
    सचे (f == रिक्त)
      पटिनिवत्त ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    पटिनिवत्त 0;  /* no errors */
  }
}


स्थिर पूर्णाङ्क ll_loadlib (lua_State *L) {
  ध्रुव वर्ण *path = luaL_checkstring(L, 1);
  ध्रुव वर्ण *init = luaL_checkstring(L, 2);
  पूर्णाङ्क stat = lookforfunc(L, path, init);
  सचे (l_likely(stat == 0))  /* no errors? */
    पटिनिवत्त 1;  /* return the loaded function */
  अञ्ञथा {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_निवेशय(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    पटिनिवत्त 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


स्थिर पूर्णाङ्क readable (ध्रुव वर्ण *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  सचे (f == रिक्त) पटिनिवत्त 0;  /* open failed */
  fclose(f);
  पटिनिवत्त 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
स्थिर ध्रुव वर्ण *getnextfilename (वर्ण **path, वर्ण *end) {
  वर्ण *sep;
  वर्ण *name = *path;
  सचे (name == end)
    पटिनिवत्त रिक्त;  /* no more names */
  अञ्ञथा सचे (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  सचे (sep == रिक्त)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  पटिनिवत्त name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
स्थिर शून्य pusherrornotfound (lua_State *L, ध्रुव वर्ण *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


स्थिर ध्रुव वर्ण *searchpath (lua_State *L, ध्रुव वर्ण *name,
                                             ध्रुव वर्ण *path,
                                             ध्रुव वर्ण *sep,
                                             ध्रुव वर्ण *dirsep) {
  luaL_Buffer buff;
  वर्ण *pathname;  /* path with name inserted */
  वर्ण *endpathname;  /* its end */
  ध्रुव वर्ण *filename;
  /* separator is non-empty and appears in 'name'? */
  सचे (*sep != '\0' && strchr(name, *sep) != रिक्त)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  याव ((filename = getnextfilename(&pathname, endpathname)) != रिक्त) {
    सचे (readable(filename))  /* does file exist and is readable? */
      पटिनिवत्त lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  पटिनिवत्त रिक्त;  /* not found */
}


स्थिर पूर्णाङ्क ll_searchpath (lua_State *L) {
  ध्रुव वर्ण *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  सचे (f != रिक्त) पटिनिवत्त 1;
  अञ्ञथा {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_निवेशय(L, -2);
    पटिनिवत्त 2;  /* return fail + error message */
  }
}


स्थिर ध्रुव वर्ण *findfile (lua_State *L, ध्रुव वर्ण *name,
                                           ध्रुव वर्ण *pname,
                                           ध्रुव वर्ण *dirsep) {
  ध्रुव वर्ण *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  सचे (l_unlikely(path == रिक्त))
    luaL_error(L, "'package.%s' must be a string", pname);
  पटिनिवत्त searchpath(L, name, path, ".", dirsep);
}


स्थिर पूर्णाङ्क checkload (lua_State *L, पूर्णाङ्क stat, ध्रुव वर्ण *filename) {
  सचे (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    पटिनिवत्त 2;  /* return open function and file name */
  }
  अञ्ञथा
    पटिनिवत्त luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


स्थिर पूर्णाङ्क searcher_Lua (lua_State *L) {
  ध्रुव वर्ण *filename;
  ध्रुव वर्ण *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  सचे (filename == रिक्त) पटिनिवत्त 1;  /* module not found in this path */
  पटिनिवत्त checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
स्थिर पूर्णाङ्क loadfunc (lua_State *L, ध्रुव वर्ण *filename, ध्रुव वर्ण *modname) {
  ध्रुव वर्ण *openfunc;
  ध्रुव वर्ण *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  सचे (mark) {
    पूर्णाङ्क stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    सचे (stat != ERRFUNC) पटिनिवत्त stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  पटिनिवत्त lookforfunc(L, filename, openfunc);
}


स्थिर पूर्णाङ्क searcher_C (lua_State *L) {
  ध्रुव वर्ण *name = luaL_checkstring(L, 1);
  ध्रुव वर्ण *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  सचे (filename == रिक्त) पटिनिवत्त 1;  /* module not found in this path */
  पटिनिवत्त checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


स्थिर पूर्णाङ्क searcher_Croot (lua_State *L) {
  ध्रुव वर्ण *filename;
  ध्रुव वर्ण *name = luaL_checkstring(L, 1);
  ध्रुव वर्ण *p = strchr(name, '.');
  पूर्णाङ्क stat;
  सचे (p == रिक्त) पटिनिवत्त 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  सचे (filename == रिक्त) पटिनिवत्त 1;  /* root not found */
  सचे ((stat = loadfunc(L, filename, name)) != 0) {
    सचे (stat != ERRFUNC)
      पटिनिवत्त checkload(L, 0, filename);  /* real error */
    अञ्ञथा {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      पटिनिवत्त 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  पटिनिवत्त 2;
}


स्थिर पूर्णाङ्क searcher_preload (lua_State *L) {
  ध्रुव वर्ण *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  सचे (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    पटिनिवत्त 1;
  }
  अञ्ञथा {
    lua_pushliteral(L, ":preload:");
    पटिनिवत्त 2;
  }
}


स्थिर शून्य findloader (lua_State *L, ध्रुव वर्ण *name) {
  पूर्णाङ्क i;
  luaL_Buffer सन्देशः;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  सचे (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &सन्देशः);
  luaL_addstring(&सन्देशः, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  अत्थाय (i = 1; ; i++) {
    सचे (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_निष्कासय(L, 1);  /* remove nil */
      luaL_buffsub(&सन्देशः, 2);  /* remove last prefix */
      luaL_pushresult(&सन्देशः);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    सचे (lua_isfunction(L, -2))  /* did it find a loader? */
      पटिनिवत्त;  /* module loader found */
    अञ्ञथा सचे (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_निष्कासय(L, 1);  /* remove extra return */
      luaL_addvalue(&सन्देशः);  /* concatenate error message */
      luaL_addstring(&सन्देशः, "\n\t");  /* prefix for next message */
    }
    अञ्ञथा  /* no error message */
      lua_निष्कासय(L, 2);  /* remove both returns */
  }
}


स्थिर पूर्णाङ्क ll_require (lua_State *L) {
  ध्रुव वर्ण *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  सचे (lua_toboolean(L, -1))  /* is it there? */
    पटिनिवत्त 1;  /* package is already loaded */
  /* else must load package */
  lua_निष्कासय(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  सचे (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  अञ्ञथा
    lua_निष्कासय(L, 1);  /* pop nil */
  सचे (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  पटिनिवत्त 2;  /* return module result and loader data */
}

/* }====================================================== */




स्थिर ध्रुव luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", रिक्त},
  {"cpath", रिक्त},
  {"path", रिक्त},
  {"searchers", रिक्त},
  {"loaded", रिक्त},
  {रिक्त, रिक्त}
};


स्थिर ध्रुव luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {रिक्त, रिक्त}
};


स्थिर शून्य createsearcherstable (lua_State *L) {
  स्थिर ध्रुव lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    रिक्त
  };
  पूर्णाङ्क i;
  /* create 'searchers' table */
  lua_createtable(L, परिमाण(searchers)/परिमाण(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  अत्थाय (i=0; searchers[i] != रिक्त; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API पूर्णाङ्क luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_निष्कासय(L, 1);  /* will not use it now */
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
  lua_निष्कासय(L, 1);  /* pop global table */
  पटिनिवत्त 1;  /* return 'package' table */
}

