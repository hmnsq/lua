/*
** $Id: lauxlib.c $
** Auxiliary functions for building Lua libraries
** See Copyright Notice in lua.h
*/

#घोषणा lauxlib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <errno.h>
#समावेश <stdarg.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>


/*
** This file uses only the official API of Lua.
** Any function declared here could be written as an application function.
*/

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "llimits.h"


/*
** {======================================================
** Traceback
** =======================================================
*/


#घोषणा LEVELS1	10	/* size of the first part of the stack */
#घोषणा LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
अटल पूर्णांक findfield (lua_State *L, पूर्णांक objidx, पूर्णांक level) {
  अगर (level == 0 || !lua_istable(L, -1))
    वापस 0;  /* not found */
  lua_pushnil(L);  /* start 'next' loop */
  जबतक (lua_अग्र(L, -2)) {  /* for each pair in table */
    अगर (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      अगर (lua_rawequal(L, objidx, -1)) {  /* found object? */
        lua_निकालो(L, 1);  /* remove value (but keep name) */
        वापस 1;
      }
      अन्यथा अगर (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        lua_pushliteral(L, ".");  /* place '.' between the two names */
        lua_replace(L, -3);  /* (in the slot occupied by table) */
        lua_concat(L, 3);  /* lib_name.field_name */
        वापस 1;
      }
    }
    lua_निकालो(L, 1);  /* remove value */
  }
  वापस 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
अटल पूर्णांक pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  पूर्णांक शीर्ष = lua_gettop(L);
  lua_getinfo(L, "f", ar);  /* push function */
  lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  luaL_checkstack(L, 6, "not enough stack");  /* slots for 'findfield' */
  अगर (findfield(L, शीर्ष + 1, 2)) {
    स्थिर अक्षर *name = lua_tostring(L, -1);
    अगर (म_भेदन(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      lua_pushstring(L, name + 3);  /* push name without prefix */
      lua_remove(L, -2);  /* remove original name */
    }
    lua_copy(L, -1, शीर्ष + 1);  /* copy name to proper place */
    lua_settop(L, शीर्ष + 1);  /* remove table "loaded" and name copy */
    वापस 1;
  }
  अन्यथा {
    lua_settop(L, शीर्ष);  /* remove function and global table */
    वापस 0;
  }
}


अटल व्योम pushfuncname (lua_State *L, lua_Debug *ar) {
  अगर (*ar->namewhat != '\0')  /* is there a name from code? */
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  अन्यथा अगर (*ar->what == 'm')  /* main? */
      lua_pushliteral(L, "main chunk");
  अन्यथा अगर (pushglobalfuncname(L, ar)) {  /* try a global name */
    lua_pushfstring(L, "function '%s'", lua_tostring(L, -1));
    lua_remove(L, -2);  /* remove name */
  }
  अन्यथा अगर (*ar->what != 'C')  /* for Lua functions, use <file:line> */
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  अन्यथा  /* nothing left... */
    lua_pushliteral(L, "?");
}


अटल पूर्णांक lastlevel (lua_State *L) {
  lua_Debug ar;
  पूर्णांक li = 1, le = 1;
  /* find an upper bound */
  जबतक (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  जबतक (li < le) {
    पूर्णांक m = (li + le)/2;
    अगर (lua_getstack(L, m, &ar)) li = m + 1;
    अन्यथा le = m;
  }
  वापस le - 1;
}


LUALIB_API व्योम luaL_traceback (lua_State *L, lua_State *L1,
                                स्थिर अक्षर *संदेश, पूर्णांक level) {
  lua_L_बफर b;
  lua_Debug ar;
  पूर्णांक last = lastlevel(L1);
  पूर्णांक limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  luaL_buffinit(L, &b);
  अगर (संदेश) {
    luaL_addstring(&b, संदेश);
    luaL_addchar(&b, '\n');
  }
  luaL_addstring(&b, "stack traceback:");
  जबतक (lua_getstack(L1, level++, &ar)) {
    अगर (limit2show-- == 0) {  /* too many levels? */
      पूर्णांक n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    अन्यथा {
      lua_getinfo(L1, "Slnt", &ar);
      अगर (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      अन्यथा
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      अगर (ar.istailcall)
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

LUALIB_API पूर्णांक luaL_argerror (lua_State *L, पूर्णांक arg, स्थिर अक्षर *extramsg) {
  lua_Debug ar;
  स्थिर अक्षर *argword;
  अगर (!lua_getstack(L, 0, &ar))  /* no stack frame? */
    वापस luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "nt", &ar);
  अगर (arg <= ar.extraargs)  /* error in an extra argument? */
    argword =  "extra argument";
  अन्यथा {
    arg -= ar.extraargs;  /* do not count extra arguments */
    अगर (म_भेद(ar.namewhat, "method") == 0) {  /* colon syntax? */
      arg--;  /* do not count (extra) self argument */
      अगर (arg == 0)  /* error in self argument? */
        वापस luaL_error(L, "calling '%s' on bad self (%s)",
                               ar.name, extramsg);
      /* else go through; error in a regular argument */
    }
    argword = "argument";
  }
  अगर (ar.name == शून्य)
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tostring(L, -1) : "?";
  वापस luaL_error(L, "bad %s #%d to '%s' (%s)",
                       argword, arg, ar.name, extramsg);
}


LUALIB_API पूर्णांक luaL_typeerror (lua_State *L, पूर्णांक arg, स्थिर अक्षर *tname) {
  स्थिर अक्षर *संदेश;
  स्थिर अक्षर *typearg;  /* name for the type of the actual argument */
  अगर (luaL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = lua_tostring(L, -1);  /* use the given type name */
  अन्यथा अगर (lua_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  अन्यथा
    typearg = luaL_typename(L, arg);  /* standard name */
  संदेश = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  वापस luaL_argerror(L, arg, संदेश);
}


अटल व्योम tag_error (lua_State *L, पूर्णांक arg, पूर्णांक tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}


/*
** The use of 'lua_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API व्योम luaL_where (lua_State *L, पूर्णांक level) {
  lua_Debug ar;
  अगर (lua_getstack(L, level, &ar)) {  /* check function at level */
    lua_getinfo(L, "Sl", &ar);  /* get info about it */
    अगर (ar.currentline > 0) {  /* is there info? */
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      वापस;
    }
  }
  lua_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'lua_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** a memory error instead of the given message.)
*/
LUALIB_API पूर्णांक luaL_error (lua_State *L, स्थिर अक्षर *fmt, ...) {
  बहु_सूची argp;
  बहु_शुरू(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  बहु_पूर्ण(argp);
  lua_concat(L, 2);
  वापस lua_error(L);
}


LUALIB_API पूर्णांक luaL_fileresult (lua_State *L, पूर्णांक stat, स्थिर अक्षर *fname) {
  पूर्णांक en = त्रुटि_सं;  /* calls to Lua API may change this value */
  अगर (stat) {
    lua_pushboolean(L, 1);
    वापस 1;
  }
  अन्यथा {
    स्थिर अक्षर *संदेश;
    luaL_pushfail(L);
    संदेश = (en != 0) ? म_त्रुटि(en) : "(no extra info)";
    अगर (fname)
      lua_pushfstring(L, "%s: %s", fname, संदेश);
    अन्यथा
      lua_pushstring(L, संदेश);
    lua_pushinteger(L, en);
    वापस 3;
  }
}


#अगर !defined(l_inspectstat)	/* { */

#अगर defined(LUA_USE_POSIX)

#समावेश <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#घोषणा l_inspectstat(stat,what)  \
   अगर (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   अन्यथा अगर (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#अन्यथा

#घोषणा l_inspectstat(stat,what)  /* no op */

#पूर्ण_अगर

#पूर्ण_अगर				/* } */


LUALIB_API पूर्णांक luaL_execresult (lua_State *L, पूर्णांक stat) {
  अगर (stat != 0 && त्रुटि_सं != 0)  /* error with an 'errno'? */
    वापस luaL_fileresult(L, 0, शून्य);
  अन्यथा {
    स्थिर अक्षर *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    अगर (*what == 'e' && stat == 0)  /* successful termination? */
      lua_pushboolean(L, 1);
    अन्यथा
      luaL_pushfail(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    वापस 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API पूर्णांक luaL_newmetatable (lua_State *L, स्थिर अक्षर *tname) {
  अगर (luaL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    वापस 0;  /* leave previous value on top, but return 0 */
  lua_निकालो(L, 1);
  lua_createtable(L, 0, 2);  /* create metatable */
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");  /* metatable.__name = tname */
  lua_pushvalue(L, -1);
  lua_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  वापस 1;
}


LUALIB_API व्योम luaL_setmetatable (lua_State *L, स्थिर अक्षर *tname) {
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}


LUALIB_API व्योम *luaL_testudata (lua_State *L, पूर्णांक ud, स्थिर अक्षर *tname) {
  व्योम *p = lua_touserdata(L, ud);
  अगर (p != शून्य) {  /* value is a userdata? */
    अगर (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      luaL_getmetatable(L, tname);  /* get correct metatable */
      अगर (!lua_rawequal(L, -1, -2))  /* not the same? */
        p = शून्य;  /* value is a userdata with wrong metatable */
      lua_निकालो(L, 2);  /* remove both metatables */
      वापस p;
    }
  }
  वापस शून्य;  /* value is not a userdata with a metatable */
}


LUALIB_API व्योम *luaL_checkudata (lua_State *L, पूर्णांक ud, स्थिर अक्षर *tname) {
  व्योम *p = luaL_testudata(L, ud, tname);
  luaL_argexpected(L, p != शून्य, ud, tname);
  वापस p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API पूर्णांक luaL_checkoption (lua_State *L, पूर्णांक arg, स्थिर अक्षर *def,
                                 स्थिर अक्षर *स्थिर lst[]) {
  स्थिर अक्षर *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  पूर्णांक i;
  क्रम (i=0; lst[i]; i++)
    अगर (म_भेद(lst[i], name) == 0)
      वापस i;
  वापस luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, Lua will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API व्योम luaL_checkstack (lua_State *L, पूर्णांक space, स्थिर अक्षर *संदेश) {
  अगर (l_unlikely(!lua_checkstack(L, space))) {
    अगर (संदेश)
      luaL_error(L, "stack overflow (%s)", संदेश);
    अन्यथा
      luaL_error(L, "stack overflow");
  }
}


LUALIB_API व्योम luaL_checktype (lua_State *L, पूर्णांक arg, पूर्णांक t) {
  अगर (l_unlikely(lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API व्योम luaL_checkany (lua_State *L, पूर्णांक arg) {
  अगर (l_unlikely(lua_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API स्थिर अक्षर *luaL_checklstring (lua_State *L, पूर्णांक arg, माप_प्रकार *len) {
  स्थिर अक्षर *s = lua_tolstring(L, arg, len);
  अगर (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  वापस s;
}


LUALIB_API स्थिर अक्षर *luaL_optlstring (lua_State *L, पूर्णांक arg,
                                        स्थिर अक्षर *def, माप_प्रकार *len) {
  अगर (lua_isnoneornil(L, arg)) {
    अगर (len)
      *len = (def ? म_माप(def) : 0);
    वापस def;
  }
  अन्यथा वापस luaL_checklstring(L, arg, len);
}


LUALIB_API lua_संख्या luaL_checknumber (lua_State *L, पूर्णांक arg) {
  पूर्णांक isnum;
  lua_संख्या d = lua_tonumberx(L, arg, &isnum);
  अगर (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  वापस d;
}


LUALIB_API lua_संख्या luaL_optnumber (lua_State *L, पूर्णांक arg, lua_संख्या def) {
  वापस luaL_opt(L, luaL_checknumber, arg, def);
}


अटल व्योम interror (lua_State *L, पूर्णांक arg) {
  अगर (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  अन्यथा
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API lua_Integer luaL_checkinteger (lua_State *L, पूर्णांक arg) {
  पूर्णांक isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  अगर (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  वापस d;
}


LUALIB_API lua_Integer luaL_optinteger (lua_State *L, पूर्णांक arg,
                                                      lua_Integer def) {
  वापस luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
प्रकार काष्ठा UBox {
  व्योम *box;
  माप_प्रकार bsize;
} UBox;


/* Resize the buffer used by a box. Optimize for the common case of
** resizing to the old size. (For instance, __gc will resize the box
** to 0 even after it was closed. 'pushresult' may also resize it to a
** final size that is equal to the one set when the buffer was created.)
*/
अटल व्योम *resizebox (lua_State *L, पूर्णांक idx, माप_प्रकार newsize) {
  UBox *box = (UBox *)lua_touserdata(L, idx);
  अगर (box->bsize == newsize)  /* not changing size? */
    वापस box->box;  /* keep the buffer */
  अन्यथा {
    व्योम *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);
    व्योम *अस्थायी = allocf(ud, box->box, box->bsize, newsize);
    अगर (l_unlikely(अस्थायी == शून्य && newsize > 0)) {  /* allocation error? */
      lua_pushliteral(L, "not enough memory");
      lua_error(L);  /* raise a memory error */
    }
    box->box = अस्थायी;
    box->bsize = newsize;
    वापस अस्थायी;
  }
}


अटल पूर्णांक boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  वापस 0;
}


अटल स्थिर luaL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {शून्य, शून्य}
};


/*
** Get/create metatable (MT) for boxes
*/
अटल व्योम getBoxMT (lua_State *L) {
  स्थिर अक्षर *BOXMT = "_UBOX*"; /* key for the metatable */
  अगर (luaL_getmetatable(L, BOXMT) == LUA_TNIL) {  /* MT not created yet? */
    luaL_newlibtable(L, boxmt);  /* create it */
    luaL_setfuncs(L, boxmt, 0);  /* initialize it */
    lua_copy(L, -1, -2);  /* change stack from nil,MT to MT,MT */
    lua_setfield(L, LUA_REGISTRYINDEX, BOXMT);  /* store MT in the registry */
  }
}


अटल व्योम newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, माप(UBox), 0);
  box->box = शून्य;
  box->bsize = 0;
  getBoxMT(L);
  lua_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#घोषणा buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#घोषणा checkbufferlevel(B,idx)  \
  lua_assert(buffonstack(B) ? lua_touserdata(B->L, idx) != शून्य  \
                            : lua_touserdata(B->L, idx) == (व्योम*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes plus one for a terminating zero.
*/
अटल माप_प्रकार newbuffsize (lua_L_बफर *B, माप_प्रकार sz) {
  माप_प्रकार newsize = B->आकार;
  अगर (l_unlikely(sz >= MAX_आकार - B->n))
    वापस cast_sizet(luaL_error(B->L, "resulting string too large"));
  /* else  B->n + sz + 1 <= MAX_SIZE */
  अगर (newsize <= MAX_आकार/3 * 2)  /* no overflow? */
    newsize += (newsize >> 1);  /* new size *= 1.5 */
  अगर (newsize < B->n + sz + 1)  /* not big enough? */
    newsize = B->n + sz + 1;
  वापस newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
अटल अक्षर *prepbuffsize (lua_L_बफर *B, माप_प्रकार sz, पूर्णांक boxidx) {
  checkbufferlevel(B, boxidx);
  अगर (B->आकार - B->n >= sz)  /* enough space? */
    वापस B->b + B->n;
  अन्यथा {
    lua_State *L = B->L;
    अक्षर *newbuff;
    माप_प्रकार newsize = newbuffsize(B, sz);
    /* create larger buffer */
    अगर (buffonstack(B))  /* buffer already has a box? */
      newbuff = (अक्षर *)resizebox(L, boxidx, newsize);  /* resize it */
    अन्यथा {  /* no box yet */
      lua_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      lua_जोड़ो(L, boxidx);  /* move box to its intended position */
      lua_toclose(L, boxidx);
      newbuff = (अक्षर *)resizebox(L, boxidx, newsize);
      स_नकल(newbuff, B->b, B->n * माप(अक्षर));  /* copy original content */
    }
    B->b = newbuff;
    B->आकार = newsize;
    वापस newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API अक्षर *luaL_prepbuffsize (lua_L_बफर *B, माप_प्रकार sz) {
  वापस prepbuffsize(B, sz, -1);
}


LUALIB_API व्योम luaL_addlstring (lua_L_बफर *B, स्थिर अक्षर *s, माप_प्रकार l) {
  अगर (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    अक्षर *b = prepbuffsize(B, l, -1);
    स_नकल(b, s, l * माप(अक्षर));
    luaL_addsize(B, l);
  }
}


LUALIB_API व्योम luaL_addstring (lua_L_बफर *B, स्थिर अक्षर *s) {
  luaL_addlstring(B, s, म_माप(s));
}


LUALIB_API व्योम luaL_pushresult (lua_L_बफर *B) {
  lua_State *L = B->L;
  checkbufferlevel(B, -1);
  अगर (!buffonstack(B))  /* using static buffer? */
    lua_pushlstring(L, B->b, B->n);  /* save result as regular string */
  अन्यथा {  /* reuse buffer already allocated */
    UBox *box = (UBox *)lua_touserdata(L, -1);
    व्योम *ud;
    lua_Alloc allocf = lua_getallocf(L, &ud);  /* function to free buffer */
    माप_प्रकार len = B->n;  /* final string length */
    अक्षर *s;
    resizebox(L, -1, len + 1);  /* adjust box size to content size */
    s = (अक्षर*)box->box;  /* final buffer address */
    s[len] = '\0';  /* add ending zero */
    /* clear box, as Lua will take control of the buffer */
    box->bsize = 0;  box->box = शून्य;
    lua_pushexternalstring(L, s, len, allocf, ud);
    lua_closeslot(L, -2);  /* close the box */
    lua_gc(L, LUA_GCSTEP, len);
  }
  lua_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API व्योम luaL_pushresultsize (lua_L_बफर *B, माप_प्रकार sz) {
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
LUALIB_API व्योम luaL_addvalue (lua_L_बफर *B) {
  lua_State *L = B->L;
  माप_प्रकार len;
  स्थिर अक्षर *s = lua_tolstring(L, -1, &len);
  अक्षर *b = prepbuffsize(B, len, -2);
  स_नकल(b, s, len * माप(अक्षर));
  luaL_addsize(B, len);
  lua_निकालो(L, 1);  /* pop string */
}


LUALIB_API व्योम luaL_buffinit (lua_State *L, lua_L_बफर *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->आकार = LUAL_BUFFERSIZE;
  lua_pushlightuserdata(L, (व्योम*)B);  /* push placeholder */
}


LUALIB_API अक्षर *luaL_buffinitsize (lua_State *L, lua_L_बफर *B, माप_प्रकार sz) {
  luaL_buffinit(L, B);
  वापस prepbuffsize(B, sz, -1);
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
LUALIB_API पूर्णांक luaL_ref (lua_State *L, पूर्णांक t) {
  पूर्णांक ref;
  अगर (lua_isnil(L, -1)) {
    lua_निकालो(L, 1);  /* remove from stack */
    वापस LUA_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = lua_absindex(L, t);
  अगर (lua_rawgeti(L, t, 1) == LUA_TNUMBER)  /* already initialized? */
    ref = (पूर्णांक)lua_tointeger(L, -1);  /* ref = t[1] */
  अन्यथा {  /* first access */
    lua_assert(!lua_toboolean(L, -1));  /* must be nil or false */
    ref = 0;  /* list is empty */
    lua_pushinteger(L, 0);  /* initialize as an empty list */
    lua_rawseti(L, t, 1);  /* ref = t[1] = 0 */
  }
  lua_निकालो(L, 1);  /* remove element from stack */
  अगर (ref != 0) {  /* any free element? */
    lua_rawgeti(L, t, ref);  /* remove it from list */
    lua_rawseti(L, t, 1);  /* (t[1] = t[ref]) */
  }
  अन्यथा  /* no free elements */
    ref = (पूर्णांक)lua_rawlen(L, t) + 1;  /* get a new reference */
  lua_rawseti(L, t, ref);
  वापस ref;
}


LUALIB_API व्योम luaL_unref (lua_State *L, पूर्णांक t, पूर्णांक ref) {
  अगर (ref >= 0) {
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

प्रकार काष्ठा LoadF {
  अचिन्हित n;  /* number of pre-read characters */
  खाता *f;  /* file being read */
  अक्षर buff[बफ_मान];  /* area for reading file */
} LoadF;


अटल स्थिर अक्षर *getF (lua_State *L, व्योम *ud, माप_प्रकार *आकार) {
  LoadF *lf = (LoadF *)ud;
  UNUSED(L);
  अगर (lf->n > 0) {  /* are there pre-read characters to be read? */
    *आकार = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  अन्यथा {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    अगर (ख_पूर्ण(lf->f)) वापस शून्य;
    *आकार = ख_पढ़ो(lf->buff, 1, माप(lf->buff), lf->f);  /* read block */
  }
  वापस lf->buff;
}


अटल पूर्णांक errfile (lua_State *L, स्थिर अक्षर *what, पूर्णांक fnameindex) {
  पूर्णांक err = त्रुटि_सं;
  स्थिर अक्षर *filename = lua_tostring(L, fnameindex) + 1;
  अगर (err != 0)
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, म_त्रुटि(err));
  अन्यथा
    lua_pushfstring(L, "cannot %s %s", what, filename);
  lua_remove(L, fnameindex);
  वापस LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
अटल पूर्णांक skipBOM (खाता *f) {
  पूर्णांक c = अ_लो(f);  /* read first character */
  अगर (c == 0xEF && अ_लो(f) == 0xBB && अ_लो(f) == 0xBF)  /* correct BOM? */
    वापस अ_लो(f);  /* ignore BOM and return next char */
  अन्यथा  /* no (valid) BOM */
    वापस c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
अटल पूर्णांक skipcomment (खाता *f, पूर्णांक *cp) {
  पूर्णांक c = *cp = skipBOM(f);
  अगर (c == '#') {  /* first line is a comment (Unix exec. file)? */
    करो {  /* skip first line */
      c = अ_लो(f);
    } जबतक (c != खातापूर्ण && c != '\n');
    *cp = अ_लो(f);  /* next character after comment, if present */
    वापस 1;  /* there was a comment */
  }
  अन्यथा वापस 0;  /* no comment */
}


LUALIB_API पूर्णांक luaL_loadfilex (lua_State *L, स्थिर अक्षर *filename,
                                             स्थिर अक्षर *mode) {
  LoadF lf;
  पूर्णांक status, readstatus;
  पूर्णांक c;
  पूर्णांक fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
  अगर (filename == शून्य) {
    lua_pushliteral(L, "=stdin");
    lf.f = मानक_निवेश;
  }
  अन्यथा {
    lua_pushfstring(L, "@%s", filename);
    त्रुटि_सं = 0;
    lf.f = ख_खोलो(filename, "r");
    अगर (lf.f == शून्य) वापस errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  अगर (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  अगर (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    अगर (filename) {  /* "real" file? */
      त्रुटि_सं = 0;
      lf.f = ख_व_खोलो(filename, "rb", lf.f);  /* reopen in binary mode */
      अगर (lf.f == शून्य) वापस errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  अगर (c != खातापूर्ण)
    lf.buff[lf.n++] = cast_char(c);  /* 'c' is the first character */
  status = lua_load(L, getF, &lf, lua_tostring(L, -1), mode);
  readstatus = ख_त्रुटि(lf.f);
  त्रुटि_सं = 0;  /* no useful error number until here */
  अगर (filename) ख_बंद(lf.f);  /* close file (even in case of errors) */
  अगर (readstatus) {
    lua_settop(L, fnameindex);  /* ignore results from 'lua_load' */
    वापस errfile(L, "read", fnameindex);
  }
  lua_remove(L, fnameindex);
  वापस status;
}


प्रकार काष्ठा LoadS {
  स्थिर अक्षर *s;
  माप_प्रकार आकार;
} LoadS;


अटल स्थिर अक्षर *getS (lua_State *L, व्योम *ud, माप_प्रकार *आकार) {
  LoadS *ls = (LoadS *)ud;
  UNUSED(L);
  अगर (ls->आकार == 0) वापस शून्य;
  *आकार = ls->आकार;
  ls->आकार = 0;
  वापस ls->s;
}


LUALIB_API पूर्णांक luaL_loadbufferx (lua_State *L, स्थिर अक्षर *buff, माप_प्रकार आकार,
                                 स्थिर अक्षर *name, स्थिर अक्षर *mode) {
  LoadS ls;
  ls.s = buff;
  ls.आकार = आकार;
  वापस lua_load(L, getS, &ls, name, mode);
}


LUALIB_API पूर्णांक luaL_loadstring (lua_State *L, स्थिर अक्षर *s) {
  वापस luaL_loadbufferx(L, s, म_माप(s), s, "t");
}

/* }====================================================== */



LUALIB_API पूर्णांक luaL_getmetafield (lua_State *L, पूर्णांक obj, स्थिर अक्षर *event) {
  अगर (!lua_getmetatable(L, obj))  /* no metatable? */
    वापस LUA_TNIL;
  अन्यथा {
    पूर्णांक tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    अगर (tt == LUA_TNIL)  /* is metafield nil? */
      lua_निकालो(L, 2);  /* remove metatable and metafield */
    अन्यथा
      lua_remove(L, -2);  /* remove only metatable */
    वापस tt;  /* return metafield type */
  }
}


LUALIB_API पूर्णांक luaL_callmeta (lua_State *L, पूर्णांक obj, स्थिर अक्षर *event) {
  obj = lua_absindex(L, obj);
  अगर (luaL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    वापस 0;
  lua_pushvalue(L, obj);
  lua_call(L, 1, 1);
  वापस 1;
}


LUALIB_API lua_Integer luaL_len (lua_State *L, पूर्णांक idx) {
  lua_Integer l;
  पूर्णांक isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  अगर (l_unlikely(!isnum))
    luaL_error(L, "object length is not an integer");
  lua_निकालो(L, 1);  /* remove object */
  वापस l;
}


LUALIB_API स्थिर अक्षर *luaL_tolstring (lua_State *L, पूर्णांक idx, माप_प्रकार *len) {
  idx = lua_absindex(L,idx);
  अगर (luaL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    अगर (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  अन्यथा {
    चयन (lua_type(L, idx)) {
      हाल LUA_TNUMBER: {
        अक्षर buff[LUA_N2SBUFFSZ];
        lua_numbertocstring(L, idx, buff);
        lua_pushstring(L, buff);
        अवरोध;
      }
      हाल LUA_TSTRING:
        lua_pushvalue(L, idx);
        अवरोध;
      हाल LUA_TBOOLEAN:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        अवरोध;
      हाल LUA_TNIL:
        lua_pushliteral(L, "nil");
        अवरोध;
      शेष: {
        पूर्णांक tt = luaL_getmetafield(L, idx, "__name");  /* try name */
        स्थिर अक्षर *kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                                                 luaL_typename(L, idx);
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        अगर (tt != LUA_TNIL)
          lua_remove(L, -2);  /* remove '__name' */
        अवरोध;
      }
    }
  }
  वापस lua_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API व्योम luaL_setfuncs (lua_State *L, स्थिर luaL_Reg *l, पूर्णांक nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  क्रम (; l->name != शून्य; l++) {  /* fill the table with given functions */
    अगर (l->func == शून्य)  /* placeholder? */
      lua_pushboolean(L, 0);
    अन्यथा {
      पूर्णांक i;
      क्रम (i = 0; i < nup; i++)  /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_निकालो(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API पूर्णांक luaL_getsubtable (lua_State *L, पूर्णांक idx, स्थिर अक्षर *fname) {
  अगर (lua_getfield(L, idx, fname) == LUA_TTABLE)
    वापस 1;  /* table already there */
  अन्यथा {
    lua_निकालो(L, 1);  /* remove previous result */
    idx = lua_absindex(L, idx);
    lua_newtable(L);
    lua_pushvalue(L, -1);  /* copy to be left at top */
    lua_setfield(L, idx, fname);  /* assign new table to field */
    वापस 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API व्योम luaL_requiref (lua_State *L, स्थिर अक्षर *modname,
                               lua_CFunction openf, पूर्णांक glb) {
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  अगर (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_निकालो(L, 1);  /* remove field */
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  अगर (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API व्योम luaL_addgsub (lua_L_बफर *b, स्थिर अक्षर *s,
                                     स्थिर अक्षर *p, स्थिर अक्षर *r) {
  स्थिर अक्षर *wild;
  माप_प्रकार l = म_माप(p);
  जबतक ((wild = म_माला(s, p)) != शून्य) {
    luaL_addlstring(b, s, ct_diff2sz(wild - s));  /* push prefix */
    luaL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  luaL_addstring(b, s);  /* push last suffix */
}


LUALIB_API स्थिर अक्षर *luaL_gsub (lua_State *L, स्थिर अक्षर *s,
                                  स्थिर अक्षर *p, स्थिर अक्षर *r) {
  lua_L_बफर b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  वापस lua_tostring(L, -1);
}


व्योम *luaL_alloc (व्योम *ud, व्योम *ptr, माप_प्रकार osize, माप_प्रकार nsize) {
  UNUSED(ud); UNUSED(osize);
  अगर (nsize == 0) {
    मुक्त(ptr);
    वापस शून्य;
  }
  अन्यथा
    वापस पुनः_स्मृति(ptr, nsize);
}


/*
** Standard panic function just prints an error message. The test
** with 'lua_type' avoids possible memory errors in 'lua_tostring'.
*/
अटल पूर्णांक panic (lua_State *L) {
  स्थिर अक्षर *संदेश = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        संदेश);
  वापस 0;  /* return to Lua to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
अटल व्योम warnfoff (व्योम *ud, स्थिर अक्षर *message, पूर्णांक tocont);
अटल व्योम warnfon (व्योम *ud, स्थिर अक्षर *message, पूर्णांक tocont);
अटल व्योम warnfcont (व्योम *ud, स्थिर अक्षर *message, पूर्णांक tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
अटल पूर्णांक checkcontrol (lua_State *L, स्थिर अक्षर *message, पूर्णांक tocont) {
  अगर (tocont || *(message++) != '@')  /* not a control message? */
    वापस 0;
  अन्यथा {
    अगर (म_भेद(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);  /* turn warnings off */
    अन्यथा अगर (म_भेद(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);   /* turn warnings on */
    वापस 1;  /* it was a control message */
  }
}


अटल व्योम warnfoff (व्योम *ud, स्थिर अक्षर *message, पूर्णांक tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
अटल व्योम warnfcont (व्योम *ud, स्थिर अक्षर *message, पूर्णांक tocont) {
  lua_State *L = (lua_State *)ud;
  lua_writestringerror("%s", message);  /* write message */
  अगर (tocont)  /* not the last part? */
    lua_setwarnf(L, warnfcont, L);  /* to be continued */
  अन्यथा {  /* last part */
    lua_writestringerror("%s", "\n");  /* finish message with end-of-line */
    lua_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


अटल व्योम warnfon (व्योम *ud, स्थिर अक्षर *message, पूर्णांक tocont) {
  अगर (checkcontrol((lua_State *)ud, message, tocont))  /* control message? */
    वापस;  /* nothing else to be done */
  lua_writestringerror("%s", "Lua warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}



/*
** A function to compute an unsigned int with some level of
** randomness. Rely on Address Space Layout Randomization (if present)
** and the current time.
*/
#अगर !defined(luai_makeseed)

#समावेश <time.h>


/* Size for the buffer, in bytes */
#घोषणा BUFSEEDB	(माप(व्योम*) + माप(समय_प्रकार))

/* Size for the buffer in int's, rounded up */
#घोषणा BUFSEED		((BUFSEEDB + माप(पूर्णांक) - 1) / माप(पूर्णांक))

/*
** Copy the contents of variable 'v' into the buffer pointed by 'b'.
** (The '&b[0]' disguises 'b' to fix an absurd warning from clang.)
*/
#घोषणा addbuff(b,v)	(स_नकल(&b[0], &(v), माप(v)), b += माप(v))


अटल अचिन्हित पूर्णांक luai_makeseed (व्योम) {
  अचिन्हित पूर्णांक buff[BUFSEED];
  अचिन्हित पूर्णांक res;
  अचिन्हित पूर्णांक i;
  समय_प्रकार t = समय(शून्य);
  अक्षर *b = (अक्षर*)buff;
  addbuff(b, b);  /* local variable's address */
  addbuff(b, t);  /* time */
  /* fill (rare but possible) remain of the buffer with zeros */
  स_रखो(b, 0, माप(buff) - BUFSEEDB);
  res = buff[0];
  क्रम (i = 1; i < BUFSEED; i++)
    res ^= (res >> 3) + (res << 7) + buff[i];
  वापस res;
}

#पूर्ण_अगर


LUALIB_API अचिन्हित पूर्णांक luaL_makeseed (lua_State *L) {
  UNUSED(L);
  वापस luai_makeseed();
}


/*
** Use the name with parentheses so that headers can redefine it
** as a macro.
*/
LUALIB_API lua_State *(luaL_newstate) (व्योम) {
  lua_State *L = lua_newstate(luaL_alloc, शून्य, luaL_makeseed(शून्य));
  अगर (l_likely(L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfon, L);
  }
  वापस L;
}


LUALIB_API व्योम luaL_checkversion_ (lua_State *L, lua_संख्या ver, माप_प्रकार sz) {
  lua_संख्या v = lua_version(L);
  अगर (sz != LUAL_NUMSIZES)  /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  अन्यथा अगर (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

