/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#अगर_अघोषित lzio_h
#घोषणा lzio_h

#समावेश "lua.h"

#समावेश "lmem.h"


#घोषणा EOZ	(-1)			/* end of stream */

प्रकार काष्ठा Zio ZIO;

#घोषणा zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


प्रकार काष्ठा Mbuffer {
  अक्षर *बफर;
  माप_प्रकार n;
  माप_प्रकार buffsize;
} Mbuffer;

#घोषणा luaZ_initbuffer(L, buff) ((buff)->बफर = शून्य, (buff)->buffsize = 0)

#घोषणा lua_Z_बफर(buff)	((buff)->बफर)
#घोषणा luaZ_sizebuffer(buff)	((buff)->buffsize)
#घोषणा luaZ_bufflen(buff)	((buff)->n)

#घोषणा luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#घोषणा luaZ_resetbuffer(buff) ((buff)->n = 0)


#घोषणा luaZ_resizebuffer(L, buff, आकार) \
	((buff)->बफर = luaM_reallocvchar(L, (buff)->बफर, \
				(buff)->buffsize, आकार), \
	(buff)->buffsize = आकार)

#घोषणा luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC व्योम luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        व्योम *तथ्य);
LUAI_FUNC माप_प्रकार luaZ_read (ZIO* z, व्योम *b, माप_प्रकार n);	/* read next n bytes */

LUAI_FUNC स्थिर व्योम *luaZ_getaddr (ZIO* z, माप_प्रकार n);


/* --------- Private Part ------------------ */

काष्ठा Zio {
  माप_प्रकार n;			/* bytes still unread */
  स्थिर अक्षर *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  व्योम *तथ्य;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC पूर्णांक luaZ_fill (ZIO *z);

#पूर्ण_अगर
