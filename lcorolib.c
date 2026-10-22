/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#घोषणा lcorolib_c
#घोषणा LUA_LIB

#समावेश "lprefix.h"


#समावेश <stdlib.h>

#समावेश "lua.h"

#समावेश "lauxlib.h"
#समावेश "lualib.h"
#समावेश "llimits.h"


अटल lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  वापस co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
अटल पूर्णांक auxresume (lua_State *L, lua_State *co, पूर्णांक narg) {
  पूर्णांक status, nres;
  अगर (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    वापस -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  अगर (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    अगर (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_निकालो(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      वापस -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    वापस nres;
  }
  अन्यथा {
    lua_xmove(co, L, 1);  /* move error message */
    वापस -1;  /* error flag */
  }
}


अटल पूर्णांक luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  पूर्णांक r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  अगर (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_जोड़ो(L, -2);
    वापस 2;  /* return false + error message */
  }
  अन्यथा {
    lua_pushboolean(L, 1);
    lua_जोड़ो(L, -(r + 1));
    वापस r + 1;  /* return true + 'resume' returns */
  }
}


अटल पूर्णांक luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  पूर्णांक r = auxresume(L, co, lua_gettop(L));
  अगर (l_unlikely(r < 0)) {  /* error? */
    पूर्णांक stat = lua_status(co);
    अगर (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      अगर (stat == LUA_OK)
        वापस 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    अगर (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_जोड़ो(L, -2);
      lua_concat(L, 2);
    }
    वापस lua_error(L);  /* propagate error */
  }
  वापस r;
}


अटल पूर्णांक luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  वापस 1;
}


अटल पूर्णांक luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  वापस 1;
}


अटल पूर्णांक luaB_yield (lua_State *L) {
  वापस lua_yield(L, lua_gettop(L));
}


#घोषणा COS_RUN		0
#घोषणा COS_DEAD	1
#घोषणा COS_YIELD	2
#घोषणा COS_NORM	3


अटल स्थिर अक्षर *स्थिर statname[] =
  {"running", "dead", "suspended", "normal"};


अटल पूर्णांक auxstatus (lua_State *L, lua_State *co) {
  अगर (L == co) वापस COS_RUN;
  अन्यथा {
    चयन (lua_status(co)) {
      हाल LUA_YIELD:
        वापस COS_YIELD;
      हाल LUA_OK: {
        lua_Debug ar;
        अगर (lua_getstack(co, 0, &ar))  /* does it have frames? */
          वापस COS_NORM;  /* it is running */
        अन्यथा अगर (lua_gettop(co) == 0)
            वापस COS_DEAD;
        अन्यथा
          वापस COS_YIELD;  /* initial state */
      }
      शेष:  /* some error occurred */
        वापस COS_DEAD;
    }
  }
}


अटल पूर्णांक luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  वापस 1;
}


अटल lua_State *getoptco (lua_State *L) {
  वापस (lua_isnone(L, 1) ? L : getco(L));
}


अटल पूर्णांक luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  वापस 1;
}


अटल पूर्णांक luaB_corunning (lua_State *L) {
  पूर्णांक ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  वापस 2;
}


अटल पूर्णांक luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  पूर्णांक status = auxstatus(L, co);
  चयन (status) {
    हाल COS_DEAD: हाल COS_YIELD: {
      status = lua_closethread(co, L);
      अगर (status == LUA_OK) {
        lua_pushboolean(L, 1);
        वापस 1;
      }
      अन्यथा {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        वापस 2;
      }
    }
    हाल COS_NORM:
      वापस luaL_error(L, "cannot close a %s coroutine", statname[status]);
    हाल COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      अगर (lua_tothread(L, -1) == co)
        वापस luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    शेष:
      lua_assert(0);
      वापस 0;
  }
}


अटल स्थिर luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {शून्य, शून्य}
};



LUAMOD_API पूर्णांक luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  वापस 1;
}

