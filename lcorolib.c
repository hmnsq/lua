/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lualib.h"
#அந்தர்பாவ "llimits.h"


நிலையான lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  திரும்பு co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
நிலையான முழு auxresume (lua_State *L, lua_State *co, முழு narg) {
  முழு status, nres;
  எனில் (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    திரும்பு -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  எனில் (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    எனில் (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_எடு(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      திரும்பு -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    திரும்பு nres;
  }
  இல்லையெனில் {
    lua_xmove(co, L, 1);  /* move error message */
    திரும்பு -1;  /* error flag */
  }
}


நிலையான முழு luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  முழு r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  எனில் (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_செருகு(L, -2);
    திரும்பு 2;  /* return false + error message */
  }
  இல்லையெனில் {
    lua_pushboolean(L, 1);
    lua_செருகு(L, -(r + 1));
    திரும்பு r + 1;  /* return true + 'resume' returns */
  }
}


நிலையான முழு luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  முழு r = auxresume(L, co, lua_gettop(L));
  எனில் (l_unlikely(r < 0)) {  /* error? */
    முழு stat = lua_status(co);
    எனில் (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      எனில் (stat == LUA_OK)
        திரும்பு 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    எனில் (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_செருகு(L, -2);
      lua_concat(L, 2);
    }
    திரும்பு lua_error(L);  /* propagate error */
  }
  திரும்பு r;
}


நிலையான முழு luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  திரும்பு 1;
}


நிலையான முழு luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  திரும்பு 1;
}


நிலையான முழு luaB_yield (lua_State *L) {
  திரும்பு lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


நிலையான மாறிலி எழுத்து *மாறிலி statname[] =
  {"running", "dead", "suspended", "normal"};


நிலையான முழு auxstatus (lua_State *L, lua_State *co) {
  எனில் (L == co) திரும்பு COS_RUN;
  இல்லையெனில் {
    தேர்வு (lua_status(co)) {
      நிலை LUA_YIELD:
        திரும்பு COS_YIELD;
      நிலை LUA_OK: {
        lua_Debug ar;
        எனில் (lua_getstack(co, 0, &ar))  /* does it have frames? */
          திரும்பு COS_NORM;  /* it is running */
        இல்லையெனில் எனில் (lua_gettop(co) == 0)
            திரும்பு COS_DEAD;
        இல்லையெனில்
          திரும்பு COS_YIELD;  /* initial state */
      }
      இயல்பு:  /* some error occurred */
        திரும்பு COS_DEAD;
    }
  }
}


நிலையான முழு luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  திரும்பு 1;
}


நிலையான lua_State *getoptco (lua_State *L) {
  திரும்பு (lua_isnone(L, 1) ? L : getco(L));
}


நிலையான முழு luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  திரும்பு 1;
}


நிலையான முழு luaB_corunning (lua_State *L) {
  முழு ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  திரும்பு 2;
}


நிலையான முழு luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  முழு status = auxstatus(L, co);
  தேர்வு (status) {
    நிலை COS_DEAD: நிலை COS_YIELD: {
      status = lua_closethread(co, L);
      எனில் (status == LUA_OK) {
        lua_pushboolean(L, 1);
        திரும்பு 1;
      }
      இல்லையெனில் {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        திரும்பு 2;
      }
    }
    நிலை COS_NORM:
      திரும்பு luaL_error(L, "cannot close a %s coroutine", statname[status]);
    நிலை COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      எனில் (lua_tothread(L, -1) == co)
        திரும்பு luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    இயல்பு:
      lua_assert(0);
      திரும்பு 0;
  }
}


நிலையான மாறிலி luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {வெற்று, வெற்று}
};



LUAMOD_API முழு luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  திரும்பு 1;
}

