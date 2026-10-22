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
static จำนวนเต็ม findfield (lua_State *L, จำนวนเต็ม objidx, จำนวนเต็ม level) {
  ถ้า (level == 0 || !lua_istable(L, -1))
    คืน 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  ขณะ (lua_next(L, -2)) {  /* for each pair in table */
    ถ้า (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      ถ้า (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_pop(L, 1);  /* remove value (but keep name) */
        คืน 1;
      }
      มิฉะนั้น ถ้า (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        คืน 1;
      }
    }
    lua_pop(L, 1);  /* remove value */
  }
  คืน 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
static จำนวนเต็ม pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  จำนวนเต็ม top = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  ถ้า (findfield(L, top + 1, 2)) {
    const var_na *name = lua_tostring(L, -1);
    ถ้า (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, top + 1);  /* copy name to proper place */
    lua_settop(L, top + 1);  /* remove table "loaded" and name copy */
    คืน 1;
  }
  มิฉะนั้น {
    lua_settop(L, top);  /* remove function and global table */
    คืน 0;
  }
}


static void pushfuncname (lua_State *L, lua_Debug *ar) {
  ถ้า (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  มิฉะนั้น ถ้า (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  มิฉะนั้น ถ้า (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  มิฉะนั้น ถ้า (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  มิฉะนั้น  /* nothing left... */
    lua_pushliteral(L, "?");
}


static จำนวนเต็ม lastlevel (lua_State *L) {
  lua_Debug ar;
  จำนวนเต็ม li = 1, le = 1;
  /* find an upper bound */
  ขณะ (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  ขณะ (li < le) {
    จำนวนเต็ม m = (li + le)/2;
    ถ้า (lua_getstack(L, m, &ar)) li = m + 1;
    มิฉะนั้น le = m;
  }
  คืน le - 1;
}


LUALIB_API void luaL_traceback (lua_State *L, lua_State *L1,
                                const var_na *msg, จำนวนเต็ม level) {
  luaL_Buffer b;
  lua_Debug ar;
  จำนวนเต็ม last = lastlevel(L1);
  จำนวนเต็ม limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  ถ้า (msg) {
    luaL_addstring(&b, msg);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  ขณะ (lua_getstack(L1, level++, &ar)) {
    ถ้า (limit2show-- == 0) {  /* too many levels? */
      จำนวนเต็ม n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    มิฉะนั้น {
      lua_getinfo(L1, "Slnt", &ar);
      ถ้า (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      มิฉะนั้น
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      ถ้า (ar.istailcall)
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

LUALIB_API จำนวนเต็ม luaL_argerror (lua_State *L, จำนวนเต็ม arg, const var_na *extramsg) {
  lua_Debug ar;
  const var_na *argword;
  ถ้า (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    คืน luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  ถ้า (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  มิฉะนั้น {
    arg -= ar.extraargs;  /* do not count extra arguments */
    ถ้า (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      ถ้า (arg == 0)  /* error in self argument? */
        คืน luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  ถ้า (ar.name == NULL)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  คืน luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API จำนวนเต็ม luaL_typeerror (lua_State *L, จำนวนเต็ม arg, const var_na *tname) {
  const var_na *msg;
  const var_na *typearg;  /* name for the type of the actual argument */
  ถ้า (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  มิฉะนั้น ถ้า (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  มิฉะนั้น
    typearg = luaL_typename(L, arg);  /* standard name */
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  คืน luaL_argerror(L, arg, msg);
}


static void tag_error (lua_State *L, จำนวนเต็ม arg, จำนวนเต็ม tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API void luaL_where (lua_State *L, จำนวนเต็ม level) {
  lua_Debug ar;
  ถ้า (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    ถ้า (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      คืน;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API จำนวนเต็ม luaL_error (lua_State *L, const var_na *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  คืน lua_error(L);
}


LUALIB_API จำนวนเต็ม luaL_fileresult (lua_State *L, จำนวนเต็ม stat, const var_na *fname) {
  จำนวนเต็ม en = errno;  /* calls to Lua API may change this value */
  ถ้า (stat) {
    lua_pushboolean(L, 1);
    คืน 1;
  }
  มิฉะนั้น {
    const var_na *msg;
    luaL_pushfail(L);
    msg = (en != 0) ? strerror(en) : "(no extra info)";
    ถ้า (fname)
      lua_pushfstring(L, "%s: %s", fname, msg);
    มิฉะนั้น
      lua_pushstring(L, msg);
    lua_pushinteger(L, en);
    คืน 3;
  }
}


#ถ้า !defined(l_inspectstat)	/* { */

#ถ้า defined(LUA_USE_POSIX)

#include <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   ถ้า (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   มิฉะนั้น ถ้า (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#มิฉะนั้น

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API จำนวนเต็ม luaL_execresult (lua_State *L, จำนวนเต็ม stat) {
  ถ้า (stat != 0 && errno != 0)  /* error with an 'errno'? */
    คืน luaL_fileresult(L, 0, NULL);
  มิฉะนั้น {
    const var_na *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    ถ้า (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    มิฉะนั้น
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    คืน 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API จำนวนเต็ม luaL_newmetatable (lua_State *L, const var_na *tname) {
  ถ้า (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    คืน 0;  /* leave previous value on top, but return 0 */
  lua_pop(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  คืน 1;
}


LUALIB_API void luaL_setmetatable (lua_State *L, const var_na *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API void *luaL_testudata (lua_State *L, จำนวนเต็ม ud, const var_na *tname) {
  void *p = lua_touserdata(L, ud);
  ถ้า (p != NULL) {  /* value is a userdata? */
    ถ้า (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      ถ้า (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      lua_pop(L, 2);  /* remove both metatables */
      คืน p;
    }
  }
  คืน NULL;  /* value is not a userdata with a metatable */
}


LUALIB_API void *luaL_checkudata (lua_State *L, จำนวนเต็ม ud, const var_na *tname) {
  void *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != NULL, ud, tname);
  คืน p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API จำนวนเต็ม luaL_checkoption (lua_State *L, จำนวนเต็ม arg, const var_na *def,
                                 const var_na *const lst[]) {
  const var_na *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  จำนวนเต็ม i;
  สำหรับ (i=0; lst[i]; i++)
    ถ้า (strcmp(lst[i], name) == 0)
      คืน i;
  คืน luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API void luaL_checkstack (lua_State *L, จำนวนเต็ม space, const var_na *msg) {
  ถ้า (l_unlikely(!lua_checkstack(L, space))) {
    ถ้า (msg)
      luaL_error(L, "stack overflow (%s)", msg);
    มิฉะนั้น
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API void luaL_checktype (lua_State *L, จำนวนเต็ม arg, จำนวนเต็ม t) {
  ถ้า (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API void luaL_checkany (lua_State *L, จำนวนเต็ม arg) {
  ถ้า (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API const var_na *luaL_checklstring (lua_State *L, จำนวนเต็ม arg, size_t *len) {
  const var_na *s = lua_tolstring(L, arg, len);
  ถ้า (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  คืน s;
}


LUALIB_API const var_na *luaL_optlstring (lua_State *L, จำนวนเต็ม arg,
                                        const var_na *def, size_t *len) {
  ถ้า (lua_isnoneornil(L, arg)) {
    ถ้า (len)
      *len = (def ? strlen(def) : 0);
    คืน def;
  }
  มิฉะนั้น คืน luaL_checklstring(L, arg, len);
}


LUALIB_API lua_Number luaL_checknumber (lua_State *L, จำนวนเต็ม arg) {
  จำนวนเต็ม isnum;
  lua_Number d = lua_tonumberx(L, arg, &isnum);
  ถ้า (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  คืน d;
}


LUALIB_API lua_Number luaL_optnumber (lua_State *L, จำนวนเต็ม arg, lua_Number def) {
  คืน luaL_opt(L, luaL_checknumber, arg, def);
}


static void interror (lua_State *L, จำนวนเต็ม arg) {
  ถ้า (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  มิฉะนั้น
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, จำนวนเต็ม arg) {
  จำนวนเต็ม isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  ถ้า (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  คืน d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, จำนวนเต็ม arg,
                                                      lua_Integer def) {
  คืน luaL_opt(L, luaL_checkinteger, arg, def);
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
static void *resizebox (lua_State *L, จำนวนเต็ม idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  ถ้า (box->bsize == newsize)  /* not changing size? */
    คืน box->box;  /* keep the buffer */
  มิฉะนั้น {
    void *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    void *temp = allocf(ud, box->box, box->bsize, newsize);
    ถ้า (l_unlikely(temp == NULL && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    คืน temp;
  }
}


static จำนวนเต็ม boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  คืน 0;
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
  ถ้า (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
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
  ถ้า (l_unlikely(sz >= MAX_SIZE - B->n))
    คืน cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  ถ้า (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  ถ้า (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  คืน newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
static var_na *prepbuffsize (luaL_Buffer *B, size_t sz, จำนวนเต็ม boxidx) {
  checkbufferlevel(B, boxidx);
  ถ้า (B->size - B->n >= sz)  /* enough space? */
    คืน B->b + B->n;
  มิฉะนั้น {
    lua_State *L = B->L;
    var_na *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    ถ้า (buffonstack(B))  /* buffer already has a box? */
      newbuff = (var_na *)resizebox(L, boxidx, newsize);  /* resize it */
    มิฉะนั้น {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_insert(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (var_na *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * parimaa_na(var_na));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    คืน newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API var_na *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  คืน prepbuffsize(B, sz, -1);
}


LUALIB_API void luaL_addlstring (luaL_Buffer *B, const var_na *s, size_t l) {
  ถ้า (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
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
  ถ้า (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  มิฉะนั้น {  /* reuse buffer already allocated */
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
  คืน prepbuffsize(B, sz, -1);
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
LUALIB_API จำนวนเต็ม luaL_ref (lua_State *L, จำนวนเต็ม t) {
  จำนวนเต็ม ref;
  ถ้า (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* remove from stack */
    คืน LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  ถ้า (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (จำนวนเต็ม)lua_tointeger(L, -1);  /* ref = t[1] */
  มิฉะนั้น {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_pop(L, 1);  /* remove element from stack */
  ถ้า (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  มิฉะนั้น  /* no free elements */
    ref = (จำนวนเต็ม)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  คืน ref;
}


LUALIB_API void luaL_unref (lua_State *L, จำนวนเต็ม t, จำนวนเต็ม ref) {
  ถ้า (ref >= 0) {
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
  ถ้า (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  มิฉะนั้น {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    ถ้า (feof(lf->f)) คืน NULL;
    *size = fread(lf->buff, 1, parimaa_na(lf->buff), lf->f);  /* read block */
  }
  คืน lf->buff;
}


static จำนวนเต็ม errfile (lua_State *L, const var_na *what, จำนวนเต็ม fnameindex) {
  จำนวนเต็ม err = errno;
  const var_na *filename = lua_tostring(L, fnameindex) + 1;
  ถ้า (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  มิฉะนั้น
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  คืน LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
static จำนวนเต็ม skipBOM (FILE *f) {
  จำนวนเต็ม c = getc(f);  /* read first character */
  ถ้า (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    คืน getc(f);  /* ignore BOM and return next char */
  มิฉะนั้น  /* no (valid) BOM */
    คืน c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static จำนวนเต็ม skipcomment (FILE *f, จำนวนเต็ม *cp) {
  จำนวนเต็ม c = *cp = skipBOM(f);
  ถ้า (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = getc(f);
    } ขณะ (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    คืน 1;  /* there was a comment */
  }
  มิฉะนั้น คืน 0;  /* no comment */
}


LUALIB_API จำนวนเต็ม luaL_loadfilex (lua_State *L, const var_na *filename,
                                             const var_na *mode) {
  LoadF lf;
  จำนวนเต็ม status, readstatus;
  จำนวนเต็ม c;
  จำนวนเต็ม fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ถ้า (filename == NULL) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  มิฉะนั้น {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    ถ้า (lf.f == NULL) คืน errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  ถ้า (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  ถ้า (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    ถ้า (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      ถ้า (lf.f == NULL) คืน errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  ถ้า (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  ถ้า (filename) fclose(lf.f);  /* close file (even in case of errors) */
  ถ้า (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    คืน errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  คืน status;
}


typedef sa_nracanaa LoadS {
  const var_na *s;
  size_t size;
} LoadS;


static const var_na *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  ถ้า (ls->size == 0) คืน NULL;
  *size = ls->size;
  ls->size = 0;
  คืน ls->s;
}


LUALIB_API จำนวนเต็ม luaL_loadbufferx (lua_State *L, const var_na *buff, size_t size,
                                 const var_na *name, const var_na *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  คืน lua_load(L, getS, &ls, name, mode);
}


LUALIB_API จำนวนเต็ม luaL_loadstring (lua_State *L, const var_na *s) {
  คืน luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API จำนวนเต็ม luaL_getmetafield (lua_State *L, จำนวนเต็ม obj, const var_na *event) {
  ถ้า (!lua_getmetatable(L, obj))  /* no metatable? */
    คืน LUA_TNIL;
  มิฉะนั้น {
    จำนวนเต็ม tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    ถ้า (tt == LUA_TNIL)  /* is metafield nil? */
      lua_pop(L, 2);  /* remove metatable and metafield */
    มิฉะนั้น
      lua_remove(L, -2);  /* remove only metatable */
    คืน tt;  /* return metafield type */
  }
}


LUALIB_API จำนวนเต็ม luaL_callmeta (lua_State *L, จำนวนเต็ม obj, const var_na *event) {
  obj = lua_absindex(L, obj);
  ถ้า (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    คืน 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  คืน 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, จำนวนเต็ม idx) {
  lua_Integer l;
  จำนวนเต็ม isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  ถ้า (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_pop(L, 1);  /* remove object */
  คืน l;
}


LUALIB_API const var_na *luaL_tolstring (lua_State *L, จำนวนเต็ม idx, size_t *len) {
  idx = lua_absindex(L,idx);
  ถ้า (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    ถ้า (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  มิฉะนั้น {
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
        จำนวนเต็ม tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        const var_na *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        ถ้า (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        bha_nga;
      }
    }
  }
  คืน lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, จำนวนเต็ม nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  สำหรับ (; l->name != NULL; l++) {  /* fill the table with given functions */
    ถ้า (l->func == NULL)  /* placeholder? */
      lua_pushboolean(L, 0);
    มิฉะนั้น {
      จำนวนเต็ม i;
      สำหรับ (i = 0; i < nup; i++)  /* copy upvalues to the top */
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
LUALIB_API จำนวนเต็ม luaL_getsubtable (lua_State *L, จำนวนเต็ม idx, const var_na *fname) {
  ถ้า (lua_getfield(L, idx, fname) == LUA_TTABLE)
    คืน 1;  /* table already there */
  มิฉะนั้น {
    lua_pop(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    คืน 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API void luaL_requiref (lua_State *L, const var_na *modname,
                               lua_CFunction openf, จำนวนเต็ม glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  ถ้า (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_pop(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  ถ้า (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API void luaL_addgsub (luaL_Buffer *b, const var_na *s,
                                     const var_na *p, const var_na *r) {
  const var_na *wild;
  size_t l = strlen(p);
  ขณะ ((wild = strstr(s, p)) != NULL) {
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
  คืน lua_tostring(L, -1);
}


void *luaL_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  ถ้า (nsize == 0) {
    free(ptr);
    คืน NULL;
  }
  มิฉะนั้น
    คืน realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
static จำนวนเต็ม panic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        msg);
  คืน 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
static void warnfoff (void *ud, const var_na *message, จำนวนเต็ม tocont);
static void warnfon (void *ud, const var_na *message, จำนวนเต็ม tocont);
static void warnfcont (void *ud, const var_na *message, จำนวนเต็ม tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
static จำนวนเต็ม checkcontrol (lua_State *L, const var_na *message, จำนวนเต็ม tocont) {
  ถ้า (tocont || *(message++) != '@')  /* not a control message? */
    คืน 0;
  มิฉะนั้น {
    ถ้า (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    มิฉะนั้น ถ้า (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    คืน 1;  /* it was a control message */
  }
}


static void warnfoff (void *ud, const var_na *message, จำนวนเต็ม tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
static void warnfcont (void *ud, const var_na *message, จำนวนเต็ม tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  ถ้า (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  มิฉะนั้น {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


static void warnfon (void *ud, const var_na *message, จำนวนเต็ม tocont) {
  ถ้า (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    คืน;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#ถ้า !defined(luai_makeseed)

#include <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(parimaa_na(void*) + parimaa_na(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + parimaa_na(จำนวนเต็ม) - 1) / parimaa_na(จำนวนเต็ม))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), parimaa_na(v)), b += parimaa_na(v))


static unsigned จำนวนเต็ม luai_makeseed (void) {
  unsigned จำนวนเต็ม buff[BUFSEED];
  unsigned จำนวนเต็ม res;
  unsigned จำนวนเต็ม i;
  time_t t = time(NULL);
  var_na *b = (var_na*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, parimaa_na(buff) - BUFSEEDB);
  res = buff[0];
  สำหรับ (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  คืน res;
}

#endif


LUALIB_API unsigned จำนวนเต็ม luaL_makeseed (lua_State *L) {
  UNUSED(L);
  คืน luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (void) {
  lua_State *L = lua_newstate(luaL_alloc, NULL, luaL_makeseed(NULL));
  ถ้า (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  คืน L;
}


LUALIB_API void luaL_checkversion_ (lua_State *L, lua_Number ver, size_t sz) {
  lua_Number v = lua_version(L);
  ถ้า (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  มิฉะนั้น ถ้า (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

