/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lzio_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"


ਪੂਰਨ luaZ_fill (ZIO *z) {
  size_t ਮਾਪ;
  lua_State *L = z->L;
  ਅਚਲ ਅੱਖਰ *buff;
  lua_unlock(L);
  buff = z->reader(L, z->ਅੰਕੜੇ, &ਮਾਪ);
  lua_ਤਾਲਾ(L);
  ਜੇ (buff == ਨਿਰਾ || ਮਾਪ == 0)
    ਵਾਪਸ EOZ;
  z->n = ਮਾਪ - 1;  /* discount char being returned */
  z->p = buff;
  ਵਾਪਸ cast_uchar(*(z->p++));
}


ਖਾਲੀ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, ਖਾਲੀ *ਅੰਕੜੇ) {
  z->L = L;
  z->reader = reader;
  z->ਅੰਕੜੇ = ਅੰਕੜੇ;
  z->n = 0;
  z->p = ਨਿਰਾ;
}


/* --------------------------------------------------------------- read --- */

ਸਥਿਰ ਪੂਰਨ checkbuffer (ZIO *z) {
  ਜੇ (z->n == 0) {  /* no bytes in buffer? */
    ਜੇ (luaZ_fill(z) == EOZ)  /* try to read more */
      ਵਾਪਸ 0;  /* no more input */
    ਨਹੀਂਤਾਂ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ਵਾਪਸ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, ਖਾਲੀ *b, size_t n) {
  ਜਦੋਂ (n) {
    size_t m;
    ਜੇ (!checkbuffer(z))
      ਵਾਪਸ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (ਅੱਖਰ *)b + m;
    n -= m;
  }
  ਵਾਪਸ 0;
}


ਅਚਲ ਖਾਲੀ *luaZ_getaddr (ZIO* z, size_t n) {
  ਅਚਲ ਖਾਲੀ *res;
  ਜੇ (!checkbuffer(z))
    ਵਾਪਸ ਨਿਰਾ;  /* no more input */
  ਜੇ (z->n < n)  /* not enough bytes? */
    ਵਾਪਸ ਨਿਰਾ;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ਵਾਪਸ res;
}
