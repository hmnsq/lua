/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lauxlib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


/*
** {======================================================
** Traceback
** =======================================================
*/


#ವ್ಯಾಖ್ಯಾನಿಸು LEVELS1	10	/* size of the first part of the stack */
#ವ್ಯಾಖ್ಯಾನಿಸು LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ findfield (lua_State *L, ಪೂರ್ಣಾಂಕ objidx, ಪೂರ್ಣಾಂಕ level) {
  ಆದರೆ (level == 0 || !lua_istable(L, -1))
    ಹಿಂತಿರುಗಿ 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  ಆಗ (lua_ಮುಂದಿನ(L, -2)) {  /* for each pair in table */
    ಆದರೆ (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      ಆದರೆ (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_ತೆಗೆ(L, 1);  /* remove value (but keep name) */
        ಹಿಂತಿರುಗಿ 1;
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        ಹಿಂತಿರುಗಿ 1;
      }
    }
    lua_ತೆಗೆ(L, 1);  /* remove value */
  }
  ಹಿಂತಿರುಗಿ 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  ಪೂರ್ಣಾಂಕ ತುದಿ = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_ಕೋಷ್ಟಕ);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  ಆದರೆ (findfield(L, ತುದಿ + 1, 2)) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = lua_tostring(L, -1);
    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, ತುದಿ + 1);  /* copy name to proper place */
    lua_settop(L, ತುದಿ + 1);  /* remove table "loaded" and name copy */
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_settop(L, ತುದಿ);  /* remove function and global table */
    ಹಿಂತಿರುಗಿ 0;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ pushfuncname (lua_State *L, lua_Debug *ar) {
  ಆದರೆ (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  ಇಲ್ಲದಿದ್ದರೆ  /* nothing left... */
    lua_pushliteral(L, "?");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lastlevel (lua_State *L) {
  lua_Debug ar;
  ಪೂರ್ಣಾಂಕ li = 1, le = 1;
  /* find an upper bound */
  ಆಗ (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  ಆಗ (li < le) {
    ಪೂರ್ಣಾಂಕ m = (li + le)/2;
    ಆದರೆ (lua_getstack(L, m, &ar)) li = m + 1;
    ಇಲ್ಲದಿದ್ದರೆ le = m;
  }
  ಹಿಂತಿರುಗಿ le - 1;
}


LUALIB_API ಶೂನ್ಯ luaL_traceback (lua_State *L, lua_State *L1,
                                ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ level) {
  lua_L_ಬಫರ್ b;
  lua_Debug ar;
  ಪೂರ್ಣಾಂಕ ಕೊನೆಯ = lastlevel(L1);
  ಪೂರ್ಣಾಂಕ limit2show = (ಕೊನೆಯ - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  ಆದರೆ (ಸಂದೇಶ) {
    luaL_addstring(&b, ಸಂದೇಶ);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  ಆಗ (lua_getstack(L1, level++, &ar)) {
    ಆದರೆ (limit2show-- == 0) {  /* too many levels? */
      ಪೂರ್ಣಾಂಕ n = ಕೊನೆಯ - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      lua_getinfo(L1, "Slnt", &ar);
      ಆದರೆ (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      ಇಲ್ಲದಿದ್ದರೆ
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      ಆದರೆ (ar.istailcall)
        luaL_addstring(&b, "\n\t(...tail calls...)");
    }
  }
  luaL_pushresult(&b);
}

/* }====================================================== */


/*
** {======================================================
** Error-report functions
** =======================================================
*/

LUALIB_API ಪೂರ್ಣಾಂಕ luaL_argerror (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extramsg) {
  lua_Debug ar;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argword;
  ಆದರೆ (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ಹಿಂತಿರುಗಿ luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  ಆದರೆ (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  ಇಲ್ಲದಿದ್ದರೆ {
    arg -= ar.extraargs;  /* do not count extra arguments */
    ಆದರೆ (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      ಆದರೆ (arg == 0)  /* error in self argument? */
        ಹಿಂತಿರುಗಿ luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  ಆದರೆ (ar.name == ಖಾಲಿ)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  ಹಿಂತಿರುಗಿ luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API ಪೂರ್ಣಾಂಕ luaL_typeerror (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *typearg;  /* name for the type of the actual argument */
  ಆದರೆ (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  ಇಲ್ಲದಿದ್ದರೆ
    typearg = luaL_typename(L, arg);  /* standard name */
  ಸಂದೇಶ = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  ಹಿಂತಿರುಗಿ luaL_argerror(L, arg, ಸಂದೇಶ);
}


ಸ್ಥಿರ ಶೂನ್ಯ ಟ್ಯಾಗ್_error (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್) {
  luaL_typeerror(L, arg, lua_typename(L, ಟ್ಯಾಗ್));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API ಶೂನ್ಯ luaL_where (lua_State *L, ಪೂರ್ಣಾಂಕ level) {
  lua_Debug ar;
  ಆದರೆ (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    ಆದರೆ (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      ಹಿಂತಿರುಗಿ;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API ಪೂರ್ಣಾಂಕ luaL_error (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  ಹಿಂತಿರುಗಿ lua_error(L);
}


LUALIB_API ಪೂರ್ಣಾಂಕ luaL_fileresult (lua_State *L, ಪೂರ್ಣಾಂಕ stat, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname) {
  ಪೂರ್ಣಾಂಕ en = errno;  /* calls to Lua API may change this value */
  ಆದರೆ (stat) {
    lua_pushboolean(L, 1);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;
    luaL_pushfail(L);
    ಸಂದೇಶ = (en != 0) ? strerror(en) : "(no extra info)";
    ಆದರೆ (fname)
      lua_pushfstring(L, "%s: %s", fname, ಸಂದೇಶ);
    ಇಲ್ಲದಿದ್ದರೆ
      lua_pushstring(L, ಸಂದೇಶ);
    lua_pushinteger(L, en);
    ಹಿಂತಿರುಗಿ 3;
  }
}


#ಆದರೆ !defined(l_inspectstat)	/* { */

#ಆದರೆ defined(LUA_USE_POSIX)

#ಅನ್ತರ್ಭಾವ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#ವ್ಯಾಖ್ಯಾನಿಸು l_inspectstat(stat,what)  \
   ಆದರೆ (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#ಇಲ್ಲದಿದ್ದರೆ

#ವ್ಯಾಖ್ಯಾನಿಸು l_inspectstat(stat,what)  /* no op */

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ				/* } */


LUALIB_API ಪೂರ್ಣಾಂಕ luaL_execresult (lua_State *L, ಪೂರ್ಣಾಂಕ stat) {
  ಆದರೆ (stat != 0 && errno != 0)  /* error with an 'errno'? */
    ಹಿಂತಿರುಗಿ luaL_fileresult(L, 0, ಖಾಲಿ);
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    ಆದರೆ (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    ಇಲ್ಲದಿದ್ದರೆ
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    ಹಿಂತಿರುಗಿ 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API ಪೂರ್ಣಾಂಕ luaL_newmetatable (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  ಆದರೆ (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    ಹಿಂತಿರುಗಿ 0;  /* leave previous value on top, but return 0 */
  lua_ತೆಗೆ(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  ಹಿಂತಿರುಗಿ 1;
}


LUALIB_API ಶೂನ್ಯ luaL_setmetatable (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API ಶೂನ್ಯ *luaL_testudata (lua_State *L, ಪೂರ್ಣಾಂಕ ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  ಶೂನ್ಯ *p = lua_touserdata(L, ud);
  ಆದರೆ (p != ಖಾಲಿ) {  /* value is a userdata? */
    ಆದರೆ (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      ಆದರೆ (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ಖಾಲಿ;  /* value is a userdata with wrong metatable */
      lua_ತೆಗೆ(L, 2);  /* remove both metatables */
      ಹಿಂತಿರುಗಿ p;
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* value is not a userdata with a metatable */
}


LUALIB_API ಶೂನ್ಯ *luaL_checkudata (lua_State *L, ಪೂರ್ಣಾಂಕ ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  ಶೂನ್ಯ *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ಖಾಲಿ, ud, tname);
  ಹಿಂತಿರುಗಿ p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API ಪೂರ್ಣಾಂಕ luaL_checkoption (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *def,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ lst[]) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i=0; lst[i]; i++)
    ಆದರೆ (strcmp(lst[i], name) == 0)
      ಹಿಂತಿರುಗಿ i;
  ಹಿಂತಿರುಗಿ luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API ಶೂನ್ಯ luaL_checkstack (lua_State *L, ಪೂರ್ಣಾಂಕ space, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
  ಆದರೆ (l_unlikely(!lua_checkstack(L, space))) {
    ಆದರೆ (ಸಂದೇಶ)
      luaL_error(L, "stack overflow (%s)", ಸಂದೇಶ);
    ಇಲ್ಲದಿದ್ದರೆ
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API ಶೂನ್ಯ luaL_checktype (lua_State *L, ಪೂರ್ಣಾಂಕ arg, ಪೂರ್ಣಾಂಕ t) {
  ಆದರೆ (l_unlikely(lua_type(L, arg) != t))
    ಟ್ಯಾಗ್_error(L, arg, t);
}


LUALIB_API ಶೂನ್ಯ luaL_checkany (lua_State *L, ಪೂರ್ಣಾಂಕ arg) {
  ಆದರೆ (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaL_checklstring (lua_State *L, ಪೂರ್ಣಾಂಕ arg, size_t *len) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_tolstring(L, arg, len);
  ಆದರೆ (l_unlikely(!s)) ಟ್ಯಾಗ್_error(L, arg, LUA_TSTRING);
  ಹಿಂತಿರುಗಿ s;
}


LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaL_optlstring (lua_State *L, ಪೂರ್ಣಾಂಕ arg,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *def, size_t *len) {
  ಆದರೆ (lua_isnoneornil(L, arg)) {
    ಆದರೆ (len)
      *len = (def ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(def) : 0);
    ಹಿಂತಿರುಗಿ def;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ luaL_checklstring(L, arg, len);
}


LUALIB_API lua_ಸಂಖ್ಯೆ luaL_checknumber (lua_State *L, ಪೂರ್ಣಾಂಕ arg) {
  ಪೂರ್ಣಾಂಕ isnum;
  lua_ಸಂಖ್ಯೆ d = lua_tonumberx(L, arg, &isnum);
  ಆದರೆ (l_unlikely(!isnum))
    ಟ್ಯಾಗ್_error(L, arg, LUA_TNUMBER);
  ಹಿಂತಿರುಗಿ d;
}


LUALIB_API lua_ಸಂಖ್ಯೆ luaL_optnumber (lua_State *L, ಪೂರ್ಣಾಂಕ arg, lua_ಸಂಖ್ಯೆ def) {
  ಹಿಂತಿರುಗಿ luaL_opt(L, luaL_checknumber, arg, def);
}


ಸ್ಥಿರ ಶೂನ್ಯ interror (lua_State *L, ಪೂರ್ಣಾಂಕ arg) {
  ಆದರೆ (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  ಇಲ್ಲದಿದ್ದರೆ
    ಟ್ಯಾಗ್_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, ಪೂರ್ಣಾಂಕ arg) {
  ಪೂರ್ಣಾಂಕ isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  ಆದರೆ (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  ಹಿಂತಿರುಗಿ d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, ಪೂರ್ಣಾಂಕ arg,
                                                      lua_Integer def) {
  ಹಿಂತಿರುಗಿ luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ UBox {
  ಶೂನ್ಯ *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ *resizebox (lua_State *L, ಪೂರ್ಣಾಂಕ idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  ಆದರೆ (box->bsize == newsize)  /* not changing size? */
    ಹಿಂತಿರುಗಿ box->box;  /* keep the buffer */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಶೂನ್ಯ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    ಶೂನ್ಯ *ತಾತ್ಕಾಲಿಕ = allocf(ud, box->box, box->bsize, newsize);
    ಆದರೆ (l_unlikely(ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = ತಾತ್ಕಾಲಿಕ;
    box->bsize = newsize;
    ಹಿಂತಿರುಗಿ ತಾತ್ಕಾಲಿಕ;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ಖಾಲಿ, ಖಾಲಿ}
};


/*
** Get/create metatable (MT) for boxes
*/
ಸ್ಥಿರ ಶೂನ್ಯ getBoxMT (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *BOXMT = "_UBOX*"; /* key for the metatable */
  ಆದರೆ (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, ಗಾತ್ರ(UBox), 0);
  box->box = ಖಾಲಿ;
  box->bsize = 0;
  getBoxMT(L);
  lua_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#ವ್ಯಾಖ್ಯಾನಿಸು buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು checkbufferlevel(B,idx)  \
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ಖಾಲಿ  \
                            : lua_touserdata(B->L, idx) == (ಶೂನ್ಯ*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
ಸ್ಥಿರ size_t newbuffsize (lua_L_ಬಫರ್ *B, size_t sz) {
  size_t newsize = B->ಗಾತ್ರ_2;
  ಆದರೆ (l_unlikely(sz >= MAX_ಗಾತ್ರ - B->n))
    ಹಿಂತಿರುಗಿ cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  ಆದರೆ (newsize <= MAX_ಗಾತ್ರ/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  ಆದರೆ (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  ಹಿಂತಿರುಗಿ newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
ಸ್ಥಿರ ಅಕ್ಷರ *prepbuffsize (lua_L_ಬಫರ್ *B, size_t sz, ಪೂರ್ಣಾಂಕ boxidx) {
  checkbufferlevel(B, boxidx);
  ಆದರೆ (B->ಗಾತ್ರ_2 - B->n >= sz)  /* enough space? */
    ಹಿಂತಿರುಗಿ B->b + B->n;
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_State *L = B->L;
    ಅಕ್ಷರ *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    ಆದರೆ (buffonstack(B))  /* buffer already has a box? */
      newbuff = (ಅಕ್ಷರ *)resizebox(L, boxidx, newsize);  /* resize it */
    ಇಲ್ಲದಿದ್ದರೆ {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_ಸೇರಿಸು(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (ಅಕ್ಷರ *)resizebox(L, boxidx, newsize);
      ಸ್ಮರಣೆ_ನಕಲು(newbuff, B->b, B->n * ಗಾತ್ರ(ಅಕ್ಷರ));  /* copy original content */
    }
    B->b = newbuff;
    B->ಗಾತ್ರ_2 = newsize;
    ಹಿಂತಿರುಗಿ newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API ಅಕ್ಷರ *luaL_prepbuffsize (lua_L_ಬಫರ್ *B, size_t sz) {
  ಹಿಂತಿರುಗಿ prepbuffsize(B, sz, -1);
}


LUALIB_API ಶೂನ್ಯ luaL_addlstring (lua_L_ಬಫರ್ *B, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t l) {
  ಆದರೆ (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    ಅಕ್ಷರ *b = prepbuffsize(B, l, -1);
    ಸ್ಮರಣೆ_ನಕಲು(b, s, l * ಗಾತ್ರ(ಅಕ್ಷರ));
    luaL_addsize(B, l);
  }
}


LUALIB_API ಶೂನ್ಯ luaL_addstring (lua_L_ಬಫರ್ *B, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s) {
  luaL_addlstring(B, s, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s));
}


LUALIB_API ಶೂನ್ಯ luaL_pushresult (lua_L_ಬಫರ್ *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  ಆದರೆ (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  ಇಲ್ಲದಿದ್ದರೆ {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    ಶೂನ್ಯ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    ಅಕ್ಷರ *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (ಅಕ್ಷರ*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ಖಾಲಿ;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API ಶೂನ್ಯ luaL_pushresultsize (lua_L_ಬಫರ್ *B, size_t sz) {
  luaL_addsize(B, sz);
  luaL_pushresult(B);
}


/*
** 'luaL_addvalue' is the only function in the Buffer system where the
** box (if existent) is not on the top of the stack. So, instead of
** calling 'luaL_addlstring', it replicates the code using -2 as the
** last argument to 'prepbuffsize', signaling that the box is (or will
** be) below the string being added to the buffer. (Box creation can
** trigger an emergency GC, so we should not remove the string from the
** stack before we have the space guaranteed.)
*/
LUALIB_API ಶೂನ್ಯ luaL_addvalue (lua_L_ಬಫರ್ *B) {
  lua_State *L = B->L;
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_tolstring(L, -1, &len);
  ಅಕ್ಷರ *b = prepbuffsize(B, len, -2);
  ಸ್ಮರಣೆ_ನಕಲು(b, s, len * ಗಾತ್ರ(ಅಕ್ಷರ));
  luaL_addsize(B, len);
  lua_ತೆಗೆ(L, 1);  /* pop string */
}


LUALIB_API ಶೂನ್ಯ luaL_buffinit (lua_State *L, lua_L_ಬಫರ್ *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->ಗಾತ್ರ_2 = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (ಶೂನ್ಯ*)B);  /* push placeholder */
}


LUALIB_API ಅಕ್ಷರ *luaL_buffinitsize (lua_State *L, lua_L_ಬಫರ್ *B, size_t sz) {
  luaL_buffinit(L, B);
  ಹಿಂತಿರುಗಿ prepbuffsize(B, sz, -1);
}

/* }====================================================== */


/*
** {======================================================
** Reference system
** =======================================================
*/

/*
** The previously freed references form a linked list: t[1] is the index
** of a first free index, t[t[1]] is the index of the second element,
** etc. A zero signals the end of the list.
*/
LUALIB_API ಪೂರ್ಣಾಂಕ luaL_ref (lua_State *L, ಪೂರ್ಣಾಂಕ t) {
  ಪೂರ್ಣಾಂಕ ref;
  ಆದರೆ (lua_isnil(L, -1)) {
    lua_ತೆಗೆ(L, 1);  /* remove from stack */
    ಹಿಂತಿರುಗಿ LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  ಆದರೆ (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (ಪೂರ್ಣಾಂಕ)lua_tointeger(L, -1);  /* ref = t[1] */
  ಇಲ್ಲದಿದ್ದರೆ {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_ತೆಗೆ(L, 1);  /* remove element from stack */
  ಆದರೆ (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  ಇಲ್ಲದಿದ್ದರೆ  /* no free elements */
    ref = (ಪೂರ್ಣಾಂಕ)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  ಹಿಂತಿರುಗಿ ref;
}


LUALIB_API ಶೂನ್ಯ luaL_unref (lua_State *L, ಪೂರ್ಣಾಂಕ t, ಪೂರ್ಣಾಂಕ ref) {
  ಆದರೆ (ref >= 0) {
    t = lua_absindex(L, t);
    lua_rawgeti(L, t, 1);
    lua_assert(lua_isinteger(L, -1));
    lua_rawseti(L, t, ref);  /* t[ref] = t[1] */
    lua_pushinteger(L, ref);
    lua_rawseti(L, t, 1);  /* t[1] = ref */
  }
}

/* }====================================================== */


/*
** {======================================================
** Load functions
** =======================================================
*/

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ತುಂಬು_F {
  ಚಿಹ್ನೆರಹಿತ n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  ಅಕ್ಷರ buff[BUFSIZ];  /* area for reading file */
} ತುಂಬು_F;


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getF (lua_State *L, ಶೂನ್ಯ *ud, size_t *ಗಾತ್ರ_2) {
  ತುಂಬು_F *lf = (ತುಂಬು_F *)ud;
  ಬಳಸದ(L);
  ಆದರೆ (lf->n > 0) {  /* are there pre-read characters to be read? */
    *ಗಾತ್ರ_2 = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    ಆದರೆ (feof(lf->f)) ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    *ಗಾತ್ರ_2 = fread(lf->buff, 1, ಗಾತ್ರ(lf->buff), lf->f);  /* read block */
  }
  ಹಿಂತಿರುಗಿ lf->buff;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ errfile (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what, ಪೂರ್ಣಾಂಕ fnameindex) {
  ಪೂರ್ಣಾಂಕ err = errno;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename = lua_tostring(L, fnameindex) + 1;
  ಆದರೆ (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  ಇಲ್ಲದಿದ್ದರೆ
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  ಹಿಂತಿರುಗಿ LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ skipBOM (FILE *f) {
  ಪೂರ್ಣಾಂಕ c = getc(f);  /* read first character */
  ಆದರೆ (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    ಹಿಂತಿರುಗಿ getc(f);  /* ignore BOM and return next char */
  ಇಲ್ಲದಿದ್ದರೆ  /* no (valid) BOM */
    ಹಿಂತಿರುಗಿ c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ skipcomment (FILE *f, ಪೂರ್ಣಾಂಕ *cp) {
  ಪೂರ್ಣಾಂಕ c = *cp = skipBOM(f);
  ಆದರೆ (c == '#') {  /* first line is a comment (Unix exec. file)? */
    ಮಾಡು {  /* skip first line */
      c = getc(f);
    } ಆಗ (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    ಹಿಂತಿರುಗಿ 1;  /* there was a comment */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;  /* no comment */
}


LUALIB_API ಪೂರ್ಣಾಂಕ luaL_loadfilex (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
                                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
  ತುಂಬು_F lf;
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, readstatus;
  ಪೂರ್ಣಾಂಕ c;
  ಪೂರ್ಣಾಂಕ fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ಆದರೆ (filename == ಖಾಲಿ) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    ಆದರೆ (lf.f == ಖಾಲಿ) ಹಿಂತಿರುಗಿ errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  ಆದರೆ (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  ಆದರೆ (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    ಆದರೆ (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      ಆದರೆ (lf.f == ಖಾಲಿ) ಹಿಂತಿರುಗಿ errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  ಆದರೆ (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  ಸ್ಥಿತಿ_2 = lua_ತುಂಬು(L, getF, &lf, lua_tostring(L, -1), ಬಹುಲಕ);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  ಆದರೆ (filename) fclose(lf.f);  /* close file (even in case of errors) */
  ಆದರೆ (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    ಹಿಂತಿರುಗಿ errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ತುಂಬು_S {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
  size_t ಗಾತ್ರ_2;
} ತುಂಬು_S;


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getS (lua_State *L, ಶೂನ್ಯ *ud, size_t *ಗಾತ್ರ_2) {
  ತುಂಬು_S *ls = (ತುಂಬು_S *)ud;
  ಬಳಸದ(L);
  ಆದರೆ (ls->ಗಾತ್ರ_2 == 0) ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  *ಗಾತ್ರ_2 = ls->ಗಾತ್ರ_2;
  ls->ಗಾತ್ರ_2 = 0;
  ಹಿಂತಿರುಗಿ ls->s;
}


LUALIB_API ಪೂರ್ಣಾಂಕ luaL_loadbufferx (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buff, size_t ಗಾತ್ರ_2,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ) {
  ತುಂಬು_S ls;
  ls.s = buff;
  ls.ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
  ಹಿಂತಿರುಗಿ lua_ತುಂಬು(L, getS, &ls, name, ಬಹುಲಕ);
}


LUALIB_API ಪೂರ್ಣಾಂಕ luaL_loadstring (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s) {
  ಹಿಂತಿರುಗಿ luaL_loadbufferx(L, s, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s), s, "t");
}

/* }====================================================== */



LUALIB_API ಪೂರ್ಣಾಂಕ luaL_getmetafield (lua_State *L, ಪೂರ್ಣಾಂಕ obj, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *event) {
  ಆದರೆ (!lua_getmetatable(L, obj))  /* no metatable? */
    ಹಿಂತಿರುಗಿ LUA_TNIL;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    ಆದರೆ (tt == LUA_TNIL)  /* is metafield nil? */
      lua_ತೆಗೆ(L, 2);  /* remove metatable and metafield */
    ಇಲ್ಲದಿದ್ದರೆ
      lua_remove(L, -2);  /* remove only metatable */
    ಹಿಂತಿರುಗಿ tt;  /* return metafield type */
  }
}


LUALIB_API ಪೂರ್ಣಾಂಕ luaL_callmeta (lua_State *L, ಪೂರ್ಣಾಂಕ obj, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *event) {
  obj = lua_absindex(L, obj);
  ಆದರೆ (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    ಹಿಂತಿರುಗಿ 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  ಹಿಂತಿರುಗಿ 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, ಪೂರ್ಣಾಂಕ idx) {
  lua_Integer l;
  ಪೂರ್ಣಾಂಕ isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  ಆದರೆ (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_ತೆಗೆ(L, 1);  /* remove object */
  ಹಿಂತಿರುಗಿ l;
}


LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaL_tolstring (lua_State *L, ಪೂರ್ಣಾಂಕ idx, size_t *len) {
  idx = lua_absindex(L,idx);
  ಆದರೆ (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    ಆದರೆ (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆಯ್ಕೆ (lua_type(L, idx)) {
      ಸ್ಥಿತಿ LUA_TNUMBER: {
        ಅಕ್ಷರ buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ LUA_TSTRING:
        lua_pushvalue(L, idx);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_TNIL:
        lua_pushliteral(L, "nil");
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಪೂರ್ಣಾಂಕ tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಗೆ = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", ಬಗೆ, lua_topointer(L, idx));
        ಆದರೆ (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಹಿಂತಿರುಗಿ lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API ಶೂನ್ಯ luaL_setfuncs (lua_State *L, ಸ್ಥಿರಾಂಕ luaL_Reg *l, ಪೂರ್ಣಾಂಕ nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  ಗಾಗಿ (; l->name != ಖಾಲಿ; l++) {  /* fill the table with given functions */
    ಆದರೆ (l->func == ಖಾಲಿ)  /* placeholder? */
      lua_pushboolean(L, 0);
    ಇಲ್ಲದಿದ್ದರೆ {
      ಪೂರ್ಣಾಂಕ i;
      ಗಾಗಿ (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_ತೆಗೆ(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API ಪೂರ್ಣಾಂಕ luaL_getsubtable (lua_State *L, ಪೂರ್ಣಾಂಕ idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fname) {
  ಆದರೆ (lua_getfield(L, idx, fname) == LUA_TTABLE)
    ಹಿಂತಿರುಗಿ 1;  /* table already there */
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_ತೆಗೆ(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    ಹಿಂತಿರುಗಿ 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API ಶೂನ್ಯ luaL_requiref (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *modname,
                               lua_CFunction openf, ಪೂರ್ಣಾಂಕ glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_ಕೋಷ್ಟಕ);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  ಆದರೆ (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_ತೆಗೆ(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  ಆದರೆ (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API ಶೂನ್ಯ luaL_addgsub (lua_L_ಬಫರ್ *b, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *r) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *wild;
  size_t l = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p);
  ಆಗ ((wild = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(s, p)) != ಖಾಲಿ) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaL_gsub (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
                                  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *r) {
  lua_L_ಬಫರ್ b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  ಹಿಂತಿರುಗಿ lua_tostring(L, -1);
}


ಶೂನ್ಯ *luaL_alloc (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize, size_t nsize) {
  ಬಳಸದ(ud); ಬಳಸದ(osize);
  ಆದರೆ (nsize == 0) {
    ಬಿಡುಗಡೆ(ptr);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ panic (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        ಸಂದೇಶ);
  ಹಿಂತಿರುಗಿ 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
ಸ್ಥಿರ ಶೂನ್ಯ warnfoff (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ಪೂರ್ಣಾಂಕ tocont);
ಸ್ಥಿರ ಶೂನ್ಯ warnfon (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ಪೂರ್ಣಾಂಕ tocont);
ಸ್ಥಿರ ಶೂನ್ಯ warnfcont (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ಪೂರ್ಣಾಂಕ tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkcontrol (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ಪೂರ್ಣಾಂಕ tocont) {
  ಆದರೆ (tocont || *(ಸಂದೇಶ_2++) != '@')  /* not a control message? */
    ಹಿಂತಿರುಗಿ 0;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (strcmp(ಸಂದೇಶ_2, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ಸಂದೇಶ_2, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    ಹಿಂತಿರುಗಿ 1;  /* it was a control message */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ warnfoff (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ಪೂರ್ಣಾಂಕ tocont) {
  checkcontrol((lua_State *)ud, ಸಂದೇಶ_2, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ warnfcont (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ಪೂರ್ಣಾಂಕ tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", ಸಂದೇಶ_2);  /* write message */
  ಆದರೆ (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  ಇಲ್ಲದಿದ್ದರೆ {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ warnfon (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ_2, ಪೂರ್ಣಾಂಕ tocont) {
  ಆದರೆ (checkcontrol((lua_State *)ud, ಸಂದೇಶ_2, tocont))  /* control message? */
    ಹಿಂತಿರುಗಿ;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, ಸಂದೇಶ_2, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#ಆದರೆ !defined(luai_makeseed)

#ಅನ್ತರ್ಭಾವ <time.h>


/* Size for the buffer, in bytes */
#ವ್ಯಾಖ್ಯಾನಿಸು BUFSEEDB	(ಗಾತ್ರ(ಶೂನ್ಯ*) + ಗಾತ್ರ(time_t))

/* Size for the buffer in int's, rounded up */
#ವ್ಯಾಖ್ಯಾನಿಸು BUFSEED		((BUFSEEDB + ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) - 1) / ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು addbuff(b,v)	(ಸ್ಮರಣೆ_ನಕಲು(&b[0], &(v), ಗಾತ್ರ(v)), b += ಗಾತ್ರ(v))


ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ luai_makeseed (ಶೂನ್ಯ) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ buff[BUFSEED];
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ res;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  time_t t = time(ಖಾಲಿ);
  ಅಕ್ಷರ *b = (ಅಕ್ಷರ*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /*ಸಮಯ*/
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, ಗಾತ್ರ(buff) - BUFSEEDB);
  res = buff[0];
  ಗಾಗಿ (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  ಹಿಂತಿರುಗಿ res;
}

#ಆದರೆ_ಅಂತ್ಯ


LUALIB_API ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ luaL_makeseed (lua_State *L) {
  ಬಳಸದ(L);
  ಹಿಂತಿರುಗಿ luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (ಶೂನ್ಯ) {
  lua_State *L = lua_newstate(luaL_alloc, ಖಾಲಿ, luaL_makeseed(ಖಾಲಿ));
  ಆದರೆ (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  ಹಿಂತಿರುಗಿ L;
}


LUALIB_API ಶೂನ್ಯ luaL_checkversion_ (lua_State *L, lua_ಸಂಖ್ಯೆ ver, size_t sz) {
  lua_ಸಂಖ್ಯೆ v = lua_ಆವೃತ್ತಿ(L);
  ಆದರೆ (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

