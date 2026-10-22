/* implementation for lib2-v2 */

#समावेश <string.h>

#समावेश "lua.h"
#समावेश "lauxlib.h"

अटल पूर्णांक id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_जोड़ो(L, 1);
  वापस lua_gettop(L);
}


काष्ठा STR {
  व्योम *ud;
  lua_Alloc allocf;
};


अटल व्योम *t_freestr (व्योम *ud, व्योम *ptr, माप_प्रकार osize, माप_प्रकार nsize) {
  काष्ठा STR *blk = (काष्ठा STR*)ptr - 1;
  blk->allocf(blk->ud, blk, माप(काष्ठा STR) + osize, 0);
  वापस शून्य;
}


अटल पूर्णांक newstr (lua_State *L) {
  माप_प्रकार len;
  स्थिर अक्षर *str = luaL_checklstring(L, 1, &len);
  व्योम *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  काष्ठा STR *blk = (काष्ठा STR*)allocf(ud, शून्य, 0,
                                        len + 1 + माप(काष्ठा STR));
  अगर (blk == शून्य) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  स_नकल(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (अक्षर *)(blk + 1), len, t_freestr, L);
  वापस 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
अटल व्योम initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


अटल स्थिर काष्ठा luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {शून्य, शून्य}
};


LUAMOD_API पूर्णांक luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  वापस 1;
}


