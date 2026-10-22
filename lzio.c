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
  माप_प्रकार size;
  lua_State *L = z->L;
  अचर वर्ण *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  यदि (buff == रिक्त || size == 0)
    फर्काऊ EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  फर्काऊ cast_uchar(*(z->p++));
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
  यदि (z->n == 0) {  /* no bytes in buffer? */
    यदि (luaZ_fill(z) == EOZ)  /* try to read more */
      फर्काऊ 0;  /* no more input */
    अन्यथा {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  फर्काऊ 1;  /* now buffer has something */
}


माप_प्रकार luaZ_read (ZIO *z, शून्य *b, माप_प्रकार n) {
  जबसम्म (n) {
    माप_प्रकार m;
    यदि (!checkbuffer(z))
      फर्काऊ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    स_नकल(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (वर्ण *)b + m;
    n -= m;
  }
  फर्काऊ 0;
}


अचर शून्य *luaZ_getaddr (ZIO* z, माप_प्रकार n) {
  अचर शून्य *res;
  यदि (!checkbuffer(z))
    फर्काऊ रिक्त;  /* no more input */
  यदि (z->n < n)  /* not enough bytes? */
    फर्काऊ रिक्त;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  फर्काऊ res;
}
