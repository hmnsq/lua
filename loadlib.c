/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#تعریف_کرو loadlib_c
#تعریف_کرو LUA_LIB

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
#تعریف_کرو LUA_CSUBSEP		LUA_DIRSEP
#اگر_ختم

#اگر !defined(LUA_LSUBSEP)
#تعریف_کرو LUA_LSUBSEP		LUA_DIRSEP
#اگر_ختم


/* prefix for open functions in C libraries */
#تعریف_کرو LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#تعریف_کرو LUA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
ساکن ثابت حرف *ثابت CLIBS = "_CLIBS";

#تعریف_کرو LIB_FAIL	"open"


#تعریف_کرو setprogdir(L)           ((خالی)0)


/* cast void* to a Lua function */
#تعریف_کرو cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
ساکن خالی lsys_unloadlib (خالی *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
ساکن خالی *lsys_بھرو (lua_State *L, ثابت حرف *راستہ, صحیح seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
ساکن lua_CFunction lsys_sym (lua_State *L, خالی *lib, ثابت حرف *sym);




#اگر defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#شامل <dlfcn.h>


ساکن خالی lsys_unloadlib (خالی *lib) {
  dlclose(lib);
}


ساکن خالی *lsys_بھرو (lua_State *L, ثابت حرف *راستہ, صحیح seeglb) {
  خالی *lib = dlopen(راستہ, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  اگر (l_unlikely(lib == تہی))
    lua_pushstring(L, dlerror());
  واپس lib;
}


ساکن lua_CFunction lsys_sym (lua_State *L, خالی *lib, ثابت حرف *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  اگر (l_unlikely(f == تہی))
    lua_pushstring(L, dlerror());
  واپس f;
}

/* }====================================================== */



#وگرنہ_اگر defined(LUA_DL_DLL)	/* }{ */
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
#تعریف_کرو LUA_LLE_FLAGS	0
#اگر_ختم


#تعریف_منسوخ setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
ساکن خالی setprogdir (lua_State *L) {
  حرف buff[MAX_راستہ + 1];
  حرف *lb;
  DWORD nsize = حجم(buff)/حجم(حرف);
  DWORD n = GetModuleFileNameA(تہی, buff, nsize);  /* get exec. name */
  اگر (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == تہی)
    luaL_error(L, "unable to get ModuleFileName");
  ورنہ {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




ساکن خالی pusherror (lua_State *L) {
  صحیح error = Get_آخری_Error();
  حرف بفر[128];
  اگر (Format_پیغام_A(FORMAT_پیغام_IGNORE_INSERTS | FORMAT_پیغام_FROM_SYSTEM,
      تہی, error, 0, بفر, حجم(بفر)/حجم(حرف), تہی))
    lua_pushstring(L, بفر);
  ورنہ
    lua_pushfstring(L, "system error %d\n", error);
}

ساکن خالی lsys_unloadlib (خالی *lib) {
  FreeLibrary((HMODULE)lib);
}


ساکن خالی *lsys_بھرو (lua_State *L, ثابت حرف *راستہ, صحیح seeglb) {
  HMODULE lib = بھرو_Library_Ex_A(راستہ, تہی, LUA_LLE_FLAGS);
  (خالی)(seeglb);  /* not used: symbols are 'global' by default */
  اگر (lib == تہی) pusherror(L);
  واپس lib;
}


ساکن lua_CFunction lsys_sym (lua_State *L, خالی *lib, ثابت حرف *sym) {
  lua_CFunction f = cast_Lfunc(Get_Proc_پتہ((HMODULE)lib, sym));
  اگر (f == تہی) pusherror(L);
  واپس f;
}

/* }====================================================== */


#ورنہ				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#تعریف_منسوخ LIB_FAIL
#تعریف_کرو LIB_FAIL	"absent"


#تعریف_کرو DLMSG	"dynamic libraries not enabled; check your Lua installation"


ساکن خالی lsys_unloadlib (خالی *lib) {
  (خالی)(lib);  /* not used */
}


ساکن خالی *lsys_بھرو (lua_State *L, ثابت حرف *راستہ, صحیح seeglb) {
  (خالی)(راستہ); (خالی)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  واپس تہی;
}


ساکن lua_CFunction lsys_sym (lua_State *L, خالی *lib, ثابت حرف *sym) {
  (خالی)(lib); (خالی)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  واپس تہی;
}

/* }====================================================== */
#اگر_ختم				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#اگر !defined(LUA_راستہ_VAR)
#تعریف_کرو LUA_راستہ_VAR    "LUA_PATH"
#اگر_ختم

#اگر !defined(LUA_CPATH_VAR)
#تعریف_کرو LUA_CPATH_VAR   "LUA_CPATH"
#اگر_ختم



/*
** return registry.LUA_NOENV as a boolean
*/
ساکن صحیح noenv (lua_State *L) {
  صحیح b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_نکالو(L, 1);  /* remove value */
  واپس b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
ساکن خالی setpath (lua_State *L, ثابت حرف *fieldname,
                                   ثابت حرف *envname,
                                   ثابت حرف *dft) {
  ثابت حرف *dftmark;
  ثابت حرف *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ثابت حرف *راستہ = getenv(nver);  /* try versioned name */
  اگر (راستہ == تہی)  /* no versioned environment variable? */
    راستہ = getenv(envname);  /* try unversioned name */
  اگر (راستہ == تہی || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, لڑی_لمبائی(dft), تہی, تہی);  /* use default */
  ورنہ اگر ((dftmark = لڑی_تلاش(راستہ, LUA_راستہ_SEP LUA_راستہ_SEP)) == تہی)
    lua_pushstring(L, راستہ);  /* nothing to change */
  ورنہ {  /* path contains a ";;": insert default path in its place */
    size_t len = لڑی_لمبائی(راستہ);
    lua_L_بفر b;
    luaL_buffinit(L, &b);
    اگر (راستہ < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, راستہ, ct_diff2sz(dftmark - راستہ));  /* add it */
      luaL_addchar(&b, *LUA_راستہ_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    اگر (dftmark < راستہ + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_راستہ_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((راستہ + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_نکالو(L, 1);  /* pop versioned variable name ('nver') */
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
ساکن خالی *checkclib (lua_State *L, ثابت حرف *راستہ) {
  خالی *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, راستہ);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_نکالو(L, 2);  /* pop CLIBS table and 'plib' */
  واپس plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
ساکن خالی *freelib (خالی *ud, خالی *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (خالی)ptr; (خالی)osize; (خالی)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  واپس تہی;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
ساکن خالی createlibstr (lua_State *L, خالی *plib) {
  /* common content for all library strings */
  ساکن ثابت حرف dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, حجم(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
ساکن خالی addtoclib (lua_State *L, ثابت حرف *راستہ, خالی *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, راستہ);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_نکالو(L, 1);  /* pop CLIBS table */
}


/* error codes for 'lookforfunc' */
#تعریف_کرو ERRLIB		1
#تعریف_کرو ERRFUNC		2

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
ساکن صحیح lookforfunc (lua_State *L, ثابت حرف *راستہ, ثابت حرف *sym) {
  خالی *reg = checkclib(L, راستہ);  /* check loaded C libraries */
  اگر (reg == تہی) {  /* must load library? */
    reg = lsys_بھرو(L, راستہ, *sym == '*');  /* global symbols if 'sym'=='*' */
    اگر (reg == تہی) واپس ERRLIB;  /* unable to load library */
    addtoclib(L, راستہ, reg);
  }
  اگر (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    واپس 0;  /* no errors */
  }
  ورنہ {
    lua_CFunction f = lsys_sym(L, reg, sym);
    اگر (f == تہی)
      واپس ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    واپس 0;  /* no errors */
  }
}


ساکن صحیح ll_loadlib (lua_State *L) {
  ثابت حرف *راستہ = luaL_checkstring(L, 1);
  ثابت حرف *init = luaL_checkstring(L, 2);
  صحیح stat = lookforfunc(L, راستہ, init);
  اگر (l_likely(stat == 0))  /* no errors? */
    واپس 1;  /* return the loaded function */
  ورنہ {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_داخل_کرو(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    واپس 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


ساکن صحیح readable (ثابت حرف *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  اگر (f == تہی) واپس 0;  /* open failed */
  fclose(f);
  واپس 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
ساکن ثابت حرف *getnextfilename (حرف **راستہ, حرف *انتہا) {
  حرف *sep;
  حرف *name = *راستہ;
  اگر (name == انتہا)
    واپس تہی;  /* no more names */
  ورنہ اگر (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_راستہ_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_راستہ_SEP);  /* find next separator */
  اگر (sep == تہی)  /* separator not found? */
    sep = انتہا;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *راستہ = sep;  /* will start next search from here */
  واپس name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
ساکن خالی pusherrornotfound (lua_State *L, ثابت حرف *راستہ) {
  lua_L_بفر b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, راستہ, LUA_راستہ_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


ساکن ثابت حرف *searchpath (lua_State *L, ثابت حرف *name,
                                             ثابت حرف *راستہ,
                                             ثابت حرف *sep,
                                             ثابت حرف *dirsep) {
  lua_L_بفر buff;
  حرف *pathname;  /* path with name inserted */
  حرف *endpathname;  /* its end */
  ثابت حرف *filename;
  /* separator is non-empty and appears in 'name'? */
  اگر (*sep != '\0' && strchr(name, *sep) != تہی)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, راستہ, LUA_راستہ_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  جب ((filename = getnextfilename(&pathname, endpathname)) != تہی) {
    اگر (readable(filename))  /* does file exist and is readable? */
      واپس lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  واپس تہی;  /* not found */
}


ساکن صحیح ll_searchpath (lua_State *L) {
  ثابت حرف *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  اگر (f != تہی) واپس 1;
  ورنہ {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_داخل_کرو(L, -2);
    واپس 2;  /* return fail + error message */
  }
}


ساکن ثابت حرف *findfile (lua_State *L, ثابت حرف *name,
                                           ثابت حرف *pname,
                                           ثابت حرف *dirsep) {
  ثابت حرف *راستہ;
  lua_getfield(L, lua_upvalueindex(1), pname);
  راستہ = lua_tostring(L, -1);
  اگر (l_unlikely(راستہ == تہی))
    luaL_error(L, "'package.%s' must be a string", pname);
  واپس searchpath(L, name, راستہ, ".", dirsep);
}


ساکن صحیح checkload (lua_State *L, صحیح stat, ثابت حرف *filename) {
  اگر (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    واپس 2;  /* return open function and file name */
  }
  ورنہ
    واپس luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


ساکن صحیح searcher_Lua (lua_State *L) {
  ثابت حرف *filename;
  ثابت حرف *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  اگر (filename == تہی) واپس 1;  /* module not found in this path */
  واپس checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
ساکن صحیح loadfunc (lua_State *L, ثابت حرف *filename, ثابت حرف *modname) {
  ثابت حرف *openfunc;
  ثابت حرف *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  اگر (mark) {
    صحیح stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    اگر (stat != ERRFUNC) واپس stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  واپس lookforfunc(L, filename, openfunc);
}


ساکن صحیح searcher_C (lua_State *L) {
  ثابت حرف *name = luaL_checkstring(L, 1);
  ثابت حرف *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  اگر (filename == تہی) واپس 1;  /* module not found in this path */
  واپس checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


ساکن صحیح searcher_Croot (lua_State *L) {
  ثابت حرف *filename;
  ثابت حرف *name = luaL_checkstring(L, 1);
  ثابت حرف *p = strchr(name, '.');
  صحیح stat;
  اگر (p == تہی) واپس 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  اگر (filename == تہی) واپس 1;  /* root not found */
  اگر ((stat = loadfunc(L, filename, name)) != 0) {
    اگر (stat != ERRFUNC)
      واپس checkload(L, 0, filename);  /* real error */
    ورنہ {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      واپس 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  واپس 2;
}


ساکن صحیح searcher_preload (lua_State *L) {
  ثابت حرف *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_جدول);
  اگر (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    واپس 1;
  }
  ورنہ {
    lua_pushliteral(L, ":preload:");
    واپس 2;
  }
}


ساکن خالی findloader (lua_State *L, ثابت حرف *name) {
  صحیح i;
  lua_L_بفر پیغام;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  اگر (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &پیغام);
  luaL_addstring(&پیغام, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  برائے (i = 1; ; i++) {
    اگر (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_نکالو(L, 1);  /* remove nil */
      luaL_buffsub(&پیغام, 2);  /* remove last prefix */
      luaL_pushresult(&پیغام);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    اگر (lua_isfunction(L, -2))  /* did it find a loader? */
      واپس;  /* module loader found */
    ورنہ اگر (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_نکالو(L, 1);  /* remove extra return */
      luaL_addvalue(&پیغام);  /* concatenate error message */
      luaL_addstring(&پیغام, "\n\t");  /* prefix for next message */
    }
    ورنہ  /* no error message */
      lua_نکالو(L, 2);  /* remove both returns */
  }
}


ساکن صحیح ll_require (lua_State *L) {
  ثابت حرف *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_جدول);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  اگر (lua_toboolean(L, -1))  /* is it there? */
    واپس 1;  /* package is already loaded */
  /* else must load package */
  lua_نکالو(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  اگر (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  ورنہ
    lua_نکالو(L, 1);  /* pop nil */
  اگر (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  واپس 2;  /* return module result and loader data */
}

/* }====================================================== */




ساکن ثابت luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", تہی},
  {"cpath", تہی},
  {"path", تہی},
  {"searchers", تہی},
  {"loaded", تہی},
  {تہی, تہی}
};


ساکن ثابت luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {تہی, تہی}
};


ساکن خالی createsearcherstable (lua_State *L) {
  ساکن ثابت lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    تہی
  };
  صحیح i;
  /* create 'searchers' table */
  lua_createtable(L, حجم(searchers)/حجم(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  برائے (i=0; searchers[i] != تہی; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API صحیح luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_نکالو(L, 1);  /* will not use it now */
  luaL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LUA_راستہ_VAR, LUA_راستہ_DEFAULT);
  setpath(L, "cpath", LUA_CPATH_VAR, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_راستہ_SEP "\n" LUA_راستہ_MARK "\n"
                     LUA_EXEC_DIR "\n" LUA_IGMARK "\n");
  lua_setfield(L, -2, "config");
  /* set field 'loaded' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_جدول);
  lua_setfield(L, -2, "loaded");
  /* set field 'preload' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_جدول);
  lua_setfield(L, -2, "preload");
  lua_pushglobaltable(L);
  lua_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  luaL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  lua_نکالو(L, 1);  /* pop global table */
  واپس 1;  /* return 'package' table */
}

