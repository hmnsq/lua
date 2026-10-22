/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#شامل "lprefix.h"


#شامل <stdlib.h>

#شامل "lua.h"

#شامل "lauxlib.h"
#شامل "lualib.h"
#شامل "llimits.h"


ایستا lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  بازگشت co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ایستا صحیح auxresume (lua_State *L, lua_State *co, صحیح narg) {
  صحیح status, nres;
  اگر (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    بازگشت -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  اگر (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    اگر (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_برداشت(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      بازگشت -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    بازگشت nres;
  }
  وگرنه {
    lua_xmove(co, L, 1);  /* move error message */
    بازگشت -1;  /* error flag */
  }
}


ایستا صحیح luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  صحیح r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  اگر (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_درج(L, -2);
    بازگشت 2;  /* return false + error message */
  }
  وگرنه {
    lua_pushboolean(L, 1);
    lua_درج(L, -(r + 1));
    بازگشت r + 1;  /* return true + 'resume' returns */
  }
}


ایستا صحیح luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  صحیح r = auxresume(L, co, lua_gettop(L));
  اگر (l_unlikely(r < 0)) {  /* error? */
    صحیح stat = lua_status(co);
    اگر (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      اگر (stat == LUA_OK)
        بازگشت 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    اگر (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_درج(L, -2);
      lua_concat(L, 2);
    }
    بازگشت lua_error(L);  /* propagate error */
  }
  بازگشت r;
}


ایستا صحیح luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  بازگشت 1;
}


ایستا صحیح luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  بازگشت 1;
}


ایستا صحیح luaB_yield (lua_State *L) {
  بازگشت lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


ایستا ثابت نویسه *ثابت statname[] =
  {"running", "dead", "suspended", "normal"};


ایستا صحیح auxstatus (lua_State *L, lua_State *co) {
  اگر (L == co) بازگشت COS_RUN;
  وگرنه {
    انتخاب (lua_status(co)) {
      حالت LUA_YIELD:
        بازگشت COS_YIELD;
      حالت LUA_OK: {
        lua_Debug ar;
        اگر (lua_getstack(co, 0, &ar))  /* does it have frames? */
          بازگشت COS_NORM;  /* it is running */
        وگرنه اگر (lua_gettop(co) == 0)
            بازگشت COS_DEAD;
        وگرنه
          بازگشت COS_YIELD;  /* initial state */
      }
      پیشفرض:  /* some error occurred */
        بازگشت COS_DEAD;
    }
  }
}


ایستا صحیح luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  بازگشت 1;
}


ایستا lua_State *getoptco (lua_State *L) {
  بازگشت (lua_isnone(L, 1) ? L : getco(L));
}


ایستا صحیح luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  بازگشت 1;
}


ایستا صحیح luaB_corunning (lua_State *L) {
  صحیح ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  بازگشت 2;
}


ایستا صحیح luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  صحیح status = auxstatus(L, co);
  انتخاب (status) {
    حالت COS_DEAD: حالت COS_YIELD: {
      status = lua_closethread(co, L);
      اگر (status == LUA_OK) {
        lua_pushboolean(L, 1);
        بازگشت 1;
      }
      وگرنه {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        بازگشت 2;
      }
    }
    حالت COS_NORM:
      بازگشت luaL_error(L, "cannot close a %s coroutine", statname[status]);
    حالت COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      اگر (lua_tothread(L, -1) == co)
        بازگشت luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    پیشفرض:
      lua_assert(0);
      بازگشت 0;
  }
}


ایستا ثابت luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {پوچ, پوچ}
};



LUAMOD_API صحیح luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  بازگشت 1;
}

