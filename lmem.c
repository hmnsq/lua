/*
** $Id: lmem.c $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#تعریف_کرو lmem_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <stddef.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"



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




#اگر defined(EMERGENCYGCTESTS)
/*
** First allocation will fail except when freeing a block (frees never
** fail) and when it cannot try again; this fail will trigger 'tryagain'
** and a full GC cycle at every allocation.
*/
ساکن خالی *firsttry (global_State *g, خالی *بلاک, size_t os, size_t ns) {
  اگر (ns > 0 && cantryagain(g))
    واپس تہی;  /* fail */
  ورنہ  /* normal allocation */
    واپس callfrealloc(g, بلاک, os, ns);
}
#ورنہ
#تعریف_کرو firsttry(g,بلاک,os,ns)    callfrealloc(g, بلاک, os, ns)
#اگر_ختم





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


خالی *luaM_growaux_ (lua_State *L, خالی *بلاک, صحیح nelems, صحیح *psize,
                     غیرعلامتی سائز_elems, صحیح حد_بندی, ثابت حرف *what) {
  خالی *newblock;
  صحیح سائز = *psize;
  اگر (nelems + 1 <= سائز)  /* does one extra element still fit? */
    واپس بلاک;  /* nothing to be done */
  اگر (سائز >= حد_بندی / 2) {  /* cannot double it? */
    اگر (l_unlikely(سائز >= حد_بندی))  /* cannot grow even a little? */
      luaG_runerror(L, "too many %s (limit is %d)", what, حد_بندی);
    سائز = حد_بندی;  /* still have at least one free place */
  }
  ورنہ {
    سائز *= 2;
    اگر (سائز < MINSIZEARRAY)
      سائز = MINSIZEARRAY;  /* minimum size */
  }
  lua_assert(nelems + 1 <= سائز && سائز <= حد_بندی);
  /* 'limit' ensures that multiplication will not overflow */
  newblock = luaM_saferealloc_(L, بلاک, cast_sizet(*psize) * سائز_elems,
                                         cast_sizet(سائز) * سائز_elems);
  *psize = سائز;  /* update only when everything else is OK */
  واپس newblock;
}


/*
** In prototypes, the size of the array is also its number of
** elements (to save memory). So, if it cannot shrink an array
** to its number of elements, the only option is to raise an
** error.
*/
خالی *luaM_shrinkvector_ (lua_State *L, خالی *بلاک, صحیح *سائز,
                          صحیح final_n, غیرعلامتی سائز_elem) {
  خالی *newblock;
  size_t oldsize = cast_sizet(*سائز) * سائز_elem;
  size_t newsize = cast_sizet(final_n) * سائز_elem;
  lua_assert(newsize <= oldsize);
  newblock = luaM_saferealloc_(L, بلاک, oldsize, newsize);
  *سائز = final_n;
  واپس newblock;
}

/* }================================================================== */


l_noret luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}


/*
** Free memory
*/
خالی luaM_free_ (lua_State *L, خالی *بلاک, size_t osize) {
  global_State *g = G(L);
  lua_assert((osize == 0) == (بلاک == تہی));
  callfrealloc(g, بلاک, osize, 0);
  g->GCdebt += cast(l_mem, osize);
}


/*
** In case of allocation fail, this function will do an emergency
** collection to free some memory and then try the allocation again.
*/
ساکن خالی *tryagain (lua_State *L, خالی *بلاک,
                       size_t osize, size_t nsize) {
  global_State *g = G(L);
  اگر (cantryagain(g)) {
    luaC_fullgc(L, 1);  /* try to free some memory... */
    واپس callfrealloc(g, بلاک, osize, nsize);  /* try again */
  }
  ورنہ واپس تہی;  /* cannot run an emergency collection */
}


/*
** Generic allocation routine.
*/
خالی *luaM_realloc_ (lua_State *L, خالی *بلاک, size_t osize, size_t nsize) {
  خالی *newblock;
  global_State *g = G(L);
  lua_assert((osize == 0) == (بلاک == تہی));
  newblock = firsttry(g, بلاک, osize, nsize);
  اگر (l_unlikely(newblock == تہی && nsize > 0)) {
    newblock = tryagain(L, بلاک, osize, nsize);
    اگر (newblock == تہی)  /* still no memory? */
      واپس تہی;  /* do not update 'GCdebt' */
  }
  lua_assert((nsize == 0) == (newblock == تہی));
  g->GCdebt -= cast(l_mem, nsize) - cast(l_mem, osize);
  واپس newblock;
}


خالی *luaM_saferealloc_ (lua_State *L, خالی *بلاک, size_t osize,
                                                    size_t nsize) {
  خالی *newblock = luaM_realloc_(L, بلاک, osize, nsize);
  اگر (l_unlikely(newblock == تہی && nsize > 0))  /* allocation failed? */
    luaM_error(L);
  واپس newblock;
}


خالی *luaM_malloc_ (lua_State *L, size_t سائز, صحیح لیبل) {
  اگر (سائز == 0)
    واپس تہی;  /* that's all */
  ورنہ {
    global_State *g = G(L);
    خالی *newblock = firsttry(g, تہی, cast_sizet(لیبل), سائز);
    اگر (l_unlikely(newblock == تہی)) {
      newblock = tryagain(L, تہی, cast_sizet(لیبل), سائز);
      اگر (newblock == تہی)
        luaM_error(L);
    }
    g->GCdebt -= cast(l_mem, سائز);
    واپس newblock;
  }
}
