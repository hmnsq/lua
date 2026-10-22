/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lcorolib_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_LIB

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"


ਸਥਿਰ lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  ਵਾਪਸ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ਸਥਿਰ ਪੂਰਨ auxresume (lua_State *L, lua_State *co, ਪੂਰਨ narg) {
  ਪੂਰਨ ਸਥਿਤੀ, nres;
  ਜੇ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ਵਾਪਸ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  ਸਥਿਤੀ = lua_resume(co, L, narg, &nres);
  ਜੇ (l_likely(ਸਥਿਤੀ == LUA_OK || ਸਥਿਤੀ == LUA_YIELD)) {
    ਜੇ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_ਕੱਢੋ(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ਵਾਪਸ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ਵਾਪਸ nres;
  }
  ਨਹੀਂਤਾਂ {
    lua_xmove(co, L, 1);  /* move error message */
    ਵਾਪਸ -1;  /* error flag */
  }
}


ਸਥਿਰ ਪੂਰਨ luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ਪੂਰਨ r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ਜੇ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_ਪਾਓ(L, -2);
    ਵਾਪਸ 2;  /* return false + error message */
  }
  ਨਹੀਂਤਾਂ {
    lua_pushboolean(L, 1);
    lua_ਪਾਓ(L, -(r + 1));
    ਵਾਪਸ r + 1;  /* return true + 'resume' returns */
  }
}


ਸਥਿਰ ਪੂਰਨ luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ਪੂਰਨ r = auxresume(L, co, lua_gettop(L));
  ਜੇ (l_unlikely(r < 0)) {  /* error? */
    ਪੂਰਨ stat = lua_ਸਥਿਤੀ(co);
    ਜੇ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ਜੇ (stat == LUA_OK)
        ਵਾਪਸ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ਜੇ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_ਪਾਓ(L, -2);
      lua_concat(L, 2);
    }
    ਵਾਪਸ lua_error(L);  /* propagate error */
  }
  ਵਾਪਸ r;
}


ਸਥਿਰ ਪੂਰਨ luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ luaB_yield (lua_State *L) {
  ਵਾਪਸ lua_yield(L, lua_gettop(L));
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COS_ਲੜੀਵਾਰ		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COS_DEAD	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COS_YIELD	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COS_NORM	3


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ statname[] =
  {"running", "dead", "suspended", "normal"};


ਸਥਿਰ ਪੂਰਨ auxstatus (lua_State *L, lua_State *co) {
  ਜੇ (L == co) ਵਾਪਸ COS_ਲੜੀਵਾਰ;
  ਨਹੀਂਤਾਂ {
    ਚੋਣ (lua_ਸਥਿਤੀ(co)) {
      ਹਾਲਤ LUA_YIELD:
        ਵਾਪਸ COS_YIELD;
      ਹਾਲਤ LUA_OK: {
        lua_Debug ar;
        ਜੇ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ਵਾਪਸ COS_NORM;  /* it is running */
        ਨਹੀਂਤਾਂ ਜੇ (lua_gettop(co) == 0)
            ਵਾਪਸ COS_DEAD;
        ਨਹੀਂਤਾਂ
          ਵਾਪਸ COS_YIELD;  /* initial state */
      }
      ਮੂਲ:  /* some error occurred */
        ਵਾਪਸ COS_DEAD;
    }
  }
}


ਸਥਿਰ ਪੂਰਨ luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ਵਾਪਸ 1;
}


ਸਥਿਰ lua_State *getoptco (lua_State *L) {
  ਵਾਪਸ (lua_isnone(L, 1) ? L : getco(L));
}


ਸਥਿਰ ਪੂਰਨ luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ luaB_corunning (lua_State *L) {
  ਪੂਰਨ ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ਵਾਪਸ 2;
}


ਸਥਿਰ ਪੂਰਨ luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ਪੂਰਨ ਸਥਿਤੀ = auxstatus(L, co);
  ਚੋਣ (ਸਥਿਤੀ) {
    ਹਾਲਤ COS_DEAD: ਹਾਲਤ COS_YIELD: {
      ਸਥਿਤੀ = lua_closethread(co, L);
      ਜੇ (ਸਥਿਤੀ == LUA_OK) {
        lua_pushboolean(L, 1);
        ਵਾਪਸ 1;
      }
      ਨਹੀਂਤਾਂ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ਵਾਪਸ 2;
      }
    }
    ਹਾਲਤ COS_NORM:
      ਵਾਪਸ luaL_error(L, "cannot close a %s coroutine", statname[ਸਥਿਤੀ]);
    ਹਾਲਤ COS_ਲੜੀਵਾਰ:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ਜੇ (lua_tothread(L, -1) == co)
        ਵਾਪਸ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    ਮੂਲ:
      lua_assert(0);
      ਵਾਪਸ 0;
  }
}


ਸਥਿਰ ਅਚਲ luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {ਨਿਰਾ, ਨਿਰਾ}
};



LUAMOD_API ਪੂਰਨ luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ਵਾਪਸ 1;
}

