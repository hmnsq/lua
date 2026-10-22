/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "llimits.h"


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
স্থির পূর্ণসংখ্যা findfield (lua_State *L, পূর্ণসংখ্যা objidx, পূর্ণসংখ্যা level) {
  যদি (level == 0 || !lua_istable(L, -1))
    ফেরত 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  যখন (lua_পরবর্তী(L, -2)) {  /* for each pair in table */
    যদি (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      যদি (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_বের_করো(L, 1);  /* remove value (but keep name) */
        ফেরত 1;
      }
      অন্যথা যদি (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        ফেরত 1;
      }
    }
    lua_বের_করো(L, 1);  /* remove value */
  }
  ফেরত 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
স্থির পূর্ণসংখ্যা pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  পূর্ণসংখ্যা শীর্ষ = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  যদি (findfield(L, শীর্ষ + 1, 2)) {
    ধ্রুবক অক্ষর *name = lua_tostring(L, -1);
    যদি (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, শীর্ষ + 1);  /* copy name to proper place */
    lua_settop(L, শীর্ষ + 1);  /* remove table "loaded" and name copy */
    ফেরত 1;
  }
  অন্যথা {
    lua_settop(L, শীর্ষ);  /* remove function and global table */
    ফেরত 0;
  }
}


স্থির শূন্য pushfuncname (lua_State *L, lua_Debug *ar) {
  যদি (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  অন্যথা যদি (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  অন্যথা যদি (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  অন্যথা যদি (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  অন্যথা  /* nothing left... */
    lua_pushliteral(L, "?");
}


স্থির পূর্ণসংখ্যা lastlevel (lua_State *L) {
  lua_Debug ar;
  পূর্ণসংখ্যা li = 1, le = 1;
  /* find an upper bound */
  যখন (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  যখন (li < le) {
    পূর্ণসংখ্যা m = (li + le)/2;
    যদি (lua_getstack(L, m, &ar)) li = m + 1;
    অন্যথা le = m;
  }
  ফেরত le - 1;
}


LUALIB_API শূন্য luaL_traceback (lua_State *L, lua_State *L1,
                                ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা level) {
  lua_L_বাফার b;
  lua_Debug ar;
  পূর্ণসংখ্যা last = lastlevel(L1);
  পূর্ণসংখ্যা limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  যদি (বার্তা) {
    luaL_addstring(&b, বার্তা);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  যখন (lua_getstack(L1, level++, &ar)) {
    যদি (limit2show-- == 0) {  /* too many levels? */
      পূর্ণসংখ্যা n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    অন্যথা {
      lua_getinfo(L1, "Slnt", &ar);
      যদি (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      অন্যথা
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      যদি (ar.istailcall)
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

LUALIB_API পূর্ণসংখ্যা luaL_argerror (lua_State *L, পূর্ণসংখ্যা arg, ধ্রুবক অক্ষর *extramsg) {
  lua_Debug ar;
  ধ্রুবক অক্ষর *argword;
  যদি (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    ফেরত luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  যদি (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  অন্যথা {
    arg -= ar.extraargs;  /* do not count extra arguments */
    যদি (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      যদি (arg == 0)  /* error in self argument? */
        ফেরত luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  যদি (ar.name == ফাঁকা)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  ফেরত luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API পূর্ণসংখ্যা luaL_typeerror (lua_State *L, পূর্ণসংখ্যা arg, ধ্রুবক অক্ষর *tname) {
  ধ্রুবক অক্ষর *বার্তা;
  ধ্রুবক অক্ষর *typearg;  /* name for the type of the actual argument */
  যদি (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  অন্যথা যদি (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  অন্যথা
    typearg = luaL_typename(L, arg);  /* standard name */
  বার্তা = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  ফেরত luaL_argerror(L, arg, বার্তা);
}


স্থির শূন্য tag_error (lua_State *L, পূর্ণসংখ্যা arg, পূর্ণসংখ্যা tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API শূন্য luaL_where (lua_State *L, পূর্ণসংখ্যা level) {
  lua_Debug ar;
  যদি (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    যদি (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      ফেরত;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API পূর্ণসংখ্যা luaL_error (lua_State *L, ধ্রুবক অক্ষর *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  ফেরত lua_error(L);
}


LUALIB_API পূর্ণসংখ্যা luaL_fileresult (lua_State *L, পূর্ণসংখ্যা stat, ধ্রুবক অক্ষর *fname) {
  পূর্ণসংখ্যা en = errno;  /* calls to Lua API may change this value */
  যদি (stat) {
    lua_pushboolean(L, 1);
    ফেরত 1;
  }
  অন্যথা {
    ধ্রুবক অক্ষর *বার্তা;
    luaL_pushfail(L);
    বার্তা = (en != 0) ? strerror(en) : "(no extra info)";
    যদি (fname)
      lua_pushfstring(L, "%s: %s", fname, বার্তা);
    অন্যথা
      lua_pushstring(L, বার্তা);
    lua_pushinteger(L, en);
    ফেরত 3;
  }
}


#যদি !defined(l_inspectstat)	/* { */

#যদি defined(LUA_USE_POSIX)

#অন্তর্ভাব <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   যদি (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   অন্যথা যদি (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#অন্যথা

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API পূর্ণসংখ্যা luaL_execresult (lua_State *L, পূর্ণসংখ্যা stat) {
  যদি (stat != 0 && errno != 0)  /* error with an 'errno'? */
    ফেরত luaL_fileresult(L, 0, ফাঁকা);
  অন্যথা {
    ধ্রুবক অক্ষর *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    যদি (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    অন্যথা
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    ফেরত 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API পূর্ণসংখ্যা luaL_newmetatable (lua_State *L, ধ্রুবক অক্ষর *tname) {
  যদি (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    ফেরত 0;  /* leave previous value on top, but return 0 */
  lua_বের_করো(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  ফেরত 1;
}


LUALIB_API শূন্য luaL_setmetatable (lua_State *L, ধ্রুবক অক্ষর *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API শূন্য *luaL_testudata (lua_State *L, পূর্ণসংখ্যা ud, ধ্রুবক অক্ষর *tname) {
  শূন্য *p = lua_touserdata(L, ud);
  যদি (p != ফাঁকা) {  /* value is a userdata? */
    যদি (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      যদি (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ফাঁকা;  /* value is a userdata with wrong metatable */
      lua_বের_করো(L, 2);  /* remove both metatables */
      ফেরত p;
    }
  }
  ফেরত ফাঁকা;  /* value is not a userdata with a metatable */
}


LUALIB_API শূন্য *luaL_checkudata (lua_State *L, পূর্ণসংখ্যা ud, ধ্রুবক অক্ষর *tname) {
  শূন্য *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ফাঁকা, ud, tname);
  ফেরত p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API পূর্ণসংখ্যা luaL_checkoption (lua_State *L, পূর্ণসংখ্যা arg, ধ্রুবক অক্ষর *def,
                                 ধ্রুবক অক্ষর *ধ্রুবক lst[]) {
  ধ্রুবক অক্ষর *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  পূর্ণসংখ্যা i;
  জন্য (i=0; lst[i]; i++)
    যদি (strcmp(lst[i], name) == 0)
      ফেরত i;
  ফেরত luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API শূন্য luaL_checkstack (lua_State *L, পূর্ণসংখ্যা space, ধ্রুবক অক্ষর *বার্তা) {
  যদি (l_unlikely(!lua_checkstack(L, space))) {
    যদি (বার্তা)
      luaL_error(L, "stack overflow (%s)", বার্তা);
    অন্যথা
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API শূন্য luaL_checktype (lua_State *L, পূর্ণসংখ্যা arg, পূর্ণসংখ্যা t) {
  যদি (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API শূন্য luaL_checkany (lua_State *L, পূর্ণসংখ্যা arg) {
  যদি (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ধ্রুবক অক্ষর *luaL_checklstring (lua_State *L, পূর্ণসংখ্যা arg, size_t *len) {
  ধ্রুবক অক্ষর *s = lua_tolstring(L, arg, len);
  যদি (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  ফেরত s;
}


LUALIB_API ধ্রুবক অক্ষর *luaL_optlstring (lua_State *L, পূর্ণসংখ্যা arg,
                                        ধ্রুবক অক্ষর *def, size_t *len) {
  যদি (lua_isnoneornil(L, arg)) {
    যদি (len)
      *len = (def ? strlen(def) : 0);
    ফেরত def;
  }
  অন্যথা ফেরত luaL_checklstring(L, arg, len);
}


LUALIB_API lua_সংখ্যা luaL_checknumber (lua_State *L, পূর্ণসংখ্যা arg) {
  পূর্ণসংখ্যা isnum;
  lua_সংখ্যা d = lua_tonumberx(L, arg, &isnum);
  যদি (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  ফেরত d;
}


LUALIB_API lua_সংখ্যা luaL_optnumber (lua_State *L, পূর্ণসংখ্যা arg, lua_সংখ্যা def) {
  ফেরত luaL_opt(L, luaL_checknumber, arg, def);
}


স্থির শূন্য interror (lua_State *L, পূর্ণসংখ্যা arg) {
  যদি (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  অন্যথা
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, পূর্ণসংখ্যা arg) {
  পূর্ণসংখ্যা isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  যদি (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  ফেরত d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, পূর্ণসংখ্যা arg,
                                                      lua_Integer def) {
  ফেরত luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
প্রকারলেখ কাঠামো UBox {
  শূন্য *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
স্থির শূন্য *resizebox (lua_State *L, পূর্ণসংখ্যা idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  যদি (box->bsize == newsize)  /* not changing size? */
    ফেরত box->box;  /* keep the buffer */
  অন্যথা {
    শূন্য *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    শূন্য *অস্থায়ী = allocf(ud, box->box, box->bsize, newsize);
    যদি (l_unlikely(অস্থায়ী == ফাঁকা && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = অস্থায়ী;
    box->bsize = newsize;
    ফেরত অস্থায়ী;
  }
}


স্থির পূর্ণসংখ্যা boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  ফেরত 0;
}


স্থির ধ্রুবক luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ফাঁকা, ফাঁকা}
};


/*
** Get/create metatable (MT) for boxes
*/
স্থির শূন্য getBoxMT (lua_State *L) {
  ধ্রুবক অক্ষর *BOXMT = "_UBOX*"; /* key for the metatable */
  যদি (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


স্থির শূন্য newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, আকার(UBox), 0);
  box->box = ফাঁকা;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ফাঁকা  \
                            : lua_touserdata(B->L, idx) == (শূন্য*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
স্থির size_t newbuffsize (lua_L_বাফার *B, size_t sz) {
  size_t newsize = B->আকার_2;
  যদি (l_unlikely(sz >= MAX_আকার - B->n))
    ফেরত cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  যদি (newsize <= MAX_আকার/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  যদি (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  ফেরত newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
স্থির অক্ষর *prepbuffsize (lua_L_বাফার *B, size_t sz, পূর্ণসংখ্যা boxidx) {
  checkbufferlevel(B, boxidx);
  যদি (B->আকার_2 - B->n >= sz)  /* enough space? */
    ফেরত B->b + B->n;
  অন্যথা {
    lua_State *L = B->L;
    অক্ষর *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    যদি (buffonstack(B))  /* buffer already has a box? */
      newbuff = (অক্ষর *)resizebox(L, boxidx, newsize);  /* resize it */
    অন্যথা {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_ঢোকাও(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (অক্ষর *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * আকার(অক্ষর));  /* copy original content */
    }
    B->b = newbuff;
    B->আকার_2 = newsize;
    ফেরত newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API অক্ষর *luaL_prepbuffsize (lua_L_বাফার *B, size_t sz) {
  ফেরত prepbuffsize(B, sz, -1);
}


LUALIB_API শূন্য luaL_addlstring (lua_L_বাফার *B, ধ্রুবক অক্ষর *s, size_t l) {
  যদি (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    অক্ষর *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * আকার(অক্ষর));
    luaL_addsize(B, l);
  }
}


LUALIB_API শূন্য luaL_addstring (lua_L_বাফার *B, ধ্রুবক অক্ষর *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API শূন্য luaL_pushresult (lua_L_বাফার *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  যদি (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  অন্যথা {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    শূন্য *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    অক্ষর *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (অক্ষর*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ফাঁকা;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API শূন্য luaL_pushresultsize (lua_L_বাফার *B, size_t sz) {
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
LUALIB_API শূন্য luaL_addvalue (lua_L_বাফার *B) {
  lua_State *L = B->L;
  size_t len;
  ধ্রুবক অক্ষর *s = lua_tolstring(L, -1, &len);
  অক্ষর *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * আকার(অক্ষর));
  luaL_addsize(B, len);
  lua_বের_করো(L, 1);  /* pop string */
}


LUALIB_API শূন্য luaL_buffinit (lua_State *L, lua_L_বাফার *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->আকার_2 = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (শূন্য*)B);  /* push placeholder */
}


LUALIB_API অক্ষর *luaL_buffinitsize (lua_State *L, lua_L_বাফার *B, size_t sz) {
  luaL_buffinit(L, B);
  ফেরত prepbuffsize(B, sz, -1);
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
LUALIB_API পূর্ণসংখ্যা luaL_ref (lua_State *L, পূর্ণসংখ্যা t) {
  পূর্ণসংখ্যা ref;
  যদি (lua_isnil(L, -1)) {
    lua_বের_করো(L, 1);  /* remove from stack */
    ফেরত LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  যদি (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (পূর্ণসংখ্যা)lua_tointeger(L, -1);  /* ref = t[1] */
  অন্যথা {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_বের_করো(L, 1);  /* remove element from stack */
  যদি (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  অন্যথা  /* no free elements */
    ref = (পূর্ণসংখ্যা)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  ফেরত ref;
}


LUALIB_API শূন্য luaL_unref (lua_State *L, পূর্ণসংখ্যা t, পূর্ণসংখ্যা ref) {
  যদি (ref >= 0) {
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

প্রকারলেখ কাঠামো LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  অক্ষর buff[BUFSIZ];  /* area for reading file */
} LoadF;


স্থির ধ্রুবক অক্ষর *getF (lua_State *L, শূন্য *ud, size_t *আকার_2) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  যদি (lf->n > 0) {  /* are there pre-read characters to be read? */
    *আকার_2 = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  অন্যথা {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    যদি (feof(lf->f)) ফেরত ফাঁকা;
    *আকার_2 = fread(lf->buff, 1, আকার(lf->buff), lf->f);  /* read block */
  }
  ফেরত lf->buff;
}


স্থির পূর্ণসংখ্যা errfile (lua_State *L, ধ্রুবক অক্ষর *what, পূর্ণসংখ্যা fnameindex) {
  পূর্ণসংখ্যা err = errno;
  ধ্রুবক অক্ষর *filename = lua_tostring(L, fnameindex) + 1;
  যদি (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  অন্যথা
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  ফেরত LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
স্থির পূর্ণসংখ্যা skipBOM (FILE *f) {
  পূর্ণসংখ্যা c = getc(f);  /* read first character */
  যদি (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    ফেরত getc(f);  /* ignore BOM and return next char */
  অন্যথা  /* no (valid) BOM */
    ফেরত c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
স্থির পূর্ণসংখ্যা skipcomment (FILE *f, পূর্ণসংখ্যা *cp) {
  পূর্ণসংখ্যা c = *cp = skipBOM(f);
  যদি (c == '#') {  /* first line is a comment (Unix exec. file)? */
    করো {  /* skip first line */
      c = getc(f);
    } যখন (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    ফেরত 1;  /* there was a comment */
  }
  অন্যথা ফেরত 0;  /* no comment */
}


LUALIB_API পূর্ণসংখ্যা luaL_loadfilex (lua_State *L, ধ্রুবক অক্ষর *filename,
                                             ধ্রুবক অক্ষর *mode) {
  LoadF lf;
  পূর্ণসংখ্যা status, readstatus;
  পূর্ণসংখ্যা c;
  পূর্ণসংখ্যা fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  যদি (filename == ফাঁকা) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  অন্যথা {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    যদি (lf.f == ফাঁকা) ফেরত errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  যদি (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  যদি (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    যদি (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      যদি (lf.f == ফাঁকা) ফেরত errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  যদি (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  যদি (filename) fclose(lf.f);  /* close file (even in case of errors) */
  যদি (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    ফেরত errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  ফেরত status;
}


প্রকারলেখ কাঠামো LoadS {
  ধ্রুবক অক্ষর *s;
  size_t আকার_2;
} LoadS;


স্থির ধ্রুবক অক্ষর *getS (lua_State *L, শূন্য *ud, size_t *আকার_2) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  যদি (ls->আকার_2 == 0) ফেরত ফাঁকা;
  *আকার_2 = ls->আকার_2;
  ls->আকার_2 = 0;
  ফেরত ls->s;
}


LUALIB_API পূর্ণসংখ্যা luaL_loadbufferx (lua_State *L, ধ্রুবক অক্ষর *buff, size_t আকার_2,
                                 ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *mode) {
  LoadS ls;
  ls.s = buff;
  ls.আকার_2 = আকার_2;
  ফেরত lua_load(L, getS, &ls, name, mode);
}


LUALIB_API পূর্ণসংখ্যা luaL_loadstring (lua_State *L, ধ্রুবক অক্ষর *s) {
  ফেরত luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API পূর্ণসংখ্যা luaL_getmetafield (lua_State *L, পূর্ণসংখ্যা obj, ধ্রুবক অক্ষর *event) {
  যদি (!lua_getmetatable(L, obj))  /* no metatable? */
    ফেরত LUA_TNIL;
  অন্যথা {
    পূর্ণসংখ্যা tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    যদি (tt == LUA_TNIL)  /* is metafield nil? */
      lua_বের_করো(L, 2);  /* remove metatable and metafield */
    অন্যথা
      lua_remove(L, -2);  /* remove only metatable */
    ফেরত tt;  /* return metafield type */
  }
}


LUALIB_API পূর্ণসংখ্যা luaL_callmeta (lua_State *L, পূর্ণসংখ্যা obj, ধ্রুবক অক্ষর *event) {
  obj = lua_absindex(L, obj);
  যদি (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    ফেরত 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  ফেরত 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, পূর্ণসংখ্যা idx) {
  lua_Integer l;
  পূর্ণসংখ্যা isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  যদি (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_বের_করো(L, 1);  /* remove object */
  ফেরত l;
}


LUALIB_API ধ্রুবক অক্ষর *luaL_tolstring (lua_State *L, পূর্ণসংখ্যা idx, size_t *len) {
  idx = lua_absindex(L,idx);
  যদি (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    যদি (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  অন্যথা {
    নির্বাচন (lua_type(L, idx)) {
      ক্ষেত্র LUA_TNUMBER: {
        অক্ষর buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        বিরতি;
      }
      ক্ষেত্র LUA_TSTRING:
        lua_pushvalue(L, idx);
        বিরতি;
      ক্ষেত্র LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        বিরতি;
      ক্ষেত্র LUA_TNIL:
        lua_pushliteral(L, "nil");
        বিরতি;
      পূর্বনির্ধারিত: {
        পূর্ণসংখ্যা tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ধ্রুবক অক্ষর *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        যদি (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        বিরতি;
      }
    }
  }
  ফেরত lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API শূন্য luaL_setfuncs (lua_State *L, ধ্রুবক luaL_Reg *l, পূর্ণসংখ্যা nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  জন্য (; l->name != ফাঁকা; l++) {  /* fill the table with given functions */
    যদি (l->func == ফাঁকা)  /* placeholder? */
      lua_pushboolean(L, 0);
    অন্যথা {
      পূর্ণসংখ্যা i;
      জন্য (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_বের_করো(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API পূর্ণসংখ্যা luaL_getsubtable (lua_State *L, পূর্ণসংখ্যা idx, ধ্রুবক অক্ষর *fname) {
  যদি (lua_getfield(L, idx, fname) == LUA_TTABLE)
    ফেরত 1;  /* table already there */
  অন্যথা {
    lua_বের_করো(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    ফেরত 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API শূন্য luaL_requiref (lua_State *L, ধ্রুবক অক্ষর *modname,
                               lua_CFunction openf, পূর্ণসংখ্যা glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  যদি (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_বের_করো(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  যদি (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API শূন্য luaL_addgsub (lua_L_বাফার *b, ধ্রুবক অক্ষর *s,
                                     ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *r) {
  ধ্রুবক অক্ষর *wild;
  size_t l = strlen(p);
  যখন ((wild = strstr(s, p)) != ফাঁকা) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ধ্রুবক অক্ষর *luaL_gsub (lua_State *L, ধ্রুবক অক্ষর *s,
                                  ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *r) {
  lua_L_বাফার b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  ফেরত lua_tostring(L, -1);
}


শূন্য *luaL_alloc (শূন্য *ud, শূন্য *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  যদি (nsize == 0) {
    মুক্ত(ptr);
    ফেরত ফাঁকা;
  }
  অন্যথা
    ফেরত realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
স্থির পূর্ণসংখ্যা panic (lua_State *L) {
  ধ্রুবক অক্ষর *বার্তা = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        বার্তা);
  ফেরত 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
স্থির শূন্য warnfoff (শূন্য *ud, ধ্রুবক অক্ষর *message, পূর্ণসংখ্যা tocont);
স্থির শূন্য warnfon (শূন্য *ud, ধ্রুবক অক্ষর *message, পূর্ণসংখ্যা tocont);
স্থির শূন্য warnfcont (শূন্য *ud, ধ্রুবক অক্ষর *message, পূর্ণসংখ্যা tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
স্থির পূর্ণসংখ্যা checkcontrol (lua_State *L, ধ্রুবক অক্ষর *message, পূর্ণসংখ্যা tocont) {
  যদি (tocont || *(message++) != '@')  /* not a control message? */
    ফেরত 0;
  অন্যথা {
    যদি (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    অন্যথা যদি (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    ফেরত 1;  /* it was a control message */
  }
}


স্থির শূন্য warnfoff (শূন্য *ud, ধ্রুবক অক্ষর *message, পূর্ণসংখ্যা tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
স্থির শূন্য warnfcont (শূন্য *ud, ধ্রুবক অক্ষর *message, পূর্ণসংখ্যা tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  যদি (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  অন্যথা {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


স্থির শূন্য warnfon (শূন্য *ud, ধ্রুবক অক্ষর *message, পূর্ণসংখ্যা tocont) {
  যদি (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    ফেরত;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#যদি !defined(luai_makeseed)

#অন্তর্ভাব <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(আকার(শূন্য*) + আকার(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + আকার(পূর্ণসংখ্যা) - 1) / আকার(পূর্ণসংখ্যা))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), আকার(v)), b += আকার(v))


স্থির unsigned পূর্ণসংখ্যা luai_makeseed (শূন্য) {
  unsigned পূর্ণসংখ্যা buff[BUFSEED];
  unsigned পূর্ণসংখ্যা res;
  unsigned পূর্ণসংখ্যা i;
  time_t t = time(ফাঁকা);
  অক্ষর *b = (অক্ষর*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, আকার(buff) - BUFSEEDB);
  res = buff[0];
  জন্য (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  ফেরত res;
}

#endif


LUALIB_API unsigned পূর্ণসংখ্যা luaL_makeseed (lua_State *L) {
  UNUSED(L);
  ফেরত luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (শূন্য) {
  lua_State *L = lua_newstate(luaL_alloc, ফাঁকা, luaL_makeseed(ফাঁকা));
  যদি (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  ফেরত L;
}


LUALIB_API শূন্য luaL_checkversion_ (lua_State *L, lua_সংখ্যা ver, size_t sz) {
  lua_সংখ্যা v = lua_version(L);
  যদি (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  অন্যথা যদি (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

