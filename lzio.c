/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#నిర్వచించు lzio_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lzio.h"


పూర్ణాంకం luaZ_fill (ZIO *z) {
  size_t కొలత;
  lua_State *L = z->L;
  స్థిరరాశి అక్షరం *buff;
  lua_unlock(L);
  buff = z->reader(L, z->దత్తాంశం, &కొలత);
  lua_తాళం(L);
  ఒకవేళ (buff == ఖాళీ || కొలత == 0)
    తిరిగి EOZ;
  z->n = కొలత - 1;  /* discount char being returned */
  z->p = buff;
  తిరిగి cast_uchar(*(z->p++));
}


శూన్య luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, శూన్య *దత్తాంశం) {
  z->L = L;
  z->reader = reader;
  z->దత్తాంశం = దత్తాంశం;
  z->n = 0;
  z->p = ఖాళీ;
}


/* --------------------------------------------------------------- read --- */

స్థిర పూర్ణాంకం checkbuffer (ZIO *z) {
  ఒకవేళ (z->n == 0) {  /* no bytes in buffer? */
    ఒకవేళ (luaZ_fill(z) == EOZ)  /* try to read more */
      తిరిగి 0;  /* no more input */
    లేకపోతే {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  తిరిగి 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, శూన్య *b, size_t n) {
  అయితే (n) {
    size_t m;
    ఒకవేళ (!checkbuffer(z))
      తిరిగి n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    జ్ఞాపకం_నకలు(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (అక్షరం *)b + m;
    n -= m;
  }
  తిరిగి 0;
}


స్థిరరాశి శూన్య *luaZ_getaddr (ZIO* z, size_t n) {
  స్థిరరాశి శూన్య *res;
  ఒకవేళ (!checkbuffer(z))
    తిరిగి ఖాళీ;  /* no more input */
  ఒకవేళ (z->n < n)  /* not enough bytes? */
    తిరిగి ఖాళీ;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  తిరిగి res;
}
