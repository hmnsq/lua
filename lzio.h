/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#که_نه_تعریف_شوی lzio_h
#تعریف_کړه lzio_h

#ضمن "lua.h"

#ضمن "lmem.h"


#تعریف_کړه EOZ	(-1)			/* end of stream */

تعريفنوع بنية Zio ZIO;

#تعریف_کړه zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


تعريفنوع بنية Mbuffer {
  حرف *بفر;
  size_t n;
  size_t buffsize;
} Mbuffer;

#تعریف_کړه luaZ_initbuffer(L, buff) ((buff)->بفر = عدم, (buff)->buffsize = 0)

#تعریف_کړه lua_Z_بفر(buff)	((buff)->بفر)
#تعریف_کړه luaZ_sizebuffer(buff)	((buff)->buffsize)
#تعریف_کړه luaZ_bufflen(buff)	((buff)->n)

#تعریف_کړه luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#تعریف_کړه luaZ_resetbuffer(buff) ((buff)->n = 0)


#تعریف_کړه luaZ_resizebuffer(L, buff, اندازه) \
	((buff)->بفر = luaM_reallocvchar(L, (buff)->بفر, \
				(buff)->buffsize, اندازه), \
	(buff)->buffsize = اندازه)

#تعریف_کړه luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC فراغ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        فراغ *معلومات);
LUAI_FUNC size_t luaZ_read (ZIO* z, فراغ *b, size_t n);	/* read next n bytes */

LUAI_FUNC ثابت فراغ *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

بنية Zio {
  size_t n;			/* bytes still unread */
  ثابت حرف *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  فراغ *معلومات;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC صحيح luaZ_fill (ZIO *z);

#د_که_پای
