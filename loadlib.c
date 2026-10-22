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

#شامل "lprefix.h"


#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#اگر !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#اگر !defined(LUA_LSUBSEP)
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
ایستا ثابت نویسه *ثابت CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((تهی)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
ایستا تهی lsys_unloadlib (تهی *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
ایستا تهی *lsys_load (lua_State *L, ثابت نویسه *path, صحیح seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
ایستا lua_CFunction lsys_sym (lua_State *L, تهی *lib, ثابت نویسه *sym);




#اگر defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#شامل <dlfcn.h>


ایستا تهی lsys_unloadlib (تهی *lib) {
  dlclose(lib);
}


ایستا تهی *lsys_load (lua_State *L, ثابت نویسه *path, صحیح seeglb) {
  تهی *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  اگر (l_unlikely(lib == پوچ))
    lua_pushstring(L, dlerror());
  بازگشت lib;
}


ایستا lua_CFunction lsys_sym (lua_State *L, تهی *lib, ثابت نویسه *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  اگر (l_unlikely(f == پوچ))
    lua_pushstring(L, dlerror());
  بازگشت f;
}

/* }====================================================== */



#وگرنهاگر defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#شامل <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#اگر !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
ایستا تهی setprogdir (lua_State *L) {
  نویسه buff[MAX_PATH + 1];
  نویسه *lb;
  DWORD nsize = اندازه(buff)/اندازه(نویسه);
  DWORD n = GetModuleFileNameA(پوچ, buff, nsize);  /* get exec. name */
  اگر (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == پوچ)
    luaL_error(L, "unable to get ModuleFileName");
  وگرنه {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




ایستا تهی pusherror (lua_State *L) {
  صحیح error = GetLastError();
  نویسه buffer[128];
  اگر (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      پوچ, error, 0, buffer, اندازه(buffer)/اندازه(نویسه), پوچ))
    lua_pushstring(L, buffer);
  وگرنه
    lua_pushfstring(L, "system error %d\n", error);
}

ایستا تهی lsys_unloadlib (تهی *lib) {
  FreeLibrary((HMODULE)lib);
}


ایستا تهی *lsys_load (lua_State *L, ثابت نویسه *path, صحیح seeglb) {
  HMODULE lib = LoadLibraryExA(path, پوچ, LUA_LLE_FLAGS);
  (تهی)(seeglb);  /* not used: symbols are 'global' by default */
  اگر (lib == پوچ) pusherror(L);
  بازگشت lib;
}


ایستا lua_CFunction lsys_sym (lua_State *L, تهی *lib, ثابت نویسه *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  اگر (f == پوچ) pusherror(L);
  بازگشت f;
}

/* }====================================================== */


#وگرنه				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


ایستا تهی lsys_unloadlib (تهی *lib) {
  (تهی)(lib);  /* not used */
}


ایستا تهی *lsys_load (lua_State *L, ثابت نویسه *path, صحیح seeglb) {
  (تهی)(path); (تهی)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  بازگشت پوچ;
}


ایستا lua_CFunction lsys_sym (lua_State *L, تهی *lib, ثابت نویسه *sym) {
  (تهی)(lib); (تهی)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  بازگشت پوچ;
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
#اگر !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#اگر !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
ایستا صحیح noenv (lua_State *L) {
  صحیح b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_برداشت(L, 1);  /* remove value */
  بازگشت b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
ایستا تهی setpath (lua_State *L, ثابت نویسه *fieldname,
                                   ثابت نویسه *envname,
                                   ثابت نویسه *dft) {
  ثابت نویسه *dftmark;
  ثابت نویسه *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ثابت نویسه *path = getenv(nver);  /* try versioned name */
  اگر (path == پوچ)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  اگر (path == پوچ || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), پوچ, پوچ);  /* use default */
  وگرنه اگر ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == پوچ)
    lua_pushstring(L, path);  /* nothing to change */
  وگرنه {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    اگر (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    اگر (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_برداشت(L, 1);  /* pop versioned variable name ('nver') */
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
ایستا تهی *checkclib (lua_State *L, ثابت نویسه *path) {
  تهی *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_برداشت(L, 2);  /* pop CLIBS table and 'plib' */
  بازگشت plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
ایستا تهی *freelib (تهی *ud, تهی *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (تهی)ptr; (تهی)osize; (تهی)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  بازگشت پوچ;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
ایستا تهی createlibstr (lua_State *L, تهی *plib) {
  /* common content for all library strings */
  ایستا ثابت نویسه dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, اندازه(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
ایستا تهی addtoclib (lua_State *L, ثابت نویسه *path, تهی *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_برداشت(L, 1);  /* pop CLIBS table */
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
ایستا صحیح lookforfunc (lua_State *L, ثابت نویسه *path, ثابت نویسه *sym) {
  تهی *reg = checkclib(L, path);  /* check loaded C libraries */
  اگر (reg == پوچ) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    اگر (reg == پوچ) بازگشت ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  اگر (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    بازگشت 0;  /* no errors */
  }
  وگرنه {
    lua_CFunction f = lsys_sym(L, reg, sym);
    اگر (f == پوچ)
      بازگشت ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    بازگشت 0;  /* no errors */
  }
}


ایستا صحیح ll_loadlib (lua_State *L) {
  ثابت نویسه *path = luaL_checkstring(L, 1);
  ثابت نویسه *init = luaL_checkstring(L, 2);
  صحیح stat = lookforfunc(L, path, init);
  اگر (l_likely(stat == 0))  /* no errors? */
    بازگشت 1;  /* return the loaded function */
  وگرنه {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_درج(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    بازگشت 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


ایستا صحیح readable (ثابت نویسه *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  اگر (f == پوچ) بازگشت 0;  /* open failed */
  fclose(f);
  بازگشت 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
ایستا ثابت نویسه *getnextfilename (نویسه **path, نویسه *end) {
  نویسه *sep;
  نویسه *name = *path;
  اگر (name == end)
    بازگشت پوچ;  /* no more names */
  وگرنه اگر (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  اگر (sep == پوچ)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  بازگشت name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
ایستا تهی pusherrornotfound (lua_State *L, ثابت نویسه *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


ایستا ثابت نویسه *searchpath (lua_State *L, ثابت نویسه *name,
                                             ثابت نویسه *path,
                                             ثابت نویسه *sep,
                                             ثابت نویسه *dirsep) {
  luaL_Buffer buff;
  نویسه *pathname;  /* path with name inserted */
  نویسه *endpathname;  /* its end */
  ثابت نویسه *filename;
  /* separator is non-empty and appears in 'name'? */
  اگر (*sep != '\0' && strchr(name, *sep) != پوچ)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  وقتی ((filename = getnextfilename(&pathname, endpathname)) != پوچ) {
    اگر (readable(filename))  /* does file exist and is readable? */
      بازگشت lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  بازگشت پوچ;  /* not found */
}


ایستا صحیح ll_searchpath (lua_State *L) {
  ثابت نویسه *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  اگر (f != پوچ) بازگشت 1;
  وگرنه {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_درج(L, -2);
    بازگشت 2;  /* return fail + error message */
  }
}


ایستا ثابت نویسه *findfile (lua_State *L, ثابت نویسه *name,
                                           ثابت نویسه *pname,
                                           ثابت نویسه *dirsep) {
  ثابت نویسه *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  اگر (l_unlikely(path == پوچ))
    luaL_error(L, "'package.%s' must be a string", pname);
  بازگشت searchpath(L, name, path, ".", dirsep);
}


ایستا صحیح checkload (lua_State *L, صحیح stat, ثابت نویسه *filename) {
  اگر (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    بازگشت 2;  /* return open function and file name */
  }
  وگرنه
    بازگشت luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


ایستا صحیح searcher_Lua (lua_State *L) {
  ثابت نویسه *filename;
  ثابت نویسه *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  اگر (filename == پوچ) بازگشت 1;  /* module not found in this path */
  بازگشت checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
ایستا صحیح loadfunc (lua_State *L, ثابت نویسه *filename, ثابت نویسه *modname) {
  ثابت نویسه *openfunc;
  ثابت نویسه *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  اگر (mark) {
    صحیح stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    اگر (stat != ERRFUNC) بازگشت stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  بازگشت lookforfunc(L, filename, openfunc);
}


ایستا صحیح searcher_C (lua_State *L) {
  ثابت نویسه *name = luaL_checkstring(L, 1);
  ثابت نویسه *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  اگر (filename == پوچ) بازگشت 1;  /* module not found in this path */
  بازگشت checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


ایستا صحیح searcher_Croot (lua_State *L) {
  ثابت نویسه *filename;
  ثابت نویسه *name = luaL_checkstring(L, 1);
  ثابت نویسه *p = strchr(name, '.');
  صحیح stat;
  اگر (p == پوچ) بازگشت 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  اگر (filename == پوچ) بازگشت 1;  /* root not found */
  اگر ((stat = loadfunc(L, filename, name)) != 0) {
    اگر (stat != ERRFUNC)
      بازگشت checkload(L, 0, filename);  /* real error */
    وگرنه {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      بازگشت 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  بازگشت 2;
}


ایستا صحیح searcher_preload (lua_State *L) {
  ثابت نویسه *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  اگر (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    بازگشت 1;
  }
  وگرنه {
    lua_pushliteral(L, ":preload:");
    بازگشت 2;
  }
}


ایستا تهی findloader (lua_State *L, ثابت نویسه *name) {
  صحیح i;
  luaL_Buffer پیام;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  اگر (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &پیام);
  luaL_addstring(&پیام, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  برای (i = 1; ; i++) {
    اگر (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_برداشت(L, 1);  /* remove nil */
      luaL_buffsub(&پیام, 2);  /* remove last prefix */
      luaL_pushresult(&پیام);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    اگر (lua_isfunction(L, -2))  /* did it find a loader? */
      بازگشت;  /* module loader found */
    وگرنه اگر (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_برداشت(L, 1);  /* remove extra return */
      luaL_addvalue(&پیام);  /* concatenate error message */
      luaL_addstring(&پیام, "\n\t");  /* prefix for next message */
    }
    وگرنه  /* no error message */
      lua_برداشت(L, 2);  /* remove both returns */
  }
}


ایستا صحیح ll_require (lua_State *L) {
  ثابت نویسه *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  اگر (lua_toboolean(L, -1))  /* is it there? */
    بازگشت 1;  /* package is already loaded */
  /* else must load package */
  lua_برداشت(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  اگر (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  وگرنه
    lua_برداشت(L, 1);  /* pop nil */
  اگر (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  بازگشت 2;  /* return module result and loader data */
}

/* }====================================================== */




ایستا ثابت luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", پوچ},
  {"cpath", پوچ},
  {"path", پوچ},
  {"searchers", پوچ},
  {"loaded", پوچ},
  {پوچ, پوچ}
};


ایستا ثابت luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {پوچ, پوچ}
};


ایستا تهی createsearcherstable (lua_State *L) {
  ایستا ثابت lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    پوچ
  };
  صحیح i;
  /* create 'searchers' table */
  lua_createtable(L, اندازه(searchers)/اندازه(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  برای (i=0; searchers[i] != پوچ; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API صحیح luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_برداشت(L, 1);  /* will not use it now */
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
  lua_برداشت(L, 1);  /* pop global table */
  بازگشت 1;  /* return 'package' table */
}

