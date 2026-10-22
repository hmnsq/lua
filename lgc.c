/*
** $Id: lgc.c $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#నిర్వచించు lgc_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"

#అన్తర్భావ <string.h>


#అన్తర్భావ "lua.h"

#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "ltm.h"


/*
** Maximum number of elements to sweep in each single step.
** (Large enough to dissipate fixed overheads but small enough
** to allow small steps for the collector.)
*/
#నిర్వచించు GCSWEEPMAX	20


/*
** Cost (in work units) of running one finalizer.
*/
#నిర్వచించు CWUFIN	10


/* mask with all color bits */
#నిర్వచించు maskcolors	(bitmask(BLACKBIT) | WHITEBITS)

/* mask with all GC bits */
#నిర్వచించు maskgcbits      (maskcolors | AGEBITS)


/* macro to erase all color bits then set only the current white bit */
#నిర్వచించు makewhite(g,x)	\
  (x->marked = cast_byte((x->marked & ~maskcolors) | luaC_white(g)))

/* make an object gray (neither white nor black) */
#నిర్వచించు set2gray(x)	resetbits(x->marked, maskcolors)


/* make an object black (coming from any color) */
#నిర్వచించు set2black(x)  \
  (x->marked = cast_byte((x->marked & ~WHITEBITS) | bitmask(BLACKBIT)))


#నిర్వచించు valiswhite(x)   (iscollectable(x) && iswhite(gcvalue(x)))

#నిర్వచించు keyiswhite(n)   (keyiscollectable(n) && iswhite(gckey(n)))


/*
** Protected access to objects in values
*/
#నిర్వచించు gcvalueN(o)     (iscollectable(o) ? gcvalue(o) : ఖాళీ)


/*
** Access to collectable objects in array part of tables
*/
#నిర్వచించు gcvalarr(t,i)  \
	((*get_Arr_ట్యాగ్(t,i) & బిట్టు_ISCOLLECTABLE) ? getArrVal(t,i)->gc : ఖాళీ)


#నిర్వచించు markvalue(g,o) { checkliveness(mainthread(g),o); \
  ఒకవేళ (valiswhite(o)) reallymarkobject(g,gcvalue(o)); }

#నిర్వచించు markkey(g, n)	{ ఒకవేళ keyiswhite(n) reallymarkobject(g,gckey(n)); }

#నిర్వచించు markobject(g,t)	{ ఒకవేళ (iswhite(t)) reallymarkobject(g, obj2gco(t)); }

/*
** mark an object that can be NULL (either because it is really optional,
** or it was stripped as debug info, or inside an uncompleted structure)
*/
#నిర్వచించు markobjectN(g,t)	{ ఒకవేళ (t) markobject(g,t); }


స్థిర శూన్య reallymarkobject (global_State *g, GCObject *o);
స్థిర శూన్య atomic (lua_State *L);
స్థిర శూన్య entersweep (lua_State *L);


/*
** {======================================================
** Generic functions
** =======================================================
*/


/*
** one after last element in a hash array
*/
#నిర్వచించు gnodelast(h)	gnode(h, cast_sizet(sizenode(h)))


స్థిర l_mem objsize (GCObject *o) {
  lu_mem res;
  ఎంపిక (o->tt) {
    స్థితి LUA_VTABLE: {
      res = lua_H_కొలత(gco2t(o));
      ఆపు;
    }
    స్థితి LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      res = కొలత_Lclosure(cl->nupvalues);
      ఆపు;
    }
    స్థితి LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      res = కొలత_Cclosure(cl->nupvalues);
      ఆపు;
    }
    స్థితి LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      res = sizeudata(u->nuvalue, u->len);
      ఆపు;
    }
    స్థితి LUA_VPROTO: {
      res = luaF_protosize(gco2p(o));
      ఆపు;
    }
    స్థితి LUA_VTHREAD: {
      res = luaE_threadsize(gco2th(o));
      ఆపు;
    }
    స్థితి LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      res = sizestrshr(cast_uint(ts->shrlen));
      ఆపు;
    }
    స్థితి LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      res = luaS_sizelngstr(ts->u.lnglen, ts->shrlen);
      ఆపు;
    }
    స్థితి LUA_VUPVAL: {
      res = పరిమాణం(పైకి_Val);
      ఆపు;
    }
    అప్రమేయ: res = 0; lua_assert(0);
  }
  తిరిగి cast(l_mem, res);
}


స్థిర GCObject **getgclist (GCObject *o) {
  ఎంపిక (o->tt) {
    స్థితి LUA_VTABLE: తిరిగి &gco2t(o)->gclist;
    స్థితి LUA_VLCL: తిరిగి &gco2lcl(o)->gclist;
    స్థితి LUA_VCCL: తిరిగి &gco2ccl(o)->gclist;
    స్థితి LUA_VTHREAD: తిరిగి &gco2th(o)->gclist;
    స్థితి LUA_VPROTO: తిరిగి &gco2p(o)->gclist;
    స్థితి LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      lua_assert(u->nuvalue > 0);
      తిరిగి &u->gclist;
    }
    అప్రమేయ: lua_assert(0); తిరిగి 0;
  }
}


/*
** Link a collectable object 'o' with a known type into the list 'p'.
** (Must be a macro to access the 'gclist' field in different types.)
*/
#నిర్వచించు linkgclist(o,p)	linkgclist_(obj2gco(o), &(o)->gclist, &(p))

స్థిర శూన్య linkgclist_ (GCObject *o, GCObject **pnext, GCObject **list) {
  lua_assert(!isgray(o));  /* cannot be in a gray list */
  *pnext = *list;
  *list = o;
  set2gray(o);  /* now it is */
}


/*
** Link a generic collectable object 'o' into the list 'p'.
*/
#నిర్వచించు linkobjgclist(o,p) linkgclist_(obj2gco(o), getgclist(o), &(p))



/*
** Clear keys for empty entries in tables. If entry is empty, mark its
** entry as dead. This allows the collection of the key, but keeps its
** entry in the table: its removal could break a chain and could break
** a table traversal.  Other places never manipulate dead keys, because
** its associated empty value is enough to signal that the entry is
** logically empty.
*/
స్థిర శూన్య clearkey (కణుపు_2 *n) {
  lua_assert(isempty(gval(n)));
  ఒకవేళ (keyiscollectable(n))
    setdeadkey(n);  /* unused key; remove it */
}


/*
** tells whether a key or value can be cleared from a weak
** table. Non-collectable objects are never removed from weak
** tables. Strings behave as 'values', so are never removed too. for
** other objects: if really collected, cannot keep them; for objects
** being finalized, keep them in keys, but not in values
*/
స్థిర పూర్ణాంకం iscleared (global_State *g, స్థిరరాశి GCObject *o) {
  ఒకవేళ (o == ఖాళీ) తిరిగి 0;  /* non-collectable value */
  లేకపోతే ఒకవేళ (novariant(o->tt) == LUA_TSTRING) {
    markobject(g, o);  /* strings are 'values', so are never weak */
    తిరిగి 0;
  }
  లేకపోతే తిరిగి iswhite(o);
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
శూన్య luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v) {
  global_State *g = G(L);
  lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
  ఒకవేళ (keepinvariant(g)) {  /* must keep invariant? */
    reallymarkobject(g, v);  /* restore invariant */
    ఒకవేళ (isold(o)) {
      lua_assert(!isold(v));  /* white object could not be old */
      setage(v, G_OLD0);  /* restore generational invariant */
    }
  }
  లేకపోతే {  /* sweep phase */
    lua_assert(issweepphase(g));
    ఒకవేళ (g->gckind != KGC_GENMINOR)  /* incremental mode? */
      makewhite(g, o);  /* mark 'o' as white to avoid other barriers */
  }
}


