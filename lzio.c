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


целое luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const char *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  если (buff == NULL || size == 0)
    вернуть EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  вернуть cast_uchar(*(z->p++));
}


void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */

static целое checkbuffer (ZIO *z) {
  если (z->n == 0) {  /* no bytes in buffer? */
    если (luaZ_fill(z) == EOZ)  /* try to read more */
      вернуть 0;  /* no more input */
    иначе {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  вернуть 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, void *b, size_t n) {
  пока (n) {
    size_t m;
    если (!checkbuffer(z))
      вернуть n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  вернуть 0;
}


const void *luaZ_getaddr (ZIO* z, size_t n) {
  const void *res;
  если (!checkbuffer(z))
    вернуть NULL;  /* no more input */
  если (z->n < n)  /* not enough bytes? */
    вернуть NULL;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  вернуть res;
}
