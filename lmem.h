/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل lmem_h
#وصف_ڪريو lmem_h


#ضمن <stddef.h>

#ضمن "llimits.h"
#ضمن "lua.h"


#وصف_ڪريو luaM_error(L)	luaD_throw(L, LUA_ERRMEM)


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
#وصف_ڪريو luaM_testsize(n,e)  \
	(حجم(n) >= حجم(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#وصف_ڪريو luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#وصف_ڪريو lua_M_حد_بندي_N(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/حجم(t)) ? (n) :  \
     cast_int((MAX_SIZET/حجم(t))))


/*
** Arrays of chars do not need any test
*/
#وصف_ڪريو luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*حجم(حرف), (n)*حجم(حرف)))

#وصف_ڪريو luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#وصف_ڪريو luaM_free(L, b)		luaM_free_(L, (b), حجم(*(b)))
#وصف_ڪريو luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*حجم(*(b)))

#وصف_ڪريو luaM_new(L,t)		cast(t*, luaM_malloc_(L, حجم(t), 0))
#وصف_ڪريو luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*حجم(t), 0))
#وصف_ڪريو luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,حجم(t)), luaM_newvector(L,n,t))

#وصف_ڪريو luaM_newobject(L,ٽيگ,s)	luaM_malloc_(L, (s), ٽيگ)

#وصف_ڪريو luaM_newblock(L, سائيز)	luaM_newvector(L, سائيز, حرف)

#وصف_ڪريو luaM_growvector(L,v,nelems,سائيز,t,حد_بندي,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(سائيز),حجم(t), \
                         lua_M_حد_بندي_N(حد_بندي,t),e)))

#وصف_ڪريو luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * حجم(t), \
                                  cast_sizet(n) * حجم(t))))

#وصف_ڪريو luaM_shrinkvector(L,v,سائيز,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(سائيز), fs, حجم(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC فراغ *luaM_realloc_ (lua_State *L, فراغ *بلاڪ, size_t oldsize,
                                                          size_t سائيز);
LUAI_FUNC فراغ *luaM_saferealloc_ (lua_State *L, فراغ *بلاڪ, size_t oldsize,
                                                              size_t سائيز);
LUAI_FUNC فراغ luaM_free_ (lua_State *L, فراغ *بلاڪ, size_t osize);
LUAI_FUNC فراغ *luaM_growaux_ (lua_State *L, فراغ *بلاڪ, صحيح nelems,
                               صحيح *سائيز, بنانشان سائيز_elem, صحيح حد_بندي,
                               ثابت حرف *what);
LUAI_FUNC فراغ *luaM_shrinkvector_ (lua_State *L, فراغ *بلاڪ, صحيح *nelem,
                                    صحيح final_n, بنانشان سائيز_elem);
LUAI_FUNC فراغ *luaM_malloc_ (lua_State *L, size_t سائيز, صحيح ٽيگ);

#جيڪڏهن_پورو

