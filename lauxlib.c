/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#include "lprefix.h"


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#include "lua.h"

#include "lauxlib.h"
#include "llimits.h"


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
static ចំនួនគត់ findfield (lua_State *L, ចំនួនគត់ objidx, ចំនួនគត់ level) {
  ប្រសិនបើ (level == 0 || !lua_istable(L, -1))
    ត្រឡប់ 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  ខណៈពេល (lua_next(L, -2)) {  /* for each pair in table */
    ប្រសិនបើ (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      ប្រសិនបើ (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_pop(L, 1);  /* remove value (but keep name) */
        ត្រឡប់ 1;
      }
      បើមិនដូច្នេះ ប្រសិនបើ (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        ត្រឡប់ 1;
      }
    }
    lua_pop(L, 1);  /* remove value */
  }
  ត្រឡប់ 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
static ចំនួនគត់ pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  ចំនួនគត់ top = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  ប្រសិនបើ (findfield(L, top + 1, 2)) {
    const var_na *name = lua_tostring(L, -1);
    ប្រសិនបើ (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, top + 1);  /* copy name to proper place */
    lua_settop(L, top + 1);  /* remove table "loaded" and name copy */
    ត្រឡប់ 1;
  }
  បើមិនដូច្នេះ {
    lua_settop(L, top);  /* remove function and global table */
    ត្រឡប់ 0;
  }
}


