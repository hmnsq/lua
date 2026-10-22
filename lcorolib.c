/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "lualib.h"
#અન્તર્ભાવ "llimits.h"


સ્થિર lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  પાછા co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
સ્થિર પૂર્ણાંક auxresume (lua_State *L, lua_State *co, પૂર્ણાંક narg) {
  પૂર્ણાંક status, nres;
  જો (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    પાછા -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  જો (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    જો (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_કાઢો(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      પાછા -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    પાછા nres;
  }
  નહીંતર {
    lua_xmove(co, L, 1);  /* move error message */
    પાછા -1;  /* error flag */
  }
}


સ્થિર પૂર્ણાંક luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  પૂર્ણાંક r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  જો (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_ઉમેરો(L, -2);
    પાછા 2;  /* return false + error message */
  }
  નહીંતર {
    lua_pushboolean(L, 1);
    lua_ઉમેરો(L, -(r + 1));
    પાછા r + 1;  /* return true + 'resume' returns */
  }
}


સ્થિર પૂર્ણાંક luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  પૂર્ણાંક r = auxresume(L, co, lua_gettop(L));
  જો (l_unlikely(r < 0)) {  /* error? */
    પૂર્ણાંક stat = lua_status(co);
    જો (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      જો (stat == LUA_OK)
        પાછા 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    જો (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_ઉમેરો(L, -2);
      lua_concat(L, 2);
    }
    પાછા lua_error(L);  /* propagate error */
  }
  પાછા r;
}


સ્થિર પૂર્ણાંક luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  પાછા 1;
}


સ્થિર પૂર્ણાંક luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  પાછા 1;
}


સ્થિર પૂર્ણાંક luaB_yield (lua_State *L) {
  પાછા lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


સ્થિર અચલ અક્ષર *અચલ statname[] =
  {"running", "dead", "suspended", "normal"};


સ્થિર પૂર્ણાંક auxstatus (lua_State *L, lua_State *co) {
  જો (L == co) પાછા COS_RUN;
  નહીંતર {
    પસંદગી (lua_status(co)) {
      સ્થિતિ LUA_YIELD:
        પાછા COS_YIELD;
      સ્થિતિ LUA_OK: {
        lua_Debug ar;
        જો (lua_getstack(co, 0, &ar))  /* does it have frames? */
          પાછા COS_NORM;  /* it is running */
        નહીંતર જો (lua_gettop(co) == 0)
            પાછા COS_DEAD;
        નહીંતર
          પાછા COS_YIELD;  /* initial state */
      }
      મૂળભૂત:  /* some error occurred */
        પાછા COS_DEAD;
    }
  }
}


સ્થિર પૂર્ણાંક luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  પાછા 1;
}


સ્થિર lua_State *getoptco (lua_State *L) {
  પાછા (lua_isnone(L, 1) ? L : getco(L));
}


સ્થિર પૂર્ણાંક luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  પાછા 1;
}


સ્થિર પૂર્ણાંક luaB_corunning (lua_State *L) {
  પૂર્ણાંક ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  પાછા 2;
}


સ્થિર પૂર્ણાંક luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  પૂર્ણાંક status = auxstatus(L, co);
  પસંદગી (status) {
    સ્થિતિ COS_DEAD: સ્થિતિ COS_YIELD: {
      status = lua_closethread(co, L);
      જો (status == LUA_OK) {
        lua_pushboolean(L, 1);
        પાછા 1;
      }
      નહીંતર {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        પાછા 2;
      }
    }
    સ્થિતિ COS_NORM:
      પાછા luaL_error(L, "cannot close a %s coroutine", statname[status]);
    સ્થિતિ COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      જો (lua_tothread(L, -1) == co)
        પાછા luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    મૂળભૂત:
      lua_assert(0);
      પાછા 0;
  }
}


સ્થિર અચલ luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {ખાલી, ખાલી}
};



LUAMOD_API પૂર્ણાંક luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  પાછા 1;
}

