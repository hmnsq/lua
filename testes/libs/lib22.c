/* implementation for lib2-v2 */

#شامل <string.h>

#شامل "lua.h"
#شامل "lauxlib.h"

ساکن صحیح id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_داخل_کرو(L, 1);
  واپس lua_gettop(L);
}


ساخت STR {
  خالی *ud;
  lua_Alloc allocf;
};


ساکن خالی *t_freestr (خالی *ud, خالی *ptr, size_t osize, size_t nsize) {
  ساخت STR *blk = (ساخت STR*)ptr - 1;
  blk->allocf(blk->ud, blk, حجم(ساخت STR) + osize, 0);
  واپس تہی;
}


ساکن صحیح newstr (lua_State *L) {
  size_t len;
  ثابت حرف *str = luaL_checklstring(L, 1, &len);
  خالی *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  ساخت STR *blk = (ساخت STR*)allocf(ud, تہی, 0,
                                        len + 1 + حجم(ساخت STR));
  اگر (blk == تہی) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  حافظہ_نقل(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (حرف *)(blk + 1), len, t_freestr, L);
  واپس 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
ساکن خالی initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


ساکن ثابت ساخت luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {تہی, تہی}
};


LUAMOD_API صحیح luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  واپس 1;
}


