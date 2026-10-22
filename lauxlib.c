/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "llimits.h"


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
நிலையான முழு findfield (lua_State *L, முழு objidx, முழு level) {
  எனில் (level == 0 || !lua_istable(L, -1))
    திரும்பு 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  போது (lua_next(L, -2)) {  /* for each pair in table */
    எனில் (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      எனில் (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_எடு(L, 1);  /* remove value (but keep name) */
        திரும்பு 1;
      }
      இல்லையெனில் எனில் (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        திரும்பு 1;
      }
    }
    lua_எடு(L, 1);  /* remove value */
  }
  திரும்பு 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
நிலையான முழு pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  முழு உச்சி = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  எனில் (findfield(L, உச்சி + 1, 2)) {
    மாறிலி எழுத்து *name = lua_tostring(L, -1);
    எனில் (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, உச்சி + 1);  /* copy name to proper place */
    lua_settop(L, உச்சி + 1);  /* remove table "loaded" and name copy */
    திரும்பு 1;
  }
  இல்லையெனில் {
    lua_settop(L, உச்சி);  /* remove function and global table */
    திரும்பு 0;
  }
}


நிலையான வெறுமை pushfuncname (lua_State *L, lua_Debug *ar) {
  எனில் (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  இல்லையெனில் எனில் (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  இல்லையெனில் எனில் (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  இல்லையெனில் எனில் (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  இல்லையெனில்  /* nothing left... */
    lua_pushliteral(L, "?");
}


நிலையான முழு lastlevel (lua_State *L) {
  lua_Debug ar;
  முழு li = 1, le = 1;
  /* find an upper bound */
  போது (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  போது (li < le) {
    முழு m = (li + le)/2;
    எனில் (lua_getstack(L, m, &ar)) li = m + 1;
    இல்லையெனில் le = m;
  }
  திரும்பு le - 1;
}


LUALIB_API வெறுமை luaL_traceback (lua_State *L, lua_State *L1,
                                மாறிலி எழுத்து *செய்தி, முழு level) {
  luaL_Buffer b;
  lua_Debug ar;
  முழு last = lastlevel(L1);
  முழு limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  எனில் (செய்தி) {
    luaL_addstring(&b, செய்தி);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  போது (lua_getstack(L1, level++, &ar)) {
    எனில் (limit2show-- == 0) {  /* too many levels? */
      முழு n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    இல்லையெனில் {
      lua_getinfo(L1, "Slnt", &ar);
      எனில் (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      இல்லையெனில்
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      எனில் (ar.istailcall)
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

LUALIB_API முழு luaL_argerror (lua_State *L, முழு arg, மாறிலி எழுத்து *extramsg) {
  lua_Debug ar;
  மாறிலி எழுத்து *argword;
  எனில் (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    திரும்பு luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  எனில் (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  இல்லையெனில் {
    arg -= ar.extraargs;  /* do not count extra arguments */
    எனில் (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      எனில் (arg == 0)  /* error in self argument? */
        திரும்பு luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  எனில் (ar.name == வெற்று)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  திரும்பு luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API முழு luaL_typeerror (lua_State *L, முழு arg, மாறிலி எழுத்து *tname) {
  மாறிலி எழுத்து *செய்தி;
  மாறிலி எழுத்து *typearg;  /* name for the type of the actual argument */
  எனில் (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  இல்லையெனில் எனில் (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  இல்லையெனில்
    typearg = luaL_typename(L, arg);  /* standard name */
  செய்தி = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  திரும்பு luaL_argerror(L, arg, செய்தி);
}


நிலையான வெறுமை tag_error (lua_State *L, முழு arg, முழு tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API வெறுமை luaL_where (lua_State *L, முழு level) {
  lua_Debug ar;
  எனில் (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    எனில் (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      திரும்பு;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API முழு luaL_error (lua_State *L, மாறிலி எழுத்து *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  திரும்பு lua_error(L);
}


LUALIB_API முழு luaL_fileresult (lua_State *L, முழு stat, மாறிலி எழுத்து *fname) {
  முழு en = errno;  /* calls to Lua API may change this value */
  எனில் (stat) {
    lua_pushboolean(L, 1);
    திரும்பு 1;
  }
  இல்லையெனில் {
    மாறிலி எழுத்து *செய்தி;
    luaL_pushfail(L);
    செய்தி = (en != 0) ? strerror(en) : "(no extra info)";
    எனில் (fname)
      lua_pushfstring(L, "%s: %s", fname, செய்தி);
    இல்லையெனில்
      lua_pushstring(L, செய்தி);
    lua_pushinteger(L, en);
    திரும்பு 3;
  }
}


#எனில் !defined(l_inspectstat)	/* { */

#எனில் defined(LUA_USE_POSIX)

#அந்தர்பாவ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   எனில் (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   இல்லையெனில் எனில் (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#இல்லையெனில்

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API முழு luaL_execresult (lua_State *L, முழு stat) {
  எனில் (stat != 0 && errno != 0)  /* error with an 'errno'? */
    திரும்பு luaL_fileresult(L, 0, வெற்று);
  இல்லையெனில் {
    மாறிலி எழுத்து *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    எனில் (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    இல்லையெனில்
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    திரும்பு 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API முழு luaL_newmetatable (lua_State *L, மாறிலி எழுத்து *tname) {
  எனில் (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    திரும்பு 0;  /* leave previous value on top, but return 0 */
  lua_எடு(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  திரும்பு 1;
}


LUALIB_API வெறுமை luaL_setmetatable (lua_State *L, மாறிலி எழுத்து *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API வெறுமை *luaL_testudata (lua_State *L, முழு ud, மாறிலி எழுத்து *tname) {
  வெறுமை *p = lua_touserdata(L, ud);
  எனில் (p != வெற்று) {  /* value is a userdata? */
    எனில் (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      எனில் (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = வெற்று;  /* value is a userdata with wrong metatable */
      lua_எடு(L, 2);  /* remove both metatables */
      திரும்பு p;
    }
  }
  திரும்பு வெற்று;  /* value is not a userdata with a metatable */
}


LUALIB_API வெறுமை *luaL_checkudata (lua_State *L, முழு ud, மாறிலி எழுத்து *tname) {
  வெறுமை *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != வெற்று, ud, tname);
  திரும்பு p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API முழு luaL_checkoption (lua_State *L, முழு arg, மாறிலி எழுத்து *def,
                                 மாறிலி எழுத்து *மாறிலி lst[]) {
  மாறிலி எழுத்து *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  முழு i;
  க்கு (i=0; lst[i]; i++)
    எனில் (strcmp(lst[i], name) == 0)
      திரும்பு i;
  திரும்பு luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API வெறுமை luaL_checkstack (lua_State *L, முழு space, மாறிலி எழுத்து *செய்தி) {
  எனில் (l_unlikely(!lua_checkstack(L, space))) {
    எனில் (செய்தி)
      luaL_error(L, "stack overflow (%s)", செய்தி);
    இல்லையெனில்
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API வெறுமை luaL_checktype (lua_State *L, முழு arg, முழு t) {
  எனில் (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API வெறுமை luaL_checkany (lua_State *L, முழு arg) {
  எனில் (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API மாறிலி எழுத்து *luaL_checklstring (lua_State *L, முழு arg, size_t *len) {
  மாறிலி எழுத்து *s = lua_tolstring(L, arg, len);
  எனில் (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  திரும்பு s;
}


LUALIB_API மாறிலி எழுத்து *luaL_optlstring (lua_State *L, முழு arg,
                                        மாறிலி எழுத்து *def, size_t *len) {
  எனில் (lua_isnoneornil(L, arg)) {
    எனில் (len)
      *len = (def ? strlen(def) : 0);
    திரும்பு def;
  }
  இல்லையெனில் திரும்பு luaL_checklstring(L, arg, len);
}


LUALIB_API lua_எண் luaL_checknumber (lua_State *L, முழு arg) {
  முழு isnum;
  lua_எண் d = lua_tonumberx(L, arg, &isnum);
  எனில் (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  திரும்பு d;
}


LUALIB_API lua_எண் luaL_optnumber (lua_State *L, முழு arg, lua_எண் def) {
  திரும்பு luaL_opt(L, luaL_checknumber, arg, def);
}


நிலையான வெறுமை interror (lua_State *L, முழு arg) {
  எனில் (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  இல்லையெனில்
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, முழு arg) {
  முழு isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  எனில் (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  திரும்பு d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, முழு arg,
                                                      lua_Integer def) {
  திரும்பு luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
வகைவரை கட்டமைப்பு UBox {
  வெறுமை *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
நிலையான வெறுமை *resizebox (lua_State *L, முழு idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  எனில் (box->bsize == newsize)  /* not changing size? */
    திரும்பு box->box;  /* keep the buffer */
  இல்லையெனில் {
    வெறுமை *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    வெறுமை *temp = allocf(ud, box->box, box->bsize, newsize);
    எனில் (l_unlikely(temp == வெற்று && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    திரும்பு temp;
  }
}


நிலையான முழு boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  திரும்பு 0;
}


நிலையான மாறிலி luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {வெற்று, வெற்று}
};


/*
** Get/create metatable (MT) for boxes
*/
நிலையான வெறுமை getBoxMT (lua_State *L) {
  மாறிலி எழுத்து *BOXMT = "_UBOX*"; /* key for the metatable */
  எனில் (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


நிலையான வெறுமை newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, அளவு(UBox), 0);
  box->box = வெற்று;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != வெற்று  \
                            : lua_touserdata(B->L, idx) == (வெறுமை*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
நிலையான size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  எனில் (l_unlikely(sz >= MAX_SIZE - B->n))
    திரும்பு cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  எனில் (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  எனில் (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  திரும்பு newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
நிலையான எழுத்து *prepbuffsize (luaL_Buffer *B, size_t sz, முழு boxidx) {
  checkbufferlevel(B, boxidx);
  எனில் (B->size - B->n >= sz)  /* enough space? */
    திரும்பு B->b + B->n;
  இல்லையெனில் {
    lua_State *L = B->L;
    எழுத்து *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    எனில் (buffonstack(B))  /* buffer already has a box? */
      newbuff = (எழுத்து *)resizebox(L, boxidx, newsize);  /* resize it */
    இல்லையெனில் {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_செருகு(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (எழுத்து *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * அளவு(எழுத்து));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    திரும்பு newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API எழுத்து *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  திரும்பு prepbuffsize(B, sz, -1);
}


LUALIB_API வெறுமை luaL_addlstring (luaL_Buffer *B, மாறிலி எழுத்து *s, size_t l) {
  எனில் (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    எழுத்து *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * அளவு(எழுத்து));
    luaL_addsize(B, l);
  }
}


LUALIB_API வெறுமை luaL_addstring (luaL_Buffer *B, மாறிலி எழுத்து *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API வெறுமை luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  எனில் (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  இல்லையெனில் {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    வெறுமை *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    எழுத்து *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (எழுத்து*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = வெற்று;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API வெறுமை luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API வெறுமை luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  மாறிலி எழுத்து *s = lua_tolstring(L, -1, &len);
  எழுத்து *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * அளவு(எழுத்து));
  luaL_addsize(B, len);
  lua_எடு(L, 1);  /* pop string */
}


LUALIB_API வெறுமை luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (வெறுமை*)B);  /* push placeholder */
}


LUALIB_API எழுத்து *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  திரும்பு prepbuffsize(B, sz, -1);
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
LUALIB_API முழு luaL_ref (lua_State *L, முழு t) {
  முழு ref;
  எனில் (lua_isnil(L, -1)) {
    lua_எடு(L, 1);  /* remove from stack */
    திரும்பு LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  எனில் (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (முழு)lua_tointeger(L, -1);  /* ref = t[1] */
  இல்லையெனில் {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_எடு(L, 1);  /* remove element from stack */
  எனில் (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  இல்லையெனில்  /* no free elements */
    ref = (முழு)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  திரும்பு ref;
}


LUALIB_API வெறுமை luaL_unref (lua_State *L, முழு t, முழு ref) {
  எனில் (ref >= 0) {
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

வகைவரை கட்டமைப்பு LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  எழுத்து buff[BUFSIZ];  /* area for reading file */
} LoadF;


நிலையான மாறிலி எழுத்து *getF (lua_State *L, வெறுமை *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  எனில் (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  இல்லையெனில் {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    எனில் (feof(lf->f)) திரும்பு வெற்று;
    *size = fread(lf->buff, 1, அளவு(lf->buff), lf->f);  /* read block */
  }
  திரும்பு lf->buff;
}


நிலையான முழு errfile (lua_State *L, மாறிலி எழுத்து *what, முழு fnameindex) {
  முழு err = errno;
  மாறிலி எழுத்து *filename = lua_tostring(L, fnameindex) + 1;
  எனில் (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  இல்லையெனில்
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  திரும்பு LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
நிலையான முழு skipBOM (FILE *f) {
  முழு c = getc(f);  /* read first character */
  எனில் (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    திரும்பு getc(f);  /* ignore BOM and return next char */
  இல்லையெனில்  /* no (valid) BOM */
    திரும்பு c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
நிலையான முழு skipcomment (FILE *f, முழு *cp) {
  முழு c = *cp = skipBOM(f);
  எனில் (c == '#') {  /* first line is a comment (Unix exec. file)? */
    செய் {  /* skip first line */
      c = getc(f);
    } போது (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    திரும்பு 1;  /* there was a comment */
  }
  இல்லையெனில் திரும்பு 0;  /* no comment */
}


LUALIB_API முழு luaL_loadfilex (lua_State *L, மாறிலி எழுத்து *filename,
                                             மாறிலி எழுத்து *mode) {
  LoadF lf;
  முழு status, readstatus;
  முழு c;
  முழு fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  எனில் (filename == வெற்று) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  இல்லையெனில் {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    எனில் (lf.f == வெற்று) திரும்பு errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  எனில் (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  எனில் (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    எனில் (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      எனில் (lf.f == வெற்று) திரும்பு errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  எனில் (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  எனில் (filename) fclose(lf.f);  /* close file (even in case of errors) */
  எனில் (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    திரும்பு errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  திரும்பு status;
}


வகைவரை கட்டமைப்பு LoadS {
  மாறிலி எழுத்து *s;
  size_t size;
} LoadS;


நிலையான மாறிலி எழுத்து *getS (lua_State *L, வெறுமை *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  எனில் (ls->size == 0) திரும்பு வெற்று;
  *size = ls->size;
  ls->size = 0;
  திரும்பு ls->s;
}


LUALIB_API முழு luaL_loadbufferx (lua_State *L, மாறிலி எழுத்து *buff, size_t size,
                                 மாறிலி எழுத்து *name, மாறிலி எழுத்து *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  திரும்பு lua_load(L, getS, &ls, name, mode);
}


LUALIB_API முழு luaL_loadstring (lua_State *L, மாறிலி எழுத்து *s) {
  திரும்பு luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API முழு luaL_getmetafield (lua_State *L, முழு obj, மாறிலி எழுத்து *event) {
  எனில் (!lua_getmetatable(L, obj))  /* no metatable? */
    திரும்பு LUA_TNIL;
  இல்லையெனில் {
    முழு tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    எனில் (tt == LUA_TNIL)  /* is metafield nil? */
      lua_எடு(L, 2);  /* remove metatable and metafield */
    இல்லையெனில்
      lua_remove(L, -2);  /* remove only metatable */
    திரும்பு tt;  /* return metafield type */
  }
}


LUALIB_API முழு luaL_callmeta (lua_State *L, முழு obj, மாறிலி எழுத்து *event) {
  obj = lua_absindex(L, obj);
  எனில் (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    திரும்பு 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  திரும்பு 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, முழு idx) {
  lua_Integer l;
  முழு isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  எனில் (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_எடு(L, 1);  /* remove object */
  திரும்பு l;
}


LUALIB_API மாறிலி எழுத்து *luaL_tolstring (lua_State *L, முழு idx, size_t *len) {
  idx = lua_absindex(L,idx);
  எனில் (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    எனில் (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  இல்லையெனில் {
    தேர்வு (lua_type(L, idx)) {
      நிலை LUA_TNUMBER: {
        எழுத்து buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        நிறுத்து;
      }
      நிலை LUA_TSTRING:
        lua_pushvalue(L, idx);
        நிறுத்து;
      நிலை LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        நிறுத்து;
      நிலை LUA_TNIL:
        lua_pushliteral(L, "nil");
        நிறுத்து;
      இயல்பு: {
        முழு tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        மாறிலி எழுத்து *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        எனில் (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        நிறுத்து;
      }
    }
  }
  திரும்பு lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API வெறுமை luaL_setfuncs (lua_State *L, மாறிலி luaL_Reg *l, முழு nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  க்கு (; l->name != வெற்று; l++) {  /* fill the table with given functions */
    எனில் (l->func == வெற்று)  /* placeholder? */
      lua_pushboolean(L, 0);
    இல்லையெனில் {
      முழு i;
      க்கு (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_எடு(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API முழு luaL_getsubtable (lua_State *L, முழு idx, மாறிலி எழுத்து *fname) {
  எனில் (lua_getfield(L, idx, fname) == LUA_TTABLE)
    திரும்பு 1;  /* table already there */
  இல்லையெனில் {
    lua_எடு(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    திரும்பு 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API வெறுமை luaL_requiref (lua_State *L, மாறிலி எழுத்து *modname,
                               lua_CFunction openf, முழு glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  எனில் (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_எடு(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  எனில் (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API வெறுமை luaL_addgsub (luaL_Buffer *b, மாறிலி எழுத்து *s,
                                     மாறிலி எழுத்து *p, மாறிலி எழுத்து *r) {
  மாறிலி எழுத்து *wild;
  size_t l = strlen(p);
  போது ((wild = strstr(s, p)) != வெற்று) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API மாறிலி எழுத்து *luaL_gsub (lua_State *L, மாறிலி எழுத்து *s,
                                  மாறிலி எழுத்து *p, மாறிலி எழுத்து *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  திரும்பு lua_tostring(L, -1);
}


வெறுமை *luaL_alloc (வெறுமை *ud, வெறுமை *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  எனில் (nsize == 0) {
    விடுவி(ptr);
    திரும்பு வெற்று;
  }
  இல்லையெனில்
    திரும்பு realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
நிலையான முழு panic (lua_State *L) {
  மாறிலி எழுத்து *செய்தி = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        செய்தி);
  திரும்பு 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
நிலையான வெறுமை warnfoff (வெறுமை *ud, மாறிலி எழுத்து *message, முழு tocont);
நிலையான வெறுமை warnfon (வெறுமை *ud, மாறிலி எழுத்து *message, முழு tocont);
நிலையான வெறுமை warnfcont (வெறுமை *ud, மாறிலி எழுத்து *message, முழு tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
நிலையான முழு checkcontrol (lua_State *L, மாறிலி எழுத்து *message, முழு tocont) {
  எனில் (tocont || *(message++) != '@')  /* not a control message? */
    திரும்பு 0;
  இல்லையெனில் {
    எனில் (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    இல்லையெனில் எனில் (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    திரும்பு 1;  /* it was a control message */
  }
}


நிலையான வெறுமை warnfoff (வெறுமை *ud, மாறிலி எழுத்து *message, முழு tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
நிலையான வெறுமை warnfcont (வெறுமை *ud, மாறிலி எழுத்து *message, முழு tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  எனில் (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  இல்லையெனில் {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


நிலையான வெறுமை warnfon (வெறுமை *ud, மாறிலி எழுத்து *message, முழு tocont) {
  எனில் (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    திரும்பு;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#எனில் !defined(luai_makeseed)

#அந்தர்பாவ <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(அளவு(வெறுமை*) + அளவு(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + அளவு(முழு) - 1) / அளவு(முழு))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), அளவு(v)), b += அளவு(v))


நிலையான unsigned முழு luai_makeseed (வெறுமை) {
  unsigned முழு buff[BUFSEED];
  unsigned முழு res;
  unsigned முழு i;
  time_t t = time(வெற்று);
  எழுத்து *b = (எழுத்து*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, அளவு(buff) - BUFSEEDB);
  res = buff[0];
  க்கு (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  திரும்பு res;
}

#endif


LUALIB_API unsigned முழு luaL_makeseed (lua_State *L) {
  UNUSED(L);
  திரும்பு luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (வெறுமை) {
  lua_State *L = lua_newstate(luaL_alloc, வெற்று, luaL_makeseed(வெற்று));
  எனில் (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  திரும்பு L;
}


LUALIB_API வெறுமை luaL_checkversion_ (lua_State *L, lua_எண் ver, size_t sz) {
  lua_எண் v = lua_version(L);
  எனில் (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  இல்லையெனில் எனில் (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

