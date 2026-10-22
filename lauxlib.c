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
static số_nguyên findfield (lua_State *L, số_nguyên objidx, số_nguyên level) {
  nếu (level == 0 || !lua_istable(L, -1))
    trả_về 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  trong_khi (lua_next(L, -2)) {  /* for each pair in table */
    nếu (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      nếu (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_pop(L, 1);  /* remove value (but keep name) */
        trả_về 1;
      }
      không_thì nếu (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        trả_về 1;
      }
    }
    lua_pop(L, 1);  /* remove value */
  }
  trả_về 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
static số_nguyên pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  số_nguyên top = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  nếu (findfield(L, top + 1, 2)) {
    const char *name = lua_tostring(L, -1);
    nếu (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, top + 1);  /* copy name to proper place */
    lua_settop(L, top + 1);  /* remove table "loaded" and name copy */
    trả_về 1;
  }
  không_thì {
    lua_settop(L, top);  /* remove function and global table */
    trả_về 0;
  }
}


static void pushfuncname (lua_State *L, lua_Debug *ar) {
  nếu (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  không_thì nếu (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  không_thì nếu (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  không_thì nếu (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  không_thì  /* nothing left... */
    lua_pushliteral(L, "?");
}


static số_nguyên lastlevel (lua_State *L) {
  lua_Debug ar;
  số_nguyên li = 1, le = 1;
  /* find an upper bound */
  trong_khi (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  trong_khi (li < le) {
    số_nguyên m = (li + le)/2;
    nếu (lua_getstack(L, m, &ar)) li = m + 1;
    không_thì le = m;
  }
  trả_về le - 1;
}


LUALIB_API void luaL_traceback (lua_State *L, lua_State *L1,
                                const char *msg, số_nguyên level) {
  luaL_Buffer b;
  lua_Debug ar;
  số_nguyên last = lastlevel(L1);
  số_nguyên limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  nếu (msg) {
    luaL_addstring(&b, msg);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  trong_khi (lua_getstack(L1, level++, &ar)) {
    nếu (limit2show-- == 0) {  /* too many levels? */
      số_nguyên n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    không_thì {
      lua_getinfo(L1, "Slnt", &ar);
      nếu (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      không_thì
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      nếu (ar.istailcall)
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

LUALIB_API số_nguyên luaL_argerror (lua_State *L, số_nguyên arg, const char *extramsg) {
  lua_Debug ar;
  const char *argword;
  nếu (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    trả_về luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  nếu (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  không_thì {
    arg -= ar.extraargs;  /* do not count extra arguments */
    nếu (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      nếu (arg == 0)  /* error in self argument? */
        trả_về luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  nếu (ar.name == NULL)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  trả_về luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API số_nguyên luaL_typeerror (lua_State *L, số_nguyên arg, const char *tname) {
  const char *msg;
  const char *typearg;  /* name for the type of the actual argument */
  nếu (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  không_thì nếu (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  không_thì
    typearg = luaL_typename(L, arg);  /* standard name */
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  trả_về luaL_argerror(L, arg, msg);
}


static void tag_error (lua_State *L, số_nguyên arg, số_nguyên tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API void luaL_where (lua_State *L, số_nguyên level) {
  lua_Debug ar;
  nếu (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    nếu (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      trả_về;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API số_nguyên luaL_error (lua_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  trả_về lua_error(L);
}


LUALIB_API số_nguyên luaL_fileresult (lua_State *L, số_nguyên stat, const char *fname) {
  số_nguyên en = errno;  /* calls to Lua API may change this value */
  nếu (stat) {
    lua_pushboolean(L, 1);
    trả_về 1;
  }
  không_thì {
    const char *msg;
    luaL_pushfail(L);
    msg = (en != 0) ? strerror(en) : "(no extra info)";
    nếu (fname)
      lua_pushfstring(L, "%s: %s", fname, msg);
    không_thì
      lua_pushstring(L, msg);
    lua_pushinteger(L, en);
    trả_về 3;
  }
}


#nếu !defined(l_inspectstat)	/* { */

#nếu defined(LUA_USE_POSIX)

#include <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   nếu (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   không_thì nếu (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#không_thì

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API số_nguyên luaL_execresult (lua_State *L, số_nguyên stat) {
  nếu (stat != 0 && errno != 0)  /* error with an 'errno'? */
    trả_về luaL_fileresult(L, 0, NULL);
  không_thì {
    const char *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    nếu (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    không_thì
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    trả_về 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API số_nguyên luaL_newmetatable (lua_State *L, const char *tname) {
  nếu (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    trả_về 0;  /* leave previous value on top, but return 0 */
  lua_pop(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  trả_về 1;
}


LUALIB_API void luaL_setmetatable (lua_State *L, const char *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API void *luaL_testudata (lua_State *L, số_nguyên ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  nếu (p != NULL) {  /* value is a userdata? */
    nếu (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      nếu (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      lua_pop(L, 2);  /* remove both metatables */
      trả_về p;
    }
  }
  trả_về NULL;  /* value is not a userdata with a metatable */
}


LUALIB_API void *luaL_checkudata (lua_State *L, số_nguyên ud, const char *tname) {
  void *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != NULL, ud, tname);
  trả_về p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API số_nguyên luaL_checkoption (lua_State *L, số_nguyên arg, const char *def,
                                 const char *const lst[]) {
  const char *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  số_nguyên i;
  cho (i=0; lst[i]; i++)
    nếu (strcmp(lst[i], name) == 0)
      trả_về i;
  trả_về luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API void luaL_checkstack (lua_State *L, số_nguyên space, const char *msg) {
  nếu (l_unlikely(!lua_checkstack(L, space))) {
    nếu (msg)
      luaL_error(L, "stack overflow (%s)", msg);
    không_thì
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API void luaL_checktype (lua_State *L, số_nguyên arg, số_nguyên t) {
  nếu (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API void luaL_checkany (lua_State *L, số_nguyên arg) {
  nếu (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API const char *luaL_checklstring (lua_State *L, số_nguyên arg, size_t *len) {
  const char *s = lua_tolstring(L, arg, len);
  nếu (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  trả_về s;
}


LUALIB_API const char *luaL_optlstring (lua_State *L, số_nguyên arg,
                                        const char *def, size_t *len) {
  nếu (lua_isnoneornil(L, arg)) {
    nếu (len)
      *len = (def ? strlen(def) : 0);
    trả_về def;
  }
  không_thì trả_về luaL_checklstring(L, arg, len);
}


LUALIB_API lua_Number luaL_checknumber (lua_State *L, số_nguyên arg) {
  số_nguyên isnum;
  lua_Number d = lua_tonumberx(L, arg, &isnum);
  nếu (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  trả_về d;
}


LUALIB_API lua_Number luaL_optnumber (lua_State *L, số_nguyên arg, lua_Number def) {
  trả_về luaL_opt(L, luaL_checknumber, arg, def);
}


static void interror (lua_State *L, số_nguyên arg) {
  nếu (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  không_thì
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, số_nguyên arg) {
  số_nguyên isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  nếu (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  trả_về d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, số_nguyên arg,
                                                      lua_Integer def) {
  trả_về luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
typedef struct UBox {
  void *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
static void *resizebox (lua_State *L, số_nguyên idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  nếu (box->bsize == newsize)  /* not changing size? */
    trả_về box->box;  /* keep the buffer */
  không_thì {
    void *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    void *temp = allocf(ud, box->box, box->bsize, newsize);
    nếu (l_unlikely(temp == NULL && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    trả_về temp;
  }
}


static số_nguyên boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  trả_về 0;
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
  const char *BOXMT = "_UBOX*"; /* key for the metatable */
  nếu (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


static void newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, sizeof(UBox), 0);
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
  nếu (l_unlikely(sz >= MAX_SIZE - B->n))
    trả_về cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  nếu (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  nếu (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  trả_về newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
static char *prepbuffsize (luaL_Buffer *B, size_t sz, số_nguyên boxidx) {
  checkbufferlevel(B, boxidx);
  nếu (B->size - B->n >= sz)  /* enough space? */
    trả_về B->b + B->n;
  không_thì {
    lua_State *L = B->L;
    char *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    nếu (buffonstack(B))  /* buffer already has a box? */
      newbuff = (char *)resizebox(L, boxidx, newsize);  /* resize it */
    không_thì {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_insert(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (char *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * sizeof(char));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    trả_về newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API char *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  trả_về prepbuffsize(B, sz, -1);
}


LUALIB_API void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l) {
  nếu (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    char *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * sizeof(char));
    luaL_addsize(B, l);
  }
}


LUALIB_API void luaL_addstring (luaL_Buffer *B, const char *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API void luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  nếu (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  không_thì {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    void *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    char *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (char*)box->box;  /* final buffer address */
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
  const char *s = lua_tolstring(L, -1, &len);
  char *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * sizeof(char));
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


LUALIB_API char *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  trả_về prepbuffsize(B, sz, -1);
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
LUALIB_API số_nguyên luaL_ref (lua_State *L, số_nguyên t) {
  số_nguyên ref;
  nếu (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* remove from stack */
    trả_về LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  nếu (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (số_nguyên)lua_tointeger(L, -1);  /* ref = t[1] */
  không_thì {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_pop(L, 1);  /* remove element from stack */
  nếu (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  không_thì  /* no free elements */
    ref = (số_nguyên)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  trả_về ref;
}


LUALIB_API void luaL_unref (lua_State *L, số_nguyên t, số_nguyên ref) {
  nếu (ref >= 0) {
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

typedef struct LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  char buff[BUFSIZ];  /* area for reading file */
} LoadF;


static const char *getF (lua_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  nếu (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  không_thì {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    nếu (feof(lf->f)) trả_về NULL;
    *size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);  /* read block */
  }
  trả_về lf->buff;
}


static số_nguyên errfile (lua_State *L, const char *what, số_nguyên fnameindex) {
  số_nguyên err = errno;
  const char *filename = lua_tostring(L, fnameindex) + 1;
  nếu (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  không_thì
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  trả_về LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
static số_nguyên skipBOM (FILE *f) {
  số_nguyên c = getc(f);  /* read first character */
  nếu (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    trả_về getc(f);  /* ignore BOM and return next char */
  không_thì  /* no (valid) BOM */
    trả_về c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static số_nguyên skipcomment (FILE *f, số_nguyên *cp) {
  số_nguyên c = *cp = skipBOM(f);
  nếu (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = getc(f);
    } trong_khi (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    trả_về 1;  /* there was a comment */
  }
  không_thì trả_về 0;  /* no comment */
}


LUALIB_API số_nguyên luaL_loadfilex (lua_State *L, const char *filename,
                                             const char *mode) {
  LoadF lf;
  số_nguyên status, readstatus;
  số_nguyên c;
  số_nguyên fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  nếu (filename == NULL) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  không_thì {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    nếu (lf.f == NULL) trả_về errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  nếu (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  nếu (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    nếu (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      nếu (lf.f == NULL) trả_về errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  nếu (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  nếu (filename) fclose(lf.f);  /* close file (even in case of errors) */
  nếu (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    trả_về errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  trả_về status;
}


typedef struct LoadS {
  const char *s;
  size_t size;
} LoadS;


static const char *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  nếu (ls->size == 0) trả_về NULL;
  *size = ls->size;
  ls->size = 0;
  trả_về ls->s;
}


LUALIB_API số_nguyên luaL_loadbufferx (lua_State *L, const char *buff, size_t size,
                                 const char *name, const char *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  trả_về lua_load(L, getS, &ls, name, mode);
}


LUALIB_API số_nguyên luaL_loadstring (lua_State *L, const char *s) {
  trả_về luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API số_nguyên luaL_getmetafield (lua_State *L, số_nguyên obj, const char *event) {
  nếu (!lua_getmetatable(L, obj))  /* no metatable? */
    trả_về LUA_TNIL;
  không_thì {
    số_nguyên tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    nếu (tt == LUA_TNIL)  /* is metafield nil? */
      lua_pop(L, 2);  /* remove metatable and metafield */
    không_thì
      lua_remove(L, -2);  /* remove only metatable */
    trả_về tt;  /* return metafield type */
  }
}


LUALIB_API số_nguyên luaL_callmeta (lua_State *L, số_nguyên obj, const char *event) {
  obj = lua_absindex(L, obj);
  nếu (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    trả_về 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  trả_về 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, số_nguyên idx) {
  lua_Integer l;
  số_nguyên isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  nếu (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_pop(L, 1);  /* remove object */
  trả_về l;
}


LUALIB_API const char *luaL_tolstring (lua_State *L, số_nguyên idx, size_t *len) {
  idx = lua_absindex(L,idx);
  nếu (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    nếu (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  không_thì {
    switch (lua_type(L, idx)) {
      case LUA_TNUMBER: {
        char buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        break;
      }
      case LUA_TSTRING:
        lua_pushvalue(L, idx);
        break;
      case LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        break;
      case LUA_TNIL:
        lua_pushliteral(L, "nil");
        break;
      default: {
        số_nguyên tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        const char *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        nếu (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        break;
      }
    }
  }
  trả_về lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, số_nguyên nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  cho (; l->name != NULL; l++) {  /* fill the table with given functions */
    nếu (l->func == NULL)  /* placeholder? */
      lua_pushboolean(L, 0);
    không_thì {
      số_nguyên i;
      cho (i = 0; i < nup; i++)  /* copy upvalues to the top */
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
LUALIB_API số_nguyên luaL_getsubtable (lua_State *L, số_nguyên idx, const char *fname) {
  nếu (lua_getfield(L, idx, fname) == LUA_TTABLE)
    trả_về 1;  /* table already there */
  không_thì {
    lua_pop(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    trả_về 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API void luaL_requiref (lua_State *L, const char *modname,
                               lua_CFunction openf, số_nguyên glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  nếu (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_pop(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  nếu (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API void luaL_addgsub (luaL_Buffer *b, const char *s,
                                     const char *p, const char *r) {
  const char *wild;
  size_t l = strlen(p);
  trong_khi ((wild = strstr(s, p)) != NULL) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API const char *luaL_gsub (lua_State *L, const char *s,
                                  const char *p, const char *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  trả_về lua_tostring(L, -1);
}


void *luaL_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  nếu (nsize == 0) {
    free(ptr);
    trả_về NULL;
  }
  không_thì
    trả_về realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
static số_nguyên panic (lua_State *L) {
  const char *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        msg);
  trả_về 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
static void warnfoff (void *ud, const char *message, số_nguyên tocont);
static void warnfon (void *ud, const char *message, số_nguyên tocont);
static void warnfcont (void *ud, const char *message, số_nguyên tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
static số_nguyên checkcontrol (lua_State *L, const char *message, số_nguyên tocont) {
  nếu (tocont || *(message++) != '@')  /* not a control message? */
    trả_về 0;
  không_thì {
    nếu (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    không_thì nếu (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    trả_về 1;  /* it was a control message */
  }
}


static void warnfoff (void *ud, const char *message, số_nguyên tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
static void warnfcont (void *ud, const char *message, số_nguyên tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  nếu (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  không_thì {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


static void warnfon (void *ud, const char *message, số_nguyên tocont) {
  nếu (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    trả_về;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#nếu !defined(luai_makeseed)

#include <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(sizeof(void*) + sizeof(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + sizeof(số_nguyên) - 1) / sizeof(số_nguyên))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), sizeof(v)), b += sizeof(v))


static unsigned số_nguyên luai_makeseed (void) {
  unsigned số_nguyên buff[BUFSEED];
  unsigned số_nguyên res;
  unsigned số_nguyên i;
  time_t t = time(NULL);
  char *b = (char*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, sizeof(buff) - BUFSEEDB);
  res = buff[0];
  cho (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  trả_về res;
}

#endif


LUALIB_API unsigned số_nguyên luaL_makeseed (lua_State *L) {
  UNUSED(L);
  trả_về luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (void) {
  lua_State *L = lua_newstate(luaL_alloc, NULL, luaL_makeseed(NULL));
  nếu (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  trả_về L;
}


LUALIB_API void luaL_checkversion_ (lua_State *L, lua_Number ver, size_t sz) {
  lua_Number v = lua_version(L);
  nếu (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  không_thì nếu (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

