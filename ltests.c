/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <setjmp.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lapi.h"
#અન્તર્ભાવ "lauxlib.h"
#અન્તર્ભાવ "lcode.h"
#અન્તર્ભાવ "lctype.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lfunc.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lopcodes.h"
#અન્તર્ભાવ "lopnames.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#જો defined(LUA_DEBUG)


શૂન્ય *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


સ્થિર પૂર્ણાંક runC (lua_State *L, lua_State *L1, અચલ અક્ષર *pc);


સ્થિર શૂન્ય setnameval (lua_State *L, અચલ અક્ષર *name, પૂર્ણાંક val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


સ્થિર શૂન્ય pushobject (lua_State *L, અચલ TValue *o) {
  setobj2s(L, L->ટોચ.p, o);
  api_incr_ટોચ(L);
}


સ્થિર શૂન્ય badexit (અચલ અક્ષર *fmt, અચલ અક્ષર *s1, અચલ અક્ષર *s2) {
  fprintf(stderr, fmt, s1);
  જો (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


સ્થિર પૂર્ણાંક tpanic (lua_State *L) {
  અચલ અક્ષર *સંદેશ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  પાછા (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   સંદેશ, ખાલી),
          0);  /* do not return to Lua */
}


/*
** Warning function for tests. First, it concatenates all parts of
** a warning in buffer 'buff'. Then, it has three modes:
** - 0.normal: messages starting with '#' are shown on standard output;
** - other messages abort the tests (they represent real warning
** conditions; the standard tests should not generate these conditions
** unexpectedly);
** - 1.allow: all messages are shown;
** - 2.store: all warnings go to the global '_WARN';
*/
સ્થિર શૂન્ય warnf (શૂન્ય *ud, અચલ અક્ષર *સંદેશ, પૂર્ણાંક tocont) {
  lua_State *L = cast(lua_State *, ud);
  સ્થિર અક્ષર buff[200] = "";  /* should be enough for tests... */
  સ્થિર પૂર્ણાંક onoff = 0;
  સ્થિર પૂર્ણાંક mode = 0;  /* start in normal mode */
  સ્થિર પૂર્ણાંક lasttocont = 0;
  જો (!lasttocont && !tocont && *સંદેશ == '@') {  /* control message? */
    જો (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", સંદેશ, buff);
    જો (strcmp(સંદેશ, "@off") == 0)
      onoff = 0;
    નહીંતર જો (strcmp(સંદેશ, "@on") == 0)
      onoff = 1;
    નહીંતર જો (strcmp(સંદેશ, "@normal") == 0)
      mode = 0;
    નહીંતર જો (strcmp(સંદેશ, "@allow") == 0)
      mode = 1;
    નહીંતર જો (strcmp(સંદેશ, "@store") == 0)
      mode = 2;
    નહીંતર
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              સંદેશ, ખાલી);
    પાછા;
  }
  lasttocont = tocont;
  જો (strlen(સંદેશ) >= કદ(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", સંદેશ, buff);
  strcat(buff, સંદેશ);  /* add new message to current warning */
  જો (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    જો (!lua_toboolean(L, -1))
      lua_કાઢો(L, 1);  /* ok, no previous unexpected warning */
    નહીંતર {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    પસંદગી (mode) {
      સ્થિતિ 0: {  /* normal */
        જો (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ખાલી);
      }  /* FALLTHROUGH */
      સ્થિતિ 1: {  /* allow */
        જો (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        તોડો;
      }
      સ્થિતિ 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        તોડો;
      }
    }
    buff[0] = '\0';  /* prepare buffer for next warning */
  }
}


/*
** {======================================================================
** Controlled version for realloc.
** =======================================================================
*/

#define MARK		0x55  /* 01010101 (a nice pattern) */

પ્રકારવ્યાખ્યા સંઘ memHeader {
  LUAI_MAXALIGN;
  સંરચના {
    size_t size;
    પૂર્ણાંક type;
  } d;
} memHeader;


#જો !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#નહીંતર

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


સ્થિર શૂન્ય freeblock (Memcontrol *mc, memHeader *block) {
  જો (block) {
    size_t size = block->d.size;
    પૂર્ણાંક i;
    માટે (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, કદ(memHeader) + size + MARKSIZE);  /* erase block */
    મુક્ત(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


શૂન્ય *debug_realloc (શૂન્ય *ud, શૂન્ય *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  પૂર્ણાંક type;
  જો (mc->memlimit == 0) {  /* first time? */
    અક્ષર *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, ખાલી, 10) : ULONG_MAX;
  }
  જો (block == ખાલી) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  નહીંતર {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  જો (size == 0) {
    freeblock(mc, block);
    પાછા ખાલી;
  }
  જો (mc->failnext) {
    mc->failnext = 0;
    પાછા ખાલી;  /* fake a single memory allocation error */
  }
  જો (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    જો (mc->countlimit == 0)
      પાછા ખાલી;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  જો (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    પાછા ખાલી;  /* fake a memory allocation error */
  નહીંતર {
    memHeader *newblock;
    પૂર્ણાંક i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = કદ(memHeader) + size + MARKSIZE;
    જો (realsize < size) પાછા ખાલી;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    જો (newblock == ખાલી)
      પાછા ખાલી;  /* really out of memory? */
    જો (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    માટે (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    જો (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    પાછા newblock + 1;
  }
}


/* }====================================================================== */



/*
** {=====================================================================
** Functions to check memory consistency.
** Most of these checks are done through asserts, so this code does
** not make sense with asserts off. For this reason, it uses 'assert'
** directly, instead of 'lua_assert'.
** ======================================================================
*/

#અન્તર્ભાવ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
સ્થિર પૂર્ણાંક testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  જો (isdead(g,t)) પાછા 0;
  જો (issweepphase(g))
    પાછા 1;  /* no invariants */
  નહીંતર જો (g->gckind != KGC_GENMINOR)
    પાછા !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  નહીંતર {  /* generational mode */
    જો ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      પાછા 0;
    જો ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      પાછા 0;
    પાછા 1;
  }
}


સ્થિર શૂન્ય printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (શૂન્ય *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  જો (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


શૂન્ય lua_printobj (lua_State *L, સંરચના GCObject *o) {
  printobj(G(L), o);
}


શૂન્ય lua_printvalue (TValue *v) {
  પસંદગી (ttypetag(v)) {
    સ્થિતિ LUA_VNUMINT: સ્થિતિ LUA_VNUMFLT: {
      અક્ષર buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      તોડો;
    }
    સ્થિતિ LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); તોડો;
    સ્થિતિ LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); તોડો;
    સ્થિતિ LUA_VFALSE:
      printf("%s", "false"); તોડો;
    સ્થિતિ LUA_VTRUE:
      printf("%s", "true"); તોડો;
    સ્થિતિ LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); તોડો;
    સ્થિતિ LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); તોડો;
    સ્થિતિ LUA_VNIL:
      printf("nil"); તોડો;
    સ્થિતિ LUA_VLCF:
      printf("light C function: %p", fvalue(v)); તોડો;
    સ્થિતિ LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); તોડો;
    સ્થિતિ LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); તોડો;
    સ્થિતિ LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); તોડો;
    સ્થિતિ LUA_VTABLE:
      printf("table: %p", hvalue(v)); તોડો;
    મૂળભૂત:
      lua_assert(0);
  }
}


સ્થિર પૂર્ણાંક testobjref (global_State *g, GCObject *f, GCObject *t) {
  પૂર્ણાંક r1 = testobjref1(g, f, t);
  જો (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  પાછા r1;
}


સ્થિર શૂન્ય checkobjref (global_State *g, GCObject *f, GCObject *t) {
    પ્રતિજ્ઞા(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ જો (t) checkobjref(g,f,obj2gco(t)); }


સ્થિર શૂન્ય checkvalref (global_State *g, GCObject *f, અચલ TValue *t) {
  પ્રતિજ્ઞા(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


સ્થિર શૂન્ય checktable (global_State *g, Table *h) {
  unsigned પૂર્ણાંક i;
  unsigned પૂર્ણાંક asize = h->asize;
  ગાંઠ_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  માટે (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  માટે (n = gnode(h, 0); n < limit; n++) {
    જો (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      પ્રતિજ્ઞા(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


સ્થિર શૂન્ય checkudata (global_State *g, Udata *u) {
  પૂર્ણાંક i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  માટે (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


સ્થિર શૂન્ય checkproto (global_State *g, Proto *f) {
  પૂર્ણાંક i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  માટે (i=0; i<f->sizek; i++) {
    જો (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  માટે (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  માટે (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  માટે (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


સ્થિર શૂન્ય checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  પૂર્ણાંક i;
  માટે (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


સ્થિર શૂન્ય checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  પૂર્ણાંક i;
  checkobjrefN(g, clgc, cl->p);
  માટે (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    જો (uv) {
      checkobjrefN(g, clgc, uv);
      જો (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


સ્થિર પૂર્ણાંક lua_checkpc (CallInfo *ci) {
  જો (!isLua(ci)) પાછા 1;
  નહીંતર {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    પાછા p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


સ્થિર શૂન્ય check_ઢગલો (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  પ્રતિજ્ઞા(!isdead(g, L1));
  જો (L1->ઢગલો.p == ખાલી) {  /* incomplete thread? */
    પ્રતિજ્ઞા(L1->openupval == ખાલી && L1->ci == ખાલી);
    પાછા;
  }
  માટે (uv = L1->openupval; uv != ખાલી; uv = uv->u.open.next)
    પ્રતિજ્ઞા(upisopen(uv));  /* must be open */
  પ્રતિજ્ઞા(L1->ટોચ.p <= L1->ઢગલો_last.p);
  પ્રતિજ્ઞા(L1->tbclist.p <= L1->ટોચ.p);
  માટે (ci = L1->ci; ci != ખાલી; ci = ci->previous) {
    પ્રતિજ્ઞા(ci->ટોચ.p <= L1->ઢગલો_last.p);
    પ્રતિજ્ઞા(lua_checkpc(ci));
  }
  માટે (o = L1->ઢગલો.p; o < L1->ઢગલો_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


સ્થિર શૂન્ય checkrefs (global_State *g, GCObject *o) {
  પસંદગી (o->tt) {
    સ્થિતિ LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      તોડો;
    }
    સ્થિતિ LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      તોડો;
    }
    સ્થિતિ LUA_VTABLE: {
      checktable(g, gco2t(o));
      તોડો;
    }
    સ્થિતિ LUA_VTHREAD: {
      check_ઢગલો(g, gco2th(o));
      તોડો;
    }
    સ્થિતિ LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      તોડો;
    }
    સ્થિતિ LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      તોડો;
    }
    સ્થિતિ LUA_VPROTO: {
      checkproto(g, gco2p(o));
      તોડો;
    }
    સ્થિતિ LUA_VSHRSTR:
    સ્થિતિ LUA_VLNGSTR: {
      પ્રતિજ્ઞા(!isgray(o));  /* strings are never gray */
      તોડો;
    }
    મૂળભૂત: પ્રતિજ્ઞા(0);
  }
}


/*
** Check consistency of an object:
** - Dead objects can only happen in the 'allgc' list during a sweep
** phase (controlled by the caller through 'maybedead').
** - During pause, all objects must be white.
** - In generational mode:
**   * objects must be old enough for their lists ('listage').
**   * old objects cannot be white.
**   * old objects must be black, except for 'touched1', 'old0',
**     threads, and open upvalues.
**   * 'touched1' objects must be gray.
*/
સ્થિર શૂન્ય checkobject (global_State *g, GCObject *o, પૂર્ણાંક maybedead,
                         પૂર્ણાંક listage) {
  જો (isdead(g, o))
    પ્રતિજ્ઞા(maybedead);
  નહીંતર {
    પ્રતિજ્ઞા(g->gcstate != GCSpause || iswhite(o));
    જો (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      પ્રતિજ્ઞા(getage(o) >= listage);
      જો (isold(o)) {
        પ્રતિજ્ઞા(!iswhite(o));
        પ્રતિજ્ઞા(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      પ્રતિજ્ઞા(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


સ્થિર l_mem checkgraylist (global_State *g, GCObject *o) {
  પૂર્ણાંક total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  જ્યારે (o) {
    પ્રતિજ્ઞા(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    પ્રતિજ્ઞા(!testbit(o->marked, TESTBIT));
    જો (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    પસંદગી (o->tt) {
      સ્થિતિ LUA_VTABLE: o = gco2t(o)->gclist; તોડો;
      સ્થિતિ LUA_VLCL: o = gco2lcl(o)->gclist; તોડો;
      સ્થિતિ LUA_VCCL: o = gco2ccl(o)->gclist; તોડો;
      સ્થિતિ LUA_VTHREAD: o = gco2th(o)->gclist; તોડો;
      સ્થિતિ LUA_VPROTO: o = gco2p(o)->gclist; તોડો;
      સ્થિતિ LUA_VUSERDATA:
        પ્રતિજ્ઞા(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        તોડો;
      મૂળભૂત: પ્રતિજ્ઞા(0);  /* other objects cannot be in a gray list */
    }
  }
  પાછા total;
}


/*
** Check objects in gray lists.
*/
સ્થિર l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  જો (!keepinvariant(g)) પાછા total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  પાછા total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
સ્થિર શૂન્ય incifingray (global_State *g, GCObject *o, l_mem *ગણતરી) {
  જો (!keepinvariant(g))
    પાછા;  /* gray lists not being kept in these phases */
  જો (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    પ્રતિજ્ઞા(!isgray(o) || upisopen(gco2upv(o)));
    પાછા;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  જો (isgray(o) || getage(o) == G_TOUCHED2) {
    (*ગણતરી)++;
    પ્રતિજ્ઞા(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


સ્થિર l_mem checklist (global_State *g, પૂર્ણાંક maybedead, પૂર્ણાંક tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  માટે (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    પ્રતિજ્ઞા(!tof == !tofinalize(o));
  }
  માટે (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    પ્રતિજ્ઞા(!tof == !tofinalize(o));
  }
  માટે (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    પ્રતિજ્ઞા(!tof == !tofinalize(o));
  }
  માટે (o = reallyold; o != ખાલી; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    પ્રતિજ્ઞા(!tof == !tofinalize(o));
  }
  પાછા total;
}


પૂર્ણાંક lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  પૂર્ણાંક maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  જો (keepinvariant(g)) {
    પ્રતિજ્ઞા(!iswhite(mainthread(g)));
    પ્રતિજ્ઞા(!iswhite(gcvalue(&g->l_registry)));
  }
  પ્રતિજ્ઞા(!isdead(g, gcvalue(&g->l_registry)));
  પ્રતિજ્ઞા(g->sweepgc == ખાલી || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  માટે (o = g->fixedgc; o != ખાલી; o = o->next) {
    પ્રતિજ્ઞા(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  માટે (o = g->tobefnz; o != ખાલી; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    પ્રતિજ્ઞા(tofinalize(o));
    પ્રતિજ્ઞા(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  જો (keepinvariant(g))
    પ્રતિજ્ઞા(totalin == totalshould);
  પાછા 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


સ્થિર અક્ષર *buildop (Proto *p, પૂર્ણાંક pc, અક્ષર *buff) {
  અક્ષર *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  અચલ અક્ષર *name = opnames[o];
  પૂર્ણાંક લીટી = luaG_getfuncline(p, pc);
  પૂર્ણાંક lineinfo = (p->lineinfo != ખાલી) ? p->lineinfo[pc] : 0;
  જો (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  નહીંતર
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", લીટી, pc);
  પસંદગી (getOpMode(o)) {
    સ્થિતિ iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      તોડો;
    સ્થિતિ ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      તોડો;
    સ્થિતિ iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      તોડો;
    સ્થિતિ iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      તોડો;
    સ્થિતિ iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      તોડો;
    સ્થિતિ isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      તોડો;
  }
  પાછા obuff;
}


#જો 0
શૂન્ય luaI_printcode (Proto *pt, પૂર્ણાંક size) {
  પૂર્ણાંક pc;
  માટે (pc=0; pc<size; pc++) {
    અક્ષર buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


શૂન્ય luaI_printinst (Proto *pt, પૂર્ણાંક pc) {
  અક્ષર buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


સ્થિર પૂર્ણાંક listcode (lua_State *L) {
  પૂર્ણાંક pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  માટે (pc=0; pc<p->sizecode; pc++) {
    અક્ષર buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક printcode (lua_State *L) {
  પૂર્ણાંક pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  માટે (pc=0; pc<p->sizecode; pc++) {
    અક્ષર buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  પાછા 0;
}


સ્થિર પૂર્ણાંક listk (lua_State *L) {
  Proto *p;
  પૂર્ણાંક i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  માટે (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક listabslineinfo (lua_State *L) {
  Proto *p;
  પૂર્ણાંક i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ખાલી, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  માટે (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].લીટી);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક listlocals (lua_State *L) {
  Proto *p;
  પૂર્ણાંક pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  પૂર્ણાંક i = 0;
  અચલ અક્ષર *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  જ્યારે ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ખાલી)
    lua_pushstring(L, name);
  પાછા i-1;
}

/* }====================================================== */



શૂન્ય lua_printstack (lua_State *L) {
  પૂર્ણાંક i;
  પૂર્ણાંક n = lua_gettop(L);
  printf("stack: >>\n");
  માટે (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


પૂર્ણાંક lua_printallstack (lua_State *L) {
  StkId p;
  પૂર્ણાંક i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  માટે (p = L->ઢગલો.p; p < L->ટોચ.p; p++) {
    જો (ci != ખાલી && p == ci->func.p) {
      printf("  ---\n");
      જો (ci == L->ci)
        ci = ખાલી;  /* printed last frame */
      નહીંતર
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  પાછા 0;
}


સ્થિર પૂર્ણાંક get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", અંક_OPCODES);
  પાછા 1;
}


સ્થિર પૂર્ણાંક get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", કદ(lua_State));
  setnameval(L, "global state", કદ(global_State));
  setnameval(L, "TValue", કદ(TValue));
  setnameval(L, "Node", કદ(ગાંઠ_2));
  setnameval(L, "stack Value", કદ(ઢગલો_કિંમત));
  પાછા 1;
}


સ્થિર પૂર્ણાંક mem_query (lua_State *L) {
  જો (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    પાછા 3;
  }
  નહીંતર જો (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    જો (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    પાછા 0;
  }
  નહીંતર {
    અચલ અક્ષર *t = luaL_checkstring(L, 1);
    પૂર્ણાંક i;
    માટે (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      જો (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        પાછા 1;
      }
    }
    પાછા luaL_error(L, "unknown type '%s'", t);
  }
}


સ્થિર પૂર્ણાંક alloc_ગણતરી (lua_State *L) {
  જો (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  નહીંતર
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  પાછા 0;
}


સ્થિર પૂર્ણાંક alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  પાછા 0;
}


સ્થિર પૂર્ણાંક settrick (lua_State *L) {
  જો (ttisnil(obj_at(L, 1)))
    l_Trick = ખાલી;
  નહીંતર
    l_Trick = gcvalue(obj_at(L, 1));
  પાછા 0;
}


સ્થિર પૂર્ણાંક gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  જો (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  નહીંતર {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  જો (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  નહીંતર {
    સ્થિર અચલ અક્ષર *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  જો (!iscollectable(o))
    printf("no collectable\n");
  નહીંતર {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  પાછા 0;
}


સ્થિર અચલ અક્ષર *અચલ statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

સ્થિર પૂર્ણાંક gc_state (lua_State *L) {
  સ્થિર અચલ પૂર્ણાંક states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  પૂર્ણાંક option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  જો (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    પાછા 1;
  }
  નહીંતર {
    જો (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    જો (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    પાછા 0;
  }
}


સ્થિર પૂર્ણાંક tracinggc = 0;
શૂન્ય luai_tracegctest (lua_State *L, પૂર્ણાંક first) {
  જો (!tracinggc) પાછા;
  નહીંતર {
    global_State *g = G(L);
    lua_unlock(L);
    g->gcstp = GCSTPGC;
    lua_checkstack(L, 10);
    lua_getfield(L, LUA_REGISTRYINDEX, "tracegc");
    lua_pushboolean(L, first);
    lua_call(L, 1, 0);
    g->gcstp = 0;
    lua_lock(L);
  }
}


સ્થિર પૂર્ણાંક tracegc (lua_State *L) {
  જો (lua_isnil(L, 1))
    tracinggc = 0;
  નહીંતર {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  પાછા 0;
}


સ્થિર પૂર્ણાંક hash_query (lua_State *L) {
  જો (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    જો (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  નહીંતર {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->ગાંઠ));
  }
  પાછા 1;
}


સ્થિર પૂર્ણાંક stacklevel (lua_State *L) {
  પૂર્ણાંક a = 0;
  lua_pushinteger(L, cast_Integer(L->ટોચ.p - L->ઢગલો.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  પાછા 5;
}


સ્થિર પૂર્ણાંક resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  જ્યારે (ci->next != ખાલી) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  પાછા 0;
}


સ્થિર પૂર્ણાંક reallocstack (lua_State *L) {
  પૂર્ણાંક n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->ટોચ.p - L->ઢગલો.p) + n, 1);
  lua_unlock(L);
  પાછા 0;
}


સ્થિર પૂર્ણાંક table_query (lua_State *L) {
  અચલ Table *t;
  પૂર્ણાંક i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned પૂર્ણાંક asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  જો (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    પાછા 3;
  }
  નહીંતર જો (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    જો (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->ટોચ.p));
    નહીંતર
      setnilvalue2s(L->ટોચ.p);
    api_incr_ટોચ(L);
    lua_pushnil(L);
  }
  નહીંતર જો (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    જો (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    નહીંતર
      lua_pushliteral(L, "<undef>");
    જો (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    નહીંતર
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->ગાંઠ[i]));
  }
  પાછા 3;
}


સ્થિર પૂર્ણાંક gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  પાછા 6;
}


સ્થિર પૂર્ણાંક test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  પાછા 1;
}


સ્થિર પૂર્ણાંક test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  પાછા 1;
}


સ્થિર પૂર્ણાંક string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  પૂર્ણાંક s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  જો (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    પાછા 2;
  }
  નહીંતર જો (s < tb->size) {
    TString *ts;
    પૂર્ણાંક n = 0;
    માટે (ts = tb->hash[s]; ts != ખાલી; ts = ts->u.hnext) {
      setsvalue2s(L, L->ટોચ.p, ts);
      api_incr_ટોચ(L);
      n++;
    }
    પાછા n;
  }
  નહીંતર પાછા 0;
}


સ્થિર પૂર્ણાંક getreftable (lua_State *L) {
  જો (lua_istable(L, 2))  /* is there a table as second argument? */
    પાછા 2;  /* use it as the table */
  નહીંતર
    પાછા LUA_REGISTRYINDEX;  /* default is to use the register */
}


સ્થિર પૂર્ણાંક tref (lua_State *L) {
  પૂર્ણાંક t = getreftable(L);
  પૂર્ણાંક level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  પાછા 1;
}


સ્થિર પૂર્ણાંક getref (lua_State *L) {
  પૂર્ણાંક t = getreftable(L);
  પૂર્ણાંક level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  પાછા 1;
}

સ્થિર પૂર્ણાંક unref (lua_State *L) {
  પૂર્ણાંક t = getreftable(L);
  પૂર્ણાંક level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  પાછા 0;
}


સ્થિર પૂર્ણાંક upvalue (lua_State *L) {
  પૂર્ણાંક n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  જો (lua_isnone(L, 3)) {
    અચલ અક્ષર *name = lua_getupvalue(L, 1, n);
    જો (name == ખાલી) પાછા 0;
    lua_pushstring(L, name);
    પાછા 2;
  }
  નહીંતર {
    અચલ અક્ષર *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    પાછા 1;
  }
}


સ્થિર પૂર્ણાંક newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  પૂર્ણાંક nuv = cast_int(luaL_optinteger(L, 2, 0));
  અક્ષર *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  જ્યારે (size--) *p++ = '\0';
  પાછા 1;
}


સ્થિર પૂર્ણાંક pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  પાછા 1;
}


સ્થિર પૂર્ણાંક udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  પાછા 1;
}


સ્થિર પૂર્ણાંક doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  અચલ અક્ષર *s = luaL_checklstring(L, 1, &l);
  પૂર્ણાંક status = luaL_loadbufferx(L1, s, l, s, "t");
  જો (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  પાછા 1;
}


સ્થિર પૂર્ણાંક s2d (lua_State *L) {
  lua_pushnumber(L, cast_અંક(*cast(અચલ દ્વિગુણ *, luaL_checkstring(L, 1))));
  પાછા 1;
}


સ્થિર પૂર્ણાંક d2s (lua_State *L) {
  દ્વિગુણ d = cast(દ્વિગુણ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), કદ(d));
  પાછા 1;
}


સ્થિર પૂર્ણાંક num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  પાછા 1;
}


સ્થિર પૂર્ણાંક makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  પાછા 1;
}


સ્થિર પૂર્ણાંક newstate (lua_State *L) {
  શૂન્ય *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  જો (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  નહીંતર
    lua_pushnil(L);
  પાછા 1;
}


સ્થિર lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ખાલી, 1, "state expected");
  પાછા L1;
}


સ્થિર પૂર્ણાંક loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  પૂર્ણાંક load = cast_int(luaL_checkinteger(L, 2));
  પૂર્ણાંક preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ખાલી, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  પાછા 0;
}

સ્થિર પૂર્ણાંક closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  પાછા 0;
}

સ્થિર પૂર્ણાંક doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  અચલ અક્ષર *code = luaL_checklstring(L, 2, &lcode);
  પૂર્ણાંક status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  જો (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  જો (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    પાછા 3;
  }
  નહીંતર {
    પૂર્ણાંક i = 0;
    જ્યારે (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_કાઢો(L1, i-1);
    પાછા i-1;
  }
}


સ્થિર પૂર્ણાંક log2_aux (lua_State *L) {
  unsigned પૂર્ણાંક x = (unsigned પૂર્ણાંક)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  પાછા 1;
}


સંરચના Aux { jmp_buf jb; અચલ અક્ષર *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
સ્થિર પૂર્ણાંક panicback (lua_State *L) {
  સંરચના Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (સંરચના Aux *)lua_touserdata(L, -1);
  lua_કાઢો(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  પાછા 1;  /* to avoid warnings */
}

સ્થિર પૂર્ણાંક checkpanic (lua_State *L) {
  સંરચના Aux b;
  શૂન્ય *ud;
  lua_State *L1;
  અચલ અક્ષર *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  જો (L1 == ખાલી) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    પાછા 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  જો (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  નહીંતર {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  પાછા 1;
}


સ્થિર પૂર્ણાંક externKstr (lua_State *L) {
  size_t len;
  અચલ અક્ષર *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ખાલી, ખાલી);
  પાછા 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
સ્થિર પૂર્ણાંક externstr (lua_State *L) {
  size_t len;
  અચલ અક્ષર *s = luaL_checklstring(L, 1, &len);
  શૂન્ય *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  અક્ષર *buff = cast_charp((*allocf)(ud, ખાલી, 0, len + 1));
  જો (buff == ખાલી) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * કદ(અક્ષર));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  પાછા 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


સ્થિર શૂન્ય sethookaux (lua_State *L, પૂર્ણાંક mask, પૂર્ણાંક ગણતરી, અચલ અક્ષર *code);

સ્થિર અચલ અક્ષર *અચલ delimits = " \t\n,;";

સ્થિર શૂન્ય skip (અચલ અક્ષર **pc) {
  માટે (;;) {
    જો (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    નહીંતર જો (**pc == '#') {  /* comment? */
      જ્યારે (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    નહીંતર તોડો;
  }
}

સ્થિર પૂર્ણાંક getnum_aux (lua_State *L, lua_State *L1, અચલ અક્ષર **pc) {
  પૂર્ણાંક res = 0;
  પૂર્ણાંક sig = 1;
  skip(pc);
  જો (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_કાઢો(L1, 1);
    (*pc)++;
    પાછા res;
  }
  નહીંતર જો (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    પાછા res;
  }
  નહીંતર જો (**pc == '!') {
    (*pc)++;
    જો (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    નહીંતર જો (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    નહીંતર lua_assert(0);
    (*pc)++;
    પાછા res;
  }
  નહીંતર જો (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  જો (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  જ્યારે (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  પાછા sig*res;
}

સ્થિર અચલ અક્ષર *getstring_aux (lua_State *L, અક્ષર *buff, અચલ અક્ષર **pc) {
  પૂર્ણાંક i = 0;
  skip(pc);
  જો (**pc == '"' || **pc == '\'') {  /* quoted string? */
    પૂર્ણાંક quote = *(*pc)++;
    જ્યારે (**pc != quote) {
      જો (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  નહીંતર {
    જ્યારે (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  પાછા buff;
}


સ્થિર પૂર્ણાંક getindex_aux (lua_State *L, lua_State *L1, અચલ અક્ષર **pc) {
  skip(pc);
  પસંદગી (*(*pc)++) {
    સ્થિતિ 'R': પાછા LUA_REGISTRYINDEX;
    સ્થિતિ 'U': પાછા lua_upvalueindex(getnum_aux(L, L1, pc));
    મૂળભૂત: {
      પૂર્ણાંક n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      જો (n == 0) પાછા 0;
      નહીંતર પાછા lua_absindex(L1, n);
    }
  }
}


સ્થિર અચલ અક્ષર *અચલ statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
સ્થિર શૂન્ય regcodes (lua_State *L) {
  unsigned પૂર્ણાંક i;
  માટે (i = 0; i < કદ(statcodes) / કદ(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


સ્થિર પૂર્ણાંક testC (lua_State *L);
સ્થિર પૂર્ણાંક Cfunck (lua_State *L, પૂર્ણાંક status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
સ્થિર અચલ અક્ષર ops[] = "+-*%^/\\&|~<>_!";

સ્થિર પૂર્ણાંક runC (lua_State *L, lua_State *L1, અચલ અક્ષર *pc) {
  અક્ષર buff[300];
  પૂર્ણાંક status = 0;
  જો (pc == ખાલી) પાછા luaL_error(L, "attempt to runC null script");
  માટે (;;) {
    અચલ અક્ષર *inst = getstring;
    જો EQ("") પાછા 0;
    નહીંતર જો EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    નહીંતર જો EQ("append") {
      પૂર્ણાંક t = getindex;
      પૂર્ણાંક i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    નહીંતર જો EQ("arith") {
      પૂર્ણાંક op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    નહીંતર જો EQ("call") {
      પૂર્ણાંક narg = getnum;
      પૂર્ણાંક nres = getnum;
      lua_call(L1, narg, nres);
    }
    નહીંતર જો EQ("callk") {
      પૂર્ણાંક narg = getnum;
      પૂર્ણાંક nres = getnum;
      પૂર્ણાંક i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    નહીંતર જો EQ("checkstack") {
      પૂર્ણાંક sz = getnum;
      અચલ અક્ષર *સંદેશ = getstring;
      જો (*સંદેશ == '\0')
        સંદેશ = ખાલી;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, સંદેશ);
    }
    નહીંતર જો EQ("rawcheckstack") {
      પૂર્ણાંક sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    નહીંતર જો EQ("compare") {
      અચલ અક્ષર *opt = getstring;  /* EQ, LT, or LE */
      પૂર્ણાંક op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      પૂર્ણાંક a = getindex;
      પૂર્ણાંક b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    નહીંતર જો EQ("concat") {
      lua_concat(L1, getnum);
    }
    નહીંતર જો EQ("copy") {
      પૂર્ણાંક f = getindex;
      lua_copy(L1, f, getindex);
    }
    નહીંતર જો EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    નહીંતર જો EQ("getfield") {
      પૂર્ણાંક t = getindex;
      પૂર્ણાંક tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    નહીંતર જો EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    નહીંતર જો EQ("getmetatable") {
      જો (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    નહીંતર જો EQ("gettable") {
      પૂર્ણાંક tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    નહીંતર જો EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    નહીંતર જો EQ("gsub") {
      પૂર્ણાંક a = getnum; પૂર્ણાંક b = getnum; પૂર્ણાંક c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    નહીંતર જો EQ("insert") {
      lua_ઉમેરો(L1, getnum);
    }
    નહીંતર જો EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    નહીંતર જો EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    નહીંતર જો EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    નહીંતર જો EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    નહીંતર જો EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    નહીંતર જો EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    નહીંતર જો EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    નહીંતર જો EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    નહીંતર જો EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    નહીંતર જો EQ("len") {
      lua_len(L1, getindex);
    }
    નહીંતર જો EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    નહીંતર જો EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    નહીંતર જો EQ("loadstring") {
      size_t slen;
      અચલ અક્ષર *s = luaL_checklstring(L1, getnum, &slen);
      અચલ અક્ષર *name = getstring;
      અચલ અક્ષર *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    નહીંતર જો EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    નહીંતર જો EQ("newtable") {
      lua_newtable(L1);
    }
    નહીંતર જો EQ("newthread") {
      lua_newthread(L1);
    }
    નહીંતર જો EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    નહીંતર જો EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    નહીંતર જો EQ("next") {
      lua_next(L1, -2);
    }
    નહીંતર જો EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    નહીંતર જો EQ("pcall") {
      પૂર્ણાંક narg = getnum;
      પૂર્ણાંક nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    નહીંતર જો EQ("pcallk") {
      પૂર્ણાંક narg = getnum;
      પૂર્ણાંક nres = getnum;
      પૂર્ણાંક i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    નહીંતર જો EQ("pop") {
      lua_કાઢો(L1, getnum);
    }
    નહીંતર જો EQ("printstack") {
      પૂર્ણાંક n = getnum;
      જો (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      નહીંતર lua_printstack(L1);
    }
    નહીંતર જો EQ("print") {
      અચલ અક્ષર *સંદેશ = getstring;
      printf("%s\n", સંદેશ);
    }
    નહીંતર જો EQ("warningC") {
      અચલ અક્ષર *સંદેશ = getstring;
      lua_warning(L1, સંદેશ, 1);
    }
    નહીંતર જો EQ("warning") {
      અચલ અક્ષર *સંદેશ = getstring;
      lua_warning(L1, સંદેશ, 0);
    }
    નહીંતર જો EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    નહીંતર જો EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    નહીંતર જો EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    નહીંતર જો EQ("pushnil") {
      lua_pushnil(L1);
    }
    નહીંતર જો EQ("pushnum") {
      lua_pushnumber(L1, (lua_સંખ્યા)getnum);
    }
    નહીંતર જો EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    નહીંતર જો EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    નહીંતર જો EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    નહીંતર જો EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    નહીંતર જો EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (પૂર્ણાંક)lua_tointeger(L, -1));
    }
    નહીંતર જો EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    નહીંતર જો EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    નહીંતર જો EQ("rawget") {
      પૂર્ણાંક t = getindex;
      lua_rawget(L1, t);
    }
    નહીંતર જો EQ("rawgeti") {
      પૂર્ણાંક t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    નહીંતર જો EQ("rawgetp") {
      પૂર્ણાંક t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    નહીંતર જો EQ("rawset") {
      પૂર્ણાંક t = getindex;
      lua_rawset(L1, t);
    }
    નહીંતર જો EQ("rawseti") {
      પૂર્ણાંક t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    નહીંતર જો EQ("rawsetp") {
      પૂર્ણાંક t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    નહીંતર જો EQ("remove") {
      lua_remove(L1, getnum);
    }
    નહીંતર જો EQ("replace") {
      lua_replace(L1, getindex);
    }
    નહીંતર જો EQ("resume") {
      પૂર્ણાંક i = getindex;
      પૂર્ણાંક nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    નહીંતર જો EQ("traceback") {
      અચલ અક્ષર *સંદેશ = getstring;
      પૂર્ણાંક level = getnum;
      luaL_traceback(L1, L1, સંદેશ, level);
    }
    નહીંતર જો EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    નહીંતર જો EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    નહીંતર જો EQ("return") {
      પૂર્ણાંક n = getnum;
      જો (L1 != L) {
        પૂર્ણાંક i;
        માટે (i = 0; i < n; i++) {
          પૂર્ણાંક idx = -(n - i);
          પસંદગી (lua_type(L1, idx)) {
            સ્થિતિ LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              તોડો;
            મૂળભૂત:
              lua_pushstring(L, lua_tostring(L1, idx));
              તોડો;
          }
        }
      }
      પાછા n;
    }
    નહીંતર જો EQ("rotate") {
      પૂર્ણાંક i = getindex;
      lua_rotate(L1, i, getnum);
    }
    નહીંતર જો EQ("setfield") {
      પૂર્ણાંક t = getindex;
      અચલ અક્ષર *s = getstring;
      lua_setfield(L1, t, s);
    }
    નહીંતર જો EQ("seti") {
      પૂર્ણાંક t = getindex;
      lua_seti(L1, t, getnum);
    }
    નહીંતર જો EQ("setglobal") {
      અચલ અક્ષર *s = getstring;
      lua_setglobal(L1, s);
    }
    નહીંતર જો EQ("sethook") {
      પૂર્ણાંક mask = getnum;
      પૂર્ણાંક ગણતરી = getnum;
      અચલ અક્ષર *s = getstring;
      sethookaux(L1, mask, ગણતરી, s);
    }
    નહીંતર જો EQ("setmetatable") {
      પૂર્ણાંક idx = getindex;
      lua_setmetatable(L1, idx);
    }
    નહીંતર જો EQ("settable") {
      lua_settable(L1, getindex);
    }
    નહીંતર જો EQ("settop") {
      lua_settop(L1, getnum);
    }
    નહીંતર જો EQ("testudata") {
      પૂર્ણાંક i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ખાલી);
    }
    નહીંતર જો EQ("error") {
      lua_error(L1);
    }
    નહીંતર જો EQ("abort") {
      abort();
    }
    નહીંતર જો EQ("throw") {
#જો defined(__cplusplus)
સ્થિર સંરચના X { પૂર્ણાંક x; } x;
      throw x;
#નહીંતર
      luaL_error(L1, "C++");
#endif
      તોડો;
    }
    નહીંતર જો EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    નહીંતર જો EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    નહીંતર જો EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    નહીંતર જો EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    નહીંતર જો EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    નહીંતર જો EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    નહીંતર જો EQ("tostring") {
      અચલ અક્ષર *s = lua_tostring(L1, getindex);
      અચલ અક્ષર *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ખાલી && s1 == ખાલી) || strcmp(s, s1) == 0);
    }
    નહીંતર જો EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ખાલી);
    }
    નહીંતર જો EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    નહીંતર જો EQ("xmove") {
      પૂર્ણાંક f = getindex;
      પૂર્ણાંક t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      પૂર્ણાંક n = getnum;
      જો (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    નહીંતર જો EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    નહીંતર જો EQ("yield") {
      પાછા lua_yield(L1, getnum);
    }
    નહીંતર જો EQ("yieldk") {
      પૂર્ણાંક nres = getnum;
      પૂર્ણાંક i = getindex;
      પાછા lua_yieldk(L1, nres, i, Cfunck);
    }
    નહીંતર જો EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    નહીંતર જો EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    નહીંતર જો EQ("argerror") {
      પૂર્ણાંક arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    નહીંતર luaL_error(L, "unknown instruction %s", buff);
  }
  પાછા 0;
}


સ્થિર પૂર્ણાંક testC (lua_State *L) {
  lua_State *L1;
  અચલ અક્ષર *pc;
  જો (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  નહીંતર જો (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  નહીંતર {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  પાછા runC(L, L1, pc);
}


સ્થિર પૂર્ણાંક Cfunc (lua_State *L) {
  પાછા runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


સ્થિર પૂર્ણાંક Cfunck (lua_State *L, પૂર્ણાંક status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  પાછા runC(L, L, lua_tostring(L, cast_int(ctx)));
}


સ્થિર પૂર્ણાંક makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  પાછા 1;
}


/* }====================================================== */


/*
** {======================================================
** tests for C hooks
** =======================================================
*/

/*
** C hook that runs the C script stored in registry.C_HOOK[L]
*/
સ્થિર શૂન્ય Chook (lua_State *L, lua_Debug *ar) {
  અચલ અક્ષર *scpt;
  અચલ અક્ષર *અચલ events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_કાઢો(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
સ્થિર શૂન્ય sethookaux (lua_State *L, પૂર્ણાંક mask, પૂર્ણાંક ગણતરી, અચલ અક્ષર *scpt) {
  જો (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ખાલી, 0, 0);  /* turn off hooks */
    પાછા;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  જો (!lua_istable(L, -1)) {  /* no hook table? */
    lua_કાઢો(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, ગણતરી);
}


સ્થિર પૂર્ણાંક sethook (lua_State *L) {
  જો (lua_isnoneornil(L, 1))
    lua_sethook(L, ખાલી, 0, 0);  /* turn off hooks */
  નહીંતર {
    અચલ અક્ષર *scpt = luaL_checkstring(L, 1);
    અચલ અક્ષર *smask = luaL_checkstring(L, 2);
    પૂર્ણાંક ગણતરી = cast_int(luaL_optinteger(L, 3, 0));
    પૂર્ણાંક mask = 0;
    જો (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    જો (strchr(smask, 'r')) mask |= LUA_MASKRET;
    જો (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    જો (ગણતરી > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, ગણતરી, scpt);
  }
  પાછા 0;
}


સ્થિર પૂર્ણાંક coresume (lua_State *L) {
  પૂર્ણાંક status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  જો (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_ઉમેરો(L, -2);
    પાછા 2;  /* return false + error message */
  }
  નહીંતર {
    lua_pushboolean(L, 1);
    પાછા 1;
  }
}

#જો !defined(LUA_USE_POSIX)

#define nonblock	ખાલી

#નહીંતર

#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <fcntl.h>

સ્થિર પૂર્ણાંક nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  પૂર્ણાંક fd = fileno(f);
  પૂર્ણાંક flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  પાછા 0;
}
#endif

/* }====================================================== */



સ્થિર અચલ સંરચના luaL_Reg tests_funcs[] = {
  {"checkmemory", lua_checkmemory},
  {"closestate", closestate},
  {"d2s", d2s},
  {"doonnewstack", doonnewstack},
  {"doremote", doremote},
  {"gccolor", gc_color},
  {"gcage", gc_age},
  {"gcstate", gc_state},
  {"tracegc", tracegc},
  {"pobj", gc_printobj},
  {"getref", getref},
  {"hash", hash_query},
  {"log2", log2_aux},
  {"limits", get_limits},
  {"listcode", listcode},
  {"printcode", printcode},
  {"printallstack", lua_printallstack},
  {"listk", listk},
  {"listabslineinfo", listabslineinfo},
  {"listlocals", listlocals},
  {"loadlib", loadlib},
  {"checkpanic", checkpanic},
  {"newstate", newstate},
  {"newuserdata", newuserdata},
  {"num2int", num2int},
  {"makeseed", makeseed},
  {"pushuserdata", pushuserdata},
  {"gcquery", gc_query},
  {"querystr", string_query},
  {"querytab", table_query},
  {"codeparam", test_codeparam},
  {"applyparam", test_applyparam},
  {"ref", tref},
  {"resume", coresume},
  {"s2d", s2d},
  {"sethook", sethook},
  {"stacklevel", stacklevel},
  {"resetCI", resetCI},
  {"reallocstack", reallocstack},
  {"sizes", get_sizes},
  {"testC", testC},
  {"makeCfunc", makeCfunc},
  {"totalmem", mem_query},
  {"alloccount", alloc_ગણતરી},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {ખાલી, ખાલી}
};


સ્થિર શૂન્ય checkfinalmem (શૂન્ય) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


પૂર્ણાંક luaB_opentests (lua_State *L) {
  શૂન્ય *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_atpanic(L, &tpanic);
  lua_setwarnf(L, &warnf, L);
  lua_pushboolean(L, 0);
  lua_setglobal(L, "_WARN");  /* _WARN = false */
  regcodes(L);
  atexit(checkfinalmem);
  lua_assert(f == debug_realloc && ud == cast_voidp(&l_memcontrol));
  lua_setallocf(L, f, ud);  /* exercise this function */
  luaL_newlib(L, tests_funcs);
  પાછા 1;
}

#endif

