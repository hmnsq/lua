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
  반환 co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static 정수 auxresume (lua_State *L, lua_State *co, 정수 narg) {
  정수 status, nres;
  만약 (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    반환 -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  만약 (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    만약 (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      반환 -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    반환 nres;
  }
  아니면 {
    lua_xmove(co, L, 1);  /* move error message */
    반환 -1;  /* error flag */
  }
}


static 정수 luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  정수 r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  만약 (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    반환 2;  /* return false + error message */
  }
  아니면 {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    반환 r + 1;  /* return true + 'resume' returns */
  }
}


static 정수 luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  정수 r = auxresume(L, co, lua_gettop(L));
  만약 (l_unlikely(r < 0)) {  /* error? */
    정수 stat = lua_status(co);
    만약 (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      만약 (stat == LUA_OK)
        반환 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    만약 (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    반환 lua_error(L);  /* propagate error */
  }
  반환 r;
}


static 정수 luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  반환 1;
}


static 정수 luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  반환 1;
}


static 정수 luaB_yield (lua_State *L) {
  반환 lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static 정수 auxstatus (lua_State *L, lua_State *co) {
  만약 (L == co) 반환 COS_RUN;
  아니면 {
    switch (lua_status(co)) {
      case LUA_YIELD:
        반환 COS_YIELD;
      case LUA_OK: {
        lua_Debug ar;
        만약 (lua_getstack(co, 0, &ar))  /* does it have frames? */
          반환 COS_NORM;  /* it is running */
        아니면 만약 (lua_gettop(co) == 0)
            반환 COS_DEAD;
        아니면
          반환 COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        반환 COS_DEAD;
    }
  }
}


static 정수 luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  반환 1;
}


static lua_State *getoptco (lua_State *L) {
  반환 (lua_isnone(L, 1) ? L : getco(L));
}


static 정수 luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  반환 1;
}


static 정수 luaB_corunning (lua_State *L) {
  정수 ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  반환 2;
}


static 정수 luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  정수 status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = lua_closethread(co, L);
      만약 (status == LUA_OK) {
        lua_pushboolean(L, 1);
        반환 1;
      }
      아니면 {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        반환 2;
      }
    }
    case COS_NORM:
      반환 luaL_error(L, "cannot close a %s coroutine", statname[status]);
    case COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      만약 (lua_tothread(L, -1) == co)
        반환 luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      반환 0;
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



LUAMOD_API 정수 luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  반환 1;
}

