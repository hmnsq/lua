/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ifndef lmem_h
#define lmem_h


#अन्तर्भाव <stddef.h>

#अन्तर्भाव "llimits.h"
#अन्तर्भाव "lua.h"


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
	(परिमाण(n) >= परिमाण(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#define luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#define luaM_limitN(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/परिमाण(t)) ? (n) :  \
     cast_int((MAX_SIZET/परिमाण(t))))


/*
** Arrays of chars do not need any test
*/
#define luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*परिमाण(वर्ण), (n)*परिमाण(वर्ण)))

#define luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#define luaM_free(L, b)		luaM_free_(L, (b), परिमाण(*(b)))
#define luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*परिमाण(*(b)))

#define luaM_new(L,t)		cast(t*, luaM_malloc_(L, परिमाण(t), 0))
#define luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*परिमाण(t), 0))
#define luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,परिमाण(t)), luaM_newvector(L,n,t))

#define luaM_newobject(L,tag,s)	luaM_malloc_(L, (s), tag)

#define luaM_newblock(L, size)	luaM_newvector(L, size, वर्ण)

#define luaM_growvector(L,v,nelems,size,t,limit,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(size),परिमाण(t), \
                         luaM_limitN(limit,t),e)))

#define luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * परिमाण(t), \
                                  cast_sizet(n) * परिमाण(t))))

#define luaM_shrinkvector(L,v,size,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(size), fs, परिमाण(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC शून्य *luaM_realloc_ (lua_State *L, शून्य *block, size_t oldsize,
                                                          size_t size);
LUAI_FUNC शून्य *luaM_saferealloc_ (lua_State *L, शून्य *block, size_t oldsize,
                                                              size_t size);
LUAI_FUNC शून्य luaM_free_ (lua_State *L, शून्य *block, size_t osize);
LUAI_FUNC शून्य *luaM_growaux_ (lua_State *L, शून्य *block, पूर्णाङ्क nelems,
                               पूर्णाङ्क *size, unsigned size_elem, पूर्णाङ्क limit,
                               ध्रुव वर्ण *what);
LUAI_FUNC शून्य *luaM_shrinkvector_ (lua_State *L, शून्य *block, पूर्णाङ्क *nelem,
                                    पूर्णाङ्क final_n, unsigned size_elem);
LUAI_FUNC शून्य *luaM_malloc_ (lua_State *L, size_t size, पूर्णाङ्क tag);

#endif

