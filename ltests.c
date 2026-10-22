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
#만약 defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static 정수 runC (lua_State *L, lua_State *L1, const char *pc);


static void setnameval (lua_State *L, const char *name, 정수 val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const char *fmt, const char *s1, const char *s2) {
  fprintf(stderr, fmt, s1);
  만약 (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static 정수 tpanic (lua_State *L) {
  const char *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  반환 (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const char *msg, 정수 tocont) {
  lua_State *L = cast(lua_State *, ud);
  static char buff[200] = "";  /* should be enough for tests... */
  static 정수 onoff = 0;
  static 정수 mode = 0;  /* start in normal mode */
  static 정수 lasttocont = 0;
  만약 (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    만약 (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    만약 (strcmp(msg, "@off") == 0)
      onoff = 0;
    아니면 만약 (strcmp(msg, "@on") == 0)
      onoff = 1;
    아니면 만약 (strcmp(msg, "@normal") == 0)
      mode = 0;
    아니면 만약 (strcmp(msg, "@allow") == 0)
      mode = 1;
    아니면 만약 (strcmp(msg, "@store") == 0)
      mode = 2;
    아니면
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    반환;
  }
  lasttocont = tocont;
  만약 (strlen(msg) >= sizeof(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  만약 (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    만약 (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    아니면 {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      case 0: {  /* normal */
        만약 (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      case 1: {  /* allow */
        만약 (onoff)
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
    정수 type;
  } d;
} memHeader;


#만약 !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#아니면

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  만약 (block) {
    size_t size = block->d.size;
    정수 i;
    위해 (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  정수 type;
  만약 (mc->memlimit == 0) {  /* first time? */
    char *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  만약 (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  아니면 {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  만약 (size == 0) {
    freeblock(mc, block);
    반환 NULL;
  }
  만약 (mc->failnext) {
    mc->failnext = 0;
    반환 NULL;  /* fake a single memory allocation error */
  }
  만약 (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    만약 (mc->countlimit == 0)
      반환 NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  만약 (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    반환 NULL;  /* fake a memory allocation error */
  아니면 {
    memHeader *newblock;
    정수 i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = sizeof(memHeader) + size + MARKSIZE;
    만약 (realsize < size) 반환 NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    만약 (newblock == NULL)
      반환 NULL;  /* really out of memory? */
    만약 (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    위해 (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    만약 (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    반환 newblock + 1;
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
static 정수 testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  만약 (isdead(g,t)) 반환 0;
  만약 (issweepphase(g))
    반환 1;  /* no invariants */
  아니면 만약 (g->gckind != KGC_GENMINOR)
    반환 !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  아니면 {  /* generational mode */
    만약 ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      반환 0;
    만약 ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      반환 0;
    반환 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  만약 (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static 정수 testobjref (global_State *g, GCObject *f, GCObject *t) {
  정수 r1 = testobjref1(g, f, t);
  만약 (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  반환 r1;
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
#define checkobjrefN(g,f,t)	{ 만약 (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned 정수 i;
  unsigned 정수 asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  위해 (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  위해 (n = gnode(h, 0); n < limit; n++) {
    만약 (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  정수 i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  위해 (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  정수 i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  위해 (i=0; i<f->sizek; i++) {
    만약 (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  위해 (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  위해 (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  위해 (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  정수 i;
  위해 (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  정수 i;
  checkobjrefN(g, clgc, cl->p);
  위해 (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    만약 (uv) {
      checkobjrefN(g, clgc, uv);
      만약 (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static 정수 lua_checkpc (CallInfo *ci) {
  만약 (!isLua(ci)) 반환 1;
  아니면 {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    반환 p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  만약 (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    반환;
  }
  위해 (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  위해 (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  위해 (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, 정수 maybedead,
                         정수 listage) {
  만약 (isdead(g, o))
    assert(maybedead);
  아니면 {
    assert(g->gcstate != GCSpause || iswhite(o));
    만약 (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      만약 (isold(o)) {
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
  정수 total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  동안 (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    만약 (keepinvariant(g))
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
  반환 total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  만약 (!keepinvariant(g)) 반환 total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  반환 total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  만약 (!keepinvariant(g))
    반환;  /* gray lists not being kept in these phases */
  만약 (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    반환;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  만약 (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, 정수 maybedead, 정수 tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  위해 (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  위해 (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  위해 (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  위해 (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  반환 total;
}


정수 lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  정수 maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  만약 (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  위해 (o = g->fixedgc; o != NULL; o = o->next) {
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
  위해 (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  만약 (keepinvariant(g))
    assert(totalin == totalshould);
  반환 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static char *buildop (Proto *p, 정수 pc, char *buff) {
  char *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const char *name = opnames[o];
  정수 line = luaG_getfuncline(p, pc);
  정수 lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  만약 (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  아니면
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
  반환 obuff;
}


#만약 0
void luaI_printcode (Proto *pt, 정수 size) {
  정수 pc;
  위해 (pc=0; pc<size; pc++) {
    char buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, 정수 pc) {
  char buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static 정수 listcode (lua_State *L) {
  정수 pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  위해 (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  반환 1;
}


static 정수 printcode (lua_State *L) {
  정수 pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  위해 (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  반환 0;
}


static 정수 listk (lua_State *L) {
  Proto *p;
  정수 i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  위해 (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  반환 1;
}


static 정수 listabslineinfo (lua_State *L) {
  Proto *p;
  정수 i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  위해 (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  반환 1;
}


static 정수 listlocals (lua_State *L) {
  Proto *p;
  정수 pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  정수 i = 0;
  const char *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  동안 ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  반환 i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  정수 i;
  정수 n = lua_gettop(L);
  printf("stack: >>\n");
  위해 (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


정수 lua_printallstack (lua_State *L) {
  StkId p;
  정수 i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  위해 (p = L->stack.p; p < L->top.p; p++) {
    만약 (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      만약 (ci == L->ci)
        ci = NULL;  /* printed last frame */
      아니면
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  반환 0;
}


static 정수 get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  반환 1;
}


static 정수 get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", sizeof(lua_State));
  setnameval(L, "global state", sizeof(global_State));
  setnameval(L, "TValue", sizeof(TValue));
  setnameval(L, "Node", sizeof(Node));
  setnameval(L, "stack Value", sizeof(StackValue));
  반환 1;
}


static 정수 mem_query (lua_State *L) {
  만약 (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    반환 3;
  }
  아니면 만약 (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    만약 (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    반환 0;
  }
  아니면 {
    const char *t = luaL_checkstring(L, 1);
    정수 i;
    위해 (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      만약 (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        반환 1;
      }
    }
    반환 luaL_error(L, "unknown type '%s'", t);
  }
}


static 정수 alloc_count (lua_State *L) {
  만약 (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  아니면
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  반환 0;
}


static 정수 alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  반환 0;
}


static 정수 settrick (lua_State *L) {
  만약 (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  아니면
    l_Trick = gcvalue(obj_at(L, 1));
  반환 0;
}


static 정수 gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  만약 (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  아니면 {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  반환 1;
}


static 정수 gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  만약 (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  아니면 {
    static const char *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  반환 1;
}


static 정수 gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  만약 (!iscollectable(o))
    printf("no collectable\n");
  아니면 {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  반환 0;
}


static const char *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static 정수 gc_state (lua_State *L) {
  static const 정수 states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  정수 option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  만약 (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    반환 1;
  }
  아니면 {
    만약 (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    만약 (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    반환 0;
  }
}


static 정수 tracinggc = 0;
void luai_tracegctest (lua_State *L, 정수 first) {
  만약 (!tracinggc) 반환;
  아니면 {
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


static 정수 tracegc (lua_State *L) {
  만약 (lua_isnil(L, 1))
    tracinggc = 0;
  아니면 {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  반환 0;
}


static 정수 hash_query (lua_State *L) {
  만약 (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    만약 (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  아니면 {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  반환 1;
}


static 정수 stacklevel (lua_State *L) {
  정수 a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  반환 5;
}


static 정수 resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  동안 (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  반환 0;
}


static 정수 reallocstack (lua_State *L) {
  정수 n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  반환 0;
}


static 정수 table_query (lua_State *L) {
  const Table *t;
  정수 i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned 정수 asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  만약 (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    반환 3;
  }
  아니면 만약 (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    만약 (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    아니면
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  아니면 만약 (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    만약 (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    아니면
      lua_pushliteral(L, "<undef>");
    만약 (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    아니면
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  반환 3;
}


static 정수 gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  반환 6;
}


static 정수 test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  반환 1;
}


static 정수 test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  반환 1;
}


static 정수 string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  정수 s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  만약 (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    반환 2;
  }
  아니면 만약 (s < tb->size) {
    TString *ts;
    정수 n = 0;
    위해 (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    반환 n;
  }
  아니면 반환 0;
}


static 정수 getreftable (lua_State *L) {
  만약 (lua_istable(L, 2))  /* is there a table as second argument? */
    반환 2;  /* use it as the table */
  아니면
    반환 LUA_REGISTRYINDEX;  /* default is to use the register */
}


static 정수 tref (lua_State *L) {
  정수 t = getreftable(L);
  정수 level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  반환 1;
}


static 정수 getref (lua_State *L) {
  정수 t = getreftable(L);
  정수 level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  반환 1;
}

static 정수 unref (lua_State *L) {
  정수 t = getreftable(L);
  정수 level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  반환 0;
}


static 정수 upvalue (lua_State *L) {
  정수 n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  만약 (lua_isnone(L, 3)) {
    const char *name = lua_getupvalue(L, 1, n);
    만약 (name == NULL) 반환 0;
    lua_pushstring(L, name);
    반환 2;
  }
  아니면 {
    const char *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    반환 1;
  }
}


static 정수 newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  정수 nuv = cast_int(luaL_optinteger(L, 2, 0));
  char *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  동안 (size--) *p++ = '\0';
  반환 1;
}


static 정수 pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  반환 1;
}


static 정수 udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  반환 1;
}


static 정수 doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  정수 status = luaL_loadbufferx(L1, s, l, s, "t");
  만약 (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  반환 1;
}


static 정수 s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const double *, luaL_checkstring(L, 1))));
  반환 1;
}


static 정수 d2s (lua_State *L) {
  double d = cast(double, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), sizeof(d));
  반환 1;
}


static 정수 num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  반환 1;
}


static 정수 makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  반환 1;
}


static 정수 newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  만약 (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  아니면
    lua_pushnil(L);
  반환 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  반환 L1;
}


static 정수 loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  정수 load = cast_int(luaL_checkinteger(L, 2));
  정수 preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  반환 0;
}

static 정수 closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  반환 0;
}

static 정수 doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const char *code = luaL_checklstring(L, 2, &lcode);
  정수 status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  만약 (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  만약 (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    반환 3;
  }
  아니면 {
    정수 i = 0;
    동안 (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    반환 i-1;
  }
}


static 정수 log2_aux (lua_State *L) {
  unsigned 정수 x = (unsigned 정수)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  반환 1;
}


struct Aux { jmp_buf jb; const char *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static 정수 panicback (lua_State *L) {
  struct Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (struct Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  반환 1;  /* to avoid warnings */
}

static 정수 checkpanic (lua_State *L) {
  struct Aux b;
  void *ud;
  lua_State *L1;
  const char *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  만약 (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    반환 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  만약 (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  아니면 {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  반환 1;
}


static 정수 externKstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  반환 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static 정수 externstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  char *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  만약 (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * sizeof(char));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  반환 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, 정수 mask, 정수 count, const char *code);

static const char *const delimits = " \t\n,;";

static void skip (const char **pc) {
  위해 (;;) {
    만약 (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    아니면 만약 (**pc == '#') {  /* comment? */
      동안 (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    아니면 break;
  }
}

static 정수 getnum_aux (lua_State *L, lua_State *L1, const char **pc) {
  정수 res = 0;
  정수 sig = 1;
  skip(pc);
  만약 (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    반환 res;
  }
  아니면 만약 (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    반환 res;
  }
  아니면 만약 (**pc == '!') {
    (*pc)++;
    만약 (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    아니면 만약 (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    아니면 lua_assert(0);
    (*pc)++;
    반환 res;
  }
  아니면 만약 (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  만약 (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  동안 (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  반환 sig*res;
}

static const char *getstring_aux (lua_State *L, char *buff, const char **pc) {
  정수 i = 0;
  skip(pc);
  만약 (**pc == '"' || **pc == '\'') {  /* quoted string? */
    정수 quote = *(*pc)++;
    동안 (**pc != quote) {
      만약 (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  아니면 {
    동안 (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  반환 buff;
}


static 정수 getindex_aux (lua_State *L, lua_State *L1, const char **pc) {
  skip(pc);
  switch (*(*pc)++) {
    case 'R': 반환 LUA_REGISTRYINDEX;
    case 'U': 반환 lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      정수 n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      만약 (n == 0) 반환 0;
      아니면 반환 lua_absindex(L1, n);
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
  unsigned 정수 i;
  위해 (i = 0; i < sizeof(statcodes) / sizeof(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static 정수 testC (lua_State *L);
static 정수 Cfunck (lua_State *L, 정수 status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const char ops[] = "+-*%^/\\&|~<>_!";

static 정수 runC (lua_State *L, lua_State *L1, const char *pc) {
  char buff[300];
  정수 status = 0;
  만약 (pc == NULL) 반환 luaL_error(L, "attempt to runC null script");
  위해 (;;) {
    const char *inst = getstring;
    만약 EQ("") 반환 0;
    아니면 만약 EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    아니면 만약 EQ("append") {
      정수 t = getindex;
      정수 i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    아니면 만약 EQ("arith") {
      정수 op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    아니면 만약 EQ("call") {
      정수 narg = getnum;
      정수 nres = getnum;
      lua_call(L1, narg, nres);
    }
    아니면 만약 EQ("callk") {
      정수 narg = getnum;
      정수 nres = getnum;
      정수 i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    아니면 만약 EQ("checkstack") {
      정수 sz = getnum;
      const char *msg = getstring;
      만약 (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    아니면 만약 EQ("rawcheckstack") {
      정수 sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    아니면 만약 EQ("compare") {
      const char *opt = getstring;  /* EQ, LT, or LE */
      정수 op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      정수 a = getindex;
      정수 b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    아니면 만약 EQ("concat") {
      lua_concat(L1, getnum);
    }
    아니면 만약 EQ("copy") {
      정수 f = getindex;
      lua_copy(L1, f, getindex);
    }
    아니면 만약 EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    아니면 만약 EQ("getfield") {
      정수 t = getindex;
      정수 tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    아니면 만약 EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    아니면 만약 EQ("getmetatable") {
      만약 (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    아니면 만약 EQ("gettable") {
      정수 tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    아니면 만약 EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    아니면 만약 EQ("gsub") {
      정수 a = getnum; 정수 b = getnum; 정수 c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    아니면 만약 EQ("insert") {
      lua_insert(L1, getnum);
    }
    아니면 만약 EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    아니면 만약 EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    아니면 만약 EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    아니면 만약 EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    아니면 만약 EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    아니면 만약 EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    아니면 만약 EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    아니면 만약 EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    아니면 만약 EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    아니면 만약 EQ("len") {
      lua_len(L1, getindex);
    }
    아니면 만약 EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    아니면 만약 EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    아니면 만약 EQ("loadstring") {
      size_t slen;
      const char *s = luaL_checklstring(L1, getnum, &slen);
      const char *name = getstring;
      const char *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    아니면 만약 EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    아니면 만약 EQ("newtable") {
      lua_newtable(L1);
    }
    아니면 만약 EQ("newthread") {
      lua_newthread(L1);
    }
    아니면 만약 EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    아니면 만약 EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    아니면 만약 EQ("next") {
      lua_next(L1, -2);
    }
    아니면 만약 EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    아니면 만약 EQ("pcall") {
      정수 narg = getnum;
      정수 nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    아니면 만약 EQ("pcallk") {
      정수 narg = getnum;
      정수 nres = getnum;
      정수 i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    아니면 만약 EQ("pop") {
      lua_pop(L1, getnum);
    }
    아니면 만약 EQ("printstack") {
      정수 n = getnum;
      만약 (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      아니면 lua_printstack(L1);
    }
    아니면 만약 EQ("print") {
      const char *msg = getstring;
      printf("%s\n", msg);
    }
    아니면 만약 EQ("warningC") {
      const char *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    아니면 만약 EQ("warning") {
      const char *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    아니면 만약 EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    아니면 만약 EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    아니면 만약 EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    아니면 만약 EQ("pushnil") {
      lua_pushnil(L1);
    }
    아니면 만약 EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    아니면 만약 EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    아니면 만약 EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    아니면 만약 EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    아니면 만약 EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    아니면 만약 EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (정수)lua_tointeger(L, -1));
    }
    아니면 만약 EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    아니면 만약 EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    아니면 만약 EQ("rawget") {
      정수 t = getindex;
      lua_rawget(L1, t);
    }
    아니면 만약 EQ("rawgeti") {
      정수 t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    아니면 만약 EQ("rawgetp") {
      정수 t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    아니면 만약 EQ("rawset") {
      정수 t = getindex;
      lua_rawset(L1, t);
    }
    아니면 만약 EQ("rawseti") {
      정수 t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    아니면 만약 EQ("rawsetp") {
      정수 t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    아니면 만약 EQ("remove") {
      lua_remove(L1, getnum);
    }
    아니면 만약 EQ("replace") {
      lua_replace(L1, getindex);
    }
    아니면 만약 EQ("resume") {
      정수 i = getindex;
      정수 nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    아니면 만약 EQ("traceback") {
      const char *msg = getstring;
      정수 level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    아니면 만약 EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    아니면 만약 EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    아니면 만약 EQ("return") {
      정수 n = getnum;
      만약 (L1 != L) {
        정수 i;
        위해 (i = 0; i < n; i++) {
          정수 idx = -(n - i);
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
      반환 n;
    }
    아니면 만약 EQ("rotate") {
      정수 i = getindex;
      lua_rotate(L1, i, getnum);
    }
    아니면 만약 EQ("setfield") {
      정수 t = getindex;
      const char *s = getstring;
      lua_setfield(L1, t, s);
    }
    아니면 만약 EQ("seti") {
      정수 t = getindex;
      lua_seti(L1, t, getnum);
    }
    아니면 만약 EQ("setglobal") {
      const char *s = getstring;
      lua_setglobal(L1, s);
    }
    아니면 만약 EQ("sethook") {
      정수 mask = getnum;
      정수 count = getnum;
      const char *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    아니면 만약 EQ("setmetatable") {
      정수 idx = getindex;
      lua_setmetatable(L1, idx);
    }
    아니면 만약 EQ("settable") {
      lua_settable(L1, getindex);
    }
    아니면 만약 EQ("settop") {
      lua_settop(L1, getnum);
    }
    아니면 만약 EQ("testudata") {
      정수 i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    아니면 만약 EQ("error") {
      lua_error(L1);
    }
    아니면 만약 EQ("abort") {
      abort();
    }
    아니면 만약 EQ("throw") {
#만약 defined(__cplusplus)
static struct X { 정수 x; } x;
      throw x;
#아니면
      luaL_error(L1, "C++");
#endif
      break;
    }
    아니면 만약 EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    아니면 만약 EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    아니면 만약 EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    아니면 만약 EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    아니면 만약 EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    아니면 만약 EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    아니면 만약 EQ("tostring") {
      const char *s = lua_tostring(L1, getindex);
      const char *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    아니면 만약 EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    아니면 만약 EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    아니면 만약 EQ("xmove") {
      정수 f = getindex;
      정수 t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      정수 n = getnum;
      만약 (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    아니면 만약 EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    아니면 만약 EQ("yield") {
      반환 lua_yield(L1, getnum);
    }
    아니면 만약 EQ("yieldk") {
      정수 nres = getnum;
      정수 i = getindex;
      반환 lua_yieldk(L1, nres, i, Cfunck);
    }
    아니면 만약 EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    아니면 만약 EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    아니면 만약 EQ("argerror") {
      정수 arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    아니면 luaL_error(L, "unknown instruction %s", buff);
  }
  반환 0;
}


static 정수 testC (lua_State *L) {
  lua_State *L1;
  const char *pc;
  만약 (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  아니면 만약 (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  아니면 {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  반환 runC(L, L1, pc);
}


static 정수 Cfunc (lua_State *L) {
  반환 runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static 정수 Cfunck (lua_State *L, 정수 status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  반환 runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static 정수 makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  반환 1;
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
static void sethookaux (lua_State *L, 정수 mask, 정수 count, const char *scpt) {
  만약 (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    반환;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  만약 (!lua_istable(L, -1)) {  /* no hook table? */
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


static 정수 sethook (lua_State *L) {
  만약 (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  아니면 {
    const char *scpt = luaL_checkstring(L, 1);
    const char *smask = luaL_checkstring(L, 2);
    정수 count = cast_int(luaL_optinteger(L, 3, 0));
    정수 mask = 0;
    만약 (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    만약 (strchr(smask, 'r')) mask |= LUA_MASKRET;
    만약 (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    만약 (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  반환 0;
}


static 정수 coresume (lua_State *L) {
  정수 status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  만약 (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    반환 2;  /* return false + error message */
  }
  아니면 {
    lua_pushboolean(L, 1);
    반환 1;
  }
}

#만약 !defined(LUA_USE_POSIX)

#define nonblock	NULL

#아니면

#include <unistd.h>
#include <fcntl.h>

static 정수 nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  정수 fd = fileno(f);
  정수 flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  반환 0;
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


정수 luaB_opentests (lua_State *L) {
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
  반환 1;
}

#endif

