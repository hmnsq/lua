/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#परिभाषय lzio_c
#परिभाषय LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lzio.h"


पूर्णाङ्क luaZ_fill (ZIO *z) {
  size_t आकार;
  lua_State *L = z->L;
  ध्रुव वर्ण *buff;
  lua_unlock(L);
  buff = z->reader(L, z->दत्त, &आकार);
  lua_तालकम्(L);
  यदि (buff == रिक्त || आकार == 0)
    प्रत्यागम EOZ;
  z->n = आकार - 1;  /* discount char being returned */
  z->p = buff;
  प्रत्यागम cast_uchar(*(z->p++));
}


शून्य luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, शून्य *दत्त) {
  z->L = L;
  z->reader = reader;
  z->दत्त = दत्त;
  z->n = 0;
  z->p = रिक्त;
}


/* --------------------------------------------------------------- read --- */

स्थिर पूर्णाङ्क checkbuffer (ZIO *z) {
  यदि (z->n == 0) {  /* no bytes in buffer? */
    यदि (luaZ_fill(z) == EOZ)  /* try to read more */
      प्रत्यागम 0;  /* no more input */
    अन्यथा {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  प्रत्यागम 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, शून्य *b, size_t n) {
  यावत् (n) {
    size_t m;
    यदि (!checkbuffer(z))
      प्रत्यागम n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    स्मृतिं_प्रतिलिख(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (वर्ण *)b + m;
    n -= m;
  }
  प्रत्यागम 0;
}


ध्रुव शून्य *luaZ_getaddr (ZIO* z, size_t n) {
  ध्रुव शून्य *res;
  यदि (!checkbuffer(z))
    प्रत्यागम रिक्त;  /* no more input */
  यदि (z->n < n)  /* not enough bytes? */
    प्रत्यागम रिक्त;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  प्रत्यागम res;
}
