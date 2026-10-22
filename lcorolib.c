/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#include "lprefix.h"


#include <stdlib.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "llimits.h"


static lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  ꦮꦭꦶ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static puur_naa_nka auxresume (lua_State *L, lua_State *co, puur_naa_nka narg) {
  puur_naa_nka status, nres;
  ꦪꦼꦤ꧀ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ꦮꦭꦶ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  ꦪꦼꦤ꧀ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    ꦪꦼꦤ꧀ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ꦮꦭꦶ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ꦮꦭꦶ nres;
  }
  ꦭꦶꦪ {
    lua_xmove(co, L, 1);  /* move error message */
    ꦮꦭꦶ -1;  /* error flag */
  }
}


static puur_naa_nka luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  puur_naa_nka r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ꦪꦼꦤ꧀ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ꦮꦭꦶ 2;  /* return false + error message */
  }
  ꦭꦶꦪ {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    ꦮꦭꦶ r + 1;  /* return true + 'resume' returns */
  }
}


static puur_naa_nka luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  puur_naa_nka r = auxresume(L, co, lua_gettop(L));
  ꦪꦼꦤ꧀ (l_unlikely(r < 0)) {  /* error? */
    puur_naa_nka stat = lua_status(co);
    ꦪꦼꦤ꧀ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ꦪꦼꦤ꧀ (stat == LUA_OK)
        ꦮꦭꦶ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ꦪꦼꦤ꧀ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    ꦮꦭꦶ lua_error(L);  /* propagate error */
  }
  ꦮꦭꦶ r;
}


static puur_naa_nka luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_yield (lua_State *L) {
  ꦮꦭꦶ lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const var_na *const statname[] =
  {"running", "dead", "suspended", "normal"};


static puur_naa_nka auxstatus (lua_State *L, lua_State *co) {
  ꦪꦼꦤ꧀ (L == co) ꦮꦭꦶ COS_RUN;
  ꦭꦶꦪ {
    switch (lua_status(co)) {
      pak_sa LUA_YIELD:
        ꦮꦭꦶ COS_YIELD;
      pak_sa LUA_OK: {
        lua_Debug ar;
        ꦪꦼꦤ꧀ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ꦮꦭꦶ COS_NORM;  /* it is running */
        ꦭꦶꦪ ꦪꦼꦤ꧀ (lua_gettop(co) == 0)
            ꦮꦭꦶ COS_DEAD;
        ꦭꦶꦪ
          ꦮꦭꦶ COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        ꦮꦭꦶ COS_DEAD;
    }
  }
}


static puur_naa_nka luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ꦮꦭꦶ 1;
}


static lua_State *getoptco (lua_State *L) {
  ꦮꦭꦶ (lua_isnone(L, 1) ? L : getco(L));
}


static puur_naa_nka luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ꦮꦭꦶ 1;
}


static puur_naa_nka luaB_corunning (lua_State *L) {
  puur_naa_nka ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ꦮꦭꦶ 2;
}


static puur_naa_nka luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  puur_naa_nka status = auxstatus(L, co);
  switch (status) {
    pak_sa COS_DEAD: pak_sa COS_YIELD: {
      status = lua_closethread(co, L);
      ꦪꦼꦤ꧀ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ꦮꦭꦶ 1;
      }
      ꦭꦶꦪ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ꦮꦭꦶ 2;
      }
    }
    pak_sa COS_NORM:
      ꦮꦭꦶ luaL_error(L, "cannot close a %s coroutine", statname[status]);
    pak_sa COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ꦪꦼꦤ꧀ (lua_tothread(L, -1) == co)
        ꦮꦭꦶ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      ꦮꦭꦶ 0;
  }
}


static const luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {NULL, NULL}
};



LUAMOD_API puur_naa_nka luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ꦮꦭꦶ 1;
}

