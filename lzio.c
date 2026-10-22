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


puur_naa_nka luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const var_na *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  ᨀᨘ (buff == NULL || size == 0)
    ᨀᨘᨒ EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  ᨀᨘᨒ cast_uchar(*(z->p++));
}


void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */

static puur_naa_nka checkbuffer (ZIO *z) {
  ᨀᨘ (z->n == 0) {  /* no bytes in buffer? */
    ᨀᨘ (luaZ_fill(z) == EOZ)  /* try to read more */
      ᨀᨘᨒ 0;  /* no more input */
    ᨀᨘᨁ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ᨀᨘᨒ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, void *b, size_t n) {
  ᨀᨛ (n) {
    size_t m;
    ᨀᨘ (!checkbuffer(z))
      ᨀᨘᨒ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (var_na *)b + m;
    n -= m;
  }
  ᨀᨘᨒ 0;
}


const void *luaZ_getaddr (ZIO* z, size_t n) {
  const void *res;
  ᨀᨘ (!checkbuffer(z))
    ᨀᨘᨒ NULL;  /* no more input */
  ᨀᨘ (z->n < n)  /* not enough bytes? */
    ᨀᨘᨒ NULL;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ᨀᨘᨒ res;
}
