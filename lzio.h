/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lmem.h"


#define EOZ	(-1)			/* end of stream */

வகைவரை கட்டமைப்பு Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


வகைவரை கட்டமைப்பு Mbuffer {
  எழுத்து *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = வெற்று, (buff)->buffsize = 0)

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


LUAI_FUNC வெறுமை luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        வெறுமை *data);
LUAI_FUNC size_t luaZ_read (ZIO* z, வெறுமை *b, size_t n);	/* read next n bytes */

LUAI_FUNC மாறிலி வெறுமை *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

கட்டமைப்பு Zio {
  size_t n;			/* bytes still unread */
  மாறிலி எழுத்து *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  வெறுமை *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC முழு luaZ_fill (ZIO *z);

#endif
