/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <setjmp.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lapi.h"
#অন্তর্ভাৱ "lauxlib.h"
#অন্তর্ভাৱ "lcode.h"
#অন্তর্ভাৱ "lctype.h"
#অন্তর্ভাৱ "ldebug.h"
#অন্তর্ভাৱ "ldo.h"
#অন্তর্ভাৱ "lfunc.h"
#অন্তর্ভাৱ "lmem.h"
#অন্তর্ভাৱ "lopcodes.h"
#অন্তর্ভাৱ "lopnames.h"
#অন্তর্ভাৱ "lstate.h"
#অন্তর্ভাৱ "lstring.h"
#অন্তর্ভাৱ "ltable.h"
#অন্তর্ভাৱ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#যদি defined(LUA_DEBUG)


শূন্য *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


স্থির পূর্ণাঙ্ক runC (lua_State *L, lua_State *L1, ধ্রুৱ ৱর্ণ *pc);


স্থির শূন্য setnameval (lua_State *L, ধ্রুৱ ৱর্ণ *name, পূর্ণাঙ্ক val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


স্থির শূন্য pushobject (lua_State *L, ধ্রুৱ TValue *o) {
  setobj2s(L, L->শীৰ্ষ.p, o);
  api_incr_শীৰ্ষ(L);
}


স্থির শূন্য badexit (ধ্রুৱ ৱর্ণ *fmt, ধ্রুৱ ৱর্ণ *s1, ধ্রুৱ ৱর্ণ *s2) {
  fprintf(stderr, fmt, s1);
  যদি (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


স্থির পূর্ণাঙ্ক tpanic (lua_State *L) {
  ধ্রুৱ ৱর্ণ *বাৰ্তা = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ঘূৰাই (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   বাৰ্তা, রিক্ত),
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
স্থির শূন্য warnf (শূন্য *ud, ধ্রুৱ ৱর্ণ *বাৰ্তা, পূর্ণাঙ্ক tocont) {
  lua_State *L = cast(lua_State *, ud);
  স্থির ৱর্ণ buff[200] = "";  /* should be enough for tests... */
  স্থির পূর্ণাঙ্ক onoff = 0;
  স্থির পূর্ণাঙ্ক mode = 0;  /* start in normal mode */
  স্থির পূর্ণাঙ্ক lasttocont = 0;
  যদি (!lasttocont && !tocont && *বাৰ্তা == '@') {  /* control message? */
    যদি (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", বাৰ্তা, buff);
    যদি (strcmp(বাৰ্তা, "@off") == 0)
      onoff = 0;
    অন্যথা যদি (strcmp(বাৰ্তা, "@on") == 0)
      onoff = 1;
    অন্যথা যদি (strcmp(বাৰ্তা, "@normal") == 0)
      mode = 0;
    অন্যথা যদি (strcmp(বাৰ্তা, "@allow") == 0)
      mode = 1;
    অন্যথা যদি (strcmp(বাৰ্তা, "@store") == 0)
      mode = 2;
    অন্যথা
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              বাৰ্তা, রিক্ত);
    ঘূৰাই;
  }
  lasttocont = tocont;
  যদি (strlen(বাৰ্তা) >= পরিমাণ(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", বাৰ্তা, buff);
  strcat(buff, বাৰ্তা);  /* add new message to current warning */
  যদি (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    যদি (!lua_toboolean(L, -1))
      lua_বেৰ_কৰো(L, 1);  /* ok, no previous unexpected warning */
    অন্যথা {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    ৱিকল্প (mode) {
      পক্ষ 0: {  /* normal */
        যদি (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, রিক্ত);
      }  /* FALLTHROUGH */
      পক্ষ 1: {  /* allow */
        যদি (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        ভঙ্গ;
      }
      পক্ষ 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        ভঙ্গ;
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

প্রকারনির্দেশ সঙ্ঘ memHeader {
  LUAI_MAXALIGN;
  সংরচনা {
    size_t size;
    পূর্ণাঙ্ক type;
  } d;
} memHeader;


#যদি !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#অন্যথা

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


স্থির শূন্য freeblock (Memcontrol *mc, memHeader *block) {
  যদি (block) {
    size_t size = block->d.size;
    পূর্ণাঙ্ক i;
    বাবে (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, পরিমাণ(memHeader) + size + MARKSIZE);  /* erase block */
    মুক্ত(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


শূন্য *debug_realloc (শূন্য *ud, শূন্য *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  পূর্ণাঙ্ক type;
  যদি (mc->memlimit == 0) {  /* first time? */
    ৱর্ণ *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, রিক্ত, 10) : ULONG_MAX;
  }
  যদি (block == রিক্ত) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  অন্যথা {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  যদি (size == 0) {
    freeblock(mc, block);
    ঘূৰাই রিক্ত;
  }
  যদি (mc->failnext) {
    mc->failnext = 0;
    ঘূৰাই রিক্ত;  /* fake a single memory allocation error */
  }
  যদি (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    যদি (mc->countlimit == 0)
      ঘূৰাই রিক্ত;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  যদি (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ঘূৰাই রিক্ত;  /* fake a memory allocation error */
  অন্যথা {
    memHeader *newblock;
    পূর্ণাঙ্ক i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = পরিমাণ(memHeader) + size + MARKSIZE;
    যদি (realsize < size) ঘূৰাই রিক্ত;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    যদি (newblock == রিক্ত)
      ঘূৰাই রিক্ত;  /* really out of memory? */
    যদি (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    বাবে (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    যদি (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ঘূৰাই newblock + 1;
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

#অন্তর্ভাৱ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
স্থির পূর্ণাঙ্ক testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  যদি (isdead(g,t)) ঘূৰাই 0;
  যদি (issweepphase(g))
    ঘূৰাই 1;  /* no invariants */
  অন্যথা যদি (g->gckind != KGC_GENMINOR)
    ঘূৰাই !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  অন্যথা {  /* generational mode */
    যদি ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ঘূৰাই 0;
    যদি ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ঘূৰাই 0;
    ঘূৰাই 1;
  }
}


স্থির শূন্য printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (শূন্য *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  যদি (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


শূন্য lua_printobj (lua_State *L, সংরচনা GCObject *o) {
  printobj(G(L), o);
}


শূন্য lua_printvalue (TValue *v) {
  ৱিকল্প (ttypetag(v)) {
    পক্ষ LUA_VNUMINT: পক্ষ LUA_VNUMFLT: {
      ৱর্ণ buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      ভঙ্গ;
    }
    পক্ষ LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); ভঙ্গ;
    পক্ষ LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); ভঙ্গ;
    পক্ষ LUA_VFALSE:
      printf("%s", "false"); ভঙ্গ;
    পক্ষ LUA_VTRUE:
      printf("%s", "true"); ভঙ্গ;
    পক্ষ LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); ভঙ্গ;
    পক্ষ LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); ভঙ্গ;
    পক্ষ LUA_VNIL:
      printf("nil"); ভঙ্গ;
    পক্ষ LUA_VLCF:
      printf("light C function: %p", fvalue(v)); ভঙ্গ;
    পক্ষ LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); ভঙ্গ;
    পক্ষ LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); ভঙ্গ;
    পক্ষ LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); ভঙ্গ;
    পক্ষ LUA_VTABLE:
      printf("table: %p", hvalue(v)); ভঙ্গ;
    স্ৱভাৱ:
      lua_assert(0);
  }
}


স্থির পূর্ণাঙ্ক testobjref (global_State *g, GCObject *f, GCObject *t) {
  পূর্ণাঙ্ক r1 = testobjref1(g, f, t);
  যদি (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ঘূৰাই r1;
}


স্থির শূন্য checkobjref (global_State *g, GCObject *f, GCObject *t) {
    প্রতিজ্ঞা(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ যদি (t) checkobjref(g,f,obj2gco(t)); }


স্থির শূন্য checkvalref (global_State *g, GCObject *f, ধ্রুৱ TValue *t) {
  প্রতিজ্ঞা(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


স্থির শূন্য checktable (global_State *g, Table *h) {
  unsigned পূর্ণাঙ্ক i;
  unsigned পূর্ণাঙ্ক asize = h->asize;
  গ্ৰন্থি_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  বাবে (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  বাবে (n = gnode(h, 0); n < limit; n++) {
    যদি (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      প্রতিজ্ঞা(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


স্থির শূন্য checkudata (global_State *g, Udata *u) {
  পূর্ণাঙ্ক i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  বাবে (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


স্থির শূন্য checkproto (global_State *g, Proto *f) {
  পূর্ণাঙ্ক i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  বাবে (i=0; i<f->sizek; i++) {
    যদি (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  বাবে (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  বাবে (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  বাবে (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


স্থির শূন্য checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  পূর্ণাঙ্ক i;
  বাবে (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


স্থির শূন্য checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  পূর্ণাঙ্ক i;
  checkobjrefN(g, clgc, cl->p);
  বাবে (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    যদি (uv) {
      checkobjrefN(g, clgc, uv);
      যদি (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


স্থির পূর্ণাঙ্ক lua_checkpc (CallInfo *ci) {
  যদি (!isLua(ci)) ঘূৰাই 1;
  অন্যথা {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ঘূৰাই p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


স্থির শূন্য check_স্তূপ (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  প্রতিজ্ঞা(!isdead(g, L1));
  যদি (L1->স্তূপ.p == রিক্ত) {  /* incomplete thread? */
    প্রতিজ্ঞা(L1->openupval == রিক্ত && L1->ci == রিক্ত);
    ঘূৰাই;
  }
  বাবে (uv = L1->openupval; uv != রিক্ত; uv = uv->u.open.next)
    প্রতিজ্ঞা(upisopen(uv));  /* must be open */
  প্রতিজ্ঞা(L1->শীৰ্ষ.p <= L1->স্তূপ_last.p);
  প্রতিজ্ঞা(L1->tbclist.p <= L1->শীৰ্ষ.p);
  বাবে (ci = L1->ci; ci != রিক্ত; ci = ci->previous) {
    প্রতিজ্ঞা(ci->শীৰ্ষ.p <= L1->স্তূপ_last.p);
    প্রতিজ্ঞা(lua_checkpc(ci));
  }
  বাবে (o = L1->স্তূপ.p; o < L1->স্তূপ_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


স্থির শূন্য checkrefs (global_State *g, GCObject *o) {
  ৱিকল্প (o->tt) {
    পক্ষ LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      ভঙ্গ;
    }
    পক্ষ LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      ভঙ্গ;
    }
    পক্ষ LUA_VTABLE: {
      checktable(g, gco2t(o));
      ভঙ্গ;
    }
    পক্ষ LUA_VTHREAD: {
      check_স্তূপ(g, gco2th(o));
      ভঙ্গ;
    }
    পক্ষ LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      ভঙ্গ;
    }
    পক্ষ LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      ভঙ্গ;
    }
    পক্ষ LUA_VPROTO: {
      checkproto(g, gco2p(o));
      ভঙ্গ;
    }
    পক্ষ LUA_VSHRSTR:
    পক্ষ LUA_VLNGSTR: {
      প্রতিজ্ঞা(!isgray(o));  /* strings are never gray */
      ভঙ্গ;
    }
    স্ৱভাৱ: প্রতিজ্ঞা(0);
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
স্থির শূন্য checkobject (global_State *g, GCObject *o, পূর্ণাঙ্ক maybedead,
                         পূর্ণাঙ্ক listage) {
  যদি (isdead(g, o))
    প্রতিজ্ঞা(maybedead);
  অন্যথা {
    প্রতিজ্ঞা(g->gcstate != GCSpause || iswhite(o));
    যদি (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      প্রতিজ্ঞা(getage(o) >= listage);
      যদি (isold(o)) {
        প্রতিজ্ঞা(!iswhite(o));
        প্রতিজ্ঞা(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      প্রতিজ্ঞা(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


স্থির l_mem checkgraylist (global_State *g, GCObject *o) {
  পূর্ণাঙ্ক total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  যেতিয়ালৈ (o) {
    প্রতিজ্ঞা(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    প্রতিজ্ঞা(!testbit(o->marked, TESTBIT));
    যদি (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    ৱিকল্প (o->tt) {
      পক্ষ LUA_VTABLE: o = gco2t(o)->gclist; ভঙ্গ;
      পক্ষ LUA_VLCL: o = gco2lcl(o)->gclist; ভঙ্গ;
      পক্ষ LUA_VCCL: o = gco2ccl(o)->gclist; ভঙ্গ;
      পক্ষ LUA_VTHREAD: o = gco2th(o)->gclist; ভঙ্গ;
      পক্ষ LUA_VPROTO: o = gco2p(o)->gclist; ভঙ্গ;
      পক্ষ LUA_VUSERDATA:
        প্রতিজ্ঞা(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        ভঙ্গ;
      স্ৱভাৱ: প্রতিজ্ঞা(0);  /* other objects cannot be in a gray list */
    }
  }
  ঘূৰাই total;
}


/*
** Check objects in gray lists.
*/
স্থির l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  যদি (!keepinvariant(g)) ঘূৰাই total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ঘূৰাই total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
স্থির শূন্য incifingray (global_State *g, GCObject *o, l_mem *গুনতি) {
  যদি (!keepinvariant(g))
    ঘূৰাই;  /* gray lists not being kept in these phases */
  যদি (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    প্রতিজ্ঞা(!isgray(o) || upisopen(gco2upv(o)));
    ঘূৰাই;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  যদি (isgray(o) || getage(o) == G_TOUCHED2) {
    (*গুনতি)++;
    প্রতিজ্ঞা(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


স্থির l_mem checklist (global_State *g, পূর্ণাঙ্ক maybedead, পূর্ণাঙ্ক tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  বাবে (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  বাবে (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  বাবে (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  বাবে (o = reallyold; o != রিক্ত; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  ঘূৰাই total;
}


পূর্ণাঙ্ক lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  পূর্ণাঙ্ক maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  যদি (keepinvariant(g)) {
    প্রতিজ্ঞা(!iswhite(mainthread(g)));
    প্রতিজ্ঞা(!iswhite(gcvalue(&g->l_registry)));
  }
  প্রতিজ্ঞা(!isdead(g, gcvalue(&g->l_registry)));
  প্রতিজ্ঞা(g->sweepgc == রিক্ত || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  বাবে (o = g->fixedgc; o != রিক্ত; o = o->next) {
    প্রতিজ্ঞা(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  বাবে (o = g->tobefnz; o != রিক্ত; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    প্রতিজ্ঞা(tofinalize(o));
    প্রতিজ্ঞা(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  যদি (keepinvariant(g))
    প্রতিজ্ঞা(totalin == totalshould);
  ঘূৰাই 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


স্থির ৱর্ণ *buildop (Proto *p, পূর্ণাঙ্ক pc, ৱর্ণ *buff) {
  ৱর্ণ *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  ধ্রুৱ ৱর্ণ *name = opnames[o];
  পূর্ণাঙ্ক সাৰি = luaG_getfuncline(p, pc);
  পূর্ণাঙ্ক lineinfo = (p->lineinfo != রিক্ত) ? p->lineinfo[pc] : 0;
  যদি (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  অন্যথা
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", সাৰি, pc);
  ৱিকল্প (getOpMode(o)) {
    পক্ষ iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      ভঙ্গ;
    পক্ষ ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      ভঙ্গ;
    পক্ষ iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      ভঙ্গ;
    পক্ষ iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      ভঙ্গ;
    পক্ষ iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      ভঙ্গ;
    পক্ষ isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      ভঙ্গ;
  }
  ঘূৰাই obuff;
}


#যদি 0
শূন্য luaI_printcode (Proto *pt, পূর্ণাঙ্ক size) {
  পূর্ণাঙ্ক pc;
  বাবে (pc=0; pc<size; pc++) {
    ৱর্ণ buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


শূন্য luaI_printinst (Proto *pt, পূর্ণাঙ্ক pc) {
  ৱর্ণ buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


স্থির পূর্ণাঙ্ক listcode (lua_State *L) {
  পূর্ণাঙ্ক pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  বাবে (pc=0; pc<p->sizecode; pc++) {
    ৱর্ণ buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক printcode (lua_State *L) {
  পূর্ণাঙ্ক pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  বাবে (pc=0; pc<p->sizecode; pc++) {
    ৱর্ণ buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক listk (lua_State *L) {
  Proto *p;
  পূর্ণাঙ্ক i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  বাবে (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক listabslineinfo (lua_State *L) {
  Proto *p;
  পূর্ণাঙ্ক i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != রিক্ত, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  বাবে (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].সাৰি);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক listlocals (lua_State *L) {
  Proto *p;
  পূর্ণাঙ্ক pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  পূর্ণাঙ্ক i = 0;
  ধ্রুৱ ৱর্ণ *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  যেতিয়ালৈ ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != রিক্ত)
    lua_pushstring(L, name);
  ঘূৰাই i-1;
}

/* }====================================================== */



শূন্য lua_printstack (lua_State *L) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক n = lua_gettop(L);
  printf("stack: >>\n");
  বাবে (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


পূর্ণাঙ্ক lua_printallstack (lua_State *L) {
  StkId p;
  পূর্ণাঙ্ক i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  বাবে (p = L->স্তূপ.p; p < L->শীৰ্ষ.p; p++) {
    যদি (ci != রিক্ত && p == ci->func.p) {
      printf("  ---\n");
      যদি (ci == L->ci)
        ci = রিক্ত;  /* printed last frame */
      অন্যথা
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", অঙ্ক_OPCODES);
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", পরিমাণ(lua_State));
  setnameval(L, "global state", পরিমাণ(global_State));
  setnameval(L, "TValue", পরিমাণ(TValue));
  setnameval(L, "Node", পরিমাণ(গ্ৰন্থি_2));
  setnameval(L, "stack Value", পরিমাণ(স্তূপ_মান));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক mem_query (lua_State *L) {
  যদি (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ঘূৰাই 3;
  }
  অন্যথা যদি (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    যদি (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ঘূৰাই 0;
  }
  অন্যথা {
    ধ্রুৱ ৱর্ণ *t = luaL_checkstring(L, 1);
    পূর্ণাঙ্ক i;
    বাবে (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      যদি (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ঘূৰাই 1;
      }
    }
    ঘূৰাই luaL_error(L, "unknown type '%s'", t);
  }
}


স্থির পূর্ণাঙ্ক alloc_গুনতি (lua_State *L) {
  যদি (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  অন্যথা
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক settrick (lua_State *L) {
  যদি (ttisnil(obj_at(L, 1)))
    l_Trick = রিক্ত;
  অন্যথা
    l_Trick = gcvalue(obj_at(L, 1));
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  যদি (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  অন্যথা {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  যদি (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  অন্যথা {
    স্থির ধ্রুৱ ৱর্ণ *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  যদি (!iscollectable(o))
    printf("no collectable\n");
  অন্যথা {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ঘূৰাই 0;
}


স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

স্থির পূর্ণাঙ্ক gc_state (lua_State *L) {
  স্থির ধ্রুৱ পূর্ণাঙ্ক states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  পূর্ণাঙ্ক option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  যদি (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ঘূৰাই 1;
  }
  অন্যথা {
    যদি (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    যদি (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ঘূৰাই 0;
  }
}


স্থির পূর্ণাঙ্ক tracinggc = 0;
শূন্য luai_tracegctest (lua_State *L, পূর্ণাঙ্ক first) {
  যদি (!tracinggc) ঘূৰাই;
  অন্যথা {
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


স্থির পূর্ণাঙ্ক tracegc (lua_State *L) {
  যদি (lua_isnil(L, 1))
    tracinggc = 0;
  অন্যথা {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক hash_query (lua_State *L) {
  যদি (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    যদি (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  অন্যথা {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->গ্ৰন্থি));
  }
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক stacklevel (lua_State *L) {
  পূর্ণাঙ্ক a = 0;
  lua_pushinteger(L, cast_Integer(L->শীৰ্ষ.p - L->স্তূপ.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ঘূৰাই 5;
}


স্থির পূর্ণাঙ্ক resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  যেতিয়ালৈ (ci->next != রিক্ত) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক reallocstack (lua_State *L) {
  পূর্ণাঙ্ক n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->শীৰ্ষ.p - L->স্তূপ.p) + n, 1);
  lua_unlock(L);
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক table_query (lua_State *L) {
  ধ্রুৱ Table *t;
  পূর্ণাঙ্ক i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned পূর্ণাঙ্ক asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  যদি (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ঘূৰাই 3;
  }
  অন্যথা যদি (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    যদি (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->শীৰ্ষ.p));
    অন্যথা
      setnilvalue2s(L->শীৰ্ষ.p);
    api_incr_শীৰ্ষ(L);
    lua_pushnil(L);
  }
  অন্যথা যদি (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    যদি (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    অন্যথা
      lua_pushliteral(L, "<undef>");
    যদি (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    অন্যথা
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->গ্ৰন্থি[i]));
  }
  ঘূৰাই 3;
}


স্থির পূর্ণাঙ্ক gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ঘূৰাই 6;
}


স্থির পূর্ণাঙ্ক test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  পূর্ণাঙ্ক s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  যদি (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ঘূৰাই 2;
  }
  অন্যথা যদি (s < tb->size) {
    TString *ts;
    পূর্ণাঙ্ক n = 0;
    বাবে (ts = tb->hash[s]; ts != রিক্ত; ts = ts->u.hnext) {
      setsvalue2s(L, L->শীৰ্ষ.p, ts);
      api_incr_শীৰ্ষ(L);
      n++;
    }
    ঘূৰাই n;
  }
  অন্যথা ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক getreftable (lua_State *L) {
  যদি (lua_istable(L, 2))  /* is there a table as second argument? */
    ঘূৰাই 2;  /* use it as the table */
  অন্যথা
    ঘূৰাই LUA_REGISTRYINDEX;  /* default is to use the register */
}


স্থির পূর্ণাঙ্ক tref (lua_State *L) {
  পূর্ণাঙ্ক t = getreftable(L);
  পূর্ণাঙ্ক level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক getref (lua_State *L) {
  পূর্ণাঙ্ক t = getreftable(L);
  পূর্ণাঙ্ক level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক unref (lua_State *L) {
  পূর্ণাঙ্ক t = getreftable(L);
  পূর্ণাঙ্ক level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক upvalue (lua_State *L) {
  পূর্ণাঙ্ক n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  যদি (lua_isnone(L, 3)) {
    ধ্রুৱ ৱর্ণ *name = lua_getupvalue(L, 1, n);
    যদি (name == রিক্ত) ঘূৰাই 0;
    lua_pushstring(L, name);
    ঘূৰাই 2;
  }
  অন্যথা {
    ধ্রুৱ ৱর্ণ *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ঘূৰাই 1;
  }
}


স্থির পূর্ণাঙ্ক newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  পূর্ণাঙ্ক nuv = cast_int(luaL_optinteger(L, 2, 0));
  ৱর্ণ *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  যেতিয়ালৈ (size--) *p++ = '\0';
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L, 1, &l);
  পূর্ণাঙ্ক status = luaL_loadbufferx(L1, s, l, s, "t");
  যদি (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক s2d (lua_State *L) {
  lua_pushnumber(L, cast_অঙ্ক(*cast(ধ্রুৱ দ্ৱিগুণ *, luaL_checkstring(L, 1))));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক d2s (lua_State *L) {
  দ্ৱিগুণ d = cast(দ্ৱিগুণ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), পরিমাণ(d));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক newstate (lua_State *L) {
  শূন্য *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  যদি (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  অন্যথা
    lua_pushnil(L);
  ঘূৰাই 1;
}


স্থির lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != রিক্ত, 1, "state expected");
  ঘূৰাই L1;
}


স্থির পূর্ণাঙ্ক loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  পূর্ণাঙ্ক load = cast_int(luaL_checkinteger(L, 2));
  পূর্ণাঙ্ক preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", রিক্ত, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ধ্রুৱ ৱর্ণ *code = luaL_checklstring(L, 2, &lcode);
  পূর্ণাঙ্ক status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  যদি (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  যদি (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ঘূৰাই 3;
  }
  অন্যথা {
    পূর্ণাঙ্ক i = 0;
    যেতিয়ালৈ (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_বেৰ_কৰো(L1, i-1);
    ঘূৰাই i-1;
  }
}


স্থির পূর্ণাঙ্ক log2_aux (lua_State *L) {
  unsigned পূর্ণাঙ্ক x = (unsigned পূর্ণাঙ্ক)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ঘূৰাই 1;
}


সংরচনা Aux { jmp_buf jb; ধ্রুৱ ৱর্ণ *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
স্থির পূর্ণাঙ্ক panicback (lua_State *L) {
  সংরচনা Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (সংরচনা Aux *)lua_touserdata(L, -1);
  lua_বেৰ_কৰো(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ঘূৰাই 1;  /* to avoid warnings */
}

স্থির পূর্ণাঙ্ক checkpanic (lua_State *L) {
  সংরচনা Aux b;
  শূন্য *ud;
  lua_State *L1;
  ধ্রুৱ ৱর্ণ *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  যদি (L1 == রিক্ত) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ঘূৰাই 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  যদি (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  অন্যথা {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ঘূৰাই 1;
}


স্থির পূর্ণাঙ্ক externKstr (lua_State *L) {
  size_t len;
  ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, রিক্ত, রিক্ত);
  ঘূৰাই 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
স্থির পূর্ণাঙ্ক externstr (lua_State *L) {
  size_t len;
  ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L, 1, &len);
  শূন্য *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  ৱর্ণ *buff = cast_charp((*allocf)(ud, রিক্ত, 0, len + 1));
  যদি (buff == রিক্ত) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * পরিমাণ(ৱর্ণ));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ঘূৰাই 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


স্থির শূন্য sethookaux (lua_State *L, পূর্ণাঙ্ক mask, পূর্ণাঙ্ক গুনতি, ধ্রুৱ ৱর্ণ *code);

স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ delimits = " \t\n,;";

স্থির শূন্য skip (ধ্রুৱ ৱর্ণ **pc) {
  বাবে (;;) {
    যদি (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    অন্যথা যদি (**pc == '#') {  /* comment? */
      যেতিয়ালৈ (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    অন্যথা ভঙ্গ;
  }
}

স্থির পূর্ণাঙ্ক getnum_aux (lua_State *L, lua_State *L1, ধ্রুৱ ৱর্ণ **pc) {
  পূর্ণাঙ্ক res = 0;
  পূর্ণাঙ্ক sig = 1;
  skip(pc);
  যদি (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_বেৰ_কৰো(L1, 1);
    (*pc)++;
    ঘূৰাই res;
  }
  অন্যথা যদি (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ঘূৰাই res;
  }
  অন্যথা যদি (**pc == '!') {
    (*pc)++;
    যদি (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    অন্যথা যদি (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    অন্যথা lua_assert(0);
    (*pc)++;
    ঘূৰাই res;
  }
  অন্যথা যদি (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  যদি (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  যেতিয়ালৈ (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ঘূৰাই sig*res;
}

স্থির ধ্রুৱ ৱর্ণ *getstring_aux (lua_State *L, ৱর্ণ *buff, ধ্রুৱ ৱর্ণ **pc) {
  পূর্ণাঙ্ক i = 0;
  skip(pc);
  যদি (**pc == '"' || **pc == '\'') {  /* quoted string? */
    পূর্ণাঙ্ক quote = *(*pc)++;
    যেতিয়ালৈ (**pc != quote) {
      যদি (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  অন্যথা {
    যেতিয়ালৈ (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ঘূৰাই buff;
}


স্থির পূর্ণাঙ্ক getindex_aux (lua_State *L, lua_State *L1, ধ্রুৱ ৱর্ণ **pc) {
  skip(pc);
  ৱিকল্প (*(*pc)++) {
    পক্ষ 'R': ঘূৰাই LUA_REGISTRYINDEX;
    পক্ষ 'U': ঘূৰাই lua_upvalueindex(getnum_aux(L, L1, pc));
    স্ৱভাৱ: {
      পূর্ণাঙ্ক n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      যদি (n == 0) ঘূৰাই 0;
      অন্যথা ঘূৰাই lua_absindex(L1, n);
    }
  }
}


স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
স্থির শূন্য regcodes (lua_State *L) {
  unsigned পূর্ণাঙ্ক i;
  বাবে (i = 0; i < পরিমাণ(statcodes) / পরিমাণ(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


স্থির পূর্ণাঙ্ক testC (lua_State *L);
স্থির পূর্ণাঙ্ক Cfunck (lua_State *L, পূর্ণাঙ্ক status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
স্থির ধ্রুৱ ৱর্ণ ops[] = "+-*%^/\\&|~<>_!";

স্থির পূর্ণাঙ্ক runC (lua_State *L, lua_State *L1, ধ্রুৱ ৱর্ণ *pc) {
  ৱর্ণ buff[300];
  পূর্ণাঙ্ক status = 0;
  যদি (pc == রিক্ত) ঘূৰাই luaL_error(L, "attempt to runC null script");
  বাবে (;;) {
    ধ্রুৱ ৱর্ণ *inst = getstring;
    যদি EQ("") ঘূৰাই 0;
    অন্যথা যদি EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    অন্যথা যদি EQ("append") {
      পূর্ণাঙ্ক t = getindex;
      পূর্ণাঙ্ক i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    অন্যথা যদি EQ("arith") {
      পূর্ণাঙ্ক op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    অন্যথা যদি EQ("call") {
      পূর্ণাঙ্ক narg = getnum;
      পূর্ণাঙ্ক nres = getnum;
      lua_call(L1, narg, nres);
    }
    অন্যথা যদি EQ("callk") {
      পূর্ণাঙ্ক narg = getnum;
      পূর্ণাঙ্ক nres = getnum;
      পূর্ণাঙ্ক i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    অন্যথা যদি EQ("checkstack") {
      পূর্ণাঙ্ক sz = getnum;
      ধ্রুৱ ৱর্ণ *বাৰ্তা = getstring;
      যদি (*বাৰ্তা == '\0')
        বাৰ্তা = রিক্ত;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, বাৰ্তা);
    }
    অন্যথা যদি EQ("rawcheckstack") {
      পূর্ণাঙ্ক sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    অন্যথা যদি EQ("compare") {
      ধ্রুৱ ৱর্ণ *opt = getstring;  /* EQ, LT, or LE */
      পূর্ণাঙ্ক op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      পূর্ণাঙ্ক a = getindex;
      পূর্ণাঙ্ক b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    অন্যথা যদি EQ("concat") {
      lua_concat(L1, getnum);
    }
    অন্যথা যদি EQ("copy") {
      পূর্ণাঙ্ক f = getindex;
      lua_copy(L1, f, getindex);
    }
    অন্যথা যদি EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    অন্যথা যদি EQ("getfield") {
      পূর্ণাঙ্ক t = getindex;
      পূর্ণাঙ্ক tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    অন্যথা যদি EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    অন্যথা যদি EQ("getmetatable") {
      যদি (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    অন্যথা যদি EQ("gettable") {
      পূর্ণাঙ্ক tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    অন্যথা যদি EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    অন্যথা যদি EQ("gsub") {
      পূর্ণাঙ্ক a = getnum; পূর্ণাঙ্ক b = getnum; পূর্ণাঙ্ক c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    অন্যথা যদি EQ("insert") {
      lua_ঢোকাও(L1, getnum);
    }
    অন্যথা যদি EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    অন্যথা যদি EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    অন্যথা যদি EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    অন্যথা যদি EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    অন্যথা যদি EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    অন্যথা যদি EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    অন্যথা যদি EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    অন্যথা যদি EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    অন্যথা যদি EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    অন্যথা যদি EQ("len") {
      lua_len(L1, getindex);
    }
    অন্যথা যদি EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    অন্যথা যদি EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    অন্যথা যদি EQ("loadstring") {
      size_t slen;
      ধ্রুৱ ৱর্ণ *s = luaL_checklstring(L1, getnum, &slen);
      ধ্রুৱ ৱর্ণ *name = getstring;
      ধ্রুৱ ৱর্ণ *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    অন্যথা যদি EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    অন্যথা যদি EQ("newtable") {
      lua_newtable(L1);
    }
    অন্যথা যদি EQ("newthread") {
      lua_newthread(L1);
    }
    অন্যথা যদি EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    অন্যথা যদি EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    অন্যথা যদি EQ("next") {
      lua_next(L1, -2);
    }
    অন্যথা যদি EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    অন্যথা যদি EQ("pcall") {
      পূর্ণাঙ্ক narg = getnum;
      পূর্ণাঙ্ক nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    অন্যথা যদি EQ("pcallk") {
      পূর্ণাঙ্ক narg = getnum;
      পূর্ণাঙ্ক nres = getnum;
      পূর্ণাঙ্ক i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    অন্যথা যদি EQ("pop") {
      lua_বেৰ_কৰো(L1, getnum);
    }
    অন্যথা যদি EQ("printstack") {
      পূর্ণাঙ্ক n = getnum;
      যদি (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      অন্যথা lua_printstack(L1);
    }
    অন্যথা যদি EQ("print") {
      ধ্রুৱ ৱর্ণ *বাৰ্তা = getstring;
      printf("%s\n", বাৰ্তা);
    }
    অন্যথা যদি EQ("warningC") {
      ধ্রুৱ ৱর্ণ *বাৰ্তা = getstring;
      lua_warning(L1, বাৰ্তা, 1);
    }
    অন্যথা যদি EQ("warning") {
      ধ্রুৱ ৱর্ণ *বাৰ্তা = getstring;
      lua_warning(L1, বাৰ্তা, 0);
    }
    অন্যথা যদি EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    অন্যথা যদি EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    অন্যথা যদি EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    অন্যথা যদি EQ("pushnil") {
      lua_pushnil(L1);
    }
    অন্যথা যদি EQ("pushnum") {
      lua_pushnumber(L1, (lua_সংখ্যা)getnum);
    }
    অন্যথা যদি EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    অন্যথা যদি EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    অন্যথা যদি EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    অন্যথা যদি EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    অন্যথা যদি EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (পূর্ণাঙ্ক)lua_tointeger(L, -1));
    }
    অন্যথা যদি EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    অন্যথা যদি EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    অন্যথা যদি EQ("rawget") {
      পূর্ণাঙ্ক t = getindex;
      lua_rawget(L1, t);
    }
    অন্যথা যদি EQ("rawgeti") {
      পূর্ণাঙ্ক t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    অন্যথা যদি EQ("rawgetp") {
      পূর্ণাঙ্ক t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    অন্যথা যদি EQ("rawset") {
      পূর্ণাঙ্ক t = getindex;
      lua_rawset(L1, t);
    }
    অন্যথা যদি EQ("rawseti") {
      পূর্ণাঙ্ক t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    অন্যথা যদি EQ("rawsetp") {
      পূর্ণাঙ্ক t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    অন্যথা যদি EQ("remove") {
      lua_remove(L1, getnum);
    }
    অন্যথা যদি EQ("replace") {
      lua_replace(L1, getindex);
    }
    অন্যথা যদি EQ("resume") {
      পূর্ণাঙ্ক i = getindex;
      পূর্ণাঙ্ক nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    অন্যথা যদি EQ("traceback") {
      ধ্রুৱ ৱর্ণ *বাৰ্তা = getstring;
      পূর্ণাঙ্ক level = getnum;
      luaL_traceback(L1, L1, বাৰ্তা, level);
    }
    অন্যথা যদি EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    অন্যথা যদি EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    অন্যথা যদি EQ("return") {
      পূর্ণাঙ্ক n = getnum;
      যদি (L1 != L) {
        পূর্ণাঙ্ক i;
        বাবে (i = 0; i < n; i++) {
          পূর্ণাঙ্ক idx = -(n - i);
          ৱিকল্প (lua_type(L1, idx)) {
            পক্ষ LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              ভঙ্গ;
            স্ৱভাৱ:
              lua_pushstring(L, lua_tostring(L1, idx));
              ভঙ্গ;
          }
        }
      }
      ঘূৰাই n;
    }
    অন্যথা যদি EQ("rotate") {
      পূর্ণাঙ্ক i = getindex;
      lua_rotate(L1, i, getnum);
    }
    অন্যথা যদি EQ("setfield") {
      পূর্ণাঙ্ক t = getindex;
      ধ্রুৱ ৱর্ণ *s = getstring;
      lua_setfield(L1, t, s);
    }
    অন্যথা যদি EQ("seti") {
      পূর্ণাঙ্ক t = getindex;
      lua_seti(L1, t, getnum);
    }
    অন্যথা যদি EQ("setglobal") {
      ধ্রুৱ ৱর্ণ *s = getstring;
      lua_setglobal(L1, s);
    }
    অন্যথা যদি EQ("sethook") {
      পূর্ণাঙ্ক mask = getnum;
      পূর্ণাঙ্ক গুনতি = getnum;
      ধ্রুৱ ৱর্ণ *s = getstring;
      sethookaux(L1, mask, গুনতি, s);
    }
    অন্যথা যদি EQ("setmetatable") {
      পূর্ণাঙ্ক idx = getindex;
      lua_setmetatable(L1, idx);
    }
    অন্যথা যদি EQ("settable") {
      lua_settable(L1, getindex);
    }
    অন্যথা যদি EQ("settop") {
      lua_settop(L1, getnum);
    }
    অন্যথা যদি EQ("testudata") {
      পূর্ণাঙ্ক i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != রিক্ত);
    }
    অন্যথা যদি EQ("error") {
      lua_error(L1);
    }
    অন্যথা যদি EQ("abort") {
      abort();
    }
    অন্যথা যদি EQ("throw") {
#যদি defined(__cplusplus)
স্থির সংরচনা X { পূর্ণাঙ্ক x; } x;
      throw x;
#অন্যথা
      luaL_error(L1, "C++");
#endif
      ভঙ্গ;
    }
    অন্যথা যদি EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    অন্যথা যদি EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    অন্যথা যদি EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    অন্যথা যদি EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    অন্যথা যদি EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    অন্যথা যদি EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    অন্যথা যদি EQ("tostring") {
      ধ্রুৱ ৱর্ণ *s = lua_tostring(L1, getindex);
      ধ্রুৱ ৱর্ণ *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == রিক্ত && s1 == রিক্ত) || strcmp(s, s1) == 0);
    }
    অন্যথা যদি EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, রিক্ত);
    }
    অন্যথা যদি EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    অন্যথা যদি EQ("xmove") {
      পূর্ণাঙ্ক f = getindex;
      পূর্ণাঙ্ক t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      পূর্ণাঙ্ক n = getnum;
      যদি (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    অন্যথা যদি EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    অন্যথা যদি EQ("yield") {
      ঘূৰাই lua_yield(L1, getnum);
    }
    অন্যথা যদি EQ("yieldk") {
      পূর্ণাঙ্ক nres = getnum;
      পূর্ণাঙ্ক i = getindex;
      ঘূৰাই lua_yieldk(L1, nres, i, Cfunck);
    }
    অন্যথা যদি EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    অন্যথা যদি EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    অন্যথা যদি EQ("argerror") {
      পূর্ণাঙ্ক arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    অন্যথা luaL_error(L, "unknown instruction %s", buff);
  }
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক testC (lua_State *L) {
  lua_State *L1;
  ধ্রুৱ ৱর্ণ *pc;
  যদি (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  অন্যথা যদি (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  অন্যথা {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ঘূৰাই runC(L, L1, pc);
}


স্থির পূর্ণাঙ্ক Cfunc (lua_State *L) {
  ঘূৰাই runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


স্থির পূর্ণাঙ্ক Cfunck (lua_State *L, পূর্ণাঙ্ক status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ঘূৰাই runC(L, L, lua_tostring(L, cast_int(ctx)));
}


স্থির পূর্ণাঙ্ক makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ঘূৰাই 1;
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
স্থির শূন্য Chook (lua_State *L, lua_Debug *ar) {
  ধ্রুৱ ৱর্ণ *scpt;
  ধ্রুৱ ৱর্ণ *ধ্রুৱ events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_বেৰ_কৰো(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
স্থির শূন্য sethookaux (lua_State *L, পূর্ণাঙ্ক mask, পূর্ণাঙ্ক গুনতি, ধ্রুৱ ৱর্ণ *scpt) {
  যদি (*scpt == '\0') {  /* no script? */
    lua_sethook(L, রিক্ত, 0, 0);  /* turn off hooks */
    ঘূৰাই;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  যদি (!lua_istable(L, -1)) {  /* no hook table? */
    lua_বেৰ_কৰো(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, গুনতি);
}


স্থির পূর্ণাঙ্ক sethook (lua_State *L) {
  যদি (lua_isnoneornil(L, 1))
    lua_sethook(L, রিক্ত, 0, 0);  /* turn off hooks */
  অন্যথা {
    ধ্রুৱ ৱর্ণ *scpt = luaL_checkstring(L, 1);
    ধ্রুৱ ৱর্ণ *smask = luaL_checkstring(L, 2);
    পূর্ণাঙ্ক গুনতি = cast_int(luaL_optinteger(L, 3, 0));
    পূর্ণাঙ্ক mask = 0;
    যদি (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    যদি (strchr(smask, 'r')) mask |= LUA_MASKRET;
    যদি (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    যদি (গুনতি > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, গুনতি, scpt);
  }
  ঘূৰাই 0;
}


স্থির পূর্ণাঙ্ক coresume (lua_State *L) {
  পূর্ণাঙ্ক status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  যদি (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_ঢোকাও(L, -2);
    ঘূৰাই 2;  /* return false + error message */
  }
  অন্যথা {
    lua_pushboolean(L, 1);
    ঘূৰাই 1;
  }
}

#যদি !defined(LUA_USE_POSIX)

#define nonblock	রিক্ত

#অন্যথা

#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <fcntl.h>

স্থির পূর্ণাঙ্ক nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  পূর্ণাঙ্ক fd = fileno(f);
  পূর্ণাঙ্ক flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ঘূৰাই 0;
}
#endif

/* }====================================================== */



স্থির ধ্রুৱ সংরচনা luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_গুনতি},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {রিক্ত, রিক্ত}
};


স্থির শূন্য checkfinalmem (শূন্য) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


পূর্ণাঙ্ক luaB_opentests (lua_State *L) {
  শূন্য *ud;
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
  ঘূৰাই 1;
}

#endif

