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
  повернути co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static ціле auxresume (lua_State *L, lua_State *co, ціле narg) {
  ціле status, nres;
  якщо (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    повернути -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  якщо (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    якщо (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      повернути -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    повернути nres;
  }
  інакше {
    lua_xmove(co, L, 1);  /* move error message */
    повернути -1;  /* error flag */
  }
}


static ціле luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ціле r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  якщо (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    повернути 2;  /* return false + error message */
  }
  інакше {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    повернути r + 1;  /* return true + 'resume' returns */
  }
}


static ціле luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ціле r = auxresume(L, co, lua_gettop(L));
  якщо (l_unlikely(r < 0)) {  /* error? */
    ціле stat = lua_status(co);
    якщо (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      якщо (stat == LUA_OK)
        повернути 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    якщо (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    повернути lua_error(L);  /* propagate error */
  }
  повернути r;
}


static ціле luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  повернути 1;
}


static ціле luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  повернути 1;
}


static ціле luaB_yield (lua_State *L) {
  повернути lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static ціле auxstatus (lua_State *L, lua_State *co) {
  якщо (L == co) повернути COS_RUN;
  інакше {
    switch (lua_status(co)) {
      case LUA_YIELD:
        повернути COS_YIELD;
      case LUA_OK: {
        lua_Debug ar;
        якщо (lua_getstack(co, 0, &ar))  /* does it have frames? */
          повернути COS_NORM;  /* it is running */
        інакше якщо (lua_gettop(co) == 0)
            повернути COS_DEAD;
        інакше
          повернути COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        повернути COS_DEAD;
    }
  }
}


static ціле luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  повернути 1;
}


static lua_State *getoptco (lua_State *L) {
  повернути (lua_isnone(L, 1) ? L : getco(L));
}


static ціле luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  повернути 1;
}


static ціле luaB_corunning (lua_State *L) {
  ціле ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  повернути 2;
}


static ціле luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ціле status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = lua_closethread(co, L);
      якщо (status == LUA_OK) {
        lua_pushboolean(L, 1);
        повернути 1;
      }
      інакше {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        повернути 2;
      }
    }
    case COS_NORM:
      повернути luaL_error(L, "cannot close a %s coroutine", statname[status]);
    case COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      якщо (lua_tothread(L, -1) == co)
        повернути luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      повернути 0;
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



LUAMOD_API ціле luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  повернути 1;
}

