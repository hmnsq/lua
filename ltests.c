/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <limits.h>
#ضمن <setjmp.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "lauxlib.h"
#ضمن "lcode.h"
#ضمن "lctype.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "lmem.h"
#ضمن "lopcodes.h"
#ضمن "lopnames.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#إذا defined(LUA_DEBUG)


فراغ *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


ساكن صحيح runC (lua_State *L, lua_State *L1, ثابت حرف *pc);


ساكن فراغ setnameval (lua_State *L, ثابت حرف *name, صحيح val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


ساكن فراغ pushobject (lua_State *L, ثابت TValue *o) {
  setobj2s(L, L->كولي.p, o);
  api_incr_كولي(L);
}


ساكن فراغ badexit (ثابت حرف *fmt, ثابت حرف *s1, ثابت حرف *s2) {
  fprintf(stderr, fmt, s1);
  إذا (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


ساكن صحيح tpanic (lua_State *L) {
  ثابت حرف *سكو = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  أرجع (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   سكو, عدم),
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
ساكن فراغ warnf (فراغ *ud, ثابت حرف *سكو, صحيح tocont) {
  lua_State *L = cast(lua_State *, ud);
  ساكن حرف buff[200] = "";  /* should be enough for tests... */
  ساكن صحيح onoff = 0;
  ساكن صحيح mode = 0;  /* start in normal mode */
  ساكن صحيح lasttocont = 0;
  إذا (!lasttocont && !tocont && *سكو == '@') {  /* control message? */
    إذا (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", سكو, buff);
    إذا (strcmp(سكو, "@off") == 0)
      onoff = 0;
    وإلا إذا (strcmp(سكو, "@on") == 0)
      onoff = 1;
    وإلا إذا (strcmp(سكو, "@normal") == 0)
      mode = 0;
    وإلا إذا (strcmp(سكو, "@allow") == 0)
      mode = 1;
    وإلا إذا (strcmp(سكو, "@store") == 0)
      mode = 2;
    وإلا
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              سكو, عدم);
    أرجع;
  }
  lasttocont = tocont;
  إذا (strlen(سكو) >= حجم(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", سكو, buff);
  strcat(buff, سكو);  /* add new message to current warning */
  إذا (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    إذا (!lua_toboolean(L, -1))
      lua_چرو(L, 1);  /* ok, no previous unexpected warning */
    وإلا {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    بدل (mode) {
      حالة 0: {  /* normal */
        إذا (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, عدم);
      }  /* FALLTHROUGH */
      حالة 1: {  /* allow */
        إذا (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        اكسر;
      }
      حالة 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        اكسر;
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

تعريفنوع اتحاد memHeader {
  LUAI_MAXALIGN;
  بنية {
    size_t size;
    صحيح type;
  } d;
} memHeader;


#إذا !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#وإلا

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


ساكن فراغ freeblock (Memcontrol *mc, memHeader *block) {
  إذا (block) {
    size_t size = block->d.size;
    صحيح i;
    لكل (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, حجم(memHeader) + size + MARKSIZE);  /* erase block */
    حرر(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


فراغ *debug_realloc (فراغ *ud, فراغ *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  صحيح type;
  إذا (mc->memlimit == 0) {  /* first time? */
    حرف *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, عدم, 10) : ULONG_MAX;
  }
  إذا (block == عدم) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  وإلا {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  إذا (size == 0) {
    freeblock(mc, block);
    أرجع عدم;
  }
  إذا (mc->failnext) {
    mc->failnext = 0;
    أرجع عدم;  /* fake a single memory allocation error */
  }
  إذا (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    إذا (mc->countlimit == 0)
      أرجع عدم;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  إذا (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    أرجع عدم;  /* fake a memory allocation error */
  وإلا {
    memHeader *newblock;
    صحيح i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = حجم(memHeader) + size + MARKSIZE;
    إذا (realsize < size) أرجع عدم;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    إذا (newblock == عدم)
      أرجع عدم;  /* really out of memory? */
    إذا (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    لكل (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    إذا (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    أرجع newblock + 1;
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

#ضمن <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
ساكن صحيح testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  إذا (isdead(g,t)) أرجع 0;
  إذا (issweepphase(g))
    أرجع 1;  /* no invariants */
  وإلا إذا (g->gckind != KGC_GENMINOR)
    أرجع !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  وإلا {  /* generational mode */
    إذا ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      أرجع 0;
    إذا ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      أرجع 0;
    أرجع 1;
  }
}


ساكن فراغ printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (فراغ *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  إذا (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


فراغ lua_printobj (lua_State *L, بنية GCObject *o) {
  printobj(G(L), o);
}


فراغ lua_printvalue (TValue *v) {
  بدل (ttypetag(v)) {
    حالة LUA_VNUMINT: حالة LUA_VNUMFLT: {
      حرف buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      اكسر;
    }
    حالة LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); اكسر;
    حالة LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); اكسر;
    حالة LUA_VFALSE:
      printf("%s", "false"); اكسر;
    حالة LUA_VTRUE:
      printf("%s", "true"); اكسر;
    حالة LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); اكسر;
    حالة LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); اكسر;
    حالة LUA_VNIL:
      printf("nil"); اكسر;
    حالة LUA_VLCF:
      printf("light C function: %p", fvalue(v)); اكسر;
    حالة LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); اكسر;
    حالة LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); اكسر;
    حالة LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); اكسر;
    حالة LUA_VTABLE:
      printf("table: %p", hvalue(v)); اكسر;
    افتراضي:
      lua_assert(0);
  }
}


ساكن صحيح testobjref (global_State *g, GCObject *f, GCObject *t) {
  صحيح r1 = testobjref1(g, f, t);
  إذا (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  أرجع r1;
}


ساكن فراغ checkobjref (global_State *g, GCObject *f, GCObject *t) {
    أكد(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ إذا (t) checkobjref(g,f,obj2gco(t)); }


ساكن فراغ checkvalref (global_State *g, GCObject *f, ثابت TValue *t) {
  أكد(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


ساكن فراغ checktable (global_State *g, Table *h) {
  unsigned صحيح i;
  unsigned صحيح asize = h->asize;
  كلي_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  لكل (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  لكل (n = gnode(h, 0); n < limit; n++) {
    إذا (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      أكد(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


ساكن فراغ checkudata (global_State *g, Udata *u) {
  صحيح i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  لكل (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


ساكن فراغ checkproto (global_State *g, Proto *f) {
  صحيح i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  لكل (i=0; i<f->sizek; i++) {
    إذا (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  لكل (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  لكل (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  لكل (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


ساكن فراغ checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  صحيح i;
  لكل (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


ساكن فراغ checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  صحيح i;
  checkobjrefN(g, clgc, cl->p);
  لكل (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    إذا (uv) {
      checkobjrefN(g, clgc, uv);
      إذا (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


ساكن صحيح lua_checkpc (CallInfo *ci) {
  إذا (!isLua(ci)) أرجع 1;
  وإلا {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    أرجع p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


ساكن فراغ check_تر (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  أكد(!isdead(g, L1));
  إذا (L1->تر.p == عدم) {  /* incomplete thread? */
    أكد(L1->openupval == عدم && L1->ci == عدم);
    أرجع;
  }
  لكل (uv = L1->openupval; uv != عدم; uv = uv->u.open.next)
    أكد(upisopen(uv));  /* must be open */
  أكد(L1->كولي.p <= L1->تر_last.p);
  أكد(L1->tbclist.p <= L1->كولي.p);
  لكل (ci = L1->ci; ci != عدم; ci = ci->previous) {
    أكد(ci->كولي.p <= L1->تر_last.p);
    أكد(lua_checkpc(ci));
  }
  لكل (o = L1->تر.p; o < L1->تر_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


ساكن فراغ checkrefs (global_State *g, GCObject *o) {
  بدل (o->tt) {
    حالة LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      اكسر;
    }
    حالة LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      اكسر;
    }
    حالة LUA_VTABLE: {
      checktable(g, gco2t(o));
      اكسر;
    }
    حالة LUA_VTHREAD: {
      check_تر(g, gco2th(o));
      اكسر;
    }
    حالة LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      اكسر;
    }
    حالة LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      اكسر;
    }
    حالة LUA_VPROTO: {
      checkproto(g, gco2p(o));
      اكسر;
    }
    حالة LUA_VSHRSTR:
    حالة LUA_VLNGSTR: {
      أكد(!isgray(o));  /* strings are never gray */
      اكسر;
    }
    افتراضي: أكد(0);
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
ساكن فراغ checkobject (global_State *g, GCObject *o, صحيح maybedead,
                         صحيح listage) {
  إذا (isdead(g, o))
    أكد(maybedead);
  وإلا {
    أكد(g->gcstate != GCSpause || iswhite(o));
    إذا (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      أكد(getage(o) >= listage);
      إذا (isold(o)) {
        أكد(!iswhite(o));
        أكد(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      أكد(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


ساكن l_mem checkgraylist (global_State *g, GCObject *o) {
  صحيح total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  طالما (o) {
    أكد(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    أكد(!testbit(o->marked, TESTBIT));
    إذا (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    بدل (o->tt) {
      حالة LUA_VTABLE: o = gco2t(o)->gclist; اكسر;
      حالة LUA_VLCL: o = gco2lcl(o)->gclist; اكسر;
      حالة LUA_VCCL: o = gco2ccl(o)->gclist; اكسر;
      حالة LUA_VTHREAD: o = gco2th(o)->gclist; اكسر;
      حالة LUA_VPROTO: o = gco2p(o)->gclist; اكسر;
      حالة LUA_VUSERDATA:
        أكد(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        اكسر;
      افتراضي: أكد(0);  /* other objects cannot be in a gray list */
    }
  }
  أرجع total;
}


/*
** Check objects in gray lists.
*/
ساكن l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  إذا (!keepinvariant(g)) أرجع total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  أرجع total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
ساكن فراغ incifingray (global_State *g, GCObject *o, l_mem *كدي) {
  إذا (!keepinvariant(g))
    أرجع;  /* gray lists not being kept in these phases */
  إذا (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    أكد(!isgray(o) || upisopen(gco2upv(o)));
    أرجع;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  إذا (isgray(o) || getage(o) == G_TOUCHED2) {
    (*كدي)++;
    أكد(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


ساكن l_mem checklist (global_State *g, صحيح maybedead, صحيح tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  لكل (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  لكل (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  لكل (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  لكل (o = reallyold; o != عدم; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  أرجع total;
}


صحيح lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  صحيح maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  إذا (keepinvariant(g)) {
    أكد(!iswhite(mainthread(g)));
    أكد(!iswhite(gcvalue(&g->l_registry)));
  }
  أكد(!isdead(g, gcvalue(&g->l_registry)));
  أكد(g->sweepgc == عدم || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  لكل (o = g->fixedgc; o != عدم; o = o->next) {
    أكد(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  لكل (o = g->tobefnz; o != عدم; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    أكد(tofinalize(o));
    أكد(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  إذا (keepinvariant(g))
    أكد(totalin == totalshould);
  أرجع 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


ساكن حرف *buildop (Proto *p, صحيح pc, حرف *buff) {
  حرف *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  ثابت حرف *name = opnames[o];
  صحيح لي = luaG_getfuncline(p, pc);
  صحيح lineinfo = (p->lineinfo != عدم) ? p->lineinfo[pc] : 0;
  إذا (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  وإلا
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", لي, pc);
  بدل (getOpMode(o)) {
    حالة iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      اكسر;
    حالة ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      اكسر;
    حالة iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      اكسر;
    حالة iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      اكسر;
    حالة iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      اكسر;
    حالة isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      اكسر;
  }
  أرجع obuff;
}


#إذا 0
فراغ luaI_printcode (Proto *pt, صحيح size) {
  صحيح pc;
  لكل (pc=0; pc<size; pc++) {
    حرف buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


فراغ luaI_printinst (Proto *pt, صحيح pc) {
  حرف buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


ساكن صحيح listcode (lua_State *L) {
  صحيح pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  لكل (pc=0; pc<p->sizecode; pc++) {
    حرف buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  أرجع 1;
}


ساكن صحيح printcode (lua_State *L) {
  صحيح pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  لكل (pc=0; pc<p->sizecode; pc++) {
    حرف buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  أرجع 0;
}


ساكن صحيح listk (lua_State *L) {
  Proto *p;
  صحيح i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  لكل (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  أرجع 1;
}


ساكن صحيح listabslineinfo (lua_State *L) {
  Proto *p;
  صحيح i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != عدم, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  لكل (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].لي);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  أرجع 1;
}


ساكن صحيح listlocals (lua_State *L) {
  Proto *p;
  صحيح pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  صحيح i = 0;
  ثابت حرف *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  طالما ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != عدم)
    lua_pushstring(L, name);
  أرجع i-1;
}

/* }====================================================== */



فراغ lua_printstack (lua_State *L) {
  صحيح i;
  صحيح n = lua_gettop(L);
  printf("stack: >>\n");
  لكل (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


صحيح lua_printallstack (lua_State *L) {
  StkId p;
  صحيح i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  لكل (p = L->تر.p; p < L->كولي.p; p++) {
    إذا (ci != عدم && p == ci->func.p) {
      printf("  ---\n");
      إذا (ci == L->ci)
        ci = عدم;  /* printed last frame */
      وإلا
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  أرجع 0;
}


ساكن صحيح get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", لمب_OPCODES);
  أرجع 1;
}


ساكن صحيح get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", حجم(lua_State));
  setnameval(L, "global state", حجم(global_State));
  setnameval(L, "TValue", حجم(TValue));
  setnameval(L, "Node", حجم(كلي_2));
  setnameval(L, "stack Value", حجم(تر_درج));
  أرجع 1;
}


ساكن صحيح mem_query (lua_State *L) {
  إذا (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    أرجع 3;
  }
  وإلا إذا (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    إذا (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    أرجع 0;
  }
  وإلا {
    ثابت حرف *t = luaL_checkstring(L, 1);
    صحيح i;
    لكل (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      إذا (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        أرجع 1;
      }
    }
    أرجع luaL_error(L, "unknown type '%s'", t);
  }
}


ساكن صحيح alloc_كدي (lua_State *L) {
  إذا (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  وإلا
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  أرجع 0;
}


ساكن صحيح alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  أرجع 0;
}


ساكن صحيح settrick (lua_State *L) {
  إذا (ttisnil(obj_at(L, 1)))
    l_Trick = عدم;
  وإلا
    l_Trick = gcvalue(obj_at(L, 1));
  أرجع 0;
}


ساكن صحيح gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  إذا (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  وإلا {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  أرجع 1;
}


ساكن صحيح gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  إذا (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  وإلا {
    ساكن ثابت حرف *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  أرجع 1;
}


ساكن صحيح gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  إذا (!iscollectable(o))
    printf("no collectable\n");
  وإلا {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  أرجع 0;
}


ساكن ثابت حرف *ثابت statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

ساكن صحيح gc_state (lua_State *L) {
  ساكن ثابت صحيح states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  صحيح option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  إذا (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    أرجع 1;
  }
  وإلا {
    إذا (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    إذا (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    أرجع 0;
  }
}


ساكن صحيح tracinggc = 0;
فراغ luai_tracegctest (lua_State *L, صحيح first) {
  إذا (!tracinggc) أرجع;
  وإلا {
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


ساكن صحيح tracegc (lua_State *L) {
  إذا (lua_isnil(L, 1))
    tracinggc = 0;
  وإلا {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  أرجع 0;
}


ساكن صحيح hash_query (lua_State *L) {
  إذا (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    إذا (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  وإلا {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->كلي));
  }
  أرجع 1;
}


ساكن صحيح stacklevel (lua_State *L) {
  صحيح a = 0;
  lua_pushinteger(L, cast_Integer(L->كولي.p - L->تر.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  أرجع 5;
}


ساكن صحيح resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  طالما (ci->next != عدم) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  أرجع 0;
}


ساكن صحيح reallocstack (lua_State *L) {
  صحيح n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->كولي.p - L->تر.p) + n, 1);
  lua_unlock(L);
  أرجع 0;
}


ساكن صحيح table_query (lua_State *L) {
  ثابت Table *t;
  صحيح i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned صحيح asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  إذا (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    أرجع 3;
  }
  وإلا إذا (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    إذا (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->كولي.p));
    وإلا
      setnilvalue2s(L->كولي.p);
    api_incr_كولي(L);
    lua_pushnil(L);
  }
  وإلا إذا (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    إذا (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    وإلا
      lua_pushliteral(L, "<undef>");
    إذا (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    وإلا
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->كلي[i]));
  }
  أرجع 3;
}


ساكن صحيح gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  أرجع 6;
}


ساكن صحيح test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  أرجع 1;
}


ساكن صحيح test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  أرجع 1;
}


ساكن صحيح string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  صحيح s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  إذا (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    أرجع 2;
  }
  وإلا إذا (s < tb->size) {
    TString *ts;
    صحيح n = 0;
    لكل (ts = tb->hash[s]; ts != عدم; ts = ts->u.hnext) {
      setsvalue2s(L, L->كولي.p, ts);
      api_incr_كولي(L);
      n++;
    }
    أرجع n;
  }
  وإلا أرجع 0;
}


ساكن صحيح getreftable (lua_State *L) {
  إذا (lua_istable(L, 2))  /* is there a table as second argument? */
    أرجع 2;  /* use it as the table */
  وإلا
    أرجع LUA_REGISTRYINDEX;  /* default is to use the register */
}


ساكن صحيح tref (lua_State *L) {
  صحيح t = getreftable(L);
  صحيح level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  أرجع 1;
}


ساكن صحيح getref (lua_State *L) {
  صحيح t = getreftable(L);
  صحيح level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  أرجع 1;
}

ساكن صحيح unref (lua_State *L) {
  صحيح t = getreftable(L);
  صحيح level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  أرجع 0;
}


ساكن صحيح upvalue (lua_State *L) {
  صحيح n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  إذا (lua_isnone(L, 3)) {
    ثابت حرف *name = lua_getupvalue(L, 1, n);
    إذا (name == عدم) أرجع 0;
    lua_pushstring(L, name);
    أرجع 2;
  }
  وإلا {
    ثابت حرف *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    أرجع 1;
  }
}


ساكن صحيح newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  صحيح nuv = cast_int(luaL_optinteger(L, 2, 0));
  حرف *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  طالما (size--) *p++ = '\0';
  أرجع 1;
}


ساكن صحيح pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  أرجع 1;
}


ساكن صحيح udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  أرجع 1;
}


ساكن صحيح doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  صحيح status = luaL_loadbufferx(L1, s, l, s, "t");
  إذا (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  أرجع 1;
}


ساكن صحيح s2d (lua_State *L) {
  lua_pushnumber(L, cast_لمب(*cast(ثابت مزدوج *, luaL_checkstring(L, 1))));
  أرجع 1;
}


ساكن صحيح d2s (lua_State *L) {
  مزدوج d = cast(مزدوج, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), حجم(d));
  أرجع 1;
}


ساكن صحيح num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  أرجع 1;
}


ساكن صحيح makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  أرجع 1;
}


ساكن صحيح newstate (lua_State *L) {
  فراغ *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  إذا (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  وإلا
    lua_pushnil(L);
  أرجع 1;
}


ساكن lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != عدم, 1, "state expected");
  أرجع L1;
}


ساكن صحيح loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  صحيح load = cast_int(luaL_checkinteger(L, 2));
  صحيح preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", عدم, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  أرجع 0;
}

ساكن صحيح closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  أرجع 0;
}

ساكن صحيح doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ثابت حرف *code = luaL_checklstring(L, 2, &lcode);
  صحيح status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  إذا (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  إذا (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    أرجع 3;
  }
  وإلا {
    صحيح i = 0;
    طالما (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_چرو(L1, i-1);
    أرجع i-1;
  }
}


ساكن صحيح log2_aux (lua_State *L) {
  unsigned صحيح x = (unsigned صحيح)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  أرجع 1;
}


بنية Aux { jmp_buf jb; ثابت حرف *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
ساكن صحيح panicback (lua_State *L) {
  بنية Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (بنية Aux *)lua_touserdata(L, -1);
  lua_چرو(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  أرجع 1;  /* to avoid warnings */
}

ساكن صحيح checkpanic (lua_State *L) {
  بنية Aux b;
  فراغ *ud;
  lua_State *L1;
  ثابت حرف *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  إذا (L1 == عدم) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    أرجع 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  إذا (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  وإلا {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  أرجع 1;
}


ساكن صحيح externKstr (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, عدم, عدم);
  أرجع 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
ساكن صحيح externstr (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  فراغ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  حرف *buff = cast_charp((*allocf)(ud, عدم, 0, len + 1));
  إذا (buff == عدم) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * حجم(حرف));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  أرجع 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


ساكن فراغ sethookaux (lua_State *L, صحيح mask, صحيح كدي, ثابت حرف *code);

ساكن ثابت حرف *ثابت delimits = " \t\n,;";

ساكن فراغ skip (ثابت حرف **pc) {
  لكل (;;) {
    إذا (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    وإلا إذا (**pc == '#') {  /* comment? */
      طالما (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    وإلا اكسر;
  }
}

ساكن صحيح getnum_aux (lua_State *L, lua_State *L1, ثابت حرف **pc) {
  صحيح res = 0;
  صحيح sig = 1;
  skip(pc);
  إذا (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_چرو(L1, 1);
    (*pc)++;
    أرجع res;
  }
  وإلا إذا (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    أرجع res;
  }
  وإلا إذا (**pc == '!') {
    (*pc)++;
    إذا (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    وإلا إذا (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    وإلا lua_assert(0);
    (*pc)++;
    أرجع res;
  }
  وإلا إذا (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  إذا (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  طالما (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  أرجع sig*res;
}

ساكن ثابت حرف *getstring_aux (lua_State *L, حرف *buff, ثابت حرف **pc) {
  صحيح i = 0;
  skip(pc);
  إذا (**pc == '"' || **pc == '\'') {  /* quoted string? */
    صحيح quote = *(*pc)++;
    طالما (**pc != quote) {
      إذا (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  وإلا {
    طالما (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  أرجع buff;
}


ساكن صحيح getindex_aux (lua_State *L, lua_State *L1, ثابت حرف **pc) {
  skip(pc);
  بدل (*(*pc)++) {
    حالة 'R': أرجع LUA_REGISTRYINDEX;
    حالة 'U': أرجع lua_upvalueindex(getnum_aux(L, L1, pc));
    افتراضي: {
      صحيح n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      إذا (n == 0) أرجع 0;
      وإلا أرجع lua_absindex(L1, n);
    }
  }
}


ساكن ثابت حرف *ثابت statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
ساكن فراغ regcodes (lua_State *L) {
  unsigned صحيح i;
  لكل (i = 0; i < حجم(statcodes) / حجم(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


ساكن صحيح testC (lua_State *L);
ساكن صحيح Cfunck (lua_State *L, صحيح status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
ساكن ثابت حرف ops[] = "+-*%^/\\&|~<>_!";

ساكن صحيح runC (lua_State *L, lua_State *L1, ثابت حرف *pc) {
  حرف buff[300];
  صحيح status = 0;
  إذا (pc == عدم) أرجع luaL_error(L, "attempt to runC null script");
  لكل (;;) {
    ثابت حرف *inst = getstring;
    إذا EQ("") أرجع 0;
    وإلا إذا EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    وإلا إذا EQ("append") {
      صحيح t = getindex;
      صحيح i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    وإلا إذا EQ("arith") {
      صحيح op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    وإلا إذا EQ("call") {
      صحيح narg = getnum;
      صحيح nres = getnum;
      lua_call(L1, narg, nres);
    }
    وإلا إذا EQ("callk") {
      صحيح narg = getnum;
      صحيح nres = getnum;
      صحيح i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    وإلا إذا EQ("checkstack") {
      صحيح sz = getnum;
      ثابت حرف *سكو = getstring;
      إذا (*سكو == '\0')
        سكو = عدم;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, سكو);
    }
    وإلا إذا EQ("rawcheckstack") {
      صحيح sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    وإلا إذا EQ("compare") {
      ثابت حرف *opt = getstring;  /* EQ, LT, or LE */
      صحيح op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      صحيح a = getindex;
      صحيح b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    وإلا إذا EQ("concat") {
      lua_concat(L1, getnum);
    }
    وإلا إذا EQ("copy") {
      صحيح f = getindex;
      lua_copy(L1, f, getindex);
    }
    وإلا إذا EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    وإلا إذا EQ("getfield") {
      صحيح t = getindex;
      صحيح tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    وإلا إذا EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    وإلا إذا EQ("getmetatable") {
      إذا (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    وإلا إذا EQ("gettable") {
      صحيح tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    وإلا إذا EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    وإلا إذا EQ("gsub") {
      صحيح a = getnum; صحيح b = getnum; صحيح c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    وإلا إذا EQ("insert") {
      lua_سك(L1, getnum);
    }
    وإلا إذا EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    وإلا إذا EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    وإلا إذا EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    وإلا إذا EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    وإلا إذا EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    وإلا إذا EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    وإلا إذا EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    وإلا إذا EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    وإلا إذا EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    وإلا إذا EQ("len") {
      lua_len(L1, getindex);
    }
    وإلا إذا EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    وإلا إذا EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    وإلا إذا EQ("loadstring") {
      size_t slen;
      ثابت حرف *s = luaL_checklstring(L1, getnum, &slen);
      ثابت حرف *name = getstring;
      ثابت حرف *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    وإلا إذا EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    وإلا إذا EQ("newtable") {
      lua_newtable(L1);
    }
    وإلا إذا EQ("newthread") {
      lua_newthread(L1);
    }
    وإلا إذا EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    وإلا إذا EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    وإلا إذا EQ("next") {
      lua_next(L1, -2);
    }
    وإلا إذا EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    وإلا إذا EQ("pcall") {
      صحيح narg = getnum;
      صحيح nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    وإلا إذا EQ("pcallk") {
      صحيح narg = getnum;
      صحيح nres = getnum;
      صحيح i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    وإلا إذا EQ("pop") {
      lua_چرو(L1, getnum);
    }
    وإلا إذا EQ("printstack") {
      صحيح n = getnum;
      إذا (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      وإلا lua_printstack(L1);
    }
    وإلا إذا EQ("print") {
      ثابت حرف *سكو = getstring;
      printf("%s\n", سكو);
    }
    وإلا إذا EQ("warningC") {
      ثابت حرف *سكو = getstring;
      lua_warning(L1, سكو, 1);
    }
    وإلا إذا EQ("warning") {
      ثابت حرف *سكو = getstring;
      lua_warning(L1, سكو, 0);
    }
    وإلا إذا EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    وإلا إذا EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    وإلا إذا EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    وإلا إذا EQ("pushnil") {
      lua_pushnil(L1);
    }
    وإلا إذا EQ("pushnum") {
      lua_pushnumber(L1, (lua_لمب)getnum);
    }
    وإلا إذا EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    وإلا إذا EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    وإلا إذا EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    وإلا إذا EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    وإلا إذا EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (صحيح)lua_tointeger(L, -1));
    }
    وإلا إذا EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    وإلا إذا EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    وإلا إذا EQ("rawget") {
      صحيح t = getindex;
      lua_rawget(L1, t);
    }
    وإلا إذا EQ("rawgeti") {
      صحيح t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    وإلا إذا EQ("rawgetp") {
      صحيح t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    وإلا إذا EQ("rawset") {
      صحيح t = getindex;
      lua_rawset(L1, t);
    }
    وإلا إذا EQ("rawseti") {
      صحيح t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    وإلا إذا EQ("rawsetp") {
      صحيح t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    وإلا إذا EQ("remove") {
      lua_remove(L1, getnum);
    }
    وإلا إذا EQ("replace") {
      lua_replace(L1, getindex);
    }
    وإلا إذا EQ("resume") {
      صحيح i = getindex;
      صحيح nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    وإلا إذا EQ("traceback") {
      ثابت حرف *سكو = getstring;
      صحيح level = getnum;
      luaL_traceback(L1, L1, سكو, level);
    }
    وإلا إذا EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    وإلا إذا EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    وإلا إذا EQ("return") {
      صحيح n = getnum;
      إذا (L1 != L) {
        صحيح i;
        لكل (i = 0; i < n; i++) {
          صحيح idx = -(n - i);
          بدل (lua_type(L1, idx)) {
            حالة LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              اكسر;
            افتراضي:
              lua_pushstring(L, lua_tostring(L1, idx));
              اكسر;
          }
        }
      }
      أرجع n;
    }
    وإلا إذا EQ("rotate") {
      صحيح i = getindex;
      lua_rotate(L1, i, getnum);
    }
    وإلا إذا EQ("setfield") {
      صحيح t = getindex;
      ثابت حرف *s = getstring;
      lua_setfield(L1, t, s);
    }
    وإلا إذا EQ("seti") {
      صحيح t = getindex;
      lua_seti(L1, t, getnum);
    }
    وإلا إذا EQ("setglobal") {
      ثابت حرف *s = getstring;
      lua_setglobal(L1, s);
    }
    وإلا إذا EQ("sethook") {
      صحيح mask = getnum;
      صحيح كدي = getnum;
      ثابت حرف *s = getstring;
      sethookaux(L1, mask, كدي, s);
    }
    وإلا إذا EQ("setmetatable") {
      صحيح idx = getindex;
      lua_setmetatable(L1, idx);
    }
    وإلا إذا EQ("settable") {
      lua_settable(L1, getindex);
    }
    وإلا إذا EQ("settop") {
      lua_settop(L1, getnum);
    }
    وإلا إذا EQ("testudata") {
      صحيح i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != عدم);
    }
    وإلا إذا EQ("error") {
      lua_error(L1);
    }
    وإلا إذا EQ("abort") {
      abort();
    }
    وإلا إذا EQ("throw") {
#إذا defined(__cplusplus)
ساكن بنية X { صحيح x; } x;
      throw x;
#وإلا
      luaL_error(L1, "C++");
#endif
      اكسر;
    }
    وإلا إذا EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    وإلا إذا EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    وإلا إذا EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    وإلا إذا EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    وإلا إذا EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    وإلا إذا EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    وإلا إذا EQ("tostring") {
      ثابت حرف *s = lua_tostring(L1, getindex);
      ثابت حرف *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == عدم && s1 == عدم) || strcmp(s, s1) == 0);
    }
    وإلا إذا EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, عدم);
    }
    وإلا إذا EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    وإلا إذا EQ("xmove") {
      صحيح f = getindex;
      صحيح t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      صحيح n = getnum;
      إذا (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    وإلا إذا EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    وإلا إذا EQ("yield") {
      أرجع lua_yield(L1, getnum);
    }
    وإلا إذا EQ("yieldk") {
      صحيح nres = getnum;
      صحيح i = getindex;
      أرجع lua_yieldk(L1, nres, i, Cfunck);
    }
    وإلا إذا EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    وإلا إذا EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    وإلا إذا EQ("argerror") {
      صحيح arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    وإلا luaL_error(L, "unknown instruction %s", buff);
  }
  أرجع 0;
}


ساكن صحيح testC (lua_State *L) {
  lua_State *L1;
  ثابت حرف *pc;
  إذا (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  وإلا إذا (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  وإلا {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  أرجع runC(L, L1, pc);
}


ساكن صحيح Cfunc (lua_State *L) {
  أرجع runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


ساكن صحيح Cfunck (lua_State *L, صحيح status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  أرجع runC(L, L, lua_tostring(L, cast_int(ctx)));
}


ساكن صحيح makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  أرجع 1;
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
ساكن فراغ Chook (lua_State *L, lua_Debug *ar) {
  ثابت حرف *scpt;
  ثابت حرف *ثابت events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_چرو(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
ساكن فراغ sethookaux (lua_State *L, صحيح mask, صحيح كدي, ثابت حرف *scpt) {
  إذا (*scpt == '\0') {  /* no script? */
    lua_sethook(L, عدم, 0, 0);  /* turn off hooks */
    أرجع;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  إذا (!lua_istable(L, -1)) {  /* no hook table? */
    lua_چرو(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, كدي);
}


ساكن صحيح sethook (lua_State *L) {
  إذا (lua_isnoneornil(L, 1))
    lua_sethook(L, عدم, 0, 0);  /* turn off hooks */
  وإلا {
    ثابت حرف *scpt = luaL_checkstring(L, 1);
    ثابت حرف *smask = luaL_checkstring(L, 2);
    صحيح كدي = cast_int(luaL_optinteger(L, 3, 0));
    صحيح mask = 0;
    إذا (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    إذا (strchr(smask, 'r')) mask |= LUA_MASKRET;
    إذا (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    إذا (كدي > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, كدي, scpt);
  }
  أرجع 0;
}


ساكن صحيح coresume (lua_State *L) {
  صحيح status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  إذا (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_سك(L, -2);
    أرجع 2;  /* return false + error message */
  }
  وإلا {
    lua_pushboolean(L, 1);
    أرجع 1;
  }
}

#إذا !defined(LUA_USE_POSIX)

#define nonblock	عدم

#وإلا

#ضمن <unistd.h>
#ضمن <fcntl.h>

ساكن صحيح nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  صحيح fd = fileno(f);
  صحيح flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  أرجع 0;
}
#endif

/* }====================================================== */



ساكن ثابت بنية luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_كدي},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {عدم, عدم}
};


ساكن فراغ checkfinalmem (فراغ) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


صحيح luaB_opentests (lua_State *L) {
  فراغ *ud;
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
  أرجع 1;
}

#endif

