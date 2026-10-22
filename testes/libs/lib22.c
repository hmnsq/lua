/* implementation for lib2-v2 */

#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"
#ಅನ್ತರ್ಭಾವ "lauxlib.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ id (lua_State *L) {
  lua_pushboolean(L, 1);
  lua_ಸೇರಿಸು(L, 1);
  ಹಿಂತಿರುಗಿ lua_gettop(L);
}


ರಚನೆ STR {
  ಶೂನ್ಯ *ud;
  lua_Alloc allocf;
};


ಸ್ಥಿರ ಶೂನ್ಯ *t_freestr (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ptr, size_t osize, size_t nsize) {
  ರಚನೆ STR *blk = (ರಚನೆ STR*)ptr - 1;
  blk->allocf(blk->ud, blk, ಗಾತ್ರ(ರಚನೆ STR) + osize, 0);
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newstr (lua_State *L) {
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str = luaL_checklstring(L, 1, &len);
  ಶೂನ್ಯ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  ರಚನೆ STR *blk = (ರಚನೆ STR*)allocf(ud, ಖಾಲಿ, 0,
                                        len + 1 + ಗಾತ್ರ(ರಚನೆ STR));
  ಆದರೆ (blk == ಖಾಲಿ) {  /* allocation error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  blk->ud = ud;  blk->allocf = allocf;
  ಸ್ಮರಣೆ_ನಕಲು(blk + 1, str, len + 1);
  lua_pushexternalstring(L, (ಅಕ್ಷರ *)(blk + 1), len, t_freestr, L);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Create an external string and keep it in the registry, so that it
** will test that the library code is still available (to deallocate
** this string) when closing the state.
*/
ಸ್ಥಿರ ಶೂನ್ಯ initstr (lua_State *L) {
  lua_pushcfunction(L, newstr);
  lua_pushstring(L,
     "012345678901234567890123456789012345678901234567890123456789");
  lua_call(L, 1, 1);  /* call newstr("0123...") */
  luaL_ref(L, LUA_REGISTRYINDEX);  /* keep string in the registry */
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ luaL_Reg funcs[] = {
  {"id", id},
  {"newstr", newstr},
  {ಖಾಲಿ, ಖಾಲಿ}
};


LUAMOD_API ಪೂರ್ಣಾಂಕ luaopen_lib2 (lua_State *L) {
  lua_settop(L, 2);
  lua_setglobal(L, "y");  /* y gets 2nd parameter */
  lua_setglobal(L, "x");  /* x gets 1st parameter */
  initstr(L);
  luaL_newlib(L, funcs);
  ಹಿಂತಿರುಗಿ 1;
}


