/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ loadlib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#ਜੇ !defined(LUA_CSUBSEP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CSUBSEP		LUA_DIRSEP
#ਜੇ_ਅੰਤ

#ਜੇ !defined(LUA_LSUBSEP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LSUBSEP		LUA_DIRSEP
#ਜੇ_ਅੰਤ


/* prefix for open functions in C libraries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ CLIBS = "_CLIBS";

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIB_FAIL	"open"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setprogdir(L)           ((ਖਾਲੀ)0)


/* cast void* to a Lua function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
ਸਥਿਰ ਖਾਲੀ lsys_unloadlib (ਖਾਲੀ *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
ਸਥਿਰ ਖਾਲੀ *lsys_ਲੱਦੋ (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
ਸਥਿਰ lua_CFunction lsys_sym (lua_State *L, ਖਾਲੀ *lib, ਅਚਲ ਅੱਖਰ *sym);




#ਜੇ defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>


ਸਥਿਰ ਖਾਲੀ lsys_unloadlib (ਖਾਲੀ *lib) {
  dlclose(lib);
}


ਸਥਿਰ ਖਾਲੀ *lsys_ਲੱਦੋ (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ seeglb) {
  ਖਾਲੀ *lib = dlopen(ਰਸਤਾ, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  ਜੇ (l_unlikely(lib == ਨਿਰਾ))
    lua_pushstring(L, dlerror());
  ਵਾਪਸ lib;
}


ਸਥਿਰ lua_CFunction lsys_sym (lua_State *L, ਖਾਲੀ *lib, ਅਚਲ ਅੱਖਰ *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  ਜੇ (l_unlikely(f == ਨਿਰਾ))
    lua_pushstring(L, dlerror());
  ਵਾਪਸ f;
}

/* }====================================================== */



#ਜਾਂਜੇ defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#ਜੇ !defined(LUA_LLE_FLAGS)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LLE_FLAGS	0
#ਜੇ_ਅੰਤ


