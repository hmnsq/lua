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
#ຖ້າ defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static ຈຳນວນເຕັມ runC (lua_State *L, lua_State *L1, const var_na *pc);


static void setnameval (lua_State *L, const var_na *name, ຈຳນວນເຕັມ val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const var_na *fmt, const var_na *s1, const var_na *s2) {
  fprintf(stderr, fmt, s1);
  ຖ້າ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static ຈຳນວນເຕັມ tpanic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ສົ່ງຄືນ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const var_na *msg, ຈຳນວນເຕັມ tocont) {
  lua_State *L = cast(lua_State *, ud);
  static var_na buff[200] = "";  /* should be enough for tests... */
  static ຈຳນວນເຕັມ onoff = 0;
  static ຈຳນວນເຕັມ mode = 0;  /* start in normal mode */
  static ຈຳນວນເຕັມ lasttocont = 0;
  ຖ້າ (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    ຖ້າ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    ຖ້າ (strcmp(msg, "@off") == 0)
      onoff = 0;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(msg, "@on") == 0)
      onoff = 1;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(msg, "@normal") == 0)
      mode = 0;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(msg, "@allow") == 0)
      mode = 1;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(msg, "@store") == 0)
      mode = 2;
    ບໍ່ດັ່ງນັ້ນ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    ສົ່ງຄືນ;
  }
  lasttocont = tocont;
  ຖ້າ (strlen(msg) >= parimaa_na(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  ຖ້າ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ຖ້າ (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    ບໍ່ດັ່ງນັ້ນ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      pak_sa 0: {  /* normal */
        ຖ້າ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      pak_sa 1: {  /* allow */
        ຖ້າ (onoff)
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
    ຈຳນວນເຕັມ type;
  } d;
} memHeader;


#ຖ້າ !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#ບໍ່ດັ່ງນັ້ນ

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  ຖ້າ (block) {
    size_t size = block->d.size;
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  ຈຳນວນເຕັມ type;
  ຖ້າ (mc->memlimit == 0) {  /* first time? */
    var_na *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  ຖ້າ (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  ບໍ່ດັ່ງນັ້ນ {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  ຖ້າ (size == 0) {
    freeblock(mc, block);
    ສົ່ງຄືນ NULL;
  }
  ຖ້າ (mc->failnext) {
    mc->failnext = 0;
    ສົ່ງຄືນ NULL;  /* fake a single memory allocation error */
  }
  ຖ້າ (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    ຖ້າ (mc->countlimit == 0)
      ສົ່ງຄືນ NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ຖ້າ (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ສົ່ງຄືນ NULL;  /* fake a memory allocation error */
  ບໍ່ດັ່ງນັ້ນ {
    memHeader *newblock;
    ຈຳນວນເຕັມ i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = parimaa_na(memHeader) + size + MARKSIZE;
    ຖ້າ (realsize < size) ສົ່ງຄືນ NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    ຖ້າ (newblock == NULL)
      ສົ່ງຄືນ NULL;  /* really out of memory? */
    ຖ້າ (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    ສຳລັບ (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    ຖ້າ (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ສົ່ງຄືນ newblock + 1;
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
static ຈຳນວນເຕັມ testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ຖ້າ (isdead(g,t)) ສົ່ງຄືນ 0;
  ຖ້າ (issweepphase(g))
    ສົ່ງຄືນ 1;  /* no invariants */
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (g->gckind != KGC_GENMINOR)
    ສົ່ງຄືນ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  ບໍ່ດັ່ງນັ້ນ {  /* generational mode */
    ຖ້າ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ສົ່ງຄືນ 0;
    ຖ້າ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ສົ່ງຄືນ 0;
    ສົ່ງຄືນ 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ຖ້າ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static ຈຳນວນເຕັມ testobjref (global_State *g, GCObject *f, GCObject *t) {
  ຈຳນວນເຕັມ r1 = testobjref1(g, f, t);
  ຖ້າ (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ສົ່ງຄືນ r1;
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
#define checkobjrefN(g,f,t)	{ ຖ້າ (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned ຈຳນວນເຕັມ i;
  unsigned ຈຳນວນເຕັມ asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  ສຳລັບ (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  ສຳລັບ (n = gnode(h, 0); n < limit; n++) {
    ຖ້າ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  ຈຳນວນເຕັມ i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  ສຳລັບ (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  ຈຳນວນເຕັມ i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  ສຳລັບ (i=0; i<f->sizek; i++) {
    ຖ້າ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  ສຳລັບ (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  ສຳລັບ (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  ສຳລັບ (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ຈຳນວນເຕັມ i;
  ສຳລັບ (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ຈຳນວນເຕັມ i;
  checkobjrefN(g, clgc, cl->p);
  ສຳລັບ (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    ຖ້າ (uv) {
      checkobjrefN(g, clgc, uv);
      ຖ້າ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static ຈຳນວນເຕັມ lua_checkpc (CallInfo *ci) {
  ຖ້າ (!isLua(ci)) ສົ່ງຄືນ 1;
  ບໍ່ດັ່ງນັ້ນ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ສົ່ງຄືນ p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  ຖ້າ (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    ສົ່ງຄືນ;
  }
  ສຳລັບ (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  ສຳລັບ (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  ສຳລັບ (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, ຈຳນວນເຕັມ maybedead,
                         ຈຳນວນເຕັມ listage) {
  ຖ້າ (isdead(g, o))
    assert(maybedead);
  ບໍ່ດັ່ງນັ້ນ {
    assert(g->gcstate != GCSpause || iswhite(o));
    ຖ້າ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      ຖ້າ (isold(o)) {
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
  ຈຳນວນເຕັມ total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ໃນຂະນະທີ່ (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    ຖ້າ (keepinvariant(g))
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
  ສົ່ງຄືນ total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  ຖ້າ (!keepinvariant(g)) ສົ່ງຄືນ total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ສົ່ງຄືນ total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  ຖ້າ (!keepinvariant(g))
    ສົ່ງຄືນ;  /* gray lists not being kept in these phases */
  ຖ້າ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    ສົ່ງຄືນ;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ຖ້າ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, ຈຳນວນເຕັມ maybedead, ຈຳນວນເຕັມ tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  ສຳລັບ (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ສຳລັບ (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ສຳລັບ (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ສຳລັບ (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ສົ່ງຄືນ total;
}


ຈຳນວນເຕັມ lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ຈຳນວນເຕັມ maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ຖ້າ (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  ສຳລັບ (o = g->fixedgc; o != NULL; o = o->next) {
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
  ສຳລັບ (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ຖ້າ (keepinvariant(g))
    assert(totalin == totalshould);
  ສົ່ງຄືນ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static var_na *buildop (Proto *p, ຈຳນວນເຕັມ pc, var_na *buff) {
  var_na *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const var_na *name = opnames[o];
  ຈຳນວນເຕັມ line = luaG_getfuncline(p, pc);
  ຈຳນວນເຕັມ lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  ຖ້າ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  ບໍ່ດັ່ງນັ້ນ
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
  ສົ່ງຄືນ obuff;
}


#ຖ້າ 0
void luaI_printcode (Proto *pt, ຈຳນວນເຕັມ size) {
  ຈຳນວນເຕັມ pc;
  ສຳລັບ (pc=0; pc<size; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, ຈຳນວນເຕັມ pc) {
  var_na buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static ຈຳນວນເຕັມ listcode (lua_State *L) {
  ຈຳນວນເຕັມ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  ສຳລັບ (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ printcode (lua_State *L) {
  ຈຳນວນເຕັມ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  ສຳລັບ (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ listk (lua_State *L) {
  Proto *p;
  ຈຳນວນເຕັມ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  ສຳລັບ (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ listabslineinfo (lua_State *L) {
  Proto *p;
  ຈຳນວນເຕັມ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  ສຳລັບ (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ listlocals (lua_State *L) {
  Proto *p;
  ຈຳນວນເຕັມ pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ຈຳນວນເຕັມ i = 0;
  const var_na *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ໃນຂະນະທີ່ ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  ສົ່ງຄືນ i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  ຈຳນວນເຕັມ i;
  ຈຳນວນເຕັມ n = lua_gettop(L);
  printf("stack: >>\n");
  ສຳລັບ (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


ຈຳນວນເຕັມ lua_printallstack (lua_State *L) {
  StkId p;
  ຈຳນວນເຕັມ i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  ສຳລັບ (p = L->stack.p; p < L->top.p; p++) {
    ຖ້າ (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      ຖ້າ (ci == L->ci)
        ci = NULL;  /* printed last frame */
      ບໍ່ດັ່ງນັ້ນ
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", parimaa_na(lua_State));
  setnameval(L, "global state", parimaa_na(global_State));
  setnameval(L, "TValue", parimaa_na(TValue));
  setnameval(L, "Node", parimaa_na(Node));
  setnameval(L, "stack Value", parimaa_na(StackValue));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ mem_query (lua_State *L) {
  ຖ້າ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ສົ່ງຄືນ 3;
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    ຖ້າ (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ສົ່ງຄືນ 0;
  }
  ບໍ່ດັ່ງນັ້ນ {
    const var_na *t = luaL_checkstring(L, 1);
    ຈຳນວນເຕັມ i;
    ສຳລັບ (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ຖ້າ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ສົ່ງຄືນ 1;
      }
    }
    ສົ່ງຄືນ luaL_error(L, "unknown type '%s'", t);
  }
}


static ຈຳນວນເຕັມ alloc_count (lua_State *L) {
  ຖ້າ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  ບໍ່ດັ່ງນັ້ນ
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ settrick (lua_State *L) {
  ຖ້າ (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  ບໍ່ດັ່ງນັ້ນ
    l_Trick = gcvalue(obj_at(L, 1));
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ຖ້າ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ບໍ່ດັ່ງນັ້ນ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ຖ້າ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ບໍ່ດັ່ງນັ້ນ {
    static const var_na *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ຖ້າ (!iscollectable(o))
    printf("no collectable\n");
  ບໍ່ດັ່ງນັ້ນ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ສົ່ງຄືນ 0;
}


static const var_na *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static ຈຳນວນເຕັມ gc_state (lua_State *L) {
  static const ຈຳນວນເຕັມ states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ຈຳນວນເຕັມ option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ຖ້າ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ສົ່ງຄືນ 1;
  }
  ບໍ່ດັ່ງນັ້ນ {
    ຖ້າ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    ຖ້າ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ສົ່ງຄືນ 0;
  }
}


static ຈຳນວນເຕັມ tracinggc = 0;
void luai_tracegctest (lua_State *L, ຈຳນວນເຕັມ first) {
  ຖ້າ (!tracinggc) ສົ່ງຄືນ;
  ບໍ່ດັ່ງນັ້ນ {
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


static ຈຳນວນເຕັມ tracegc (lua_State *L) {
  ຖ້າ (lua_isnil(L, 1))
    tracinggc = 0;
  ບໍ່ດັ່ງນັ້ນ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ hash_query (lua_State *L) {
  ຖ້າ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ຖ້າ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  ບໍ່ດັ່ງນັ້ນ {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ stacklevel (lua_State *L) {
  ຈຳນວນເຕັມ a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ສົ່ງຄືນ 5;
}


static ຈຳນວນເຕັມ resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ໃນຂະນະທີ່ (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ reallocstack (lua_State *L) {
  ຈຳນວນເຕັມ n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ table_query (lua_State *L) {
  const Table *t;
  ຈຳນວນເຕັມ i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned ຈຳນວນເຕັມ asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ຖ້າ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ສົ່ງຄືນ 3;
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ຖ້າ (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    ບໍ່ດັ່ງນັ້ນ
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ຖ້າ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    ບໍ່ດັ່ງນັ້ນ
      lua_pushliteral(L, "<undef>");
    ຖ້າ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    ບໍ່ດັ່ງນັ້ນ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  ສົ່ງຄືນ 3;
}


static ຈຳນວນເຕັມ gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ສົ່ງຄືນ 6;
}


static ຈຳນວນເຕັມ test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ຈຳນວນເຕັມ s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ຖ້າ (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ສົ່ງຄືນ 2;
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (s < tb->size) {
    TString *ts;
    ຈຳນວນເຕັມ n = 0;
    ສຳລັບ (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    ສົ່ງຄືນ n;
  }
  ບໍ່ດັ່ງນັ້ນ ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ getreftable (lua_State *L) {
  ຖ້າ (lua_istable(L, 2))  /* is there a table as second argument? */
    ສົ່ງຄືນ 2;  /* use it as the table */
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ LUA_REGISTRYINDEX;  /* default is to use the register */
}


static ຈຳນວນເຕັມ tref (lua_State *L) {
  ຈຳນວນເຕັມ t = getreftable(L);
  ຈຳນວນເຕັມ level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ getref (lua_State *L) {
  ຈຳນວນເຕັມ t = getreftable(L);
  ຈຳນວນເຕັມ level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ສົ່ງຄືນ 1;
}

static ຈຳນວນເຕັມ unref (lua_State *L) {
  ຈຳນວນເຕັມ t = getreftable(L);
  ຈຳນວນເຕັມ level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ upvalue (lua_State *L) {
  ຈຳນວນເຕັມ n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ຖ້າ (lua_isnone(L, 3)) {
    const var_na *name = lua_getupvalue(L, 1, n);
    ຖ້າ (name == NULL) ສົ່ງຄືນ 0;
    lua_pushstring(L, name);
    ສົ່ງຄືນ 2;
  }
  ບໍ່ດັ່ງນັ້ນ {
    const var_na *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ສົ່ງຄືນ 1;
  }
}


static ຈຳນວນເຕັມ newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  ຈຳນວນເຕັມ nuv = cast_int(luaL_optinteger(L, 2, 0));
  var_na *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  ໃນຂະນະທີ່ (size--) *p++ = '\0';
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  ຈຳນວນເຕັມ status = luaL_loadbufferx(L1, s, l, s, "t");
  ຖ້າ (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const dviguu_na *, luaL_checkstring(L, 1))));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ d2s (lua_State *L) {
  dviguu_na d = cast(dviguu_na, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), parimaa_na(d));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ຖ້າ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  ບໍ່ດັ່ງນັ້ນ
    lua_pushnil(L);
  ສົ່ງຄືນ 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  ສົ່ງຄືນ L1;
}


static ຈຳນວນເຕັມ loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ຈຳນວນເຕັມ load = cast_int(luaL_checkinteger(L, 2));
  ຈຳນວນເຕັມ preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const var_na *code = luaL_checklstring(L, 2, &lcode);
  ຈຳນວນເຕັມ status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  ຖ້າ (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ຖ້າ (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ສົ່ງຄືນ 3;
  }
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ i = 0;
    ໃນຂະນະທີ່ (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    ສົ່ງຄືນ i-1;
  }
}


static ຈຳນວນເຕັມ log2_aux (lua_State *L) {
  unsigned ຈຳນວນເຕັມ x = (unsigned ຈຳນວນເຕັມ)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ສົ່ງຄືນ 1;
}


sa_nracanaa Aux { jmp_buf jb; const var_na *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static ຈຳນວນເຕັມ panicback (lua_State *L) {
  sa_nracanaa Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (sa_nracanaa Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ສົ່ງຄືນ 1;  /* to avoid warnings */
}

static ຈຳນວນເຕັມ checkpanic (lua_State *L) {
  sa_nracanaa Aux b;
  void *ud;
  lua_State *L1;
  const var_na *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ຖ້າ (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ສົ່ງຄືນ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ຖ້າ (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  ບໍ່ດັ່ງນັ້ນ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ສົ່ງຄືນ 1;
}


static ຈຳນວນເຕັມ externKstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  ສົ່ງຄືນ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static ຈຳນວນເຕັມ externstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  var_na *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  ຖ້າ (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * parimaa_na(var_na));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ສົ່ງຄືນ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, ຈຳນວນເຕັມ mask, ຈຳນວນເຕັມ count, const var_na *code);

static const var_na *const delimits = " \t\n,;";

static void skip (const var_na **pc) {
  ສຳລັບ (;;) {
    ຖ້າ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (**pc == '#') {  /* comment? */
      ໃນຂະນະທີ່ (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    ບໍ່ດັ່ງນັ້ນ bha_nga;
  }
}

static ຈຳນວນເຕັມ getnum_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  ຈຳນວນເຕັມ res = 0;
  ຈຳນວນເຕັມ sig = 1;
  skip(pc);
  ຖ້າ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    ສົ່ງຄືນ res;
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ສົ່ງຄືນ res;
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (**pc == '!') {
    (*pc)++;
    ຖ້າ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    ບໍ່ດັ່ງນັ້ນ lua_assert(0);
    (*pc)++;
    ສົ່ງຄືນ res;
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ຖ້າ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ໃນຂະນະທີ່ (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ສົ່ງຄືນ sig*res;
}

static const var_na *getstring_aux (lua_State *L, var_na *buff, const var_na **pc) {
  ຈຳນວນເຕັມ i = 0;
  skip(pc);
  ຖ້າ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ຈຳນວນເຕັມ quote = *(*pc)++;
    ໃນຂະນະທີ່ (**pc != quote) {
      ຖ້າ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  ບໍ່ດັ່ງນັ້ນ {
    ໃນຂະນະທີ່ (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ສົ່ງຄືນ buff;
}


static ຈຳນວນເຕັມ getindex_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  skip(pc);
  switch (*(*pc)++) {
    pak_sa 'R': ສົ່ງຄືນ LUA_REGISTRYINDEX;
    pak_sa 'U': ສົ່ງຄືນ lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      ຈຳນວນເຕັມ n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ຖ້າ (n == 0) ສົ່ງຄືນ 0;
      ບໍ່ດັ່ງນັ້ນ ສົ່ງຄືນ lua_absindex(L1, n);
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
  unsigned ຈຳນວນເຕັມ i;
  ສຳລັບ (i = 0; i < parimaa_na(statcodes) / parimaa_na(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static ຈຳນວນເຕັມ testC (lua_State *L);
static ຈຳນວນເຕັມ Cfunck (lua_State *L, ຈຳນວນເຕັມ status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const var_na ops[] = "+-*%^/\\&|~<>_!";

static ຈຳນວນເຕັມ runC (lua_State *L, lua_State *L1, const var_na *pc) {
  var_na buff[300];
  ຈຳນວນເຕັມ status = 0;
  ຖ້າ (pc == NULL) ສົ່ງຄືນ luaL_error(L, "attempt to runC null script");
  ສຳລັບ (;;) {
    const var_na *inst = getstring;
    ຖ້າ EQ("") ສົ່ງຄືນ 0;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("append") {
      ຈຳນວນເຕັມ t = getindex;
      ຈຳນວນເຕັມ i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("arith") {
      ຈຳນວນເຕັມ op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("call") {
      ຈຳນວນເຕັມ narg = getnum;
      ຈຳນວນເຕັມ nres = getnum;
      lua_call(L1, narg, nres);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("callk") {
      ຈຳນວນເຕັມ narg = getnum;
      ຈຳນວນເຕັມ nres = getnum;
      ຈຳນວນເຕັມ i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("checkstack") {
      ຈຳນວນເຕັມ sz = getnum;
      const var_na *msg = getstring;
      ຖ້າ (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rawcheckstack") {
      ຈຳນວນເຕັມ sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("compare") {
      const var_na *opt = getstring;  /* EQ, LT, or LE */
      ຈຳນວນເຕັມ op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ຈຳນວນເຕັມ a = getindex;
      ຈຳນວນເຕັມ b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("concat") {
      lua_concat(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("copy") {
      ຈຳນວນເຕັມ f = getindex;
      lua_copy(L1, f, getindex);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("getfield") {
      ຈຳນວນເຕັມ t = getindex;
      ຈຳນວນເຕັມ tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("getmetatable") {
      ຖ້າ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("gettable") {
      ຈຳນວນເຕັມ tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("gsub") {
      ຈຳນວນເຕັມ a = getnum; ຈຳນວນເຕັມ b = getnum; ຈຳນວນເຕັມ c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("insert") {
      lua_insert(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("len") {
      lua_len(L1, getindex);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("loadstring") {
      size_t slen;
      const var_na *s = luaL_checklstring(L1, getnum, &slen);
      const var_na *name = getstring;
      const var_na *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("newtable") {
      lua_newtable(L1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("newthread") {
      lua_newthread(L1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("next") {
      lua_next(L1, -2);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pcall") {
      ຈຳນວນເຕັມ narg = getnum;
      ຈຳນວນເຕັມ nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pcallk") {
      ຈຳນວນເຕັມ narg = getnum;
      ຈຳນວນເຕັມ nres = getnum;
      ຈຳນວນເຕັມ i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pop") {
      lua_pop(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("printstack") {
      ຈຳນວນເຕັມ n = getnum;
      ຖ້າ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      ບໍ່ດັ່ງນັ້ນ lua_printstack(L1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("print") {
      const var_na *msg = getstring;
      printf("%s\n", msg);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("warningC") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("warning") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushnil") {
      lua_pushnil(L1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ຈຳນວນເຕັມ)lua_tointeger(L, -1));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rawget") {
      ຈຳນວນເຕັມ t = getindex;
      lua_rawget(L1, t);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rawgeti") {
      ຈຳນວນເຕັມ t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rawgetp") {
      ຈຳນວນເຕັມ t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rawset") {
      ຈຳນວນເຕັມ t = getindex;
      lua_rawset(L1, t);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rawseti") {
      ຈຳນວນເຕັມ t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rawsetp") {
      ຈຳນວນເຕັມ t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("remove") {
      lua_remove(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("replace") {
      lua_replace(L1, getindex);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("resume") {
      ຈຳນວນເຕັມ i = getindex;
      ຈຳນວນເຕັມ nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("traceback") {
      const var_na *msg = getstring;
      ຈຳນວນເຕັມ level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("return") {
      ຈຳນວນເຕັມ n = getnum;
      ຖ້າ (L1 != L) {
        ຈຳນວນເຕັມ i;
        ສຳລັບ (i = 0; i < n; i++) {
          ຈຳນວນເຕັມ idx = -(n - i);
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
      ສົ່ງຄືນ n;
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("rotate") {
      ຈຳນວນເຕັມ i = getindex;
      lua_rotate(L1, i, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("setfield") {
      ຈຳນວນເຕັມ t = getindex;
      const var_na *s = getstring;
      lua_setfield(L1, t, s);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("seti") {
      ຈຳນວນເຕັມ t = getindex;
      lua_seti(L1, t, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("setglobal") {
      const var_na *s = getstring;
      lua_setglobal(L1, s);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("sethook") {
      ຈຳນວນເຕັມ mask = getnum;
      ຈຳນວນເຕັມ count = getnum;
      const var_na *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("setmetatable") {
      ຈຳນວນເຕັມ idx = getindex;
      lua_setmetatable(L1, idx);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("settable") {
      lua_settable(L1, getindex);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("settop") {
      lua_settop(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("testudata") {
      ຈຳນວນເຕັມ i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("error") {
      lua_error(L1);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("abort") {
      abort();
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("throw") {
#ຖ້າ defined(__cplusplus)
static sa_nracanaa X { ຈຳນວນເຕັມ x; } x;
      throw x;
#ບໍ່ດັ່ງນັ້ນ
      luaL_error(L1, "C++");
#endif
      bha_nga;
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("tostring") {
      const var_na *s = lua_tostring(L1, getindex);
      const var_na *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("xmove") {
      ຈຳນວນເຕັມ f = getindex;
      ຈຳນວນເຕັມ t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ຈຳນວນເຕັມ n = getnum;
      ຖ້າ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("yield") {
      ສົ່ງຄືນ lua_yield(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("yieldk") {
      ຈຳນວນເຕັມ nres = getnum;
      ຈຳນວນເຕັມ i = getindex;
      ສົ່ງຄືນ lua_yieldk(L1, nres, i, Cfunck);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ EQ("argerror") {
      ຈຳນວນເຕັມ arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    ບໍ່ດັ່ງນັ້ນ luaL_error(L, "unknown instruction %s", buff);
  }
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ testC (lua_State *L) {
  lua_State *L1;
  const var_na *pc;
  ຖ້າ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  ບໍ່ດັ່ງນັ້ນ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ສົ່ງຄືນ runC(L, L1, pc);
}


static ຈຳນວນເຕັມ Cfunc (lua_State *L) {
  ສົ່ງຄືນ runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static ຈຳນວນເຕັມ Cfunck (lua_State *L, ຈຳນວນເຕັມ status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ສົ່ງຄືນ runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static ຈຳນວນເຕັມ makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ສົ່ງຄືນ 1;
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
static void sethookaux (lua_State *L, ຈຳນວນເຕັມ mask, ຈຳນວນເຕັມ count, const var_na *scpt) {
  ຖ້າ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    ສົ່ງຄືນ;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ຖ້າ (!lua_istable(L, -1)) {  /* no hook table? */
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


static ຈຳນວນເຕັມ sethook (lua_State *L) {
  ຖ້າ (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  ບໍ່ດັ່ງນັ້ນ {
    const var_na *scpt = luaL_checkstring(L, 1);
    const var_na *smask = luaL_checkstring(L, 2);
    ຈຳນວນເຕັມ count = cast_int(luaL_optinteger(L, 3, 0));
    ຈຳນວນເຕັມ mask = 0;
    ຖ້າ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    ຖ້າ (strchr(smask, 'r')) mask |= LUA_MASKRET;
    ຖ້າ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    ຖ້າ (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  ສົ່ງຄືນ 0;
}


static ຈຳນວນເຕັມ coresume (lua_State *L) {
  ຈຳນວນເຕັມ status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  ຖ້າ (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ສົ່ງຄືນ 2;  /* return false + error message */
  }
  ບໍ່ດັ່ງນັ້ນ {
    lua_pushboolean(L, 1);
    ສົ່ງຄືນ 1;
  }
}

#ຖ້າ !defined(LUA_USE_POSIX)

#define nonblock	NULL

#ບໍ່ດັ່ງນັ້ນ

#include <unistd.h>
#include <fcntl.h>

static ຈຳນວນເຕັມ nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ຈຳນວນເຕັມ fd = fileno(f);
  ຈຳນວນເຕັມ flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ສົ່ງຄືນ 0;
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


ຈຳນວນເຕັມ luaB_opentests (lua_State *L) {
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
  ສົ່ງຄືນ 1;
}

#endif

