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
#nếu defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static số_nguyên runC (lua_State *L, lua_State *L1, const char *pc);


static void setnameval (lua_State *L, const char *name, số_nguyên val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const char *fmt, const char *s1, const char *s2) {
  fprintf(stderr, fmt, s1);
  nếu (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static số_nguyên tpanic (lua_State *L) {
  const char *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  trả_về (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const char *msg, số_nguyên tocont) {
  lua_State *L = cast(lua_State *, ud);
  static char buff[200] = "";  /* should be enough for tests... */
  static số_nguyên onoff = 0;
  static số_nguyên mode = 0;  /* start in normal mode */
  static số_nguyên lasttocont = 0;
  nếu (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    nếu (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    nếu (strcmp(msg, "@off") == 0)
      onoff = 0;
    không_thì nếu (strcmp(msg, "@on") == 0)
      onoff = 1;
    không_thì nếu (strcmp(msg, "@normal") == 0)
      mode = 0;
    không_thì nếu (strcmp(msg, "@allow") == 0)
      mode = 1;
    không_thì nếu (strcmp(msg, "@store") == 0)
      mode = 2;
    không_thì
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    trả_về;
  }
  lasttocont = tocont;
  nếu (strlen(msg) >= sizeof(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  nếu (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    nếu (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    không_thì {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      case 0: {  /* normal */
        nếu (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      case 1: {  /* allow */
        nếu (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        break;
      }
      case 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        break;
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

typedef union memHeader {
  LUAI_MAXALIGN;
  struct {
    size_t size;
    số_nguyên type;
  } d;
} memHeader;


#nếu !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#không_thì

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  nếu (block) {
    size_t size = block->d.size;
    số_nguyên i;
    cho (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, sizeof(memHeader) + size + MARKSIZE);  /* erase block */
    free(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


void *debug_realloc (void *ud, void *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  số_nguyên type;
  nếu (mc->memlimit == 0) {  /* first time? */
    char *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  nếu (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  không_thì {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  nếu (size == 0) {
    freeblock(mc, block);
    trả_về NULL;
  }
  nếu (mc->failnext) {
    mc->failnext = 0;
    trả_về NULL;  /* fake a single memory allocation error */
  }
  nếu (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    nếu (mc->countlimit == 0)
      trả_về NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  nếu (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    trả_về NULL;  /* fake a memory allocation error */
  không_thì {
    memHeader *newblock;
    số_nguyên i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = sizeof(memHeader) + size + MARKSIZE;
    nếu (realsize < size) trả_về NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    nếu (newblock == NULL)
      trả_về NULL;  /* really out of memory? */
    nếu (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    cho (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    nếu (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    trả_về newblock + 1;
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
static số_nguyên testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  nếu (isdead(g,t)) trả_về 0;
  nếu (issweepphase(g))
    trả_về 1;  /* no invariants */
  không_thì nếu (g->gckind != KGC_GENMINOR)
    trả_về !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  không_thì {  /* generational mode */
    nếu ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      trả_về 0;
    nếu ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      trả_về 0;
    trả_về 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  nếu (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


void lua_printobj (lua_State *L, struct GCObject *o) {
  printobj(G(L), o);
}


void lua_printvalue (TValue *v) {
  switch (ttypetag(v)) {
    case LUA_VNUMINT: case LUA_VNUMFLT: {
      char buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      break;
    }
    case LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); break;
    case LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); break;
    case LUA_VFALSE:
      printf("%s", "false"); break;
    case LUA_VTRUE:
      printf("%s", "true"); break;
    case LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); break;
    case LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); break;
    case LUA_VNIL:
      printf("nil"); break;
    case LUA_VLCF:
      printf("light C function: %p", fvalue(v)); break;
    case LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); break;
    case LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); break;
    case LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); break;
    case LUA_VTABLE:
      printf("table: %p", hvalue(v)); break;
    default:
      lua_assert(0);
  }
}


static số_nguyên testobjref (global_State *g, GCObject *f, GCObject *t) {
  số_nguyên r1 = testobjref1(g, f, t);
  nếu (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  trả_về r1;
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
#define checkobjrefN(g,f,t)	{ nếu (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned số_nguyên i;
  unsigned số_nguyên asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  cho (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  cho (n = gnode(h, 0); n < limit; n++) {
    nếu (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  số_nguyên i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  cho (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  số_nguyên i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  cho (i=0; i<f->sizek; i++) {
    nếu (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  cho (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  cho (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  cho (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  số_nguyên i;
  cho (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  số_nguyên i;
  checkobjrefN(g, clgc, cl->p);
  cho (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    nếu (uv) {
      checkobjrefN(g, clgc, uv);
      nếu (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static số_nguyên lua_checkpc (CallInfo *ci) {
  nếu (!isLua(ci)) trả_về 1;
  không_thì {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    trả_về p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  nếu (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    trả_về;
  }
  cho (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  cho (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  cho (o = L1->stack.p; o < L1->stack_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


static void checkrefs (global_State *g, GCObject *o) {
  switch (o->tt) {
    case LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      break;
    }
    case LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      break;
    }
    case LUA_VTABLE: {
      checktable(g, gco2t(o));
      break;
    }
    case LUA_VTHREAD: {
      check_stack(g, gco2th(o));
      break;
    }
    case LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      break;
    }
    case LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      break;
    }
    case LUA_VPROTO: {
      checkproto(g, gco2p(o));
      break;
    }
    case LUA_VSHRSTR:
    case LUA_VLNGSTR: {
      assert(!isgray(o));  /* strings are never gray */
      break;
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
static void checkobject (global_State *g, GCObject *o, số_nguyên maybedead,
                         số_nguyên listage) {
  nếu (isdead(g, o))
    assert(maybedead);
  không_thì {
    assert(g->gcstate != GCSpause || iswhite(o));
    nếu (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      nếu (isold(o)) {
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
  số_nguyên total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  trong_khi (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    nếu (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    switch (o->tt) {
      case LUA_VTABLE: o = gco2t(o)->gclist; break;
      case LUA_VLCL: o = gco2lcl(o)->gclist; break;
      case LUA_VCCL: o = gco2ccl(o)->gclist; break;
      case LUA_VTHREAD: o = gco2th(o)->gclist; break;
      case LUA_VPROTO: o = gco2p(o)->gclist; break;
      case LUA_VUSERDATA:
        assert(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        break;
      default: assert(0);  /* other objects cannot be in a gray list */
    }
  }
  trả_về total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  nếu (!keepinvariant(g)) trả_về total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  trả_về total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  nếu (!keepinvariant(g))
    trả_về;  /* gray lists not being kept in these phases */
  nếu (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    trả_về;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  nếu (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, số_nguyên maybedead, số_nguyên tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  cho (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  cho (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  cho (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  cho (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  trả_về total;
}


số_nguyên lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  số_nguyên maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  nếu (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  cho (o = g->fixedgc; o != NULL; o = o->next) {
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
  cho (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  nếu (keepinvariant(g))
    assert(totalin == totalshould);
  trả_về 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static char *buildop (Proto *p, số_nguyên pc, char *buff) {
  char *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const char *name = opnames[o];
  số_nguyên line = luaG_getfuncline(p, pc);
  số_nguyên lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  nếu (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  không_thì
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", line, pc);
  switch (getOpMode(o)) {
    case iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      break;
    case ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      break;
    case iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      break;
    case iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      break;
    case iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      break;
    case isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      break;
  }
  trả_về obuff;
}


#nếu 0
void luaI_printcode (Proto *pt, số_nguyên size) {
  số_nguyên pc;
  cho (pc=0; pc<size; pc++) {
    char buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, số_nguyên pc) {
  char buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static số_nguyên listcode (lua_State *L) {
  số_nguyên pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  cho (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  trả_về 1;
}


static số_nguyên printcode (lua_State *L) {
  số_nguyên pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  cho (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  trả_về 0;
}


static số_nguyên listk (lua_State *L) {
  Proto *p;
  số_nguyên i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  cho (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  trả_về 1;
}


static số_nguyên listabslineinfo (lua_State *L) {
  Proto *p;
  số_nguyên i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  cho (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  trả_về 1;
}


static số_nguyên listlocals (lua_State *L) {
  Proto *p;
  số_nguyên pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  số_nguyên i = 0;
  const char *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  trong_khi ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  trả_về i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  số_nguyên i;
  số_nguyên n = lua_gettop(L);
  printf("stack: >>\n");
  cho (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


số_nguyên lua_printallstack (lua_State *L) {
  StkId p;
  số_nguyên i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  cho (p = L->stack.p; p < L->top.p; p++) {
    nếu (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      nếu (ci == L->ci)
        ci = NULL;  /* printed last frame */
      không_thì
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  trả_về 0;
}


static số_nguyên get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  trả_về 1;
}


static số_nguyên get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", sizeof(lua_State));
  setnameval(L, "global state", sizeof(global_State));
  setnameval(L, "TValue", sizeof(TValue));
  setnameval(L, "Node", sizeof(Node));
  setnameval(L, "stack Value", sizeof(StackValue));
  trả_về 1;
}


static số_nguyên mem_query (lua_State *L) {
  nếu (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    trả_về 3;
  }
  không_thì nếu (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    nếu (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    trả_về 0;
  }
  không_thì {
    const char *t = luaL_checkstring(L, 1);
    số_nguyên i;
    cho (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      nếu (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        trả_về 1;
      }
    }
    trả_về luaL_error(L, "unknown type '%s'", t);
  }
}


static số_nguyên alloc_count (lua_State *L) {
  nếu (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  không_thì
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  trả_về 0;
}


static số_nguyên alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  trả_về 0;
}


static số_nguyên settrick (lua_State *L) {
  nếu (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  không_thì
    l_Trick = gcvalue(obj_at(L, 1));
  trả_về 0;
}


static số_nguyên gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  nếu (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  không_thì {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  trả_về 1;
}


static số_nguyên gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  nếu (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  không_thì {
    static const char *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  trả_về 1;
}


static số_nguyên gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  nếu (!iscollectable(o))
    printf("no collectable\n");
  không_thì {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  trả_về 0;
}


static const char *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static số_nguyên gc_state (lua_State *L) {
  static const số_nguyên states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  số_nguyên option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  nếu (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    trả_về 1;
  }
  không_thì {
    nếu (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    nếu (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    trả_về 0;
  }
}


static số_nguyên tracinggc = 0;
void luai_tracegctest (lua_State *L, số_nguyên first) {
  nếu (!tracinggc) trả_về;
  không_thì {
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


static số_nguyên tracegc (lua_State *L) {
  nếu (lua_isnil(L, 1))
    tracinggc = 0;
  không_thì {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  trả_về 0;
}


static số_nguyên hash_query (lua_State *L) {
  nếu (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    nếu (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  không_thì {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  trả_về 1;
}


static số_nguyên stacklevel (lua_State *L) {
  số_nguyên a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  trả_về 5;
}


static số_nguyên resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  trong_khi (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  trả_về 0;
}


static số_nguyên reallocstack (lua_State *L) {
  số_nguyên n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  trả_về 0;
}


static số_nguyên table_query (lua_State *L) {
  const Table *t;
  số_nguyên i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned số_nguyên asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  nếu (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    trả_về 3;
  }
  không_thì nếu (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    nếu (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    không_thì
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  không_thì nếu (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    nếu (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    không_thì
      lua_pushliteral(L, "<undef>");
    nếu (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    không_thì
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  trả_về 3;
}


static số_nguyên gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  trả_về 6;
}


static số_nguyên test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  trả_về 1;
}


static số_nguyên test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  trả_về 1;
}


static số_nguyên string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  số_nguyên s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  nếu (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    trả_về 2;
  }
  không_thì nếu (s < tb->size) {
    TString *ts;
    số_nguyên n = 0;
    cho (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    trả_về n;
  }
  không_thì trả_về 0;
}


static số_nguyên getreftable (lua_State *L) {
  nếu (lua_istable(L, 2))  /* is there a table as second argument? */
    trả_về 2;  /* use it as the table */
  không_thì
    trả_về LUA_REGISTRYINDEX;  /* default is to use the register */
}


static số_nguyên tref (lua_State *L) {
  số_nguyên t = getreftable(L);
  số_nguyên level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  trả_về 1;
}


static số_nguyên getref (lua_State *L) {
  số_nguyên t = getreftable(L);
  số_nguyên level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  trả_về 1;
}

static số_nguyên unref (lua_State *L) {
  số_nguyên t = getreftable(L);
  số_nguyên level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  trả_về 0;
}


static số_nguyên upvalue (lua_State *L) {
  số_nguyên n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  nếu (lua_isnone(L, 3)) {
    const char *name = lua_getupvalue(L, 1, n);
    nếu (name == NULL) trả_về 0;
    lua_pushstring(L, name);
    trả_về 2;
  }
  không_thì {
    const char *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    trả_về 1;
  }
}


static số_nguyên newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  số_nguyên nuv = cast_int(luaL_optinteger(L, 2, 0));
  char *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  trong_khi (size--) *p++ = '\0';
  trả_về 1;
}


static số_nguyên pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  trả_về 1;
}


static số_nguyên udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  trả_về 1;
}


static số_nguyên doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  số_nguyên status = luaL_loadbufferx(L1, s, l, s, "t");
  nếu (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  trả_về 1;
}


static số_nguyên s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const double *, luaL_checkstring(L, 1))));
  trả_về 1;
}


static số_nguyên d2s (lua_State *L) {
  double d = cast(double, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), sizeof(d));
  trả_về 1;
}


static số_nguyên num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  trả_về 1;
}


static số_nguyên makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  trả_về 1;
}


static số_nguyên newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  nếu (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  không_thì
    lua_pushnil(L);
  trả_về 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  trả_về L1;
}


static số_nguyên loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  số_nguyên load = cast_int(luaL_checkinteger(L, 2));
  số_nguyên preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  trả_về 0;
}

static số_nguyên closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  trả_về 0;
}

static số_nguyên doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const char *code = luaL_checklstring(L, 2, &lcode);
  số_nguyên status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  nếu (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  nếu (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    trả_về 3;
  }
  không_thì {
    số_nguyên i = 0;
    trong_khi (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    trả_về i-1;
  }
}


static số_nguyên log2_aux (lua_State *L) {
  unsigned số_nguyên x = (unsigned số_nguyên)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  trả_về 1;
}


struct Aux { jmp_buf jb; const char *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static số_nguyên panicback (lua_State *L) {
  struct Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (struct Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  trả_về 1;  /* to avoid warnings */
}

static số_nguyên checkpanic (lua_State *L) {
  struct Aux b;
  void *ud;
  lua_State *L1;
  const char *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  nếu (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    trả_về 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  nếu (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  không_thì {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  trả_về 1;
}


static số_nguyên externKstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  trả_về 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static số_nguyên externstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  char *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  nếu (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * sizeof(char));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  trả_về 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, số_nguyên mask, số_nguyên count, const char *code);

static const char *const delimits = " \t\n,;";

static void skip (const char **pc) {
  cho (;;) {
    nếu (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    không_thì nếu (**pc == '#') {  /* comment? */
      trong_khi (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    không_thì break;
  }
}

static số_nguyên getnum_aux (lua_State *L, lua_State *L1, const char **pc) {
  số_nguyên res = 0;
  số_nguyên sig = 1;
  skip(pc);
  nếu (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    trả_về res;
  }
  không_thì nếu (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    trả_về res;
  }
  không_thì nếu (**pc == '!') {
    (*pc)++;
    nếu (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    không_thì nếu (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    không_thì lua_assert(0);
    (*pc)++;
    trả_về res;
  }
  không_thì nếu (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  nếu (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  trong_khi (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  trả_về sig*res;
}

static const char *getstring_aux (lua_State *L, char *buff, const char **pc) {
  số_nguyên i = 0;
  skip(pc);
  nếu (**pc == '"' || **pc == '\'') {  /* quoted string? */
    số_nguyên quote = *(*pc)++;
    trong_khi (**pc != quote) {
      nếu (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  không_thì {
    trong_khi (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  trả_về buff;
}


static số_nguyên getindex_aux (lua_State *L, lua_State *L1, const char **pc) {
  skip(pc);
  switch (*(*pc)++) {
    case 'R': trả_về LUA_REGISTRYINDEX;
    case 'U': trả_về lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      số_nguyên n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      nếu (n == 0) trả_về 0;
      không_thì trả_về lua_absindex(L1, n);
    }
  }
}


static const char *const statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
static void regcodes (lua_State *L) {
  unsigned số_nguyên i;
  cho (i = 0; i < sizeof(statcodes) / sizeof(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static số_nguyên testC (lua_State *L);
static số_nguyên Cfunck (lua_State *L, số_nguyên status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const char ops[] = "+-*%^/\\&|~<>_!";

static số_nguyên runC (lua_State *L, lua_State *L1, const char *pc) {
  char buff[300];
  số_nguyên status = 0;
  nếu (pc == NULL) trả_về luaL_error(L, "attempt to runC null script");
  cho (;;) {
    const char *inst = getstring;
    nếu EQ("") trả_về 0;
    không_thì nếu EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    không_thì nếu EQ("append") {
      số_nguyên t = getindex;
      số_nguyên i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    không_thì nếu EQ("arith") {
      số_nguyên op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    không_thì nếu EQ("call") {
      số_nguyên narg = getnum;
      số_nguyên nres = getnum;
      lua_call(L1, narg, nres);
    }
    không_thì nếu EQ("callk") {
      số_nguyên narg = getnum;
      số_nguyên nres = getnum;
      số_nguyên i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    không_thì nếu EQ("checkstack") {
      số_nguyên sz = getnum;
      const char *msg = getstring;
      nếu (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    không_thì nếu EQ("rawcheckstack") {
      số_nguyên sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    không_thì nếu EQ("compare") {
      const char *opt = getstring;  /* EQ, LT, or LE */
      số_nguyên op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      số_nguyên a = getindex;
      số_nguyên b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    không_thì nếu EQ("concat") {
      lua_concat(L1, getnum);
    }
    không_thì nếu EQ("copy") {
      số_nguyên f = getindex;
      lua_copy(L1, f, getindex);
    }
    không_thì nếu EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    không_thì nếu EQ("getfield") {
      số_nguyên t = getindex;
      số_nguyên tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    không_thì nếu EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    không_thì nếu EQ("getmetatable") {
      nếu (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    không_thì nếu EQ("gettable") {
      số_nguyên tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    không_thì nếu EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    không_thì nếu EQ("gsub") {
      số_nguyên a = getnum; số_nguyên b = getnum; số_nguyên c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    không_thì nếu EQ("insert") {
      lua_insert(L1, getnum);
    }
    không_thì nếu EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    không_thì nếu EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    không_thì nếu EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    không_thì nếu EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    không_thì nếu EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    không_thì nếu EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    không_thì nếu EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    không_thì nếu EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    không_thì nếu EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    không_thì nếu EQ("len") {
      lua_len(L1, getindex);
    }
    không_thì nếu EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    không_thì nếu EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    không_thì nếu EQ("loadstring") {
      size_t slen;
      const char *s = luaL_checklstring(L1, getnum, &slen);
      const char *name = getstring;
      const char *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    không_thì nếu EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    không_thì nếu EQ("newtable") {
      lua_newtable(L1);
    }
    không_thì nếu EQ("newthread") {
      lua_newthread(L1);
    }
    không_thì nếu EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    không_thì nếu EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    không_thì nếu EQ("next") {
      lua_next(L1, -2);
    }
    không_thì nếu EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    không_thì nếu EQ("pcall") {
      số_nguyên narg = getnum;
      số_nguyên nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    không_thì nếu EQ("pcallk") {
      số_nguyên narg = getnum;
      số_nguyên nres = getnum;
      số_nguyên i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    không_thì nếu EQ("pop") {
      lua_pop(L1, getnum);
    }
    không_thì nếu EQ("printstack") {
      số_nguyên n = getnum;
      nếu (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      không_thì lua_printstack(L1);
    }
    không_thì nếu EQ("print") {
      const char *msg = getstring;
      printf("%s\n", msg);
    }
    không_thì nếu EQ("warningC") {
      const char *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    không_thì nếu EQ("warning") {
      const char *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    không_thì nếu EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    không_thì nếu EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    không_thì nếu EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    không_thì nếu EQ("pushnil") {
      lua_pushnil(L1);
    }
    không_thì nếu EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    không_thì nếu EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    không_thì nếu EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    không_thì nếu EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    không_thì nếu EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    không_thì nếu EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (số_nguyên)lua_tointeger(L, -1));
    }
    không_thì nếu EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    không_thì nếu EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    không_thì nếu EQ("rawget") {
      số_nguyên t = getindex;
      lua_rawget(L1, t);
    }
    không_thì nếu EQ("rawgeti") {
      số_nguyên t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    không_thì nếu EQ("rawgetp") {
      số_nguyên t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    không_thì nếu EQ("rawset") {
      số_nguyên t = getindex;
      lua_rawset(L1, t);
    }
    không_thì nếu EQ("rawseti") {
      số_nguyên t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    không_thì nếu EQ("rawsetp") {
      số_nguyên t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    không_thì nếu EQ("remove") {
      lua_remove(L1, getnum);
    }
    không_thì nếu EQ("replace") {
      lua_replace(L1, getindex);
    }
    không_thì nếu EQ("resume") {
      số_nguyên i = getindex;
      số_nguyên nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    không_thì nếu EQ("traceback") {
      const char *msg = getstring;
      số_nguyên level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    không_thì nếu EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    không_thì nếu EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    không_thì nếu EQ("return") {
      số_nguyên n = getnum;
      nếu (L1 != L) {
        số_nguyên i;
        cho (i = 0; i < n; i++) {
          số_nguyên idx = -(n - i);
          switch (lua_type(L1, idx)) {
            case LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              break;
            default:
              lua_pushstring(L, lua_tostring(L1, idx));
              break;
          }
        }
      }
      trả_về n;
    }
    không_thì nếu EQ("rotate") {
      số_nguyên i = getindex;
      lua_rotate(L1, i, getnum);
    }
    không_thì nếu EQ("setfield") {
      số_nguyên t = getindex;
      const char *s = getstring;
      lua_setfield(L1, t, s);
    }
    không_thì nếu EQ("seti") {
      số_nguyên t = getindex;
      lua_seti(L1, t, getnum);
    }
    không_thì nếu EQ("setglobal") {
      const char *s = getstring;
      lua_setglobal(L1, s);
    }
    không_thì nếu EQ("sethook") {
      số_nguyên mask = getnum;
      số_nguyên count = getnum;
      const char *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    không_thì nếu EQ("setmetatable") {
      số_nguyên idx = getindex;
      lua_setmetatable(L1, idx);
    }
    không_thì nếu EQ("settable") {
      lua_settable(L1, getindex);
    }
    không_thì nếu EQ("settop") {
      lua_settop(L1, getnum);
    }
    không_thì nếu EQ("testudata") {
      số_nguyên i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    không_thì nếu EQ("error") {
      lua_error(L1);
    }
    không_thì nếu EQ("abort") {
      abort();
    }
    không_thì nếu EQ("throw") {
#nếu defined(__cplusplus)
static struct X { số_nguyên x; } x;
      throw x;
#không_thì
      luaL_error(L1, "C++");
#endif
      break;
    }
    không_thì nếu EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    không_thì nếu EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    không_thì nếu EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    không_thì nếu EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    không_thì nếu EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    không_thì nếu EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    không_thì nếu EQ("tostring") {
      const char *s = lua_tostring(L1, getindex);
      const char *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    không_thì nếu EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    không_thì nếu EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    không_thì nếu EQ("xmove") {
      số_nguyên f = getindex;
      số_nguyên t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      số_nguyên n = getnum;
      nếu (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    không_thì nếu EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    không_thì nếu EQ("yield") {
      trả_về lua_yield(L1, getnum);
    }
    không_thì nếu EQ("yieldk") {
      số_nguyên nres = getnum;
      số_nguyên i = getindex;
      trả_về lua_yieldk(L1, nres, i, Cfunck);
    }
    không_thì nếu EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    không_thì nếu EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    không_thì nếu EQ("argerror") {
      số_nguyên arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    không_thì luaL_error(L, "unknown instruction %s", buff);
  }
  trả_về 0;
}


static số_nguyên testC (lua_State *L) {
  lua_State *L1;
  const char *pc;
  nếu (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  không_thì nếu (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  không_thì {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  trả_về runC(L, L1, pc);
}


static số_nguyên Cfunc (lua_State *L) {
  trả_về runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static số_nguyên Cfunck (lua_State *L, số_nguyên status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  trả_về runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static số_nguyên makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  trả_về 1;
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
  const char *scpt;
  const char *const events [] = {"call", "ret", "line", "count", "tailcall"};
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
static void sethookaux (lua_State *L, số_nguyên mask, số_nguyên count, const char *scpt) {
  nếu (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    trả_về;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  nếu (!lua_istable(L, -1)) {  /* no hook table? */
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


static số_nguyên sethook (lua_State *L) {
  nếu (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  không_thì {
    const char *scpt = luaL_checkstring(L, 1);
    const char *smask = luaL_checkstring(L, 2);
    số_nguyên count = cast_int(luaL_optinteger(L, 3, 0));
    số_nguyên mask = 0;
    nếu (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    nếu (strchr(smask, 'r')) mask |= LUA_MASKRET;
    nếu (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    nếu (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  trả_về 0;
}


static số_nguyên coresume (lua_State *L) {
  số_nguyên status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  nếu (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    trả_về 2;  /* return false + error message */
  }
  không_thì {
    lua_pushboolean(L, 1);
    trả_về 1;
  }
}

#nếu !defined(LUA_USE_POSIX)

#define nonblock	NULL

#không_thì

#include <unistd.h>
#include <fcntl.h>

static số_nguyên nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  số_nguyên fd = fileno(f);
  số_nguyên flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  trả_về 0;
}
#endif

/* }====================================================== */



static const struct luaL_Reg tests_funcs[] = {
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


số_nguyên luaB_opentests (lua_State *L) {
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
  trả_về 1;
}

#endif

