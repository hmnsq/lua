/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#ضمن "lprefix.h"


#ضمن <stdlib.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


ساكن lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  ܦܢܐ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ساكن صحيح auxresume (lua_State *L, lua_State *co, صحيح narg) {
  صحيح status, nres;
  ܐܢ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ܦܢܐ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  ܐܢ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    ܐܢ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_pop(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ܦܢܐ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ܦܢܐ nres;
  }
  ܐܚܪܢܐ {
    lua_xmove(co, L, 1);  /* move error message */
    ܦܢܐ -1;  /* error flag */
  }
}


ساكن صحيح luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  صحيح r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ܐܢ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ܦܢܐ 2;  /* return false + error message */
  }
  ܐܚܪܢܐ {
    lua_pushboolean(L, 1);
    lua_insert(L, -(r + 1));
    ܦܢܐ r + 1;  /* return true + 'resume' returns */
  }
}


ساكن صحيح luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  صحيح r = auxresume(L, co, lua_gettop(L));
  ܐܢ (l_unlikely(r < 0)) {  /* error? */
    صحيح stat = lua_status(co);
    ܐܢ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ܐܢ (stat == LUA_OK)
        ܦܢܐ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ܐܢ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_insert(L, -2);
      lua_concat(L, 2);
    }
    ܦܢܐ lua_error(L);  /* propagate error */
  }
  ܦܢܐ r;
}


ساكن صحيح luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ܦܢܐ 1;
}


ساكن صحيح luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ܦܢܐ 1;
}


ساكن صحيح luaB_yield (lua_State *L) {
  ܦܢܐ lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


ساكن ثابت حرف *ثابت statname[] =
  {"running", "dead", "suspended", "normal"};


ساكن صحيح auxstatus (lua_State *L, lua_State *co) {
  ܐܢ (L == co) ܦܢܐ COS_RUN;
  ܐܚܪܢܐ {
    بدل (lua_status(co)) {
      حالة LUA_YIELD:
        ܦܢܐ COS_YIELD;
      حالة LUA_OK: {
        lua_Debug ar;
        ܐܢ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ܦܢܐ COS_NORM;  /* it is running */
        ܐܚܪܢܐ ܐܢ (lua_gettop(co) == 0)
            ܦܢܐ COS_DEAD;
        ܐܚܪܢܐ
          ܦܢܐ COS_YIELD;  /* initial state */
      }
      افتراضي:  /* some error occurred */
        ܦܢܐ COS_DEAD;
    }
  }
}


ساكن صحيح luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ܦܢܐ 1;
}


ساكن lua_State *getoptco (lua_State *L) {
  ܦܢܐ (lua_isnone(L, 1) ? L : getco(L));
}


ساكن صحيح luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ܦܢܐ 1;
}


ساكن صحيح luaB_corunning (lua_State *L) {
  صحيح ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ܦܢܐ 2;
}


ساكن صحيح luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  صحيح status = auxstatus(L, co);
  بدل (status) {
    حالة COS_DEAD: حالة COS_YIELD: {
      status = lua_closethread(co, L);
      ܐܢ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ܦܢܐ 1;
      }
      ܐܚܪܢܐ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ܦܢܐ 2;
      }
    }
    حالة COS_NORM:
      ܦܢܐ luaL_error(L, "cannot close a %s coroutine", statname[status]);
    حالة COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ܐܢ (lua_tothread(L, -1) == co)
        ܦܢܐ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    افتراضي:
      lua_assert(0);
      ܦܢܐ 0;
  }
}


ساكن ثابت luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {عدم, عدم}
};



LUAMOD_API صحيح luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ܦܢܐ 1;
}

