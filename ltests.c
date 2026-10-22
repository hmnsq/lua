/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <setjmp.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lapi.h"
#অন্তর্ভাব "lauxlib.h"
#অন্তর্ভাব "lcode.h"
#অন্তর্ভাব "lctype.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lopcodes.h"
#অন্তর্ভাব "lopnames.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#যদি defined(LUA_DEBUG)


শূন্য *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


স্থির পূর্ণসংখ্যা runC (lua_State *L, lua_State *L1, ধ্রুবক অক্ষর *pc);


স্থির শূন্য setnameval (lua_State *L, ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


স্থির শূন্য pushobject (lua_State *L, ধ্রুবক TValue *o) {
  setobj2s(L, L->শীর্ষ.p, o);
  api_incr_শীর্ষ(L);
}


স্থির শূন্য badexit (ধ্রুবক অক্ষর *fmt, ধ্রুবক অক্ষর *s1, ধ্রুবক অক্ষর *s2) {
  fprintf(stderr, fmt, s1);
  যদি (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.যোগ = 0;
  exit(EXIT_FAILURE);
}


স্থির পূর্ণসংখ্যা tpanic (lua_State *L) {
  ধ্রুবক অক্ষর *বার্তা = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ফেরত (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   বার্তা, ফাঁকা),
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
স্থির শূন্য warnf (শূন্য *ud, ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা tocont) {
  lua_State *L = cast(lua_State *, ud);
  স্থির অক্ষর buff[200] = "";  /* should be enough for tests... */
  স্থির পূর্ণসংখ্যা onoff = 0;
  স্থির পূর্ণসংখ্যা mode = 0;  /* start in normal mode */
  স্থির পূর্ণসংখ্যা lasttocont = 0;
  যদি (!lasttocont && !tocont && *বার্তা == '@') {  /* control message? */
    যদি (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", বার্তা, buff);
    যদি (strcmp(বার্তা, "@off") == 0)
      onoff = 0;
    অন্যথা যদি (strcmp(বার্তা, "@on") == 0)
      onoff = 1;
    অন্যথা যদি (strcmp(বার্তা, "@normal") == 0)
      mode = 0;
    অন্যথা যদি (strcmp(বার্তা, "@allow") == 0)
      mode = 1;
    অন্যথা যদি (strcmp(বার্তা, "@store") == 0)
      mode = 2;
    অন্যথা
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              বার্তা, ফাঁকা);
    ফেরত;
  }
  lasttocont = tocont;
  যদি (strlen(বার্তা) >= আকার(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", বার্তা, buff);
  strcat(buff, বার্তা);  /* add new message to current warning */
  যদি (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    যদি (!lua_toboolean(L, -1))
      lua_বের_করো(L, 1);  /* ok, no previous unexpected warning */
    অন্যথা {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    নির্বাচন (mode) {
      ক্ষেত্র 0: {  /* normal */
        যদি (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ফাঁকা);
      }  /* FALLTHROUGH */
      ক্ষেত্র 1: {  /* allow */
        যদি (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        বিরতি;
      }
      ক্ষেত্র 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        বিরতি;
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

প্রকারলেখ সংঘ memHeader {
  LUAI_MAXALIGN;
  কাঠামো {
    size_t আকার_2;
    পূর্ণসংখ্যা type;
  } d;
} memHeader;


#যদি !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,আকার_2)	memset(mem, -MARK, আকার_2)

#অন্যথা

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,আকার_2)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


স্থির শূন্য freeblock (Memcontrol *mc, memHeader *block) {
  যদি (block) {
    size_t আকার_2 = block->d.আকার_2;
    পূর্ণসংখ্যা i;
    জন্য (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + আকার_2 + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, আকার(memHeader) + আকার_2 + MARKSIZE);  /* erase block */
    মুক্ত(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->যোগ -= আকার_2;
  }
}


শূন্য *debug_realloc (শূন্য *ud, শূন্য *b, size_t oldsize, size_t আকার_2) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  পূর্ণসংখ্যা type;
  যদি (mc->memlimit == 0) {  /* first time? */
    অক্ষর *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, ফাঁকা, 10) : ULONG_MAX;
  }
  যদি (block == ফাঁকা) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  অন্যথা {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.আকার_2);
  }
  যদি (আকার_2 == 0) {
    freeblock(mc, block);
    ফেরত ফাঁকা;
  }
  যদি (mc->failnext) {
    mc->failnext = 0;
    ফেরত ফাঁকা;  /* fake a single memory allocation error */
  }
  যদি (mc->countlimit != ~0UL && আকার_2 != oldsize) {  /* count limit in use? */
    যদি (mc->countlimit == 0)
      ফেরত ফাঁকা;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  যদি (আকার_2 > oldsize && mc->যোগ+আকার_2-oldsize > mc->memlimit)
    ফেরত ফাঁকা;  /* fake a memory allocation error */
  অন্যথা {
    memHeader *newblock;
    পূর্ণসংখ্যা i;
    size_t commonsize = (oldsize < আকার_2) ? oldsize : আকার_2;
    size_t realsize = আকার(memHeader) + আকার_2 + MARKSIZE;
    যদি (realsize < আকার_2) ফেরত ফাঁকা;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    যদি (newblock == ফাঁকা)
      ফেরত ফাঁকা;  /* really out of memory? */
    যদি (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, আকার_2 - commonsize);
    /* initialize marks after block */
    জন্য (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + আকার_2 + i) = MARK;
    newblock->d.আকার_2 = আকার_2;
    newblock->d.type = type;
    mc->যোগ += আকার_2;
    যদি (mc->যোগ > mc->maxmem)
      mc->maxmem = mc->যোগ;
    mc->numblocks++;
    mc->objcount[type]++;
    ফেরত newblock + 1;
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

#অন্তর্ভাব <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
স্থির পূর্ণসংখ্যা testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  যদি (isdead(g,t)) ফেরত 0;
  যদি (issweepphase(g))
    ফেরত 1;  /* no invariants */
  অন্যথা যদি (g->gckind != KGC_GENMINOR)
    ফেরত !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  অন্যথা {  /* generational mode */
    যদি ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ফেরত 0;
    যদি ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ফেরত 0;
    ফেরত 1;
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


শূন্য lua_printobj (lua_State *L, কাঠামো GCObject *o) {
  printobj(G(L), o);
}


শূন্য lua_printvalue (TValue *v) {
  নির্বাচন (ttypetag(v)) {
    ক্ষেত্র LUA_VNUMINT: ক্ষেত্র LUA_VNUMFLT: {
      অক্ষর buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      বিরতি;
    }
    ক্ষেত্র LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); বিরতি;
    ক্ষেত্র LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); বিরতি;
    ক্ষেত্র LUA_VFALSE:
      printf("%s", "false"); বিরতি;
    ক্ষেত্র LUA_VTRUE:
      printf("%s", "true"); বিরতি;
    ক্ষেত্র LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); বিরতি;
    ক্ষেত্র LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); বিরতি;
    ক্ষেত্র LUA_VNIL:
      printf("nil"); বিরতি;
    ক্ষেত্র LUA_VLCF:
      printf("light C function: %p", fvalue(v)); বিরতি;
    ক্ষেত্র LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); বিরতি;
    ক্ষেত্র LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); বিরতি;
    ক্ষেত্র LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); বিরতি;
    ক্ষেত্র LUA_VTABLE:
      printf("table: %p", hvalue(v)); বিরতি;
    পূর্বনির্ধারিত:
      lua_assert(0);
  }
}


স্থির পূর্ণসংখ্যা testobjref (global_State *g, GCObject *f, GCObject *t) {
  পূর্ণসংখ্যা r1 = testobjref1(g, f, t);
  যদি (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ফেরত r1;
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


স্থির শূন্য checkvalref (global_State *g, GCObject *f, ধ্রুবক TValue *t) {
  প্রতিজ্ঞা(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


স্থির শূন্য checktable (global_State *g, Table *h) {
  unsigned পূর্ণসংখ্যা i;
  unsigned পূর্ণসংখ্যা asize = h->asize;
  গ্রন্থি_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  জন্য (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  জন্য (n = gnode(h, 0); n < limit; n++) {
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
  পূর্ণসংখ্যা i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  জন্য (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


স্থির শূন্য checkproto (global_State *g, Proto *f) {
  পূর্ণসংখ্যা i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  জন্য (i=0; i<f->sizek; i++) {
    যদি (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  জন্য (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  জন্য (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  জন্য (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


স্থির শূন্য checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  পূর্ণসংখ্যা i;
  জন্য (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


স্থির শূন্য checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  পূর্ণসংখ্যা i;
  checkobjrefN(g, clgc, cl->p);
  জন্য (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    যদি (uv) {
      checkobjrefN(g, clgc, uv);
      যদি (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


স্থির পূর্ণসংখ্যা lua_checkpc (CallInfo *ci) {
  যদি (!isLua(ci)) ফেরত 1;
  অন্যথা {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ফেরত p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


স্থির শূন্য check_স্তূপ (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  প্রতিজ্ঞা(!isdead(g, L1));
  যদি (L1->স্তূপ.p == ফাঁকা) {  /* incomplete thread? */
    প্রতিজ্ঞা(L1->openupval == ফাঁকা && L1->ci == ফাঁকা);
    ফেরত;
  }
  জন্য (uv = L1->openupval; uv != ফাঁকা; uv = uv->u.open.পরবর্তী)
    প্রতিজ্ঞা(upisopen(uv));  /* must be open */
  প্রতিজ্ঞা(L1->শীর্ষ.p <= L1->স্তূপ_last.p);
  প্রতিজ্ঞা(L1->tbclist.p <= L1->শীর্ষ.p);
  জন্য (ci = L1->ci; ci != ফাঁকা; ci = ci->previous) {
    প্রতিজ্ঞা(ci->শীর্ষ.p <= L1->স্তূপ_last.p);
    প্রতিজ্ঞা(lua_checkpc(ci));
  }
  জন্য (o = L1->স্তূপ.p; o < L1->স্তূপ_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


স্থির শূন্য checkrefs (global_State *g, GCObject *o) {
  নির্বাচন (o->tt) {
    ক্ষেত্র LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      বিরতি;
    }
    ক্ষেত্র LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      বিরতি;
    }
    ক্ষেত্র LUA_VTABLE: {
      checktable(g, gco2t(o));
      বিরতি;
    }
    ক্ষেত্র LUA_VTHREAD: {
      check_স্তূপ(g, gco2th(o));
      বিরতি;
    }
    ক্ষেত্র LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      বিরতি;
    }
    ক্ষেত্র LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      বিরতি;
    }
    ক্ষেত্র LUA_VPROTO: {
      checkproto(g, gco2p(o));
      বিরতি;
    }
    ক্ষেত্র LUA_VSHRSTR:
    ক্ষেত্র LUA_VLNGSTR: {
      প্রতিজ্ঞা(!isgray(o));  /* strings are never gray */
      বিরতি;
    }
    পূর্বনির্ধারিত: প্রতিজ্ঞা(0);
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
স্থির শূন্য checkobject (global_State *g, GCObject *o, পূর্ণসংখ্যা maybedead,
                         পূর্ণসংখ্যা listage) {
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
  পূর্ণসংখ্যা যোগ = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  যখন (o) {
    প্রতিজ্ঞা(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    প্রতিজ্ঞা(!testbit(o->marked, TESTBIT));
    যদি (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    যোগ++;
    নির্বাচন (o->tt) {
      ক্ষেত্র LUA_VTABLE: o = gco2t(o)->gclist; বিরতি;
      ক্ষেত্র LUA_VLCL: o = gco2lcl(o)->gclist; বিরতি;
      ক্ষেত্র LUA_VCCL: o = gco2ccl(o)->gclist; বিরতি;
      ক্ষেত্র LUA_VTHREAD: o = gco2th(o)->gclist; বিরতি;
      ক্ষেত্র LUA_VPROTO: o = gco2p(o)->gclist; বিরতি;
      ক্ষেত্র LUA_VUSERDATA:
        প্রতিজ্ঞা(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        বিরতি;
      পূর্বনির্ধারিত: প্রতিজ্ঞা(0);  /* other objects cannot be in a gray list */
    }
  }
  ফেরত যোগ;
}


/*
** Check objects in gray lists.
*/
স্থির l_mem checkgrays (global_State *g) {
  l_mem যোগ = 0;  /* count number of elements in all lists */
  যদি (!keepinvariant(g)) ফেরত যোগ;
  যোগ += checkgraylist(g, g->gray);
  যোগ += checkgraylist(g, g->grayagain);
  যোগ += checkgraylist(g, g->weak);
  যোগ += checkgraylist(g, g->allweak);
  যোগ += checkgraylist(g, g->ephemeron);
  ফেরত যোগ;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
স্থির শূন্য incifingray (global_State *g, GCObject *o, l_mem *গুনতি) {
  যদি (!keepinvariant(g))
    ফেরত;  /* gray lists not being kept in these phases */
  যদি (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    প্রতিজ্ঞা(!isgray(o) || upisopen(gco2upv(o)));
    ফেরত;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  যদি (isgray(o) || getage(o) == G_TOUCHED2) {
    (*গুনতি)++;
    প্রতিজ্ঞা(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


স্থির l_mem checklist (global_State *g, পূর্ণসংখ্যা maybedead, পূর্ণসংখ্যা tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem যোগ = 0;  /* number of object that should be in  gray lists */
  জন্য (o = newl; o != survival; o = o->পরবর্তী) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &যোগ);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  জন্য (o = survival; o != old; o = o->পরবর্তী) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &যোগ);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  জন্য (o = old; o != reallyold; o = o->পরবর্তী) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &যোগ);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  জন্য (o = reallyold; o != ফাঁকা; o = o->পরবর্তী) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &যোগ);
    প্রতিজ্ঞা(!tof == !tofinalize(o));
  }
  ফেরত যোগ;
}


পূর্ণসংখ্যা lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  পূর্ণসংখ্যা maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  যদি (keepinvariant(g)) {
    প্রতিজ্ঞা(!iswhite(mainthread(g)));
    প্রতিজ্ঞা(!iswhite(gcvalue(&g->l_registry)));
  }
  প্রতিজ্ঞা(!isdead(g, gcvalue(&g->l_registry)));
  প্রতিজ্ঞা(g->sweepgc == ফাঁকা || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  জন্য (o = g->fixedgc; o != ফাঁকা; o = o->পরবর্তী) {
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
  জন্য (o = g->tobefnz; o != ফাঁকা; o = o->পরবর্তী) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    প্রতিজ্ঞা(tofinalize(o));
    প্রতিজ্ঞা(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  যদি (keepinvariant(g))
    প্রতিজ্ঞা(totalin == totalshould);
  ফেরত 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


স্থির অক্ষর *buildop (Proto *p, পূর্ণসংখ্যা pc, অক্ষর *buff) {
  অক্ষর *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  ধ্রুবক অক্ষর *name = opnames[o];
  পূর্ণসংখ্যা সারি = luaG_getfuncline(p, pc);
  পূর্ণসংখ্যা lineinfo = (p->lineinfo != ফাঁকা) ? p->lineinfo[pc] : 0;
  যদি (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  অন্যথা
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", সারি, pc);
  নির্বাচন (getOpMode(o)) {
    ক্ষেত্র iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      বিরতি;
    ক্ষেত্র ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      বিরতি;
    ক্ষেত্র iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      বিরতি;
    ক্ষেত্র iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      বিরতি;
    ক্ষেত্র iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      বিরতি;
    ক্ষেত্র isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      বিরতি;
  }
  ফেরত obuff;
}


#যদি 0
শূন্য luaI_printcode (Proto *pt, পূর্ণসংখ্যা আকার_2) {
  পূর্ণসংখ্যা pc;
  জন্য (pc=0; pc<আকার_2; pc++) {
    অক্ষর buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


শূন্য luaI_printinst (Proto *pt, পূর্ণসংখ্যা pc) {
  অক্ষর buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


স্থির পূর্ণসংখ্যা listcode (lua_State *L) {
  পূর্ণসংখ্যা pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  জন্য (pc=0; pc<p->sizecode; pc++) {
    অক্ষর buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা printcode (lua_State *L) {
  পূর্ণসংখ্যা pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  জন্য (pc=0; pc<p->sizecode; pc++) {
    অক্ষর buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা listk (lua_State *L) {
  Proto *p;
  পূর্ণসংখ্যা i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  জন্য (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা listabslineinfo (lua_State *L) {
  Proto *p;
  পূর্ণসংখ্যা i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ফাঁকা, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  জন্য (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].সারি);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা listlocals (lua_State *L) {
  Proto *p;
  পূর্ণসংখ্যা pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  পূর্ণসংখ্যা i = 0;
  ধ্রুবক অক্ষর *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  যখন ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ফাঁকা)
    lua_pushstring(L, name);
  ফেরত i-1;
}

/* }====================================================== */



শূন্য lua_printstack (lua_State *L) {
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n = lua_gettop(L);
  printf("stack: >>\n");
  জন্য (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


পূর্ণসংখ্যা lua_printallstack (lua_State *L) {
  StkId p;
  পূর্ণসংখ্যা i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  জন্য (p = L->স্তূপ.p; p < L->শীর্ষ.p; p++) {
    যদি (ci != ফাঁকা && p == ci->func.p) {
      printf("  ---\n");
      যদি (ci == L->ci)
        ci = ফাঁকা;  /* printed last frame */
      অন্যথা
        ci = ci->পরবর্তী;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", অঙ্ক_OPCODES);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", আকার(lua_State));
  setnameval(L, "global state", আকার(global_State));
  setnameval(L, "TValue", আকার(TValue));
  setnameval(L, "Node", আকার(গ্রন্থি_2));
  setnameval(L, "stack Value", আকার(স্তূপ_মান));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা mem_query (lua_State *L) {
  যদি (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.যোগ));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ফেরত 3;
  }
  অন্যথা যদি (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    যদি (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ফেরত 0;
  }
  অন্যথা {
    ধ্রুবক অক্ষর *t = luaL_checkstring(L, 1);
    পূর্ণসংখ্যা i;
    জন্য (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      যদি (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ফেরত 1;
      }
    }
    ফেরত luaL_error(L, "unknown type '%s'", t);
  }
}


স্থির পূর্ণসংখ্যা alloc_গুনতি (lua_State *L) {
  যদি (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  অন্যথা
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা settrick (lua_State *L) {
  যদি (ttisnil(obj_at(L, 1)))
    l_Trick = ফাঁকা;
  অন্যথা
    l_Trick = gcvalue(obj_at(L, 1));
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা gc_color (lua_State *L) {
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
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  যদি (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  অন্যথা {
    স্থির ধ্রুবক অক্ষর *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা gc_printobj (lua_State *L) {
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
  ফেরত 0;
}


স্থির ধ্রুবক অক্ষর *ধ্রুবক statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

স্থির পূর্ণসংখ্যা gc_state (lua_State *L) {
  স্থির ধ্রুবক পূর্ণসংখ্যা states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  পূর্ণসংখ্যা option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  যদি (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ফেরত 1;
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
    ফেরত 0;
  }
}


স্থির পূর্ণসংখ্যা tracinggc = 0;
শূন্য luai_tracegctest (lua_State *L, পূর্ণসংখ্যা first) {
  যদি (!tracinggc) ফেরত;
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


স্থির পূর্ণসংখ্যা tracegc (lua_State *L) {
  যদি (lua_isnil(L, 1))
    tracinggc = 0;
  অন্যথা {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা hash_query (lua_State *L) {
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
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->গ্রন্থি));
  }
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা stacklevel (lua_State *L) {
  পূর্ণসংখ্যা a = 0;
  lua_pushinteger(L, cast_Integer(L->শীর্ষ.p - L->স্তূপ.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ফেরত 5;
}


স্থির পূর্ণসংখ্যা resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  যখন (ci->পরবর্তী != ফাঁকা) {
    CallInfo *tofree = ci->পরবর্তী;
    ci->পরবর্তী = ci->পরবর্তী->পরবর্তী;
    luaM_free(L, tofree);
    L->nci--;
  }
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা reallocstack (lua_State *L) {
  পূর্ণসংখ্যা n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->শীর্ষ.p - L->স্তূপ.p) + n, 1);
  lua_unlock(L);
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা table_query (lua_State *L) {
  ধ্রুবক Table *t;
  পূর্ণসংখ্যা i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned পূর্ণসংখ্যা asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  যদি (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ফেরত 3;
  }
  অন্যথা যদি (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    যদি (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->শীর্ষ.p));
    অন্যথা
      setnilvalue2s(L->শীর্ষ.p);
    api_incr_শীর্ষ(L);
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
    lua_pushinteger(L, gnext(&t->গ্রন্থি[i]));
  }
  ফেরত 3;
}


স্থির পূর্ণসংখ্যা gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ফেরত 6;
}


স্থির পূর্ণসংখ্যা test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  পূর্ণসংখ্যা s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  যদি (s == -1) {
    lua_pushinteger(L ,tb->আকার_2);
    lua_pushinteger(L ,tb->nuse);
    ফেরত 2;
  }
  অন্যথা যদি (s < tb->আকার_2) {
    TString *ts;
    পূর্ণসংখ্যা n = 0;
    জন্য (ts = tb->hash[s]; ts != ফাঁকা; ts = ts->u.hnext) {
      setsvalue2s(L, L->শীর্ষ.p, ts);
      api_incr_শীর্ষ(L);
      n++;
    }
    ফেরত n;
  }
  অন্যথা ফেরত 0;
}


স্থির পূর্ণসংখ্যা getreftable (lua_State *L) {
  যদি (lua_istable(L, 2))  /* is there a table as second argument? */
    ফেরত 2;  /* use it as the table */
  অন্যথা
    ফেরত LUA_REGISTRYINDEX;  /* default is to use the register */
}


স্থির পূর্ণসংখ্যা tref (lua_State *L) {
  পূর্ণসংখ্যা t = getreftable(L);
  পূর্ণসংখ্যা level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা getref (lua_State *L) {
  পূর্ণসংখ্যা t = getreftable(L);
  পূর্ণসংখ্যা level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ফেরত 1;
}

স্থির পূর্ণসংখ্যা unref (lua_State *L) {
  পূর্ণসংখ্যা t = getreftable(L);
  পূর্ণসংখ্যা level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা upvalue (lua_State *L) {
  পূর্ণসংখ্যা n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  যদি (lua_isnone(L, 3)) {
    ধ্রুবক অক্ষর *name = lua_getupvalue(L, 1, n);
    যদি (name == ফাঁকা) ফেরত 0;
    lua_pushstring(L, name);
    ফেরত 2;
  }
  অন্যথা {
    ধ্রুবক অক্ষর *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ফেরত 1;
  }
}


স্থির পূর্ণসংখ্যা newuserdata (lua_State *L) {
  size_t আকার_2 = cast_sizet(luaL_optinteger(L, 1, 0));
  পূর্ণসংখ্যা nuv = cast_int(luaL_optinteger(L, 2, 0));
  অক্ষর *p = cast_charp(lua_newuserdatauv(L, আকার_2, nuv));
  যখন (আকার_2--) *p++ = '\0';
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &l);
  পূর্ণসংখ্যা status = luaL_loadbufferx(L1, s, l, s, "t");
  যদি (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা s2d (lua_State *L) {
  lua_pushnumber(L, cast_অঙ্ক(*cast(ধ্রুবক দ্বিগুণ *, luaL_checkstring(L, 1))));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা d2s (lua_State *L) {
  দ্বিগুণ d = cast(দ্বিগুণ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), আকার(d));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা newstate (lua_State *L) {
  শূন্য *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  যদি (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  অন্যথা
    lua_pushnil(L);
  ফেরত 1;
}


স্থির lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ফাঁকা, 1, "state expected");
  ফেরত L1;
}


স্থির পূর্ণসংখ্যা loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  পূর্ণসংখ্যা load = cast_int(luaL_checkinteger(L, 2));
  পূর্ণসংখ্যা preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ফাঁকা, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ফেরত 0;
}

স্থির পূর্ণসংখ্যা closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ফেরত 0;
}

স্থির পূর্ণসংখ্যা doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ধ্রুবক অক্ষর *code = luaL_checklstring(L, 2, &lcode);
  পূর্ণসংখ্যা status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  যদি (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  যদি (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ফেরত 3;
  }
  অন্যথা {
    পূর্ণসংখ্যা i = 0;
    যখন (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_বের_করো(L1, i-1);
    ফেরত i-1;
  }
}


স্থির পূর্ণসংখ্যা log2_aux (lua_State *L) {
  unsigned পূর্ণসংখ্যা x = (unsigned পূর্ণসংখ্যা)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ফেরত 1;
}


কাঠামো Aux { jmp_buf jb; ধ্রুবক অক্ষর *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
স্থির পূর্ণসংখ্যা panicback (lua_State *L) {
  কাঠামো Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (কাঠামো Aux *)lua_touserdata(L, -1);
  lua_বের_করো(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ফেরত 1;  /* to avoid warnings */
}

স্থির পূর্ণসংখ্যা checkpanic (lua_State *L) {
  কাঠামো Aux b;
  শূন্য *ud;
  lua_State *L1;
  ধ্রুবক অক্ষর *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  যদি (L1 == ফাঁকা) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ফেরত 1;
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
  ফেরত 1;
}


স্থির পূর্ণসংখ্যা externKstr (lua_State *L) {
  size_t len;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ফাঁকা, ফাঁকা);
  ফেরত 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
স্থির পূর্ণসংখ্যা externstr (lua_State *L) {
  size_t len;
  ধ্রুবক অক্ষর *s = luaL_checklstring(L, 1, &len);
  শূন্য *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  অক্ষর *buff = cast_charp((*allocf)(ud, ফাঁকা, 0, len + 1));
  যদি (buff == ফাঁকা) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * আকার(অক্ষর));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ফেরত 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


স্থির শূন্য sethookaux (lua_State *L, পূর্ণসংখ্যা mask, পূর্ণসংখ্যা গুনতি, ধ্রুবক অক্ষর *code);

স্থির ধ্রুবক অক্ষর *ধ্রুবক delimits = " \t\n,;";

স্থির শূন্য skip (ধ্রুবক অক্ষর **pc) {
  জন্য (;;) {
    যদি (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    অন্যথা যদি (**pc == '#') {  /* comment? */
      যখন (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    অন্যথা বিরতি;
  }
}

স্থির পূর্ণসংখ্যা getnum_aux (lua_State *L, lua_State *L1, ধ্রুবক অক্ষর **pc) {
  পূর্ণসংখ্যা res = 0;
  পূর্ণসংখ্যা sig = 1;
  skip(pc);
  যদি (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_বের_করো(L1, 1);
    (*pc)++;
    ফেরত res;
  }
  অন্যথা যদি (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ফেরত res;
  }
  অন্যথা যদি (**pc == '!') {
    (*pc)++;
    যদি (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    অন্যথা যদি (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    অন্যথা lua_assert(0);
    (*pc)++;
    ফেরত res;
  }
  অন্যথা যদি (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  যদি (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  যখন (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ফেরত sig*res;
}

স্থির ধ্রুবক অক্ষর *getstring_aux (lua_State *L, অক্ষর *buff, ধ্রুবক অক্ষর **pc) {
  পূর্ণসংখ্যা i = 0;
  skip(pc);
  যদি (**pc == '"' || **pc == '\'') {  /* quoted string? */
    পূর্ণসংখ্যা quote = *(*pc)++;
    যখন (**pc != quote) {
      যদি (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  অন্যথা {
    যখন (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ফেরত buff;
}


স্থির পূর্ণসংখ্যা getindex_aux (lua_State *L, lua_State *L1, ধ্রুবক অক্ষর **pc) {
  skip(pc);
  নির্বাচন (*(*pc)++) {
    ক্ষেত্র 'R': ফেরত LUA_REGISTRYINDEX;
    ক্ষেত্র 'U': ফেরত lua_upvalueindex(getnum_aux(L, L1, pc));
    পূর্বনির্ধারিত: {
      পূর্ণসংখ্যা n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      যদি (n == 0) ফেরত 0;
      অন্যথা ফেরত lua_absindex(L1, n);
    }
  }
}


স্থির ধ্রুবক অক্ষর *ধ্রুবক statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
স্থির শূন্য regcodes (lua_State *L) {
  unsigned পূর্ণসংখ্যা i;
  জন্য (i = 0; i < আকার(statcodes) / আকার(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


স্থির পূর্ণসংখ্যা testC (lua_State *L);
স্থির পূর্ণসংখ্যা Cfunck (lua_State *L, পূর্ণসংখ্যা status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
স্থির ধ্রুবক অক্ষর ops[] = "+-*%^/\\&|~<>_!";

স্থির পূর্ণসংখ্যা runC (lua_State *L, lua_State *L1, ধ্রুবক অক্ষর *pc) {
  অক্ষর buff[300];
  পূর্ণসংখ্যা status = 0;
  যদি (pc == ফাঁকা) ফেরত luaL_error(L, "attempt to runC null script");
  জন্য (;;) {
    ধ্রুবক অক্ষর *inst = getstring;
    যদি EQ("") ফেরত 0;
    অন্যথা যদি EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    অন্যথা যদি EQ("append") {
      পূর্ণসংখ্যা t = getindex;
      পূর্ণসংখ্যা i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    অন্যথা যদি EQ("arith") {
      পূর্ণসংখ্যা op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    অন্যথা যদি EQ("call") {
      পূর্ণসংখ্যা narg = getnum;
      পূর্ণসংখ্যা nres = getnum;
      lua_call(L1, narg, nres);
    }
    অন্যথা যদি EQ("callk") {
      পূর্ণসংখ্যা narg = getnum;
      পূর্ণসংখ্যা nres = getnum;
      পূর্ণসংখ্যা i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    অন্যথা যদি EQ("checkstack") {
      পূর্ণসংখ্যা sz = getnum;
      ধ্রুবক অক্ষর *বার্তা = getstring;
      যদি (*বার্তা == '\0')
        বার্তা = ফাঁকা;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, বার্তা);
    }
    অন্যথা যদি EQ("rawcheckstack") {
      পূর্ণসংখ্যা sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    অন্যথা যদি EQ("compare") {
      ধ্রুবক অক্ষর *opt = getstring;  /* EQ, LT, or LE */
      পূর্ণসংখ্যা op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      পূর্ণসংখ্যা a = getindex;
      পূর্ণসংখ্যা b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    অন্যথা যদি EQ("concat") {
      lua_concat(L1, getnum);
    }
    অন্যথা যদি EQ("copy") {
      পূর্ণসংখ্যা f = getindex;
      lua_copy(L1, f, getindex);
    }
    অন্যথা যদি EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    অন্যথা যদি EQ("getfield") {
      পূর্ণসংখ্যা t = getindex;
      পূর্ণসংখ্যা tp = lua_getfield(L1, t, getstring);
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
      পূর্ণসংখ্যা tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    অন্যথা যদি EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    অন্যথা যদি EQ("gsub") {
      পূর্ণসংখ্যা a = getnum; পূর্ণসংখ্যা b = getnum; পূর্ণসংখ্যা c = getnum;
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
      ধ্রুবক অক্ষর *s = luaL_checklstring(L1, getnum, &slen);
      ধ্রুবক অক্ষর *name = getstring;
      ধ্রুবক অক্ষর *mode = getstring;
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
      lua_পরবর্তী(L1, -2);
    }
    অন্যথা যদি EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    অন্যথা যদি EQ("pcall") {
      পূর্ণসংখ্যা narg = getnum;
      পূর্ণসংখ্যা nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    অন্যথা যদি EQ("pcallk") {
      পূর্ণসংখ্যা narg = getnum;
      পূর্ণসংখ্যা nres = getnum;
      পূর্ণসংখ্যা i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    অন্যথা যদি EQ("pop") {
      lua_বের_করো(L1, getnum);
    }
    অন্যথা যদি EQ("printstack") {
      পূর্ণসংখ্যা n = getnum;
      যদি (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      অন্যথা lua_printstack(L1);
    }
    অন্যথা যদি EQ("print") {
      ধ্রুবক অক্ষর *বার্তা = getstring;
      printf("%s\n", বার্তা);
    }
    অন্যথা যদি EQ("warningC") {
      ধ্রুবক অক্ষর *বার্তা = getstring;
      lua_warning(L1, বার্তা, 1);
    }
    অন্যথা যদি EQ("warning") {
      ধ্রুবক অক্ষর *বার্তা = getstring;
      lua_warning(L1, বার্তা, 0);
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
      lua_pushfstring(L1, lua_tostring(L, -2), (পূর্ণসংখ্যা)lua_tointeger(L, -1));
    }
    অন্যথা যদি EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    অন্যথা যদি EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    অন্যথা যদি EQ("rawget") {
      পূর্ণসংখ্যা t = getindex;
      lua_rawget(L1, t);
    }
    অন্যথা যদি EQ("rawgeti") {
      পূর্ণসংখ্যা t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    অন্যথা যদি EQ("rawgetp") {
      পূর্ণসংখ্যা t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    অন্যথা যদি EQ("rawset") {
      পূর্ণসংখ্যা t = getindex;
      lua_rawset(L1, t);
    }
    অন্যথা যদি EQ("rawseti") {
      পূর্ণসংখ্যা t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    অন্যথা যদি EQ("rawsetp") {
      পূর্ণসংখ্যা t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    অন্যথা যদি EQ("remove") {
      lua_remove(L1, getnum);
    }
    অন্যথা যদি EQ("replace") {
      lua_replace(L1, getindex);
    }
    অন্যথা যদি EQ("resume") {
      পূর্ণসংখ্যা i = getindex;
      পূর্ণসংখ্যা nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    অন্যথা যদি EQ("traceback") {
      ধ্রুবক অক্ষর *বার্তা = getstring;
      পূর্ণসংখ্যা level = getnum;
      luaL_traceback(L1, L1, বার্তা, level);
    }
    অন্যথা যদি EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    অন্যথা যদি EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    অন্যথা যদি EQ("return") {
      পূর্ণসংখ্যা n = getnum;
      যদি (L1 != L) {
        পূর্ণসংখ্যা i;
        জন্য (i = 0; i < n; i++) {
          পূর্ণসংখ্যা idx = -(n - i);
          নির্বাচন (lua_type(L1, idx)) {
            ক্ষেত্র LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              বিরতি;
            পূর্বনির্ধারিত:
              lua_pushstring(L, lua_tostring(L1, idx));
              বিরতি;
          }
        }
      }
      ফেরত n;
    }
    অন্যথা যদি EQ("rotate") {
      পূর্ণসংখ্যা i = getindex;
      lua_rotate(L1, i, getnum);
    }
    অন্যথা যদি EQ("setfield") {
      পূর্ণসংখ্যা t = getindex;
      ধ্রুবক অক্ষর *s = getstring;
      lua_setfield(L1, t, s);
    }
    অন্যথা যদি EQ("seti") {
      পূর্ণসংখ্যা t = getindex;
      lua_seti(L1, t, getnum);
    }
    অন্যথা যদি EQ("setglobal") {
      ধ্রুবক অক্ষর *s = getstring;
      lua_setglobal(L1, s);
    }
    অন্যথা যদি EQ("sethook") {
      পূর্ণসংখ্যা mask = getnum;
      পূর্ণসংখ্যা গুনতি = getnum;
      ধ্রুবক অক্ষর *s = getstring;
      sethookaux(L1, mask, গুনতি, s);
    }
    অন্যথা যদি EQ("setmetatable") {
      পূর্ণসংখ্যা idx = getindex;
      lua_setmetatable(L1, idx);
    }
    অন্যথা যদি EQ("settable") {
      lua_settable(L1, getindex);
    }
    অন্যথা যদি EQ("settop") {
      lua_settop(L1, getnum);
    }
    অন্যথা যদি EQ("testudata") {
      পূর্ণসংখ্যা i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ফাঁকা);
    }
    অন্যথা যদি EQ("error") {
      lua_error(L1);
    }
    অন্যথা যদি EQ("abort") {
      abort();
    }
    অন্যথা যদি EQ("throw") {
#যদি defined(__cplusplus)
স্থির কাঠামো X { পূর্ণসংখ্যা x; } x;
      throw x;
#অন্যথা
      luaL_error(L1, "C++");
#endif
      বিরতি;
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
      ধ্রুবক অক্ষর *s = lua_tostring(L1, getindex);
      ধ্রুবক অক্ষর *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ফাঁকা && s1 == ফাঁকা) || strcmp(s, s1) == 0);
    }
    অন্যথা যদি EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ফাঁকা);
    }
    অন্যথা যদি EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    অন্যথা যদি EQ("xmove") {
      পূর্ণসংখ্যা f = getindex;
      পূর্ণসংখ্যা t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      পূর্ণসংখ্যা n = getnum;
      যদি (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    অন্যথা যদি EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    অন্যথা যদি EQ("yield") {
      ফেরত lua_yield(L1, getnum);
    }
    অন্যথা যদি EQ("yieldk") {
      পূর্ণসংখ্যা nres = getnum;
      পূর্ণসংখ্যা i = getindex;
      ফেরত lua_yieldk(L1, nres, i, Cfunck);
    }
    অন্যথা যদি EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    অন্যথা যদি EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    অন্যথা যদি EQ("argerror") {
      পূর্ণসংখ্যা arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    অন্যথা luaL_error(L, "unknown instruction %s", buff);
  }
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা testC (lua_State *L) {
  lua_State *L1;
  ধ্রুবক অক্ষর *pc;
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
  ফেরত runC(L, L1, pc);
}


স্থির পূর্ণসংখ্যা Cfunc (lua_State *L) {
  ফেরত runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


স্থির পূর্ণসংখ্যা Cfunck (lua_State *L, পূর্ণসংখ্যা status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ফেরত runC(L, L, lua_tostring(L, cast_int(ctx)));
}


স্থির পূর্ণসংখ্যা makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ফেরত 1;
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
  ধ্রুবক অক্ষর *scpt;
  ধ্রুবক অক্ষর *ধ্রুবক events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_বের_করো(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
স্থির শূন্য sethookaux (lua_State *L, পূর্ণসংখ্যা mask, পূর্ণসংখ্যা গুনতি, ধ্রুবক অক্ষর *scpt) {
  যদি (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ফাঁকা, 0, 0);  /* turn off hooks */
    ফেরত;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  যদি (!lua_istable(L, -1)) {  /* no hook table? */
    lua_বের_করো(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, গুনতি);
}


স্থির পূর্ণসংখ্যা sethook (lua_State *L) {
  যদি (lua_isnoneornil(L, 1))
    lua_sethook(L, ফাঁকা, 0, 0);  /* turn off hooks */
  অন্যথা {
    ধ্রুবক অক্ষর *scpt = luaL_checkstring(L, 1);
    ধ্রুবক অক্ষর *smask = luaL_checkstring(L, 2);
    পূর্ণসংখ্যা গুনতি = cast_int(luaL_optinteger(L, 3, 0));
    পূর্ণসংখ্যা mask = 0;
    যদি (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    যদি (strchr(smask, 'r')) mask |= LUA_MASKRET;
    যদি (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    যদি (গুনতি > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, গুনতি, scpt);
  }
  ফেরত 0;
}


স্থির পূর্ণসংখ্যা coresume (lua_State *L) {
  পূর্ণসংখ্যা status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  যদি (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_ঢোকাও(L, -2);
    ফেরত 2;  /* return false + error message */
  }
  অন্যথা {
    lua_pushboolean(L, 1);
    ফেরত 1;
  }
}

#যদি !defined(LUA_USE_POSIX)

#define nonblock	ফাঁকা

#অন্যথা

#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <fcntl.h>

স্থির পূর্ণসংখ্যা nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  পূর্ণসংখ্যা fd = fileno(f);
  পূর্ণসংখ্যা flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ফেরত 0;
}
#endif

/* }====================================================== */



স্থির ধ্রুবক কাঠামো luaL_Reg tests_funcs[] = {
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
  {ফাঁকা, ফাঁকা}
};


স্থির শূন্য checkfinalmem (শূন্য) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.যোগ == 0);
}


পূর্ণসংখ্যা luaB_opentests (lua_State *L) {
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
  ফেরত 1;
}

#endif

