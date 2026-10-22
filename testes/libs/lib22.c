/* implementation for lib2-v2 */

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"

ਸਥਿਰ ਪੂਰਨ id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_ਪਾਓ(L, 1);
  ਵਾਪਸ lua_gettop(L);
}


ਬਣਤਰ STR {
  ਖਾਲੀ *ud;
  lua_Alloc allocf;
};


ਸਥਿਰ ਖਾਲੀ *t_freestr (ਖਾਲੀ *ud, ਖਾਲੀ *ptr, size_t osize, size_t nsize) {
  ਬਣਤਰ STR *blk = (ਬਣਤਰ STR*)ptr - 1;
  blk->allocf(blk->ud, blk, ਆਕਾਰ(ਬਣਤਰ STR) + osize, 0);
  ਵਾਪਸ ਨਿਰਾ;
}


ਸਥਿਰ ਪੂਰਨ newstr (lua_State *L) {
  size_t len;
  ਅਚਲ ਅੱਖਰ *str = luaL_checklstring(L, 1, &len);
  ਖਾਲੀ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  ਬਣਤਰ STR *blk = (ਬਣਤਰ STR*)allocf(ud, ਨਿਰਾ, 0,
                                        len + 1 + ਆਕਾਰ(ਬਣਤਰ STR));
  ਜੇ (blk == ਨਿਰਾ) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (ਅੱਖਰ *)(blk + 1), len, t_freestr, L);
  ਵਾਪਸ 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
ਸਥਿਰ ਖਾਲੀ initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


ਸਥਿਰ ਅਚਲ ਬਣਤਰ luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {ਨਿਰਾ, ਨਿਰਾ}
};


LUAMOD_API ਪੂਰਨ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  ਵਾਪਸ 1;
}


