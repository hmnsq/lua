/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"


ସ୍ଥିର lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  ଫେରନ୍ତୁ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା auxresume (lua_State *L, lua_State *co, ପୂର୍ଣ୍ଣସଂଖ୍ୟା narg) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, nres;
  ଯଦି (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    ଫେରନ୍ତୁ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  ଯଦି (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    ଯଦି (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_କାଢ଼(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      ଫେରନ୍ତୁ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    ଫେରନ୍ତୁ nres;
  }
  ଅନ୍ୟଥା {
    lua_xmove(co, L, 1);  /* move error message */
    ଫେରନ୍ତୁ -1;  /* error flag */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  ଯଦି (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_ଭର୍ତ୍ତି(L, -2);
    ଫେରନ୍ତୁ 2;  /* return false + error message */
  }
  ଅନ୍ୟଥା {
    lua_pushboolean(L, 1);
    lua_ଭର୍ତ୍ତି(L, -(r + 1));
    ଫେରନ୍ତୁ r + 1;  /* return true + 'resume' returns */
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = auxresume(L, co, lua_gettop(L));
  ଯଦି (l_unlikely(r < 0)) {  /* error? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat = lua_status(co);
    ଯଦି (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      ଯଦି (stat == LUA_OK)
        ଫେରନ୍ତୁ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    ଯଦି (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_ଭର୍ତ୍ତି(L, -2);
      lua_concat(L, 2);
    }
    ଫେରନ୍ତୁ lua_error(L);  /* propagate error */
  }
  ଫେରନ୍ତୁ r;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_yield (lua_State *L) {
  ଫେରନ୍ତୁ lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ statname[] =
  {"running", "dead", "suspended", "normal"};


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା auxstatus (lua_State *L, lua_State *co) {
  ଯଦି (L == co) ଫେରନ୍ତୁ COS_RUN;
  ଅନ୍ୟଥା {
    ବାଛ (lua_status(co)) {
      ସ୍ଥିତି LUA_YIELD:
        ଫେରନ୍ତୁ COS_YIELD;
      ସ୍ଥିତି LUA_OK: {
        lua_Debug ar;
        ଯଦି (lua_getstack(co, 0, &ar))  /* does it have frames? */
          ଫେରନ୍ତୁ COS_NORM;  /* it is running */
        ଅନ୍ୟଥା ଯଦି (lua_gettop(co) == 0)
            ଫେରନ୍ତୁ COS_DEAD;
        ଅନ୍ୟଥା
          ଫେରନ୍ତୁ COS_YIELD;  /* initial state */
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  /* some error occurred */
        ଫେରନ୍ତୁ COS_DEAD;
    }
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର lua_State *getoptco (lua_State *L) {
  ଫେରନ୍ତୁ (lua_isnone(L, 1) ? L : getco(L));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_corunning (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  ଫେରନ୍ତୁ 2;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = auxstatus(L, co);
  ବାଛ (status) {
    ସ୍ଥିତି COS_DEAD: ସ୍ଥିତି COS_YIELD: {
      status = lua_closethread(co, L);
      ଯଦି (status == LUA_OK) {
        lua_pushboolean(L, 1);
        ଫେରନ୍ତୁ 1;
      }
      ଅନ୍ୟଥା {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        ଫେରନ୍ତୁ 2;
      }
    }
    ସ୍ଥିତି COS_NORM:
      ଫେରନ୍ତୁ luaL_error(L, "cannot close a %s coroutine", statname[status]);
    ସ୍ଥିତି COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      ଯଦି (lua_tothread(L, -1) == co)
        ଫେରନ୍ତୁ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      lua_assert(0);
      ଫେରନ୍ତୁ 0;
  }
}


ସ୍ଥିର ଅଚଳ luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {ଖାଲି, ଖାଲି}
};



LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  ଫେରନ୍ତୁ 1;
}

