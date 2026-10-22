/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "llimits.h"


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
സ്ഥിതം പൂർണ്ണസംഖ്യ findfield (lua_State *L, പൂർണ്ണസംഖ്യ objidx, പൂർണ്ണസംഖ്യ level) {
  എങ്കിൽ (level == 0 || !lua_istable(L, -1))
    തിരികെ 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  ആകുമ്പോൾ (lua_next(L, -2)) {  /* for each pair in table */
    എങ്കിൽ (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      എങ്കിൽ (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_എടുക്കുക(L, 1);  /* remove value (but keep name) */
        തിരികെ 1;
      }
      അല്ലെങ്കിൽ എങ്കിൽ (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        തിരികെ 1;
      }
    }
    lua_എടുക്കുക(L, 1);  /* remove value */
  }
  തിരികെ 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  പൂർണ്ണസംഖ്യ മുകൾ = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  എങ്കിൽ (findfield(L, മുകൾ + 1, 2)) {
    സ്ഥിരം അക്ഷരം *name = lua_tostring(L, -1);
    എങ്കിൽ (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, മുകൾ + 1);  /* copy name to proper place */
    lua_settop(L, മുകൾ + 1);  /* remove table "loaded" and name copy */
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ {
    lua_settop(L, മുകൾ);  /* remove function and global table */
    തിരികെ 0;
  }
}


സ്ഥിതം ശൂന്യം pushfuncname (lua_State *L, lua_Debug *ar) {
  എങ്കിൽ (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  അല്ലെങ്കിൽ എങ്കിൽ (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  അല്ലെങ്കിൽ എങ്കിൽ (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  അല്ലെങ്കിൽ  /* nothing left... */
    lua_pushliteral(L, "?");
}


സ്ഥിതം പൂർണ്ണസംഖ്യ lastlevel (lua_State *L) {
  lua_Debug ar;
  പൂർണ്ണസംഖ്യ li = 1, le = 1;
  /* find an upper bound */
  ആകുമ്പോൾ (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  ആകുമ്പോൾ (li < le) {
    പൂർണ്ണസംഖ്യ m = (li + le)/2;
    എങ്കിൽ (lua_getstack(L, m, &ar)) li = m + 1;
    അല്ലെങ്കിൽ le = m;
  }
  തിരികെ le - 1;
}


LUALIB_API ശൂന്യം luaL_traceback (lua_State *L, lua_State *L1,
                                സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ level) {
  luaL_Buffer b;
  lua_Debug ar;
  പൂർണ്ണസംഖ്യ last = lastlevel(L1);
  പൂർണ്ണസംഖ്യ limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  എങ്കിൽ (സന്ദേശം) {
    luaL_addstring(&b, സന്ദേശം);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  ആകുമ്പോൾ (lua_getstack(L1, level++, &ar)) {
    എങ്കിൽ (limit2show-- == 0) {  /* too many levels? */
      പൂർണ്ണസംഖ്യ n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    അല്ലെങ്കിൽ {
      lua_getinfo(L1, "Slnt", &ar);
      എങ്കിൽ (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      അല്ലെങ്കിൽ
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      എങ്കിൽ (ar.istailcall)
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

LUALIB_API പൂർണ്ണസംഖ്യ luaL_argerror (lua_State *L, പൂർണ്ണസംഖ്യ arg, സ്ഥിരം അക്ഷരം *extramsg) {
  lua_Debug ar;
  സ്ഥിരം അക്ഷരം *argword;
  എങ്കിൽ (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    തിരികെ luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  എങ്കിൽ (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  അല്ലെങ്കിൽ {
    arg -= ar.extraargs;  /* do not count extra arguments */
    എങ്കിൽ (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      എങ്കിൽ (arg == 0)  /* error in self argument? */
        തിരികെ luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  എങ്കിൽ (ar.name == ശൂന്യ)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  തിരികെ luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API പൂർണ്ണസംഖ്യ luaL_typeerror (lua_State *L, പൂർണ്ണസംഖ്യ arg, സ്ഥിരം അക്ഷരം *tname) {
  സ്ഥിരം അക്ഷരം *സന്ദേശം;
  സ്ഥിരം അക്ഷരം *typearg;  /* name for the type of the actual argument */
  എങ്കിൽ (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  അല്ലെങ്കിൽ എങ്കിൽ (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  അല്ലെങ്കിൽ
    typearg = luaL_typename(L, arg);  /* standard name */
  സന്ദേശം = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  തിരികെ luaL_argerror(L, arg, സന്ദേശം);
}


സ്ഥിതം ശൂന്യം tag_error (lua_State *L, പൂർണ്ണസംഖ്യ arg, പൂർണ്ണസംഖ്യ tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API ശൂന്യം luaL_where (lua_State *L, പൂർണ്ണസംഖ്യ level) {
  lua_Debug ar;
  എങ്കിൽ (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    എങ്കിൽ (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      തിരികെ;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API പൂർണ്ണസംഖ്യ luaL_error (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  തിരികെ lua_error(L);
}


LUALIB_API പൂർണ്ണസംഖ്യ luaL_fileresult (lua_State *L, പൂർണ്ണസംഖ്യ stat, സ്ഥിരം അക്ഷരം *fname) {
  പൂർണ്ണസംഖ്യ en = errno;  /* calls to Lua API may change this value */
  എങ്കിൽ (stat) {
    lua_pushboolean(L, 1);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *സന്ദേശം;
    luaL_pushfail(L);
    സന്ദേശം = (en != 0) ? strerror(en) : "(no extra info)";
    എങ്കിൽ (fname)
      lua_pushfstring(L, "%s: %s", fname, സന്ദേശം);
    അല്ലെങ്കിൽ
      lua_pushstring(L, സന്ദേശം);
    lua_pushinteger(L, en);
    തിരികെ 3;
  }
}


#എങ്കിൽ !defined(l_inspectstat)	/* { */

#എങ്കിൽ defined(LUA_USE_POSIX)

#അന്തര്ഭാവ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   എങ്കിൽ (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   അല്ലെങ്കിൽ എങ്കിൽ (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#അല്ലെങ്കിൽ

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API പൂർണ്ണസംഖ്യ luaL_execresult (lua_State *L, പൂർണ്ണസംഖ്യ stat) {
  എങ്കിൽ (stat != 0 && errno != 0)  /* error with an 'errno'? */
    തിരികെ luaL_fileresult(L, 0, ശൂന്യ);
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    എങ്കിൽ (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    അല്ലെങ്കിൽ
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    തിരികെ 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API പൂർണ്ണസംഖ്യ luaL_newmetatable (lua_State *L, സ്ഥിരം അക്ഷരം *tname) {
  എങ്കിൽ (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    തിരികെ 0;  /* leave previous value on top, but return 0 */
  lua_എടുക്കുക(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  തിരികെ 1;
}


LUALIB_API ശൂന്യം luaL_setmetatable (lua_State *L, സ്ഥിരം അക്ഷരം *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API ശൂന്യം *luaL_testudata (lua_State *L, പൂർണ്ണസംഖ്യ ud, സ്ഥിരം അക്ഷരം *tname) {
  ശൂന്യം *p = lua_touserdata(L, ud);
  എങ്കിൽ (p != ശൂന്യ) {  /* value is a userdata? */
    എങ്കിൽ (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      എങ്കിൽ (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ശൂന്യ;  /* value is a userdata with wrong metatable */
      lua_എടുക്കുക(L, 2);  /* remove both metatables */
      തിരികെ p;
    }
  }
  തിരികെ ശൂന്യ;  /* value is not a userdata with a metatable */
}


LUALIB_API ശൂന്യം *luaL_checkudata (lua_State *L, പൂർണ്ണസംഖ്യ ud, സ്ഥിരം അക്ഷരം *tname) {
  ശൂന്യം *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ശൂന്യ, ud, tname);
  തിരികെ p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API പൂർണ്ണസംഖ്യ luaL_checkoption (lua_State *L, പൂർണ്ണസംഖ്യ arg, സ്ഥിരം അക്ഷരം *def,
                                 സ്ഥിരം അക്ഷരം *സ്ഥിരം lst[]) {
  സ്ഥിരം അക്ഷരം *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i=0; lst[i]; i++)
    എങ്കിൽ (strcmp(lst[i], name) == 0)
      തിരികെ i;
  തിരികെ luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API ശൂന്യം luaL_checkstack (lua_State *L, പൂർണ്ണസംഖ്യ space, സ്ഥിരം അക്ഷരം *സന്ദേശം) {
  എങ്കിൽ (l_unlikely(!lua_checkstack(L, space))) {
    എങ്കിൽ (സന്ദേശം)
      luaL_error(L, "stack overflow (%s)", സന്ദേശം);
    അല്ലെങ്കിൽ
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API ശൂന്യം luaL_checktype (lua_State *L, പൂർണ്ണസംഖ്യ arg, പൂർണ്ണസംഖ്യ t) {
  എങ്കിൽ (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API ശൂന്യം luaL_checkany (lua_State *L, പൂർണ്ണസംഖ്യ arg) {
  എങ്കിൽ (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API സ്ഥിരം അക്ഷരം *luaL_checklstring (lua_State *L, പൂർണ്ണസംഖ്യ arg, size_t *len) {
  സ്ഥിരം അക്ഷരം *s = lua_tolstring(L, arg, len);
  എങ്കിൽ (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  തിരികെ s;
}


LUALIB_API സ്ഥിരം അക്ഷരം *luaL_optlstring (lua_State *L, പൂർണ്ണസംഖ്യ arg,
                                        സ്ഥിരം അക്ഷരം *def, size_t *len) {
  എങ്കിൽ (lua_isnoneornil(L, arg)) {
    എങ്കിൽ (len)
      *len = (def ? strlen(def) : 0);
    തിരികെ def;
  }
  അല്ലെങ്കിൽ തിരികെ luaL_checklstring(L, arg, len);
}


LUALIB_API lua_സംഖ്യ luaL_checknumber (lua_State *L, പൂർണ്ണസംഖ്യ arg) {
  പൂർണ്ണസംഖ്യ isnum;
  lua_സംഖ്യ d = lua_tonumberx(L, arg, &isnum);
  എങ്കിൽ (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  തിരികെ d;
}


LUALIB_API lua_സംഖ്യ luaL_optnumber (lua_State *L, പൂർണ്ണസംഖ്യ arg, lua_സംഖ്യ def) {
  തിരികെ luaL_opt(L, luaL_checknumber, arg, def);
}


സ്ഥിതം ശൂന്യം interror (lua_State *L, പൂർണ്ണസംഖ്യ arg) {
  എങ്കിൽ (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  അല്ലെങ്കിൽ
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, പൂർണ്ണസംഖ്യ arg) {
  പൂർണ്ണസംഖ്യ isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  എങ്കിൽ (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  തിരികെ d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, പൂർണ്ണസംഖ്യ arg,
                                                      lua_Integer def) {
  തിരികെ luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
തരനിർവചനം ഘടന UBox {
  ശൂന്യം *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
സ്ഥിതം ശൂന്യം *resizebox (lua_State *L, പൂർണ്ണസംഖ്യ idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  എങ്കിൽ (box->bsize == newsize)  /* not changing size? */
    തിരികെ box->box;  /* keep the buffer */
  അല്ലെങ്കിൽ {
    ശൂന്യം *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    ശൂന്യം *temp = allocf(ud, box->box, box->bsize, newsize);
    എങ്കിൽ (l_unlikely(temp == ശൂന്യ && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    തിരികെ temp;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  തിരികെ 0;
}


സ്ഥിതം സ്ഥിരം luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ശൂന്യ, ശൂന്യ}
};


/*
** Get/create metatable (MT) for boxes
*/
സ്ഥിതം ശൂന്യം getBoxMT (lua_State *L) {
  സ്ഥിരം അക്ഷരം *BOXMT = "_UBOX*"; /* key for the metatable */
  എങ്കിൽ (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


സ്ഥിതം ശൂന്യം newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, വലുപ്പം(UBox), 0);
  box->box = ശൂന്യ;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ശൂന്യ  \
                            : lua_touserdata(B->L, idx) == (ശൂന്യം*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
സ്ഥിതം size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  എങ്കിൽ (l_unlikely(sz >= MAX_SIZE - B->n))
    തിരികെ cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  എങ്കിൽ (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  എങ്കിൽ (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  തിരികെ newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
സ്ഥിതം അക്ഷരം *prepbuffsize (luaL_Buffer *B, size_t sz, പൂർണ്ണസംഖ്യ boxidx) {
  checkbufferlevel(B, boxidx);
  എങ്കിൽ (B->size - B->n >= sz)  /* enough space? */
    തിരികെ B->b + B->n;
  അല്ലെങ്കിൽ {
    lua_State *L = B->L;
    അക്ഷരം *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    എങ്കിൽ (buffonstack(B))  /* buffer already has a box? */
      newbuff = (അക്ഷരം *)resizebox(L, boxidx, newsize);  /* resize it */
    അല്ലെങ്കിൽ {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_തിരുകുക(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (അക്ഷരം *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * വലുപ്പം(അക്ഷരം));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    തിരികെ newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API അക്ഷരം *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  തിരികെ prepbuffsize(B, sz, -1);
}


LUALIB_API ശൂന്യം luaL_addlstring (luaL_Buffer *B, സ്ഥിരം അക്ഷരം *s, size_t l) {
  എങ്കിൽ (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    അക്ഷരം *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * വലുപ്പം(അക്ഷരം));
    luaL_addsize(B, l);
  }
}


LUALIB_API ശൂന്യം luaL_addstring (luaL_Buffer *B, സ്ഥിരം അക്ഷരം *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API ശൂന്യം luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  എങ്കിൽ (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  അല്ലെങ്കിൽ {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    ശൂന്യം *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    അക്ഷരം *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (അക്ഷരം*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ശൂന്യ;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API ശൂന്യം luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API ശൂന്യം luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  സ്ഥിരം അക്ഷരം *s = lua_tolstring(L, -1, &len);
  അക്ഷരം *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * വലുപ്പം(അക്ഷരം));
  luaL_addsize(B, len);
  lua_എടുക്കുക(L, 1);  /* pop string */
}


LUALIB_API ശൂന്യം luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (ശൂന്യം*)B);  /* push placeholder */
}


LUALIB_API അക്ഷരം *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  തിരികെ prepbuffsize(B, sz, -1);
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
LUALIB_API പൂർണ്ണസംഖ്യ luaL_ref (lua_State *L, പൂർണ്ണസംഖ്യ t) {
  പൂർണ്ണസംഖ്യ ref;
  എങ്കിൽ (lua_isnil(L, -1)) {
    lua_എടുക്കുക(L, 1);  /* remove from stack */
    തിരികെ LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  എങ്കിൽ (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (പൂർണ്ണസംഖ്യ)lua_tointeger(L, -1);  /* ref = t[1] */
  അല്ലെങ്കിൽ {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_എടുക്കുക(L, 1);  /* remove element from stack */
  എങ്കിൽ (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  അല്ലെങ്കിൽ  /* no free elements */
    ref = (പൂർണ്ണസംഖ്യ)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  തിരികെ ref;
}


LUALIB_API ശൂന്യം luaL_unref (lua_State *L, പൂർണ്ണസംഖ്യ t, പൂർണ്ണസംഖ്യ ref) {
  എങ്കിൽ (ref >= 0) {
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

തരനിർവചനം ഘടന LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  അക്ഷരം buff[BUFSIZ];  /* area for reading file */
} LoadF;


സ്ഥിതം സ്ഥിരം അക്ഷരം *getF (lua_State *L, ശൂന്യം *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  എങ്കിൽ (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  അല്ലെങ്കിൽ {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    എങ്കിൽ (feof(lf->f)) തിരികെ ശൂന്യ;
    *size = fread(lf->buff, 1, വലുപ്പം(lf->buff), lf->f);  /* read block */
  }
  തിരികെ lf->buff;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ errfile (lua_State *L, സ്ഥിരം അക്ഷരം *what, പൂർണ്ണസംഖ്യ fnameindex) {
  പൂർണ്ണസംഖ്യ err = errno;
  സ്ഥിരം അക്ഷരം *filename = lua_tostring(L, fnameindex) + 1;
  എങ്കിൽ (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  അല്ലെങ്കിൽ
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  തിരികെ LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ skipBOM (FILE *f) {
  പൂർണ്ണസംഖ്യ c = getc(f);  /* read first character */
  എങ്കിൽ (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    തിരികെ getc(f);  /* ignore BOM and return next char */
  അല്ലെങ്കിൽ  /* no (valid) BOM */
    തിരികെ c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ skipcomment (FILE *f, പൂർണ്ണസംഖ്യ *cp) {
  പൂർണ്ണസംഖ്യ c = *cp = skipBOM(f);
  എങ്കിൽ (c == '#') {  /* first line is a comment (Unix exec. file)? */
    ചെയ്യുക {  /* skip first line */
      c = getc(f);
    } ആകുമ്പോൾ (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    തിരികെ 1;  /* there was a comment */
  }
  അല്ലെങ്കിൽ തിരികെ 0;  /* no comment */
}


LUALIB_API പൂർണ്ണസംഖ്യ luaL_loadfilex (lua_State *L, സ്ഥിരം അക്ഷരം *filename,
                                             സ്ഥിരം അക്ഷരം *mode) {
  LoadF lf;
  പൂർണ്ണസംഖ്യ status, readstatus;
  പൂർണ്ണസംഖ്യ c;
  പൂർണ്ണസംഖ്യ fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  എങ്കിൽ (filename == ശൂന്യ) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  അല്ലെങ്കിൽ {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    എങ്കിൽ (lf.f == ശൂന്യ) തിരികെ errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  എങ്കിൽ (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  എങ്കിൽ (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    എങ്കിൽ (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      എങ്കിൽ (lf.f == ശൂന്യ) തിരികെ errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  എങ്കിൽ (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  എങ്കിൽ (filename) fclose(lf.f);  /* close file (even in case of errors) */
  എങ്കിൽ (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    തിരികെ errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  തിരികെ status;
}


തരനിർവചനം ഘടന LoadS {
  സ്ഥിരം അക്ഷരം *s;
  size_t size;
} LoadS;


സ്ഥിതം സ്ഥിരം അക്ഷരം *getS (lua_State *L, ശൂന്യം *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  എങ്കിൽ (ls->size == 0) തിരികെ ശൂന്യ;
  *size = ls->size;
  ls->size = 0;
  തിരികെ ls->s;
}


LUALIB_API പൂർണ്ണസംഖ്യ luaL_loadbufferx (lua_State *L, സ്ഥിരം അക്ഷരം *buff, size_t size,
                                 സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  തിരികെ lua_load(L, getS, &ls, name, mode);
}


LUALIB_API പൂർണ്ണസംഖ്യ luaL_loadstring (lua_State *L, സ്ഥിരം അക്ഷരം *s) {
  തിരികെ luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API പൂർണ്ണസംഖ്യ luaL_getmetafield (lua_State *L, പൂർണ്ണസംഖ്യ obj, സ്ഥിരം അക്ഷരം *event) {
  എങ്കിൽ (!lua_getmetatable(L, obj))  /* no metatable? */
    തിരികെ LUA_TNIL;
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    എങ്കിൽ (tt == LUA_TNIL)  /* is metafield nil? */
      lua_എടുക്കുക(L, 2);  /* remove metatable and metafield */
    അല്ലെങ്കിൽ
      lua_remove(L, -2);  /* remove only metatable */
    തിരികെ tt;  /* return metafield type */
  }
}


LUALIB_API പൂർണ്ണസംഖ്യ luaL_callmeta (lua_State *L, പൂർണ്ണസംഖ്യ obj, സ്ഥിരം അക്ഷരം *event) {
  obj = lua_absindex(L, obj);
  എങ്കിൽ (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    തിരികെ 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  തിരികെ 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, പൂർണ്ണസംഖ്യ idx) {
  lua_Integer l;
  പൂർണ്ണസംഖ്യ isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  എങ്കിൽ (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_എടുക്കുക(L, 1);  /* remove object */
  തിരികെ l;
}


LUALIB_API സ്ഥിരം അക്ഷരം *luaL_tolstring (lua_State *L, പൂർണ്ണസംഖ്യ idx, size_t *len) {
  idx = lua_absindex(L,idx);
  എങ്കിൽ (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    എങ്കിൽ (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  അല്ലെങ്കിൽ {
    തിരഞ്ഞെടുപ്പ് (lua_type(L, idx)) {
      അവസ്ഥ LUA_TNUMBER: {
        അക്ഷരം buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        നിർത്തുക;
      }
      അവസ്ഥ LUA_TSTRING:
        lua_pushvalue(L, idx);
        നിർത്തുക;
      അവസ്ഥ LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        നിർത്തുക;
      അവസ്ഥ LUA_TNIL:
        lua_pushliteral(L, "nil");
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി: {
        പൂർണ്ണസംഖ്യ tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        സ്ഥിരം അക്ഷരം *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        എങ്കിൽ (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        നിർത്തുക;
      }
    }
  }
  തിരികെ lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API ശൂന്യം luaL_setfuncs (lua_State *L, സ്ഥിരം luaL_Reg *l, പൂർണ്ണസംഖ്യ nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  വേണ്ടി (; l->name != ശൂന്യ; l++) {  /* fill the table with given functions */
    എങ്കിൽ (l->func == ശൂന്യ)  /* placeholder? */
      lua_pushboolean(L, 0);
    അല്ലെങ്കിൽ {
      പൂർണ്ണസംഖ്യ i;
      വേണ്ടി (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_എടുക്കുക(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API പൂർണ്ണസംഖ്യ luaL_getsubtable (lua_State *L, പൂർണ്ണസംഖ്യ idx, സ്ഥിരം അക്ഷരം *fname) {
  എങ്കിൽ (lua_getfield(L, idx, fname) == LUA_TTABLE)
    തിരികെ 1;  /* table already there */
  അല്ലെങ്കിൽ {
    lua_എടുക്കുക(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    തിരികെ 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API ശൂന്യം luaL_requiref (lua_State *L, സ്ഥിരം അക്ഷരം *modname,
                               lua_CFunction openf, പൂർണ്ണസംഖ്യ glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  എങ്കിൽ (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_എടുക്കുക(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  എങ്കിൽ (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API ശൂന്യം luaL_addgsub (luaL_Buffer *b, സ്ഥിരം അക്ഷരം *s,
                                     സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *r) {
  സ്ഥിരം അക്ഷരം *wild;
  size_t l = strlen(p);
  ആകുമ്പോൾ ((wild = strstr(s, p)) != ശൂന്യ) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API സ്ഥിരം അക്ഷരം *luaL_gsub (lua_State *L, സ്ഥിരം അക്ഷരം *s,
                                  സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  തിരികെ lua_tostring(L, -1);
}


ശൂന്യം *luaL_alloc (ശൂന്യം *ud, ശൂന്യം *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  എങ്കിൽ (nsize == 0) {
    സ്വതന്ത്രം(ptr);
    തിരികെ ശൂന്യ;
  }
  അല്ലെങ്കിൽ
    തിരികെ realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ panic (lua_State *L) {
  സ്ഥിരം അക്ഷരം *സന്ദേശം = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        സന്ദേശം);
  തിരികെ 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
സ്ഥിതം ശൂന്യം warnfoff (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *message, പൂർണ്ണസംഖ്യ tocont);
സ്ഥിതം ശൂന്യം warnfon (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *message, പൂർണ്ണസംഖ്യ tocont);
സ്ഥിതം ശൂന്യം warnfcont (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *message, പൂർണ്ണസംഖ്യ tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ checkcontrol (lua_State *L, സ്ഥിരം അക്ഷരം *message, പൂർണ്ണസംഖ്യ tocont) {
  എങ്കിൽ (tocont || *(message++) != '@')  /* not a control message? */
    തിരികെ 0;
  അല്ലെങ്കിൽ {
    എങ്കിൽ (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    തിരികെ 1;  /* it was a control message */
  }
}


സ്ഥിതം ശൂന്യം warnfoff (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *message, പൂർണ്ണസംഖ്യ tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
സ്ഥിതം ശൂന്യം warnfcont (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *message, പൂർണ്ണസംഖ്യ tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  എങ്കിൽ (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  അല്ലെങ്കിൽ {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


സ്ഥിതം ശൂന്യം warnfon (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *message, പൂർണ്ണസംഖ്യ tocont) {
  എങ്കിൽ (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    തിരികെ;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#എങ്കിൽ !defined(luai_makeseed)

#അന്തര്ഭാവ <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(വലുപ്പം(ശൂന്യം*) + വലുപ്പം(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + വലുപ്പം(പൂർണ്ണസംഖ്യ) - 1) / വലുപ്പം(പൂർണ്ണസംഖ്യ))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), വലുപ്പം(v)), b += വലുപ്പം(v))


സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ luai_makeseed (ശൂന്യം) {
  unsigned പൂർണ്ണസംഖ്യ buff[BUFSEED];
  unsigned പൂർണ്ണസംഖ്യ res;
  unsigned പൂർണ്ണസംഖ്യ i;
  time_t t = time(ശൂന്യ);
  അക്ഷരം *b = (അക്ഷരം*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, വലുപ്പം(buff) - BUFSEEDB);
  res = buff[0];
  വേണ്ടി (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  തിരികെ res;
}

#endif


LUALIB_API unsigned പൂർണ്ണസംഖ്യ luaL_makeseed (lua_State *L) {
  UNUSED(L);
  തിരികെ luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (ശൂന്യം) {
  lua_State *L = lua_newstate(luaL_alloc, ശൂന്യ, luaL_makeseed(ശൂന്യ));
  എങ്കിൽ (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  തിരികെ L;
}


LUALIB_API ശൂന്യം luaL_checkversion_ (lua_State *L, lua_സംഖ്യ ver, size_t sz) {
  lua_സംഖ്യ v = lua_version(L);
  എങ്കിൽ (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  അല്ലെങ്കിൽ എങ്കിൽ (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

