/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#నిర్వచించు lauxlib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "llimits.h"


/*
** {======================================================
** Traceback
** =======================================================
*/


#నిర్వచించు LEVELS1	10	/* size of the first part of the stack */
#నిర్వచించు LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
స్థిర పూర్ణాంకం findfield (lua_State *L, పూర్ణాంకం objidx, పూర్ణాంకం level) {
  ఒకవేళ (level == 0 || !lua_istable(L, -1))
    తిరిగి 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  అయితే (lua_తదుపరి(L, -2)) {  /* for each pair in table */
    ఒకవేళ (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      ఒకవేళ (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_తీయి(L, 1);  /* remove value (but keep name) */
        తిరిగి 1;
      }
      లేకపోతే ఒకవేళ (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        తిరిగి 1;
      }
    }
    lua_తీయి(L, 1);  /* remove value */
  }
  తిరిగి 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
స్థిర పూర్ణాంకం pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  పూర్ణాంకం శిఖరం = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_పట్టిక);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  ఒకవేళ (findfield(L, శిఖరం + 1, 2)) {
    స్థిరరాశి అక్షరం *name = lua_tostring(L, -1);
    ఒకవేళ (అక్షరశ్రేణి_పోలిక(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, శిఖరం + 1);  /* copy name to proper place */
    lua_settop(L, శిఖరం + 1);  /* remove table "loaded" and name copy */
    తిరిగి 1;
  }
  లేకపోతే {
    lua_settop(L, శిఖరం);  /* remove function and global table */
    తిరిగి 0;
  }
}


