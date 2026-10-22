/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"


ಸ್ಥಿರ lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  ಪ್ರತ್ಯಾಗಮ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ auxresume (lua_State *L, lua_State *co, ಪೂರ್ಣಾಙ್ಕ narg) {
  ಪೂರ್ಣಾಙ್ಕ status, nres;
  ಯದಿ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ಪ್ರತ್ಯಾಗಮ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  ಯದಿ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    ಯದಿ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_ತೆಗೆ(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ಪ್ರತ್ಯಾಗಮ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ಪ್ರತ್ಯಾಗಮ nres;
  }
  ಅನ್ಯಥಾ {
    lua_xmove(co, L, 1);  /* move error message */
    ಪ್ರತ್ಯಾಗಮ -1;  /* error flag */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ಪೂರ್ಣಾಙ್ಕ r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ಯದಿ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_ಸೇರಿಸು(L, -2);
    ಪ್ರತ್ಯಾಗಮ 2;  /* return false + error message */
  }
  ಅನ್ಯಥಾ {
    lua_pushboolean(L, 1);
    lua_ಸೇರಿಸು(L, -(r + 1));
    ಪ್ರತ್ಯಾಗಮ r + 1;  /* return true + 'resume' returns */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ಪೂರ್ಣಾಙ್ಕ r = auxresume(L, co, lua_gettop(L));
  ಯದಿ (l_unlikely(r < 0)) {  /* error? */
    ಪೂರ್ಣಾಙ್ಕ stat = lua_status(co);
    ಯದಿ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ಯದಿ (stat == LUA_OK)
        ಪ್ರತ್ಯಾಗಮ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ಯದಿ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_ಸೇರಿಸು(L, -2);
      lua_concat(L, 2);
    }
    ಪ್ರತ್ಯಾಗಮ lua_error(L);  /* propagate error */
  }
  ಪ್ರತ್ಯಾಗಮ r;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_yield (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ statname[] =
  {"running", "dead", "suspended", "normal"};


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ auxstatus (lua_State *L, lua_State *co) {
  ಯದಿ (L == co) ಪ್ರತ್ಯಾಗಮ COS_RUN;
  ಅನ್ಯಥಾ {
    ವಿಕಲ್ಪ (lua_status(co)) {
      ಪಕ್ಷ LUA_YIELD:
        ಪ್ರತ್ಯಾಗಮ COS_YIELD;
      ಪಕ್ಷ LUA_OK: {
        lua_Debug ar;
        ಯದಿ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ಪ್ರತ್ಯಾಗಮ COS_NORM;  /* it is running */
        ಅನ್ಯಥಾ ಯದಿ (lua_gettop(co) == 0)
            ಪ್ರತ್ಯಾಗಮ COS_DEAD;
        ಅನ್ಯಥಾ
          ಪ್ರತ್ಯಾಗಮ COS_YIELD;  /* initial state */
      }
      ಸ್ವಭಾವ:  /* some error occurred */
        ಪ್ರತ್ಯಾಗಮ COS_DEAD;
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ lua_State *getoptco (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ (lua_isnone(L, 1) ? L : getco(L));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_corunning (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ಪ್ರತ್ಯಾಗಮ 2;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ಪೂರ್ಣಾಙ್ಕ status = auxstatus(L, co);
  ವಿಕಲ್ಪ (status) {
    ಪಕ್ಷ COS_DEAD: ಪಕ್ಷ COS_YIELD: {
      status = lua_closethread(co, L);
      ಯದಿ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಅನ್ಯಥಾ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ಪ್ರತ್ಯಾಗಮ 2;
      }
    }
    ಪಕ್ಷ COS_NORM:
      ಪ್ರತ್ಯಾಗಮ luaL_error(L, "cannot close a %s coroutine", statname[status]);
    ಪಕ್ಷ COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ಯದಿ (lua_tothread(L, -1) == co)
        ಪ್ರತ್ಯಾಗಮ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    ಸ್ವಭಾವ:
      lua_assert(0);
      ಪ್ರತ್ಯಾಗಮ 0;
  }
}


ಸ್ಥಿರ ಧ್ರುವ luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};



LUAMOD_API ಪೂರ್ಣಾಙ್ಕ luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ಪ್ರತ್ಯಾಗಮ 1;
}

