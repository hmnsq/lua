/*
** $Id: lgc.c $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lgc_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>


#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"


/*
** Maximum number of elements to sweep in each single step.
** (Large enough to dissipate fixed overheads but small enough
** to allow small steps for the collector.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GCSWEEPMAX	20


/*
** Cost (in work units) of running one finalizer.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CWUFIN	10


/* mask with all color bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ maskcolors	(bitmask(BLACKBIT) | WHITEBITS)

/* mask with all GC bits */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ maskgcbits      (maskcolors | AGEBITS)


/* macro to erase all color bits then set only the current white bit */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ makewhite(g,x)	\
  (x->marked = cast_byte((x->marked & ~maskcolors) | luaC_white(g)))

/* make an object gray (neither white nor black) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ set2gray(x)	resetbits(x->marked, maskcolors)


/* make an object black (coming from any color) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ set2black(x)  \
  (x->marked = cast_byte((x->marked & ~WHITEBITS) | bitmask(BLACKBIT)))


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ valiswhite(x)   (iscollectable(x) && iswhite(gcvalue(x)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ keyiswhite(n)   (keyiscollectable(n) && iswhite(gckey(n)))


/*
** Protected access to objects in values
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gcvalueN(o)     (iscollectable(o) ? gcvalue(o) : ਨਿਰਾ)


/*
** Access to collectable objects in array part of tables
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gcvalarr(t,i)  \
	((*get_Arr_ਟੈਗ(t,i) & ਦੋਆਧਾਰੀ_ਅੰਕ_ISCOLLECTABLE) ? getArrVal(t,i)->gc : ਨਿਰਾ)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ markvalue(g,o) { checkliveness(mainthread(g),o); \
  ਜੇ (valiswhite(o)) reallymarkobject(g,gcvalue(o)); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ markkey(g, n)	{ ਜੇ keyiswhite(n) reallymarkobject(g,gckey(n)); }

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ markobject(g,t)	{ ਜੇ (iswhite(t)) reallymarkobject(g, obj2gco(t)); }

/*
** mark an object that can be NULL (either because it is really optional,
** or it was stripped as debug info, or inside an uncompleted structure)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ markobjectN(g,t)	{ ਜੇ (t) markobject(g,t); }


ਸਥਿਰ ਖਾਲੀ reallymarkobject (global_State *g, GCObject *o);
ਸਥਿਰ ਖਾਲੀ atomic (lua_State *L);
ਸਥਿਰ ਖਾਲੀ entersweep (lua_State *L);


/*
** {======================================================
** Generic functions
** =======================================================
*/


/*
** one after last element in a hash array
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ gnodelast(h)	gnode(h, cast_sizet(sizenode(h)))


ਸਥਿਰ l_mem objsize (GCObject *o) {
  lu_mem res;
  ਚੋਣ (o->tt) {
    ਹਾਲਤ LUA_VTABLE: {
      res = lua_H_ਮਾਪ(gco2t(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      res = ਮਾਪ_Lclosure(cl->nupvalues);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      res = ਮਾਪ_Cclosure(cl->nupvalues);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      res = sizeudata(u->nuvalue, u->len);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VPROTO: {
      res = luaF_protosize(gco2p(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VTHREAD: {
      res = luaE_threadsize(gco2th(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      res = sizestrshr(cast_uint(ts->shrlen));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      res = luaS_sizelngstr(ts->u.lnglen, ts->shrlen);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VUPVAL: {
      res = ਆਕਾਰ(ਉੱਪਰ_Val);
      ਰੋਕੋ;
    }
    ਮੂਲ: res = 0; lua_assert(0);
  }
  ਵਾਪਸ cast(l_mem, res);
}


ਸਥਿਰ GCObject **getgclist (GCObject *o) {
  ਚੋਣ (o->tt) {
    ਹਾਲਤ LUA_VTABLE: ਵਾਪਸ &gco2t(o)->gclist;
    ਹਾਲਤ LUA_VLCL: ਵਾਪਸ &gco2lcl(o)->gclist;
    ਹਾਲਤ LUA_VCCL: ਵਾਪਸ &gco2ccl(o)->gclist;
    ਹਾਲਤ LUA_VTHREAD: ਵਾਪਸ &gco2th(o)->gclist;
    ਹਾਲਤ LUA_VPROTO: ਵਾਪਸ &gco2p(o)->gclist;
    ਹਾਲਤ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      lua_assert(u->nuvalue > 0);
      ਵਾਪਸ &u->gclist;
    }
    ਮੂਲ: lua_assert(0); ਵਾਪਸ 0;
  }
}


/*
** Link a collectable object 'o' with a known type into the list 'p'.
** (Must be a macro to access the 'gclist' field in different types.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ linkgclist(o,p)	linkgclist_(obj2gco(o), &(o)->gclist, &(p))

ਸਥਿਰ ਖਾਲੀ linkgclist_ (GCObject *o, GCObject **pnext, GCObject **list) {
  lua_assert(!isgray(o));  /* cannot be in a gray list */
  *pnext = *list;
  *list = o;
  set2gray(o);  /* now it is */
}


/*
** Link a generic collectable object 'o' into the list 'p'.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ linkobjgclist(o,p) linkgclist_(obj2gco(o), getgclist(o), &(p))



/*
** Clear keys for empty entries in tables. If entry is empty, mark its
** entry as dead. This allows the collection of the key, but keeps its
** entry in the table: its removal could break a chain and could break
** a table traversal.  Other places never manipulate dead keys, because
** its associated empty value is enough to signal that the entry is
** logically empty.
*/
ਸਥਿਰ ਖਾਲੀ clearkey (ਗੰਢ_2 *n) {
  lua_assert(isempty(gval(n)));
  ਜੇ (keyiscollectable(n))
    setdeadkey(n);  /* unused key; remove it */
}


