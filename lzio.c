/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lapi.h"
#অন্তর্ভাৱ "llimits.h"
#অন্তর্ভাৱ "lmem.h"
#অন্তর্ভাৱ "lstate.h"
#অন্তর্ভাৱ "lzio.h"


পূর্ণাঙ্ক luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  ধ্রুৱ ৱর্ণ *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  যদি (buff == রিক্ত || size == 0)
    ঘূৰাই EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  ঘূৰাই cast_uchar(*(z->p++));
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
      ঘূৰাই 0;  /* no more input */
    অন্যথা {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ঘূৰাই 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, শূন্য *b, size_t n) {
  যেতিয়ালৈ (n) {
    size_t m;
    যদি (!checkbuffer(z))
      ঘূৰাই n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (ৱর্ণ *)b + m;
    n -= m;
  }
  ঘূৰাই 0;
}


ধ্রুৱ শূন্য *luaZ_getaddr (ZIO* z, size_t n) {
  ধ্রুৱ শূন্য *res;
  যদি (!checkbuffer(z))
    ঘূৰাই রিক্ত;  /* no more input */
  যদি (z->n < n)  /* not enough bytes? */
    ঘূৰাই রিক্ত;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ঘূৰাই res;
}
