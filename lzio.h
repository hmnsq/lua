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

प्रकारव्याख्या संरचना Zio ZIO;

#define zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


प्रकारव्याख्या संरचना Mbuffer {
  अक्षर *buffer;
  माप_प्रकार n;
  माप_प्रकार buffsize;
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
LUAI_FUNC माप_प्रकार luaZ_read (ZIO* z, शून्य *b, माप_प्रकार n);	/* read next n bytes */

LUAI_FUNC स्थिरांक शून्य *luaZ_getaddr (ZIO* z, माप_प्रकार n);


/* --------- Private Part ------------------ */

संरचना Zio {
  माप_प्रकार n;			/* bytes still unread */
  स्थिरांक अक्षर *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  शून्य *data;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC पूर्णांक luaZ_fill (ZIO *z);

#endif
