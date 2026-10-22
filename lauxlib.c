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
static ཧྲིལ findfield (lua_State *L, ཧྲིལ objidx, ཧྲིལ level) {
  གལ་སྲིད (level == 0 || !lua_istable(L, -1))
    ལོག 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  སྐབས (lua_next(L, -2)) {  /* for each pair in table */
    གལ་སྲིད (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      གལ་སྲིད (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_pop(L, 1);  /* remove value (but keep name) */
        ལོག 1;
      }
      དེ་མིན གལ་སྲིད (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        ལོག 1;
      }
    }
    lua_pop(L, 1);  /* remove value */
  }
  ལོག 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
static ཧྲིལ pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  ཧྲིལ top = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  གལ་སྲིད (findfield(L, top + 1, 2)) {
    const var_na *name = lua_tostring(L, -1);
    གལ་སྲིད (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, top + 1);  /* copy name to proper place */
    lua_settop(L, top + 1);  /* remove table "loaded" and name copy */
    ལོག 1;
  }
  དེ་མིན {
    lua_settop(L, top);  /* remove function and global table */
    ལོག 0;
  }
}


static void pushfuncname (lua_State *L, lua_Debug *ar) {
  གལ་སྲིད (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  དེ་མིན གལ་སྲིད (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  དེ་མིན གལ་སྲིད (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  དེ་མིན གལ་སྲིད (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  དེ་མིན  /* nothing left... */
    lua_pushliteral(L, "?");
}


static ཧྲིལ lastlevel (lua_State *L) {
  lua_Debug ar;
  ཧྲིལ li = 1, le = 1;
  /* find an upper bound */
  སྐབས (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  སྐབས (li < le) {
    ཧྲིལ m = (li + le)/2;
    གལ་སྲིད (lua_getstack(L, m, &ar)) li = m + 1;
    དེ་མིན le = m;
  }
  ལོག le - 1;
}


LUALIB_API void luaL_traceback (lua_State *L, lua_State *L1,
                                const var_na *msg, ཧྲིལ level) {
  luaL_Buffer b;
  lua_Debug ar;
  ཧྲིལ last = lastlevel(L1);
  ཧྲིལ limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  གལ་སྲིད (msg) {
    luaL_addstring(&b, msg);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  སྐབས (lua_getstack(L1, level++, &ar)) {
    གལ་སྲིད (limit2show-- == 0) {  /* too many levels? */
      ཧྲིལ n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    དེ་མིན {
      lua_getinfo(L1, "Slnt", &ar);
      གལ་སྲིད (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      དེ་མིན
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      གལ་སྲིད (ar.istailcall)
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

LUALIB_API ཧྲིལ luaL_argerror (lua_State *L, ཧྲིལ arg, const var_na *extramsg) {
  lua_Debug ar;
  const var_na *argword;
  གལ་སྲིད (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ལོག luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  གལ་སྲིད (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  དེ་མིན {
    arg -= ar.extraargs;  /* do not count extra arguments */
    གལ་སྲིད (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      གལ་སྲིད (arg == 0)  /* error in self argument? */
        ལོག luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  གལ་སྲིད (ar.name == NULL)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  ལོག luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API ཧྲིལ luaL_typeerror (lua_State *L, ཧྲིལ arg, const var_na *tname) {
  const var_na *msg;
  const var_na *typearg;  /* name for the type of the actual argument */
  གལ་སྲིད (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  དེ་མིན གལ་སྲིད (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  དེ་མིན
    typearg = luaL_typename(L, arg);  /* standard name */
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  ལོག luaL_argerror(L, arg, msg);
}


static void tag_error (lua_State *L, ཧྲིལ arg, ཧྲིལ tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API void luaL_where (lua_State *L, ཧྲིལ level) {
  lua_Debug ar;
  གལ་སྲིད (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    གལ་སྲིད (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      ལོག;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API ཧྲིལ luaL_error (lua_State *L, const var_na *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  ལོག lua_error(L);
}


LUALIB_API ཧྲིལ luaL_fileresult (lua_State *L, ཧྲིལ stat, const var_na *fname) {
  ཧྲིལ en = errno;  /* calls to Lua API may change this value */
  གལ་སྲིད (stat) {
    lua_pushboolean(L, 1);
    ལོག 1;
  }
  དེ་མིན {
    const var_na *msg;
    luaL_pushfail(L);
    msg = (en != 0) ? strerror(en) : "(no extra info)";
    གལ་སྲིད (fname)
      lua_pushfstring(L, "%s: %s", fname, msg);
    དེ་མིན
      lua_pushstring(L, msg);
    lua_pushinteger(L, en);
    ལོག 3;
  }
}


#གལ་སྲིད !defined(l_inspectstat)	/* { */

#གལ་སྲིད defined(LUA_USE_POSIX)

#include <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   གལ་སྲིད (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   དེ་མིན གལ་སྲིད (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#དེ་མིན

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API ཧྲིལ luaL_execresult (lua_State *L, ཧྲིལ stat) {
  གལ་སྲིད (stat != 0 && errno != 0)  /* error with an 'errno'? */
    ལོག luaL_fileresult(L, 0, NULL);
  དེ་མིན {
    const var_na *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    གལ་སྲིད (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    དེ་མིན
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    ལོག 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API ཧྲིལ luaL_newmetatable (lua_State *L, const var_na *tname) {
  གལ་སྲིད (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    ལོག 0;  /* leave previous value on top, but return 0 */
  lua_pop(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  ལོག 1;
}


LUALIB_API void luaL_setmetatable (lua_State *L, const var_na *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API void *luaL_testudata (lua_State *L, ཧྲིལ ud, const var_na *tname) {
  void *p = lua_touserdata(L, ud);
  གལ་སྲིད (p != NULL) {  /* value is a userdata? */
    གལ་སྲིད (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      གལ་སྲིད (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      lua_pop(L, 2);  /* remove both metatables */
      ལོག p;
    }
  }
  ལོག NULL;  /* value is not a userdata with a metatable */
}


LUALIB_API void *luaL_checkudata (lua_State *L, ཧྲིལ ud, const var_na *tname) {
  void *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != NULL, ud, tname);
  ལོག p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API ཧྲིལ luaL_checkoption (lua_State *L, ཧྲིལ arg, const var_na *def,
                                 const var_na *const lst[]) {
  const var_na *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  ཧྲིལ i;
  དོན་ལུ (i=0; lst[i]; i++)
    གལ་སྲིད (strcmp(lst[i], name) == 0)
      ལོག i;
  ལོག luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API void luaL_checkstack (lua_State *L, ཧྲིལ space, const var_na *msg) {
  གལ་སྲིད (l_unlikely(!lua_checkstack(L, space))) {
    གལ་སྲིད (msg)
      luaL_error(L, "stack overflow (%s)", msg);
    དེ་མིན
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API void luaL_checktype (lua_State *L, ཧྲིལ arg, ཧྲིལ t) {
  གལ་སྲིད (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API void luaL_checkany (lua_State *L, ཧྲིལ arg) {
  གལ་སྲིད (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API const var_na *luaL_checklstring (lua_State *L, ཧྲིལ arg, size_t *len) {
  const var_na *s = lua_tolstring(L, arg, len);
  གལ་སྲིད (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  ལོག s;
}


LUALIB_API const var_na *luaL_optlstring (lua_State *L, ཧྲིལ arg,
                                        const var_na *def, size_t *len) {
  གལ་སྲིད (lua_isnoneornil(L, arg)) {
    གལ་སྲིད (len)
      *len = (def ? strlen(def) : 0);
    ལོག def;
  }
  དེ་མིན ལོག luaL_checklstring(L, arg, len);
}


LUALIB_API lua_Number luaL_checknumber (lua_State *L, ཧྲིལ arg) {
  ཧྲིལ isnum;
  lua_Number d = lua_tonumberx(L, arg, &isnum);
  གལ་སྲིད (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  ལོག d;
}


LUALIB_API lua_Number luaL_optnumber (lua_State *L, ཧྲིལ arg, lua_Number def) {
  ལོག luaL_opt(L, luaL_checknumber, arg, def);
}


static void interror (lua_State *L, ཧྲིལ arg) {
  གལ་སྲིད (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  དེ་མིན
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, ཧྲིལ arg) {
  ཧྲིལ isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  གལ་སྲིད (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  ལོག d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, ཧྲིལ arg,
                                                      lua_Integer def) {
  ལོག luaL_opt(L, luaL_checkinteger, arg, def);
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
static void *resizebox (lua_State *L, ཧྲིལ idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  གལ་སྲིད (box->bsize == newsize)  /* not changing size? */
    ལོག box->box;  /* keep the buffer */
  དེ་མིན {
    void *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    void *temp = allocf(ud, box->box, box->bsize, newsize);
    གལ་སྲིད (l_unlikely(temp == NULL && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    ལོག temp;
  }
}


static ཧྲིལ boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  ལོག 0;
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
  གལ་སྲིད (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
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
  གལ་སྲིད (l_unlikely(sz >= MAX_SIZE - B->n))
    ལོག cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  གལ་སྲིད (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  གལ་སྲིད (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  ལོག newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
static var_na *prepbuffsize (luaL_Buffer *B, size_t sz, ཧྲིལ boxidx) {
  checkbufferlevel(B, boxidx);
  གལ་སྲིད (B->size - B->n >= sz)  /* enough space? */
    ལོག B->b + B->n;
  དེ་མིན {
    lua_State *L = B->L;
    var_na *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    གལ་སྲིད (buffonstack(B))  /* buffer already has a box? */
      newbuff = (var_na *)resizebox(L, boxidx, newsize);  /* resize it */
    དེ་མིན {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_insert(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (var_na *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * parimaa_na(var_na));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    ལོག newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API var_na *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  ལོག prepbuffsize(B, sz, -1);
}


LUALIB_API void luaL_addlstring (luaL_Buffer *B, const var_na *s, size_t l) {
  གལ་སྲིད (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
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
  གལ་སྲིད (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  དེ་མིན {  /* reuse buffer already allocated */
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
  ལོག prepbuffsize(B, sz, -1);
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
LUALIB_API ཧྲིལ luaL_ref (lua_State *L, ཧྲིལ t) {
  ཧྲིལ ref;
  གལ་སྲིད (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* remove from stack */
    ལོག LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  གལ་སྲིད (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (ཧྲིལ)lua_tointeger(L, -1);  /* ref = t[1] */
  དེ་མིན {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_pop(L, 1);  /* remove element from stack */
  གལ་སྲིད (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  དེ་མིན  /* no free elements */
    ref = (ཧྲིལ)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  ལོག ref;
}


LUALIB_API void luaL_unref (lua_State *L, ཧྲིལ t, ཧྲིལ ref) {
  གལ་སྲིད (ref >= 0) {
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
  གལ་སྲིད (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  དེ་མིན {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    གལ་སྲིད (feof(lf->f)) ལོག NULL;
    *size = fread(lf->buff, 1, parimaa_na(lf->buff), lf->f);  /* read block */
  }
  ལོག lf->buff;
}


static ཧྲིལ errfile (lua_State *L, const var_na *what, ཧྲིལ fnameindex) {
  ཧྲིལ err = errno;
  const var_na *filename = lua_tostring(L, fnameindex) + 1;
  གལ་སྲིད (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  དེ་མིན
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  ལོག LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
static ཧྲིལ skipBOM (FILE *f) {
  ཧྲིལ c = getc(f);  /* read first character */
  གལ་སྲིད (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    ལོག getc(f);  /* ignore BOM and return next char */
  དེ་མིན  /* no (valid) BOM */
    ལོག c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static ཧྲིལ skipcomment (FILE *f, ཧྲིལ *cp) {
  ཧྲིལ c = *cp = skipBOM(f);
  གལ་སྲིད (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = getc(f);
    } སྐབས (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    ལོག 1;  /* there was a comment */
  }
  དེ་མིན ལོག 0;  /* no comment */
}


LUALIB_API ཧྲིལ luaL_loadfilex (lua_State *L, const var_na *filename,
                                             const var_na *mode) {
  LoadF lf;
  ཧྲིལ status, readstatus;
  ཧྲིལ c;
  ཧྲིལ fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  གལ་སྲིད (filename == NULL) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  དེ་མིན {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    གལ་སྲིད (lf.f == NULL) ལོག errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  གལ་སྲིད (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  གལ་སྲིད (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    གལ་སྲིད (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      གལ་སྲིད (lf.f == NULL) ལོག errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  གལ་སྲིད (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  གལ་སྲིད (filename) fclose(lf.f);  /* close file (even in case of errors) */
  གལ་སྲིད (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    ལོག errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  ལོག status;
}


typedef sa_nracanaa LoadS {
  const var_na *s;
  size_t size;
} LoadS;


static const var_na *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  གལ་སྲིད (ls->size == 0) ལོག NULL;
  *size = ls->size;
  ls->size = 0;
  ལོག ls->s;
}


LUALIB_API ཧྲིལ luaL_loadbufferx (lua_State *L, const var_na *buff, size_t size,
                                 const var_na *name, const var_na *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  ལོག lua_load(L, getS, &ls, name, mode);
}


LUALIB_API ཧྲིལ luaL_loadstring (lua_State *L, const var_na *s) {
  ལོག luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API ཧྲིལ luaL_getmetafield (lua_State *L, ཧྲིལ obj, const var_na *event) {
  གལ་སྲིད (!lua_getmetatable(L, obj))  /* no metatable? */
    ལོག LUA_TNIL;
  དེ་མིན {
    ཧྲིལ tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    གལ་སྲིད (tt == LUA_TNIL)  /* is metafield nil? */
      lua_pop(L, 2);  /* remove metatable and metafield */
    དེ་མིན
      lua_remove(L, -2);  /* remove only metatable */
    ལོག tt;  /* return metafield type */
  }
}


LUALIB_API ཧྲིལ luaL_callmeta (lua_State *L, ཧྲིལ obj, const var_na *event) {
  obj = lua_absindex(L, obj);
  གལ་སྲིད (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    ལོག 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  ལོག 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, ཧྲིལ idx) {
  lua_Integer l;
  ཧྲིལ isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  གལ་སྲིད (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_pop(L, 1);  /* remove object */
  ལོག l;
}


LUALIB_API const var_na *luaL_tolstring (lua_State *L, ཧྲིལ idx, size_t *len) {
  idx = lua_absindex(L,idx);
  གལ་སྲིད (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    གལ་སྲིད (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  དེ་མིན {
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
        ཧྲིལ tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        const var_na *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        གལ་སྲིད (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        bha_nga;
      }
    }
  }
  ལོག lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, ཧྲིལ nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  དོན་ལུ (; l->name != NULL; l++) {  /* fill the table with given functions */
    གལ་སྲིད (l->func == NULL)  /* placeholder? */
      lua_pushboolean(L, 0);
    དེ་མིན {
      ཧྲིལ i;
      དོན་ལུ (i = 0; i < nup; i++)  /* copy upvalues to the top */
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
LUALIB_API ཧྲིལ luaL_getsubtable (lua_State *L, ཧྲིལ idx, const var_na *fname) {
  གལ་སྲིད (lua_getfield(L, idx, fname) == LUA_TTABLE)
    ལོག 1;  /* table already there */
  དེ་མིན {
    lua_pop(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    ལོག 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API void luaL_requiref (lua_State *L, const var_na *modname,
                               lua_CFunction openf, ཧྲིལ glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  གལ་སྲིད (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_pop(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  གལ་སྲིད (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API void luaL_addgsub (luaL_Buffer *b, const var_na *s,
                                     const var_na *p, const var_na *r) {
  const var_na *wild;
  size_t l = strlen(p);
  སྐབས ((wild = strstr(s, p)) != NULL) {
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
  ལོག lua_tostring(L, -1);
}


void *luaL_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  གལ་སྲིད (nsize == 0) {
    free(ptr);
    ལོག NULL;
  }
  དེ་མིན
    ལོག realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
static ཧྲིལ panic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        msg);
  ལོག 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
static void warnfoff (void *ud, const var_na *message, ཧྲིལ tocont);
static void warnfon (void *ud, const var_na *message, ཧྲིལ tocont);
static void warnfcont (void *ud, const var_na *message, ཧྲིལ tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
static ཧྲིལ checkcontrol (lua_State *L, const var_na *message, ཧྲིལ tocont) {
  གལ་སྲིད (tocont || *(message++) != '@')  /* not a control message? */
    ལོག 0;
  དེ་མིན {
    གལ་སྲིད (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    དེ་མིན གལ་སྲིད (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    ལོག 1;  /* it was a control message */
  }
}


static void warnfoff (void *ud, const var_na *message, ཧྲིལ tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
static void warnfcont (void *ud, const var_na *message, ཧྲིལ tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  གལ་སྲིད (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  དེ་མིན {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


static void warnfon (void *ud, const var_na *message, ཧྲིལ tocont) {
  གལ་སྲིད (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    ལོག;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#གལ་སྲིད !defined(luai_makeseed)

#include <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(parimaa_na(void*) + parimaa_na(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + parimaa_na(ཧྲིལ) - 1) / parimaa_na(ཧྲིལ))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), parimaa_na(v)), b += parimaa_na(v))


static unsigned ཧྲིལ luai_makeseed (void) {
  unsigned ཧྲིལ buff[BUFSEED];
  unsigned ཧྲིལ res;
  unsigned ཧྲིལ i;
  time_t t = time(NULL);
  var_na *b = (var_na*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, parimaa_na(buff) - BUFSEEDB);
  res = buff[0];
  དོན་ལུ (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  ལོག res;
}

#endif


LUALIB_API unsigned ཧྲིལ luaL_makeseed (lua_State *L) {
  UNUSED(L);
  ལོག luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (void) {
  lua_State *L = lua_newstate(luaL_alloc, NULL, luaL_makeseed(NULL));
  གལ་སྲིད (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  ལོག L;
}


LUALIB_API void luaL_checkversion_ (lua_State *L, lua_Number ver, size_t sz) {
  lua_Number v = lua_version(L);
  གལ་སྲིད (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  དེ་མིན གལ་སྲིད (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

