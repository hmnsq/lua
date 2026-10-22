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
#जर !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#जर !defined(LUA_LSUBSEP)
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
स्थिर स्थिरांक अक्षर *स्थिरांक CLIBS = "_CLIBS";

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
स्थिर शून्य *lsys_load (lua_State *L, स्थिरांक अक्षर *path, पूर्णांक seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, स्थिरांक अक्षर *sym);




#जर defined(LUA_USE_DLOPEN)	/* { */
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


स्थिर शून्य *lsys_load (lua_State *L, स्थिरांक अक्षर *path, पूर्णांक seeglb) {
  शून्य *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  जर (l_unlikely(lib == रिक्त))
    lua_pushstring(L, dlerror());
  परत lib;
}


स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, स्थिरांक अक्षर *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  जर (l_unlikely(f == रिक्त))
    lua_pushstring(L, dlerror());
  परत f;
}

/* }====================================================== */



#किंवाजर defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#अन्तर्भाव <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#जर !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
स्थिर शून्य setprogdir (lua_State *L) {
  अक्षर buff[MAX_PATH + 1];
  अक्षर *lb;
  DWORD nsize = आकार(buff)/आकार(अक्षर);
  DWORD n = GetModuleFileNameA(रिक्त, buff, nsize);  /* get exec. name */
  जर (n == 0 || n == nsize || (lb = म_खोजप(buff, '\\')) == रिक्त)
    luaL_error(L, "unable to get ModuleFileName");
  अन्यथा {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




स्थिर शून्य pusherror (lua_State *L) {
  पूर्णांक error = GetLastError();
  अक्षर buffer[128];
  जर (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      रिक्त, error, 0, buffer, आकार(buffer)/आकार(अक्षर), रिक्त))
    lua_pushstring(L, buffer);
  अन्यथा
    lua_pushfstring(L, "system error %d\n", error);
}

स्थिर शून्य lsys_unloadlib (शून्य *lib) {
  FreeLibrary((HMODULE)lib);
}


स्थिर शून्य *lsys_load (lua_State *L, स्थिरांक अक्षर *path, पूर्णांक seeglb) {
  HMODULE lib = LoadLibraryExA(path, रिक्त, LUA_LLE_FLAGS);
  (शून्य)(seeglb);  /* not used: symbols are 'global' by default */
  जर (lib == रिक्त) pusherror(L);
  परत lib;
}


स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, स्थिरांक अक्षर *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  जर (f == रिक्त) pusherror(L);
  परत f;
}

/* }====================================================== */


#अन्यथा				/* }{ */
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


स्थिर शून्य *lsys_load (lua_State *L, स्थिरांक अक्षर *path, पूर्णांक seeglb) {
  (शून्य)(path); (शून्य)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  परत रिक्त;
}


