/* implementation for lib2-v2 */

#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"
#അന്തര്ഭാവ "lauxlib.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_തിരുകുക(L, 1);
  തിരികെ lua_gettop(L);
}


ഘടന STR {
  ശൂന്യം *ud;
  lua_Alloc allocf;
};


സ്ഥിതം ശൂന്യം *t_freestr (ശൂന്യം *ud, ശൂന്യം *ptr, size_t osize, size_t nsize) {
  ഘടന STR *blk = (ഘടന STR*)ptr - 1;
  blk->allocf(blk->ud, blk, വലുപ്പം(ഘടന STR) + osize, 0);
  തിരികെ ശൂന്യ;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ newstr (lua_State *L) {
  size_t len;
  സ്ഥിരം അക്ഷരം *str = luaL_checklstring(L, 1, &len);
  ശൂന്യം *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  ഘടന STR *blk = (ഘടന STR*)allocf(ud, ശൂന്യ, 0,
                                        len + 1 + വലുപ്പം(ഘടന STR));
  എങ്കിൽ (blk == ശൂന്യ) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (അക്ഷരം *)(blk + 1), len, t_freestr, L);
  തിരികെ 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
സ്ഥിതം ശൂന്യം initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


സ്ഥിതം സ്ഥിരം ഘടന luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {ശൂന്യ, ശൂന്യ}
};


LUAMOD_API പൂർണ്ണസംഖ്യ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  തിരികെ 1;
}


