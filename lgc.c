/*
** $Id: lgc.c $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lgc_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"

#ಅನ್ತರ್ಭಾವ <string.h>


#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"


/*
** Maximum number of elements to sweep in each single step.
** (Large enough to dissipate fixed overheads but small enough
** to allow small steps for the collector.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು GCSWEEPMAX	20


/*
** Cost (in work units) of running one finalizer.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು CWUFIN	10


/* mask with all color bits */
#ವ್ಯಾಖ್ಯಾನಿಸು maskcolors	(bitmask(BLACKBIT) | WHITEBITS)

/* mask with all GC bits */
#ವ್ಯಾಖ್ಯಾನಿಸು maskgcbits      (maskcolors | AGEBITS)


/* macro to erase all color bits then set only the current white bit */
#ವ್ಯಾಖ್ಯಾನಿಸು makewhite(g,x)	\
  (x->marked = cast_byte((x->marked & ~maskcolors) | luaC_white(g)))

/* make an object gray (neither white nor black) */
#ವ್ಯಾಖ್ಯಾನಿಸು set2gray(x)	resetbits(x->marked, maskcolors)


/* make an object black (coming from any color) */
#ವ್ಯಾಖ್ಯಾನಿಸು set2black(x)  \
  (x->marked = cast_byte((x->marked & ~WHITEBITS) | bitmask(BLACKBIT)))


#ವ್ಯಾಖ್ಯಾನಿಸು valiswhite(x)   (iscollectable(x) && iswhite(gcvalue(x)))

#ವ್ಯಾಖ್ಯಾನಿಸು keyiswhite(n)   (keyiscollectable(n) && iswhite(gckey(n)))


/*
** Protected access to objects in values
*/
#ವ್ಯಾಖ್ಯಾನಿಸು gcvalueN(o)     (iscollectable(o) ? gcvalue(o) : ಖಾಲಿ)


/*
** Access to collectable objects in array part of tables
*/
#ವ್ಯಾಖ್ಯಾನಿಸು gcvalarr(t,i)  \
	((*get_Arr_ಟ್ಯಾಗ್(t,i) & ಬಿಟ್_ISCOLLECTABLE) ? getArrVal(t,i)->gc : ಖಾಲಿ)


#ವ್ಯಾಖ್ಯಾನಿಸು markvalue(g,o) { checkliveness(mainthread(g),o); \
  ಆದರೆ (valiswhite(o)) reallymarkobject(g,gcvalue(o)); }

#ವ್ಯಾಖ್ಯಾನಿಸು markkey(g, n)	{ ಆದರೆ keyiswhite(n) reallymarkobject(g,gckey(n)); }

#ವ್ಯಾಖ್ಯಾನಿಸು markobject(g,t)	{ ಆದರೆ (iswhite(t)) reallymarkobject(g, obj2gco(t)); }

/*
** mark an object that can be NULL (either because it is really optional,
** or it was stripped as debug info, or inside an uncompleted structure)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು markobjectN(g,t)	{ ಆದರೆ (t) markobject(g,t); }


ಸ್ಥಿರ ಶೂನ್ಯ reallymarkobject (global_State *g, GCObject *o);
ಸ್ಥಿರ ಶೂನ್ಯ atomic (lua_State *L);
ಸ್ಥಿರ ಶೂನ್ಯ entersweep (lua_State *L);


/*
** {======================================================
** Generic functions
** =======================================================
*/


/*
** one after last element in a hash array
*/
#ವ್ಯಾಖ್ಯಾನಿಸು gnodelast(h)	gnode(h, cast_sizet(sizenode(h)))


