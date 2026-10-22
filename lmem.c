/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <stddef.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"



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




#اگر defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ایستا تهی *firsttry (global_State *g, تهی *block, size_t os, size_t ns) {
  اگر (ns > 0 && cantryagain(g))
    بازگشت پوچ;  /* fail */
  وگرنه  /* normal allocation */
    بازگشت callfrealloc(g, block, os, ns);
}
#وگرنه
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


تهی *luaM_growaux_ (lua_State *L, تهی *block, صحیح nelems, صحیح *psize,
                     unsigned size_elems, صحیح limit, ثابت نویسه *what) {
  تهی *newblock;
  صحیح size = *psize;
  اگر (nelems + 1 <= size)  /* does one extra element still fit? */
    بازگشت block;  /* nothing to be done */
  اگر (size >= limit / 2) {  /* cannot double it? */
    اگر (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  وگرنه {
    size *= 2;
    اگر (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  بازگشت newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
تهی *luaM_shrinkvector_ (lua_State *L, تهی *block, صحیح *size,
                          صحیح final_n, unsigned size_elem) {
  تهی *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  بازگشت newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
تهی luaM_free_ (lua_State *L, تهی *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == پوچ));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ایستا تهی *tryagain (lua_State *L, تهی *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  اگر (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    بازگشت callfrealloc(g, block, osize, nsize);  /* try again */
  }
  وگرنه بازگشت پوچ;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
تهی *luaM_realloc_ (lua_State *L, تهی *block, size_t osize, size_t nsize) {
  تهی *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == پوچ));
  newblock = firsttry(g, block, osize, nsize);
  اگر (l_unlikely(newblock == پوچ && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    اگر (newblock == پوچ)  /* still no memory? */
      بازگشت پوچ;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == پوچ));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  بازگشت newblock;
}


تهی *luaM_saferealloc_ (lua_State *L, تهی *block, size_t osize,
                                                    size_t nsize) {
  تهی *newblock = luaM_realloc_(L, block, osize, nsize);
  اگر (l_unlikely(newblock == پوچ && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  بازگشت newblock;
}


تهی *luaM_malloc_ (lua_State *L, size_t size, صحیح tag) {
  اگر (size == 0)
    بازگشت پوچ;  /* that's all */
  وگرنه {
    global_State *g = G(L);
    تهی *newblock = firsttry(g, پوچ, cast_sizet(tag), size);
    اگر (l_unlikely(newblock == پوچ)) {
      newblock = tryagain(L, پوچ, cast_sizet(tag), size);
      اگر (newblock == پوچ)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    بازگشت newblock;
  }
}
