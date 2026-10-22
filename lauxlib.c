/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lauxlib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


/*
** {======================================================
** Traceback
** =======================================================
*/


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEVELS1	10	/* size of the first part of the stack */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
ਸਥਿਰ ਪੂਰਨ findfield (lua_State *L, ਪੂਰਨ objidx, ਪੂਰਨ level) {
  ਜੇ (level == 0 || !lua_istable(L, -1))
    ਵਾਪਸ 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  ਜਦੋਂ (lua_ਅਗਲਾ(L, -2)) {  /* for each pair in table */
    ਜੇ (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      ਜੇ (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_ਕੱਢੋ(L, 1);  /* remove value (but keep name) */
        ਵਾਪਸ 1;
      }
      ਨਹੀਂਤਾਂ ਜੇ (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        ਵਾਪਸ 1;
      }
    }
    lua_ਕੱਢੋ(L, 1);  /* remove value */
  }
  ਵਾਪਸ 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
ਸਥਿਰ ਪੂਰਨ pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  ਪੂਰਨ ਸਿਖਰ = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_ਸਾਰਣੀ);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  ਜੇ (findfield(L, ਸਿਖਰ + 1, 2)) {
    ਅਚਲ ਅੱਖਰ *name = lua_tostring(L, -1);
    ਜੇ (ਲੜੀ_ਤੁਲਨਾ(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, ਸਿਖਰ + 1);  /* copy name to proper place */
    lua_settop(L, ਸਿਖਰ + 1);  /* remove table "loaded" and name copy */
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ {
    lua_settop(L, ਸਿਖਰ);  /* remove function and global table */
    ਵਾਪਸ 0;
  }
}


ਸਥਿਰ ਖਾਲੀ pushfuncname (lua_State *L, lua_Debug *ar) {
  ਜੇ (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  ਨਹੀਂਤਾਂ ਜੇ (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  ਨਹੀਂਤਾਂ ਜੇ (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  ਨਹੀਂਤਾਂ ਜੇ (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  ਨਹੀਂਤਾਂ  /* nothing left... */
    lua_pushliteral(L, "?");
}


ਸਥਿਰ ਪੂਰਨ lastlevel (lua_State *L) {
  lua_Debug ar;
  ਪੂਰਨ li = 1, le = 1;
  /* find an upper bound */
  ਜਦੋਂ (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  ਜਦੋਂ (li < le) {
    ਪੂਰਨ m = (li + le)/2;
    ਜੇ (lua_getstack(L, m, &ar)) li = m + 1;
    ਨਹੀਂਤਾਂ le = m;
  }
  ਵਾਪਸ le - 1;
}


LUALIB_API ਖਾਲੀ luaL_traceback (lua_State *L, lua_State *L1,
                                ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ level) {
  lua_L_ਬਫਰ b;
  lua_Debug ar;
  ਪੂਰਨ ਆਖਰੀ = lastlevel(L1);
  ਪੂਰਨ limit2show = (ਆਖਰੀ - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  ਜੇ (ਸੁਨੇਹਾ) {
    luaL_addstring(&b, ਸੁਨੇਹਾ);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  ਜਦੋਂ (lua_getstack(L1, level++, &ar)) {
    ਜੇ (limit2show-- == 0) {  /* too many levels? */
      ਪੂਰਨ n = ਆਖਰੀ - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    ਨਹੀਂਤਾਂ {
      lua_getinfo(L1, "Slnt", &ar);
      ਜੇ (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      ਨਹੀਂਤਾਂ
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      ਜੇ (ar.istailcall)
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

LUALIB_API ਪੂਰਨ luaL_argerror (lua_State *L, ਪੂਰਨ arg, ਅਚਲ ਅੱਖਰ *extramsg) {
  lua_Debug ar;
  ਅਚਲ ਅੱਖਰ *argword;
  ਜੇ (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ਵਾਪਸ luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  ਜੇ (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  ਨਹੀਂਤਾਂ {
    arg -= ar.extraargs;  /* do not count extra arguments */
    ਜੇ (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      ਜੇ (arg == 0)  /* error in self argument? */
        ਵਾਪਸ luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  ਜੇ (ar.name == ਨਿਰਾ)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  ਵਾਪਸ luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API ਪੂਰਨ luaL_typeerror (lua_State *L, ਪੂਰਨ arg, ਅਚਲ ਅੱਖਰ *tname) {
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;
  ਅਚਲ ਅੱਖਰ *typearg;  /* name for the type of the actual argument */
  ਜੇ (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  ਨਹੀਂਤਾਂ ਜੇ (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  ਨਹੀਂਤਾਂ
    typearg = luaL_typename(L, arg);  /* standard name */
  ਸੁਨੇਹਾ = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  ਵਾਪਸ luaL_argerror(L, arg, ਸੁਨੇਹਾ);
}


ਸਥਿਰ ਖਾਲੀ ਟੈਗ_error (lua_State *L, ਪੂਰਨ arg, ਪੂਰਨ ਟੈਗ) {
  luaL_typeerror(L, arg, lua_typename(L, ਟੈਗ));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API ਖਾਲੀ luaL_where (lua_State *L, ਪੂਰਨ level) {
  lua_Debug ar;
  ਜੇ (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    ਜੇ (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      ਵਾਪਸ;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API ਪੂਰਨ luaL_error (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  ਵਾਪਸ lua_error(L);
}


LUALIB_API ਪੂਰਨ luaL_fileresult (lua_State *L, ਪੂਰਨ stat, ਅਚਲ ਅੱਖਰ *fname) {
  ਪੂਰਨ en = errno;  /* calls to Lua API may change this value */
  ਜੇ (stat) {
    lua_pushboolean(L, 1);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;
    luaL_pushfail(L);
    ਸੁਨੇਹਾ = (en != 0) ? strerror(en) : "(no extra info)";
    ਜੇ (fname)
      lua_pushfstring(L, "%s: %s", fname, ਸੁਨੇਹਾ);
    ਨਹੀਂਤਾਂ
      lua_pushstring(L, ਸੁਨੇਹਾ);
    lua_pushinteger(L, en);
    ਵਾਪਸ 3;
  }
}


#ਜੇ !defined(l_inspectstat)	/* { */

#ਜੇ defined(LUA_USE_POSIX)

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_inspectstat(stat,what)  \
   ਜੇ (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   ਨਹੀਂਤਾਂ ਜੇ (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#ਨਹੀਂਤਾਂ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_inspectstat(stat,what)  /* no op */

#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ				/* } */


LUALIB_API ਪੂਰਨ luaL_execresult (lua_State *L, ਪੂਰਨ stat) {
  ਜੇ (stat != 0 && errno != 0)  /* error with an 'errno'? */
    ਵਾਪਸ luaL_fileresult(L, 0, ਨਿਰਾ);
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    ਜੇ (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    ਨਹੀਂਤਾਂ
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    ਵਾਪਸ 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API ਪੂਰਨ luaL_newmetatable (lua_State *L, ਅਚਲ ਅੱਖਰ *tname) {
  ਜੇ (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    ਵਾਪਸ 0;  /* leave previous value on top, but return 0 */
  lua_ਕੱਢੋ(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  ਵਾਪਸ 1;
}


LUALIB_API ਖਾਲੀ luaL_setmetatable (lua_State *L, ਅਚਲ ਅੱਖਰ *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API ਖਾਲੀ *luaL_testudata (lua_State *L, ਪੂਰਨ ud, ਅਚਲ ਅੱਖਰ *tname) {
  ਖਾਲੀ *p = lua_touserdata(L, ud);
  ਜੇ (p != ਨਿਰਾ) {  /* value is a userdata? */
    ਜੇ (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      ਜੇ (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ਨਿਰਾ;  /* value is a userdata with wrong metatable */
      lua_ਕੱਢੋ(L, 2);  /* remove both metatables */
      ਵਾਪਸ p;
    }
  }
  ਵਾਪਸ ਨਿਰਾ;  /* value is not a userdata with a metatable */
}


LUALIB_API ਖਾਲੀ *luaL_checkudata (lua_State *L, ਪੂਰਨ ud, ਅਚਲ ਅੱਖਰ *tname) {
  ਖਾਲੀ *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ਨਿਰਾ, ud, tname);
  ਵਾਪਸ p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API ਪੂਰਨ luaL_checkoption (lua_State *L, ਪੂਰਨ arg, ਅਚਲ ਅੱਖਰ *def,
                                 ਅਚਲ ਅੱਖਰ *ਅਚਲ lst[]) {
  ਅਚਲ ਅੱਖਰ *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  ਪੂਰਨ i;
  ਲਈ (i=0; lst[i]; i++)
    ਜੇ (strcmp(lst[i], name) == 0)
      ਵਾਪਸ i;
  ਵਾਪਸ luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API ਖਾਲੀ luaL_checkstack (lua_State *L, ਪੂਰਨ space, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
  ਜੇ (l_unlikely(!lua_checkstack(L, space))) {
    ਜੇ (ਸੁਨੇਹਾ)
      luaL_error(L, "stack overflow (%s)", ਸੁਨੇਹਾ);
    ਨਹੀਂਤਾਂ
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API ਖਾਲੀ luaL_checktype (lua_State *L, ਪੂਰਨ arg, ਪੂਰਨ t) {
  ਜੇ (l_unlikely(lua_type(L, arg) != t))
    ਟੈਗ_error(L, arg, t);
}


LUALIB_API ਖਾਲੀ luaL_checkany (lua_State *L, ਪੂਰਨ arg) {
  ਜੇ (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ਅਚਲ ਅੱਖਰ *luaL_checklstring (lua_State *L, ਪੂਰਨ arg, size_t *len) {
  ਅਚਲ ਅੱਖਰ *s = lua_tolstring(L, arg, len);
  ਜੇ (l_unlikely(!s)) ਟੈਗ_error(L, arg, LUA_TSTRING);
  ਵਾਪਸ s;
}


LUALIB_API ਅਚਲ ਅੱਖਰ *luaL_optlstring (lua_State *L, ਪੂਰਨ arg,
                                        ਅਚਲ ਅੱਖਰ *def, size_t *len) {
  ਜੇ (lua_isnoneornil(L, arg)) {
    ਜੇ (len)
      *len = (def ? ਲੜੀ_ਲੰਬਾਈ(def) : 0);
    ਵਾਪਸ def;
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ luaL_checklstring(L, arg, len);
}


LUALIB_API lua_ਸੰਖਿਆ luaL_checknumber (lua_State *L, ਪੂਰਨ arg) {
  ਪੂਰਨ isnum;
  lua_ਸੰਖਿਆ d = lua_tonumberx(L, arg, &isnum);
  ਜੇ (l_unlikely(!isnum))
    ਟੈਗ_error(L, arg, LUA_TNUMBER);
  ਵਾਪਸ d;
}


LUALIB_API lua_ਸੰਖਿਆ luaL_optnumber (lua_State *L, ਪੂਰਨ arg, lua_ਸੰਖਿਆ def) {
  ਵਾਪਸ luaL_opt(L, luaL_checknumber, arg, def);
}


ਸਥਿਰ ਖਾਲੀ interror (lua_State *L, ਪੂਰਨ arg) {
  ਜੇ (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  ਨਹੀਂਤਾਂ
    ਟੈਗ_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, ਪੂਰਨ arg) {
  ਪੂਰਨ isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  ਜੇ (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  ਵਾਪਸ d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, ਪੂਰਨ arg,
                                                      lua_Integer def) {
  ਵਾਪਸ luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ UBox {
  ਖਾਲੀ *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
ਸਥਿਰ ਖਾਲੀ *resizebox (lua_State *L, ਪੂਰਨ idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  ਜੇ (box->bsize == newsize)  /* not changing size? */
    ਵਾਪਸ box->box;  /* keep the buffer */
  ਨਹੀਂਤਾਂ {
    ਖਾਲੀ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    ਖਾਲੀ *ਆਰਜ਼ੀ = allocf(ud, box->box, box->bsize, newsize);
    ਜੇ (l_unlikely(ਆਰਜ਼ੀ == ਨਿਰਾ && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = ਆਰਜ਼ੀ;
    box->bsize = newsize;
    ਵਾਪਸ ਆਰਜ਼ੀ;
  }
}


ਸਥਿਰ ਪੂਰਨ boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਅਚਲ luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ਨਿਰਾ, ਨਿਰਾ}
};


/*
** Get/create metatable (MT) for boxes
*/
ਸਥਿਰ ਖਾਲੀ getBoxMT (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *BOXMT = "_UBOX*"; /* key for the metatable */
  ਜੇ (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


ਸਥਿਰ ਖਾਲੀ newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, ਆਕਾਰ(UBox), 0);
  box->box = ਨਿਰਾ;
  box->bsize = 0;
  getBoxMT(L);
  lua_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checkbufferlevel(B,idx)  \
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ਨਿਰਾ  \
                            : lua_touserdata(B->L, idx) == (ਖਾਲੀ*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
ਸਥਿਰ size_t newbuffsize (lua_L_ਬਫਰ *B, size_t sz) {
  size_t newsize = B->ਮਾਪ;
  ਜੇ (l_unlikely(sz >= MAX_ਮਾਪ - B->n))
    ਵਾਪਸ cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  ਜੇ (newsize <= MAX_ਮਾਪ/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  ਜੇ (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  ਵਾਪਸ newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
ਸਥਿਰ ਅੱਖਰ *prepbuffsize (lua_L_ਬਫਰ *B, size_t sz, ਪੂਰਨ boxidx) {
  checkbufferlevel(B, boxidx);
  ਜੇ (B->ਮਾਪ - B->n >= sz)  /* enough space? */
    ਵਾਪਸ B->b + B->n;
  ਨਹੀਂਤਾਂ {
    lua_State *L = B->L;
    ਅੱਖਰ *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    ਜੇ (buffonstack(B))  /* buffer already has a box? */
      newbuff = (ਅੱਖਰ *)resizebox(L, boxidx, newsize);  /* resize it */
    ਨਹੀਂਤਾਂ {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_ਪਾਓ(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (ਅੱਖਰ *)resizebox(L, boxidx, newsize);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(newbuff, B->b, B->n * ਆਕਾਰ(ਅੱਖਰ));  /* copy original content */
    }
    B->b = newbuff;
    B->ਮਾਪ = newsize;
    ਵਾਪਸ newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API ਅੱਖਰ *luaL_prepbuffsize (lua_L_ਬਫਰ *B, size_t sz) {
  ਵਾਪਸ prepbuffsize(B, sz, -1);
}


LUALIB_API ਖਾਲੀ luaL_addlstring (lua_L_ਬਫਰ *B, ਅਚਲ ਅੱਖਰ *s, size_t l) {
  ਜੇ (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    ਅੱਖਰ *b = prepbuffsize(B, l, -1);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(b, s, l * ਆਕਾਰ(ਅੱਖਰ));
    luaL_addsize(B, l);
  }
}


LUALIB_API ਖਾਲੀ luaL_addstring (lua_L_ਬਫਰ *B, ਅਚਲ ਅੱਖਰ *s) {
  luaL_addlstring(B, s, ਲੜੀ_ਲੰਬਾਈ(s));
}


LUALIB_API ਖਾਲੀ luaL_pushresult (lua_L_ਬਫਰ *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  ਜੇ (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  ਨਹੀਂਤਾਂ {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    ਖਾਲੀ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    ਅੱਖਰ *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (ਅੱਖਰ*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ਨਿਰਾ;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API ਖਾਲੀ luaL_pushresultsize (lua_L_ਬਫਰ *B, size_t sz) {
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
LUALIB_API ਖਾਲੀ luaL_addvalue (lua_L_ਬਫਰ *B) {
  lua_State *L = B->L;
  size_t len;
  ਅਚਲ ਅੱਖਰ *s = lua_tolstring(L, -1, &len);
  ਅੱਖਰ *b = prepbuffsize(B, len, -2);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(b, s, len * ਆਕਾਰ(ਅੱਖਰ));
  luaL_addsize(B, len);
  lua_ਕੱਢੋ(L, 1);  /* pop string */
}


LUALIB_API ਖਾਲੀ luaL_buffinit (lua_State *L, lua_L_ਬਫਰ *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->ਮਾਪ = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (ਖਾਲੀ*)B);  /* push placeholder */
}


LUALIB_API ਅੱਖਰ *luaL_buffinitsize (lua_State *L, lua_L_ਬਫਰ *B, size_t sz) {
  luaL_buffinit(L, B);
  ਵਾਪਸ prepbuffsize(B, sz, -1);
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
LUALIB_API ਪੂਰਨ luaL_ref (lua_State *L, ਪੂਰਨ t) {
  ਪੂਰਨ ref;
  ਜੇ (lua_isnil(L, -1)) {
    lua_ਕੱਢੋ(L, 1);  /* remove from stack */
    ਵਾਪਸ LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  ਜੇ (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (ਪੂਰਨ)lua_tointeger(L, -1);  /* ref = t[1] */
  ਨਹੀਂਤਾਂ {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_ਕੱਢੋ(L, 1);  /* remove element from stack */
  ਜੇ (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  ਨਹੀਂਤਾਂ  /* no free elements */
    ref = (ਪੂਰਨ)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  ਵਾਪਸ ref;
}


LUALIB_API ਖਾਲੀ luaL_unref (lua_State *L, ਪੂਰਨ t, ਪੂਰਨ ref) {
  ਜੇ (ref >= 0) {
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

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਲੱਦੋ_F {
  ਬਿਨਾਨਿਸ਼ਾਨੀ n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  ਅੱਖਰ buff[BUFSIZ];  /* area for reading file */
} ਲੱਦੋ_F;


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getF (lua_State *L, ਖਾਲੀ *ud, size_t *ਮਾਪ) {
  ਲੱਦੋ_F *lf = (ਲੱਦੋ_F *)ud;
  ਅਣਵਰਤਿਆ(L);
  ਜੇ (lf->n > 0) {  /* are there pre-read characters to be read? */
    *ਮਾਪ = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  ਨਹੀਂਤਾਂ {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    ਜੇ (feof(lf->f)) ਵਾਪਸ ਨਿਰਾ;
    *ਮਾਪ = fread(lf->buff, 1, ਆਕਾਰ(lf->buff), lf->f);  /* read block */
  }
  ਵਾਪਸ lf->buff;
}


ਸਥਿਰ ਪੂਰਨ errfile (lua_State *L, ਅਚਲ ਅੱਖਰ *what, ਪੂਰਨ fnameindex) {
  ਪੂਰਨ err = errno;
  ਅਚਲ ਅੱਖਰ *filename = lua_tostring(L, fnameindex) + 1;
  ਜੇ (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  ਨਹੀਂਤਾਂ
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  ਵਾਪਸ LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
ਸਥਿਰ ਪੂਰਨ skipBOM (FILE *f) {
  ਪੂਰਨ c = getc(f);  /* read first character */
  ਜੇ (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    ਵਾਪਸ getc(f);  /* ignore BOM and return next char */
  ਨਹੀਂਤਾਂ  /* no (valid) BOM */
    ਵਾਪਸ c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
ਸਥਿਰ ਪੂਰਨ skipcomment (FILE *f, ਪੂਰਨ *cp) {
  ਪੂਰਨ c = *cp = skipBOM(f);
  ਜੇ (c == '#') {  /* first line is a comment (Unix exec. file)? */
    ਕਰੋ {  /* skip first line */
      c = getc(f);
    } ਜਦੋਂ (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    ਵਾਪਸ 1;  /* there was a comment */
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ 0;  /* no comment */
}


LUALIB_API ਪੂਰਨ luaL_loadfilex (lua_State *L, ਅਚਲ ਅੱਖਰ *filename,
                                             ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
  ਲੱਦੋ_F lf;
  ਪੂਰਨ ਸਥਿਤੀ, readstatus;
  ਪੂਰਨ c;
  ਪੂਰਨ fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ਜੇ (filename == ਨਿਰਾ) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  ਨਹੀਂਤਾਂ {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    ਜੇ (lf.f == ਨਿਰਾ) ਵਾਪਸ errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  ਜੇ (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  ਜੇ (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    ਜੇ (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      ਜੇ (lf.f == ਨਿਰਾ) ਵਾਪਸ errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  ਜੇ (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  ਸਥਿਤੀ = lua_ਲੱਦੋ(L, getF, &lf, lua_tostring(L, -1), ਬਹੁਲਕ);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  ਜੇ (filename) fclose(lf.f);  /* close file (even in case of errors) */
  ਜੇ (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    ਵਾਪਸ errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  ਵਾਪਸ ਸਥਿਤੀ;
}


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਲੱਦੋ_S {
  ਅਚਲ ਅੱਖਰ *s;
  size_t ਮਾਪ;
} ਲੱਦੋ_S;


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getS (lua_State *L, ਖਾਲੀ *ud, size_t *ਮਾਪ) {
  ਲੱਦੋ_S *ls = (ਲੱਦੋ_S *)ud;
  ਅਣਵਰਤਿਆ(L);
  ਜੇ (ls->ਮਾਪ == 0) ਵਾਪਸ ਨਿਰਾ;
  *ਮਾਪ = ls->ਮਾਪ;
  ls->ਮਾਪ = 0;
  ਵਾਪਸ ls->s;
}


LUALIB_API ਪੂਰਨ luaL_loadbufferx (lua_State *L, ਅਚਲ ਅੱਖਰ *buff, size_t ਮਾਪ,
                                 ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ) {
  ਲੱਦੋ_S ls;
  ls.s = buff;
  ls.ਮਾਪ = ਮਾਪ;
  ਵਾਪਸ lua_ਲੱਦੋ(L, getS, &ls, name, ਬਹੁਲਕ);
}


LUALIB_API ਪੂਰਨ luaL_loadstring (lua_State *L, ਅਚਲ ਅੱਖਰ *s) {
  ਵਾਪਸ luaL_loadbufferx(L, s, ਲੜੀ_ਲੰਬਾਈ(s), s, "t");
}

/* }====================================================== */



LUALIB_API ਪੂਰਨ luaL_getmetafield (lua_State *L, ਪੂਰਨ obj, ਅਚਲ ਅੱਖਰ *event) {
  ਜੇ (!lua_getmetatable(L, obj))  /* no metatable? */
    ਵਾਪਸ LUA_TNIL;
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    ਜੇ (tt == LUA_TNIL)  /* is metafield nil? */
      lua_ਕੱਢੋ(L, 2);  /* remove metatable and metafield */
    ਨਹੀਂਤਾਂ
      lua_remove(L, -2);  /* remove only metatable */
    ਵਾਪਸ tt;  /* return metafield type */
  }
}


LUALIB_API ਪੂਰਨ luaL_callmeta (lua_State *L, ਪੂਰਨ obj, ਅਚਲ ਅੱਖਰ *event) {
  obj = lua_absindex(L, obj);
  ਜੇ (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    ਵਾਪਸ 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  ਵਾਪਸ 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, ਪੂਰਨ idx) {
  lua_Integer l;
  ਪੂਰਨ isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  ਜੇ (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_ਕੱਢੋ(L, 1);  /* remove object */
  ਵਾਪਸ l;
}


LUALIB_API ਅਚਲ ਅੱਖਰ *luaL_tolstring (lua_State *L, ਪੂਰਨ idx, size_t *len) {
  idx = lua_absindex(L,idx);
  ਜੇ (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    ਜੇ (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  ਨਹੀਂਤਾਂ {
    ਚੋਣ (lua_type(L, idx)) {
      ਹਾਲਤ LUA_TNUMBER: {
        ਅੱਖਰ buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        ਰੋਕੋ;
      }
      ਹਾਲਤ LUA_TSTRING:
        lua_pushvalue(L, idx);
        ਰੋਕੋ;
      ਹਾਲਤ LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        ਰੋਕੋ;
      ਹਾਲਤ LUA_TNIL:
        lua_pushliteral(L, "nil");
        ਰੋਕੋ;
      ਮੂਲ: {
        ਪੂਰਨ tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ਅਚਲ ਅੱਖਰ *ਕਿਸਮ = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", ਕਿਸਮ, lua_topointer(L, idx));
        ਜੇ (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        ਰੋਕੋ;
      }
    }
  }
  ਵਾਪਸ lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API ਖਾਲੀ luaL_setfuncs (lua_State *L, ਅਚਲ luaL_Reg *l, ਪੂਰਨ nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  ਲਈ (; l->name != ਨਿਰਾ; l++) {  /* fill the table with given functions */
    ਜੇ (l->func == ਨਿਰਾ)  /* placeholder? */
      lua_pushboolean(L, 0);
    ਨਹੀਂਤਾਂ {
      ਪੂਰਨ i;
      ਲਈ (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_ਕੱਢੋ(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API ਪੂਰਨ luaL_getsubtable (lua_State *L, ਪੂਰਨ idx, ਅਚਲ ਅੱਖਰ *fname) {
  ਜੇ (lua_getfield(L, idx, fname) == LUA_TTABLE)
    ਵਾਪਸ 1;  /* table already there */
  ਨਹੀਂਤਾਂ {
    lua_ਕੱਢੋ(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    ਵਾਪਸ 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API ਖਾਲੀ luaL_requiref (lua_State *L, ਅਚਲ ਅੱਖਰ *modname,
                               lua_CFunction openf, ਪੂਰਨ glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_ਸਾਰਣੀ);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  ਜੇ (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_ਕੱਢੋ(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  ਜੇ (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API ਖਾਲੀ luaL_addgsub (lua_L_ਬਫਰ *b, ਅਚਲ ਅੱਖਰ *s,
                                     ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *r) {
  ਅਚਲ ਅੱਖਰ *wild;
  size_t l = ਲੜੀ_ਲੰਬਾਈ(p);
  ਜਦੋਂ ((wild = ਲੜੀ_ਖੋਜ(s, p)) != ਨਿਰਾ) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ਅਚਲ ਅੱਖਰ *luaL_gsub (lua_State *L, ਅਚਲ ਅੱਖਰ *s,
                                  ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *r) {
  lua_L_ਬਫਰ b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  ਵਾਪਸ lua_tostring(L, -1);
}


ਖਾਲੀ *luaL_alloc (ਖਾਲੀ *ud, ਖਾਲੀ *ptr, size_t osize, size_t nsize) {
  ਅਣਵਰਤਿਆ(ud); ਅਣਵਰਤਿਆ(osize);
  ਜੇ (nsize == 0) {
    ਮੁਕਤ(ptr);
    ਵਾਪਸ ਨਿਰਾ;
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
ਸਥਿਰ ਪੂਰਨ panic (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        ਸੁਨੇਹਾ);
  ਵਾਪਸ 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
ਸਥਿਰ ਖਾਲੀ warnfoff (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ਪੂਰਨ tocont);
ਸਥਿਰ ਖਾਲੀ warnfon (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ਪੂਰਨ tocont);
ਸਥਿਰ ਖਾਲੀ warnfcont (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ਪੂਰਨ tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
ਸਥਿਰ ਪੂਰਨ checkcontrol (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ਪੂਰਨ tocont) {
  ਜੇ (tocont || *(ਸੁਨੇਹਾ_2++) != '@')  /* not a control message? */
    ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ {
    ਜੇ (strcmp(ਸੁਨੇਹਾ_2, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਸੁਨੇਹਾ_2, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    ਵਾਪਸ 1;  /* it was a control message */
  }
}


ਸਥਿਰ ਖਾਲੀ warnfoff (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ਪੂਰਨ tocont) {
  checkcontrol((lua_State *)ud, ਸੁਨੇਹਾ_2, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
ਸਥਿਰ ਖਾਲੀ warnfcont (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ਪੂਰਨ tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", ਸੁਨੇਹਾ_2);  /* write message */
  ਜੇ (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  ਨਹੀਂਤਾਂ {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


ਸਥਿਰ ਖਾਲੀ warnfon (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ_2, ਪੂਰਨ tocont) {
  ਜੇ (checkcontrol((lua_State *)ud, ਸੁਨੇਹਾ_2, tocont))  /* control message? */
    ਵਾਪਸ;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, ਸੁਨੇਹਾ_2, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#ਜੇ !defined(luai_makeseed)

#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>


/* Size for the buffer, in bytes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFSEEDB	(ਆਕਾਰ(ਖਾਲੀ*) + ਆਕਾਰ(time_t))

/* Size for the buffer in int's, rounded up */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFSEED		((BUFSEEDB + ਆਕਾਰ(ਪੂਰਨ) - 1) / ਆਕਾਰ(ਪੂਰਨ))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ addbuff(b,v)	(ਸਮ੍ਰਿਤੀ_ਨਕਲ(&b[0], &(v), ਆਕਾਰ(v)), b += ਆਕਾਰ(v))


ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ luai_makeseed (ਖਾਲੀ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ buff[BUFSEED];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ res;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  time_t t = time(ਨਿਰਾ);
  ਅੱਖਰ *b = (ਅੱਖਰ*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /*ਸਮਾਂ*/
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, ਆਕਾਰ(buff) - BUFSEEDB);
  res = buff[0];
  ਲਈ (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  ਵਾਪਸ res;
}

#ਜੇ_ਅੰਤ


LUALIB_API ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ luaL_makeseed (lua_State *L) {
  ਅਣਵਰਤਿਆ(L);
  ਵਾਪਸ luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (ਖਾਲੀ) {
  lua_State *L = lua_newstate(luaL_alloc, ਨਿਰਾ, luaL_makeseed(ਨਿਰਾ));
  ਜੇ (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  ਵਾਪਸ L;
}


LUALIB_API ਖਾਲੀ luaL_checkversion_ (lua_State *L, lua_ਸੰਖਿਆ ver, size_t sz) {
  lua_ਸੰਖਿਆ v = lua_ਸੰਸਕਰਣ(L);
  ਜੇ (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  ਨਹੀਂਤਾਂ ਜੇ (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

