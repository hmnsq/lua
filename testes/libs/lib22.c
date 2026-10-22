/* implementation for lib2-v2 */

#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"
#অন্তর্ভাব "lauxlib.h"

স্থির পূর্ণসংখ্যা id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_ঢোকাও(L, 1);
  ফেরত lua_gettop(L);
}


কাঠামো STR {
  শূন্য *ud;
  lua_Alloc allocf;
};


স্থির শূন্য *t_freestr (শূন্য *ud, শূন্য *ptr, size_t osize, size_t nsize) {
  কাঠামো STR *blk = (কাঠামো STR*)ptr - 1;
  blk->allocf(blk->ud, blk, আকার(কাঠামো STR) + osize, 0);
  ফেরত ফাঁকা;
}


স্থির পূর্ণসংখ্যা newstr (lua_State *L) {
  size_t len;
  ধ্রুবক অক্ষর *str = luaL_checklstring(L, 1, &len);
  শূন্য *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  কাঠামো STR *blk = (কাঠামো STR*)allocf(ud, ফাঁকা, 0,
                                        len + 1 + আকার(কাঠামো STR));
  যদি (blk == ফাঁকা) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (অক্ষর *)(blk + 1), len, t_freestr, L);
  ফেরত 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
স্থির শূন্য initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


স্থির ধ্রুবক কাঠামো luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {ফাঁকা, ফাঁকা}
};


LUAMOD_API পূর্ণসংখ্যা luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  ফেরত 1;
}


