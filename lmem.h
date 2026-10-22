/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lmem_h
#ವ್ಯಾಖ್ಯಾನಿಸು lmem_h


#ಅನ್ತರ್ಭಾವ <stddef.h>

#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lua.h"


#ವ್ಯಾಖ್ಯಾನಿಸು luaM_error(L)	luaD_throw(L, LUA_ERRMEM)


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
#ವ್ಯಾಖ್ಯಾನಿಸು luaM_testsize(n,e)  \
	(ಗಾತ್ರ(n) >= ಗಾತ್ರ(size_t) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು lua_M_ಎಲ್ಲೆ_N(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/ಗಾತ್ರ(t)) ? (n) :  \
     cast_int((MAX_SIZET/ಗಾತ್ರ(t))))


/*
** Arrays of chars do not need any test
*/
#ವ್ಯಾಖ್ಯಾನಿಸು luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*ಗಾತ್ರ(ಅಕ್ಷರ), (n)*ಗಾತ್ರ(ಅಕ್ಷರ)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#ವ್ಯಾಖ್ಯಾನಿಸು luaM_free(L, b)		luaM_free_(L, (b), ಗಾತ್ರ(*(b)))
#ವ್ಯಾಖ್ಯಾನಿಸು luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*ಗಾತ್ರ(*(b)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_new(L,t)		cast(t*, luaM_malloc_(L, ಗಾತ್ರ(t), 0))
#ವ್ಯಾಖ್ಯಾನಿಸು luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*ಗಾತ್ರ(t), 0))
#ವ್ಯಾಖ್ಯಾನಿಸು luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,ಗಾತ್ರ(t)), luaM_newvector(L,n,t))

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_newobject(L,ಟ್ಯಾಗ್,s)	luaM_malloc_(L, (s), ಟ್ಯಾಗ್)

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_newblock(L, ಗಾತ್ರ_2)	luaM_newvector(L, ಗಾತ್ರ_2, ಅಕ್ಷರ)

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_growvector(L,v,nelems,ಗಾತ್ರ_2,t,ಎಲ್ಲೆ,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(ಗಾತ್ರ_2),ಗಾತ್ರ(t), \
                         lua_M_ಎಲ್ಲೆ_N(ಎಲ್ಲೆ,t),e)))

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * ಗಾತ್ರ(t), \
                                  cast_sizet(n) * ಗಾತ್ರ(t))))

#ವ್ಯಾಖ್ಯಾನಿಸು luaM_shrinkvector(L,v,ಗಾತ್ರ_2,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(ಗಾತ್ರ_2), fs, ಗಾತ್ರ(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC ಶೂನ್ಯ *luaM_realloc_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, size_t oldsize,
                                                          size_t ಗಾತ್ರ_2);
LUAI_FUNC ಶೂನ್ಯ *luaM_saferealloc_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, size_t oldsize,
                                                              size_t ಗಾತ್ರ_2);
LUAI_FUNC ಶೂನ್ಯ luaM_free_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, size_t osize);
LUAI_FUNC ಶೂನ್ಯ *luaM_growaux_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, ಪೂರ್ಣಾಂಕ nelems,
                               ಪೂರ್ಣಾಂಕ *ಗಾತ್ರ_2, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_elem, ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ,
                               ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what);
LUAI_FUNC ಶೂನ್ಯ *luaM_shrinkvector_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, ಪೂರ್ಣಾಂಕ *nelem,
                                    ಪೂರ್ಣಾಂಕ final_n, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_elem);
LUAI_FUNC ಶೂನ್ಯ *luaM_malloc_ (lua_State *L, size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್);

#ಆದರೆ_ಅಂತ್ಯ

