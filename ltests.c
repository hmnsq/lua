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
#གལ་ཏེ defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static ཧྲིལ runC (lua_State *L, lua_State *L1, const var_na *pc);


static void setnameval (lua_State *L, const var_na *name, ཧྲིལ val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const var_na *fmt, const var_na *s1, const var_na *s2) {
  fprintf(stderr, fmt, s1);
  གལ་ཏེ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static ཧྲིལ tpanic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ཕྱིར (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const var_na *msg, ཧྲིལ tocont) {
  lua_State *L = cast(lua_State *, ud);
  static var_na buff[200] = "";  /* should be enough for tests... */
  static ཧྲིལ onoff = 0;
  static ཧྲིལ mode = 0;  /* start in normal mode */
  static ཧྲིལ lasttocont = 0;
  གལ་ཏེ (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    གལ་ཏེ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    གལ་ཏེ (strcmp(msg, "@off") == 0)
      onoff = 0;
    དེ་མིན གལ་ཏེ (strcmp(msg, "@on") == 0)
      onoff = 1;
    དེ་མིན གལ་ཏེ (strcmp(msg, "@normal") == 0)
      mode = 0;
    དེ་མིན གལ་ཏེ (strcmp(msg, "@allow") == 0)
      mode = 1;
    དེ་མིན གལ་ཏེ (strcmp(msg, "@store") == 0)
      mode = 2;
    དེ་མིན
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    ཕྱིར;
  }
  lasttocont = tocont;
  གལ་ཏེ (strlen(msg) >= parimaa_na(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  གལ་ཏེ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    གལ་ཏེ (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    དེ་མིན {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      pak_sa 0: {  /* normal */
        གལ་ཏེ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      pak_sa 1: {  /* allow */
        གལ་ཏེ (onoff)
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
    ཧྲིལ type;
  } d;
} memHeader;


#གལ་ཏེ !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#དེ་མིན

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  གལ་ཏེ (block) {
    size_t size = block->d.size;
    ཧྲིལ i;
    ཆེད་དུ (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  ཧྲིལ type;
  གལ་ཏེ (mc->memlimit == 0) {  /* first time? */
    var_na *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  གལ་ཏེ (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  དེ་མིན {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  གལ་ཏེ (size == 0) {
    freeblock(mc, block);
    ཕྱིར NULL;
  }
  གལ་ཏེ (mc->failnext) {
    mc->failnext = 0;
    ཕྱིར NULL;  /* fake a single memory allocation error */
  }
  གལ་ཏེ (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    གལ་ཏེ (mc->countlimit == 0)
      ཕྱིར NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  གལ་ཏེ (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ཕྱིར NULL;  /* fake a memory allocation error */
  དེ་མིན {
    memHeader *newblock;
    ཧྲིལ i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = parimaa_na(memHeader) + size + MARKSIZE;
    གལ་ཏེ (realsize < size) ཕྱིར NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    གལ་ཏེ (newblock == NULL)
      ཕྱིར NULL;  /* really out of memory? */
    གལ་ཏེ (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    ཆེད་དུ (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    གལ་ཏེ (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ཕྱིར newblock + 1;
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
static ཧྲིལ testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  གལ་ཏེ (isdead(g,t)) ཕྱིར 0;
  གལ་ཏེ (issweepphase(g))
    ཕྱིར 1;  /* no invariants */
  དེ་མིན གལ་ཏེ (g->gckind != KGC_GENMINOR)
    ཕྱིར !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  དེ་མིན {  /* generational mode */
    གལ་ཏེ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ཕྱིར 0;
    གལ་ཏེ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ཕྱིར 0;
    ཕྱིར 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  གལ་ཏེ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static ཧྲིལ testobjref (global_State *g, GCObject *f, GCObject *t) {
  ཧྲིལ r1 = testobjref1(g, f, t);
  གལ་ཏེ (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ཕྱིར r1;
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
#define checkobjrefN(g,f,t)	{ གལ་ཏེ (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned ཧྲིལ i;
  unsigned ཧྲིལ asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  ཆེད་དུ (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  ཆེད་དུ (n = gnode(h, 0); n < limit; n++) {
    གལ་ཏེ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  ཧྲིལ i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  ཆེད་དུ (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  ཧྲིལ i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  ཆེད་དུ (i=0; i<f->sizek; i++) {
    གལ་ཏེ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  ཆེད་དུ (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  ཆེད་དུ (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  ཆེད་དུ (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ཧྲིལ i;
  ཆེད་དུ (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ཧྲིལ i;
  checkobjrefN(g, clgc, cl->p);
  ཆེད་དུ (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    གལ་ཏེ (uv) {
      checkobjrefN(g, clgc, uv);
      གལ་ཏེ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static ཧྲིལ lua_checkpc (CallInfo *ci) {
  གལ་ཏེ (!isLua(ci)) ཕྱིར 1;
  དེ་མིན {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ཕྱིར p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  གལ་ཏེ (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    ཕྱིར;
  }
  ཆེད་དུ (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  ཆེད་དུ (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  ཆེད་དུ (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, ཧྲིལ maybedead,
                         ཧྲིལ listage) {
  གལ་ཏེ (isdead(g, o))
    assert(maybedead);
  དེ་མིན {
    assert(g->gcstate != GCSpause || iswhite(o));
    གལ་ཏེ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      གལ་ཏེ (isold(o)) {
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
  ཧྲིལ total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  སྐབས (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    གལ་ཏེ (keepinvariant(g))
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
  ཕྱིར total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  གལ་ཏེ (!keepinvariant(g)) ཕྱིར total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ཕྱིར total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  གལ་ཏེ (!keepinvariant(g))
    ཕྱིར;  /* gray lists not being kept in these phases */
  གལ་ཏེ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    ཕྱིར;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  གལ་ཏེ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, ཧྲིལ maybedead, ཧྲིལ tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  ཆེད་དུ (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ཆེད་དུ (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ཆེད་དུ (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ཆེད་དུ (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ཕྱིར total;
}


ཧྲིལ lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ཧྲིལ maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  གལ་ཏེ (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  ཆེད་དུ (o = g->fixedgc; o != NULL; o = o->next) {
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
  ཆེད་དུ (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  གལ་ཏེ (keepinvariant(g))
    assert(totalin == totalshould);
  ཕྱིར 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static var_na *buildop (Proto *p, ཧྲིལ pc, var_na *buff) {
  var_na *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const var_na *name = opnames[o];
  ཧྲིལ line = luaG_getfuncline(p, pc);
  ཧྲིལ lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  གལ་ཏེ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  དེ་མིན
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
  ཕྱིར obuff;
}


#གལ་ཏེ 0
void luaI_printcode (Proto *pt, ཧྲིལ size) {
  ཧྲིལ pc;
  ཆེད་དུ (pc=0; pc<size; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, ཧྲིལ pc) {
  var_na buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static ཧྲིལ listcode (lua_State *L) {
  ཧྲིལ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  ཆེད་དུ (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ཕྱིར 1;
}


static ཧྲིལ printcode (lua_State *L) {
  ཧྲིལ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  ཆེད་དུ (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ཕྱིར 0;
}


static ཧྲིལ listk (lua_State *L) {
  Proto *p;
  ཧྲིལ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  ཆེད་དུ (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ཕྱིར 1;
}


static ཧྲིལ listabslineinfo (lua_State *L) {
  Proto *p;
  ཧྲིལ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  ཆེད་དུ (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ཕྱིར 1;
}


static ཧྲིལ listlocals (lua_State *L) {
  Proto *p;
  ཧྲིལ pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ཧྲིལ i = 0;
  const var_na *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  སྐབས ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  ཕྱིར i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  ཧྲིལ i;
  ཧྲིལ n = lua_gettop(L);
  printf("stack: >>\n");
  ཆེད་དུ (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


ཧྲིལ lua_printallstack (lua_State *L) {
  StkId p;
  ཧྲིལ i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  ཆེད་དུ (p = L->stack.p; p < L->top.p; p++) {
    གལ་ཏེ (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      གལ་ཏེ (ci == L->ci)
        ci = NULL;  /* printed last frame */
      དེ་མིན
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ཕྱིར 0;
}


static ཧྲིལ get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  ཕྱིར 1;
}


static ཧྲིལ get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", parimaa_na(lua_State));
  setnameval(L, "global state", parimaa_na(global_State));
  setnameval(L, "TValue", parimaa_na(TValue));
  setnameval(L, "Node", parimaa_na(Node));
  setnameval(L, "stack Value", parimaa_na(StackValue));
  ཕྱིར 1;
}


static ཧྲིལ mem_query (lua_State *L) {
  གལ་ཏེ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ཕྱིར 3;
  }
  དེ་མིན གལ་ཏེ (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    གལ་ཏེ (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ཕྱིར 0;
  }
  དེ་མིན {
    const var_na *t = luaL_checkstring(L, 1);
    ཧྲིལ i;
    ཆེད་དུ (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      གལ་ཏེ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ཕྱིར 1;
      }
    }
    ཕྱིར luaL_error(L, "unknown type '%s'", t);
  }
}


static ཧྲིལ alloc_count (lua_State *L) {
  གལ་ཏེ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  དེ་མིན
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ཕྱིར 0;
}


static ཧྲིལ alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ཕྱིར 0;
}


static ཧྲིལ settrick (lua_State *L) {
  གལ་ཏེ (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  དེ་མིན
    l_Trick = gcvalue(obj_at(L, 1));
  ཕྱིར 0;
}


static ཧྲིལ gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  གལ་ཏེ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  དེ་མིན {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ཕྱིར 1;
}


static ཧྲིལ gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  གལ་ཏེ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  དེ་མིན {
    static const var_na *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ཕྱིར 1;
}


static ཧྲིལ gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  གལ་ཏེ (!iscollectable(o))
    printf("no collectable\n");
  དེ་མིན {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ཕྱིར 0;
}


static const var_na *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static ཧྲིལ gc_state (lua_State *L) {
  static const ཧྲིལ states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ཧྲིལ option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  གལ་ཏེ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ཕྱིར 1;
  }
  དེ་མིན {
    གལ་ཏེ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    གལ་ཏེ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ཕྱིར 0;
  }
}


static ཧྲིལ tracinggc = 0;
void luai_tracegctest (lua_State *L, ཧྲིལ first) {
  གལ་ཏེ (!tracinggc) ཕྱིར;
  དེ་མིན {
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


static ཧྲིལ tracegc (lua_State *L) {
  གལ་ཏེ (lua_isnil(L, 1))
    tracinggc = 0;
  དེ་མིན {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ཕྱིར 0;
}


static ཧྲིལ hash_query (lua_State *L) {
  གལ་ཏེ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    གལ་ཏེ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  དེ་མིན {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  ཕྱིར 1;
}


static ཧྲིལ stacklevel (lua_State *L) {
  ཧྲིལ a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ཕྱིར 5;
}


static ཧྲིལ resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  སྐབས (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ཕྱིར 0;
}


static ཧྲིལ reallocstack (lua_State *L) {
  ཧྲིལ n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  ཕྱིར 0;
}


static ཧྲིལ table_query (lua_State *L) {
  const Table *t;
  ཧྲིལ i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned ཧྲིལ asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  གལ་ཏེ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ཕྱིར 3;
  }
  དེ་མིན གལ་ཏེ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    གལ་ཏེ (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    དེ་མིན
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  དེ་མིན གལ་ཏེ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    གལ་ཏེ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    དེ་མིན
      lua_pushliteral(L, "<undef>");
    གལ་ཏེ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    དེ་མིན
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  ཕྱིར 3;
}


static ཧྲིལ gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ཕྱིར 6;
}


static ཧྲིལ test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ཕྱིར 1;
}


static ཧྲིལ test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ཕྱིར 1;
}


static ཧྲིལ string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ཧྲིལ s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  གལ་ཏེ (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ཕྱིར 2;
  }
  དེ་མིན གལ་ཏེ (s < tb->size) {
    TString *ts;
    ཧྲིལ n = 0;
    ཆེད་དུ (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    ཕྱིར n;
  }
  དེ་མིན ཕྱིར 0;
}


static ཧྲིལ getreftable (lua_State *L) {
  གལ་ཏེ (lua_istable(L, 2))  /* is there a table as second argument? */
    ཕྱིར 2;  /* use it as the table */
  དེ་མིན
    ཕྱིར LUA_REGISTRYINDEX;  /* default is to use the register */
}


static ཧྲིལ tref (lua_State *L) {
  ཧྲིལ t = getreftable(L);
  ཧྲིལ level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ཕྱིར 1;
}


static ཧྲིལ getref (lua_State *L) {
  ཧྲིལ t = getreftable(L);
  ཧྲིལ level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ཕྱིར 1;
}

static ཧྲིལ unref (lua_State *L) {
  ཧྲིལ t = getreftable(L);
  ཧྲིལ level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ཕྱིར 0;
}


static ཧྲིལ upvalue (lua_State *L) {
  ཧྲིལ n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  གལ་ཏེ (lua_isnone(L, 3)) {
    const var_na *name = lua_getupvalue(L, 1, n);
    གལ་ཏེ (name == NULL) ཕྱིར 0;
    lua_pushstring(L, name);
    ཕྱིར 2;
  }
  དེ་མིན {
    const var_na *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ཕྱིར 1;
  }
}


static ཧྲིལ newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  ཧྲིལ nuv = cast_int(luaL_optinteger(L, 2, 0));
  var_na *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  སྐབས (size--) *p++ = '\0';
  ཕྱིར 1;
}


static ཧྲིལ pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ཕྱིར 1;
}


static ཧྲིལ udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ཕྱིར 1;
}


static ཧྲིལ doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  ཧྲིལ status = luaL_loadbufferx(L1, s, l, s, "t");
  གལ་ཏེ (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ཕྱིར 1;
}


static ཧྲིལ s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const dviguu_na *, luaL_checkstring(L, 1))));
  ཕྱིར 1;
}


static ཧྲིལ d2s (lua_State *L) {
  dviguu_na d = cast(dviguu_na, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), parimaa_na(d));
  ཕྱིར 1;
}


static ཧྲིལ num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ཕྱིར 1;
}


static ཧྲིལ makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ཕྱིར 1;
}


static ཧྲིལ newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  གལ་ཏེ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  དེ་མིན
    lua_pushnil(L);
  ཕྱིར 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  ཕྱིར L1;
}


static ཧྲིལ loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ཧྲིལ load = cast_int(luaL_checkinteger(L, 2));
  ཧྲིལ preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ཕྱིར 0;
}

static ཧྲིལ closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ཕྱིར 0;
}

static ཧྲིལ doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const var_na *code = luaL_checklstring(L, 2, &lcode);
  ཧྲིལ status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  གལ་ཏེ (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  གལ་ཏེ (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ཕྱིར 3;
  }
  དེ་མིན {
    ཧྲིལ i = 0;
    སྐབས (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    ཕྱིར i-1;
  }
}


static ཧྲིལ log2_aux (lua_State *L) {
  unsigned ཧྲིལ x = (unsigned ཧྲིལ)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ཕྱིར 1;
}


sa_nracanaa Aux { jmp_buf jb; const var_na *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static ཧྲིལ panicback (lua_State *L) {
  sa_nracanaa Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (sa_nracanaa Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ཕྱིར 1;  /* to avoid warnings */
}

static ཧྲིལ checkpanic (lua_State *L) {
  sa_nracanaa Aux b;
  void *ud;
  lua_State *L1;
  const var_na *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  གལ་ཏེ (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ཕྱིར 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  གལ་ཏེ (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  དེ་མིན {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ཕྱིར 1;
}


static ཧྲིལ externKstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  ཕྱིར 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static ཧྲིལ externstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  var_na *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  གལ་ཏེ (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * parimaa_na(var_na));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ཕྱིར 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, ཧྲིལ mask, ཧྲིལ count, const var_na *code);

static const var_na *const delimits = " \t\n,;";

static void skip (const var_na **pc) {
  ཆེད་དུ (;;) {
    གལ་ཏེ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    དེ་མིན གལ་ཏེ (**pc == '#') {  /* comment? */
      སྐབས (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    དེ་མིན bha_nga;
  }
}

static ཧྲིལ getnum_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  ཧྲིལ res = 0;
  ཧྲིལ sig = 1;
  skip(pc);
  གལ་ཏེ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    ཕྱིར res;
  }
  དེ་མིན གལ་ཏེ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ཕྱིར res;
  }
  དེ་མིན གལ་ཏེ (**pc == '!') {
    (*pc)++;
    གལ་ཏེ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    དེ་མིན གལ་ཏེ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    དེ་མིན lua_assert(0);
    (*pc)++;
    ཕྱིར res;
  }
  དེ་མིན གལ་ཏེ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  གལ་ཏེ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  སྐབས (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ཕྱིར sig*res;
}

static const var_na *getstring_aux (lua_State *L, var_na *buff, const var_na **pc) {
  ཧྲིལ i = 0;
  skip(pc);
  གལ་ཏེ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ཧྲིལ quote = *(*pc)++;
    སྐབས (**pc != quote) {
      གལ་ཏེ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  དེ་མིན {
    སྐབས (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ཕྱིར buff;
}


static ཧྲིལ getindex_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  skip(pc);
  switch (*(*pc)++) {
    pak_sa 'R': ཕྱིར LUA_REGISTRYINDEX;
    pak_sa 'U': ཕྱིར lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      ཧྲིལ n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      གལ་ཏེ (n == 0) ཕྱིར 0;
      དེ་མིན ཕྱིར lua_absindex(L1, n);
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
  unsigned ཧྲིལ i;
  ཆེད་དུ (i = 0; i < parimaa_na(statcodes) / parimaa_na(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static ཧྲིལ testC (lua_State *L);
static ཧྲིལ Cfunck (lua_State *L, ཧྲིལ status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const var_na ops[] = "+-*%^/\\&|~<>_!";

static ཧྲིལ runC (lua_State *L, lua_State *L1, const var_na *pc) {
  var_na buff[300];
  ཧྲིལ status = 0;
  གལ་ཏེ (pc == NULL) ཕྱིར luaL_error(L, "attempt to runC null script");
  ཆེད་དུ (;;) {
    const var_na *inst = getstring;
    གལ་ཏེ EQ("") ཕྱིར 0;
    དེ་མིན གལ་ཏེ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    དེ་མིན གལ་ཏེ EQ("append") {
      ཧྲིལ t = getindex;
      ཧྲིལ i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    དེ་མིན གལ་ཏེ EQ("arith") {
      ཧྲིལ op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    དེ་མིན གལ་ཏེ EQ("call") {
      ཧྲིལ narg = getnum;
      ཧྲིལ nres = getnum;
      lua_call(L1, narg, nres);
    }
    དེ་མིན གལ་ཏེ EQ("callk") {
      ཧྲིལ narg = getnum;
      ཧྲིལ nres = getnum;
      ཧྲིལ i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    དེ་མིན གལ་ཏེ EQ("checkstack") {
      ཧྲིལ sz = getnum;
      const var_na *msg = getstring;
      གལ་ཏེ (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    དེ་མིན གལ་ཏེ EQ("rawcheckstack") {
      ཧྲིལ sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    དེ་མིན གལ་ཏེ EQ("compare") {
      const var_na *opt = getstring;  /* EQ, LT, or LE */
      ཧྲིལ op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ཧྲིལ a = getindex;
      ཧྲིལ b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    དེ་མིན གལ་ཏེ EQ("concat") {
      lua_concat(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("copy") {
      ཧྲིལ f = getindex;
      lua_copy(L1, f, getindex);
    }
    དེ་མིན གལ་ཏེ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    དེ་མིན གལ་ཏེ EQ("getfield") {
      ཧྲིལ t = getindex;
      ཧྲིལ tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    དེ་མིན གལ་ཏེ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    དེ་མིན གལ་ཏེ EQ("getmetatable") {
      གལ་ཏེ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    དེ་མིན གལ་ཏེ EQ("gettable") {
      ཧྲིལ tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    དེ་མིན གལ་ཏེ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    དེ་མིན གལ་ཏེ EQ("gsub") {
      ཧྲིལ a = getnum; ཧྲིལ b = getnum; ཧྲིལ c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    དེ་མིན གལ་ཏེ EQ("insert") {
      lua_insert(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("len") {
      lua_len(L1, getindex);
    }
    དེ་མིན གལ་ཏེ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    དེ་མིན གལ་ཏེ EQ("loadstring") {
      size_t slen;
      const var_na *s = luaL_checklstring(L1, getnum, &slen);
      const var_na *name = getstring;
      const var_na *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    དེ་མིན གལ་ཏེ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    དེ་མིན གལ་ཏེ EQ("newtable") {
      lua_newtable(L1);
    }
    དེ་མིན གལ་ཏེ EQ("newthread") {
      lua_newthread(L1);
    }
    དེ་མིན གལ་ཏེ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    དེ་མིན གལ་ཏེ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    དེ་མིན གལ་ཏེ EQ("next") {
      lua_next(L1, -2);
    }
    དེ་མིན གལ་ཏེ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    དེ་མིན གལ་ཏེ EQ("pcall") {
      ཧྲིལ narg = getnum;
      ཧྲིལ nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("pcallk") {
      ཧྲིལ narg = getnum;
      ཧྲིལ nres = getnum;
      ཧྲིལ i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    དེ་མིན གལ་ཏེ EQ("pop") {
      lua_pop(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("printstack") {
      ཧྲིལ n = getnum;
      གལ་ཏེ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      དེ་མིན lua_printstack(L1);
    }
    དེ་མིན གལ་ཏེ EQ("print") {
      const var_na *msg = getstring;
      printf("%s\n", msg);
    }
    དེ་མིན གལ་ཏེ EQ("warningC") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    དེ་མིན གལ་ཏེ EQ("warning") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    དེ་མིན གལ་ཏེ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("pushnil") {
      lua_pushnil(L1);
    }
    དེ་མིན གལ་ཏེ EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    དེ་མིན གལ་ཏེ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    དེ་མིན གལ་ཏེ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    དེ་མིན གལ་ཏེ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    དེ་མིན གལ་ཏེ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    དེ་མིན གལ་ཏེ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ཧྲིལ)lua_tointeger(L, -1));
    }
    དེ་མིན གལ་ཏེ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    དེ་མིན གལ་ཏེ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    དེ་མིན གལ་ཏེ EQ("rawget") {
      ཧྲིལ t = getindex;
      lua_rawget(L1, t);
    }
    དེ་མིན གལ་ཏེ EQ("rawgeti") {
      ཧྲིལ t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("rawgetp") {
      ཧྲིལ t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    དེ་མིན གལ་ཏེ EQ("rawset") {
      ཧྲིལ t = getindex;
      lua_rawset(L1, t);
    }
    དེ་མིན གལ་ཏེ EQ("rawseti") {
      ཧྲིལ t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("rawsetp") {
      ཧྲིལ t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    དེ་མིན གལ་ཏེ EQ("remove") {
      lua_remove(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("replace") {
      lua_replace(L1, getindex);
    }
    དེ་མིན གལ་ཏེ EQ("resume") {
      ཧྲིལ i = getindex;
      ཧྲིལ nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    དེ་མིན གལ་ཏེ EQ("traceback") {
      const var_na *msg = getstring;
      ཧྲིལ level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    དེ་མིན གལ་ཏེ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    དེ་མིན གལ་ཏེ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    དེ་མིན གལ་ཏེ EQ("return") {
      ཧྲིལ n = getnum;
      གལ་ཏེ (L1 != L) {
        ཧྲིལ i;
        ཆེད་དུ (i = 0; i < n; i++) {
          ཧྲིལ idx = -(n - i);
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
      ཕྱིར n;
    }
    དེ་མིན གལ་ཏེ EQ("rotate") {
      ཧྲིལ i = getindex;
      lua_rotate(L1, i, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("setfield") {
      ཧྲིལ t = getindex;
      const var_na *s = getstring;
      lua_setfield(L1, t, s);
    }
    དེ་མིན གལ་ཏེ EQ("seti") {
      ཧྲིལ t = getindex;
      lua_seti(L1, t, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("setglobal") {
      const var_na *s = getstring;
      lua_setglobal(L1, s);
    }
    དེ་མིན གལ་ཏེ EQ("sethook") {
      ཧྲིལ mask = getnum;
      ཧྲིལ count = getnum;
      const var_na *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    དེ་མིན གལ་ཏེ EQ("setmetatable") {
      ཧྲིལ idx = getindex;
      lua_setmetatable(L1, idx);
    }
    དེ་མིན གལ་ཏེ EQ("settable") {
      lua_settable(L1, getindex);
    }
    དེ་མིན གལ་ཏེ EQ("settop") {
      lua_settop(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("testudata") {
      ཧྲིལ i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    དེ་མིན གལ་ཏེ EQ("error") {
      lua_error(L1);
    }
    དེ་མིན གལ་ཏེ EQ("abort") {
      abort();
    }
    དེ་མིན གལ་ཏེ EQ("throw") {
#གལ་ཏེ defined(__cplusplus)
static sa_nracanaa X { ཧྲིལ x; } x;
      throw x;
#དེ་མིན
      luaL_error(L1, "C++");
#endif
      bha_nga;
    }
    དེ་མིན གལ་ཏེ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    དེ་མིན གལ་ཏེ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    དེ་མིན གལ་ཏེ EQ("tostring") {
      const var_na *s = lua_tostring(L1, getindex);
      const var_na *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    དེ་མིན གལ་ཏེ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    དེ་མིན གལ་ཏེ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    དེ་མིན གལ་ཏེ EQ("xmove") {
      ཧྲིལ f = getindex;
      ཧྲིལ t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ཧྲིལ n = getnum;
      གལ་ཏེ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    དེ་མིན གལ་ཏེ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    དེ་མིན གལ་ཏེ EQ("yield") {
      ཕྱིར lua_yield(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("yieldk") {
      ཧྲིལ nres = getnum;
      ཧྲིལ i = getindex;
      ཕྱིར lua_yieldk(L1, nres, i, Cfunck);
    }
    དེ་མིན གལ་ཏེ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    དེ་མིན གལ་ཏེ EQ("argerror") {
      ཧྲིལ arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    དེ་མིན luaL_error(L, "unknown instruction %s", buff);
  }
  ཕྱིར 0;
}


static ཧྲིལ testC (lua_State *L) {
  lua_State *L1;
  const var_na *pc;
  གལ་ཏེ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  དེ་མིན གལ་ཏེ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  དེ་མིན {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ཕྱིར runC(L, L1, pc);
}


static ཧྲིལ Cfunc (lua_State *L) {
  ཕྱིར runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static ཧྲིལ Cfunck (lua_State *L, ཧྲིལ status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ཕྱིར runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static ཧྲིལ makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ཕྱིར 1;
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
static void sethookaux (lua_State *L, ཧྲིལ mask, ཧྲིལ count, const var_na *scpt) {
  གལ་ཏེ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    ཕྱིར;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  གལ་ཏེ (!lua_istable(L, -1)) {  /* no hook table? */
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


static ཧྲིལ sethook (lua_State *L) {
  གལ་ཏེ (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  དེ་མིན {
    const var_na *scpt = luaL_checkstring(L, 1);
    const var_na *smask = luaL_checkstring(L, 2);
    ཧྲིལ count = cast_int(luaL_optinteger(L, 3, 0));
    ཧྲིལ mask = 0;
    གལ་ཏེ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    གལ་ཏེ (strchr(smask, 'r')) mask |= LUA_MASKRET;
    གལ་ཏེ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    གལ་ཏེ (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  ཕྱིར 0;
}


static ཧྲིལ coresume (lua_State *L) {
  ཧྲིལ status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  གལ་ཏེ (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ཕྱིར 2;  /* return false + error message */
  }
  དེ་མིན {
    lua_pushboolean(L, 1);
    ཕྱིར 1;
  }
}

#གལ་ཏེ !defined(LUA_USE_POSIX)

#define nonblock	NULL

#དེ་མིན

#include <unistd.h>
#include <fcntl.h>

static ཧྲིལ nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ཧྲིལ fd = fileno(f);
  ཧྲིལ flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ཕྱིར 0;
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


ཧྲིལ luaB_opentests (lua_State *L) {
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
  ཕྱིར 1;
}

#endif

