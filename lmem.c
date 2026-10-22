/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#وصف_ڪريو lmem_c
#وصف_ڪريو LUA_CORE

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
#وصف_ڪريو callfrealloc(g,بلاڪ,os,ns)    ((*g->frealloc)(g->ud, بلاڪ, os, ns))


/*
** When an allocation fails, it will try again after an emergency
** collection, except when it cannot run a collection.  The GC should
** not be called while the state is not fully built, as the collector
** is not yet fully initialized. Also, it should not be called when
** 'gcstopem' is true, because then the interpreter is in the middle of
** a collection step.
*/
#وصف_ڪريو cantryagain(g)	(completestate(g) && !g->gcstopem)




#إذا defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ساكن فراغ *firsttry (global_State *g, فراغ *بلاڪ, size_t os, size_t ns) {
  إذا (ns > 0 && cantryagain(g))
    أرجع عدم;  /* fail */
  وإلا  /* normal allocation */
    أرجع callfrealloc(g, بلاڪ, os, ns);
}
#وإلا
#وصف_ڪريو firsttry(g,بلاڪ,os,ns)    callfrealloc(g, بلاڪ, os, ns)
#جيڪڏهن_پورو





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
#وصف_ڪريو MINSIZEARRAY	4


فراغ *luaM_growaux_ (lua_State *L, فراغ *بلاڪ, صحيح nelems, صحيح *psize,
                     بنانشان سائيز_elems, صحيح حد_بندي, ثابت حرف *what) {
  فراغ *newblock;
  صحيح سائيز = *psize;
  إذا (nelems + 1 <= سائيز)  /* does one extra element still fit? */
    أرجع بلاڪ;  /* nothing to be done */
  إذا (سائيز >= حد_بندي / 2) {  /* cannot double it? */
    إذا (l_unlikely(سائيز >= حد_بندي))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, حد_بندي);
    سائيز = حد_بندي;  /* still have at least one free place */
  }
  وإلا {
    سائيز *= 2;
    إذا (سائيز < MINSIZEARRAY)
      سائيز = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= سائيز && سائيز <= حد_بندي);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, بلاڪ, cast_sizet(*psize) * سائيز_elems,
                                         cast_sizet(سائيز) * سائيز_elems);
  *psize = سائيز;  /* update only when everything else is OK */
  أرجع newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
فراغ *luaM_shrinkvector_ (lua_State *L, فراغ *بلاڪ, صحيح *سائيز,
                          صحيح final_n, بنانشان سائيز_elem) {
  فراغ *newblock;
  size_t oldsize = cast_sizet(*سائيز) * سائيز_elem;
  size_t newsize = cast_sizet(final_n) * سائيز_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, بلاڪ, oldsize, newsize);
  *سائيز = final_n;
  أرجع newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
فراغ luaM_free_ (lua_State *L, فراغ *بلاڪ, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (بلاڪ == عدم));
  callfrealloc(g, بلاڪ, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ساكن فراغ *tryagain (lua_State *L, فراغ *بلاڪ,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  إذا (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    أرجع callfrealloc(g, بلاڪ, osize, nsize);  /* try again */
  }
  وإلا أرجع عدم;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
فراغ *luaM_realloc_ (lua_State *L, فراغ *بلاڪ, size_t osize, size_t nsize) {
  فراغ *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (بلاڪ == عدم));
  newblock = firsttry(g, بلاڪ, osize, nsize);
  إذا (l_unlikely(newblock == عدم && nsize > 0)) {
    newblock = tryagain(L, بلاڪ, osize, nsize);
    إذا (newblock == عدم)  /* still no memory? */
      أرجع عدم;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == عدم));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  أرجع newblock;
}


فراغ *luaM_saferealloc_ (lua_State *L, فراغ *بلاڪ, size_t osize,
                                                    size_t nsize) {
  فراغ *newblock = luaM_realloc_(L, بلاڪ, osize, nsize);
  إذا (l_unlikely(newblock == عدم && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  أرجع newblock;
}


فراغ *luaM_malloc_ (lua_State *L, size_t سائيز, صحيح ٽيگ) {
  إذا (سائيز == 0)
    أرجع عدم;  /* that's all */
  وإلا {
    global_State *g = G(L);
    فراغ *newblock = firsttry(g, عدم, cast_sizet(ٽيگ), سائيز);
    إذا (l_unlikely(newblock == عدم)) {
      newblock = tryagain(L, عدم, cast_sizet(ٽيگ), سائيز);
      إذا (newblock == عدم)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, سائيز);
    أرجع newblock;
  }
}