/*
** barrier that moves collector backward, that is, mark the black object
** pointing to a white object as gray again.
*/
శూన్య luaC_barrierback_ (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(isblack(o) && !isdead(g, o));
  lua_assert((g->gckind != KGC_GENMINOR)
          || (isold(o) && getage(o) != G_TOUCHED1));
  ఒకవేళ (getage(o) == G_TOUCHED2)  /* already in gray list? */
    set2gray(o);  /* make it gray to become touched1 */
  లేకపోతే  /* link it in 'grayagain' and paint it gray */
    linkobjgclist(o, g->grayagain);
  ఒకవేళ (isold(o))  /* generational mode? */
    setage(o, G_TOUCHED1);  /* touched in current cycle */
}


శూన్య luaC_fix (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(g->allgc == o);  /* object must be 1st in 'allgc' list! */
  set2gray(o);  /* they will be gray forever */
  setage(o, G_OLD);  /* and old forever */
  g->allgc = o->తదుపరి;  /* remove object from 'allgc' list */
  o->తదుపరి = g->fixedgc;  /* link it to 'fixedgc' list */
  g->fixedgc = o;
}


/*
** create a new collectable object (with given type, size, and offset)
** and link it to 'allgc' list.
*/
GCObject *luaC_newobjdt (lua_State *L, lu_byte tt, size_t sz, size_t స్థానభ్రంశం) {
  global_State *g = G(L);
  అక్షరం *p = cast_charp(luaM_newobject(L, novariant(tt), sz));
  GCObject *o = cast(GCObject *, p + స్థానభ్రంశం);
  o->marked = luaC_white(g);
  o->tt = tt;
  o->తదుపరి = g->allgc;
  g->allgc = o;
  తిరిగి o;
}


/*
** create a new collectable object with no offset.
*/
GCObject *luaC_newobj (lua_State *L, lu_byte tt, size_t sz) {
  తిరిగి luaC_newobjdt(L, tt, sz, 0);
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
స్థిర శూన్య reallymarkobject (global_State *g, GCObject *o) {
  g->GCmarked += objsize(o);
  ఎంపిక (o->tt) {
    స్థితి LUA_VSHRSTR:
    స్థితి LUA_VLNGSTR: {
      set2black(o);  /* nothing to visit */
      ఆపు;
    }
    స్థితి LUA_VUPVAL: {
      పైకి_Val *uv = gco2upv(o);
      ఒకవేళ (upisopen(uv))
        set2gray(uv);  /* open upvalues are kept gray */
      లేకపోతే
        set2black(uv);  /* closed upvalues are visited here */
      markvalue(g, uv->v.p);  /* mark its content */
      ఆపు;
    }
    స్థితి LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      ఒకవేళ (u->nuvalue == 0) {  /* no user values? */
        markobjectN(g, u->metatable);  /* mark its metatable */
        set2black(u);  /* nothing else to mark */
        ఆపు;
      }
      /* else... */
    }  /* FALLTHROUGH */
    స్థితి LUA_VLCL: స్థితి LUA_VCCL: స్థితి LUA_VTABLE:
    స్థితి LUA_VTHREAD: స్థితి LUA_VPROTO: {
      linkobjgclist(o, g->బూడిద);  /* to be visited later */
      ఆపు;
    }
    అప్రమేయ: lua_assert(0); ఆపు;
  }
}


/*
** mark metamethods for basic types
*/
స్థిర శూన్య markmt (global_State *g) {
  పూర్ణాంకం i;
  కొరకు (i=0; i < LUA_NUMTYPES; i++)
    markobjectN(g, g->mt[i]);
}


