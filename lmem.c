/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <stddef.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "ldebug.h"
#অন্তর্ভাৱ "ldo.h"
#অন্তর্ভাৱ "lgc.h"
#অন্তর্ভাৱ "lmem.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lstate.h"



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




#যদি defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
স্থির শূন্য *firsttry (global_State *g, শূন্য *block, size_t os, size_t ns) {
  যদি (ns > 0 && cantryagain(g))
    ঘূৰাই রিক্ত;  /* fail */
  অন্যথা  /* normal allocation */
    ঘূৰাই callfrealloc(g, block, os, ns);
}
#অন্যথা
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


শূন্য *luaM_growaux_ (lua_State *L, শূন্য *block, পূর্ণাঙ্ক nelems, পূর্ণাঙ্ক *psize,
                     unsigned size_elems, পূর্ণাঙ্ক limit, ধ্রুৱ ৱর্ণ *what) {
  শূন্য *newblock;
  পূর্ণাঙ্ক size = *psize;
  যদি (nelems + 1 <= size)  /* does one extra element still fit? */
    ঘূৰাই block;  /* nothing to be done */
  যদি (size >= limit / 2) {  /* cannot double it? */
    যদি (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  অন্যথা {
    size *= 2;
    যদি (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  ঘূৰাই newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
শূন্য *luaM_shrinkvector_ (lua_State *L, শূন্য *block, পূর্ণাঙ্ক *size,
                          পূর্ণাঙ্ক final_n, unsigned size_elem) {
  শূন্য *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  ঘূৰাই newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
শূন্য luaM_free_ (lua_State *L, শূন্য *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == রিক্ত));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
স্থির শূন্য *tryagain (lua_State *L, শূন্য *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  যদি (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    ঘূৰাই callfrealloc(g, block, osize, nsize);  /* try again */
  }
  অন্যথা ঘূৰাই রিক্ত;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
শূন্য *luaM_realloc_ (lua_State *L, শূন্য *block, size_t osize, size_t nsize) {
  শূন্য *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == রিক্ত));
  newblock = firsttry(g, block, osize, nsize);
  যদি (l_unlikely(newblock == রিক্ত && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    যদি (newblock == রিক্ত)  /* still no memory? */
      ঘূৰাই রিক্ত;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == রিক্ত));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  ঘূৰাই newblock;
}


শূন্য *luaM_saferealloc_ (lua_State *L, শূন্য *block, size_t osize,
                                                    size_t nsize) {
  শূন্য *newblock = luaM_realloc_(L, block, osize, nsize);
  যদি (l_unlikely(newblock == রিক্ত && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  ঘূৰাই newblock;
}


শূন্য *luaM_malloc_ (lua_State *L, size_t size, পূর্ণাঙ্ক tag) {
  যদি (size == 0)
    ঘূৰাই রিক্ত;  /* that's all */
  অন্যথা {
    global_State *g = G(L);
    শূন্য *newblock = firsttry(g, রিক্ত, cast_sizet(tag), size);
    যদি (l_unlikely(newblock == রিক্ত)) {
      newblock = tryagain(L, রিক্ত, cast_sizet(tag), size);
      যদি (newblock == রিক্ত)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    ঘূৰাই newblock;
  }
}
