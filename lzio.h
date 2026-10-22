/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#اگر_تعریف_نشده lzio_h
#تعریف_کن lzio_h

#ضمن "lua.h"

#ضمن "lmem.h"


#تعریف_کن EOZ	(-1)			/* end of stream */

تعريفنوع بنية Zio ZIO;

#تعریف_کن zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


تعريفنوع بنية Mbuffer {
  حرف *میانگیر;
  size_t n;
  size_t buffsize;
} Mbuffer;

#تعریف_کن luaZ_initbuffer(L, buff) ((buff)->میانگیر = عدم, (buff)->buffsize = 0)

#تعریف_کن lua_Z_میانگیر(buff)	((buff)->میانگیر)
#تعریف_کن luaZ_sizebuffer(buff)	((buff)->buffsize)
#تعریف_کن luaZ_bufflen(buff)	((buff)->n)

#تعریف_کن luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#تعریف_کن luaZ_resetbuffer(buff) ((buff)->n = 0)


#تعریف_کن luaZ_resizebuffer(L, buff, اندازه) \
	((buff)->میانگیر = luaM_reallocvchar(L, (buff)->میانگیر, \
				(buff)->buffsize, اندازه), \
	(buff)->buffsize = اندازه)

#تعریف_کن luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC فراغ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        فراغ *داده);
LUAI_FUNC size_t luaZ_read (ZIO* z, فراغ *b, size_t n);	/* read next n bytes */

LUAI_FUNC ثابت فراغ *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

بنية Zio {
  size_t n;			/* bytes still unread */
  ثابت حرف *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  فراغ *داده;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC صحيح luaZ_fill (ZIO *z);

#پایان_اگر
