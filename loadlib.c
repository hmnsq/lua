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

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#எனில் !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#எனில் !defined(LUA_LSUBSEP)
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
நிலையான மாறிலி எழுத்து *மாறிலி CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((வெறுமை)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
நிலையான வெறுமை lsys_unloadlib (வெறுமை *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
நிலையான வெறுமை *lsys_load (lua_State *L, மாறிலி எழுத்து *path, முழு seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
நிலையான lua_CFunction lsys_sym (lua_State *L, வெறுமை *lib, மாறிலி எழுத்து *sym);




#எனில் defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#அந்தர்பாவ <dlfcn.h>


நிலையான வெறுமை lsys_unloadlib (வெறுமை *lib) {
  dlclose(lib);
}


நிலையான வெறுமை *lsys_load (lua_State *L, மாறிலி எழுத்து *path, முழு seeglb) {
  வெறுமை *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  எனில் (l_unlikely(lib == வெற்று))
    lua_pushstring(L, dlerror());
  திரும்பு lib;
}


நிலையான lua_CFunction lsys_sym (lua_State *L, வெறுமை *lib, மாறிலி எழுத்து *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  எனில் (l_unlikely(f == வெற்று))
    lua_pushstring(L, dlerror());
  திரும்பு f;
}

/* }====================================================== */



#அல்லதெனில் defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#அந்தர்பாவ <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#எனில் !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
நிலையான வெறுமை setprogdir (lua_State *L) {
  எழுத்து buff[MAX_PATH + 1];
  எழுத்து *lb;
  DWORD nsize = அளவு(buff)/அளவு(எழுத்து);
  DWORD n = GetModuleFileNameA(வெற்று, buff, nsize);  /* get exec. name */
  எனில் (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == வெற்று)
    luaL_error(L, "unable to get ModuleFileName");
  இல்லையெனில் {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




நிலையான வெறுமை pusherror (lua_State *L) {
  முழு error = GetLastError();
  எழுத்து buffer[128];
  எனில் (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      வெற்று, error, 0, buffer, அளவு(buffer)/அளவு(எழுத்து), வெற்று))
    lua_pushstring(L, buffer);
  இல்லையெனில்
    lua_pushfstring(L, "system error %d\n", error);
}

நிலையான வெறுமை lsys_unloadlib (வெறுமை *lib) {
  FreeLibrary((HMODULE)lib);
}


நிலையான வெறுமை *lsys_load (lua_State *L, மாறிலி எழுத்து *path, முழு seeglb) {
  HMODULE lib = LoadLibraryExA(path, வெற்று, LUA_LLE_FLAGS);
  (வெறுமை)(seeglb);  /* not used: symbols are 'global' by default */
  எனில் (lib == வெற்று) pusherror(L);
  திரும்பு lib;
}


நிலையான lua_CFunction lsys_sym (lua_State *L, வெறுமை *lib, மாறிலி எழுத்து *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  எனில் (f == வெற்று) pusherror(L);
  திரும்பு f;
}

/* }====================================================== */


#இல்லையெனில்				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


நிலையான வெறுமை lsys_unloadlib (வெறுமை *lib) {
  (வெறுமை)(lib);  /* not used */
}


நிலையான வெறுமை *lsys_load (lua_State *L, மாறிலி எழுத்து *path, முழு seeglb) {
  (வெறுமை)(path); (வெறுமை)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  திரும்பு வெற்று;
}


நிலையான lua_CFunction lsys_sym (lua_State *L, வெறுமை *lib, மாறிலி எழுத்து *sym) {
  (வெறுமை)(lib); (வெறுமை)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  திரும்பு வெற்று;
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
#எனில் !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#எனில் !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
நிலையான முழு noenv (lua_State *L) {
  முழு b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_எடு(L, 1);  /* remove value */
  திரும்பு b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
நிலையான வெறுமை setpath (lua_State *L, மாறிலி எழுத்து *fieldname,
                                   மாறிலி எழுத்து *envname,
                                   மாறிலி எழுத்து *dft) {
  மாறிலி எழுத்து *dftmark;
  மாறிலி எழுத்து *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  மாறிலி எழுத்து *path = getenv(nver);  /* try versioned name */
  எனில் (path == வெற்று)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  எனில் (path == வெற்று || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), வெற்று, வெற்று);  /* use default */
  இல்லையெனில் எனில் ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == வெற்று)
    lua_pushstring(L, path);  /* nothing to change */
  இல்லையெனில் {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    எனில் (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    எனில் (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_எடு(L, 1);  /* pop versioned variable name ('nver') */
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
நிலையான வெறுமை *checkclib (lua_State *L, மாறிலி எழுத்து *path) {
  வெறுமை *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_எடு(L, 2);  /* pop CLIBS table and 'plib' */
  திரும்பு plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
நிலையான வெறுமை *freelib (வெறுமை *ud, வெறுமை *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (வெறுமை)ptr; (வெறுமை)osize; (வெறுமை)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  திரும்பு வெற்று;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
நிலையான வெறுமை createlibstr (lua_State *L, வெறுமை *plib) {
  /* common content for all library strings */
  நிலையான மாறிலி எழுத்து dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, அளவு(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
நிலையான வெறுமை addtoclib (lua_State *L, மாறிலி எழுத்து *path, வெறுமை *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_எடு(L, 1);  /* pop CLIBS table */
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
நிலையான முழு lookforfunc (lua_State *L, மாறிலி எழுத்து *path, மாறிலி எழுத்து *sym) {
  வெறுமை *reg = checkclib(L, path);  /* check loaded C libraries */
  எனில் (reg == வெற்று) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    எனில் (reg == வெற்று) திரும்பு ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  எனில் (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    திரும்பு 0;  /* no errors */
  }
  இல்லையெனில் {
    lua_CFunction f = lsys_sym(L, reg, sym);
    எனில் (f == வெற்று)
      திரும்பு ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    திரும்பு 0;  /* no errors */
  }
}


நிலையான முழு ll_loadlib (lua_State *L) {
  மாறிலி எழுத்து *path = luaL_checkstring(L, 1);
  மாறிலி எழுத்து *init = luaL_checkstring(L, 2);
  முழு stat = lookforfunc(L, path, init);
  எனில் (l_likely(stat == 0))  /* no errors? */
    திரும்பு 1;  /* return the loaded function */
  இல்லையெனில் {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_செருகு(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    திரும்பு 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


நிலையான முழு readable (மாறிலி எழுத்து *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  எனில் (f == வெற்று) திரும்பு 0;  /* open failed */
  fclose(f);
  திரும்பு 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
நிலையான மாறிலி எழுத்து *getnextfilename (எழுத்து **path, எழுத்து *end) {
  எழுத்து *sep;
  எழுத்து *name = *path;
  எனில் (name == end)
    திரும்பு வெற்று;  /* no more names */
  இல்லையெனில் எனில் (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  எனில் (sep == வெற்று)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  திரும்பு name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
நிலையான வெறுமை pusherrornotfound (lua_State *L, மாறிலி எழுத்து *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


நிலையான மாறிலி எழுத்து *searchpath (lua_State *L, மாறிலி எழுத்து *name,
                                             மாறிலி எழுத்து *path,
                                             மாறிலி எழுத்து *sep,
                                             மாறிலி எழுத்து *dirsep) {
  luaL_Buffer buff;
  எழுத்து *pathname;  /* path with name inserted */
  எழுத்து *endpathname;  /* its end */
  மாறிலி எழுத்து *filename;
  /* separator is non-empty and appears in 'name'? */
  எனில் (*sep != '\0' && strchr(name, *sep) != வெற்று)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  போது ((filename = getnextfilename(&pathname, endpathname)) != வெற்று) {
    எனில் (readable(filename))  /* does file exist and is readable? */
      திரும்பு lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  திரும்பு வெற்று;  /* not found */
}


நிலையான முழு ll_searchpath (lua_State *L) {
  மாறிலி எழுத்து *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  எனில் (f != வெற்று) திரும்பு 1;
  இல்லையெனில் {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_செருகு(L, -2);
    திரும்பு 2;  /* return fail + error message */
  }
}


நிலையான மாறிலி எழுத்து *findfile (lua_State *L, மாறிலி எழுத்து *name,
                                           மாறிலி எழுத்து *pname,
                                           மாறிலி எழுத்து *dirsep) {
  மாறிலி எழுத்து *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  எனில் (l_unlikely(path == வெற்று))
    luaL_error(L, "'package.%s' must be a string", pname);
  திரும்பு searchpath(L, name, path, ".", dirsep);
}


நிலையான முழு checkload (lua_State *L, முழு stat, மாறிலி எழுத்து *filename) {
  எனில் (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    திரும்பு 2;  /* return open function and file name */
  }
  இல்லையெனில்
    திரும்பு luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


நிலையான முழு searcher_Lua (lua_State *L) {
  மாறிலி எழுத்து *filename;
  மாறிலி எழுத்து *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  எனில் (filename == வெற்று) திரும்பு 1;  /* module not found in this path */
  திரும்பு checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
நிலையான முழு loadfunc (lua_State *L, மாறிலி எழுத்து *filename, மாறிலி எழுத்து *modname) {
  மாறிலி எழுத்து *openfunc;
  மாறிலி எழுத்து *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  எனில் (mark) {
    முழு stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    எனில் (stat != ERRFUNC) திரும்பு stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  திரும்பு lookforfunc(L, filename, openfunc);
}


நிலையான முழு searcher_C (lua_State *L) {
  மாறிலி எழுத்து *name = luaL_checkstring(L, 1);
  மாறிலி எழுத்து *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  எனில் (filename == வெற்று) திரும்பு 1;  /* module not found in this path */
  திரும்பு checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


நிலையான முழு searcher_Croot (lua_State *L) {
  மாறிலி எழுத்து *filename;
  மாறிலி எழுத்து *name = luaL_checkstring(L, 1);
  மாறிலி எழுத்து *p = strchr(name, '.');
  முழு stat;
  எனில் (p == வெற்று) திரும்பு 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  எனில் (filename == வெற்று) திரும்பு 1;  /* root not found */
  எனில் ((stat = loadfunc(L, filename, name)) != 0) {
    எனில் (stat != ERRFUNC)
      திரும்பு checkload(L, 0, filename);  /* real error */
    இல்லையெனில் {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      திரும்பு 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  திரும்பு 2;
}


நிலையான முழு searcher_preload (lua_State *L) {
  மாறிலி எழுத்து *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  எனில் (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    திரும்பு 1;
  }
  இல்லையெனில் {
    lua_pushliteral(L, ":preload:");
    திரும்பு 2;
  }
}


நிலையான வெறுமை findloader (lua_State *L, மாறிலி எழுத்து *name) {
  முழு i;
  luaL_Buffer செய்தி;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  எனில் (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &செய்தி);
  luaL_addstring(&செய்தி, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  க்கு (i = 1; ; i++) {
    எனில் (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_எடு(L, 1);  /* remove nil */
      luaL_buffsub(&செய்தி, 2);  /* remove last prefix */
      luaL_pushresult(&செய்தி);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    எனில் (lua_isfunction(L, -2))  /* did it find a loader? */
      திரும்பு;  /* module loader found */
    இல்லையெனில் எனில் (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_எடு(L, 1);  /* remove extra return */
      luaL_addvalue(&செய்தி);  /* concatenate error message */
      luaL_addstring(&செய்தி, "\n\t");  /* prefix for next message */
    }
    இல்லையெனில்  /* no error message */
      lua_எடு(L, 2);  /* remove both returns */
  }
}


நிலையான முழு ll_require (lua_State *L) {
  மாறிலி எழுத்து *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  எனில் (lua_toboolean(L, -1))  /* is it there? */
    திரும்பு 1;  /* package is already loaded */
  /* else must load package */
  lua_எடு(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  எனில் (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  இல்லையெனில்
    lua_எடு(L, 1);  /* pop nil */
  எனில் (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  திரும்பு 2;  /* return module result and loader data */
}

/* }====================================================== */




நிலையான மாறிலி luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", வெற்று},
  {"cpath", வெற்று},
  {"path", வெற்று},
  {"searchers", வெற்று},
  {"loaded", வெற்று},
  {வெற்று, வெற்று}
};


நிலையான மாறிலி luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {வெற்று, வெற்று}
};


நிலையான வெறுமை createsearcherstable (lua_State *L) {
  நிலையான மாறிலி lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    வெற்று
  };
  முழு i;
  /* create 'searchers' table */
  lua_createtable(L, அளவு(searchers)/அளவு(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  க்கு (i=0; searchers[i] != வெற்று; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API முழு luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_எடு(L, 1);  /* will not use it now */
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
  lua_எடு(L, 1);  /* pop global table */
  திரும்பு 1;  /* return 'package' table */
}

