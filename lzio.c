/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lzio.h"


पूर्णाङ्क luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  ध्रुव वर्ण *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  जइ (buff == रिक्त || size == 0)
    प्रत्यागम EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  प्रत्यागम cast_uchar(*(z->p++));
}


शून्य luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, शून्य *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = रिक्त;
}


/* --------------------------------------------------------------- read --- */

स्थिर पूर्णाङ्क checkbuffer (ZIO *z) {
  जइ (z->n == 0) {  /* no bytes in buffer? */
    जइ (luaZ_fill(z) == EOZ)  /* try to read more */
      प्रत्यागम 0;  /* no more input */
    अण्णहा {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  प्रत्यागम 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, शून्य *b, size_t n) {
  जाव (n) {
    size_t m;
    जइ (!checkbuffer(z))
      प्रत्यागम n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (वर्ण *)b + m;
    n -= m;
  }
  प्रत्यागम 0;
}


ध्रुव शून्य *luaZ_getaddr (ZIO* z, size_t n) {
  ध्रुव शून्य *res;
  जइ (!checkbuffer(z))
    प्रत्यागम रिक्त;  /* no more input */
  जइ (z->n < n)  /* not enough bytes? */
    प्रत्यागम रिक्त;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  प्रत्यागम res;
}
