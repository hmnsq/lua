/* implementation for lib2-v2 */

#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"
#অন্তর্ভাব "lauxlib.h"

স্থির পূর্ণাঙ্ক id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_ঢোকাও(L, 1);
  প্রত্যাগম lua_gettop(L);
}


সংরচনা STR {
  শূন্য *ud;
  lua_Alloc allocf;
};


স্থির শূন্য *t_freestr (শূন্য *ud, শূন্য *ptr, size_t osize, size_t nsize) {
  সংরচনা STR *blk = (সংরচনা STR*)ptr - 1;
  blk->allocf(blk->ud, blk, পরিমাণ(সংরচনা STR) + osize, 0);
  প্রত্যাগম রিক্ত;
}


স্থির পূর্ণাঙ্ক newstr (lua_State *L) {
  size_t len;
  ধ্রুব বর্ণ *str = luaL_checklstring(L, 1, &len);
  শূন্য *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  সংরচনা STR *blk = (সংরচনা STR*)allocf(ud, রিক্ত, 0,
                                        len + 1 + পরিমাণ(সংরচনা STR));
  যদি (blk == রিক্ত) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (বর্ণ *)(blk + 1), len, t_freestr, L);
  প্রত্যাগম 1;
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


স্থির ধ্রুব সংরচনা luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {রিক্ত, রিক্ত}
};


LUAMOD_API পূর্ণাঙ্ক luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  প্রত্যাগম 1;
}


