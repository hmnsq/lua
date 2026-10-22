/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#घोषणा loadlib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#अगर !defined(LUA_CSUBSEP)
#घोषणा LUA_CSUBSEP		LUA_DIRSEP
#पूर्ण_अगर

#अगर !defined(LUA_LSUBSEP)
#घोषणा LUA_LSUBSEP		LUA_DIRSEP
#पूर्ण_अगर


/* prefix for open functions in C libraries */
#घोषणा LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#घोषणा LUA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
अटल स्थिर अक्षर *स्थिर CLIBS = "_CLIBS";

#घोषणा LIB_FAIL	"open"


#घोषणा setprogdir(L)           ((व्योम)0)


/* cast void* to a Lua function */
#घोषणा cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
अटल व्योम lsys_unloadlib (व्योम *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
अटल व्योम *lsys_load (lua_State *L, स्थिर अक्षर *path, पूर्णांक seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
अटल lua_CFunction lsys_sym (lua_State *L, व्योम *lib, स्थिर अक्षर *sym);




#अगर defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#समावेश <dlfcn.h>


अटल व्योम lsys_unloadlib (व्योम *lib) {
  dlclose(lib);
}


अटल व्योम *lsys_load (lua_State *L, स्थिर अक्षर *path, पूर्णांक seeglb) {
  व्योम *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  अगर (l_unlikely(lib == शून्य))
    lua_pushstring(L, dlerror());
  वापस lib;
}


अटल lua_CFunction lsys_sym (lua_State *L, व्योम *lib, स्थिर अक्षर *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  अगर (l_unlikely(f == शून्य))
    lua_pushstring(L, dlerror());
  वापस f;
}

/* }====================================================== */



#या_अगर defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#समावेश <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#अगर !defined(LUA_LLE_FLAGS)
#घोषणा LUA_LLE_FLAGS	0
#पूर्ण_अगर


#अघोषित setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
अटल व्योम setprogdir (lua_State *L) {
  अक्षर buff[MAX_PATH + 1];
  अक्षर *lb;
  DWORD nsize = माप(buff)/माप(अक्षर);
  DWORD n = GetModuleFileNameA(शून्य, buff, nsize);  /* get exec. name */
  अगर (n == 0 || n == nsize || (lb = म_खोजप(buff, '\\')) == शून्य)
    luaL_error(L, "unable to get ModuleFileName");
  अन्यथा {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




अटल व्योम pusherror (lua_State *L) {
  पूर्णांक error = GetLastError();
  अक्षर बफर[128];
  अगर (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      शून्य, error, 0, बफर, माप(बफर)/माप(अक्षर), शून्य))
    lua_pushstring(L, बफर);
  अन्यथा
    lua_pushfstring(L, "system error %d\n", error);
}

अटल व्योम lsys_unloadlib (व्योम *lib) {
  FreeLibrary((HMODULE)lib);
}


अटल व्योम *lsys_load (lua_State *L, स्थिर अक्षर *path, पूर्णांक seeglb) {
  HMODULE lib = LoadLibraryExA(path, शून्य, LUA_LLE_FLAGS);
  (व्योम)(seeglb);  /* not used: symbols are 'global' by default */
  अगर (lib == शून्य) pusherror(L);
  वापस lib;
}


अटल lua_CFunction lsys_sym (lua_State *L, व्योम *lib, स्थिर अक्षर *sym) {
  lua_CFunction f = cast_Lfunc(GetProcAddress((HMODULE)lib, sym));
  अगर (f == शून्य) pusherror(L);
  वापस f;
}

/* }====================================================== */


#अन्यथा				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#अघोषित LIB_FAIL
#घोषणा LIB_FAIL	"absent"


#घोषणा DLMSG	"dynamic libraries not enabled; check your Lua installation"


अटल व्योम lsys_unloadlib (व्योम *lib) {
  (व्योम)(lib);  /* not used */
}


अटल व्योम *lsys_load (lua_State *L, स्थिर अक्षर *path, पूर्णांक seeglb) {
  (व्योम)(path); (व्योम)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  वापस शून्य;
}


अटल lua_CFunction lsys_sym (lua_State *L, व्योम *lib, स्थिर अक्षर *sym) {
  (व्योम)(lib); (व्योम)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  वापस शून्य;
}

/* }====================================================== */
#पूर्ण_अगर				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#अगर !defined(LUA_PATH_VAR)
#घोषणा LUA_PATH_VAR    "LUA_PATH"
#पूर्ण_अगर

#अगर !defined(LUA_CPATH_VAR)
#घोषणा LUA_CPATH_VAR   "LUA_CPATH"
#पूर्ण_अगर



/*
** return registry.LUA_NOENV as a boolean
*/
अटल पूर्णांक noenv (lua_State *L) {
  पूर्णांक b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_निकालो(L, 1);  /* remove value */
  वापस b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
अटल व्योम setpath (lua_State *L, स्थिर अक्षर *fieldname,
                                   स्थिर अक्षर *envname,
                                   स्थिर अक्षर *dft) {
  स्थिर अक्षर *dftmark;
  स्थिर अक्षर *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  स्थिर अक्षर *path = दो_पर्या(nver);  /* try versioned name */
  अगर (path == शून्य)  /* no versioned environment variable? */
    path = दो_पर्या(envname);  /* try unversioned name */
  अगर (path == शून्य || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, म_माप(dft), शून्य, शून्य);  /* use default */
  अन्यथा अगर ((dftmark = म_माला(path, LUA_PATH_SEP LUA_PATH_SEP)) == शून्य)
    lua_pushstring(L, path);  /* nothing to change */
  अन्यथा {  /* path contains a ";;": insert default path in its place */
    माप_प्रकार len = म_माप(path);
    lua_L_बफर b;
    luaL_buffinit(L, &b);
    अगर (path < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, path, ct_diff2sz(dftmark - path));  /* add it */
      luaL_addchar(&b, *LUA_PATH_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    अगर (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_PATH_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((path + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_निकालो(L, 1);  /* pop versioned variable name ('nver') */
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
अटल व्योम *checkclib (lua_State *L, स्थिर अक्षर *path) {
  व्योम *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_निकालो(L, 2);  /* pop CLIBS table and 'plib' */
  वापस plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
अटल व्योम *freelib (व्योम *ud, व्योम *ptr, माप_प्रकार osize, माप_प्रकार nsize) {
  /* string itself is irrelevant and static */
  (व्योम)ptr; (व्योम)osize; (व्योम)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  वापस शून्य;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
अटल व्योम createlibstr (lua_State *L, व्योम *plib) {
  /* common content for all library strings */
  अटल स्थिर अक्षर dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, माप(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
अटल व्योम addtoclib (lua_State *L, स्थिर अक्षर *path, व्योम *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, path);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_निकालो(L, 1);  /* pop CLIBS table */
}


/* error codes for 'lookforfunc' */
#घोषणा ERRLIB		1
#घोषणा ERRFUNC		2

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
अटल पूर्णांक lookforfunc (lua_State *L, स्थिर अक्षर *path, स्थिर अक्षर *sym) {
  व्योम *reg = checkclib(L, path);  /* check loaded C libraries */
  अगर (reg == शून्य) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    अगर (reg == शून्य) वापस ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  अगर (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    वापस 0;  /* no errors */
  }
  अन्यथा {
    lua_CFunction f = lsys_sym(L, reg, sym);
    अगर (f == शून्य)
      वापस ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    वापस 0;  /* no errors */
  }
}


अटल पूर्णांक ll_loadlib (lua_State *L) {
  स्थिर अक्षर *path = luaL_checkstring(L, 1);
  स्थिर अक्षर *init = luaL_checkstring(L, 2);
  पूर्णांक stat = lookforfunc(L, path, init);
  अगर (l_likely(stat == 0))  /* no errors? */
    वापस 1;  /* return the loaded function */
  अन्यथा {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_जोड़ो(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    वापस 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


अटल पूर्णांक readable (स्थिर अक्षर *filename) {
  खाता *f = ख_खोलो(filename, "r");  /* try to open file */
  अगर (f == शून्य) वापस 0;  /* open failed */
  ख_बंद(f);
  वापस 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
अटल स्थिर अक्षर *getnextfilename (अक्षर **path, अक्षर *end) {
  अक्षर *sep;
  अक्षर *name = *path;
  अगर (name == end)
    वापस शून्य;  /* no more names */
  अन्यथा अगर (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = म_अक्षर(name, *LUA_PATH_SEP);  /* find next separator */
  अगर (sep == शून्य)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  वापस name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
अटल व्योम pusherrornotfound (lua_State *L, स्थिर अक्षर *path) {
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, LUA_PATH_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


अटल स्थिर अक्षर *searchpath (lua_State *L, स्थिर अक्षर *name,
                                             स्थिर अक्षर *path,
                                             स्थिर अक्षर *sep,
                                             स्थिर अक्षर *dirsep) {
  lua_L_बफर buff;
  अक्षर *pathname;  /* path with name inserted */
  अक्षर *endpathname;  /* its end */
  स्थिर अक्षर *filename;
  /* separator is non-empty and appears in 'name'? */
  अगर (*sep != '\0' && म_अक्षर(name, *sep) != शून्य)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, path, LUA_PATH_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  जबतक ((filename = getnextfilename(&pathname, endpathname)) != शून्य) {
    अगर (readable(filename))  /* does file exist and is readable? */
      वापस lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  वापस शून्य;  /* not found */
}


अटल पूर्णांक ll_searchpath (lua_State *L) {
  स्थिर अक्षर *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  अगर (f != शून्य) वापस 1;
  अन्यथा {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_जोड़ो(L, -2);
    वापस 2;  /* return fail + error message */
  }
}


अटल स्थिर अक्षर *findfile (lua_State *L, स्थिर अक्षर *name,
                                           स्थिर अक्षर *pname,
                                           स्थिर अक्षर *dirsep) {
  स्थिर अक्षर *path;
  lua_getfield(L, lua_upvalueindex(1), pname);
  path = lua_tostring(L, -1);
  अगर (l_unlikely(path == शून्य))
    luaL_error(L, "'package.%s' must be a string", pname);
  वापस searchpath(L, name, path, ".", dirsep);
}


अटल पूर्णांक checkload (lua_State *L, पूर्णांक stat, स्थिर अक्षर *filename) {
  अगर (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    वापस 2;  /* return open function and file name */
  }
  अन्यथा
    वापस luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


अटल पूर्णांक searcher_Lua (lua_State *L) {
  स्थिर अक्षर *filename;
  स्थिर अक्षर *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  अगर (filename == शून्य) वापस 1;  /* module not found in this path */
  वापस checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
अटल पूर्णांक loadfunc (lua_State *L, स्थिर अक्षर *filename, स्थिर अक्षर *modname) {
  स्थिर अक्षर *openfunc;
  स्थिर अक्षर *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = म_अक्षर(modname, *LUA_IGMARK);
  अगर (mark) {
    पूर्णांक stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    अगर (stat != ERRFUNC) वापस stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  वापस lookforfunc(L, filename, openfunc);
}


अटल पूर्णांक searcher_C (lua_State *L) {
  स्थिर अक्षर *name = luaL_checkstring(L, 1);
  स्थिर अक्षर *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  अगर (filename == शून्य) वापस 1;  /* module not found in this path */
  वापस checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


अटल पूर्णांक searcher_Croot (lua_State *L) {
  स्थिर अक्षर *filename;
  स्थिर अक्षर *name = luaL_checkstring(L, 1);
  स्थिर अक्षर *p = म_अक्षर(name, '.');
  पूर्णांक stat;
  अगर (p == शून्य) वापस 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  अगर (filename == शून्य) वापस 1;  /* root not found */
  अगर ((stat = loadfunc(L, filename, name)) != 0) {
    अगर (stat != ERRFUNC)
      वापस checkload(L, 0, filename);  /* real error */
    अन्यथा {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      वापस 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  वापस 2;
}


अटल पूर्णांक searcher_preload (lua_State *L) {
  स्थिर अक्षर *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  अगर (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    वापस 1;
  }
  अन्यथा {
    lua_pushliteral(L, ":preload:");
    वापस 2;
  }
}


अटल व्योम findloader (lua_State *L, स्थिर अक्षर *name) {
  पूर्णांक i;
  lua_L_बफर संदेश;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  अगर (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &संदेश);
  luaL_addstring(&संदेश, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  क्रम (i = 1; ; i++) {
    अगर (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_निकालो(L, 1);  /* remove nil */
      luaL_buffsub(&संदेश, 2);  /* remove last prefix */
      luaL_pushresult(&संदेश);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    अगर (lua_isfunction(L, -2))  /* did it find a loader? */
      वापस;  /* module loader found */
    अन्यथा अगर (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_निकालो(L, 1);  /* remove extra return */
      luaL_addvalue(&संदेश);  /* concatenate error message */
      luaL_addstring(&संदेश, "\n\t");  /* prefix for next message */
    }
    अन्यथा  /* no error message */
      lua_निकालो(L, 2);  /* remove both returns */
  }
}


अटल पूर्णांक ll_require (lua_State *L) {
  स्थिर अक्षर *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  अगर (lua_toboolean(L, -1))  /* is it there? */
    वापस 1;  /* package is already loaded */
  /* else must load package */
  lua_निकालो(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  अगर (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  अन्यथा
    lua_निकालो(L, 1);  /* pop nil */
  अगर (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  वापस 2;  /* return module result and loader data */
}

/* }====================================================== */




अटल स्थिर luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", शून्य},
  {"cpath", शून्य},
  {"path", शून्य},
  {"searchers", शून्य},
  {"loaded", शून्य},
  {शून्य, शून्य}
};


अटल स्थिर luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {शून्य, शून्य}
};


अटल व्योम createsearcherstable (lua_State *L) {
  अटल स्थिर lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    शून्य
  };
  पूर्णांक i;
  /* create 'searchers' table */
  lua_createtable(L, माप(searchers)/माप(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  क्रम (i=0; searchers[i] != शून्य; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API पूर्णांक luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_निकालो(L, 1);  /* will not use it now */
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
  lua_निकालो(L, 1);  /* pop global table */
  वापस 1;  /* return 'package' table */
}

