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


цео luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const char *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  ако (buff == NULL || size == 0)
    врати EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  врати cast_uchar(*(z->p++));
}


void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */

static цео checkbuffer (ZIO *z) {
  ако (z->n == 0) {  /* no bytes in buffer? */
    ако (luaZ_fill(z) == EOZ)  /* try to read more */
      врати 0;  /* no more input */
    инако {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  врати 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, void *b, size_t n) {
  док (n) {
    size_t m;
    ако (!checkbuffer(z))
      врати n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  врати 0;
}


const void *luaZ_getaddr (ZIO* z, size_t n) {
  const void *res;
  ако (!checkbuffer(z))
    врати NULL;  /* no more input */
  ако (z->n < n)  /* not enough bytes? */
    врати NULL;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  врати res;
}
