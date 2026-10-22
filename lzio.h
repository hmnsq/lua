/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#اگر_غیر_تعریف_شدہ lzio_h
#تعریف_کرو lzio_h

#شامل "lua.h"

#شامل "lmem.h"


#تعریف_کرو EOZ	(-1)			/* end of stream */

قسمتعریف ساخت Zio ZIO;

#تعریف_کرو zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


قسمتعریف ساخت Mbuffer {
  حرف *بفر;
  size_t n;
  size_t buffsize;
} Mbuffer;

#تعریف_کرو luaZ_initbuffer(L, buff) ((buff)->بفر = تہی, (buff)->buffsize = 0)

#تعریف_کرو lua_Z_بفر(buff)	((buff)->بفر)
#تعریف_کرو luaZ_sizebuffer(buff)	((buff)->buffsize)
#تعریف_کرو luaZ_bufflen(buff)	((buff)->n)

#تعریف_کرو luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#تعریف_کرو luaZ_resetbuffer(buff) ((buff)->n = 0)


#تعریف_کرو luaZ_resizebuffer(L, buff, سائز) \
	((buff)->بفر = luaM_reallocvchar(L, (buff)->بفر, \
				(buff)->buffsize, سائز), \
	(buff)->buffsize = سائز)

#تعریف_کرو luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC خالی luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        خالی *ڈیٹا);
LUAI_FUNC size_t luaZ_read (ZIO* z, خالی *b, size_t n);	/* read next n bytes */

LUAI_FUNC ثابت خالی *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

ساخت Zio {
  size_t n;			/* bytes still unread */
  ثابت حرف *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  خالی *ڈیٹا;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC صحیح luaZ_fill (ZIO *z);

#اگر_ختم