ಸ್ಥಿರ l_mem objsize (GCObject *o) {
  lu_mem res;
  ಆಯ್ಕೆ (o->tt) {
    ಸ್ಥಿತಿ LUA_VTABLE: {
      res = lua_H_ಗಾತ್ರ(gco2t(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      res = ಗಾತ್ರ_Lclosure(cl->nupvalues);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      res = ಗಾತ್ರ_Cclosure(cl->nupvalues);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      res = sizeudata(u->nuvalue, u->len);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VPROTO: {
      res = luaF_protosize(gco2p(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VTHREAD: {
      res = luaE_threadsize(gco2th(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      res = sizestrshr(cast_uint(ts->shrlen));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      res = luaS_sizelngstr(ts->u.lnglen, ts->shrlen);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VUPVAL: {
      res = ಗಾತ್ರ(ಮೇಲೆ_Val);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: res = 0; lua_assert(0);
  }
  ಹಿಂತಿರುಗಿ cast(l_mem, res);
}


ಸ್ಥಿರ GCObject **getgclist (GCObject *o) {
  ಆಯ್ಕೆ (o->tt) {
    ಸ್ಥಿತಿ LUA_VTABLE: ಹಿಂತಿರುಗಿ &gco2t(o)->gclist;
    ಸ್ಥಿತಿ LUA_VLCL: ಹಿಂತಿರುಗಿ &gco2lcl(o)->gclist;
    ಸ್ಥಿತಿ LUA_VCCL: ಹಿಂತಿರುಗಿ &gco2ccl(o)->gclist;
    ಸ್ಥಿತಿ LUA_VTHREAD: ಹಿಂತಿರುಗಿ &gco2th(o)->gclist;
    ಸ್ಥಿತಿ LUA_VPROTO: ಹಿಂತಿರುಗಿ &gco2p(o)->gclist;
    ಸ್ಥಿತಿ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      lua_assert(u->nuvalue > 0);
      ಹಿಂತಿರುಗಿ &u->gclist;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0); ಹಿಂತಿರುಗಿ 0;
  }
}


/*
** Link a collectable object 'o' with a known type into the list 'p'.
** (Must be a macro to access the 'gclist' field in different types.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು linkgclist(o,p)	linkgclist_(obj2gco(o), &(o)->gclist, &(p))

ಸ್ಥಿರ ಶೂನ್ಯ linkgclist_ (GCObject *o, GCObject **pnext, GCObject **list) {
  lua_assert(!isgray(o));  /* cannot be in a gray list */
  *pnext = *list;
  *list = o;
  set2gray(o);  /* now it is */
}


/*
** Link a generic collectable object 'o' into the list 'p'.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು linkobjgclist(o,p) linkgclist_(obj2gco(o), getgclist(o), &(p))



/*
** Clear keys for empty entries in tables. If entry is empty, mark its
** entry as dead. This allows the collection of the key, but keeps its
** entry in the table: its removal could break a chain and could break
** a table traversal.  Other places never manipulate dead keys, because
** its associated empty value is enough to signal that the entry is
** logically empty.
*/
ಸ್ಥಿರ ಶೂನ್ಯ clearkey (ಗಂಟು_2 *n) {
  lua_assert(isempty(gval(n)));
  ಆದರೆ (keyiscollectable(n))
    setdeadkey(n);  /* unused key; remove it */
}


/*
** tells whether a key or value can be cleared from a weak
** table. Non-collectable objects are never removed from weak
** tables. Strings behave as 'values', so are never removed too. for
** other objects: if really collected, cannot keep them; for objects
** being finalized, keep them in keys, but not in values
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ iscleared (global_State *g, ಸ್ಥಿರಾಂಕ GCObject *o) {
  ಆದರೆ (o == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 0;  /* non-collectable value */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (novariant(o->tt) == LUA_TSTRING) {
    markobject(g, o);  /* strings are 'values', so are never weak */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ iswhite(o);
}


/*
** Barrier that moves collector forward, that is, marks the white object
** 'v' being pointed by the black object 'o'.  In the generational
** mode, 'v' must also become old, if 'o' is old; however, it cannot
** be changed directly to OLD, because it may still point to non-old
** objects. So, it is marked as OLD0. In the next cycle it will become
** OLD1, and in the next it will finally become OLD (regular old). By
** then, any object it points to will also be old.  If called in the
** incremental sweep phase, it clears the black object to white (sweep
** it) to avoid other barrier calls for this same object. (That cannot
** be done is generational mode, as its sweep does not distinguish
** white from dead.)
*/
ಶೂನ್ಯ luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v) {
  global_State *g = G(L);
  lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
  ಆದರೆ (keepinvariant(g)) {  /* must keep invariant? */
    reallymarkobject(g, v);  /* restore invariant */
    ಆದರೆ (isold(o)) {
      lua_assert(!isold(v));  /* white object could not be old */
      setage(v, G_OLD0);  /* restore generational invariant */
    }
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* sweep phase */
    lua_assert(issweepphase(g));
    ಆದರೆ (g->gckind != KGC_GENMINOR)  /* incremental mode? */
      makewhite(g, o);  /* mark 'o' as white to avoid other barriers */
  }
}


/*
** barrier that moves collector backward, that is, mark the black object
** pointing to a white object as gray again.
*/
ಶೂನ್ಯ luaC_barrierback_ (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(isblack(o) && !isdead(g, o));
  lua_assert((g->gckind != KGC_GENMINOR)
          || (isold(o) && getage(o) != G_TOUCHED1));
  ಆದರೆ (getage(o) == G_TOUCHED2)  /* already in gray list? */
    set2gray(o);  /* make it gray to become touched1 */
  ಇಲ್ಲದಿದ್ದರೆ  /* link it in 'grayagain' and paint it gray */
    linkobjgclist(o, g->grayagain);
  ಆದರೆ (isold(o))  /* generational mode? */
    setage(o, G_TOUCHED1);  /* touched in current cycle */
}


ಶೂನ್ಯ luaC_fix (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(g->allgc == o);  /* object must be 1st in 'allgc' list! */
  set2gray(o);  /* they will be gray forever */
  setage(o, G_OLD);  /* and old forever */
  g->allgc = o->ಮುಂದಿನ;  /* remove object from 'allgc' list */
  o->ಮುಂದಿನ = g->fixedgc;  /* link it to 'fixedgc' list */
  g->fixedgc = o;
}


/*
** create a new collectable object (with given type, size, and offset)
** and link it to 'allgc' list.
*/
GCObject *luaC_newobjdt (lua_State *L, lu_byte tt, size_t sz, size_t ಸ್ಥಾನಾಂತರ) {
  global_State *g = G(L);
  ಅಕ್ಷರ *p = cast_charp(luaM_newobject(L, novariant(tt), sz));
  GCObject *o = cast(GCObject *, p + ಸ್ಥಾನಾಂತರ);
  o->marked = luaC_white(g);
  o->tt = tt;
  o->ಮುಂದಿನ = g->allgc;
  g->allgc = o;
  ಹಿಂತಿರುಗಿ o;
}


/*
** create a new collectable object with no offset.
*/
GCObject *luaC_newobj (lua_State *L, lu_byte tt, size_t sz) {
  ಹಿಂತಿರುಗಿ luaC_newobjdt(L, tt, sz, 0);
}

/* }====================================================== */



/*
** {======================================================
** Mark functions
** =======================================================
*/


/*
** Mark an object.  Userdata with no user values, strings, and closed
** upvalues are visited and turned black here.  Open upvalues are
** already indirectly linked through their respective threads in the
** 'twups' list, so they don't go to the gray list; nevertheless, they
** are kept gray to avoid barriers, as their values will be revisited
** by the thread or by 'remarkupvals'.  Other objects are added to the
** gray list to be visited (and turned black) later.  Both userdata and
** upvalues can call this function recursively, but this recursion goes
** for at most two levels: An upvalue cannot refer to another upvalue
** (only closures can), and a userdata's metatable must be a table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ reallymarkobject (global_State *g, GCObject *o) {
  g->GCmarked += objsize(o);
  ಆಯ್ಕೆ (o->tt) {
    ಸ್ಥಿತಿ LUA_VSHRSTR:
    ಸ್ಥಿತಿ LUA_VLNGSTR: {
      set2black(o);  /* nothing to visit */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VUPVAL: {
      ಮೇಲೆ_Val *uv = gco2upv(o);
      ಆದರೆ (upisopen(uv))
        set2gray(uv);  /* open upvalues are kept gray */
      ಇಲ್ಲದಿದ್ದರೆ
        set2black(uv);  /* closed upvalues are visited here */
      markvalue(g, uv->v.p);  /* mark its content */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      ಆದರೆ (u->nuvalue == 0) {  /* no user values? */
        markobjectN(g, u->metatable);  /* mark its metatable */
        set2black(u);  /* nothing else to mark */
        ನಿಲ್ಲಿಸು;
      }
      /* else... */
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ LUA_VLCL: ಸ್ಥಿತಿ LUA_VCCL: ಸ್ಥಿತಿ LUA_VTABLE:
    ಸ್ಥಿತಿ LUA_VTHREAD: ಸ್ಥಿತಿ LUA_VPROTO: {
      linkobjgclist(o, g->ಬೂದು);  /* to be visited later */
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0); ನಿಲ್ಲಿಸು;
  }
}


/*
** mark metamethods for basic types
*/
ಸ್ಥಿರ ಶೂನ್ಯ markmt (global_State *g) {
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i=0; i < LUA_NUMTYPES; i++)
    markobjectN(g, g->mt[i]);
}


/*
** mark all objects in list of being-finalized
*/
ಸ್ಥಿರ ಶೂನ್ಯ markbeingfnz (global_State *g) {
  GCObject *o;
  ಗಾಗಿ (o = g->tobefnz; o != ಖಾಲಿ; o = o->ಮುಂದಿನ)
    markobject(g, o);
}


/*
** For each non-marked thread, simulates a barrier between each open
** upvalue and its value. (If the thread is collected, the value will be
** assigned to the upvalue, but then it can be too late for the barrier
** to act. The "barrier" does not need to check colors: A non-marked
** thread must be young; upvalues cannot be older than their threads; so
** any visited upvalue must be young too.) Also removes the thread from
** the list, as it was already visited. Removes also threads with no
** upvalues, as they have nothing to be checked. (If the thread gets an
** upvalue later, it will be linked in the list again.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ remarkupvals (global_State *g) {
  lua_State *thread;
  lua_State **p = &g->twups;
  ಆಗ ((thread = *p) != ಖಾಲಿ) {
    ಆದರೆ (!iswhite(thread) && thread->openupval != ಖಾಲಿ)
      p = &thread->twups;  /* keep marked thread with upvalues in the list */
    ಇಲ್ಲದಿದ್ದರೆ {  /* thread is not marked or without upvalues */
      ಮೇಲೆ_Val *uv;
      lua_assert(!isold(thread) || thread->openupval == ಖಾಲಿ);
      *p = thread->twups;  /* remove thread from the list */
      thread->twups = thread;  /* mark that it is out of list */
      ಗಾಗಿ (uv = thread->openupval; uv != ಖಾಲಿ; uv = uv->u.open.ಮುಂದಿನ) {
        lua_assert(getage(uv) <= getage(thread));
        ಆದರೆ (!iswhite(uv)) {  /* upvalue already visited? */
          lua_assert(upisopen(uv) && isgray(uv));
          markvalue(g, uv->v.p);  /* mark its value */
        }
      }
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ cleargraylists (global_State *g) {
  g->ಬೂದು = g->grayagain = ಖಾಲಿ;
  g->weak = g->allweak = g->ephemeron = ಖಾಲಿ;
}


/*
** mark root set and reset all gray lists, to start a new collection.
** 'GCmarked' is initialized to count the total number of live bytes
** during a cycle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ restartcollection (global_State *g) {
  cleargraylists(g);
  g->GCmarked = 0;
  markobject(g, mainthread(g));
  markvalue(g, &g->l_registry);
  markmt(g);
  markbeingfnz(g);  /* mark any finalizing object left from previous cycle */
}

/* }====================================================== */


/*
** {======================================================
** Traverse functions
** =======================================================
*/


/*
** Check whether object 'o' should be kept in the 'grayagain' list for
** post-processing by 'correctgraylist'. (It could put all old objects
** in the list and leave all the work to 'correctgraylist', but it is
** more efficient to avoid adding elements that will be removed.) Only
** TOUCHED1 objects need to be in the list. TOUCHED2 doesn't need to go
** back to a gray list, but then it must become OLD. (That is what
** 'correctgraylist' does when it finds a TOUCHED2 object.)
** This function is a no-op in incremental mode, as objects cannot be
** marked as touched in that mode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ genlink (global_State *g, GCObject *o) {
  lua_assert(isblack(o));
  ಆದರೆ (getage(o) == G_TOUCHED1) {  /* touched in this cycle? */
    linkobjgclist(o, g->grayagain);  /* link it back in 'grayagain' */
  }  /* everything else do not need to be linked back */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (getage(o) == G_TOUCHED2)
    setage(o, G_OLD);  /* advance age */
}


/*
** Traverse a table with weak values and link it to proper list. During
** propagate phase, keep it in 'grayagain' list, to be revisited in the
** atomic phase. In the atomic phase, if table has any white value,
** put it in 'weak' list, to be cleared; otherwise, call 'genlink'
** to check table age in generational mode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ traverseweakvalue (global_State *g, ಕೋಷ್ಟಕ *h) {
  ಗಂಟು_2 *n, *ಎಲ್ಲೆ = gnodelast(h);
  /* if there is array part, assume it may have white values (it is not
     worth traversing it now just to check) */
  ಪೂರ್ಣಾಂಕ hasclears = (h->asize > 0);
  ಗಾಗಿ (n = gnode(h, 0); n < ಎಲ್ಲೆ; n++) {  /* traverse hash part */
    ಆದರೆ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ಇಲ್ಲದಿದ್ದರೆ {
      lua_assert(!keyisnil(n));
      markkey(g, n);
      ಆದರೆ (!hasclears && iscleared(g, gcvalueN(gval(n))))  /* a white value? */
        hasclears = 1;  /* table will have to be cleared */
    }
  }
  ಆದರೆ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (hasclears)
      linkgclist(h, g->weak);  /* has to be cleared later */
  ಇಲ್ಲದಿದ್ದರೆ
    genlink(g, obj2gco(h));
}


/*
** Traverse the array part of a table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ traversearray (global_State *g, ಕೋಷ್ಟಕ *h) {
  ಚಿಹ್ನೆರಹಿತ asize = h->asize;
  ಪೂರ್ಣಾಂಕ marked = 0;  /* true if some object is marked in this traversal */
  ಚಿಹ್ನೆರಹಿತ i;
  ಗಾಗಿ (i = 0; i < asize; i++) {
    GCObject *o = gcvalarr(h, i);
    ಆದರೆ (o != ಖಾಲಿ && iswhite(o)) {
      marked = 1;
      reallymarkobject(g, o);
    }
  }
  ಹಿಂತಿರುಗಿ marked;
}


/*
** Traverse an ephemeron table and link it to proper list. Returns true
** iff any object was marked during this traversal (which implies that
** convergence has to continue). During propagation phase, keep table
** in 'grayagain' list, to be visited again in the atomic phase. In
** the atomic phase, if table has any white->white entry, it has to
** be revisited during ephemeron convergence (as that key may turn
** black). Otherwise, if it has any white key, table has to be cleared
** (in the atomic phase). In generational mode, some tables
** must be kept in some gray list for post-processing; this is done
** by 'genlink'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ traverseephemeron (global_State *g, ಕೋಷ್ಟಕ *h, ಪೂರ್ಣಾಂಕ inv) {
  ಪೂರ್ಣಾಂಕ hasclears = 0;  /* true if table has white keys */
  ಪೂರ್ಣಾಂಕ hasww = 0;  /* true if table has entry "white-key -> white-value" */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nsize = sizenode(h);
  ಪೂರ್ಣಾಂಕ marked = traversearray(g, h);  /* traverse array part */
  /* traverse hash part; if 'inv', traverse descending
     (see 'convergeephemerons') */
  ಗಾಗಿ (i = 0; i < nsize; i++) {
    ಗಂಟು_2 *n = inv ? gnode(h, nsize - 1 - i) : gnode(h, i);
    ಆದರೆ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (iscleared(g, gckeyN(n))) {  /* key is not marked (yet)? */
      hasclears = 1;  /* table must be cleared */
      ಆದರೆ (valiswhite(gval(n)))  /* value not marked yet? */
        hasww = 1;  /* white-white entry */
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (valiswhite(gval(n))) {  /* value not marked yet? */
      marked = 1;
      reallymarkobject(g, gcvalue(gval(n)));  /* mark it now */
    }
  }
  /* link table into proper list */
  ಆದರೆ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (hasww)  /* table has white->white entries? */
    linkgclist(h, g->ephemeron);  /* have to propagate again */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (hasclears)  /* table has white keys? */
    linkgclist(h, g->allweak);  /* may have to clean white keys */
  ಇಲ್ಲದಿದ್ದರೆ
    genlink(g, obj2gco(h));  /* check whether collector still needs to see it */
  ಹಿಂತಿರುಗಿ marked;
}


ಸ್ಥಿರ ಶೂನ್ಯ traversestrongtable (global_State *g, ಕೋಷ್ಟಕ *h) {
  ಗಂಟು_2 *n, *ಎಲ್ಲೆ = gnodelast(h);
  traversearray(g, h);
  ಗಾಗಿ (n = gnode(h, 0); n < ಎಲ್ಲೆ; n++) {  /* traverse hash part */
    ಆದರೆ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ಇಲ್ಲದಿದ್ದರೆ {
      lua_assert(!keyisnil(n));
      markkey(g, n);
      markvalue(g, gval(n));
    }
  }
  genlink(g, obj2gco(h));
}


/*
** (result & 1) iff weak values; (result & 2) iff weak keys.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getmode (global_State *g, ಕೋಷ್ಟಕ *h) {
  ಸ್ಥಿರಾಂಕ TValue *ಬಹುಲಕ = gfasttm(g, h->metatable, TM_ಬಹುಲಕ);
  ಆದರೆ (ಬಹುಲಕ == ಖಾಲಿ || !ttisstring(ಬಹುಲಕ))
    ಹಿಂತಿರುಗಿ 0;  /* ignore non-string modes */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *smode = getstr(tsvalue(ಬಹುಲಕ));
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *weakkey = strchr(smode, 'k');
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *weakvalue = strchr(smode, 'v');
    ಹಿಂತಿರುಗಿ ((weakkey != ಖಾಲಿ) << 1) | (weakvalue != ಖಾಲಿ);
  }
}


