/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#यदि_अपरिभाषितम् lzio_h
#परिभाषय lzio_h

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lmem.h"


#परिभाषय EOZ	(-1)			/* end of stream */

प्रकारनिर्देश संरचना Zio ZIO;

#परिभाषय zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


प्रकारनिर्देश संरचना Mbuffer {
  वर्ण *बफर;
  size_t n;
  size_t buffsize;
} Mbuffer;

#परिभाषय luaZ_initbuffer(L, buff) ((buff)->बफर = रिक्त, (buff)->buffsize = 0)

#परिभाषय lua_Z_बफर(buff)	((buff)->बफर)
#परिभाषय luaZ_sizebuffer(buff)	((buff)->buffsize)
#परिभाषय luaZ_bufflen(buff)	((buff)->n)

#परिभाषय luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#परिभाषय luaZ_resetbuffer(buff) ((buff)->n = 0)


#परिभाषय luaZ_resizebuffer(L, buff, आकार) \
	((buff)->बफर = luaM_reallocvchar(L, (buff)->बफर, \
				(buff)->buffsize, आकार), \
	(buff)->buffsize = आकार)

#परिभाषय luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC शून्य luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        शून्य *दत्त);
LUAI_FUNC size_t luaZ_read (ZIO* z, शून्य *b, size_t n);	/* read next n bytes */

LUAI_FUNC ध्रुव शून्य *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

संरचना Zio {
  size_t n;			/* bytes still unread */
  ध्रुव वर्ण *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  शून्य *दत्त;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC पूर्णाङ्क luaZ_fill (ZIO *z);

#यदि_अन्तः
