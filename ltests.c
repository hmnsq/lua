/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#include "lprefix.h"


#include <limits.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "lauxlib.h"
#include "lcode.h"
#include "lctype.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lopcodes.h"
#include "lopnames.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#ပေႃး defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static puur_naa_nka runC (lua_State *L, lua_State *L1, const var_na *pc);


static void setnameval (lua_State *L, const var_na *name, puur_naa_nka val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const var_na *fmt, const var_na *s1, const var_na *s2) {
  fprintf(stderr, fmt, s1);
  ပေႃး (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static puur_naa_nka tpanic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ပွၵ်ႈ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   msg, NULL),
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
static void warnf (void *ud, const var_na *msg, puur_naa_nka tocont) {
  lua_State *L = cast(lua_State *, ud);
  static var_na buff[200] = "";  /* should be enough for tests... */
  static puur_naa_nka onoff = 0;
  static puur_naa_nka mode = 0;  /* start in normal mode */
  static puur_naa_nka lasttocont = 0;
  ပေႃး (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    ပေႃး (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    ပေႃး (strcmp(msg, "@off") == 0)
      onoff = 0;
    ပေႃးဢမ်ႇ ပေႃး (strcmp(msg, "@on") == 0)
      onoff = 1;
    ပေႃးဢမ်ႇ ပေႃး (strcmp(msg, "@normal") == 0)
      mode = 0;
    ပေႃးဢမ်ႇ ပေႃး (strcmp(msg, "@allow") == 0)
      mode = 1;
    ပေႃးဢမ်ႇ ပေႃး (strcmp(msg, "@store") == 0)
      mode = 2;
    ပေႃးဢမ်ႇ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    ပွၵ်ႈ;
  }
  lasttocont = tocont;
  ပေႃး (strlen(msg) >= parimaa_na(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  ပေႃး (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ပေႃး (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    ပေႃးဢမ်ႇ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      pak_sa 0: {  /* normal */
        ပေႃး (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      pak_sa 1: {  /* allow */
        ပေႃး (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        bha_nga;
      }
      pak_sa 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        bha_nga;
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

typedef sa_ngha memHeader {
  LUAI_MAXALIGN;
  sa_nracanaa {
    size_t size;
    puur_naa_nka type;
  } d;
} memHeader;


#ပေႃး !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#ပေႃးဢမ်ႇ

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  ပေႃး (block) {
    size_t size = block->d.size;
    puur_naa_nka i;
    for (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, parimaa_na(memHeader) + size + MARKSIZE);  /* erase block */
    free(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


void *debug_realloc (void *ud, void *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  puur_naa_nka type;
  ပေႃး (mc->memlimit == 0) {  /* first time? */
    var_na *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  ပေႃး (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  ပေႃးဢမ်ႇ {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  ပေႃး (size == 0) {
    freeblock(mc, block);
    ပွၵ်ႈ NULL;
  }
  ပေႃး (mc->failnext) {
    mc->failnext = 0;
    ပွၵ်ႈ NULL;  /* fake a single memory allocation error */
  }
  ပေႃး (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    ပေႃး (mc->countlimit == 0)
      ပွၵ်ႈ NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ပေႃး (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ပွၵ်ႈ NULL;  /* fake a memory allocation error */
  ပေႃးဢမ်ႇ {
    memHeader *newblock;
    puur_naa_nka i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = parimaa_na(memHeader) + size + MARKSIZE;
    ပေႃး (realsize < size) ပွၵ်ႈ NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    ပေႃး (newblock == NULL)
      ပွၵ်ႈ NULL;  /* really out of memory? */
    ပေႃး (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    for (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    ပေႃး (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ပွၵ်ႈ newblock + 1;
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

#include <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
static puur_naa_nka testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ပေႃး (isdead(g,t)) ပွၵ်ႈ 0;
  ပေႃး (issweepphase(g))
    ပွၵ်ႈ 1;  /* no invariants */
  ပေႃးဢမ်ႇ ပေႃး (g->gckind != KGC_GENMINOR)
    ပွၵ်ႈ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  ပေႃးဢမ်ႇ {  /* generational mode */
    ပေႃး ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ပွၵ်ႈ 0;
    ပေႃး ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ပွၵ်ႈ 0;
    ပွၵ်ႈ 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ပေႃး (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


void lua_printobj (lua_State *L, sa_nracanaa GCObject *o) {
  printobj(G(L), o);
}


void lua_printvalue (TValue *v) {
  switch (ttypetag(v)) {
    pak_sa LUA_VNUMINT: pak_sa LUA_VNUMFLT: {
      var_na buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      bha_nga;
    }
    pak_sa LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); bha_nga;
    pak_sa LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); bha_nga;
    pak_sa LUA_VFALSE:
      printf("%s", "false"); bha_nga;
    pak_sa LUA_VTRUE:
      printf("%s", "true"); bha_nga;
    pak_sa LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); bha_nga;
    pak_sa LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); bha_nga;
    pak_sa LUA_VNIL:
      printf("nil"); bha_nga;
    pak_sa LUA_VLCF:
      printf("light C function: %p", fvalue(v)); bha_nga;
    pak_sa LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); bha_nga;
    pak_sa LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); bha_nga;
    pak_sa LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); bha_nga;
    pak_sa LUA_VTABLE:
      printf("table: %p", hvalue(v)); bha_nga;
    default:
      lua_assert(0);
  }
}


static puur_naa_nka testobjref (global_State *g, GCObject *f, GCObject *t) {
  puur_naa_nka r1 = testobjref1(g, f, t);
  ပေႃး (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ပွၵ်ႈ r1;
}


static void checkobjref (global_State *g, GCObject *f, GCObject *t) {
    assert(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ ပေႃး (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned puur_naa_nka i;
  unsigned puur_naa_nka asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  for (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  for (n = gnode(h, 0); n < limit; n++) {
    ပေႃး (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  puur_naa_nka i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  for (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  puur_naa_nka i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  for (i=0; i<f->sizek; i++) {
    ပေႃး (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  for (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  for (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  for (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  puur_naa_nka i;
  for (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  puur_naa_nka i;
  checkobjrefN(g, clgc, cl->p);
  for (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    ပေႃး (uv) {
      checkobjrefN(g, clgc, uv);
      ပေႃး (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static puur_naa_nka lua_checkpc (CallInfo *ci) {
  ပေႃး (!isLua(ci)) ပွၵ်ႈ 1;
  ပေႃးဢမ်ႇ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ပွၵ်ႈ p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  ပေႃး (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    ပွၵ်ႈ;
  }
  for (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  for (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  for (o = L1->stack.p; o < L1->stack_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


static void checkrefs (global_State *g, GCObject *o) {
  switch (o->tt) {
    pak_sa LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      bha_nga;
    }
    pak_sa LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      bha_nga;
    }
    pak_sa LUA_VTABLE: {
      checktable(g, gco2t(o));
      bha_nga;
    }
    pak_sa LUA_VTHREAD: {
      check_stack(g, gco2th(o));
      bha_nga;
    }
    pak_sa LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      bha_nga;
    }
    pak_sa LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      bha_nga;
    }
    pak_sa LUA_VPROTO: {
      checkproto(g, gco2p(o));
      bha_nga;
    }
    pak_sa LUA_VSHRSTR:
    pak_sa LUA_VLNGSTR: {
      assert(!isgray(o));  /* strings are never gray */
      bha_nga;
    }
    default: assert(0);
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
static void checkobject (global_State *g, GCObject *o, puur_naa_nka maybedead,
                         puur_naa_nka listage) {
  ပေႃး (isdead(g, o))
    assert(maybedead);
  ပေႃးဢမ်ႇ {
    assert(g->gcstate != GCSpause || iswhite(o));
    ပေႃး (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      ပေႃး (isold(o)) {
        assert(!iswhite(o));
        assert(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      assert(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


static l_mem checkgraylist (global_State *g, GCObject *o) {
  puur_naa_nka total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ၶၢဝ်း (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    ပေႃး (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    switch (o->tt) {
      pak_sa LUA_VTABLE: o = gco2t(o)->gclist; bha_nga;
      pak_sa LUA_VLCL: o = gco2lcl(o)->gclist; bha_nga;
      pak_sa LUA_VCCL: o = gco2ccl(o)->gclist; bha_nga;
      pak_sa LUA_VTHREAD: o = gco2th(o)->gclist; bha_nga;
      pak_sa LUA_VPROTO: o = gco2p(o)->gclist; bha_nga;
      pak_sa LUA_VUSERDATA:
        assert(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        bha_nga;
      default: assert(0);  /* other objects cannot be in a gray list */
    }
  }
  ပွၵ်ႈ total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  ပေႃး (!keepinvariant(g)) ပွၵ်ႈ total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ပွၵ်ႈ total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  ပေႃး (!keepinvariant(g))
    ပွၵ်ႈ;  /* gray lists not being kept in these phases */
  ပေႃး (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    ပွၵ်ႈ;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ပေႃး (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, puur_naa_nka maybedead, puur_naa_nka tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  for (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  for (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  for (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  for (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ပွၵ်ႈ total;
}


puur_naa_nka lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  puur_naa_nka maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ပေႃး (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  for (o = g->fixedgc; o != NULL; o = o->next) {
    assert(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  for (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ပေႃး (keepinvariant(g))
    assert(totalin == totalshould);
  ပွၵ်ႈ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static var_na *buildop (Proto *p, puur_naa_nka pc, var_na *buff) {
  var_na *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const var_na *name = opnames[o];
  puur_naa_nka line = luaG_getfuncline(p, pc);
  puur_naa_nka lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  ပေႃး (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  ပေႃးဢမ်ႇ
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", line, pc);
  switch (getOpMode(o)) {
    pak_sa iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      bha_nga;
    pak_sa ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      bha_nga;
    pak_sa iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      bha_nga;
    pak_sa iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      bha_nga;
    pak_sa iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      bha_nga;
    pak_sa isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      bha_nga;
  }
  ပွၵ်ႈ obuff;
}


#ပေႃး 0
void luaI_printcode (Proto *pt, puur_naa_nka size) {
  puur_naa_nka pc;
  for (pc=0; pc<size; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, puur_naa_nka pc) {
  var_na buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static puur_naa_nka listcode (lua_State *L) {
  puur_naa_nka pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  for (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ပွၵ်ႈ 1;
}


static puur_naa_nka printcode (lua_State *L) {
  puur_naa_nka pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  for (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ပွၵ်ႈ 0;
}


static puur_naa_nka listk (lua_State *L) {
  Proto *p;
  puur_naa_nka i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  for (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ပွၵ်ႈ 1;
}


static puur_naa_nka listabslineinfo (lua_State *L) {
  Proto *p;
  puur_naa_nka i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  for (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ပွၵ်ႈ 1;
}


static puur_naa_nka listlocals (lua_State *L) {
  Proto *p;
  puur_naa_nka pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  puur_naa_nka i = 0;
  const var_na *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ၶၢဝ်း ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  ပွၵ်ႈ i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  puur_naa_nka i;
  puur_naa_nka n = lua_gettop(L);
  printf("stack: >>\n");
  for (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


puur_naa_nka lua_printallstack (lua_State *L) {
  StkId p;
  puur_naa_nka i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  for (p = L->stack.p; p < L->top.p; p++) {
    ပေႃး (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      ပေႃး (ci == L->ci)
        ci = NULL;  /* printed last frame */
      ပေႃးဢမ်ႇ
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ပွၵ်ႈ 0;
}


static puur_naa_nka get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  ပွၵ်ႈ 1;
}


static puur_naa_nka get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", parimaa_na(lua_State));
  setnameval(L, "global state", parimaa_na(global_State));
  setnameval(L, "TValue", parimaa_na(TValue));
  setnameval(L, "Node", parimaa_na(Node));
  setnameval(L, "stack Value", parimaa_na(StackValue));
  ပွၵ်ႈ 1;
}


static puur_naa_nka mem_query (lua_State *L) {
  ပေႃး (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ပွၵ်ႈ 3;
  }
  ပေႃးဢမ်ႇ ပေႃး (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    ပေႃး (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ပွၵ်ႈ 0;
  }
  ပေႃးဢမ်ႇ {
    const var_na *t = luaL_checkstring(L, 1);
    puur_naa_nka i;
    for (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ပေႃး (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ပွၵ်ႈ 1;
      }
    }
    ပွၵ်ႈ luaL_error(L, "unknown type '%s'", t);
  }
}


static puur_naa_nka alloc_count (lua_State *L) {
  ပေႃး (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  ပေႃးဢမ်ႇ
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ပွၵ်ႈ 0;
}


static puur_naa_nka alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ပွၵ်ႈ 0;
}


static puur_naa_nka settrick (lua_State *L) {
  ပေႃး (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  ပေႃးဢမ်ႇ
    l_Trick = gcvalue(obj_at(L, 1));
  ပွၵ်ႈ 0;
}


static puur_naa_nka gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ပေႃး (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ပေႃးဢမ်ႇ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ပွၵ်ႈ 1;
}


static puur_naa_nka gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ပေႃး (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ပေႃးဢမ်ႇ {
    static const var_na *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ပွၵ်ႈ 1;
}


static puur_naa_nka gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ပေႃး (!iscollectable(o))
    printf("no collectable\n");
  ပေႃးဢမ်ႇ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ပွၵ်ႈ 0;
}


static const var_na *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static puur_naa_nka gc_state (lua_State *L) {
  static const puur_naa_nka states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  puur_naa_nka option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ပေႃး (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ပွၵ်ႈ 1;
  }
  ပေႃးဢမ်ႇ {
    ပေႃး (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    ပေႃး (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ပွၵ်ႈ 0;
  }
}


static puur_naa_nka tracinggc = 0;
void luai_tracegctest (lua_State *L, puur_naa_nka first) {
  ပေႃး (!tracinggc) ပွၵ်ႈ;
  ပေႃးဢမ်ႇ {
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


static puur_naa_nka tracegc (lua_State *L) {
  ပေႃး (lua_isnil(L, 1))
    tracinggc = 0;
  ပေႃးဢမ်ႇ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ပွၵ်ႈ 0;
}


static puur_naa_nka hash_query (lua_State *L) {
  ပေႃး (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ပေႃး (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  ပေႃးဢမ်ႇ {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  ပွၵ်ႈ 1;
}


static puur_naa_nka stacklevel (lua_State *L) {
  puur_naa_nka a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ပွၵ်ႈ 5;
}


static puur_naa_nka resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ၶၢဝ်း (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ပွၵ်ႈ 0;
}


static puur_naa_nka reallocstack (lua_State *L) {
  puur_naa_nka n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  ပွၵ်ႈ 0;
}


static puur_naa_nka table_query (lua_State *L) {
  const Table *t;
  puur_naa_nka i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned puur_naa_nka asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ပေႃး (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ပွၵ်ႈ 3;
  }
  ပေႃးဢမ်ႇ ပေႃး (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ပေႃး (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    ပေႃးဢမ်ႇ
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  ပေႃးဢမ်ႇ ပေႃး (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ပေႃး (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    ပေႃးဢမ်ႇ
      lua_pushliteral(L, "<undef>");
    ပေႃး (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    ပေႃးဢမ်ႇ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  ပွၵ်ႈ 3;
}


static puur_naa_nka gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ပွၵ်ႈ 6;
}


static puur_naa_nka test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ပွၵ်ႈ 1;
}


static puur_naa_nka test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ပွၵ်ႈ 1;
}


static puur_naa_nka string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  puur_naa_nka s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ပေႃး (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ပွၵ်ႈ 2;
  }
  ပေႃးဢမ်ႇ ပေႃး (s < tb->size) {
    TString *ts;
    puur_naa_nka n = 0;
    for (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    ပွၵ်ႈ n;
  }
  ပေႃးဢမ်ႇ ပွၵ်ႈ 0;
}


static puur_naa_nka getreftable (lua_State *L) {
  ပေႃး (lua_istable(L, 2))  /* is there a table as second argument? */
    ပွၵ်ႈ 2;  /* use it as the table */
  ပေႃးဢမ်ႇ
    ပွၵ်ႈ LUA_REGISTRYINDEX;  /* default is to use the register */
}


static puur_naa_nka tref (lua_State *L) {
  puur_naa_nka t = getreftable(L);
  puur_naa_nka level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ပွၵ်ႈ 1;
}


static puur_naa_nka getref (lua_State *L) {
  puur_naa_nka t = getreftable(L);
  puur_naa_nka level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ပွၵ်ႈ 1;
}

static puur_naa_nka unref (lua_State *L) {
  puur_naa_nka t = getreftable(L);
  puur_naa_nka level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ပွၵ်ႈ 0;
}


static puur_naa_nka upvalue (lua_State *L) {
  puur_naa_nka n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ပေႃး (lua_isnone(L, 3)) {
    const var_na *name = lua_getupvalue(L, 1, n);
    ပေႃး (name == NULL) ပွၵ်ႈ 0;
    lua_pushstring(L, name);
    ပွၵ်ႈ 2;
  }
  ပေႃးဢမ်ႇ {
    const var_na *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ပွၵ်ႈ 1;
  }
}


static puur_naa_nka newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  puur_naa_nka nuv = cast_int(luaL_optinteger(L, 2, 0));
  var_na *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  ၶၢဝ်း (size--) *p++ = '\0';
  ပွၵ်ႈ 1;
}


static puur_naa_nka pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ပွၵ်ႈ 1;
}


static puur_naa_nka udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ပွၵ်ႈ 1;
}


static puur_naa_nka doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  puur_naa_nka status = luaL_loadbufferx(L1, s, l, s, "t");
  ပေႃး (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ပွၵ်ႈ 1;
}


static puur_naa_nka s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const dviguu_na *, luaL_checkstring(L, 1))));
  ပွၵ်ႈ 1;
}


static puur_naa_nka d2s (lua_State *L) {
  dviguu_na d = cast(dviguu_na, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), parimaa_na(d));
  ပွၵ်ႈ 1;
}


static puur_naa_nka num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ပွၵ်ႈ 1;
}


static puur_naa_nka makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ပွၵ်ႈ 1;
}


static puur_naa_nka newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ပေႃး (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  ပေႃးဢမ်ႇ
    lua_pushnil(L);
  ပွၵ်ႈ 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  ပွၵ်ႈ L1;
}


static puur_naa_nka loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  puur_naa_nka load = cast_int(luaL_checkinteger(L, 2));
  puur_naa_nka preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ပွၵ်ႈ 0;
}

static puur_naa_nka closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ပွၵ်ႈ 0;
}

static puur_naa_nka doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const var_na *code = luaL_checklstring(L, 2, &lcode);
  puur_naa_nka status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  ပေႃး (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ပေႃး (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ပွၵ်ႈ 3;
  }
  ပေႃးဢမ်ႇ {
    puur_naa_nka i = 0;
    ၶၢဝ်း (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    ပွၵ်ႈ i-1;
  }
}


static puur_naa_nka log2_aux (lua_State *L) {
  unsigned puur_naa_nka x = (unsigned puur_naa_nka)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ပွၵ်ႈ 1;
}


sa_nracanaa Aux { jmp_buf jb; const var_na *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static puur_naa_nka panicback (lua_State *L) {
  sa_nracanaa Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (sa_nracanaa Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ပွၵ်ႈ 1;  /* to avoid warnings */
}

static puur_naa_nka checkpanic (lua_State *L) {
  sa_nracanaa Aux b;
  void *ud;
  lua_State *L1;
  const var_na *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ပေႃး (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ပွၵ်ႈ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ပေႃး (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  ပေႃးဢမ်ႇ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ပွၵ်ႈ 1;
}


static puur_naa_nka externKstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  ပွၵ်ႈ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static puur_naa_nka externstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  var_na *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  ပေႃး (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * parimaa_na(var_na));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ပွၵ်ႈ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, puur_naa_nka mask, puur_naa_nka count, const var_na *code);

static const var_na *const delimits = " \t\n,;";

static void skip (const var_na **pc) {
  for (;;) {
    ပေႃး (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    ပေႃးဢမ်ႇ ပေႃး (**pc == '#') {  /* comment? */
      ၶၢဝ်း (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    ပေႃးဢမ်ႇ bha_nga;
  }
}

static puur_naa_nka getnum_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  puur_naa_nka res = 0;
  puur_naa_nka sig = 1;
  skip(pc);
  ပေႃး (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    ပွၵ်ႈ res;
  }
  ပေႃးဢမ်ႇ ပေႃး (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ပွၵ်ႈ res;
  }
  ပေႃးဢမ်ႇ ပေႃး (**pc == '!') {
    (*pc)++;
    ပေႃး (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    ပေႃးဢမ်ႇ ပေႃး (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    ပေႃးဢမ်ႇ lua_assert(0);
    (*pc)++;
    ပွၵ်ႈ res;
  }
  ပေႃးဢမ်ႇ ပေႃး (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ပေႃး (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ၶၢဝ်း (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ပွၵ်ႈ sig*res;
}

static const var_na *getstring_aux (lua_State *L, var_na *buff, const var_na **pc) {
  puur_naa_nka i = 0;
  skip(pc);
  ပေႃး (**pc == '"' || **pc == '\'') {  /* quoted string? */
    puur_naa_nka quote = *(*pc)++;
    ၶၢဝ်း (**pc != quote) {
      ပေႃး (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  ပေႃးဢမ်ႇ {
    ၶၢဝ်း (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ပွၵ်ႈ buff;
}


static puur_naa_nka getindex_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  skip(pc);
  switch (*(*pc)++) {
    pak_sa 'R': ပွၵ်ႈ LUA_REGISTRYINDEX;
    pak_sa 'U': ပွၵ်ႈ lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      puur_naa_nka n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ပေႃး (n == 0) ပွၵ်ႈ 0;
      ပေႃးဢမ်ႇ ပွၵ်ႈ lua_absindex(L1, n);
    }
  }
}


static const var_na *const statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
static void regcodes (lua_State *L) {
  unsigned puur_naa_nka i;
  for (i = 0; i < parimaa_na(statcodes) / parimaa_na(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static puur_naa_nka testC (lua_State *L);
static puur_naa_nka Cfunck (lua_State *L, puur_naa_nka status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const var_na ops[] = "+-*%^/\\&|~<>_!";

static puur_naa_nka runC (lua_State *L, lua_State *L1, const var_na *pc) {
  var_na buff[300];
  puur_naa_nka status = 0;
  ပေႃး (pc == NULL) ပွၵ်ႈ luaL_error(L, "attempt to runC null script");
  for (;;) {
    const var_na *inst = getstring;
    ပေႃး EQ("") ပွၵ်ႈ 0;
    ပေႃးဢမ်ႇ ပေႃး EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("append") {
      puur_naa_nka t = getindex;
      puur_naa_nka i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("arith") {
      puur_naa_nka op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("call") {
      puur_naa_nka narg = getnum;
      puur_naa_nka nres = getnum;
      lua_call(L1, narg, nres);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("callk") {
      puur_naa_nka narg = getnum;
      puur_naa_nka nres = getnum;
      puur_naa_nka i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("checkstack") {
      puur_naa_nka sz = getnum;
      const var_na *msg = getstring;
      ပေႃး (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rawcheckstack") {
      puur_naa_nka sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("compare") {
      const var_na *opt = getstring;  /* EQ, LT, or LE */
      puur_naa_nka op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      puur_naa_nka a = getindex;
      puur_naa_nka b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("concat") {
      lua_concat(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("copy") {
      puur_naa_nka f = getindex;
      lua_copy(L1, f, getindex);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("getfield") {
      puur_naa_nka t = getindex;
      puur_naa_nka tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("getmetatable") {
      ပေႃး (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("gettable") {
      puur_naa_nka tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("gsub") {
      puur_naa_nka a = getnum; puur_naa_nka b = getnum; puur_naa_nka c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("insert") {
      lua_insert(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("len") {
      lua_len(L1, getindex);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("loadstring") {
      size_t slen;
      const var_na *s = luaL_checklstring(L1, getnum, &slen);
      const var_na *name = getstring;
      const var_na *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("newtable") {
      lua_newtable(L1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("newthread") {
      lua_newthread(L1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("next") {
      lua_next(L1, -2);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pcall") {
      puur_naa_nka narg = getnum;
      puur_naa_nka nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pcallk") {
      puur_naa_nka narg = getnum;
      puur_naa_nka nres = getnum;
      puur_naa_nka i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pop") {
      lua_pop(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("printstack") {
      puur_naa_nka n = getnum;
      ပေႃး (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      ပေႃးဢမ်ႇ lua_printstack(L1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("print") {
      const var_na *msg = getstring;
      printf("%s\n", msg);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("warningC") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("warning") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushnil") {
      lua_pushnil(L1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (puur_naa_nka)lua_tointeger(L, -1));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rawget") {
      puur_naa_nka t = getindex;
      lua_rawget(L1, t);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rawgeti") {
      puur_naa_nka t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rawgetp") {
      puur_naa_nka t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rawset") {
      puur_naa_nka t = getindex;
      lua_rawset(L1, t);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rawseti") {
      puur_naa_nka t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rawsetp") {
      puur_naa_nka t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("remove") {
      lua_remove(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("replace") {
      lua_replace(L1, getindex);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("resume") {
      puur_naa_nka i = getindex;
      puur_naa_nka nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("traceback") {
      const var_na *msg = getstring;
      puur_naa_nka level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("return") {
      puur_naa_nka n = getnum;
      ပေႃး (L1 != L) {
        puur_naa_nka i;
        for (i = 0; i < n; i++) {
          puur_naa_nka idx = -(n - i);
          switch (lua_type(L1, idx)) {
            pak_sa LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              bha_nga;
            default:
              lua_pushstring(L, lua_tostring(L1, idx));
              bha_nga;
          }
        }
      }
      ပွၵ်ႈ n;
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("rotate") {
      puur_naa_nka i = getindex;
      lua_rotate(L1, i, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("setfield") {
      puur_naa_nka t = getindex;
      const var_na *s = getstring;
      lua_setfield(L1, t, s);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("seti") {
      puur_naa_nka t = getindex;
      lua_seti(L1, t, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("setglobal") {
      const var_na *s = getstring;
      lua_setglobal(L1, s);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("sethook") {
      puur_naa_nka mask = getnum;
      puur_naa_nka count = getnum;
      const var_na *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("setmetatable") {
      puur_naa_nka idx = getindex;
      lua_setmetatable(L1, idx);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("settable") {
      lua_settable(L1, getindex);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("settop") {
      lua_settop(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("testudata") {
      puur_naa_nka i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("error") {
      lua_error(L1);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("abort") {
      abort();
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("throw") {
#ပေႃး defined(__cplusplus)
static sa_nracanaa X { puur_naa_nka x; } x;
      throw x;
#ပေႃးဢမ်ႇ
      luaL_error(L1, "C++");
#endif
      bha_nga;
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("tostring") {
      const var_na *s = lua_tostring(L1, getindex);
      const var_na *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("xmove") {
      puur_naa_nka f = getindex;
      puur_naa_nka t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      puur_naa_nka n = getnum;
      ပေႃး (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("yield") {
      ပွၵ်ႈ lua_yield(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("yieldk") {
      puur_naa_nka nres = getnum;
      puur_naa_nka i = getindex;
      ပွၵ်ႈ lua_yieldk(L1, nres, i, Cfunck);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    ပေႃးဢမ်ႇ ပေႃး EQ("argerror") {
      puur_naa_nka arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    ပေႃးဢမ်ႇ luaL_error(L, "unknown instruction %s", buff);
  }
  ပွၵ်ႈ 0;
}


static puur_naa_nka testC (lua_State *L) {
  lua_State *L1;
  const var_na *pc;
  ပေႃး (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  ပေႃးဢမ်ႇ ပေႃး (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  ပေႃးဢမ်ႇ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ပွၵ်ႈ runC(L, L1, pc);
}


static puur_naa_nka Cfunc (lua_State *L) {
  ပွၵ်ႈ runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static puur_naa_nka Cfunck (lua_State *L, puur_naa_nka status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ပွၵ်ႈ runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static puur_naa_nka makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ပွၵ်ႈ 1;
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
static void Chook (lua_State *L, lua_Debug *ar) {
  const var_na *scpt;
  const var_na *const events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_pop(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
static void sethookaux (lua_State *L, puur_naa_nka mask, puur_naa_nka count, const var_na *scpt) {
  ပေႃး (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    ပွၵ်ႈ;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ပေႃး (!lua_istable(L, -1)) {  /* no hook table? */
    lua_pop(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, count);
}


static puur_naa_nka sethook (lua_State *L) {
  ပေႃး (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  ပေႃးဢမ်ႇ {
    const var_na *scpt = luaL_checkstring(L, 1);
    const var_na *smask = luaL_checkstring(L, 2);
    puur_naa_nka count = cast_int(luaL_optinteger(L, 3, 0));
    puur_naa_nka mask = 0;
    ပေႃး (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    ပေႃး (strchr(smask, 'r')) mask |= LUA_MASKRET;
    ပေႃး (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    ပေႃး (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  ပွၵ်ႈ 0;
}


static puur_naa_nka coresume (lua_State *L) {
  puur_naa_nka status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  ပေႃး (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ပွၵ်ႈ 2;  /* return false + error message */
  }
  ပေႃးဢမ်ႇ {
    lua_pushboolean(L, 1);
    ပွၵ်ႈ 1;
  }
}

#ပေႃး !defined(LUA_USE_POSIX)

#define nonblock	NULL

#ပေႃးဢမ်ႇ

#include <unistd.h>
#include <fcntl.h>

static puur_naa_nka nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  puur_naa_nka fd = fileno(f);
  puur_naa_nka flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ပွၵ်ႈ 0;
}
#endif

/* }====================================================== */



static const sa_nracanaa luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_count},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {NULL, NULL}
};


static void checkfinalmem (void) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


puur_naa_nka luaB_opentests (lua_State *L) {
  void *ud;
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
  ပွၵ်ႈ 1;
}

#endif

