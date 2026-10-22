/* implementation for lib2-v2 */

#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"
#अन्तर्भाव "lauxlib.h"

स्थिर पूर्णाङ्क id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_घाला(L, 1);
  प्रत्यागम lua_gettop(L);
}


संरचना STR {
  शून्य *ud;
  lua_Alloc allocf;
};


स्थिर शून्य *t_freestr (शून्य *ud, शून्य *ptr, size_t osize, size_t nsize) {
  संरचना STR *blk = (संरचना STR*)ptr - 1;
  blk->allocf(blk->ud, blk, परिमाण(संरचना STR) + osize, 0);
  प्रत्यागम रिक्त;
}


स्थिर पूर्णाङ्क newstr (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *str = luaL_checklstring(L, 1, &len);
  शून्य *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  संरचना STR *blk = (संरचना STR*)allocf(ud, रिक्त, 0,
                                        len + 1 + परिमाण(संरचना STR));
  यदि (blk == रिक्त) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (वर्ण *)(blk + 1), len, t_freestr, L);
  प्रत्यागम 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
स्थिर शून्य initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


स्थिर ध्रुव संरचना luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णाङ्क luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  प्रत्यागम 1;
}


