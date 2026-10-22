/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


ಪೂರ್ಣಾಙ್ಕ luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  ಧ್ರುವ ವರ್ಣ *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  ಯದಿ (buff == ರಿಕ್ತ || size == 0)
    ಪ್ರತ್ಯಾಗಮ EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  ಪ್ರತ್ಯಾಗಮ cast_uchar(*(z->p++));
}


ಶೂನ್ಯ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, ಶೂನ್ಯ *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = ರಿಕ್ತ;
}


/* --------------------------------------------------------------- read --- */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkbuffer (ZIO *z) {
  ಯದಿ (z->n == 0) {  /* no bytes in buffer? */
    ಯದಿ (luaZ_fill(z) == EOZ)  /* try to read more */
      ಪ್ರತ್ಯಾಗಮ 0;  /* no more input */
    ಅನ್ಯಥಾ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, ಶೂನ್ಯ *b, size_t n) {
  ಯಾವತ್ (n) {
    size_t m;
    ಯದಿ (!checkbuffer(z))
      ಪ್ರತ್ಯಾಗಮ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (ವರ್ಣ *)b + m;
    n -= m;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಧ್ರುವ ಶೂನ್ಯ *luaZ_getaddr (ZIO* z, size_t n) {
  ಧ್ರುವ ಶೂನ್ಯ *res;
  ಯದಿ (!checkbuffer(z))
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* no more input */
  ಯದಿ (z->n < n)  /* not enough bytes? */
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ಪ್ರತ್ಯಾಗಮ res;
}
