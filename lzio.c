/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#تعریف_کرو lzio_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "llimits.h"
#شامل "lmem.h"
#شامل "lstate.h"
#شامل "lzio.h"


صحیح luaZ_fill (ZIO *z) {
  size_t سائز;
  lua_State *L = z->L;
  ثابت حرف *buff;
  lua_unlock(L);
  buff = z->reader(L, z->ڈیٹا, &سائز);
  lua_تالا(L);
  اگر (buff == تہی || سائز == 0)
    واپس EOZ;
  z->n = سائز - 1;  /* discount char being returned */
  z->p = buff;
  واپس cast_uchar(*(z->p++));
}


خالی luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, خالی *ڈیٹا) {
  z->L = L;
  z->reader = reader;
  z->ڈیٹا = ڈیٹا;
  z->n = 0;
  z->p = تہی;
}


/* --------------------------------------------------------------- read --- */

ساکن صحیح checkbuffer (ZIO *z) {
  اگر (z->n == 0) {  /* no bytes in buffer? */
    اگر (luaZ_fill(z) == EOZ)  /* try to read more */
      واپس 0;  /* no more input */
    ورنہ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  واپس 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, خالی *b, size_t n) {
  جب (n) {
    size_t m;
    اگر (!checkbuffer(z))
      واپس n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    حافظہ_نقل(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (حرف *)b + m;
    n -= m;
  }
  واپس 0;
}


ثابت خالی *luaZ_getaddr (ZIO* z, size_t n) {
  ثابت خالی *res;
  اگر (!checkbuffer(z))
    واپس تہی;  /* no more input */
  اگر (z->n < n)  /* not enough bytes? */
    واپس تہی;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  واپس res;
}
