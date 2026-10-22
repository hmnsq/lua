/*
** $Id: loadlib.c $
** Dynamic library loader for Lua
** See Copyright Notice in lua.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#تعریف_کړه loadlib_c
#تعریف_کړه LUA_LIB

#ضمن "lprefix.h"


#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


/*
** LUA_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LUA_LSUBSEP is the character that replaces dots in submodule names
** when searching for a Lua loader.
*/
#إذا !defined(LUA_CSUBSEP)
#تعریف_کړه LUA_CSUBSEP		LUA_DIRSEP
#د_که_پای

#إذا !defined(LUA_LSUBSEP)
#تعریف_کړه LUA_LSUBSEP		LUA_DIRSEP
#د_که_پای


/* prefix for open functions in C libraries */
#تعریف_کړه LUA_POF		"luaopen_"

/* separator for open functions in C libraries */
#تعریف_کړه LUA_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
ساكن ثابت حرف *ثابت CLIBS = "_CLIBS";

#تعریف_کړه LIB_FAIL	"open"


#تعریف_کړه setprogdir(L)           ((فراغ)0)


/* cast void* to a Lua function */
#تعریف_کړه cast_Lfunc(p)	cast(lua_CFunction, cast_func(p))


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
ساكن فراغ lsys_unloadlib (فراغ *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
ساكن فراغ *lsys_بارول (lua_State *L, ثابت حرف *لار, صحيح seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
ساكن lua_CFunction lsys_sym (lua_State *L, فراغ *lib, ثابت حرف *sym);




#إذا defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface,
** which is available in all POSIX systems.
** =========================================================================
*/

#ضمن <dlfcn.h>


ساكن فراغ lsys_unloadlib (فراغ *lib) {
  dlclose(lib);
}


ساكن فراغ *lsys_بارول (lua_State *L, ثابت حرف *لار, صحيح seeglb) {
  فراغ *lib = dlopen(لار, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  إذا (l_unlikely(lib == عدم))
    lua_pushstring(L, dlerror());
  أرجع lib;
}


ساكن lua_CFunction lsys_sym (lua_State *L, فراغ *lib, ثابت حرف *sym) {
  lua_CFunction f = cast_Lfunc(dlsym(lib, sym));
  إذا (l_unlikely(f == عدم))
    lua_pushstring(L, dlerror());
  أرجع f;
}

/* }====================================================== */



#وإلاإذا defined(LUA_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#ضمن <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#إذا !defined(LUA_LLE_FLAGS)
#تعریف_کړه LUA_LLE_FLAGS	0
#د_که_پای


#تعریف_لغوه setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LUA_EXEC_DIR with the executable's path.
*/
ساكن فراغ setprogdir (lua_State *L) {
  حرف buff[MAX_لار + 1];
  حرف *lb;
  DWORD nsize = حجم(buff)/حجم(حرف);
  DWORD n = GetModuleFileNameA(عدم, buff, nsize);  /* get exec. name */
  إذا (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == عدم)
    luaL_error(L, "unable to get ModuleFileName");
  وإلا {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    luaL_gsub(L, lua_tostring(L, -1), LUA_EXEC_DIR, buff);
    lua_remove(L, -2);  /* remove original string */
  }
}




ساكن فراغ pusherror (lua_State *L) {
  صحيح error = Get_وروستی_Error();
  حرف بفر[128];
  إذا (Format_پیغام_A(FORMAT_پیغام_IGNORE_INSERTS | FORMAT_پیغام_FROM_SYSTEM,
      عدم, error, 0, بفر, حجم(بفر)/حجم(حرف), عدم))
    lua_pushstring(L, بفر);
  وإلا
    lua_pushfstring(L, "system error %d\n", error);
}

ساكن فراغ lsys_unloadlib (فراغ *lib) {
  FreeLibrary((HMODULE)lib);
}


ساكن فراغ *lsys_بارول (lua_State *L, ثابت حرف *لار, صحيح seeglb) {
  HMODULE lib = بارول_Library_Ex_A(لار, عدم, LUA_LLE_FLAGS);
  (فراغ)(seeglb);  /* not used: symbols are 'global' by default */
  إذا (lib == عدم) pusherror(L);
  أرجع lib;
}


ساكن lua_CFunction lsys_sym (lua_State *L, فراغ *lib, ثابت حرف *sym) {
  lua_CFunction f = cast_Lfunc(Get_Proc_پته((HMODULE)lib, sym));
  إذا (f == عدم) pusherror(L);
  أرجع f;
}

/* }====================================================== */


#وإلا				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#تعریف_لغوه LIB_FAIL
#تعریف_کړه LIB_FAIL	"absent"


#تعریف_کړه DLMSG	"dynamic libraries not enabled; check your Lua installation"


ساكن فراغ lsys_unloadlib (فراغ *lib) {
  (فراغ)(lib);  /* not used */
}


ساكن فراغ *lsys_بارول (lua_State *L, ثابت حرف *لار, صحيح seeglb) {
  (فراغ)(لار); (فراغ)(seeglb);  /* not used */
  lua_pushliteral(L, DLMSG);
  أرجع عدم;
}


ساكن lua_CFunction lsys_sym (lua_State *L, فراغ *lib, ثابت حرف *sym) {
  (فراغ)(lib); (فراغ)(sym);  /* not used */
  lua_pushliteral(L, DLMSG);
  أرجع عدم;
}

/* }====================================================== */
#د_که_پای				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LUA_PATH_VAR and LUA_CPATH_VAR are the names of the environment
** variables that Lua check to set its paths.
*/
#إذا !defined(LUA_لار_VAR)
#تعریف_کړه LUA_لار_VAR    "LUA_PATH"
#د_که_پای

#إذا !defined(LUA_CPATH_VAR)
#تعریف_کړه LUA_CPATH_VAR   "LUA_CPATH"
#د_که_پای



/*
** return registry.LUA_NOENV as a boolean
*/
ساكن صحيح noenv (lua_State *L) {
  صحيح b;
  lua_getfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_راوباسه(L, 1);  /* remove value */
  أرجع b;
}


/*
** Set a path. (If using the default path, assume it is a string
** literal in C and create it as an external string.)
*/
ساكن فراغ setpath (lua_State *L, ثابت حرف *fieldname,
                                   ثابت حرف *envname,
                                   ثابت حرف *dft) {
  ثابت حرف *dftmark;
  ثابت حرف *nver = lua_pushfstring(L, "%s%s", envname, LUA_VERSUFFIX);
  ثابت حرف *لار = getenv(nver);  /* try versioned name */
  إذا (لار == عدم)  /* no versioned environment variable? */
    لار = getenv(envname);  /* try unversioned name */
  إذا (لار == عدم || noenv(L))  /* no environment variable? */
    lua_pushexternalstring(L, dft, مزي_اوږدوالی(dft), عدم, عدم);  /* use default */
  وإلا إذا ((dftmark = مزي_پلټنه(لار, LUA_لار_SEP LUA_لار_SEP)) == عدم)
    lua_pushstring(L, لار);  /* nothing to change */
  وإلا {  /* path contains a ";;": insert default path in its place */
    size_t len = مزي_اوږدوالی(لار);
    lua_L_بفر b;
    luaL_buffinit(L, &b);
    إذا (لار < dftmark) {  /* is there a prefix before ';;'? */
      luaL_addlstring(&b, لار, ct_diff2sz(dftmark - لار));  /* add it */
      luaL_addchar(&b, *LUA_لار_SEP);
    }
    luaL_addstring(&b, dft);  /* add default */
    إذا (dftmark < لار + len - 2) {  /* is there a suffix after ';;'? */
      luaL_addchar(&b, *LUA_لار_SEP);
      luaL_addlstring(&b, dftmark + 2, ct_diff2sz((لار + len - 2) - dftmark));
    }
    luaL_pushresult(&b);
  }
  setprogdir(L);
  lua_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  lua_راوباسه(L, 1);  /* pop versioned variable name ('nver') */
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
ساكن فراغ *checkclib (lua_State *L, ثابت حرف *لار) {
  فراغ *plib;
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_getfield(L, -1, لار);
  plib = lua_touserdata(L, -1);  /* plib = CLIBS[path] */
  lua_راوباسه(L, 2);  /* pop CLIBS table and 'plib' */
  أرجع plib;
}


/*
** Deallocate function for library strings.
** Unload the DLL associated with the string being deallocated.
*/
ساكن فراغ *freelib (فراغ *ud, فراغ *ptr, size_t osize, size_t nsize) {
  /* string itself is irrelevant and static */
  (فراغ)ptr; (فراغ)osize; (فراغ)nsize;
  lsys_unloadlib(ud);  /* unload library represented by the string */
  أرجع عدم;
}


/*
** Create a library string that, when deallocated, will unload 'plib'
*/
ساكن فراغ createlibstr (lua_State *L, فراغ *plib) {
  /* common content for all library strings */
  ساكن ثابت حرف dummy[] = "01234567890";
  lua_pushexternalstring(L, dummy, حجم(dummy) - 1, freelib, plib);
}


/*
** registry.CLIBS[path] = plib          -- for queries.
** Also create a reference to strlib, so that the library string will
** only be collected when registry.CLIBS is collected.
*/
ساكن فراغ addtoclib (lua_State *L, ثابت حرف *لار, فراغ *plib) {
  lua_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_setfield(L, -2, لار);  /* CLIBS[path] = plib */
  createlibstr(L, plib);
  luaL_ref(L, -2);  /* keep library string in CLIBS */
  lua_راوباسه(L, 1);  /* pop CLIBS table */
}


/* error codes for 'lookforfunc' */
#تعریف_کړه ERRLIB		1
#تعریف_کړه ERRFUNC		2

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
ساكن صحيح lookforfunc (lua_State *L, ثابت حرف *لار, ثابت حرف *sym) {
  فراغ *reg = checkclib(L, لار);  /* check loaded C libraries */
  إذا (reg == عدم) {  /* must load library? */
    reg = lsys_بارول(L, لار, *sym == '*');  /* global symbols if 'sym'=='*' */
    إذا (reg == عدم) أرجع ERRLIB;  /* unable to load library */
    addtoclib(L, لار, reg);
  }
  إذا (*sym == '*') {  /* loading only library (no function)? */
    lua_pushboolean(L, 1);  /* return 'true' */
    أرجع 0;  /* no errors */
  }
  وإلا {
    lua_CFunction f = lsys_sym(L, reg, sym);
    إذا (f == عدم)
      أرجع ERRFUNC;  /* unable to find function */
    lua_pushcfunction(L, f);  /* else create new function */
    أرجع 0;  /* no errors */
  }
}


ساكن صحيح ll_loadlib (lua_State *L) {
  ثابت حرف *لار = luaL_checkstring(L, 1);
  ثابت حرف *init = luaL_checkstring(L, 2);
  صحيح stat = lookforfunc(L, لار, init);
  إذا (l_likely(stat == 0))  /* no errors? */
    أرجع 1;  /* return the loaded function */
  وإلا {  /* error; error message is on stack top */
    luaL_pushfail(L);
    lua_ورننباسه(L, -2);
    lua_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    أرجع 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


ساكن صحيح readable (ثابت حرف *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  إذا (f == عدم) أرجع 0;  /* open failed */
  fclose(f);
  أرجع 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
ساكن ثابت حرف *getnextfilename (حرف **لار, حرف *پای_ټکی) {
  حرف *sep;
  حرف *name = *لار;
  إذا (name == پای_ټکی)
    أرجع عدم;  /* no more names */
  وإلا إذا (*name == '\0') {  /* from previous iteration? */
    *name = *LUA_لار_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LUA_لار_SEP);  /* find next separator */
  إذا (sep == عدم)  /* separator not found? */
    sep = پای_ټکی;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *لار = sep;  /* will start next search from here */
  أرجع name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
ساكن فراغ pusherrornotfound (lua_State *L, ثابت حرف *لار) {
  lua_L_بفر b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, لار, LUA_لار_SEP, "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


ساكن ثابت حرف *searchpath (lua_State *L, ثابت حرف *name,
                                             ثابت حرف *لار,
                                             ثابت حرف *sep,
                                             ثابت حرف *dirsep) {
  lua_L_بفر buff;
  حرف *pathname;  /* path with name inserted */
  حرف *endpathname;  /* its end */
  ثابت حرف *filename;
  /* separator is non-empty and appears in 'name'? */
  إذا (*sep != '\0' && strchr(name, *sep) != عدم)
    name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  luaL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  luaL_addgsub(&buff, لار, LUA_لار_MARK, name);
  luaL_addchar(&buff, '\0');
  pathname = luaL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + luaL_bufflen(&buff) - 1;
  طالما ((filename = getnextfilename(&pathname, endpathname)) != عدم) {
    إذا (readable(filename))  /* does file exist and is readable? */
      أرجع lua_pushstring(L, filename);  /* save and return name */
  }
  luaL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, lua_tostring(L, -1));  /* create error message */
  أرجع عدم;  /* not found */
}


ساكن صحيح ll_searchpath (lua_State *L) {
  ثابت حرف *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LUA_DIRSEP));
  إذا (f != عدم) أرجع 1;
  وإلا {  /* error message is on top of the stack */
    luaL_pushfail(L);
    lua_ورننباسه(L, -2);
    أرجع 2;  /* return fail + error message */
  }
}


ساكن ثابت حرف *findfile (lua_State *L, ثابت حرف *name,
                                           ثابت حرف *pname,
                                           ثابت حرف *dirsep) {
  ثابت حرف *لار;
  lua_getfield(L, lua_upvalueindex(1), pname);
  لار = lua_tostring(L, -1);
  إذا (l_unlikely(لار == عدم))
    luaL_error(L, "'package.%s' must be a string", pname);
  أرجع searchpath(L, name, لار, ".", dirsep);
}


ساكن صحيح checkload (lua_State *L, صحيح stat, ثابت حرف *filename) {
  إذا (l_likely(stat)) {  /* module loaded successfully? */
    lua_pushstring(L, filename);  /* will be 2nd argument to module */
    أرجع 2;  /* return open function and file name */
  }
  وإلا
    أرجع luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tostring(L, 1), filename, lua_tostring(L, -1));
}


ساكن صحيح searcher_Lua (lua_State *L) {
  ثابت حرف *filename;
  ثابت حرف *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LUA_LSUBSEP);
  إذا (filename == عدم) أرجع 1;  /* module not found in this path */
  أرجع checkload(L, (luaL_loadfilex(L, filename, "bt") == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
ساكن صحيح loadfunc (lua_State *L, ثابت حرف *filename, ثابت حرف *modname) {
  ثابت حرف *openfunc;
  ثابت حرف *mark;
  modname = luaL_gsub(L, modname, ".", LUA_OFSEP);
  mark = strchr(modname, *LUA_IGMARK);
  إذا (mark) {
    صحيح stat;
    openfunc = lua_pushlstring(L, modname, ct_diff2sz(mark - modname));
    openfunc = lua_pushfstring(L, LUA_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    إذا (stat != ERRFUNC) أرجع stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = lua_pushfstring(L, LUA_POF"%s", modname);
  أرجع lookforfunc(L, filename, openfunc);
}


ساكن صحيح searcher_C (lua_State *L) {
  ثابت حرف *name = luaL_checkstring(L, 1);
  ثابت حرف *filename = findfile(L, name, "cpath", LUA_CSUBSEP);
  إذا (filename == عدم) أرجع 1;  /* module not found in this path */
  أرجع checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


ساكن صحيح searcher_Croot (lua_State *L) {
  ثابت حرف *filename;
  ثابت حرف *name = luaL_checkstring(L, 1);
  ثابت حرف *p = strchr(name, '.');
  صحيح stat;
  إذا (p == عدم) أرجع 0;  /* is root */
  lua_pushlstring(L, name, ct_diff2sz(p - name));
  filename = findfile(L, lua_tostring(L, -1), "cpath", LUA_CSUBSEP);
  إذا (filename == عدم) أرجع 1;  /* root not found */
  إذا ((stat = loadfunc(L, filename, name)) != 0) {
    إذا (stat != ERRFUNC)
      أرجع checkload(L, 0, filename);  /* real error */
    وإلا {  /* open function not found */
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      أرجع 1;
    }
  }
  lua_pushstring(L, filename);  /* will be 2nd argument to module */
  أرجع 2;
}


ساكن صحيح searcher_preload (lua_State *L) {
  ثابت حرف *name = luaL_checkstring(L, 1);
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_PRELOAD_جدول);
  إذا (lua_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    lua_pushfstring(L, "no field package.preload['%s']", name);
    أرجع 1;
  }
  وإلا {
    lua_pushliteral(L, ":preload:");
    أرجع 2;
  }
}


ساكن فراغ findloader (lua_State *L, ثابت حرف *name) {
  صحيح i;
  lua_L_بفر پیغام;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  إذا (l_unlikely(lua_getfield(L, lua_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &پیغام);
  luaL_addstring(&پیغام, "\n\t");  /* error-message prefix for first message */
  /*  iterate over available searchers to find a loader */
  لكل (i = 1; ; i++) {
    إذا (l_unlikely(lua_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      lua_راوباسه(L, 1);  /* remove nil */
      luaL_buffsub(&پیغام, 2);  /* remove last prefix */
      luaL_pushresult(&پیغام);  /* create error message */
      luaL_error(L, "module '%s' not found:%s", name, lua_tostring(L, -1));
    }
    lua_pushstring(L, name);
    lua_call(L, 1, 2);  /* call it */
    إذا (lua_isfunction(L, -2))  /* did it find a loader? */
      أرجع;  /* module loader found */
    وإلا إذا (lua_isstring(L, -2)) {  /* searcher returned error message? */
      lua_راوباسه(L, 1);  /* remove extra return */
      luaL_addvalue(&پیغام);  /* concatenate error message */
      luaL_addstring(&پیغام, "\n\t");  /* prefix for next message */
    }
    وإلا  /* no error message */
      lua_راوباسه(L, 2);  /* remove both returns */
  }
}


ساكن صحيح ll_require (lua_State *L) {
  ثابت حرف *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_جدول);
  lua_getfield(L, 2, name);  /* LOADED[name] */
  إذا (lua_toboolean(L, -1))  /* is it there? */
    أرجع 1;  /* package is already loaded */
  /* else must load package */
  lua_راوباسه(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  lua_rotate(L, -2, 1);  /* function <-> loader data */
  lua_pushvalue(L, 1);  /* name is 1st argument to module loader */
  lua_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  lua_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  إذا (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* LOADED[name] = returned value */
  وإلا
    lua_راوباسه(L, 1);  /* pop nil */
  إذا (lua_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_copy(L, -1, -2);  /* replace loader result */
    lua_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  lua_rotate(L, -2, 1);  /* loader data <-> module result  */
  أرجع 2;  /* return module result and loader data */
}

/* }====================================================== */




ساكن ثابت luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", عدم},
  {"cpath", عدم},
  {"path", عدم},
  {"searchers", عدم},
  {"loaded", عدم},
  {عدم, عدم}
};


ساكن ثابت luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {عدم, عدم}
};


ساكن فراغ createsearcherstable (lua_State *L) {
  ساكن ثابت lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    عدم
  };
  صحيح i;
  /* create 'searchers' table */
  lua_createtable(L, حجم(searchers)/حجم(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  لكل (i=0; searchers[i] != عدم; i++) {
    lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


LUAMOD_API صحيح luaopen_package (lua_State *L) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  lua_راوباسه(L, 1);  /* will not use it now */
  luaL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LUA_لار_VAR, LUA_لار_DEFAULT);
  setpath(L, "cpath", LUA_CPATH_VAR, LUA_CPATH_DEFAULT);
  /* store config information */
  lua_pushliteral(L, LUA_DIRSEP "\n" LUA_لار_SEP "\n" LUA_لار_MARK "\n"
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
  lua_راوباسه(L, 1);  /* pop global table */
  أرجع 1;  /* return 'package' table */
}

