/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lzio_h
#ವ್ಯಾಖ್ಯಾನಿಸು lzio_h

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lmem.h"


#ವ್ಯಾಖ್ಯಾನಿಸು EOZ	(-1)			/* end of stream */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Zio ZIO;

#ವ್ಯಾಖ್ಯಾನಿಸು zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Mbuffer {
  ಅಕ್ಷರ *ಬಫರ್;
  size_t n;
  size_t buffsize;
} Mbuffer;

#ವ್ಯಾಖ್ಯಾನಿಸು luaZ_initbuffer(L, buff) ((buff)->ಬಫರ್ = ಖಾಲಿ, (buff)->buffsize = 0)

#ವ್ಯಾಖ್ಯಾನಿಸು lua_Z_ಬಫರ್(buff)	((buff)->ಬಫರ್)
#ವ್ಯಾಖ್ಯಾನಿಸು luaZ_sizebuffer(buff)	((buff)->buffsize)
#ವ್ಯಾಖ್ಯಾನಿಸು luaZ_bufflen(buff)	((buff)->n)

#ವ್ಯಾಖ್ಯಾನಿಸು luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#ವ್ಯಾಖ್ಯಾನಿಸು luaZ_resetbuffer(buff) ((buff)->n = 0)


#ವ್ಯಾಖ್ಯಾನಿಸು luaZ_resizebuffer(L, buff, ಗಾತ್ರ_2) \
	((buff)->ಬಫರ್ = luaM_reallocvchar(L, (buff)->ಬಫರ್, \
				(buff)->buffsize, ಗಾತ್ರ_2), \
	(buff)->buffsize = ಗಾತ್ರ_2)

#ವ್ಯಾಖ್ಯಾನಿಸು luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC ಶೂನ್ಯ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        ಶೂನ್ಯ *ದತ್ತಾಂಶ);
LUAI_FUNC size_t luaZ_read (ZIO* z, ಶೂನ್ಯ *b, size_t n);	/* read next n bytes */

LUAI_FUNC ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

ರಚನೆ Zio {
  size_t n;			/* bytes still unread */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  ಶೂನ್ಯ *ದತ್ತಾಂಶ;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC ಪೂರ್ಣಾಂಕ luaZ_fill (ZIO *z);

#ಆದರೆ_ಅಂತ್ಯ
