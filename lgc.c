/*
** $Id: lgc.c $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#define lgc_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"

#അന്തര്ഭാവ <string.h>


#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "ltm.h"


/*
** Maximum number of elements to sweep in each single step.
** (Large enough to dissipate fixed overheads but small enough
** to allow small steps for the collector.)
*/
#define GCSWEEPMAX	20


/*
** Cost (in work units) of running one finalizer.
*/
#define CWUFIN	10


/* mask with all color bits */
#define maskcolors	(bitmask(BLACKBIT) | WHITEBITS)

/* mask with all GC bits */
#define maskgcbits      (maskcolors | AGEBITS)


/* macro to erase all color bits then set only the current white bit */
#define makewhite(g,x)	\
  (x->marked = cast_byte((x->marked & ~maskcolors) | luaC_white(g)))

/* make an object gray (neither white nor black) */
#define set2gray(x)	resetbits(x->marked, maskcolors)


/* make an object black (coming from any color) */
#define set2black(x)  \
  (x->marked = cast_byte((x->marked & ~WHITEBITS) | bitmask(BLACKBIT)))


#define valiswhite(x)   (iscollectable(x) && iswhite(gcvalue(x)))

#define keyiswhite(n)   (keyiscollectable(n) && iswhite(gckey(n)))


/*
** Protected access to objects in values
*/
#define gcvalueN(o)     (iscollectable(o) ? gcvalue(o) : ശൂന്യ)


/*
** Access to collectable objects in array part of tables
*/
#define gcvalarr(t,i)  \
	((*getArrTag(t,i) & BIT_ISCOLLECTABLE) ? getArrVal(t,i)->gc : ശൂന്യ)


#define markvalue(g,o) { checkliveness(mainthread(g),o); \
  എങ്കിൽ (valiswhite(o)) reallymarkobject(g,gcvalue(o)); }

#define markkey(g, n)	{ എങ്കിൽ keyiswhite(n) reallymarkobject(g,gckey(n)); }

#define markobject(g,t)	{ എങ്കിൽ (iswhite(t)) reallymarkobject(g, obj2gco(t)); }

/*
** mark an object that can be NULL (either because it is really optional,
** or it was stripped as debug info, or inside an uncompleted structure)
*/
#define markobjectN(g,t)	{ എങ്കിൽ (t) markobject(g,t); }


സ്ഥിതം ശൂന്യം reallymarkobject (global_State *g, GCObject *o);
സ്ഥിതം ശൂന്യം atomic (lua_State *L);
സ്ഥിതം ശൂന്യം entersweep (lua_State *L);


/*
** {======================================================
** Generic functions
** =======================================================
*/


/*
** one after last element in a hash array
*/
#define gnodelast(h)	gnode(h, cast_sizet(sizenode(h)))


