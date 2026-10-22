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
#အကယ်၍ defined(LUA_DEBUG)


void *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


static ကိန်းပြည့် runC (lua_State *L, lua_State *L1, const var_na *pc);


static void setnameval (lua_State *L, const var_na *name, ကိန်းပြည့် val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


static void pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top.p, o);
  api_incr_top(L);
}


static void badexit (const var_na *fmt, const var_na *s1, const var_na *s2) {
  fprintf(stderr, fmt, s1);
  အကယ်၍ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


static ကိန်းပြည့် tpanic (lua_State *L) {
  const var_na *msg = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ပြန်ပေး (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
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
static void warnf (void *ud, const var_na *msg, ကိန်းပြည့် tocont) {
  lua_State *L = cast(lua_State *, ud);
  static var_na buff[200] = "";  /* should be enough for tests... */
  static ကိန်းပြည့် onoff = 0;
  static ကိန်းပြည့် mode = 0;  /* start in normal mode */
  static ကိန်းပြည့် lasttocont = 0;
  အကယ်၍ (!lasttocont && !tocont && *msg == '@') {  /* control message? */
    အကယ်၍ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", msg, buff);
    အကယ်၍ (strcmp(msg, "@off") == 0)
      onoff = 0;
    မဟုတ်ပါက အကယ်၍ (strcmp(msg, "@on") == 0)
      onoff = 1;
    မဟုတ်ပါက အကယ်၍ (strcmp(msg, "@normal") == 0)
      mode = 0;
    မဟုတ်ပါက အကယ်၍ (strcmp(msg, "@allow") == 0)
      mode = 1;
    မဟုတ်ပါက အကယ်၍ (strcmp(msg, "@store") == 0)
      mode = 2;
    မဟုတ်ပါက
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              msg, NULL);
    ပြန်ပေး;
  }
  lasttocont = tocont;
  အကယ်၍ (strlen(msg) >= parimaa_na(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", msg, buff);
  strcat(buff, msg);  /* add new message to current warning */
  အကယ်၍ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    အကယ်၍ (!lua_toboolean(L, -1))
      lua_pop(L, 1);  /* ok, no previous unexpected warning */
    မဟုတ်ပါက {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    switch (mode) {
      pak_sa 0: {  /* normal */
        အကယ်၍ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, NULL);
      }  /* FALLTHROUGH */
      pak_sa 1: {  /* allow */
        အကယ်၍ (onoff)
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
    ကိန်းပြည့် type;
  } d;
} memHeader;


#အကယ်၍ !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#မဟုတ်ပါက

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


static void freeblock (Memcontrol *mc, memHeader *block) {
  အကယ်၍ (block) {
    size_t size = block->d.size;
    ကိန်းပြည့် i;
    အတွက် (i = 0; i < MARKSIZE; i++)  /* check marks after block */
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
  ကိန်းပြည့် type;
  အကယ်၍ (mc->memlimit == 0) {  /* first time? */
    var_na *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, NULL, 10) : ULONG_MAX;
  }
  အကယ်၍ (block == NULL) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  မဟုတ်ပါက {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  အကယ်၍ (size == 0) {
    freeblock(mc, block);
    ပြန်ပေး NULL;
  }
  အကယ်၍ (mc->failnext) {
    mc->failnext = 0;
    ပြန်ပေး NULL;  /* fake a single memory allocation error */
  }
  အကယ်၍ (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    အကယ်၍ (mc->countlimit == 0)
      ပြန်ပေး NULL;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  အကယ်၍ (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ပြန်ပေး NULL;  /* fake a memory allocation error */
  မဟုတ်ပါက {
    memHeader *newblock;
    ကိန်းပြည့် i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = parimaa_na(memHeader) + size + MARKSIZE;
    အကယ်၍ (realsize < size) ပြန်ပေး NULL;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    အကယ်၍ (newblock == NULL)
      ပြန်ပေး NULL;  /* really out of memory? */
    အကယ်၍ (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    အတွက် (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    အကယ်၍ (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ပြန်ပေး newblock + 1;
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
static ကိန်းပြည့် testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  အကယ်၍ (isdead(g,t)) ပြန်ပေး 0;
  အကယ်၍ (issweepphase(g))
    ပြန်ပေး 1;  /* no invariants */
  မဟုတ်ပါက အကယ်၍ (g->gckind != KGC_GENMINOR)
    ပြန်ပေး !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  မဟုတ်ပါက {  /* generational mode */
    အကယ်၍ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ပြန်ပေး 0;
    အကယ်၍ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ပြန်ပေး 0;
    ပြန်ပေး 1;
  }
}


static void printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (void *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  အကယ်၍ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
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


static ကိန်းပြည့် testobjref (global_State *g, GCObject *f, GCObject *t) {
  ကိန်းပြည့် r1 = testobjref1(g, f, t);
  အကယ်၍ (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ပြန်ပေး r1;
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
#define checkobjrefN(g,f,t)	{ အကယ်၍ (t) checkobjref(g,f,obj2gco(t)); }


static void checkvalref (global_State *g, GCObject *f, const TValue *t) {
  assert(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


static void checktable (global_State *g, Table *h) {
  unsigned ကိန်းပြည့် i;
  unsigned ကိန်းပြည့် asize = h->asize;
  Node *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  အတွက် (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  အတွက် (n = gnode(h, 0); n < limit; n++) {
    အကယ်၍ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      assert(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


static void checkudata (global_State *g, Udata *u) {
  ကိန်းပြည့် i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  အတွက် (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


static void checkproto (global_State *g, Proto *f) {
  ကိန်းပြည့် i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  အတွက် (i=0; i<f->sizek; i++) {
    အကယ်၍ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  အတွက် (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  အတွက် (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  အတွက် (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


static void checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ကိန်းပြည့် i;
  အတွက် (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


static void checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ကိန်းပြည့် i;
  checkobjrefN(g, clgc, cl->p);
  အတွက် (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    အကယ်၍ (uv) {
      checkobjrefN(g, clgc, uv);
      အကယ်၍ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


static ကိန်းပြည့် lua_checkpc (CallInfo *ci) {
  အကယ်၍ (!isLua(ci)) ပြန်ပေး 1;
  မဟုတ်ပါက {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ပြန်ပေး p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


static void check_stack (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  assert(!isdead(g, L1));
  အကယ်၍ (L1->stack.p == NULL) {  /* incomplete thread? */
    assert(L1->openupval == NULL && L1->ci == NULL);
    ပြန်ပေး;
  }
  အတွက် (uv = L1->openupval; uv != NULL; uv = uv->u.open.next)
    assert(upisopen(uv));  /* must be open */
  assert(L1->top.p <= L1->stack_last.p);
  assert(L1->tbclist.p <= L1->top.p);
  အတွက် (ci = L1->ci; ci != NULL; ci = ci->previous) {
    assert(ci->top.p <= L1->stack_last.p);
    assert(lua_checkpc(ci));
  }
  အတွက် (o = L1->stack.p; o < L1->stack_last.p; o++)
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
static void checkobject (global_State *g, GCObject *o, ကိန်းပြည့် maybedead,
                         ကိန်းပြည့် listage) {
  အကယ်၍ (isdead(g, o))
    assert(maybedead);
  မဟုတ်ပါက {
    assert(g->gcstate != GCSpause || iswhite(o));
    အကယ်၍ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      assert(getage(o) >= listage);
      အကယ်၍ (isold(o)) {
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
  ကိန်းပြည့် total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  နေစဉ် (o) {
    assert(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    assert(!testbit(o->marked, TESTBIT));
    အကယ်၍ (keepinvariant(g))
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
  ပြန်ပေး total;
}


/*
** Check objects in gray lists.
*/
static l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  အကယ်၍ (!keepinvariant(g)) ပြန်ပေး total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ပြန်ပေး total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
static void incifingray (global_State *g, GCObject *o, l_mem *count) {
  အကယ်၍ (!keepinvariant(g))
    ပြန်ပေး;  /* gray lists not being kept in these phases */
  အကယ်၍ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    assert(!isgray(o) || upisopen(gco2upv(o)));
    ပြန်ပေး;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  အကယ်၍ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*count)++;
    assert(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


static l_mem checklist (global_State *g, ကိန်းပြည့် maybedead, ကိန်းပြည့် tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  အတွက် (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  အတွက် (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  အတွက် (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  အတွက် (o = reallyold; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    assert(!tof == !tofinalize(o));
  }
  ပြန်ပေး total;
}


ကိန်းပြည့် lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ကိန်းပြည့် maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  အကယ်၍ (keepinvariant(g)) {
    assert(!iswhite(mainthread(g)));
    assert(!iswhite(gcvalue(&g->l_registry)));
  }
  assert(!isdead(g, gcvalue(&g->l_registry)));
  assert(g->sweepgc == NULL || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  အတွက် (o = g->fixedgc; o != NULL; o = o->next) {
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
  အတွက် (o = g->tobefnz; o != NULL; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    assert(tofinalize(o));
    assert(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  အကယ်၍ (keepinvariant(g))
    assert(totalin == totalshould);
  ပြန်ပေး 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


static var_na *buildop (Proto *p, ကိန်းပြည့် pc, var_na *buff) {
  var_na *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  const var_na *name = opnames[o];
  ကိန်းပြည့် line = luaG_getfuncline(p, pc);
  ကိန်းပြည့် lineinfo = (p->lineinfo != NULL) ? p->lineinfo[pc] : 0;
  အကယ်၍ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  မဟုတ်ပါက
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
  ပြန်ပေး obuff;
}


#အကယ်၍ 0
void luaI_printcode (Proto *pt, ကိန်းပြည့် size) {
  ကိန်းပြည့် pc;
  အတွက် (pc=0; pc<size; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


void luaI_printinst (Proto *pt, ကိန်းပြည့် pc) {
  var_na buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


static ကိန်းပြည့် listcode (lua_State *L) {
  ကိန်းပြည့် pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  အတွက် (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် printcode (lua_State *L) {
  ကိန်းပြည့် pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  အတွက် (pc=0; pc<p->sizecode; pc++) {
    var_na buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ပြန်ပေး 0;
}


static ကိန်းပြည့် listk (lua_State *L) {
  Proto *p;
  ကိန်းပြည့် i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  အတွက် (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် listabslineinfo (lua_State *L) {
  Proto *p;
  ကိန်းပြည့် i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != NULL, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  အတွက် (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].line);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် listlocals (lua_State *L) {
  Proto *p;
  ကိန်းပြည့် pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ကိန်းပြည့် i = 0;
  const var_na *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  နေစဉ် ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != NULL)
    lua_pushstring(L, name);
  ပြန်ပေး i-1;
}

/* }====================================================== */



void lua_printstack (lua_State *L) {
  ကိန်းပြည့် i;
  ကိန်းပြည့် n = lua_gettop(L);
  printf("stack: >>\n");
  အတွက် (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


ကိန်းပြည့် lua_printallstack (lua_State *L) {
  StkId p;
  ကိန်းပြည့် i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  အတွက် (p = L->stack.p; p < L->top.p; p++) {
    အကယ်၍ (ci != NULL && p == ci->func.p) {
      printf("  ---\n");
      အကယ်၍ (ci == L->ci)
        ci = NULL;  /* printed last frame */
      မဟုတ်ပါက
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ပြန်ပေး 0;
}


static ကိန်းပြည့် get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", NUM_OPCODES);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", parimaa_na(lua_State));
  setnameval(L, "global state", parimaa_na(global_State));
  setnameval(L, "TValue", parimaa_na(TValue));
  setnameval(L, "Node", parimaa_na(Node));
  setnameval(L, "stack Value", parimaa_na(StackValue));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် mem_query (lua_State *L) {
  အကယ်၍ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ပြန်ပေး 3;
  }
  မဟုတ်ပါက အကယ်၍ (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    အကယ်၍ (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ပြန်ပေး 0;
  }
  မဟုတ်ပါက {
    const var_na *t = luaL_checkstring(L, 1);
    ကိန်းပြည့် i;
    အတွက် (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      အကယ်၍ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ပြန်ပေး 1;
      }
    }
    ပြန်ပေး luaL_error(L, "unknown type '%s'", t);
  }
}


static ကိန်းပြည့် alloc_count (lua_State *L) {
  အကယ်၍ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  မဟုတ်ပါက
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ပြန်ပေး 0;
}


static ကိန်းပြည့် alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ပြန်ပေး 0;
}


static ကိန်းပြည့် settrick (lua_State *L) {
  အကယ်၍ (ttisnil(obj_at(L, 1)))
    l_Trick = NULL;
  မဟုတ်ပါက
    l_Trick = gcvalue(obj_at(L, 1));
  ပြန်ပေး 0;
}


static ကိန်းပြည့် gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  အကယ်၍ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  မဟုတ်ပါက {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  အကယ်၍ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  မဟုတ်ပါက {
    static const var_na *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  အကယ်၍ (!iscollectable(o))
    printf("no collectable\n");
  မဟုတ်ပါက {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ပြန်ပေး 0;
}


static const var_na *const statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

static ကိန်းပြည့် gc_state (lua_State *L) {
  static const ကိန်းပြည့် states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ကိန်းပြည့် option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  အကယ်၍ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ပြန်ပေး 1;
  }
  မဟုတ်ပါက {
    အကယ်၍ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    အကယ်၍ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ပြန်ပေး 0;
  }
}


static ကိန်းပြည့် tracinggc = 0;
void luai_tracegctest (lua_State *L, ကိန်းပြည့် first) {
  အကယ်၍ (!tracinggc) ပြန်ပေး;
  မဟုတ်ပါက {
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


static ကိန်းပြည့် tracegc (lua_State *L) {
  အကယ်၍ (lua_isnil(L, 1))
    tracinggc = 0;
  မဟုတ်ပါက {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ပြန်ပေး 0;
}


static ကိန်းပြည့် hash_query (lua_State *L) {
  အကယ်၍ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    အကယ်၍ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  မဟုတ်ပါက {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->node));
  }
  ပြန်ပေး 1;
}


static ကိန်းပြည့် stacklevel (lua_State *L) {
  ကိန်းပြည့် a = 0;
  lua_pushinteger(L, cast_Integer(L->top.p - L->stack.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ပြန်ပေး 5;
}


static ကိန်းပြည့် resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  နေစဉ် (ci->next != NULL) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ပြန်ပေး 0;
}


static ကိန်းပြည့် reallocstack (lua_State *L) {
  ကိန်းပြည့် n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->top.p - L->stack.p) + n, 1);
  lua_unlock(L);
  ပြန်ပေး 0;
}


static ကိန်းပြည့် table_query (lua_State *L) {
  const Table *t;
  ကိန်းပြည့် i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned ကိန်းပြည့် asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  အကယ်၍ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ပြန်ပေး 3;
  }
  မဟုတ်ပါက အကယ်၍ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    အကယ်၍ (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->top.p));
    မဟုတ်ပါက
      setnilvalue2s(L->top.p);
    api_incr_top(L);
    lua_pushnil(L);
  }
  မဟုတ်ပါက အကယ်၍ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    အကယ်၍ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    မဟုတ်ပါက
      lua_pushliteral(L, "<undef>");
    အကယ်၍ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    မဟုတ်ပါက
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->node[i]));
  }
  ပြန်ပေး 3;
}


static ကိန်းပြည့် gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ပြန်ပေး 6;
}


static ကိန်းပြည့် test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ကိန်းပြည့် s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  အကယ်၍ (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ပြန်ပေး 2;
  }
  မဟုတ်ပါက အကယ်၍ (s < tb->size) {
    TString *ts;
    ကိန်းပြည့် n = 0;
    အတွက် (ts = tb->hash[s]; ts != NULL; ts = ts->u.hnext) {
      setsvalue2s(L, L->top.p, ts);
      api_incr_top(L);
      n++;
    }
    ပြန်ပေး n;
  }
  မဟုတ်ပါက ပြန်ပေး 0;
}


static ကိန်းပြည့် getreftable (lua_State *L) {
  အကယ်၍ (lua_istable(L, 2))  /* is there a table as second argument? */
    ပြန်ပေး 2;  /* use it as the table */
  မဟုတ်ပါက
    ပြန်ပေး LUA_REGISTRYINDEX;  /* default is to use the register */
}


static ကိန်းပြည့် tref (lua_State *L) {
  ကိန်းပြည့် t = getreftable(L);
  ကိန်းပြည့် level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ပြန်ပေး 1;
}


static ကိန်းပြည့် getref (lua_State *L) {
  ကိန်းပြည့် t = getreftable(L);
  ကိန်းပြည့် level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ပြန်ပေး 1;
}

static ကိန်းပြည့် unref (lua_State *L) {
  ကိန်းပြည့် t = getreftable(L);
  ကိန်းပြည့် level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ပြန်ပေး 0;
}


static ကိန်းပြည့် upvalue (lua_State *L) {
  ကိန်းပြည့် n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  အကယ်၍ (lua_isnone(L, 3)) {
    const var_na *name = lua_getupvalue(L, 1, n);
    အကယ်၍ (name == NULL) ပြန်ပေး 0;
    lua_pushstring(L, name);
    ပြန်ပေး 2;
  }
  မဟုတ်ပါက {
    const var_na *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ပြန်ပေး 1;
  }
}


static ကိန်းပြည့် newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  ကိန်းပြည့် nuv = cast_int(luaL_optinteger(L, 2, 0));
  var_na *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  နေစဉ် (size--) *p++ = '\0';
  ပြန်ပေး 1;
}


static ကိန်းပြည့် pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  const var_na *s = luaL_checklstring(L, 1, &l);
  ကိန်းပြည့် status = luaL_loadbufferx(L1, s, l, s, "t");
  အကယ်၍ (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် s2d (lua_State *L) {
  lua_pushnumber(L, cast_num(*cast(const dviguu_na *, luaL_checkstring(L, 1))));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် d2s (lua_State *L) {
  dviguu_na d = cast(dviguu_na, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), parimaa_na(d));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ပြန်ပေး 1;
}


static ကိန်းပြည့် newstate (lua_State *L) {
  void *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  အကယ်၍ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  မဟုတ်ပါက
    lua_pushnil(L);
  ပြန်ပေး 1;
}


static lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != NULL, 1, "state expected");
  ပြန်ပေး L1;
}


static ကိန်းပြည့် loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ကိန်းပြည့် load = cast_int(luaL_checkinteger(L, 2));
  ကိန်းပြည့် preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", NULL, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ပြန်ပေး 0;
}

static ကိန်းပြည့် closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ပြန်ပေး 0;
}

static ကိန်းပြည့် doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  const var_na *code = luaL_checklstring(L, 2, &lcode);
  ကိန်းပြည့် status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  အကယ်၍ (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  အကယ်၍ (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ပြန်ပေး 3;
  }
  မဟုတ်ပါက {
    ကိန်းပြည့် i = 0;
    နေစဉ် (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_pop(L1, i-1);
    ပြန်ပေး i-1;
  }
}


static ကိန်းပြည့် log2_aux (lua_State *L) {
  unsigned ကိန်းပြည့် x = (unsigned ကိန်းပြည့်)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ပြန်ပေး 1;
}


sa_nracanaa Aux { jmp_buf jb; const var_na *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
static ကိန်းပြည့် panicback (lua_State *L) {
  sa_nracanaa Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (sa_nracanaa Aux *)lua_touserdata(L, -1);
  lua_pop(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ပြန်ပေး 1;  /* to avoid warnings */
}

static ကိန်းပြည့် checkpanic (lua_State *L) {
  sa_nracanaa Aux b;
  void *ud;
  lua_State *L1;
  const var_na *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  အကယ်၍ (L1 == NULL) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ပြန်ပေး 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  အကယ်၍ (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  မဟုတ်ပါက {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ပြန်ပေး 1;
}


static ကိန်းပြည့် externKstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, NULL, NULL);
  ပြန်ပေး 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
static ကိန်းပြည့် externstr (lua_State *L) {
  size_t len;
  const var_na *s = luaL_checklstring(L, 1, &len);
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  var_na *buff = cast_charp((*allocf)(ud, NULL, 0, len + 1));
  အကယ်၍ (buff == NULL) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * parimaa_na(var_na));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ပြန်ပေး 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


static void sethookaux (lua_State *L, ကိန်းပြည့် mask, ကိန်းပြည့် count, const var_na *code);

static const var_na *const delimits = " \t\n,;";

static void skip (const var_na **pc) {
  အတွက် (;;) {
    အကယ်၍ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    မဟုတ်ပါက အကယ်၍ (**pc == '#') {  /* comment? */
      နေစဉ် (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    မဟုတ်ပါက bha_nga;
  }
}

static ကိန်းပြည့် getnum_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  ကိန်းပြည့် res = 0;
  ကိန်းပြည့် sig = 1;
  skip(pc);
  အကယ်၍ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_pop(L1, 1);
    (*pc)++;
    ပြန်ပေး res;
  }
  မဟုတ်ပါက အကယ်၍ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ပြန်ပေး res;
  }
  မဟုတ်ပါက အကယ်၍ (**pc == '!') {
    (*pc)++;
    အကယ်၍ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    မဟုတ်ပါက အကယ်၍ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    မဟုတ်ပါက lua_assert(0);
    (*pc)++;
    ပြန်ပေး res;
  }
  မဟုတ်ပါက အကယ်၍ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  အကယ်၍ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  နေစဉ် (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ပြန်ပေး sig*res;
}

static const var_na *getstring_aux (lua_State *L, var_na *buff, const var_na **pc) {
  ကိန်းပြည့် i = 0;
  skip(pc);
  အကယ်၍ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ကိန်းပြည့် quote = *(*pc)++;
    နေစဉ် (**pc != quote) {
      အကယ်၍ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  မဟုတ်ပါက {
    နေစဉ် (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ပြန်ပေး buff;
}


static ကိန်းပြည့် getindex_aux (lua_State *L, lua_State *L1, const var_na **pc) {
  skip(pc);
  switch (*(*pc)++) {
    pak_sa 'R': ပြန်ပေး LUA_REGISTRYINDEX;
    pak_sa 'U': ပြန်ပေး lua_upvalueindex(getnum_aux(L, L1, pc));
    default: {
      ကိန်းပြည့် n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      အကယ်၍ (n == 0) ပြန်ပေး 0;
      မဟုတ်ပါက ပြန်ပေး lua_absindex(L1, n);
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
  unsigned ကိန်းပြည့် i;
  အတွက် (i = 0; i < parimaa_na(statcodes) / parimaa_na(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


static ကိန်းပြည့် testC (lua_State *L);
static ကိန်းပြည့် Cfunck (lua_State *L, ကိန်းပြည့် status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
static const var_na ops[] = "+-*%^/\\&|~<>_!";

static ကိန်းပြည့် runC (lua_State *L, lua_State *L1, const var_na *pc) {
  var_na buff[300];
  ကိန်းပြည့် status = 0;
  အကယ်၍ (pc == NULL) ပြန်ပေး luaL_error(L, "attempt to runC null script");
  အတွက် (;;) {
    const var_na *inst = getstring;
    အကယ်၍ EQ("") ပြန်ပေး 0;
    မဟုတ်ပါက အကယ်၍ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    မဟုတ်ပါက အကယ်၍ EQ("append") {
      ကိန်းပြည့် t = getindex;
      ကိန်းပြည့် i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("arith") {
      ကိန်းပြည့် op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    မဟုတ်ပါက အကယ်၍ EQ("call") {
      ကိန်းပြည့် narg = getnum;
      ကိန်းပြည့် nres = getnum;
      lua_call(L1, narg, nres);
    }
    မဟုတ်ပါက အကယ်၍ EQ("callk") {
      ကိန်းပြည့် narg = getnum;
      ကိန်းပြည့် nres = getnum;
      ကိန်းပြည့် i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    မဟုတ်ပါက အကယ်၍ EQ("checkstack") {
      ကိန်းပြည့် sz = getnum;
      const var_na *msg = getstring;
      အကယ်၍ (*msg == '\0')
        msg = NULL;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, msg);
    }
    မဟုတ်ပါက အကယ်၍ EQ("rawcheckstack") {
      ကိန်းပြည့် sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    မဟုတ်ပါက အကယ်၍ EQ("compare") {
      const var_na *opt = getstring;  /* EQ, LT, or LE */
      ကိန်းပြည့် op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ကိန်းပြည့် a = getindex;
      ကိန်းပြည့် b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    မဟုတ်ပါက အကယ်၍ EQ("concat") {
      lua_concat(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("copy") {
      ကိန်းပြည့် f = getindex;
      lua_copy(L1, f, getindex);
    }
    မဟုတ်ပါက အကယ်၍ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    မဟုတ်ပါက အကယ်၍ EQ("getfield") {
      ကိန်းပြည့် t = getindex;
      ကိန်းပြည့် tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    မဟုတ်ပါက အကယ်၍ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    မဟုတ်ပါက အကယ်၍ EQ("getmetatable") {
      အကယ်၍ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("gettable") {
      ကိန်းပြည့် tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    မဟုတ်ပါက အကယ်၍ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    မဟုတ်ပါက အကယ်၍ EQ("gsub") {
      ကိန်းပြည့် a = getnum; ကိန်းပြည့် b = getnum; ကိန်းပြည့် c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    မဟုတ်ပါက အကယ်၍ EQ("insert") {
      lua_insert(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("len") {
      lua_len(L1, getindex);
    }
    မဟုတ်ပါက အကယ်၍ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    မဟုတ်ပါက အကယ်၍ EQ("loadstring") {
      size_t slen;
      const var_na *s = luaL_checklstring(L1, getnum, &slen);
      const var_na *name = getstring;
      const var_na *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    မဟုတ်ပါက အကယ်၍ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    မဟုတ်ပါက အကယ်၍ EQ("newtable") {
      lua_newtable(L1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("newthread") {
      lua_newthread(L1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    မဟုတ်ပါက အကယ်၍ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    မဟုတ်ပါက အကယ်၍ EQ("next") {
      lua_next(L1, -2);
    }
    မဟုတ်ပါက အကယ်၍ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    မဟုတ်ပါက အကယ်၍ EQ("pcall") {
      ကိန်းပြည့် narg = getnum;
      ကိန်းပြည့် nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pcallk") {
      ကိန်းပြည့် narg = getnum;
      ကိန်းပြည့် nres = getnum;
      ကိန်းပြည့် i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pop") {
      lua_pop(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("printstack") {
      ကိန်းပြည့် n = getnum;
      အကယ်၍ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      မဟုတ်ပါက lua_printstack(L1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("print") {
      const var_na *msg = getstring;
      printf("%s\n", msg);
    }
    မဟုတ်ပါက အကယ်၍ EQ("warningC") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("warning") {
      const var_na *msg = getstring;
      lua_warning(L1, msg, 0);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushnil") {
      lua_pushnil(L1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushnum") {
      lua_pushnumber(L1, (lua_Number)getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ကိန်းပြည့်)lua_tointeger(L, -1));
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    မဟုတ်ပါက အကယ်၍ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    မဟုတ်ပါက အကယ်၍ EQ("rawget") {
      ကိန်းပြည့် t = getindex;
      lua_rawget(L1, t);
    }
    မဟုတ်ပါက အကယ်၍ EQ("rawgeti") {
      ကိန်းပြည့် t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("rawgetp") {
      ကိန်းပြည့် t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    မဟုတ်ပါက အကယ်၍ EQ("rawset") {
      ကိန်းပြည့် t = getindex;
      lua_rawset(L1, t);
    }
    မဟုတ်ပါက အကယ်၍ EQ("rawseti") {
      ကိန်းပြည့် t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("rawsetp") {
      ကိန်းပြည့် t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    မဟုတ်ပါက အကယ်၍ EQ("remove") {
      lua_remove(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("replace") {
      lua_replace(L1, getindex);
    }
    မဟုတ်ပါက အကယ်၍ EQ("resume") {
      ကိန်းပြည့် i = getindex;
      ကိန်းပြည့် nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    မဟုတ်ပါက အကယ်၍ EQ("traceback") {
      const var_na *msg = getstring;
      ကိန်းပြည့် level = getnum;
      luaL_traceback(L1, L1, msg, level);
    }
    မဟုတ်ပါက အကယ်၍ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    မဟုတ်ပါက အကယ်၍ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("return") {
      ကိန်းပြည့် n = getnum;
      အကယ်၍ (L1 != L) {
        ကိန်းပြည့် i;
        အတွက် (i = 0; i < n; i++) {
          ကိန်းပြည့် idx = -(n - i);
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
      ပြန်ပေး n;
    }
    မဟုတ်ပါက အကယ်၍ EQ("rotate") {
      ကိန်းပြည့် i = getindex;
      lua_rotate(L1, i, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("setfield") {
      ကိန်းပြည့် t = getindex;
      const var_na *s = getstring;
      lua_setfield(L1, t, s);
    }
    မဟုတ်ပါက အကယ်၍ EQ("seti") {
      ကိန်းပြည့် t = getindex;
      lua_seti(L1, t, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("setglobal") {
      const var_na *s = getstring;
      lua_setglobal(L1, s);
    }
    မဟုတ်ပါက အကယ်၍ EQ("sethook") {
      ကိန်းပြည့် mask = getnum;
      ကိန်းပြည့် count = getnum;
      const var_na *s = getstring;
      sethookaux(L1, mask, count, s);
    }
    မဟုတ်ပါက အကယ်၍ EQ("setmetatable") {
      ကိန်းပြည့် idx = getindex;
      lua_setmetatable(L1, idx);
    }
    မဟုတ်ပါက အကယ်၍ EQ("settable") {
      lua_settable(L1, getindex);
    }
    မဟုတ်ပါက အကယ်၍ EQ("settop") {
      lua_settop(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("testudata") {
      ကိန်းပြည့် i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != NULL);
    }
    မဟုတ်ပါက အကယ်၍ EQ("error") {
      lua_error(L1);
    }
    မဟုတ်ပါက အကယ်၍ EQ("abort") {
      abort();
    }
    မဟုတ်ပါက အကယ်၍ EQ("throw") {
#အကယ်၍ defined(__cplusplus)
static sa_nracanaa X { ကိန်းပြည့် x; } x;
      throw x;
#မဟုတ်ပါက
      luaL_error(L1, "C++");
#endif
      bha_nga;
    }
    မဟုတ်ပါက အကယ်၍ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    မဟုတ်ပါက အကယ်၍ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    မဟုတ်ပါက အကယ်၍ EQ("tostring") {
      const var_na *s = lua_tostring(L1, getindex);
      const var_na *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == NULL && s1 == NULL) || strcmp(s, s1) == 0);
    }
    မဟုတ်ပါက အကယ်၍ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, NULL);
    }
    မဟုတ်ပါက အကယ်၍ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    မဟုတ်ပါက အကယ်၍ EQ("xmove") {
      ကိန်းပြည့် f = getindex;
      ကိန်းပြည့် t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ကိန်းပြည့် n = getnum;
      အကယ်၍ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    မဟုတ်ပါက အကယ်၍ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    မဟုတ်ပါက အကယ်၍ EQ("yield") {
      ပြန်ပေး lua_yield(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("yieldk") {
      ကိန်းပြည့် nres = getnum;
      ကိန်းပြည့် i = getindex;
      ပြန်ပေး lua_yieldk(L1, nres, i, Cfunck);
    }
    မဟုတ်ပါက အကယ်၍ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    မဟုတ်ပါက အကယ်၍ EQ("argerror") {
      ကိန်းပြည့် arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    မဟုတ်ပါက luaL_error(L, "unknown instruction %s", buff);
  }
  ပြန်ပေး 0;
}


static ကိန်းပြည့် testC (lua_State *L) {
  lua_State *L1;
  const var_na *pc;
  အကယ်၍ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  မဟုတ်ပါက အကယ်၍ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  မဟုတ်ပါက {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ပြန်ပေး runC(L, L1, pc);
}


static ကိန်းပြည့် Cfunc (lua_State *L) {
  ပြန်ပေး runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


static ကိန်းပြည့် Cfunck (lua_State *L, ကိန်းပြည့် status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ပြန်ပေး runC(L, L, lua_tostring(L, cast_int(ctx)));
}


static ကိန်းပြည့် makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ပြန်ပေး 1;
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
static void sethookaux (lua_State *L, ကိန်းပြည့် mask, ကိန်းပြည့် count, const var_na *scpt) {
  အကယ်၍ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
    ပြန်ပေး;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  အကယ်၍ (!lua_istable(L, -1)) {  /* no hook table? */
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


static ကိန်းပြည့် sethook (lua_State *L) {
  အကယ်၍ (lua_isnoneornil(L, 1))
    lua_sethook(L, NULL, 0, 0);  /* turn off hooks */
  မဟုတ်ပါက {
    const var_na *scpt = luaL_checkstring(L, 1);
    const var_na *smask = luaL_checkstring(L, 2);
    ကိန်းပြည့် count = cast_int(luaL_optinteger(L, 3, 0));
    ကိန်းပြည့် mask = 0;
    အကယ်၍ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    အကယ်၍ (strchr(smask, 'r')) mask |= LUA_MASKRET;
    အကယ်၍ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    အကယ်၍ (count > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, count, scpt);
  }
  ပြန်ပေး 0;
}


static ကိန်းပြည့် coresume (lua_State *L) {
  ကိန်းပြည့် status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  အကယ်၍ (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_insert(L, -2);
    ပြန်ပေး 2;  /* return false + error message */
  }
  မဟုတ်ပါက {
    lua_pushboolean(L, 1);
    ပြန်ပေး 1;
  }
}

#အကယ်၍ !defined(LUA_USE_POSIX)

#define nonblock	NULL

#မဟုတ်ပါက

#include <unistd.h>
#include <fcntl.h>

static ကိန်းပြည့် nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ကိန်းပြည့် fd = fileno(f);
  ကိန်းပြည့် flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ပြန်ပေး 0;
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


ကိန်းပြည့် luaB_opentests (lua_State *L) {
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
  ပြန်ပေး 1;
}

#endif

