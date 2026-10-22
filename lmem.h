/*
** $Id: lmem.h $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#अगर_अघोषित lmem_h
#घोषणा lmem_h


#समावेश <stddef.h>

#समावेश "llimits.h"
#समावेश "lua.h"


#घोषणा luaM_error(L)	luaD_throw(L, LUA_ERRMEM)


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
#घोषणा luaM_testsize(n,e)  \
	(माप(n) >= माप(माप_प्रकार) && cast_sizet((n)) + 1 > MAX_SIZET/(e))

#घोषणा luaM_checksize(L,n,e)  \
	(luaM_testsize(n,e) ? luaM_toobig(L) : cast_void(0))


/*
** Computes the minimum between 'n' and 'MAX_SIZET/sizeof(t)', so that
** the result is not larger than 'n' and cannot overflow a 'size_t'
** when multiplied by the size of type 't'. (Assumes that 'n' is an
** 'int' and that 'int' is not larger than 'size_t'.)
*/
#घोषणा luaM_limitN(n,t)  \
  ((cast_sizet(n) <= MAX_SIZET/माप(t)) ? (n) :  \
     cast_int((MAX_SIZET/माप(t))))


/*
** Arrays of chars do not need any test
*/
#घोषणा luaM_reallocvchar(L,b,on,n)  \
  cast_charp(luaM_saferealloc_(L, (b), (on)*माप(अक्षर), (n)*माप(अक्षर)))

#घोषणा luaM_freemem(L, b, s)	luaM_free_(L, (b), (s))
#घोषणा luaM_free(L, b)		luaM_free_(L, (b), माप(*(b)))
#घोषणा luaM_freearray(L, b, n)   luaM_free_(L, (b), (n)*माप(*(b)))

#घोषणा luaM_new(L,t)		cast(t*, luaM_malloc_(L, माप(t), 0))
#घोषणा luaM_newvector(L,n,t)  \
	cast(t*, luaM_malloc_(L, cast_sizet(n)*माप(t), 0))
#घोषणा luaM_newvectorchecked(L,n,t) \
  (luaM_checksize(L,n,माप(t)), luaM_newvector(L,n,t))

#घोषणा luaM_newobject(L,tag,s)	luaM_malloc_(L, (s), tag)

#घोषणा luaM_newblock(L, आकार)	luaM_newvector(L, आकार, अक्षर)

#घोषणा luaM_growvector(L,v,nelems,आकार,t,limit,e) \
	((v)=cast(t *, luaM_growaux_(L,v,nelems,&(आकार),माप(t), \
                         luaM_limitN(limit,t),e)))

#घोषणा luaM_reallocvector(L, v,oldn,n,t) \
   (cast(t *, luaM_realloc_(L, v, cast_sizet(oldn) * माप(t), \
                                  cast_sizet(n) * माप(t))))

#घोषणा luaM_shrinkvector(L,v,आकार,fs,t) \
   ((v)=cast(t *, luaM_shrinkvector_(L, v, &(आकार), fs, माप(t))))

LUAI_FUNC l_noret luaM_toobig (lua_State *L);

/* not to be called directly */
LUAI_FUNC व्योम *luaM_realloc_ (lua_State *L, व्योम *block, माप_प्रकार oldsize,
                                                          माप_प्रकार आकार);
LUAI_FUNC व्योम *luaM_saferealloc_ (lua_State *L, व्योम *block, माप_प्रकार oldsize,
                                                              माप_प्रकार आकार);
LUAI_FUNC व्योम luaM_free_ (lua_State *L, व्योम *block, माप_प्रकार osize);
LUAI_FUNC व्योम *luaM_growaux_ (lua_State *L, व्योम *block, पूर्णांक nelems,
                               पूर्णांक *आकार, अचिन्हित आकार_elem, पूर्णांक limit,
                               स्थिर अक्षर *what);
LUAI_FUNC व्योम *luaM_shrinkvector_ (lua_State *L, व्योम *block, पूर्णांक *nelem,
                                    पूर्णांक final_n, अचिन्हित आकार_elem);
LUAI_FUNC व्योम *luaM_malloc_ (lua_State *L, माप_प्रकार आकार, पूर्णांक tag);

#पूर्ण_अगर

