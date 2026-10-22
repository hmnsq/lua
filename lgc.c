/*
** $Id: lgc.c $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#تعریف_کرو lgc_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"

#شامل <string.h>


#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "ltm.h"


/*
** Maximum number of elements to sweep in each single step.
** (Large enough to dissipate fixed overheads but small enough
** to allow small steps for the collector.)
*/
#تعریف_کرو GCSWEEPMAX	20


/*
** Cost (in work units) of running one finalizer.
*/
#تعریف_کرو CWUFIN	10


/* mask with all color bits */
#تعریف_کرو maskcolors	(bitmask(BLACKBIT) | WHITEBITS)

/* mask with all GC bits */
#تعریف_کرو maskgcbits      (maskcolors | AGEBITS)


/* macro to erase all color bits then set only the current white bit */
#تعریف_کرو makewhite(g,x)	\
  (x->marked = cast_byte((x->marked & ~maskcolors) | luaC_white(g)))

/* make an object gray (neither white nor black) */
#تعریف_کرو set2gray(x)	resetbits(x->marked, maskcolors)


/* make an object black (coming from any color) */
#تعریف_کرو set2black(x)  \
  (x->marked = cast_byte((x->marked & ~WHITEBITS) | bitmask(BLACKBIT)))


#تعریف_کرو valiswhite(x)   (iscollectable(x) && iswhite(gcvalue(x)))

#تعریف_کرو keyiswhite(n)   (keyiscollectable(n) && iswhite(gckey(n)))


/*
** Protected access to objects in values
*/
#تعریف_کرو gcvalueN(o)     (iscollectable(o) ? gcvalue(o) : تہی)


/*
** Access to collectable objects in array part of tables
*/
#تعریف_کرو gcvalarr(t,i)  \
	((*get_Arr_لیبل(t,i) & ثنائی_ہندسہ_ISCOLLECTABLE) ? getArrVal(t,i)->gc : تہی)


#تعریف_کرو markvalue(g,o) { checkliveness(mainthread(g),o); \
  اگر (valiswhite(o)) reallymarkobject(g,gcvalue(o)); }

#تعریف_کرو markkey(g, n)	{ اگر keyiswhite(n) reallymarkobject(g,gckey(n)); }

#تعریف_کرو markobject(g,t)	{ اگر (iswhite(t)) reallymarkobject(g, obj2gco(t)); }

/*
** mark an object that can be NULL (either because it is really optional,
** or it was stripped as debug info, or inside an uncompleted structure)
*/
#تعریف_کرو markobjectN(g,t)	{ اگر (t) markobject(g,t); }


ساکن خالی reallymarkobject (global_State *g, GCObject *o);
ساکن خالی atomic (lua_State *L);
ساکن خالی entersweep (lua_State *L);


/*
** {======================================================
** Generic functions
** =======================================================
*/


/*
** one after last element in a hash array
*/
#تعریف_کرو gnodelast(h)	gnode(h, cast_sizet(sizenode(h)))


ساکن l_mem objsize (GCObject *o) {
  lu_mem res;
  انتخاب (o->tt) {
    حالت LUA_VTABLE: {
      res = lua_H_سائز(gco2t(o));
      توڑو;
    }
    حالت LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      res = سائز_Lclosure(cl->nupvalues);
      توڑو;
    }
    حالت LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      res = سائز_Cclosure(cl->nupvalues);
      توڑو;
    }
    حالت LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      res = sizeudata(u->nuvalue, u->len);
      توڑو;
    }
    حالت LUA_VPROTO: {
      res = luaF_protosize(gco2p(o));
      توڑو;
    }
    حالت LUA_VTHREAD: {
      res = luaE_threadsize(gco2th(o));
      توڑو;
    }
    حالت LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      res = sizestrshr(cast_uint(ts->shrlen));
      توڑو;
    }
    حالت LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      res = luaS_sizelngstr(ts->u.lnglen, ts->shrlen);
      توڑو;
    }
    حالت LUA_VUPVAL: {
      res = حجم(اوپر_Val);
      توڑو;
    }
    طےشدہ: res = 0; lua_assert(0);
  }
  واپس cast(l_mem, res);
}


ساکن GCObject **getgclist (GCObject *o) {
  انتخاب (o->tt) {
    حالت LUA_VTABLE: واپس &gco2t(o)->gclist;
    حالت LUA_VLCL: واپس &gco2lcl(o)->gclist;
    حالت LUA_VCCL: واپس &gco2ccl(o)->gclist;
    حالت LUA_VTHREAD: واپس &gco2th(o)->gclist;
    حالت LUA_VPROTO: واپس &gco2p(o)->gclist;
    حالت LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      lua_assert(u->nuvalue > 0);
      واپس &u->gclist;
    }
    طےشدہ: lua_assert(0); واپس 0;
  }
}


/*
** Link a collectable object 'o' with a known type into the list 'p'.
** (Must be a macro to access the 'gclist' field in different types.)
*/
#تعریف_کرو linkgclist(o,p)	linkgclist_(obj2gco(o), &(o)->gclist, &(p))

ساکن خالی linkgclist_ (GCObject *o, GCObject **pnext, GCObject **list) {
  lua_assert(!isgray(o));  /* cannot be in a gray list */
  *pnext = *list;
  *list = o;
  set2gray(o);  /* now it is */
}


/*
** Link a generic collectable object 'o' into the list 'p'.
*/
#تعریف_کرو linkobjgclist(o,p) linkgclist_(obj2gco(o), getgclist(o), &(p))



/*
** Clear keys for empty entries in tables. If entry is empty, mark its
** entry as dead. This allows the collection of the key, but keeps its
** entry in the table: its removal could break a chain and could break
** a table traversal.  Other places never manipulate dead keys, because
** its associated empty value is enough to signal that the entry is
** logically empty.
*/
ساکن خالی clearkey (گرہ_2 *n) {
  lua_assert(isempty(gval(n)));
  اگر (keyiscollectable(n))
    setdeadkey(n);  /* unused key; remove it */
}


/*
** tells whether a key or value can be cleared from a weak
** table. Non-collectable objects are never removed from weak
** tables. Strings behave as 'values', so are never removed too. for
** other objects: if really collected, cannot keep them; for objects
** being finalized, keep them in keys, but not in values
*/
ساکن صحیح iscleared (global_State *g, ثابت GCObject *o) {
  اگر (o == تہی) واپس 0;  /* non-collectable value */
  ورنہ اگر (novariant(o->tt) == LUA_TSTRING) {
    markobject(g, o);  /* strings are 'values', so are never weak */
    واپس 0;
  }
  ورنہ واپس iswhite(o);
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
خالی luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v) {
  global_State *g = G(L);
  lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
  اگر (keepinvariant(g)) {  /* must keep invariant? */
    reallymarkobject(g, v);  /* restore invariant */
    اگر (isold(o)) {
      lua_assert(!isold(v));  /* white object could not be old */
      setage(v, G_OLD0);  /* restore generational invariant */
    }
  }
  ورنہ {  /* sweep phase */
    lua_assert(issweepphase(g));
    اگر (g->gckind != KGC_GENMINOR)  /* incremental mode? */
      makewhite(g, o);  /* mark 'o' as white to avoid other barriers */
  }
}


