/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stddef.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"



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
#define callfrealloc(g,block,os,ns)    ((*g->frealloc)(g->ud, block, os, ns))


/*
** When an allocation fails, it will try again after an emergency
** collection, except when it cannot run a collection.  The GC should
** not be called while the state is not fully built, as the collector
** is not yet fully initialized. Also, it should not be called when
** 'gcstopem' is true, because then the interpreter is in the middle of
** a collection step.
*/
#define cantryagain(g)	(completestate(g) && !g->gcstopem)




#यदि defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
स्थिर शून्य *firsttry (global_State *g, शून्य *block, size_t os, size_t ns) {
  यदि (ns > 0 && cantryagain(g))
    प्रत्यागम रिक्त;  /* fail */
  अन्यथा  /* normal allocation */
    प्रत्यागम callfrealloc(g, block, os, ns);
}
#अन्यथा
#define firsttry(g,block,os,ns)    callfrealloc(g, block, os, ns)
#endif





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
#define MINSIZEARRAY	4


शून्य *luaM_growaux_ (lua_State *L, शून्य *block, पूर्णाङ्क nelems, पूर्णाङ्क *psize,
                     unsigned size_elems, पूर्णाङ्क limit, ध्रुव वर्ण *what) {
  शून्य *newblock;
  पूर्णाङ्क size = *psize;
  यदि (nelems + 1 <= size)  /* does one extra element still fit? */
    प्रत्यागम block;  /* nothing to be done */
  यदि (size >= limit / 2) {  /* cannot double it? */
    यदि (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  अन्यथा {
    size *= 2;
    यदि (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  प्रत्यागम newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
शून्य *luaM_shrinkvector_ (lua_State *L, शून्य *block, पूर्णाङ्क *size,
                          पूर्णाङ्क final_n, unsigned size_elem) {
  शून्य *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  प्रत्यागम newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
शून्य luaM_free_ (lua_State *L, शून्य *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == रिक्त));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
स्थिर शून्य *tryagain (lua_State *L, शून्य *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  यदि (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    प्रत्यागम callfrealloc(g, block, osize, nsize);  /* try again */
  }
  अन्यथा प्रत्यागम रिक्त;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
शून्य *luaM_realloc_ (lua_State *L, शून्य *block, size_t osize, size_t nsize) {
  शून्य *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == रिक्त));
  newblock = firsttry(g, block, osize, nsize);
  यदि (l_unlikely(newblock == रिक्त && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    यदि (newblock == रिक्त)  /* still no memory? */
      प्रत्यागम रिक्त;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == रिक्त));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  प्रत्यागम newblock;
}


शून्य *luaM_saferealloc_ (lua_State *L, शून्य *block, size_t osize,
                                                    size_t nsize) {
  शून्य *newblock = luaM_realloc_(L, block, osize, nsize);
  यदि (l_unlikely(newblock == रिक्त && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  प्रत्यागम newblock;
}


शून्य *luaM_malloc_ (lua_State *L, size_t size, पूर्णाङ्क tag) {
  यदि (size == 0)
    प्रत्यागम रिक्त;  /* that's all */
  अन्यथा {
    global_State *g = G(L);
    शून्य *newblock = firsttry(g, रिक्त, cast_sizet(tag), size);
    यदि (l_unlikely(newblock == रिक्त)) {
      newblock = tryagain(L, रिक्त, cast_sizet(tag), size);
      यदि (newblock == रिक्त)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    प्रत्यागम newblock;
  }
}
