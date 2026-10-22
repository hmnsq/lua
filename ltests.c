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
#もし defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static 整数 runC (lua_State *L, lua_State *L1, const char *pc);


static void setnameval (lua_State *L, const char *name, 整数 val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const char *fmt, const char *s1, const char *s2) {
  fprintf(stderr, fmt, s1);
  もし (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static 整数 tpanic (lua_State *L) {
  const char *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  戻る (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const char *msg, 整数 tocont) {
  lua_State *L = cast(lua_State *, ud);
  static char buff[200] = "";  /* should be enough for tests... */
  static 整数 onoff = 0;
  static 整数 mode = 0;  /* start in normal mode */
  static 整数 lasttocont = 0;
  もし (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    もし (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    もし (strcmp(msg, "@off") == 0)
      onoff = 0;
    否则 もし (strcmp(msg, "@on") == 0)
      onoff = 1;
    否则 もし (strcmp(msg, "@normal") == 0)
      mode = 0;
    否则 もし (strcmp(msg, "@allow") == 0)
      mode = 1;
    否则 もし (strcmp(msg, "@store") == 0)
      mode = 2;
    否则
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    戻る;
  }
  lasttocont = tocont;
  もし (strlen(msg) >= sizeof(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  もし (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    もし (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    否则 {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      case 0: {  /* normal */
        もし (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      case 1: {  /* allow */
        もし (onoff)
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
    整数 type;
  } d;
} memHeader;


#もし !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#否则

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  もし (block) {
    size_t size = block->d.size;
    整数 i;
    ために (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  整数 type;
  もし (mc->memlimit == 0) {  /* first time? */
    char *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  もし (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  否则 {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  もし (size == 0) {
    freeblock(mc, block);
    戻る NULL;
  }
  もし (mc->failnext) {
    mc->failnext = 0;
    戻る NULL;  /* fake a single memory allocation error */
  }
  もし (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    もし (mc->countlimit == 0)
      戻る NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  もし (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    戻る NULL;  /* fake a memory allocation error */
  否则 {
    memHeader *newblock;
    整数 i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = sizeof(memHeader) + size + MARKSIZE;
    もし (realsize < size) 戻る NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    もし (newblock == NULL)
      戻る NULL;  /* really out of memory? */
    もし (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    ために (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    もし (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    戻る newblock + 1;
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
static 整数 testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  もし (isdead(g,t)) 戻る 0;
  もし (issweepphase(g))
    戻る 1;  /* no invariants */
  否则 もし (g->gckind != KGC_GENMINOR)
    戻る !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  否则 {  /* generational mode */
    もし ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      戻る 0;
    もし ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      戻る 0;
    戻る 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  もし (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static 整数 testobjref (global_State *g, GCObject *f, GCObject *t) {
  整数 r1 = testobjref1(g, f, t);
  もし (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  戻る r1;
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
#define checkobjrefN(g,f,t)	{ もし (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned 整数 i;
  unsigned 整数 asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  ために (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  ために (n = gnode(h, 0); n < limit; n++) {
    もし (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  整数 i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  ために (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  整数 i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  ために (i=0; i<f->sizek; i++) {
    もし (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  ために (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  ために (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  ために (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  整数 i;
  ために (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  整数 i;
  checkobjrefN(g, clgc, cl->p);
  ために (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    もし (uv) {
      checkobjrefN(g, clgc, uv);
      もし (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static 整数 lua_checkpc (CallInfo *ci) {
  もし (!isLua(ci)) 戻る 1;
  否则 {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    戻る p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  もし (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    戻る;
  }
  ために (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  ために (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  ために (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, 整数 maybedead,
                         整数 listage) {
  もし (isdead(g, o))
    assert(maybedead);
  否则 {
    assert(g->gcstate != GCSpause || iswhite(o));
    もし (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      もし (isold(o)) {
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
  整数 total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ながら (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    もし (keepinvariant(g))
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
  戻る total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  もし (!keepinvariant(g)) 戻る total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  戻る total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  もし (!keepinvariant(g))
    戻る;  /* gray lists not being kept in these phases */
  もし (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    戻る;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  もし (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, 整数 maybedead, 整数 tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  ために (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ために (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ために (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ために (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  戻る total;
}


整数 lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  整数 maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  もし (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  ために (o = g->fixedgc; o != NULL; o = o->next) {
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
  ために (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  もし (keepinvariant(g))
    assert(totalin == totalshould);
  戻る 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static char *buildop (Proto *p, 整数 pc, char *buff) {
  char *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const char *name = opnames[o];
  整数 line = luaG_getfuncline(p, pc);
  整数 lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  もし (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  否则
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
  戻る obuff;
}


#もし 0
void luaI_printcode (Proto *pt, 整数 size) {
  整数 pc;
  ために (pc=0; pc<size; pc++) {
    char buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, 整数 pc) {
  char buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static 整数 listcode (lua_State *L) {
  整数 pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  ために (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  戻る 1;
}


static 整数 printcode (lua_State *L) {
  整数 pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  ために (pc=0; pc<p->sizecode; pc++) {
    char buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  戻る 0;
}


static 整数 listk (lua_State *L) {
  Proto *p;
  整数 i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  ために (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  戻る 1;
}


static 整数 listabslineinfo (lua_State *L) {
  Proto *p;
  整数 i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  ために (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  戻る 1;
}


static 整数 listlocals (lua_State *L) {
  Proto *p;
  整数 pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  整数 i = 0;
  const char *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ながら ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  戻る i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  整数 i;
  整数 n = lua_gettop(L);
  printf("stack: >>\n");
  ために (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


整数 lua_printallstack (lua_State *L) {
  StkId p;
  整数 i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  ために (p = L->stack.p; p < L->top.p; p++) {
    もし (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      もし (ci == L->ci)
        ci = NULL;  /* printed last frame */
      否则
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  戻る 0;
}


static 整数 get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  戻る 1;
}


static 整数 get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", sizeof(lua_State));
  setnameval(L, "global state", sizeof(global_State));
  setnameval(L, "TValue", sizeof(TValue));
  setnameval(L, "Node", sizeof(Node));
  setnameval(L, "stack Value", sizeof(StackValue));
  戻る 1;
}


static 整数 mem_query (lua_State *L) {
  もし (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    戻る 3;
  }
  否则 もし (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    もし (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    戻る 0;
  }
  否则 {
    const char *t = luaL_checkstring(L, 1);
    整数 i;
    ために (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      もし (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        戻る 1;
      }
    }
    戻る luaL_error(L, "unknown type '%s'", t);
  }
}


static 整数 alloc_count (lua_State *L) {
  もし (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  否则
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  戻る 0;
}


static 整数 alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  戻る 0;
}


static 整数 settrick (lua_State *L) {
  もし (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  否则
    l_Trick = gcvalue(obj_at(L, 1));
  戻る 0;
}


static 整数 gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  もし (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  否则 {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  戻る 1;
}


static 整数 gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  もし (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  否则 {
    static const char *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  戻る 1;
}


static 整数 gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  もし (!iscollectable(o))
    printf("no collectable\n");
  否则 {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  戻る 0;
}


static const char *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static 整数 gc_state (lua_State *L) {
  static const 整数 states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  整数 option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  もし (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    戻る 1;
  }
  否则 {
    もし (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    もし (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    戻る 0;
  }
}


static 整数 tracinggc = 0;
void luai_tracegctest (lua_State *L, 整数 first) {
  もし (!tracinggc) 戻る;
  否则 {
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


static 整数 tracegc (lua_State *L) {
  もし (lua_isnil(L, 1))
    tracinggc = 0;
  否则 {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  戻る 0;
}


static 整数 hash_query (lua_State *L) {
  もし (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    もし (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  否则 {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  戻る 1;
}


static 整数 stacklevel (lua_State *L) {
  整数 a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  戻る 5;
}


static 整数 resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ながら (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  戻る 0;
}


static 整数 reallocstack (lua_State *L) {
  整数 n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  戻る 0;
}


static 整数 table_query (lua_State *L) {
  const Table *t;
  整数 i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned 整数 asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  もし (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    戻る 3;
  }
  否则 もし (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    もし (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    否则
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  否则 もし (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    もし (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    否则
      lua_pushliteral(L, "<undef>");
    もし (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    否则
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  戻る 3;
}


static 整数 gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  戻る 6;
}


static 整数 test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  戻る 1;
}


static 整数 test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  戻る 1;
}


static 整数 string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  整数 s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  もし (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    戻る 2;
  }
  否则 もし (s < tb->size) {
    TString *ts;
    整数 n = 0;
    ために (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    戻る n;
  }
  否则 戻る 0;
}


static 整数 getreftable (lua_State *L) {
  もし (lua_istable(L, 2))  /* is there a table as second argument? */
    戻る 2;  /* use it as the table */
  否则
    戻る LUA_REGISTRYINDEX;  /* default is to use the register */
}


static 整数 tref (lua_State *L) {
  整数 t = getreftable(L);
  整数 level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  戻る 1;
}


static 整数 getref (lua_State *L) {
  整数 t = getreftable(L);
  整数 level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  戻る 1;
}

static 整数 unref (lua_State *L) {
  整数 t = getreftable(L);
  整数 level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  戻る 0;
}


static 整数 upvalue (lua_State *L) {
  整数 n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  もし (lua_isnone(L, 3)) {
    const char *name = lua_getupvalue(L, 1, n);
    もし (name == NULL) 戻る 0;
    lua_pushstring(L, name);
    戻る 2;
  }
  否则 {
    const char *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    戻る 1;
  }
}


static 整数 newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  整数 nuv = cast_int(luaL_optinteger(L, 2, 0));
  char *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  ながら (size--) *p++ = '\0';
  戻る 1;
}


static 整数 pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  戻る 1;
}


static 整数 udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  戻る 1;
}


static 整数 doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  整数 status = luaL_loadbufferx(L1, s, l, s, "t");
  もし (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  戻る 1;
}


static 整数 s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const double *, luaL_checkstring(L, 1))));
  戻る 1;
}


static 整数 d2s (lua_State *L) {
  double d = cast(double, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), sizeof(d));
  戻る 1;
}


static 整数 num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  戻る 1;
}


static 整数 makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  戻る 1;
}


static 整数 newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  もし (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  否则
    lua_pushnil(L);
  戻る 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  戻る L1;
}


static 整数 loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  整数 load = cast_int(luaL_checkinteger(L, 2));
  整数 preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  戻る 0;
}

static 整数 closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  戻る 0;
}

static 整数 doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const char *code = luaL_checklstring(L, 2, &lcode);
  整数 status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  もし (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  もし (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    戻る 3;
  }
  否则 {
    整数 i = 0;
    ながら (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    戻る i-1;
  }
}


static 整数 log2_aux (lua_State *L) {
  unsigned 整数 x = (unsigned 整数)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  戻る 1;
}


struct Aux { jmp_buf jb; const char *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static 整数 panicback (lua_State *L) {
  struct Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (struct Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  戻る 1;  /* to avoid warnings */
}

static 整数 checkpanic (lua_State *L) {
  struct Aux b;
  void *ud;
  lua_State *L1;
  const char *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  もし (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    戻る 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  もし (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  否则 {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  戻る 1;
}


static 整数 externKstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  戻る 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static 整数 externstr (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  char *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  もし (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * sizeof(char));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  戻る 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, 整数 mask, 整数 count, const char *code);

static const char *const delimits = " \t\n,;";

static void skip (const char **pc) {
  ために (;;) {
    もし (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    否则 もし (**pc == '#') {  /* comment? */
      ながら (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    否则 break;
  }
}

static 整数 getnum_aux (lua_State *L, lua_State *L1, const char **pc) {
  整数 res = 0;
  整数 sig = 1;
  skip(pc);
  もし (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    戻る res;
  }
  否则 もし (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    戻る res;
  }
  否则 もし (**pc == '!') {
    (*pc)++;
    もし (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    否则 もし (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    否则 lua_assert(0);
    (*pc)++;
    戻る res;
  }
  否则 もし (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  もし (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ながら (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  戻る sig*res;
}

static const char *getstring_aux (lua_State *L, char *buff, const char **pc) {
  整数 i = 0;
  skip(pc);
  もし (**pc == '"' || **pc == '\'') {  /* quoted string? */
    整数 quote = *(*pc)++;
    ながら (**pc != quote) {
      もし (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  否则 {
    ながら (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  戻る buff;
}


static 整数 getindex_aux (lua_State *L, lua_State *L1, const char **pc) {
  skip(pc);
  switch (*(*pc)++) {
    case 'R': 戻る LUA_REGISTRYINDEX;
    case 'U': 戻る lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      整数 n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      もし (n == 0) 戻る 0;
      否则 戻る lua_absindex(L1, n);
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
  unsigned 整数 i;
  ために (i = 0; i < sizeof(statcodes) / sizeof(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static 整数 testC (lua_State *L);
static 整数 Cfunck (lua_State *L, 整数 status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const char ops[] = "+-*%^/\\&|~<>_!";

static 整数 runC (lua_State *L, lua_State *L1, const char *pc) {
  char buff[300];
  整数 status = 0;
  もし (pc == NULL) 戻る luaL_error(L, "attempt to runC null script");
  ために (;;) {
    const char *inst = getstring;
    もし EQ("") 戻る 0;
    否则 もし EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    否则 もし EQ("append") {
      整数 t = getindex;
      整数 i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    否则 もし EQ("arith") {
      整数 op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    否则 もし EQ("call") {
      整数 narg = getnum;
      整数 nres = getnum;
      lua_call(L1, narg, nres);
    }
    否则 もし EQ("callk") {
      整数 narg = getnum;
      整数 nres = getnum;
      整数 i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    否则 もし EQ("checkstack") {
      整数 sz = getnum;
      const char *msg = getstring;
      もし (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    否则 もし EQ("rawcheckstack") {
      整数 sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    否则 もし EQ("compare") {
      const char *opt = getstring;  /* EQ, LT, or LE */
      整数 op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      整数 a = getindex;
      整数 b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    否则 もし EQ("concat") {
      lua_concat(L1, getnum);
    }
    否则 もし EQ("copy") {
      整数 f = getindex;
      lua_copy(L1, f, getindex);
    }
    否则 もし EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    否则 もし EQ("getfield") {
      整数 t = getindex;
      整数 tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    否则 もし EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    否则 もし EQ("getmetatable") {
      もし (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    否则 もし EQ("gettable") {
      整数 tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    否则 もし EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    否则 もし EQ("gsub") {
      整数 a = getnum; 整数 b = getnum; 整数 c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    否则 もし EQ("insert") {
      lua_insert(L1, getnum);
    }
    否则 もし EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    否则 もし EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    否则 もし EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    否则 もし EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    否则 もし EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    否则 もし EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    否则 もし EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    否则 もし EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    否则 もし EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    否则 もし EQ("len") {
      lua_len(L1, getindex);
    }
    否则 もし EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    否则 もし EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    否则 もし EQ("loadstring") {
      size_t slen;
      const char *s = luaL_checklstring(L1, getnum, &slen);
      const char *name = getstring;
      const char *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    否则 もし EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    否则 もし EQ("newtable") {
      lua_newtable(L1);
    }
    否则 もし EQ("newthread") {
      lua_newthread(L1);
    }
    否则 もし EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    否则 もし EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    否则 もし EQ("next") {
      lua_next(L1, -2);
    }
    否则 もし EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    否则 もし EQ("pcall") {
      整数 narg = getnum;
      整数 nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    否则 もし EQ("pcallk") {
      整数 narg = getnum;
      整数 nres = getnum;
      整数 i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    否则 もし EQ("pop") {
      lua_pop(L1, getnum);
    }
    否则 もし EQ("printstack") {
      整数 n = getnum;
      もし (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      否则 lua_printstack(L1);
    }
    否则 もし EQ("print") {
      const char *msg = getstring;
      printf("%s\n", msg);
    }
    否则 もし EQ("warningC") {
      const char *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    否则 もし EQ("warning") {
      const char *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    否则 もし EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    否则 もし EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    否则 もし EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    否则 もし EQ("pushnil") {
      lua_pushnil(L1);
    }
    否则 もし EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    否则 もし EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    否则 もし EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    否则 もし EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    否则 もし EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    否则 もし EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (整数)lua_tointeger(L, -1));
    }
    否则 もし EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    否则 もし EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    否则 もし EQ("rawget") {
      整数 t = getindex;
      lua_rawget(L1, t);
    }
    否则 もし EQ("rawgeti") {
      整数 t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    否则 もし EQ("rawgetp") {
      整数 t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    否则 もし EQ("rawset") {
      整数 t = getindex;
      lua_rawset(L1, t);
    }
    否则 もし EQ("rawseti") {
      整数 t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    否则 もし EQ("rawsetp") {
      整数 t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    否则 もし EQ("remove") {
      lua_remove(L1, getnum);
    }
    否则 もし EQ("replace") {
      lua_replace(L1, getindex);
    }
    否则 もし EQ("resume") {
      整数 i = getindex;
      整数 nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    否则 もし EQ("traceback") {
      const char *msg = getstring;
      整数 level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    否则 もし EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    否则 もし EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    否则 もし EQ("return") {
      整数 n = getnum;
      もし (L1 != L) {
        整数 i;
        ために (i = 0; i < n; i++) {
          整数 idx = -(n - i);
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
      戻る n;
    }
    否则 もし EQ("rotate") {
      整数 i = getindex;
      lua_rotate(L1, i, getnum);
    }
    否则 もし EQ("setfield") {
      整数 t = getindex;
      const char *s = getstring;
      lua_setfield(L1, t, s);
    }
    否则 もし EQ("seti") {
      整数 t = getindex;
      lua_seti(L1, t, getnum);
    }
    否则 もし EQ("setglobal") {
      const char *s = getstring;
      lua_setglobal(L1, s);
    }
    否则 もし EQ("sethook") {
      整数 mask = getnum;
      整数 count = getnum;
      const char *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    否则 もし EQ("setmetatable") {
      整数 idx = getindex;
      lua_setmetatable(L1, idx);
    }
    否则 もし EQ("settable") {
      lua_settable(L1, getindex);
    }
    否则 もし EQ("settop") {
      lua_settop(L1, getnum);
    }
    否则 もし EQ("testudata") {
      整数 i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    否则 もし EQ("error") {
      lua_error(L1);
    }
    否则 もし EQ("abort") {
      abort();
    }
    否则 もし EQ("throw") {
#もし defined(__cplusplus)
static struct X { 整数 x; } x;
      throw x;
#否则
      luaL_error(L1, "C++");
#endif
      break;
    }
    否则 もし EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    否则 もし EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    否则 もし EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    否则 もし EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    否则 もし EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    否则 もし EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    否则 もし EQ("tostring") {
      const char *s = lua_tostring(L1, getindex);
      const char *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    否则 もし EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    否则 もし EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    否则 もし EQ("xmove") {
      整数 f = getindex;
      整数 t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      整数 n = getnum;
      もし (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    否则 もし EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    否则 もし EQ("yield") {
      戻る lua_yield(L1, getnum);
    }
    否则 もし EQ("yieldk") {
      整数 nres = getnum;
      整数 i = getindex;
      戻る lua_yieldk(L1, nres, i, Cfunck);
    }
    否则 もし EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    否则 もし EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    否则 もし EQ("argerror") {
      整数 arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    否则 luaL_error(L, "unknown instruction %s", buff);
  }
  戻る 0;
}


static 整数 testC (lua_State *L) {
  lua_State *L1;
  const char *pc;
  もし (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  否则 もし (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  否则 {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  戻る runC(L, L1, pc);
}


static 整数 Cfunc (lua_State *L) {
  戻る runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static 整数 Cfunck (lua_State *L, 整数 status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  戻る runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static 整数 makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  戻る 1;
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
static void sethookaux (lua_State *L, 整数 mask, 整数 count, const char *scpt) {
  もし (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    戻る;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  もし (!lua_istable(L, -1)) {  /* no hook table? */
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


static 整数 sethook (lua_State *L) {
  もし (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  否则 {
    const char *scpt = luaL_checkstring(L, 1);
    const char *smask = luaL_checkstring(L, 2);
    整数 count = cast_int(luaL_optinteger(L, 3, 0));
    整数 mask = 0;
    もし (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    もし (strchr(smask, 'r')) mask |= LUA_MASKRET;
    もし (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    もし (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  戻る 0;
}


static 整数 coresume (lua_State *L) {
  整数 status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  もし (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    戻る 2;  /* return false + error message */
  }
  否则 {
    lua_pushboolean(L, 1);
    戻る 1;
  }
}

#もし !defined(LUA_USE_POSIX)

#define nonblock	NULL

#否则

#include <unistd.h>
#include <fcntl.h>

static 整数 nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  整数 fd = fileno(f);
  整数 flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  戻る 0;
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


整数 luaB_opentests (lua_State *L) {
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
  戻る 1;
}

#endif

