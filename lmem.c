/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

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




#𐤀𐤌 defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ساكن فراغ *firsttry (global_State *g, فراغ *block, size_t os, size_t ns) {
  𐤀𐤌 (ns > 0 && cantryagain(g))
    أرجع عدم;  /* fail */
  وإلا  /* normal allocation */
    أرجع callfrealloc(g, block, os, ns);
}
#وإلا
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


فراغ *luaM_growaux_ (lua_State *L, فراغ *block, صحيح nelems, صحيح *psize,
                     unsigned size_elems, صحيح limit, ثابت حرف *what) {
  فراغ *newblock;
  صحيح size = *psize;
  𐤀𐤌 (nelems + 1 <= size)  /* does one extra element still fit? */
    أرجع block;  /* nothing to be done */
  𐤀𐤌 (size >= limit / 2) {  /* cannot double it? */
    𐤀𐤌 (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  وإلا {
    size *= 2;
    𐤀𐤌 (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  أرجع newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
فراغ *luaM_shrinkvector_ (lua_State *L, فراغ *block, صحيح *size,
                          صحيح final_n, unsigned size_elem) {
  فراغ *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  أرجع newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
فراغ luaM_free_ (lua_State *L, فراغ *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == عدم));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ساكن فراغ *tryagain (lua_State *L, فراغ *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  𐤀𐤌 (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    أرجع callfrealloc(g, block, osize, nsize);  /* try again */
  }
  وإلا أرجع عدم;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
فراغ *luaM_realloc_ (lua_State *L, فراغ *block, size_t osize, size_t nsize) {
  فراغ *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == عدم));
  newblock = firsttry(g, block, osize, nsize);
  𐤀𐤌 (l_unlikely(newblock == عدم && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    𐤀𐤌 (newblock == عدم)  /* still no memory? */
      أرجع عدم;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == عدم));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  أرجع newblock;
}


فراغ *luaM_saferealloc_ (lua_State *L, فراغ *block, size_t osize,
                                                    size_t nsize) {
  فراغ *newblock = luaM_realloc_(L, block, osize, nsize);
  𐤀𐤌 (l_unlikely(newblock == عدم && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  أرجع newblock;
}


فراغ *luaM_malloc_ (lua_State *L, size_t size, صحيح tag) {
  𐤀𐤌 (size == 0)
    أرجع عدم;  /* that's all */
  وإلا {
    global_State *g = G(L);
    فراغ *newblock = firsttry(g, عدم, cast_sizet(tag), size);
    𐤀𐤌 (l_unlikely(newblock == عدم)) {
      newblock = tryagain(L, عدم, cast_sizet(tag), size);
      𐤀𐤌 (newblock == عدم)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    أرجع newblock;
  }
}