स्थिर lua_CFunction lsys_sym (lua_State *L, शून्य *lib, स्थिरांक अक्षर *sym) {
  (शून्य)(lib); (शून्य)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  परत रिक्त;
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
#जर !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#जर !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
स्थिर पूर्णांक noenv (lua_State *L) {
  पूर्णांक b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_काढा(L, 1);  /* remove value */
  परत b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
स्थिर शून्य setpath (lua_State *L, स्थिरांक अक्षर *fieldname,
                                   स्थिरांक अक्षर *envname,
                                   स्थिरांक अक्षर *dft) {
  स्थिरांक अक्षर *dftmark;
  स्थिरांक अक्षर *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  स्थिरांक अक्षर *path = दो_पर्या(nver);  /* try versioned name */
  जर (path == रिक्त)  /* no versioned environment variable? */
    path = दो_पर्या(envname);  /* try unversioned name */
  जर (path == रिक्त || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, म_माप(dft), रिक्त, रिक्त);  /* use default */
  अन्यथा जर ((dftmark = म_माला(path, LUA_PATH_SEP LUA_PATH_SEP)) == रिक्त)
    lua_pushstring(L, path);  /* nothing to change */
  अन्यथा {  /* path contains a ";;": insert default path in its place */
    माप_प्रकार len = म_माप(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    जर (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    जर (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_काढा(L, 1);  /* pop versioned variable name ('nver') */
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
स्थिर शून्य *checkclib (lua_State *L, स्थिरांक अक्षर *path) {
  शून्य *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_काढा(L, 2);  /* pop CLIBS table and 'plib' */
  परत plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
स्थिर शून्य *freelib (शून्य *ud, शून्य *ptr, माप_प्रकार osize, माप_प्रकार nsize) {
  /* string itself is irrelevant and static */
  (शून्य)ptr; (शून्य)osize; (शून्य)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  परत रिक्त;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
स्थिर शून्य createlibstr (lua_State *L, शून्य *plib) {
  /* common content for all library strings */
  स्थिर स्थिरांक अक्षर dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, आकार(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
स्थिर शून्य addtoclib (lua_State *L, स्थिरांक अक्षर *path, शून्य *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_काढा(L, 1);  /* pop CLIBS table */
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
स्थिर पूर्णांक lookforfunc (lua_State *L, स्थिरांक अक्षर *path, स्थिरांक अक्षर *sym) {
  शून्य *reg = checkclib(L, path);  /* check loaded C libraries */
  जर (reg == रिक्त) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    जर (reg == रिक्त) परत ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  जर (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    परत 0;  /* no errors */
  }
  अन्यथा {
    lua_CFunction f = lsys_sym(L, reg, sym);
    जर (f == रिक्त)
      परत ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    परत 0;  /* no errors */
  }
}


स्थिर पूर्णांक ll_loadlib (lua_State *L) {
  स्थिरांक अक्षर *path = luaL_checkstring(L, 1);
  स्थिरांक अक्षर *init = luaL_checkstring(L, 2);
  पूर्णांक stat = lookforfunc(L, path, init);
  जर (l_likely(stat == 0))  /* no errors? */
    परत 1;  /* return the loaded function */
  अन्यथा {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_घाला(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    परत 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


स्थिर पूर्णांक readable (स्थिरांक अक्षर *filename) {
  खाता *f = ख_खोलो(filename, "r");  /* try to open file */
  जर (f == रिक्त) परत 0;  /* open failed */
  ख_बंद(f);
  परत 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
स्थिर स्थिरांक अक्षर *getnextfilename (अक्षर **path, अक्षर *end) {
  अक्षर *sep;
  अक्षर *name = *path;
  जर (name == end)
    परत रिक्त;  /* no more names */
  अन्यथा जर (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = म_अक्षर(name, *LUA_PATH_SEP);  /* find next separator */
  जर (sep == रिक्त)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  परत name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
स्थिर शून्य pusherrornotfound (lua_State *L, स्थिरांक अक्षर *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


स्थिर स्थिरांक अक्षर *searchpath (lua_State *L, स्थिरांक अक्षर *name,
                                             स्थिरांक अक्षर *path,
                                             स्थिरांक अक्षर *sep,
                                             स्थिरांक अक्षर *dirsep) {
  luaL_Buffer buff;
  अक्षर *pathname;  /* path with name inserted */
  अक्षर *endpathname;  /* its end */
  स्थिरांक अक्षर *filename;
  /* separator is non-empty and appears in 'name'? */
  जर (*sep != '\0' && म_अक्षर(name, *sep) != रिक्त)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  पर्यंत ((filename = getnextfilename(&pathname, endpathname)) != रिक्त) {
    जर (readable(filename))  /* does file exist and is readable? */
      परत lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  परत रिक्त;  /* not found */
}


स्थिर पूर्णांक ll_searchpath (lua_State *L) {
  स्थिरांक अक्षर *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  जर (f != रिक्त) परत 1;
  अन्यथा {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_घाला(L, -2);
    परत 2;  /* return fail + error message */
  }
}


स्थिर स्थिरांक अक्षर *findfile (lua_State *L, स्थिरांक अक्षर *name,
                                           स्थिरांक अक्षर *pname,
                                           स्थिरांक अक्षर *dirsep) {
  स्थिरांक अक्षर *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  जर (l_unlikely(path == रिक्त))
    luaL_error(L, "'package.%s' must be a string", pname);
  परत searchpath(L, name, path, ".", dirsep);
}


स्थिर पूर्णांक checkload (lua_State *L, पूर्णांक stat, स्थिरांक अक्षर *filename) {
  जर (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    परत 2;  /* return open function and file name */
  }
  अन्यथा
    परत luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


स्थिर पूर्णांक searcher_Lua (lua_State *L) {
  स्थिरांक अक्षर *filename;
  स्थिरांक अक्षर *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  जर (filename == रिक्त) परत 1;  /* module not found in this path */
  परत checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
स्थिर पूर्णांक loadfunc (lua_State *L, स्थिरांक अक्षर *filename, स्थिरांक अक्षर *modname) {
  स्थिरांक अक्षर *openfunc;
  स्थिरांक अक्षर *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = म_अक्षर(modname, *LUA_IGMARK);
  जर (mark) {
    पूर्णांक stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    जर (stat != ERRFUNC) परत stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  परत lookforfunc(L, filename, openfunc);
}


स्थिर पूर्णांक searcher_C (lua_State *L) {
  स्थिरांक अक्षर *name = luaL_checkstring(L, 1);
  स्थिरांक अक्षर *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  जर (filename == रिक्त) परत 1;  /* module not found in this path */
  परत checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


स्थिर पूर्णांक searcher_Croot (lua_State *L) {
  स्थिरांक अक्षर *filename;
  स्थिरांक अक्षर *name = luaL_checkstring(L, 1);
  स्थिरांक अक्षर *p = म_अक्षर(name, '.');
  पूर्णांक stat;
  जर (p == रिक्त) परत 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  जर (filename == रिक्त) परत 1;  /* root not found */
  जर ((stat = loadfunc(L, filename, name)) != 0) {
    जर (stat != ERRFUNC)
      परत checkload(L, 0, filename);  /* real error */
    अन्यथा {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      परत 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  परत 2;
}


स्थिर पूर्णांक searcher_preload (lua_State *L) {
  स्थिरांक अक्षर *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  जर (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    परत 1;
  }
  अन्यथा {
    lua_pushliteral(L, ":preload:");
    परत 2;
  }
}


स्थिर शून्य findloader (lua_State *L, स्थिरांक अक्षर *name) {
  पूर्णांक i;
  luaL_Buffer संदेश;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  जर (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &संदेश);
  luaL_addstring(&संदेश, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  साठी (i = 1; ; i++) {
    जर (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_काढा(L, 1);  /* remove nil */
      luaL_buffsub(&संदेश, 2);  /* remove last prefix */
      luaL_pushresult(&संदेश);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    जर (lua_isfunction(L, -2))  /* did it find a loader? */
      परत;  /* module loader found */
    अन्यथा जर (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_काढा(L, 1);  /* remove extra return */
      luaL_addvalue(&संदेश);  /* concatenate error message */
      luaL_addstring(&संदेश, "\n\t");  /* prefix for next message */
    }
    अन्यथा  /* no error message */
      lua_काढा(L, 2);  /* remove both returns */
  }
}


स्थिर पूर्णांक ll_require (lua_State *L) {
  स्थिरांक अक्षर *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  जर (lua_toboolean(L, -1))  /* is it there? */
    परत 1;  /* package is already loaded */
  /* else must load package */
  lua_काढा(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  जर (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  अन्यथा
    lua_काढा(L, 1);  /* pop nil */
  जर (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  परत 2;  /* return module result and loader data */
}

/* }====================================================== */




स्थिर स्थिरांक luaL_Reg pk_funcs[] = {
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


स्थिर स्थिरांक luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {रिक्त, रिक्त}
};


स्थिर शून्य createsearcherstable (lua_State *L) {
  स्थिर स्थिरांक lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    रिक्त
  };
  पूर्णांक i;
  /* create 'searchers' table */
  lua_createtable(L, आकार(searchers)/आकार(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  साठी (i=0; searchers[i] != रिक्त; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API पूर्णांक luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_काढा(L, 1);  /* will not use it now */
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
  lua_काढा(L, 1);  /* pop global table */
  परत 1;  /* return 'package' table */
}

