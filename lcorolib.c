/*
** $Id: lcorolib.c $
** Coroutine Library
** See Copyright Notice in lua.h
*/

#define lcorolib_c
#define LUA_LIB

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lualib.h"
#അന്തര്ഭാവ "llimits.h"


സ്ഥിതം lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  തിരികെ co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ auxresume (lua_State *L, lua_State *co, പൂർണ്ണസംഖ്യ narg) {
  പൂർണ്ണസംഖ്യ status, nres;
  എങ്കിൽ (l_unlikely(!lua_checkstack(co, narg))) {
    lua_pushliteral(L, "too many arguments to resume");
    തിരികെ -1;  /* error flag */
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  എങ്കിൽ (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    എങ്കിൽ (l_unlikely(!lua_checkstack(L, nres + 1))) {
      lua_എടുക്കുക(co, nres);  /* remove results anyway */
      lua_pushliteral(L, "too many results to resume");
      തിരികെ -1;  /* error flag */
    }
    lua_xmove(co, L, nres);  /* move yielded values */
    തിരികെ nres;
  }
  അല്ലെങ്കിൽ {
    lua_xmove(co, L, 1);  /* move error message */
    തിരികെ -1;  /* error flag */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  പൂർണ്ണസംഖ്യ r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  എങ്കിൽ (l_unlikely(r < 0)) {
    lua_pushboolean(L, 0);
    lua_തിരുകുക(L, -2);
    തിരികെ 2;  /* return false + error message */
  }
  അല്ലെങ്കിൽ {
    lua_pushboolean(L, 1);
    lua_തിരുകുക(L, -(r + 1));
    തിരികെ r + 1;  /* return true + 'resume' returns */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, lua_upvalueindex(1));
  പൂർണ്ണസംഖ്യ r = auxresume(L, co, lua_gettop(L));
  എങ്കിൽ (l_unlikely(r < 0)) {  /* error? */
    പൂർണ്ണസംഖ്യ stat = lua_status(co);
    എങ്കിൽ (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = lua_closethread(co, L);  /* close its tbc variables */
      എങ്കിൽ (stat == LUA_OK)
        തിരികെ 0;  /* nothing to return */
      lua_xmove(co, L, 1);  /* move error message to the caller */
    }
    എങ്കിൽ (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        lua_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      luaL_where(L, 1);  /* add extra info, if available */
      lua_തിരുകുക(L, -2);
      lua_concat(L, 2);
    }
    തിരികെ lua_error(L);  /* propagate error */
  }
  തിരികെ r;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);  /* move function to top */
  lua_xmove(L, NL, 1);  /* move function from L to NL */
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_yield (lua_State *L) {
  തിരികെ lua_yield(L, lua_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം statname[] =
  {"running", "dead", "suspended", "normal"};


സ്ഥിതം പൂർണ്ണസംഖ്യ auxstatus (lua_State *L, lua_State *co) {
  എങ്കിൽ (L == co) തിരികെ COS_RUN;
  അല്ലെങ്കിൽ {
    തിരഞ്ഞെടുപ്പ് (lua_status(co)) {
      അവസ്ഥ LUA_YIELD:
        തിരികെ COS_YIELD;
      അവസ്ഥ LUA_OK: {
        lua_Debug ar;
        എങ്കിൽ (lua_getstack(co, 0, &ar))  /* does it have frames? */
          തിരികെ COS_NORM;  /* it is running */
        അല്ലെങ്കിൽ എങ്കിൽ (lua_gettop(co) == 0)
            തിരികെ COS_DEAD;
        അല്ലെങ്കിൽ
          തിരികെ COS_YIELD;  /* initial state */
      }
      സ്ഥിരസ്ഥിതി:  /* some error occurred */
        തിരികെ COS_DEAD;
    }
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  തിരികെ 1;
}


സ്ഥിതം lua_State *getoptco (lua_State *L) {
  തിരികെ (lua_isnone(L, 1) ? L : getco(L));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_yieldable (lua_State *L) {
  lua_State *co = getoptco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_corunning (lua_State *L) {
  പൂർണ്ണസംഖ്യ ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  തിരികെ 2;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ luaB_close (lua_State *L) {
  lua_State *co = getoptco(L);
  പൂർണ്ണസംഖ്യ status = auxstatus(L, co);
  തിരഞ്ഞെടുപ്പ് (status) {
    അവസ്ഥ COS_DEAD: അവസ്ഥ COS_YIELD: {
      status = lua_closethread(co, L);
      എങ്കിൽ (status == LUA_OK) {
        lua_pushboolean(L, 1);
        തിരികെ 1;
      }
      അല്ലെങ്കിൽ {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);  /* move error message */
        തിരികെ 2;
      }
    }
    അവസ്ഥ COS_NORM:
      തിരികെ luaL_error(L, "cannot close a %s coroutine", statname[status]);
    അവസ്ഥ COS_RUN:
      lua_geti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);  /* get main */
      എങ്കിൽ (lua_tothread(L, -1) == co)
        തിരികെ luaL_error(L, "cannot close main thread");
      lua_closethread(co, L);  /* close itself */
      /* previous call does not return *//* FALLTHROUGH */
    സ്ഥിരസ്ഥിതി:
      lua_assert(0);
      തിരികെ 0;
  }
}


സ്ഥിതം സ്ഥിരം luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {ശൂന്യ, ശൂന്യ}
};



LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_coroutine (lua_State *L) {
  luaL_newlib(L, co_funcs);
  തിരികെ 1;
}

