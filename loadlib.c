/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು loadlib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#ಆದರೆ !defined(LUA_CSUBSEP)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CSUBSEP		LUA_DIRSEP
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(LUA_LSUBSEP)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LSUBSEP		LUA_DIRSEP
#ಆದರೆ_ಅಂತ್ಯ


/* prefix for open functions in C libraries */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ CLIBS = "_CLIBS";

#ವ್ಯಾಖ್ಯಾನಿಸು LIB_FAIL	"open"


#ವ್ಯಾಖ್ಯಾನಿಸು setprogdir(L)           ((ಶೂನ್ಯ)0)


/* cast void* to a Lua function */
#ವ್ಯಾಖ್ಯಾನಿಸು cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *lsys_ತುಂಬು (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sym);




#ಆದರೆ defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#ಅನ್ತರ್ಭಾವ <dlfcn.h>


ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib) {
  dlclose(lib);
}


ಸ್ಥಿರ ಶೂನ್ಯ *lsys_ತುಂಬು (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ seeglb) {
  ಶೂನ್ಯ *lib = dlopen(ಮಾರ್ಗ, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  ಆದರೆ (l_unlikely(lib == ಖಾಲಿ))
    lua_pushstring(L, dlerror());
  ಹಿಂತಿರುಗಿ lib;
}


ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  ಆದರೆ (l_unlikely(f == ಖಾಲಿ))
    lua_pushstring(L, dlerror());
  ಹಿಂತಿರುಗಿ f;
}

/* }====================================================== */



#ಅಥವಾಆದರೆ defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#ಅನ್ತರ್ಭಾವ <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#ಆದರೆ !defined(LUA_LLE_FLAGS)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LLE_FLAGS	0
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setprogdir (lua_State *L) {
  ಅಕ್ಷರ buff[MAX_ಮಾರ್ಗ + 1];
  ಅಕ್ಷರ *lb;
  DWORD nsize = ಗಾತ್ರ(buff)/ಗಾತ್ರ(ಅಕ್ಷರ);
  DWORD n = GetModuleFileNameA(ಖಾಲಿ, buff, nsize);  /* get exec. name */
  ಆದರೆ (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == ಖಾಲಿ)
    luaL_error(L, "unable to get ModuleFileName");
  ಇಲ್ಲದಿದ್ದರೆ {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




ಸ್ಥಿರ ಶೂನ್ಯ pusherror (lua_State *L) {
  ಪೂರ್ಣಾಂಕ error = Get_ಕೊನೆಯ_Error();
  ಅಕ್ಷರ ಬಫರ್[128];
  ಆದರೆ (Format_ಸಂದೇಶ_A(FORMAT_ಸಂದೇಶ_IGNORE_INSERTS | FORMAT_ಸಂದೇಶ_FROM_SYSTEM,
      ಖಾಲಿ, error, 0, ಬಫರ್, ಗಾತ್ರ(ಬಫರ್)/ಗಾತ್ರ(ಅಕ್ಷರ), ಖಾಲಿ))
    lua_pushstring(L, ಬಫರ್);
  ಇಲ್ಲದಿದ್ದರೆ
    lua_pushfstring(L, "system error %d\n", error);
}

ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib) {
  FreeLibrary((HMODULE)lib);
}


ಸ್ಥಿರ ಶೂನ್ಯ *lsys_ತುಂಬು (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ seeglb) {
  HMODULE lib = ತುಂಬು_Library_Ex_A(ಮಾರ್ಗ, ಖಾಲಿ, LUA_LLE_FLAGS);
  (ಶೂನ್ಯ)(seeglb);  /* not used: symbols are 'global' by default */
  ಆದರೆ (lib == ಖಾಲಿ) pusherror(L);
  ಹಿಂತಿರುಗಿ lib;
}


ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sym) {
  lua_CFunction f = cast_Lfunc(Get_Proc_ವಿಳಾಸ((HMODULE)lib, sym));
  ಆದರೆ (f == ಖಾಲಿ) pusherror(L);
  ಹಿಂತಿರುಗಿ f;
}

/* }====================================================== */


#ಇಲ್ಲದಿದ್ದರೆ				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು LIB_FAIL
#ವ್ಯಾಖ್ಯಾನಿಸು LIB_FAIL	"absent"


#ವ್ಯಾಖ್ಯಾನಿಸು DLMSG	"dynamic libraries not enabled; check your Lua installation"


