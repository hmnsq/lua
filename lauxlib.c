/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#ضمن "lprefix.h"


#ضمن <errno.h>
#ضمن <stdarg.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "llimits.h"


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
ساكن صحيح findfield (lua_State *L, صحيح objidx, صحيح level) {
  إذا (level == 0 || !lua_istable(L, -1))
    أرجع 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  طالما (lua_next(L, -2)) {  /* for each pair in table */
    إذا (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      إذا (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_چىقىرىش(L, 1);  /* remove value (but keep name) */
        أرجع 1;
      }
      وإلا إذا (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        أرجع 1;
      }
    }
    lua_چىقىرىش(L, 1);  /* remove value */
  }
  أرجع 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
ساكن صحيح pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  صحيح چوققا = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  إذا (findfield(L, چوققا + 1, 2)) {
    ثابت حرف *name = lua_tostring(L, -1);
    إذا (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, چوققا + 1);  /* copy name to proper place */
    lua_settop(L, چوققا + 1);  /* remove table "loaded" and name copy */
    أرجع 1;
  }
  وإلا {
    lua_settop(L, چوققا);  /* remove function and global table */
    أرجع 0;
  }
}


ساكن فراغ pushfuncname (lua_State *L, lua_Debug *ar) {
  إذا (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  وإلا إذا (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  وإلا إذا (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  وإلا إذا (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  وإلا  /* nothing left... */
    lua_pushliteral(L, "?");
}


ساكن صحيح lastlevel (lua_State *L) {
  lua_Debug ar;
  صحيح li = 1, le = 1;
  /* find an upper bound */
  طالما (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  طالما (li < le) {
    صحيح m = (li + le)/2;
    إذا (lua_getstack(L, m, &ar)) li = m + 1;
    وإلا le = m;
  }
  أرجع le - 1;
}


LUALIB_API فراغ luaL_traceback (lua_State *L, lua_State *L1,
                                ثابت حرف *ئۇچۇر, صحيح level) {
  luaL_Buffer b;
  lua_Debug ar;
  صحيح last = lastlevel(L1);
  صحيح limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  إذا (ئۇچۇر) {
    luaL_addstring(&b, ئۇچۇر);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  طالما (lua_getstack(L1, level++, &ar)) {
    إذا (limit2show-- == 0) {  /* too many levels? */
      صحيح n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    وإلا {
      lua_getinfo(L1, "Slnt", &ar);
      إذا (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      وإلا
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      إذا (ar.istailcall)
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

LUALIB_API صحيح luaL_argerror (lua_State *L, صحيح arg, ثابت حرف *extramsg) {
  lua_Debug ar;
  ثابت حرف *argword;
  إذا (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    أرجع luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  إذا (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  وإلا {
    arg -= ar.extraargs;  /* do not count extra arguments */
    إذا (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      إذا (arg == 0)  /* error in self argument? */
        أرجع luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  إذا (ar.name == عدم)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  أرجع luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API صحيح luaL_typeerror (lua_State *L, صحيح arg, ثابت حرف *tname) {
  ثابت حرف *ئۇچۇر;
  ثابت حرف *typearg;  /* name for the type of the actual argument */
  إذا (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  وإلا إذا (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  وإلا
    typearg = luaL_typename(L, arg);  /* standard name */
  ئۇچۇر = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  أرجع luaL_argerror(L, arg, ئۇچۇر);
}


ساكن فراغ tag_error (lua_State *L, صحيح arg, صحيح tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API فراغ luaL_where (lua_State *L, صحيح level) {
  lua_Debug ar;
  إذا (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    إذا (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      أرجع;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API صحيح luaL_error (lua_State *L, ثابت حرف *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  أرجع lua_error(L);
}


LUALIB_API صحيح luaL_fileresult (lua_State *L, صحيح stat, ثابت حرف *fname) {
  صحيح en = errno;  /* calls to Lua API may change this value */
  إذا (stat) {
    lua_pushboolean(L, 1);
    أرجع 1;
  }
  وإلا {
    ثابت حرف *ئۇچۇر;
    luaL_pushfail(L);
    ئۇچۇر = (en != 0) ? strerror(en) : "(no extra info)";
    إذا (fname)
      lua_pushfstring(L, "%s: %s", fname, ئۇچۇر);
    وإلا
      lua_pushstring(L, ئۇچۇر);
    lua_pushinteger(L, en);
    أرجع 3;
  }
}


#إذا !defined(l_inspectstat)	/* { */

#إذا defined(LUA_USE_POSIX)

#ضمن <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   إذا (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   وإلا إذا (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#وإلا

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API صحيح luaL_execresult (lua_State *L, صحيح stat) {
  إذا (stat != 0 && errno != 0)  /* error with an 'errno'? */
    أرجع luaL_fileresult(L, 0, عدم);
  وإلا {
    ثابت حرف *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    إذا (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    وإلا
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    أرجع 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API صحيح luaL_newmetatable (lua_State *L, ثابت حرف *tname) {
  إذا (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    أرجع 0;  /* leave previous value on top, but return 0 */
  lua_چىقىرىش(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  أرجع 1;
}


LUALIB_API فراغ luaL_setmetatable (lua_State *L, ثابت حرف *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API فراغ *luaL_testudata (lua_State *L, صحيح ud, ثابت حرف *tname) {
  فراغ *p = lua_touserdata(L, ud);
  إذا (p != عدم) {  /* value is a userdata? */
    إذا (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      إذا (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = عدم;  /* value is a userdata with wrong metatable */
      lua_چىقىرىش(L, 2);  /* remove both metatables */
      أرجع p;
    }
  }
  أرجع عدم;  /* value is not a userdata with a metatable */
}


LUALIB_API فراغ *luaL_checkudata (lua_State *L, صحيح ud, ثابت حرف *tname) {
  فراغ *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != عدم, ud, tname);
  أرجع p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API صحيح luaL_checkoption (lua_State *L, صحيح arg, ثابت حرف *def,
                                 ثابت حرف *ثابت lst[]) {
  ثابت حرف *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  صحيح i;
  لكل (i=0; lst[i]; i++)
    إذا (strcmp(lst[i], name) == 0)
      أرجع i;
  أرجع luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API فراغ luaL_checkstack (lua_State *L, صحيح space, ثابت حرف *ئۇچۇر) {
  إذا (l_unlikely(!lua_checkstack(L, space))) {
    إذا (ئۇچۇر)
      luaL_error(L, "stack overflow (%s)", ئۇچۇر);
    وإلا
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API فراغ luaL_checktype (lua_State *L, صحيح arg, صحيح t) {
  إذا (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API فراغ luaL_checkany (lua_State *L, صحيح arg) {
  إذا (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ثابت حرف *luaL_checklstring (lua_State *L, صحيح arg, size_t *len) {
  ثابت حرف *s = lua_tolstring(L, arg, len);
  إذا (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  أرجع s;
}


LUALIB_API ثابت حرف *luaL_optlstring (lua_State *L, صحيح arg,
                                        ثابت حرف *def, size_t *len) {
  إذا (lua_isnoneornil(L, arg)) {
    إذا (len)
      *len = (def ? strlen(def) : 0);
    أرجع def;
  }
  وإلا أرجع luaL_checklstring(L, arg, len);
}


LUALIB_API lua_سان luaL_checknumber (lua_State *L, صحيح arg) {
  صحيح isnum;
  lua_سان d = lua_tonumberx(L, arg, &isnum);
  إذا (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  أرجع d;
}


LUALIB_API lua_سان luaL_optnumber (lua_State *L, صحيح arg, lua_سان def) {
  أرجع luaL_opt(L, luaL_checknumber, arg, def);
}


ساكن فراغ interror (lua_State *L, صحيح arg) {
  إذا (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  وإلا
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, صحيح arg) {
  صحيح isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  إذا (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  أرجع d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, صحيح arg,
                                                      lua_Integer def) {
  أرجع luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
تعريفنوع بنية UBox {
  فراغ *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
ساكن فراغ *resizebox (lua_State *L, صحيح idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  إذا (box->bsize == newsize)  /* not changing size? */
    أرجع box->box;  /* keep the buffer */
  وإلا {
    فراغ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    فراغ *temp = allocf(ud, box->box, box->bsize, newsize);
    إذا (l_unlikely(temp == عدم && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    أرجع temp;
  }
}


ساكن صحيح boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  أرجع 0;
}


ساكن ثابت luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {عدم, عدم}
};


/*
** Get/create metatable (MT) for boxes
*/
ساكن فراغ getBoxMT (lua_State *L) {
  ثابت حرف *BOXMT = "_UBOX*"; /* key for the metatable */
  إذا (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


ساكن فراغ newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, حجم(UBox), 0);
  box->box = عدم;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != عدم  \
                            : lua_touserdata(B->L, idx) == (فراغ*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
ساكن size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  إذا (l_unlikely(sz >= MAX_SIZE - B->n))
    أرجع cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  إذا (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  إذا (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  أرجع newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
ساكن حرف *prepbuffsize (luaL_Buffer *B, size_t sz, صحيح boxidx) {
  checkbufferlevel(B, boxidx);
  إذا (B->size - B->n >= sz)  /* enough space? */
    أرجع B->b + B->n;
  وإلا {
    lua_State *L = B->L;
    حرف *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    إذا (buffonstack(B))  /* buffer already has a box? */
      newbuff = (حرف *)resizebox(L, boxidx, newsize);  /* resize it */
    وإلا {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_قىستۇرۇش(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (حرف *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * حجم(حرف));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    أرجع newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API حرف *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  أرجع prepbuffsize(B, sz, -1);
}


LUALIB_API فراغ luaL_addlstring (luaL_Buffer *B, ثابت حرف *s, size_t l) {
  إذا (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    حرف *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * حجم(حرف));
    luaL_addsize(B, l);
  }
}


LUALIB_API فراغ luaL_addstring (luaL_Buffer *B, ثابت حرف *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API فراغ luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  إذا (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  وإلا {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    فراغ *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    حرف *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (حرف*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = عدم;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API فراغ luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API فراغ luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  ثابت حرف *s = lua_tolstring(L, -1, &len);
  حرف *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * حجم(حرف));
  luaL_addsize(B, len);
  lua_چىقىرىش(L, 1);  /* pop string */
}


LUALIB_API فراغ luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (فراغ*)B);  /* push placeholder */
}


LUALIB_API حرف *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  أرجع prepbuffsize(B, sz, -1);
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
LUALIB_API صحيح luaL_ref (lua_State *L, صحيح t) {
  صحيح ref;
  إذا (lua_isnil(L, -1)) {
    lua_چىقىرىش(L, 1);  /* remove from stack */
    أرجع LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  إذا (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (صحيح)lua_tointeger(L, -1);  /* ref = t[1] */
  وإلا {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_چىقىرىش(L, 1);  /* remove element from stack */
  إذا (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  وإلا  /* no free elements */
    ref = (صحيح)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  أرجع ref;
}


LUALIB_API فراغ luaL_unref (lua_State *L, صحيح t, صحيح ref) {
  إذا (ref >= 0) {
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

تعريفنوع بنية LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  حرف buff[BUFSIZ];  /* area for reading file */
} LoadF;


ساكن ثابت حرف *getF (lua_State *L, فراغ *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  إذا (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  وإلا {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    إذا (feof(lf->f)) أرجع عدم;
    *size = fread(lf->buff, 1, حجم(lf->buff), lf->f);  /* read block */
  }
  أرجع lf->buff;
}


ساكن صحيح errfile (lua_State *L, ثابت حرف *what, صحيح fnameindex) {
  صحيح err = errno;
  ثابت حرف *filename = lua_tostring(L, fnameindex) + 1;
  إذا (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  وإلا
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  أرجع LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
ساكن صحيح skipBOM (FILE *f) {
  صحيح c = getc(f);  /* read first character */
  إذا (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    أرجع getc(f);  /* ignore BOM and return next char */
  وإلا  /* no (valid) BOM */
    أرجع c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
ساكن صحيح skipcomment (FILE *f, صحيح *cp) {
  صحيح c = *cp = skipBOM(f);
  إذا (c == '#') {  /* first line is a comment (Unix exec. file)? */
    افعل {  /* skip first line */
      c = getc(f);
    } طالما (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    أرجع 1;  /* there was a comment */
  }
  وإلا أرجع 0;  /* no comment */
}


LUALIB_API صحيح luaL_loadfilex (lua_State *L, ثابت حرف *filename,
                                             ثابت حرف *mode) {
  LoadF lf;
  صحيح status, readstatus;
  صحيح c;
  صحيح fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  إذا (filename == عدم) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  وإلا {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    إذا (lf.f == عدم) أرجع errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  إذا (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  إذا (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    إذا (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      إذا (lf.f == عدم) أرجع errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  إذا (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  إذا (filename) fclose(lf.f);  /* close file (even in case of errors) */
  إذا (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    أرجع errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  أرجع status;
}


تعريفنوع بنية LoadS {
  ثابت حرف *s;
  size_t size;
} LoadS;


ساكن ثابت حرف *getS (lua_State *L, فراغ *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  إذا (ls->size == 0) أرجع عدم;
  *size = ls->size;
  ls->size = 0;
  أرجع ls->s;
}


LUALIB_API صحيح luaL_loadbufferx (lua_State *L, ثابت حرف *buff, size_t size,
                                 ثابت حرف *name, ثابت حرف *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  أرجع lua_load(L, getS, &ls, name, mode);
}


LUALIB_API صحيح luaL_loadstring (lua_State *L, ثابت حرف *s) {
  أرجع luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API صحيح luaL_getmetafield (lua_State *L, صحيح obj, ثابت حرف *event) {
  إذا (!lua_getmetatable(L, obj))  /* no metatable? */
    أرجع LUA_TNIL;
  وإلا {
    صحيح tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    إذا (tt == LUA_TNIL)  /* is metafield nil? */
      lua_چىقىرىش(L, 2);  /* remove metatable and metafield */
    وإلا
      lua_remove(L, -2);  /* remove only metatable */
    أرجع tt;  /* return metafield type */
  }
}


LUALIB_API صحيح luaL_callmeta (lua_State *L, صحيح obj, ثابت حرف *event) {
  obj = lua_absindex(L, obj);
  إذا (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    أرجع 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  أرجع 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, صحيح idx) {
  lua_Integer l;
  صحيح isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  إذا (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_چىقىرىش(L, 1);  /* remove object */
  أرجع l;
}


LUALIB_API ثابت حرف *luaL_tolstring (lua_State *L, صحيح idx, size_t *len) {
  idx = lua_absindex(L,idx);
  إذا (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    إذا (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  وإلا {
    بدل (lua_type(L, idx)) {
      حالة LUA_TNUMBER: {
        حرف buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        اكسر;
      }
      حالة LUA_TSTRING:
        lua_pushvalue(L, idx);
        اكسر;
      حالة LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        اكسر;
      حالة LUA_TNIL:
        lua_pushliteral(L, "nil");
        اكسر;
      افتراضي: {
        صحيح tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ثابت حرف *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        إذا (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        اكسر;
      }
    }
  }
  أرجع lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API فراغ luaL_setfuncs (lua_State *L, ثابت luaL_Reg *l, صحيح nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  لكل (; l->name != عدم; l++) {  /* fill the table with given functions */
    إذا (l->func == عدم)  /* placeholder? */
      lua_pushboolean(L, 0);
    وإلا {
      صحيح i;
      لكل (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_چىقىرىش(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API صحيح luaL_getsubtable (lua_State *L, صحيح idx, ثابت حرف *fname) {
  إذا (lua_getfield(L, idx, fname) == LUA_TTABLE)
    أرجع 1;  /* table already there */
  وإلا {
    lua_چىقىرىش(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    أرجع 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API فراغ luaL_requiref (lua_State *L, ثابت حرف *modname,
                               lua_CFunction openf, صحيح glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  إذا (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_چىقىرىش(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  إذا (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API فراغ luaL_addgsub (luaL_Buffer *b, ثابت حرف *s,
                                     ثابت حرف *p, ثابت حرف *r) {
  ثابت حرف *wild;
  size_t l = strlen(p);
  طالما ((wild = strstr(s, p)) != عدم) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ثابت حرف *luaL_gsub (lua_State *L, ثابت حرف *s,
                                  ثابت حرف *p, ثابت حرف *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  أرجع lua_tostring(L, -1);
}


فراغ *luaL_alloc (فراغ *ud, فراغ *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  إذا (nsize == 0) {
    حرر(ptr);
    أرجع عدم;
  }
  وإلا
    أرجع realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
ساكن صحيح panic (lua_State *L) {
  ثابت حرف *ئۇچۇر = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        ئۇچۇر);
  أرجع 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
ساكن فراغ warnfoff (فراغ *ud, ثابت حرف *message, صحيح tocont);
ساكن فراغ warnfon (فراغ *ud, ثابت حرف *message, صحيح tocont);
ساكن فراغ warnfcont (فراغ *ud, ثابت حرف *message, صحيح tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
ساكن صحيح checkcontrol (lua_State *L, ثابت حرف *message, صحيح tocont) {
  إذا (tocont || *(message++) != '@')  /* not a control message? */
    أرجع 0;
  وإلا {
    إذا (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    وإلا إذا (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    أرجع 1;  /* it was a control message */
  }
}


ساكن فراغ warnfoff (فراغ *ud, ثابت حرف *message, صحيح tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
ساكن فراغ warnfcont (فراغ *ud, ثابت حرف *message, صحيح tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  إذا (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  وإلا {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


ساكن فراغ warnfon (فراغ *ud, ثابت حرف *message, صحيح tocont) {
  إذا (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    أرجع;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#إذا !defined(luai_makeseed)

#ضمن <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(حجم(فراغ*) + حجم(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + حجم(صحيح) - 1) / حجم(صحيح))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), حجم(v)), b += حجم(v))


ساكن unsigned صحيح luai_makeseed (فراغ) {
  unsigned صحيح buff[BUFSEED];
  unsigned صحيح res;
  unsigned صحيح i;
  time_t t = time(عدم);
  حرف *b = (حرف*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, حجم(buff) - BUFSEEDB);
  res = buff[0];
  لكل (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  أرجع res;
}

#endif


LUALIB_API unsigned صحيح luaL_makeseed (lua_State *L) {
  UNUSED(L);
  أرجع luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (فراغ) {
  lua_State *L = lua_newstate(luaL_alloc, عدم, luaL_makeseed(عدم));
  إذا (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  أرجع L;
}


LUALIB_API فراغ luaL_checkversion_ (lua_State *L, lua_سان ver, size_t sz) {
  lua_سان v = lua_version(L);
  إذا (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  وإلا إذا (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

