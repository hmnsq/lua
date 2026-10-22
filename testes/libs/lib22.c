/* implementation for lib2-v2 */

#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"
#అన్తర్భావ "lauxlib.h"

స్థిర పూర్ణాంకం id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_చొప్పించు(L, 1);
  తిరిగి lua_gettop(L);
}


నిర్మాణం STR {
  శూన్య *ud;
  lua_Alloc allocf;
};


స్థిర శూన్య *t_freestr (శూన్య *ud, శూన్య *ptr, size_t osize, size_t nsize) {
  నిర్మాణం STR *blk = (నిర్మాణం STR*)ptr - 1;
  blk->allocf(blk->ud, blk, పరిమాణం(నిర్మాణం STR) + osize, 0);
  తిరిగి ఖాళీ;
}


స్థిర పూర్ణాంకం newstr (lua_State *L) {
  size_t len;
  స్థిరరాశి అక్షరం *str = luaL_checklstring(L, 1, &len);
  శూన్య *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  నిర్మాణం STR *blk = (నిర్మాణం STR*)allocf(ud, ఖాళీ, 0,
                                        len + 1 + పరిమాణం(నిర్మాణం STR));
  ఒకవేళ (blk == ఖాళీ) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  జ్ఞాపకం_నకలు(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (అక్షరం *)(blk + 1), len, t_freestr, L);
  తిరిగి 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
స్థిర శూన్య initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


స్థిర స్థిరరాశి నిర్మాణం luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {ఖాళీ, ఖాళీ}
};


LUAMOD_API పూర్ణాంకం luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  తిరిగి 1;
}


