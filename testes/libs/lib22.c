/* implementation for lib2-v2 */

#ضمن <string.h>

#ضمن "lua.h"
#ضمن "lauxlib.h"

ساكن صحيح id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_insert(L, 1);
  أرجع lua_gettop(L);
}


بنية STR {
  فراغ *ud;
  lua_Alloc allocf;
};


ساكن فراغ *t_freestr (فراغ *ud, فراغ *ptr, size_t osize, size_t nsize) {
  بنية STR *blk = (بنية STR*)ptr - 1;
  blk->allocf(blk->ud, blk, حجم(بنية STR) + osize, 0);
  أرجع عدم;
}


ساكن صحيح newstr (lua_State *L) {
  size_t len;
  ثابت حرف *str = luaL_checklstring(L, 1, &len);
  فراغ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  بنية STR *blk = (بنية STR*)allocf(ud, عدم, 0,
                                        len + 1 + حجم(بنية STR));
  𐤀𐤌 (blk == عدم) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  memcpy(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (حرف *)(blk + 1), len, t_freestr, L);
  أرجع 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
ساكن فراغ initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


ساكن ثابت بنية luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {عدم, عدم}
};


LUAMOD_API صحيح luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  أرجع 1;
}


