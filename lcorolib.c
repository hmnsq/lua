/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#تعریف_کړه lcorolib_c
#تعریف_کړه LUA_LIB

#ضمن "lprefix.h"


#ضمن <stdlib.h>

#ضمن "lua.h"

#ضمن "lauxlib.h"
#ضمن "lualib.h"
#ضمن "llimits.h"


ساكن lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  أرجع co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ساكن صحيح auxresume (lua_State *L, lua_State *co, صحيح narg) {
  صحيح حالت, nres;
  إذا (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    أرجع -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  حالت = lua_resume(co, L, narg, &nres);
  إذا (l_likely(حالت == LUA_OK || حالت == LUA_YIELD)) {
    إذا (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_راوباسه(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      أرجع -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    أرجع nres;
  }
  وإلا {
    lua_xmove(co, L, 1);  /* move error message */
    أرجع -1;  /* error flag */
  }
}


ساكن صحيح luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  صحيح r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  إذا (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_ورننباسه(L, -2);
    أرجع 2;  /* return false + error message */
  }
  وإلا {
    lua_pushboolean(L, 1);
    lua_ورننباسه(L, -(r + 1));
    أرجع r + 1;  /* return true + 'resume' returns */
  }
}


ساكن صحيح luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  صحيح r = auxresume(L, co, lua_gettop(L));
  إذا (l_unlikely(r < 0)) {  /* error? */
    صحيح stat = lua_حالت(co);
    إذا (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      إذا (stat == LUA_OK)
        أرجع 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    إذا (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_ورننباسه(L, -2);
      lua_concat(L, 2);
    }
    أرجع lua_error(L);  /* propagate error */
  }
  أرجع r;
}


ساكن صحيح luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  أرجع 1;
}


ساكن صحيح luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  أرجع 1;
}


ساكن صحيح luaB_yield (lua_State *L) {
  أرجع lua_yield(L, lua_gettop(L));
}


#تعریف_کړه COS_پرلپسې		0
#تعریف_کړه COS_DEAD	1
#تعریف_کړه COS_YIELD	2
#تعریف_کړه COS_NORM	3


ساكن ثابت حرف *ثابت statname[] =
  {"running", "dead", "suspended", "normal"};


ساكن صحيح auxstatus (lua_State *L, lua_State *co) {
  إذا (L == co) أرجع COS_پرلپسې;
  وإلا {
    بدل (lua_حالت(co)) {
      حالة LUA_YIELD:
        أرجع COS_YIELD;
      حالة LUA_OK: {
        lua_Debug ar;
        إذا (lua_getstack(co, 0, &ar))  /* does it have frames? */
          أرجع COS_NORM;  /* it is running */
        وإلا إذا (lua_gettop(co) == 0)
            أرجع COS_DEAD;
        وإلا
          أرجع COS_YIELD;  /* initial state */
      }
      افتراضي:  /* some error occurred */
        أرجع COS_DEAD;
    }
  }
}


ساكن صحيح luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  أرجع 1;
}


ساكن lua_State *getoptco (lua_State *L) {
  أرجع (lua_isnone(L, 1) ? L : getco(L));
}


ساكن صحيح luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  أرجع 1;
}


ساكن صحيح luaB_corunning (lua_State *L) {
  صحيح ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  أرجع 2;
}


ساكن صحيح luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  صحيح حالت = auxstatus(L, co);
  بدل (حالت) {
    حالة COS_DEAD: حالة COS_YIELD: {
      حالت = lua_closethread(co, L);
      إذا (حالت == LUA_OK) {
        lua_pushboolean(L, 1);
        أرجع 1;
      }
      وإلا {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        أرجع 2;
      }
    }
    حالة COS_NORM:
      أرجع luaL_error(L, "cannot close a %s coroutine", statname[حالت]);
    حالة COS_پرلپسې:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      إذا (lua_tothread(L, -1) == co)
        أرجع luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    افتراضي:
      lua_assert(0);
      أرجع 0;
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
  أرجع 1;
}

