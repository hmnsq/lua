/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#تعریف_کړه lzio_c
#تعریف_کړه LUA_CORE

#ضمن "lprefix.h"


#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "llimits.h"
#ضمن "lmem.h"
#ضمن "lstate.h"
#ضمن "lzio.h"


صحيح luaZ_fill (ZIO *z) {
  size_t اندازه;
  lua_State *L = z->L;
  ثابت حرف *buff;
  lua_unlock(L);
  buff = z->reader(L, z->معلومات, &اندازه);
  lua_قلف(L);
  إذا (buff == عدم || اندازه == 0)
    أرجع EOZ;
  z->n = اندازه - 1;  /* discount char being returned */
  z->p = buff;
  أرجع cast_uchar(*(z->p++));
}


فراغ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, فراغ *معلومات) {
  z->L = L;
  z->reader = reader;
  z->معلومات = معلومات;
  z->n = 0;
  z->p = عدم;
}


/* --------------------------------------------------------------- read --- */

ساكن صحيح checkbuffer (ZIO *z) {
  إذا (z->n == 0) {  /* no bytes in buffer? */
    إذا (luaZ_fill(z) == EOZ)  /* try to read more */
      أرجع 0;  /* no more input */
    وإلا {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  أرجع 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, فراغ *b, size_t n) {
  طالما (n) {
    size_t m;
    إذا (!checkbuffer(z))
      أرجع n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    حافظه_نقل(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (حرف *)b + m;
    n -= m;
  }
  أرجع 0;
}


ثابت فراغ *luaZ_getaddr (ZIO* z, size_t n) {
  ثابت فراغ *res;
  إذا (!checkbuffer(z))
    أرجع عدم;  /* no more input */
  إذا (z->n < n)  /* not enough bytes? */
    أرجع عدم;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  أرجع res;
}
