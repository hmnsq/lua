/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lmem_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lmem_h


#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_error(L)	luaD_throw(L, LUA_ERRMEM)


/*
** This macro tests whether it is safe to multiply 'n' by the size of
** type 't' without overflows. Because 'e' is always constant, it avoids
** the runtime division MAX_SIZET/(e).
** (The macro is somewhat complex to avoid warnings:  The 'sizeof'
** comparison avoids a runtime comparison when overflow cannot occur.
** The compiler should be able to optimize the real test by itself, but
** when it does it, it may give a warning about "comparison is always
** false due to limited range of data type"; the +1 tricks the compiler,
** avoiding this warning but also this optimization.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_testsize(n,e)  \
	(ਆਕਾਰ(n) >= ਆਕਾਰ(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_M_ਹੱਦਬੰਦੀ_N(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/ਆਕਾਰ(t)) ? (n) :  \
     cast_int((MAX_SIZET/ਆਕਾਰ(t))))


/*
** Arrays of chars do not need any test
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*ਆਕਾਰ(ਅੱਖਰ), (n)*ਆਕਾਰ(ਅੱਖਰ)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_free(L, b)		luaM_free_(L, (b), ਆਕਾਰ(*(b)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*ਆਕਾਰ(*(b)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_new(L,t)		cast(t*, luaM_malloc_(L, ਆਕਾਰ(t), 0))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*ਆਕਾਰ(t), 0))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,ਆਕਾਰ(t)), luaM_newvector(L,n,t))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_newobject(L,ਟੈਗ,s)	luaM_malloc_(L, (s), ਟੈਗ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_newblock(L, ਮਾਪ)	luaM_newvector(L, ਮਾਪ, ਅੱਖਰ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_growvector(L,v,nelems,ਮਾਪ,t,ਹੱਦਬੰਦੀ,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(ਮਾਪ),ਆਕਾਰ(t), \
                         lua_M_ਹੱਦਬੰਦੀ_N(ਹੱਦਬੰਦੀ,t),e)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * ਆਕਾਰ(t), \
                                  cast_sizet(n) * ਆਕਾਰ(t))))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaM_shrinkvector(L,v,ਮਾਪ,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(ਮਾਪ), fs, ਆਕਾਰ(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC ਖਾਲੀ *luaM_realloc_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, size_t oldsize,
                                                          size_t ਮਾਪ);
LUAI_FUNC ਖਾਲੀ *luaM_saferealloc_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, size_t oldsize,
                                                              size_t ਮਾਪ);
LUAI_FUNC ਖਾਲੀ luaM_free_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, size_t osize);
LUAI_FUNC ਖਾਲੀ *luaM_growaux_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, ਪੂਰਨ nelems,
                               ਪੂਰਨ *ਮਾਪ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_elem, ਪੂਰਨ ਹੱਦਬੰਦੀ,
                               ਅਚਲ ਅੱਖਰ *what);
LUAI_FUNC ਖਾਲੀ *luaM_shrinkvector_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, ਪੂਰਨ *nelem,
                                    ਪੂਰਨ final_n, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_elem);
LUAI_FUNC ਖਾਲੀ *luaM_malloc_ (lua_State *L, size_t ਮਾਪ, ਪੂਰਨ ਟੈਗ);

#ਜੇ_ਅੰਤ

