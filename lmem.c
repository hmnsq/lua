/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#घोषणा lmem_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <stddef.h>

#समावेश "lua.h"

#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lgc.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lstate.h"



/*
** About the realloc function:
** void *frealloc (void *ud, void *ptr, size_t osize, size_t nsize);
** ('osize' is the old size, 'nsize' is the new size)
**
** - frealloc(ud, p, x, 0) frees the block 'p' and returns NULL.
** Particularly, frealloc(ud, NULL, 0, 0) does nothing,
** which is equivalent to free(NULL) in ISO C.
**
** - frealloc(ud, NULL, x, s) creates a new block of size 's'
** (no matter 'x'). Returns NULL if it cannot create the new block.
**
** - otherwise, frealloc(ud, b, x, y) reallocates the block 'b' from
** size 'x' to size 'y'. Returns NULL if it cannot reallocate the
** block to the new size.
*/


/*
** Macro to call the allocation function.
*/
#घोषणा callfrealloc(g,block,os,ns)    ((*g->frealloc)(g->ud, block, os, ns))


/*
** When an allocation fails, it will try again after an emergency
** collection, except when it cannot run a collection.  The GC should
** not be called while the state is not fully built, as the collector
** is not yet fully initialized. Also, it should not be called when
** 'gcstopem' is true, because then the interpreter is in the middle of
** a collection step.
*/
#घोषणा cantryagain(g)	(completestate(g) && !g->gcstopem)




#अगर defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
अटल व्योम *firsttry (global_State *g, व्योम *block, माप_प्रकार os, माप_प्रकार ns) {
  अगर (ns > 0 && cantryagain(g))
    वापस शून्य;  /* fail */
  अन्यथा  /* normal allocation */
    वापस callfrealloc(g, block, os, ns);
}
#अन्यथा
#घोषणा firsttry(g,block,os,ns)    callfrealloc(g, block, os, ns)
#पूर्ण_अगर





/*
** {==================================================================
** Functions to allocate/deallocate arrays for the Parser
** ===================================================================
*/

/*
** Minimum size for arrays during parsing, to avoid overhead of
** reallocating to size 1, then 2, and then 4. All these arrays
** will be reallocated to exact sizes or erased when parsing ends.
*/
#घोषणा MINSIZEARRAY	4


व्योम *luaM_growaux_ (lua_State *L, व्योम *block, पूर्णांक nelems, पूर्णांक *psize,
                     अचिन्हित आकार_elems, पूर्णांक limit, स्थिर अक्षर *what) {
  व्योम *newblock;
  पूर्णांक आकार = *psize;
  अगर (nelems + 1 <= आकार)  /* does one extra element still fit? */
    वापस block;  /* nothing to be done */
  अगर (आकार >= limit / 2) {  /* cannot double it? */
    अगर (l_unlikely(आकार >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    आकार = limit;  /* still have at least one free place */
  }
  अन्यथा {
    आकार *= 2;
    अगर (आकार < MINSIZEARRAY)
      आकार = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= आकार && आकार <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * आकार_elems,
                                         cast_sizet(आकार) * आकार_elems);
  *psize = आकार;  /* update only when everything else is OK */
  वापस newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
व्योम *luaM_shrinkvector_ (lua_State *L, व्योम *block, पूर्णांक *आकार,
                          पूर्णांक final_n, अचिन्हित आकार_elem) {
  व्योम *newblock;
  माप_प्रकार oldsize = cast_sizet(*आकार) * आकार_elem;
  माप_प्रकार newsize = cast_sizet(final_n) * आकार_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *आकार = final_n;
  वापस newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
व्योम luaM_free_ (lua_State *L, व्योम *block, माप_प्रकार osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == शून्य));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
अटल व्योम *tryagain (lua_State *L, व्योम *block,
                       माप_प्रकार osize, माप_प्रकार nsize) {
  global_State *g = G(L);
  अगर (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    वापस callfrealloc(g, block, osize, nsize);  /* try again */
  }
  अन्यथा वापस शून्य;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
व्योम *luaM_realloc_ (lua_State *L, व्योम *block, माप_प्रकार osize, माप_प्रकार nsize) {
  व्योम *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == शून्य));
  newblock = firsttry(g, block, osize, nsize);
  अगर (l_unlikely(newblock == शून्य && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    अगर (newblock == शून्य)  /* still no memory? */
      वापस शून्य;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == शून्य));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  वापस newblock;
}


व्योम *luaM_saferealloc_ (lua_State *L, व्योम *block, माप_प्रकार osize,
                                                    माप_प्रकार nsize) {
  व्योम *newblock = luaM_realloc_(L, block, osize, nsize);
  अगर (l_unlikely(newblock == शून्य && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  वापस newblock;
}


व्योम *luaM_malloc_ (lua_State *L, माप_प्रकार आकार, पूर्णांक tag) {
  अगर (आकार == 0)
    वापस शून्य;  /* that's all */
  अन्यथा {
    global_State *g = G(L);
    व्योम *newblock = firsttry(g, शून्य, cast_sizet(tag), आकार);
    अगर (l_unlikely(newblock == शून्य)) {
      newblock = tryagain(L, शून्य, cast_sizet(tag), आकार);
      अगर (newblock == शून्य)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, आकार);
    वापस newblock;
  }
}
