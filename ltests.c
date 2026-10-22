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
#නම් defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static පූර්ණ_සංඛ්‍යා runC (lua_State *L, lua_State *L1, const var_na *pc);


static void setnameval (lua_State *L, const var_na *name, පූර්ණ_සංඛ්‍යා val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const var_na *fmt, const var_na *s1, const var_na *s2) {
  fprintf(stderr, fmt, s1);
  නම් (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static පූර්ණ_සංඛ්‍යා tpanic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ආපසු (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const var_na *msg, පූර්ණ_සංඛ්‍යා tocont) {
  lua_State *L = cast(lua_State *, ud);
  static var_na buff[200] = "";  /* should be enough for tests... */
  static පූර්ණ_සංඛ්‍යා onoff = 0;
  static පූර්ණ_සංඛ්‍යා mode = 0;  /* start in normal mode */
  static පූර්ණ_සංඛ්‍යා lasttocont = 0;
  නම් (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    නම් (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    නම් (strcmp(msg, "@off") == 0)
      onoff = 0;
    නැතහොත් නම් (strcmp(msg, "@on") == 0)
      onoff = 1;
    නැතහොත් නම් (strcmp(msg, "@normal") == 0)
      mode = 0;
    නැතහොත් නම් (strcmp(msg, "@allow") == 0)
      mode = 1;
    නැතහොත් නම් (strcmp(msg, "@store") == 0)
      mode = 2;
    නැතහොත්
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    ආපසු;
  }
  lasttocont = tocont;
  නම් (strlen(msg) >= parimaa_na(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  නම් (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    නම් (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    නැතහොත් {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      pak_sa 0: {  /* normal */
        නම් (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      pak_sa 1: {  /* allow */
        නම් (onoff)
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
    පූර්ණ_සංඛ්‍යා type;
  } d;
} memHeader;


#නම් !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#නැතහොත්

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  නම් (block) {
    size_t size = block->d.size;
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  පූර්ණ_සංඛ්‍යා type;
  නම් (mc->memlimit == 0) {  /* first time? */
    var_na *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  නම් (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  නැතහොත් {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  නම් (size == 0) {
    freeblock(mc, block);
    ආපසු NULL;
  }
  නම් (mc->failnext) {
    mc->failnext = 0;
    ආපසු NULL;  /* fake a single memory allocation error */
  }
  නම් (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    නම් (mc->countlimit == 0)
      ආපසු NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  නම් (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ආපසු NULL;  /* fake a memory allocation error */
  නැතහොත් {
    memHeader *newblock;
    පූර්ණ_සංඛ්‍යා i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = parimaa_na(memHeader) + size + MARKSIZE;
    නම් (realsize < size) ආපසු NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    නම් (newblock == NULL)
      ආපසු NULL;  /* really out of memory? */
    නම් (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    සඳහා (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    නම් (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ආපසු newblock + 1;
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
static පූර්ණ_සංඛ්‍යා testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  නම් (isdead(g,t)) ආපසු 0;
  නම් (issweepphase(g))
    ආපසු 1;  /* no invariants */
  නැතහොත් නම් (g->gckind != KGC_GENMINOR)
    ආපසු !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  නැතහොත් {  /* generational mode */
    නම් ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ආපසු 0;
    නම් ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ආපසු 0;
    ආපසු 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  නම් (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static පූර්ණ_සංඛ්‍යා testobjref (global_State *g, GCObject *f, GCObject *t) {
  පූර්ණ_සංඛ්‍යා r1 = testobjref1(g, f, t);
  නම් (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ආපසු r1;
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
#define checkobjrefN(g,f,t)	{ නම් (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned පූර්ණ_සංඛ්‍යා i;
  unsigned පූර්ණ_සංඛ්‍යා asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  සඳහා (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  සඳහා (n = gnode(h, 0); n < limit; n++) {
    නම් (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  පූර්ණ_සංඛ්‍යා i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  සඳහා (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  පූර්ණ_සංඛ්‍යා i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  සඳහා (i=0; i<f->sizek; i++) {
    නම් (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  සඳහා (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  සඳහා (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  සඳහා (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  පූර්ණ_සංඛ්‍යා i;
  සඳහා (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  පූර්ණ_සංඛ්‍යා i;
  checkobjrefN(g, clgc, cl->p);
  සඳහා (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    නම් (uv) {
      checkobjrefN(g, clgc, uv);
      නම් (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static පූර්ණ_සංඛ්‍යා lua_checkpc (CallInfo *ci) {
  නම් (!isLua(ci)) ආපසු 1;
  නැතහොත් {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ආපසු p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  නම් (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    ආපසු;
  }
  සඳහා (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  සඳහා (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  සඳහා (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, පූර්ණ_සංඛ්‍යා maybedead,
                         පූර්ණ_සංඛ්‍යා listage) {
  නම් (isdead(g, o))
    assert(maybedead);
  නැතහොත් {
    assert(g->gcstate != GCSpause || iswhite(o));
    නම් (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      නම් (isold(o)) {
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
  පූර්ණ_සංඛ්‍යා total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  අතරතුර (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    නම් (keepinvariant(g))
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
  ආපසු total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  නම් (!keepinvariant(g)) ආපසු total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ආපසු total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  නම් (!keepinvariant(g))
    ආපසු;  /* gray lists not being kept in these phases */
  නම් (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    ආපසු;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  නම් (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, පූර්ණ_සංඛ්‍යා maybedead, පූර්ණ_සංඛ්‍යා tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  සඳහා (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  සඳහා (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  සඳහා (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  සඳහා (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ආපසු total;
}


පූර්ණ_සංඛ්‍යා lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  පූර්ණ_සංඛ්‍යා maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  නම් (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  සඳහා (o = g->fixedgc; o != NULL; o = o->next) {
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
  සඳහා (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  නම් (keepinvariant(g))
    assert(totalin == totalshould);
  ආපසු 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static var_na *buildop (Proto *p, පූර්ණ_සංඛ්‍යා pc, var_na *buff) {
  var_na *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const var_na *name = opnames[o];
  පූර්ණ_සංඛ්‍යා line = luaG_getfuncline(p, pc);
  පූර්ණ_සංඛ්‍යා lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  නම් (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  නැතහොත්
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
  ආපසු obuff;
}


#නම් 0
void luaI_printcode (Proto *pt, පූර්ණ_සංඛ්‍යා size) {
  පූර්ණ_සංඛ්‍යා pc;
  සඳහා (pc=0; pc<size; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, පූර්ණ_සංඛ්‍යා pc) {
  var_na buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static පූර්ණ_සංඛ්‍යා listcode (lua_State *L) {
  පූර්ණ_සංඛ්‍යා pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  සඳහා (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා printcode (lua_State *L) {
  පූර්ණ_සංඛ්‍යා pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  සඳහා (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා listk (lua_State *L) {
  Proto *p;
  පූර්ණ_සංඛ්‍යා i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  සඳහා (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා listabslineinfo (lua_State *L) {
  Proto *p;
  පූර්ණ_සංඛ්‍යා i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  සඳහා (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා listlocals (lua_State *L) {
  Proto *p;
  පූර්ණ_සංඛ්‍යා pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  පූර්ණ_සංඛ්‍යා i = 0;
  const var_na *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  අතරතුර ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  ආපසු i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා n = lua_gettop(L);
  printf("stack: >>\n");
  සඳහා (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


පූර්ණ_සංඛ්‍යා lua_printallstack (lua_State *L) {
  StkId p;
  පූර්ණ_සංඛ්‍යා i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  සඳහා (p = L->stack.p; p < L->top.p; p++) {
    නම් (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      නම් (ci == L->ci)
        ci = NULL;  /* printed last frame */
      නැතහොත්
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", parimaa_na(lua_State));
  setnameval(L, "global state", parimaa_na(global_State));
  setnameval(L, "TValue", parimaa_na(TValue));
  setnameval(L, "Node", parimaa_na(Node));
  setnameval(L, "stack Value", parimaa_na(StackValue));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා mem_query (lua_State *L) {
  නම් (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ආපසු 3;
  }
  නැතහොත් නම් (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    නම් (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ආපසු 0;
  }
  නැතහොත් {
    const var_na *t = luaL_checkstring(L, 1);
    පූර්ණ_සංඛ්‍යා i;
    සඳහා (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      නම් (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ආපසු 1;
      }
    }
    ආපසු luaL_error(L, "unknown type '%s'", t);
  }
}


static පූර්ණ_සංඛ්‍යා alloc_count (lua_State *L) {
  නම් (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  නැතහොත්
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා settrick (lua_State *L) {
  නම් (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  නැතහොත්
    l_Trick = gcvalue(obj_at(L, 1));
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  නම් (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  නැතහොත් {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  නම් (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  නැතහොත් {
    static const var_na *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  නම් (!iscollectable(o))
    printf("no collectable\n");
  නැතහොත් {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ආපසු 0;
}


static const var_na *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static පූර්ණ_සංඛ්‍යා gc_state (lua_State *L) {
  static const පූර්ණ_සංඛ්‍යා states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  පූර්ණ_සංඛ්‍යා option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  නම් (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ආපසු 1;
  }
  නැතහොත් {
    නම් (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    නම් (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ආපසු 0;
  }
}


static පූර්ණ_සංඛ්‍යා tracinggc = 0;
void luai_tracegctest (lua_State *L, පූර්ණ_සංඛ්‍යා first) {
  නම් (!tracinggc) ආපසු;
  නැතහොත් {
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


static පූර්ණ_සංඛ්‍යා tracegc (lua_State *L) {
  නම් (lua_isnil(L, 1))
    tracinggc = 0;
  නැතහොත් {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා hash_query (lua_State *L) {
  නම් (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    නම් (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  නැතහොත් {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා stacklevel (lua_State *L) {
  පූර්ණ_සංඛ්‍යා a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ආපසු 5;
}


static පූර්ණ_සංඛ්‍යා resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  අතරතුර (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා reallocstack (lua_State *L) {
  පූර්ණ_සංඛ්‍යා n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා table_query (lua_State *L) {
  const Table *t;
  පූර්ණ_සංඛ්‍යා i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned පූර්ණ_සංඛ්‍යා asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  නම් (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ආපසු 3;
  }
  නැතහොත් නම් (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    නම් (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    නැතහොත්
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  නැතහොත් නම් (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    නම් (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    නැතහොත්
      lua_pushliteral(L, "<undef>");
    නම් (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    නැතහොත්
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  ආපසු 3;
}


static පූර්ණ_සංඛ්‍යා gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ආපසු 6;
}


static පූර්ණ_සංඛ්‍යා test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  පූර්ණ_සංඛ්‍යා s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  නම් (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ආපසු 2;
  }
  නැතහොත් නම් (s < tb->size) {
    TString *ts;
    පූර්ණ_සංඛ්‍යා n = 0;
    සඳහා (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    ආපසු n;
  }
  නැතහොත් ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා getreftable (lua_State *L) {
  නම් (lua_istable(L, 2))  /* is there a table as second argument? */
    ආපසු 2;  /* use it as the table */
  නැතහොත්
    ආපසු LUA_REGISTRYINDEX;  /* default is to use the register */
}


static පූර්ණ_සංඛ්‍යා tref (lua_State *L) {
  පූර්ණ_සංඛ්‍යා t = getreftable(L);
  පූර්ණ_සංඛ්‍යා level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා getref (lua_State *L) {
  පූර්ණ_සංඛ්‍යා t = getreftable(L);
  පූර්ණ_සංඛ්‍යා level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ආපසු 1;
}

static පූර්ණ_සංඛ්‍යා unref (lua_State *L) {
  පූර්ණ_සංඛ්‍යා t = getreftable(L);
  පූර්ණ_සංඛ්‍යා level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා upvalue (lua_State *L) {
  පූර්ණ_සංඛ්‍යා n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  නම් (lua_isnone(L, 3)) {
    const var_na *name = lua_getupvalue(L, 1, n);
    නම් (name == NULL) ආපසු 0;
    lua_pushstring(L, name);
    ආපසු 2;
  }
  නැතහොත් {
    const var_na *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ආපසු 1;
  }
}


static පූර්ණ_සංඛ්‍යා newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  පූර්ණ_සංඛ්‍යා nuv = cast_int(luaL_optinteger(L, 2, 0));
  var_na *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  අතරතුර (size--) *p++ = '\0';
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  පූර්ණ_සංඛ්‍යා status = luaL_loadbufferx(L1, s, l, s, "t");
  නම් (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const dviguu_na *, luaL_checkstring(L, 1))));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා d2s (lua_State *L) {
  dviguu_na d = cast(dviguu_na, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), parimaa_na(d));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  නම් (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  නැතහොත්
    lua_pushnil(L);
  ආපසු 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  ආපසු L1;
}


static පූර්ණ_සංඛ්‍යා loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  පූර්ණ_සංඛ්‍යා load = cast_int(luaL_checkinteger(L, 2));
  පූර්ණ_සංඛ්‍යා preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const var_na *code = luaL_checklstring(L, 2, &lcode);
  පූර්ණ_සංඛ්‍යා status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  නම් (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  නම් (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ආපසු 3;
  }
  නැතහොත් {
    පූර්ණ_සංඛ්‍යා i = 0;
    අතරතුර (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    ආපසු i-1;
  }
}


static පූර්ණ_සංඛ්‍යා log2_aux (lua_State *L) {
  unsigned පූර්ණ_සංඛ්‍යා x = (unsigned පූර්ණ_සංඛ්‍යා)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ආපසු 1;
}


sa_nracanaa Aux { jmp_buf jb; const var_na *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static පූර්ණ_සංඛ්‍යා panicback (lua_State *L) {
  sa_nracanaa Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (sa_nracanaa Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ආපසු 1;  /* to avoid warnings */
}

static පූර්ණ_සංඛ්‍යා checkpanic (lua_State *L) {
  sa_nracanaa Aux b;
  void *ud;
  lua_State *L1;
  const var_na *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  නම් (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ආපසු 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  නම් (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  නැතහොත් {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ආපසු 1;
}


static පූර්ණ_සංඛ්‍යා externKstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  ආපසු 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static පූර්ණ_සංඛ්‍යා externstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  var_na *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  නම් (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * parimaa_na(var_na));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ආපසු 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, පූර්ණ_සංඛ්‍යා mask, පූර්ණ_සංඛ්‍යා count, const var_na *code);

static const var_na *const delimits = " \t\n,;";

static void skip (const var_na **pc) {
  සඳහා (;;) {
    නම් (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    නැතහොත් නම් (**pc == '#') {  /* comment? */
      අතරතුර (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    නැතහොත් bha_nga;
  }
}

static පූර්ණ_සංඛ්‍යා getnum_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  පූර්ණ_සංඛ්‍යා res = 0;
  පූර්ණ_සංඛ්‍යා sig = 1;
  skip(pc);
  නම් (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    ආපසු res;
  }
  නැතහොත් නම් (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ආපසු res;
  }
  නැතහොත් නම් (**pc == '!') {
    (*pc)++;
    නම් (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    නැතහොත් නම් (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    නැතහොත් lua_assert(0);
    (*pc)++;
    ආපසු res;
  }
  නැතහොත් නම් (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  නම් (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  අතරතුර (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ආපසු sig*res;
}

static const var_na *getstring_aux (lua_State *L, var_na *buff, const var_na **pc) {
  පූර්ණ_සංඛ්‍යා i = 0;
  skip(pc);
  නම් (**pc == '"' || **pc == '\'') {  /* quoted string? */
    පූර්ණ_සංඛ්‍යා quote = *(*pc)++;
    අතරතුර (**pc != quote) {
      නම් (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  නැතහොත් {
    අතරතුර (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ආපසු buff;
}


static පූර්ණ_සංඛ්‍යා getindex_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  skip(pc);
  switch (*(*pc)++) {
    pak_sa 'R': ආපසු LUA_REGISTRYINDEX;
    pak_sa 'U': ආපසු lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      පූර්ණ_සංඛ්‍යා n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      නම් (n == 0) ආපසු 0;
      නැතහොත් ආපසු lua_absindex(L1, n);
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
  unsigned පූර්ණ_සංඛ්‍යා i;
  සඳහා (i = 0; i < parimaa_na(statcodes) / parimaa_na(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static පූර්ණ_සංඛ්‍යා testC (lua_State *L);
static පූර්ණ_සංඛ්‍යා Cfunck (lua_State *L, පූර්ණ_සංඛ්‍යා status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const var_na ops[] = "+-*%^/\\&|~<>_!";

static පූර්ණ_සංඛ්‍යා runC (lua_State *L, lua_State *L1, const var_na *pc) {
  var_na buff[300];
  පූර්ණ_සංඛ්‍යා status = 0;
  නම් (pc == NULL) ආපසු luaL_error(L, "attempt to runC null script");
  සඳහා (;;) {
    const var_na *inst = getstring;
    නම් EQ("") ආපසු 0;
    නැතහොත් නම් EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    නැතහොත් නම් EQ("append") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      පූර්ණ_සංඛ්‍යා i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    නැතහොත් නම් EQ("arith") {
      පූර්ණ_සංඛ්‍යා op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    නැතහොත් නම් EQ("call") {
      පූර්ණ_සංඛ්‍යා narg = getnum;
      පූර්ණ_සංඛ්‍යා nres = getnum;
      lua_call(L1, narg, nres);
    }
    නැතහොත් නම් EQ("callk") {
      පූර්ණ_සංඛ්‍යා narg = getnum;
      පූර්ණ_සංඛ්‍යා nres = getnum;
      පූර්ණ_සංඛ්‍යා i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    නැතහොත් නම් EQ("checkstack") {
      පූර්ණ_සංඛ්‍යා sz = getnum;
      const var_na *msg = getstring;
      නම් (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    නැතහොත් නම් EQ("rawcheckstack") {
      පූර්ණ_සංඛ්‍යා sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    නැතහොත් නම් EQ("compare") {
      const var_na *opt = getstring;  /* EQ, LT, or LE */
      පූර්ණ_සංඛ්‍යා op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      පූර්ණ_සංඛ්‍යා a = getindex;
      පූර්ණ_සංඛ්‍යා b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    නැතහොත් නම් EQ("concat") {
      lua_concat(L1, getnum);
    }
    නැතහොත් නම් EQ("copy") {
      පූර්ණ_සංඛ්‍යා f = getindex;
      lua_copy(L1, f, getindex);
    }
    නැතහොත් නම් EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    නැතහොත් නම් EQ("getfield") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      පූර්ණ_සංඛ්‍යා tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    නැතහොත් නම් EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    නැතහොත් නම් EQ("getmetatable") {
      නම් (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    නැතහොත් නම් EQ("gettable") {
      පූර්ණ_සංඛ්‍යා tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    නැතහොත් නම් EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    නැතහොත් නම් EQ("gsub") {
      පූර්ණ_සංඛ්‍යා a = getnum; පූර්ණ_සංඛ්‍යා b = getnum; පූර්ණ_සංඛ්‍යා c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    නැතහොත් නම් EQ("insert") {
      lua_insert(L1, getnum);
    }
    නැතහොත් නම් EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    නැතහොත් නම් EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    නැතහොත් නම් EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    නැතහොත් නම් EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    නැතහොත් නම් EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    නැතහොත් නම් EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    නැතහොත් නම් EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    නැතහොත් නම් EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    නැතහොත් නම් EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    නැතහොත් නම් EQ("len") {
      lua_len(L1, getindex);
    }
    නැතහොත් නම් EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    නැතහොත් නම් EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    නැතහොත් නම් EQ("loadstring") {
      size_t slen;
      const var_na *s = luaL_checklstring(L1, getnum, &slen);
      const var_na *name = getstring;
      const var_na *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    නැතහොත් නම් EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    නැතහොත් නම් EQ("newtable") {
      lua_newtable(L1);
    }
    නැතහොත් නම් EQ("newthread") {
      lua_newthread(L1);
    }
    නැතහොත් නම් EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    නැතහොත් නම් EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    නැතහොත් නම් EQ("next") {
      lua_next(L1, -2);
    }
    නැතහොත් නම් EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    නැතහොත් නම් EQ("pcall") {
      පූර්ණ_සංඛ්‍යා narg = getnum;
      පූර්ණ_සංඛ්‍යා nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    නැතහොත් නම් EQ("pcallk") {
      පූර්ණ_සංඛ්‍යා narg = getnum;
      පූර්ණ_සංඛ්‍යා nres = getnum;
      පූර්ණ_සංඛ්‍යා i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    නැතහොත් නම් EQ("pop") {
      lua_pop(L1, getnum);
    }
    නැතහොත් නම් EQ("printstack") {
      පූර්ණ_සංඛ්‍යා n = getnum;
      නම් (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      නැතහොත් lua_printstack(L1);
    }
    නැතහොත් නම් EQ("print") {
      const var_na *msg = getstring;
      printf("%s\n", msg);
    }
    නැතහොත් නම් EQ("warningC") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    නැතහොත් නම් EQ("warning") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    නැතහොත් නම් EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    නැතහොත් නම් EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    නැතහොත් නම් EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    නැතහොත් නම් EQ("pushnil") {
      lua_pushnil(L1);
    }
    නැතහොත් නම් EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    නැතහොත් නම් EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    නැතහොත් නම් EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    නැතහොත් නම් EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    නැතහොත් නම් EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    නැතහොත් නම් EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (පූර්ණ_සංඛ්‍යා)lua_tointeger(L, -1));
    }
    නැතහොත් නම් EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    නැතහොත් නම් EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    නැතහොත් නම් EQ("rawget") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_rawget(L1, t);
    }
    නැතහොත් නම් EQ("rawgeti") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    නැතහොත් නම් EQ("rawgetp") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    නැතහොත් නම් EQ("rawset") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_rawset(L1, t);
    }
    නැතහොත් නම් EQ("rawseti") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    නැතහොත් නම් EQ("rawsetp") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    නැතහොත් නම් EQ("remove") {
      lua_remove(L1, getnum);
    }
    නැතහොත් නම් EQ("replace") {
      lua_replace(L1, getindex);
    }
    නැතහොත් නම් EQ("resume") {
      පූර්ණ_සංඛ්‍යා i = getindex;
      පූර්ණ_සංඛ්‍යා nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    නැතහොත් නම් EQ("traceback") {
      const var_na *msg = getstring;
      පූර්ණ_සංඛ්‍යා level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    නැතහොත් නම් EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    නැතහොත් නම් EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    නැතහොත් නම් EQ("return") {
      පූර්ණ_සංඛ්‍යා n = getnum;
      නම් (L1 != L) {
        පූර්ණ_සංඛ්‍යා i;
        සඳහා (i = 0; i < n; i++) {
          පූර්ණ_සංඛ්‍යා idx = -(n - i);
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
      ආපසු n;
    }
    නැතහොත් නම් EQ("rotate") {
      පූර්ණ_සංඛ්‍යා i = getindex;
      lua_rotate(L1, i, getnum);
    }
    නැතහොත් නම් EQ("setfield") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      const var_na *s = getstring;
      lua_setfield(L1, t, s);
    }
    නැතහොත් නම් EQ("seti") {
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_seti(L1, t, getnum);
    }
    නැතහොත් නම් EQ("setglobal") {
      const var_na *s = getstring;
      lua_setglobal(L1, s);
    }
    නැතහොත් නම් EQ("sethook") {
      පූර්ණ_සංඛ්‍යා mask = getnum;
      පූර්ණ_සංඛ්‍යා count = getnum;
      const var_na *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    නැතහොත් නම් EQ("setmetatable") {
      පූර්ණ_සංඛ්‍යා idx = getindex;
      lua_setmetatable(L1, idx);
    }
    නැතහොත් නම් EQ("settable") {
      lua_settable(L1, getindex);
    }
    නැතහොත් නම් EQ("settop") {
      lua_settop(L1, getnum);
    }
    නැතහොත් නම් EQ("testudata") {
      පූර්ණ_සංඛ්‍යා i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    නැතහොත් නම් EQ("error") {
      lua_error(L1);
    }
    නැතහොත් නම් EQ("abort") {
      abort();
    }
    නැතහොත් නම් EQ("throw") {
#නම් defined(__cplusplus)
static sa_nracanaa X { පූර්ණ_සංඛ්‍යා x; } x;
      throw x;
#නැතහොත්
      luaL_error(L1, "C++");
#endif
      bha_nga;
    }
    නැතහොත් නම් EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    නැතහොත් නම් EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    නැතහොත් නම් EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    නැතහොත් නම් EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    නැතහොත් නම් EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    නැතහොත් නම් EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    නැතහොත් නම් EQ("tostring") {
      const var_na *s = lua_tostring(L1, getindex);
      const var_na *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    නැතහොත් නම් EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    නැතහොත් නම් EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    නැතහොත් නම් EQ("xmove") {
      පූර්ණ_සංඛ්‍යා f = getindex;
      පූර්ණ_සංඛ්‍යා t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      පූර්ණ_සංඛ්‍යා n = getnum;
      නම් (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    නැතහොත් නම් EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    නැතහොත් නම් EQ("yield") {
      ආපසු lua_yield(L1, getnum);
    }
    නැතහොත් නම් EQ("yieldk") {
      පූර්ණ_සංඛ්‍යා nres = getnum;
      පූර්ණ_සංඛ්‍යා i = getindex;
      ආපසු lua_yieldk(L1, nres, i, Cfunck);
    }
    නැතහොත් නම් EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    නැතහොත් නම් EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    නැතහොත් නම් EQ("argerror") {
      පූර්ණ_සංඛ්‍යා arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    නැතහොත් luaL_error(L, "unknown instruction %s", buff);
  }
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා testC (lua_State *L) {
  lua_State *L1;
  const var_na *pc;
  නම් (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  නැතහොත් නම් (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  නැතහොත් {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ආපසු runC(L, L1, pc);
}


static පූර්ණ_සංඛ්‍යා Cfunc (lua_State *L) {
  ආපසු runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static පූර්ණ_සංඛ්‍යා Cfunck (lua_State *L, පූර්ණ_සංඛ්‍යා status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ආපසු runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static පූර්ණ_සංඛ්‍යා makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ආපසු 1;
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
static void sethookaux (lua_State *L, පූර්ණ_සංඛ්‍යා mask, පූර්ණ_සංඛ්‍යා count, const var_na *scpt) {
  නම් (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    ආපසු;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  නම් (!lua_istable(L, -1)) {  /* no hook table? */
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


static පූර්ණ_සංඛ්‍යා sethook (lua_State *L) {
  නම් (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  නැතහොත් {
    const var_na *scpt = luaL_checkstring(L, 1);
    const var_na *smask = luaL_checkstring(L, 2);
    පූර්ණ_සංඛ්‍යා count = cast_int(luaL_optinteger(L, 3, 0));
    පූර්ණ_සංඛ්‍යා mask = 0;
    නම් (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    නම් (strchr(smask, 'r')) mask |= LUA_MASKRET;
    නම් (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    නම් (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා coresume (lua_State *L) {
  පූර්ණ_සංඛ්‍යා status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  නම් (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ආපසු 2;  /* return false + error message */
  }
  නැතහොත් {
    lua_pushboolean(L, 1);
    ආපසු 1;
  }
}

#නම් !defined(LUA_USE_POSIX)

#define nonblock	NULL

#නැතහොත්

#include <unistd.h>
#include <fcntl.h>

static පූර්ණ_සංඛ්‍යා nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  පූර්ණ_සංඛ්‍යා fd = fileno(f);
  පූර්ණ_සංඛ්‍යා flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ආපසු 0;
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


පූර්ණ_සංඛ්‍යා luaB_opentests (lua_State *L) {
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
  ආපසු 1;
}

#endif

