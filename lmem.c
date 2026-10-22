/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lmem_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

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
#ವ್ಯಾಖ್ಯಾನಿಸು callfrealloc(g,ಖಂಡ,os,ns)    ((*g->frealloc)(g->ud, ಖಂಡ, os, ns))


/*
** When an allocation fails, it will try again after an emergency
** collection, except when it cannot run a collection.  The GC should
** not be called while the state is not fully built, as the collector
** is not yet fully initialized. Also, it should not be called when
** 'gcstopem' is true, because then the interpreter is in the middle of
** a collection step.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು cantryagain(g)	(completestate(g) && !g->gcstopem)




#ಆದರೆ defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *firsttry (global_State *g, ಶೂನ್ಯ *ಖಂಡ, size_t os, size_t ns) {
  ಆದರೆ (ns > 0 && cantryagain(g))
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* fail */
  ಇಲ್ಲದಿದ್ದರೆ  /* normal allocation */
    ಹಿಂತಿರುಗಿ callfrealloc(g, ಖಂಡ, os, ns);
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು firsttry(g,ಖಂಡ,os,ns)    callfrealloc(g, ಖಂಡ, os, ns)
#ಆದರೆ_ಅಂತ್ಯ





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
#ವ್ಯಾಖ್ಯಾನಿಸು MINSIZEARRAY	4


ಶೂನ್ಯ *luaM_growaux_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, ಪೂರ್ಣಾಂಕ nelems, ಪೂರ್ಣಾಂಕ *psize,
                     ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_elems, ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what) {
  ಶೂನ್ಯ *newblock;
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = *psize;
  ಆದರೆ (nelems + 1 <= ಗಾತ್ರ_2)  /* does one extra element still fit? */
    ಹಿಂತಿರುಗಿ ಖಂಡ;  /* nothing to be done */
  ಆದರೆ (ಗಾತ್ರ_2 >= ಎಲ್ಲೆ / 2) {  /* cannot double it? */
    ಆದರೆ (l_unlikely(ಗಾತ್ರ_2 >= ಎಲ್ಲೆ))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, ಎಲ್ಲೆ);
    ಗಾತ್ರ_2 = ಎಲ್ಲೆ;  /* still have at least one free place */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಗಾತ್ರ_2 *= 2;
    ಆದರೆ (ಗಾತ್ರ_2 < MINSIZEARRAY)
      ಗಾತ್ರ_2 = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= ಗಾತ್ರ_2 && ಗಾತ್ರ_2 <= ಎಲ್ಲೆ);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, ಖಂಡ, cast_sizet(*psize) * ಗಾತ್ರ_elems,
                                         cast_sizet(ಗಾತ್ರ_2) * ಗಾತ್ರ_elems);
  *psize = ಗಾತ್ರ_2;  /* update only when everything else is OK */
  ಹಿಂತಿರುಗಿ newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
ಶೂನ್ಯ *luaM_shrinkvector_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, ಪೂರ್ಣಾಂಕ *ಗಾತ್ರ_2,
                          ಪೂರ್ಣಾಂಕ final_n, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_elem) {
  ಶೂನ್ಯ *newblock;
  size_t oldsize = cast_sizet(*ಗಾತ್ರ_2) * ಗಾತ್ರ_elem;
  size_t newsize = cast_sizet(final_n) * ಗಾತ್ರ_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, ಖಂಡ, oldsize, newsize);
  *ಗಾತ್ರ_2 = final_n;
  ಹಿಂತಿರುಗಿ newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
ಶೂನ್ಯ luaM_free_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (ಖಂಡ == ಖಾಲಿ));
  callfrealloc(g, ಖಂಡ, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *tryagain (lua_State *L, ಶೂನ್ಯ *ಖಂಡ,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  ಆದರೆ (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    ಹಿಂತಿರುಗಿ callfrealloc(g, ಖಂಡ, osize, nsize);  /* try again */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
ಶೂನ್ಯ *luaM_realloc_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, size_t osize, size_t nsize) {
  ಶೂನ್ಯ *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (ಖಂಡ == ಖಾಲಿ));
  newblock = firsttry(g, ಖಂಡ, osize, nsize);
  ಆದರೆ (l_unlikely(newblock == ಖಾಲಿ && nsize > 0)) {
    newblock = tryagain(L, ಖಂಡ, osize, nsize);
    ಆದರೆ (newblock == ಖಾಲಿ)  /* still no memory? */
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == ಖಾಲಿ));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  ಹಿಂತಿರುಗಿ newblock;
}


ಶೂನ್ಯ *luaM_saferealloc_ (lua_State *L, ಶೂನ್ಯ *ಖಂಡ, size_t osize,
                                                    size_t nsize) {
  ಶೂನ್ಯ *newblock = luaM_realloc_(L, ಖಂಡ, osize, nsize);
  ಆದರೆ (l_unlikely(newblock == ಖಾಲಿ && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  ಹಿಂತಿರುಗಿ newblock;
}


ಶೂನ್ಯ *luaM_malloc_ (lua_State *L, size_t ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್) {
  ಆದರೆ (ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* that's all */
  ಇಲ್ಲದಿದ್ದರೆ {
    global_State *g = G(L);
    ಶೂನ್ಯ *newblock = firsttry(g, ಖಾಲಿ, cast_sizet(ಟ್ಯಾಗ್), ಗಾತ್ರ_2);
    ಆದರೆ (l_unlikely(newblock == ಖಾಲಿ)) {
      newblock = tryagain(L, ಖಾಲಿ, cast_sizet(ಟ್ಯಾಗ್), ಗಾತ್ರ_2);
      ಆದರೆ (newblock == ಖಾಲಿ)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ newblock;
  }
}
