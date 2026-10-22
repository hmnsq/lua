/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lapi.h"
#அந்தர்பாவ "llimits.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lzio.h"


முழு luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  மாறிலி எழுத்து *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  எனில் (buff == வெற்று || size == 0)
    திரும்பு EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  திரும்பு cast_uchar(*(z->p++));
}


வெறுமை luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, வெறுமை *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = வெற்று;
}


/* --------------------------------------------------------------- read --- */

நிலையான முழு checkbuffer (ZIO *z) {
  எனில் (z->n == 0) {  /* no bytes in buffer? */
    எனில் (luaZ_fill(z) == EOZ)  /* try to read more */
      திரும்பு 0;  /* no more input */
    இல்லையெனில் {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  திரும்பு 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, வெறுமை *b, size_t n) {
  போது (n) {
    size_t m;
    எனில் (!checkbuffer(z))
      திரும்பு n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (எழுத்து *)b + m;
    n -= m;
  }
  திரும்பு 0;
}


மாறிலி வெறுமை *luaZ_getaddr (ZIO* z, size_t n) {
  மாறிலி வெறுமை *res;
  எனில் (!checkbuffer(z))
    திரும்பு வெற்று;  /* no more input */
  எனில் (z->n < n)  /* not enough bytes? */
    திரும்பு வெற்று;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  திரும்பு res;
}
