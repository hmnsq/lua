/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#నిర్వచించు lmem_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stddef.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"



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
#నిర్వచించు callfrealloc(g,ఖండం,os,ns)    ((*g->frealloc)(g->ud, ఖండం, os, ns))


/*
** When an allocation fails, it will try again after an emergency
** collection, except when it cannot run a collection.  The GC should
** not be called while the state is not fully built, as the collector
** is not yet fully initialized. Also, it should not be called when
** 'gcstopem' is true, because then the interpreter is in the middle of
** a collection step.
*/
#నిర్వచించు cantryagain(g)	(completestate(g) && !g->gcstopem)




#ఒకవేళ defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
స్థిర శూన్య *firsttry (global_State *g, శూన్య *ఖండం, size_t os, size_t ns) {
  ఒకవేళ (ns > 0 && cantryagain(g))
    తిరిగి ఖాళీ;  /* fail */
  లేకపోతే  /* normal allocation */
    తిరిగి callfrealloc(g, ఖండం, os, ns);
}
#లేకపోతే
#నిర్వచించు firsttry(g,ఖండం,os,ns)    callfrealloc(g, ఖండం, os, ns)
#అయితే_ముగింపు





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
#నిర్వచించు MINSIZEARRAY	4


శూన్య *luaM_growaux_ (lua_State *L, శూన్య *ఖండం, పూర్ణాంకం nelems, పూర్ణాంకం *psize,
                     గుర్తులేని కొలత_elems, పూర్ణాంకం హద్దు, స్థిరరాశి అక్షరం *what) {
  శూన్య *newblock;
  పూర్ణాంకం కొలత = *psize;
  ఒకవేళ (nelems + 1 <= కొలత)  /* does one extra element still fit? */
    తిరిగి ఖండం;  /* nothing to be done */
  ఒకవేళ (కొలత >= హద్దు / 2) {  /* cannot double it? */
    ఒకవేళ (l_unlikely(కొలత >= హద్దు))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, హద్దు);
    కొలత = హద్దు;  /* still have at least one free place */
  }
  లేకపోతే {
    కొలత *= 2;
    ఒకవేళ (కొలత < MINSIZEARRAY)
      కొలత = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= కొలత && కొలత <= హద్దు);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, ఖండం, cast_sizet(*psize) * కొలత_elems,
                                         cast_sizet(కొలత) * కొలత_elems);
  *psize = కొలత;  /* update only when everything else is OK */
  తిరిగి newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
శూన్య *luaM_shrinkvector_ (lua_State *L, శూన్య *ఖండం, పూర్ణాంకం *కొలత,
                          పూర్ణాంకం final_n, గుర్తులేని కొలత_elem) {
  శూన్య *newblock;
  size_t oldsize = cast_sizet(*కొలత) * కొలత_elem;
  size_t newsize = cast_sizet(final_n) * కొలత_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, ఖండం, oldsize, newsize);
  *కొలత = final_n;
  తిరిగి newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
శూన్య luaM_free_ (lua_State *L, శూన్య *ఖండం, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (ఖండం == ఖాళీ));
  callfrealloc(g, ఖండం, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
స్థిర శూన్య *tryagain (lua_State *L, శూన్య *ఖండం,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  ఒకవేళ (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    తిరిగి callfrealloc(g, ఖండం, osize, nsize);  /* try again */
  }
  లేకపోతే తిరిగి ఖాళీ;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
శూన్య *luaM_realloc_ (lua_State *L, శూన్య *ఖండం, size_t osize, size_t nsize) {
  శూన్య *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (ఖండం == ఖాళీ));
  newblock = firsttry(g, ఖండం, osize, nsize);
  ఒకవేళ (l_unlikely(newblock == ఖాళీ && nsize > 0)) {
    newblock = tryagain(L, ఖండం, osize, nsize);
    ఒకవేళ (newblock == ఖాళీ)  /* still no memory? */
      తిరిగి ఖాళీ;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == ఖాళీ));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  తిరిగి newblock;
}


శూన్య *luaM_saferealloc_ (lua_State *L, శూన్య *ఖండం, size_t osize,
                                                    size_t nsize) {
  శూన్య *newblock = luaM_realloc_(L, ఖండం, osize, nsize);
  ఒకవేళ (l_unlikely(newblock == ఖాళీ && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  తిరిగి newblock;
}


శూన్య *luaM_malloc_ (lua_State *L, size_t కొలత, పూర్ణాంకం ట్యాగ్) {
  ఒకవేళ (కొలత == 0)
    తిరిగి ఖాళీ;  /* that's all */
  లేకపోతే {
    global_State *g = G(L);
    శూన్య *newblock = firsttry(g, ఖాళీ, cast_sizet(ట్యాగ్), కొలత);
    ఒకవేళ (l_unlikely(newblock == ఖాళీ)) {
      newblock = tryagain(L, ఖాళీ, cast_sizet(ట్యాగ్), కొలత);
      ఒకవేళ (newblock == ఖాళీ)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, కొలత);
    తిరిగి newblock;
  }
}