static void pushfuncname (lua_State *L, lua_Debug *ar) {
  ប្រសិនបើ (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  បើមិនដូច្នេះ ប្រសិនបើ (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  បើមិនដូច្នេះ ប្រសិនបើ (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  បើមិនដូច្នេះ ប្រសិនបើ (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  បើមិនដូច្នេះ  /* nothing left... */
    lua_pushliteral(L, "?");
}


static ចំនួនគត់ lastlevel (lua_State *L) {
  lua_Debug ar;
  ចំនួនគត់ li = 1, le = 1;
  /* find an upper bound */
  ខណៈពេល (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  ខណៈពេល (li < le) {
    ចំនួនគត់ m = (li + le)/2;
    ប្រសិនបើ (lua_getstack(L, m, &ar)) li = m + 1;
    បើមិនដូច្នេះ le = m;
  }
  ត្រឡប់ le - 1;
}


LUALIB_API void luaL_traceback (lua_State *L, lua_State *L1,
                                const var_na *msg, ចំនួនគត់ level) {
  luaL_Buffer b;
  lua_Debug ar;
  ចំនួនគត់ last = lastlevel(L1);
  ចំនួនគត់ limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  ប្រសិនបើ (msg) {
    luaL_addstring(&b, msg);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  ខណៈពេល (lua_getstack(L1, level++, &ar)) {
    ប្រសិនបើ (limit2show-- == 0) {  /* too many levels? */
      ចំនួនគត់ n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    បើមិនដូច្នេះ {
      lua_getinfo(L1, "Slnt", &ar);
      ប្រសិនបើ (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      បើមិនដូច្នេះ
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      ប្រសិនបើ (ar.istailcall)
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

LUALIB_API ចំនួនគត់ luaL_argerror (lua_State *L, ចំនួនគត់ arg, const var_na *extramsg) {
  lua_Debug ar;
  const var_na *argword;
  ប្រសិនបើ (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ត្រឡប់ luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  ប្រសិនបើ (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  បើមិនដូច្នេះ {
    arg -= ar.extraargs;  /* do not count extra arguments */
    ប្រសិនបើ (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      ប្រសិនបើ (arg == 0)  /* error in self argument? */
        ត្រឡប់ luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  ប្រសិនបើ (ar.name == NULL)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  ត្រឡប់ luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API ចំនួនគត់ luaL_typeerror (lua_State *L, ចំនួនគត់ arg, const var_na *tname) {
  const var_na *msg;
  const var_na *typearg;  /* name for the type of the actual argument */
  ប្រសិនបើ (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  បើមិនដូច្នេះ ប្រសិនបើ (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  បើមិនដូច្នេះ
    typearg = luaL_typename(L, arg);  /* standard name */
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  ត្រឡប់ luaL_argerror(L, arg, msg);
}


static void tag_error (lua_State *L, ចំនួនគត់ arg, ចំនួនគត់ tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API void luaL_where (lua_State *L, ចំនួនគត់ level) {
  lua_Debug ar;
  ប្រសិនបើ (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    ប្រសិនបើ (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      ត្រឡប់;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API ចំនួនគត់ luaL_error (lua_State *L, const var_na *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  ត្រឡប់ lua_error(L);
}


LUALIB_API ចំនួនគត់ luaL_fileresult (lua_State *L, ចំនួនគត់ stat, const var_na *fname) {
  ចំនួនគត់ en = errno;  /* calls to Lua API may change this value */
  ប្រសិនបើ (stat) {
    lua_pushboolean(L, 1);
    ត្រឡប់ 1;
  }
  បើមិនដូច្នេះ {
    const var_na *msg;
    luaL_pushfail(L);
    msg = (en != 0) ? strerror(en) : "(no extra info)";
    ប្រសិនបើ (fname)
      lua_pushfstring(L, "%s: %s", fname, msg);
    បើមិនដូច្នេះ
      lua_pushstring(L, msg);
    lua_pushinteger(L, en);
    ត្រឡប់ 3;
  }
}


#ប្រសិនបើ !defined(l_inspectstat)	/* { */

#ប្រសិនបើ defined(LUA_USE_POSIX)

#include <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   ប្រសិនបើ (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   បើមិនដូច្នេះ ប្រសិនបើ (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#បើមិនដូច្នេះ

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API ចំនួនគត់ luaL_execresult (lua_State *L, ចំនួនគត់ stat) {
  ប្រសិនបើ (stat != 0 && errno != 0)  /* error with an 'errno'? */
    ត្រឡប់ luaL_fileresult(L, 0, NULL);
  បើមិនដូច្នេះ {
    const var_na *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    ប្រសិនបើ (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    បើមិនដូច្នេះ
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    ត្រឡប់ 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API ចំនួនគត់ luaL_newmetatable (lua_State *L, const var_na *tname) {
  ប្រសិនបើ (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    ត្រឡប់ 0;  /* leave previous value on top, but return 0 */
  lua_pop(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  ត្រឡប់ 1;
}


LUALIB_API void luaL_setmetatable (lua_State *L, const var_na *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API void *luaL_testudata (lua_State *L, ចំនួនគត់ ud, const var_na *tname) {
  void *p = lua_touserdata(L, ud);
  ប្រសិនបើ (p != NULL) {  /* value is a userdata? */
    ប្រសិនបើ (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      ប្រសិនបើ (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      lua_pop(L, 2);  /* remove both metatables */
      ត្រឡប់ p;
    }
  }
  ត្រឡប់ NULL;  /* value is not a userdata with a metatable */
}


LUALIB_API void *luaL_checkudata (lua_State *L, ចំនួនគត់ ud, const var_na *tname) {
  void *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != NULL, ud, tname);
  ត្រឡប់ p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API ចំនួនគត់ luaL_checkoption (lua_State *L, ចំនួនគត់ arg, const var_na *def,
                                 const var_na *const lst[]) {
  const var_na *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  ចំនួនគត់ i;
  សម្រាប់ (i=0; lst[i]; i++)
    ប្រសិនបើ (strcmp(lst[i], name) == 0)
      ត្រឡប់ i;
  ត្រឡប់ luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API void luaL_checkstack (lua_State *L, ចំនួនគត់ space, const var_na *msg) {
  ប្រសិនបើ (l_unlikely(!lua_checkstack(L, space))) {
    ប្រសិនបើ (msg)
      luaL_error(L, "stack overflow (%s)", msg);
    បើមិនដូច្នេះ
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API void luaL_checktype (lua_State *L, ចំនួនគត់ arg, ចំនួនគត់ t) {
  ប្រសិនបើ (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API void luaL_checkany (lua_State *L, ចំនួនគត់ arg) {
  ប្រសិនបើ (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API const var_na *luaL_checklstring (lua_State *L, ចំនួនគត់ arg, size_t *len) {
  const var_na *s = lua_tolstring(L, arg, len);
  ប្រសិនបើ (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  ត្រឡប់ s;
}


LUALIB_API const var_na *luaL_optlstring (lua_State *L, ចំនួនគត់ arg,
                                        const var_na *def, size_t *len) {
  ប្រសិនបើ (lua_isnoneornil(L, arg)) {
    ប្រសិនបើ (len)
      *len = (def ? strlen(def) : 0);
    ត្រឡប់ def;
  }
  បើមិនដូច្នេះ ត្រឡប់ luaL_checklstring(L, arg, len);
}


LUALIB_API lua_Number luaL_checknumber (lua_State *L, ចំនួនគត់ arg) {
  ចំនួនគត់ isnum;
  lua_Number d = lua_tonumberx(L, arg, &isnum);
  ប្រសិនបើ (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  ត្រឡប់ d;
}


LUALIB_API lua_Number luaL_optnumber (lua_State *L, ចំនួនគត់ arg, lua_Number def) {
  ត្រឡប់ luaL_opt(L, luaL_checknumber, arg, def);
}


static void interror (lua_State *L, ចំនួនគត់ arg) {
  ប្រសិនបើ (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  បើមិនដូច្នេះ
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, ចំនួនគត់ arg) {
  ចំនួនគត់ isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  ប្រសិនបើ (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  ត្រឡប់ d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, ចំនួនគត់ arg,
                                                      lua_Integer def) {
  ត្រឡប់ luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
typedef sa_nracanaa UBox {
  void *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
static void *resizebox (lua_State *L, ចំនួនគត់ idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  ប្រសិនបើ (box->bsize == newsize)  /* not changing size? */
    ត្រឡប់ box->box;  /* keep the buffer */
  បើមិនដូច្នេះ {
    void *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    void *temp = allocf(ud, box->box, box->bsize, newsize);
    ប្រសិនបើ (l_unlikely(temp == NULL && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    ត្រឡប់ temp;
  }
}


static ចំនួនគត់ boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  ត្រឡប់ 0;
}


static const luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {NULL, NULL}
};


/*
** Get/create metatable (MT) for boxes
*/
static void getBoxMT (lua_State *L) {
  const var_na *BOXMT = "_UBOX*"; /* key for the metatable */
  ប្រសិនបើ (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


static void newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, parimaa_na(UBox), 0);
  box->box = NULL;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != NULL  \
                            : lua_touserdata(B->L, idx) == (void*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
static size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  ប្រសិនបើ (l_unlikely(sz >= MAX_SIZE - B->n))
    ត្រឡប់ cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  ប្រសិនបើ (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  ប្រសិនបើ (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  ត្រឡប់ newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
static var_na *prepbuffsize (luaL_Buffer *B, size_t sz, ចំនួនគត់ boxidx) {
  checkbufferlevel(B, boxidx);
  ប្រសិនបើ (B->size - B->n >= sz)  /* enough space? */
    ត្រឡប់ B->b + B->n;
  បើមិនដូច្នេះ {
    lua_State *L = B->L;
    var_na *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    ប្រសិនបើ (buffonstack(B))  /* buffer already has a box? */
      newbuff = (var_na *)resizebox(L, boxidx, newsize);  /* resize it */
    បើមិនដូច្នេះ {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_insert(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (var_na *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * parimaa_na(var_na));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    ត្រឡប់ newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API var_na *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  ត្រឡប់ prepbuffsize(B, sz, -1);
}


LUALIB_API void luaL_addlstring (luaL_Buffer *B, const var_na *s, size_t l) {
  ប្រសិនបើ (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    var_na *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * parimaa_na(var_na));
    luaL_addsize(B, l);
  }
}


LUALIB_API void luaL_addstring (luaL_Buffer *B, const var_na *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API void luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  ប្រសិនបើ (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  បើមិនដូច្នេះ {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    void *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    var_na *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (var_na*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = NULL;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API void luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API void luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  const var_na *s = lua_tolstring(L, -1, &len);
  var_na *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * parimaa_na(var_na));
  luaL_addsize(B, len);
  lua_pop(L, 1);  /* pop string */
}


LUALIB_API void luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (void*)B);  /* push placeholder */
}


LUALIB_API var_na *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  ត្រឡប់ prepbuffsize(B, sz, -1);
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
LUALIB_API ចំនួនគត់ luaL_ref (lua_State *L, ចំនួនគត់ t) {
  ចំនួនគត់ ref;
  ប្រសិនបើ (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* remove from stack */
    ត្រឡប់ LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  ប្រសិនបើ (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (ចំនួនគត់)lua_tointeger(L, -1);  /* ref = t[1] */
  បើមិនដូច្នេះ {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_pop(L, 1);  /* remove element from stack */
  ប្រសិនបើ (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  បើមិនដូច្នេះ  /* no free elements */
    ref = (ចំនួនគត់)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  ត្រឡប់ ref;
}


LUALIB_API void luaL_unref (lua_State *L, ចំនួនគត់ t, ចំនួនគត់ ref) {
  ប្រសិនបើ (ref >= 0) {
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

typedef sa_nracanaa LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  var_na buff[BUFSIZ];  /* area for reading file */
} LoadF;


static const var_na *getF (lua_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  ប្រសិនបើ (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  បើមិនដូច្នេះ {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    ប្រសិនបើ (feof(lf->f)) ត្រឡប់ NULL;
    *size = fread(lf->buff, 1, parimaa_na(lf->buff), lf->f);  /* read block */
  }
  ត្រឡប់ lf->buff;
}


static ចំនួនគត់ errfile (lua_State *L, const var_na *what, ចំនួនគត់ fnameindex) {
  ចំនួនគត់ err = errno;
  const var_na *filename = lua_tostring(L, fnameindex) + 1;
  ប្រសិនបើ (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  បើមិនដូច្នេះ
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  ត្រឡប់ LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
static ចំនួនគត់ skipBOM (FILE *f) {
  ចំនួនគត់ c = getc(f);  /* read first character */
  ប្រសិនបើ (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    ត្រឡប់ getc(f);  /* ignore BOM and return next char */
  បើមិនដូច្នេះ  /* no (valid) BOM */
    ត្រឡប់ c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static ចំនួនគត់ skipcomment (FILE *f, ចំនួនគត់ *cp) {
  ចំនួនគត់ c = *cp = skipBOM(f);
  ប្រសិនបើ (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = getc(f);
    } ខណៈពេល (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    ត្រឡប់ 1;  /* there was a comment */
  }
  បើមិនដូច្នេះ ត្រឡប់ 0;  /* no comment */
}


LUALIB_API ចំនួនគត់ luaL_loadfilex (lua_State *L, const var_na *filename,
                                             const var_na *mode) {
  LoadF lf;
  ចំនួនគត់ status, readstatus;
  ចំនួនគត់ c;
  ចំនួនគត់ fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ប្រសិនបើ (filename == NULL) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  បើមិនដូច្នេះ {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    ប្រសិនបើ (lf.f == NULL) ត្រឡប់ errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  ប្រសិនបើ (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  ប្រសិនបើ (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    ប្រសិនបើ (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      ប្រសិនបើ (lf.f == NULL) ត្រឡប់ errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  ប្រសិនបើ (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  ប្រសិនបើ (filename) fclose(lf.f);  /* close file (even in case of errors) */
  ប្រសិនបើ (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    ត្រឡប់ errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  ត្រឡប់ status;
}


typedef sa_nracanaa LoadS {
  const var_na *s;
  size_t size;
} LoadS;


static const var_na *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  ប្រសិនបើ (ls->size == 0) ត្រឡប់ NULL;
  *size = ls->size;
  ls->size = 0;
  ត្រឡប់ ls->s;
}


LUALIB_API ចំនួនគត់ luaL_loadbufferx (lua_State *L, const var_na *buff, size_t size,
                                 const var_na *name, const var_na *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  ត្រឡប់ lua_load(L, getS, &ls, name, mode);
}


LUALIB_API ចំនួនគត់ luaL_loadstring (lua_State *L, const var_na *s) {
  ត្រឡប់ luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API ចំនួនគត់ luaL_getmetafield (lua_State *L, ចំនួនគត់ obj, const var_na *event) {
  ប្រសិនបើ (!lua_getmetatable(L, obj))  /* no metatable? */
    ត្រឡប់ LUA_TNIL;
  បើមិនដូច្នេះ {
    ចំនួនគត់ tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    ប្រសិនបើ (tt == LUA_TNIL)  /* is metafield nil? */
      lua_pop(L, 2);  /* remove metatable and metafield */
    បើមិនដូច្នេះ
      lua_remove(L, -2);  /* remove only metatable */
    ត្រឡប់ tt;  /* return metafield type */
  }
}


LUALIB_API ចំនួនគត់ luaL_callmeta (lua_State *L, ចំនួនគត់ obj, const var_na *event) {
  obj = lua_absindex(L, obj);
  ប្រសិនបើ (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    ត្រឡប់ 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  ត្រឡប់ 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, ចំនួនគត់ idx) {
  lua_Integer l;
  ចំនួនគត់ isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  ប្រសិនបើ (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_pop(L, 1);  /* remove object */
  ត្រឡប់ l;
}


LUALIB_API const var_na *luaL_tolstring (lua_State *L, ចំនួនគត់ idx, size_t *len) {
  idx = lua_absindex(L,idx);
  ប្រសិនបើ (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    ប្រសិនបើ (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  បើមិនដូច្នេះ {
    switch (lua_type(L, idx)) {
      pak_sa LUA_TNUMBER: {
        var_na buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        bha_nga;
      }
      pak_sa LUA_TSTRING:
        lua_pushvalue(L, idx);
        bha_nga;
      pak_sa LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        bha_nga;
      pak_sa LUA_TNIL:
        lua_pushliteral(L, "nil");
        bha_nga;
      default: {
        ចំនួនគត់ tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        const var_na *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        ប្រសិនបើ (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        bha_nga;
      }
    }
  }
  ត្រឡប់ lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, ចំនួនគត់ nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  សម្រាប់ (; l->name != NULL; l++) {  /* fill the table with given functions */
    ប្រសិនបើ (l->func == NULL)  /* placeholder? */
      lua_pushboolean(L, 0);
    បើមិនដូច្នេះ {
      ចំនួនគត់ i;
      សម្រាប់ (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API ចំនួនគត់ luaL_getsubtable (lua_State *L, ចំនួនគត់ idx, const var_na *fname) {
  ប្រសិនបើ (lua_getfield(L, idx, fname) == LUA_TTABLE)
    ត្រឡប់ 1;  /* table already there */
  បើមិនដូច្នេះ {
    lua_pop(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    ត្រឡប់ 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API void luaL_requiref (lua_State *L, const var_na *modname,
                               lua_CFunction openf, ចំនួនគត់ glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  ប្រសិនបើ (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_pop(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  ប្រសិនបើ (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API void luaL_addgsub (luaL_Buffer *b, const var_na *s,
                                     const var_na *p, const var_na *r) {
  const var_na *wild;
  size_t l = strlen(p);
  ខណៈពេល ((wild = strstr(s, p)) != NULL) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API const var_na *luaL_gsub (lua_State *L, const var_na *s,
                                  const var_na *p, const var_na *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  ត្រឡប់ lua_tostring(L, -1);
}


void *luaL_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  ប្រសិនបើ (nsize == 0) {
    free(ptr);
    ត្រឡប់ NULL;
  }
  បើមិនដូច្នេះ
    ត្រឡប់ realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
static ចំនួនគត់ panic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        msg);
  ត្រឡប់ 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
static void warnfoff (void *ud, const var_na *message, ចំនួនគត់ tocont);
static void warnfon (void *ud, const var_na *message, ចំនួនគត់ tocont);
static void warnfcont (void *ud, const var_na *message, ចំនួនគត់ tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
static ចំនួនគត់ checkcontrol (lua_State *L, const var_na *message, ចំនួនគត់ tocont) {
  ប្រសិនបើ (tocont || *(message++) != '@')  /* not a control message? */
    ត្រឡប់ 0;
  បើមិនដូច្នេះ {
    ប្រសិនបើ (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    បើមិនដូច្នេះ ប្រសិនបើ (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    ត្រឡប់ 1;  /* it was a control message */
  }
}


static void warnfoff (void *ud, const var_na *message, ចំនួនគត់ tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
static void warnfcont (void *ud, const var_na *message, ចំនួនគត់ tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  ប្រសិនបើ (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  បើមិនដូច្នេះ {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


static void warnfon (void *ud, const var_na *message, ចំនួនគត់ tocont) {
  ប្រសិនបើ (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    ត្រឡប់;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#ប្រសិនបើ !defined(luai_makeseed)

#include <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(parimaa_na(void*) + parimaa_na(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + parimaa_na(ចំនួនគត់) - 1) / parimaa_na(ចំនួនគត់))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), parimaa_na(v)), b += parimaa_na(v))


static unsigned ចំនួនគត់ luai_makeseed (void) {
  unsigned ចំនួនគត់ buff[BUFSEED];
  unsigned ចំនួនគត់ res;
  unsigned ចំនួនគត់ i;
  time_t t = time(NULL);
  var_na *b = (var_na*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, parimaa_na(buff) - BUFSEEDB);
  res = buff[0];
  សម្រាប់ (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  ត្រឡប់ res;
}

#endif


LUALIB_API unsigned ចំនួនគត់ luaL_makeseed (lua_State *L) {
  UNUSED(L);
  ត្រឡប់ luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (void) {
  lua_State *L = lua_newstate(luaL_alloc, NULL, luaL_makeseed(NULL));
  ប្រសិនបើ (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  ត្រឡប់ L;
}


LUALIB_API void luaL_checkversion_ (lua_State *L, lua_Number ver, size_t sz) {
  lua_Number v = lua_version(L);
  ប្រសិនបើ (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  បើមិនដូច្នេះ ប្រសិនបើ (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