/*
** tells whether a key or value can be cleared from a weak
** table. Non-collectable objects are never removed from weak
** tables. Strings behave as 'values', so are never removed too. for
** other objects: if really collected, cannot keep them; for objects
** being finalized, keep them in keys, but not in values
*/
ਸਥਿਰ ਪੂਰਨ iscleared (global_State *g, ਅਚਲ GCObject *o) {
  ਜੇ (o == ਨਿਰਾ) ਵਾਪਸ 0;  /* non-collectable value */
  ਨਹੀਂਤਾਂ ਜੇ (novariant(o->tt) == LUA_TSTRING) {
    markobject(g, o);  /* strings are 'values', so are never weak */
    ਵਾਪਸ 0;
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ iswhite(o);
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
ਖਾਲੀ luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v) {
  global_State *g = G(L);
  lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
  ਜੇ (keepinvariant(g)) {  /* must keep invariant? */
    reallymarkobject(g, v);  /* restore invariant */
    ਜੇ (isold(o)) {
      lua_assert(!isold(v));  /* white object could not be old */
      setage(v, G_OLD0);  /* restore generational invariant */
    }
  }
  ਨਹੀਂਤਾਂ {  /* sweep phase */
    lua_assert(issweepphase(g));
    ਜੇ (g->gckind != KGC_GENMINOR)  /* incremental mode? */
      makewhite(g, o);  /* mark 'o' as white to avoid other barriers */
  }
}


/*
** barrier that moves collector backward, that is, mark the black object
** pointing to a white object as gray again.
*/
ਖਾਲੀ luaC_barrierback_ (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(isblack(o) && !isdead(g, o));
  lua_assert((g->gckind != KGC_GENMINOR)
          || (isold(o) && getage(o) != G_TOUCHED1));
  ਜੇ (getage(o) == G_TOUCHED2)  /* already in gray list? */
    set2gray(o);  /* make it gray to become touched1 */
  ਨਹੀਂਤਾਂ  /* link it in 'grayagain' and paint it gray */
    linkobjgclist(o, g->grayagain);
  ਜੇ (isold(o))  /* generational mode? */
    setage(o, G_TOUCHED1);  /* touched in current cycle */
}


ਖਾਲੀ luaC_fix (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(g->allgc == o);  /* object must be 1st in 'allgc' list! */
  set2gray(o);  /* they will be gray forever */
  setage(o, G_OLD);  /* and old forever */
  g->allgc = o->ਅਗਲਾ;  /* remove object from 'allgc' list */
  o->ਅਗਲਾ = g->fixedgc;  /* link it to 'fixedgc' list */
  g->fixedgc = o;
}


/*
** create a new collectable object (with given type, size, and offset)
** and link it to 'allgc' list.
*/
GCObject *luaC_newobjdt (lua_State *L, lu_byte tt, size_t sz, size_t ਵਿੱਥ) {
  global_State *g = G(L);
  ਅੱਖਰ *p = cast_charp(luaM_newobject(L, novariant(tt), sz));
  GCObject *o = cast(GCObject *, p + ਵਿੱਥ);
  o->marked = luaC_white(g);
  o->tt = tt;
  o->ਅਗਲਾ = g->allgc;
  g->allgc = o;
  ਵਾਪਸ o;
}


/*
** create a new collectable object with no offset.
*/
GCObject *luaC_newobj (lua_State *L, lu_byte tt, size_t sz) {
  ਵਾਪਸ luaC_newobjdt(L, tt, sz, 0);
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
ਸਥਿਰ ਖਾਲੀ reallymarkobject (global_State *g, GCObject *o) {
  g->GCmarked += objsize(o);
  ਚੋਣ (o->tt) {
    ਹਾਲਤ LUA_VSHRSTR:
    ਹਾਲਤ LUA_VLNGSTR: {
      set2black(o);  /* nothing to visit */
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VUPVAL: {
      ਉੱਪਰ_Val *uv = gco2upv(o);
      ਜੇ (upisopen(uv))
        set2gray(uv);  /* open upvalues are kept gray */
      ਨਹੀਂਤਾਂ
        set2black(uv);  /* closed upvalues are visited here */
      markvalue(g, uv->v.p);  /* mark its content */
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      ਜੇ (u->nuvalue == 0) {  /* no user values? */
        markobjectN(g, u->metatable);  /* mark its metatable */
        set2black(u);  /* nothing else to mark */
        ਰੋਕੋ;
      }
      /* else... */
    }  /* FALLTHROUGH */
    ਹਾਲਤ LUA_VLCL: ਹਾਲਤ LUA_VCCL: ਹਾਲਤ LUA_VTABLE:
    ਹਾਲਤ LUA_VTHREAD: ਹਾਲਤ LUA_VPROTO: {
      linkobjgclist(o, g->ਸਲੇਟੀ);  /* to be visited later */
      ਰੋਕੋ;
    }
    ਮੂਲ: lua_assert(0); ਰੋਕੋ;
  }
}


/*
** mark metamethods for basic types
*/
ਸਥਿਰ ਖਾਲੀ markmt (global_State *g) {
  ਪੂਰਨ i;
  ਲਈ (i=0; i < LUA_NUMTYPES; i++)
    markobjectN(g, g->mt[i]);
}


/*
** mark all objects in list of being-finalized
*/
ਸਥਿਰ ਖਾਲੀ markbeingfnz (global_State *g) {
  GCObject *o;
  ਲਈ (o = g->tobefnz; o != ਨਿਰਾ; o = o->ਅਗਲਾ)
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
ਸਥਿਰ ਖਾਲੀ remarkupvals (global_State *g) {
  lua_State *thread;
  lua_State **p = &g->twups;
  ਜਦੋਂ ((thread = *p) != ਨਿਰਾ) {
    ਜੇ (!iswhite(thread) && thread->openupval != ਨਿਰਾ)
      p = &thread->twups;  /* keep marked thread with upvalues in the list */
    ਨਹੀਂਤਾਂ {  /* thread is not marked or without upvalues */
      ਉੱਪਰ_Val *uv;
      lua_assert(!isold(thread) || thread->openupval == ਨਿਰਾ);
      *p = thread->twups;  /* remove thread from the list */
      thread->twups = thread;  /* mark that it is out of list */
      ਲਈ (uv = thread->openupval; uv != ਨਿਰਾ; uv = uv->u.open.ਅਗਲਾ) {
        lua_assert(getage(uv) <= getage(thread));
        ਜੇ (!iswhite(uv)) {  /* upvalue already visited? */
          lua_assert(upisopen(uv) && isgray(uv));
          markvalue(g, uv->v.p);  /* mark its value */
        }
      }
    }
  }
}


ਸਥਿਰ ਖਾਲੀ cleargraylists (global_State *g) {
  g->ਸਲੇਟੀ = g->grayagain = ਨਿਰਾ;
  g->weak = g->allweak = g->ephemeron = ਨਿਰਾ;
}


/*
** mark root set and reset all gray lists, to start a new collection.
** 'GCmarked' is initialized to count the total number of live bytes
** during a cycle.
*/
ਸਥਿਰ ਖਾਲੀ restartcollection (global_State *g) {
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
ਸਥਿਰ ਖਾਲੀ genlink (global_State *g, GCObject *o) {
  lua_assert(isblack(o));
  ਜੇ (getage(o) == G_TOUCHED1) {  /* touched in this cycle? */
    linkobjgclist(o, g->grayagain);  /* link it back in 'grayagain' */
  }  /* everything else do not need to be linked back */
  ਨਹੀਂਤਾਂ ਜੇ (getage(o) == G_TOUCHED2)
    setage(o, G_OLD);  /* advance age */
}


/*
** Traverse a table with weak values and link it to proper list. During
** propagate phase, keep it in 'grayagain' list, to be revisited in the
** atomic phase. In the atomic phase, if table has any white value,
** put it in 'weak' list, to be cleared; otherwise, call 'genlink'
** to check table age in generational mode.
*/
ਸਥਿਰ ਖਾਲੀ traverseweakvalue (global_State *g, ਸਾਰਣੀ *h) {
  ਗੰਢ_2 *n, *ਹੱਦਬੰਦੀ = gnodelast(h);
  /* if there is array part, assume it may have white values (it is not
     worth traversing it now just to check) */
  ਪੂਰਨ hasclears = (h->asize > 0);
  ਲਈ (n = gnode(h, 0); n < ਹੱਦਬੰਦੀ; n++) {  /* traverse hash part */
    ਜੇ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ਨਹੀਂਤਾਂ {
      lua_assert(!keyisnil(n));
      markkey(g, n);
      ਜੇ (!hasclears && iscleared(g, gcvalueN(gval(n))))  /* a white value? */
        hasclears = 1;  /* table will have to be cleared */
    }
  }
  ਜੇ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  ਨਹੀਂਤਾਂ ਜੇ (hasclears)
      linkgclist(h, g->weak);  /* has to be cleared later */
  ਨਹੀਂਤਾਂ
    genlink(g, obj2gco(h));
}


/*
** Traverse the array part of a table.
*/
ਸਥਿਰ ਪੂਰਨ traversearray (global_State *g, ਸਾਰਣੀ *h) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ asize = h->asize;
  ਪੂਰਨ marked = 0;  /* true if some object is marked in this traversal */
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ਲਈ (i = 0; i < asize; i++) {
    GCObject *o = gcvalarr(h, i);
    ਜੇ (o != ਨਿਰਾ && iswhite(o)) {
      marked = 1;
      reallymarkobject(g, o);
    }
  }
  ਵਾਪਸ marked;
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
ਸਥਿਰ ਪੂਰਨ traverseephemeron (global_State *g, ਸਾਰਣੀ *h, ਪੂਰਨ inv) {
  ਪੂਰਨ hasclears = 0;  /* true if table has white keys */
  ਪੂਰਨ hasww = 0;  /* true if table has entry "white-key -> white-value" */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nsize = sizenode(h);
  ਪੂਰਨ marked = traversearray(g, h);  /* traverse array part */
  /* traverse hash part; if 'inv', traverse descending
     (see 'convergeephemerons') */
  ਲਈ (i = 0; i < nsize; i++) {
    ਗੰਢ_2 *n = inv ? gnode(h, nsize - 1 - i) : gnode(h, i);
    ਜੇ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ਨਹੀਂਤਾਂ ਜੇ (iscleared(g, gckeyN(n))) {  /* key is not marked (yet)? */
      hasclears = 1;  /* table must be cleared */
      ਜੇ (valiswhite(gval(n)))  /* value not marked yet? */
        hasww = 1;  /* white-white entry */
    }
    ਨਹੀਂਤਾਂ ਜੇ (valiswhite(gval(n))) {  /* value not marked yet? */
      marked = 1;
      reallymarkobject(g, gcvalue(gval(n)));  /* mark it now */
    }
  }
  /* link table into proper list */
  ਜੇ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  ਨਹੀਂਤਾਂ ਜੇ (hasww)  /* table has white->white entries? */
    linkgclist(h, g->ephemeron);  /* have to propagate again */
  ਨਹੀਂਤਾਂ ਜੇ (hasclears)  /* table has white keys? */
    linkgclist(h, g->allweak);  /* may have to clean white keys */
  ਨਹੀਂਤਾਂ
    genlink(g, obj2gco(h));  /* check whether collector still needs to see it */
  ਵਾਪਸ marked;
}


ਸਥਿਰ ਖਾਲੀ traversestrongtable (global_State *g, ਸਾਰਣੀ *h) {
  ਗੰਢ_2 *n, *ਹੱਦਬੰਦੀ = gnodelast(h);
  traversearray(g, h);
  ਲਈ (n = gnode(h, 0); n < ਹੱਦਬੰਦੀ; n++) {  /* traverse hash part */
    ਜੇ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ਨਹੀਂਤਾਂ {
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
ਸਥਿਰ ਪੂਰਨ getmode (global_State *g, ਸਾਰਣੀ *h) {
  ਅਚਲ TValue *ਬਹੁਲਕ = gfasttm(g, h->metatable, TM_ਬਹੁਲਕ);
  ਜੇ (ਬਹੁਲਕ == ਨਿਰਾ || !ttisstring(ਬਹੁਲਕ))
    ਵਾਪਸ 0;  /* ignore non-string modes */
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *smode = getstr(tsvalue(ਬਹੁਲਕ));
    ਅਚਲ ਅੱਖਰ *weakkey = strchr(smode, 'k');
    ਅਚਲ ਅੱਖਰ *weakvalue = strchr(smode, 'v');
    ਵਾਪਸ ((weakkey != ਨਿਰਾ) << 1) | (weakvalue != ਨਿਰਾ);
  }
}


ਸਥਿਰ l_mem traversetable (global_State *g, ਸਾਰਣੀ *h) {
  markobjectN(g, h->metatable);
  ਚੋਣ (getmode(g, h)) {
    ਹਾਲਤ 0:  /* not weak */
      traversestrongtable(g, h);
      ਰੋਕੋ;
    ਹਾਲਤ 1:  /* weak values */
      traverseweakvalue(g, h);
      ਰੋਕੋ;
    ਹਾਲਤ 2:  /* weak keys */
      traverseephemeron(g, h, 0);
      ਰੋਕੋ;
    ਹਾਲਤ 3:  /* all weak; nothing to traverse */
      ਜੇ (g->gcstate == GCSpropagate)
        linkgclist(h, g->grayagain);  /* must visit again its metatable */
      ਨਹੀਂਤਾਂ
        linkgclist(h, g->allweak);  /* must clear collected entries */
      ਰੋਕੋ;
  }
  ਵਾਪਸ cast(l_mem, 1 + 2*sizenode(h) + h->asize);
}


ਸਥਿਰ l_mem traverseudata (global_State *g, Udata *u) {
  ਪੂਰਨ i;
  markobjectN(g, u->metatable);  /* mark its metatable */
  ਲਈ (i = 0; i < u->nuvalue; i++)
    markvalue(g, &u->uv[i].uv);
  genlink(g, obj2gco(u));
  ਵਾਪਸ 1 + u->nuvalue;
}


/*
** Traverse a prototype. (While a prototype is being build, its
** arrays can be larger than needed; the extra slots are filled with
** NULL, so the use of 'markobjectN')
*/
ਸਥਿਰ l_mem traverseproto (global_State *g, Proto *f) {
  ਪੂਰਨ i;
  markobjectN(g, f->ਸਰੋਤ);
  ਲਈ (i = 0; i < f->sizek; i++)  /* mark literals */
    markvalue(g, &f->k[i]);
  ਲਈ (i = 0; i < f->sizeupvalues; i++)  /* mark upvalue names */
    markobjectN(g, f->upvalues[i].name);
  ਲਈ (i = 0; i < f->sizep; i++)  /* mark nested protos */
    markobjectN(g, f->p[i]);
  ਲਈ (i = 0; i < f->sizelocvars; i++)  /* mark local-variable names */
    markobjectN(g, f->locvars[i].varname);
  ਵਾਪਸ 1 + f->sizek + f->sizeupvalues + f->sizep + f->sizelocvars;
}


ਸਥਿਰ l_mem traverseCclosure (global_State *g, CClosure *cl) {
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < cl->nupvalues; i++)  /* mark its upvalues */
    markvalue(g, &cl->upvalue[i]);
  ਵਾਪਸ 1 + cl->nupvalues;
}

/*
** Traverse a Lua closure, marking its prototype and its upvalues.
** (Both can be NULL while closure is being created.)
*/
ਸਥਿਰ l_mem traverseLclosure (global_State *g, LClosure *cl) {
  ਪੂਰਨ i;
  markobjectN(g, cl->p);  /* mark its prototype */
  ਲਈ (i = 0; i < cl->nupvalues; i++) {  /* visit its upvalues */
    ਉੱਪਰ_Val *uv = cl->upvals[i];
    markobjectN(g, uv);  /* mark upvalue */
  }
  ਵਾਪਸ 1 + cl->nupvalues;
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
ਸਥਿਰ l_mem traversethread (global_State *g, lua_State *th) {
  ਉੱਪਰ_Val *uv;
  StkId o = th->ਢੇਰ.p;
  ਜੇ (isold(th) || g->gcstate == GCSpropagate)
    linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
  ਜੇ (o == ਨਿਰਾ)
    ਵਾਪਸ 0;  /* stack not completely built yet */
  lua_assert(g->gcstate == GCSatomic ||
             th->openupval == ਨਿਰਾ || isintwups(th));
  ਲਈ (; o < th->ਸਿਖਰ.p; o++)  /* mark live elements in the stack */
    markvalue(g, s2v(o));
  ਲਈ (uv = th->openupval; uv != ਨਿਰਾ; uv = uv->u.open.ਅਗਲਾ)
    markobject(g, uv);  /* open upvalues cannot be collected */
  ਜੇ (g->gcstate == GCSatomic) {  /* final traversal? */
    ਜੇ (!g->gcemergency)
      luaD_shrinkstack(th); /* do not change stack in emergency cycle */
    ਲਈ (o = th->ਸਿਖਰ.p; o < th->ਢੇਰ_ਆਖਰੀ.p + EXTRA_ਢੇਰ; o++)
      setnilvalue2s(o);  /* clear dead stack slice */
    /* 'remarkupvals' may have removed thread from 'twups' list */
    ਜੇ (!isintwups(th) && th->openupval != ਨਿਰਾ) {
      th->twups = g->twups;  /* link it back to the list */
      g->twups = th;
    }
  }
  ਵਾਪਸ 1 + (th->ਸਿਖਰ.p - th->ਢੇਰ.p);
}


/*
** traverse one gray object, turning it to black. Return an estimate
** of the number of slots traversed.
*/
ਸਥਿਰ l_mem propagatemark (global_State *g) {
  GCObject *o = g->ਸਲੇਟੀ;
  nw2black(o);
  g->ਸਲੇਟੀ = *getgclist(o);  /* remove from 'gray' list */
  ਚੋਣ (o->tt) {
    ਹਾਲਤ LUA_VTABLE: ਵਾਪਸ traversetable(g, gco2t(o));
    ਹਾਲਤ LUA_VUSERDATA: ਵਾਪਸ traverseudata(g, gco2u(o));
    ਹਾਲਤ LUA_VLCL: ਵਾਪਸ traverseLclosure(g, gco2lcl(o));
    ਹਾਲਤ LUA_VCCL: ਵਾਪਸ traverseCclosure(g, gco2ccl(o));
    ਹਾਲਤ LUA_VPROTO: ਵਾਪਸ traverseproto(g, gco2p(o));
    ਹਾਲਤ LUA_VTHREAD: ਵਾਪਸ traversethread(g, gco2th(o));
    ਮੂਲ: lua_assert(0); ਵਾਪਸ 0;
  }
}


ਸਥਿਰ ਖਾਲੀ propagateall (global_State *g) {
  ਜਦੋਂ (g->ਸਲੇਟੀ)
    propagatemark(g);
}


/*
** Traverse all ephemeron tables propagating marks from keys to values.
** Repeat until it converges, that is, nothing new is marked. 'dir'
** inverts the direction of the traversals, trying to speed up
** convergence on chains in the same table.
*/
ਸਥਿਰ ਖਾਲੀ convergeephemerons (global_State *g) {
  ਪੂਰਨ ਬਦਲਿਆ;
  ਪੂਰਨ dir = 0;
  ਕਰੋ {
    GCObject *w;
    GCObject *ਅਗਲਾ = g->ephemeron;  /* get ephemeron list */
    g->ephemeron = ਨਿਰਾ;  /* tables may return to this list when traversed */
    ਬਦਲਿਆ = 0;
    ਜਦੋਂ ((w = ਅਗਲਾ) != ਨਿਰਾ) {  /* for each ephemeron table */
      ਸਾਰਣੀ *h = gco2t(w);
      ਅਗਲਾ = h->gclist;  /* list is rebuilt during loop */
      nw2black(h);  /* out of the list (for now) */
      ਜੇ (traverseephemeron(g, h, dir)) {  /* marked some value? */
        propagateall(g);  /* propagate changes */
        ਬਦਲਿਆ = 1;  /* will have to revisit all ephemeron tables */
      }
    }
    dir = !dir;  /* invert direction next time */
  } ਜਦੋਂ (ਬਦਲਿਆ);  /* repeat until no more changes */
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
ਸਥਿਰ ਖਾਲੀ clearbykeys (global_State *g, GCObject *l) {
  ਲਈ (; l; l = gco2t(l)->gclist) {
    ਸਾਰਣੀ *h = gco2t(l);
    ਗੰਢ_2 *ਹੱਦਬੰਦੀ = gnodelast(h);
    ਗੰਢ_2 *n;
    ਲਈ (n = gnode(h, 0); n < ਹੱਦਬੰਦੀ; n++) {
      ਜੇ (iscleared(g, gckeyN(n)))  /* unmarked key? */
        setempty(gval(n));  /* remove entry */
      ਜੇ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


/*
** clear entries with unmarked values from all weaktables in list 'l' up
** to element 'f'
*/
ਸਥਿਰ ਖਾਲੀ clearbyvalues (global_State *g, GCObject *l, GCObject *f) {
  ਲਈ (; l != f; l = gco2t(l)->gclist) {
    ਸਾਰਣੀ *h = gco2t(l);
    ਗੰਢ_2 *n, *ਹੱਦਬੰਦੀ = gnodelast(h);
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ asize = h->asize;
    ਲਈ (i = 0; i < asize; i++) {
      GCObject *o = gcvalarr(h, i);
      ਜੇ (iscleared(g, o))  /* value was collected? */
        *get_Arr_ਟੈਗ(h, i) = LUA_VEMPTY;  /* remove entry */
    }
    ਲਈ (n = gnode(h, 0); n < ਹੱਦਬੰਦੀ; n++) {
      ਜੇ (iscleared(g, gcvalueN(gval(n))))  /* unmarked value? */
        setempty(gval(n));  /* remove entry */
      ਜੇ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


ਸਥਿਰ ਖਾਲੀ freeupval (lua_State *L, ਉੱਪਰ_Val *uv) {
  ਜੇ (upisopen(uv))
    luaF_unlinkupval(uv);
  luaM_free(L, uv);
}


ਸਥਿਰ ਖਾਲੀ freeobj (lua_State *L, GCObject *o) {
  assert_ਕੋਡ(l_mem newmem = gettotalbytes(G(L)) - objsize(o));
  ਚੋਣ (o->tt) {
    ਹਾਲਤ LUA_VPROTO:
      luaF_freeproto(L, gco2p(o));
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VUPVAL:
      freeupval(L, gco2upv(o));
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      luaM_freemem(L, cl, ਮਾਪ_Lclosure(cl->nupvalues));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      luaM_freemem(L, cl, ਮਾਪ_Cclosure(cl->nupvalues));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VTABLE:
      luaH_free(L, gco2t(o));
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VTHREAD:
      luaE_freethread(L, gco2th(o));
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      luaM_freemem(L, o, sizeudata(u->nuvalue, u->len));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      luaS_remove(L, ts);  /* remove it from hash table */
      luaM_freemem(L, ts, sizestrshr(cast_uint(ts->shrlen)));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      ਜੇ (ts->shrlen == LSTRMEM)  /* must free external string? */
        (*ts->falloc)(ts->ud, ts->contents, ts->u.lnglen + 1, 0);
      luaM_freemem(L, ts, luaS_sizelngstr(ts->u.lnglen, ts->shrlen));
      ਰੋਕੋ;
    }
    ਮੂਲ: lua_assert(0);
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
ਸਥਿਰ GCObject **sweeplist (lua_State *L, GCObject **p, l_mem countin) {
  global_State *g = G(L);
  ਪੂਰਨ ow = otherwhite(g);
  ਪੂਰਨ white = luaC_white(g);  /* current white */
  ਜਦੋਂ (*p != ਨਿਰਾ && countin-- > 0) {
    GCObject *curr = *p;
    ਪੂਰਨ marked = curr->marked;
    ਜੇ (isdeadm(ow, marked)) {  /* is 'curr' dead? */
      *p = curr->ਅਗਲਾ;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ਨਹੀਂਤਾਂ {  /* change mark to 'white' and age to 'new' */
      curr->marked = cast_byte((marked & ~maskgcbits) | white | G_NEW);
      p = &curr->ਅਗਲਾ;  /* go to next element */
    }
  }
  ਵਾਪਸ (*p == ਨਿਰਾ) ? ਨਿਰਾ : p;
}


/*
** sweep a list until a live object (or end of list)
*/
ਸਥਿਰ GCObject **sweeptolive (lua_State *L, GCObject **p) {
  GCObject **old = p;
  ਕਰੋ {
    p = sweeplist(L, p, 1);
  } ਜਦੋਂ (p == old);
  ਵਾਪਸ p;
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
ਸਥਿਰ ਖਾਲੀ checkSizes (lua_State *L, global_State *g) {
  ਜੇ (!g->gcemergency) {
    ਜੇ (g->strt.nuse < g->strt.ਮਾਪ / 4)  /* string table too big? */
      luaS_resize(L, g->strt.ਮਾਪ / 2);
  }
}


/*
** Get the next udata to be finalized from the 'tobefnz' list, and
** link it back into the 'allgc' list.
*/
ਸਥਿਰ GCObject *udata2finalize (global_State *g) {
  GCObject *o = g->tobefnz;  /* get first element */
  lua_assert(tofinalize(o));
  g->tobefnz = o->ਅਗਲਾ;  /* remove it from 'tobefnz' list */
  o->ਅਗਲਾ = g->allgc;  /* return it to 'allgc' list */
  g->allgc = o;
  resetbit(o->marked, FINALIZEDBIT);  /* object is "normal" again */
  ਜੇ (issweepphase(g))
    makewhite(g, o);  /* "sweep" object */
  ਨਹੀਂਤਾਂ ਜੇ (getage(o) == G_OLD1)
    g->firstold1 = o;  /* it is the first OLD1 object in the list */
  ਵਾਪਸ o;
}


ਸਥਿਰ ਖਾਲੀ dothecall (lua_State *L, ਖਾਲੀ *ud) {
  ਅਣਵਰਤਿਆ(ud);
  luaD_callnoyield(L, L->ਸਿਖਰ.p - 2, 0);
}


ਸਥਿਰ ਖਾਲੀ GCTM (lua_State *L) {
  global_State *g = G(L);
  ਅਚਲ TValue *tm;
  TValue v;
  lua_assert(!g->gcemergency);
  setgcovalue(L, &v, udata2finalize(g));
  tm = luaT_gettmbyobj(L, &v, TM_GC);
  ਜੇ (!notm(tm)) {  /* is there a finalizer? */
    TStatus ਸਥਿਤੀ;
    lu_byte oldah = L->allowhook;
    lu_byte oldgcstp  = g->gcstp;
    g->gcstp |= GCSTPGC;  /* avoid GC steps */
    L->allowhook = 0;  /* stop debug hooks during GC metamethod */
    setobj2s(L, L->ਸਿਖਰ.p++, tm);  /* push finalizer... */
    setobj2s(L, L->ਸਿਖਰ.p++, &v);  /* ... and its argument */
    L->ci->callstatus |= CIST_FIN;  /* will run a finalizer */
    ਸਥਿਤੀ = luaD_pcall(L, dothecall, ਨਿਰਾ, savestack(L, L->ਸਿਖਰ.p - 2), 0);
    L->ci->callstatus &= ~CIST_FIN;  /* not running a finalizer anymore */
    L->allowhook = oldah;  /* restore hooks */
    g->gcstp = oldgcstp;  /* restore state */
    ਜੇ (l_unlikely(ਸਥਿਤੀ != LUA_OK)) {  /* error while running __gc? */
      luaE_warnerror(L, "__gc");
      L->ਸਿਖਰ.p--;  /* pops error object */
    }
  }
}


/*
** call all pending finalizers
*/
ਸਥਿਰ ਖਾਲੀ callallpendingfinalizers (lua_State *L) {
  global_State *g = G(L);
  ਜਦੋਂ (g->tobefnz)
    GCTM(L);
}


/*
** find last 'next' field in list 'p' list (to add elements in its end)
*/
ਸਥਿਰ GCObject **findlast (GCObject **p) {
  ਜਦੋਂ (*p != ਨਿਰਾ)
    p = &(*p)->ਅਗਲਾ;
  ਵਾਪਸ p;
}


/*
** Move all unreachable objects (or 'all' objects) that need
** finalization from list 'finobj' to list 'tobefnz' (to be finalized).
** (Note that objects after 'finobjold1' cannot be white, so they
** don't need to be traversed. In incremental mode, 'finobjold1' is NULL,
** so the whole list is traversed.)
*/
ਸਥਿਰ ਖਾਲੀ separatetobefnz (global_State *g, ਪੂਰਨ all) {
  GCObject *curr;
  GCObject **p = &g->finobj;
  GCObject **lastnext = findlast(&g->tobefnz);
  ਜਦੋਂ ((curr = *p) != g->finobjold1) {  /* traverse all finalizable objects */
    lua_assert(tofinalize(curr));
    ਜੇ (!(iswhite(curr) || all))  /* not being collected? */
      p = &curr->ਅਗਲਾ;  /* don't bother with it */
    ਨਹੀਂਤਾਂ {
      ਜੇ (curr == g->finobjsur)  /* removing 'finobjsur'? */
        g->finobjsur = curr->ਅਗਲਾ;  /* correct it */
      *p = curr->ਅਗਲਾ;  /* remove 'curr' from 'finobj' list */
      curr->ਅਗਲਾ = *lastnext;  /* link at the end of 'tobefnz' list */
      *lastnext = curr;
      lastnext = &curr->ਅਗਲਾ;
    }
  }
}


/*
** If pointer 'p' points to 'o', move it to the next element.
*/
ਸਥਿਰ ਖਾਲੀ checkpointer (GCObject **p, GCObject *o) {
  ਜੇ (o == *p)
    *p = o->ਅਗਲਾ;
}


/*
** Correct pointers to objects inside 'allgc' list when
** object 'o' is being removed from the list.
*/
ਸਥਿਰ ਖਾਲੀ correctpointers (global_State *g, GCObject *o) {
  checkpointer(&g->survival, o);
  checkpointer(&g->old1, o);
  checkpointer(&g->reallyold, o);
  checkpointer(&g->firstold1, o);
}


/*
** if object 'o' has a finalizer, remove it from 'allgc' list (must
** search the list to find it) and link it in 'finobj' list.
*/
ਖਾਲੀ luaC_checkfinalizer (lua_State *L, GCObject *o, ਸਾਰਣੀ *mt) {
  global_State *g = G(L);
  ਜੇ (tofinalize(o) ||                 /* obj. is already marked... */
      gfasttm(g, mt, TM_GC) == ਨਿਰਾ ||    /* or has no finalizer... */
      (g->gcstp & GCSTPCLS))                   /* or closing state? */
    ਵਾਪਸ;  /* nothing to be done */
  ਨਹੀਂਤਾਂ {  /* move 'o' to 'finobj' list */
    GCObject **p;
    ਜੇ (issweepphase(g)) {
      makewhite(g, o);  /* "sweep" object 'o' */
      ਜੇ (g->sweepgc == &o->ਅਗਲਾ)  /* should not remove 'sweepgc' object */
        g->sweepgc = sweeptolive(L, g->sweepgc);  /* change 'sweepgc' */
    }
    ਨਹੀਂਤਾਂ
      correctpointers(g, o);
    /* search for pointer pointing to 'o' */
    ਲਈ (p = &g->allgc; *p != o; p = &(*p)->ਅਗਲਾ) { /*ਖਾਲੀ_ਥਾਵਾਂ*/ }
    *p = o->ਅਗਲਾ;  /* remove 'o' from 'allgc' list */
    o->ਅਗਲਾ = g->finobj;  /* link it in 'finobj' list */
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
ਸਥਿਰ ਖਾਲੀ setpause (global_State *g) {
  l_mem ਹੱਦ = applygcparam(g, PAUSE, g->GCmarked);
  l_mem debt = ਹੱਦ - gettotalbytes(g);
  ਜੇ (debt < 0) debt = 0;
  luaE_setdebt(g, debt);
}


/*
** Sweep a list of objects to enter generational mode.  Deletes dead
** objects and turns the non dead to old. All non-dead threads---which
** are now old---must be in a gray list. Everything else is not in a
** gray list. Open upvalues are also kept gray.
*/
ਸਥਿਰ ਖਾਲੀ sweep2old (lua_State *L, GCObject **p) {
  GCObject *curr;
  global_State *g = G(L);
  ਜਦੋਂ ((curr = *p) != ਨਿਰਾ) {
    ਜੇ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(isdead(g, curr));
      *p = curr->ਅਗਲਾ;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ਨਹੀਂਤਾਂ {  /* all surviving objects become old */
      setage(curr, G_OLD);
      ਜੇ (curr->tt == LUA_VTHREAD) {  /* threads must be watched */
        lua_State *th = gco2th(curr);
        linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
      }
      ਨਹੀਂਤਾਂ ਜੇ (curr->tt == LUA_VUPVAL && upisopen(gco2upv(curr)))
        set2gray(curr);  /* open upvalues are always gray */
      ਨਹੀਂਤਾਂ  /* everything else is black */
        nw2black(curr);
      p = &curr->ਅਗਲਾ;  /* go to next element */
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
ਸਥਿਰ GCObject **sweepgen (lua_State *L, global_State *g, GCObject **p,
                            GCObject *ਹੱਦਬੰਦੀ, GCObject **pfirstold1,
                            l_mem *paddedold) {
  ਸਥਿਰ ਅਚਲ lu_byte nextage[] = {
    G_SURVIVAL,  /* from G_NEW */
    G_OLD1,      /* from G_SURVIVAL */
    G_OLD1,      /* from G_OLD0 */
    G_OLD,       /* from G_OLD1 */
    G_OLD,       /* from G_OLD (do not change) */
    G_TOUCHED1,  /* from G_TOUCHED1 (do not change) */
    G_TOUCHED2   /* from G_TOUCHED2 (do not change) */
  };
  l_mem addedold = 0;
  ਪੂਰਨ white = luaC_white(g);
  GCObject *curr;
  ਜਦੋਂ ((curr = *p) != ਹੱਦਬੰਦੀ) {
    ਜੇ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(!isold(curr) && isdead(g, curr));
      *p = curr->ਅਗਲਾ;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ਨਹੀਂਤਾਂ {  /* correct mark and age */
      ਪੂਰਨ age = getage(curr);
      ਜੇ (age == G_NEW) {  /* new objects go back to white */
        ਪੂਰਨ marked = curr->marked & ~maskgcbits;  /* erase GC bits */
        curr->marked = cast_byte(marked | G_SURVIVAL | white);
      }
      ਨਹੀਂਤਾਂ {  /* all other objects will be old, and so keep their color */
        lua_assert(age != G_OLD1);  /* advanced in 'markold' */
        setage(curr, nextage[age]);
        ਜੇ (getage(curr) == G_OLD1) {
          addedold += objsize(curr);  /* bytes becoming old */
          ਜੇ (*pfirstold1 == ਨਿਰਾ)
            *pfirstold1 = curr;  /* first OLD1 object in the list */
        }
      }
      p = &curr->ਅਗਲਾ;  /* go to next element */
    }
  }
  *paddedold += addedold;
  ਵਾਪਸ p;
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
ਸਥਿਰ GCObject **correctgraylist (GCObject **p) {
  GCObject *curr;
  ਜਦੋਂ ((curr = *p) != ਨਿਰਾ) {
    GCObject **ਅਗਲਾ = getgclist(curr);
    ਜੇ (iswhite(curr))
      ਜਾਓ remove;  /* remove all white objects */
    ਨਹੀਂਤਾਂ ਜੇ (getage(curr) == G_TOUCHED1) {  /* touched in this cycle? */
      lua_assert(isgray(curr));
      nw2black(curr);  /* make it black, for next barrier */
      setage(curr, G_TOUCHED2);
      ਜਾਓ remain;  /* keep it in the list and go to next element */
    }
    ਨਹੀਂਤਾਂ ਜੇ (curr->tt == LUA_VTHREAD) {
      lua_assert(isgray(curr));
      ਜਾਓ remain;  /* keep non-white threads on the list */
    }
    ਨਹੀਂਤਾਂ {  /* everything else is removed */
      lua_assert(isold(curr));  /* young objects should be white here */
      ਜੇ (getage(curr) == G_TOUCHED2)  /* advance from TOUCHED2... */
        setage(curr, G_OLD);  /* ... to OLD */
      nw2black(curr);  /* make object black (to be removed) */
      ਜਾਓ remove;
    }
    remove: *p = *ਅਗਲਾ; ਜਾਰੀ;
    remain: p = ਅਗਲਾ; ਜਾਰੀ;
  }
  ਵਾਪਸ p;
}


/*
** Correct all gray lists, coalescing them into 'grayagain'.
*/
ਸਥਿਰ ਖਾਲੀ correctgraylists (global_State *g) {
  GCObject **list = correctgraylist(&g->grayagain);
  *list = g->weak; g->weak = ਨਿਰਾ;
  list = correctgraylist(list);
  *list = g->allweak; g->allweak = ਨਿਰਾ;
  list = correctgraylist(list);
  *list = g->ephemeron; g->ephemeron = ਨਿਰਾ;
  correctgraylist(list);
}


/*
** Mark black 'OLD1' objects when starting a new young collection.
** Gray objects are already in some gray list, and so will be visited in
** the atomic step.
*/
ਸਥਿਰ ਖਾਲੀ markold (global_State *g, GCObject *from, GCObject *ਮੰਜ਼ਿਲ) {
  GCObject *p;
  ਲਈ (p = from; p != ਮੰਜ਼ਿਲ; p = p->ਅਗਲਾ) {
    ਜੇ (getage(p) == G_OLD1) {
      lua_assert(!iswhite(p));
      setage(p, G_OLD);  /* now they are old */
      ਜੇ (isblack(p))
        reallymarkobject(g, p);
    }
  }
}


/*
** Finish a young-generation collection.
*/
ਸਥਿਰ ਖਾਲੀ finishgencycle (lua_State *L, global_State *g) {
  correctgraylists(g);
  checkSizes(L, g);
  g->gcstate = GCSpropagate;  /* skip restart */
  ਜੇ (g->tobefnz != ਨਿਰਾ && !g->gcemergency && luaD_checkminstack(L))
    callallpendingfinalizers(L);
}


/*
** Shifts from a minor collection to major collections. It starts in
** the "sweep all" state to clear all objects, which are mostly black
** in generational mode.
*/
ਸਥਿਰ ਖਾਲੀ minor2inc (lua_State *L, global_State *g, lu_byte ਕਿਸਮ) {
  g->GCmajorminor = g->GCmarked;  /* number of live bytes */
  g->gckind = ਕਿਸਮ;
  g->reallyold = g->old1 = g->survival = ਨਿਰਾ;
  g->finobjrold = g->finobjold1 = g->finobjsur = ਨਿਰਾ;
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
ਸਥਿਰ ਪੂਰਨ checkminormajor (global_State *g) {
  l_mem ਹੱਦਬੰਦੀ = applygcparam(g, MINORMAJOR, g->GCmajorminor);
  ਜੇ (ਹੱਦਬੰਦੀ == 0)
    ਵਾਪਸ 0;  /* special case: 'minormajor' 0 stops major collections */
  ਵਾਪਸ (g->GCmarked >= ਹੱਦਬੰਦੀ);
}

/*
** Does a young collection. First, mark 'OLD1' objects. Then does the
** atomic step. Then, check whether to continue in minor mode. If so,
** sweep all lists and advance pointers. Finally, finish the collection.
*/
ਸਥਿਰ ਖਾਲੀ youngcollection (lua_State *L, global_State *g) {
  l_mem addedold1 = 0;
  l_mem marked = g->GCmarked;  /* preserve 'g->GCmarked' */
  GCObject **psurvival;  /* to point to first non-dead survival object */
  GCObject *dummy;  /* dummy out parameter to 'sweepgen' */
  lua_assert(g->gcstate == GCSpropagate);
  ਜੇ (g->firstold1) {  /* are there regular OLD1 objects? */
    markold(g, g->firstold1, g->reallyold);  /* mark them */
    g->firstold1 = ਨਿਰਾ;  /* no more OLD1 objects (for now) */
  }
  markold(g, g->finobj, g->finobjrold);
  markold(g, g->tobefnz, ਨਿਰਾ);

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
  dummy = ਨਿਰਾ;  /* no 'firstold1' optimization for 'finobj' lists */
  psurvival = sweepgen(L, g, &g->finobj, g->finobjsur, &dummy, &addedold1);
  /* sweep 'survival' */
  sweepgen(L, g, psurvival, g->finobjold1, &dummy, &addedold1);
  g->finobjrold = g->finobjold1;
  g->finobjold1 = *psurvival;  /* 'survival' survivals are old now */
  g->finobjsur = g->finobj;  /* all news are survivals */

  sweepgen(L, g, &g->tobefnz, ਨਿਰਾ, &dummy, &addedold1);

  /* keep total number of added old1 bytes */
  g->GCmarked = marked + addedold1;

  /* decide whether to shift to major mode */
  ਜੇ (checkminormajor(g)) {
    minor2inc(L, g, KGC_GENMAJOR);  /* go to major mode */
    g->GCmarked = 0;  /* avoid pause in first major cycle (see 'setpause') */
  }
  ਨਹੀਂਤਾਂ
    finishgencycle(L, g);  /* still in minor mode; finish it */
}


/*
** Clears all gray lists, sweeps objects, and prepare sublists to enter
** generational mode. The sweeps remove dead objects and turn all
** surviving objects to old. Threads go back to 'grayagain'; everything
** else is turned black (not in any gray list).
*/
ਸਥਿਰ ਖਾਲੀ atomic2gen (lua_State *L, global_State *g) {
  cleargraylists(g);
  /* sweep all elements making them old */
  g->gcstate = GCSswpallgc;
  sweep2old(L, &g->allgc);
  /* everything alive now is old */
  g->reallyold = g->old1 = g->survival = g->allgc;
  g->firstold1 = ਨਿਰਾ;  /* there are no OLD1 objects anywhere */

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
ਸਥਿਰ ਖਾਲੀ setminordebt (global_State *g) {
  luaE_setdebt(g, applygcparam(g, MINORMUL, g->GCmajorminor));
}


/*
** Enter generational mode. Must go until the end of an atomic cycle
** to ensure that all objects are correctly marked and weak tables
** are cleared. Then, turn all objects into old and finishes the
** collection.
*/
ਸਥਿਰ ਖਾਲੀ entergen (lua_State *L, global_State *g) {
  luaC_runtilstate(L, GCSpause, 1);  /* prepare to start a new cycle */
  luaC_runtilstate(L, GCSpropagate, 1);  /* start new cycle */
  atomic(L);  /* propagates all and then do the atomic stuff */
  atomic2gen(L, g);
  setminordebt(g);  /* set debt assuming next cycle will be minor */
}


/*
** Change collector mode to 'newmode'.
*/
ਖਾਲੀ luaC_changemode (lua_State *L, ਪੂਰਨ newmode) {
  global_State *g = G(L);
  ਜੇ (g->gckind == KGC_GENMAJOR)  /* doing major collections? */
    g->gckind = KGC_INC;  /* already incremental but in name */
  ਜੇ (newmode != g->gckind) {  /* does it need to change? */
    ਜੇ (newmode == KGC_INC)  /* entering incremental mode? */
      minor2inc(L, g, KGC_INC);  /* entering incremental mode */
    ਨਹੀਂਤਾਂ {
      lua_assert(newmode == KGC_GENMINOR);
      entergen(L, g);
    }
  }
}


/*
** Does a full collection in generational mode.
*/
ਸਥਿਰ ਖਾਲੀ fullgen (lua_State *L, global_State *g) {
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
ਸਥਿਰ ਪੂਰਨ checkmajorminor (lua_State *L, global_State *g) {
  ਜੇ (g->gckind == KGC_GENMAJOR) {  /* generational mode? */
    l_mem numbytes = gettotalbytes(g);
    l_mem addedbytes = numbytes - g->GCmajorminor;
    l_mem ਹੱਦਬੰਦੀ = (addedbytes < 0) ? 0
                                   : applygcparam(g, MAJORMINOR, addedbytes);
    l_mem tobecollected = numbytes - g->GCmarked;
    ਜੇ (tobecollected > ਹੱਦਬੰਦੀ) {
      atomic2gen(L, g);  /* return to generational mode */
      setminordebt(g);
      ਵਾਪਸ 1;  /* exit incremental collection */
    }
  }
  g->GCmajorminor = g->GCmarked;  /* prepare for next collection */
  ਵਾਪਸ 0;  /* stay doing incremental collections */
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
ਸਥਿਰ ਖਾਲੀ entersweep (lua_State *L) {
  global_State *g = G(L);
  g->gcstate = GCSswpallgc;
  lua_assert(g->sweepgc == ਨਿਰਾ);
  g->sweepgc = sweeptolive(L, &g->allgc);
}


/*
** Delete all objects in list 'p' until (but not including) object
** 'limit'.
*/
ਸਥਿਰ ਖਾਲੀ deletelist (lua_State *L, GCObject *p, GCObject *ਹੱਦਬੰਦੀ) {
  ਜਦੋਂ (p != ਹੱਦਬੰਦੀ) {
    GCObject *ਅਗਲਾ = p->ਅਗਲਾ;
    freeobj(L, p);
    p = ਅਗਲਾ;
  }
}


/*
** Call all finalizers of the objects in the given Lua state, and
** then free all objects, except for the main thread.
*/
ਖਾਲੀ luaC_freeallobjects (lua_State *L) {
  global_State *g = G(L);
  g->gcstp = GCSTPCLS;  /* no extra finalizers after here */
  luaC_changemode(L, KGC_INC);
  separatetobefnz(g, 1);  /* separate all objects with finalizers */
  lua_assert(g->finobj == ਨਿਰਾ);
  callallpendingfinalizers(L);
  deletelist(L, g->allgc, obj2gco(mainthread(g)));
  lua_assert(g->finobj == ਨਿਰਾ);  /* no new finalizers */
  deletelist(L, g->fixedgc, ਨਿਰਾ);  /* collect fixed objects */
  lua_assert(g->strt.nuse == 0);
}


ਸਥਿਰ ਖਾਲੀ atomic (lua_State *L) {
  global_State *g = G(L);
  GCObject *origweak, *origall;
  GCObject *grayagain = g->grayagain;  /* save original list */
  g->grayagain = ਨਿਰਾ;
  lua_assert(g->ephemeron == ਨਿਰਾ && g->weak == ਨਿਰਾ);
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
  g->ਸਲੇਟੀ = grayagain;
  propagateall(g);  /* traverse 'grayagain' list */
  convergeephemerons(g);
  /* at this point, all strongly accessible objects are marked. */
  /* Clear values from weak tables, before checking finalizers */
  clearbyvalues(g, g->weak, ਨਿਰਾ);
  clearbyvalues(g, g->allweak, ਨਿਰਾ);
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
  lua_assert(g->ਸਲੇਟੀ == ਨਿਰਾ);
}


/*
** Do a sweep step. The normal case (not fast) sweeps at most GCSWEEPMAX
** elements. The fast case sweeps the whole list.
*/
ਸਥਿਰ ਖਾਲੀ sweepstep (lua_State *L, global_State *g,
                       lu_byte nextstate, GCObject **nextlist, ਪੂਰਨ fast) {
  ਜੇ (g->sweepgc)
    g->sweepgc = sweeplist(L, g->sweepgc, fast ? MAX_LMEM : GCSWEEPMAX);
  ਨਹੀਂਤਾਂ {  /* enter next state */
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ step2pause	-3  /* finished collection; entered pause state */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ atomicstep	-2  /* atomic step */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ step2minor	-1  /* moved to minor collections */


ਸਥਿਰ l_mem singlestep (lua_State *L, ਪੂਰਨ fast) {
  global_State *g = G(L);
  l_mem stepresult;
  lua_assert(!g->gcstopem);  /* collector is not reentrant */
  g->gcstopem = 1;  /* no emergency collections while collecting */
  ਚੋਣ (g->gcstate) {
    ਹਾਲਤ GCSpause: {
      restartcollection(g);
      g->gcstate = GCSpropagate;
      stepresult = 1;
      ਰੋਕੋ;
    }
    ਹਾਲਤ GCSpropagate: {
      ਜੇ (fast || g->ਸਲੇਟੀ == ਨਿਰਾ) {
        g->gcstate = GCSenteratomic;  /* finish propagate phase */
        stepresult = 1;
      }
      ਨਹੀਂਤਾਂ
        stepresult = propagatemark(g);  /* traverse one gray object */
      ਰੋਕੋ;
    }
    ਹਾਲਤ GCSenteratomic: {
      atomic(L);
      ਜੇ (checkmajorminor(L, g))
        stepresult = step2minor;
      ਨਹੀਂਤਾਂ {
        entersweep(L);
        stepresult = atomicstep;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ GCSswpallgc: {  /* sweep "regular" objects */
      sweepstep(L, g, GCSswpfinobj, &g->finobj, fast);
      stepresult = GCSWEEPMAX;
      ਰੋਕੋ;
    }
    ਹਾਲਤ GCSswpfinobj: {  /* sweep objects with finalizers */
      sweepstep(L, g, GCSswptobefnz, &g->tobefnz, fast);
      stepresult = GCSWEEPMAX;
      ਰੋਕੋ;
    }
    ਹਾਲਤ GCSswptobefnz: {  /* sweep objects to be finalized */
      sweepstep(L, g, GCSswpend, ਨਿਰਾ, fast);
      stepresult = GCSWEEPMAX;
      ਰੋਕੋ;
    }
    ਹਾਲਤ GCSswpend: {  /* finish sweeps */
      checkSizes(L, g);
      g->gcstate = GCScallfin;
      stepresult = GCSWEEPMAX;
      ਰੋਕੋ;
    }
    ਹਾਲਤ GCScallfin: {  /* call finalizers */
      ਜੇ (g->tobefnz && !g->gcemergency && luaD_checkminstack(L)) {
        g->gcstopem = 0;  /* ok collections during finalizers */
        GCTM(L);  /* call one finalizer */
        stepresult = CWUFIN;
      }
      ਨਹੀਂਤਾਂ {  /* no more finalizers or emergency mode or not enough stack
                 to run finalizers */
        g->gcstate = GCSpause;  /* finish collection */
        stepresult = step2pause;
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: lua_assert(0); ਵਾਪਸ 0;
  }
  g->gcstopem = 0;
  ਵਾਪਸ stepresult;
}


/*
** Advances the garbage collector until it reaches the given state.
** (The option 'fast' is only for testing; in normal code, 'fast'
** here is always true.)
*/
ਖਾਲੀ luaC_runtilstate (lua_State *L, ਪੂਰਨ state, ਪੂਰਨ fast) {
  global_State *g = G(L);
  lua_assert(g->gckind == KGC_INC);
  ਜਦੋਂ (state != g->gcstate)
    singlestep(L, fast);
}



/*
** Performs a basic incremental step. The step size is
** converted from bytes to "units of work"; then the function loops
** running single steps until adding that many units of work or
** finishing a cycle (pause state). Finally, it sets the debt that
** controls when next step will be performed.
*/
ਸਥਿਰ ਖਾਲੀ incstep (lua_State *L, global_State *g) {
  l_mem stepsize = applygcparam(g, STEPSIZE, 100);
  l_mem work2do = applygcparam(g, STEPMUL, stepsize / cast_int(ਆਕਾਰ(ਖਾਲੀ*)));
  l_mem stres;
  ਪੂਰਨ fast = (work2do == 0);  /* special case: do a full collection */
  ਕਰੋ {  /* repeat until enough work */
    stres = singlestep(L, fast);  /* perform one single step */
    ਜੇ (stres == step2minor)  /* returned to minor collections? */
      ਵਾਪਸ;  /* nothing else to be done here */
    ਨਹੀਂਤਾਂ ਜੇ (stres == step2pause || (stres == atomicstep && !fast))
      ਰੋਕੋ;  /* end of cycle or atomic */
    ਨਹੀਂਤਾਂ
      work2do -= stres;
  } ਜਦੋਂ (fast || work2do > 0);
  ਜੇ (g->gcstate == GCSpause)
    setpause(g);  /* pause until next cycle */
  ਨਹੀਂਤਾਂ
    luaE_setdebt(g, stepsize);
}


#ਜੇ !defined(luai_tracegc)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_tracegc(L,f)		((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

/*
** Performs a basic GC step if collector is running. (If collector was
** stopped by the user, set a reasonable debt to avoid it being called
** at every single check.)
*/
ਖਾਲੀ lua_C_ਕਦਮ (lua_State *L) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  ਜੇ (!gcrunning(g)) {  /* not running? */
    ਜੇ (g->gcstp & GCSTPUSR)  /* stopped by the user? */
      luaE_setdebt(g, 20000);
  }
  ਨਹੀਂਤਾਂ {
    luai_tracegc(L, 1);  /* for internal debugging */
    ਚੋਣ (g->gckind) {
      ਹਾਲਤ KGC_INC: ਹਾਲਤ KGC_GENMAJOR:
        incstep(L, g);
        ਰੋਕੋ;
      ਹਾਲਤ KGC_GENMINOR:
        youngcollection(L, g);
        setminordebt(g);
        ਰੋਕੋ;
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
ਸਥਿਰ ਖਾਲੀ fullinc (lua_State *L, global_State *g) {
  ਜੇ (keepinvariant(g))  /* black objects? */
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
ਖਾਲੀ luaC_fullgc (lua_State *L, ਪੂਰਨ isemergency) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  g->gcemergency = cast_byte(isemergency);  /* set flag */
  ਚੋਣ (g->gckind) {
    ਹਾਲਤ KGC_GENMINOR: fullgen(L, g); ਰੋਕੋ;
    ਹਾਲਤ KGC_INC: fullinc(L, g); ਰੋਕੋ;
    ਹਾਲਤ KGC_GENMAJOR:
      g->gckind = KGC_INC;
      fullinc(L, g);
      g->gckind = KGC_GENMAJOR;
      ਰੋਕੋ;
  }
  g->gcemergency = 0;
}

/* }====================================================== */


