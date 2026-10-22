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
  परत co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
स्थिर पूर्णांक auxresume (lua_State *L, lua_State *co, पूर्णांक narg) {
  पूर्णांक status, nres;
  जर (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    परत -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  जर (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    जर (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_काढा(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      परत -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    परत nres;
  }
  अन्यथा {
    lua_xmove(co, L, 1);  /* move error message */
    परत -1;  /* error flag */
  }
}


स्थिर पूर्णांक luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  पूर्णांक r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  जर (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_घाला(L, -2);
    परत 2;  /* return false + error message */
  }
  अन्यथा {
    lua_pushboolean(L, 1);
    lua_घाला(L, -(r + 1));
    परत r + 1;  /* return true + 'resume' returns */
  }
}


स्थिर पूर्णांक luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  पूर्णांक r = auxresume(L, co, lua_gettop(L));
  जर (l_unlikely(r < 0)) {  /* error? */
    पूर्णांक stat = lua_status(co);
    जर (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      जर (stat == LUA_OK)
        परत 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    जर (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_घाला(L, -2);
      lua_concat(L, 2);
    }
    परत lua_error(L);  /* propagate error */
  }
  परत r;
}


स्थिर पूर्णांक luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  परत 1;
}


स्थिर पूर्णांक luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  परत 1;
}


स्थिर पूर्णांक luaB_yield (lua_State *L) {
  परत lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


स्थिर स्थिरांक अक्षर *स्थिरांक statname[] =
  {"running", "dead", "suspended", "normal"};


स्थिर पूर्णांक auxstatus (lua_State *L, lua_State *co) {
  जर (L == co) परत COS_RUN;
  अन्यथा {
    निवड (lua_status(co)) {
      स्थिती LUA_YIELD:
        परत COS_YIELD;
      स्थिती LUA_OK: {
        lua_Debug ar;
        जर (lua_getstack(co, 0, &ar))  /* does it have frames? */
          परत COS_NORM;  /* it is running */
        अन्यथा जर (lua_gettop(co) == 0)
            परत COS_DEAD;
        अन्यथा
          परत COS_YIELD;  /* initial state */
      }
      पूर्वनिर्धारित:  /* some error occurred */
        परत COS_DEAD;
    }
  }
}


स्थिर पूर्णांक luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  परत 1;
}


स्थिर lua_State *getoptco (lua_State *L) {
  परत (lua_isnone(L, 1) ? L : getco(L));
}


स्थिर पूर्णांक luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  परत 1;
}


स्थिर पूर्णांक luaB_corunning (lua_State *L) {
  पूर्णांक ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  परत 2;
}


स्थिर पूर्णांक luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  पूर्णांक status = auxstatus(L, co);
  निवड (status) {
    स्थिती COS_DEAD: स्थिती COS_YIELD: {
      status = lua_closethread(co, L);
      जर (status == LUA_OK) {
        lua_pushboolean(L, 1);
        परत 1;
      }
      अन्यथा {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        परत 2;
      }
    }
    स्थिती COS_NORM:
      परत luaL_error(L, "cannot close a %s coroutine", statname[status]);
    स्थिती COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      जर (lua_tothread(L, -1) == co)
        परत luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    पूर्वनिर्धारित:
      lua_assert(0);
      परत 0;
  }
}


स्थिर स्थिरांक luaL_Reg co_funcs[] = {
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



LUAMOD_API पूर्णांक luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  परत 1;
}

