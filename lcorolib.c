/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#నిర్వచించు lcorolib_c
#నిర్వచించు LUA_LIB

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lualib.h"
#అన్తర్భావ "llimits.h"


స్థిర lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  తిరిగి co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
స్థిర పూర్ణాంకం auxresume (lua_State *L, lua_State *co, పూర్ణాంకం narg) {
  పూర్ణాంకం స్థితి_2, nres;
  ఒకవేళ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    తిరిగి -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  స్థితి_2 = lua_resume(co, L, narg, &nres);
  ఒకవేళ (l_likely(స్థితి_2 == LUA_OK || స్థితి_2 == LUA_YIELD)) {
    ఒకవేళ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_తీయి(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      తిరిగి -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    తిరిగి nres;
  }
  లేకపోతే {
    lua_xmove(co, L, 1);  /* move error message */
    తిరిగి -1;  /* error flag */
  }
}


స్థిర పూర్ణాంకం luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  పూర్ణాంకం r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ఒకవేళ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_చొప్పించు(L, -2);
    తిరిగి 2;  /* return false + error message */
  }
  లేకపోతే {
    lua_pushboolean(L, 1);
    lua_చొప్పించు(L, -(r + 1));
    తిరిగి r + 1;  /* return true + 'resume' returns */
  }
}


స్థిర పూర్ణాంకం luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  పూర్ణాంకం r = auxresume(L, co, lua_gettop(L));
  ఒకవేళ (l_unlikely(r < 0)) {  /* error? */
    పూర్ణాంకం stat = lua_స్థితి(co);
    ఒకవేళ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ఒకవేళ (stat == LUA_OK)
        తిరిగి 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ఒకవేళ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_చొప్పించు(L, -2);
      lua_concat(L, 2);
    }
    తిరిగి lua_error(L);  /* propagate error */
  }
  తిరిగి r;
}


స్థిర పూర్ణాంకం luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_yield (lua_State *L) {
  తిరిగి lua_yield(L, lua_gettop(L));
}


#నిర్వచించు COS_నిరంతరం		0
#నిర్వచించు COS_DEAD	1
#నిర్వచించు COS_YIELD	2
#నిర్వచించు COS_NORM	3


స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి statname[] =
  {"running", "dead", "suspended", "normal"};


స్థిర పూర్ణాంకం auxstatus (lua_State *L, lua_State *co) {
  ఒకవేళ (L == co) తిరిగి COS_నిరంతరం;
  లేకపోతే {
    ఎంపిక (lua_స్థితి(co)) {
      స్థితి LUA_YIELD:
        తిరిగి COS_YIELD;
      స్థితి LUA_OK: {
        lua_Debug ar;
        ఒకవేళ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          తిరిగి COS_NORM;  /* it is running */
        లేకపోతే ఒకవేళ (lua_gettop(co) == 0)
            తిరిగి COS_DEAD;
        లేకపోతే
          తిరిగి COS_YIELD;  /* initial state */
      }
      అప్రమేయ:  /* some error occurred */
        తిరిగి COS_DEAD;
    }
  }
}


స్థిర పూర్ణాంకం luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  తిరిగి 1;
}


స్థిర lua_State *getoptco (lua_State *L) {
  తిరిగి (lua_isnone(L, 1) ? L : getco(L));
}


స్థిర పూర్ణాంకం luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం luaB_corunning (lua_State *L) {
  పూర్ణాంకం ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  తిరిగి 2;
}


స్థిర పూర్ణాంకం luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  పూర్ణాంకం స్థితి_2 = auxstatus(L, co);
  ఎంపిక (స్థితి_2) {
    స్థితి COS_DEAD: స్థితి COS_YIELD: {
      స్థితి_2 = lua_closethread(co, L);
      ఒకవేళ (స్థితి_2 == LUA_OK) {
        lua_pushboolean(L, 1);
        తిరిగి 1;
      }
      లేకపోతే {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        తిరిగి 2;
      }
    }
    స్థితి COS_NORM:
      తిరిగి luaL_error(L, "cannot close a %s coroutine", statname[స్థితి_2]);
    స్థితి COS_నిరంతరం:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ఒకవేళ (lua_tothread(L, -1) == co)
        తిరిగి luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    అప్రమేయ:
      lua_assert(0);
      తిరిగి 0;
  }
}


స్థిర స్థిరరాశి luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {ఖాళీ, ఖాళీ}
};



LUAMOD_API పూర్ణాంకం luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  తిరిగి 1;
}

