/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stddef.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"



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




#எனில் defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
நிலையான வெறுமை *firsttry (global_State *g, வெறுமை *block, size_t os, size_t ns) {
  எனில் (ns > 0 && cantryagain(g))
    திரும்பு வெற்று;  /* fail */
  இல்லையெனில்  /* normal allocation */
    திரும்பு callfrealloc(g, block, os, ns);
}
#இல்லையெனில்
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


வெறுமை *luaM_growaux_ (lua_State *L, வெறுமை *block, முழு nelems, முழு *psize,
                     unsigned size_elems, முழு limit, மாறிலி எழுத்து *what) {
  வெறுமை *newblock;
  முழு size = *psize;
  எனில் (nelems + 1 <= size)  /* does one extra element still fit? */
    திரும்பு block;  /* nothing to be done */
  எனில் (size >= limit / 2) {  /* cannot double it? */
    எனில் (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  இல்லையெனில் {
    size *= 2;
    எனில் (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  திரும்பு newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
வெறுமை *luaM_shrinkvector_ (lua_State *L, வெறுமை *block, முழு *size,
                          முழு final_n, unsigned size_elem) {
  வெறுமை *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  திரும்பு newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
வெறுமை luaM_free_ (lua_State *L, வெறுமை *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == வெற்று));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
நிலையான வெறுமை *tryagain (lua_State *L, வெறுமை *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  எனில் (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    திரும்பு callfrealloc(g, block, osize, nsize);  /* try again */
  }
  இல்லையெனில் திரும்பு வெற்று;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
வெறுமை *luaM_realloc_ (lua_State *L, வெறுமை *block, size_t osize, size_t nsize) {
  வெறுமை *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == வெற்று));
  newblock = firsttry(g, block, osize, nsize);
  எனில் (l_unlikely(newblock == வெற்று && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    எனில் (newblock == வெற்று)  /* still no memory? */
      திரும்பு வெற்று;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == வெற்று));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  திரும்பு newblock;
}


வெறுமை *luaM_saferealloc_ (lua_State *L, வெறுமை *block, size_t osize,
                                                    size_t nsize) {
  வெறுமை *newblock = luaM_realloc_(L, block, osize, nsize);
  எனில் (l_unlikely(newblock == வெற்று && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  திரும்பு newblock;
}


வெறுமை *luaM_malloc_ (lua_State *L, size_t size, முழு tag) {
  எனில் (size == 0)
    திரும்பு வெற்று;  /* that's all */
  இல்லையெனில் {
    global_State *g = G(L);
    வெறுமை *newblock = firsttry(g, வெற்று, cast_sizet(tag), size);
    எனில் (l_unlikely(newblock == வெற்று)) {
      newblock = tryagain(L, வெற்று, cast_sizet(tag), size);
      எனில் (newblock == வெற்று)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    திரும்பு newblock;
  }
}
