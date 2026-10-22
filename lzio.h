/*
** $Id: lzio.h $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lzio_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lzio_h

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EOZ	(-1)			/* end of stream */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Zio ZIO;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ zgetc(z)  (((z)->n--)>0 ?  cast_uchar(*(z)->p++) : luaZ_fill(z))


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Mbuffer {
  ਅੱਖਰ *ਬਫਰ;
  size_t n;
  size_t buffsize;
} Mbuffer;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaZ_initbuffer(L, buff) ((buff)->ਬਫਰ = ਨਿਰਾ, (buff)->buffsize = 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_Z_ਬਫਰ(buff)	((buff)->ਬਫਰ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaZ_sizebuffer(buff)	((buff)->buffsize)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaZ_bufflen(buff)	((buff)->n)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaZ_buffremove(buff,i)	((buff)->n -= cast_sizet(i))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaZ_resetbuffer(buff) ((buff)->n = 0)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaZ_resizebuffer(L, buff, ਮਾਪ) \
	((buff)->ਬਫਰ = luaM_reallocvchar(L, (buff)->ਬਫਰ, \
				(buff)->buffsize, ਮਾਪ), \
	(buff)->buffsize = ਮਾਪ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)


LUAI_FUNC ਖਾਲੀ luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        ਖਾਲੀ *ਅੰਕੜੇ);
LUAI_FUNC size_t luaZ_read (ZIO* z, ਖਾਲੀ *b, size_t n);	/* read next n bytes */

LUAI_FUNC ਅਚਲ ਖਾਲੀ *luaZ_getaddr (ZIO* z, size_t n);


/* --------- Private Part ------------------ */

ਬਣਤਰ Zio {
  size_t n;			/* bytes still unread */
  ਅਚਲ ਅੱਖਰ *p;		/* current position in buffer */
  lua_Reader reader;		/* reader function */
  ਖਾਲੀ *ਅੰਕੜੇ;			/* additional data */
  lua_State *L;			/* Lua state (for reader) */
};


LUAI_FUNC ਪੂਰਨ luaZ_fill (ZIO *z);

#ਜੇ_ਅੰਤ
