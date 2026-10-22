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
  вернуть co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static целое auxresume (lua_State *L, lua_State *co, целое narg) {
  целое status, nres;
  если (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    вернуть -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  если (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    если (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      вернуть -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    вернуть nres;
  }
  иначе {
    lua_xmove(co, L, 1);  /* move error message */
    вернуть -1;  /* error flag */
  }
}


static целое luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  целое r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  если (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    вернуть 2;  /* return false + error message */
  }
  иначе {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    вернуть r + 1;  /* return true + 'resume' returns */
  }
}


static целое luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  целое r = auxresume(L, co, lua_gettop(L));
  если (l_unlikely(r < 0)) {  /* error? */
    целое stat = lua_status(co);
    если (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      если (stat == LUA_OK)
        вернуть 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    если (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    вернуть lua_error(L);  /* propagate error */
  }
  вернуть r;
}


static целое luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  вернуть 1;
}


static целое luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  вернуть 1;
}


static целое luaB_yield (lua_State *L) {
  вернуть lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static целое auxstatus (lua_State *L, lua_State *co) {
  если (L == co) вернуть COS_RUN;
  иначе {
    switch (lua_status(co)) {
      case LUA_YIELD:
        вернуть COS_YIELD;
      case LUA_OK: {
        lua_Debug ar;
        если (lua_getstack(co, 0, &ar))  /* does it have frames? */
          вернуть COS_NORM;  /* it is running */
        иначе если (lua_gettop(co) == 0)
            вернуть COS_DEAD;
        иначе
          вернуть COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        вернуть COS_DEAD;
    }
  }
}


static целое luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  вернуть 1;
}


static lua_State *getoptco (lua_State *L) {
  вернуть (lua_isnone(L, 1) ? L : getco(L));
}


static целое luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  вернуть 1;
}


static целое luaB_corunning (lua_State *L) {
  целое ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  вернуть 2;
}


static целое luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  целое status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = lua_closethread(co, L);
      если (status == LUA_OK) {
        lua_pushboolean(L, 1);
        вернуть 1;
      }
      иначе {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        вернуть 2;
      }
    }
    case COS_NORM:
      вернуть luaL_error(L, "cannot close a %s coroutine", statname[status]);
    case COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      если (lua_tothread(L, -1) == co)
        вернуть luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      вернуть 0;
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



LUAMOD_API целое luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  вернуть 1;
}

