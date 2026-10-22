/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lmem.h"


#define EOZ	(-1)			/* end of stream */

प्रकारनिर्देश संरचना Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


प्रकारनिर्देश संरचना Mbuffer {
  वर्ण *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = रिक्त, (buff)->buffsize = 0)

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


LUAI_FUNC शून्य luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        शून्य *data);
LUAI_FUNC size_t luaZ_read (ZIO* z, शून्य *b, size_t n);	/* read next n bytes */

LUAI_FUNC ध्रुव शून्य *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

संरचना Zio {
  size_t n;			/* bytes still unread */
  ध्रुव वर्ण *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  शून्य *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC पूर्णाङ्क luaZ_fill (ZIO *z);

#endif
