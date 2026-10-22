/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#شامل "lprefix.h"


#شامل <errno.h>
#شامل <stdarg.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "llimits.h"


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
ایستا صحیح findfield (lua_State *L, صحیح objidx, صحیح level) {
  اگر (level == 0 || !lua_istable(L, -1))
    بازگشت 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  وقتی (lua_next(L, -2)) {  /* for each pair in table */
    اگر (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      اگر (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_برداشت(L, 1);  /* remove value (but keep name) */
        بازگشت 1;
      }
      وگرنه اگر (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        بازگشت 1;
      }
    }
    lua_برداشت(L, 1);  /* remove value */
  }
  بازگشت 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
ایستا صحیح pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  صحیح بالاترین = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  اگر (findfield(L, بالاترین + 1, 2)) {
    ثابت نویسه *name = lua_tostring(L, -1);
    اگر (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, بالاترین + 1);  /* copy name to proper place */
    lua_settop(L, بالاترین + 1);  /* remove table "loaded" and name copy */
    بازگشت 1;
  }
  وگرنه {
    lua_settop(L, بالاترین);  /* remove function and global table */
    بازگشت 0;
  }
}


ایستا تهی pushfuncname (lua_State *L, lua_Debug *ar) {
  اگر (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  وگرنه اگر (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  وگرنه اگر (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  وگرنه اگر (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  وگرنه  /* nothing left... */
    lua_pushliteral(L, "?");
}


ایستا صحیح lastlevel (lua_State *L) {
  lua_Debug ar;
  صحیح li = 1, le = 1;
  /* find an upper bound */
  وقتی (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  وقتی (li < le) {
    صحیح m = (li + le)/2;
    اگر (lua_getstack(L, m, &ar)) li = m + 1;
    وگرنه le = m;
  }
  بازگشت le - 1;
}


LUALIB_API تهی luaL_traceback (lua_State *L, lua_State *L1,
                                ثابت نویسه *پیام, صحیح level) {
  luaL_Buffer b;
  lua_Debug ar;
  صحیح last = lastlevel(L1);
  صحیح limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  اگر (پیام) {
    luaL_addstring(&b, پیام);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  وقتی (lua_getstack(L1, level++, &ar)) {
    اگر (limit2show-- == 0) {  /* too many levels? */
      صحیح n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    وگرنه {
      lua_getinfo(L1, "Slnt", &ar);
      اگر (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      وگرنه
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      اگر (ar.istailcall)
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

LUALIB_API صحیح luaL_argerror (lua_State *L, صحیح arg, ثابت نویسه *extramsg) {
  lua_Debug ar;
  ثابت نویسه *argword;
  اگر (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    بازگشت luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  اگر (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  وگرنه {
    arg -= ar.extraargs;  /* do not count extra arguments */
    اگر (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      اگر (arg == 0)  /* error in self argument? */
        بازگشت luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  اگر (ar.name == پوچ)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  بازگشت luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API صحیح luaL_typeerror (lua_State *L, صحیح arg, ثابت نویسه *tname) {
  ثابت نویسه *پیام;
  ثابت نویسه *typearg;  /* name for the type of the actual argument */
  اگر (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  وگرنه اگر (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  وگرنه
    typearg = luaL_typename(L, arg);  /* standard name */
  پیام = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  بازگشت luaL_argerror(L, arg, پیام);
}


ایستا تهی tag_error (lua_State *L, صحیح arg, صحیح tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API تهی luaL_where (lua_State *L, صحیح level) {
  lua_Debug ar;
  اگر (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    اگر (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      بازگشت;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API صحیح luaL_error (lua_State *L, ثابت نویسه *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  بازگشت lua_error(L);
}


LUALIB_API صحیح luaL_fileresult (lua_State *L, صحیح stat, ثابت نویسه *fname) {
  صحیح en = errno;  /* calls to Lua API may change this value */
  اگر (stat) {
    lua_pushboolean(L, 1);
    بازگشت 1;
  }
  وگرنه {
    ثابت نویسه *پیام;
    luaL_pushfail(L);
    پیام = (en != 0) ? strerror(en) : "(no extra info)";
    اگر (fname)
      lua_pushfstring(L, "%s: %s", fname, پیام);
    وگرنه
      lua_pushstring(L, پیام);
    lua_pushinteger(L, en);
    بازگشت 3;
  }
}


#اگر !defined(l_inspectstat)	/* { */

#اگر defined(LUA_USE_POSIX)

#شامل <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   اگر (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   وگرنه اگر (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#وگرنه

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API صحیح luaL_execresult (lua_State *L, صحیح stat) {
  اگر (stat != 0 && errno != 0)  /* error with an 'errno'? */
    بازگشت luaL_fileresult(L, 0, پوچ);
  وگرنه {
    ثابت نویسه *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    اگر (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    وگرنه
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    بازگشت 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API صحیح luaL_newmetatable (lua_State *L, ثابت نویسه *tname) {
  اگر (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    بازگشت 0;  /* leave previous value on top, but return 0 */
  lua_برداشت(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  بازگشت 1;
}


LUALIB_API تهی luaL_setmetatable (lua_State *L, ثابت نویسه *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API تهی *luaL_testudata (lua_State *L, صحیح ud, ثابت نویسه *tname) {
  تهی *p = lua_touserdata(L, ud);
  اگر (p != پوچ) {  /* value is a userdata? */
    اگر (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      اگر (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = پوچ;  /* value is a userdata with wrong metatable */
      lua_برداشت(L, 2);  /* remove both metatables */
      بازگشت p;
    }
  }
  بازگشت پوچ;  /* value is not a userdata with a metatable */
}


LUALIB_API تهی *luaL_checkudata (lua_State *L, صحیح ud, ثابت نویسه *tname) {
  تهی *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != پوچ, ud, tname);
  بازگشت p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API صحیح luaL_checkoption (lua_State *L, صحیح arg, ثابت نویسه *def,
                                 ثابت نویسه *ثابت lst[]) {
  ثابت نویسه *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  صحیح i;
  برای (i=0; lst[i]; i++)
    اگر (strcmp(lst[i], name) == 0)
      بازگشت i;
  بازگشت luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API تهی luaL_checkstack (lua_State *L, صحیح space, ثابت نویسه *پیام) {
  اگر (l_unlikely(!lua_checkstack(L, space))) {
    اگر (پیام)
      luaL_error(L, "stack overflow (%s)", پیام);
    وگرنه
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API تهی luaL_checktype (lua_State *L, صحیح arg, صحیح t) {
  اگر (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API تهی luaL_checkany (lua_State *L, صحیح arg) {
  اگر (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ثابت نویسه *luaL_checklstring (lua_State *L, صحیح arg, size_t *len) {
  ثابت نویسه *s = lua_tolstring(L, arg, len);
  اگر (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  بازگشت s;
}


LUALIB_API ثابت نویسه *luaL_optlstring (lua_State *L, صحیح arg,
                                        ثابت نویسه *def, size_t *len) {
  اگر (lua_isnoneornil(L, arg)) {
    اگر (len)
      *len = (def ? strlen(def) : 0);
    بازگشت def;
  }
  وگرنه بازگشت luaL_checklstring(L, arg, len);
}


LUALIB_API lua_عدد luaL_checknumber (lua_State *L, صحیح arg) {
  صحیح isnum;
  lua_عدد d = lua_tonumberx(L, arg, &isnum);
  اگر (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  بازگشت d;
}


LUALIB_API lua_عدد luaL_optnumber (lua_State *L, صحیح arg, lua_عدد def) {
  بازگشت luaL_opt(L, luaL_checknumber, arg, def);
}


ایستا تهی interror (lua_State *L, صحیح arg) {
  اگر (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  وگرنه
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, صحیح arg) {
  صحیح isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  اگر (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  بازگشت d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, صحیح arg,
                                                      lua_Integer def) {
  بازگشت luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
تعریفنوع ساختار UBox {
  تهی *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
ایستا تهی *resizebox (lua_State *L, صحیح idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  اگر (box->bsize == newsize)  /* not changing size? */
    بازگشت box->box;  /* keep the buffer */
  وگرنه {
    تهی *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    تهی *temp = allocf(ud, box->box, box->bsize, newsize);
    اگر (l_unlikely(temp == پوچ && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    بازگشت temp;
  }
}


ایستا صحیح boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  بازگشت 0;
}


ایستا ثابت luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {پوچ, پوچ}
};


/*
** Get/create metatable (MT) for boxes
*/
ایستا تهی getBoxMT (lua_State *L) {
  ثابت نویسه *BOXMT = "_UBOX*"; /* key for the metatable */
  اگر (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


ایستا تهی newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, اندازه(UBox), 0);
  box->box = پوچ;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != پوچ  \
                            : lua_touserdata(B->L, idx) == (تهی*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
ایستا size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  اگر (l_unlikely(sz >= MAX_SIZE - B->n))
    بازگشت cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  اگر (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  اگر (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  بازگشت newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
ایستا نویسه *prepbuffsize (luaL_Buffer *B, size_t sz, صحیح boxidx) {
  checkbufferlevel(B, boxidx);
  اگر (B->size - B->n >= sz)  /* enough space? */
    بازگشت B->b + B->n;
  وگرنه {
    lua_State *L = B->L;
    نویسه *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    اگر (buffonstack(B))  /* buffer already has a box? */
      newbuff = (نویسه *)resizebox(L, boxidx, newsize);  /* resize it */
    وگرنه {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_درج(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (نویسه *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * اندازه(نویسه));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    بازگشت newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API نویسه *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  بازگشت prepbuffsize(B, sz, -1);
}


LUALIB_API تهی luaL_addlstring (luaL_Buffer *B, ثابت نویسه *s, size_t l) {
  اگر (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    نویسه *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * اندازه(نویسه));
    luaL_addsize(B, l);
  }
}


LUALIB_API تهی luaL_addstring (luaL_Buffer *B, ثابت نویسه *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API تهی luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  اگر (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  وگرنه {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    تهی *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    نویسه *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (نویسه*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = پوچ;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API تهی luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API تهی luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  ثابت نویسه *s = lua_tolstring(L, -1, &len);
  نویسه *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * اندازه(نویسه));
  luaL_addsize(B, len);
  lua_برداشت(L, 1);  /* pop string */
}


LUALIB_API تهی luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (تهی*)B);  /* push placeholder */
}


LUALIB_API نویسه *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  بازگشت prepbuffsize(B, sz, -1);
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
LUALIB_API صحیح luaL_ref (lua_State *L, صحیح t) {
  صحیح ref;
  اگر (lua_isnil(L, -1)) {
    lua_برداشت(L, 1);  /* remove from stack */
    بازگشت LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  اگر (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (صحیح)lua_tointeger(L, -1);  /* ref = t[1] */
  وگرنه {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_برداشت(L, 1);  /* remove element from stack */
  اگر (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  وگرنه  /* no free elements */
    ref = (صحیح)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  بازگشت ref;
}


LUALIB_API تهی luaL_unref (lua_State *L, صحیح t, صحیح ref) {
  اگر (ref >= 0) {
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

تعریفنوع ساختار LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  نویسه buff[BUFSIZ];  /* area for reading file */
} LoadF;


ایستا ثابت نویسه *getF (lua_State *L, تهی *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  اگر (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  وگرنه {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    اگر (feof(lf->f)) بازگشت پوچ;
    *size = fread(lf->buff, 1, اندازه(lf->buff), lf->f);  /* read block */
  }
  بازگشت lf->buff;
}


ایستا صحیح errfile (lua_State *L, ثابت نویسه *what, صحیح fnameindex) {
  صحیح err = errno;
  ثابت نویسه *filename = lua_tostring(L, fnameindex) + 1;
  اگر (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  وگرنه
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  بازگشت LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
ایستا صحیح skipBOM (FILE *f) {
  صحیح c = getc(f);  /* read first character */
  اگر (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    بازگشت getc(f);  /* ignore BOM and return next char */
  وگرنه  /* no (valid) BOM */
    بازگشت c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
ایستا صحیح skipcomment (FILE *f, صحیح *cp) {
  صحیح c = *cp = skipBOM(f);
  اگر (c == '#') {  /* first line is a comment (Unix exec. file)? */
    انجامبده {  /* skip first line */
      c = getc(f);
    } وقتی (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    بازگشت 1;  /* there was a comment */
  }
  وگرنه بازگشت 0;  /* no comment */
}


LUALIB_API صحیح luaL_loadfilex (lua_State *L, ثابت نویسه *filename,
                                             ثابت نویسه *mode) {
  LoadF lf;
  صحیح status, readstatus;
  صحیح c;
  صحیح fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  اگر (filename == پوچ) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  وگرنه {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    اگر (lf.f == پوچ) بازگشت errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  اگر (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  اگر (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    اگر (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      اگر (lf.f == پوچ) بازگشت errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  اگر (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  اگر (filename) fclose(lf.f);  /* close file (even in case of errors) */
  اگر (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    بازگشت errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  بازگشت status;
}


تعریفنوع ساختار LoadS {
  ثابت نویسه *s;
  size_t size;
} LoadS;


ایستا ثابت نویسه *getS (lua_State *L, تهی *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  اگر (ls->size == 0) بازگشت پوچ;
  *size = ls->size;
  ls->size = 0;
  بازگشت ls->s;
}


LUALIB_API صحیح luaL_loadbufferx (lua_State *L, ثابت نویسه *buff, size_t size,
                                 ثابت نویسه *name, ثابت نویسه *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  بازگشت lua_load(L, getS, &ls, name, mode);
}


LUALIB_API صحیح luaL_loadstring (lua_State *L, ثابت نویسه *s) {
  بازگشت luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API صحیح luaL_getmetafield (lua_State *L, صحیح obj, ثابت نویسه *event) {
  اگر (!lua_getmetatable(L, obj))  /* no metatable? */
    بازگشت LUA_TNIL;
  وگرنه {
    صحیح tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    اگر (tt == LUA_TNIL)  /* is metafield nil? */
      lua_برداشت(L, 2);  /* remove metatable and metafield */
    وگرنه
      lua_remove(L, -2);  /* remove only metatable */
    بازگشت tt;  /* return metafield type */
  }
}


LUALIB_API صحیح luaL_callmeta (lua_State *L, صحیح obj, ثابت نویسه *event) {
  obj = lua_absindex(L, obj);
  اگر (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    بازگشت 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  بازگشت 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, صحیح idx) {
  lua_Integer l;
  صحیح isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  اگر (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_برداشت(L, 1);  /* remove object */
  بازگشت l;
}


LUALIB_API ثابت نویسه *luaL_tolstring (lua_State *L, صحیح idx, size_t *len) {
  idx = lua_absindex(L,idx);
  اگر (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    اگر (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  وگرنه {
    انتخاب (lua_type(L, idx)) {
      حالت LUA_TNUMBER: {
        نویسه buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        بشکن;
      }
      حالت LUA_TSTRING:
        lua_pushvalue(L, idx);
        بشکن;
      حالت LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        بشکن;
      حالت LUA_TNIL:
        lua_pushliteral(L, "nil");
        بشکن;
      پیشفرض: {
        صحیح tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ثابت نویسه *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        اگر (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        بشکن;
      }
    }
  }
  بازگشت lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API تهی luaL_setfuncs (lua_State *L, ثابت luaL_Reg *l, صحیح nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  برای (; l->name != پوچ; l++) {  /* fill the table with given functions */
    اگر (l->func == پوچ)  /* placeholder? */
      lua_pushboolean(L, 0);
    وگرنه {
      صحیح i;
      برای (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_برداشت(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API صحیح luaL_getsubtable (lua_State *L, صحیح idx, ثابت نویسه *fname) {
  اگر (lua_getfield(L, idx, fname) == LUA_TTABLE)
    بازگشت 1;  /* table already there */
  وگرنه {
    lua_برداشت(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    بازگشت 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API تهی luaL_requiref (lua_State *L, ثابت نویسه *modname,
                               lua_CFunction openf, صحیح glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  اگر (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_برداشت(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  اگر (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API تهی luaL_addgsub (luaL_Buffer *b, ثابت نویسه *s,
                                     ثابت نویسه *p, ثابت نویسه *r) {
  ثابت نویسه *wild;
  size_t l = strlen(p);
  وقتی ((wild = strstr(s, p)) != پوچ) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ثابت نویسه *luaL_gsub (lua_State *L, ثابت نویسه *s,
                                  ثابت نویسه *p, ثابت نویسه *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  بازگشت lua_tostring(L, -1);
}


تهی *luaL_alloc (تهی *ud, تهی *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  اگر (nsize == 0) {
    آزاد(ptr);
    بازگشت پوچ;
  }
  وگرنه
    بازگشت realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
ایستا صحیح panic (lua_State *L) {
  ثابت نویسه *پیام = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        پیام);
  بازگشت 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
ایستا تهی warnfoff (تهی *ud, ثابت نویسه *message, صحیح tocont);
ایستا تهی warnfon (تهی *ud, ثابت نویسه *message, صحیح tocont);
ایستا تهی warnfcont (تهی *ud, ثابت نویسه *message, صحیح tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
ایستا صحیح checkcontrol (lua_State *L, ثابت نویسه *message, صحیح tocont) {
  اگر (tocont || *(message++) != '@')  /* not a control message? */
    بازگشت 0;
  وگرنه {
    اگر (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    وگرنه اگر (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    بازگشت 1;  /* it was a control message */
  }
}


ایستا تهی warnfoff (تهی *ud, ثابت نویسه *message, صحیح tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
ایستا تهی warnfcont (تهی *ud, ثابت نویسه *message, صحیح tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  اگر (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  وگرنه {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


ایستا تهی warnfon (تهی *ud, ثابت نویسه *message, صحیح tocont) {
  اگر (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    بازگشت;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#اگر !defined(luai_makeseed)

#شامل <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(اندازه(تهی*) + اندازه(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + اندازه(صحیح) - 1) / اندازه(صحیح))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), اندازه(v)), b += اندازه(v))


ایستا unsigned صحیح luai_makeseed (تهی) {
  unsigned صحیح buff[BUFSEED];
  unsigned صحیح res;
  unsigned صحیح i;
  time_t t = time(پوچ);
  نویسه *b = (نویسه*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, اندازه(buff) - BUFSEEDB);
  res = buff[0];
  برای (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  بازگشت res;
}

#endif


LUALIB_API unsigned صحیح luaL_makeseed (lua_State *L) {
  UNUSED(L);
  بازگشت luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (تهی) {
  lua_State *L = lua_newstate(luaL_alloc, پوچ, luaL_makeseed(پوچ));
  اگر (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  بازگشت L;
}


LUALIB_API تهی luaL_checkversion_ (lua_State *L, lua_عدد ver, size_t sz) {
  lua_عدد v = lua_version(L);
  اگر (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  وگرنه اگر (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

