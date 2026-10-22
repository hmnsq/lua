/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#నిర్వచించు loadlib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#ఒకవేళ !defined(LUA_CSUBSEP)
#నిర్వచించు LUA_CSUBSEP		LUA_DIRSEP
#అయితే_ముగింపు

#ఒకవేళ !defined(LUA_LSUBSEP)
#నిర్వచించు LUA_LSUBSEP		LUA_DIRSEP
#అయితే_ముగింపు


/* prefix for open functions in C libraries */
#నిర్వచించు LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#నిర్వచించు LUA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి CLIBS = "_CLIBS";

#నిర్వచించు LIB_FAIL	"open"


#నిర్వచించు setprogdir(L)           ((శూన్య)0)


/* cast void* to a Lua function */
#నిర్వచించు cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
స్థిర శూన్య lsys_unloadlib (శూన్య *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
స్థిర శూన్య *lsys_ఎక్కించు (lua_State *L, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
స్థిర lua_CFunction lsys_sym (lua_State *L, శూన్య *lib, స్థిరరాశి అక్షరం *sym);




#ఒకవేళ defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#అన్తర్భావ <dlfcn.h>


స్థిర శూన్య lsys_unloadlib (శూన్య *lib) {
  dlclose(lib);
}


స్థిర శూన్య *lsys_ఎక్కించు (lua_State *L, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం seeglb) {
  శూన్య *lib = dlopen(మార్గం, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  ఒకవేళ (l_unlikely(lib == ఖాళీ))
    lua_pushstring(L, dlerror());
  తిరిగి lib;
}


స్థిర lua_CFunction lsys_sym (lua_State *L, శూన్య *lib, స్థిరరాశి అక్షరం *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  ఒకవేళ (l_unlikely(f == ఖాళీ))
    lua_pushstring(L, dlerror());
  తిరిగి f;
}

/* }====================================================== */



#లేదావేళ defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#అన్తర్భావ <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#ఒకవేళ !defined(LUA_LLE_FLAGS)
#నిర్వచించు LUA_LLE_FLAGS	0
#అయితే_ముగింపు


#నిర్వచనం_రద్దు setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
స్థిర శూన్య setprogdir (lua_State *L) {
  అక్షరం buff[MAX_మార్గం + 1];
  అక్షరం *lb;
  DWORD nsize = పరిమాణం(buff)/పరిమాణం(అక్షరం);
  DWORD n = GetModuleFileNameA(ఖాళీ, buff, nsize);  /* get exec. name */
  ఒకవేళ (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == ఖాళీ)
    luaL_error(L, "unable to get ModuleFileName");
  లేకపోతే {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




స్థిర శూన్య pusherror (lua_State *L) {
  పూర్ణాంకం error = Get_చివరి_Error();
  అక్షరం బఫరు[128];
  ఒకవేళ (Format_సందేశం_A(FORMAT_సందేశం_IGNORE_INSERTS | FORMAT_సందేశం_FROM_SYSTEM,
      ఖాళీ, error, 0, బఫరు, పరిమాణం(బఫరు)/పరిమాణం(అక్షరం), ఖాళీ))
    lua_pushstring(L, బఫరు);
  లేకపోతే
    lua_pushfstring(L, "system error %d\n", error);
}

స్థిర శూన్య lsys_unloadlib (శూన్య *lib) {
  FreeLibrary((HMODULE)lib);
}


స్థిర శూన్య *lsys_ఎక్కించు (lua_State *L, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం seeglb) {
  HMODULE lib = ఎక్కించు_Library_Ex_A(మార్గం, ఖాళీ, LUA_LLE_FLAGS);
  (శూన్య)(seeglb);  /* not used: symbols are 'global' by default */
  ఒకవేళ (lib == ఖాళీ) pusherror(L);
  తిరిగి lib;
}


స్థిర lua_CFunction lsys_sym (lua_State *L, శూన్య *lib, స్థిరరాశి అక్షరం *sym) {
  lua_CFunction f = cast_Lfunc(Get_Proc_చిరునామా((HMODULE)lib, sym));
  ఒకవేళ (f == ఖాళీ) pusherror(L);
  తిరిగి f;
}

/* }====================================================== */


#లేకపోతే				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#నిర్వచనం_రద్దు LIB_FAIL
#నిర్వచించు LIB_FAIL	"absent"


#నిర్వచించు DLMSG	"dynamic libraries not enabled; check your Lua installation"


స్థిర శూన్య lsys_unloadlib (శూన్య *lib) {
  (శూన్య)(lib);  /* not used */
}


స్థిర శూన్య *lsys_ఎక్కించు (lua_State *L, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం seeglb) {
  (శూన్య)(మార్గం); (శూన్య)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  తిరిగి ఖాళీ;
}


స్థిర lua_CFunction lsys_sym (lua_State *L, శూన్య *lib, స్థిరరాశి అక్షరం *sym) {
  (శూన్య)(lib); (శూన్య)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  తిరిగి ఖాళీ;
}

/* }====================================================== */
#అయితే_ముగింపు				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#ఒకవేళ !defined(LUA_మార్గం_VAR)
#నిర్వచించు LUA_మార్గం_VAR    "LUA_PATH"
#అయితే_ముగింపు

#ఒకవేళ !defined(LUA_CPATH_VAR)
#నిర్వచించు LUA_CPATH_VAR   "LUA_CPATH"
#అయితే_ముగింపు



/*
** return registry.LUA_NOENV as a boolean
*/
స్థిర పూర్ణాంకం noenv (lua_State *L) {
  పూర్ణాంకం b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_తీయి(L, 1);  /* remove value */
  తిరిగి b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
స్థిర శూన్య setpath (lua_State *L, స్థిరరాశి అక్షరం *fieldname,
                                   స్థిరరాశి అక్షరం *envname,
                                   స్థిరరాశి అక్షరం *dft) {
  స్థిరరాశి అక్షరం *dftmark;
  స్థిరరాశి అక్షరం *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  స్థిరరాశి అక్షరం *మార్గం = getenv(nver);  /* try versioned name */
  ఒకవేళ (మార్గం == ఖాళీ)  /* no versioned environment variable? */
    మార్గం = getenv(envname);  /* try unversioned name */
  ఒకవేళ (మార్గం == ఖాళీ || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, అక్షరశ్రేణి_పొడవు(dft), ఖాళీ, ఖాళీ);  /* use default */
  లేకపోతే ఒకవేళ ((dftmark = అక్షరశ్రేణి_శోధన(మార్గం, LUA_మార్గం_SEP LUA_మార్గం_SEP)) == ఖాళీ)
    lua_pushstring(L, మార్గం);  /* nothing to change */
  లేకపోతే {  /* path contains a ";;": insert default path in its place */
    size_t len = అక్షరశ్రేణి_పొడవు(మార్గం);
    lua_L_బఫరు b;
    luaL_buffinit(L, &b);
    ఒకవేళ (మార్గం < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, మార్గం, ct_diff2sz(dftmark - మార్గం));  /* add it */
      luaL_addchar(&b, *LUA_మార్గం_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    ఒకవేళ (dftmark < మార్గం + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_మార్గం_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((మార్గం + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_తీయి(L, 1);  /* pop versioned variable name ('nver') */
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
స్థిర శూన్య *checkclib (lua_State *L, స్థిరరాశి అక్షరం *మార్గం) {
  శూన్య *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, మార్గం);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_తీయి(L, 2);  /* pop CLIBS table and 'plib' */
  తిరిగి plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
స్థిర శూన్య *freelib (శూన్య *ud, శూన్య *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (శూన్య)ptr; (శూన్య)osize; (శూన్య)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  తిరిగి ఖాళీ;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
స్థిర శూన్య createlibstr (lua_State *L, శూన్య *plib) {
  /* common content for all library strings */
  స్థిర స్థిరరాశి అక్షరం dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, పరిమాణం(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
స్థిర శూన్య addtoclib (lua_State *L, స్థిరరాశి అక్షరం *మార్గం, శూన్య *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, మార్గం);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_తీయి(L, 1);  /* pop CLIBS table */
}


/* error codes for 'lookforfunc' */
#నిర్వచించు ERRLIB		1
#నిర్వచించు ERRFUNC		2

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
స్థిర పూర్ణాంకం lookforfunc (lua_State *L, స్థిరరాశి అక్షరం *మార్గం, స్థిరరాశి అక్షరం *sym) {
  శూన్య *reg = checkclib(L, మార్గం);  /* check loaded C libraries */
  ఒకవేళ (reg == ఖాళీ) {  /* must load library? */
    reg = lsys_ఎక్కించు(L, మార్గం, *sym == '*');  /* global symbols if 'sym'=='*' */
    ఒకవేళ (reg == ఖాళీ) తిరిగి ERRLIB;  /* unable to load library */
    addtoclib(L, మార్గం, reg);
  }
  ఒకవేళ (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    తిరిగి 0;  /* no errors */
  }
  లేకపోతే {
    lua_CFunction f = lsys_sym(L, reg, sym);
    ఒకవేళ (f == ఖాళీ)
      తిరిగి ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    తిరిగి 0;  /* no errors */
  }
}


స్థిర పూర్ణాంకం ll_loadlib (lua_State *L) {
  స్థిరరాశి అక్షరం *మార్గం = luaL_checkstring(L, 1);
  స్థిరరాశి అక్షరం *init = luaL_checkstring(L, 2);
  పూర్ణాంకం stat = lookforfunc(L, మార్గం, init);
  ఒకవేళ (l_likely(stat == 0))  /* no errors? */
    తిరిగి 1;  /* return the loaded function */
  లేకపోతే {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_చొప్పించు(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    తిరిగి 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


స్థిర పూర్ణాంకం readable (స్థిరరాశి అక్షరం *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  ఒకవేళ (f == ఖాళీ) తిరిగి 0;  /* open failed */
  fclose(f);
  తిరిగి 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
స్థిర స్థిరరాశి అక్షరం *getnextfilename (అక్షరం **మార్గం, అక్షరం *చివర) {
  అక్షరం *sep;
  అక్షరం *name = *మార్గం;
  ఒకవేళ (name == చివర)
    తిరిగి ఖాళీ;  /* no more names */
  లేకపోతే ఒకవేళ (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_మార్గం_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_మార్గం_SEP);  /* find next separator */
  ఒకవేళ (sep == ఖాళీ)  /* separator not found? */
    sep = చివర;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *మార్గం = sep;  /* will start next search from here */
  తిరిగి name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
స్థిర శూన్య pusherrornotfound (lua_State *L, స్థిరరాశి అక్షరం *మార్గం) {
  lua_L_బఫరు b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, మార్గం, LUA_మార్గం_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


స్థిర స్థిరరాశి అక్షరం *searchpath (lua_State *L, స్థిరరాశి అక్షరం *name,
                                             స్థిరరాశి అక్షరం *మార్గం,
                                             స్థిరరాశి అక్షరం *sep,
                                             స్థిరరాశి అక్షరం *dirsep) {
  lua_L_బఫరు buff;
  అక్షరం *pathname;  /* path with name inserted */
  అక్షరం *endpathname;  /* its end */
  స్థిరరాశి అక్షరం *filename;
  /* separator is non-empty and appears in 'name'? */
  ఒకవేళ (*sep != '\0' && strchr(name, *sep) != ఖాళీ)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, మార్గం, LUA_మార్గం_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  అయితే ((filename = getnextfilename(&pathname, endpathname)) != ఖాళీ) {
    ఒకవేళ (readable(filename))  /* does file exist and is readable? */
      తిరిగి lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  తిరిగి ఖాళీ;  /* not found */
}


స్థిర పూర్ణాంకం ll_searchpath (lua_State *L) {
  స్థిరరాశి అక్షరం *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  ఒకవేళ (f != ఖాళీ) తిరిగి 1;
  లేకపోతే {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_చొప్పించు(L, -2);
    తిరిగి 2;  /* return fail + error message */
  }
}


స్థిర స్థిరరాశి అక్షరం *findfile (lua_State *L, స్థిరరాశి అక్షరం *name,
                                           స్థిరరాశి అక్షరం *pname,
                                           స్థిరరాశి అక్షరం *dirsep) {
  స్థిరరాశి అక్షరం *మార్గం;
  lua_getfield(L, lua_upvalueindex(1), pname);
  మార్గం = lua_tostring(L, -1);
  ఒకవేళ (l_unlikely(మార్గం == ఖాళీ))
    luaL_error(L, "'package.%s' must be a string", pname);
  తిరిగి searchpath(L, name, మార్గం, ".", dirsep);
}


స్థిర పూర్ణాంకం checkload (lua_State *L, పూర్ణాంకం stat, స్థిరరాశి అక్షరం *filename) {
  ఒకవేళ (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    తిరిగి 2;  /* return open function and file name */
  }
  లేకపోతే
    తిరిగి luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


స్థిర పూర్ణాంకం searcher_Lua (lua_State *L) {
  స్థిరరాశి అక్షరం *filename;
  స్థిరరాశి అక్షరం *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  ఒకవేళ (filename == ఖాళీ) తిరిగి 1;  /* module not found in this path */
  తిరిగి checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
స్థిర పూర్ణాంకం loadfunc (lua_State *L, స్థిరరాశి అక్షరం *filename, స్థిరరాశి అక్షరం *modname) {
  స్థిరరాశి అక్షరం *openfunc;
  స్థిరరాశి అక్షరం *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  ఒకవేళ (mark) {
    పూర్ణాంకం stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    ఒకవేళ (stat != ERRFUNC) తిరిగి stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  తిరిగి lookforfunc(L, filename, openfunc);
}


స్థిర పూర్ణాంకం searcher_C (lua_State *L) {
  స్థిరరాశి అక్షరం *name = luaL_checkstring(L, 1);
  స్థిరరాశి అక్షరం *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  ఒకవేళ (filename == ఖాళీ) తిరిగి 1;  /* module not found in this path */
  తిరిగి checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


స్థిర పూర్ణాంకం searcher_Croot (lua_State *L) {
  స్థిరరాశి అక్షరం *filename;
  స్థిరరాశి అక్షరం *name = luaL_checkstring(L, 1);
  స్థిరరాశి అక్షరం *p = strchr(name, '.');
  పూర్ణాంకం stat;
  ఒకవేళ (p == ఖాళీ) తిరిగి 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  ఒకవేళ (filename == ఖాళీ) తిరిగి 1;  /* root not found */
  ఒకవేళ ((stat = loadfunc(L, filename, name)) != 0) {
    ఒకవేళ (stat != ERRFUNC)
      తిరిగి checkload(L, 0, filename);  /* real error */
    లేకపోతే {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      తిరిగి 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  తిరిగి 2;
}


స్థిర పూర్ణాంకం searcher_preload (lua_State *L) {
  స్థిరరాశి అక్షరం *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_పట్టిక);
  ఒకవేళ (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    తిరిగి 1;
  }
  లేకపోతే {
    lua_pushliteral(L, ":preload:");
    తిరిగి 2;
  }
}


స్థిర శూన్య findloader (lua_State *L, స్థిరరాశి అక్షరం *name) {
  పూర్ణాంకం i;
  lua_L_బఫరు సందేశం;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  ఒకవేళ (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &సందేశం);
  luaL_addstring(&సందేశం, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  కొరకు (i = 1; ; i++) {
    ఒకవేళ (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_తీయి(L, 1);  /* remove nil */
      luaL_buffsub(&సందేశం, 2);  /* remove last prefix */
      luaL_pushresult(&సందేశం);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    ఒకవేళ (lua_isfunction(L, -2))  /* did it find a loader? */
      తిరిగి;  /* module loader found */
    లేకపోతే ఒకవేళ (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_తీయి(L, 1);  /* remove extra return */
      luaL_addvalue(&సందేశం);  /* concatenate error message */
      luaL_addstring(&సందేశం, "\n\t");  /* prefix for next message */
    }
    లేకపోతే  /* no error message */
      lua_తీయి(L, 2);  /* remove both returns */
  }
}


స్థిర పూర్ణాంకం ll_require (lua_State *L) {
  స్థిరరాశి అక్షరం *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_పట్టిక);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  ఒకవేళ (lua_toboolean(L, -1))  /* is it there? */
    తిరిగి 1;  /* package is already loaded */
  /* else must load package */
  lua_తీయి(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  ఒకవేళ (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  లేకపోతే
    lua_తీయి(L, 1);  /* pop nil */
  ఒకవేళ (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  తిరిగి 2;  /* return module result and loader data */
}

/* }====================================================== */




స్థిర స్థిరరాశి luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", ఖాళీ},
  {"cpath", ఖాళీ},
  {"path", ఖాళీ},
  {"searchers", ఖాళీ},
  {"loaded", ఖాళీ},
  {ఖాళీ, ఖాళీ}
};


స్థిర స్థిరరాశి luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {ఖాళీ, ఖాళీ}
};


స్థిర శూన్య createsearcherstable (lua_State *L) {
  స్థిర స్థిరరాశి lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    ఖాళీ
  };
  పూర్ణాంకం i;
  /* create 'searchers' table */
  lua_createtable(L, పరిమాణం(searchers)/పరిమాణం(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  కొరకు (i=0; searchers[i] != ఖాళీ; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API పూర్ణాంకం luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_తీయి(L, 1);  /* will not use it now */
  luaL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LUA_మార్గం_VAR, LUA_మార్గం_DEFAULT);
  setpath(L, "cpath", LUA_CPATH_VAR, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_మార్గం_SEP "\n" LUA_మార్గం_MARK "\n"
                     LUA_EXEC_DIR "\n" LUA_IGMARK "\n");
  lua_setfield(L, -2, "config");
  /* set field 'loaded' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_పట్టిక);
  lua_setfield(L, -2, "loaded");
  /* set field 'preload' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_పట్టిక);
  lua_setfield(L, -2, "preload");
  lua_pushglobaltable(L);
  lua_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  luaL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  lua_తీయి(L, 1);  /* pop global table */
  తిరిగి 1;  /* return 'package' table */
}

