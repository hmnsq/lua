/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ifndef lmem_h
#define lmem_h


#அந்தர்பாவ <stddef.h>

#அந்தர்பாவ "llimits.h"
#அந்தர்பாவ "lua.h"


#define luaM_error(L)	luaD_throw(L, LUA_ERRMEM)


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
#define luaM_testsize(n,e)  \
	(அளவு(n) >= அளவு(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#define luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#define luaM_limitN(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/அளவு(t)) ? (n) :  \
     cast_int((MAX_SIZET/அளவு(t))))


/*
** Arrays of chars do not need any test
*/
#define luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*அளவு(எழுத்து), (n)*அளவு(எழுத்து)))

#define luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#define luaM_free(L, b)		luaM_free_(L, (b), அளவு(*(b)))
#define luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*அளவு(*(b)))

#define luaM_new(L,t)		cast(t*, luaM_malloc_(L, அளவு(t), 0))
#define luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*அளவு(t), 0))
#define luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,அளவு(t)), luaM_newvector(L,n,t))

#define luaM_newobject(L,tag,s)	luaM_malloc_(L, (s), tag)

#define luaM_newblock(L, size)	luaM_newvector(L, size, எழுத்து)

#define luaM_growvector(L,v,nelems,size,t,limit,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(size),அளவு(t), \
                         luaM_limitN(limit,t),e)))

#define luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * அளவு(t), \
                                  cast_sizet(n) * அளவு(t))))

#define luaM_shrinkvector(L,v,size,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(size), fs, அளவு(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC வெறுமை *luaM_realloc_ (lua_State *L, வெறுமை *block, size_t oldsize,
                                                          size_t size);
LUAI_FUNC வெறுமை *luaM_saferealloc_ (lua_State *L, வெறுமை *block, size_t oldsize,
                                                              size_t size);
LUAI_FUNC வெறுமை luaM_free_ (lua_State *L, வெறுமை *block, size_t osize);
LUAI_FUNC வெறுமை *luaM_growaux_ (lua_State *L, வெறுமை *block, முழு nelems,
                               முழு *size, unsigned size_elem, முழு limit,
                               மாறிலி எழுத்து *what);
LUAI_FUNC வெறுமை *luaM_shrinkvector_ (lua_State *L, வெறுமை *block, முழு *nelem,
                                    முழு final_n, unsigned size_elem);
LUAI_FUNC வெறுமை *luaM_malloc_ (lua_State *L, size_t size, முழு tag);

#endif

