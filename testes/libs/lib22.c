/* implementation for lib2-v2 */

#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"
#અન્તર્ભાવ "lauxlib.h"

સ્થિર પૂર્ણાંક id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_ઉમેરો(L, 1);
  પાછા lua_gettop(L);
}


સંરચના STR {
  શૂન્ય *ud;
  lua_Alloc allocf;
};


સ્થિર શૂન્ય *t_freestr (શૂન્ય *ud, શૂન્ય *ptr, size_t osize, size_t nsize) {
  સંરચના STR *blk = (સંરચના STR*)ptr - 1;
  blk->allocf(blk->ud, blk, કદ(સંરચના STR) + osize, 0);
  પાછા ખાલી;
}


સ્થિર પૂર્ણાંક newstr (lua_State *L) {
  size_t len;
  અચલ અક્ષર *str = luaL_checklstring(L, 1, &len);
  શૂન્ય *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  સંરચના STR *blk = (સંરચના STR*)allocf(ud, ખાલી, 0,
                                        len + 1 + કદ(સંરચના STR));
  જો (blk == ખાલી) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (અક્ષર *)(blk + 1), len, t_freestr, L);
  પાછા 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
સ્થિર શૂન્ય initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


સ્થિર અચલ સંરચના luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {ખાલી, ખાલી}
};


LUAMOD_API પૂર્ણાંક luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  પાછા 1;
}


