/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#تعریف_کرو lcorolib_c
#تعریف_کرو LUA_LIB

#شامل "lprefix.h"


#شامل <stdlib.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


ساکن lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  واپس co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ساکن صحیح auxresume (lua_State *L, lua_State *co, صحیح narg) {
  صحیح حالت_2, nres;
  اگر (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    واپس -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  حالت_2 = lua_resume(co, L, narg, &nres);
  اگر (l_likely(حالت_2 == LUA_OK || حالت_2 == LUA_YIELD)) {
    اگر (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_نکالو(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      واپس -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    واپس nres;
  }
  ورنہ {
    lua_xmove(co, L, 1);  /* move error message */
    واپس -1;  /* error flag */
  }
}


ساکن صحیح luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  صحیح r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  اگر (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_داخل_کرو(L, -2);
    واپس 2;  /* return false + error message */
  }
  ورنہ {
    lua_pushboolean(L, 1);
    lua_داخل_کرو(L, -(r + 1));
    واپس r + 1;  /* return true + 'resume' returns */
  }
}


ساکن صحیح luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  صحیح r = auxresume(L, co, lua_gettop(L));
  اگر (l_unlikely(r < 0)) {  /* error? */
    صحیح stat = lua_حالت(co);
    اگر (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      اگر (stat == LUA_OK)
        واپس 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    اگر (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_داخل_کرو(L, -2);
      lua_concat(L, 2);
    }
    واپس lua_error(L);  /* propagate error */
  }
  واپس r;
}


ساکن صحیح luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  واپس 1;
}


ساکن صحیح luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  واپس 1;
}


ساکن صحیح luaB_yield (lua_State *L) {
  واپس lua_yield(L, lua_gettop(L));
}


#تعریف_کرو COS_تسلسل		0
#تعریف_کرو COS_DEAD	1
#تعریف_کرو COS_YIELD	2
#تعریف_کرو COS_NORM	3


ساکن ثابت حرف *ثابت statname[] =
  {"running", "dead", "suspended", "normal"};


ساکن صحیح auxstatus (lua_State *L, lua_State *co) {
  اگر (L == co) واپس COS_تسلسل;
  ورنہ {
    انتخاب (lua_حالت(co)) {
      حالت LUA_YIELD:
        واپس COS_YIELD;
      حالت LUA_OK: {
        lua_Debug ar;
        اگر (lua_getstack(co, 0, &ar))  /* does it have frames? */
          واپس COS_NORM;  /* it is running */
        ورنہ اگر (lua_gettop(co) == 0)
            واپس COS_DEAD;
        ورنہ
          واپس COS_YIELD;  /* initial state */
      }
      طےشدہ:  /* some error occurred */
        واپس COS_DEAD;
    }
  }
}


ساکن صحیح luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  واپس 1;
}


ساکن lua_State *getoptco (lua_State *L) {
  واپس (lua_isnone(L, 1) ? L : getco(L));
}


ساکن صحیح luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  واپس 1;
}


ساکن صحیح luaB_corunning (lua_State *L) {
  صحیح ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  واپس 2;
}


ساکن صحیح luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  صحیح حالت_2 = auxstatus(L, co);
  انتخاب (حالت_2) {
    حالت COS_DEAD: حالت COS_YIELD: {
      حالت_2 = lua_closethread(co, L);
      اگر (حالت_2 == LUA_OK) {
        lua_pushboolean(L, 1);
        واپس 1;
      }
      ورنہ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        واپس 2;
      }
    }
    حالت COS_NORM:
      واپس luaL_error(L, "cannot close a %s coroutine", statname[حالت_2]);
    حالت COS_تسلسل:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      اگر (lua_tothread(L, -1) == co)
        واپس luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    طےشدہ:
      lua_assert(0);
      واپس 0;
  }
}


ساکن ثابت luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {تہی, تہی}
};



LUAMOD_API صحیح luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  واپس 1;
}

