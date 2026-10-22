/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#घोषणा lzio_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lapi.h"
#समावेश "llimits.h"
#समावेश "lmem.h"
#समावेश "lstate.h"
#समावेश "lzio.h"


पूर्णांक luaZ_fill (ZIO *z) {
  माप_प्रकार आकार;
  lua_State *L = z->L;
  स्थिर अक्षर *buff;
  lua_unlock(L);
  buff = z->reader(L, z->तथ्य, &आकार);
  lua_lock(L);
  अगर (buff == शून्य || आकार == 0)
    वापस EOZ;
  z->n = आकार - 1;  /* discount char being returned */
  z->p = buff;
  वापस cast_uchar(*(z->p++));
}


व्योम luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, व्योम *तथ्य) {
  z->L = L;
  z->reader = reader;
  z->तथ्य = तथ्य;
  z->n = 0;
  z->p = शून्य;
}


/* --------------------------------------------------------------- read --- */

अटल पूर्णांक checkbuffer (ZIO *z) {
  अगर (z->n == 0) {  /* no bytes in buffer? */
    अगर (luaZ_fill(z) == EOZ)  /* try to read more */
      वापस 0;  /* no more input */
    अन्यथा {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  वापस 1;  /* now buffer has something */
}


माप_प्रकार luaZ_read (ZIO *z, व्योम *b, माप_प्रकार n) {
  जबतक (n) {
    माप_प्रकार m;
    अगर (!checkbuffer(z))
      वापस n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    स_नकल(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (अक्षर *)b + m;
    n -= m;
  }
  वापस 0;
}


स्थिर व्योम *luaZ_getaddr (ZIO* z, माप_प्रकार n) {
  स्थिर व्योम *res;
  अगर (!checkbuffer(z))
    वापस शून्य;  /* no more input */
  अगर (z->n < n)  /* not enough bytes? */
    वापस शून्य;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  वापस res;
}