ಸ್ಥಿರ l_mem traversetable (global_State *g, ಕೋಷ್ಟಕ *h) {
  markobjectN(g, h->metatable);
  ಆಯ್ಕೆ (getmode(g, h)) {
    ಸ್ಥಿತಿ 0:  /* not weak */
      traversestrongtable(g, h);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1:  /* weak values */
      traverseweakvalue(g, h);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 2:  /* weak keys */
      traverseephemeron(g, h, 0);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 3:  /* all weak; nothing to traverse */
      ಆದರೆ (g->gcstate == GCSpropagate)
        linkgclist(h, g->grayagain);  /* must visit again its metatable */
      ಇಲ್ಲದಿದ್ದರೆ
        linkgclist(h, g->allweak);  /* must clear collected entries */
      ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ cast(l_mem, 1 + 2*sizenode(h) + h->asize);
}


ಸ್ಥಿರ l_mem traverseudata (global_State *g, Udata *u) {
  ಪೂರ್ಣಾಂಕ i;
  markobjectN(g, u->metatable);  /* mark its metatable */
  ಗಾಗಿ (i = 0; i < u->nuvalue; i++)
    markvalue(g, &u->uv[i].uv);
  genlink(g, obj2gco(u));
  ಹಿಂತಿರುಗಿ 1 + u->nuvalue;
}


/*
** Traverse a prototype. (While a prototype is being build, its
** arrays can be larger than needed; the extra slots are filled with
** NULL, so the use of 'markobjectN')
*/
ಸ್ಥಿರ l_mem traverseproto (global_State *g, Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  markobjectN(g, f->ಮೂಲ);
  ಗಾಗಿ (i = 0; i < f->sizek; i++)  /* mark literals */
    markvalue(g, &f->k[i]);
  ಗಾಗಿ (i = 0; i < f->sizeupvalues; i++)  /* mark upvalue names */
    markobjectN(g, f->upvalues[i].name);
  ಗಾಗಿ (i = 0; i < f->sizep; i++)  /* mark nested protos */
    markobjectN(g, f->p[i]);
  ಗಾಗಿ (i = 0; i < f->sizelocvars; i++)  /* mark local-variable names */
    markobjectN(g, f->locvars[i].varname);
  ಹಿಂತಿರುಗಿ 1 + f->sizek + f->sizeupvalues + f->sizep + f->sizelocvars;
}


ಸ್ಥಿರ l_mem traverseCclosure (global_State *g, CClosure *cl) {
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < cl->nupvalues; i++)  /* mark its upvalues */
    markvalue(g, &cl->upvalue[i]);
  ಹಿಂತಿರುಗಿ 1 + cl->nupvalues;
}

/*
** Traverse a Lua closure, marking its prototype and its upvalues.
** (Both can be NULL while closure is being created.)
*/
ಸ್ಥಿರ l_mem traverseLclosure (global_State *g, LClosure *cl) {
  ಪೂರ್ಣಾಂಕ i;
  markobjectN(g, cl->p);  /* mark its prototype */
  ಗಾಗಿ (i = 0; i < cl->nupvalues; i++) {  /* visit its upvalues */
    ಮೇಲೆ_Val *uv = cl->upvals[i];
    markobjectN(g, uv);  /* mark upvalue */
  }
  ಹಿಂತಿರುಗಿ 1 + cl->nupvalues;
}


