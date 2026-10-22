/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "llimits.h"
#شامل "lmem.h"
#شامل "lstate.h"
#شامل "lzio.h"


صحیح luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  ثابت نویسه *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  اگر (buff == پوچ || size == 0)
    بازگشت EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  بازگشت cast_uchar(*(z->p++));
}


تهی luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, تهی *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = پوچ;
}


/* --------------------------------------------------------------- read --- */

ایستا صحیح checkbuffer (ZIO *z) {
  اگر (z->n == 0) {  /* no bytes in buffer? */
    اگر (luaZ_fill(z) == EOZ)  /* try to read more */
      بازگشت 0;  /* no more input */
    وگرنه {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  بازگشت 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, تهی *b, size_t n) {
  وقتی (n) {
    size_t m;
    اگر (!checkbuffer(z))
      بازگشت n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (نویسه *)b + m;
    n -= m;
  }
  بازگشت 0;
}


ثابت تهی *luaZ_getaddr (ZIO* z, size_t n) {
  ثابت تهی *res;
  اگر (!checkbuffer(z))
    بازگشت پوچ;  /* no more input */
  اگر (z->n < n)  /* not enough bytes? */
    بازگشت پوچ;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  بازگشت res;
}
