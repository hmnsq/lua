/* implementation for lib2-v2 */

#شامل <string.h>

#شامل "lua.h"
#شامل "lauxlib.h"

ایستا صحیح id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_درج(L, 1);
  بازگشت lua_gettop(L);
}


ساختار STR {
  تهی *ud;
  lua_Alloc allocf;
};


ایستا تهی *t_freestr (تهی *ud, تهی *ptr, size_t osize, size_t nsize) {
  ساختار STR *blk = (ساختار STR*)ptr - 1;
  blk->allocf(blk->ud, blk, اندازه(ساختار STR) + osize, 0);
  بازگشت پوچ;
}


ایستا صحیح newstr (lua_State *L) {
  size_t len;
  ثابت نویسه *str = luaL_checklstring(L, 1, &len);
  تهی *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  ساختار STR *blk = (ساختار STR*)allocf(ud, پوچ, 0,
                                        len + 1 + اندازه(ساختار STR));
  اگر (blk == پوچ) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (نویسه *)(blk + 1), len, t_freestr, L);
  بازگشت 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
ایستا تهی initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


ایستا ثابت ساختار luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {پوچ, پوچ}
};


LUAMOD_API صحیح luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  بازگشت 1;
}


