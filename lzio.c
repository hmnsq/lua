/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lapi.h"
#অন্তর্ভাব "llimits.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lzio.h"


পূর্ণসংখ্যা luaZ_fill (ZIO *z) {
  size_t আকার_2;
  lua_State *L = z->L;
  ধ্রুবক অক্ষর *buff;
  lua_unlock(L);
  buff = z->reader(L, z->তথ্য, &আকার_2);
  lua_lock(L);
  যদি (buff == ফাঁকা || আকার_2 == 0)
    ফেরত EOZ;
  z->n = আকার_2 - 1;  /* discount char being returned */
  z->p = buff;
  ফেরত cast_uchar(*(z->p++));
}


শূন্য luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, শূন্য *তথ্য) {
  z->L = L;
  z->reader = reader;
  z->তথ্য = তথ্য;
  z->n = 0;
  z->p = ফাঁকা;
}


/* --------------------------------------------------------------- read --- */

স্থির পূর্ণসংখ্যা checkbuffer (ZIO *z) {
  যদি (z->n == 0) {  /* no bytes in buffer? */
    যদি (luaZ_fill(z) == EOZ)  /* try to read more */
      ফেরত 0;  /* no more input */
    অন্যথা {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ফেরত 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, শূন্য *b, size_t n) {
  যখন (n) {
    size_t m;
    যদি (!checkbuffer(z))
      ফেরত n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (অক্ষর *)b + m;
    n -= m;
  }
  ফেরত 0;
}


ধ্রুবক শূন্য *luaZ_getaddr (ZIO* z, size_t n) {
  ধ্রুবক শূন্য *res;
  যদি (!checkbuffer(z))
    ফেরত ফাঁকা;  /* no more input */
  যদি (z->n < n)  /* not enough bytes? */
    ফেরত ফাঁকা;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ফেরত res;
}
