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
  trả_về co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static số_nguyên auxresume (lua_State *L, lua_State *co, số_nguyên narg) {
  số_nguyên status, nres;
  nếu (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    trả_về -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  nếu (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    nếu (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      trả_về -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    trả_về nres;
  }
  không_thì {
    lua_xmove(co, L, 1);  /* move error message */
    trả_về -1;  /* error flag */
  }
}


static số_nguyên luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  số_nguyên r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  nếu (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    trả_về 2;  /* return false + error message */
  }
  không_thì {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    trả_về r + 1;  /* return true + 'resume' returns */
  }
}


static số_nguyên luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  số_nguyên r = auxresume(L, co, lua_gettop(L));
  nếu (l_unlikely(r < 0)) {  /* error? */
    số_nguyên stat = lua_status(co);
    nếu (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      nếu (stat == LUA_OK)
        trả_về 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    nếu (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    trả_về lua_error(L);  /* propagate error */
  }
  trả_về r;
}


static số_nguyên luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  trả_về 1;
}


static số_nguyên luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  trả_về 1;
}


static số_nguyên luaB_yield (lua_State *L) {
  trả_về lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static số_nguyên auxstatus (lua_State *L, lua_State *co) {
  nếu (L == co) trả_về COS_RUN;
  không_thì {
    switch (lua_status(co)) {
      case LUA_YIELD:
        trả_về COS_YIELD;
      case LUA_OK: {
        lua_Debug ar;
        nếu (lua_getstack(co, 0, &ar))  /* does it have frames? */
          trả_về COS_NORM;  /* it is running */
        không_thì nếu (lua_gettop(co) == 0)
            trả_về COS_DEAD;
        không_thì
          trả_về COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        trả_về COS_DEAD;
    }
  }
}


static số_nguyên luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  trả_về 1;
}


static lua_State *getoptco (lua_State *L) {
  trả_về (lua_isnone(L, 1) ? L : getco(L));
}


static số_nguyên luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  trả_về 1;
}


static số_nguyên luaB_corunning (lua_State *L) {
  số_nguyên ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  trả_về 2;
}


static số_nguyên luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  số_nguyên status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = lua_closethread(co, L);
      nếu (status == LUA_OK) {
        lua_pushboolean(L, 1);
        trả_về 1;
      }
      không_thì {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        trả_về 2;
      }
    }
    case COS_NORM:
      trả_về luaL_error(L, "cannot close a %s coroutine", statname[status]);
    case COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      nếu (lua_tothread(L, -1) == co)
        trả_về luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      trả_về 0;
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



LUAMOD_API số_nguyên luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  trả_về 1;
}

