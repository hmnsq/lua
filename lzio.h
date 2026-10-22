/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lmem.h"


#define EOZ	(-1)			/* end of stream */

প্রকারনির্দেশ সংরচনা Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


প্রকারনির্দেশ সংরচনা Mbuffer {
  বর্ণ *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = রিক্ত, (buff)->buffsize = 0)

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


LUAI_FUNC শূন্য luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        শূন্য *data);
LUAI_FUNC size_t luaZ_read (ZIO* z, শূন্য *b, size_t n);	/* read next n bytes */

LUAI_FUNC ধ্রুব শূন্য *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

সংরচনা Zio {
  size_t n;			/* bytes still unread */
  ধ্রুব বর্ণ *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  শূন্য *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC পূর্ণাঙ্ক luaZ_fill (ZIO *z);

#endif