/*
** Traverse a thread, marking the elements in the stack up to its top
** and cleaning the rest of the stack in the final traversal. That
** ensures that the entire stack have valid (non-dead) objects.
** Threads have no barriers. In gen. mode, old threads must be visited
** at every cycle, because they might point to young objects.  In inc.
** mode, the thread can still be modified before the end of the cycle,
** and therefore it must be visited again in the atomic phase. To ensure
** these visits, threads must return to a gray list if they are not new
** (which can only happen in generational mode) or if the traverse is in
** the propagate phase (which can only happen in incremental mode).
*/
ಸ್ಥಿರ l_mem traversethread (global_State *g, lua_State *th) {
  ಮೇಲೆ_Val *uv;
  StkId o = th->ರಾಶಿ.p;
  ಆದರೆ (isold(th) || g->gcstate == GCSpropagate)
    linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
  ಆದರೆ (o == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;  /* stack not completely built yet */
  lua_assert(g->gcstate == GCSatomic ||
             th->openupval == ಖಾಲಿ || isintwups(th));
  ಗಾಗಿ (; o < th->ತುದಿ.p; o++)  /* mark live elements in the stack */
    markvalue(g, s2v(o));
  ಗಾಗಿ (uv = th->openupval; uv != ಖಾಲಿ; uv = uv->u.open.ಮುಂದಿನ)
    markobject(g, uv);  /* open upvalues cannot be collected */
  ಆದರೆ (g->gcstate == GCSatomic) {  /* final traversal? */
    ಆದರೆ (!g->gcemergency)
      luaD_shrinkstack(th); /* do not change stack in emergency cycle */
    ಗಾಗಿ (o = th->ತುದಿ.p; o < th->ರಾಶಿ_ಕೊನೆಯ.p + EXTRA_ರಾಶಿ; o++)
      setnilvalue2s(o);  /* clear dead stack slice */
    /* 'remarkupvals' may have removed thread from 'twups' list */
    ಆದರೆ (!isintwups(th) && th->openupval != ಖಾಲಿ) {
      th->twups = g->twups;  /* link it back to the list */
      g->twups = th;
    }
  }
  ಹಿಂತಿರುಗಿ 1 + (th->ತುದಿ.p - th->ರಾಶಿ.p);
}


/*
** traverse one gray object, turning it to black. Return an estimate
** of the number of slots traversed.
*/
ಸ್ಥಿರ l_mem propagatemark (global_State *g) {
  GCObject *o = g->ಬೂದು;
  nw2black(o);
  g->ಬೂದು = *getgclist(o);  /* remove from 'gray' list */
  ಆಯ್ಕೆ (o->tt) {
    ಸ್ಥಿತಿ LUA_VTABLE: ಹಿಂತಿರುಗಿ traversetable(g, gco2t(o));
    ಸ್ಥಿತಿ LUA_VUSERDATA: ಹಿಂತಿರುಗಿ traverseudata(g, gco2u(o));
    ಸ್ಥಿತಿ LUA_VLCL: ಹಿಂತಿರುಗಿ traverseLclosure(g, gco2lcl(o));
    ಸ್ಥಿತಿ LUA_VCCL: ಹಿಂತಿರುಗಿ traverseCclosure(g, gco2ccl(o));
    ಸ್ಥಿತಿ LUA_VPROTO: ಹಿಂತಿರುಗಿ traverseproto(g, gco2p(o));
    ಸ್ಥಿತಿ LUA_VTHREAD: ಹಿಂತಿರುಗಿ traversethread(g, gco2th(o));
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0); ಹಿಂತಿರುಗಿ 0;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ propagateall (global_State *g) {
  ಆಗ (g->ಬೂದು)
    propagatemark(g);
}


/*
** Traverse all ephemeron tables propagating marks from keys to values.
** Repeat until it converges, that is, nothing new is marked. 'dir'
** inverts the direction of the traversals, trying to speed up
** convergence on chains in the same table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ convergeephemerons (global_State *g) {
  ಪೂರ್ಣಾಂಕ ಬದಲಾಯಿತು;
  ಪೂರ್ಣಾಂಕ dir = 0;
  ಮಾಡು {
    GCObject *w;
    GCObject *ಮುಂದಿನ = g->ephemeron;  /* get ephemeron list */
    g->ephemeron = ಖಾಲಿ;  /* tables may return to this list when traversed */
    ಬದಲಾಯಿತು = 0;
    ಆಗ ((w = ಮುಂದಿನ) != ಖಾಲಿ) {  /* for each ephemeron table */
      ಕೋಷ್ಟಕ *h = gco2t(w);
      ಮುಂದಿನ = h->gclist;  /* list is rebuilt during loop */
      nw2black(h);  /* out of the list (for now) */
      ಆದರೆ (traverseephemeron(g, h, dir)) {  /* marked some value? */
        propagateall(g);  /* propagate changes */
        ಬದಲಾಯಿತು = 1;  /* will have to revisit all ephemeron tables */
      }
    }
    dir = !dir;  /* invert direction next time */
  } ಆಗ (ಬದಲಾಯಿತು);  /* repeat until no more changes */
}

/* }====================================================== */


/*
** {======================================================
** Sweep Functions
** =======================================================
*/


