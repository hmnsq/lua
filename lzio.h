/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#నిర్వచితం_కాకపోతే lzio_h
#నిర్వచించు lzio_h

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lmem.h"


#నిర్వచించు EOZ	(-1)			/* end of stream */

రకనిర్వచనం నిర్మాణం Zio ZIO;

#నిర్వచించు zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


రకనిర్వచనం నిర్మాణం Mbuffer {
  అక్షరం *బఫరు;
  size_t n;
  size_t buffsize;
} Mbuffer;

#నిర్వచించు luaZ_initbuffer(L, buff) ((buff)->బఫరు = ఖాళీ, (buff)->buffsize = 0)

#నిర్వచించు lua_Z_బఫరు(buff)	((buff)->బఫరు)
#నిర్వచించు luaZ_sizebuffer(buff)	((buff)->buffsize)
#నిర్వచించు luaZ_bufflen(buff)	((buff)->n)

#నిర్వచించు luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#నిర్వచించు luaZ_resetbuffer(buff) ((buff)->n = 0)


#నిర్వచించు luaZ_resizebuffer(L, buff, కొలత) \
	((buff)->బఫరు = luaM_reallocvchar(L, (buff)->బఫరు, \
				(buff)->buffsize, కొలత), \
	(buff)->buffsize = కొలత)

#నిర్వచించు luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC శూన్య luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        శూన్య *దత్తాంశం);
LUAI_FUNC size_t luaZ_read (ZIO* z, శూన్య *b, size_t n);	/* read next n bytes */

LUAI_FUNC స్థిరరాశి శూన్య *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

నిర్మాణం Zio {
  size_t n;			/* bytes still unread */
  స్థిరరాశి అక్షరం *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  శూన్య *దత్తాంశం;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC పూర్ణాంకం luaZ_fill (ZIO *z);

#అయితే_ముగింపు
