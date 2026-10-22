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
#ако defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static цео runC (lua_State *L, lua_State *L1, const char *pc);


static void setnameval (lua_State *L, const char *name, цео val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const char *fmt, const char *s1, const char *s2) {
  fprintf(stderr, fmt, s1);
  ако (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static цео tpanic (lua_State *L) {
  const char *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  врати (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const char *msg, цео tocont) {
  lua_State *L = cast(lua_State *, ud);
  static char buff[200] = "";  /* should be enough for tests... */
  static цео onoff = 0;
  static цео mode = 0;  /* start in normal mode */
  static цео lasttocont = 0;
  ако (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    ако (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    ако (strcmp(msg, "@off") == 0)
      onoff = 0;
    инако ако (strcmp(msg, "@on") == 0)
      onoff = 1;
    инако ако (strcmp(msg, "@normal") == 0)
      mode = 0;
    инако ако (strcmp(msg, "@allow") == 0)
      mode = 1;
    инако ако (strcmp(msg, "@store") == 0)
      mode = 2;
    инако
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    врати;
  }
  lasttocont = tocont;
  ако (strlen(msg) >= sizeof(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  ако (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ако (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    инако {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      case 0: {  /* normal */
        ако (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      case 1: {  /* allow */
        ако (onoff)
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
    цео type;
  } d;
} memHeader;


#ако !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#инако

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  ако (block) {
    size_t size = block->d.size;
    цео i;
    за (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  цео type;
  ако (mc->memlimit == 0) {  /* first time? */
    char *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  ако (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  инако {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  ако (size == 0) {
    freeblock(mc, block);
    врати NULL;
  }
  ако (mc->failnext) {
    mc->failnext = 0;
    врати NULL;  /* fake a single memory allocation error */
  }
  ако (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    ако (mc->countlimit == 0)
      врати NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ако (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    врати NULL;  /* fake a memory allocation error */
  инако {
    memHeader *newblock;
    цео i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = sizeof(memHeader) + size + MARKSIZE;
    ако (realsize < size) врати NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    ако (newblock == NULL)
      врати NULL;  /* really out of memory? */
    ако (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    за (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    ако (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    врати newblock + 1;
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
static цео testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ако (isdead(g,t)) врати 0;
  ако (issweepphase(g))
    врати 1;  /* no invariants */
  инако ако (g->gckind != KGC_GENMINOR)
    врати !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  инако {  /* generational mode */
    ако ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      врати 0;
    ако ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      врати 0;
    врати 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ако (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static цео testobjref (global_State *g, GCObject *f, GCObject *t) {
  цео r1 = testobjref1(g, f, t);
  ако (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  врати r1;
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
#define checkobjrefN(g,f,t)	{ ако (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned цео i;
  unsigned цео asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  за (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  за (n = gnode(h, 0); n < limit; n++) {
    ако (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  цео i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  за (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  цео i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  за (i=0; i<f->sizek; i++) {
    ако (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  за (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  за (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  за (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  цео i;
  за (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  цео i;
  checkobjrefN(g, clgc, cl->p);
  за (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    ако (uv) {
      checkobjrefN(g, clgc, uv);
      ако (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static цео lua_checkpc (CallInfo *ci) {
  ако (!isLua(ci)) врати 1;
  инако {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    врати p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  ако (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    врати;
  }
  за (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  за (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  за (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, цео maybedead,
                         цео listage) {
  ако (isdead(g, o))
    assert(maybedead);
  инако {
    assert(g->gcstate != GCSpause || iswhite(o));
    ако (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      ако (isold(o)) {
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
  цео total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  док (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    ако (keepinvariant(g))
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
  врати total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  ако (!keepinvariant(g)) врати total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  врати total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  ако (!keepinvariant(g))
    врати;  /* gray lists not being kept in these phases */
  ако (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    врати;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ако (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, цео maybedead, цео tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  за (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  за (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  за (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  за (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  врати total;
}


цео lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  цео maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ако (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  за (o = g->fixedgc; o != NULL; o = o->next) {
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
  за (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ако (keepinvariant(g))
    assert(totalin == totalshould);
  врати 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static char *buildop (Proto *p, цео pc, char *buff) {
  char *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const char *name = opnames[o];
  цео line = luaG_getfuncline(p, pc);
  цео lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  ако (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  инако
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
  врати obuff;
}


#ако 0
void luaI_printcode (Proto *pt, цео size) {
  цео pc;
  за (pc=0; pc<size; pc++) {
    char buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, цео pc) {
  char buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static цео listcode (lua_State *L) {
  цео pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  за (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  врати 1;
}


static цео printcode (lua_State *L) {
  цео pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  за (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  врати 0;
}


static цео listk (lua_State *L) {
  Proto *p;
  цео i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  за (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  врати 1;
}


static цео listabslineinfo (lua_State *L) {
  Proto *p;
  цео i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  за (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  врати 1;
}


static цео listlocals (lua_State *L) {
  Proto *p;
  цео pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  цео i = 0;
  const char *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  док ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  врати i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  цео i;
  цео n = lua_gettop(L);
  printf("stack: >>\n");
  за (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


цео lua_printallstack (lua_State *L) {
  StkId p;
  цео i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  за (p = L->stack.p; p < L->top.p; p++) {
    ако (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      ако (ci == L->ci)
        ci = NULL;  /* printed last frame */
      инако
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  врати 0;
}


static цео get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  врати 1;
}


static цео get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", sizeof(lua_State));
  setnameval(L, "global state", sizeof(global_State));
  setnameval(L, "TValue", sizeof(TValue));
  setnameval(L, "Node", sizeof(Node));
  setnameval(L, "stack Value", sizeof(StackValue));
  врати 1;
}


static цео mem_query (lua_State *L) {
  ако (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    врати 3;
  }
  инако ако (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    ако (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    врати 0;
  }
  инако {
    const char *t = luaL_checkstring(L, 1);
    цео i;
    за (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ако (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        врати 1;
      }
    }
    врати luaL_error(L, "unknown type '%s'", t);
  }
}


static цео alloc_count (lua_State *L) {
  ако (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  инако
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  врати 0;
}


static цео alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  врати 0;
}


static цео settrick (lua_State *L) {
  ако (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  инако
    l_Trick = gcvalue(obj_at(L, 1));
  врати 0;
}


static цео gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ако (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  инако {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  врати 1;
}


static цео gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ако (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  инако {
    static const char *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  врати 1;
}


static цео gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ако (!iscollectable(o))
    printf("no collectable\n");
  инако {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  врати 0;
}


static const char *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static цео gc_state (lua_State *L) {
  static const цео states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  цео option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ако (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    врати 1;
  }
  инако {
    ако (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    ако (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    врати 0;
  }
}


static цео tracinggc = 0;
void luai_tracegctest (lua_State *L, цео first) {
  ако (!tracinggc) врати;
  инако {
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


static цео tracegc (lua_State *L) {
  ако (lua_isnil(L, 1))
    tracinggc = 0;
  инако {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  врати 0;
}


static цео hash_query (lua_State *L) {
  ако (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ако (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  инако {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  врати 1;
}


static цео stacklevel (lua_State *L) {
  цео a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  врати 5;
}


static цео resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  док (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  врати 0;
}


static цео reallocstack (lua_State *L) {
  цео n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  врати 0;
}


static цео table_query (lua_State *L) {
  const Table *t;
  цео i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned цео asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ако (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    врати 3;
  }
  инако ако (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ако (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    инако
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  инако ако (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ако (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    инако
      lua_pushliteral(L, "<undef>");
    ако (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    инако
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  врати 3;
}


static цео gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  врати 6;
}


static цео test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  врати 1;
}


static цео test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  врати 1;
}


static цео string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  цео s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ако (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    врати 2;
  }
  инако ако (s < tb->size) {
    TString *ts;
    цео n = 0;
    за (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    врати n;
  }
  инако врати 0;
}


static цео getreftable (lua_State *L) {
  ако (lua_istable(L, 2))  /* is there a table as second argument? */
    врати 2;  /* use it as the table */
  инако
    врати LUA_REGISTRYINDEX;  /* default is to use the register */
}


static цео tref (lua_State *L) {
  цео t = getreftable(L);
  цео level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  врати 1;
}


static цео getref (lua_State *L) {
  цео t = getreftable(L);
  цео level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  врати 1;
}

static цео unref (lua_State *L) {
  цео t = getreftable(L);
  цео level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  врати 0;
}


static цео upvalue (lua_State *L) {
  цео n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ако (lua_isnone(L, 3)) {
    const char *name = lua_getupvalue(L, 1, n);
    ако (name == NULL) врати 0;
    lua_pushstring(L, name);
    врати 2;
  }
  инако {
    const char *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    врати 1;
  }
}


static цео newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  цео nuv = cast_int(luaL_optinteger(L, 2, 0));
  char *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  док (size--) *p++ = '\0';
  врати 1;
}


static цео pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  врати 1;
}


static цео udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  врати 1;
}


static цео doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  цео status = luaL_loadbufferx(L1, s, l, s, "t");
  ако (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  врати 1;
}


static цео s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const double *, luaL_checkstring(L, 1))));
  врати 1;
}


static цео d2s (lua_State *L) {
  double d = cast(double, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), sizeof(d));
  врати 1;
}


static цео num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  врати 1;
}


static цео makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  врати 1;
}


static цео newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ако (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  инако
    lua_pushnil(L);
  врати 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  врати L1;
}


static цео loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  цео load = cast_int(luaL_checkinteger(L, 2));
  цео preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  врати 0;
}

static цео closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  врати 0;
}

static цео doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const char *code = luaL_checklstring(L, 2, &lcode);
  цео status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  ако (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ако (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    врати 3;
  }
  инако {
    цео i = 0;
    док (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    врати i-1;
  }
}


static цео log2_aux (lua_State *L) {
  unsigned цео x = (unsigned цео)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  врати 1;
}


struct Aux { jmp_buf jb; const char *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static цео panicback (lua_State *L) {
  struct Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (struct Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  врати 1;  /* to avoid warnings */
}

static цео checkpanic (lua_State *L) {
  struct Aux b;
  void *ud;
  lua_State *L1;
  const char *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ако (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    врати 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ако (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  инако {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  врати 1;
}


static цео externKstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  врати 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static цео externstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  char *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  ако (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * sizeof(char));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  врати 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, цео mask, цео count, const char *code);

static const char *const delimits = " \t\n,;";

static void skip (const char **pc) {
  за (;;) {
    ако (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    инако ако (**pc == '#') {  /* comment? */
      док (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    инако break;
  }
}

static цео getnum_aux (lua_State *L, lua_State *L1, const char **pc) {
  цео res = 0;
  цео sig = 1;
  skip(pc);
  ако (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    врати res;
  }
  инако ако (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    врати res;
  }
  инако ако (**pc == '!') {
    (*pc)++;
    ако (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    инако ако (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    инако lua_assert(0);
    (*pc)++;
    врати res;
  }
  инако ако (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ако (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  док (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  врати sig*res;
}

static const char *getstring_aux (lua_State *L, char *buff, const char **pc) {
  цео i = 0;
  skip(pc);
  ако (**pc == '"' || **pc == '\'') {  /* quoted string? */
    цео quote = *(*pc)++;
    док (**pc != quote) {
      ако (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  инако {
    док (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  врати buff;
}


static цео getindex_aux (lua_State *L, lua_State *L1, const char **pc) {
  skip(pc);
  switch (*(*pc)++) {
    case 'R': врати LUA_REGISTRYINDEX;
    case 'U': врати lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      цео n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ако (n == 0) врати 0;
      инако врати lua_absindex(L1, n);
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
  unsigned цео i;
  за (i = 0; i < sizeof(statcodes) / sizeof(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static цео testC (lua_State *L);
static цео Cfunck (lua_State *L, цео status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const char ops[] = "+-*%^/\\&|~<>_!";

static цео runC (lua_State *L, lua_State *L1, const char *pc) {
  char buff[300];
  цео status = 0;
  ако (pc == NULL) врати luaL_error(L, "attempt to runC null script");
  за (;;) {
    const char *inst = getstring;
    ако EQ("") врати 0;
    инако ако EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    инако ако EQ("append") {
      цео t = getindex;
      цео i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    инако ако EQ("arith") {
      цео op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    инако ако EQ("call") {
      цео narg = getnum;
      цео nres = getnum;
      lua_call(L1, narg, nres);
    }
    инако ако EQ("callk") {
      цео narg = getnum;
      цео nres = getnum;
      цео i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    инако ако EQ("checkstack") {
      цео sz = getnum;
      const char *msg = getstring;
      ако (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    инако ако EQ("rawcheckstack") {
      цео sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    инако ако EQ("compare") {
      const char *opt = getstring;  /* EQ, LT, or LE */
      цео op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      цео a = getindex;
      цео b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    инако ако EQ("concat") {
      lua_concat(L1, getnum);
    }
    инако ако EQ("copy") {
      цео f = getindex;
      lua_copy(L1, f, getindex);
    }
    инако ако EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    инако ако EQ("getfield") {
      цео t = getindex;
      цео tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    инако ако EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    инако ако EQ("getmetatable") {
      ако (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    инако ако EQ("gettable") {
      цео tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    инако ако EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    инако ако EQ("gsub") {
      цео a = getnum; цео b = getnum; цео c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    инако ако EQ("insert") {
      lua_insert(L1, getnum);
    }
    инако ако EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    инако ако EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    инако ако EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    инако ако EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    инако ако EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    инако ако EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    инако ако EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    инако ако EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    инако ако EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    инако ако EQ("len") {
      lua_len(L1, getindex);
    }
    инако ако EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    инако ако EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    инако ако EQ("loadstring") {
      size_t slen;
      const char *s = luaL_checklstring(L1, getnum, &slen);
      const char *name = getstring;
      const char *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    инако ако EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    инако ако EQ("newtable") {
      lua_newtable(L1);
    }
    инако ако EQ("newthread") {
      lua_newthread(L1);
    }
    инако ако EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    инако ако EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    инако ако EQ("next") {
      lua_next(L1, -2);
    }
    инако ако EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    инако ако EQ("pcall") {
      цео narg = getnum;
      цео nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    инако ако EQ("pcallk") {
      цео narg = getnum;
      цео nres = getnum;
      цео i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    инако ако EQ("pop") {
      lua_pop(L1, getnum);
    }
    инако ако EQ("printstack") {
      цео n = getnum;
      ако (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      инако lua_printstack(L1);
    }
    инако ако EQ("print") {
      const char *msg = getstring;
      printf("%s\n", msg);
    }
    инако ако EQ("warningC") {
      const char *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    инако ако EQ("warning") {
      const char *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    инако ако EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    инако ако EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    инако ако EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    инако ако EQ("pushnil") {
      lua_pushnil(L1);
    }
    инако ако EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    инако ако EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    инако ако EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    инако ако EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    инако ако EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    инако ако EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (цео)lua_tointeger(L, -1));
    }
    инако ако EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    инако ако EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    инако ако EQ("rawget") {
      цео t = getindex;
      lua_rawget(L1, t);
    }
    инако ако EQ("rawgeti") {
      цео t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    инако ако EQ("rawgetp") {
      цео t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    инако ако EQ("rawset") {
      цео t = getindex;
      lua_rawset(L1, t);
    }
    инако ако EQ("rawseti") {
      цео t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    инако ако EQ("rawsetp") {
      цео t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    инако ако EQ("remove") {
      lua_remove(L1, getnum);
    }
    инако ако EQ("replace") {
      lua_replace(L1, getindex);
    }
    инако ако EQ("resume") {
      цео i = getindex;
      цео nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    инако ако EQ("traceback") {
      const char *msg = getstring;
      цео level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    инако ако EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    инако ако EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    инако ако EQ("return") {
      цео n = getnum;
      ако (L1 != L) {
        цео i;
        за (i = 0; i < n; i++) {
          цео idx = -(n - i);
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
      врати n;
    }
    инако ако EQ("rotate") {
      цео i = getindex;
      lua_rotate(L1, i, getnum);
    }
    инако ако EQ("setfield") {
      цео t = getindex;
      const char *s = getstring;
      lua_setfield(L1, t, s);
    }
    инако ако EQ("seti") {
      цео t = getindex;
      lua_seti(L1, t, getnum);
    }
    инако ако EQ("setglobal") {
      const char *s = getstring;
      lua_setglobal(L1, s);
    }
    инако ако EQ("sethook") {
      цео mask = getnum;
      цео count = getnum;
      const char *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    инако ако EQ("setmetatable") {
      цео idx = getindex;
      lua_setmetatable(L1, idx);
    }
    инако ако EQ("settable") {
      lua_settable(L1, getindex);
    }
    инако ако EQ("settop") {
      lua_settop(L1, getnum);
    }
    инако ако EQ("testudata") {
      цео i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    инако ако EQ("error") {
      lua_error(L1);
    }
    инако ако EQ("abort") {
      abort();
    }
    инако ако EQ("throw") {
#ако defined(__cplusplus)
static struct X { цео x; } x;
      throw x;
#инако
      luaL_error(L1, "C++");
#endif
      break;
    }
    инако ако EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    инако ако EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    инако ако EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    инако ако EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    инако ако EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    инако ако EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    инако ако EQ("tostring") {
      const char *s = lua_tostring(L1, getindex);
      const char *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    инако ако EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    инако ако EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    инако ако EQ("xmove") {
      цео f = getindex;
      цео t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      цео n = getnum;
      ако (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    инако ако EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    инако ако EQ("yield") {
      врати lua_yield(L1, getnum);
    }
    инако ако EQ("yieldk") {
      цео nres = getnum;
      цео i = getindex;
      врати lua_yieldk(L1, nres, i, Cfunck);
    }
    инако ако EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    инако ако EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    инако ако EQ("argerror") {
      цео arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    инако luaL_error(L, "unknown instruction %s", buff);
  }
  врати 0;
}


static цео testC (lua_State *L) {
  lua_State *L1;
  const char *pc;
  ако (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  инако ако (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  инако {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  врати runC(L, L1, pc);
}


static цео Cfunc (lua_State *L) {
  врати runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static цео Cfunck (lua_State *L, цео status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  врати runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static цео makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  врати 1;
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
static void sethookaux (lua_State *L, цео mask, цео count, const char *scpt) {
  ако (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    врати;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ако (!lua_istable(L, -1)) {  /* no hook table? */
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


static цео sethook (lua_State *L) {
  ако (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  инако {
    const char *scpt = luaL_checkstring(L, 1);
    const char *smask = luaL_checkstring(L, 2);
    цео count = cast_int(luaL_optinteger(L, 3, 0));
    цео mask = 0;
    ако (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    ако (strchr(smask, 'r')) mask |= LUA_MASKRET;
    ако (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    ако (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  врати 0;
}


static цео coresume (lua_State *L) {
  цео status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  ако (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    врати 2;  /* return false + error message */
  }
  инако {
    lua_pushboolean(L, 1);
    врати 1;
  }
}

#ако !defined(LUA_USE_POSIX)

#define nonblock	NULL

#инако

#include <unistd.h>
#include <fcntl.h>

static цео nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  цео fd = fileno(f);
  цео flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  врати 0;
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


цео luaB_opentests (lua_State *L) {
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
  врати 1;
}

#endif

