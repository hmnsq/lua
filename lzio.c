/*
** $Id: lzio.c $
** Buffered streams
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lzio_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


ಪೂರ್ಣಾಂಕ luaZ_fill (ZIO *z) {
  size_t ಗಾತ್ರ_2;
  lua_State *L = z->L;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buff;
  lua_unlock(L);
  buff = z->reader(L, z->ದತ್ತಾಂಶ, &ಗಾತ್ರ_2);
  lua_ಬೀಗ(L);
  ಆದರೆ (buff == ಖಾಲಿ || ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ EOZ;
  z->n = ಗಾತ್ರ_2 - 1;  /* discount char being returned */
  z->p = buff;
  ಹಿಂತಿರುಗಿ cast_uchar(*(z->p++));
}


ಶೂನ್ಯ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, ಶೂನ್ಯ *ದತ್ತಾಂಶ) {
  z->L = L;
  z->reader = reader;
  z->ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
  z->n = 0;
  z->p = ಖಾಲಿ;
}


/* --------------------------------------------------------------- read --- */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkbuffer (ZIO *z) {
  ಆದರೆ (z->n == 0) {  /* no bytes in buffer? */
    ಆದರೆ (luaZ_fill(z) == EOZ)  /* try to read more */
      ಹಿಂತಿರುಗಿ 0;  /* no more input */
    ಇಲ್ಲದಿದ್ದರೆ {
      z->n++;  /* luaZ_fill consumed first byte; put it back */
      z->p--;
    }
  }
  ಹಿಂತಿರುಗಿ 1;  /* now buffer has something */
}


size_t luaZ_read (ZIO *z, ಶೂನ್ಯ *b, size_t n) {
  ಆಗ (n) {
    size_t m;
    ಆದರೆ (!checkbuffer(z))
      ಹಿಂತಿರುಗಿ n;  /* no more input; return number of missing bytes */
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    ಸ್ಮರಣೆ_ನಕಲು(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (ಅಕ್ಷರ *)b + m;
    n -= m;
  }
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *luaZ_getaddr (ZIO* z, size_t n) {
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *res;
  ಆದರೆ (!checkbuffer(z))
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no more input */
  ಆದರೆ (z->n < n)  /* not enough bytes? */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* block not whole; cannot give an address */
  res = z->p;  /* get block address */
  z->n -= n;  /* consume these bytes */
  z->p += n;
  ಹಿಂತಿರುಗಿ res;
}
