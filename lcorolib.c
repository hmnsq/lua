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
  ཕྱིར co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static ཧྲིལ auxresume (lua_State *L, lua_State *co, ཧྲིལ narg) {
  ཧྲིལ status, nres;
  གལ་ཏེ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ཕྱིར -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  གལ་ཏེ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    གལ་ཏེ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ཕྱིར -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ཕྱིར nres;
  }
  དེ་མིན {
    lua_xmove(co, L, 1);  /* move error message */
    ཕྱིར -1;  /* error flag */
  }
}


static ཧྲིལ luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ཧྲིལ r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  གལ་ཏེ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ཕྱིར 2;  /* return false + error message */
  }
  དེ་མིན {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    ཕྱིར r + 1;  /* return true + 'resume' returns */
  }
}


static ཧྲིལ luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ཧྲིལ r = auxresume(L, co, lua_gettop(L));
  གལ་ཏེ (l_unlikely(r < 0)) {  /* error? */
    ཧྲིལ stat = lua_status(co);
    གལ་ཏེ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      གལ་ཏེ (stat == LUA_OK)
        ཕྱིར 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    གལ་ཏེ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    ཕྱིར lua_error(L);  /* propagate error */
  }
  ཕྱིར r;
}


static ཧྲིལ luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ཕྱིར 1;
}


static ཧྲིལ luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ཕྱིར 1;
}


static ཧྲིལ luaB_yield (lua_State *L) {
  ཕྱིར lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const var_na *const statname[] =
  {"running", "dead", "suspended", "normal"};


static ཧྲིལ auxstatus (lua_State *L, lua_State *co) {
  གལ་ཏེ (L == co) ཕྱིར COS_RUN;
  དེ་མིན {
    switch (lua_status(co)) {
      pak_sa LUA_YIELD:
        ཕྱིར COS_YIELD;
      pak_sa LUA_OK: {
        lua_Debug ar;
        གལ་ཏེ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ཕྱིར COS_NORM;  /* it is running */
        དེ་མིན གལ་ཏེ (lua_gettop(co) == 0)
            ཕྱིར COS_DEAD;
        དེ་མིན
          ཕྱིར COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        ཕྱིར COS_DEAD;
    }
  }
}


static ཧྲིལ luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ཕྱིར 1;
}


static lua_State *getoptco (lua_State *L) {
  ཕྱིར (lua_isnone(L, 1) ? L : getco(L));
}


static ཧྲིལ luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ཕྱིར 1;
}


static ཧྲིལ luaB_corunning (lua_State *L) {
  ཧྲིལ ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ཕྱིར 2;
}


static ཧྲིལ luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ཧྲིལ status = auxstatus(L, co);
  switch (status) {
    pak_sa COS_DEAD: pak_sa COS_YIELD: {
      status = lua_closethread(co, L);
      གལ་ཏེ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ཕྱིར 1;
      }
      དེ་མིན {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ཕྱིར 2;
      }
    }
    pak_sa COS_NORM:
      ཕྱིར luaL_error(L, "cannot close a %s coroutine", statname[status]);
    pak_sa COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      གལ་ཏེ (lua_tothread(L, -1) == co)
        ཕྱིར luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      ཕྱིར 0;
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



LUAMOD_API ཧྲིལ luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ཕྱིར 1;
}

