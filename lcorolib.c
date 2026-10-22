/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lualib.h"
#अन्तर्भाव "llimits.h"


स्थिर lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  प्रत्यागम co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
स्थिर पूर्णाङ्क auxresume (lua_State *L, lua_State *co, पूर्णाङ्क narg) {
  पूर्णाङ्क status, nres;
  जइ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    प्रत्यागम -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  जइ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    जइ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_निष्कासय(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      प्रत्यागम -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    प्रत्यागम nres;
  }
  अण्णहा {
    lua_xmove(co, L, 1);  /* move error message */
    प्रत्यागम -1;  /* error flag */
  }
}


स्थिर पूर्णाङ्क luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  पूर्णाङ्क r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  जइ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_निवेशय(L, -2);
    प्रत्यागम 2;  /* return false + error message */
  }
  अण्णहा {
    lua_pushboolean(L, 1);
    lua_निवेशय(L, -(r + 1));
    प्रत्यागम r + 1;  /* return true + 'resume' returns */
  }
}


स्थिर पूर्णाङ्क luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  पूर्णाङ्क r = auxresume(L, co, lua_gettop(L));
  जइ (l_unlikely(r < 0)) {  /* error? */
    पूर्णाङ्क stat = lua_status(co);
    जइ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      जइ (stat == LUA_OK)
        प्रत्यागम 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    जइ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_निवेशय(L, -2);
      lua_concat(L, 2);
    }
    प्रत्यागम lua_error(L);  /* propagate error */
  }
  प्रत्यागम r;
}


स्थिर पूर्णाङ्क luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क luaB_yield (lua_State *L) {
  प्रत्यागम lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


स्थिर ध्रुव वर्ण *ध्रुव statname[] =
  {"running", "dead", "suspended", "normal"};


स्थिर पूर्णाङ्क auxstatus (lua_State *L, lua_State *co) {
  जइ (L == co) प्रत्यागम COS_RUN;
  अण्णहा {
    विकल्प (lua_status(co)) {
      पक्ष LUA_YIELD:
        प्रत्यागम COS_YIELD;
      पक्ष LUA_OK: {
        lua_Debug ar;
        जइ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          प्रत्यागम COS_NORM;  /* it is running */
        अण्णहा जइ (lua_gettop(co) == 0)
            प्रत्यागम COS_DEAD;
        अण्णहा
          प्रत्यागम COS_YIELD;  /* initial state */
      }
      स्वभाव:  /* some error occurred */
        प्रत्यागम COS_DEAD;
    }
  }
}


स्थिर पूर्णाङ्क luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  प्रत्यागम 1;
}


स्थिर lua_State *getoptco (lua_State *L) {
  प्रत्यागम (lua_isnone(L, 1) ? L : getco(L));
}


स्थिर पूर्णाङ्क luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क luaB_corunning (lua_State *L) {
  पूर्णाङ्क ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  प्रत्यागम 2;
}


स्थिर पूर्णाङ्क luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  पूर्णाङ्क status = auxstatus(L, co);
  विकल्प (status) {
    पक्ष COS_DEAD: पक्ष COS_YIELD: {
      status = lua_closethread(co, L);
      जइ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        प्रत्यागम 1;
      }
      अण्णहा {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        प्रत्यागम 2;
      }
    }
    पक्ष COS_NORM:
      प्रत्यागम luaL_error(L, "cannot close a %s coroutine", statname[status]);
    पक्ष COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      जइ (lua_tothread(L, -1) == co)
        प्रत्यागम luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    स्वभाव:
      lua_assert(0);
      प्रत्यागम 0;
  }
}


स्थिर ध्रुव luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {रिक्त, रिक्त}
};



LUAMOD_API पूर्णाङ्क luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  प्रत्यागम 1;
}