/*
** clear entries with unmarked keys from all weaktables in list 'l'
*/
ಸ್ಥಿರ ಶೂನ್ಯ clearbykeys (global_State *g, GCObject *l) {
  ಗಾಗಿ (; l; l = gco2t(l)->gclist) {
    ಕೋಷ್ಟಕ *h = gco2t(l);
    ಗಂಟು_2 *ಎಲ್ಲೆ = gnodelast(h);
    ಗಂಟು_2 *n;
    ಗಾಗಿ (n = gnode(h, 0); n < ಎಲ್ಲೆ; n++) {
      ಆದರೆ (iscleared(g, gckeyN(n)))  /* unmarked key? */
        setempty(gval(n));  /* remove entry */
      ಆದರೆ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


/*
** clear entries with unmarked values from all weaktables in list 'l' up
** to element 'f'
*/
ಸ್ಥಿರ ಶೂನ್ಯ clearbyvalues (global_State *g, GCObject *l, GCObject *f) {
  ಗಾಗಿ (; l != f; l = gco2t(l)->gclist) {
    ಕೋಷ್ಟಕ *h = gco2t(l);
    ಗಂಟು_2 *n, *ಎಲ್ಲೆ = gnodelast(h);
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ asize = h->asize;
    ಗಾಗಿ (i = 0; i < asize; i++) {
      GCObject *o = gcvalarr(h, i);
      ಆದರೆ (iscleared(g, o))  /* value was collected? */
        *get_Arr_ಟ್ಯಾಗ್(h, i) = LUA_VEMPTY;  /* remove entry */
    }
    ಗಾಗಿ (n = gnode(h, 0); n < ಎಲ್ಲೆ; n++) {
      ಆದರೆ (iscleared(g, gcvalueN(gval(n))))  /* unmarked value? */
        setempty(gval(n));  /* remove entry */
      ಆದರೆ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ freeupval (lua_State *L, ಮೇಲೆ_Val *uv) {
  ಆದರೆ (upisopen(uv))
    luaF_unlinkupval(uv);
  luaM_free(L, uv);
}


ಸ್ಥಿರ ಶೂನ್ಯ freeobj (lua_State *L, GCObject *o) {
  assert_ಸಂಕೇತ(l_mem newmem = gettotalbytes(G(L)) - objsize(o));
  ಆಯ್ಕೆ (o->tt) {
    ಸ್ಥಿತಿ LUA_VPROTO:
      luaF_freeproto(L, gco2p(o));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VUPVAL:
      freeupval(L, gco2upv(o));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      luaM_freemem(L, cl, ಗಾತ್ರ_Lclosure(cl->nupvalues));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      luaM_freemem(L, cl, ಗಾತ್ರ_Cclosure(cl->nupvalues));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VTABLE:
      luaH_free(L, gco2t(o));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VTHREAD:
      luaE_freethread(L, gco2th(o));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      luaM_freemem(L, o, sizeudata(u->nuvalue, u->len));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      luaS_remove(L, ts);  /* remove it from hash table */
      luaM_freemem(L, ts, sizestrshr(cast_uint(ts->shrlen)));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      ಆದರೆ (ts->shrlen == LSTRMEM)  /* must free external string? */
        (*ts->falloc)(ts->ud, ts->contents, ts->u.lnglen + 1, 0);
      luaM_freemem(L, ts, luaS_sizelngstr(ts->u.lnglen, ts->shrlen));
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0);
  }
  lua_assert(gettotalbytes(G(L)) == newmem);
}


/*
** sweep at most 'countin' elements from a list of GCObjects erasing dead
** objects, where a dead object is one marked with the old (non current)
** white; change all non-dead objects back to white (and new), preparing
** for next collection cycle. Return where to continue the traversal or
** NULL if list is finished.
*/
ಸ್ಥಿರ GCObject **sweeplist (lua_State *L, GCObject **p, l_mem countin) {
  global_State *g = G(L);
  ಪೂರ್ಣಾಂಕ ow = otherwhite(g);
  ಪೂರ್ಣಾಂಕ white = luaC_white(g);  /* current white */
  ಆಗ (*p != ಖಾಲಿ && countin-- > 0) {
    GCObject *curr = *p;
    ಪೂರ್ಣಾಂಕ marked = curr->marked;
    ಆದರೆ (isdeadm(ow, marked)) {  /* is 'curr' dead? */
      *p = curr->ಮುಂದಿನ;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* change mark to 'white' and age to 'new' */
      curr->marked = cast_byte((marked & ~maskgcbits) | white | G_NEW);
      p = &curr->ಮುಂದಿನ;  /* go to next element */
    }
  }
  ಹಿಂತಿರುಗಿ (*p == ಖಾಲಿ) ? ಖಾಲಿ : p;
}


/*
** sweep a list until a live object (or end of list)
*/
ಸ್ಥಿರ GCObject **sweeptolive (lua_State *L, GCObject **p) {
  GCObject **old = p;
  ಮಾಡು {
    p = sweeplist(L, p, 1);
  } ಆಗ (p == old);
  ಹಿಂತಿರುಗಿ p;
}

/* }====================================================== */


/*
** {======================================================
** Finalization
** =======================================================
*/

/*
** If possible, shrink string table.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkSizes (lua_State *L, global_State *g) {
  ಆದರೆ (!g->gcemergency) {
    ಆದರೆ (g->strt.nuse < g->strt.ಗಾತ್ರ_2 / 4)  /* string table too big? */
      luaS_resize(L, g->strt.ಗಾತ್ರ_2 / 2);
  }
}


/*
** Get the next udata to be finalized from the 'tobefnz' list, and
** link it back into the 'allgc' list.
*/
ಸ್ಥಿರ GCObject *udata2finalize (global_State *g) {
  GCObject *o = g->tobefnz;  /* get first element */
  lua_assert(tofinalize(o));
  g->tobefnz = o->ಮುಂದಿನ;  /* remove it from 'tobefnz' list */
  o->ಮುಂದಿನ = g->allgc;  /* return it to 'allgc' list */
  g->allgc = o;
  resetbit(o->marked, FINALIZEDBIT);  /* object is "normal" again */
  ಆದರೆ (issweepphase(g))
    makewhite(g, o);  /* "sweep" object */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (getage(o) == G_OLD1)
    g->firstold1 = o;  /* it is the first OLD1 object in the list */
  ಹಿಂತಿರುಗಿ o;
}


ಸ್ಥಿರ ಶೂನ್ಯ dothecall (lua_State *L, ಶೂನ್ಯ *ud) {
  ಬಳಸದ(ud);
  luaD_callnoyield(L, L->ತುದಿ.p - 2, 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ GCTM (lua_State *L) {
  global_State *g = G(L);
  ಸ್ಥಿರಾಂಕ TValue *tm;
  TValue v;
  lua_assert(!g->gcemergency);
  setgcovalue(L, &v, udata2finalize(g));
  tm = luaT_gettmbyobj(L, &v, TM_GC);
  ಆದರೆ (!notm(tm)) {  /* is there a finalizer? */
    TStatus ಸ್ಥಿತಿ_2;
    lu_byte oldah = L->allowhook;
    lu_byte oldgcstp  = g->gcstp;
    g->gcstp |= GCSTPGC;  /* avoid GC steps */
    L->allowhook = 0;  /* stop debug hooks during GC metamethod */
    setobj2s(L, L->ತುದಿ.p++, tm);  /* push finalizer... */
    setobj2s(L, L->ತುದಿ.p++, &v);  /* ... and its argument */
    L->ci->callstatus |= CIST_FIN;  /* will run a finalizer */
    ಸ್ಥಿತಿ_2 = luaD_pcall(L, dothecall, ಖಾಲಿ, savestack(L, L->ತುದಿ.p - 2), 0);
    L->ci->callstatus &= ~CIST_FIN;  /* not running a finalizer anymore */
    L->allowhook = oldah;  /* restore hooks */
    g->gcstp = oldgcstp;  /* restore state */
    ಆದರೆ (l_unlikely(ಸ್ಥಿತಿ_2 != LUA_OK)) {  /* error while running __gc? */
      luaE_warnerror(L, "__gc");
      L->ತುದಿ.p--;  /* pops error object */
    }
  }
}


/*
** call all pending finalizers
*/
ಸ್ಥಿರ ಶೂನ್ಯ callallpendingfinalizers (lua_State *L) {
  global_State *g = G(L);
  ಆಗ (g->tobefnz)
    GCTM(L);
}


/*
** find last 'next' field in list 'p' list (to add elements in its end)
*/
ಸ್ಥಿರ GCObject **findlast (GCObject **p) {
  ಆಗ (*p != ಖಾಲಿ)
    p = &(*p)->ಮುಂದಿನ;
  ಹಿಂತಿರುಗಿ p;
}


/*
** Move all unreachable objects (or 'all' objects) that need
** finalization from list 'finobj' to list 'tobefnz' (to be finalized).
** (Note that objects after 'finobjold1' cannot be white, so they
** don't need to be traversed. In incremental mode, 'finobjold1' is NULL,
** so the whole list is traversed.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ separatetobefnz (global_State *g, ಪೂರ್ಣಾಂಕ all) {
  GCObject *curr;
  GCObject **p = &g->finobj;
  GCObject **lastnext = findlast(&g->tobefnz);
  ಆಗ ((curr = *p) != g->finobjold1) {  /* traverse all finalizable objects */
    lua_assert(tofinalize(curr));
    ಆದರೆ (!(iswhite(curr) || all))  /* not being collected? */
      p = &curr->ಮುಂದಿನ;  /* don't bother with it */
    ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (curr == g->finobjsur)  /* removing 'finobjsur'? */
        g->finobjsur = curr->ಮುಂದಿನ;  /* correct it */
      *p = curr->ಮುಂದಿನ;  /* remove 'curr' from 'finobj' list */
      curr->ಮುಂದಿನ = *lastnext;  /* link at the end of 'tobefnz' list */
      *lastnext = curr;
      lastnext = &curr->ಮುಂದಿನ;
    }
  }
}


/*
** If pointer 'p' points to 'o', move it to the next element.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkpointer (GCObject **p, GCObject *o) {
  ಆದರೆ (o == *p)
    *p = o->ಮುಂದಿನ;
}


/*
** Correct pointers to objects inside 'allgc' list when
** object 'o' is being removed from the list.
*/
ಸ್ಥಿರ ಶೂನ್ಯ correctpointers (global_State *g, GCObject *o) {
  checkpointer(&g->survival, o);
  checkpointer(&g->old1, o);
  checkpointer(&g->reallyold, o);
  checkpointer(&g->firstold1, o);
}


/*
** if object 'o' has a finalizer, remove it from 'allgc' list (must
** search the list to find it) and link it in 'finobj' list.
*/
ಶೂನ್ಯ luaC_checkfinalizer (lua_State *L, GCObject *o, ಕೋಷ್ಟಕ *mt) {
  global_State *g = G(L);
  ಆದರೆ (tofinalize(o) ||                 /* obj. is already marked... */
      gfasttm(g, mt, TM_GC) == ಖಾಲಿ ||    /* or has no finalizer... */
      (g->gcstp & GCSTPCLS))                   /* or closing state? */
    ಹಿಂತಿರುಗಿ;  /* nothing to be done */
  ಇಲ್ಲದಿದ್ದರೆ {  /* move 'o' to 'finobj' list */
    GCObject **p;
    ಆದರೆ (issweepphase(g)) {
      makewhite(g, o);  /* "sweep" object 'o' */
      ಆದರೆ (g->sweepgc == &o->ಮುಂದಿನ)  /* should not remove 'sweepgc' object */
        g->sweepgc = sweeptolive(L, g->sweepgc);  /* change 'sweepgc' */
    }
    ಇಲ್ಲದಿದ್ದರೆ
      correctpointers(g, o);
    /* search for pointer pointing to 'o' */
    ಗಾಗಿ (p = &g->allgc; *p != o; p = &(*p)->ಮುಂದಿನ) { /*ಖಾಲಿಗಳು*/ }
    *p = o->ಮುಂದಿನ;  /* remove 'o' from 'allgc' list */
    o->ಮುಂದಿನ = g->finobj;  /* link it in 'finobj' list */
    g->finobj = o;
    l_setbit(o->marked, FINALIZEDBIT);  /* mark it as such */
  }
}

/* }====================================================== */


/*
** {======================================================
** Generational Collector
** =======================================================
*/

/*
** Fields 'GCmarked' and 'GCmajorminor' are used to control the pace and
** the mode of the collector. They play several roles, depending on the
** mode of the collector:
** * KGC_INC:
**     GCmarked: number of marked bytes during a cycle.
**     GCmajorminor: not used.
** * KGC_GENMINOR
**     GCmarked: number of bytes that became old since last major collection.
**     GCmajorminor: number of bytes marked in last major collection.
** * KGC_GENMAJOR
**     GCmarked: number of bytes that became old since last major collection.
**     GCmajorminor: number of bytes marked in last major collection.
*/


/*
** Set the "time" to wait before starting a new incremental cycle;
** cycle will start when number of bytes in use hits the threshold of
** approximately (marked * pause / 100).
*/
ಸ್ಥಿರ ಶೂನ್ಯ setpause (global_State *g) {
  l_mem ಮಿತಿ = applygcparam(g, PAUSE, g->GCmarked);
  l_mem debt = ಮಿತಿ - gettotalbytes(g);
  ಆದರೆ (debt < 0) debt = 0;
  luaE_setdebt(g, debt);
}


/*
** Sweep a list of objects to enter generational mode.  Deletes dead
** objects and turns the non dead to old. All non-dead threads---which
** are now old---must be in a gray list. Everything else is not in a
** gray list. Open upvalues are also kept gray.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sweep2old (lua_State *L, GCObject **p) {
  GCObject *curr;
  global_State *g = G(L);
  ಆಗ ((curr = *p) != ಖಾಲಿ) {
    ಆದರೆ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(isdead(g, curr));
      *p = curr->ಮುಂದಿನ;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* all surviving objects become old */
      setage(curr, G_OLD);
      ಆದರೆ (curr->tt == LUA_VTHREAD) {  /* threads must be watched */
        lua_State *th = gco2th(curr);
        linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (curr->tt == LUA_VUPVAL && upisopen(gco2upv(curr)))
        set2gray(curr);  /* open upvalues are always gray */
      ಇಲ್ಲದಿದ್ದರೆ  /* everything else is black */
        nw2black(curr);
      p = &curr->ಮುಂದಿನ;  /* go to next element */
    }
  }
}


