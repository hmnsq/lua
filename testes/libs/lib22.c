/* implementation for lib2-v2 */

#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"
#அந்தர்பாவ "lauxlib.h"

நிலையான முழு id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_செருகு(L, 1);
  திரும்பு lua_gettop(L);
}


கட்டமைப்பு STR {
  வெறுமை *ud;
  lua_Alloc allocf;
};


நிலையான வெறுமை *t_freestr (வெறுமை *ud, வெறுமை *ptr, size_t osize, size_t nsize) {
  கட்டமைப்பு STR *blk = (கட்டமைப்பு STR*)ptr - 1;
  blk->allocf(blk->ud, blk, அளவு(கட்டமைப்பு STR) + osize, 0);
  திரும்பு வெற்று;
}


நிலையான முழு newstr (lua_State *L) {
  size_t len;
  மாறிலி எழுத்து *str = luaL_checklstring(L, 1, &len);
  வெறுமை *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  கட்டமைப்பு STR *blk = (கட்டமைப்பு STR*)allocf(ud, வெற்று, 0,
                                        len + 1 + அளவு(கட்டமைப்பு STR));
  எனில் (blk == வெற்று) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (எழுத்து *)(blk + 1), len, t_freestr, L);
  திரும்பு 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
நிலையான வெறுமை initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


நிலையான மாறிலி கட்டமைப்பு luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {வெற்று, வெற்று}
};


LUAMOD_API முழு luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  திரும்பு 1;
}


