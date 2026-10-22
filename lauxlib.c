/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#تعریف_کرو lauxlib_c
#تعریف_کرو LUA_LIB

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


#تعریف_کرو LEVELS1	10	/* size of the first part of the stack */
#تعریف_کرو LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
ساکن صحیح findfield (lua_State *L, صحیح objidx, صحیح level) {
  اگر (level == 0 || !lua_istable(L, -1))
    واپس 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  جب (lua_اگلا(L, -2)) {  /* for each pair in table */
    اگر (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      اگر (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_نکالو(L, 1);  /* remove value (but keep name) */
        واپس 1;
      }
      ورنہ اگر (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        واپس 1;
      }
    }
    lua_نکالو(L, 1);  /* remove value */
  }
  واپس 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
ساکن صحیح pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  صحیح چوٹی = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_جدول);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  اگر (findfield(L, چوٹی + 1, 2)) {
    ثابت حرف *name = lua_tostring(L, -1);
    اگر (لڑی_موازنہ(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, چوٹی + 1);  /* copy name to proper place */
    lua_settop(L, چوٹی + 1);  /* remove table "loaded" and name copy */
    واپس 1;
  }
  ورنہ {
    lua_settop(L, چوٹی);  /* remove function and global table */
    واپس 0;
  }
}


ساکن خالی pushfuncname (lua_State *L, lua_Debug *ar) {
  اگر (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  ورنہ اگر (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  ورنہ اگر (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  ورنہ اگر (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  ورنہ  /* nothing left... */
    lua_pushliteral(L, "?");
}


ساکن صحیح lastlevel (lua_State *L) {
  lua_Debug ar;
  صحیح li = 1, le = 1;
  /* find an upper bound */
  جب (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  جب (li < le) {
    صحیح m = (li + le)/2;
    اگر (lua_getstack(L, m, &ar)) li = m + 1;
    ورنہ le = m;
  }
  واپس le - 1;
}


LUALIB_API خالی luaL_traceback (lua_State *L, lua_State *L1,
                                ثابت حرف *پیغام, صحیح level) {
  lua_L_بفر b;
  lua_Debug ar;
  صحیح آخری = lastlevel(L1);
  صحیح limit2show = (آخری - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  اگر (پیغام) {
    luaL_addstring(&b, پیغام);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  جب (lua_getstack(L1, level++, &ar)) {
    اگر (limit2show-- == 0) {  /* too many levels? */
      صحیح n = آخری - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    ورنہ {
      lua_getinfo(L1, "Slnt", &ar);
      اگر (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      ورنہ
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

LUALIB_API صحیح luaL_argerror (lua_State *L, صحیح arg, ثابت حرف *extramsg) {
  lua_Debug ar;
  ثابت حرف *argword;
  اگر (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    واپس luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  اگر (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  ورنہ {
    arg -= ar.extraargs;  /* do not count extra arguments */
    اگر (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      اگر (arg == 0)  /* error in self argument? */
        واپس luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  اگر (ar.name == تہی)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  واپس luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API صحیح luaL_typeerror (lua_State *L, صحیح arg, ثابت حرف *tname) {
  ثابت حرف *پیغام;
  ثابت حرف *typearg;  /* name for the type of the actual argument */
  اگر (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  ورنہ اگر (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  ورنہ
    typearg = luaL_typename(L, arg);  /* standard name */
  پیغام = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  واپس luaL_argerror(L, arg, پیغام);
}


ساکن خالی لیبل_error (lua_State *L, صحیح arg, صحیح لیبل) {
  luaL_typeerror(L, arg, lua_typename(L, لیبل));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API خالی luaL_where (lua_State *L, صحیح level) {
  lua_Debug ar;
  اگر (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    اگر (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      واپس;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API صحیح luaL_error (lua_State *L, ثابت حرف *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  واپس lua_error(L);
}


LUALIB_API صحیح luaL_fileresult (lua_State *L, صحیح stat, ثابت حرف *fname) {
  صحیح en = errno;  /* calls to Lua API may change this value */
  اگر (stat) {
    lua_pushboolean(L, 1);
    واپس 1;
  }
  ورنہ {
    ثابت حرف *پیغام;
    luaL_pushfail(L);
    پیغام = (en != 0) ? strerror(en) : "(no extra info)";
    اگر (fname)
      lua_pushfstring(L, "%s: %s", fname, پیغام);
    ورنہ
      lua_pushstring(L, پیغام);
    lua_pushinteger(L, en);
    واپس 3;
  }
}


#اگر !defined(l_inspectstat)	/* { */

#اگر defined(LUA_USE_POSIX)

#شامل <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#تعریف_کرو l_inspectstat(stat,what)  \
   اگر (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   ورنہ اگر (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#ورنہ

#تعریف_کرو l_inspectstat(stat,what)  /* no op */

#اگر_ختم

#اگر_ختم				/* } */


LUALIB_API صحیح luaL_execresult (lua_State *L, صحیح stat) {
  اگر (stat != 0 && errno != 0)  /* error with an 'errno'? */
    واپس luaL_fileresult(L, 0, تہی);
  ورنہ {
    ثابت حرف *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    اگر (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    ورنہ
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    واپس 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API صحیح luaL_newmetatable (lua_State *L, ثابت حرف *tname) {
  اگر (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    واپس 0;  /* leave previous value on top, but return 0 */
  lua_نکالو(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  واپس 1;
}


LUALIB_API خالی luaL_setmetatable (lua_State *L, ثابت حرف *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API خالی *luaL_testudata (lua_State *L, صحیح ud, ثابت حرف *tname) {
  خالی *p = lua_touserdata(L, ud);
  اگر (p != تہی) {  /* value is a userdata? */
    اگر (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      اگر (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = تہی;  /* value is a userdata with wrong metatable */
      lua_نکالو(L, 2);  /* remove both metatables */
      واپس p;
    }
  }
  واپس تہی;  /* value is not a userdata with a metatable */
}


LUALIB_API خالی *luaL_checkudata (lua_State *L, صحیح ud, ثابت حرف *tname) {
  خالی *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != تہی, ud, tname);
  واپس p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API صحیح luaL_checkoption (lua_State *L, صحیح arg, ثابت حرف *def,
                                 ثابت حرف *ثابت lst[]) {
  ثابت حرف *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  صحیح i;
  برائے (i=0; lst[i]; i++)
    اگر (strcmp(lst[i], name) == 0)
      واپس i;
  واپس luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API خالی luaL_checkstack (lua_State *L, صحیح space, ثابت حرف *پیغام) {
  اگر (l_unlikely(!lua_checkstack(L, space))) {
    اگر (پیغام)
      luaL_error(L, "stack overflow (%s)", پیغام);
    ورنہ
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API خالی luaL_checktype (lua_State *L, صحیح arg, صحیح t) {
  اگر (l_unlikely(lua_type(L, arg) != t))
    لیبل_error(L, arg, t);
}


LUALIB_API خالی luaL_checkany (lua_State *L, صحیح arg) {
  اگر (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API ثابت حرف *luaL_checklstring (lua_State *L, صحیح arg, size_t *len) {
  ثابت حرف *s = lua_tolstring(L, arg, len);
  اگر (l_unlikely(!s)) لیبل_error(L, arg, LUA_TSTRING);
  واپس s;
}


LUALIB_API ثابت حرف *luaL_optlstring (lua_State *L, صحیح arg,
                                        ثابت حرف *def, size_t *len) {
  اگر (lua_isnoneornil(L, arg)) {
    اگر (len)
      *len = (def ? لڑی_لمبائی(def) : 0);
    واپس def;
  }
  ورنہ واپس luaL_checklstring(L, arg, len);
}


LUALIB_API lua_نمبر luaL_checknumber (lua_State *L, صحیح arg) {
  صحیح isnum;
  lua_نمبر d = lua_tonumberx(L, arg, &isnum);
  اگر (l_unlikely(!isnum))
    لیبل_error(L, arg, LUA_TNUMBER);
  واپس d;
}


LUALIB_API lua_نمبر luaL_optnumber (lua_State *L, صحیح arg, lua_نمبر def) {
  واپس luaL_opt(L, luaL_checknumber, arg, def);
}


ساکن خالی interror (lua_State *L, صحیح arg) {
  اگر (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  ورنہ
    لیبل_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, صحیح arg) {
  صحیح isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  اگر (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  واپس d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, صحیح arg,
                                                      lua_Integer def) {
  واپس luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
قسمتعریف ساخت UBox {
  خالی *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
ساکن خالی *resizebox (lua_State *L, صحیح idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  اگر (box->bsize == newsize)  /* not changing size? */
    واپس box->box;  /* keep the buffer */
  ورنہ {
    خالی *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    خالی *عارضی = allocf(ud, box->box, box->bsize, newsize);
    اگر (l_unlikely(عارضی == تہی && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = عارضی;
    box->bsize = newsize;
    واپس عارضی;
  }
}


ساکن صحیح boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  واپس 0;
}


ساکن ثابت luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {تہی, تہی}
};


/*
** Get/create metatable (MT) for boxes
*/
ساکن خالی getBoxMT (lua_State *L) {
  ثابت حرف *BOXMT = "_UBOX*"; /* key for the metatable */
  اگر (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


ساکن خالی newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, حجم(UBox), 0);
  box->box = تہی;
  box->bsize = 0;
  getBoxMT(L);
  lua_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#تعریف_کرو buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#تعریف_کرو checkbufferlevel(B,idx)  \
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != تہی  \
                            : lua_touserdata(B->L, idx) == (خالی*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
ساکن size_t newbuffsize (lua_L_بفر *B, size_t sz) {
  size_t newsize = B->سائز;
  اگر (l_unlikely(sz >= MAX_سائز - B->n))
    واپس cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  اگر (newsize <= MAX_سائز/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  اگر (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  واپس newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
ساکن حرف *prepbuffsize (lua_L_بفر *B, size_t sz, صحیح boxidx) {
  checkbufferlevel(B, boxidx);
  اگر (B->سائز - B->n >= sz)  /* enough space? */
    واپس B->b + B->n;
  ورنہ {
    lua_State *L = B->L;
    حرف *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    اگر (buffonstack(B))  /* buffer already has a box? */
      newbuff = (حرف *)resizebox(L, boxidx, newsize);  /* resize it */
    ورنہ {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_داخل_کرو(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (حرف *)resizebox(L, boxidx, newsize);
      حافظہ_نقل(newbuff, B->b, B->n * حجم(حرف));  /* copy original content */
    }
    B->b = newbuff;
    B->سائز = newsize;
    واپس newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API حرف *luaL_prepbuffsize (lua_L_بفر *B, size_t sz) {
  واپس prepbuffsize(B, sz, -1);
}


LUALIB_API خالی luaL_addlstring (lua_L_بفر *B, ثابت حرف *s, size_t l) {
  اگر (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    حرف *b = prepbuffsize(B, l, -1);
    حافظہ_نقل(b, s, l * حجم(حرف));
    luaL_addsize(B, l);
  }
}


LUALIB_API خالی luaL_addstring (lua_L_بفر *B, ثابت حرف *s) {
  luaL_addlstring(B, s, لڑی_لمبائی(s));
}


LUALIB_API خالی luaL_pushresult (lua_L_بفر *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  اگر (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  ورنہ {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    خالی *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    حرف *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (حرف*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = تہی;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API خالی luaL_pushresultsize (lua_L_بفر *B, size_t sz) {
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
LUALIB_API خالی luaL_addvalue (lua_L_بفر *B) {
  lua_State *L = B->L;
  size_t len;
  ثابت حرف *s = lua_tolstring(L, -1, &len);
  حرف *b = prepbuffsize(B, len, -2);
  حافظہ_نقل(b, s, len * حجم(حرف));
  luaL_addsize(B, len);
  lua_نکالو(L, 1);  /* pop string */
}


LUALIB_API خالی luaL_buffinit (lua_State *L, lua_L_بفر *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->سائز = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (خالی*)B);  /* push placeholder */
}


LUALIB_API حرف *luaL_buffinitsize (lua_State *L, lua_L_بفر *B, size_t sz) {
  luaL_buffinit(L, B);
  واپس prepbuffsize(B, sz, -1);
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
    lua_نکالو(L, 1);  /* remove from stack */
    واپس LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  اگر (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (صحیح)lua_tointeger(L, -1);  /* ref = t[1] */
  ورنہ {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_نکالو(L, 1);  /* remove element from stack */
  اگر (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  ورنہ  /* no free elements */
    ref = (صحیح)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  واپس ref;
}


LUALIB_API خالی luaL_unref (lua_State *L, صحیح t, صحیح ref) {
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

قسمتعریف ساخت بھرو_F {
  غیرعلامتی n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  حرف buff[BUFSIZ];  /* area for reading file */
} بھرو_F;


ساکن ثابت حرف *getF (lua_State *L, خالی *ud, size_t *سائز) {
  بھرو_F *lf = (بھرو_F *)ud;
  غیر_مستعمل(L);
  اگر (lf->n > 0) {  /* are there pre-read characters to be read? */
    *سائز = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  ورنہ {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    اگر (feof(lf->f)) واپس تہی;
    *سائز = fread(lf->buff, 1, حجم(lf->buff), lf->f);  /* read block */
  }
  واپس lf->buff;
}


ساکن صحیح errfile (lua_State *L, ثابت حرف *what, صحیح fnameindex) {
  صحیح err = errno;
  ثابت حرف *filename = lua_tostring(L, fnameindex) + 1;
  اگر (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  ورنہ
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  واپس LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
ساکن صحیح skipBOM (FILE *f) {
  صحیح c = getc(f);  /* read first character */
  اگر (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    واپس getc(f);  /* ignore BOM and return next char */
  ورنہ  /* no (valid) BOM */
    واپس c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
ساکن صحیح skipcomment (FILE *f, صحیح *cp) {
  صحیح c = *cp = skipBOM(f);
  اگر (c == '#') {  /* first line is a comment (Unix exec. file)? */
    کرو {  /* skip first line */
      c = getc(f);
    } جب (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    واپس 1;  /* there was a comment */
  }
  ورنہ واپس 0;  /* no comment */
}


LUALIB_API صحیح luaL_loadfilex (lua_State *L, ثابت حرف *filename,
                                             ثابت حرف *منوال) {
  بھرو_F lf;
  صحیح حالت_2, readstatus;
  صحیح c;
  صحیح fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  اگر (filename == تہی) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  ورنہ {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    اگر (lf.f == تہی) واپس errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  اگر (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  اگر (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    اگر (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      اگر (lf.f == تہی) واپس errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  اگر (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  حالت_2 = lua_بھرو(L, getF, &lf, lua_tostring(L, -1), منوال);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  اگر (filename) fclose(lf.f);  /* close file (even in case of errors) */
  اگر (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    واپس errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  واپس حالت_2;
}


قسمتعریف ساخت بھرو_S {
  ثابت حرف *s;
  size_t سائز;
} بھرو_S;


ساکن ثابت حرف *getS (lua_State *L, خالی *ud, size_t *سائز) {
  بھرو_S *ls = (بھرو_S *)ud;
  غیر_مستعمل(L);
  اگر (ls->سائز == 0) واپس تہی;
  *سائز = ls->سائز;
  ls->سائز = 0;
  واپس ls->s;
}


LUALIB_API صحیح luaL_loadbufferx (lua_State *L, ثابت حرف *buff, size_t سائز,
                                 ثابت حرف *name, ثابت حرف *منوال) {
  بھرو_S ls;
  ls.s = buff;
  ls.سائز = سائز;
  واپس lua_بھرو(L, getS, &ls, name, منوال);
}


LUALIB_API صحیح luaL_loadstring (lua_State *L, ثابت حرف *s) {
  واپس luaL_loadbufferx(L, s, لڑی_لمبائی(s), s, "t");
}

/* }====================================================== */



LUALIB_API صحیح luaL_getmetafield (lua_State *L, صحیح obj, ثابت حرف *event) {
  اگر (!lua_getmetatable(L, obj))  /* no metatable? */
    واپس LUA_TNIL;
  ورنہ {
    صحیح tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    اگر (tt == LUA_TNIL)  /* is metafield nil? */
      lua_نکالو(L, 2);  /* remove metatable and metafield */
    ورنہ
      lua_remove(L, -2);  /* remove only metatable */
    واپس tt;  /* return metafield type */
  }
}


LUALIB_API صحیح luaL_callmeta (lua_State *L, صحیح obj, ثابت حرف *event) {
  obj = lua_absindex(L, obj);
  اگر (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    واپس 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  واپس 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, صحیح idx) {
  lua_Integer l;
  صحیح isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  اگر (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_نکالو(L, 1);  /* remove object */
  واپس l;
}


LUALIB_API ثابت حرف *luaL_tolstring (lua_State *L, صحیح idx, size_t *len) {
  idx = lua_absindex(L,idx);
  اگر (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    اگر (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  ورنہ {
    انتخاب (lua_type(L, idx)) {
      حالت LUA_TNUMBER: {
        حرف buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        توڑو;
      }
      حالت LUA_TSTRING:
        lua_pushvalue(L, idx);
        توڑو;
      حالت LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        توڑو;
      حالت LUA_TNIL:
        lua_pushliteral(L, "nil");
        توڑو;
      طےشدہ: {
        صحیح tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        ثابت حرف *قسم = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", قسم, lua_topointer(L, idx));
        اگر (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        توڑو;
      }
    }
  }
  واپس lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API خالی luaL_setfuncs (lua_State *L, ثابت luaL_Reg *l, صحیح nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  برائے (; l->name != تہی; l++) {  /* fill the table with given functions */
    اگر (l->func == تہی)  /* placeholder? */
      lua_pushboolean(L, 0);
    ورنہ {
      صحیح i;
      برائے (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_نکالو(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API صحیح luaL_getsubtable (lua_State *L, صحیح idx, ثابت حرف *fname) {
  اگر (lua_getfield(L, idx, fname) == LUA_TTABLE)
    واپس 1;  /* table already there */
  ورنہ {
    lua_نکالو(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    واپس 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API خالی luaL_requiref (lua_State *L, ثابت حرف *modname,
                               lua_CFunction openf, صحیح glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_جدول);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  اگر (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_نکالو(L, 1);  /* remove field */
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


LUALIB_API خالی luaL_addgsub (lua_L_بفر *b, ثابت حرف *s,
                                     ثابت حرف *p, ثابت حرف *r) {
  ثابت حرف *wild;
  size_t l = لڑی_لمبائی(p);
  جب ((wild = لڑی_تلاش(s, p)) != تہی) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API ثابت حرف *luaL_gsub (lua_State *L, ثابت حرف *s,
                                  ثابت حرف *p, ثابت حرف *r) {
  lua_L_بفر b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  واپس lua_tostring(L, -1);
}


خالی *luaL_alloc (خالی *ud, خالی *ptr, size_t osize, size_t nsize) {
  غیر_مستعمل(ud); غیر_مستعمل(osize);
  اگر (nsize == 0) {
    آزاد(ptr);
    واپس تہی;
  }
  ورنہ
    واپس realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
ساکن صحیح panic (lua_State *L) {
  ثابت حرف *پیغام = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        پیغام);
  واپس 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
ساکن خالی warnfoff (خالی *ud, ثابت حرف *پیغام_2, صحیح tocont);
ساکن خالی warnfon (خالی *ud, ثابت حرف *پیغام_2, صحیح tocont);
ساکن خالی warnfcont (خالی *ud, ثابت حرف *پیغام_2, صحیح tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
ساکن صحیح checkcontrol (lua_State *L, ثابت حرف *پیغام_2, صحیح tocont) {
  اگر (tocont || *(پیغام_2++) != '@')  /* not a control message? */
    واپس 0;
  ورنہ {
    اگر (strcmp(پیغام_2, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    ورنہ اگر (strcmp(پیغام_2, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    واپس 1;  /* it was a control message */
  }
}


ساکن خالی warnfoff (خالی *ud, ثابت حرف *پیغام_2, صحیح tocont) {
  checkcontrol((lua_State *)ud, پیغام_2, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
ساکن خالی warnfcont (خالی *ud, ثابت حرف *پیغام_2, صحیح tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", پیغام_2);  /* write message */
  اگر (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  ورنہ {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


ساکن خالی warnfon (خالی *ud, ثابت حرف *پیغام_2, صحیح tocont) {
  اگر (checkcontrol((lua_State *)ud, پیغام_2, tocont))  /* control message? */
    واپس;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, پیغام_2, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#اگر !defined(luai_makeseed)

#شامل <time.h>


/* Size for the buffer, in bytes */
#تعریف_کرو BUFSEEDB	(حجم(خالی*) + حجم(time_t))

/* Size for the buffer in int's, rounded up */
#تعریف_کرو BUFSEED		((BUFSEEDB + حجم(صحیح) - 1) / حجم(صحیح))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#تعریف_کرو addbuff(b,v)	(حافظہ_نقل(&b[0], &(v), حجم(v)), b += حجم(v))


ساکن غیرعلامتی صحیح luai_makeseed (خالی) {
  غیرعلامتی صحیح buff[BUFSEED];
  غیرعلامتی صحیح res;
  غیرعلامتی صحیح i;
  time_t t = time(تہی);
  حرف *b = (حرف*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /*وقت*/
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, حجم(buff) - BUFSEEDB);
  res = buff[0];
  برائے (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  واپس res;
}

#اگر_ختم


LUALIB_API غیرعلامتی صحیح luaL_makeseed (lua_State *L) {
  غیر_مستعمل(L);
  واپس luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (خالی) {
  lua_State *L = lua_newstate(luaL_alloc, تہی, luaL_makeseed(تہی));
  اگر (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  واپس L;
}


LUALIB_API خالی luaL_checkversion_ (lua_State *L, lua_نمبر ver, size_t sz) {
  lua_نمبر v = lua_نسخہ(L);
  اگر (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  ورنہ اگر (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