/*
** barrier that moves collector backward, that is, mark the black object
** pointing to a white object as gray again.
*/
خالی luaC_barrierback_ (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(isblack(o) && !isdead(g, o));
  lua_assert((g->gckind != KGC_GENMINOR)
          || (isold(o) && getage(o) != G_TOUCHED1));
  اگر (getage(o) == G_TOUCHED2)  /* already in gray list? */
    set2gray(o);  /* make it gray to become touched1 */
  ورنہ  /* link it in 'grayagain' and paint it gray */
    linkobjgclist(o, g->grayagain);
  اگر (isold(o))  /* generational mode? */
    setage(o, G_TOUCHED1);  /* touched in current cycle */
}


خالی luaC_fix (lua_State *L, GCObject *o) {
  global_State *g = G(L);
  lua_assert(g->allgc == o);  /* object must be 1st in 'allgc' list! */
  set2gray(o);  /* they will be gray forever */
  setage(o, G_OLD);  /* and old forever */
  g->allgc = o->اگلا;  /* remove object from 'allgc' list */
  o->اگلا = g->fixedgc;  /* link it to 'fixedgc' list */
  g->fixedgc = o;
}


/*
** create a new collectable object (with given type, size, and offset)
** and link it to 'allgc' list.
*/
GCObject *luaC_newobjdt (lua_State *L, lu_byte tt, size_t sz, size_t انحراف) {
  global_State *g = G(L);
  حرف *p = cast_charp(luaM_newobject(L, novariant(tt), sz));
  GCObject *o = cast(GCObject *, p + انحراف);
  o->marked = luaC_white(g);
  o->tt = tt;
  o->اگلا = g->allgc;
  g->allgc = o;
  واپس o;
}


/*
** create a new collectable object with no offset.
*/
GCObject *luaC_newobj (lua_State *L, lu_byte tt, size_t sz) {
  واپس luaC_newobjdt(L, tt, sz, 0);
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
ساکن خالی reallymarkobject (global_State *g, GCObject *o) {
  g->GCmarked += objsize(o);
  انتخاب (o->tt) {
    حالت LUA_VSHRSTR:
    حالت LUA_VLNGSTR: {
      set2black(o);  /* nothing to visit */
      توڑو;
    }
    حالت LUA_VUPVAL: {
      اوپر_Val *uv = gco2upv(o);
      اگر (upisopen(uv))
        set2gray(uv);  /* open upvalues are kept gray */
      ورنہ
        set2black(uv);  /* closed upvalues are visited here */
      markvalue(g, uv->v.p);  /* mark its content */
      توڑو;
    }
    حالت LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      اگر (u->nuvalue == 0) {  /* no user values? */
        markobjectN(g, u->metatable);  /* mark its metatable */
        set2black(u);  /* nothing else to mark */
        توڑو;
      }
      /* else... */
    }  /* FALLTHROUGH */
    حالت LUA_VLCL: حالت LUA_VCCL: حالت LUA_VTABLE:
    حالت LUA_VTHREAD: حالت LUA_VPROTO: {
      linkobjgclist(o, g->سرمئی);  /* to be visited later */
      توڑو;
    }
    طےشدہ: lua_assert(0); توڑو;
  }
}


/*
** mark metamethods for basic types
*/
ساکن خالی markmt (global_State *g) {
  صحیح i;
  برائے (i=0; i < LUA_NUMTYPES; i++)
    markobjectN(g, g->mt[i]);
}