#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
ਸਥਿਰ ਖਾਲੀ setprogdir (lua_State *L) {
  ਅੱਖਰ buff[MAX_ਰਸਤਾ + 1];
  ਅੱਖਰ *lb;
  DWORD nsize = ਆਕਾਰ(buff)/ਆਕਾਰ(ਅੱਖਰ);
  DWORD n = GetModuleFileNameA(ਨਿਰਾ, buff, nsize);  /* get exec. name */
  ਜੇ (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == ਨਿਰਾ)
    luaL_error(L, "unable to get ModuleFileName");
  ਨਹੀਂਤਾਂ {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




ਸਥਿਰ ਖਾਲੀ pusherror (lua_State *L) {
  ਪੂਰਨ error = Get_ਆਖਰੀ_Error();
  ਅੱਖਰ ਬਫਰ[128];
  ਜੇ (Format_ਸੁਨੇਹਾ_A(FORMAT_ਸੁਨੇਹਾ_IGNORE_INSERTS | FORMAT_ਸੁਨੇਹਾ_FROM_SYSTEM,
      ਨਿਰਾ, error, 0, ਬਫਰ, ਆਕਾਰ(ਬਫਰ)/ਆਕਾਰ(ਅੱਖਰ), ਨਿਰਾ))
    lua_pushstring(L, ਬਫਰ);
  ਨਹੀਂਤਾਂ
    lua_pushfstring(L, "system error %d\n", error);
}

ਸਥਿਰ ਖਾਲੀ lsys_unloadlib (ਖਾਲੀ *lib) {
  FreeLibrary((HMODULE)lib);
}


ਸਥਿਰ ਖਾਲੀ *lsys_ਲੱਦੋ (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ seeglb) {
  HMODULE lib = ਲੱਦੋ_Library_Ex_A(ਰਸਤਾ, ਨਿਰਾ, LUA_LLE_FLAGS);
  (ਖਾਲੀ)(seeglb);  /* not used: symbols are 'global' by default */
  ਜੇ (lib == ਨਿਰਾ) pusherror(L);
  ਵਾਪਸ lib;
}


ਸਥਿਰ lua_CFunction lsys_sym (lua_State *L, ਖਾਲੀ *lib, ਅਚਲ ਅੱਖਰ *sym) {
  lua_CFunction f = cast_Lfunc(Get_Proc_ਪਤਾ((HMODULE)lib, sym));
  ਜੇ (f == ਨਿਰਾ) pusherror(L);
  ਵਾਪਸ f;
}

/* }====================================================== */


#ਨਹੀਂਤਾਂ				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LIB_FAIL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIB_FAIL	"absent"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DLMSG	"dynamic libraries not enabled; check your Lua installation"


ਸਥਿਰ ਖਾਲੀ lsys_unloadlib (ਖਾਲੀ *lib) {
  (ਖਾਲੀ)(lib);  /* not used */
}


ਸਥਿਰ ਖਾਲੀ *lsys_ਲੱਦੋ (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ seeglb) {
  (ਖਾਲੀ)(ਰਸਤਾ); (ਖਾਲੀ)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  ਵਾਪਸ ਨਿਰਾ;
}


ਸਥਿਰ lua_CFunction lsys_sym (lua_State *L, ਖਾਲੀ *lib, ਅਚਲ ਅੱਖਰ *sym) {
  (ਖਾਲੀ)(lib); (ਖਾਲੀ)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  ਵਾਪਸ ਨਿਰਾ;
}

/* }====================================================== */
#ਜੇ_ਅੰਤ				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#ਜੇ !defined(LUA_ਰਸਤਾ_VAR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_ਰਸਤਾ_VAR    "LUA_PATH"
#ਜੇ_ਅੰਤ

#ਜੇ !defined(LUA_CPATH_VAR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CPATH_VAR   "LUA_CPATH"
#ਜੇ_ਅੰਤ



/*
** return registry.LUA_NOENV as a boolean
*/
ਸਥਿਰ ਪੂਰਨ noenv (lua_State *L) {
  ਪੂਰਨ b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_ਕੱਢੋ(L, 1);  /* remove value */
  ਵਾਪਸ b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
ਸਥਿਰ ਖਾਲੀ setpath (lua_State *L, ਅਚਲ ਅੱਖਰ *fieldname,
                                   ਅਚਲ ਅੱਖਰ *envname,
                                   ਅਚਲ ਅੱਖਰ *dft) {
  ਅਚਲ ਅੱਖਰ *dftmark;
  ਅਚਲ ਅੱਖਰ *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ = getenv(nver);  /* try versioned name */
  ਜੇ (ਰਸਤਾ == ਨਿਰਾ)  /* no versioned environment variable? */
    ਰਸਤਾ = getenv(envname);  /* try unversioned name */
  ਜੇ (ਰਸਤਾ == ਨਿਰਾ || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, ਲੜੀ_ਲੰਬਾਈ(dft), ਨਿਰਾ, ਨਿਰਾ);  /* use default */
  ਨਹੀਂਤਾਂ ਜੇ ((dftmark = ਲੜੀ_ਖੋਜ(ਰਸਤਾ, LUA_ਰਸਤਾ_SEP LUA_ਰਸਤਾ_SEP)) == ਨਿਰਾ)
    lua_pushstring(L, ਰਸਤਾ);  /* nothing to change */
  ਨਹੀਂਤਾਂ {  /* path contains a ";;": insert default path in its place */
    size_t len = ਲੜੀ_ਲੰਬਾਈ(ਰਸਤਾ);
    lua_L_ਬਫਰ b;
    luaL_buffinit(L, &b);
    ਜੇ (ਰਸਤਾ < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, ਰਸਤਾ, ct_diff2sz(dftmark - ਰਸਤਾ));  /* add it */
      luaL_addchar(&b, *LUA_ਰਸਤਾ_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    ਜੇ (dftmark < ਰਸਤਾ + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_ਰਸਤਾ_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((ਰਸਤਾ + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_ਕੱਢੋ(L, 1);  /* pop versioned variable name ('nver') */
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
ਸਥਿਰ ਖਾਲੀ *checkclib (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ) {
  ਖਾਲੀ *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, ਰਸਤਾ);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_ਕੱਢੋ(L, 2);  /* pop CLIBS table and 'plib' */
  ਵਾਪਸ plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
ਸਥਿਰ ਖਾਲੀ *freelib (ਖਾਲੀ *ud, ਖਾਲੀ *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (ਖਾਲੀ)ptr; (ਖਾਲੀ)osize; (ਖਾਲੀ)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  ਵਾਪਸ ਨਿਰਾ;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
ਸਥਿਰ ਖਾਲੀ createlibstr (lua_State *L, ਖਾਲੀ *plib) {
  /* common content for all library strings */
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, ਆਕਾਰ(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
ਸਥਿਰ ਖਾਲੀ addtoclib (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਖਾਲੀ *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, ਰਸਤਾ);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_ਕੱਢੋ(L, 1);  /* pop CLIBS table */
}


/* error codes for 'lookforfunc' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERRLIB		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERRFUNC		2

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
ਸਥਿਰ ਪੂਰਨ lookforfunc (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅਚਲ ਅੱਖਰ *sym) {
  ਖਾਲੀ *reg = checkclib(L, ਰਸਤਾ);  /* check loaded C libraries */
  ਜੇ (reg == ਨਿਰਾ) {  /* must load library? */
    reg = lsys_ਲੱਦੋ(L, ਰਸਤਾ, *sym == '*');  /* global symbols if 'sym'=='*' */
    ਜੇ (reg == ਨਿਰਾ) ਵਾਪਸ ERRLIB;  /* unable to load library */
    addtoclib(L, ਰਸਤਾ, reg);
  }
  ਜੇ (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    ਵਾਪਸ 0;  /* no errors */
  }
  ਨਹੀਂਤਾਂ {
    lua_CFunction f = lsys_sym(L, reg, sym);
    ਜੇ (f == ਨਿਰਾ)
      ਵਾਪਸ ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    ਵਾਪਸ 0;  /* no errors */
  }
}


ਸਥਿਰ ਪੂਰਨ ll_loadlib (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ = luaL_checkstring(L, 1);
  ਅਚਲ ਅੱਖਰ *init = luaL_checkstring(L, 2);
  ਪੂਰਨ stat = lookforfunc(L, ਰਸਤਾ, init);
  ਜੇ (l_likely(stat == 0))  /* no errors? */
    ਵਾਪਸ 1;  /* return the loaded function */
  ਨਹੀਂਤਾਂ {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_ਪਾਓ(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    ਵਾਪਸ 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


ਸਥਿਰ ਪੂਰਨ readable (ਅਚਲ ਅੱਖਰ *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  ਜੇ (f == ਨਿਰਾ) ਵਾਪਸ 0;  /* open failed */
  fclose(f);
  ਵਾਪਸ 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getnextfilename (ਅੱਖਰ **ਰਸਤਾ, ਅੱਖਰ *ਸਿਰਾ) {
  ਅੱਖਰ *sep;
  ਅੱਖਰ *name = *ਰਸਤਾ;
  ਜੇ (name == ਸਿਰਾ)
    ਵਾਪਸ ਨਿਰਾ;  /* no more names */
  ਨਹੀਂਤਾਂ ਜੇ (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_ਰਸਤਾ_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_ਰਸਤਾ_SEP);  /* find next separator */
  ਜੇ (sep == ਨਿਰਾ)  /* separator not found? */
    sep = ਸਿਰਾ;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *ਰਸਤਾ = sep;  /* will start next search from here */
  ਵਾਪਸ name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
ਸਥਿਰ ਖਾਲੀ pusherrornotfound (lua_State *L, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ) {
  lua_L_ਬਫਰ b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, ਰਸਤਾ, LUA_ਰਸਤਾ_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *searchpath (lua_State *L, ਅਚਲ ਅੱਖਰ *name,
                                             ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
                                             ਅਚਲ ਅੱਖਰ *sep,
                                             ਅਚਲ ਅੱਖਰ *dirsep) {
  lua_L_ਬਫਰ buff;
  ਅੱਖਰ *pathname;  /* path with name inserted */
  ਅੱਖਰ *endpathname;  /* its end */
  ਅਚਲ ਅੱਖਰ *filename;
  /* separator is non-empty and appears in 'name'? */
  ਜੇ (*sep != '\0' && strchr(name, *sep) != ਨਿਰਾ)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, ਰਸਤਾ, LUA_ਰਸਤਾ_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  ਜਦੋਂ ((filename = getnextfilename(&pathname, endpathname)) != ਨਿਰਾ) {
    ਜੇ (readable(filename))  /* does file exist and is readable? */
      ਵਾਪਸ lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  ਵਾਪਸ ਨਿਰਾ;  /* not found */
}


ਸਥਿਰ ਪੂਰਨ ll_searchpath (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  ਜੇ (f != ਨਿਰਾ) ਵਾਪਸ 1;
  ਨਹੀਂਤਾਂ {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_ਪਾਓ(L, -2);
    ਵਾਪਸ 2;  /* return fail + error message */
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *findfile (lua_State *L, ਅਚਲ ਅੱਖਰ *name,
                                           ਅਚਲ ਅੱਖਰ *pname,
                                           ਅਚਲ ਅੱਖਰ *dirsep) {
  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ;
  lua_getfield(L, lua_upvalueindex(1), pname);
  ਰਸਤਾ = lua_tostring(L, -1);
  ਜੇ (l_unlikely(ਰਸਤਾ == ਨਿਰਾ))
    luaL_error(L, "'package.%s' must be a string", pname);
  ਵਾਪਸ searchpath(L, name, ਰਸਤਾ, ".", dirsep);
}


ਸਥਿਰ ਪੂਰਨ checkload (lua_State *L, ਪੂਰਨ stat, ਅਚਲ ਅੱਖਰ *filename) {
  ਜੇ (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    ਵਾਪਸ 2;  /* return open function and file name */
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


ਸਥਿਰ ਪੂਰਨ searcher_Lua (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *filename;
  ਅਚਲ ਅੱਖਰ *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  ਜੇ (filename == ਨਿਰਾ) ਵਾਪਸ 1;  /* module not found in this path */
  ਵਾਪਸ checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
ਸਥਿਰ ਪੂਰਨ loadfunc (lua_State *L, ਅਚਲ ਅੱਖਰ *filename, ਅਚਲ ਅੱਖਰ *modname) {
  ਅਚਲ ਅੱਖਰ *openfunc;
  ਅਚਲ ਅੱਖਰ *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  ਜੇ (mark) {
    ਪੂਰਨ stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    ਜੇ (stat != ERRFUNC) ਵਾਪਸ stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  ਵਾਪਸ lookforfunc(L, filename, openfunc);
}


ਸਥਿਰ ਪੂਰਨ searcher_C (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *name = luaL_checkstring(L, 1);
  ਅਚਲ ਅੱਖਰ *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  ਜੇ (filename == ਨਿਰਾ) ਵਾਪਸ 1;  /* module not found in this path */
  ਵਾਪਸ checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


ਸਥਿਰ ਪੂਰਨ searcher_Croot (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *filename;
  ਅਚਲ ਅੱਖਰ *name = luaL_checkstring(L, 1);
  ਅਚਲ ਅੱਖਰ *p = strchr(name, '.');
  ਪੂਰਨ stat;
  ਜੇ (p == ਨਿਰਾ) ਵਾਪਸ 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  ਜੇ (filename == ਨਿਰਾ) ਵਾਪਸ 1;  /* root not found */
  ਜੇ ((stat = loadfunc(L, filename, name)) != 0) {
    ਜੇ (stat != ERRFUNC)
      ਵਾਪਸ checkload(L, 0, filename);  /* real error */
    ਨਹੀਂਤਾਂ {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      ਵਾਪਸ 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  ਵਾਪਸ 2;
}


ਸਥਿਰ ਪੂਰਨ searcher_preload (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_ਸਾਰਣੀ);
  ਜੇ (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ {
    lua_pushliteral(L, ":preload:");
    ਵਾਪਸ 2;
  }
}


ਸਥਿਰ ਖਾਲੀ findloader (lua_State *L, ਅਚਲ ਅੱਖਰ *name) {
  ਪੂਰਨ i;
  lua_L_ਬਫਰ ਸੁਨੇਹਾ;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  ਜੇ (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &ਸੁਨੇਹਾ);
  luaL_addstring(&ਸੁਨੇਹਾ, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  ਲਈ (i = 1; ; i++) {
    ਜੇ (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_ਕੱਢੋ(L, 1);  /* remove nil */
      luaL_buffsub(&ਸੁਨੇਹਾ, 2);  /* remove last prefix */
      luaL_pushresult(&ਸੁਨੇਹਾ);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    ਜੇ (lua_isfunction(L, -2))  /* did it find a loader? */
      ਵਾਪਸ;  /* module loader found */
    ਨਹੀਂਤਾਂ ਜੇ (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_ਕੱਢੋ(L, 1);  /* remove extra return */
      luaL_addvalue(&ਸੁਨੇਹਾ);  /* concatenate error message */
      luaL_addstring(&ਸੁਨੇਹਾ, "\n\t");  /* prefix for next message */
    }
    ਨਹੀਂਤਾਂ  /* no error message */
      lua_ਕੱਢੋ(L, 2);  /* remove both returns */
  }
}


ਸਥਿਰ ਪੂਰਨ ll_require (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_ਸਾਰਣੀ);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  ਜੇ (lua_toboolean(L, -1))  /* is it there? */
    ਵਾਪਸ 1;  /* package is already loaded */
  /* else must load package */
  lua_ਕੱਢੋ(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  ਜੇ (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  ਨਹੀਂਤਾਂ
    lua_ਕੱਢੋ(L, 1);  /* pop nil */
  ਜੇ (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  ਵਾਪਸ 2;  /* return module result and loader data */
}

/* }====================================================== */




ਸਥਿਰ ਅਚਲ luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", ਨਿਰਾ},
  {"cpath", ਨਿਰਾ},
  {"path", ਨਿਰਾ},
  {"searchers", ਨਿਰਾ},
  {"loaded", ਨਿਰਾ},
  {ਨਿਰਾ, ਨਿਰਾ}
};


ਸਥਿਰ ਅਚਲ luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {ਨਿਰਾ, ਨਿਰਾ}
};


ਸਥਿਰ ਖਾਲੀ createsearcherstable (lua_State *L) {
  ਸਥਿਰ ਅਚਲ lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    ਨਿਰਾ
  };
  ਪੂਰਨ i;
  /* create 'searchers' table */
  lua_createtable(L, ਆਕਾਰ(searchers)/ਆਕਾਰ(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  ਲਈ (i=0; searchers[i] != ਨਿਰਾ; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API ਪੂਰਨ luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_ਕੱਢੋ(L, 1);  /* will not use it now */
  luaL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LUA_ਰਸਤਾ_VAR, LUA_ਰਸਤਾ_DEFAULT);
  setpath(L, "cpath", LUA_CPATH_VAR, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_ਰਸਤਾ_SEP "\n" LUA_ਰਸਤਾ_MARK "\n"
                     LUA_EXEC_DIR "\n" LUA_IGMARK "\n");
  lua_setfield(L, -2, "config");
  /* set field 'loaded' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_ਸਾਰਣੀ);
  lua_setfield(L, -2, "loaded");
  /* set field 'preload' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_ਸਾਰਣੀ);
  lua_setfield(L, -2, "preload");
  lua_pushglobaltable(L);
  lua_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  luaL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  lua_ਕੱਢੋ(L, 1);  /* pop global table */
  ਵਾਪਸ 1;  /* return 'package' table */
}

