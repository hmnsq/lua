/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef lzio_h
#define lzio_h

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lmem.h"


#define EOZ	(-1)			/* end of stream */

തരനിർവചനം ഘടന Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


തരനിർവചനം ഘടന Mbuffer {
  അക്ഷരം *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->buffer = ശൂന്യ, (buff)->buffsize = 0)

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


LUAI_FUNC ശൂന്യം luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        ശൂന്യം *data);
LUAI_FUNC size_t luaZ_read (ZIO* z, ശൂന്യം *b, size_t n);	/* read next n bytes */

LUAI_FUNC സ്ഥിരം ശൂന്യം *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

ഘടന Zio {
  size_t n;			/* bytes still unread */
  സ്ഥിരം അക്ഷരം *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  ശൂന്യം *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC പൂർണ്ണസംഖ്യ luaZ_fill (ZIO *z);

#endif
