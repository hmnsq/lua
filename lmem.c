/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stddef.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"



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




#ಯದಿ defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *firsttry (global_State *g, ಶೂನ್ಯ *block, size_t os, size_t ns) {
  ಯದಿ (ns > 0 && cantryagain(g))
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* fail */
  ಅನ್ಯಥಾ  /* normal allocation */
    ಪ್ರತ್ಯಾಗಮ callfrealloc(g, block, os, ns);
}
#ಅನ್ಯಥಾ
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


ಶೂನ್ಯ *luaM_growaux_ (lua_State *L, ಶೂನ್ಯ *block, ಪೂರ್ಣಾಙ್ಕ nelems, ಪೂರ್ಣಾಙ್ಕ *psize,
                     unsigned size_elems, ಪೂರ್ಣಾಙ್ಕ limit, ಧ್ರುವ ವರ್ಣ *what) {
  ಶೂನ್ಯ *newblock;
  ಪೂರ್ಣಾಙ್ಕ size = *psize;
  ಯದಿ (nelems + 1 <= size)  /* does one extra element still fit? */
    ಪ್ರತ್ಯಾಗಮ block;  /* nothing to be done */
  ಯದಿ (size >= limit / 2) {  /* cannot double it? */
    ಯದಿ (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  ಅನ್ಯಥಾ {
    size *= 2;
    ಯದಿ (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  ಪ್ರತ್ಯಾಗಮ newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
ಶೂನ್ಯ *luaM_shrinkvector_ (lua_State *L, ಶೂನ್ಯ *block, ಪೂರ್ಣಾಙ್ಕ *size,
                          ಪೂರ್ಣಾಙ್ಕ final_n, unsigned size_elem) {
  ಶೂನ್ಯ *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  ಪ್ರತ್ಯಾಗಮ newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
ಶೂನ್ಯ luaM_free_ (lua_State *L, ಶೂನ್ಯ *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ರಿಕ್ತ));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *tryagain (lua_State *L, ಶೂನ್ಯ *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  ಯದಿ (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    ಪ್ರತ್ಯಾಗಮ callfrealloc(g, block, osize, nsize);  /* try again */
  }
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
ಶೂನ್ಯ *luaM_realloc_ (lua_State *L, ಶೂನ್ಯ *block, size_t osize, size_t nsize) {
  ಶೂನ್ಯ *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ರಿಕ್ತ));
  newblock = firsttry(g, block, osize, nsize);
  ಯದಿ (l_unlikely(newblock == ರಿಕ್ತ && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    ಯದಿ (newblock == ರಿಕ್ತ)  /* still no memory? */
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == ರಿಕ್ತ));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  ಪ್ರತ್ಯಾಗಮ newblock;
}


ಶೂನ್ಯ *luaM_saferealloc_ (lua_State *L, ಶೂನ್ಯ *block, size_t osize,
                                                    size_t nsize) {
  ಶೂನ್ಯ *newblock = luaM_realloc_(L, block, osize, nsize);
  ಯದಿ (l_unlikely(newblock == ರಿಕ್ತ && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  ಪ್ರತ್ಯಾಗಮ newblock;
}


ಶೂನ್ಯ *luaM_malloc_ (lua_State *L, size_t size, ಪೂರ್ಣಾಙ್ಕ tag) {
  ಯದಿ (size == 0)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* that's all */
  ಅನ್ಯಥಾ {
    global_State *g = G(L);
    ಶೂನ್ಯ *newblock = firsttry(g, ರಿಕ್ತ, cast_sizet(tag), size);
    ಯದಿ (l_unlikely(newblock == ರಿಕ್ತ)) {
      newblock = tryagain(L, ರಿಕ್ತ, cast_sizet(tag), size);
      ಯದಿ (newblock == ರಿಕ್ತ)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    ಪ್ರತ್ಯಾಗಮ newblock;
  }
}
