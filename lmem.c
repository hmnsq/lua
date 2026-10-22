/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <stddef.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"



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




#જો defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
સ્થિર શૂન્ય *firsttry (global_State *g, શૂન્ય *block, size_t os, size_t ns) {
  જો (ns > 0 && cantryagain(g))
    પાછા ખાલી;  /* fail */
  નહીંતર  /* normal allocation */
    પાછા callfrealloc(g, block, os, ns);
}
#નહીંતર
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


શૂન્ય *luaM_growaux_ (lua_State *L, શૂન્ય *block, પૂર્ણાંક nelems, પૂર્ણાંક *psize,
                     unsigned size_elems, પૂર્ણાંક limit, અચલ અક્ષર *what) {
  શૂન્ય *newblock;
  પૂર્ણાંક size = *psize;
  જો (nelems + 1 <= size)  /* does one extra element still fit? */
    પાછા block;  /* nothing to be done */
  જો (size >= limit / 2) {  /* cannot double it? */
    જો (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  નહીંતર {
    size *= 2;
    જો (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  પાછા newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
શૂન્ય *luaM_shrinkvector_ (lua_State *L, શૂન્ય *block, પૂર્ણાંક *size,
                          પૂર્ણાંક final_n, unsigned size_elem) {
  શૂન્ય *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  પાછા newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
શૂન્ય luaM_free_ (lua_State *L, શૂન્ય *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ખાલી));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
સ્થિર શૂન્ય *tryagain (lua_State *L, શૂન્ય *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  જો (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    પાછા callfrealloc(g, block, osize, nsize);  /* try again */
  }
  નહીંતર પાછા ખાલી;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
શૂન્ય *luaM_realloc_ (lua_State *L, શૂન્ય *block, size_t osize, size_t nsize) {
  શૂન્ય *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ખાલી));
  newblock = firsttry(g, block, osize, nsize);
  જો (l_unlikely(newblock == ખાલી && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    જો (newblock == ખાલી)  /* still no memory? */
      પાછા ખાલી;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == ખાલી));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  પાછા newblock;
}


શૂન્ય *luaM_saferealloc_ (lua_State *L, શૂન્ય *block, size_t osize,
                                                    size_t nsize) {
  શૂન્ય *newblock = luaM_realloc_(L, block, osize, nsize);
  જો (l_unlikely(newblock == ખાલી && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  પાછા newblock;
}


શૂન્ય *luaM_malloc_ (lua_State *L, size_t size, પૂર્ણાંક tag) {
  જો (size == 0)
    પાછા ખાલી;  /* that's all */
  નહીંતર {
    global_State *g = G(L);
    શૂન્ય *newblock = firsttry(g, ખાલી, cast_sizet(tag), size);
    જો (l_unlikely(newblock == ખાલી)) {
      newblock = tryagain(L, ખાલી, cast_sizet(tag), size);
      જો (newblock == ખાલી)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    પાછા newblock;
  }
}
