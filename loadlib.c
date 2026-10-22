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

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lualib.h"
#অন্তর্ভাব "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#যদি !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#যদি !defined(LUA_LSUBSEP)
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
স্থির ধ্রুবক অক্ষর *ধ্রুবক CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((শূন্য)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
স্থির শূন্য lsys_unloadlib (শূন্য *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
স্থির শূন্য *lsys_load (lua_State *L, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
স্থির lua_CFunction lsys_sym (lua_State *L, শূন্য *lib, ধ্রুবক অক্ষর *sym);




#যদি defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#অন্তর্ভাব <dlfcn.h>


স্থির শূন্য lsys_unloadlib (শূন্য *lib) {
  dlclose(lib);
}


স্থির শূন্য *lsys_load (lua_State *L, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা seeglb) {
  শূন্য *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  যদি (l_unlikely(lib == ফাঁকা))
    lua_pushstring(L, dlerror());
  ফেরত lib;
}


স্থির lua_CFunction lsys_sym (lua_State *L, শূন্য *lib, ধ্রুবক অক্ষর *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  যদি (l_unlikely(f == ফাঁকা))
    lua_pushstring(L, dlerror());
  ফেরত f;
}

/* }====================================================== */



#নতুবা_যদি defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#অন্তর্ভাব <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#যদি !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
স্থির শূন্য setprogdir (lua_State *L) {
  অক্ষর buff[MAX_PATH + 1];
  অক্ষর *lb;
  DWORD nsize = আকার(buff)/আকার(অক্ষর);
  DWORD n = GetModuleFileNameA(ফাঁকা, buff, nsize);  /* get exec. name */
  যদি (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == ফাঁকা)
    luaL_error(L, "unable to get ModuleFileName");
  অন্যথা {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




স্থির শূন্য pusherror (lua_State *L) {
  পূর্ণসংখ্যা error = GetLastError();
  অক্ষর বাফার[128];
  যদি (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      ফাঁকা, error, 0, বাফার, আকার(বাফার)/আকার(অক্ষর), ফাঁকা))
    lua_pushstring(L, বাফার);
  অন্যথা
    lua_pushfstring(L, "system error %d\n", error);
}

স্থির শূন্য lsys_unloadlib (শূন্য *lib) {
  FreeLibrary((HMODULE)lib);
}


স্থির শূন্য *lsys_load (lua_State *L, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা seeglb) {
  HMODULE lib = LoadLibraryExA(path, ফাঁকা, LUA_LLE_FLAGS);
  (শূন্য)(seeglb);  /* not used: symbols are 'global' by default */
  যদি (lib == ফাঁকা) pusherror(L);
  ফেরত lib;
}


স্থির lua_CFunction lsys_sym (lua_State *L, শূন্য *lib, ধ্রুবক অক্ষর *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  যদি (f == ফাঁকা) pusherror(L);
  ফেরত f;
}

/* }====================================================== */


#অন্যথা				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


স্থির শূন্য lsys_unloadlib (শূন্য *lib) {
  (শূন্য)(lib);  /* not used */
}


স্থির শূন্য *lsys_load (lua_State *L, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা seeglb) {
  (শূন্য)(path); (শূন্য)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  ফেরত ফাঁকা;
}


স্থির lua_CFunction lsys_sym (lua_State *L, শূন্য *lib, ধ্রুবক অক্ষর *sym) {
  (শূন্য)(lib); (শূন্য)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  ফেরত ফাঁকা;
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
#যদি !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#যদি !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
স্থির পূর্ণসংখ্যা noenv (lua_State *L) {
  পূর্ণসংখ্যা b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_বের_করো(L, 1);  /* remove value */
  ফেরত b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
স্থির শূন্য setpath (lua_State *L, ধ্রুবক অক্ষর *fieldname,
                                   ধ্রুবক অক্ষর *envname,
                                   ধ্রুবক অক্ষর *dft) {
  ধ্রুবক অক্ষর *dftmark;
  ধ্রুবক অক্ষর *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ধ্রুবক অক্ষর *path = getenv(nver);  /* try versioned name */
  যদি (path == ফাঁকা)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  যদি (path == ফাঁকা || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), ফাঁকা, ফাঁকা);  /* use default */
  অন্যথা যদি ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == ফাঁকা)
    lua_pushstring(L, path);  /* nothing to change */
  অন্যথা {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    lua_L_বাফার b;
    luaL_buffinit(L, &b);
    যদি (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    যদি (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_বের_করো(L, 1);  /* pop versioned variable name ('nver') */
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
স্থির শূন্য *checkclib (lua_State *L, ধ্রুবক অক্ষর *path) {
  শূন্য *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_বের_করো(L, 2);  /* pop CLIBS table and 'plib' */
  ফেরত plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
স্থির শূন্য *freelib (শূন্য *ud, শূন্য *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (শূন্য)ptr; (শূন্য)osize; (শূন্য)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  ফেরত ফাঁকা;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
স্থির শূন্য createlibstr (lua_State *L, শূন্য *plib) {
  /* common content for all library strings */
  স্থির ধ্রুবক অক্ষর dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, আকার(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
স্থির শূন্য addtoclib (lua_State *L, ধ্রুবক অক্ষর *path, শূন্য *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_বের_করো(L, 1);  /* pop CLIBS table */
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
স্থির পূর্ণসংখ্যা lookforfunc (lua_State *L, ধ্রুবক অক্ষর *path, ধ্রুবক অক্ষর *sym) {
  শূন্য *reg = checkclib(L, path);  /* check loaded C libraries */
  যদি (reg == ফাঁকা) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    যদি (reg == ফাঁকা) ফেরত ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  যদি (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    ফেরত 0;  /* no errors */
  }
  অন্যথা {
    lua_CFunction f = lsys_sym(L, reg, sym);
    যদি (f == ফাঁকা)
      ফেরত ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    ফেরত 0;  /* no errors */
  }
}


স্থির পূর্ণসংখ্যা ll_loadlib (lua_State *L) {
  ধ্রুবক অক্ষর *path = luaL_checkstring(L, 1);
  ধ্রুবক অক্ষর *init = luaL_checkstring(L, 2);
  পূর্ণসংখ্যা stat = lookforfunc(L, path, init);
  যদি (l_likely(stat == 0))  /* no errors? */
    ফেরত 1;  /* return the loaded function */
  অন্যথা {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_ঢোকাও(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    ফেরত 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


স্থির পূর্ণসংখ্যা readable (ধ্রুবক অক্ষর *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  যদি (f == ফাঁকা) ফেরত 0;  /* open failed */
  fclose(f);
  ফেরত 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
স্থির ধ্রুবক অক্ষর *getnextfilename (অক্ষর **path, অক্ষর *end) {
  অক্ষর *sep;
  অক্ষর *name = *path;
  যদি (name == end)
    ফেরত ফাঁকা;  /* no more names */
  অন্যথা যদি (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  যদি (sep == ফাঁকা)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  ফেরত name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
স্থির শূন্য pusherrornotfound (lua_State *L, ধ্রুবক অক্ষর *path) {
  lua_L_বাফার b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


স্থির ধ্রুবক অক্ষর *searchpath (lua_State *L, ধ্রুবক অক্ষর *name,
                                             ধ্রুবক অক্ষর *path,
                                             ধ্রুবক অক্ষর *sep,
                                             ধ্রুবক অক্ষর *dirsep) {
  lua_L_বাফার buff;
  অক্ষর *pathname;  /* path with name inserted */
  অক্ষর *endpathname;  /* its end */
  ধ্রুবক অক্ষর *filename;
  /* separator is non-empty and appears in 'name'? */
  যদি (*sep != '\0' && strchr(name, *sep) != ফাঁকা)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  যখন ((filename = getnextfilename(&pathname, endpathname)) != ফাঁকা) {
    যদি (readable(filename))  /* does file exist and is readable? */
      ফেরত lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  ফেরত ফাঁকা;  /* not found */
}


স্থির পূর্ণসংখ্যা ll_searchpath (lua_State *L) {
  ধ্রুবক অক্ষর *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  যদি (f != ফাঁকা) ফেরত 1;
  অন্যথা {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_ঢোকাও(L, -2);
    ফেরত 2;  /* return fail + error message */
  }
}


স্থির ধ্রুবক অক্ষর *findfile (lua_State *L, ধ্রুবক অক্ষর *name,
                                           ধ্রুবক অক্ষর *pname,
                                           ধ্রুবক অক্ষর *dirsep) {
  ধ্রুবক অক্ষর *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  যদি (l_unlikely(path == ফাঁকা))
    luaL_error(L, "'package.%s' must be a string", pname);
  ফেরত searchpath(L, name, path, ".", dirsep);
}


স্থির পূর্ণসংখ্যা checkload (lua_State *L, পূর্ণসংখ্যা stat, ধ্রুবক অক্ষর *filename) {
  যদি (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    ফেরত 2;  /* return open function and file name */
  }
  অন্যথা
    ফেরত luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


স্থির পূর্ণসংখ্যা searcher_Lua (lua_State *L) {
  ধ্রুবক অক্ষর *filename;
  ধ্রুবক অক্ষর *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  যদি (filename == ফাঁকা) ফেরত 1;  /* module not found in this path */
  ফেরত checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
স্থির পূর্ণসংখ্যা loadfunc (lua_State *L, ধ্রুবক অক্ষর *filename, ধ্রুবক অক্ষর *modname) {
  ধ্রুবক অক্ষর *openfunc;
  ধ্রুবক অক্ষর *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  যদি (mark) {
    পূর্ণসংখ্যা stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    যদি (stat != ERRFUNC) ফেরত stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  ফেরত lookforfunc(L, filename, openfunc);
}


স্থির পূর্ণসংখ্যা searcher_C (lua_State *L) {
  ধ্রুবক অক্ষর *name = luaL_checkstring(L, 1);
  ধ্রুবক অক্ষর *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  যদি (filename == ফাঁকা) ফেরত 1;  /* module not found in this path */
  ফেরত checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


স্থির পূর্ণসংখ্যা searcher_Croot (lua_State *L) {
  ধ্রুবক অক্ষর *filename;
  ধ্রুবক অক্ষর *name = luaL_checkstring(L, 1);
  ধ্রুবক অক্ষর *p = strchr(name, '.');
  পূর্ণসংখ্যা stat;
  যদি (p == ফাঁকা) ফেরত 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  যদি (filename == ফাঁকা) ফেরত 1;  /* root not found */
  যদি ((stat = loadfunc(L, filename, name)) != 0) {
    যদি (stat != ERRFUNC)
      ফেরত checkload(L, 0, filename);  /* real error */
    অন্যথা {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      ফেরত 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  ফেরত 2;
}


স্থির পূর্ণসংখ্যা searcher_preload (lua_State *L) {
  ধ্রুবক অক্ষর *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  যদি (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    ফেরত 1;
  }
  অন্যথা {
    lua_pushliteral(L, ":preload:");
    ফেরত 2;
  }
}


স্থির শূন্য findloader (lua_State *L, ধ্রুবক অক্ষর *name) {
  পূর্ণসংখ্যা i;
  lua_L_বাফার বার্তা;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  যদি (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &বার্তা);
  luaL_addstring(&বার্তা, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  জন্য (i = 1; ; i++) {
    যদি (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_বের_করো(L, 1);  /* remove nil */
      luaL_buffsub(&বার্তা, 2);  /* remove last prefix */
      luaL_pushresult(&বার্তা);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    যদি (lua_isfunction(L, -2))  /* did it find a loader? */
      ফেরত;  /* module loader found */
    অন্যথা যদি (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_বের_করো(L, 1);  /* remove extra return */
      luaL_addvalue(&বার্তা);  /* concatenate error message */
      luaL_addstring(&বার্তা, "\n\t");  /* prefix for next message */
    }
    অন্যথা  /* no error message */
      lua_বের_করো(L, 2);  /* remove both returns */
  }
}


স্থির পূর্ণসংখ্যা ll_require (lua_State *L) {
  ধ্রুবক অক্ষর *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  যদি (lua_toboolean(L, -1))  /* is it there? */
    ফেরত 1;  /* package is already loaded */
  /* else must load package */
  lua_বের_করো(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  যদি (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  অন্যথা
    lua_বের_করো(L, 1);  /* pop nil */
  যদি (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  ফেরত 2;  /* return module result and loader data */
}

/* }====================================================== */




স্থির ধ্রুবক luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", ফাঁকা},
  {"cpath", ফাঁকা},
  {"path", ফাঁকা},
  {"searchers", ফাঁকা},
  {"loaded", ফাঁকা},
  {ফাঁকা, ফাঁকা}
};


স্থির ধ্রুবক luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {ফাঁকা, ফাঁকা}
};


স্থির শূন্য createsearcherstable (lua_State *L) {
  স্থির ধ্রুবক lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    ফাঁকা
  };
  পূর্ণসংখ্যা i;
  /* create 'searchers' table */
  lua_createtable(L, আকার(searchers)/আকার(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  জন্য (i=0; searchers[i] != ফাঁকা; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API পূর্ণসংখ্যা luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_বের_করো(L, 1);  /* will not use it now */
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
  lua_বের_করো(L, 1);  /* pop global table */
  ফেরত 1;  /* return 'package' table */
}

