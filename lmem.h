/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی lmem_h
#تعریف_کړه lmem_h


#ضمن <stddef.h>

#ضمن "llimits.h"
#ضمن "lua.h"


#تعریف_کړه luaM_error(L)	luaD_throw(L, LUA_ERRMEM)


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
#تعریف_کړه luaM_testsize(n,e)  \
	(حجم(n) >= حجم(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#تعریف_کړه luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#تعریف_کړه lua_M_حد_N(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/حجم(t)) ? (n) :  \
     cast_int((MAX_SIZET/حجم(t))))


/*
** Arrays of chars do not need any test
*/
#تعریف_کړه luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*حجم(حرف), (n)*حجم(حرف)))

#تعریف_کړه luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#تعریف_کړه luaM_free(L, b)		luaM_free_(L, (b), حجم(*(b)))
#تعریف_کړه luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*حجم(*(b)))

#تعریف_کړه luaM_new(L,t)		cast(t*, luaM_malloc_(L, حجم(t), 0))
#تعریف_کړه luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*حجم(t), 0))
#تعریف_کړه luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,حجم(t)), luaM_newvector(L,n,t))

#تعریف_کړه luaM_newobject(L,نښان,s)	luaM_malloc_(L, (s), نښان)

#تعریف_کړه luaM_newblock(L, اندازه)	luaM_newvector(L, اندازه, حرف)

#تعریف_کړه luaM_growvector(L,v,nelems,اندازه,t,حد,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(اندازه),حجم(t), \
                         lua_M_حد_N(حد,t),e)))

#تعریف_کړه luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * حجم(t), \
                                  cast_sizet(n) * حجم(t))))

#تعریف_کړه luaM_shrinkvector(L,v,اندازه,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(اندازه), fs, حجم(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC فراغ *luaM_realloc_ (lua_State *L, فراغ *بلاک, size_t oldsize,
                                                          size_t اندازه);
LUAI_FUNC فراغ *luaM_saferealloc_ (lua_State *L, فراغ *بلاک, size_t oldsize,
                                                              size_t اندازه);
LUAI_FUNC فراغ luaM_free_ (lua_State *L, فراغ *بلاک, size_t osize);
LUAI_FUNC فراغ *luaM_growaux_ (lua_State *L, فراغ *بلاک, صحيح nelems,
                               صحيح *اندازه, بینښه اندازه_elem, صحيح حد,
                               ثابت حرف *what);
LUAI_FUNC فراغ *luaM_shrinkvector_ (lua_State *L, فراغ *بلاک, صحيح *nelem,
                                    صحيح final_n, بینښه اندازه_elem);
LUAI_FUNC فراغ *luaM_malloc_ (lua_State *L, size_t اندازه, صحيح نښان);

#د_که_پای

