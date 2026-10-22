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
  ត្រឡប់ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static ចំនួនគត់ auxresume (lua_State *L, lua_State *co, ចំនួនគត់ narg) {
  ចំនួនគត់ status, nres;
  ប្រសិនបើ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ត្រឡប់ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  ប្រសិនបើ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    ប្រសិនបើ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ត្រឡប់ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ត្រឡប់ nres;
  }
  បើមិនដូច្នេះ {
    lua_xmove(co, L, 1);  /* move error message */
    ត្រឡប់ -1;  /* error flag */
  }
}


static ចំនួនគត់ luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ចំនួនគត់ r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ប្រសិនបើ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ត្រឡប់ 2;  /* return false + error message */
  }
  បើមិនដូច្នេះ {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    ត្រឡប់ r + 1;  /* return true + 'resume' returns */
  }
}


static ចំនួនគត់ luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ចំនួនគត់ r = auxresume(L, co, lua_gettop(L));
  ប្រសិនបើ (l_unlikely(r < 0)) {  /* error? */
    ចំនួនគត់ stat = lua_status(co);
    ប្រសិនបើ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ប្រសិនបើ (stat == LUA_OK)
        ត្រឡប់ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ប្រសិនបើ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    ត្រឡប់ lua_error(L);  /* propagate error */
  }
  ត្រឡប់ r;
}


static ចំនួនគត់ luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ត្រឡប់ 1;
}


static ចំនួនគត់ luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ត្រឡប់ 1;
}


static ចំនួនគត់ luaB_yield (lua_State *L) {
  ត្រឡប់ lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const var_na *const statname[] =
  {"running", "dead", "suspended", "normal"};


static ចំនួនគត់ auxstatus (lua_State *L, lua_State *co) {
  ប្រសិនបើ (L == co) ត្រឡប់ COS_RUN;
  បើមិនដូច្នេះ {
    switch (lua_status(co)) {
      pak_sa LUA_YIELD:
        ត្រឡប់ COS_YIELD;
      pak_sa LUA_OK: {
        lua_Debug ar;
        ប្រសិនបើ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ត្រឡប់ COS_NORM;  /* it is running */
        បើមិនដូច្នេះ ប្រសិនបើ (lua_gettop(co) == 0)
            ត្រឡប់ COS_DEAD;
        បើមិនដូច្នេះ
          ត្រឡប់ COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        ត្រឡប់ COS_DEAD;
    }
  }
}


static ចំនួនគត់ luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ត្រឡប់ 1;
}


static lua_State *getoptco (lua_State *L) {
  ត្រឡប់ (lua_isnone(L, 1) ? L : getco(L));
}


static ចំនួនគត់ luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ត្រឡប់ 1;
}


static ចំនួនគត់ luaB_corunning (lua_State *L) {
  ចំនួនគត់ ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ត្រឡប់ 2;
}


static ចំនួនគត់ luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ចំនួនគត់ status = auxstatus(L, co);
  switch (status) {
    pak_sa COS_DEAD: pak_sa COS_YIELD: {
      status = lua_closethread(co, L);
      ប្រសិនបើ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ត្រឡប់ 1;
      }
      បើមិនដូច្នេះ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ត្រឡប់ 2;
      }
    }
    pak_sa COS_NORM:
      ត្រឡប់ luaL_error(L, "cannot close a %s coroutine", statname[status]);
    pak_sa COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ប្រសិនបើ (lua_tothread(L, -1) == co)
        ត្រឡប់ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      ត្រឡប់ 0;
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



LUAMOD_API ចំនួនគត់ luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ត្រឡប់ 1;
}