/*
** Sweep for generational mode. Delete dead objects. (Because the
** collection is not incremental, there are no "new white" objects
** during the sweep. So, any white object must be dead.) For
** non-dead objects, advance their ages and clear the color of
** new objects. (Old objects keep their colors.)
** The ages of G_TOUCHED1 and G_TOUCHED2 objects cannot be advanced
** here, because these old-generation objects are usually not swept
** here.  They will all be advanced in 'correctgraylist'. That function
** will also remove objects turned white here from any gray list.
*/
ಸ್ಥಿರ GCObject **sweepgen (lua_State *L, global_State *g, GCObject **p,
                            GCObject *ಎಲ್ಲೆ, GCObject **pfirstold1,
                            l_mem *paddedold) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ lu_byte nextage[] = {
    G_SURVIVAL,  /* from G_NEW */
    G_OLD1,      /* from G_SURVIVAL */
    G_OLD1,      /* from G_OLD0 */
    G_OLD,       /* from G_OLD1 */
    G_OLD,       /* from G_OLD (do not change) */
    G_TOUCHED1,  /* from G_TOUCHED1 (do not change) */
    G_TOUCHED2   /* from G_TOUCHED2 (do not change) */
  };
  l_mem addedold = 0;
  ಪೂರ್ಣಾಂಕ white = luaC_white(g);
  GCObject *curr;
  ಆಗ ((curr = *p) != ಎಲ್ಲೆ) {
    ಆದರೆ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(!isold(curr) && isdead(g, curr));
      *p = curr->ಮುಂದಿನ;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* correct mark and age */
      ಪೂರ್ಣಾಂಕ age = getage(curr);
      ಆದರೆ (age == G_NEW) {  /* new objects go back to white */
        ಪೂರ್ಣಾಂಕ marked = curr->marked & ~maskgcbits;  /* erase GC bits */
        curr->marked = cast_byte(marked | G_SURVIVAL | white);
      }
      ಇಲ್ಲದಿದ್ದರೆ {  /* all other objects will be old, and so keep their color */
        lua_assert(age != G_OLD1);  /* advanced in 'markold' */
        setage(curr, nextage[age]);
        ಆದರೆ (getage(curr) == G_OLD1) {
          addedold += objsize(curr);  /* bytes becoming old */
          ಆದರೆ (*pfirstold1 == ಖಾಲಿ)
            *pfirstold1 = curr;  /* first OLD1 object in the list */
        }
      }
      p = &curr->ಮುಂದಿನ;  /* go to next element */
    }
  }
  *paddedold += addedold;
  ಹಿಂತಿರುಗಿ p;
}


/*
** Correct a list of gray objects. Return a pointer to the last element
** left on the list, so that we can link another list to the end of
** this one.
** Because this correction is done after sweeping, young objects might
** be turned white and still be in the list. They are only removed.
** 'TOUCHED1' objects are advanced to 'TOUCHED2' and remain on the list;
** Non-white threads also remain on the list. 'TOUCHED2' objects and
** anything else become regular old, are marked black, and are removed
** from the list.
*/
ಸ್ಥಿರ GCObject **correctgraylist (GCObject **p) {
  GCObject *curr;
  ಆಗ ((curr = *p) != ಖಾಲಿ) {
    GCObject **ಮುಂದಿನ = getgclist(curr);
    ಆದರೆ (iswhite(curr))
      ಹೋಗು remove;  /* remove all white objects */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (getage(curr) == G_TOUCHED1) {  /* touched in this cycle? */
      lua_assert(isgray(curr));
      nw2black(curr);  /* make it black, for next barrier */
      setage(curr, G_TOUCHED2);
      ಹೋಗು remain;  /* keep it in the list and go to next element */
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (curr->tt == LUA_VTHREAD) {
      lua_assert(isgray(curr));
      ಹೋಗು remain;  /* keep non-white threads on the list */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* everything else is removed */
      lua_assert(isold(curr));  /* young objects should be white here */
      ಆದರೆ (getage(curr) == G_TOUCHED2)  /* advance from TOUCHED2... */
        setage(curr, G_OLD);  /* ... to OLD */
      nw2black(curr);  /* make object black (to be removed) */
      ಹೋಗು remove;
    }
    remove: *p = *ಮುಂದಿನ; ಮುಂದುವರಿ;
    remain: p = ಮುಂದಿನ; ಮುಂದುವರಿ;
  }
  ಹಿಂತಿರುಗಿ p;
}


/*
** Correct all gray lists, coalescing them into 'grayagain'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ correctgraylists (global_State *g) {
  GCObject **list = correctgraylist(&g->grayagain);
  *list = g->weak; g->weak = ಖಾಲಿ;
  list = correctgraylist(list);
  *list = g->allweak; g->allweak = ಖಾಲಿ;
  list = correctgraylist(list);
  *list = g->ephemeron; g->ephemeron = ಖಾಲಿ;
  correctgraylist(list);
}


/*
** Mark black 'OLD1' objects when starting a new young collection.
** Gray objects are already in some gray list, and so will be visited in
** the atomic step.
*/
ಸ್ಥಿರ ಶೂನ್ಯ markold (global_State *g, GCObject *from, GCObject *ಗಮ್ಯ) {
  GCObject *p;
  ಗಾಗಿ (p = from; p != ಗಮ್ಯ; p = p->ಮುಂದಿನ) {
    ಆದರೆ (getage(p) == G_OLD1) {
      lua_assert(!iswhite(p));
      setage(p, G_OLD);  /* now they are old */
      ಆದರೆ (isblack(p))
        reallymarkobject(g, p);
    }
  }
}


/*
** Finish a young-generation collection.
*/
ಸ್ಥಿರ ಶೂನ್ಯ finishgencycle (lua_State *L, global_State *g) {
  correctgraylists(g);
  checkSizes(L, g);
  g->gcstate = GCSpropagate;  /* skip restart */
  ಆದರೆ (g->tobefnz != ಖಾಲಿ && !g->gcemergency && luaD_checkminstack(L))
    callallpendingfinalizers(L);
}


/*
** Shifts from a minor collection to major collections. It starts in
** the "sweep all" state to clear all objects, which are mostly black
** in generational mode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ minor2inc (lua_State *L, global_State *g, lu_byte ಬಗೆ) {
  g->GCmajorminor = g->GCmarked;  /* number of live bytes */
  g->gckind = ಬಗೆ;
  g->reallyold = g->old1 = g->survival = ಖಾಲಿ;
  g->finobjrold = g->finobjold1 = g->finobjsur = ಖಾಲಿ;
  entersweep(L);  /* continue as an incremental cycle */
  /* set a debt equal to the step size */
  luaE_setdebt(g, applygcparam(g, STEPSIZE, 100));
}


/*
** Decide whether to shift to major mode. It shifts if the accumulated
** number of added old bytes (counted in 'GCmarked') is larger than
** 'minormajor'% of the number of lived bytes after the last major
** collection. (This number is kept in 'GCmajorminor'.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkminormajor (global_State *g) {
  l_mem ಎಲ್ಲೆ = applygcparam(g, MINORMAJOR, g->GCmajorminor);
  ಆದರೆ (ಎಲ್ಲೆ == 0)
    ಹಿಂತಿರುಗಿ 0;  /* special case: 'minormajor' 0 stops major collections */
  ಹಿಂತಿರುಗಿ (g->GCmarked >= ಎಲ್ಲೆ);
}

/*
** Does a young collection. First, mark 'OLD1' objects. Then does the
** atomic step. Then, check whether to continue in minor mode. If so,
** sweep all lists and advance pointers. Finally, finish the collection.
*/
ಸ್ಥಿರ ಶೂನ್ಯ youngcollection (lua_State *L, global_State *g) {
  l_mem addedold1 = 0;
  l_mem marked = g->GCmarked;  /* preserve 'g->GCmarked' */
  GCObject **psurvival;  /* to point to first non-dead survival object */
  GCObject *dummy;  /* dummy out parameter to 'sweepgen' */
  lua_assert(g->gcstate == GCSpropagate);
  ಆದರೆ (g->firstold1) {  /* are there regular OLD1 objects? */
    markold(g, g->firstold1, g->reallyold);  /* mark them */
    g->firstold1 = ಖಾಲಿ;  /* no more OLD1 objects (for now) */
  }
  markold(g, g->finobj, g->finobjrold);
  markold(g, g->tobefnz, ಖಾಲಿ);

  atomic(L);  /* will lose 'g->marked' */

  /* sweep nursery and get a pointer to its last live element */
  g->gcstate = GCSswpallgc;
  psurvival = sweepgen(L, g, &g->allgc, g->survival, &g->firstold1, &addedold1);
  /* sweep 'survival' */
  sweepgen(L, g, psurvival, g->old1, &g->firstold1, &addedold1);
  g->reallyold = g->old1;
  g->old1 = *psurvival;  /* 'survival' survivals are old now */
  g->survival = g->allgc;  /* all news are survivals */

  /* repeat for 'finobj' lists */
  dummy = ಖಾಲಿ;  /* no 'firstold1' optimization for 'finobj' lists */
  psurvival = sweepgen(L, g, &g->finobj, g->finobjsur, &dummy, &addedold1);
  /* sweep 'survival' */
  sweepgen(L, g, psurvival, g->finobjold1, &dummy, &addedold1);
  g->finobjrold = g->finobjold1;
  g->finobjold1 = *psurvival;  /* 'survival' survivals are old now */
  g->finobjsur = g->finobj;  /* all news are survivals */

  sweepgen(L, g, &g->tobefnz, ಖಾಲಿ, &dummy, &addedold1);

  /* keep total number of added old1 bytes */
  g->GCmarked = marked + addedold1;

  /* decide whether to shift to major mode */
  ಆದರೆ (checkminormajor(g)) {
    minor2inc(L, g, KGC_GENMAJOR);  /* go to major mode */
    g->GCmarked = 0;  /* avoid pause in first major cycle (see 'setpause') */
  }
  ಇಲ್ಲದಿದ್ದರೆ
    finishgencycle(L, g);  /* still in minor mode; finish it */
}


