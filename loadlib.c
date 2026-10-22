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

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#ଯଦି !defined(LUA_CSUBSEP)
#define LUA_CSUBSEP		LUA_DIRSEP
#endif

#ଯଦି !defined(LUA_LSUBSEP)
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
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((ଶୂନ୍ୟ)0)


/* cast void* to a Lua function */
#define cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
ସ୍ଥିର ଶୂନ୍ୟ lsys_unloadlib (ଶୂନ୍ୟ *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
ସ୍ଥିର ଶୂନ୍ୟ *lsys_load (lua_State *L, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
ସ୍ଥିର lua_CFunction lsys_sym (lua_State *L, ଶୂନ୍ୟ *lib, ଅଚଳ ଅକ୍ଷର *sym);




#ଯଦି defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#ଅନ୍ତର୍ଭାଵ <dlfcn.h>


ସ୍ଥିର ଶୂନ୍ୟ lsys_unloadlib (ଶୂନ୍ୟ *lib) {
  dlclose(lib);
}


ସ୍ଥିର ଶୂନ୍ୟ *lsys_load (lua_State *L, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା seeglb) {
  ଶୂନ୍ୟ *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  ଯଦି (l_unlikely(lib == ଖାଲି))
    lua_pushstring(L, dlerror());
  ଫେରନ୍ତୁ lib;
}


ସ୍ଥିର lua_CFunction lsys_sym (lua_State *L, ଶୂନ୍ୟ *lib, ଅଚଳ ଅକ୍ଷର *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  ଯଦି (l_unlikely(f == ଖାଲି))
    lua_pushstring(L, dlerror());
  ଫେରନ୍ତୁ f;
}

/* }====================================================== */



#ନଚେତଯଦି defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#ଅନ୍ତର୍ଭାଵ <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#ଯଦି !defined(LUA_LLE_FLAGS)
#define LUA_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
ସ୍ଥିର ଶୂନ୍ୟ setprogdir (lua_State *L) {
  ଅକ୍ଷର buff[MAX_PATH + 1];
  ଅକ୍ଷର *lb;
  DWORD nsize = ଆକାର(buff)/ଆକାର(ଅକ୍ଷର);
  DWORD n = GetModuleFileNameA(ଖାଲି, buff, nsize);  /* get exec. name */
  ଯଦି (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == ଖାଲି)
    luaL_error(L, "unable to get ModuleFileName");
  ଅନ୍ୟଥା {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




ସ୍ଥିର ଶୂନ୍ୟ pusherror (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା error = GetLastError();
  ଅକ୍ଷର buffer[128];
  ଯଦି (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      ଖାଲି, error, 0, buffer, ଆକାର(buffer)/ଆକାର(ଅକ୍ଷର), ଖାଲି))
    lua_pushstring(L, buffer);
  ଅନ୍ୟଥା
    lua_pushfstring(L, "system error %d\n", error);
}

ସ୍ଥିର ଶୂନ୍ୟ lsys_unloadlib (ଶୂନ୍ୟ *lib) {
  FreeLibrary((HMODULE)lib);
}


ସ୍ଥିର ଶୂନ୍ୟ *lsys_load (lua_State *L, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା seeglb) {
  HMODULE lib = LoadLibraryExA(path, ଖାଲି, LUA_LLE_FLAGS);
  (ଶୂନ୍ୟ)(seeglb);  /* not used: symbols are 'global' by default */
  ଯଦି (lib == ଖାଲି) pusherror(L);
  ଫେରନ୍ତୁ lib;
}


ସ୍ଥିର lua_CFunction lsys_sym (lua_State *L, ଶୂନ୍ୟ *lib, ଅଚଳ ଅକ୍ଷର *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  ଯଦି (f == ଖାଲି) pusherror(L);
  ଫେରନ୍ତୁ f;
}

/* }====================================================== */


#ଅନ୍ୟଥା				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your Lua installation"


ସ୍ଥିର ଶୂନ୍ୟ lsys_unloadlib (ଶୂନ୍ୟ *lib) {
  (ଶୂନ୍ୟ)(lib);  /* not used */
}


ସ୍ଥିର ଶୂନ୍ୟ *lsys_load (lua_State *L, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା seeglb) {
  (ଶୂନ୍ୟ)(path); (ଶୂନ୍ୟ)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  ଫେରନ୍ତୁ ଖାଲି;
}


ସ୍ଥିର lua_CFunction lsys_sym (lua_State *L, ଶୂନ୍ୟ *lib, ଅଚଳ ଅକ୍ଷର *sym) {
  (ଶୂନ୍ୟ)(lib); (ଶୂନ୍ୟ)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  ଫେରନ୍ତୁ ଖାଲି;
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
#ଯଦି !defined(LUA_PATH_VAR)
#define LUA_PATH_VAR    "LUA_PATH"
#endif

#ଯଦି !defined(LUA_CPATH_VAR)
#define LUA_CPATH_VAR   "LUA_CPATH"
#endif



/*
** return registry.LUA_NOENV as a boolean
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା noenv (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_କାଢ଼(L, 1);  /* remove value */
  ଫେରନ୍ତୁ b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
ସ୍ଥିର ଶୂନ୍ୟ setpath (lua_State *L, ଅଚଳ ଅକ୍ଷର *fieldname,
                                   ଅଚଳ ଅକ୍ଷର *envname,
                                   ଅଚଳ ଅକ୍ଷର *dft) {
  ଅଚଳ ଅକ୍ଷର *dftmark;
  ଅଚଳ ଅକ୍ଷର *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ଅଚଳ ଅକ୍ଷର *path = getenv(nver);  /* try versioned name */
  ଯଦି (path == ଖାଲି)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  ଯଦି (path == ଖାଲି || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, strlen(dft), ଖାଲି, ଖାଲି);  /* use default */
  ଅନ୍ୟଥା ଯଦି ((dftmark = strstr(path, LUA_PATH_SEP LUA_PATH_SEP)) == ଖାଲି)
    lua_pushstring(L, path);  /* nothing to change */
  ଅନ୍ୟଥା {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    ଯଦି (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    ଯଦି (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_କାଢ଼(L, 1);  /* pop versioned variable name ('nver') */
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
ସ୍ଥିର ଶୂନ୍ୟ *checkclib (lua_State *L, ଅଚଳ ଅକ୍ଷର *path) {
  ଶୂନ୍ୟ *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_କାଢ଼(L, 2);  /* pop CLIBS table and 'plib' */
  ଫେରନ୍ତୁ plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
ସ୍ଥିର ଶୂନ୍ୟ *freelib (ଶୂନ୍ୟ *ud, ଶୂନ୍ୟ *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (ଶୂନ୍ୟ)ptr; (ଶୂନ୍ୟ)osize; (ଶୂନ୍ୟ)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  ଫେରନ୍ତୁ ଖାଲି;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
ସ୍ଥିର ଶୂନ୍ୟ createlibstr (lua_State *L, ଶୂନ୍ୟ *plib) {
  /* common content for all library strings */
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, ଆକାର(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
ସ୍ଥିର ଶୂନ୍ୟ addtoclib (lua_State *L, ଅଚଳ ଅକ୍ଷର *path, ଶୂନ୍ୟ *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_କାଢ଼(L, 1);  /* pop CLIBS table */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା lookforfunc (lua_State *L, ଅଚଳ ଅକ୍ଷର *path, ଅଚଳ ଅକ୍ଷର *sym) {
  ଶୂନ୍ୟ *reg = checkclib(L, path);  /* check loaded C libraries */
  ଯଦି (reg == ଖାଲି) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    ଯଦି (reg == ଖାଲି) ଫେରନ୍ତୁ ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  ଯଦି (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    ଫେରନ୍ତୁ 0;  /* no errors */
  }
  ଅନ୍ୟଥା {
    lua_CFunction f = lsys_sym(L, reg, sym);
    ଯଦି (f == ଖାଲି)
      ଫେରନ୍ତୁ ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    ଫେରନ୍ତୁ 0;  /* no errors */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ll_loadlib (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *path = luaL_checkstring(L, 1);
  ଅଚଳ ଅକ୍ଷର *init = luaL_checkstring(L, 2);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat = lookforfunc(L, path, init);
  ଯଦି (l_likely(stat == 0))  /* no errors? */
    ଫେରନ୍ତୁ 1;  /* return the loaded function */
  ଅନ୍ୟଥା {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_ଭର୍ତ୍ତି(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    ଫେରନ୍ତୁ 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା readable (ଅଚଳ ଅକ୍ଷର *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  ଯଦି (f == ଖାଲି) ଫେରନ୍ତୁ 0;  /* open failed */
  fclose(f);
  ଫେରନ୍ତୁ 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getnextfilename (ଅକ୍ଷର **path, ଅକ୍ଷର *end) {
  ଅକ୍ଷର *sep;
  ଅକ୍ଷର *name = *path;
  ଯଦି (name == end)
    ଫେରନ୍ତୁ ଖାଲି;  /* no more names */
  ଅନ୍ୟଥା ଯଦି (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_PATH_SEP);  /* find next separator */
  ଯଦି (sep == ଖାଲି)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  ଫେରନ୍ତୁ name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
ସ୍ଥିର ଶୂନ୍ୟ pusherrornotfound (lua_State *L, ଅଚଳ ଅକ୍ଷର *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *searchpath (lua_State *L, ଅଚଳ ଅକ୍ଷର *name,
                                             ଅଚଳ ଅକ୍ଷର *path,
                                             ଅଚଳ ଅକ୍ଷର *sep,
                                             ଅଚଳ ଅକ୍ଷର *dirsep) {
  luaL_Buffer buff;
  ଅକ୍ଷର *pathname;  /* path with name inserted */
  ଅକ୍ଷର *endpathname;  /* its end */
  ଅଚଳ ଅକ୍ଷର *filename;
  /* separator is non-empty and appears in 'name'? */
  ଯଦି (*sep != '\0' && strchr(name, *sep) != ଖାଲି)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  ଯେତେବେଳେ ((filename = getnextfilename(&pathname, endpathname)) != ଖାଲି) {
    ଯଦି (readable(filename))  /* does file exist and is readable? */
      ଫେରନ୍ତୁ lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  ଫେରନ୍ତୁ ଖାଲି;  /* not found */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ll_searchpath (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  ଯଦି (f != ଖାଲି) ଫେରନ୍ତୁ 1;
  ଅନ୍ୟଥା {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_ଭର୍ତ୍ତି(L, -2);
    ଫେରନ୍ତୁ 2;  /* return fail + error message */
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *findfile (lua_State *L, ଅଚଳ ଅକ୍ଷର *name,
                                           ଅଚଳ ଅକ୍ଷର *pname,
                                           ଅଚଳ ଅକ୍ଷର *dirsep) {
  ଅଚଳ ଅକ୍ଷର *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  ଯଦି (l_unlikely(path == ଖାଲି))
    luaL_error(L, "'package.%s' must be a string", pname);
  ଫେରନ୍ତୁ searchpath(L, name, path, ".", dirsep);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkload (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat, ଅଚଳ ଅକ୍ଷର *filename) {
  ଯଦି (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    ଫେରନ୍ତୁ 2;  /* return open function and file name */
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା searcher_Lua (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *filename;
  ଅଚଳ ଅକ୍ଷର *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  ଯଦି (filename == ଖାଲି) ଫେରନ୍ତୁ 1;  /* module not found in this path */
  ଫେରନ୍ତୁ checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loadfunc (lua_State *L, ଅଚଳ ଅକ୍ଷର *filename, ଅଚଳ ଅକ୍ଷର *modname) {
  ଅଚଳ ଅକ୍ଷର *openfunc;
  ଅଚଳ ଅକ୍ଷର *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  ଯଦି (mark) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    ଯଦି (stat != ERRFUNC) ଫେରନ୍ତୁ stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  ଫେରନ୍ତୁ lookforfunc(L, filename, openfunc);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା searcher_C (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *name = luaL_checkstring(L, 1);
  ଅଚଳ ଅକ୍ଷର *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  ଯଦି (filename == ଖାଲି) ଫେରନ୍ତୁ 1;  /* module not found in this path */
  ଫେରନ୍ତୁ checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା searcher_Croot (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *filename;
  ଅଚଳ ଅକ୍ଷର *name = luaL_checkstring(L, 1);
  ଅଚଳ ଅକ୍ଷର *p = strchr(name, '.');
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat;
  ଯଦି (p == ଖାଲି) ଫେରନ୍ତୁ 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  ଯଦି (filename == ଖାଲି) ଫେରନ୍ତୁ 1;  /* root not found */
  ଯଦି ((stat = loadfunc(L, filename, name)) != 0) {
    ଯଦି (stat != ERRFUNC)
      ଫେରନ୍ତୁ checkload(L, 0, filename);  /* real error */
    ଅନ୍ୟଥା {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      ଫେରନ୍ତୁ 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  ଫେରନ୍ତୁ 2;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା searcher_preload (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  ଯଦି (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା {
    lua_pushliteral(L, ":preload:");
    ଫେରନ୍ତୁ 2;
  }
}


ସ୍ଥିର ଶୂନ୍ୟ findloader (lua_State *L, ଅଚଳ ଅକ୍ଷର *name) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  luaL_Buffer ସନ୍ଦେଶ;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  ଯଦି (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &ସନ୍ଦେଶ);
  luaL_addstring(&ସନ୍ଦେଶ, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  ପାଇଁ (i = 1; ; i++) {
    ଯଦି (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_କାଢ଼(L, 1);  /* remove nil */
      luaL_buffsub(&ସନ୍ଦେଶ, 2);  /* remove last prefix */
      luaL_pushresult(&ସନ୍ଦେଶ);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    ଯଦି (lua_isfunction(L, -2))  /* did it find a loader? */
      ଫେରନ୍ତୁ;  /* module loader found */
    ଅନ୍ୟଥା ଯଦି (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_କାଢ଼(L, 1);  /* remove extra return */
      luaL_addvalue(&ସନ୍ଦେଶ);  /* concatenate error message */
      luaL_addstring(&ସନ୍ଦେଶ, "\n\t");  /* prefix for next message */
    }
    ଅନ୍ୟଥା  /* no error message */
      lua_କାଢ଼(L, 2);  /* remove both returns */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ll_require (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  ଯଦି (lua_toboolean(L, -1))  /* is it there? */
    ଫେରନ୍ତୁ 1;  /* package is already loaded */
  /* else must load package */
  lua_କାଢ଼(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  ଯଦି (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  ଅନ୍ୟଥା
    lua_କାଢ଼(L, 1);  /* pop nil */
  ଯଦି (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  ଫେରନ୍ତୁ 2;  /* return module result and loader data */
}

/* }====================================================== */




ସ୍ଥିର ଅଚଳ luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", ଖାଲି},
  {"cpath", ଖାଲି},
  {"path", ଖାଲି},
  {"searchers", ଖାଲି},
  {"loaded", ଖାଲି},
  {ଖାଲି, ଖାଲି}
};


ସ୍ଥିର ଅଚଳ luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {ଖାଲି, ଖାଲି}
};


ସ୍ଥିର ଶୂନ୍ୟ createsearcherstable (lua_State *L) {
  ସ୍ଥିର ଅଚଳ lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    ଖାଲି
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  /* create 'searchers' table */
  lua_createtable(L, ଆକାର(searchers)/ଆକାର(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  ପାଇଁ (i=0; searchers[i] != ଖାଲି; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_କାଢ଼(L, 1);  /* will not use it now */
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
  lua_କାଢ଼(L, 1);  /* pop global table */
  ଫେରନ୍ତୁ 1;  /* return 'package' table */
}