స్థిర శూన్య pushfuncname (lua_State *L, lua_Debug *ar) {
  ఒకవేళ (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  లేకపోతే ఒకవేళ (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  లేకపోతే ఒకవేళ (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  లేకపోతే ఒకవేళ (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  లేకపోతే  /* nothing left... */
    lua_pushliteral(L, "?");
}


స్థిర పూర్ణాంకం lastlevel (lua_State *L) {
  lua_Debug ar;
  పూర్ణాంకం li = 1, le = 1;
  /* find an upper bound */
  అయితే (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  అయితే (li < le) {
    పూర్ణాంకం m = (li + le)/2;
    ఒకవేళ (lua_getstack(L, m, &ar)) li = m + 1;
    లేకపోతే le = m;
  }
  తిరిగి le - 1;
}


LUALIB_API శూన్య luaL_traceback (lua_State *L, lua_State *L1,
                                స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం level) {
  lua_L_బఫరు b;
  lua_Debug ar;
  పూర్ణాంకం చివరి = lastlevel(L1);
  పూర్ణాంకం limit2show = (చివరి - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  ఒకవేళ (సందేశం) {
    luaL_addstring(&b, సందేశం);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  అయితే (lua_getstack(L1, level++, &ar)) {
    ఒకవేళ (limit2show-- == 0) {  /* too many levels? */
      పూర్ణాంకం n = చివరి - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    లేకపోతే {
      lua_getinfo(L1, "Slnt", &ar);
      ఒకవేళ (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      లేకపోతే
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      ఒకవేళ (ar.istailcall)
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

LUALIB_API పూర్ణాంకం luaL_argerror (lua_State *L, పూర్ణాంకం arg, స్థిరరాశి అక్షరం *extramsg) {
  lua_Debug ar;
  స్థిరరాశి అక్షరం *argword;
  ఒకవేళ (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    తిరిగి luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  ఒకవేళ (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  లేకపోతే {
    arg -= ar.extraargs;  /* do not count extra arguments */
    ఒకవేళ (strcmp(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      ఒకవేళ (arg == 0)  /* error in self argument? */
        తిరిగి luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  ఒకవేళ (ar.name == ఖాళీ)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  తిరిగి luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API పూర్ణాంకం luaL_typeerror (lua_State *L, పూర్ణాంకం arg, స్థిరరాశి అక్షరం *tname) {
  స్థిరరాశి అక్షరం *సందేశం;
  స్థిరరాశి అక్షరం *typearg;  /* name for the type of the actual argument */
  ఒకవేళ (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  లేకపోతే ఒకవేళ (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  లేకపోతే
    typearg = luaL_typename(L, arg);  /* standard name */
  సందేశం = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  తిరిగి luaL_argerror(L, arg, సందేశం);
}


స్థిర శూన్య ట్యాగ్_error (lua_State *L, పూర్ణాంకం arg, పూర్ణాంకం ట్యాగ్) {
  luaL_typeerror(L, arg, lua_typename(L, ట్యాగ్));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API శూన్య luaL_where (lua_State *L, పూర్ణాంకం level) {
  lua_Debug ar;
  ఒకవేళ (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    ఒకవేళ (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      తిరిగి;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API పూర్ణాంకం luaL_error (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  తిరిగి lua_error(L);
}


LUALIB_API పూర్ణాంకం luaL_fileresult (lua_State *L, పూర్ణాంకం stat, స్థిరరాశి అక్షరం *fname) {
  పూర్ణాంకం en = errno;  /* calls to Lua API may change this value */
  ఒకవేళ (stat) {
    lua_pushboolean(L, 1);
    తిరిగి 1;
  }
  లేకపోతే {
    స్థిరరాశి అక్షరం *సందేశం;
    luaL_pushfail(L);
    సందేశం = (en != 0) ? strerror(en) : "(no extra info)";
    ఒకవేళ (fname)
      lua_pushfstring(L, "%s: %s", fname, సందేశం);
    లేకపోతే
      lua_pushstring(L, సందేశం);
    lua_pushinteger(L, en);
    తిరిగి 3;
  }
}


#ఒకవేళ !defined(l_inspectstat)	/* { */

#ఒకవేళ defined(LUA_USE_POSIX)

#అన్తర్భావ <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#నిర్వచించు l_inspectstat(stat,what)  \
   ఒకవేళ (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   లేకపోతే ఒకవేళ (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#లేకపోతే

#నిర్వచించు l_inspectstat(stat,what)  /* no op */

#అయితే_ముగింపు

#అయితే_ముగింపు				/* } */


LUALIB_API పూర్ణాంకం luaL_execresult (lua_State *L, పూర్ణాంకం stat) {
  ఒకవేళ (stat != 0 && errno != 0)  /* error with an 'errno'? */
    తిరిగి luaL_fileresult(L, 0, ఖాళీ);
  లేకపోతే {
    స్థిరరాశి అక్షరం *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    ఒకవేళ (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    లేకపోతే
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    తిరిగి 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API పూర్ణాంకం luaL_newmetatable (lua_State *L, స్థిరరాశి అక్షరం *tname) {
  ఒకవేళ (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    తిరిగి 0;  /* leave previous value on top, but return 0 */
  lua_తీయి(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  తిరిగి 1;
}


LUALIB_API శూన్య luaL_setmetatable (lua_State *L, స్థిరరాశి అక్షరం *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API శూన్య *luaL_testudata (lua_State *L, పూర్ణాంకం ud, స్థిరరాశి అక్షరం *tname) {
  శూన్య *p = lua_touserdata(L, ud);
  ఒకవేళ (p != ఖాళీ) {  /* value is a userdata? */
    ఒకవేళ (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      ఒకవేళ (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = ఖాళీ;  /* value is a userdata with wrong metatable */
      lua_తీయి(L, 2);  /* remove both metatables */
      తిరిగి p;
    }
  }
  తిరిగి ఖాళీ;  /* value is not a userdata with a metatable */
}


LUALIB_API శూన్య *luaL_checkudata (lua_State *L, పూర్ణాంకం ud, స్థిరరాశి అక్షరం *tname) {
  శూన్య *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != ఖాళీ, ud, tname);
  తిరిగి p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API పూర్ణాంకం luaL_checkoption (lua_State *L, పూర్ణాంకం arg, స్థిరరాశి అక్షరం *def,
                                 స్థిరరాశి అక్షరం *స్థిరరాశి lst[]) {
  స్థిరరాశి అక్షరం *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  పూర్ణాంకం i;
  కొరకు (i=0; lst[i]; i++)
    ఒకవేళ (strcmp(lst[i], name) == 0)
      తిరిగి i;
  తిరిగి luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API శూన్య luaL_checkstack (lua_State *L, పూర్ణాంకం space, స్థిరరాశి అక్షరం *సందేశం) {
  ఒకవేళ (l_unlikely(!lua_checkstack(L, space))) {
    ఒకవేళ (సందేశం)
      luaL_error(L, "stack overflow (%s)", సందేశం);
    లేకపోతే
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API శూన్య luaL_checktype (lua_State *L, పూర్ణాంకం arg, పూర్ణాంకం t) {
  ఒకవేళ (l_unlikely(lua_type(L, arg) != t))
    ట్యాగ్_error(L, arg, t);
}


LUALIB_API శూన్య luaL_checkany (lua_State *L, పూర్ణాంకం arg) {
  ఒకవేళ (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API స్థిరరాశి అక్షరం *luaL_checklstring (lua_State *L, పూర్ణాంకం arg, size_t *len) {
  స్థిరరాశి అక్షరం *s = lua_tolstring(L, arg, len);
  ఒకవేళ (l_unlikely(!s)) ట్యాగ్_error(L, arg, LUA_TSTRING);
  తిరిగి s;
}


LUALIB_API స్థిరరాశి అక్షరం *luaL_optlstring (lua_State *L, పూర్ణాంకం arg,
                                        స్థిరరాశి అక్షరం *def, size_t *len) {
  ఒకవేళ (lua_isnoneornil(L, arg)) {
    ఒకవేళ (len)
      *len = (def ? అక్షరశ్రేణి_పొడవు(def) : 0);
    తిరిగి def;
  }
  లేకపోతే తిరిగి luaL_checklstring(L, arg, len);
}


LUALIB_API lua_సంఖ్య luaL_checknumber (lua_State *L, పూర్ణాంకం arg) {
  పూర్ణాంకం isnum;
  lua_సంఖ్య d = lua_tonumberx(L, arg, &isnum);
  ఒకవేళ (l_unlikely(!isnum))
    ట్యాగ్_error(L, arg, LUA_TNUMBER);
  తిరిగి d;
}


LUALIB_API lua_సంఖ్య luaL_optnumber (lua_State *L, పూర్ణాంకం arg, lua_సంఖ్య def) {
  తిరిగి luaL_opt(L, luaL_checknumber, arg, def);
}


స్థిర శూన్య interror (lua_State *L, పూర్ణాంకం arg) {
  ఒకవేళ (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  లేకపోతే
    ట్యాగ్_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, పూర్ణాంకం arg) {
  పూర్ణాంకం isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  ఒకవేళ (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  తిరిగి d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, పూర్ణాంకం arg,
                                                      lua_Integer def) {
  తిరిగి luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
రకనిర్వచనం నిర్మాణం UBox {
  శూన్య *box;
  size_t bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
స్థిర శూన్య *resizebox (lua_State *L, పూర్ణాంకం idx, size_t newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  ఒకవేళ (box->bsize == newsize)  /* not changing size? */
    తిరిగి box->box;  /* keep the buffer */
  లేకపోతే {
    శూన్య *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    శూన్య *తాత్కాలిక = allocf(ud, box->box, box->bsize, newsize);
    ఒకవేళ (l_unlikely(తాత్కాలిక == ఖాళీ && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = తాత్కాలిక;
    box->bsize = newsize;
    తిరిగి తాత్కాలిక;
  }
}


స్థిర పూర్ణాంకం boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  తిరిగి 0;
}


స్థిర స్థిరరాశి luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {ఖాళీ, ఖాళీ}
};


/*
** Get/create metatable (MT) for boxes
*/
స్థిర శూన్య getBoxMT (lua_State *L) {
  స్థిరరాశి అక్షరం *BOXMT = "_UBOX*"; /* key for the metatable */
  ఒకవేళ (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


స్థిర శూన్య newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, పరిమాణం(UBox), 0);
  box->box = ఖాళీ;
  box->bsize = 0;
  getBoxMT(L);
  lua_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#నిర్వచించు buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#నిర్వచించు checkbufferlevel(B,idx)  \
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != ఖాళీ  \
                            : lua_touserdata(B->L, idx) == (శూన్య*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
స్థిర size_t newbuffsize (lua_L_బఫరు *B, size_t sz) {
  size_t newsize = B->కొలత;
  ఒకవేళ (l_unlikely(sz >= MAX_కొలత - B->n))
    తిరిగి cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  ఒకవేళ (newsize <= MAX_కొలత/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  ఒకవేళ (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  తిరిగి newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
స్థిర అక్షరం *prepbuffsize (lua_L_బఫరు *B, size_t sz, పూర్ణాంకం boxidx) {
  checkbufferlevel(B, boxidx);
  ఒకవేళ (B->కొలత - B->n >= sz)  /* enough space? */
    తిరిగి B->b + B->n;
  లేకపోతే {
    lua_State *L = B->L;
    అక్షరం *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    ఒకవేళ (buffonstack(B))  /* buffer already has a box? */
      newbuff = (అక్షరం *)resizebox(L, boxidx, newsize);  /* resize it */
    లేకపోతే {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_చొప్పించు(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (అక్షరం *)resizebox(L, boxidx, newsize);
      జ్ఞాపకం_నకలు(newbuff, B->b, B->n * పరిమాణం(అక్షరం));  /* copy original content */
    }
    B->b = newbuff;
    B->కొలత = newsize;
    తిరిగి newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API అక్షరం *luaL_prepbuffsize (lua_L_బఫరు *B, size_t sz) {
  తిరిగి prepbuffsize(B, sz, -1);
}


LUALIB_API శూన్య luaL_addlstring (lua_L_బఫరు *B, స్థిరరాశి అక్షరం *s, size_t l) {
  ఒకవేళ (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    అక్షరం *b = prepbuffsize(B, l, -1);
    జ్ఞాపకం_నకలు(b, s, l * పరిమాణం(అక్షరం));
    luaL_addsize(B, l);
  }
}


LUALIB_API శూన్య luaL_addstring (lua_L_బఫరు *B, స్థిరరాశి అక్షరం *s) {
  luaL_addlstring(B, s, అక్షరశ్రేణి_పొడవు(s));
}


LUALIB_API శూన్య luaL_pushresult (lua_L_బఫరు *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  ఒకవేళ (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  లేకపోతే {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    శూన్య *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    size_t len = B->n;  /* final string length */
    అక్షరం *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (అక్షరం*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = ఖాళీ;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API శూన్య luaL_pushresultsize (lua_L_బఫరు *B, size_t sz) {
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
LUALIB_API శూన్య luaL_addvalue (lua_L_బఫరు *B) {
  lua_State *L = B->L;
  size_t len;
  స్థిరరాశి అక్షరం *s = lua_tolstring(L, -1, &len);
  అక్షరం *b = prepbuffsize(B, len, -2);
  జ్ఞాపకం_నకలు(b, s, len * పరిమాణం(అక్షరం));
  luaL_addsize(B, len);
  lua_తీయి(L, 1);  /* pop string */
}


LUALIB_API శూన్య luaL_buffinit (lua_State *L, lua_L_బఫరు *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->కొలత = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (శూన్య*)B);  /* push placeholder */
}


LUALIB_API అక్షరం *luaL_buffinitsize (lua_State *L, lua_L_బఫరు *B, size_t sz) {
  luaL_buffinit(L, B);
  తిరిగి prepbuffsize(B, sz, -1);
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
LUALIB_API పూర్ణాంకం luaL_ref (lua_State *L, పూర్ణాంకం t) {
  పూర్ణాంకం ref;
  ఒకవేళ (lua_isnil(L, -1)) {
    lua_తీయి(L, 1);  /* remove from stack */
    తిరిగి LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  ఒకవేళ (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (పూర్ణాంకం)lua_tointeger(L, -1);  /* ref = t[1] */
  లేకపోతే {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_తీయి(L, 1);  /* remove element from stack */
  ఒకవేళ (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  లేకపోతే  /* no free elements */
    ref = (పూర్ణాంకం)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  తిరిగి ref;
}


LUALIB_API శూన్య luaL_unref (lua_State *L, పూర్ణాంకం t, పూర్ణాంకం ref) {
  ఒకవేళ (ref >= 0) {
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

రకనిర్వచనం నిర్మాణం ఎక్కించు_F {
  గుర్తులేని n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  అక్షరం buff[BUFSIZ];  /* area for reading file */
} ఎక్కించు_F;


స్థిర స్థిరరాశి అక్షరం *getF (lua_State *L, శూన్య *ud, size_t *కొలత) {
  ఎక్కించు_F *lf = (ఎక్కించు_F *)ud;
  వాడని(L);
  ఒకవేళ (lf->n > 0) {  /* are there pre-read characters to be read? */
    *కొలత = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  లేకపోతే {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    ఒకవేళ (feof(lf->f)) తిరిగి ఖాళీ;
    *కొలత = fread(lf->buff, 1, పరిమాణం(lf->buff), lf->f);  /* read block */
  }
  తిరిగి lf->buff;
}


స్థిర పూర్ణాంకం errfile (lua_State *L, స్థిరరాశి అక్షరం *what, పూర్ణాంకం fnameindex) {
  పూర్ణాంకం err = errno;
  స్థిరరాశి అక్షరం *filename = lua_tostring(L, fnameindex) + 1;
  ఒకవేళ (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, strerror(err));
  లేకపోతే
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  తిరిగి LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
స్థిర పూర్ణాంకం skipBOM (FILE *f) {
  పూర్ణాంకం c = getc(f);  /* read first character */
  ఒకవేళ (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    తిరిగి getc(f);  /* ignore BOM and return next char */
  లేకపోతే  /* no (valid) BOM */
    తిరిగి c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
స్థిర పూర్ణాంకం skipcomment (FILE *f, పూర్ణాంకం *cp) {
  పూర్ణాంకం c = *cp = skipBOM(f);
  ఒకవేళ (c == '#') {  /* first line is a comment (Unix exec. file)? */
    చేయి {  /* skip first line */
      c = getc(f);
    } అయితే (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    తిరిగి 1;  /* there was a comment */
  }
  లేకపోతే తిరిగి 0;  /* no comment */
}


LUALIB_API పూర్ణాంకం luaL_loadfilex (lua_State *L, స్థిరరాశి అక్షరం *filename,
                                             స్థిరరాశి అక్షరం *బహుళకం) {
  ఎక్కించు_F lf;
  పూర్ణాంకం స్థితి_2, readstatus;
  పూర్ణాంకం c;
  పూర్ణాంకం fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  ఒకవేళ (filename == ఖాళీ) {
    lua_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  లేకపోతే {
    lua_pushfstring(L, "@%s", filename);
    errno = 0;
    lf.f = fopen(filename, "r");
    ఒకవేళ (lf.f == ఖాళీ) తిరిగి errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  ఒకవేళ (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  ఒకవేళ (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    ఒకవేళ (filename) {  /* "real" file? */
      errno = 0;
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      ఒకవేళ (lf.f == ఖాళీ) తిరిగి errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  ఒకవేళ (c != EOF)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  స్థితి_2 = lua_ఎక్కించు(L, getF, &lf, lua_tostring(L, -1), బహుళకం);
  readstatus = ferror(lf.f);
  errno = 0;  /* no useful error number until here */
  ఒకవేళ (filename) fclose(lf.f);  /* close file (even in case of errors) */
  ఒకవేళ (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    తిరిగి errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  తిరిగి స్థితి_2;
}


రకనిర్వచనం నిర్మాణం ఎక్కించు_S {
  స్థిరరాశి అక్షరం *s;
  size_t కొలత;
} ఎక్కించు_S;


స్థిర స్థిరరాశి అక్షరం *getS (lua_State *L, శూన్య *ud, size_t *కొలత) {
  ఎక్కించు_S *ls = (ఎక్కించు_S *)ud;
  వాడని(L);
  ఒకవేళ (ls->కొలత == 0) తిరిగి ఖాళీ;
  *కొలత = ls->కొలత;
  ls->కొలత = 0;
  తిరిగి ls->s;
}


LUALIB_API పూర్ణాంకం luaL_loadbufferx (lua_State *L, స్థిరరాశి అక్షరం *buff, size_t కొలత,
                                 స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *బహుళకం) {
  ఎక్కించు_S ls;
  ls.s = buff;
  ls.కొలత = కొలత;
  తిరిగి lua_ఎక్కించు(L, getS, &ls, name, బహుళకం);
}


LUALIB_API పూర్ణాంకం luaL_loadstring (lua_State *L, స్థిరరాశి అక్షరం *s) {
  తిరిగి luaL_loadbufferx(L, s, అక్షరశ్రేణి_పొడవు(s), s, "t");
}

/* }====================================================== */



LUALIB_API పూర్ణాంకం luaL_getmetafield (lua_State *L, పూర్ణాంకం obj, స్థిరరాశి అక్షరం *event) {
  ఒకవేళ (!lua_getmetatable(L, obj))  /* no metatable? */
    తిరిగి LUA_TNIL;
  లేకపోతే {
    పూర్ణాంకం tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    ఒకవేళ (tt == LUA_TNIL)  /* is metafield nil? */
      lua_తీయి(L, 2);  /* remove metatable and metafield */
    లేకపోతే
      lua_remove(L, -2);  /* remove only metatable */
    తిరిగి tt;  /* return metafield type */
  }
}


LUALIB_API పూర్ణాంకం luaL_callmeta (lua_State *L, పూర్ణాంకం obj, స్థిరరాశి అక్షరం *event) {
  obj = lua_absindex(L, obj);
  ఒకవేళ (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    తిరిగి 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  తిరిగి 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, పూర్ణాంకం idx) {
  lua_Integer l;
  పూర్ణాంకం isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  ఒకవేళ (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_తీయి(L, 1);  /* remove object */
  తిరిగి l;
}


LUALIB_API స్థిరరాశి అక్షరం *luaL_tolstring (lua_State *L, పూర్ణాంకం idx, size_t *len) {
  idx = lua_absindex(L,idx);
  ఒకవేళ (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    ఒకవేళ (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  లేకపోతే {
    ఎంపిక (lua_type(L, idx)) {
      స్థితి LUA_TNUMBER: {
        అక్షరం buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        ఆపు;
      }
      స్థితి LUA_TSTRING:
        lua_pushvalue(L, idx);
        ఆపు;
      స్థితి LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        ఆపు;
      స్థితి LUA_TNIL:
        lua_pushliteral(L, "nil");
        ఆపు;
      అప్రమేయ: {
        పూర్ణాంకం tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        స్థిరరాశి అక్షరం *రకం = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", రకం, lua_topointer(L, idx));
        ఒకవేళ (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        ఆపు;
      }
    }
  }
  తిరిగి lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API శూన్య luaL_setfuncs (lua_State *L, స్థిరరాశి luaL_Reg *l, పూర్ణాంకం nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  కొరకు (; l->name != ఖాళీ; l++) {  /* fill the table with given functions */
    ఒకవేళ (l->func == ఖాళీ)  /* placeholder? */
      lua_pushboolean(L, 0);
    లేకపోతే {
      పూర్ణాంకం i;
      కొరకు (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_తీయి(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API పూర్ణాంకం luaL_getsubtable (lua_State *L, పూర్ణాంకం idx, స్థిరరాశి అక్షరం *fname) {
  ఒకవేళ (lua_getfield(L, idx, fname) == LUA_TTABLE)
    తిరిగి 1;  /* table already there */
  లేకపోతే {
    lua_తీయి(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    తిరిగి 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API శూన్య luaL_requiref (lua_State *L, స్థిరరాశి అక్షరం *modname,
                               lua_CFunction openf, పూర్ణాంకం glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_పట్టిక);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  ఒకవేళ (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_తీయి(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  ఒకవేళ (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API శూన్య luaL_addgsub (lua_L_బఫరు *b, స్థిరరాశి అక్షరం *s,
                                     స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *r) {
  స్థిరరాశి అక్షరం *wild;
  size_t l = అక్షరశ్రేణి_పొడవు(p);
  అయితే ((wild = అక్షరశ్రేణి_శోధన(s, p)) != ఖాళీ) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API స్థిరరాశి అక్షరం *luaL_gsub (lua_State *L, స్థిరరాశి అక్షరం *s,
                                  స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *r) {
  lua_L_బఫరు b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  తిరిగి lua_tostring(L, -1);
}


శూన్య *luaL_alloc (శూన్య *ud, శూన్య *ptr, size_t osize, size_t nsize) {
  వాడని(ud); వాడని(osize);
  ఒకవేళ (nsize == 0) {
    విడుదల(ptr);
    తిరిగి ఖాళీ;
  }
  లేకపోతే
    తిరిగి realloc(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
స్థిర పూర్ణాంకం panic (lua_State *L) {
  స్థిరరాశి అక్షరం *సందేశం = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        సందేశం);
  తిరిగి 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
స్థిర శూన్య warnfoff (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం_2, పూర్ణాంకం tocont);
స్థిర శూన్య warnfon (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం_2, పూర్ణాంకం tocont);
స్థిర శూన్య warnfcont (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం_2, పూర్ణాంకం tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
స్థిర పూర్ణాంకం checkcontrol (lua_State *L, స్థిరరాశి అక్షరం *సందేశం_2, పూర్ణాంకం tocont) {
  ఒకవేళ (tocont || *(సందేశం_2++) != '@')  /* not a control message? */
    తిరిగి 0;
  లేకపోతే {
    ఒకవేళ (strcmp(సందేశం_2, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    లేకపోతే ఒకవేళ (strcmp(సందేశం_2, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    తిరిగి 1;  /* it was a control message */
  }
}


స్థిర శూన్య warnfoff (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం_2, పూర్ణాంకం tocont) {
  checkcontrol((lua_State *)ud, సందేశం_2, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
స్థిర శూన్య warnfcont (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం_2, పూర్ణాంకం tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", సందేశం_2);  /* write message */
  ఒకవేళ (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  లేకపోతే {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


స్థిర శూన్య warnfon (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం_2, పూర్ణాంకం tocont) {
  ఒకవేళ (checkcontrol((lua_State *)ud, సందేశం_2, tocont))  /* control message? */
    తిరిగి;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, సందేశం_2, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#ఒకవేళ !defined(luai_makeseed)

#అన్తర్భావ <time.h>


/* Size for the buffer, in bytes */
#నిర్వచించు BUFSEEDB	(పరిమాణం(శూన్య*) + పరిమాణం(time_t))

/* Size for the buffer in int's, rounded up */
#నిర్వచించు BUFSEED		((BUFSEEDB + పరిమాణం(పూర్ణాంకం) - 1) / పరిమాణం(పూర్ణాంకం))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#నిర్వచించు addbuff(b,v)	(జ్ఞాపకం_నకలు(&b[0], &(v), పరిమాణం(v)), b += పరిమాణం(v))


స్థిర గుర్తులేని పూర్ణాంకం luai_makeseed (శూన్య) {
  గుర్తులేని పూర్ణాంకం buff[BUFSEED];
  గుర్తులేని పూర్ణాంకం res;
  గుర్తులేని పూర్ణాంకం i;
  time_t t = time(ఖాళీ);
  అక్షరం *b = (అక్షరం*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /*సమయం*/
  /* fill (rare but possible) remain of the buffer with zeros */
  memset(b, 0, పరిమాణం(buff) - BUFSEEDB);
  res = buff[0];
  కొరకు (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  తిరిగి res;
}

#అయితే_ముగింపు


LUALIB_API గుర్తులేని పూర్ణాంకం luaL_makeseed (lua_State *L) {
  వాడని(L);
  తిరిగి luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (శూన్య) {
  lua_State *L = lua_newstate(luaL_alloc, ఖాళీ, luaL_makeseed(ఖాళీ));
  ఒకవేళ (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  తిరిగి L;
}


LUALIB_API శూన్య luaL_checkversion_ (lua_State *L, lua_సంఖ్య ver, size_t sz) {
  lua_సంఖ్య v = lua_సంస్కరణ(L);
  ఒకవేళ (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  లేకపోతే ఒకవేళ (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