സ്ഥിതം l_mem objsize (GCObject *o) {
  lu_mem res;
  തിരഞ്ഞെടുപ്പ് (o->tt) {
    അവസ്ഥ LUA_VTABLE: {
      res = luaH_size(gco2t(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      res = sizeLclosure(cl->nupvalues);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      res = sizeCclosure(cl->nupvalues);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      res = sizeudata(u->nuvalue, u->len);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VPROTO: {
      res = luaF_protosize(gco2p(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VTHREAD: {
      res = luaE_threadsize(gco2th(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      res = sizestrshr(cast_uint(ts->shrlen));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      res = luaS_sizelngstr(ts->u.lnglen, ts->shrlen);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VUPVAL: {
      res = വലുപ്പം(UpVal);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: res = 0; lua_assert(0);
  }
  തിരികെ cast(l_mem, res);
}


സ്ഥിതം GCObject **getgclist (GCObject *o) {
  തിരഞ്ഞെടുപ്പ് (o->tt) {
    അവസ്ഥ LUA_VTABLE: തിരികെ &gco2t(o)->gclist;
    അവസ്ഥ LUA_VLCL: തിരികെ &gco2lcl(o)->gclist;
    അവസ്ഥ LUA_VCCL: തിരികെ &gco2ccl(o)->gclist;
    അവസ്ഥ LUA_VTHREAD: തിരികെ &gco2th(o)->gclist;
    അവസ്ഥ LUA_VPROTO: തിരികെ &gco2p(o)->gclist;
    അവസ്ഥ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      lua_assert(u->nuvalue > 0);
      തിരികെ &u->gclist;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0); തിരികെ 0;
  }
}


/*
** Link a collectable object 'o' with a known type into the list 'p'.
** (Must be a macro to access the 'gclist' field in different types.)
*/
#define linkgclist(o,p)	linkgclist_(obj2gco(o), &(o)->gclist, &(p))

സ്ഥിതം ശൂന്യം linkgclist_ (GCObject *o, GCObject **pnext, GCObject **list) {
  lua_assert(!isgray(o));  /* cannot be in a gray list */
  *pnext = *list;
  *list = o;
  set2gray(o);  /* now it is */
}


/*
** Link a generic collectable object 'o' into the list 'p'.
*/
#define linkobjgclist(o,p) linkgclist_(obj2gco(o), getgclist(o), &(p))



/*
** Clear keys for empty entries in tables. If entry is empty, mark its
** entry as dead. This allows the collection of the key, but keeps its
** entry in the table: its removal could break a chain and could break
** a table traversal.  Other places never manipulate dead keys, because
** its associated empty value is enough to signal that the entry is
** logically empty.
*/
സ്ഥിതം ശൂന്യം clearkey (കണ്ണി_2 *n) {
  lua_assert(isempty(gval(n)));
  എങ്കിൽ (keyiscollectable(n))
    setdeadkey(n);  /* unused key; remove it */
}


/*
** tells whether a key or value can be cleared from a weak
** table. Non-collectable objects are never removed from weak
** tables. Strings behave as 'values', so are never removed too. for
** other objects: if really collected, cannot keep them; for objects
** being finalized, keep them in keys, but not in values
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ iscleared (global_State *g, സ്ഥിരം GCObject *o) {
  എങ്കിൽ (o == ശൂന്യ) തിരികെ 0;  /* non-collectable value */
  അല്ലെങ്കിൽ എങ്കിൽ (novariant(o->tt) == LUA_TSTRING) {
    markobject(g, o);  /* strings are 'values', so are never weak */
    തിരികെ 0;
  }
  അല്ലെങ്കിൽ തിരികെ iswhite(o);
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
ശൂന്യം luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v) {
  global_State *g = G(L);
  lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
  എങ്കിൽ (keepinvariant(g)) {  /* must keep invariant? */
    reallymarkobject(g, v);  /* restore invariant */
    എങ്കിൽ (isold(o)) {
      lua_assert(!isold(v));  /* white object could not be old */
      setage(v, G_OLD0);  /* restore generational invariant */
    }
  }
  അല്ലെങ്കിൽ {  /* sweep phase */
    lua_assert(issweepphase(g));
    എങ്കിൽ (g->gckind != KGC_GENMINOR)  /* incremental mode? */
      makewhite(g, o);  /* mark 'o' as white to avoid other barriers */
  }
}


/*
** barrier that moves collector backward, that is, mark the black object
** pointing to a white object as gray again.
*/
ശൂന്യം luaC_barrierback_ (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(isblack(o) && !isdead(g, o));
  lua_assert((g->gckind != KGC_GENMINOR)
          || (isold(o) && getage(o) != G_TOUCHED1));
  എങ്കിൽ (getage(o) == G_TOUCHED2)  /* already in gray list? */
    set2gray(o);  /* make it gray to become touched1 */
  അല്ലെങ്കിൽ  /* link it in 'grayagain' and paint it gray */
    linkobjgclist(o, g->grayagain);
  എങ്കിൽ (isold(o))  /* generational mode? */
    setage(o, G_TOUCHED1);  /* touched in current cycle */
}


ശൂന്യം luaC_fix (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(g->allgc == o);  /* object must be 1st in 'allgc' list! */
  set2gray(o);  /* they will be gray forever */
  setage(o, G_OLD);  /* and old forever */
  g->allgc = o->next;  /* remove object from 'allgc' list */
  o->next = g->fixedgc;  /* link it to 'fixedgc' list */
  g->fixedgc = o;
}


/*
** create a new collectable object (with given type, size, and offset)
** and link it to 'allgc' list.
*/
GCObject *luaC_newobjdt (lua_State *L, lu_byte tt, size_t sz, size_t offset) {
  global_State *g = G(L);
  അക്ഷരം *p = cast_charp(luaM_newobject(L, novariant(tt), sz));
  GCObject *o = cast(GCObject *, p + offset);
  o->marked = luaC_white(g);
  o->tt = tt;
  o->next = g->allgc;
  g->allgc = o;
  തിരികെ o;
}


/*
** create a new collectable object with no offset.
*/
GCObject *luaC_newobj (lua_State *L, lu_byte tt, size_t sz) {
  തിരികെ luaC_newobjdt(L, tt, sz, 0);
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
സ്ഥിതം ശൂന്യം reallymarkobject (global_State *g, GCObject *o) {
  g->GCmarked += objsize(o);
  തിരഞ്ഞെടുപ്പ് (o->tt) {
    അവസ്ഥ LUA_VSHRSTR:
    അവസ്ഥ LUA_VLNGSTR: {
      set2black(o);  /* nothing to visit */
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VUPVAL: {
      UpVal *uv = gco2upv(o);
      എങ്കിൽ (upisopen(uv))
        set2gray(uv);  /* open upvalues are kept gray */
      അല്ലെങ്കിൽ
        set2black(uv);  /* closed upvalues are visited here */
      markvalue(g, uv->v.p);  /* mark its content */
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      എങ്കിൽ (u->nuvalue == 0) {  /* no user values? */
        markobjectN(g, u->metatable);  /* mark its metatable */
        set2black(u);  /* nothing else to mark */
        നിർത്തുക;
      }
      /* else... */
    }  /* FALLTHROUGH */
    അവസ്ഥ LUA_VLCL: അവസ്ഥ LUA_VCCL: അവസ്ഥ LUA_VTABLE:
    അവസ്ഥ LUA_VTHREAD: അവസ്ഥ LUA_VPROTO: {
      linkobjgclist(o, g->gray);  /* to be visited later */
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0); നിർത്തുക;
  }
}


/*
** mark metamethods for basic types
*/
സ്ഥിതം ശൂന്യം markmt (global_State *g) {
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i=0; i < LUA_NUMTYPES; i++)
    markobjectN(g, g->mt[i]);
}


/*
** mark all objects in list of being-finalized
*/
സ്ഥിതം ശൂന്യം markbeingfnz (global_State *g) {
  GCObject *o;
  വേണ്ടി (o = g->tobefnz; o != ശൂന്യ; o = o->next)
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
സ്ഥിതം ശൂന്യം remarkupvals (global_State *g) {
  lua_State *thread;
  lua_State **p = &g->twups;
  ആകുമ്പോൾ ((thread = *p) != ശൂന്യ) {
    എങ്കിൽ (!iswhite(thread) && thread->openupval != ശൂന്യ)
      p = &thread->twups;  /* keep marked thread with upvalues in the list */
    അല്ലെങ്കിൽ {  /* thread is not marked or without upvalues */
      UpVal *uv;
      lua_assert(!isold(thread) || thread->openupval == ശൂന്യ);
      *p = thread->twups;  /* remove thread from the list */
      thread->twups = thread;  /* mark that it is out of list */
      വേണ്ടി (uv = thread->openupval; uv != ശൂന്യ; uv = uv->u.open.next) {
        lua_assert(getage(uv) <= getage(thread));
        എങ്കിൽ (!iswhite(uv)) {  /* upvalue already visited? */
          lua_assert(upisopen(uv) && isgray(uv));
          markvalue(g, uv->v.p);  /* mark its value */
        }
      }
    }
  }
}


സ്ഥിതം ശൂന്യം cleargraylists (global_State *g) {
  g->gray = g->grayagain = ശൂന്യ;
  g->weak = g->allweak = g->ephemeron = ശൂന്യ;
}


/*
** mark root set and reset all gray lists, to start a new collection.
** 'GCmarked' is initialized to count the total number of live bytes
** during a cycle.
*/
സ്ഥിതം ശൂന്യം restartcollection (global_State *g) {
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
സ്ഥിതം ശൂന്യം genlink (global_State *g, GCObject *o) {
  lua_assert(isblack(o));
  എങ്കിൽ (getage(o) == G_TOUCHED1) {  /* touched in this cycle? */
    linkobjgclist(o, g->grayagain);  /* link it back in 'grayagain' */
  }  /* everything else do not need to be linked back */
  അല്ലെങ്കിൽ എങ്കിൽ (getage(o) == G_TOUCHED2)
    setage(o, G_OLD);  /* advance age */
}


/*
** Traverse a table with weak values and link it to proper list. During
** propagate phase, keep it in 'grayagain' list, to be revisited in the
** atomic phase. In the atomic phase, if table has any white value,
** put it in 'weak' list, to be cleared; otherwise, call 'genlink'
** to check table age in generational mode.
*/
സ്ഥിതം ശൂന്യം traverseweakvalue (global_State *g, Table *h) {
  കണ്ണി_2 *n, *limit = gnodelast(h);
  /* if there is array part, assume it may have white values (it is not
     worth traversing it now just to check) */
  പൂർണ്ണസംഖ്യ hasclears = (h->asize > 0);
  വേണ്ടി (n = gnode(h, 0); n < limit; n++) {  /* traverse hash part */
    എങ്കിൽ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    അല്ലെങ്കിൽ {
      lua_assert(!keyisnil(n));
      markkey(g, n);
      എങ്കിൽ (!hasclears && iscleared(g, gcvalueN(gval(n))))  /* a white value? */
        hasclears = 1;  /* table will have to be cleared */
    }
  }
  എങ്കിൽ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  അല്ലെങ്കിൽ എങ്കിൽ (hasclears)
      linkgclist(h, g->weak);  /* has to be cleared later */
  അല്ലെങ്കിൽ
    genlink(g, obj2gco(h));
}


/*
** Traverse the array part of a table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ traversearray (global_State *g, Table *h) {
  unsigned asize = h->asize;
  പൂർണ്ണസംഖ്യ marked = 0;  /* true if some object is marked in this traversal */
  unsigned i;
  വേണ്ടി (i = 0; i < asize; i++) {
    GCObject *o = gcvalarr(h, i);
    എങ്കിൽ (o != ശൂന്യ && iswhite(o)) {
      marked = 1;
      reallymarkobject(g, o);
    }
  }
  തിരികെ marked;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ traverseephemeron (global_State *g, Table *h, പൂർണ്ണസംഖ്യ inv) {
  പൂർണ്ണസംഖ്യ hasclears = 0;  /* true if table has white keys */
  പൂർണ്ണസംഖ്യ hasww = 0;  /* true if table has entry "white-key -> white-value" */
  unsigned പൂർണ്ണസംഖ്യ i;
  unsigned പൂർണ്ണസംഖ്യ nsize = sizenode(h);
  പൂർണ്ണസംഖ്യ marked = traversearray(g, h);  /* traverse array part */
  /* traverse hash part; if 'inv', traverse descending
     (see 'convergeephemerons') */
  വേണ്ടി (i = 0; i < nsize; i++) {
    കണ്ണി_2 *n = inv ? gnode(h, nsize - 1 - i) : gnode(h, i);
    എങ്കിൽ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    അല്ലെങ്കിൽ എങ്കിൽ (iscleared(g, gckeyN(n))) {  /* key is not marked (yet)? */
      hasclears = 1;  /* table must be cleared */
      എങ്കിൽ (valiswhite(gval(n)))  /* value not marked yet? */
        hasww = 1;  /* white-white entry */
    }
    അല്ലെങ്കിൽ എങ്കിൽ (valiswhite(gval(n))) {  /* value not marked yet? */
      marked = 1;
      reallymarkobject(g, gcvalue(gval(n)));  /* mark it now */
    }
  }
  /* link table into proper list */
  എങ്കിൽ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  അല്ലെങ്കിൽ എങ്കിൽ (hasww)  /* table has white->white entries? */
    linkgclist(h, g->ephemeron);  /* have to propagate again */
  അല്ലെങ്കിൽ എങ്കിൽ (hasclears)  /* table has white keys? */
    linkgclist(h, g->allweak);  /* may have to clean white keys */
  അല്ലെങ്കിൽ
    genlink(g, obj2gco(h));  /* check whether collector still needs to see it */
  തിരികെ marked;
}


സ്ഥിതം ശൂന്യം traversestrongtable (global_State *g, Table *h) {
  കണ്ണി_2 *n, *limit = gnodelast(h);
  traversearray(g, h);
  വേണ്ടി (n = gnode(h, 0); n < limit; n++) {  /* traverse hash part */
    എങ്കിൽ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    അല്ലെങ്കിൽ {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ getmode (global_State *g, Table *h) {
  സ്ഥിരം TValue *mode = gfasttm(g, h->metatable, TM_MODE);
  എങ്കിൽ (mode == ശൂന്യ || !ttisstring(mode))
    തിരികെ 0;  /* ignore non-string modes */
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *smode = getstr(tsvalue(mode));
    സ്ഥിരം അക്ഷരം *weakkey = strchr(smode, 'k');
    സ്ഥിരം അക്ഷരം *weakvalue = strchr(smode, 'v');
    തിരികെ ((weakkey != ശൂന്യ) << 1) | (weakvalue != ശൂന്യ);
  }
}


സ്ഥിതം l_mem traversetable (global_State *g, Table *h) {
  markobjectN(g, h->metatable);
  തിരഞ്ഞെടുപ്പ് (getmode(g, h)) {
    അവസ്ഥ 0:  /* not weak */
      traversestrongtable(g, h);
      നിർത്തുക;
    അവസ്ഥ 1:  /* weak values */
      traverseweakvalue(g, h);
      നിർത്തുക;
    അവസ്ഥ 2:  /* weak keys */
      traverseephemeron(g, h, 0);
      നിർത്തുക;
    അവസ്ഥ 3:  /* all weak; nothing to traverse */
      എങ്കിൽ (g->gcstate == GCSpropagate)
        linkgclist(h, g->grayagain);  /* must visit again its metatable */
      അല്ലെങ്കിൽ
        linkgclist(h, g->allweak);  /* must clear collected entries */
      നിർത്തുക;
  }
  തിരികെ cast(l_mem, 1 + 2*sizenode(h) + h->asize);
}


സ്ഥിതം l_mem traverseudata (global_State *g, Udata *u) {
  പൂർണ്ണസംഖ്യ i;
  markobjectN(g, u->metatable);  /* mark its metatable */
  വേണ്ടി (i = 0; i < u->nuvalue; i++)
    markvalue(g, &u->uv[i].uv);
  genlink(g, obj2gco(u));
  തിരികെ 1 + u->nuvalue;
}


/*
** Traverse a prototype. (While a prototype is being build, its
** arrays can be larger than needed; the extra slots are filled with
** NULL, so the use of 'markobjectN')
*/
സ്ഥിതം l_mem traverseproto (global_State *g, Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  markobjectN(g, f->source);
  വേണ്ടി (i = 0; i < f->sizek; i++)  /* mark literals */
    markvalue(g, &f->k[i]);
  വേണ്ടി (i = 0; i < f->sizeupvalues; i++)  /* mark upvalue names */
    markobjectN(g, f->upvalues[i].name);
  വേണ്ടി (i = 0; i < f->sizep; i++)  /* mark nested protos */
    markobjectN(g, f->p[i]);
  വേണ്ടി (i = 0; i < f->sizelocvars; i++)  /* mark local-variable names */
    markobjectN(g, f->locvars[i].varname);
  തിരികെ 1 + f->sizek + f->sizeupvalues + f->sizep + f->sizelocvars;
}


സ്ഥിതം l_mem traverseCclosure (global_State *g, CClosure *cl) {
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < cl->nupvalues; i++)  /* mark its upvalues */
    markvalue(g, &cl->upvalue[i]);
  തിരികെ 1 + cl->nupvalues;
}

/*
** Traverse a Lua closure, marking its prototype and its upvalues.
** (Both can be NULL while closure is being created.)
*/
സ്ഥിതം l_mem traverseLclosure (global_State *g, LClosure *cl) {
  പൂർണ്ണസംഖ്യ i;
  markobjectN(g, cl->p);  /* mark its prototype */
  വേണ്ടി (i = 0; i < cl->nupvalues; i++) {  /* visit its upvalues */
    UpVal *uv = cl->upvals[i];
    markobjectN(g, uv);  /* mark upvalue */
  }
  തിരികെ 1 + cl->nupvalues;
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
സ്ഥിതം l_mem traversethread (global_State *g, lua_State *th) {
  UpVal *uv;
  StkId o = th->അട്ടി.p;
  എങ്കിൽ (isold(th) || g->gcstate == GCSpropagate)
    linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
  എങ്കിൽ (o == ശൂന്യ)
    തിരികെ 0;  /* stack not completely built yet */
  lua_assert(g->gcstate == GCSatomic ||
             th->openupval == ശൂന്യ || isintwups(th));
  വേണ്ടി (; o < th->മുകൾ.p; o++)  /* mark live elements in the stack */
    markvalue(g, s2v(o));
  വേണ്ടി (uv = th->openupval; uv != ശൂന്യ; uv = uv->u.open.next)
    markobject(g, uv);  /* open upvalues cannot be collected */
  എങ്കിൽ (g->gcstate == GCSatomic) {  /* final traversal? */
    എങ്കിൽ (!g->gcemergency)
      luaD_shrinkstack(th); /* do not change stack in emergency cycle */
    വേണ്ടി (o = th->മുകൾ.p; o < th->അട്ടി_last.p + EXTRA_അട്ടി; o++)
      setnilvalue2s(o);  /* clear dead stack slice */
    /* 'remarkupvals' may have removed thread from 'twups' list */
    എങ്കിൽ (!isintwups(th) && th->openupval != ശൂന്യ) {
      th->twups = g->twups;  /* link it back to the list */
      g->twups = th;
    }
  }
  തിരികെ 1 + (th->മുകൾ.p - th->അട്ടി.p);
}


/*
** traverse one gray object, turning it to black. Return an estimate
** of the number of slots traversed.
*/
സ്ഥിതം l_mem propagatemark (global_State *g) {
  GCObject *o = g->gray;
  nw2black(o);
  g->gray = *getgclist(o);  /* remove from 'gray' list */
  തിരഞ്ഞെടുപ്പ് (o->tt) {
    അവസ്ഥ LUA_VTABLE: തിരികെ traversetable(g, gco2t(o));
    അവസ്ഥ LUA_VUSERDATA: തിരികെ traverseudata(g, gco2u(o));
    അവസ്ഥ LUA_VLCL: തിരികെ traverseLclosure(g, gco2lcl(o));
    അവസ്ഥ LUA_VCCL: തിരികെ traverseCclosure(g, gco2ccl(o));
    അവസ്ഥ LUA_VPROTO: തിരികെ traverseproto(g, gco2p(o));
    അവസ്ഥ LUA_VTHREAD: തിരികെ traversethread(g, gco2th(o));
    സ്ഥിരസ്ഥിതി: lua_assert(0); തിരികെ 0;
  }
}


സ്ഥിതം ശൂന്യം propagateall (global_State *g) {
  ആകുമ്പോൾ (g->gray)
    propagatemark(g);
}


/*
** Traverse all ephemeron tables propagating marks from keys to values.
** Repeat until it converges, that is, nothing new is marked. 'dir'
** inverts the direction of the traversals, trying to speed up
** convergence on chains in the same table.
*/
സ്ഥിതം ശൂന്യം convergeephemerons (global_State *g) {
  പൂർണ്ണസംഖ്യ changed;
  പൂർണ്ണസംഖ്യ dir = 0;
  ചെയ്യുക {
    GCObject *w;
    GCObject *next = g->ephemeron;  /* get ephemeron list */
    g->ephemeron = ശൂന്യ;  /* tables may return to this list when traversed */
    changed = 0;
    ആകുമ്പോൾ ((w = next) != ശൂന്യ) {  /* for each ephemeron table */
      Table *h = gco2t(w);
      next = h->gclist;  /* list is rebuilt during loop */
      nw2black(h);  /* out of the list (for now) */
      എങ്കിൽ (traverseephemeron(g, h, dir)) {  /* marked some value? */
        propagateall(g);  /* propagate changes */
        changed = 1;  /* will have to revisit all ephemeron tables */
      }
    }
    dir = !dir;  /* invert direction next time */
  } ആകുമ്പോൾ (changed);  /* repeat until no more changes */
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
സ്ഥിതം ശൂന്യം clearbykeys (global_State *g, GCObject *l) {
  വേണ്ടി (; l; l = gco2t(l)->gclist) {
    Table *h = gco2t(l);
    കണ്ണി_2 *limit = gnodelast(h);
    കണ്ണി_2 *n;
    വേണ്ടി (n = gnode(h, 0); n < limit; n++) {
      എങ്കിൽ (iscleared(g, gckeyN(n)))  /* unmarked key? */
        setempty(gval(n));  /* remove entry */
      എങ്കിൽ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


/*
** clear entries with unmarked values from all weaktables in list 'l' up
** to element 'f'
*/
സ്ഥിതം ശൂന്യം clearbyvalues (global_State *g, GCObject *l, GCObject *f) {
  വേണ്ടി (; l != f; l = gco2t(l)->gclist) {
    Table *h = gco2t(l);
    കണ്ണി_2 *n, *limit = gnodelast(h);
    unsigned പൂർണ്ണസംഖ്യ i;
    unsigned പൂർണ്ണസംഖ്യ asize = h->asize;
    വേണ്ടി (i = 0; i < asize; i++) {
      GCObject *o = gcvalarr(h, i);
      എങ്കിൽ (iscleared(g, o))  /* value was collected? */
        *getArrTag(h, i) = LUA_VEMPTY;  /* remove entry */
    }
    വേണ്ടി (n = gnode(h, 0); n < limit; n++) {
      എങ്കിൽ (iscleared(g, gcvalueN(gval(n))))  /* unmarked value? */
        setempty(gval(n));  /* remove entry */
      എങ്കിൽ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


സ്ഥിതം ശൂന്യം freeupval (lua_State *L, UpVal *uv) {
  എങ്കിൽ (upisopen(uv))
    luaF_unlinkupval(uv);
  luaM_free(L, uv);
}


സ്ഥിതം ശൂന്യം freeobj (lua_State *L, GCObject *o) {
  assert_code(l_mem newmem = gettotalbytes(G(L)) - objsize(o));
  തിരഞ്ഞെടുപ്പ് (o->tt) {
    അവസ്ഥ LUA_VPROTO:
      luaF_freeproto(L, gco2p(o));
      നിർത്തുക;
    അവസ്ഥ LUA_VUPVAL:
      freeupval(L, gco2upv(o));
      നിർത്തുക;
    അവസ്ഥ LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      luaM_freemem(L, cl, sizeLclosure(cl->nupvalues));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      luaM_freemem(L, cl, sizeCclosure(cl->nupvalues));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VTABLE:
      luaH_free(L, gco2t(o));
      നിർത്തുക;
    അവസ്ഥ LUA_VTHREAD:
      luaE_freethread(L, gco2th(o));
      നിർത്തുക;
    അവസ്ഥ LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      luaM_freemem(L, o, sizeudata(u->nuvalue, u->len));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      luaS_remove(L, ts);  /* remove it from hash table */
      luaM_freemem(L, ts, sizestrshr(cast_uint(ts->shrlen)));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      എങ്കിൽ (ts->shrlen == LSTRMEM)  /* must free external string? */
        (*ts->falloc)(ts->ud, ts->contents, ts->u.lnglen + 1, 0);
      luaM_freemem(L, ts, luaS_sizelngstr(ts->u.lnglen, ts->shrlen));
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0);
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
സ്ഥിതം GCObject **sweeplist (lua_State *L, GCObject **p, l_mem countin) {
  global_State *g = G(L);
  പൂർണ്ണസംഖ്യ ow = otherwhite(g);
  പൂർണ്ണസംഖ്യ white = luaC_white(g);  /* current white */
  ആകുമ്പോൾ (*p != ശൂന്യ && countin-- > 0) {
    GCObject *curr = *p;
    പൂർണ്ണസംഖ്യ marked = curr->marked;
    എങ്കിൽ (isdeadm(ow, marked)) {  /* is 'curr' dead? */
      *p = curr->next;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    അല്ലെങ്കിൽ {  /* change mark to 'white' and age to 'new' */
      curr->marked = cast_byte((marked & ~maskgcbits) | white | G_NEW);
      p = &curr->next;  /* go to next element */
    }
  }
  തിരികെ (*p == ശൂന്യ) ? ശൂന്യ : p;
}


/*
** sweep a list until a live object (or end of list)
*/
സ്ഥിതം GCObject **sweeptolive (lua_State *L, GCObject **p) {
  GCObject **old = p;
  ചെയ്യുക {
    p = sweeplist(L, p, 1);
  } ആകുമ്പോൾ (p == old);
  തിരികെ p;
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
സ്ഥിതം ശൂന്യം checkSizes (lua_State *L, global_State *g) {
  എങ്കിൽ (!g->gcemergency) {
    എങ്കിൽ (g->strt.nuse < g->strt.size / 4)  /* string table too big? */
      luaS_resize(L, g->strt.size / 2);
  }
}


/*
** Get the next udata to be finalized from the 'tobefnz' list, and
** link it back into the 'allgc' list.
*/
സ്ഥിതം GCObject *udata2finalize (global_State *g) {
  GCObject *o = g->tobefnz;  /* get first element */
  lua_assert(tofinalize(o));
  g->tobefnz = o->next;  /* remove it from 'tobefnz' list */
  o->next = g->allgc;  /* return it to 'allgc' list */
  g->allgc = o;
  resetbit(o->marked, FINALIZEDBIT);  /* object is "normal" again */
  എങ്കിൽ (issweepphase(g))
    makewhite(g, o);  /* "sweep" object */
  അല്ലെങ്കിൽ എങ്കിൽ (getage(o) == G_OLD1)
    g->firstold1 = o;  /* it is the first OLD1 object in the list */
  തിരികെ o;
}


സ്ഥിതം ശൂന്യം dothecall (lua_State *L, ശൂന്യം *ud) {
  UNUSED(ud);
  luaD_callnoyield(L, L->മുകൾ.p - 2, 0);
}


സ്ഥിതം ശൂന്യം GCTM (lua_State *L) {
  global_State *g = G(L);
  സ്ഥിരം TValue *tm;
  TValue v;
  lua_assert(!g->gcemergency);
  setgcovalue(L, &v, udata2finalize(g));
  tm = luaT_gettmbyobj(L, &v, TM_GC);
  എങ്കിൽ (!notm(tm)) {  /* is there a finalizer? */
    TStatus status;
    lu_byte oldah = L->allowhook;
    lu_byte oldgcstp  = g->gcstp;
    g->gcstp |= GCSTPGC;  /* avoid GC steps */
    L->allowhook = 0;  /* stop debug hooks during GC metamethod */
    setobj2s(L, L->മുകൾ.p++, tm);  /* push finalizer... */
    setobj2s(L, L->മുകൾ.p++, &v);  /* ... and its argument */
    L->ci->callstatus |= CIST_FIN;  /* will run a finalizer */
    status = luaD_pcall(L, dothecall, ശൂന്യ, savestack(L, L->മുകൾ.p - 2), 0);
    L->ci->callstatus &= ~CIST_FIN;  /* not running a finalizer anymore */
    L->allowhook = oldah;  /* restore hooks */
    g->gcstp = oldgcstp;  /* restore state */
    എങ്കിൽ (l_unlikely(status != LUA_OK)) {  /* error while running __gc? */
      luaE_warnerror(L, "__gc");
      L->മുകൾ.p--;  /* pops error object */
    }
  }
}


/*
** call all pending finalizers
*/
സ്ഥിതം ശൂന്യം callallpendingfinalizers (lua_State *L) {
  global_State *g = G(L);
  ആകുമ്പോൾ (g->tobefnz)
    GCTM(L);
}


/*
** find last 'next' field in list 'p' list (to add elements in its end)
*/
സ്ഥിതം GCObject **findlast (GCObject **p) {
  ആകുമ്പോൾ (*p != ശൂന്യ)
    p = &(*p)->next;
  തിരികെ p;
}


/*
** Move all unreachable objects (or 'all' objects) that need
** finalization from list 'finobj' to list 'tobefnz' (to be finalized).
** (Note that objects after 'finobjold1' cannot be white, so they
** don't need to be traversed. In incremental mode, 'finobjold1' is NULL,
** so the whole list is traversed.)
*/
സ്ഥിതം ശൂന്യം separatetobefnz (global_State *g, പൂർണ്ണസംഖ്യ all) {
  GCObject *curr;
  GCObject **p = &g->finobj;
  GCObject **lastnext = findlast(&g->tobefnz);
  ആകുമ്പോൾ ((curr = *p) != g->finobjold1) {  /* traverse all finalizable objects */
    lua_assert(tofinalize(curr));
    എങ്കിൽ (!(iswhite(curr) || all))  /* not being collected? */
      p = &curr->next;  /* don't bother with it */
    അല്ലെങ്കിൽ {
      എങ്കിൽ (curr == g->finobjsur)  /* removing 'finobjsur'? */
        g->finobjsur = curr->next;  /* correct it */
      *p = curr->next;  /* remove 'curr' from 'finobj' list */
      curr->next = *lastnext;  /* link at the end of 'tobefnz' list */
      *lastnext = curr;
      lastnext = &curr->next;
    }
  }
}


/*
** If pointer 'p' points to 'o', move it to the next element.
*/
സ്ഥിതം ശൂന്യം checkpointer (GCObject **p, GCObject *o) {
  എങ്കിൽ (o == *p)
    *p = o->next;
}


/*
** Correct pointers to objects inside 'allgc' list when
** object 'o' is being removed from the list.
*/
സ്ഥിതം ശൂന്യം correctpointers (global_State *g, GCObject *o) {
  checkpointer(&g->survival, o);
  checkpointer(&g->old1, o);
  checkpointer(&g->reallyold, o);
  checkpointer(&g->firstold1, o);
}


/*
** if object 'o' has a finalizer, remove it from 'allgc' list (must
** search the list to find it) and link it in 'finobj' list.
*/
ശൂന്യം luaC_checkfinalizer (lua_State *L, GCObject *o, Table *mt) {
  global_State *g = G(L);
  എങ്കിൽ (tofinalize(o) ||                 /* obj. is already marked... */
      gfasttm(g, mt, TM_GC) == ശൂന്യ ||    /* or has no finalizer... */
      (g->gcstp & GCSTPCLS))                   /* or closing state? */
    തിരികെ;  /* nothing to be done */
  അല്ലെങ്കിൽ {  /* move 'o' to 'finobj' list */
    GCObject **p;
    എങ്കിൽ (issweepphase(g)) {
      makewhite(g, o);  /* "sweep" object 'o' */
      എങ്കിൽ (g->sweepgc == &o->next)  /* should not remove 'sweepgc' object */
        g->sweepgc = sweeptolive(L, g->sweepgc);  /* change 'sweepgc' */
    }
    അല്ലെങ്കിൽ
      correctpointers(g, o);
    /* search for pointer pointing to 'o' */
    വേണ്ടി (p = &g->allgc; *p != o; p = &(*p)->next) { /* empty */ }
    *p = o->next;  /* remove 'o' from 'allgc' list */
    o->next = g->finobj;  /* link it in 'finobj' list */
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
സ്ഥിതം ശൂന്യം setpause (global_State *g) {
  l_mem threshold = applygcparam(g, PAUSE, g->GCmarked);
  l_mem debt = threshold - gettotalbytes(g);
  എങ്കിൽ (debt < 0) debt = 0;
  luaE_setdebt(g, debt);
}


/*
** Sweep a list of objects to enter generational mode.  Deletes dead
** objects and turns the non dead to old. All non-dead threads---which
** are now old---must be in a gray list. Everything else is not in a
** gray list. Open upvalues are also kept gray.
*/
സ്ഥിതം ശൂന്യം sweep2old (lua_State *L, GCObject **p) {
  GCObject *curr;
  global_State *g = G(L);
  ആകുമ്പോൾ ((curr = *p) != ശൂന്യ) {
    എങ്കിൽ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(isdead(g, curr));
      *p = curr->next;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    അല്ലെങ്കിൽ {  /* all surviving objects become old */
      setage(curr, G_OLD);
      എങ്കിൽ (curr->tt == LUA_VTHREAD) {  /* threads must be watched */
        lua_State *th = gco2th(curr);
        linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
      }
      അല്ലെങ്കിൽ എങ്കിൽ (curr->tt == LUA_VUPVAL && upisopen(gco2upv(curr)))
        set2gray(curr);  /* open upvalues are always gray */
      അല്ലെങ്കിൽ  /* everything else is black */
        nw2black(curr);
      p = &curr->next;  /* go to next element */
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
സ്ഥിതം GCObject **sweepgen (lua_State *L, global_State *g, GCObject **p,
                            GCObject *limit, GCObject **pfirstold1,
                            l_mem *paddedold) {
  സ്ഥിതം സ്ഥിരം lu_byte nextage[] = {
    G_SURVIVAL,  /* from G_NEW */
    G_OLD1,      /* from G_SURVIVAL */
    G_OLD1,      /* from G_OLD0 */
    G_OLD,       /* from G_OLD1 */
    G_OLD,       /* from G_OLD (do not change) */
    G_TOUCHED1,  /* from G_TOUCHED1 (do not change) */
    G_TOUCHED2   /* from G_TOUCHED2 (do not change) */
  };
  l_mem addedold = 0;
  പൂർണ്ണസംഖ്യ white = luaC_white(g);
  GCObject *curr;
  ആകുമ്പോൾ ((curr = *p) != limit) {
    എങ്കിൽ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(!isold(curr) && isdead(g, curr));
      *p = curr->next;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    അല്ലെങ്കിൽ {  /* correct mark and age */
      പൂർണ്ണസംഖ്യ age = getage(curr);
      എങ്കിൽ (age == G_NEW) {  /* new objects go back to white */
        പൂർണ്ണസംഖ്യ marked = curr->marked & ~maskgcbits;  /* erase GC bits */
        curr->marked = cast_byte(marked | G_SURVIVAL | white);
      }
      അല്ലെങ്കിൽ {  /* all other objects will be old, and so keep their color */
        lua_assert(age != G_OLD1);  /* advanced in 'markold' */
        setage(curr, nextage[age]);
        എങ്കിൽ (getage(curr) == G_OLD1) {
          addedold += objsize(curr);  /* bytes becoming old */
          എങ്കിൽ (*pfirstold1 == ശൂന്യ)
            *pfirstold1 = curr;  /* first OLD1 object in the list */
        }
      }
      p = &curr->next;  /* go to next element */
    }
  }
  *paddedold += addedold;
  തിരികെ p;
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
സ്ഥിതം GCObject **correctgraylist (GCObject **p) {
  GCObject *curr;
  ആകുമ്പോൾ ((curr = *p) != ശൂന്യ) {
    GCObject **next = getgclist(curr);
    എങ്കിൽ (iswhite(curr))
      പോകുക remove;  /* remove all white objects */
    അല്ലെങ്കിൽ എങ്കിൽ (getage(curr) == G_TOUCHED1) {  /* touched in this cycle? */
      lua_assert(isgray(curr));
      nw2black(curr);  /* make it black, for next barrier */
      setage(curr, G_TOUCHED2);
      പോകുക remain;  /* keep it in the list and go to next element */
    }
    അല്ലെങ്കിൽ എങ്കിൽ (curr->tt == LUA_VTHREAD) {
      lua_assert(isgray(curr));
      പോകുക remain;  /* keep non-white threads on the list */
    }
    അല്ലെങ്കിൽ {  /* everything else is removed */
      lua_assert(isold(curr));  /* young objects should be white here */
      എങ്കിൽ (getage(curr) == G_TOUCHED2)  /* advance from TOUCHED2... */
        setage(curr, G_OLD);  /* ... to OLD */
      nw2black(curr);  /* make object black (to be removed) */
      പോകുക remove;
    }
    remove: *p = *next; തുടരുക;
    remain: p = next; തുടരുക;
  }
  തിരികെ p;
}


/*
** Correct all gray lists, coalescing them into 'grayagain'.
*/
സ്ഥിതം ശൂന്യം correctgraylists (global_State *g) {
  GCObject **list = correctgraylist(&g->grayagain);
  *list = g->weak; g->weak = ശൂന്യ;
  list = correctgraylist(list);
  *list = g->allweak; g->allweak = ശൂന്യ;
  list = correctgraylist(list);
  *list = g->ephemeron; g->ephemeron = ശൂന്യ;
  correctgraylist(list);
}


/*
** Mark black 'OLD1' objects when starting a new young collection.
** Gray objects are already in some gray list, and so will be visited in
** the atomic step.
*/
സ്ഥിതം ശൂന്യം markold (global_State *g, GCObject *from, GCObject *to) {
  GCObject *p;
  വേണ്ടി (p = from; p != to; p = p->next) {
    എങ്കിൽ (getage(p) == G_OLD1) {
      lua_assert(!iswhite(p));
      setage(p, G_OLD);  /* now they are old */
      എങ്കിൽ (isblack(p))
        reallymarkobject(g, p);
    }
  }
}


/*
** Finish a young-generation collection.
*/
സ്ഥിതം ശൂന്യം finishgencycle (lua_State *L, global_State *g) {
  correctgraylists(g);
  checkSizes(L, g);
  g->gcstate = GCSpropagate;  /* skip restart */
  എങ്കിൽ (g->tobefnz != ശൂന്യ && !g->gcemergency && luaD_checkminstack(L))
    callallpendingfinalizers(L);
}


/*
** Shifts from a minor collection to major collections. It starts in
** the "sweep all" state to clear all objects, which are mostly black
** in generational mode.
*/
സ്ഥിതം ശൂന്യം minor2inc (lua_State *L, global_State *g, lu_byte kind) {
  g->GCmajorminor = g->GCmarked;  /* number of live bytes */
  g->gckind = kind;
  g->reallyold = g->old1 = g->survival = ശൂന്യ;
  g->finobjrold = g->finobjold1 = g->finobjsur = ശൂന്യ;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ checkminormajor (global_State *g) {
  l_mem limit = applygcparam(g, MINORMAJOR, g->GCmajorminor);
  എങ്കിൽ (limit == 0)
    തിരികെ 0;  /* special case: 'minormajor' 0 stops major collections */
  തിരികെ (g->GCmarked >= limit);
}

/*
** Does a young collection. First, mark 'OLD1' objects. Then does the
** atomic step. Then, check whether to continue in minor mode. If so,
** sweep all lists and advance pointers. Finally, finish the collection.
*/
സ്ഥിതം ശൂന്യം youngcollection (lua_State *L, global_State *g) {
  l_mem addedold1 = 0;
  l_mem marked = g->GCmarked;  /* preserve 'g->GCmarked' */
  GCObject **psurvival;  /* to point to first non-dead survival object */
  GCObject *dummy;  /* dummy out parameter to 'sweepgen' */
  lua_assert(g->gcstate == GCSpropagate);
  എങ്കിൽ (g->firstold1) {  /* are there regular OLD1 objects? */
    markold(g, g->firstold1, g->reallyold);  /* mark them */
    g->firstold1 = ശൂന്യ;  /* no more OLD1 objects (for now) */
  }
  markold(g, g->finobj, g->finobjrold);
  markold(g, g->tobefnz, ശൂന്യ);

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
  dummy = ശൂന്യ;  /* no 'firstold1' optimization for 'finobj' lists */
  psurvival = sweepgen(L, g, &g->finobj, g->finobjsur, &dummy, &addedold1);
  /* sweep 'survival' */
  sweepgen(L, g, psurvival, g->finobjold1, &dummy, &addedold1);
  g->finobjrold = g->finobjold1;
  g->finobjold1 = *psurvival;  /* 'survival' survivals are old now */
  g->finobjsur = g->finobj;  /* all news are survivals */

  sweepgen(L, g, &g->tobefnz, ശൂന്യ, &dummy, &addedold1);

  /* keep total number of added old1 bytes */
  g->GCmarked = marked + addedold1;

  /* decide whether to shift to major mode */
  എങ്കിൽ (checkminormajor(g)) {
    minor2inc(L, g, KGC_GENMAJOR);  /* go to major mode */
    g->GCmarked = 0;  /* avoid pause in first major cycle (see 'setpause') */
  }
  അല്ലെങ്കിൽ
    finishgencycle(L, g);  /* still in minor mode; finish it */
}


/*
** Clears all gray lists, sweeps objects, and prepare sublists to enter
** generational mode. The sweeps remove dead objects and turn all
** surviving objects to old. Threads go back to 'grayagain'; everything
** else is turned black (not in any gray list).
*/
സ്ഥിതം ശൂന്യം atomic2gen (lua_State *L, global_State *g) {
  cleargraylists(g);
  /* sweep all elements making them old */
  g->gcstate = GCSswpallgc;
  sweep2old(L, &g->allgc);
  /* everything alive now is old */
  g->reallyold = g->old1 = g->survival = g->allgc;
  g->firstold1 = ശൂന്യ;  /* there are no OLD1 objects anywhere */

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
സ്ഥിതം ശൂന്യം setminordebt (global_State *g) {
  luaE_setdebt(g, applygcparam(g, MINORMUL, g->GCmajorminor));
}


/*
** Enter generational mode. Must go until the end of an atomic cycle
** to ensure that all objects are correctly marked and weak tables
** are cleared. Then, turn all objects into old and finishes the
** collection.
*/
സ്ഥിതം ശൂന്യം entergen (lua_State *L, global_State *g) {
  luaC_runtilstate(L, GCSpause, 1);  /* prepare to start a new cycle */
  luaC_runtilstate(L, GCSpropagate, 1);  /* start new cycle */
  atomic(L);  /* propagates all and then do the atomic stuff */
  atomic2gen(L, g);
  setminordebt(g);  /* set debt assuming next cycle will be minor */
}


/*
** Change collector mode to 'newmode'.
*/
ശൂന്യം luaC_changemode (lua_State *L, പൂർണ്ണസംഖ്യ newmode) {
  global_State *g = G(L);
  എങ്കിൽ (g->gckind == KGC_GENMAJOR)  /* doing major collections? */
    g->gckind = KGC_INC;  /* already incremental but in name */
  എങ്കിൽ (newmode != g->gckind) {  /* does it need to change? */
    എങ്കിൽ (newmode == KGC_INC)  /* entering incremental mode? */
      minor2inc(L, g, KGC_INC);  /* entering incremental mode */
    അല്ലെങ്കിൽ {
      lua_assert(newmode == KGC_GENMINOR);
      entergen(L, g);
    }
  }
}


/*
** Does a full collection in generational mode.
*/
സ്ഥിതം ശൂന്യം fullgen (lua_State *L, global_State *g) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ checkmajorminor (lua_State *L, global_State *g) {
  എങ്കിൽ (g->gckind == KGC_GENMAJOR) {  /* generational mode? */
    l_mem numbytes = gettotalbytes(g);
    l_mem addedbytes = numbytes - g->GCmajorminor;
    l_mem limit = (addedbytes < 0) ? 0
                                   : applygcparam(g, MAJORMINOR, addedbytes);
    l_mem tobecollected = numbytes - g->GCmarked;
    എങ്കിൽ (tobecollected > limit) {
      atomic2gen(L, g);  /* return to generational mode */
      setminordebt(g);
      തിരികെ 1;  /* exit incremental collection */
    }
  }
  g->GCmajorminor = g->GCmarked;  /* prepare for next collection */
  തിരികെ 0;  /* stay doing incremental collections */
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
സ്ഥിതം ശൂന്യം entersweep (lua_State *L) {
  global_State *g = G(L);
  g->gcstate = GCSswpallgc;
  lua_assert(g->sweepgc == ശൂന്യ);
  g->sweepgc = sweeptolive(L, &g->allgc);
}


/*
** Delete all objects in list 'p' until (but not including) object
** 'limit'.
*/
സ്ഥിതം ശൂന്യം deletelist (lua_State *L, GCObject *p, GCObject *limit) {
  ആകുമ്പോൾ (p != limit) {
    GCObject *next = p->next;
    freeobj(L, p);
    p = next;
  }
}


/*
** Call all finalizers of the objects in the given Lua state, and
** then free all objects, except for the main thread.
*/
ശൂന്യം luaC_freeallobjects (lua_State *L) {
  global_State *g = G(L);
  g->gcstp = GCSTPCLS;  /* no extra finalizers after here */
  luaC_changemode(L, KGC_INC);
  separatetobefnz(g, 1);  /* separate all objects with finalizers */
  lua_assert(g->finobj == ശൂന്യ);
  callallpendingfinalizers(L);
  deletelist(L, g->allgc, obj2gco(mainthread(g)));
  lua_assert(g->finobj == ശൂന്യ);  /* no new finalizers */
  deletelist(L, g->fixedgc, ശൂന്യ);  /* collect fixed objects */
  lua_assert(g->strt.nuse == 0);
}


സ്ഥിതം ശൂന്യം atomic (lua_State *L) {
  global_State *g = G(L);
  GCObject *origweak, *origall;
  GCObject *grayagain = g->grayagain;  /* save original list */
  g->grayagain = ശൂന്യ;
  lua_assert(g->ephemeron == ശൂന്യ && g->weak == ശൂന്യ);
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
  g->gray = grayagain;
  propagateall(g);  /* traverse 'grayagain' list */
  convergeephemerons(g);
  /* at this point, all strongly accessible objects are marked. */
  /* Clear values from weak tables, before checking finalizers */
  clearbyvalues(g, g->weak, ശൂന്യ);
  clearbyvalues(g, g->allweak, ശൂന്യ);
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
  lua_assert(g->gray == ശൂന്യ);
}


/*
** Do a sweep step. The normal case (not fast) sweeps at most GCSWEEPMAX
** elements. The fast case sweeps the whole list.
*/
സ്ഥിതം ശൂന്യം sweepstep (lua_State *L, global_State *g,
                       lu_byte nextstate, GCObject **nextlist, പൂർണ്ണസംഖ്യ fast) {
  എങ്കിൽ (g->sweepgc)
    g->sweepgc = sweeplist(L, g->sweepgc, fast ? MAX_LMEM : GCSWEEPMAX);
  അല്ലെങ്കിൽ {  /* enter next state */
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

#define step2pause	-3  /* finished collection; entered pause state */
#define atomicstep	-2  /* atomic step */
#define step2minor	-1  /* moved to minor collections */


സ്ഥിതം l_mem singlestep (lua_State *L, പൂർണ്ണസംഖ്യ fast) {
  global_State *g = G(L);
  l_mem stepresult;
  lua_assert(!g->gcstopem);  /* collector is not reentrant */
  g->gcstopem = 1;  /* no emergency collections while collecting */
  തിരഞ്ഞെടുപ്പ് (g->gcstate) {
    അവസ്ഥ GCSpause: {
      restartcollection(g);
      g->gcstate = GCSpropagate;
      stepresult = 1;
      നിർത്തുക;
    }
    അവസ്ഥ GCSpropagate: {
      എങ്കിൽ (fast || g->gray == ശൂന്യ) {
        g->gcstate = GCSenteratomic;  /* finish propagate phase */
        stepresult = 1;
      }
      അല്ലെങ്കിൽ
        stepresult = propagatemark(g);  /* traverse one gray object */
      നിർത്തുക;
    }
    അവസ്ഥ GCSenteratomic: {
      atomic(L);
      എങ്കിൽ (checkmajorminor(L, g))
        stepresult = step2minor;
      അല്ലെങ്കിൽ {
        entersweep(L);
        stepresult = atomicstep;
      }
      നിർത്തുക;
    }
    അവസ്ഥ GCSswpallgc: {  /* sweep "regular" objects */
      sweepstep(L, g, GCSswpfinobj, &g->finobj, fast);
      stepresult = GCSWEEPMAX;
      നിർത്തുക;
    }
    അവസ്ഥ GCSswpfinobj: {  /* sweep objects with finalizers */
      sweepstep(L, g, GCSswptobefnz, &g->tobefnz, fast);
      stepresult = GCSWEEPMAX;
      നിർത്തുക;
    }
    അവസ്ഥ GCSswptobefnz: {  /* sweep objects to be finalized */
      sweepstep(L, g, GCSswpend, ശൂന്യ, fast);
      stepresult = GCSWEEPMAX;
      നിർത്തുക;
    }
    അവസ്ഥ GCSswpend: {  /* finish sweeps */
      checkSizes(L, g);
      g->gcstate = GCScallfin;
      stepresult = GCSWEEPMAX;
      നിർത്തുക;
    }
    അവസ്ഥ GCScallfin: {  /* call finalizers */
      എങ്കിൽ (g->tobefnz && !g->gcemergency && luaD_checkminstack(L)) {
        g->gcstopem = 0;  /* ok collections during finalizers */
        GCTM(L);  /* call one finalizer */
        stepresult = CWUFIN;
      }
      അല്ലെങ്കിൽ {  /* no more finalizers or emergency mode or not enough stack
                 to run finalizers */
        g->gcstate = GCSpause;  /* finish collection */
        stepresult = step2pause;
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0); തിരികെ 0;
  }
  g->gcstopem = 0;
  തിരികെ stepresult;
}


/*
** Advances the garbage collector until it reaches the given state.
** (The option 'fast' is only for testing; in normal code, 'fast'
** here is always true.)
*/
ശൂന്യം luaC_runtilstate (lua_State *L, പൂർണ്ണസംഖ്യ state, പൂർണ്ണസംഖ്യ fast) {
  global_State *g = G(L);
  lua_assert(g->gckind == KGC_INC);
  ആകുമ്പോൾ (state != g->gcstate)
    singlestep(L, fast);
}



/*
** Performs a basic incremental step. The step size is
** converted from bytes to "units of work"; then the function loops
** running single steps until adding that many units of work or
** finishing a cycle (pause state). Finally, it sets the debt that
** controls when next step will be performed.
*/
സ്ഥിതം ശൂന്യം incstep (lua_State *L, global_State *g) {
  l_mem stepsize = applygcparam(g, STEPSIZE, 100);
  l_mem work2do = applygcparam(g, STEPMUL, stepsize / cast_int(വലുപ്പം(ശൂന്യം*)));
  l_mem stres;
  പൂർണ്ണസംഖ്യ fast = (work2do == 0);  /* special case: do a full collection */
  ചെയ്യുക {  /* repeat until enough work */
    stres = singlestep(L, fast);  /* perform one single step */
    എങ്കിൽ (stres == step2minor)  /* returned to minor collections? */
      തിരികെ;  /* nothing else to be done here */
    അല്ലെങ്കിൽ എങ്കിൽ (stres == step2pause || (stres == atomicstep && !fast))
      നിർത്തുക;  /* end of cycle or atomic */
    അല്ലെങ്കിൽ
      work2do -= stres;
  } ആകുമ്പോൾ (fast || work2do > 0);
  എങ്കിൽ (g->gcstate == GCSpause)
    setpause(g);  /* pause until next cycle */
  അല്ലെങ്കിൽ
    luaE_setdebt(g, stepsize);
}


#എങ്കിൽ !defined(luai_tracegc)
#define luai_tracegc(L,f)		((ശൂന്യം)0)
#endif

/*
** Performs a basic GC step if collector is running. (If collector was
** stopped by the user, set a reasonable debt to avoid it being called
** at every single check.)
*/
ശൂന്യം luaC_step (lua_State *L) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  എങ്കിൽ (!gcrunning(g)) {  /* not running? */
    എങ്കിൽ (g->gcstp & GCSTPUSR)  /* stopped by the user? */
      luaE_setdebt(g, 20000);
  }
  അല്ലെങ്കിൽ {
    luai_tracegc(L, 1);  /* for internal debugging */
    തിരഞ്ഞെടുപ്പ് (g->gckind) {
      അവസ്ഥ KGC_INC: അവസ്ഥ KGC_GENMAJOR:
        incstep(L, g);
        നിർത്തുക;
      അവസ്ഥ KGC_GENMINOR:
        youngcollection(L, g);
        setminordebt(g);
        നിർത്തുക;
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
സ്ഥിതം ശൂന്യം fullinc (lua_State *L, global_State *g) {
  എങ്കിൽ (keepinvariant(g))  /* black objects? */
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
ശൂന്യം luaC_fullgc (lua_State *L, പൂർണ്ണസംഖ്യ isemergency) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  g->gcemergency = cast_byte(isemergency);  /* set flag */
  തിരഞ്ഞെടുപ്പ് (g->gckind) {
    അവസ്ഥ KGC_GENMINOR: fullgen(L, g); നിർത്തുക;
    അവസ്ഥ KGC_INC: fullinc(L, g); നിർത്തുക;
    അവസ്ഥ KGC_GENMAJOR:
      g->gckind = KGC_INC;
      fullinc(L, g);
      g->gckind = KGC_GENMAJOR;
      നിർത്തുക;
  }
  g->gcemergency = 0;
}

/* }====================================================== */


