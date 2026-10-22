/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stddef.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"



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




#ଯଦି defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ସ୍ଥିର ଶୂନ୍ୟ *firsttry (global_State *g, ଶୂନ୍ୟ *block, size_t os, size_t ns) {
  ଯଦି (ns > 0 && cantryagain(g))
    ଫେରନ୍ତୁ ଖାଲି;  /* fail */
  ଅନ୍ୟଥା  /* normal allocation */
    ଫେରନ୍ତୁ callfrealloc(g, block, os, ns);
}
#ଅନ୍ୟଥା
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


ଶୂନ୍ୟ *luaM_growaux_ (lua_State *L, ଶୂନ୍ୟ *block, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nelems, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *psize,
                     unsigned size_elems, ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit, ଅଚଳ ଅକ୍ଷର *what) {
  ଶୂନ୍ୟ *newblock;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = *psize;
  ଯଦି (nelems + 1 <= size)  /* does one extra element still fit? */
    ଫେରନ୍ତୁ block;  /* nothing to be done */
  ଯଦି (size >= limit / 2) {  /* cannot double it? */
    ଯଦି (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  ଅନ୍ୟଥା {
    size *= 2;
    ଯଦି (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  ଫେରନ୍ତୁ newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
ଶୂନ୍ୟ *luaM_shrinkvector_ (lua_State *L, ଶୂନ୍ୟ *block, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *size,
                          ପୂର୍ଣ୍ଣସଂଖ୍ୟା final_n, unsigned size_elem) {
  ଶୂନ୍ୟ *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  ଫେରନ୍ତୁ newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
ଶୂନ୍ୟ luaM_free_ (lua_State *L, ଶୂନ୍ୟ *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ଖାଲି));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ସ୍ଥିର ଶୂନ୍ୟ *tryagain (lua_State *L, ଶୂନ୍ୟ *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  ଯଦି (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    ଫେରନ୍ତୁ callfrealloc(g, block, osize, nsize);  /* try again */
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ ଖାଲି;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
ଶୂନ୍ୟ *luaM_realloc_ (lua_State *L, ଶୂନ୍ୟ *block, size_t osize, size_t nsize) {
  ଶୂନ୍ୟ *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ଖାଲି));
  newblock = firsttry(g, block, osize, nsize);
  ଯଦି (l_unlikely(newblock == ଖାଲି && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    ଯଦି (newblock == ଖାଲି)  /* still no memory? */
      ଫେରନ୍ତୁ ଖାଲି;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == ଖାଲି));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  ଫେରନ୍ତୁ newblock;
}


ଶୂନ୍ୟ *luaM_saferealloc_ (lua_State *L, ଶୂନ୍ୟ *block, size_t osize,
                                                    size_t nsize) {
  ଶୂନ୍ୟ *newblock = luaM_realloc_(L, block, osize, nsize);
  ଯଦି (l_unlikely(newblock == ଖାଲି && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  ଫେରନ୍ତୁ newblock;
}


ଶୂନ୍ୟ *luaM_malloc_ (lua_State *L, size_t size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag) {
  ଯଦି (size == 0)
    ଫେରନ୍ତୁ ଖାଲି;  /* that's all */
  ଅନ୍ୟଥା {
    global_State *g = G(L);
    ଶୂନ୍ୟ *newblock = firsttry(g, ଖାଲି, cast_sizet(tag), size);
    ଯଦି (l_unlikely(newblock == ଖାଲି)) {
      newblock = tryagain(L, ଖାଲି, cast_sizet(tag), size);
      ଯଦି (newblock == ଖାଲି)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    ଫେରନ୍ତୁ newblock;
  }
}
