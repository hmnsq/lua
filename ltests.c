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
#ប្រសិនបើ defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static ចំនួនគត់ runC (lua_State *L, lua_State *L1, const var_na *pc);


static void setnameval (lua_State *L, const var_na *name, ចំនួនគត់ val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const var_na *fmt, const var_na *s1, const var_na *s2) {
  fprintf(stderr, fmt, s1);
  ប្រសិនបើ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static ចំនួនគត់ tpanic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ត្រឡប់ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const var_na *msg, ចំនួនគត់ tocont) {
  lua_State *L = cast(lua_State *, ud);
  static var_na buff[200] = "";  /* should be enough for tests... */
  static ចំនួនគត់ onoff = 0;
  static ចំនួនគត់ mode = 0;  /* start in normal mode */
  static ចំនួនគត់ lasttocont = 0;
  ប្រសិនបើ (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    ប្រសិនបើ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    ប្រសិនបើ (strcmp(msg, "@off") == 0)
      onoff = 0;
    បើមិនដូច្នេះ ប្រសិនបើ (strcmp(msg, "@on") == 0)
      onoff = 1;
    បើមិនដូច្នេះ ប្រសិនបើ (strcmp(msg, "@normal") == 0)
      mode = 0;
    បើមិនដូច្នេះ ប្រសិនបើ (strcmp(msg, "@allow") == 0)
      mode = 1;
    បើមិនដូច្នេះ ប្រសិនបើ (strcmp(msg, "@store") == 0)
      mode = 2;
    បើមិនដូច្នេះ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    ត្រឡប់;
  }
  lasttocont = tocont;
  ប្រសិនបើ (strlen(msg) >= parimaa_na(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  ប្រសិនបើ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ប្រសិនបើ (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    បើមិនដូច្នេះ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      pak_sa 0: {  /* normal */
        ប្រសិនបើ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      pak_sa 1: {  /* allow */
        ប្រសិនបើ (onoff)
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
    ចំនួនគត់ type;
  } d;
} memHeader;


#ប្រសិនបើ !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#បើមិនដូច្នេះ

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  ប្រសិនបើ (block) {
    size_t size = block->d.size;
    ចំនួនគត់ i;
    សម្រាប់ (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  ចំនួនគត់ type;
  ប្រសិនបើ (mc->memlimit == 0) {  /* first time? */
    var_na *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  ប្រសិនបើ (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  បើមិនដូច្នេះ {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  ប្រសិនបើ (size == 0) {
    freeblock(mc, block);
    ត្រឡប់ NULL;
  }
  ប្រសិនបើ (mc->failnext) {
    mc->failnext = 0;
    ត្រឡប់ NULL;  /* fake a single memory allocation error */
  }
  ប្រសិនបើ (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    ប្រសិនបើ (mc->countlimit == 0)
      ត្រឡប់ NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ប្រសិនបើ (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ត្រឡប់ NULL;  /* fake a memory allocation error */
  បើមិនដូច្នេះ {
    memHeader *newblock;
    ចំនួនគត់ i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = parimaa_na(memHeader) + size + MARKSIZE;
    ប្រសិនបើ (realsize < size) ត្រឡប់ NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    ប្រសិនបើ (newblock == NULL)
      ត្រឡប់ NULL;  /* really out of memory? */
    ប្រសិនបើ (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    សម្រាប់ (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    ប្រសិនបើ (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ត្រឡប់ newblock + 1;
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
static ចំនួនគត់ testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ប្រសិនបើ (isdead(g,t)) ត្រឡប់ 0;
  ប្រសិនបើ (issweepphase(g))
    ត្រឡប់ 1;  /* no invariants */
  បើមិនដូច្នេះ ប្រសិនបើ (g->gckind != KGC_GENMINOR)
    ត្រឡប់ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  បើមិនដូច្នេះ {  /* generational mode */
    ប្រសិនបើ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ត្រឡប់ 0;
    ប្រសិនបើ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ត្រឡប់ 0;
    ត្រឡប់ 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ប្រសិនបើ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static ចំនួនគត់ testobjref (global_State *g, GCObject *f, GCObject *t) {
  ចំនួនគត់ r1 = testobjref1(g, f, t);
  ប្រសិនបើ (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ត្រឡប់ r1;
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
#define checkobjrefN(g,f,t)	{ ប្រសិនបើ (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned ចំនួនគត់ i;
  unsigned ចំនួនគត់ asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  សម្រាប់ (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  សម្រាប់ (n = gnode(h, 0); n < limit; n++) {
    ប្រសិនបើ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  ចំនួនគត់ i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  សម្រាប់ (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  ចំនួនគត់ i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  សម្រាប់ (i=0; i<f->sizek; i++) {
    ប្រសិនបើ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  សម្រាប់ (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  សម្រាប់ (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  សម្រាប់ (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ចំនួនគត់ i;
  សម្រាប់ (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ចំនួនគត់ i;
  checkobjrefN(g, clgc, cl->p);
  សម្រាប់ (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    ប្រសិនបើ (uv) {
      checkobjrefN(g, clgc, uv);
      ប្រសិនបើ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static ចំនួនគត់ lua_checkpc (CallInfo *ci) {
  ប្រសិនបើ (!isLua(ci)) ត្រឡប់ 1;
  បើមិនដូច្នេះ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ត្រឡប់ p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  ប្រសិនបើ (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    ត្រឡប់;
  }
  សម្រាប់ (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  សម្រាប់ (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  សម្រាប់ (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, ចំនួនគត់ maybedead,
                         ចំនួនគត់ listage) {
  ប្រសិនបើ (isdead(g, o))
    assert(maybedead);
  បើមិនដូច្នេះ {
    assert(g->gcstate != GCSpause || iswhite(o));
    ប្រសិនបើ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      ប្រសិនបើ (isold(o)) {
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
  ចំនួនគត់ total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ខណៈពេល (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    ប្រសិនបើ (keepinvariant(g))
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
  ត្រឡប់ total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  ប្រសិនបើ (!keepinvariant(g)) ត្រឡប់ total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ត្រឡប់ total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  ប្រសិនបើ (!keepinvariant(g))
    ត្រឡប់;  /* gray lists not being kept in these phases */
  ប្រសិនបើ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    ត្រឡប់;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ប្រសិនបើ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, ចំនួនគត់ maybedead, ចំនួនគត់ tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  សម្រាប់ (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  សម្រាប់ (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  សម្រាប់ (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  សម្រាប់ (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ត្រឡប់ total;
}


ចំនួនគត់ lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ចំនួនគត់ maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ប្រសិនបើ (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  សម្រាប់ (o = g->fixedgc; o != NULL; o = o->next) {
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
  សម្រាប់ (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ប្រសិនបើ (keepinvariant(g))
    assert(totalin == totalshould);
  ត្រឡប់ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static var_na *buildop (Proto *p, ចំនួនគត់ pc, var_na *buff) {
  var_na *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const var_na *name = opnames[o];
  ចំនួនគត់ line = luaG_getfuncline(p, pc);
  ចំនួនគត់ lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  ប្រសិនបើ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  បើមិនដូច្នេះ
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
  ត្រឡប់ obuff;
}


#ប្រសិនបើ 0
void luaI_printcode (Proto *pt, ចំនួនគត់ size) {
  ចំនួនគត់ pc;
  សម្រាប់ (pc=0; pc<size; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, ចំនួនគត់ pc) {
  var_na buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static ចំនួនគត់ listcode (lua_State *L) {
  ចំនួនគត់ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  សម្រាប់ (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ត្រឡប់ 1;
}


static ចំនួនគត់ printcode (lua_State *L) {
  ចំនួនគត់ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  សម្រាប់ (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ត្រឡប់ 0;
}


static ចំនួនគត់ listk (lua_State *L) {
  Proto *p;
  ចំនួនគត់ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  សម្រាប់ (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ត្រឡប់ 1;
}


static ចំនួនគត់ listabslineinfo (lua_State *L) {
  Proto *p;
  ចំនួនគត់ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  សម្រាប់ (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ត្រឡប់ 1;
}


static ចំនួនគត់ listlocals (lua_State *L) {
  Proto *p;
  ចំនួនគត់ pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ចំនួនគត់ i = 0;
  const var_na *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ខណៈពេល ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  ត្រឡប់ i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  ចំនួនគត់ i;
  ចំនួនគត់ n = lua_gettop(L);
  printf("stack: >>\n");
  សម្រាប់ (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


ចំនួនគត់ lua_printallstack (lua_State *L) {
  StkId p;
  ចំនួនគត់ i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  សម្រាប់ (p = L->stack.p; p < L->top.p; p++) {
    ប្រសិនបើ (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      ប្រសិនបើ (ci == L->ci)
        ci = NULL;  /* printed last frame */
      បើមិនដូច្នេះ
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ត្រឡប់ 0;
}


static ចំនួនគត់ get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  ត្រឡប់ 1;
}


static ចំនួនគត់ get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", parimaa_na(lua_State));
  setnameval(L, "global state", parimaa_na(global_State));
  setnameval(L, "TValue", parimaa_na(TValue));
  setnameval(L, "Node", parimaa_na(Node));
  setnameval(L, "stack Value", parimaa_na(StackValue));
  ត្រឡប់ 1;
}


static ចំនួនគត់ mem_query (lua_State *L) {
  ប្រសិនបើ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ត្រឡប់ 3;
  }
  បើមិនដូច្នេះ ប្រសិនបើ (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    ប្រសិនបើ (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ត្រឡប់ 0;
  }
  បើមិនដូច្នេះ {
    const var_na *t = luaL_checkstring(L, 1);
    ចំនួនគត់ i;
    សម្រាប់ (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ប្រសិនបើ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ត្រឡប់ 1;
      }
    }
    ត្រឡប់ luaL_error(L, "unknown type '%s'", t);
  }
}


static ចំនួនគត់ alloc_count (lua_State *L) {
  ប្រសិនបើ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  បើមិនដូច្នេះ
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ត្រឡប់ 0;
}


static ចំនួនគត់ alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ត្រឡប់ 0;
}


static ចំនួនគត់ settrick (lua_State *L) {
  ប្រសិនបើ (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  បើមិនដូច្នេះ
    l_Trick = gcvalue(obj_at(L, 1));
  ត្រឡប់ 0;
}


static ចំនួនគត់ gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ប្រសិនបើ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  បើមិនដូច្នេះ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ត្រឡប់ 1;
}


static ចំនួនគត់ gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ប្រសិនបើ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  បើមិនដូច្នេះ {
    static const var_na *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ត្រឡប់ 1;
}


static ចំនួនគត់ gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ប្រសិនបើ (!iscollectable(o))
    printf("no collectable\n");
  បើមិនដូច្នេះ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ត្រឡប់ 0;
}


static const var_na *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static ចំនួនគត់ gc_state (lua_State *L) {
  static const ចំនួនគត់ states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ចំនួនគត់ option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ប្រសិនបើ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ត្រឡប់ 1;
  }
  បើមិនដូច្នេះ {
    ប្រសិនបើ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    ប្រសិនបើ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ត្រឡប់ 0;
  }
}


static ចំនួនគត់ tracinggc = 0;
void luai_tracegctest (lua_State *L, ចំនួនគត់ first) {
  ប្រសិនបើ (!tracinggc) ត្រឡប់;
  បើមិនដូច្នេះ {
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


static ចំនួនគត់ tracegc (lua_State *L) {
  ប្រសិនបើ (lua_isnil(L, 1))
    tracinggc = 0;
  បើមិនដូច្នេះ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ត្រឡប់ 0;
}


static ចំនួនគត់ hash_query (lua_State *L) {
  ប្រសិនបើ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ប្រសិនបើ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  បើមិនដូច្នេះ {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  ត្រឡប់ 1;
}


static ចំនួនគត់ stacklevel (lua_State *L) {
  ចំនួនគត់ a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ត្រឡប់ 5;
}


static ចំនួនគត់ resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ខណៈពេល (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ត្រឡប់ 0;
}


static ចំនួនគត់ reallocstack (lua_State *L) {
  ចំនួនគត់ n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  ត្រឡប់ 0;
}


static ចំនួនគត់ table_query (lua_State *L) {
  const Table *t;
  ចំនួនគត់ i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned ចំនួនគត់ asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ប្រសិនបើ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ត្រឡប់ 3;
  }
  បើមិនដូច្នេះ ប្រសិនបើ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ប្រសិនបើ (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    បើមិនដូច្នេះ
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  បើមិនដូច្នេះ ប្រសិនបើ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ប្រសិនបើ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    បើមិនដូច្នេះ
      lua_pushliteral(L, "<undef>");
    ប្រសិនបើ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    បើមិនដូច្នេះ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  ត្រឡប់ 3;
}


static ចំនួនគត់ gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ត្រឡប់ 6;
}


static ចំនួនគត់ test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ត្រឡប់ 1;
}


static ចំនួនគត់ test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ត្រឡប់ 1;
}


static ចំនួនគត់ string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ចំនួនគត់ s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ប្រសិនបើ (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ត្រឡប់ 2;
  }
  បើមិនដូច្នេះ ប្រសិនបើ (s < tb->size) {
    TString *ts;
    ចំនួនគត់ n = 0;
    សម្រាប់ (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    ត្រឡប់ n;
  }
  បើមិនដូច្នេះ ត្រឡប់ 0;
}


static ចំនួនគត់ getreftable (lua_State *L) {
  ប្រសិនបើ (lua_istable(L, 2))  /* is there a table as second argument? */
    ត្រឡប់ 2;  /* use it as the table */
  បើមិនដូច្នេះ
    ត្រឡប់ LUA_REGISTRYINDEX;  /* default is to use the register */
}


static ចំនួនគត់ tref (lua_State *L) {
  ចំនួនគត់ t = getreftable(L);
  ចំនួនគត់ level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ត្រឡប់ 1;
}


static ចំនួនគត់ getref (lua_State *L) {
  ចំនួនគត់ t = getreftable(L);
  ចំនួនគត់ level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ត្រឡប់ 1;
}

static ចំនួនគត់ unref (lua_State *L) {
  ចំនួនគត់ t = getreftable(L);
  ចំនួនគត់ level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ត្រឡប់ 0;
}


static ចំនួនគត់ upvalue (lua_State *L) {
  ចំនួនគត់ n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ប្រសិនបើ (lua_isnone(L, 3)) {
    const var_na *name = lua_getupvalue(L, 1, n);
    ប្រសិនបើ (name == NULL) ត្រឡប់ 0;
    lua_pushstring(L, name);
    ត្រឡប់ 2;
  }
  បើមិនដូច្នេះ {
    const var_na *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ត្រឡប់ 1;
  }
}


static ចំនួនគត់ newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  ចំនួនគត់ nuv = cast_int(luaL_optinteger(L, 2, 0));
  var_na *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  ខណៈពេល (size--) *p++ = '\0';
  ត្រឡប់ 1;
}


static ចំនួនគត់ pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ត្រឡប់ 1;
}


static ចំនួនគត់ udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ត្រឡប់ 1;
}


static ចំនួនគត់ doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  ចំនួនគត់ status = luaL_loadbufferx(L1, s, l, s, "t");
  ប្រសិនបើ (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ត្រឡប់ 1;
}


static ចំនួនគត់ s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const dviguu_na *, luaL_checkstring(L, 1))));
  ត្រឡប់ 1;
}


static ចំនួនគត់ d2s (lua_State *L) {
  dviguu_na d = cast(dviguu_na, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), parimaa_na(d));
  ត្រឡប់ 1;
}


static ចំនួនគត់ num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ត្រឡប់ 1;
}


static ចំនួនគត់ makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ត្រឡប់ 1;
}


static ចំនួនគត់ newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ប្រសិនបើ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  បើមិនដូច្នេះ
    lua_pushnil(L);
  ត្រឡប់ 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  ត្រឡប់ L1;
}


static ចំនួនគត់ loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ចំនួនគត់ load = cast_int(luaL_checkinteger(L, 2));
  ចំនួនគត់ preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ត្រឡប់ 0;
}

static ចំនួនគត់ closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ត្រឡប់ 0;
}

static ចំនួនគត់ doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const var_na *code = luaL_checklstring(L, 2, &lcode);
  ចំនួនគត់ status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  ប្រសិនបើ (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ប្រសិនបើ (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ត្រឡប់ 3;
  }
  បើមិនដូច្នេះ {
    ចំនួនគត់ i = 0;
    ខណៈពេល (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    ត្រឡប់ i-1;
  }
}


static ចំនួនគត់ log2_aux (lua_State *L) {
  unsigned ចំនួនគត់ x = (unsigned ចំនួនគត់)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ត្រឡប់ 1;
}


sa_nracanaa Aux { jmp_buf jb; const var_na *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static ចំនួនគត់ panicback (lua_State *L) {
  sa_nracanaa Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (sa_nracanaa Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ត្រឡប់ 1;  /* to avoid warnings */
}

static ចំនួនគត់ checkpanic (lua_State *L) {
  sa_nracanaa Aux b;
  void *ud;
  lua_State *L1;
  const var_na *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ប្រសិនបើ (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ត្រឡប់ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ប្រសិនបើ (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  បើមិនដូច្នេះ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ត្រឡប់ 1;
}


static ចំនួនគត់ externKstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  ត្រឡប់ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static ចំនួនគត់ externstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  var_na *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  ប្រសិនបើ (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * parimaa_na(var_na));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ត្រឡប់ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, ចំនួនគត់ mask, ចំនួនគត់ count, const var_na *code);

static const var_na *const delimits = " \t\n,;";

static void skip (const var_na **pc) {
  សម្រាប់ (;;) {
    ប្រសិនបើ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    បើមិនដូច្នេះ ប្រសិនបើ (**pc == '#') {  /* comment? */
      ខណៈពេល (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    បើមិនដូច្នេះ bha_nga;
  }
}

static ចំនួនគត់ getnum_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  ចំនួនគត់ res = 0;
  ចំនួនគត់ sig = 1;
  skip(pc);
  ប្រសិនបើ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    ត្រឡប់ res;
  }
  បើមិនដូច្នេះ ប្រសិនបើ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ត្រឡប់ res;
  }
  បើមិនដូច្នេះ ប្រសិនបើ (**pc == '!') {
    (*pc)++;
    ប្រសិនបើ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    បើមិនដូច្នេះ ប្រសិនបើ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    បើមិនដូច្នេះ lua_assert(0);
    (*pc)++;
    ត្រឡប់ res;
  }
  បើមិនដូច្នេះ ប្រសិនបើ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ប្រសិនបើ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ខណៈពេល (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ត្រឡប់ sig*res;
}

static const var_na *getstring_aux (lua_State *L, var_na *buff, const var_na **pc) {
  ចំនួនគត់ i = 0;
  skip(pc);
  ប្រសិនបើ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ចំនួនគត់ quote = *(*pc)++;
    ខណៈពេល (**pc != quote) {
      ប្រសិនបើ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  បើមិនដូច្នេះ {
    ខណៈពេល (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ត្រឡប់ buff;
}


static ចំនួនគត់ getindex_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  skip(pc);
  switch (*(*pc)++) {
    pak_sa 'R': ត្រឡប់ LUA_REGISTRYINDEX;
    pak_sa 'U': ត្រឡប់ lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      ចំនួនគត់ n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ប្រសិនបើ (n == 0) ត្រឡប់ 0;
      បើមិនដូច្នេះ ត្រឡប់ lua_absindex(L1, n);
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
  unsigned ចំនួនគត់ i;
  សម្រាប់ (i = 0; i < parimaa_na(statcodes) / parimaa_na(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static ចំនួនគត់ testC (lua_State *L);
static ចំនួនគត់ Cfunck (lua_State *L, ចំនួនគត់ status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const var_na ops[] = "+-*%^/\\&|~<>_!";

static ចំនួនគត់ runC (lua_State *L, lua_State *L1, const var_na *pc) {
  var_na buff[300];
  ចំនួនគត់ status = 0;
  ប្រសិនបើ (pc == NULL) ត្រឡប់ luaL_error(L, "attempt to runC null script");
  សម្រាប់ (;;) {
    const var_na *inst = getstring;
    ប្រសិនបើ EQ("") ត្រឡប់ 0;
    បើមិនដូច្នេះ ប្រសិនបើ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("append") {
      ចំនួនគត់ t = getindex;
      ចំនួនគត់ i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("arith") {
      ចំនួនគត់ op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("call") {
      ចំនួនគត់ narg = getnum;
      ចំនួនគត់ nres = getnum;
      lua_call(L1, narg, nres);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("callk") {
      ចំនួនគត់ narg = getnum;
      ចំនួនគត់ nres = getnum;
      ចំនួនគត់ i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("checkstack") {
      ចំនួនគត់ sz = getnum;
      const var_na *msg = getstring;
      ប្រសិនបើ (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rawcheckstack") {
      ចំនួនគត់ sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("compare") {
      const var_na *opt = getstring;  /* EQ, LT, or LE */
      ចំនួនគត់ op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ចំនួនគត់ a = getindex;
      ចំនួនគត់ b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("concat") {
      lua_concat(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("copy") {
      ចំនួនគត់ f = getindex;
      lua_copy(L1, f, getindex);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("getfield") {
      ចំនួនគត់ t = getindex;
      ចំនួនគត់ tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("getmetatable") {
      ប្រសិនបើ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("gettable") {
      ចំនួនគត់ tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("gsub") {
      ចំនួនគត់ a = getnum; ចំនួនគត់ b = getnum; ចំនួនគត់ c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("insert") {
      lua_insert(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("len") {
      lua_len(L1, getindex);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("loadstring") {
      size_t slen;
      const var_na *s = luaL_checklstring(L1, getnum, &slen);
      const var_na *name = getstring;
      const var_na *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("newtable") {
      lua_newtable(L1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("newthread") {
      lua_newthread(L1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("next") {
      lua_next(L1, -2);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pcall") {
      ចំនួនគត់ narg = getnum;
      ចំនួនគត់ nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pcallk") {
      ចំនួនគត់ narg = getnum;
      ចំនួនគត់ nres = getnum;
      ចំនួនគត់ i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pop") {
      lua_pop(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("printstack") {
      ចំនួនគត់ n = getnum;
      ប្រសិនបើ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      បើមិនដូច្នេះ lua_printstack(L1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("print") {
      const var_na *msg = getstring;
      printf("%s\n", msg);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("warningC") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("warning") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushnil") {
      lua_pushnil(L1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ចំនួនគត់)lua_tointeger(L, -1));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rawget") {
      ចំនួនគត់ t = getindex;
      lua_rawget(L1, t);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rawgeti") {
      ចំនួនគត់ t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rawgetp") {
      ចំនួនគត់ t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rawset") {
      ចំនួនគត់ t = getindex;
      lua_rawset(L1, t);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rawseti") {
      ចំនួនគត់ t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rawsetp") {
      ចំនួនគត់ t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("remove") {
      lua_remove(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("replace") {
      lua_replace(L1, getindex);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("resume") {
      ចំនួនគត់ i = getindex;
      ចំនួនគត់ nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("traceback") {
      const var_na *msg = getstring;
      ចំនួនគត់ level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("return") {
      ចំនួនគត់ n = getnum;
      ប្រសិនបើ (L1 != L) {
        ចំនួនគត់ i;
        សម្រាប់ (i = 0; i < n; i++) {
          ចំនួនគត់ idx = -(n - i);
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
      ត្រឡប់ n;
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("rotate") {
      ចំនួនគត់ i = getindex;
      lua_rotate(L1, i, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("setfield") {
      ចំនួនគត់ t = getindex;
      const var_na *s = getstring;
      lua_setfield(L1, t, s);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("seti") {
      ចំនួនគត់ t = getindex;
      lua_seti(L1, t, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("setglobal") {
      const var_na *s = getstring;
      lua_setglobal(L1, s);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("sethook") {
      ចំនួនគត់ mask = getnum;
      ចំនួនគត់ count = getnum;
      const var_na *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("setmetatable") {
      ចំនួនគត់ idx = getindex;
      lua_setmetatable(L1, idx);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("settable") {
      lua_settable(L1, getindex);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("settop") {
      lua_settop(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("testudata") {
      ចំនួនគត់ i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("error") {
      lua_error(L1);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("abort") {
      abort();
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("throw") {
#ប្រសិនបើ defined(__cplusplus)
static sa_nracanaa X { ចំនួនគត់ x; } x;
      throw x;
#បើមិនដូច្នេះ
      luaL_error(L1, "C++");
#endif
      bha_nga;
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("tostring") {
      const var_na *s = lua_tostring(L1, getindex);
      const var_na *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("xmove") {
      ចំនួនគត់ f = getindex;
      ចំនួនគត់ t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ចំនួនគត់ n = getnum;
      ប្រសិនបើ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("yield") {
      ត្រឡប់ lua_yield(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("yieldk") {
      ចំនួនគត់ nres = getnum;
      ចំនួនគត់ i = getindex;
      ត្រឡប់ lua_yieldk(L1, nres, i, Cfunck);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    បើមិនដូច្នេះ ប្រសិនបើ EQ("argerror") {
      ចំនួនគត់ arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    បើមិនដូច្នេះ luaL_error(L, "unknown instruction %s", buff);
  }
  ត្រឡប់ 0;
}


static ចំនួនគត់ testC (lua_State *L) {
  lua_State *L1;
  const var_na *pc;
  ប្រសិនបើ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  បើមិនដូច្នេះ ប្រសិនបើ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  បើមិនដូច្នេះ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ត្រឡប់ runC(L, L1, pc);
}


static ចំនួនគត់ Cfunc (lua_State *L) {
  ត្រឡប់ runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static ចំនួនគត់ Cfunck (lua_State *L, ចំនួនគត់ status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ត្រឡប់ runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static ចំនួនគត់ makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ត្រឡប់ 1;
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
static void sethookaux (lua_State *L, ចំនួនគត់ mask, ចំនួនគត់ count, const var_na *scpt) {
  ប្រសិនបើ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    ត្រឡប់;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ប្រសិនបើ (!lua_istable(L, -1)) {  /* no hook table? */
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


static ចំនួនគត់ sethook (lua_State *L) {
  ប្រសិនបើ (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  បើមិនដូច្នេះ {
    const var_na *scpt = luaL_checkstring(L, 1);
    const var_na *smask = luaL_checkstring(L, 2);
    ចំនួនគត់ count = cast_int(luaL_optinteger(L, 3, 0));
    ចំនួនគត់ mask = 0;
    ប្រសិនបើ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    ប្រសិនបើ (strchr(smask, 'r')) mask |= LUA_MASKRET;
    ប្រសិនបើ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    ប្រសិនបើ (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  ត្រឡប់ 0;
}


static ចំនួនគត់ coresume (lua_State *L) {
  ចំនួនគត់ status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  ប្រសិនបើ (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ត្រឡប់ 2;  /* return false + error message */
  }
  បើមិនដូច្នេះ {
    lua_pushboolean(L, 1);
    ត្រឡប់ 1;
  }
}

#ប្រសិនបើ !defined(LUA_USE_POSIX)

#define nonblock	NULL

#បើមិនដូច្នេះ

#include <unistd.h>
#include <fcntl.h>

static ចំនួនគត់ nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ចំនួនគត់ fd = fileno(f);
  ចំនួនគត់ flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ត្រឡប់ 0;
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


ចំនួនគត់ luaB_opentests (lua_State *L) {
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
  ត្រឡប់ 1;
}

#endif