/*
** mark all objects in list of being-finalized
*/
ساکن خالی markbeingfnz (global_State *g) {
  GCObject *o;
  برائے (o = g->tobefnz; o != تہی; o = o->اگلا)
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
ساکن خالی remarkupvals (global_State *g) {
  lua_State *thread;
  lua_State **p = &g->twups;
  جب ((thread = *p) != تہی) {
    اگر (!iswhite(thread) && thread->openupval != تہی)
      p = &thread->twups;  /* keep marked thread with upvalues in the list */
    ورنہ {  /* thread is not marked or without upvalues */
      اوپر_Val *uv;
      lua_assert(!isold(thread) || thread->openupval == تہی);
      *p = thread->twups;  /* remove thread from the list */
      thread->twups = thread;  /* mark that it is out of list */
      برائے (uv = thread->openupval; uv != تہی; uv = uv->u.open.اگلا) {
        lua_assert(getage(uv) <= getage(thread));
        اگر (!iswhite(uv)) {  /* upvalue already visited? */
          lua_assert(upisopen(uv) && isgray(uv));
          markvalue(g, uv->v.p);  /* mark its value */
        }
      }
    }
  }
}


ساکن خالی cleargraylists (global_State *g) {
  g->سرمئی = g->grayagain = تہی;
  g->weak = g->allweak = g->ephemeron = تہی;
}


/*
** mark root set and reset all gray lists, to start a new collection.
** 'GCmarked' is initialized to count the total number of live bytes
** during a cycle.
*/
ساکن خالی restartcollection (global_State *g) {
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
ساکن خالی genlink (global_State *g, GCObject *o) {
  lua_assert(isblack(o));
  اگر (getage(o) == G_TOUCHED1) {  /* touched in this cycle? */
    linkobjgclist(o, g->grayagain);  /* link it back in 'grayagain' */
  }  /* everything else do not need to be linked back */
  ورنہ اگر (getage(o) == G_TOUCHED2)
    setage(o, G_OLD);  /* advance age */
}


/*
** Traverse a table with weak values and link it to proper list. During
** propagate phase, keep it in 'grayagain' list, to be revisited in the
** atomic phase. In the atomic phase, if table has any white value,
** put it in 'weak' list, to be cleared; otherwise, call 'genlink'
** to check table age in generational mode.
*/
ساکن خالی traverseweakvalue (global_State *g, جدول *h) {
  گرہ_2 *n, *حد_بندی = gnodelast(h);
  /* if there is array part, assume it may have white values (it is not
     worth traversing it now just to check) */
  صحیح hasclears = (h->asize > 0);
  برائے (n = gnode(h, 0); n < حد_بندی; n++) {  /* traverse hash part */
    اگر (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ورنہ {
      lua_assert(!keyisnil(n));
      markkey(g, n);
      اگر (!hasclears && iscleared(g, gcvalueN(gval(n))))  /* a white value? */
        hasclears = 1;  /* table will have to be cleared */
    }
  }
  اگر (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  ورنہ اگر (hasclears)
      linkgclist(h, g->weak);  /* has to be cleared later */
  ورنہ
    genlink(g, obj2gco(h));
}


/*
** Traverse the array part of a table.
*/
ساکن صحیح traversearray (global_State *g, جدول *h) {
  غیرعلامتی asize = h->asize;
  صحیح marked = 0;  /* true if some object is marked in this traversal */
  غیرعلامتی i;
  برائے (i = 0; i < asize; i++) {
    GCObject *o = gcvalarr(h, i);
    اگر (o != تہی && iswhite(o)) {
      marked = 1;
      reallymarkobject(g, o);
    }
  }
  واپس marked;
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
ساکن صحیح traverseephemeron (global_State *g, جدول *h, صحیح inv) {
  صحیح hasclears = 0;  /* true if table has white keys */
  صحیح hasww = 0;  /* true if table has entry "white-key -> white-value" */
  غیرعلامتی صحیح i;
  غیرعلامتی صحیح nsize = sizenode(h);
  صحیح marked = traversearray(g, h);  /* traverse array part */
  /* traverse hash part; if 'inv', traverse descending
     (see 'convergeephemerons') */
  برائے (i = 0; i < nsize; i++) {
    گرہ_2 *n = inv ? gnode(h, nsize - 1 - i) : gnode(h, i);
    اگر (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ورنہ اگر (iscleared(g, gckeyN(n))) {  /* key is not marked (yet)? */
      hasclears = 1;  /* table must be cleared */
      اگر (valiswhite(gval(n)))  /* value not marked yet? */
        hasww = 1;  /* white-white entry */
    }
    ورنہ اگر (valiswhite(gval(n))) {  /* value not marked yet? */
      marked = 1;
      reallymarkobject(g, gcvalue(gval(n)));  /* mark it now */
    }
  }
  /* link table into proper list */
  اگر (g->gcstate == GCSpropagate)
    linkgclist(h, g->grayagain);  /* must retraverse it in atomic phase */
  ورنہ اگر (hasww)  /* table has white->white entries? */
    linkgclist(h, g->ephemeron);  /* have to propagate again */
  ورنہ اگر (hasclears)  /* table has white keys? */
    linkgclist(h, g->allweak);  /* may have to clean white keys */
  ورنہ
    genlink(g, obj2gco(h));  /* check whether collector still needs to see it */
  واپس marked;
}


ساکن خالی traversestrongtable (global_State *g, جدول *h) {
  گرہ_2 *n, *حد_بندی = gnodelast(h);
  traversearray(g, h);
  برائے (n = gnode(h, 0); n < حد_بندی; n++) {  /* traverse hash part */
    اگر (isempty(gval(n)))  /* entry is empty? */
      clearkey(n);  /* clear its key */
    ورنہ {
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
ساکن صحیح getmode (global_State *g, جدول *h) {
  ثابت TValue *منوال = gfasttm(g, h->metatable, TM_منوال);
  اگر (منوال == تہی || !ttisstring(منوال))
    واپس 0;  /* ignore non-string modes */
  ورنہ {
    ثابت حرف *smode = getstr(tsvalue(منوال));
    ثابت حرف *weakkey = strchr(smode, 'k');
    ثابت حرف *weakvalue = strchr(smode, 'v');
    واپس ((weakkey != تہی) << 1) | (weakvalue != تہی);
  }
}


ساکن l_mem traversetable (global_State *g, جدول *h) {
  markobjectN(g, h->metatable);
  انتخاب (getmode(g, h)) {
    حالت 0:  /* not weak */
      traversestrongtable(g, h);
      توڑو;
    حالت 1:  /* weak values */
      traverseweakvalue(g, h);
      توڑو;
    حالت 2:  /* weak keys */
      traverseephemeron(g, h, 0);
      توڑو;
    حالت 3:  /* all weak; nothing to traverse */
      اگر (g->gcstate == GCSpropagate)
        linkgclist(h, g->grayagain);  /* must visit again its metatable */
      ورنہ
        linkgclist(h, g->allweak);  /* must clear collected entries */
      توڑو;
  }
  واپس cast(l_mem, 1 + 2*sizenode(h) + h->asize);
}


ساکن l_mem traverseudata (global_State *g, Udata *u) {
  صحیح i;
  markobjectN(g, u->metatable);  /* mark its metatable */
  برائے (i = 0; i < u->nuvalue; i++)
    markvalue(g, &u->uv[i].uv);
  genlink(g, obj2gco(u));
  واپس 1 + u->nuvalue;
}


/*
** Traverse a prototype. (While a prototype is being build, its
** arrays can be larger than needed; the extra slots are filled with
** NULL, so the use of 'markobjectN')
*/
ساکن l_mem traverseproto (global_State *g, Proto *f) {
  صحیح i;
  markobjectN(g, f->ماخذ);
  برائے (i = 0; i < f->sizek; i++)  /* mark literals */
    markvalue(g, &f->k[i]);
  برائے (i = 0; i < f->sizeupvalues; i++)  /* mark upvalue names */
    markobjectN(g, f->upvalues[i].name);
  برائے (i = 0; i < f->sizep; i++)  /* mark nested protos */
    markobjectN(g, f->p[i]);
  برائے (i = 0; i < f->sizelocvars; i++)  /* mark local-variable names */
    markobjectN(g, f->locvars[i].varname);
  واپس 1 + f->sizek + f->sizeupvalues + f->sizep + f->sizelocvars;
}


ساکن l_mem traverseCclosure (global_State *g, CClosure *cl) {
  صحیح i;
  برائے (i = 0; i < cl->nupvalues; i++)  /* mark its upvalues */
    markvalue(g, &cl->upvalue[i]);
  واپس 1 + cl->nupvalues;
}

/*
** Traverse a Lua closure, marking its prototype and its upvalues.
** (Both can be NULL while closure is being created.)
*/
ساکن l_mem traverseLclosure (global_State *g, LClosure *cl) {
  صحیح i;
  markobjectN(g, cl->p);  /* mark its prototype */
  برائے (i = 0; i < cl->nupvalues; i++) {  /* visit its upvalues */
    اوپر_Val *uv = cl->upvals[i];
    markobjectN(g, uv);  /* mark upvalue */
  }
  واپس 1 + cl->nupvalues;
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
ساکن l_mem traversethread (global_State *g, lua_State *th) {
  اوپر_Val *uv;
  StkId o = th->ڈھیر.p;
  اگر (isold(th) || g->gcstate == GCSpropagate)
    linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
  اگر (o == تہی)
    واپس 0;  /* stack not completely built yet */
  lua_assert(g->gcstate == GCSatomic ||
             th->openupval == تہی || isintwups(th));
  برائے (; o < th->چوٹی.p; o++)  /* mark live elements in the stack */
    markvalue(g, s2v(o));
  برائے (uv = th->openupval; uv != تہی; uv = uv->u.open.اگلا)
    markobject(g, uv);  /* open upvalues cannot be collected */
  اگر (g->gcstate == GCSatomic) {  /* final traversal? */
    اگر (!g->gcemergency)
      luaD_shrinkstack(th); /* do not change stack in emergency cycle */
    برائے (o = th->چوٹی.p; o < th->ڈھیر_آخری.p + EXTRA_ڈھیر; o++)
      setnilvalue2s(o);  /* clear dead stack slice */
    /* 'remarkupvals' may have removed thread from 'twups' list */
    اگر (!isintwups(th) && th->openupval != تہی) {
      th->twups = g->twups;  /* link it back to the list */
      g->twups = th;
    }
  }
  واپس 1 + (th->چوٹی.p - th->ڈھیر.p);
}


/*
** traverse one gray object, turning it to black. Return an estimate
** of the number of slots traversed.
*/
ساکن l_mem propagatemark (global_State *g) {
  GCObject *o = g->سرمئی;
  nw2black(o);
  g->سرمئی = *getgclist(o);  /* remove from 'gray' list */
  انتخاب (o->tt) {
    حالت LUA_VTABLE: واپس traversetable(g, gco2t(o));
    حالت LUA_VUSERDATA: واپس traverseudata(g, gco2u(o));
    حالت LUA_VLCL: واپس traverseLclosure(g, gco2lcl(o));
    حالت LUA_VCCL: واپس traverseCclosure(g, gco2ccl(o));
    حالت LUA_VPROTO: واپس traverseproto(g, gco2p(o));
    حالت LUA_VTHREAD: واپس traversethread(g, gco2th(o));
    طےشدہ: lua_assert(0); واپس 0;
  }
}


ساکن خالی propagateall (global_State *g) {
  جب (g->سرمئی)
    propagatemark(g);
}


/*
** Traverse all ephemeron tables propagating marks from keys to values.
** Repeat until it converges, that is, nothing new is marked. 'dir'
** inverts the direction of the traversals, trying to speed up
** convergence on chains in the same table.
*/
ساکن خالی convergeephemerons (global_State *g) {
  صحیح بدلا;
  صحیح dir = 0;
  کرو {
    GCObject *w;
    GCObject *اگلا = g->ephemeron;  /* get ephemeron list */
    g->ephemeron = تہی;  /* tables may return to this list when traversed */
    بدلا = 0;
    جب ((w = اگلا) != تہی) {  /* for each ephemeron table */
      جدول *h = gco2t(w);
      اگلا = h->gclist;  /* list is rebuilt during loop */
      nw2black(h);  /* out of the list (for now) */
      اگر (traverseephemeron(g, h, dir)) {  /* marked some value? */
        propagateall(g);  /* propagate changes */
        بدلا = 1;  /* will have to revisit all ephemeron tables */
      }
    }
    dir = !dir;  /* invert direction next time */
  } جب (بدلا);  /* repeat until no more changes */
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
ساکن خالی clearbykeys (global_State *g, GCObject *l) {
  برائے (; l; l = gco2t(l)->gclist) {
    جدول *h = gco2t(l);
    گرہ_2 *حد_بندی = gnodelast(h);
    گرہ_2 *n;
    برائے (n = gnode(h, 0); n < حد_بندی; n++) {
      اگر (iscleared(g, gckeyN(n)))  /* unmarked key? */
        setempty(gval(n));  /* remove entry */
      اگر (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


/*
** clear entries with unmarked values from all weaktables in list 'l' up
** to element 'f'
*/
ساکن خالی clearbyvalues (global_State *g, GCObject *l, GCObject *f) {
  برائے (; l != f; l = gco2t(l)->gclist) {
    جدول *h = gco2t(l);
    گرہ_2 *n, *حد_بندی = gnodelast(h);
    غیرعلامتی صحیح i;
    غیرعلامتی صحیح asize = h->asize;
    برائے (i = 0; i < asize; i++) {
      GCObject *o = gcvalarr(h, i);
      اگر (iscleared(g, o))  /* value was collected? */
        *get_Arr_لیبل(h, i) = LUA_VEMPTY;  /* remove entry */
    }
    برائے (n = gnode(h, 0); n < حد_بندی; n++) {
      اگر (iscleared(g, gcvalueN(gval(n))))  /* unmarked value? */
        setempty(gval(n));  /* remove entry */
      اگر (isempty(gval(n)))  /* is entry empty? */
        clearkey(n);  /* clear its key */
    }
  }
}


ساکن خالی freeupval (lua_State *L, اوپر_Val *uv) {
  اگر (upisopen(uv))
    luaF_unlinkupval(uv);
  luaM_free(L, uv);
}


ساکن خالی freeobj (lua_State *L, GCObject *o) {
  assert_رمز(l_mem newmem = gettotalbytes(G(L)) - objsize(o));
  انتخاب (o->tt) {
    حالت LUA_VPROTO:
      luaF_freeproto(L, gco2p(o));
      توڑو;
    حالت LUA_VUPVAL:
      freeupval(L, gco2upv(o));
      توڑو;
    حالت LUA_VLCL: {
      LClosure *cl = gco2lcl(o);
      luaM_freemem(L, cl, سائز_Lclosure(cl->nupvalues));
      توڑو;
    }
    حالت LUA_VCCL: {
      CClosure *cl = gco2ccl(o);
      luaM_freemem(L, cl, سائز_Cclosure(cl->nupvalues));
      توڑو;
    }
    حالت LUA_VTABLE:
      luaH_free(L, gco2t(o));
      توڑو;
    حالت LUA_VTHREAD:
      luaE_freethread(L, gco2th(o));
      توڑو;
    حالت LUA_VUSERDATA: {
      Udata *u = gco2u(o);
      luaM_freemem(L, o, sizeudata(u->nuvalue, u->len));
      توڑو;
    }
    حالت LUA_VSHRSTR: {
      TString *ts = gco2ts(o);
      luaS_remove(L, ts);  /* remove it from hash table */
      luaM_freemem(L, ts, sizestrshr(cast_uint(ts->shrlen)));
      توڑو;
    }
    حالت LUA_VLNGSTR: {
      TString *ts = gco2ts(o);
      اگر (ts->shrlen == LSTRMEM)  /* must free external string? */
        (*ts->falloc)(ts->ud, ts->contents, ts->u.lnglen + 1, 0);
      luaM_freemem(L, ts, luaS_sizelngstr(ts->u.lnglen, ts->shrlen));
      توڑو;
    }
    طےشدہ: lua_assert(0);
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
ساکن GCObject **sweeplist (lua_State *L, GCObject **p, l_mem countin) {
  global_State *g = G(L);
  صحیح ow = otherwhite(g);
  صحیح white = luaC_white(g);  /* current white */
  جب (*p != تہی && countin-- > 0) {
    GCObject *curr = *p;
    صحیح marked = curr->marked;
    اگر (isdeadm(ow, marked)) {  /* is 'curr' dead? */
      *p = curr->اگلا;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ورنہ {  /* change mark to 'white' and age to 'new' */
      curr->marked = cast_byte((marked & ~maskgcbits) | white | G_NEW);
      p = &curr->اگلا;  /* go to next element */
    }
  }
  واپس (*p == تہی) ? تہی : p;
}


/*
** sweep a list until a live object (or end of list)
*/
ساکن GCObject **sweeptolive (lua_State *L, GCObject **p) {
  GCObject **old = p;
  کرو {
    p = sweeplist(L, p, 1);
  } جب (p == old);
  واپس p;
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
ساکن خالی checkSizes (lua_State *L, global_State *g) {
  اگر (!g->gcemergency) {
    اگر (g->strt.nuse < g->strt.سائز / 4)  /* string table too big? */
      luaS_resize(L, g->strt.سائز / 2);
  }
}


/*
** Get the next udata to be finalized from the 'tobefnz' list, and
** link it back into the 'allgc' list.
*/
ساکن GCObject *udata2finalize (global_State *g) {
  GCObject *o = g->tobefnz;  /* get first element */
  lua_assert(tofinalize(o));
  g->tobefnz = o->اگلا;  /* remove it from 'tobefnz' list */
  o->اگلا = g->allgc;  /* return it to 'allgc' list */
  g->allgc = o;
  resetbit(o->marked, FINALIZEDBIT);  /* object is "normal" again */
  اگر (issweepphase(g))
    makewhite(g, o);  /* "sweep" object */
  ورنہ اگر (getage(o) == G_OLD1)
    g->firstold1 = o;  /* it is the first OLD1 object in the list */
  واپس o;
}


ساکن خالی dothecall (lua_State *L, خالی *ud) {
  غیر_مستعمل(ud);
  luaD_callnoyield(L, L->چوٹی.p - 2, 0);
}


ساکن خالی GCTM (lua_State *L) {
  global_State *g = G(L);
  ثابت TValue *tm;
  TValue v;
  lua_assert(!g->gcemergency);
  setgcovalue(L, &v, udata2finalize(g));
  tm = luaT_gettmbyobj(L, &v, TM_GC);
  اگر (!notm(tm)) {  /* is there a finalizer? */
    TStatus حالت_2;
    lu_byte oldah = L->allowhook;
    lu_byte oldgcstp  = g->gcstp;
    g->gcstp |= GCSTPGC;  /* avoid GC steps */
    L->allowhook = 0;  /* stop debug hooks during GC metamethod */
    setobj2s(L, L->چوٹی.p++, tm);  /* push finalizer... */
    setobj2s(L, L->چوٹی.p++, &v);  /* ... and its argument */
    L->ci->callstatus |= CIST_FIN;  /* will run a finalizer */
    حالت_2 = luaD_pcall(L, dothecall, تہی, savestack(L, L->چوٹی.p - 2), 0);
    L->ci->callstatus &= ~CIST_FIN;  /* not running a finalizer anymore */
    L->allowhook = oldah;  /* restore hooks */
    g->gcstp = oldgcstp;  /* restore state */
    اگر (l_unlikely(حالت_2 != LUA_OK)) {  /* error while running __gc? */
      luaE_warnerror(L, "__gc");
      L->چوٹی.p--;  /* pops error object */
    }
  }
}


/*
** call all pending finalizers
*/
ساکن خالی callallpendingfinalizers (lua_State *L) {
  global_State *g = G(L);
  جب (g->tobefnz)
    GCTM(L);
}


/*
** find last 'next' field in list 'p' list (to add elements in its end)
*/
ساکن GCObject **findlast (GCObject **p) {
  جب (*p != تہی)
    p = &(*p)->اگلا;
  واپس p;
}


/*
** Move all unreachable objects (or 'all' objects) that need
** finalization from list 'finobj' to list 'tobefnz' (to be finalized).
** (Note that objects after 'finobjold1' cannot be white, so they
** don't need to be traversed. In incremental mode, 'finobjold1' is NULL,
** so the whole list is traversed.)
*/
ساکن خالی separatetobefnz (global_State *g, صحیح all) {
  GCObject *curr;
  GCObject **p = &g->finobj;
  GCObject **lastnext = findlast(&g->tobefnz);
  جب ((curr = *p) != g->finobjold1) {  /* traverse all finalizable objects */
    lua_assert(tofinalize(curr));
    اگر (!(iswhite(curr) || all))  /* not being collected? */
      p = &curr->اگلا;  /* don't bother with it */
    ورنہ {
      اگر (curr == g->finobjsur)  /* removing 'finobjsur'? */
        g->finobjsur = curr->اگلا;  /* correct it */
      *p = curr->اگلا;  /* remove 'curr' from 'finobj' list */
      curr->اگلا = *lastnext;  /* link at the end of 'tobefnz' list */
      *lastnext = curr;
      lastnext = &curr->اگلا;
    }
  }
}


/*
** If pointer 'p' points to 'o', move it to the next element.
*/
ساکن خالی checkpointer (GCObject **p, GCObject *o) {
  اگر (o == *p)
    *p = o->اگلا;
}


/*
** Correct pointers to objects inside 'allgc' list when
** object 'o' is being removed from the list.
*/
ساکن خالی correctpointers (global_State *g, GCObject *o) {
  checkpointer(&g->survival, o);
  checkpointer(&g->old1, o);
  checkpointer(&g->reallyold, o);
  checkpointer(&g->firstold1, o);
}


/*
** if object 'o' has a finalizer, remove it from 'allgc' list (must
** search the list to find it) and link it in 'finobj' list.
*/
خالی luaC_checkfinalizer (lua_State *L, GCObject *o, جدول *mt) {
  global_State *g = G(L);
  اگر (tofinalize(o) ||                 /* obj. is already marked... */
      gfasttm(g, mt, TM_GC) == تہی ||    /* or has no finalizer... */
      (g->gcstp & GCSTPCLS))                   /* or closing state? */
    واپس;  /* nothing to be done */
  ورنہ {  /* move 'o' to 'finobj' list */
    GCObject **p;
    اگر (issweepphase(g)) {
      makewhite(g, o);  /* "sweep" object 'o' */
      اگر (g->sweepgc == &o->اگلا)  /* should not remove 'sweepgc' object */
        g->sweepgc = sweeptolive(L, g->sweepgc);  /* change 'sweepgc' */
    }
    ورنہ
      correctpointers(g, o);
    /* search for pointer pointing to 'o' */
    برائے (p = &g->allgc; *p != o; p = &(*p)->اگلا) { /*خالی_خانے*/ }
    *p = o->اگلا;  /* remove 'o' from 'allgc' list */
    o->اگلا = g->finobj;  /* link it in 'finobj' list */
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
ساکن خالی setpause (global_State *g) {
  l_mem حد = applygcparam(g, PAUSE, g->GCmarked);
  l_mem debt = حد - gettotalbytes(g);
  اگر (debt < 0) debt = 0;
  luaE_setdebt(g, debt);
}


/*
** Sweep a list of objects to enter generational mode.  Deletes dead
** objects and turns the non dead to old. All non-dead threads---which
** are now old---must be in a gray list. Everything else is not in a
** gray list. Open upvalues are also kept gray.
*/
ساکن خالی sweep2old (lua_State *L, GCObject **p) {
  GCObject *curr;
  global_State *g = G(L);
  جب ((curr = *p) != تہی) {
    اگر (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(isdead(g, curr));
      *p = curr->اگلا;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ورنہ {  /* all surviving objects become old */
      setage(curr, G_OLD);
      اگر (curr->tt == LUA_VTHREAD) {  /* threads must be watched */
        lua_State *th = gco2th(curr);
        linkgclist(th, g->grayagain);  /* insert into 'grayagain' list */
      }
      ورنہ اگر (curr->tt == LUA_VUPVAL && upisopen(gco2upv(curr)))
        set2gray(curr);  /* open upvalues are always gray */
      ورنہ  /* everything else is black */
        nw2black(curr);
      p = &curr->اگلا;  /* go to next element */
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
ساکن GCObject **sweepgen (lua_State *L, global_State *g, GCObject **p,
                            GCObject *حد_بندی, GCObject **pfirstold1,
                            l_mem *paddedold) {
  ساکن ثابت lu_byte nextage[] = {
    G_SURVIVAL,  /* from G_NEW */
    G_OLD1,      /* from G_SURVIVAL */
    G_OLD1,      /* from G_OLD0 */
    G_OLD,       /* from G_OLD1 */
    G_OLD,       /* from G_OLD (do not change) */
    G_TOUCHED1,  /* from G_TOUCHED1 (do not change) */
    G_TOUCHED2   /* from G_TOUCHED2 (do not change) */
  };
  l_mem addedold = 0;
  صحیح white = luaC_white(g);
  GCObject *curr;
  جب ((curr = *p) != حد_بندی) {
    اگر (iswhite(curr)) {  /* is 'curr' dead? */
      lua_assert(!isold(curr) && isdead(g, curr));
      *p = curr->اگلا;  /* remove 'curr' from list */
      freeobj(L, curr);  /* erase 'curr' */
    }
    ورنہ {  /* correct mark and age */
      صحیح age = getage(curr);
      اگر (age == G_NEW) {  /* new objects go back to white */
        صحیح marked = curr->marked & ~maskgcbits;  /* erase GC bits */
        curr->marked = cast_byte(marked | G_SURVIVAL | white);
      }
      ورنہ {  /* all other objects will be old, and so keep their color */
        lua_assert(age != G_OLD1);  /* advanced in 'markold' */
        setage(curr, nextage[age]);
        اگر (getage(curr) == G_OLD1) {
          addedold += objsize(curr);  /* bytes becoming old */
          اگر (*pfirstold1 == تہی)
            *pfirstold1 = curr;  /* first OLD1 object in the list */
        }
      }
      p = &curr->اگلا;  /* go to next element */
    }
  }
  *paddedold += addedold;
  واپس p;
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
ساکن GCObject **correctgraylist (GCObject **p) {
  GCObject *curr;
  جب ((curr = *p) != تہی) {
    GCObject **اگلا = getgclist(curr);
    اگر (iswhite(curr))
      جاؤ remove;  /* remove all white objects */
    ورنہ اگر (getage(curr) == G_TOUCHED1) {  /* touched in this cycle? */
      lua_assert(isgray(curr));
      nw2black(curr);  /* make it black, for next barrier */
      setage(curr, G_TOUCHED2);
      جاؤ remain;  /* keep it in the list and go to next element */
    }
    ورنہ اگر (curr->tt == LUA_VTHREAD) {
      lua_assert(isgray(curr));
      جاؤ remain;  /* keep non-white threads on the list */
    }
    ورنہ {  /* everything else is removed */
      lua_assert(isold(curr));  /* young objects should be white here */
      اگر (getage(curr) == G_TOUCHED2)  /* advance from TOUCHED2... */
        setage(curr, G_OLD);  /* ... to OLD */
      nw2black(curr);  /* make object black (to be removed) */
      جاؤ remove;
    }
    remove: *p = *اگلا; جاری;
    remain: p = اگلا; جاری;
  }
  واپس p;
}


/*
** Correct all gray lists, coalescing them into 'grayagain'.
*/
ساکن خالی correctgraylists (global_State *g) {
  GCObject **list = correctgraylist(&g->grayagain);
  *list = g->weak; g->weak = تہی;
  list = correctgraylist(list);
  *list = g->allweak; g->allweak = تہی;
  list = correctgraylist(list);
  *list = g->ephemeron; g->ephemeron = تہی;
  correctgraylist(list);
}


/*
** Mark black 'OLD1' objects when starting a new young collection.
** Gray objects are already in some gray list, and so will be visited in
** the atomic step.
*/
ساکن خالی markold (global_State *g, GCObject *from, GCObject *مقصد) {
  GCObject *p;
  برائے (p = from; p != مقصد; p = p->اگلا) {
    اگر (getage(p) == G_OLD1) {
      lua_assert(!iswhite(p));
      setage(p, G_OLD);  /* now they are old */
      اگر (isblack(p))
        reallymarkobject(g, p);
    }
  }
}


/*
** Finish a young-generation collection.
*/
ساکن خالی finishgencycle (lua_State *L, global_State *g) {
  correctgraylists(g);
  checkSizes(L, g);
  g->gcstate = GCSpropagate;  /* skip restart */
  اگر (g->tobefnz != تہی && !g->gcemergency && luaD_checkminstack(L))
    callallpendingfinalizers(L);
}


/*
** Shifts from a minor collection to major collections. It starts in
** the "sweep all" state to clear all objects, which are mostly black
** in generational mode.
*/
ساکن خالی minor2inc (lua_State *L, global_State *g, lu_byte قسم) {
  g->GCmajorminor = g->GCmarked;  /* number of live bytes */
  g->gckind = قسم;
  g->reallyold = g->old1 = g->survival = تہی;
  g->finobjrold = g->finobjold1 = g->finobjsur = تہی;
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
ساکن صحیح checkminormajor (global_State *g) {
  l_mem حد_بندی = applygcparam(g, MINORMAJOR, g->GCmajorminor);
  اگر (حد_بندی == 0)
    واپس 0;  /* special case: 'minormajor' 0 stops major collections */
  واپس (g->GCmarked >= حد_بندی);
}

/*
** Does a young collection. First, mark 'OLD1' objects. Then does the
** atomic step. Then, check whether to continue in minor mode. If so,
** sweep all lists and advance pointers. Finally, finish the collection.
*/
ساکن خالی youngcollection (lua_State *L, global_State *g) {
  l_mem addedold1 = 0;
  l_mem marked = g->GCmarked;  /* preserve 'g->GCmarked' */
  GCObject **psurvival;  /* to point to first non-dead survival object */
  GCObject *dummy;  /* dummy out parameter to 'sweepgen' */
  lua_assert(g->gcstate == GCSpropagate);
  اگر (g->firstold1) {  /* are there regular OLD1 objects? */
    markold(g, g->firstold1, g->reallyold);  /* mark them */
    g->firstold1 = تہی;  /* no more OLD1 objects (for now) */
  }
  markold(g, g->finobj, g->finobjrold);
  markold(g, g->tobefnz, تہی);

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
  dummy = تہی;  /* no 'firstold1' optimization for 'finobj' lists */
  psurvival = sweepgen(L, g, &g->finobj, g->finobjsur, &dummy, &addedold1);
  /* sweep 'survival' */
  sweepgen(L, g, psurvival, g->finobjold1, &dummy, &addedold1);
  g->finobjrold = g->finobjold1;
  g->finobjold1 = *psurvival;  /* 'survival' survivals are old now */
  g->finobjsur = g->finobj;  /* all news are survivals */

  sweepgen(L, g, &g->tobefnz, تہی, &dummy, &addedold1);

  /* keep total number of added old1 bytes */
  g->GCmarked = marked + addedold1;

  /* decide whether to shift to major mode */
  اگر (checkminormajor(g)) {
    minor2inc(L, g, KGC_GENMAJOR);  /* go to major mode */
    g->GCmarked = 0;  /* avoid pause in first major cycle (see 'setpause') */
  }
  ورنہ
    finishgencycle(L, g);  /* still in minor mode; finish it */
}


/*
** Clears all gray lists, sweeps objects, and prepare sublists to enter
** generational mode. The sweeps remove dead objects and turn all
** surviving objects to old. Threads go back to 'grayagain'; everything
** else is turned black (not in any gray list).
*/
ساکن خالی atomic2gen (lua_State *L, global_State *g) {
  cleargraylists(g);
  /* sweep all elements making them old */
  g->gcstate = GCSswpallgc;
  sweep2old(L, &g->allgc);
  /* everything alive now is old */
  g->reallyold = g->old1 = g->survival = g->allgc;
  g->firstold1 = تہی;  /* there are no OLD1 objects anywhere */

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
ساکن خالی setminordebt (global_State *g) {
  luaE_setdebt(g, applygcparam(g, MINORMUL, g->GCmajorminor));
}


/*
** Enter generational mode. Must go until the end of an atomic cycle
** to ensure that all objects are correctly marked and weak tables
** are cleared. Then, turn all objects into old and finishes the
** collection.
*/
ساکن خالی entergen (lua_State *L, global_State *g) {
  luaC_runtilstate(L, GCSpause, 1);  /* prepare to start a new cycle */
  luaC_runtilstate(L, GCSpropagate, 1);  /* start new cycle */
  atomic(L);  /* propagates all and then do the atomic stuff */
  atomic2gen(L, g);
  setminordebt(g);  /* set debt assuming next cycle will be minor */
}


/*
** Change collector mode to 'newmode'.
*/
خالی luaC_changemode (lua_State *L, صحیح newmode) {
  global_State *g = G(L);
  اگر (g->gckind == KGC_GENMAJOR)  /* doing major collections? */
    g->gckind = KGC_INC;  /* already incremental but in name */
  اگر (newmode != g->gckind) {  /* does it need to change? */
    اگر (newmode == KGC_INC)  /* entering incremental mode? */
      minor2inc(L, g, KGC_INC);  /* entering incremental mode */
    ورنہ {
      lua_assert(newmode == KGC_GENMINOR);
      entergen(L, g);
    }
  }
}


/*
** Does a full collection in generational mode.
*/
ساکن خالی fullgen (lua_State *L, global_State *g) {
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
ساکن صحیح checkmajorminor (lua_State *L, global_State *g) {
  اگر (g->gckind == KGC_GENMAJOR) {  /* generational mode? */
    l_mem numbytes = gettotalbytes(g);
    l_mem addedbytes = numbytes - g->GCmajorminor;
    l_mem حد_بندی = (addedbytes < 0) ? 0
                                   : applygcparam(g, MAJORMINOR, addedbytes);
    l_mem tobecollected = numbytes - g->GCmarked;
    اگر (tobecollected > حد_بندی) {
      atomic2gen(L, g);  /* return to generational mode */
      setminordebt(g);
      واپس 1;  /* exit incremental collection */
    }
  }
  g->GCmajorminor = g->GCmarked;  /* prepare for next collection */
  واپس 0;  /* stay doing incremental collections */
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
ساکن خالی entersweep (lua_State *L) {
  global_State *g = G(L);
  g->gcstate = GCSswpallgc;
  lua_assert(g->sweepgc == تہی);
  g->sweepgc = sweeptolive(L, &g->allgc);
}


/*
** Delete all objects in list 'p' until (but not including) object
** 'limit'.
*/
ساکن خالی deletelist (lua_State *L, GCObject *p, GCObject *حد_بندی) {
  جب (p != حد_بندی) {
    GCObject *اگلا = p->اگلا;
    freeobj(L, p);
    p = اگلا;
  }
}


/*
** Call all finalizers of the objects in the given Lua state, and
** then free all objects, except for the main thread.
*/
خالی luaC_freeallobjects (lua_State *L) {
  global_State *g = G(L);
  g->gcstp = GCSTPCLS;  /* no extra finalizers after here */
  luaC_changemode(L, KGC_INC);
  separatetobefnz(g, 1);  /* separate all objects with finalizers */
  lua_assert(g->finobj == تہی);
  callallpendingfinalizers(L);
  deletelist(L, g->allgc, obj2gco(mainthread(g)));
  lua_assert(g->finobj == تہی);  /* no new finalizers */
  deletelist(L, g->fixedgc, تہی);  /* collect fixed objects */
  lua_assert(g->strt.nuse == 0);
}


ساکن خالی atomic (lua_State *L) {
  global_State *g = G(L);
  GCObject *origweak, *origall;
  GCObject *grayagain = g->grayagain;  /* save original list */
  g->grayagain = تہی;
  lua_assert(g->ephemeron == تہی && g->weak == تہی);
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
  g->سرمئی = grayagain;
  propagateall(g);  /* traverse 'grayagain' list */
  convergeephemerons(g);
  /* at this point, all strongly accessible objects are marked. */
  /* Clear values from weak tables, before checking finalizers */
  clearbyvalues(g, g->weak, تہی);
  clearbyvalues(g, g->allweak, تہی);
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
  lua_assert(g->سرمئی == تہی);
}


/*
** Do a sweep step. The normal case (not fast) sweeps at most GCSWEEPMAX
** elements. The fast case sweeps the whole list.
*/
ساکن خالی sweepstep (lua_State *L, global_State *g,
                       lu_byte nextstate, GCObject **nextlist, صحیح fast) {
  اگر (g->sweepgc)
    g->sweepgc = sweeplist(L, g->sweepgc, fast ? MAX_LMEM : GCSWEEPMAX);
  ورنہ {  /* enter next state */
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

#تعریف_کرو step2pause	-3  /* finished collection; entered pause state */
#تعریف_کرو atomicstep	-2  /* atomic step */
#تعریف_کرو step2minor	-1  /* moved to minor collections */


ساکن l_mem singlestep (lua_State *L, صحیح fast) {
  global_State *g = G(L);
  l_mem stepresult;
  lua_assert(!g->gcstopem);  /* collector is not reentrant */
  g->gcstopem = 1;  /* no emergency collections while collecting */
  انتخاب (g->gcstate) {
    حالت GCSpause: {
      restartcollection(g);
      g->gcstate = GCSpropagate;
      stepresult = 1;
      توڑو;
    }
    حالت GCSpropagate: {
      اگر (fast || g->سرمئی == تہی) {
        g->gcstate = GCSenteratomic;  /* finish propagate phase */
        stepresult = 1;
      }
      ورنہ
        stepresult = propagatemark(g);  /* traverse one gray object */
      توڑو;
    }
    حالت GCSenteratomic: {
      atomic(L);
      اگر (checkmajorminor(L, g))
        stepresult = step2minor;
      ورنہ {
        entersweep(L);
        stepresult = atomicstep;
      }
      توڑو;
    }
    حالت GCSswpallgc: {  /* sweep "regular" objects */
      sweepstep(L, g, GCSswpfinobj, &g->finobj, fast);
      stepresult = GCSWEEPMAX;
      توڑو;
    }
    حالت GCSswpfinobj: {  /* sweep objects with finalizers */
      sweepstep(L, g, GCSswptobefnz, &g->tobefnz, fast);
      stepresult = GCSWEEPMAX;
      توڑو;
    }
    حالت GCSswptobefnz: {  /* sweep objects to be finalized */
      sweepstep(L, g, GCSswpend, تہی, fast);
      stepresult = GCSWEEPMAX;
      توڑو;
    }
    حالت GCSswpend: {  /* finish sweeps */
      checkSizes(L, g);
      g->gcstate = GCScallfin;
      stepresult = GCSWEEPMAX;
      توڑو;
    }
    حالت GCScallfin: {  /* call finalizers */
      اگر (g->tobefnz && !g->gcemergency && luaD_checkminstack(L)) {
        g->gcstopem = 0;  /* ok collections during finalizers */
        GCTM(L);  /* call one finalizer */
        stepresult = CWUFIN;
      }
      ورنہ {  /* no more finalizers or emergency mode or not enough stack
                 to run finalizers */
        g->gcstate = GCSpause;  /* finish collection */
        stepresult = step2pause;
      }
      توڑو;
    }
    طےشدہ: lua_assert(0); واپس 0;
  }
  g->gcstopem = 0;
  واپس stepresult;
}


/*
** Advances the garbage collector until it reaches the given state.
** (The option 'fast' is only for testing; in normal code, 'fast'
** here is always true.)
*/
خالی luaC_runtilstate (lua_State *L, صحیح state, صحیح fast) {
  global_State *g = G(L);
  lua_assert(g->gckind == KGC_INC);
  جب (state != g->gcstate)
    singlestep(L, fast);
}



/*
** Performs a basic incremental step. The step size is
** converted from bytes to "units of work"; then the function loops
** running single steps until adding that many units of work or
** finishing a cycle (pause state). Finally, it sets the debt that
** controls when next step will be performed.
*/
ساکن خالی incstep (lua_State *L, global_State *g) {
  l_mem stepsize = applygcparam(g, STEPSIZE, 100);
  l_mem work2do = applygcparam(g, STEPMUL, stepsize / cast_int(حجم(خالی*)));
  l_mem stres;
  صحیح fast = (work2do == 0);  /* special case: do a full collection */
  کرو {  /* repeat until enough work */
    stres = singlestep(L, fast);  /* perform one single step */
    اگر (stres == step2minor)  /* returned to minor collections? */
      واپس;  /* nothing else to be done here */
    ورنہ اگر (stres == step2pause || (stres == atomicstep && !fast))
      توڑو;  /* end of cycle or atomic */
    ورنہ
      work2do -= stres;
  } جب (fast || work2do > 0);
  اگر (g->gcstate == GCSpause)
    setpause(g);  /* pause until next cycle */
  ورنہ
    luaE_setdebt(g, stepsize);
}


#اگر !defined(luai_tracegc)
#تعریف_کرو luai_tracegc(L,f)		((خالی)0)
#اگر_ختم

/*
** Performs a basic GC step if collector is running. (If collector was
** stopped by the user, set a reasonable debt to avoid it being called
** at every single check.)
*/
خالی lua_C_قدم (lua_State *L) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  اگر (!gcrunning(g)) {  /* not running? */
    اگر (g->gcstp & GCSTPUSR)  /* stopped by the user? */
      luaE_setdebt(g, 20000);
  }
  ورنہ {
    luai_tracegc(L, 1);  /* for internal debugging */
    انتخاب (g->gckind) {
      حالت KGC_INC: حالت KGC_GENMAJOR:
        incstep(L, g);
        توڑو;
      حالت KGC_GENMINOR:
        youngcollection(L, g);
        setminordebt(g);
        توڑو;
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
ساکن خالی fullinc (lua_State *L, global_State *g) {
  اگر (keepinvariant(g))  /* black objects? */
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
خالی luaC_fullgc (lua_State *L, صحیح isemergency) {
  global_State *g = G(L);
  lua_assert(!g->gcemergency);
  g->gcemergency = cast_byte(isemergency);  /* set flag */
  انتخاب (g->gckind) {
    حالت KGC_GENMINOR: fullgen(L, g); توڑو;
    حالت KGC_INC: fullinc(L, g); توڑو;
    حالت KGC_GENMAJOR:
      g->gckind = KGC_INC;
      fullinc(L, g);
      g->gckind = KGC_GENMAJOR;
      توڑو;
  }
  g->gcemergency = 0;
}

/* }====================================================== */


