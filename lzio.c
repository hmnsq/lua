/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lapi.h"
#അന്തര്ഭാവ "llimits.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lzio.h"


പൂർണ്ണസംഖ്യ luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  സ്ഥിരം അക്ഷരം *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  എങ്കിൽ (buff == ശൂന്യ || size == 0)
    തിരികെ EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  തിരികെ cast_uchar(*(z->p++));
}


ശൂന്യം luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, ശൂന്യം *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = ശൂന്യ;
}


/* --------------------------------------------------------------- read --- */

സ്ഥിതം പൂർണ്ണസംഖ്യ checkbuffer (ZIO *z) {
  എങ്കിൽ (z->n == 0) {  /* no bytes in buffer? */
    എങ്കിൽ (luaZ_fill(z) == EOZ)  /* try to read more */
      തിരികെ 0;  /* no more input */
    അല്ലെങ്കിൽ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  തിരികെ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, ശൂന്യം *b, size_t n) {
  ആകുമ്പോൾ (n) {
    size_t m;
    എങ്കിൽ (!checkbuffer(z))
      തിരികെ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (അക്ഷരം *)b + m;
    n -= m;
  }
  തിരികെ 0;
}


സ്ഥിരം ശൂന്യം *luaZ_getaddr (ZIO* z, size_t n) {
  സ്ഥിരം ശൂന്യം *res;
  എങ്കിൽ (!checkbuffer(z))
    തിരികെ ശൂന്യ;  /* no more input */
  എങ്കിൽ (z->n < n)  /* not enough bytes? */
    തിരികെ ശൂന്യ;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  തിരികെ res;
}
