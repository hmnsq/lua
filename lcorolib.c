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
  คืน co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static จำนวนเต็ม auxresume (lua_State *L, lua_State *co, จำนวนเต็ม narg) {
  จำนวนเต็ม status, nres;
  ถ้า (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    คืน -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  ถ้า (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    ถ้า (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      คืน -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    คืน nres;
  }
  มิฉะนั้น {
    lua_xmove(co, L, 1);  /* move error message */
    คืน -1;  /* error flag */
  }
}


static จำนวนเต็ม luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  จำนวนเต็ม r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ถ้า (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    คืน 2;  /* return false + error message */
  }
  มิฉะนั้น {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    คืน r + 1;  /* return true + 'resume' returns */
  }
}


static จำนวนเต็ม luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  จำนวนเต็ม r = auxresume(L, co, lua_gettop(L));
  ถ้า (l_unlikely(r < 0)) {  /* error? */
    จำนวนเต็ม stat = lua_status(co);
    ถ้า (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ถ้า (stat == LUA_OK)
        คืน 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ถ้า (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    คืน lua_error(L);  /* propagate error */
  }
  คืน r;
}


static จำนวนเต็ม luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  คืน 1;
}


static จำนวนเต็ม luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  คืน 1;
}


static จำนวนเต็ม luaB_yield (lua_State *L) {
  คืน lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const var_na *const statname[] =
  {"running", "dead", "suspended", "normal"};


static จำนวนเต็ม auxstatus (lua_State *L, lua_State *co) {
  ถ้า (L == co) คืน COS_RUN;
  มิฉะนั้น {
    switch (lua_status(co)) {
      pak_sa LUA_YIELD:
        คืน COS_YIELD;
      pak_sa LUA_OK: {
        lua_Debug ar;
        ถ้า (lua_getstack(co, 0, &ar))  /* does it have frames? */
          คืน COS_NORM;  /* it is running */
        มิฉะนั้น ถ้า (lua_gettop(co) == 0)
            คืน COS_DEAD;
        มิฉะนั้น
          คืน COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        คืน COS_DEAD;
    }
  }
}


static จำนวนเต็ม luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  คืน 1;
}


static lua_State *getoptco (lua_State *L) {
  คืน (lua_isnone(L, 1) ? L : getco(L));
}


static จำนวนเต็ม luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  คืน 1;
}


static จำนวนเต็ม luaB_corunning (lua_State *L) {
  จำนวนเต็ม ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  คืน 2;
}


static จำนวนเต็ม luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  จำนวนเต็ม status = auxstatus(L, co);
  switch (status) {
    pak_sa COS_DEAD: pak_sa COS_YIELD: {
      status = lua_closethread(co, L);
      ถ้า (status == LUA_OK) {
        lua_pushboolean(L, 1);
        คืน 1;
      }
      มิฉะนั้น {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        คืน 2;
      }
    }
    pak_sa COS_NORM:
      คืน luaL_error(L, "cannot close a %s coroutine", statname[status]);
    pak_sa COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ถ้า (lua_tothread(L, -1) == co)
        คืน luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      คืน 0;
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



LUAMOD_API จำนวนเต็ม luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  คืน 1;
}

