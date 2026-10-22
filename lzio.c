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


পূর্ণাঙ্ক luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  ধ্রুব বর্ণ *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  যদি (buff == রিক্ত || size == 0)
    প্রত্যাগম EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  প্রত্যাগম cast_uchar(*(z->p++));
}


শূন্য luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, শূন্য *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = রিক্ত;
}


/* --------------------------------------------------------------- read --- */

স্থির পূর্ণাঙ্ক checkbuffer (ZIO *z) {
  যদি (z->n == 0) {  /* no bytes in buffer? */
    যদি (luaZ_fill(z) == EOZ)  /* try to read more */
      প্রত্যাগম 0;  /* no more input */
    অন্যথা {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  প্রত্যাগম 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, শূন্য *b, size_t n) {
  যাবত্ (n) {
    size_t m;
    যদি (!checkbuffer(z))
      প্রত্যাগম n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (বর্ণ *)b + m;
    n -= m;
  }
  প্রত্যাগম 0;
}


ধ্রুব শূন্য *luaZ_getaddr (ZIO* z, size_t n) {
  ধ্রুব শূন্য *res;
  যদি (!checkbuffer(z))
    প্রত্যাগম রিক্ত;  /* no more input */
  যদি (z->n < n)  /* not enough bytes? */
    প্রত্যাগম রিক্ত;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  প্রত্যাগম res;
}
