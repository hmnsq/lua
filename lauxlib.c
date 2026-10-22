/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "llimits.h"


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
स्थिर पूर्णाङ्क findfield (lua_State *L, पूर्णाङ्क objidx, पूर्णाङ्क level) {
  जइ (level == 0 || !lua_istable(L, -1))
    प्रत्यागम 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  जाव (lua_next(L, -2)) {  /* for each pair in table */
    जइ (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      जइ (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_निष्कासय(L, 1);  /* remove value (but keep name) */
        प्रत्यागम 1;
      }
      अण्णहा जइ (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        प्रत्यागम 1;
      }
    }
    lua_निष्कासय(L, 1);  /* remove value */
  }
  प्रत्यागम 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
स्थिर पूर्णाङ्क pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  पूर्णाङ्क शिखरम् = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  जइ (findfield(L, शिखरम् + 1, 2)) {
    ध्रुव वर्ण *name = lua_tostring(L, -1);
    जइ (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, शिखरम् + 1);  /* copy name to proper place */
    lua_settop(L, शिखरम् + 1);  /* remove table "loaded" and name copy */
    प्रत्यागम 1;
  }
  अण्णहा {
    lua_settop(L, शिखरम्);  /* remove function and global table */
    प्रत्यागम 0;
  }
}


स्थिर शून्य pushfuncname (lua_State *L, lua_Debug *ar) {
  जइ (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  अण्णहा जइ (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  अण्णहा जइ (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  अण्णहा जइ (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  अण्णहा  /* nothing left... */
    lua_pushliteral(L, "?");
}


स्थिर पूर्णाङ्क lastlevel (lua_State *L) {
  lua_Debug ar;
  पूर्णाङ्क li = 1, le = 1;
  /* find an upper bound */
  जाव (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  जाव (li < le) {
    पूर्णाङ्क m = (li + le)/2;
    जइ (lua_getstack(L, m, &ar)) li = m + 1;
    अण्णहा le = m;
  }
  प्रत्यागम le - 1;
}


LUALIB_API शून्य luaL_traceback (lua_State *L, lua_State *L1,
                                ध्रुव वर्ण *सन्देशः, पूर्णाङ्क level) {
  luaL_Buffer b;
  lua_Debug ar;
  पूर्णाङ्क last = lastlevel(L1);
  पूर्णाङ्क limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  जइ (सन्देशः) {
    luaL_addstring(&b, सन्देशः);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  जाव (lua_getstack(L1, level++, &ar)) {
    जइ (limit2show-- == 0) {  /* too many levels? */
      पूर्णाङ्क n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    अण्णहा {
      lua_getinfo(L1, "Slnt", &ar);
      जइ (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      अण्णहा
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      जइ (ar.istailcall)
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

LUALIB_API पूर्णाङ्क luaL_argerror (lua_State *L, पूर्णाङ्क arg, ध्रुव वर्ण *extramsg) {
  lua_Debug ar;
  ध्रुव वर्ण *argword;
  जइ (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    प्रत्यागम luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  जइ (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  अण्णहा {
    arg -= ar.extraargs;  /* do not count extra arguments */
    जइ (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      जइ (arg == 0)  /* error in self argument? */
        प्रत्यागम luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  जइ (ar.name == रिक्त)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  प्रत्यागम luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API पूर्णाङ्क luaL_typeerror (lua_State *L, पूर्णाङ्क arg, ध्रुव वर्ण *tname) {
  ध्रुव वर्ण *सन्देशः;
  ध्रुव वर्ण *typearg;  /* name for the type of the actual argument */
  जइ (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  अण्णहा जइ (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  अण्णहा
    typearg = luaL_typename(L, arg);  /* standard name */
  सन्देशः = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  प्रत्यागम luaL_argerror(L, arg, सन्देशः);
}


स्थिर शून्य tag_error (lua_State *L, पूर्णाङ्क arg, पूर्णाङ्क tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API शून्य luaL_where (lua_State *L, पूर्णाङ्क level) {
  lua_Debug ar;
  जइ (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    जइ (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      प्रत्यागम;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API पूर्णाङ्क luaL_error (lua_State *L, ध्रुव वर्ण *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  प्रत्यागम lua_error(L);
}


LUALIB_API पूर्णाङ्क luaL_fileresult (lua_State *L, पूर्णाङ्क stat, ध्रुव वर्ण *fname) {
  पूर्णाङ्क en = errno;  /* calls to Lua API may change this value */
  जइ (stat) {
    lua_pushboolean(L, 1);
    प्रत्यागम 1;
  }
  अण्णहा {
    ध्रुव वर्ण *सन्देशः;
    luaL_pushfail(L);
    सन्देशः = (en != 0) ? strerror(en) : "(no extra info)";
    जइ (fname)
      lua_pushfstring(L, "%s: %s", fname, सन्देशः);
    अण्णहा
      lua_pushstring(L, सन्देशः);
    lua_pushinteger(L, en);
    प्रत्यागम 3;
  }
}


#जइ !defined(l_inspectstat)	/* { */

#जइ defined(LUA_USE_POSIX)

#अन्तर्भाव <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   जइ (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   अण्णहा जइ (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#अण्णहा

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API पूर्णाङ्क luaL_execresult (lua_State *L, पूर्णाङ्क stat) {
  जइ (stat != 0 && errno != 0)  /* error with an 'errno'? */
    प्रत्यागम luaL_fileresult(L, 0, रिक्त);
  अण्णहा {
    ध्रुव वर्ण *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    जइ (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    अण्णहा
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    प्रत्यागम 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API पूर्णाङ्क luaL_newmetatable (lua_State *L, ध्रुव वर्ण *tname) {
  जइ (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    प्रत्यागम 0;  /* leave previous value on top, but return 0 */
  lua_निष्कासय(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  प्रत्यागम 1;
}


LUALIB_API शून्य luaL_setmetatable (lua_State *L, ध्रुव वर्ण *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API शून्य *luaL_testudata (lua_State *L, पूर्णाङ्क ud, ध्रुव वर्ण *tname) {
  शून्य *p = lua_touserdata(L, ud);
  जइ (p != रिक्त) {  /* value is a userdata? */
    जइ (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      जइ (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = रिक्त;  /* value is a userdata with wrong metatable */
      lua_निष्कासय(L, 2);  /* remove both metatables */
      प्रत्यागम p;
    }
  }
  प्रत्यागम रिक्त;  /* value is not a userdata with a metatable */
}


LUALIB_API शून्य *luaL_checkudata (lua_State *L, पूर्णाङ्क ud, ध्रुव वर्ण *tname) {
  शून्य *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != रिक्त, ud, tname);
  प्रत्यागम p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API पूर्णाङ्क luaL_checkoption (lua_State *L, पूर्णाङ्क arg, ध्रुव वर्ण *def,
                                 ध्रुव वर्ण *ध्रुव lst[]) {
  ध्रुव वर्ण *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  पूर्णाङ्क i;
  प्रति (i=0; lst[i]; i++)
    जइ (strcmp(lst[i], name) == 0)
      प्रत्यागम i;
  प्रत्यागम luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API शून्य luaL_checkstack (lua_State *L, पूर्णाङ्क space, ध्रुव वर्ण *सन्देशः) {
  जइ (l_unlikely(!lua_checkstack(L, space))) {
    जइ (सन्देशः)
      luaL_error(L, "stack overflow (%s)", सन्देशः);
    अण्णहा
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API शून्य luaL_checktype (lua_State *L, पूर्णाङ्क arg, पूर्णाङ्क t) {
  जइ (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API शून्य luaL_checkany (lua_State *L, पूर्णाङ्क arg) {
  जइ (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ध्रुव वर्ण *luaL_checklstring (lua_State *L, पूर्णाङ्क arg, size_t *len) {
  ध्रुव वर्ण *s = lua_tolstring(L, arg, len);
  जइ (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  प्रत्यागम s;
}


LUALIB_API ध्रुव वर्ण *luaL_optlstring (lua_State *L, पूर्णाङ्क arg,
                                        ध्रुव वर्ण *def, size_t *len) {
  जइ (lua_isnoneornil(L, arg)) {
    जइ (len)
      *len = (def ? strlen(def) : 0);
    प्रत्यागम def;
  }
  अण्णहा प्रत्यागम luaL_checklstring(L, arg, len);
}


LUALIB_API lua_सङ्ख्या luaL_checknumber (lua_State *L, पूर्णाङ्क arg) {
  पूर्णाङ्क isnum;
  lua_सङ्ख्या d = lua_tonumberx(L, arg, &isnum);
  जइ (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  प्रत्यागम d;
}


LUALIB_API lua_सङ्ख्या luaL_optnumber (lua_State *L, पूर्णाङ्क arg, lua_सङ्ख्या def) {
  प्रत्यागम luaL_opt(L, luaL_checknumber, arg, def);
}


स्थिर शून्य interror (lua_State *L, पूर्णाङ्क arg) {
  जइ (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  अण्णहा
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, पूर्णाङ्क arg) {
  पूर्णाङ्क isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  जइ (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  प्रत्यागम d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, पूर्णाङ्क arg,
                                                      lua_Integer def) {
  प्रत्यागम luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
प्रकारनिर्देश संरचना UBox {
  शून्य *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
स्थिर शून्य *resizebox (lua_State *L, पूर्णाङ्क idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  जइ (box->bsize == newsize)  /* not changing size? */
    प्रत्यागम box->box;  /* keep the buffer */
  अण्णहा {
    शून्य *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    शून्य *temp = allocf(ud, box->box, box->bsize, newsize);
    जइ (l_unlikely(temp == रिक्त && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    प्रत्यागम temp;
  }
}


स्थिर पूर्णाङ्क boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  प्रत्यागम 0;
}


स्थिर ध्रुव luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {रिक्त, रिक्त}
};


/*
** Get/create metatable (MT) for boxes
*/
स्थिर शून्य getBoxMT (lua_State *L) {
  ध्रुव वर्ण *BOXMT = "_UBOX*"; /* key for the metatable */
  जइ (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


स्थिर शून्य newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, परिमाण(UBox), 0);
  box->box = रिक्त;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != रिक्त  \
                            : lua_touserdata(B->L, idx) == (शून्य*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
स्थिर size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  जइ (l_unlikely(sz >= MAX_SIZE - B->n))
    प्रत्यागम cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  जइ (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  जइ (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  प्रत्यागम newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
स्थिर वर्ण *prepbuffsize (luaL_Buffer *B, size_t sz, पूर्णाङ्क boxidx) {
  checkbufferlevel(B, boxidx);
  जइ (B->size - B->n >= sz)  /* enough space? */
    प्रत्यागम B->b + B->n;
  अण्णहा {
    lua_State *L = B->L;
    वर्ण *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    जइ (buffonstack(B))  /* buffer already has a box? */
      newbuff = (वर्ण *)resizebox(L, boxidx, newsize);  /* resize it */
    अण्णहा {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_निवेशय(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (वर्ण *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * परिमाण(वर्ण));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    प्रत्यागम newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API वर्ण *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  प्रत्यागम prepbuffsize(B, sz, -1);
}


LUALIB_API शून्य luaL_addlstring (luaL_Buffer *B, ध्रुव वर्ण *s, size_t l) {
  जइ (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    वर्ण *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * परिमाण(वर्ण));
    luaL_addsize(B, l);
  }
}


LUALIB_API शून्य luaL_addstring (luaL_Buffer *B, ध्रुव वर्ण *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API शून्य luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  जइ (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  अण्णहा {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    शून्य *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    वर्ण *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (वर्ण*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = रिक्त;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API शून्य luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API शून्य luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  ध्रुव वर्ण *s = lua_tolstring(L, -1, &len);
  वर्ण *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * परिमाण(वर्ण));
  luaL_addsize(B, len);
  lua_निष्कासय(L, 1);  /* pop string */
}


LUALIB_API शून्य luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (शून्य*)B);  /* push placeholder */
}


LUALIB_API वर्ण *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  प्रत्यागम prepbuffsize(B, sz, -1);
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
LUALIB_API पूर्णाङ्क luaL_ref (lua_State *L, पूर्णाङ्क t) {
  पूर्णाङ्क ref;
  जइ (lua_isnil(L, -1)) {
    lua_निष्कासय(L, 1);  /* remove from stack */
    प्रत्यागम LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  जइ (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (पूर्णाङ्क)lua_tointeger(L, -1);  /* ref = t[1] */
  अण्णहा {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_निष्कासय(L, 1);  /* remove element from stack */
  जइ (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  अण्णहा  /* no free elements */
    ref = (पूर्णाङ्क)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  प्रत्यागम ref;
}


LUALIB_API शून्य luaL_unref (lua_State *L, पूर्णाङ्क t, पूर्णाङ्क ref) {
  जइ (ref >= 0) {
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

प्रकारनिर्देश संरचना LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  वर्ण buff[BUFSIZ];  /* area for reading file */
} LoadF;


स्थिर ध्रुव वर्ण *getF (lua_State *L, शून्य *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  जइ (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  अण्णहा {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    जइ (feof(lf->f)) प्रत्यागम रिक्त;
    *size = fread(lf->buff, 1, परिमाण(lf->buff), lf->f);  /* read block */
  }
  प्रत्यागम lf->buff;
}


स्थिर पूर्णाङ्क errfile (lua_State *L, ध्रुव वर्ण *what, पूर्णाङ्क fnameindex) {
  पूर्णाङ्क err = errno;
  ध्रुव वर्ण *filename = lua_tostring(L, fnameindex) + 1;
  जइ (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  अण्णहा
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  प्रत्यागम LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
स्थिर पूर्णाङ्क skipBOM (FILE *f) {
  पूर्णाङ्क c = getc(f);  /* read first character */
  जइ (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    प्रत्यागम getc(f);  /* ignore BOM and return next char */
  अण्णहा  /* no (valid) BOM */
    प्रत्यागम c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
स्थिर पूर्णाङ्क skipcomment (FILE *f, पूर्णाङ्क *cp) {
  पूर्णाङ्क c = *cp = skipBOM(f);
  जइ (c == '#') {  /* first line is a comment (Unix exec. file)? */
    करेहि {  /* skip first line */
      c = getc(f);
    } जाव (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    प्रत्यागम 1;  /* there was a comment */
  }
  अण्णहा प्रत्यागम 0;  /* no comment */
}


LUALIB_API पूर्णाङ्क luaL_loadfilex (lua_State *L, ध्रुव वर्ण *filename,
                                             ध्रुव वर्ण *mode) {
  LoadF lf;
  पूर्णाङ्क status, readstatus;
  पूर्णाङ्क c;
  पूर्णाङ्क fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  जइ (filename == रिक्त) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  अण्णहा {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    जइ (lf.f == रिक्त) प्रत्यागम errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  जइ (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  जइ (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    जइ (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      जइ (lf.f == रिक्त) प्रत्यागम errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  जइ (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  जइ (filename) fclose(lf.f);  /* close file (even in case of errors) */
  जइ (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    प्रत्यागम errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  प्रत्यागम status;
}


प्रकारनिर्देश संरचना LoadS {
  ध्रुव वर्ण *s;
  size_t size;
} LoadS;


स्थिर ध्रुव वर्ण *getS (lua_State *L, शून्य *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  जइ (ls->size == 0) प्रत्यागम रिक्त;
  *size = ls->size;
  ls->size = 0;
  प्रत्यागम ls->s;
}


LUALIB_API पूर्णाङ्क luaL_loadbufferx (lua_State *L, ध्रुव वर्ण *buff, size_t size,
                                 ध्रुव वर्ण *name, ध्रुव वर्ण *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  प्रत्यागम lua_load(L, getS, &ls, name, mode);
}


LUALIB_API पूर्णाङ्क luaL_loadstring (lua_State *L, ध्रुव वर्ण *s) {
  प्रत्यागम luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API पूर्णाङ्क luaL_getmetafield (lua_State *L, पूर्णाङ्क obj, ध्रुव वर्ण *event) {
  जइ (!lua_getmetatable(L, obj))  /* no metatable? */
    प्रत्यागम LUA_TNIL;
  अण्णहा {
    पूर्णाङ्क tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    जइ (tt == LUA_TNIL)  /* is metafield nil? */
      lua_निष्कासय(L, 2);  /* remove metatable and metafield */
    अण्णहा
      lua_remove(L, -2);  /* remove only metatable */
    प्रत्यागम tt;  /* return metafield type */
  }
}


LUALIB_API पूर्णाङ्क luaL_callmeta (lua_State *L, पूर्णाङ्क obj, ध्रुव वर्ण *event) {
  obj = lua_absindex(L, obj);
  जइ (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    प्रत्यागम 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  प्रत्यागम 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, पूर्णाङ्क idx) {
  lua_Integer l;
  पूर्णाङ्क isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  जइ (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_निष्कासय(L, 1);  /* remove object */
  प्रत्यागम l;
}


LUALIB_API ध्रुव वर्ण *luaL_tolstring (lua_State *L, पूर्णाङ्क idx, size_t *len) {
  idx = lua_absindex(L,idx);
  जइ (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    जइ (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  अण्णहा {
    विकल्प (lua_type(L, idx)) {
      पक्ष LUA_TNUMBER: {
        वर्ण buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        भङ्ग;
      }
      पक्ष LUA_TSTRING:
        lua_pushvalue(L, idx);
        भङ्ग;
      पक्ष LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        भङ्ग;
      पक्ष LUA_TNIL:
        lua_pushliteral(L, "nil");
        भङ्ग;
      स्वभाव: {
        पूर्णाङ्क tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ध्रुव वर्ण *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        जइ (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        भङ्ग;
      }
    }
  }
  प्रत्यागम lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API शून्य luaL_setfuncs (lua_State *L, ध्रुव luaL_Reg *l, पूर्णाङ्क nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  प्रति (; l->name != रिक्त; l++) {  /* fill the table with given functions */
    जइ (l->func == रिक्त)  /* placeholder? */
      lua_pushboolean(L, 0);
    अण्णहा {
      पूर्णाङ्क i;
      प्रति (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_निष्कासय(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API पूर्णाङ्क luaL_getsubtable (lua_State *L, पूर्णाङ्क idx, ध्रुव वर्ण *fname) {
  जइ (lua_getfield(L, idx, fname) == LUA_TTABLE)
    प्रत्यागम 1;  /* table already there */
  अण्णहा {
    lua_निष्कासय(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    प्रत्यागम 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API शून्य luaL_requiref (lua_State *L, ध्रुव वर्ण *modname,
                               lua_CFunction openf, पूर्णाङ्क glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  जइ (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_निष्कासय(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  जइ (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API शून्य luaL_addgsub (luaL_Buffer *b, ध्रुव वर्ण *s,
                                     ध्रुव वर्ण *p, ध्रुव वर्ण *r) {
  ध्रुव वर्ण *wild;
  size_t l = strlen(p);
  जाव ((wild = strstr(s, p)) != रिक्त) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ध्रुव वर्ण *luaL_gsub (lua_State *L, ध्रुव वर्ण *s,
                                  ध्रुव वर्ण *p, ध्रुव वर्ण *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  प्रत्यागम lua_tostring(L, -1);
}


शून्य *luaL_alloc (शून्य *ud, शून्य *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  जइ (nsize == 0) {
    मुक्त(ptr);
    प्रत्यागम रिक्त;
  }
  अण्णहा
    प्रत्यागम realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
स्थिर पूर्णाङ्क panic (lua_State *L) {
  ध्रुव वर्ण *सन्देशः = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        सन्देशः);
  प्रत्यागम 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
स्थिर शून्य warnfoff (शून्य *ud, ध्रुव वर्ण *message, पूर्णाङ्क tocont);
स्थिर शून्य warnfon (शून्य *ud, ध्रुव वर्ण *message, पूर्णाङ्क tocont);
स्थिर शून्य warnfcont (शून्य *ud, ध्रुव वर्ण *message, पूर्णाङ्क tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
स्थिर पूर्णाङ्क checkcontrol (lua_State *L, ध्रुव वर्ण *message, पूर्णाङ्क tocont) {
  जइ (tocont || *(message++) != '@')  /* not a control message? */
    प्रत्यागम 0;
  अण्णहा {
    जइ (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    अण्णहा जइ (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    प्रत्यागम 1;  /* it was a control message */
  }
}


स्थिर शून्य warnfoff (शून्य *ud, ध्रुव वर्ण *message, पूर्णाङ्क tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
स्थिर शून्य warnfcont (शून्य *ud, ध्रुव वर्ण *message, पूर्णाङ्क tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  जइ (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  अण्णहा {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


स्थिर शून्य warnfon (शून्य *ud, ध्रुव वर्ण *message, पूर्णाङ्क tocont) {
  जइ (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    प्रत्यागम;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#जइ !defined(luai_makeseed)

#अन्तर्भाव <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(परिमाण(शून्य*) + परिमाण(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + परिमाण(पूर्णाङ्क) - 1) / परिमाण(पूर्णाङ्क))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), परिमाण(v)), b += परिमाण(v))


स्थिर unsigned पूर्णाङ्क luai_makeseed (शून्य) {
  unsigned पूर्णाङ्क buff[BUFSEED];
  unsigned पूर्णाङ्क res;
  unsigned पूर्णाङ्क i;
  time_t t = time(रिक्त);
  वर्ण *b = (वर्ण*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, परिमाण(buff) - BUFSEEDB);
  res = buff[0];
  प्रति (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  प्रत्यागम res;
}

#endif


LUALIB_API unsigned पूर्णाङ्क luaL_makeseed (lua_State *L) {
  UNUSED(L);
  प्रत्यागम luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (शून्य) {
  lua_State *L = lua_newstate(luaL_alloc, रिक्त, luaL_makeseed(रिक्त));
  जइ (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  प्रत्यागम L;
}


LUALIB_API शून्य luaL_checkversion_ (lua_State *L, lua_सङ्ख्या ver, size_t sz) {
  lua_सङ्ख्या v = lua_version(L);
  जइ (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  अण्णहा जइ (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

