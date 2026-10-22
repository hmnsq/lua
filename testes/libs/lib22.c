/* implementation for lib2-v2 */

#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"
#ଅନ୍ତର୍ଭାଵ "lauxlib.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_ଭର୍ତ୍ତି(L, 1);
  ଫେରନ୍ତୁ lua_gettop(L);
}


ସଂରଚନା STR {
  ଶୂନ୍ୟ *ud;
  lua_Alloc allocf;
};


ସ୍ଥିର ଶୂନ୍ୟ *t_freestr (ଶୂନ୍ୟ *ud, ଶୂନ୍ୟ *ptr, size_t osize, size_t nsize) {
  ସଂରଚନା STR *blk = (ସଂରଚନା STR*)ptr - 1;
  blk->allocf(blk->ud, blk, ଆକାର(ସଂରଚନା STR) + osize, 0);
  ଫେରନ୍ତୁ ଖାଲି;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା newstr (lua_State *L) {
  size_t len;
  ଅଚଳ ଅକ୍ଷର *str = luaL_checklstring(L, 1, &len);
  ଶୂନ୍ୟ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  ସଂରଚନା STR *blk = (ସଂରଚନା STR*)allocf(ud, ଖାଲି, 0,
                                        len + 1 + ଆକାର(ସଂରଚନା STR));
  ଯଦି (blk == ଖାଲି) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (ଅକ୍ଷର *)(blk + 1), len, t_freestr, L);
  ଫେରନ୍ତୁ 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
ସ୍ଥିର ଶୂନ୍ୟ initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


ସ୍ଥିର ଅଚଳ ସଂରଚନା luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {ଖାଲି, ଖାଲି}
};


LUAMOD_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  ଫେରନ୍ତୁ 1;
}


