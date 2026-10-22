/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା findfield (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା objidx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level) {
  ଯଦି (level == 0 || !lua_istable(L, -1))
    ଫେରନ୍ତୁ 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  ଯେତେବେଳେ (lua_next(L, -2)) {  /* for each pair in table */
    ଯଦି (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      ଯଦି (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_କାଢ଼(L, 1);  /* remove value (but keep name) */
        ଫେରନ୍ତୁ 1;
      }
      ଅନ୍ୟଥା ଯଦି (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        ଫେରନ୍ତୁ 1;
      }
    }
    lua_କାଢ଼(L, 1);  /* remove value */
  }
  ଫେରନ୍ତୁ 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  ଯଦି (findfield(L, ଶୀର୍ଷ + 1, 2)) {
    ଅଚଳ ଅକ୍ଷର *name = lua_tostring(L, -1);
    ଯଦି (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, ଶୀର୍ଷ + 1);  /* copy name to proper place */
    lua_settop(L, ଶୀର୍ଷ + 1);  /* remove table "loaded" and name copy */
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା {
    lua_settop(L, ଶୀର୍ଷ);  /* remove function and global table */
    ଫେରନ୍ତୁ 0;
  }
}


ସ୍ଥିର ଶୂନ୍ୟ pushfuncname (lua_State *L, lua_Debug *ar) {
  ଯଦି (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  ଅନ୍ୟଥା ଯଦି (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  ଅନ୍ୟଥା ଯଦି (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  ଅନ୍ୟଥା ଯଦି (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  ଅନ୍ୟଥା  /* nothing left... */
    lua_pushliteral(L, "?");
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastlevel (lua_State *L) {
  lua_Debug ar;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା li = 1, le = 1;
  /* find an upper bound */
  ଯେତେବେଳେ (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  ଯେତେବେଳେ (li < le) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = (li + le)/2;
    ଯଦି (lua_getstack(L, m, &ar)) li = m + 1;
    ଅନ୍ୟଥା le = m;
  }
  ଫେରନ୍ତୁ le - 1;
}


LUALIB_API ଶୂନ୍ୟ luaL_traceback (lua_State *L, lua_State *L1,
                                ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level) {
  luaL_Buffer b;
  lua_Debug ar;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା last = lastlevel(L1);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  ଯଦି (ସନ୍ଦେଶ) {
    luaL_addstring(&b, ସନ୍ଦେଶ);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  ଯେତେବେଳେ (lua_getstack(L1, level++, &ar)) {
    ଯଦି (limit2show-- == 0) {  /* too many levels? */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    ଅନ୍ୟଥା {
      lua_getinfo(L1, "Slnt", &ar);
      ଯଦି (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      ଅନ୍ୟଥା
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      ଯଦି (ar.istailcall)
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

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_argerror (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ଅଚଳ ଅକ୍ଷର *extramsg) {
  lua_Debug ar;
  ଅଚଳ ଅକ୍ଷର *argword;
  ଯଦି (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ଫେରନ୍ତୁ luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  ଯଦି (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  ଅନ୍ୟଥା {
    arg -= ar.extraargs;  /* do not count extra arguments */
    ଯଦି (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      ଯଦି (arg == 0)  /* error in self argument? */
        ଫେରନ୍ତୁ luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  ଯଦି (ar.name == ଖାଲି)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  ଫେରନ୍ତୁ luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_typeerror (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ଅଚଳ ଅକ୍ଷର *tname) {
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ;
  ଅଚଳ ଅକ୍ଷର *typearg;  /* name for the type of the actual argument */
  ଯଦି (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  ଅନ୍ୟଥା ଯଦି (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  ଅନ୍ୟଥା
    typearg = luaL_typename(L, arg);  /* standard name */
  ସନ୍ଦେଶ = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  ଫେରନ୍ତୁ luaL_argerror(L, arg, ସନ୍ଦେଶ);
}


ସ୍ଥିର ଶୂନ୍ୟ tag_error (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API ଶୂନ୍ୟ luaL_where (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level) {
  lua_Debug ar;
  ଯଦି (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    ଯଦି (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      ଫେରନ୍ତୁ;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_error (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  ଫେରନ୍ତୁ lua_error(L);
}


LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_fileresult (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat, ଅଚଳ ଅକ୍ଷର *fname) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା en = errno;  /* calls to Lua API may change this value */
  ଯଦି (stat) {
    lua_pushboolean(L, 1);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ;
    luaL_pushfail(L);
    ସନ୍ଦେଶ = (en != 0) ? strerror(en) : "(no extra info)";
    ଯଦି (fname)
      lua_pushfstring(L, "%s: %s", fname, ସନ୍ଦେଶ);
    ଅନ୍ୟଥା
      lua_pushstring(L, ସନ୍ଦେଶ);
    lua_pushinteger(L, en);
    ଫେରନ୍ତୁ 3;
  }
}


#ଯଦି !defined(l_inspectstat)	/* { */

#ଯଦି defined(LUA_USE_POSIX)

#ଅନ୍ତର୍ଭାଵ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   ଯଦି (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   ଅନ୍ୟଥା ଯଦି (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#ଅନ୍ୟଥା

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_execresult (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat) {
  ଯଦି (stat != 0 && errno != 0)  /* error with an 'errno'? */
    ଫେରନ୍ତୁ luaL_fileresult(L, 0, ଖାଲି);
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    ଯଦି (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    ଅନ୍ୟଥା
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    ଫେରନ୍ତୁ 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_newmetatable (lua_State *L, ଅଚଳ ଅକ୍ଷର *tname) {
  ଯଦି (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    ଫେରନ୍ତୁ 0;  /* leave previous value on top, but return 0 */
  lua_କାଢ଼(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  ଫେରନ୍ତୁ 1;
}


LUALIB_API ଶୂନ୍ୟ luaL_setmetatable (lua_State *L, ଅଚଳ ଅକ୍ଷର *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API ଶୂନ୍ୟ *luaL_testudata (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ud, ଅଚଳ ଅକ୍ଷର *tname) {
  ଶୂନ୍ୟ *p = lua_touserdata(L, ud);
  ଯଦି (p != ଖାଲି) {  /* value is a userdata? */
    ଯଦି (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      ଯଦି (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ଖାଲି;  /* value is a userdata with wrong metatable */
      lua_କାଢ଼(L, 2);  /* remove both metatables */
      ଫେରନ୍ତୁ p;
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* value is not a userdata with a metatable */
}


LUALIB_API ଶୂନ୍ୟ *luaL_checkudata (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ud, ଅଚଳ ଅକ୍ଷର *tname) {
  ଶୂନ୍ୟ *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ଖାଲି, ud, tname);
  ଫେରନ୍ତୁ p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_checkoption (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ଅଚଳ ଅକ୍ଷର *def,
                                 ଅଚଳ ଅକ୍ଷର *ଅଚଳ lst[]) {
  ଅଚଳ ଅକ୍ଷର *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i=0; lst[i]; i++)
    ଯଦି (strcmp(lst[i], name) == 0)
      ଫେରନ୍ତୁ i;
  ଫେରନ୍ତୁ luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API ଶୂନ୍ୟ luaL_checkstack (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା space, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
  ଯଦି (l_unlikely(!lua_checkstack(L, space))) {
    ଯଦି (ସନ୍ଦେଶ)
      luaL_error(L, "stack overflow (%s)", ସନ୍ଦେଶ);
    ଅନ୍ୟଥା
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API ଶୂନ୍ୟ luaL_checktype (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା t) {
  ଯଦି (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API ଶୂନ୍ୟ luaL_checkany (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  ଯଦି (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ଅଚଳ ଅକ୍ଷର *luaL_checklstring (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, size_t *len) {
  ଅଚଳ ଅକ୍ଷର *s = lua_tolstring(L, arg, len);
  ଯଦି (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  ଫେରନ୍ତୁ s;
}


LUALIB_API ଅଚଳ ଅକ୍ଷର *luaL_optlstring (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg,
                                        ଅଚଳ ଅକ୍ଷର *def, size_t *len) {
  ଯଦି (lua_isnoneornil(L, arg)) {
    ଯଦି (len)
      *len = (def ? strlen(def) : 0);
    ଫେରନ୍ତୁ def;
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ luaL_checklstring(L, arg, len);
}


LUALIB_API lua_ସଂଖ୍ୟା luaL_checknumber (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isnum;
  lua_ସଂଖ୍ୟା d = lua_tonumberx(L, arg, &isnum);
  ଯଦି (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  ଫେରନ୍ତୁ d;
}


LUALIB_API lua_ସଂଖ୍ୟା luaL_optnumber (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg, lua_ସଂଖ୍ୟା def) {
  ଫେରନ୍ତୁ luaL_opt(L, luaL_checknumber, arg, def);
}


ସ୍ଥିର ଶୂନ୍ୟ interror (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  ଯଦି (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  ଅନ୍ୟଥା
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  ଯଦି (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  ଫେରନ୍ତୁ d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg,
                                                      lua_Integer def) {
  ଫେରନ୍ତୁ luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା UBox {
  ଶୂନ୍ୟ *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
ସ୍ଥିର ଶୂନ୍ୟ *resizebox (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  ଯଦି (box->bsize == newsize)  /* not changing size? */
    ଫେରନ୍ତୁ box->box;  /* keep the buffer */
  ଅନ୍ୟଥା {
    ଶୂନ୍ୟ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    ଶୂନ୍ୟ *temp = allocf(ud, box->box, box->bsize, newsize);
    ଯଦି (l_unlikely(temp == ଖାଲି && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    ଫେରନ୍ତୁ temp;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ଅଚଳ luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ଖାଲି, ଖାଲି}
};


/*
** Get/create metatable (MT) for boxes
*/
ସ୍ଥିର ଶୂନ୍ୟ getBoxMT (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *BOXMT = "_UBOX*"; /* key for the metatable */
  ଯଦି (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, ଆକାର(UBox), 0);
  box->box = ଖାଲି;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ଖାଲି  \
                            : lua_touserdata(B->L, idx) == (ଶୂନ୍ୟ*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
ସ୍ଥିର size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  ଯଦି (l_unlikely(sz >= MAX_SIZE - B->n))
    ଫେରନ୍ତୁ cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  ଯଦି (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  ଯଦି (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  ଫେରନ୍ତୁ newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
ସ୍ଥିର ଅକ୍ଷର *prepbuffsize (luaL_Buffer *B, size_t sz, ପୂର୍ଣ୍ଣସଂଖ୍ୟା boxidx) {
  checkbufferlevel(B, boxidx);
  ଯଦି (B->size - B->n >= sz)  /* enough space? */
    ଫେରନ୍ତୁ B->b + B->n;
  ଅନ୍ୟଥା {
    lua_State *L = B->L;
    ଅକ୍ଷର *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    ଯଦି (buffonstack(B))  /* buffer already has a box? */
      newbuff = (ଅକ୍ଷର *)resizebox(L, boxidx, newsize);  /* resize it */
    ଅନ୍ୟଥା {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_ଭର୍ତ୍ତି(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (ଅକ୍ଷର *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * ଆକାର(ଅକ୍ଷର));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    ଫେରନ୍ତୁ newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API ଅକ୍ଷର *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  ଫେରନ୍ତୁ prepbuffsize(B, sz, -1);
}


LUALIB_API ଶୂନ୍ୟ luaL_addlstring (luaL_Buffer *B, ଅଚଳ ଅକ୍ଷର *s, size_t l) {
  ଯଦି (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    ଅକ୍ଷର *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * ଆକାର(ଅକ୍ଷର));
    luaL_addsize(B, l);
  }
}


LUALIB_API ଶୂନ୍ୟ luaL_addstring (luaL_Buffer *B, ଅଚଳ ଅକ୍ଷର *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API ଶୂନ୍ୟ luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  ଯଦି (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  ଅନ୍ୟଥା {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    ଶୂନ୍ୟ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    ଅକ୍ଷର *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (ଅକ୍ଷର*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ଖାଲି;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API ଶୂନ୍ୟ luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API ଶୂନ୍ୟ luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  ଅଚଳ ଅକ୍ଷର *s = lua_tolstring(L, -1, &len);
  ଅକ୍ଷର *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * ଆକାର(ଅକ୍ଷର));
  luaL_addsize(B, len);
  lua_କାଢ଼(L, 1);  /* pop string */
}


LUALIB_API ଶୂନ୍ୟ luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (ଶୂନ୍ୟ*)B);  /* push placeholder */
}


LUALIB_API ଅକ୍ଷର *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  ଫେରନ୍ତୁ prepbuffsize(B, sz, -1);
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
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_ref (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା t) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ref;
  ଯଦି (lua_isnil(L, -1)) {
    lua_କାଢ଼(L, 1);  /* remove from stack */
    ଫେରନ୍ତୁ LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  ଯଦି (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)lua_tointeger(L, -1);  /* ref = t[1] */
  ଅନ୍ୟଥା {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_କାଢ଼(L, 1);  /* remove element from stack */
  ଯଦି (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  ଅନ୍ୟଥା  /* no free elements */
    ref = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  ଫେରନ୍ତୁ ref;
}


LUALIB_API ଶୂନ୍ୟ luaL_unref (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ref) {
  ଯଦି (ref >= 0) {
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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  ଅକ୍ଷର buff[BUFSIZ];  /* area for reading file */
} LoadF;


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getF (lua_State *L, ଶୂନ୍ୟ *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  ଯଦି (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  ଅନ୍ୟଥା {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    ଯଦି (feof(lf->f)) ଫେରନ୍ତୁ ଖାଲି;
    *size = fread(lf->buff, 1, ଆକାର(lf->buff), lf->f);  /* read block */
  }
  ଫେରନ୍ତୁ lf->buff;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା errfile (lua_State *L, ଅଚଳ ଅକ୍ଷର *what, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fnameindex) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = errno;
  ଅଚଳ ଅକ୍ଷର *filename = lua_tostring(L, fnameindex) + 1;
  ଯଦି (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  ଅନ୍ୟଥା
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  ଫେରନ୍ତୁ LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା skipBOM (FILE *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = getc(f);  /* read first character */
  ଯଦି (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    ଫେରନ୍ତୁ getc(f);  /* ignore BOM and return next char */
  ଅନ୍ୟଥା  /* no (valid) BOM */
    ଫେରନ୍ତୁ c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା skipcomment (FILE *f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *cp) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = *cp = skipBOM(f);
  ଯଦି (c == '#') {  /* first line is a comment (Unix exec. file)? */
    କର {  /* skip first line */
      c = getc(f);
    } ଯେତେବେଳେ (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    ଫେରନ୍ତୁ 1;  /* there was a comment */
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;  /* no comment */
}


LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_loadfilex (lua_State *L, ଅଚଳ ଅକ୍ଷର *filename,
                                             ଅଚଳ ଅକ୍ଷର *mode) {
  LoadF lf;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, readstatus;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ଯଦି (filename == ଖାଲି) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  ଅନ୍ୟଥା {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    ଯଦି (lf.f == ଖାଲି) ଫେରନ୍ତୁ errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  ଯଦି (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  ଯଦି (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    ଯଦି (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      ଯଦି (lf.f == ଖାଲି) ଫେରନ୍ତୁ errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  ଯଦି (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  ଯଦି (filename) fclose(lf.f);  /* close file (even in case of errors) */
  ଯଦି (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    ଫେରନ୍ତୁ errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  ଫେରନ୍ତୁ status;
}


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା LoadS {
  ଅଚଳ ଅକ୍ଷର *s;
  size_t size;
} LoadS;


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getS (lua_State *L, ଶୂନ୍ୟ *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  ଯଦି (ls->size == 0) ଫେରନ୍ତୁ ଖାଲି;
  *size = ls->size;
  ls->size = 0;
  ଫେରନ୍ତୁ ls->s;
}


LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_loadbufferx (lua_State *L, ଅଚଳ ଅକ୍ଷର *buff, size_t size,
                                 ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  ଫେରନ୍ତୁ lua_load(L, getS, &ls, name, mode);
}


LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_loadstring (lua_State *L, ଅଚଳ ଅକ୍ଷର *s) {
  ଫେରନ୍ତୁ luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_getmetafield (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା obj, ଅଚଳ ଅକ୍ଷର *event) {
  ଯଦି (!lua_getmetatable(L, obj))  /* no metatable? */
    ଫେରନ୍ତୁ LUA_TNIL;
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    ଯଦି (tt == LUA_TNIL)  /* is metafield nil? */
      lua_କାଢ଼(L, 2);  /* remove metatable and metafield */
    ଅନ୍ୟଥା
      lua_remove(L, -2);  /* remove only metatable */
    ଫେରନ୍ତୁ tt;  /* return metafield type */
  }
}


LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_callmeta (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା obj, ଅଚଳ ଅକ୍ଷର *event) {
  obj = lua_absindex(L, obj);
  ଯଦି (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    ଫେରନ୍ତୁ 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  ଫେରନ୍ତୁ 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx) {
  lua_Integer l;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  ଯଦି (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_କାଢ଼(L, 1);  /* remove object */
  ଫେରନ୍ତୁ l;
}


LUALIB_API ଅଚଳ ଅକ୍ଷର *luaL_tolstring (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, size_t *len) {
  idx = lua_absindex(L,idx);
  ଯଦି (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    ଯଦି (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  ଅନ୍ୟଥା {
    ବାଛ (lua_type(L, idx)) {
      ସ୍ଥିତି LUA_TNUMBER: {
        ଅକ୍ଷର buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି LUA_TSTRING:
        lua_pushvalue(L, idx);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_TNIL:
        lua_pushliteral(L, "nil");
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ଅଚଳ ଅକ୍ଷର *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        ଯଦି (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        ଭାଙ୍ଗ;
      }
    }
  }
  ଫେରନ୍ତୁ lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API ଶୂନ୍ୟ luaL_setfuncs (lua_State *L, ଅଚଳ luaL_Reg *l, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  ପାଇଁ (; l->name != ଖାଲି; l++) {  /* fill the table with given functions */
    ଯଦି (l->func == ଖାଲି)  /* placeholder? */
      lua_pushboolean(L, 0);
    ଅନ୍ୟଥା {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପାଇଁ (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_କାଢ଼(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_getsubtable (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, ଅଚଳ ଅକ୍ଷର *fname) {
  ଯଦି (lua_getfield(L, idx, fname) == LUA_TTABLE)
    ଫେରନ୍ତୁ 1;  /* table already there */
  ଅନ୍ୟଥା {
    lua_କାଢ଼(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    ଫେରନ୍ତୁ 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API ଶୂନ୍ୟ luaL_requiref (lua_State *L, ଅଚଳ ଅକ୍ଷର *modname,
                               lua_CFunction openf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  ଯଦି (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_କାଢ଼(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  ଯଦି (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API ଶୂନ୍ୟ luaL_addgsub (luaL_Buffer *b, ଅଚଳ ଅକ୍ଷର *s,
                                     ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *r) {
  ଅଚଳ ଅକ୍ଷର *wild;
  size_t l = strlen(p);
  ଯେତେବେଳେ ((wild = strstr(s, p)) != ଖାଲି) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ଅଚଳ ଅକ୍ଷର *luaL_gsub (lua_State *L, ଅଚଳ ଅକ୍ଷର *s,
                                  ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  ଫେରନ୍ତୁ lua_tostring(L, -1);
}


ଶୂନ୍ୟ *luaL_alloc (ଶୂନ୍ୟ *ud, ଶୂନ୍ୟ *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  ଯଦି (nsize == 0) {
    ମୁକ୍ତ(ptr);
    ଫେରନ୍ତୁ ଖାଲି;
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା panic (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        ସନ୍ଦେଶ);
  ଫେରନ୍ତୁ 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
ସ୍ଥିର ଶୂନ୍ୟ warnfoff (ଶୂନ୍ୟ *ud, ଅଚଳ ଅକ୍ଷର *message, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont);
ସ୍ଥିର ଶୂନ୍ୟ warnfon (ଶୂନ୍ୟ *ud, ଅଚଳ ଅକ୍ଷର *message, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont);
ସ୍ଥିର ଶୂନ୍ୟ warnfcont (ଶୂନ୍ୟ *ud, ଅଚଳ ଅକ୍ଷର *message, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkcontrol (lua_State *L, ଅଚଳ ଅକ୍ଷର *message, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont) {
  ଯଦି (tocont || *(message++) != '@')  /* not a control message? */
    ଫେରନ୍ତୁ 0;
  ଅନ୍ୟଥା {
    ଯଦି (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    ଅନ୍ୟଥା ଯଦି (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    ଫେରନ୍ତୁ 1;  /* it was a control message */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ warnfoff (ଶୂନ୍ୟ *ud, ଅଚଳ ଅକ୍ଷର *message, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
ସ୍ଥିର ଶୂନ୍ୟ warnfcont (ଶୂନ୍ୟ *ud, ଅଚଳ ଅକ୍ଷର *message, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  ଯଦି (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  ଅନ୍ୟଥା {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ warnfon (ଶୂନ୍ୟ *ud, ଅଚଳ ଅକ୍ଷର *message, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont) {
  ଯଦି (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    ଫେରନ୍ତୁ;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#ଯଦି !defined(luai_makeseed)

#ଅନ୍ତର୍ଭାଵ <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(ଆକାର(ଶୂନ୍ୟ*) + ଆକାର(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1) / ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), ଆକାର(v)), b += ଆକାର(v))


ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା luai_makeseed (ଶୂନ୍ୟ) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା buff[BUFSEED];
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  time_t t = time(ଖାଲି);
  ଅକ୍ଷର *b = (ଅକ୍ଷର*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, ଆକାର(buff) - BUFSEEDB);
  res = buff[0];
  ପାଇଁ (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  ଫେରନ୍ତୁ res;
}

#endif


LUALIB_API unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaL_makeseed (lua_State *L) {
  UNUSED(L);
  ଫେରନ୍ତୁ luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (ଶୂନ୍ୟ) {
  lua_State *L = lua_newstate(luaL_alloc, ଖାଲି, luaL_makeseed(ଖାଲି));
  ଯଦି (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  ଫେରନ୍ତୁ L;
}


LUALIB_API ଶୂନ୍ୟ luaL_checkversion_ (lua_State *L, lua_ସଂଖ୍ୟା ver, size_t sz) {
  lua_ସଂଖ୍ୟା v = lua_version(L);
  ଯଦି (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  ଅନ୍ୟଥା ଯଦି (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