ಸ್ಥಿರ ಶೂನ್ಯ lsys_unloadlib (ಶೂನ್ಯ *lib) {
  (ಶೂನ್ಯ)(lib);  /* not used */
}


ಸ್ಥಿರ ಶೂನ್ಯ *lsys_ತುಂಬು (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ seeglb) {
  (ಶೂನ್ಯ)(ಮಾರ್ಗ); (ಶೂನ್ಯ)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


ಸ್ಥಿರ lua_CFunction lsys_sym (lua_State *L, ಶೂನ್ಯ *lib, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sym) {
  (ಶೂನ್ಯ)(lib); (ಶೂನ್ಯ)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* }====================================================== */
#ಆದರೆ_ಅಂತ್ಯ				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#ಆದರೆ !defined(LUA_ಮಾರ್ಗ_VAR)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_ಮಾರ್ಗ_VAR    "LUA_PATH"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(LUA_CPATH_VAR)
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CPATH_VAR   "LUA_CPATH"
#ಆದರೆ_ಅಂತ್ಯ



/*
** return registry.LUA_NOENV as a boolean
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ noenv (lua_State *L) {
  ಪೂರ್ಣಾಂಕ b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_ತೆಗೆ(L, 1);  /* remove value */
  ಹಿಂತಿರುಗಿ b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ setpath (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fieldname,
                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *envname,
                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dft) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dftmark;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ = getenv(nver);  /* try versioned name */
  ಆದರೆ (ಮಾರ್ಗ == ಖಾಲಿ)  /* no versioned environment variable? */
    ಮಾರ್ಗ = getenv(envname);  /* try unversioned name */
  ಆದರೆ (ಮಾರ್ಗ == ಖಾಲಿ || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(dft), ಖಾಲಿ, ಖಾಲಿ);  /* use default */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((dftmark = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(ಮಾರ್ಗ, LUA_ಮಾರ್ಗ_SEP LUA_ಮಾರ್ಗ_SEP)) == ಖಾಲಿ)
    lua_pushstring(L, ಮಾರ್ಗ);  /* nothing to change */
  ಇಲ್ಲದಿದ್ದರೆ {  /* path contains a ";;": insert default path in its place */
    size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮಾರ್ಗ);
    lua_L_ಬಫರ್ b;
    luaL_buffinit(L, &b);
    ಆದರೆ (ಮಾರ್ಗ < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, ಮಾರ್ಗ, ct_diff2sz(dftmark - ಮಾರ್ಗ));  /* add it */
      luaL_addchar(&b, *LUA_ಮಾರ್ಗ_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    ಆದರೆ (dftmark < ಮಾರ್ಗ + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_ಮಾರ್ಗ_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((ಮಾರ್ಗ + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_ತೆಗೆ(L, 1);  /* pop versioned variable name ('nver') */
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
ಸ್ಥಿರ ಶೂನ್ಯ *checkclib (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ) {
  ಶೂನ್ಯ *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, ಮಾರ್ಗ);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_ತೆಗೆ(L, 2);  /* pop CLIBS table and 'plib' */
  ಹಿಂತಿರುಗಿ plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *freelib (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (ಶೂನ್ಯ)ptr; (ಶೂನ್ಯ)osize; (ಶೂನ್ಯ)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
ಸ್ಥಿರ ಶೂನ್ಯ createlibstr (lua_State *L, ಶೂನ್ಯ *plib) {
  /* common content for all library strings */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, ಗಾತ್ರ(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
ಸ್ಥಿರ ಶೂನ್ಯ addtoclib (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಶೂನ್ಯ *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, ಮಾರ್ಗ);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_ತೆಗೆ(L, 1);  /* pop CLIBS table */
}


/* error codes for 'lookforfunc' */
#ವ್ಯಾಖ್ಯಾನಿಸು ERRLIB		1
#ವ್ಯಾಖ್ಯಾನಿಸು ERRFUNC		2

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lookforfunc (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sym) {
  ಶೂನ್ಯ *reg = checkclib(L, ಮಾರ್ಗ);  /* check loaded C libraries */
  ಆದರೆ (reg == ಖಾಲಿ) {  /* must load library? */
    reg = lsys_ತುಂಬು(L, ಮಾರ್ಗ, *sym == '*');  /* global symbols if 'sym'=='*' */
    ಆದರೆ (reg == ಖಾಲಿ) ಹಿಂತಿರುಗಿ ERRLIB;  /* unable to load library */
    addtoclib(L, ಮಾರ್ಗ, reg);
  }
  ಆದರೆ (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    ಹಿಂತಿರುಗಿ 0;  /* no errors */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_CFunction f = lsys_sym(L, reg, sym);
    ಆದರೆ (f == ಖಾಲಿ)
      ಹಿಂತಿರುಗಿ ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    ಹಿಂತಿರುಗಿ 0;  /* no errors */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ll_loadlib (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ = luaL_checkstring(L, 1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *init = luaL_checkstring(L, 2);
  ಪೂರ್ಣಾಂಕ stat = lookforfunc(L, ಮಾರ್ಗ, init);
  ಆದರೆ (l_likely(stat == 0))  /* no errors? */
    ಹಿಂತಿರುಗಿ 1;  /* return the loaded function */
  ಇಲ್ಲದಿದ್ದರೆ {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_ಸೇರಿಸು(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    ಹಿಂತಿರುಗಿ 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ readable (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  ಆದರೆ (f == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 0;  /* open failed */
  fclose(f);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getnextfilename (ಅಕ್ಷರ **ಮಾರ್ಗ, ಅಕ್ಷರ *ಕೊನೆ) {
  ಅಕ್ಷರ *sep;
  ಅಕ್ಷರ *name = *ಮಾರ್ಗ;
  ಆದರೆ (name == ಕೊನೆ)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no more names */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_ಮಾರ್ಗ_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_ಮಾರ್ಗ_SEP);  /* find next separator */
  ಆದರೆ (sep == ಖಾಲಿ)  /* separator not found? */
    sep = ಕೊನೆ;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *ಮಾರ್ಗ = sep;  /* will start next search from here */
  ಹಿಂತಿರುಗಿ name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
ಸ್ಥಿರ ಶೂನ್ಯ pusherrornotfound (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ) {
  lua_L_ಬಫರ್ b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, ಮಾರ್ಗ, LUA_ಮಾರ್ಗ_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *searchpath (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ,
                                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sep,
                                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dirsep) {
  lua_L_ಬಫರ್ buff;
  ಅಕ್ಷರ *pathname;  /* path with name inserted */
  ಅಕ್ಷರ *endpathname;  /* its end */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename;
  /* separator is non-empty and appears in 'name'? */
  ಆದರೆ (*sep != '\0' && strchr(name, *sep) != ಖಾಲಿ)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, ಮಾರ್ಗ, LUA_ಮಾರ್ಗ_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  ಆಗ ((filename = getnextfilename(&pathname, endpathname)) != ಖಾಲಿ) {
    ಆದರೆ (readable(filename))  /* does file exist and is readable? */
      ಹಿಂತಿರುಗಿ lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* not found */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ll_searchpath (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  ಆದರೆ (f != ಖಾಲಿ) ಹಿಂತಿರುಗಿ 1;
  ಇಲ್ಲದಿದ್ದರೆ {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_ಸೇರಿಸು(L, -2);
    ಹಿಂತಿರುಗಿ 2;  /* return fail + error message */
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *findfile (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pname,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dirsep) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ;
  lua_getfield(L, lua_upvalueindex(1), pname);
  ಮಾರ್ಗ = lua_tostring(L, -1);
  ಆದರೆ (l_unlikely(ಮಾರ್ಗ == ಖಾಲಿ))
    luaL_error(L, "'package.%s' must be a string", pname);
  ಹಿಂತಿರುಗಿ searchpath(L, name, ಮಾರ್ಗ, ".", dirsep);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkload (lua_State *L, ಪೂರ್ಣಾಂಕ stat, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename) {
  ಆದರೆ (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    ಹಿಂತಿರುಗಿ 2;  /* return open function and file name */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ searcher_Lua (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  ಆದರೆ (filename == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 1;  /* module not found in this path */
  ಹಿಂತಿರುಗಿ checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loadfunc (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *modname) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *openfunc;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  ಆದರೆ (mark) {
    ಪೂರ್ಣಾಂಕ stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    ಆದರೆ (stat != ERRFUNC) ಹಿಂತಿರುಗಿ stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  ಹಿಂತಿರುಗಿ lookforfunc(L, filename, openfunc);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ searcher_C (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = luaL_checkstring(L, 1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  ಆದರೆ (filename == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 1;  /* module not found in this path */
  ಹಿಂತಿರುಗಿ checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ searcher_Croot (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = luaL_checkstring(L, 1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = strchr(name, '.');
  ಪೂರ್ಣಾಂಕ stat;
  ಆದರೆ (p == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  ಆದರೆ (filename == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 1;  /* root not found */
  ಆದರೆ ((stat = loadfunc(L, filename, name)) != 0) {
    ಆದರೆ (stat != ERRFUNC)
      ಹಿಂತಿರುಗಿ checkload(L, 0, filename);  /* real error */
    ಇಲ್ಲದಿದ್ದರೆ {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  ಹಿಂತಿರುಗಿ 2;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ searcher_preload (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_ಕೋಷ್ಟಕ);
  ಆದರೆ (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushliteral(L, ":preload:");
    ಹಿಂತಿರುಗಿ 2;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ findloader (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name) {
  ಪೂರ್ಣಾಂಕ i;
  lua_L_ಬಫರ್ ಸಂದೇಶ;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  ಆದರೆ (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &ಸಂದೇಶ);
  luaL_addstring(&ಸಂದೇಶ, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  ಗಾಗಿ (i = 1; ; i++) {
    ಆದರೆ (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_ತೆಗೆ(L, 1);  /* remove nil */
      luaL_buffsub(&ಸಂದೇಶ, 2);  /* remove last prefix */
      luaL_pushresult(&ಸಂದೇಶ);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    ಆದರೆ (lua_isfunction(L, -2))  /* did it find a loader? */
      ಹಿಂತಿರುಗಿ;  /* module loader found */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_ತೆಗೆ(L, 1);  /* remove extra return */
      luaL_addvalue(&ಸಂದೇಶ);  /* concatenate error message */
      luaL_addstring(&ಸಂದೇಶ, "\n\t");  /* prefix for next message */
    }
    ಇಲ್ಲದಿದ್ದರೆ  /* no error message */
      lua_ತೆಗೆ(L, 2);  /* remove both returns */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ll_require (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_ಕೋಷ್ಟಕ);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  ಆದರೆ (lua_toboolean(L, -1))  /* is it there? */
    ಹಿಂತಿರುಗಿ 1;  /* package is already loaded */
  /* else must load package */
  lua_ತೆಗೆ(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  ಆದರೆ (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  ಇಲ್ಲದಿದ್ದರೆ
    lua_ತೆಗೆ(L, 1);  /* pop nil */
  ಆದರೆ (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  ಹಿಂತಿರುಗಿ 2;  /* return module result and loader data */
}

/* }====================================================== */




ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", ಖಾಲಿ},
  {"cpath", ಖಾಲಿ},
  {"path", ಖಾಲಿ},
  {"searchers", ಖಾಲಿ},
  {"loaded", ಖಾಲಿ},
  {ಖಾಲಿ, ಖಾಲಿ}
};


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {ಖಾಲಿ, ಖಾಲಿ}
};


ಸ್ಥಿರ ಶೂನ್ಯ createsearcherstable (lua_State *L) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    ಖಾಲಿ
  };
  ಪೂರ್ಣಾಂಕ i;
  /* create 'searchers' table */
  lua_createtable(L, ಗಾತ್ರ(searchers)/ಗಾತ್ರ(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  ಗಾಗಿ (i=0; searchers[i] != ಖಾಲಿ; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_ತೆಗೆ(L, 1);  /* will not use it now */
  luaL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LUA_ಮಾರ್ಗ_VAR, LUA_ಮಾರ್ಗ_DEFAULT);
  setpath(L, "cpath", LUA_CPATH_VAR, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_ಮಾರ್ಗ_SEP "\n" LUA_ಮಾರ್ಗ_MARK "\n"
                     LUA_EXEC_DIR "\n" LUA_IGMARK "\n");
  lua_setfield(L, -2, "config");
  /* set field 'loaded' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_ಕೋಷ್ಟಕ);
  lua_setfield(L, -2, "loaded");
  /* set field 'preload' */
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_ಕೋಷ್ಟಕ);
  lua_setfield(L, -2, "preload");
  lua_pushglobaltable(L);
  lua_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  luaL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  lua_ತೆಗೆ(L, 1);  /* pop global table */
  ಹಿಂತಿರುಗಿ 1;  /* return 'package' table */
}

