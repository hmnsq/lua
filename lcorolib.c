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
  ආපසු co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static පූර්ණ_සංඛ්‍යා auxresume (lua_State *L, lua_State *co, පූර්ණ_සංඛ්‍යා narg) {
  පූර්ණ_සංඛ්‍යා status, nres;
  නම් (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ආපසු -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  නම් (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    නම් (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ආපසු -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ආපසු nres;
  }
  නැතහොත් {
    lua_xmove(co, L, 1);  /* move error message */
    ආපසු -1;  /* error flag */
  }
}


static පූර්ණ_සංඛ්‍යා luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  පූර්ණ_සංඛ්‍යා r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  නම් (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ආපසු 2;  /* return false + error message */
  }
  නැතහොත් {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    ආපසු r + 1;  /* return true + 'resume' returns */
  }
}


static පූර්ණ_සංඛ්‍යා luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  පූර්ණ_සංඛ්‍යා r = auxresume(L, co, lua_gettop(L));
  නම් (l_unlikely(r < 0)) {  /* error? */
    පූර්ණ_සංඛ්‍යා stat = lua_status(co);
    නම් (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      නම් (stat == LUA_OK)
        ආපසු 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    නම් (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    ආපසු lua_error(L);  /* propagate error */
  }
  ආපසු r;
}


static පූර්ණ_සංඛ්‍යා luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා luaB_yield (lua_State *L) {
  ආපසු lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const var_na *const statname[] =
  {"running", "dead", "suspended", "normal"};


static පූර්ණ_සංඛ්‍යා auxstatus (lua_State *L, lua_State *co) {
  නම් (L == co) ආපසු COS_RUN;
  නැතහොත් {
    switch (lua_status(co)) {
      pak_sa LUA_YIELD:
        ආපසු COS_YIELD;
      pak_sa LUA_OK: {
        lua_Debug ar;
        නම් (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ආපසු COS_NORM;  /* it is running */
        නැතහොත් නම් (lua_gettop(co) == 0)
            ආපසු COS_DEAD;
        නැතහොත්
          ආපසු COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        ආපසු COS_DEAD;
    }
  }
}


static පූර්ණ_සංඛ්‍යා luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ආපසු 1;
}


static lua_State *getoptco (lua_State *L) {
  ආපසු (lua_isnone(L, 1) ? L : getco(L));
}


static පූර්ණ_සංඛ්‍යා luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා luaB_corunning (lua_State *L) {
  පූර්ණ_සංඛ්‍යා ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ආපසු 2;
}


static පූර්ණ_සංඛ්‍යා luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  පූර්ණ_සංඛ්‍යා status = auxstatus(L, co);
  switch (status) {
    pak_sa COS_DEAD: pak_sa COS_YIELD: {
      status = lua_closethread(co, L);
      නම් (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ආපසු 1;
      }
      නැතහොත් {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ආපසු 2;
      }
    }
    pak_sa COS_NORM:
      ආපසු luaL_error(L, "cannot close a %s coroutine", statname[status]);
    pak_sa COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      නම් (lua_tothread(L, -1) == co)
        ආපසු luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      ආපසු 0;
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



LUAMOD_API පූර්ණ_සංඛ්‍යා luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ආපසු 1;
}

