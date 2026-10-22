/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "llimits.h"
#ضمن "lmem.h"
#ضمن "lstate.h"
#ضمن "lzio.h"


صحيح luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  ثابت حرف *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  ܐܢ (buff == عدم || size == 0)
    ܦܢܐ EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  ܦܢܐ cast_uchar(*(z->p++));
}


فراغ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, فراغ *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = عدم;
}


/* --------------------------------------------------------------- read --- */

ساكن صحيح checkbuffer (ZIO *z) {
  ܐܢ (z->n == 0) {  /* no bytes in buffer? */
    ܐܢ (luaZ_fill(z) == EOZ)  /* try to read more */
      ܦܢܐ 0;  /* no more input */
    ܐܚܪܢܐ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ܦܢܐ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, فراغ *b, size_t n) {
  ܟܡܐ (n) {
    size_t m;
    ܐܢ (!checkbuffer(z))
      ܦܢܐ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (حرف *)b + m;
    n -= m;
  }
  ܦܢܐ 0;
}


ثابت فراغ *luaZ_getaddr (ZIO* z, size_t n) {
  ثابت فراغ *res;
  ܐܢ (!checkbuffer(z))
    ܦܢܐ عدم;  /* no more input */
  ܐܢ (z->n < n)  /* not enough bytes? */
    ܦܢܐ عدم;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ܦܢܐ res;
}
