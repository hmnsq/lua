/* implementation for lib2-v2 */

#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"
#अन्तर्भाव "lauxlib.h"

स्थिर पूर्णांक id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_घाला(L, 1);
  परत lua_gettop(L);
}


संरचना STR {
  शून्य *ud;
  lua_Alloc allocf;
};


स्थिर शून्य *t_freestr (शून्य *ud, शून्य *ptr, माप_प्रकार osize, माप_प्रकार nsize) {
  संरचना STR *blk = (संरचना STR*)ptr - 1;
  blk->allocf(blk->ud, blk, आकार(संरचना STR) + osize, 0);
  परत रिक्त;
}


स्थिर पूर्णांक newstr (lua_State *L) {
  माप_प्रकार len;
  स्थिरांक अक्षर *str = luaL_checklstring(L, 1, &len);
  शून्य *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  संरचना STR *blk = (संरचना STR*)allocf(ud, रिक्त, 0,
                                        len + 1 + आकार(संरचना STR));
  जर (blk == रिक्त) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  स_नकल(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (अक्षर *)(blk + 1), len, t_freestr, L);
  परत 1;
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


स्थिर स्थिरांक संरचना luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {रिक्त, रिक्त}
};


LUAMOD_API पूर्णांक luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  परत 1;
}


