/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#define lzio_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lapi.h"
#અન્તર્ભાવ "llimits.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lzio.h"


પૂર્ણાંક luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  અચલ અક્ષર *buff;
  lua_unlock(L);
  buff = z->reader(L, z->data, &size);
  lua_lock(L);
  જો (buff == ખાલી || size == 0)
    પાછા EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  પાછા cast_uchar(*(z->p++));
}


શૂન્ય luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, શૂન્ય *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = ખાલી;
}


/* --------------------------------------------------------------- read --- */

સ્થિર પૂર્ણાંક checkbuffer (ZIO *z) {
  જો (z->n == 0) {  /* no bytes in buffer? */
    જો (luaZ_fill(z) == EOZ)  /* try to read more */
      પાછા 0;  /* no more input */
    નહીંતર {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  પાછા 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, શૂન્ય *b, size_t n) {
  જ્યારે (n) {
    size_t m;
    જો (!checkbuffer(z))
      પાછા n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (અક્ષર *)b + m;
    n -= m;
  }
  પાછા 0;
}


અચલ શૂન્ય *luaZ_getaddr (ZIO* z, size_t n) {
  અચલ શૂન્ય *res;
  જો (!checkbuffer(z))
    પાછા ખાલી;  /* no more input */
  જો (z->n < n)  /* not enough bytes? */
    પાછા ખાલી;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  પાછા res;
}
