/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stddef.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"



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




#എങ്കിൽ defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
സ്ഥിതം ശൂന്യം *firsttry (global_State *g, ശൂന്യം *block, size_t os, size_t ns) {
  എങ്കിൽ (ns > 0 && cantryagain(g))
    തിരികെ ശൂന്യ;  /* fail */
  അല്ലെങ്കിൽ  /* normal allocation */
    തിരികെ callfrealloc(g, block, os, ns);
}
#അല്ലെങ്കിൽ
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


ശൂന്യം *luaM_growaux_ (lua_State *L, ശൂന്യം *block, പൂർണ്ണസംഖ്യ nelems, പൂർണ്ണസംഖ്യ *psize,
                     unsigned size_elems, പൂർണ്ണസംഖ്യ limit, സ്ഥിരം അക്ഷരം *what) {
  ശൂന്യം *newblock;
  പൂർണ്ണസംഖ്യ size = *psize;
  എങ്കിൽ (nelems + 1 <= size)  /* does one extra element still fit? */
    തിരികെ block;  /* nothing to be done */
  എങ്കിൽ (size >= limit / 2) {  /* cannot double it? */
    എങ്കിൽ (l_unlikely(size >= limit))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;  /* still have at least one free place */
  }
  അല്ലെങ്കിൽ {
    size *= 2;
    എങ്കിൽ (size < MINSIZEARRAY)
      size = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= size && size <= limit);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, block, cast_sizet(*psize) * size_elems,
                                         cast_sizet(size) * size_elems);
  *psize = size;  /* update only when everything else is OK */
  തിരികെ newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
ശൂന്യം *luaM_shrinkvector_ (lua_State *L, ശൂന്യം *block, പൂർണ്ണസംഖ്യ *size,
                          പൂർണ്ണസംഖ്യ final_n, unsigned size_elem) {
  ശൂന്യം *newblock;
  size_t oldsize = cast_sizet(*size) * size_elem;
  size_t newsize = cast_sizet(final_n) * size_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  തിരികെ newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
ശൂന്യം luaM_free_ (lua_State *L, ശൂന്യം *block, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ശൂന്യ));
  callfrealloc(g, block, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
സ്ഥിതം ശൂന്യം *tryagain (lua_State *L, ശൂന്യം *block,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  എങ്കിൽ (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    തിരികെ callfrealloc(g, block, osize, nsize);  /* try again */
  }
  അല്ലെങ്കിൽ തിരികെ ശൂന്യ;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
ശൂന്യം *luaM_realloc_ (lua_State *L, ശൂന്യം *block, size_t osize, size_t nsize) {
  ശൂന്യം *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (block == ശൂന്യ));
  newblock = firsttry(g, block, osize, nsize);
  എങ്കിൽ (l_unlikely(newblock == ശൂന്യ && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    എങ്കിൽ (newblock == ശൂന്യ)  /* still no memory? */
      തിരികെ ശൂന്യ;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == ശൂന്യ));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  തിരികെ newblock;
}


ശൂന്യം *luaM_saferealloc_ (lua_State *L, ശൂന്യം *block, size_t osize,
                                                    size_t nsize) {
  ശൂന്യം *newblock = luaM_realloc_(L, block, osize, nsize);
  എങ്കിൽ (l_unlikely(newblock == ശൂന്യ && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  തിരികെ newblock;
}


ശൂന്യം *luaM_malloc_ (lua_State *L, size_t size, പൂർണ്ണസംഖ്യ tag) {
  എങ്കിൽ (size == 0)
    തിരികെ ശൂന്യ;  /* that's all */
  അല്ലെങ്കിൽ {
    global_State *g = G(L);
    ശൂന്യം *newblock = firsttry(g, ശൂന്യ, cast_sizet(tag), size);
    എങ്കിൽ (l_unlikely(newblock == ശൂന്യ)) {
      newblock = tryagain(L, ശൂന്യ, cast_sizet(tag), size);
      എങ്കിൽ (newblock == ശൂന്യ)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, size);
    തിരികെ newblock;
  }
}
