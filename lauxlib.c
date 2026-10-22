/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#define lauxlib_c
#define LUA_LIB

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "llimits.h"


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
સ્થિર પૂર્ણાંક findfield (lua_State *L, પૂર્ણાંક objidx, પૂર્ણાંક level) {
  જો (level == 0 || !lua_istable(L, -1))
    પાછા 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  જ્યારે (lua_next(L, -2)) {  /* for each pair in table */
    જો (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      જો (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_કાઢો(L, 1);  /* remove value (but keep name) */
        પાછા 1;
      }
      નહીંતર જો (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        પાછા 1;
      }
    }
    lua_કાઢો(L, 1);  /* remove value */
  }
  પાછા 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
સ્થિર પૂર્ણાંક pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  પૂર્ણાંક ટોચ = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  જો (findfield(L, ટોચ + 1, 2)) {
    અચલ અક્ષર *name = lua_tostring(L, -1);
    જો (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, ટોચ + 1);  /* copy name to proper place */
    lua_settop(L, ટોચ + 1);  /* remove table "loaded" and name copy */
    પાછા 1;
  }
  નહીંતર {
    lua_settop(L, ટોચ);  /* remove function and global table */
    પાછા 0;
  }
}


સ્થિર શૂન્ય pushfuncname (lua_State *L, lua_Debug *ar) {
  જો (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  નહીંતર જો (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  નહીંતર જો (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  નહીંતર જો (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  નહીંતર  /* nothing left... */
    lua_pushliteral(L, "?");
}


સ્થિર પૂર્ણાંક lastlevel (lua_State *L) {
  lua_Debug ar;
  પૂર્ણાંક li = 1, le = 1;
  /* find an upper bound */
  જ્યારે (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  જ્યારે (li < le) {
    પૂર્ણાંક m = (li + le)/2;
    જો (lua_getstack(L, m, &ar)) li = m + 1;
    નહીંતર le = m;
  }
  પાછા le - 1;
}


LUALIB_API શૂન્ય luaL_traceback (lua_State *L, lua_State *L1,
                                અચલ અક્ષર *સંદેશ, પૂર્ણાંક level) {
  luaL_Buffer b;
  lua_Debug ar;
  પૂર્ણાંક last = lastlevel(L1);
  પૂર્ણાંક limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  જો (સંદેશ) {
    luaL_addstring(&b, સંદેશ);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  જ્યારે (lua_getstack(L1, level++, &ar)) {
    જો (limit2show-- == 0) {  /* too many levels? */
      પૂર્ણાંક n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    નહીંતર {
      lua_getinfo(L1, "Slnt", &ar);
      જો (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      નહીંતર
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      જો (ar.istailcall)
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

LUALIB_API પૂર્ણાંક luaL_argerror (lua_State *L, પૂર્ણાંક arg, અચલ અક્ષર *extramsg) {
  lua_Debug ar;
  અચલ અક્ષર *argword;
  જો (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    પાછા luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  જો (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  નહીંતર {
    arg -= ar.extraargs;  /* do not count extra arguments */
    જો (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      જો (arg == 0)  /* error in self argument? */
        પાછા luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  જો (ar.name == ખાલી)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  પાછા luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API પૂર્ણાંક luaL_typeerror (lua_State *L, પૂર્ણાંક arg, અચલ અક્ષર *tname) {
  અચલ અક્ષર *સંદેશ;
  અચલ અક્ષર *typearg;  /* name for the type of the actual argument */
  જો (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  નહીંતર જો (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  નહીંતર
    typearg = luaL_typename(L, arg);  /* standard name */
  સંદેશ = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  પાછા luaL_argerror(L, arg, સંદેશ);
}


સ્થિર શૂન્ય tag_error (lua_State *L, પૂર્ણાંક arg, પૂર્ણાંક tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API શૂન્ય luaL_where (lua_State *L, પૂર્ણાંક level) {
  lua_Debug ar;
  જો (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    જો (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      પાછા;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API પૂર્ણાંક luaL_error (lua_State *L, અચલ અક્ષર *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  પાછા lua_error(L);
}


LUALIB_API પૂર્ણાંક luaL_fileresult (lua_State *L, પૂર્ણાંક stat, અચલ અક્ષર *fname) {
  પૂર્ણાંક en = errno;  /* calls to Lua API may change this value */
  જો (stat) {
    lua_pushboolean(L, 1);
    પાછા 1;
  }
  નહીંતર {
    અચલ અક્ષર *સંદેશ;
    luaL_pushfail(L);
    સંદેશ = (en != 0) ? strerror(en) : "(no extra info)";
    જો (fname)
      lua_pushfstring(L, "%s: %s", fname, સંદેશ);
    નહીંતર
      lua_pushstring(L, સંદેશ);
    lua_pushinteger(L, en);
    પાછા 3;
  }
}


#જો !defined(l_inspectstat)	/* { */

#જો defined(LUA_USE_POSIX)

#અન્તર્ભાવ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   જો (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   નહીંતર જો (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#નહીંતર

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API પૂર્ણાંક luaL_execresult (lua_State *L, પૂર્ણાંક stat) {
  જો (stat != 0 && errno != 0)  /* error with an 'errno'? */
    પાછા luaL_fileresult(L, 0, ખાલી);
  નહીંતર {
    અચલ અક્ષર *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    જો (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    નહીંતર
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    પાછા 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API પૂર્ણાંક luaL_newmetatable (lua_State *L, અચલ અક્ષર *tname) {
  જો (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    પાછા 0;  /* leave previous value on top, but return 0 */
  lua_કાઢો(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  પાછા 1;
}


LUALIB_API શૂન્ય luaL_setmetatable (lua_State *L, અચલ અક્ષર *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API શૂન્ય *luaL_testudata (lua_State *L, પૂર્ણાંક ud, અચલ અક્ષર *tname) {
  શૂન્ય *p = lua_touserdata(L, ud);
  જો (p != ખાલી) {  /* value is a userdata? */
    જો (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      જો (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ખાલી;  /* value is a userdata with wrong metatable */
      lua_કાઢો(L, 2);  /* remove both metatables */
      પાછા p;
    }
  }
  પાછા ખાલી;  /* value is not a userdata with a metatable */
}


LUALIB_API શૂન્ય *luaL_checkudata (lua_State *L, પૂર્ણાંક ud, અચલ અક્ષર *tname) {
  શૂન્ય *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ખાલી, ud, tname);
  પાછા p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API પૂર્ણાંક luaL_checkoption (lua_State *L, પૂર્ણાંક arg, અચલ અક્ષર *def,
                                 અચલ અક્ષર *અચલ lst[]) {
  અચલ અક્ષર *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  પૂર્ણાંક i;
  માટે (i=0; lst[i]; i++)
    જો (strcmp(lst[i], name) == 0)
      પાછા i;
  પાછા luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API શૂન્ય luaL_checkstack (lua_State *L, પૂર્ણાંક space, અચલ અક્ષર *સંદેશ) {
  જો (l_unlikely(!lua_checkstack(L, space))) {
    જો (સંદેશ)
      luaL_error(L, "stack overflow (%s)", સંદેશ);
    નહીંતર
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API શૂન્ય luaL_checktype (lua_State *L, પૂર્ણાંક arg, પૂર્ણાંક t) {
  જો (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API શૂન્ય luaL_checkany (lua_State *L, પૂર્ણાંક arg) {
  જો (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API અચલ અક્ષર *luaL_checklstring (lua_State *L, પૂર્ણાંક arg, size_t *len) {
  અચલ અક્ષર *s = lua_tolstring(L, arg, len);
  જો (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  પાછા s;
}


LUALIB_API અચલ અક્ષર *luaL_optlstring (lua_State *L, પૂર્ણાંક arg,
                                        અચલ અક્ષર *def, size_t *len) {
  જો (lua_isnoneornil(L, arg)) {
    જો (len)
      *len = (def ? strlen(def) : 0);
    પાછા def;
  }
  નહીંતર પાછા luaL_checklstring(L, arg, len);
}


LUALIB_API lua_સંખ્યા luaL_checknumber (lua_State *L, પૂર્ણાંક arg) {
  પૂર્ણાંક isnum;
  lua_સંખ્યા d = lua_tonumberx(L, arg, &isnum);
  જો (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  પાછા d;
}


LUALIB_API lua_સંખ્યા luaL_optnumber (lua_State *L, પૂર્ણાંક arg, lua_સંખ્યા def) {
  પાછા luaL_opt(L, luaL_checknumber, arg, def);
}


સ્થિર શૂન્ય interror (lua_State *L, પૂર્ણાંક arg) {
  જો (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  નહીંતર
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, પૂર્ણાંક arg) {
  પૂર્ણાંક isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  જો (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  પાછા d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, પૂર્ણાંક arg,
                                                      lua_Integer def) {
  પાછા luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
પ્રકારવ્યાખ્યા સંરચના UBox {
  શૂન્ય *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
સ્થિર શૂન્ય *resizebox (lua_State *L, પૂર્ણાંક idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  જો (box->bsize == newsize)  /* not changing size? */
    પાછા box->box;  /* keep the buffer */
  નહીંતર {
    શૂન્ય *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    શૂન્ય *temp = allocf(ud, box->box, box->bsize, newsize);
    જો (l_unlikely(temp == ખાલી && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = temp;
    box->bsize = newsize;
    પાછા temp;
  }
}


સ્થિર પૂર્ણાંક boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  પાછા 0;
}


સ્થિર અચલ luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ખાલી, ખાલી}
};


/*
** Get/create metatable (MT) for boxes
*/
સ્થિર શૂન્ય getBoxMT (lua_State *L) {
  અચલ અક્ષર *BOXMT = "_UBOX*"; /* key for the metatable */
  જો (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


સ્થિર શૂન્ય newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, કદ(UBox), 0);
  box->box = ખાલી;
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
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ખાલી  \
                            : lua_touserdata(B->L, idx) == (શૂન્ય*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
સ્થિર size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = B->size;
  જો (l_unlikely(sz >= MAX_SIZE - B->n))
    પાછા cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  જો (newsize <= MAX_SIZE/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  જો (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  પાછા newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
સ્થિર અક્ષર *prepbuffsize (luaL_Buffer *B, size_t sz, પૂર્ણાંક boxidx) {
  checkbufferlevel(B, boxidx);
  જો (B->size - B->n >= sz)  /* enough space? */
    પાછા B->b + B->n;
  નહીંતર {
    lua_State *L = B->L;
    અક્ષર *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    જો (buffonstack(B))  /* buffer already has a box? */
      newbuff = (અક્ષર *)resizebox(L, boxidx, newsize);  /* resize it */
    નહીંતર {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_ઉમેરો(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (અક્ષર *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * કદ(અક્ષર));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    પાછા newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API અક્ષર *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  પાછા prepbuffsize(B, sz, -1);
}


LUALIB_API શૂન્ય luaL_addlstring (luaL_Buffer *B, અચલ અક્ષર *s, size_t l) {
  જો (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    અક્ષર *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * કદ(અક્ષર));
    luaL_addsize(B, l);
  }
}


LUALIB_API શૂન્ય luaL_addstring (luaL_Buffer *B, અચલ અક્ષર *s) {
  luaL_addlstring(B, s, strlen(s));
}


LUALIB_API શૂન્ય luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  જો (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  નહીંતર {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    શૂન્ય *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    અક્ષર *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (અક્ષર*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ખાલી;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API શૂન્ય luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
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
LUALIB_API શૂન્ય luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  અચલ અક્ષર *s = lua_tolstring(L, -1, &len);
  અક્ષર *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * કદ(અક્ષર));
  luaL_addsize(B, len);
  lua_કાઢો(L, 1);  /* pop string */
}


LUALIB_API શૂન્ય luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (શૂન્ય*)B);  /* push placeholder */
}


LUALIB_API અક્ષર *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  પાછા prepbuffsize(B, sz, -1);
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
LUALIB_API પૂર્ણાંક luaL_ref (lua_State *L, પૂર્ણાંક t) {
  પૂર્ણાંક ref;
  જો (lua_isnil(L, -1)) {
    lua_કાઢો(L, 1);  /* remove from stack */
    પાછા LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  જો (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (પૂર્ણાંક)lua_tointeger(L, -1);  /* ref = t[1] */
  નહીંતર {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_કાઢો(L, 1);  /* remove element from stack */
  જો (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  નહીંતર  /* no free elements */
    ref = (પૂર્ણાંક)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  પાછા ref;
}


LUALIB_API શૂન્ય luaL_unref (lua_State *L, પૂર્ણાંક t, પૂર્ણાંક ref) {
  જો (ref >= 0) {
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

પ્રકારવ્યાખ્યા સંરચના LoadF {
  unsigned n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  અક્ષર buff[BUFSIZ];  /* area for reading file */
} LoadF;


સ્થિર અચલ અક્ષર *getF (lua_State *L, શૂન્ય *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  જો (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  નહીંતર {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    જો (feof(lf->f)) પાછા ખાલી;
    *size = fread(lf->buff, 1, કદ(lf->buff), lf->f);  /* read block */
  }
  પાછા lf->buff;
}


સ્થિર પૂર્ણાંક errfile (lua_State *L, અચલ અક્ષર *what, પૂર્ણાંક fnameindex) {
  પૂર્ણાંક err = errno;
  અચલ અક્ષર *filename = lua_tostring(L, fnameindex) + 1;
  જો (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  નહીંતર
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  પાછા LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
સ્થિર પૂર્ણાંક skipBOM (FILE *f) {
  પૂર્ણાંક c = getc(f);  /* read first character */
  જો (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    પાછા getc(f);  /* ignore BOM and return next char */
  નહીંતર  /* no (valid) BOM */
    પાછા c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
સ્થિર પૂર્ણાંક skipcomment (FILE *f, પૂર્ણાંક *cp) {
  પૂર્ણાંક c = *cp = skipBOM(f);
  જો (c == '#') {  /* first line is a comment (Unix exec. file)? */
    કરો {  /* skip first line */
      c = getc(f);
    } જ્યારે (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    પાછા 1;  /* there was a comment */
  }
  નહીંતર પાછા 0;  /* no comment */
}


LUALIB_API પૂર્ણાંક luaL_loadfilex (lua_State *L, અચલ અક્ષર *filename,
                                             અચલ અક્ષર *mode) {
  LoadF lf;
  પૂર્ણાંક status, readstatus;
  પૂર્ણાંક c;
  પૂર્ણાંક fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  જો (filename == ખાલી) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  નહીંતર {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    જો (lf.f == ખાલી) પાછા errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  જો (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  જો (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    જો (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      જો (lf.f == ખાલી) પાછા errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  જો (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  જો (filename) fclose(lf.f);  /* close file (even in case of errors) */
  જો (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    પાછા errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  પાછા status;
}


પ્રકારવ્યાખ્યા સંરચના LoadS {
  અચલ અક્ષર *s;
  size_t size;
} LoadS;


સ્થિર અચલ અક્ષર *getS (lua_State *L, શૂન્ય *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  જો (ls->size == 0) પાછા ખાલી;
  *size = ls->size;
  ls->size = 0;
  પાછા ls->s;
}


LUALIB_API પૂર્ણાંક luaL_loadbufferx (lua_State *L, અચલ અક્ષર *buff, size_t size,
                                 અચલ અક્ષર *name, અચલ અક્ષર *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  પાછા lua_load(L, getS, &ls, name, mode);
}


LUALIB_API પૂર્ણાંક luaL_loadstring (lua_State *L, અચલ અક્ષર *s) {
  પાછા luaL_loadbufferx(L, s, strlen(s), s, "t");
}

/* }====================================================== */



LUALIB_API પૂર્ણાંક luaL_getmetafield (lua_State *L, પૂર્ણાંક obj, અચલ અક્ષર *event) {
  જો (!lua_getmetatable(L, obj))  /* no metatable? */
    પાછા LUA_TNIL;
  નહીંતર {
    પૂર્ણાંક tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    જો (tt == LUA_TNIL)  /* is metafield nil? */
      lua_કાઢો(L, 2);  /* remove metatable and metafield */
    નહીંતર
      lua_remove(L, -2);  /* remove only metatable */
    પાછા tt;  /* return metafield type */
  }
}


LUALIB_API પૂર્ણાંક luaL_callmeta (lua_State *L, પૂર્ણાંક obj, અચલ અક્ષર *event) {
  obj = lua_absindex(L, obj);
  જો (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    પાછા 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  પાછા 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, પૂર્ણાંક idx) {
  lua_Integer l;
  પૂર્ણાંક isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  જો (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_કાઢો(L, 1);  /* remove object */
  પાછા l;
}


LUALIB_API અચલ અક્ષર *luaL_tolstring (lua_State *L, પૂર્ણાંક idx, size_t *len) {
  idx = lua_absindex(L,idx);
  જો (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    જો (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  નહીંતર {
    પસંદગી (lua_type(L, idx)) {
      સ્થિતિ LUA_TNUMBER: {
        અક્ષર buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        તોડો;
      }
      સ્થિતિ LUA_TSTRING:
        lua_pushvalue(L, idx);
        તોડો;
      સ્થિતિ LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        તોડો;
      સ્થિતિ LUA_TNIL:
        lua_pushliteral(L, "nil");
        તોડો;
      મૂળભૂત: {
        પૂર્ણાંક tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        અચલ અક્ષર *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        જો (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        તોડો;
      }
    }
  }
  પાછા lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API શૂન્ય luaL_setfuncs (lua_State *L, અચલ luaL_Reg *l, પૂર્ણાંક nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  માટે (; l->name != ખાલી; l++) {  /* fill the table with given functions */
    જો (l->func == ખાલી)  /* placeholder? */
      lua_pushboolean(L, 0);
    નહીંતર {
      પૂર્ણાંક i;
      માટે (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_કાઢો(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API પૂર્ણાંક luaL_getsubtable (lua_State *L, પૂર્ણાંક idx, અચલ અક્ષર *fname) {
  જો (lua_getfield(L, idx, fname) == LUA_TTABLE)
    પાછા 1;  /* table already there */
  નહીંતર {
    lua_કાઢો(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    પાછા 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API શૂન્ય luaL_requiref (lua_State *L, અચલ અક્ષર *modname,
                               lua_CFunction openf, પૂર્ણાંક glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  જો (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_કાઢો(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  જો (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API શૂન્ય luaL_addgsub (luaL_Buffer *b, અચલ અક્ષર *s,
                                     અચલ અક્ષર *p, અચલ અક્ષર *r) {
  અચલ અક્ષર *wild;
  size_t l = strlen(p);
  જ્યારે ((wild = strstr(s, p)) != ખાલી) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API અચલ અક્ષર *luaL_gsub (lua_State *L, અચલ અક્ષર *s,
                                  અચલ અક્ષર *p, અચલ અક્ષર *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  પાછા lua_tostring(L, -1);
}


શૂન્ય *luaL_alloc (શૂન્ય *ud, શૂન્ય *ptr, size_t osize, size_t nsize) {
  UNUSED(ud); UNUSED(osize);
  જો (nsize == 0) {
    મુક્ત(ptr);
    પાછા ખાલી;
  }
  નહીંતર
    પાછા realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
સ્થિર પૂર્ણાંક panic (lua_State *L) {
  અચલ અક્ષર *સંદેશ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        સંદેશ);
  પાછા 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
સ્થિર શૂન્ય warnfoff (શૂન્ય *ud, અચલ અક્ષર *message, પૂર્ણાંક tocont);
સ્થિર શૂન્ય warnfon (શૂન્ય *ud, અચલ અક્ષર *message, પૂર્ણાંક tocont);
સ્થિર શૂન્ય warnfcont (શૂન્ય *ud, અચલ અક્ષર *message, પૂર્ણાંક tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
સ્થિર પૂર્ણાંક checkcontrol (lua_State *L, અચલ અક્ષર *message, પૂર્ણાંક tocont) {
  જો (tocont || *(message++) != '@')  /* not a control message? */
    પાછા 0;
  નહીંતર {
    જો (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    નહીંતર જો (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    પાછા 1;  /* it was a control message */
  }
}


સ્થિર શૂન્ય warnfoff (શૂન્ય *ud, અચલ અક્ષર *message, પૂર્ણાંક tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
સ્થિર શૂન્ય warnfcont (શૂન્ય *ud, અચલ અક્ષર *message, પૂર્ણાંક tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  જો (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  નહીંતર {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


સ્થિર શૂન્ય warnfon (શૂન્ય *ud, અચલ અક્ષર *message, પૂર્ણાંક tocont) {
  જો (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    પાછા;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#જો !defined(luai_makeseed)

#અન્તર્ભાવ <time.h>


/* Size for the buffer, in bytes */
#define BUFSEEDB	(કદ(શૂન્ય*) + કદ(time_t))

/* Size for the buffer in int's, rounded up */
#define BUFSEED		((BUFSEEDB + કદ(પૂર્ણાંક) - 1) / કદ(પૂર્ણાંક))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#define addbuff(b,v)	(memcpy(&b[0], &(v), કદ(v)), b += કદ(v))


સ્થિર unsigned પૂર્ણાંક luai_makeseed (શૂન્ય) {
  unsigned પૂર્ણાંક buff[BUFSEED];
  unsigned પૂર્ણાંક res;
  unsigned પૂર્ણાંક i;
  time_t t = time(ખાલી);
  અક્ષર *b = (અક્ષર*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, કદ(buff) - BUFSEEDB);
  res = buff[0];
  માટે (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  પાછા res;
}

#endif


LUALIB_API unsigned પૂર્ણાંક luaL_makeseed (lua_State *L) {
  UNUSED(L);
  પાછા luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (શૂન્ય) {
  lua_State *L = lua_newstate(luaL_alloc, ખાલી, luaL_makeseed(ખાલી));
  જો (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  પાછા L;
}


LUALIB_API શૂન્ય luaL_checkversion_ (lua_State *L, lua_સંખ્યા ver, size_t sz) {
  lua_સંખ્યા v = lua_version(L);
  જો (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  નહીંતર જો (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

