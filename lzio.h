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

প্রকারলেখ কাঠামো Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


প্রকারলেখ কাঠামো Mbuffer {
  অক্ষর *বাফার;
  size_t n;
  size_t buffsize;
} Mbuffer;

#define luaZ_initbuffer(L, buff) ((buff)->বাফার = ফাঁকা, (buff)->buffsize = 0)

#define lua_Z_বাফার(buff)	((buff)->বাফার)
#define luaZ_sizebuffer(buff)	((buff)->buffsize)
#define luaZ_bufflen(buff)	((buff)->n)

#define luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#define luaZ_resetbuffer(buff) ((buff)->n = 0)


#define luaZ_resizebuffer(L, buff, আকার_2) \
	((buff)->বাফার = luaM_reallocvchar(L, (buff)->বাফার, \
				(buff)->buffsize, আকার_2), \
	(buff)->buffsize = আকার_2)

#define luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC শূন্য luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        শূন্য *তথ্য);
LUAI_FUNC size_t luaZ_read (ZIO* z, শূন্য *b, size_t n);	/* read next n bytes */

LUAI_FUNC ধ্রুবক শূন্য *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

কাঠামো Zio {
  size_t n;			/* bytes still unread */
  ধ্রুবক অক্ষর *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  শূন্য *তথ্য;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC পূর্ণসংখ্যা luaZ_fill (ZIO *z);

#endif
