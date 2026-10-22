/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lmem.h"


#define EOZ	(-1)			/* end of stream */

પ્રકારવ્યાખ્યા સંરચના Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


પ્રકારવ્યાખ્યા સંરચના Mbuffer {
  અક્ષર *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = ખાલી, (buff)->buffsize = 0)

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


LUAI_FUNC શૂન્ય luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        શૂન્ય *data);
LUAI_FUNC size_t luaZ_read (ZIO* z, શૂન્ય *b, size_t n);	/* read next n bytes */

LUAI_FUNC અચલ શૂન્ય *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

સંરચના Zio {
  size_t n;			/* bytes still unread */
  અચલ અક્ષર *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  શૂન્ય *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC પૂર્ણાંક luaZ_fill (ZIO *z);

#endif
