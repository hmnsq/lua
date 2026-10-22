/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "llimits.h"
#include "lmem.h"
#include "lstate.h"
#include "lzio.h"


ຈຳນວນເຕັມ luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const var_na *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  ຖ້າ (buff == NULL || size == 0)
    ສົ່ງຄືນ EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  ສົ່ງຄືນ cast_uchar(*(z->p++));
}


void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */

static ຈຳນວນເຕັມ checkbuffer (ZIO *z) {
  ຖ້າ (z->n == 0) {  /* no bytes in buffer? */
    ຖ້າ (luaZ_fill(z) == EOZ)  /* try to read more */
      ສົ່ງຄືນ 0;  /* no more input */
    ບໍ່ດັ່ງນັ້ນ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ສົ່ງຄືນ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, void *b, size_t n) {
  ໃນຂະນະທີ່ (n) {
    size_t m;
    ຖ້າ (!checkbuffer(z))
      ສົ່ງຄືນ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (var_na *)b + m;
    n -= m;
  }
  ສົ່ງຄືນ 0;
}


const void *luaZ_getaddr (ZIO* z, size_t n) {
  const void *res;
  ຖ້າ (!checkbuffer(z))
    ສົ່ງຄືນ NULL;  /* no more input */
  ຖ້າ (z->n < n)  /* not enough bytes? */
    ສົ່ງຄືນ NULL;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ສົ່ງຄືນ res;
}
