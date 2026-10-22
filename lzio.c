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


整数 luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const char *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  如果 (buff == NULL || size == 0)
    返回 EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  返回 cast_uchar(*(z->p++));
}


void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */

static 整数 checkbuffer (ZIO *z) {
  如果 (z->n == 0) {  /* no bytes in buffer? */
    如果 (luaZ_fill(z) == EOZ)  /* try to read more */
      返回 0;  /* no more input */
    否则 {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  返回 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, void *b, size_t n) {
  当 (n) {
    size_t m;
    如果 (!checkbuffer(z))
      返回 n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  返回 0;
}


const void *luaZ_getaddr (ZIO* z, size_t n) {
  const void *res;
  如果 (!checkbuffer(z))
    返回 NULL;  /* no more input */
  如果 (z->n < n)  /* not enough bytes? */
    返回 NULL;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  返回 res;
}