/*
** Clears all gray lists, sweeps objects, and prepare sublists to enter
** generational mode. The sweeps remove dead objects and turn all
** surviving objects to old. Threads go back to 'grayagain'; everything
** else is turned black (not in any gray list).
*/
ಸ್ಥಿರ ಶೂನ್ಯ atomic2gen (lua_State *L, global_State *g) {
  cleargraylists(g);
  /* sweep all elements making them old */
  g->gcstate = GCSswpallgc;
  sweep2old(L, &g->allgc);
  /* everything alive now is old */
  g->reallyold = g->old1 = g->survival = g->allgc;
  g->firstold1 = ಖಾಲಿ;  /* there are no OLD1 objects anywhere */

  /* repeat for 'finobj' lists */
  sweep2old(L, &g->finobj);
  g->finobjrold = g->finobjold1 = g->finobjsur = g->finobj;

  sweep2old(L, &g->tobefnz);

  g->gckind = KGC_GENMINOR;
  g->GCmajorminor = g->GCmarked;  /* "base" for number of bytes */
  g->GCmarked = 0;  /* to count the number of added old1 bytes */
  finishgencycle(L, g);
}


/*
** Set debt for the next minor collection, which will happen when
** total number of bytes grows 'genminormul'% in relation to
** the base, GCmajorminor, which is the number of bytes being used
** after the last major collection.
*/
ಸ್ಥಿರ ಶೂನ್ಯ setminordebt (global_State *g) {
  luaE_setdebt(g, applygcparam(g, MINORMUL, g->GCmajorminor));
}


/*
** Enter generational mode. Must go until the end of an atomic cycle
** to ensure that all objects are correctly marked and weak tables
** are cleared. Then, turn all objects into old and finishes the
** collection.
*/
ಸ್ಥಿರ ಶೂನ್ಯ entergen (lua_State *L, global_State *g) {
  luaC_runtilstate(L, GCSpause, 1);  /* prepare to start a new cycle */
  luaC_runtilstate(L, GCSpropagate, 1);  /* start new cycle */
  atomic(L);  /* propagates all and then do the atomic stuff */
  atomic2gen(L, g);
  setminordebt(g);  /* set debt assuming next cycle will be minor */
}


/*
** Change collector mode to 'newmode'.
*/
ಶೂನ್ಯ luaC_changemode (lua_State *L, ಪೂರ್ಣಾಂಕ newmode) {
  global_State *g = G(L);
  ಆದರೆ (g->gckind == KGC_GENMAJOR)  /* doing major collections? */
    g->gckind = KGC_INC;  /* already incremental but in name */
  ಆದರೆ (newmode != g->gckind) {  /* does it need to change? */
    ಆದರೆ (newmode == KGC_INC)  /* entering incremental mode? */
      minor2inc(L, g, KGC_INC);  /* entering incremental mode */
    ಇಲ್ಲದಿದ್ದರೆ {
      lua_assert(newmode == KGC_GENMINOR);
      entergen(L, g);
    }
  }
}


/*
** Does a full collection in generational mode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ fullgen (lua_State *L, global_State *g) {
  minor2inc(L, g, KGC_INC);
  entergen(L, g);
}


/*
** After an atomic incremental step from a major collection,
** check whether collector could return to minor collections.
** It checks whether the number of bytes 'tobecollected'
** is greater than 'majorminor'% of the number of bytes added
** since the last collection ('addedbytes').
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkmajorminor (lua_State *L, global_State *g) {
  ಆದರೆ (g->gckind == KGC_GENMAJOR) {  /* generational mode? */
    l_mem numbytes = gettotalbytes(g);
    l_mem addedbytes = numbytes - g->GCmajorminor;
    l_mem ಎಲ್ಲೆ = (addedbytes < 0) ? 0
                                   : applygcparam(g, MAJORMINOR, addedbytes);
    l_mem tobecollected = numbytes - g->GCmarked;
    ಆದರೆ (tobecollected > ಎಲ್ಲೆ) {
      atomic2gen(L, g);  /* return to generational mode */
      setminordebt(g);
      ಹಿಂತಿರುಗಿ 1;  /* exit incremental collection */
    }
  }
  g->GCmajorminor = g->GCmarked;  /* prepare for next collection */
  ಹಿಂತಿರುಗಿ 0;  /* stay doing incremental collections */
}

/* }====================================================== */


/*
** {======================================================
** GC control
** =======================================================
*/


/*
** Enter first sweep phase.
** The call to 'sweeptolive' makes the pointer point to an object
** inside the list (instead of to the header), so that the real sweep do
** not need to skip objects created between "now" and the start of the
** real sweep.
*/
ಸ್ಥಿರ ಶೂನ್ಯ entersweep (lua_State *L) {
  global_State *g = G(L);
  g->gcstate = GCSswpallgc;
  lua_assert(g->sweepgc == ಖಾಲಿ);
  g->sweepgc = sweeptolive(L, &g->allgc);
}


/*
** Delete all objects in list 'p' until (but not including) object
** 'limit'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ deletelist (lua_State *L, GCObject *p, GCObject *ಎಲ್ಲೆ) {
  ಆಗ (p != ಎಲ್ಲೆ) {
    GCObject *ಮುಂದಿನ = p->ಮುಂದಿನ;
    freeobj(L, p);
    p = ಮುಂದಿನ;
  }
}


/*
** Call all finalizers of the objects in the given Lua state, and
** then free all objects, except for the main thread.
*/
ಶೂನ್ಯ luaC_freeallobjects (lua_State *L) {
  global_State *g = G(L);
  g->gcstp = GCSTPCLS;  /* no extra finalizers after here */
  luaC_changemode(L, KGC_INC);
  separatetobefnz(g, 1);  /* separate all objects with finalizers */
  lua_assert(g->finobj == ಖಾಲಿ);
  callallpendingfinalizers(L);
  deletelist(L, g->allgc, obj2gco(mainthread(g)));
  lua_assert(g->finobj == ಖಾಲಿ);  /* no new finalizers */
  deletelist(L, g->fixedgc, ಖಾಲಿ);  /* collect fixed objects */
  lua_assert(g->strt.nuse == 0);
}


ಸ್ಥಿರ ಶೂನ್ಯ atomic (lua_State *L) {
  global_State *g = G(L);
  GCObject *origweak, *origall;
  GCObject *grayagain = g->grayagain;  /* save original list */
  g->grayagain = ಖಾಲಿ;
  lua_assert(g->ephemeron == ಖಾಲಿ && g->weak == ಖಾಲಿ);
  lua_assert(!iswhite(mainthread(g)));
  g->gcstate = GCSatomic;
  markobject(g, L);  /* mark running thread */
  /* registry and global metatables may be changed by API */
  markvalue(g, &g->l_registry);
  markmt(g);  /* mark global metatables */
  propagateall(g);  /* empties 'gray' list */
  /* remark occasional upvalues of (maybe) dead threads */
  remarkupvals(g);
  propagateall(g);  /* propagate changes */
  g->ಬೂದು = grayagain;
  propagateall(g);  /* traverse 'grayagain' list */
  convergeephemerons(g);
  /* at this point, all strongly accessible objects are marked. */
  /* Clear values from weak tables, before checking finalizers */
  clearbyvalues(g, g->weak, ಖಾಲಿ);
  clearbyvalues(g, g->allweak, ಖಾಲಿ);
  origweak = g->weak; origall = g->allweak;
  separatetobefnz(g, 0);  /* separate objects to be finalized */
  markbeingfnz(g);  /* mark objects that will be finalized */
  propagateall(g);  /* remark, to propagate 'resurrection' */
  convergeephemerons(g);
  /* at this point, all resurrected objects are marked. */
  /* remove dead objects from weak tables */
  clearbykeys(g, g->ephemeron);  /* clear keys from all ephemeron */
  clearbykeys(g, g->allweak);  /* clear keys from all 'allweak' */
  /* clear values from resurrected weak tables */
  clearbyvalues(g, g->weak, origweak);
  clearbyvalues(g, g->allweak, origall);
  luaS_clearcache(g);
  g->currentwhite = cast_byte(otherwhite(g));  /* flip current white */
  lua_assert(g->ಬೂದು == ಖಾಲಿ);
}


