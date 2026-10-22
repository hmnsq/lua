/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lapi.h"
#ଅନ୍ତର୍ଭାଵ "llimits.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lzio.h"


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  ଅଚଳ ଅକ୍ଷର *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  ଯଦି (buff == ଖାଲି || size == 0)
    ଫେରନ୍ତୁ EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  ଫେରନ୍ତୁ cast_uchar(*(z->p++));
}


ଶୂନ୍ୟ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, ଶୂନ୍ୟ *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = ଖାଲି;
}


/* --------------------------------------------------------------- read --- */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkbuffer (ZIO *z) {
  ଯଦି (z->n == 0) {  /* no bytes in buffer? */
    ଯଦି (luaZ_fill(z) == EOZ)  /* try to read more */
      ଫେରନ୍ତୁ 0;  /* no more input */
    ଅନ୍ୟଥା {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ଫେରନ୍ତୁ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, ଶୂନ୍ୟ *b, size_t n) {
  ଯେତେବେଳେ (n) {
    size_t m;
    ଯଦି (!checkbuffer(z))
      ଫେରନ୍ତୁ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (ଅକ୍ଷର *)b + m;
    n -= m;
  }
  ଫେରନ୍ତୁ 0;
}


ଅଚଳ ଶୂନ୍ୟ *luaZ_getaddr (ZIO* z, size_t n) {
  ଅଚଳ ଶୂନ୍ୟ *res;
  ଯଦି (!checkbuffer(z))
    ଫେରନ୍ତୁ ଖାଲି;  /* no more input */
  ଯଦି (z->n < n)  /* not enough bytes? */
    ଫେରନ୍ତୁ ଖାଲି;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ଫେରନ୍ତୁ res;
}
