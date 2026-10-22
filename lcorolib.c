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
  戻る co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static 整数 auxresume (lua_State *L, lua_State *co, 整数 narg) {
  整数 status, nres;
  もし (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    戻る -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  もし (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    もし (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      戻る -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    戻る nres;
  }
  否则 {
    lua_xmove(co, L, 1);  /* move error message */
    戻る -1;  /* error flag */
  }
}


static 整数 luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  整数 r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  もし (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    戻る 2;  /* return false + error message */
  }
  否则 {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    戻る r + 1;  /* return true + 'resume' returns */
  }
}


static 整数 luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  整数 r = auxresume(L, co, lua_gettop(L));
  もし (l_unlikely(r < 0)) {  /* error? */
    整数 stat = lua_status(co);
    もし (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      もし (stat == LUA_OK)
        戻る 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    もし (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    戻る lua_error(L);  /* propagate error */
  }
  戻る r;
}


static 整数 luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  戻る 1;
}


static 整数 luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  戻る 1;
}


static 整数 luaB_yield (lua_State *L) {
  戻る lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static 整数 auxstatus (lua_State *L, lua_State *co) {
  もし (L == co) 戻る COS_RUN;
  否则 {
    switch (lua_status(co)) {
      case LUA_YIELD:
        戻る COS_YIELD;
      case LUA_OK: {
        lua_Debug ar;
        もし (lua_getstack(co, 0, &ar))  /* does it have frames? */
          戻る COS_NORM;  /* it is running */
        否则 もし (lua_gettop(co) == 0)
            戻る COS_DEAD;
        否则
          戻る COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        戻る COS_DEAD;
    }
  }
}


static 整数 luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  戻る 1;
}


static lua_State *getoptco (lua_State *L) {
  戻る (lua_isnone(L, 1) ? L : getco(L));
}


static 整数 luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  戻る 1;
}


static 整数 luaB_corunning (lua_State *L) {
  整数 ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  戻る 2;
}


static 整数 luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  整数 status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = lua_closethread(co, L);
      もし (status == LUA_OK) {
        lua_pushboolean(L, 1);
        戻る 1;
      }
      否则 {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        戻る 2;
      }
    }
    case COS_NORM:
      戻る luaL_error(L, "cannot close a %s coroutine", statname[status]);
    case COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      もし (lua_tothread(L, -1) == co)
        戻る luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      戻る 0;
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



LUAMOD_API 整数 luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  戻る 1;
}