/*
** Do a sweep step. The normal case (not fast) sweeps at most GCSWEEPMAX
** elements. The fast case sweeps the whole list.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sweepstep (lua_State *L, global_State *g,
                       lu_byte nextstate, GCObject **nextlist, ಪೂರ್ಣಾಂಕ fast) {
  ಆದರೆ (g->sweepgc)
    g->sweepgc = sweeplist(L, g->sweepgc, fast ? MAX_LMEM : GCSWEEPMAX);
  ಇಲ್ಲದಿದ್ದರೆ {  /* enter next state */
    g->gcstate = nextstate;
    g->sweepgc = nextlist;
  }
}


/*
** Performs one incremental "step" in an incremental garbage collection.
** For indivisible work, a step goes to the next state. When marking
** (propagating), a step traverses one object. When sweeping, a step
** sweeps GCSWEEPMAX objects, to avoid a big overhead for sweeping
** objects one by one. (Sweeping is inexpensive, no matter the
** object.) When 'fast' is true, 'singlestep' tries to finish a state
** "as fast as possible". In particular, it skips the propagation
** phase and leaves all objects to be traversed by the atomic phase:
** That avoids traversing twice some objects, such as threads and
** weak tables.
*/

#ವ್ಯಾಖ್ಯಾನಿಸು step2pause	-3  /* finished collection; entered pause state */
#ವ್ಯಾಖ್ಯಾನಿಸು atomicstep	-2  /* atomic step */
#ವ್ಯಾಖ್ಯಾನಿಸು step2minor	-1  /* moved to minor collections */


ಸ್ಥಿರ l_mem singlestep (lua_State *L, ಪೂರ್ಣಾಂಕ fast) {
  global_State *g = G(L);
  l_mem stepresult;
  lua_assert(!g->gcstopem);  /* collector is not reentrant */
  g->gcstopem = 1;  /* no emergency collections while collecting */
  ಆಯ್ಕೆ (g->gcstate) {
    ಸ್ಥಿತಿ GCSpause: {
      restartcollection(g);
      g->gcstate = GCSpropagate;
      stepresult = 1;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ GCSpropagate: {
      ಆದರೆ (fast || g->ಬೂದು == ಖಾಲಿ) {
        g->gcstate = GCSenteratomic;  /* finish propagate phase */
        stepresult = 1;
      }
      ಇಲ್ಲದಿದ್ದರೆ
        stepresult = propagatemark(g);  /* traverse one gray object */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ GCSenteratomic: {
      atomic(L);
      ಆದರೆ (checkmajorminor(L, g))
        stepresult = step2minor;
      ಇಲ್ಲದಿದ್ದರೆ {
        entersweep(L);
        stepresult = atomicstep;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ GCSswpallgc: {  /* sweep "regular" objects */
      sweepstep(L, g, GCSswpfinobj, &g->finobj, fast);
      stepresult = GCSWEEPMAX;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ GCSswpfinobj: {  /* sweep objects with finalizers */
      sweepstep(L, g, GCSswptobefnz, &g->tobefnz, fast);
      stepresult = GCSWEEPMAX;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ GCSswptobefnz: {  /* sweep objects to be finalized */
      sweepstep(L, g, GCSswpend, ಖಾಲಿ, fast);
      stepresult = GCSWEEPMAX;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ GCSswpend: {  /* finish sweeps */
      checkSizes(L, g);
      g->gcstate = GCScallfin;
      stepresult = GCSWEEPMAX;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ GCScallfin: {  /* call finalizers */
      ಆದರೆ (g->tobefnz && !g->gcemergency && luaD_checkminstack(L)) {
        g->gcstopem = 0;  /* ok collections during finalizers */
        GCTM(L);  /* call one finalizer */
        stepresult = CWUFIN;
      }
      ಇಲ್ಲದಿದ್ದರೆ {  /* no more finalizers or emergency mode or not enough stack
                 to run finalizers */
        g->gcstate = GCSpause;  /* finish collection */
        stepresult = step2pause;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0); ಹಿಂತಿರುಗಿ 0;
  }
  g->gcstopem = 0;
  ಹಿಂತಿರುಗಿ stepresult;
}


/*
** Advances the garbage collector until it reaches the given state.
** (The option 'fast' is only for testing; in normal code, 'fast'
** here is always true.)
*/
ಶೂನ್ಯ luaC_runtilstate (lua_State *L, ಪೂರ್ಣಾಂಕ state, ಪೂರ್ಣಾಂಕ fast) {
  global_State *g = G(L);
  lua_assert(g->gckind == KGC_INC);
  ಆಗ (state != g->gcstate)
    singlestep(L, fast);
}



/*
** Performs a basic incremental step. The step size is
** converted from bytes to "units of work"; then the function loops
** running single steps until adding that many units of work or
** finishing a cycle (pause state). Finally, it sets the debt that
** controls when next step will be performed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ incstep (lua_State *L, global_State *g) {
  l_mem stepsize = applygcparam(g, STEPSIZE, 100);
  l_mem work2do = applygcparam(g, STEPMUL, stepsize / cast_int(ಗಾತ್ರ(ಶೂನ್ಯ*)));
  l_mem stres;
  ಪೂರ್ಣಾಂಕ fast = (work2do == 0);  /* special case: do a full collection */
  ಮಾಡು {  /* repeat until enough work */
    stres = singlestep(L, fast);  /* perform one single step */
    ಆದರೆ (stres == step2minor)  /* returned to minor collections? */
      ಹಿಂತಿರುಗಿ;  /* nothing else to be done here */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (stres == step2pause || (stres == atomicstep && !fast))
      ನಿಲ್ಲಿಸು;  /* end of cycle or atomic */
    ಇಲ್ಲದಿದ್ದರೆ
      work2do -= stres;
  } ಆಗ (fast || work2do > 0);
  ಆದರೆ (g->gcstate == GCSpause)
    setpause(g);  /* pause until next cycle */
  ಇಲ್ಲದಿದ್ದರೆ
    luaE_setdebt(g, stepsize);
}


#ಆದರೆ !defined(luai_tracegc)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_tracegc(L,f)		((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Performs a basic GC step if collector is running. (If collector was
** stopped by the user, set a reasonable debt to avoid it being called
** at every single check.)
*/
ಶೂನ್ಯ lua_C_ಹೆಜ್ಜೆ (lua_State *L) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  ಆದರೆ (!gcrunning(g)) {  /* not running? */
    ಆದರೆ (g->gcstp & GCSTPUSR)  /* stopped by the user? */
      luaE_setdebt(g, 20000);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    luai_tracegc(L, 1);  /* for internal debugging */
    ಆಯ್ಕೆ (g->gckind) {
      ಸ್ಥಿತಿ KGC_INC: ಸ್ಥಿತಿ KGC_GENMAJOR:
        incstep(L, g);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ KGC_GENMINOR:
        youngcollection(L, g);
        setminordebt(g);
        ನಿಲ್ಲಿಸು;
    }
    luai_tracegc(L, 0);  /* for internal debugging */
  }
}


/*
** Perform a full collection in incremental mode.
** Before running the collection, check 'keepinvariant'; if it is true,
** there may be some objects marked as black, so the collector has
** to sweep all objects to turn them back to white (as white has not
** changed, nothing will be collected).
*/
ಸ್ಥಿರ ಶೂನ್ಯ fullinc (lua_State *L, global_State *g) {
  ಆದರೆ (keepinvariant(g))  /* black objects? */
    entersweep(L); /* sweep everything to turn them back to white */
  /* finish any pending sweep phase to start a new cycle */
  luaC_runtilstate(L, GCSpause, 1);
  luaC_runtilstate(L, GCScallfin, 1);  /* run up to finalizers */
  luaC_runtilstate(L, GCSpause, 1);  /* finish collection */
  setpause(g);
}


/*
** Performs a full GC cycle; if 'isemergency', set a flag to avoid
** some operations which could change the interpreter state in some
** unexpected ways (running finalizers and shrinking some structures).
*/
ಶೂನ್ಯ luaC_fullgc (lua_State *L, ಪೂರ್ಣಾಂಕ isemergency) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  g->gcemergency = cast_byte(isemergency);  /* set flag */
  ಆಯ್ಕೆ (g->gckind) {
    ಸ್ಥಿತಿ KGC_GENMINOR: fullgen(L, g); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ KGC_INC: fullinc(L, g); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ KGC_GENMAJOR:
      g->gckind = KGC_INC;
      fullinc(L, g);
      g->gckind = KGC_GENMAJOR;
      ನಿಲ್ಲಿಸು;
  }
  g->gcemergency = 0;
}

/* }====================================================== */


