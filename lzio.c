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


पूर्णांक luaZ_fill (ZIO *z) {
  माप_प्रकार size;
  lua_State *L = z->L;
  स्थिरांक अक्षर *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  जर (buff == रिक्त || size == 0)
    परत EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  परत cast_uchar(*(z->p++));
}


शून्य luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, शून्य *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = रिक्त;
}


/* --------------------------------------------------------------- read --- */

स्थिर पूर्णांक checkbuffer (ZIO *z) {
  जर (z->n == 0) {  /* no bytes in buffer? */
    जर (luaZ_fill(z) == EOZ)  /* try to read more */
      परत 0;  /* no more input */
    अन्यथा {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  परत 1;  /* now buffer has something */
}


माप_प्रकार luaZ_read (ZIO *z, शून्य *b, माप_प्रकार n) {
  पर्यंत (n) {
    माप_प्रकार m;
    जर (!checkbuffer(z))
      परत n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    स_नकल(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (अक्षर *)b + m;
    n -= m;
  }
  परत 0;
}


स्थिरांक शून्य *luaZ_getaddr (ZIO* z, माप_प्रकार n) {
  स्थिरांक शून्य *res;
  जर (!checkbuffer(z))
    परत रिक्त;  /* no more input */
  जर (z->n < n)  /* not enough bytes? */
    परत रिक्त;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  परत res;
}
