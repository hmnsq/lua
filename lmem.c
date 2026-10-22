/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lmem_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"



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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ callfrealloc(g,ਖੰਡ,os,ns)    ((*g->frealloc)(g->ud, ਖੰਡ, os, ns))


/*
** When an allocation fails, it will try again after an emergency
** collection, except when it cannot run a collection.  The GC should
** not be called while the state is not fully built, as the collector
** is not yet fully initialized. Also, it should not be called when
** 'gcstopem' is true, because then the interpreter is in the middle of
** a collection step.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cantryagain(g)	(completestate(g) && !g->gcstopem)




#ਜੇ defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ਸਥਿਰ ਖਾਲੀ *firsttry (global_State *g, ਖਾਲੀ *ਖੰਡ, size_t os, size_t ns) {
  ਜੇ (ns > 0 && cantryagain(g))
    ਵਾਪਸ ਨਿਰਾ;  /* fail */
  ਨਹੀਂਤਾਂ  /* normal allocation */
    ਵਾਪਸ callfrealloc(g, ਖੰਡ, os, ns);
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ firsttry(g,ਖੰਡ,os,ns)    callfrealloc(g, ਖੰਡ, os, ns)
#ਜੇ_ਅੰਤ





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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MINSIZEARRAY	4


ਖਾਲੀ *luaM_growaux_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, ਪੂਰਨ nelems, ਪੂਰਨ *psize,
                     ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_elems, ਪੂਰਨ ਹੱਦਬੰਦੀ, ਅਚਲ ਅੱਖਰ *what) {
  ਖਾਲੀ *newblock;
  ਪੂਰਨ ਮਾਪ = *psize;
  ਜੇ (nelems + 1 <= ਮਾਪ)  /* does one extra element still fit? */
    ਵਾਪਸ ਖੰਡ;  /* nothing to be done */
  ਜੇ (ਮਾਪ >= ਹੱਦਬੰਦੀ / 2) {  /* cannot double it? */
    ਜੇ (l_unlikely(ਮਾਪ >= ਹੱਦਬੰਦੀ))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, ਹੱਦਬੰਦੀ);
    ਮਾਪ = ਹੱਦਬੰਦੀ;  /* still have at least one free place */
  }
  ਨਹੀਂਤਾਂ {
    ਮਾਪ *= 2;
    ਜੇ (ਮਾਪ < MINSIZEARRAY)
      ਮਾਪ = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= ਮਾਪ && ਮਾਪ <= ਹੱਦਬੰਦੀ);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, ਖੰਡ, cast_sizet(*psize) * ਮਾਪ_elems,
                                         cast_sizet(ਮਾਪ) * ਮਾਪ_elems);
  *psize = ਮਾਪ;  /* update only when everything else is OK */
  ਵਾਪਸ newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
ਖਾਲੀ *luaM_shrinkvector_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, ਪੂਰਨ *ਮਾਪ,
                          ਪੂਰਨ final_n, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_elem) {
  ਖਾਲੀ *newblock;
  size_t oldsize = cast_sizet(*ਮਾਪ) * ਮਾਪ_elem;
  size_t newsize = cast_sizet(final_n) * ਮਾਪ_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, ਖੰਡ, oldsize, newsize);
  *ਮਾਪ = final_n;
  ਵਾਪਸ newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
ਖਾਲੀ luaM_free_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (ਖੰਡ == ਨਿਰਾ));
  callfrealloc(g, ਖੰਡ, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ਸਥਿਰ ਖਾਲੀ *tryagain (lua_State *L, ਖਾਲੀ *ਖੰਡ,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  ਜੇ (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    ਵਾਪਸ callfrealloc(g, ਖੰਡ, osize, nsize);  /* try again */
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ ਨਿਰਾ;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
ਖਾਲੀ *luaM_realloc_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, size_t osize, size_t nsize) {
  ਖਾਲੀ *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (ਖੰਡ == ਨਿਰਾ));
  newblock = firsttry(g, ਖੰਡ, osize, nsize);
  ਜੇ (l_unlikely(newblock == ਨਿਰਾ && nsize > 0)) {
    newblock = tryagain(L, ਖੰਡ, osize, nsize);
    ਜੇ (newblock == ਨਿਰਾ)  /* still no memory? */
      ਵਾਪਸ ਨਿਰਾ;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == ਨਿਰਾ));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  ਵਾਪਸ newblock;
}


ਖਾਲੀ *luaM_saferealloc_ (lua_State *L, ਖਾਲੀ *ਖੰਡ, size_t osize,
                                                    size_t nsize) {
  ਖਾਲੀ *newblock = luaM_realloc_(L, ਖੰਡ, osize, nsize);
  ਜੇ (l_unlikely(newblock == ਨਿਰਾ && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  ਵਾਪਸ newblock;
}


ਖਾਲੀ *luaM_malloc_ (lua_State *L, size_t ਮਾਪ, ਪੂਰਨ ਟੈਗ) {
  ਜੇ (ਮਾਪ == 0)
    ਵਾਪਸ ਨਿਰਾ;  /* that's all */
  ਨਹੀਂਤਾਂ {
    global_State *g = G(L);
    ਖਾਲੀ *newblock = firsttry(g, ਨਿਰਾ, cast_sizet(ਟੈਗ), ਮਾਪ);
    ਜੇ (l_unlikely(newblock == ਨਿਰਾ)) {
      newblock = tryagain(L, ਨਿਰਾ, cast_sizet(ਟੈਗ), ਮਾਪ);
      ਜੇ (newblock == ਨਿਰਾ)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, ਮਾਪ);
    ਵਾਪਸ newblock;
  }
}
