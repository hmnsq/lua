/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#تعریف_کرو lmem_c
#تعریف_کرو LUA_CORE

#ضمن "lprefix.h"


#ضمن <stddef.h>

#ضمن "lua.h"

#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lgc.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lstate.h"



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
#تعریف_کرو callfrealloc(g,بلاک,os,ns)    ((*g->frealloc)(g->ud, بلاک, os, ns))


/*
** When an allocation fails, it will try again after an emergency
** collection, except when it cannot run a collection.  The GC should
** not be called while the state is not fully built, as the collector
** is not yet fully initialized. Also, it should not be called when
** 'gcstopem' is true, because then the interpreter is in the middle of
** a collection step.
*/
#تعریف_کرو cantryagain(g)	(completestate(g) && !g->gcstopem)




#إذا defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ساكن فراغ *firsttry (global_State *g, فراغ *بلاک, size_t os, size_t ns) {
  إذا (ns > 0 && cantryagain(g))
    أرجع عدم;  /* fail */
  وإلا  /* normal allocation */
    أرجع callfrealloc(g, بلاک, os, ns);
}
#وإلا
#تعریف_کرو firsttry(g,بلاک,os,ns)    callfrealloc(g, بلاک, os, ns)
#جے_انت





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
#تعریف_کرو MINSIZEARRAY	4


فراغ *luaM_growaux_ (lua_State *L, فراغ *بلاک, صحيح nelems, صحيح *psize,
                     بنانشانی ناپ_elems, صحيح حد_بندی, ثابت حرف *what) {
  فراغ *newblock;
  صحيح ناپ = *psize;
  إذا (nelems + 1 <= ناپ)  /* does one extra element still fit? */
    أرجع بلاک;  /* nothing to be done */
  إذا (ناپ >= حد_بندی / 2) {  /* cannot double it? */
    إذا (l_unlikely(ناپ >= حد_بندی))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, حد_بندی);
    ناپ = حد_بندی;  /* still have at least one free place */
  }
  وإلا {
    ناپ *= 2;
    إذا (ناپ < MINSIZEARRAY)
      ناپ = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= ناپ && ناپ <= حد_بندی);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, بلاک, cast_sizet(*psize) * ناپ_elems,
                                         cast_sizet(ناپ) * ناپ_elems);
  *psize = ناپ;  /* update only when everything else is OK */
  أرجع newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
فراغ *luaM_shrinkvector_ (lua_State *L, فراغ *بلاک, صحيح *ناپ,
                          صحيح final_n, بنانشانی ناپ_elem) {
  فراغ *newblock;
  size_t oldsize = cast_sizet(*ناپ) * ناپ_elem;
  size_t newsize = cast_sizet(final_n) * ناپ_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, بلاک, oldsize, newsize);
  *ناپ = final_n;
  أرجع newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
فراغ luaM_free_ (lua_State *L, فراغ *بلاک, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (بلاک == عدم));
  callfrealloc(g, بلاک, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ساكن فراغ *tryagain (lua_State *L, فراغ *بلاک,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  إذا (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    أرجع callfrealloc(g, بلاک, osize, nsize);  /* try again */
  }
  وإلا أرجع عدم;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
فراغ *luaM_realloc_ (lua_State *L, فراغ *بلاک, size_t osize, size_t nsize) {
  فراغ *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (بلاک == عدم));
  newblock = firsttry(g, بلاک, osize, nsize);
  إذا (l_unlikely(newblock == عدم && nsize > 0)) {
    newblock = tryagain(L, بلاک, osize, nsize);
    إذا (newblock == عدم)  /* still no memory? */
      أرجع عدم;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == عدم));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  أرجع newblock;
}


فراغ *luaM_saferealloc_ (lua_State *L, فراغ *بلاک, size_t osize,
                                                    size_t nsize) {
  فراغ *newblock = luaM_realloc_(L, بلاک, osize, nsize);
  إذا (l_unlikely(newblock == عدم && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  أرجع newblock;
}


فراغ *luaM_malloc_ (lua_State *L, size_t ناپ, صحيح ٹیگ) {
  إذا (ناپ == 0)
    أرجع عدم;  /* that's all */
  وإلا {
    global_State *g = G(L);
    فراغ *newblock = firsttry(g, عدم, cast_sizet(ٹیگ), ناپ);
    إذا (l_unlikely(newblock == عدم)) {
      newblock = tryagain(L, عدم, cast_sizet(ٹیگ), ناپ);
      إذا (newblock == عدم)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, ناپ);
    أرجع newblock;
  }
}
