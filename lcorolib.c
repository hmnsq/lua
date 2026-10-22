/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lcorolib_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


ಸ್ಥಿರ lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  ಹಿಂತಿರುಗಿ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ auxresume (lua_State *L, lua_State *co, ಪೂರ್ಣಾಂಕ narg) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, nres;
  ಆದರೆ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ಹಿಂತಿರುಗಿ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  ಸ್ಥಿತಿ_2 = lua_resume(co, L, narg, &nres);
  ಆದರೆ (l_likely(ಸ್ಥಿತಿ_2 == LUA_OK || ಸ್ಥಿತಿ_2 == LUA_YIELD)) {
    ಆದರೆ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_ತೆಗೆ(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ಹಿಂತಿರುಗಿ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ಹಿಂತಿರುಗಿ nres;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_xmove(co, L, 1);  /* move error message */
    ಹಿಂತಿರುಗಿ -1;  /* error flag */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ಪೂರ್ಣಾಂಕ r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ಆದರೆ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_ಸೇರಿಸು(L, -2);
    ಹಿಂತಿರುಗಿ 2;  /* return false + error message */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushboolean(L, 1);
    lua_ಸೇರಿಸು(L, -(r + 1));
    ಹಿಂತಿರುಗಿ r + 1;  /* return true + 'resume' returns */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ಪೂರ್ಣಾಂಕ r = auxresume(L, co, lua_gettop(L));
  ಆದರೆ (l_unlikely(r < 0)) {  /* error? */
    ಪೂರ್ಣಾಂಕ stat = lua_ಸ್ಥಿತಿ(co);
    ಆದರೆ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ಆದರೆ (stat == LUA_OK)
        ಹಿಂತಿರುಗಿ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ಆದರೆ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_ಸೇರಿಸು(L, -2);
      lua_concat(L, 2);
    }
    ಹಿಂತಿರುಗಿ lua_error(L);  /* propagate error */
  }
  ಹಿಂತಿರುಗಿ r;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_yield (lua_State *L) {
  ಹಿಂತಿರುಗಿ lua_yield(L, lua_gettop(L));
}


#ವ್ಯಾಖ್ಯಾನಿಸು COS_ಸತತ		0
#ವ್ಯಾಖ್ಯಾನಿಸು COS_DEAD	1
#ವ್ಯಾಖ್ಯಾನಿಸು COS_YIELD	2
#ವ್ಯಾಖ್ಯಾನಿಸು COS_NORM	3


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ statname[] =
  {"running", "dead", "suspended", "normal"};


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ auxstatus (lua_State *L, lua_State *co) {
  ಆದರೆ (L == co) ಹಿಂತಿರುಗಿ COS_ಸತತ;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆಯ್ಕೆ (lua_ಸ್ಥಿತಿ(co)) {
      ಸ್ಥಿತಿ LUA_YIELD:
        ಹಿಂತಿರುಗಿ COS_YIELD;
      ಸ್ಥಿತಿ LUA_OK: {
        lua_Debug ar;
        ಆದರೆ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ಹಿಂತಿರುಗಿ COS_NORM;  /* it is running */
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lua_gettop(co) == 0)
            ಹಿಂತಿರುಗಿ COS_DEAD;
        ಇಲ್ಲದಿದ್ದರೆ
          ಹಿಂತಿರುಗಿ COS_YIELD;  /* initial state */
      }
      ಪೂರ್ವನಿಯೋಜಿತ:  /* some error occurred */
        ಹಿಂತಿರುಗಿ COS_DEAD;
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ lua_State *getoptco (lua_State *L) {
  ಹಿಂತಿರುಗಿ (lua_isnone(L, 1) ? L : getco(L));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_corunning (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ಹಿಂತಿರುಗಿ 2;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = auxstatus(L, co);
  ಆಯ್ಕೆ (ಸ್ಥಿತಿ_2) {
    ಸ್ಥಿತಿ COS_DEAD: ಸ್ಥಿತಿ COS_YIELD: {
      ಸ್ಥಿತಿ_2 = lua_closethread(co, L);
      ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK) {
        lua_pushboolean(L, 1);
        ಹಿಂತಿರುಗಿ 1;
      }
      ಇಲ್ಲದಿದ್ದರೆ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ಹಿಂತಿರುಗಿ 2;
      }
    }
    ಸ್ಥಿತಿ COS_NORM:
      ಹಿಂತಿರುಗಿ luaL_error(L, "cannot close a %s coroutine", statname[ಸ್ಥಿತಿ_2]);
    ಸ್ಥಿತಿ COS_ಸತತ:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ಆದರೆ (lua_tothread(L, -1) == co)
        ಹಿಂತಿರುಗಿ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    ಪೂರ್ವನಿಯೋಜಿತ:
      lua_assert(0);
      ಹಿಂತಿರುಗಿ 0;
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {ಖಾಲಿ, ಖಾಲಿ}
};



LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ಹಿಂತಿರುಗಿ 1;
}