/*
** mark all objects in list of being-finalized
*/
స్థిర శూన్య markbeingfnz (global_State *g) {
  GCObject *o;
  కొరకు (o = g->tobefnz; o != ఖాళీ; o = o->తదుపరి)
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
స్థిర శూన్య remarkupvals (global_State *g) {
  lua_State *thread;
  lua_State **p = &g->twups;
  అయితే ((thread = *p) != ఖాళీ) {
    ఒకవేళ (!iswhite(thread) && thread->openupval != ఖాళీ)
      p = &thread->twups;  /* keep marked thread with upvalues in the list */
    లేకపోతే {  /* thread is not marked or without upvalues */
      పైకి_Val *uv;
      lua_assert(!isold(thread) || thread->openupval == ఖాళీ);
      *p = thread->twups;  /* remove thread from the list */
      thread->twups = thread;  /* mark that it is out of list */
      కొరకు (uv = thread->openupval; uv != ఖాళీ; uv = uv->u.open.తదుపరి) {
        lua_assert(getage(uv) <= getage(thread));
        ఒకవేళ (!iswhite(uv)) {  /* upvalue already visited? */
          lua_assert(upisopen(uv) && isgray(uv));
          markvalue(g, uv->v.p);  /* mark its value */
        }
      }
    }
  }
}


స్థిర శూన్య cleargraylists (global_State *g) {
  g->బూడిద = g->grayagain = ఖాళీ;
  g->weak = g->allweak = g->ephemeron = ఖాళీ;
}


/*
** mark root set and reset all gray lists, to start a new collection.
** 'GCmarked' is initialized to count the total number of live bytes
** during a cycle.
*/
స్థిర శూన్య restartcollection (global_State *g) {
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
స్థిర శూన్య genlink (global_State *g, GCObject *o) {
  lua_assert(isblack(o));
  ఒకవేళ (getage(o) == G_TOUCHED1) {  /* touched in this cycle? */
    linkobjgclist(o, g->grayagain);  /* link it back in 'grayagain' */
  }  /* everything else do not need to be linked back */
  లేకపోతే ఒకవేళ (getage(o) == G_TOUCHED2)
    setage(o, G_OLD);  /* advance age */
}


/*
** Traverse a table with weak values and link it to proper list. During
** propagate phase, keep it in 'grayagain' list, to be revisited in the
** atomic phase. In the atomic phase, if table has any white value,
** put it in 'weak' list, to be cleared; otherwise, call 'genlink'
** to check table age in generational mode.
*/
స్థిర శూన్య traverseweakvalue (global_State *g, పట్టిక *h) {
  కణుపు_2 *n, *హద్దు = gnodelast(h);
  /* if there is array part, assume it may have white values (it is not
     worth traversing it now just to check) */
  పూర్ణాంకం hasclears = (h->asize > 0);
  కొరకు (n = gnode(h, 0); n < హద్దు; n++) {  /* traverse hash part */
    ఒకవేళ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    లేకపోతే {
      lua_assert(!keyisnil(n));
      markkey(g, n);
      ఒకవేళ (!hasclears && iscleared(g, gcvalueN(gval(n))))  /* a white value? */
        hasclears = 1;  /* table will have to be cleared */
    }
  }
  ఒకవేళ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  లేకపోతే ఒకవేళ (hasclears)
      linkgclist(h, g->weak);  /* has to be cleared later */
  లేకపోతే
    genlink(g, obj2gco(h));
}


/*
** Traverse the array part of a table.
*/
స్థిర పూర్ణాంకం traversearray (global_State *g, పట్టిక *h) {
  గుర్తులేని asize = h->asize;
  పూర్ణాంకం marked = 0;  /* true if some object is marked in this traversal */
  గుర్తులేని i;
  కొరకు (i = 0; i < asize; i++) {
    GCObject *o = gcvalarr(h, i);
    ఒకవేళ (o != ఖాళీ && iswhite(o)) {
      marked = 1;
      reallymarkobject(g, o);
    }
  }
  తిరిగి marked;
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
స్థిర పూర్ణాంకం traverseephemeron (global_State *g, పట్టిక *h, పూర్ణాంకం inv) {
  పూర్ణాంకం hasclears = 0;  /* true if table has white keys */
  పూర్ణాంకం hasww = 0;  /* true if table has entry "white-key -> white-value" */
  గుర్తులేని పూర్ణాంకం i;
  గుర్తులేని పూర్ణాంకం nsize = sizenode(h);
  పూర్ణాంకం marked = traversearray(g, h);  /* traverse array part */
  /* traverse hash part; if 'inv', traverse descending
     (see 'convergeephemerons') */
  కొరకు (i = 0; i < nsize; i++) {
    కణుపు_2 *n = inv ? gnode(h, nsize - 1 - i) : gnode(h, i);
    ఒకవేళ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    లేకపోతే ఒకవేళ (iscleared(g, gckeyN(n))) {  /* key is not marked (yet)? */
      hasclears = 1;  /* table must be cleared */
      ఒకవేళ (valiswhite(gval(n)))  /* value not marked yet? */
        hasww = 1;  /* white-white entry */
    }
    లేకపోతే ఒకవేళ (valiswhite(gval(n))) {  /* value not marked yet? */
      marked = 1;
      reallymarkobject(g, gcvalue(gval(n)));  /* mark it now */
    }
  }
  /* link table into proper list */
  ఒకవేళ (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  లేకపోతే ఒకవేళ (hasww)  /* table has white->white entries? */
    linkgclist(h, g->ephemeron);  /* have to propagate again */
  లేకపోతే ఒకవేళ (hasclears)  /* table has white keys? */
    linkgclist(h, g->allweak);  /* may have to clean white keys */
  లేకపోతే
    genlink(g, obj2gco(h));  /* check whether collector still needs to see it */
  తిరిగి marked;
}


స్థిర శూన్య traversestrongtable (global_State *g, పట్టిక *h) {
  కణుపు_2 *n, *హద్దు = gnodelast(h);
  traversearray(g, h);
  కొరకు (n = gnode(h, 0); n < హద్దు; n++) {  /* traverse hash part */
    ఒకవేళ (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    లేకపోతే {
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
స్థిర పూర్ణాంకం getmode (global_State *g, పట్టిక *h) {
  స్థిరరాశి TValue *బహుళకం = gfasttm(g, h->metatable, TM_బహుళకం);
  ఒకవేళ (బహుళకం == ఖాళీ || !ttisstring(బహుళకం))
    తిరిగి 0;  /* ignore non-string modes */
  లేకపోతే {
    స్థిరరాశి అక్షరం *smode = getstr(tsvalue(బహుళకం));
    స్థిరరాశి అక్షరం *weakkey = strchr(smode, 'k');
    స్థిరరాశి అక్షరం *weakvalue = strchr(smode, 'v');
    తిరిగి ((weakkey != ఖాళీ) << 1) | (weakvalue != ఖాళీ);
  }
}


స్థిర l_mem traversetable (global_State *g, పట్టిక *h) {
  markobjectN(g, h->metatable);
  ఎంపిక (getmode(g, h)) {
    స్థితి 0:  /* not weak */
      traversestrongtable(g, h);
      ఆపు;
    స్థితి 1:  /* weak values */
      traverseweakvalue(g, h);
      ఆపు;
    స్థితి 2:  /* weak keys */
      traverseephemeron(g, h, 0);
      ఆపు;
    స్థితి 3:  /* all weak; nothing to traverse */
      ఒకవేళ (g->gcstate == GCSpropagate)
        linkgclist(h, g->grayagain);  /* must visit again its metatable */
      లేకపోతే
        linkgclist(h, g->allweak);  /* must clear collected entries */
      ఆపు;
  }
  తిరిగి cast(l_mem, 1 + 2*sizenode(h) + h->asize);
}


స్థిర l_mem traverseudata (global_State *g, Udata *u) {
  పూర్ణాంకం i;
  markobjectN(g, u->metatable);  /* mark its metatable */
  కొరకు (i = 0; i < u->nuvalue; i++)
    markvalue(g, &u->uv[i].uv);
  genlink(g, obj2gco(u));
  తిరిగి 1 + u->nuvalue;
}


/*
** Traverse a prototype. (While a prototype is being build, its
** arrays can be larger than needed; the extra slots are filled with
** NULL, so the use of 'markobjectN')
*/
స్థిర l_mem traverseproto (global_State *g, Proto *f) {
  పూర్ణాంకం i;
  markobjectN(g, f->ఆధారం);
  కొరకు (i = 0; i < f->sizek; i++)  /* mark literals */
    markvalue(g, &f->k[i]);
  కొరకు (i = 0; i < f->sizeupvalues; i++)  /* mark upvalue names */
    markobjectN(g, f->upvalues[i].name);
  కొరకు (i = 0; i < f->sizep; i++)  /* mark nested protos */
    markobjectN(g, f->p[i]);
  కొరకు (i = 0; i < f->sizelocvars; i++)  /* mark local-variable names */
    markobjectN(g, f->locvars[i].varname);
  తిరిగి 1 + f->sizek + f->sizeupvalues + f->sizep + f->sizelocvars;
}


స్థిర l_mem traverseCclosure (global_State *g, CClosure *cl) {
  పూర్ణాంకం i;
  కొరకు (i = 0; i < cl->nupvalues; i++)  /* mark its upvalues */
    markvalue(g, &cl->upvalue[i]);
  తిరిగి 1 + cl->nupvalues;
}

/*
** Traverse a Lua closure, marking its prototype and its upvalues.
** (Both can be NULL while closure is being created.)
*/
స్థిర l_mem traverseLclosure (global_State *g, LClosure *cl) {
  పూర్ణాంకం i;
  markobjectN(g, cl->p);  /* mark its prototype */
  కొరకు (i = 0; i < cl->nupvalues; i++) {  /* visit its upvalues */
    పైకి_Val *uv = cl->upvals[i];
    markobjectN(g, uv);  /* mark upvalue */
  }
  తిరిగి 1 + cl->nupvalues;
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
స్థిర l_mem traversethread (global_State *g, lua_State *th) {
  పైకి_Val *uv;
  StkId o = th->దొంతర.p;
  ఒకవేళ (isold(th) || g->gcstate == GCSpropagate)
    linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
  ఒకవేళ (o == ఖాళీ)
    తిరిగి 0;  /* stack not completely built yet */
  lua_assert(g->gcstate == GCSatomic ||
             th->openupval == ఖాళీ || isintwups(th));
  కొరకు (; o < th->శిఖరం.p; o++)  /* mark live elements in the stack */
    markvalue(g, s2v(o));
  కొరకు (uv = th->openupval; uv != ఖాళీ; uv = uv->u.open.తదుపరి)
    markobject(g, uv);  /* open upvalues cannot be collected */
  ఒకవేళ (g->gcstate == GCSatomic) {  /* final traversal? */
    ఒకవేళ (!g->gcemergency)
      luaD_shrinkstack(th); /* do not change stack in emergency cycle */
    కొరకు (o = th->శిఖరం.p; o < th->దొంతర_చివరి.p + EXTRA_దొంతర; o++)
      setnilvalue2s(o);  /* clear dead stack slice */
    /* 'remarkupvals' may have removed thread from 'twups' list */
    ఒకవేళ (!isintwups(th) && th->openupval != ఖాళీ) {
      th->twups = g->twups;  /* link it back to the list */
      g->twups = th;
    }
  }
  తిరిగి 1 + (th->శిఖరం.p - th->దొంతర.p);
}


/*
** traverse one gray object, turning it to black. Return an estimate
** of the number of slots traversed.
*/
స్థిర l_mem propagatemark (global_State *g) {
  GCObject *o = g->బూడిద;
  nw2black(o);
  g->బూడిద = *getgclist(o);  /* remove from 'gray' list */
  ఎంపిక (o->tt) {
    స్థితి LUA_VTABLE: తిరిగి traversetable(g, gco2t(o));
    స్థితి LUA_VUSERDATA: తిరిగి traverseudata(g, gco2u(o));
    స్థితి LUA_VLCL: తిరిగి traverseLclosure(g, gco2lcl(o));
    స్థితి LUA_VCCL: తిరిగి traverseCclosure(g, gco2ccl(o));
    స్థితి LUA_VPROTO: తిరిగి traverseproto(g, gco2p(o));
    స్థితి LUA_VTHREAD: తిరిగి traversethread(g, gco2th(o));
    అప్రమేయ: lua_assert(0); తిరిగి 0;
  }
}


స్థిర శూన్య propagateall (global_State *g) {
  అయితే (g->బూడిద)
    propagatemark(g);
}


/*
** Traverse all ephemeron tables propagating marks from keys to values.
** Repeat until it converges, that is, nothing new is marked. 'dir'
** inverts the direction of the traversals, trying to speed up
** convergence on chains in the same table.
*/
స్థిర శూన్య convergeephemerons (global_State *g) {
  పూర్ణాంకం మారింది;
  పూర్ణాంకం dir = 0;
  చేయి {
    GCObject *w;
    GCObject *తదుపరి = g->ephemeron;  /* get ephemeron list */
    g->ephemeron = ఖాళీ;  /* tables may return to this list when traversed */
    మారింది = 0;
    అయితే ((w = తదుపరి) != ఖాళీ) {  /* for each ephemeron table */
      పట్టిక *h = gco2t(w);
      తదుపరి = h->gclist;  /* list is rebuilt during loop */
      nw2black(h);  /* out of the list (for now) */
      ఒకవేళ (traverseephemeron(g, h, dir)) {  /* marked some value? */
        propagateall(g);  /* propagate changes */
        మారింది = 1;  /* will have to revisit all ephemeron tables */
      }
    }
    dir = !dir;  /* invert direction next time */
  } అయితే (మారింది);  /* repeat until no more changes */
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
స్థిర శూన్య clearbykeys (global_State *g, GCObject *l) {
  కొరకు (; l; l = gco2t(l)->gclist) {
    పట్టిక *h = gco2t(l);
    కణుపు_2 *హద్దు = gnodelast(h);
    కణుపు_2 *n;
    కొరకు (n = gnode(h, 0); n < హద్దు; n++) {
      ఒకవేళ (iscleared(g, gckeyN(n)))  /* unmarked key? */
        setempty(gval(n));  /* remove entry */
      ఒకవేళ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


/*
** clear entries with unmarked values from all weaktables in list 'l' up
** to element 'f'
*/
స్థిర శూన్య clearbyvalues (global_State *g, GCObject *l, GCObject *f) {
  కొరకు (; l != f; l = gco2t(l)->gclist) {
    పట్టిక *h = gco2t(l);
    కణుపు_2 *n, *హద్దు = gnodelast(h);
    గుర్తులేని పూర్ణాంకం i;
    గుర్తులేని పూర్ణాంకం asize = h->asize;
    కొరకు (i = 0; i < asize; i++) {
      GCObject *o = gcvalarr(h, i);
      ఒకవేళ (iscleared(g, o))  /* value was collected? */
        *get_Arr_ట్యాగ్(h, i) = LUA_VEMPTY;  /* remove entry */
    }
    కొరకు (n = gnode(h, 0); n < హద్దు; n++) {
      ఒకవేళ (iscleared(g, gcvalueN(gval(n))))  /* unmarked value? */
        setempty(gval(n));  /* remove entry */
      ఒకవేళ (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


స్థిర శూన్య freeupval (lua_State *L, పైకి_Val *uv) {
  ఒకవేళ (upisopen(uv))
    luaF_unlinkupval(uv);
  luaM_free(L, uv);
}


స్థిర శూన్య freeobj (lua_State *L, GCObject *o) {
  assert_సంకేతం(l_mem newmem = gettotalbytes(G(L)) - objsize(o));
  ఎంపిక (o->tt) {
    స్థితి LUA_VPROTO:
      luaF_freeproto(L, gco2p(o));
      ఆపు;
    స్థితి LUA_VUPVAL:
      freeupval(L, gco2upv(o));
      ఆపు;
    స్థితి LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      luaM_freemem(L, cl, కొలత_Lclosure(cl->nupvalues));
      ఆపు;
    }
    స్థితి LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      luaM_freemem(L, cl, కొలత_Cclosure(cl->nupvalues));
      ఆపు;
    }
    స్థితి LUA_VTABLE:
      luaH_free(L, gco2t(o));
      ఆపు;
    స్థితి LUA_VTHREAD:
      luaE_freethread(L, gco2th(o));
      ఆపు;
    స్థితి LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      luaM_freemem(L, o, sizeudata(u->nuvalue, u->len));
      ఆపు;
    }
    స్థితి LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      luaS_remove(L, ts);  /* remove it from hash table */
      luaM_freemem(L, ts, sizestrshr(cast_uint(ts->shrlen)));
      ఆపు;
    }
    స్థితి LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      ఒకవేళ (ts->shrlen == LSTRMEM)  /* must free external string? */
        (*ts->falloc)(ts->ud, ts->contents, ts->u.lnglen + 1, 0);
      luaM_freemem(L, ts, luaS_sizelngstr(ts->u.lnglen, ts->shrlen));
      ఆపు;
    }
    అప్రమేయ: lua_assert(0);
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
స్థిర GCObject **sweeplist (lua_State *L, GCObject **p, l_mem countin) {
  global_State *g = G(L);
  పూర్ణాంకం ow = otherwhite(g);
  పూర్ణాంకం white = luaC_white(g);  /* current white */
  అయితే (*p != ఖాళీ && countin-- > 0) {
    GCObject *curr = *p;
    పూర్ణాంకం marked = curr->marked;
    ఒకవేళ (isdeadm(ow, marked)) {  /* is 'curr' dead? */
      *p = curr->తదుపరి;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    లేకపోతే {  /* change mark to 'white' and age to 'new' */
      curr->marked = cast_byte((marked & ~maskgcbits) | white | G_NEW);
      p = &curr->తదుపరి;  /* go to next element */
    }
  }
  తిరిగి (*p == ఖాళీ) ? ఖాళీ : p;
}


/*
** sweep a list until a live object (or end of list)
*/
స్థిర GCObject **sweeptolive (lua_State *L, GCObject **p) {
  GCObject **old = p;
  చేయి {
    p = sweeplist(L, p, 1);
  } అయితే (p == old);
  తిరిగి p;
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
స్థిర శూన్య checkSizes (lua_State *L, global_State *g) {
  ఒకవేళ (!g->gcemergency) {
    ఒకవేళ (g->strt.nuse < g->strt.కొలత / 4)  /* string table too big? */
      luaS_resize(L, g->strt.కొలత / 2);
  }
}


/*
** Get the next udata to be finalized from the 'tobefnz' list, and
** link it back into the 'allgc' list.
*/
స్థిర GCObject *udata2finalize (global_State *g) {
  GCObject *o = g->tobefnz;  /* get first element */
  lua_assert(tofinalize(o));
  g->tobefnz = o->తదుపరి;  /* remove it from 'tobefnz' list */
  o->తదుపరి = g->allgc;  /* return it to 'allgc' list */
  g->allgc = o;
  resetbit(o->marked, FINALIZEDBIT);  /* object is "normal" again */
  ఒకవేళ (issweepphase(g))
    makewhite(g, o);  /* "sweep" object */
  లేకపోతే ఒకవేళ (getage(o) == G_OLD1)
    g->firstold1 = o;  /* it is the first OLD1 object in the list */
  తిరిగి o;
}


స్థిర శూన్య dothecall (lua_State *L, శూన్య *ud) {
  వాడని(ud);
  luaD_callnoyield(L, L->శిఖరం.p - 2, 0);
}


స్థిర శూన్య GCTM (lua_State *L) {
  global_State *g = G(L);
  స్థిరరాశి TValue *tm;
  TValue v;
  lua_assert(!g->gcemergency);
  setgcovalue(L, &v, udata2finalize(g));
  tm = luaT_gettmbyobj(L, &v, TM_GC);
  ఒకవేళ (!notm(tm)) {  /* is there a finalizer? */
    TStatus స్థితి_2;
    lu_byte oldah = L->allowhook;
    lu_byte oldgcstp  = g->gcstp;
    g->gcstp |= GCSTPGC;  /* avoid GC steps */
    L->allowhook = 0;  /* stop debug hooks during GC metamethod */
    setobj2s(L, L->శిఖరం.p++, tm);  /* push finalizer... */
    setobj2s(L, L->శిఖరం.p++, &v);  /* ... and its argument */
    L->ci->callstatus |= CIST_FIN;  /* will run a finalizer */
    స్థితి_2 = luaD_pcall(L, dothecall, ఖాళీ, savestack(L, L->శిఖరం.p - 2), 0);
    L->ci->callstatus &= ~CIST_FIN;  /* not running a finalizer anymore */
    L->allowhook = oldah;  /* restore hooks */
    g->gcstp = oldgcstp;  /* restore state */
    ఒకవేళ (l_unlikely(స్థితి_2 != LUA_OK)) {  /* error while running __gc? */
      luaE_warnerror(L, "__gc");
      L->శిఖరం.p--;  /* pops error object */
    }
  }
}


/*
** call all pending finalizers
*/
స్థిర శూన్య callallpendingfinalizers (lua_State *L) {
  global_State *g = G(L);
  అయితే (g->tobefnz)
    GCTM(L);
}


/*
** find last 'next' field in list 'p' list (to add elements in its end)
*/
స్థిర GCObject **findlast (GCObject **p) {
  అయితే (*p != ఖాళీ)
    p = &(*p)->తదుపరి;
  తిరిగి p;
}


/*
** Move all unreachable objects (or 'all' objects) that need
** finalization from list 'finobj' to list 'tobefnz' (to be finalized).
** (Note that objects after 'finobjold1' cannot be white, so they
** don't need to be traversed. In incremental mode, 'finobjold1' is NULL,
** so the whole list is traversed.)
*/
స్థిర శూన్య separatetobefnz (global_State *g, పూర్ణాంకం all) {
  GCObject *curr;
  GCObject **p = &g->finobj;
  GCObject **lastnext = findlast(&g->tobefnz);
  అయితే ((curr = *p) != g->finobjold1) {  /* traverse all finalizable objects */
    lua_assert(tofinalize(curr));
    ఒకవేళ (!(iswhite(curr) || all))  /* not being collected? */
      p = &curr->తదుపరి;  /* don't bother with it */
    లేకపోతే {
      ఒకవేళ (curr == g->finobjsur)  /* removing 'finobjsur'? */
        g->finobjsur = curr->తదుపరి;  /* correct it */
      *p = curr->తదుపరి;  /* remove 'curr' from 'finobj' list */
      curr->తదుపరి = *lastnext;  /* link at the end of 'tobefnz' list */
      *lastnext = curr;
      lastnext = &curr->తదుపరి;
    }
  }
}


/*
** If pointer 'p' points to 'o', move it to the next element.
*/
స్థిర శూన్య checkpointer (GCObject **p, GCObject *o) {
  ఒకవేళ (o == *p)
    *p = o->తదుపరి;
}


/*
** Correct pointers to objects inside 'allgc' list when
** object 'o' is being removed from the list.
*/
స్థిర శూన్య correctpointers (global_State *g, GCObject *o) {
  checkpointer(&g->survival, o);
  checkpointer(&g->old1, o);
  checkpointer(&g->reallyold, o);
  checkpointer(&g->firstold1, o);
}


/*
** if object 'o' has a finalizer, remove it from 'allgc' list (must
** search the list to find it) and link it in 'finobj' list.
*/
శూన్య luaC_checkfinalizer (lua_State *L, GCObject *o, పట్టిక *mt) {
  global_State *g = G(L);
  ఒకవేళ (tofinalize(o) ||                 /* obj. is already marked... */
      gfasttm(g, mt, TM_GC) == ఖాళీ ||    /* or has no finalizer... */
      (g->gcstp & GCSTPCLS))                   /* or closing state? */
    తిరిగి;  /* nothing to be done */
  లేకపోతే {  /* move 'o' to 'finobj' list */
    GCObject **p;
    ఒకవేళ (issweepphase(g)) {
      makewhite(g, o);  /* "sweep" object 'o' */
      ఒకవేళ (g->sweepgc == &o->తదుపరి)  /* should not remove 'sweepgc' object */
        g->sweepgc = sweeptolive(L, g->sweepgc);  /* change 'sweepgc' */
    }
    లేకపోతే
      correctpointers(g, o);
    /* search for pointer pointing to 'o' */
    కొరకు (p = &g->allgc; *p != o; p = &(*p)->తదుపరి) { /*ఖాళీలు*/ }
    *p = o->తదుపరి;  /* remove 'o' from 'allgc' list */
    o->తదుపరి = g->finobj;  /* link it in 'finobj' list */
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
స్థిర శూన్య setpause (global_State *g) {
  l_mem పరిమితి = applygcparam(g, PAUSE, g->GCmarked);
  l_mem debt = పరిమితి - gettotalbytes(g);
  ఒకవేళ (debt < 0) debt = 0;
  luaE_setdebt(g, debt);
}


/*
** Sweep a list of objects to enter generational mode.  Deletes dead
** objects and turns the non dead to old. All non-dead threads---which
** are now old---must be in a gray list. Everything else is not in a
** gray list. Open upvalues are also kept gray.
*/
స్థిర శూన్య sweep2old (lua_State *L, GCObject **p) {
  GCObject *curr;
  global_State *g = G(L);
  అయితే ((curr = *p) != ఖాళీ) {
    ఒకవేళ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(isdead(g, curr));
      *p = curr->తదుపరి;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    లేకపోతే {  /* all surviving objects become old */
      setage(curr, G_OLD);
      ఒకవేళ (curr->tt == LUA_VTHREAD) {  /* threads must be watched */
        lua_State *th = gco2th(curr);
        linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
      }
      లేకపోతే ఒకవేళ (curr->tt == LUA_VUPVAL && upisopen(gco2upv(curr)))
        set2gray(curr);  /* open upvalues are always gray */
      లేకపోతే  /* everything else is black */
        nw2black(curr);
      p = &curr->తదుపరి;  /* go to next element */
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
స్థిర GCObject **sweepgen (lua_State *L, global_State *g, GCObject **p,
                            GCObject *హద్దు, GCObject **pfirstold1,
                            l_mem *paddedold) {
  స్థిర స్థిరరాశి lu_byte nextage[] = {
    G_SURVIVAL,  /* from G_NEW */
    G_OLD1,      /* from G_SURVIVAL */
    G_OLD1,      /* from G_OLD0 */
    G_OLD,       /* from G_OLD1 */
    G_OLD,       /* from G_OLD (do not change) */
    G_TOUCHED1,  /* from G_TOUCHED1 (do not change) */
    G_TOUCHED2   /* from G_TOUCHED2 (do not change) */
  };
  l_mem addedold = 0;
  పూర్ణాంకం white = luaC_white(g);
  GCObject *curr;
  అయితే ((curr = *p) != హద్దు) {
    ఒకవేళ (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(!isold(curr) && isdead(g, curr));
      *p = curr->తదుపరి;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    లేకపోతే {  /* correct mark and age */
      పూర్ణాంకం age = getage(curr);
      ఒకవేళ (age == G_NEW) {  /* new objects go back to white */
        పూర్ణాంకం marked = curr->marked & ~maskgcbits;  /* erase GC bits */
        curr->marked = cast_byte(marked | G_SURVIVAL | white);
      }
      లేకపోతే {  /* all other objects will be old, and so keep their color */
        lua_assert(age != G_OLD1);  /* advanced in 'markold' */
        setage(curr, nextage[age]);
        ఒకవేళ (getage(curr) == G_OLD1) {
          addedold += objsize(curr);  /* bytes becoming old */
          ఒకవేళ (*pfirstold1 == ఖాళీ)
            *pfirstold1 = curr;  /* first OLD1 object in the list */
        }
      }
      p = &curr->తదుపరి;  /* go to next element */
    }
  }
  *paddedold += addedold;
  తిరిగి p;
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
స్థిర GCObject **correctgraylist (GCObject **p) {
  GCObject *curr;
  అయితే ((curr = *p) != ఖాళీ) {
    GCObject **తదుపరి = getgclist(curr);
    ఒకవేళ (iswhite(curr))
      వెళ్ళు remove;  /* remove all white objects */
    లేకపోతే ఒకవేళ (getage(curr) == G_TOUCHED1) {  /* touched in this cycle? */
      lua_assert(isgray(curr));
      nw2black(curr);  /* make it black, for next barrier */
      setage(curr, G_TOUCHED2);
      వెళ్ళు remain;  /* keep it in the list and go to next element */
    }
    లేకపోతే ఒకవేళ (curr->tt == LUA_VTHREAD) {
      lua_assert(isgray(curr));
      వెళ్ళు remain;  /* keep non-white threads on the list */
    }
    లేకపోతే {  /* everything else is removed */
      lua_assert(isold(curr));  /* young objects should be white here */
      ఒకవేళ (getage(curr) == G_TOUCHED2)  /* advance from TOUCHED2... */
        setage(curr, G_OLD);  /* ... to OLD */
      nw2black(curr);  /* make object black (to be removed) */
      వెళ్ళు remove;
    }
    remove: *p = *తదుపరి; కొనసాగు;
    remain: p = తదుపరి; కొనసాగు;
  }
  తిరిగి p;
}


/*
** Correct all gray lists, coalescing them into 'grayagain'.
*/
స్థిర శూన్య correctgraylists (global_State *g) {
  GCObject **list = correctgraylist(&g->grayagain);
  *list = g->weak; g->weak = ఖాళీ;
  list = correctgraylist(list);
  *list = g->allweak; g->allweak = ఖాళీ;
  list = correctgraylist(list);
  *list = g->ephemeron; g->ephemeron = ఖాళీ;
  correctgraylist(list);
}


/*
** Mark black 'OLD1' objects when starting a new young collection.
** Gray objects are already in some gray list, and so will be visited in
** the atomic step.
*/
స్థిర శూన్య markold (global_State *g, GCObject *from, GCObject *గమ్యం) {
  GCObject *p;
  కొరకు (p = from; p != గమ్యం; p = p->తదుపరి) {
    ఒకవేళ (getage(p) == G_OLD1) {
      lua_assert(!iswhite(p));
      setage(p, G_OLD);  /* now they are old */
      ఒకవేళ (isblack(p))
        reallymarkobject(g, p);
    }
  }
}


/*
** Finish a young-generation collection.
*/
స్థిర శూన్య finishgencycle (lua_State *L, global_State *g) {
  correctgraylists(g);
  checkSizes(L, g);
  g->gcstate = GCSpropagate;  /* skip restart */
  ఒకవేళ (g->tobefnz != ఖాళీ && !g->gcemergency && luaD_checkminstack(L))
    callallpendingfinalizers(L);
}


/*
** Shifts from a minor collection to major collections. It starts in
** the "sweep all" state to clear all objects, which are mostly black
** in generational mode.
*/
స్థిర శూన్య minor2inc (lua_State *L, global_State *g, lu_byte రకం) {
  g->GCmajorminor = g->GCmarked;  /* number of live bytes */
  g->gckind = రకం;
  g->reallyold = g->old1 = g->survival = ఖాళీ;
  g->finobjrold = g->finobjold1 = g->finobjsur = ఖాళీ;
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
స్థిర పూర్ణాంకం checkminormajor (global_State *g) {
  l_mem హద్దు = applygcparam(g, MINORMAJOR, g->GCmajorminor);
  ఒకవేళ (హద్దు == 0)
    తిరిగి 0;  /* special case: 'minormajor' 0 stops major collections */
  తిరిగి (g->GCmarked >= హద్దు);
}

/*
** Does a young collection. First, mark 'OLD1' objects. Then does the
** atomic step. Then, check whether to continue in minor mode. If so,
** sweep all lists and advance pointers. Finally, finish the collection.
*/
స్థిర శూన్య youngcollection (lua_State *L, global_State *g) {
  l_mem addedold1 = 0;
  l_mem marked = g->GCmarked;  /* preserve 'g->GCmarked' */
  GCObject **psurvival;  /* to point to first non-dead survival object */
  GCObject *dummy;  /* dummy out parameter to 'sweepgen' */
  lua_assert(g->gcstate == GCSpropagate);
  ఒకవేళ (g->firstold1) {  /* are there regular OLD1 objects? */
    markold(g, g->firstold1, g->reallyold);  /* mark them */
    g->firstold1 = ఖాళీ;  /* no more OLD1 objects (for now) */
  }
  markold(g, g->finobj, g->finobjrold);
  markold(g, g->tobefnz, ఖాళీ);

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
  dummy = ఖాళీ;  /* no 'firstold1' optimization for 'finobj' lists */
  psurvival = sweepgen(L, g, &g->finobj, g->finobjsur, &dummy, &addedold1);
  /* sweep 'survival' */
  sweepgen(L, g, psurvival, g->finobjold1, &dummy, &addedold1);
  g->finobjrold = g->finobjold1;
  g->finobjold1 = *psurvival;  /* 'survival' survivals are old now */
  g->finobjsur = g->finobj;  /* all news are survivals */

  sweepgen(L, g, &g->tobefnz, ఖాళీ, &dummy, &addedold1);

  /* keep total number of added old1 bytes */
  g->GCmarked = marked + addedold1;

  /* decide whether to shift to major mode */
  ఒకవేళ (checkminormajor(g)) {
    minor2inc(L, g, KGC_GENMAJOR);  /* go to major mode */
    g->GCmarked = 0;  /* avoid pause in first major cycle (see 'setpause') */
  }
  లేకపోతే
    finishgencycle(L, g);  /* still in minor mode; finish it */
}


/*
** Clears all gray lists, sweeps objects, and prepare sublists to enter
** generational mode. The sweeps remove dead objects and turn all
** surviving objects to old. Threads go back to 'grayagain'; everything
** else is turned black (not in any gray list).
*/
స్థిర శూన్య atomic2gen (lua_State *L, global_State *g) {
  cleargraylists(g);
  /* sweep all elements making them old */
  g->gcstate = GCSswpallgc;
  sweep2old(L, &g->allgc);
  /* everything alive now is old */
  g->reallyold = g->old1 = g->survival = g->allgc;
  g->firstold1 = ఖాళీ;  /* there are no OLD1 objects anywhere */

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
స్థిర శూన్య setminordebt (global_State *g) {
  luaE_setdebt(g, applygcparam(g, MINORMUL, g->GCmajorminor));
}


/*
** Enter generational mode. Must go until the end of an atomic cycle
** to ensure that all objects are correctly marked and weak tables
** are cleared. Then, turn all objects into old and finishes the
** collection.
*/
స్థిర శూన్య entergen (lua_State *L, global_State *g) {
  luaC_runtilstate(L, GCSpause, 1);  /* prepare to start a new cycle */
  luaC_runtilstate(L, GCSpropagate, 1);  /* start new cycle */
  atomic(L);  /* propagates all and then do the atomic stuff */
  atomic2gen(L, g);
  setminordebt(g);  /* set debt assuming next cycle will be minor */
}


/*
** Change collector mode to 'newmode'.
*/
శూన్య luaC_changemode (lua_State *L, పూర్ణాంకం newmode) {
  global_State *g = G(L);
  ఒకవేళ (g->gckind == KGC_GENMAJOR)  /* doing major collections? */
    g->gckind = KGC_INC;  /* already incremental but in name */
  ఒకవేళ (newmode != g->gckind) {  /* does it need to change? */
    ఒకవేళ (newmode == KGC_INC)  /* entering incremental mode? */
      minor2inc(L, g, KGC_INC);  /* entering incremental mode */
    లేకపోతే {
      lua_assert(newmode == KGC_GENMINOR);
      entergen(L, g);
    }
  }
}


/*
** Does a full collection in generational mode.
*/
స్థిర శూన్య fullgen (lua_State *L, global_State *g) {
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
స్థిర పూర్ణాంకం checkmajorminor (lua_State *L, global_State *g) {
  ఒకవేళ (g->gckind == KGC_GENMAJOR) {  /* generational mode? */
    l_mem numbytes = gettotalbytes(g);
    l_mem addedbytes = numbytes - g->GCmajorminor;
    l_mem హద్దు = (addedbytes < 0) ? 0
                                   : applygcparam(g, MAJORMINOR, addedbytes);
    l_mem tobecollected = numbytes - g->GCmarked;
    ఒకవేళ (tobecollected > హద్దు) {
      atomic2gen(L, g);  /* return to generational mode */
      setminordebt(g);
      తిరిగి 1;  /* exit incremental collection */
    }
  }
  g->GCmajorminor = g->GCmarked;  /* prepare for next collection */
  తిరిగి 0;  /* stay doing incremental collections */
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
స్థిర శూన్య entersweep (lua_State *L) {
  global_State *g = G(L);
  g->gcstate = GCSswpallgc;
  lua_assert(g->sweepgc == ఖాళీ);
  g->sweepgc = sweeptolive(L, &g->allgc);
}


/*
** Delete all objects in list 'p' until (but not including) object
** 'limit'.
*/
స్థిర శూన్య deletelist (lua_State *L, GCObject *p, GCObject *హద్దు) {
  అయితే (p != హద్దు) {
    GCObject *తదుపరి = p->తదుపరి;
    freeobj(L, p);
    p = తదుపరి;
  }
}


/*
** Call all finalizers of the objects in the given Lua state, and
** then free all objects, except for the main thread.
*/
శూన్య luaC_freeallobjects (lua_State *L) {
  global_State *g = G(L);
  g->gcstp = GCSTPCLS;  /* no extra finalizers after here */
  luaC_changemode(L, KGC_INC);
  separatetobefnz(g, 1);  /* separate all objects with finalizers */
  lua_assert(g->finobj == ఖాళీ);
  callallpendingfinalizers(L);
  deletelist(L, g->allgc, obj2gco(mainthread(g)));
  lua_assert(g->finobj == ఖాళీ);  /* no new finalizers */
  deletelist(L, g->fixedgc, ఖాళీ);  /* collect fixed objects */
  lua_assert(g->strt.nuse == 0);
}


స్థిర శూన్య atomic (lua_State *L) {
  global_State *g = G(L);
  GCObject *origweak, *origall;
  GCObject *grayagain = g->grayagain;  /* save original list */
  g->grayagain = ఖాళీ;
  lua_assert(g->ephemeron == ఖాళీ && g->weak == ఖాళీ);
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
  g->బూడిద = grayagain;
  propagateall(g);  /* traverse 'grayagain' list */
  convergeephemerons(g);
  /* at this point, all strongly accessible objects are marked. */
  /* Clear values from weak tables, before checking finalizers */
  clearbyvalues(g, g->weak, ఖాళీ);
  clearbyvalues(g, g->allweak, ఖాళీ);
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
  lua_assert(g->బూడిద == ఖాళీ);
}


/*
** Do a sweep step. The normal case (not fast) sweeps at most GCSWEEPMAX
** elements. The fast case sweeps the whole list.
*/
స్థిర శూన్య sweepstep (lua_State *L, global_State *g,
                       lu_byte nextstate, GCObject **nextlist, పూర్ణాంకం fast) {
  ఒకవేళ (g->sweepgc)
    g->sweepgc = sweeplist(L, g->sweepgc, fast ? MAX_LMEM : GCSWEEPMAX);
  లేకపోతే {  /* enter next state */
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

#నిర్వచించు step2pause	-3  /* finished collection; entered pause state */
#నిర్వచించు atomicstep	-2  /* atomic step */
#నిర్వచించు step2minor	-1  /* moved to minor collections */


స్థిర l_mem singlestep (lua_State *L, పూర్ణాంకం fast) {
  global_State *g = G(L);
  l_mem stepresult;
  lua_assert(!g->gcstopem);  /* collector is not reentrant */
  g->gcstopem = 1;  /* no emergency collections while collecting */
  ఎంపిక (g->gcstate) {
    స్థితి GCSpause: {
      restartcollection(g);
      g->gcstate = GCSpropagate;
      stepresult = 1;
      ఆపు;
    }
    స్థితి GCSpropagate: {
      ఒకవేళ (fast || g->బూడిద == ఖాళీ) {
        g->gcstate = GCSenteratomic;  /* finish propagate phase */
        stepresult = 1;
      }
      లేకపోతే
        stepresult = propagatemark(g);  /* traverse one gray object */
      ఆపు;
    }
    స్థితి GCSenteratomic: {
      atomic(L);
      ఒకవేళ (checkmajorminor(L, g))
        stepresult = step2minor;
      లేకపోతే {
        entersweep(L);
        stepresult = atomicstep;
      }
      ఆపు;
    }
    స్థితి GCSswpallgc: {  /* sweep "regular" objects */
      sweepstep(L, g, GCSswpfinobj, &g->finobj, fast);
      stepresult = GCSWEEPMAX;
      ఆపు;
    }
    స్థితి GCSswpfinobj: {  /* sweep objects with finalizers */
      sweepstep(L, g, GCSswptobefnz, &g->tobefnz, fast);
      stepresult = GCSWEEPMAX;
      ఆపు;
    }
    స్థితి GCSswptobefnz: {  /* sweep objects to be finalized */
      sweepstep(L, g, GCSswpend, ఖాళీ, fast);
      stepresult = GCSWEEPMAX;
      ఆపు;
    }
    స్థితి GCSswpend: {  /* finish sweeps */
      checkSizes(L, g);
      g->gcstate = GCScallfin;
      stepresult = GCSWEEPMAX;
      ఆపు;
    }
    స్థితి GCScallfin: {  /* call finalizers */
      ఒకవేళ (g->tobefnz && !g->gcemergency && luaD_checkminstack(L)) {
        g->gcstopem = 0;  /* ok collections during finalizers */
        GCTM(L);  /* call one finalizer */
        stepresult = CWUFIN;
      }
      లేకపోతే {  /* no more finalizers or emergency mode or not enough stack
                 to run finalizers */
        g->gcstate = GCSpause;  /* finish collection */
        stepresult = step2pause;
      }
      ఆపు;
    }
    అప్రమేయ: lua_assert(0); తిరిగి 0;
  }
  g->gcstopem = 0;
  తిరిగి stepresult;
}


/*
** Advances the garbage collector until it reaches the given state.
** (The option 'fast' is only for testing; in normal code, 'fast'
** here is always true.)
*/
శూన్య luaC_runtilstate (lua_State *L, పూర్ణాంకం state, పూర్ణాంకం fast) {
  global_State *g = G(L);
  lua_assert(g->gckind == KGC_INC);
  అయితే (state != g->gcstate)
    singlestep(L, fast);
}



/*
** Performs a basic incremental step. The step size is
** converted from bytes to "units of work"; then the function loops
** running single steps until adding that many units of work or
** finishing a cycle (pause state). Finally, it sets the debt that
** controls when next step will be performed.
*/
స్థిర శూన్య incstep (lua_State *L, global_State *g) {
  l_mem stepsize = applygcparam(g, STEPSIZE, 100);
  l_mem work2do = applygcparam(g, STEPMUL, stepsize / cast_int(పరిమాణం(శూన్య*)));
  l_mem stres;
  పూర్ణాంకం fast = (work2do == 0);  /* special case: do a full collection */
  చేయి {  /* repeat until enough work */
    stres = singlestep(L, fast);  /* perform one single step */
    ఒకవేళ (stres == step2minor)  /* returned to minor collections? */
      తిరిగి;  /* nothing else to be done here */
    లేకపోతే ఒకవేళ (stres == step2pause || (stres == atomicstep && !fast))
      ఆపు;  /* end of cycle or atomic */
    లేకపోతే
      work2do -= stres;
  } అయితే (fast || work2do > 0);
  ఒకవేళ (g->gcstate == GCSpause)
    setpause(g);  /* pause until next cycle */
  లేకపోతే
    luaE_setdebt(g, stepsize);
}


#ఒకవేళ !defined(luai_tracegc)
#నిర్వచించు luai_tracegc(L,f)		((శూన్య)0)
#అయితే_ముగింపు

/*
** Performs a basic GC step if collector is running. (If collector was
** stopped by the user, set a reasonable debt to avoid it being called
** at every single check.)
*/
శూన్య lua_C_అడుగు (lua_State *L) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  ఒకవేళ (!gcrunning(g)) {  /* not running? */
    ఒకవేళ (g->gcstp & GCSTPUSR)  /* stopped by the user? */
      luaE_setdebt(g, 20000);
  }
  లేకపోతే {
    luai_tracegc(L, 1);  /* for internal debugging */
    ఎంపిక (g->gckind) {
      స్థితి KGC_INC: స్థితి KGC_GENMAJOR:
        incstep(L, g);
        ఆపు;
      స్థితి KGC_GENMINOR:
        youngcollection(L, g);
        setminordebt(g);
        ఆపు;
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
స్థిర శూన్య fullinc (lua_State *L, global_State *g) {
  ఒకవేళ (keepinvariant(g))  /* black objects? */
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
శూన్య luaC_fullgc (lua_State *L, పూర్ణాంకం isemergency) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  g->gcemergency = cast_byte(isemergency);  /* set flag */
  ఎంపిక (g->gckind) {
    స్థితి KGC_GENMINOR: fullgen(L, g); ఆపు;
    స్థితి KGC_INC: fullinc(L, g); ఆపు;
    స్థితి KGC_GENMAJOR:
      g->gckind = KGC_INC;
      fullinc(L, g);
      g->gckind = KGC_GENMAJOR;
      ఆపు;
  }
  g->gcemergency = 0;
}

/* }====================================================== */


