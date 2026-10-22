/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

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


#define LEVELS1	10	/* size of the first part of the stack */
#define LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ findfield (lua_State *L, ಪೂರ್ಣಾಙ್ಕ objidx, ಪೂರ್ಣಾಙ್ಕ level) {
  ಯದಿ (level == 0 || !lua_istable(L, -1))
    ಪ್ರತ್ಯಾಗಮ 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  ಯಾವತ್ (lua_next(L, -2)) {  /* for each pair in table */
    ಯದಿ (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      ಯದಿ (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_ತೆಗೆ(L, 1);  /* remove value (but keep name) */
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಅನ್ಯಥಾ ಯದಿ (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        ಪ್ರತ್ಯಾಗಮ 1;
      }
    }
    lua_ತೆಗೆ(L, 1);  /* remove value */
  }
  ಪ್ರತ್ಯಾಗಮ 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  ಪೂರ್ಣಾಙ್ಕ ತುದಿ = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  ಯದಿ (findfield(L, ತುದಿ + 1, 2)) {
    ಧ್ರುವ ವರ್ಣ *name = lua_tostring(L, -1);
    ಯದಿ (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, ತುದಿ + 1);  /* copy name to proper place */
    lua_settop(L, ತುದಿ + 1);  /* remove table "loaded" and name copy */
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ {
    lua_settop(L, ತುದಿ);  /* remove function and global table */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ pushfuncname (lua_State *L, lua_Debug *ar) {
  ಯದಿ (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  ಅನ್ಯಥಾ ಯದಿ (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  ಅನ್ಯಥಾ ಯದಿ (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  ಅನ್ಯಥಾ ಯದಿ (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  ಅನ್ಯಥಾ  /* nothing left... */
    lua_pushliteral(L, "?");
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ lastlevel (lua_State *L) {
  lua_Debug ar;
  ಪೂರ್ಣಾಙ್ಕ li = 1, le = 1;
  /* find an upper bound */
  ಯಾವತ್ (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  ಯಾವತ್ (li < le) {
    ಪೂರ್ಣಾಙ್ಕ m = (li + le)/2;
    ಯದಿ (lua_getstack(L, m, &ar)) li = m + 1;
    ಅನ್ಯಥಾ le = m;
  }
  ಪ್ರತ್ಯಾಗಮ le - 1;
}


LUALIB_API ಶೂನ್ಯ luaL_traceback (lua_State *L, lua_State *L1,
                                ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ, ಪೂರ್ಣಾಙ್ಕ level) {
  luaL_Buffer b;
  lua_Debug ar;
  ಪೂರ್ಣಾಙ್ಕ last = lastlevel(L1);
  ಪೂರ್ಣಾಙ್ಕ limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  ಯದಿ (ಸಂದೇಶ) {
    luaL_addstring(&b, ಸಂದೇಶ);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  ಯಾವತ್ (lua_getstack(L1, level++, &ar)) {
    ಯದಿ (limit2show-- == 0) {  /* too many levels? */
      ಪೂರ್ಣಾಙ್ಕ n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    ಅನ್ಯಥಾ {
      lua_getinfo(L1, "Slnt", &ar);
      ಯದಿ (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      ಅನ್ಯಥಾ
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      ಯದಿ (ar.istailcall)
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

LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_argerror (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಧ್ರುವ ವರ್ಣ *extramsg) {
  lua_Debug ar;
  ಧ್ರುವ ವರ್ಣ *argword;
  ಯದಿ (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  ಯದಿ (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  ಅನ್ಯಥಾ {
    arg -= ar.extraargs;  /* do not count extra arguments */
    ಯದಿ (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      ಯದಿ (arg == 0)  /* error in self argument? */
        ಪ್ರತ್ಯಾಗಮ luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  ಯದಿ (ar.name == ರಿಕ್ತ)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  ಪ್ರತ್ಯಾಗಮ luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_typeerror (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಧ್ರುವ ವರ್ಣ *tname) {
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ;
  ಧ್ರುವ ವರ್ಣ *typearg;  /* name for the type of the actual argument */
  ಯದಿ (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  ಅನ್ಯಥಾ ಯದಿ (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  ಅನ್ಯಥಾ
    typearg = luaL_typename(L, arg);  /* standard name */
  ಸಂದೇಶ = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  ಪ್ರತ್ಯಾಗಮ luaL_argerror(L, arg, ಸಂದೇಶ);
}


ಸ್ಥಿರ ಶೂನ್ಯ tag_error (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಪೂರ್ಣಾಙ್ಕ tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API ಶೂನ್ಯ luaL_where (lua_State *L, ಪೂರ್ಣಾಙ್ಕ level) {
  lua_Debug ar;
  ಯದಿ (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    ಯದಿ (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_error (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  ಪ್ರತ್ಯಾಗಮ lua_error(L);
}


LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_fileresult (lua_State *L, ಪೂರ್ಣಾಙ್ಕ stat, ಧ್ರುವ ವರ್ಣ *fname) {
  ಪೂರ್ಣಾಙ್ಕ en = errno;  /* calls to Lua API may change this value */
  ಯದಿ (stat) {
    lua_pushboolean(L, 1);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ;
    luaL_pushfail(L);
    ಸಂದೇಶ = (en != 0) ? strerror(en) : "(no extra info)";
    ಯದಿ (fname)
      lua_pushfstring(L, "%s: %s", fname, ಸಂದೇಶ);
    ಅನ್ಯಥಾ
      lua_pushstring(L, ಸಂದೇಶ);
    lua_pushinteger(L, en);
    ಪ್ರತ್ಯಾಗಮ 3;
  }
}


#ಯದಿ !defined(l_inspectstat)	/* { */

#ಯದಿ defined(LUA_USE_POSIX)

#ಅನ್ತರ್ಭಾವ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   ಯದಿ (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   ಅನ್ಯಥಾ ಯದಿ (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#ಅನ್ಯಥಾ

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_execresult (lua_State *L, ಪೂರ್ಣಾಙ್ಕ stat) {
  ಯದಿ (stat != 0 && errno != 0)  /* error with an 'errno'? */
    ಪ್ರತ್ಯಾಗಮ luaL_fileresult(L, 0, ರಿಕ್ತ);
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    ಯದಿ (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    ಅನ್ಯಥಾ
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    ಪ್ರತ್ಯಾಗಮ 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_newmetatable (lua_State *L, ಧ್ರುವ ವರ್ಣ *tname) {
  ಯದಿ (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    ಪ್ರತ್ಯಾಗಮ 0;  /* leave previous value on top, but return 0 */
  lua_ತೆಗೆ(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  ಪ್ರತ್ಯಾಗಮ 1;
}


LUALIB_API ಶೂನ್ಯ luaL_setmetatable (lua_State *L, ಧ್ರುವ ವರ್ಣ *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API ಶೂನ್ಯ *luaL_testudata (lua_State *L, ಪೂರ್ಣಾಙ್ಕ ud, ಧ್ರುವ ವರ್ಣ *tname) {
  ಶೂನ್ಯ *p = lua_touserdata(L, ud);
  ಯದಿ (p != ರಿಕ್ತ) {  /* value is a userdata? */
    ಯದಿ (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      ಯದಿ (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ರಿಕ್ತ;  /* value is a userdata with wrong metatable */
      lua_ತೆಗೆ(L, 2);  /* remove both metatables */
      ಪ್ರತ್ಯಾಗಮ p;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* value is not a userdata with a metatable */
}


LUALIB_API ಶೂನ್ಯ *luaL_checkudata (lua_State *L, ಪೂರ್ಣಾಙ್ಕ ud, ಧ್ರುವ ವರ್ಣ *tname) {
  ಶೂನ್ಯ *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ರಿಕ್ತ, ud, tname);
  ಪ್ರತ್ಯಾಗಮ p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_checkoption (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಧ್ರುವ ವರ್ಣ *def,
                                 ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ lst[]) {
  ಧ್ರುವ ವರ್ಣ *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i=0; lst[i]; i++)
    ಯದಿ (strcmp(lst[i], name) == 0)
      ಪ್ರತ್ಯಾಗಮ i;
  ಪ್ರತ್ಯಾಗಮ luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API ಶೂನ್ಯ luaL_checkstack (lua_State *L, ಪೂರ್ಣಾಙ್ಕ space, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) {
  ಯದಿ (l_unlikely(!lua_checkstack(L, space))) {
    ಯದಿ (ಸಂದೇಶ)
      luaL_error(L, "stack overflow (%s)", ಸಂದೇಶ);
    ಅನ್ಯಥಾ
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API ಶೂನ್ಯ luaL_checktype (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, ಪೂರ್ಣಾಙ್ಕ t) {
  ಯದಿ (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API ಶೂನ್ಯ luaL_checkany (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg) {
  ಯದಿ (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ಧ್ರುವ ವರ್ಣ *luaL_checklstring (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, size_t *len) {
  ಧ್ರುವ ವರ್ಣ *s = lua_tolstring(L, arg, len);
  ಯದಿ (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  ಪ್ರತ್ಯಾಗಮ s;
}


LUALIB_API ಧ್ರುವ ವರ್ಣ *luaL_optlstring (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg,
                                        ಧ್ರುವ ವರ್ಣ *def, size_t *len) {
  ಯದಿ (lua_isnoneornil(L, arg)) {
    ಯದಿ (len)
      *len = (def ? strlen(def) : 0);
    ಪ್ರತ್ಯಾಗಮ def;
  }
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ luaL_checklstring(L, arg, len);
}


LUALIB_API lua_ಸಂಖ್ಯೆ luaL_checknumber (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg) {
  ಪೂರ್ಣಾಙ್ಕ isnum;
  lua_ಸಂಖ್ಯೆ d = lua_tonumberx(L, arg, &isnum);
  ಯದಿ (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  ಪ್ರತ್ಯಾಗಮ d;
}


LUALIB_API lua_ಸಂಖ್ಯೆ luaL_optnumber (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg, lua_ಸಂಖ್ಯೆ def) {
  ಪ್ರತ್ಯಾಗಮ luaL_opt(L, luaL_checknumber, arg, def);
}


ಸ್ಥಿರ ಶೂನ್ಯ interror (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg) {
  ಯದಿ (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  ಅನ್ಯಥಾ
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg) {
  ಪೂರ್ಣಾಙ್ಕ isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  ಯದಿ (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  ಪ್ರತ್ಯಾಗಮ d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, ಪೂರ್ಣಾಙ್ಕ arg,
                                                      lua_Integer def) {
  ಪ್ರತ್ಯಾಗಮ luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ UBox {
  ಶೂನ್ಯ *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ *resizebox (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  ಯದಿ (box->bsize == newsize)  /* not changing size? */
    ಪ್ರತ್ಯಾಗಮ box->box;  /* keep the buffer */
  ಅನ್ಯಥಾ {
    ಶೂನ್ಯ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    ಶೂನ್ಯ *temp = allocf(ud, box->box, box->bsize, newsize);
    ಯದಿ (l_unlikely(temp == ರಿಕ್ತ && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    ಪ್ರತ್ಯಾಗಮ temp;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


/*
** Get/create metatable (MT) for boxes
*/
ಸ್ಥಿರ ಶೂನ್ಯ getBoxMT (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *BOXMT = "_UBOX*"; /* key for the metatable */
  ಯದಿ (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, ಪರಿಮಾಣ(UBox), 0);
  box->box = ರಿಕ್ತ;
  box->bsize = 0;
  getBoxMT(L);
  lua_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#define buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#define checkbufferlevel(B,idx)  \
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ರಿಕ್ತ  \
                            : lua_touserdata(B->L, idx) == (ಶೂನ್ಯ*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
ಸ್ಥಿರ size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  ಯದಿ (l_unlikely(sz >= MAX_SIZE - B->n))
    ಪ್ರತ್ಯಾಗಮ cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  ಯದಿ (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  ಯದಿ (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  ಪ್ರತ್ಯಾಗಮ newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
ಸ್ಥಿರ ವರ್ಣ *prepbuffsize (luaL_Buffer *B, size_t sz, ಪೂರ್ಣಾಙ್ಕ boxidx) {
  checkbufferlevel(B, boxidx);
  ಯದಿ (B->size - B->n >= sz)  /* enough space? */
    ಪ್ರತ್ಯಾಗಮ B->b + B->n;
  ಅನ್ಯಥಾ {
    lua_State *L = B->L;
    ವರ್ಣ *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    ಯದಿ (buffonstack(B))  /* buffer already has a box? */
      newbuff = (ವರ್ಣ *)resizebox(L, boxidx, newsize);  /* resize it */
    ಅನ್ಯಥಾ {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_ಸೇರಿಸು(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (ವರ್ಣ *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * ಪರಿಮಾಣ(ವರ್ಣ));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    ಪ್ರತ್ಯಾಗಮ newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API ವರ್ಣ *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  ಪ್ರತ್ಯಾಗಮ prepbuffsize(B, sz, -1);
}


LUALIB_API ಶೂನ್ಯ luaL_addlstring (luaL_Buffer *B, ಧ್ರುವ ವರ್ಣ *s, size_t l) {
  ಯದಿ (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    ವರ್ಣ *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * ಪರಿಮಾಣ(ವರ್ಣ));
    luaL_addsize(B, l);
  }
}


LUALIB_API ಶೂನ್ಯ luaL_addstring (luaL_Buffer *B, ಧ್ರುವ ವರ್ಣ *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API ಶೂನ್ಯ luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  ಯದಿ (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  ಅನ್ಯಥಾ {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    ಶೂನ್ಯ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    ವರ್ಣ *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (ವರ್ಣ*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ರಿಕ್ತ;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API ಶೂನ್ಯ luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API ಶೂನ್ಯ luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  ಧ್ರುವ ವರ್ಣ *s = lua_tolstring(L, -1, &len);
  ವರ್ಣ *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * ಪರಿಮಾಣ(ವರ್ಣ));
  luaL_addsize(B, len);
  lua_ತೆಗೆ(L, 1);  /* pop string */
}


LUALIB_API ಶೂನ್ಯ luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (ಶೂನ್ಯ*)B);  /* push placeholder */
}


LUALIB_API ವರ್ಣ *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  ಪ್ರತ್ಯಾಗಮ prepbuffsize(B, sz, -1);
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
LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_ref (lua_State *L, ಪೂರ್ಣಾಙ್ಕ t) {
  ಪೂರ್ಣಾಙ್ಕ ref;
  ಯದಿ (lua_isnil(L, -1)) {
    lua_ತೆಗೆ(L, 1);  /* remove from stack */
    ಪ್ರತ್ಯಾಗಮ LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  ಯದಿ (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (ಪೂರ್ಣಾಙ್ಕ)lua_tointeger(L, -1);  /* ref = t[1] */
  ಅನ್ಯಥಾ {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_ತೆಗೆ(L, 1);  /* remove element from stack */
  ಯದಿ (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  ಅನ್ಯಥಾ  /* no free elements */
    ref = (ಪೂರ್ಣಾಙ್ಕ)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  ಪ್ರತ್ಯಾಗಮ ref;
}


LUALIB_API ಶೂನ್ಯ luaL_unref (lua_State *L, ಪೂರ್ಣಾಙ್ಕ t, ಪೂರ್ಣಾಙ್ಕ ref) {
  ಯದಿ (ref >= 0) {
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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  ವರ್ಣ buff[BUFSIZ];  /* area for reading file */
} LoadF;


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getF (lua_State *L, ಶೂನ್ಯ *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  ಯದಿ (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  ಅನ್ಯಥಾ {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    ಯದಿ (feof(lf->f)) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    *size = fread(lf->buff, 1, ಪರಿಮಾಣ(lf->buff), lf->f);  /* read block */
  }
  ಪ್ರತ್ಯಾಗಮ lf->buff;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ errfile (lua_State *L, ಧ್ರುವ ವರ್ಣ *what, ಪೂರ್ಣಾಙ್ಕ fnameindex) {
  ಪೂರ್ಣಾಙ್ಕ err = errno;
  ಧ್ರುವ ವರ್ಣ *filename = lua_tostring(L, fnameindex) + 1;
  ಯದಿ (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  ಅನ್ಯಥಾ
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  ಪ್ರತ್ಯಾಗಮ LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ skipBOM (FILE *f) {
  ಪೂರ್ಣಾಙ್ಕ c = getc(f);  /* read first character */
  ಯದಿ (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    ಪ್ರತ್ಯಾಗಮ getc(f);  /* ignore BOM and return next char */
  ಅನ್ಯಥಾ  /* no (valid) BOM */
    ಪ್ರತ್ಯಾಗಮ c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ skipcomment (FILE *f, ಪೂರ್ಣಾಙ್ಕ *cp) {
  ಪೂರ್ಣಾಙ್ಕ c = *cp = skipBOM(f);
  ಯದಿ (c == '#') {  /* first line is a comment (Unix exec. file)? */
    ಕುರು {  /* skip first line */
      c = getc(f);
    } ಯಾವತ್ (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    ಪ್ರತ್ಯಾಗಮ 1;  /* there was a comment */
  }
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ 0;  /* no comment */
}


LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_loadfilex (lua_State *L, ಧ್ರುವ ವರ್ಣ *filename,
                                             ಧ್ರುವ ವರ್ಣ *mode) {
  LoadF lf;
  ಪೂರ್ಣಾಙ್ಕ status, readstatus;
  ಪೂರ್ಣಾಙ್ಕ c;
  ಪೂರ್ಣಾಙ್ಕ fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ಯದಿ (filename == ರಿಕ್ತ) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  ಅನ್ಯಥಾ {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    ಯದಿ (lf.f == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  ಯದಿ (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  ಯದಿ (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    ಯದಿ (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      ಯದಿ (lf.f == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  ಯದಿ (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  ಯದಿ (filename) fclose(lf.f);  /* close file (even in case of errors) */
  ಯದಿ (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    ಪ್ರತ್ಯಾಗಮ errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  ಪ್ರತ್ಯಾಗಮ status;
}


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ LoadS {
  ಧ್ರುವ ವರ್ಣ *s;
  size_t size;
} LoadS;


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getS (lua_State *L, ಶೂನ್ಯ *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  ಯದಿ (ls->size == 0) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  *size = ls->size;
  ls->size = 0;
  ಪ್ರತ್ಯಾಗಮ ls->s;
}


LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_loadbufferx (lua_State *L, ಧ್ರುವ ವರ್ಣ *buff, size_t size,
                                 ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  ಪ್ರತ್ಯಾಗಮ lua_load(L, getS, &ls, name, mode);
}


LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_loadstring (lua_State *L, ಧ್ರುವ ವರ್ಣ *s) {
  ಪ್ರತ್ಯಾಗಮ luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_getmetafield (lua_State *L, ಪೂರ್ಣಾಙ್ಕ obj, ಧ್ರುವ ವರ್ಣ *event) {
  ಯದಿ (!lua_getmetatable(L, obj))  /* no metatable? */
    ಪ್ರತ್ಯಾಗಮ LUA_TNIL;
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    ಯದಿ (tt == LUA_TNIL)  /* is metafield nil? */
      lua_ತೆಗೆ(L, 2);  /* remove metatable and metafield */
    ಅನ್ಯಥಾ
      lua_remove(L, -2);  /* remove only metatable */
    ಪ್ರತ್ಯಾಗಮ tt;  /* return metafield type */
  }
}


LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_callmeta (lua_State *L, ಪೂರ್ಣಾಙ್ಕ obj, ಧ್ರುವ ವರ್ಣ *event) {
  obj = lua_absindex(L, obj);
  ಯದಿ (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    ಪ್ರತ್ಯಾಗಮ 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  ಪ್ರತ್ಯಾಗಮ 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx) {
  lua_Integer l;
  ಪೂರ್ಣಾಙ್ಕ isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  ಯದಿ (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_ತೆಗೆ(L, 1);  /* remove object */
  ಪ್ರತ್ಯಾಗಮ l;
}


LUALIB_API ಧ್ರುವ ವರ್ಣ *luaL_tolstring (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, size_t *len) {
  idx = lua_absindex(L,idx);
  ಯದಿ (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    ಯದಿ (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  ಅನ್ಯಥಾ {
    ವಿಕಲ್ಪ (lua_type(L, idx)) {
      ಪಕ್ಷ LUA_TNUMBER: {
        ವರ್ಣ buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ LUA_TSTRING:
        lua_pushvalue(L, idx);
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_TNIL:
        lua_pushliteral(L, "nil");
        ಭಙ್ಗ;
      ಸ್ವಭಾವ: {
        ಪೂರ್ಣಾಙ್ಕ tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ಧ್ರುವ ವರ್ಣ *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        ಯದಿ (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        ಭಙ್ಗ;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API ಶೂನ್ಯ luaL_setfuncs (lua_State *L, ಧ್ರುವ luaL_Reg *l, ಪೂರ್ಣಾಙ್ಕ nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  ಪ್ರತಿ (; l->name != ರಿಕ್ತ; l++) {  /* fill the table with given functions */
    ಯದಿ (l->func == ರಿಕ್ತ)  /* placeholder? */
      lua_pushboolean(L, 0);
    ಅನ್ಯಥಾ {
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪ್ರತಿ (i = 0; i < nup; i++)  /* copy upvalues to the top */
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
LUALIB_API ಪೂರ್ಣಾಙ್ಕ luaL_getsubtable (lua_State *L, ಪೂರ್ಣಾಙ್ಕ idx, ಧ್ರುವ ವರ್ಣ *fname) {
  ಯದಿ (lua_getfield(L, idx, fname) == LUA_TTABLE)
    ಪ್ರತ್ಯಾಗಮ 1;  /* table already there */
  ಅನ್ಯಥಾ {
    lua_ತೆಗೆ(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    ಪ್ರತ್ಯಾಗಮ 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API ಶೂನ್ಯ luaL_requiref (lua_State *L, ಧ್ರುವ ವರ್ಣ *modname,
                               lua_CFunction openf, ಪೂರ್ಣಾಙ್ಕ glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  ಯದಿ (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_ತೆಗೆ(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  ಯದಿ (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API ಶೂನ್ಯ luaL_addgsub (luaL_Buffer *b, ಧ್ರುವ ವರ್ಣ *s,
                                     ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *r) {
  ಧ್ರುವ ವರ್ಣ *wild;
  size_t l = strlen(p);
  ಯಾವತ್ ((wild = strstr(s, p)) != ರಿಕ್ತ) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ಧ್ರುವ ವರ್ಣ *luaL_gsub (lua_State *L, ಧ್ರುವ ವರ್ಣ *s,
                                  ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  ಪ್ರತ್ಯಾಗಮ lua_tostring(L, -1);
}


ಶೂನ್ಯ *luaL_alloc (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  ಯದಿ (nsize == 0) {
    ಮುಕ್ತ(ptr);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ panic (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        ಸಂದೇಶ);
  ಪ್ರತ್ಯಾಗಮ 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
ಸ್ಥಿರ ಶೂನ್ಯ warnfoff (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *message, ಪೂರ್ಣಾಙ್ಕ tocont);
ಸ್ಥಿರ ಶೂನ್ಯ warnfon (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *message, ಪೂರ್ಣಾಙ್ಕ tocont);
ಸ್ಥಿರ ಶೂನ್ಯ warnfcont (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *message, ಪೂರ್ಣಾಙ್ಕ tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkcontrol (lua_State *L, ಧ್ರುವ ವರ್ಣ *message, ಪೂರ್ಣಾಙ್ಕ tocont) {
  ಯದಿ (tocont || *(message++) != '@')  /* not a control message? */
    ಪ್ರತ್ಯಾಗಮ 0;
  ಅನ್ಯಥಾ {
    ಯದಿ (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    ಅನ್ಯಥಾ ಯದಿ (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    ಪ್ರತ್ಯಾಗಮ 1;  /* it was a control message */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ warnfoff (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *message, ಪೂರ್ಣಾಙ್ಕ tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
ಸ್ಥಿರ ಶೂನ್ಯ warnfcont (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *message, ಪೂರ್ಣಾಙ್ಕ tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  ಯದಿ (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  ಅನ್ಯಥಾ {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ warnfon (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *message, ಪೂರ್ಣಾಙ್ಕ tocont) {
  ಯದಿ (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    ಪ್ರತ್ಯಾಗಮ;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#ಯದಿ !defined(luai_makeseed)

#ಅನ್ತರ್ಭಾವ <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(ಪರಿಮಾಣ(ಶೂನ್ಯ*) + ಪರಿಮಾಣ(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ) - 1) / ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), ಪರಿಮಾಣ(v)), b += ಪರಿಮಾಣ(v))


ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ luai_makeseed (ಶೂನ್ಯ) {
  unsigned ಪೂರ್ಣಾಙ್ಕ buff[BUFSEED];
  unsigned ಪೂರ್ಣಾಙ್ಕ res;
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  time_t t = time(ರಿಕ್ತ);
  ವರ್ಣ *b = (ವರ್ಣ*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, ಪರಿಮಾಣ(buff) - BUFSEEDB);
  res = buff[0];
  ಪ್ರತಿ (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  ಪ್ರತ್ಯಾಗಮ res;
}

#endif


LUALIB_API unsigned ಪೂರ್ಣಾಙ್ಕ luaL_makeseed (lua_State *L) {
  UNUSED(L);
  ಪ್ರತ್ಯಾಗಮ luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (ಶೂನ್ಯ) {
  lua_State *L = lua_newstate(luaL_alloc, ರಿಕ್ತ, luaL_makeseed(ರಿಕ್ತ));
  ಯದಿ (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  ಪ್ರತ್ಯಾಗಮ L;
}


LUALIB_API ಶೂನ್ಯ luaL_checkversion_ (lua_State *L, lua_ಸಂಖ್ಯೆ ver, size_t sz) {
  lua_ಸಂಖ್ಯೆ v = lua_version(L);
  ಯದಿ (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  ಅನ್ಯಥಾ ಯದಿ (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

