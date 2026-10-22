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
  ပြန်ပေး co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static ကိန်းပြည့် auxresume (lua_State *L, lua_State *co, ကိန်းပြည့် narg) {
  ကိန်းပြည့် status, nres;
  အကယ်၍ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ပြန်ပေး -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  အကယ်၍ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    အကယ်၍ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ပြန်ပေး -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ပြန်ပေး nres;
  }
  မဟုတ်ပါက {
    lua_xmove(co, L, 1);  /* move error message */
    ပြန်ပေး -1;  /* error flag */
  }
}


static ကိန်းပြည့် luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ကိန်းပြည့် r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  အကယ်၍ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ပြန်ပေး 2;  /* return false + error message */
  }
  မဟုတ်ပါက {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    ပြန်ပေး r + 1;  /* return true + 'resume' returns */
  }
}


static ကိန်းပြည့် luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ကိန်းပြည့် r = auxresume(L, co, lua_gettop(L));
  အကယ်၍ (l_unlikely(r < 0)) {  /* error? */
    ကိန်းပြည့် stat = lua_status(co);
    အကယ်၍ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      အကယ်၍ (stat == LUA_OK)
        ပြန်ပေး 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    အကယ်၍ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    ပြန်ပေး lua_error(L);  /* propagate error */
  }
  ပြန်ပေး r;
}


static ကိန်းပြည့် luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ပြန်ပေး 1;
}


static ကိန်းပြည့် luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် luaB_yield (lua_State *L) {
  ပြန်ပေး lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const var_na *const statname[] =
  {"running", "dead", "suspended", "normal"};


static ကိန်းပြည့် auxstatus (lua_State *L, lua_State *co) {
  အကယ်၍ (L == co) ပြန်ပေး COS_RUN;
  မဟုတ်ပါက {
    switch (lua_status(co)) {
      pak_sa LUA_YIELD:
        ပြန်ပေး COS_YIELD;
      pak_sa LUA_OK: {
        lua_Debug ar;
        အကယ်၍ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ပြန်ပေး COS_NORM;  /* it is running */
        မဟုတ်ပါက အကယ်၍ (lua_gettop(co) == 0)
            ပြန်ပေး COS_DEAD;
        မဟုတ်ပါက
          ပြန်ပေး COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        ပြန်ပေး COS_DEAD;
    }
  }
}


static ကိန်းပြည့် luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ပြန်ပေး 1;
}


static lua_State *getoptco (lua_State *L) {
  ပြန်ပေး (lua_isnone(L, 1) ? L : getco(L));
}


static ကိန်းပြည့် luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် luaB_corunning (lua_State *L) {
  ကိန်းပြည့် ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ပြန်ပေး 2;
}


static ကိန်းပြည့် luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ကိန်းပြည့် status = auxstatus(L, co);
  switch (status) {
    pak_sa COS_DEAD: pak_sa COS_YIELD: {
      status = lua_closethread(co, L);
      အကယ်၍ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ပြန်ပေး 1;
      }
      မဟုတ်ပါက {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ပြန်ပေး 2;
      }
    }
    pak_sa COS_NORM:
      ပြန်ပေး luaL_error(L, "cannot close a %s coroutine", statname[status]);
    pak_sa COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      အကယ်၍ (lua_tothread(L, -1) == co)
        ပြန်ပေး luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    default:
      lua_assert(0);
      ပြန်ပေး 0;
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



LUAMOD_API ကိန်းပြည့် luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ပြန်ပေး 1;
}

