/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lmem_h
#నిర్వచించు lmem_h


#అన్తర్భావ <stddef.h>

#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lua.h"


#నిర్వచించు luaM_error(L)	luaD_throw(L, LUA_ERRMEM)


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
#నిర్వచించు luaM_testsize(n,e)  \
	(పరిమాణం(n) >= పరిమాణం(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#నిర్వచించు luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#నిర్వచించు lua_M_హద్దు_N(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/పరిమాణం(t)) ? (n) :  \
     cast_int((MAX_SIZET/పరిమాణం(t))))


/*
** Arrays of chars do not need any test
*/
#నిర్వచించు luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*పరిమాణం(అక్షరం), (n)*పరిమాణం(అక్షరం)))

#నిర్వచించు luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#నిర్వచించు luaM_free(L, b)		luaM_free_(L, (b), పరిమాణం(*(b)))
#నిర్వచించు luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*పరిమాణం(*(b)))

#నిర్వచించు luaM_new(L,t)		cast(t*, luaM_malloc_(L, పరిమాణం(t), 0))
#నిర్వచించు luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*పరిమాణం(t), 0))
#నిర్వచించు luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,పరిమాణం(t)), luaM_newvector(L,n,t))

#నిర్వచించు luaM_newobject(L,ట్యాగ్,s)	luaM_malloc_(L, (s), ట్యాగ్)

#నిర్వచించు luaM_newblock(L, కొలత)	luaM_newvector(L, కొలత, అక్షరం)

#నిర్వచించు luaM_growvector(L,v,nelems,కొలత,t,హద్దు,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(కొలత),పరిమాణం(t), \
                         lua_M_హద్దు_N(హద్దు,t),e)))

#నిర్వచించు luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * పరిమాణం(t), \
                                  cast_sizet(n) * పరిమాణం(t))))

#నిర్వచించు luaM_shrinkvector(L,v,కొలత,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(కొలత), fs, పరిమాణం(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC శూన్య *luaM_realloc_ (lua_State *L, శూన్య *ఖండం, size_t oldsize,
                                                          size_t కొలత);
LUAI_FUNC శూన్య *luaM_saferealloc_ (lua_State *L, శూన్య *ఖండం, size_t oldsize,
                                                              size_t కొలత);
LUAI_FUNC శూన్య luaM_free_ (lua_State *L, శూన్య *ఖండం, size_t osize);
LUAI_FUNC శూన్య *luaM_growaux_ (lua_State *L, శూన్య *ఖండం, పూర్ణాంకం nelems,
                               పూర్ణాంకం *కొలత, గుర్తులేని కొలత_elem, పూర్ణాంకం హద్దు,
                               స్థిరరాశి అక్షరం *what);
LUAI_FUNC శూన్య *luaM_shrinkvector_ (lua_State *L, శూన్య *ఖండం, పూర్ణాంకం *nelem,
                                    పూర్ణాంకం final_n, గుర్తులేని కొలత_elem);
LUAI_FUNC శూన్య *luaM_malloc_ (lua_State *L, size_t కొలత, పూర్ణాంకం ట్యాగ్);

#అయితే_ముగింపు

