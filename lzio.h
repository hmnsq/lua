/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lmem.h"


#define EOZ	(-1)			/* end of stream */

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Mbuffer {
  ವರ್ಣ *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = ರಿಕ್ತ, (buff)->buffsize = 0)

#define luaZ_buffer(buff)	((buff)->buffer)
#define luaZ_sizebuffer(buff)	((buff)->buffsize)
#define luaZ_bufflen(buff)	((buff)->n)

#define luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#define luaZ_resetbuffer(buff) ((buff)->n = 0)


#define luaZ_resizebuffer(L, buff, size) \
	((buff)->buffer = luaM_reallocvchar(L, (buff)->buffer, \
				(buff)->buffsize, size), \
	(buff)->buffsize = size)

#define luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC ಶೂನ್ಯ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        ಶೂನ್ಯ *data);
LUAI_FUNC size_t luaZ_read (ZIO* z, ಶೂನ್ಯ *b, size_t n);	/* read next n bytes */

LUAI_FUNC ಧ್ರುವ ಶೂನ್ಯ *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

ಸಂರಚನಾ Zio {
  size_t n;			/* bytes still unread */
  ಧ್ರುವ ವರ್ಣ *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  ಶೂನ್ಯ *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaZ_fill (ZIO *z);

#endif
