/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#నిర్వచించు ltests_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <limits.h>
#అన్తర్భావ <setjmp.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "lauxlib.h"
#అన్తర్భావ "lcode.h"
#అన్తర్భావ "lctype.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lopcodes.h"
#అన్తర్భావ "lopnames.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#ఒకవేళ defined(LUA_DEBUG)


శూన్య *l_Trick = 0;


#నిర్వచించు obj_at(L,k)	s2v(L->ci->func.p + (k))


స్థిర పూర్ణాంకం నిరంతరం_C (lua_State *L, lua_State *L1, స్థిరరాశి అక్షరం *pc);


స్థిర శూన్య setnameval (lua_State *L, స్థిరరాశి అక్షరం *name, పూర్ణాంకం val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


స్థిర శూన్య pushobject (lua_State *L, స్థిరరాశి TValue *o) {
  setobj2s(L, L->శిఖరం.p, o);
  api_incr_శిఖరం(L);
}


స్థిర శూన్య badexit (స్థిరరాశి అక్షరం *fmt, స్థిరరాశి అక్షరం *s1, స్థిరరాశి అక్షరం *s2) {
  fprintf(stderr, fmt, s1);
  ఒకవేళ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.మొత్తం = 0;
  నిష్క్రమించు(EXIT_FAILURE);
}


స్థిర పూర్ణాంకం tpanic (lua_State *L) {
  స్థిరరాశి అక్షరం *సందేశం = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  తిరిగి (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   సందేశం, ఖాళీ),
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
స్థిర శూన్య warnf (శూన్య *ud, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం tocont) {
  lua_State *L = cast(lua_State *, ud);
  స్థిర అక్షరం buff[200] = "";  /* should be enough for tests... */
  స్థిర పూర్ణాంకం onoff = 0;
  స్థిర పూర్ణాంకం బహుళకం = 0;  /* start in normal mode */
  స్థిర పూర్ణాంకం lasttocont = 0;
  ఒకవేళ (!lasttocont && !tocont && *సందేశం == '@') {  /* control message? */
    ఒకవేళ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", సందేశం, buff);
    ఒకవేళ (strcmp(సందేశం, "@off") == 0)
      onoff = 0;
    లేకపోతే ఒకవేళ (strcmp(సందేశం, "@on") == 0)
      onoff = 1;
    లేకపోతే ఒకవేళ (strcmp(సందేశం, "@normal") == 0)
      బహుళకం = 0;
    లేకపోతే ఒకవేళ (strcmp(సందేశం, "@allow") == 0)
      బహుళకం = 1;
    లేకపోతే ఒకవేళ (strcmp(సందేశం, "@store") == 0)
      బహుళకం = 2;
    లేకపోతే
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              సందేశం, ఖాళీ);
    తిరిగి;
  }
  lasttocont = tocont;
  ఒకవేళ (అక్షరశ్రేణి_పొడవు(సందేశం) >= పరిమాణం(buff) - అక్షరశ్రేణి_పొడవు(buff))
    badexit("warnf-buffer overflow (%s)\n", సందేశం, buff);
  strcat(buff, సందేశం);  /* add new message to current warning */
  ఒకవేళ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ఒకవేళ (!lua_toboolean(L, -1))
      lua_తీయి(L, 1);  /* ok, no previous unexpected warning */
    లేకపోతే {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_తాళం(L);
    ఎంపిక (బహుళకం) {
      స్థితి 0: {  /*సాధారణ*/
        ఒకవేళ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ఖాళీ);
      }  /* FALLTHROUGH */
      స్థితి 1: {  /* allow */
        ఒకవేళ (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        ఆపు;
      }
      స్థితి 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_తాళం(L);
        ఆపు;
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

#నిర్వచించు MARK		0x55  /* 01010101 (a nice pattern) */

రకనిర్వచనం సంఘం mem_శీర్షిక {
  LUAI_MAXALIGN;
  నిర్మాణం {
    size_t కొలత;
    పూర్ణాంకం type;
  } d;
} mem_శీర్షిక;


#ఒకవేళ !defined(EXTERNMEMCHECK)

/* full memory check */
#నిర్వచించు MARKSIZE	16  /* size of marks after each block */
#నిర్వచించు fillmem(mem,కొలత)	memset(mem, -MARK, కొలత)

#లేకపోతే

/* external memory check: don't do it twice */
#నిర్వచించు MARKSIZE	0
#నిర్వచించు fillmem(mem,కొలత)	/*ఖాళీలు*/

#అయితే_ముగింపు


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


స్థిర శూన్య freeblock (Memcontrol *mc, mem_శీర్షిక *ఖండం) {
  ఒకవేళ (ఖండం) {
    size_t కొలత = ఖండం->d.కొలత;
    పూర్ణాంకం i;
    కొరకు (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(ఖండం + 1) + కొలత + i) == MARK);
    mc->objcount[ఖండం->d.type]--;
    fillmem(ఖండం, పరిమాణం(mem_శీర్షిక) + కొలత + MARKSIZE);  /* erase block */
    విడుదల(ఖండం);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->మొత్తం -= కొలత;
  }
}


శూన్య *debug_realloc (శూన్య *ud, శూన్య *b, size_t oldsize, size_t కొలత) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  mem_శీర్షిక *ఖండం = cast(mem_శీర్షిక *, b);
  పూర్ణాంకం type;
  ఒకవేళ (mc->memlimit == 0) {  /* first time? */
    అక్షరం *హద్దు = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = హద్దు ? strtoul(హద్దు, ఖాళీ, 10) : ULONG_MAX;
  }
  ఒకవేళ (ఖండం == ఖాళీ) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  లేకపోతే {
    ఖండం--;  /* go to real header */
    type = ఖండం->d.type;
    lua_assert(oldsize == ఖండం->d.కొలత);
  }
  ఒకవేళ (కొలత == 0) {
    freeblock(mc, ఖండం);
    తిరిగి ఖాళీ;
  }
  ఒకవేళ (mc->failnext) {
    mc->failnext = 0;
    తిరిగి ఖాళీ;  /* fake a single memory allocation error */
  }
  ఒకవేళ (mc->countlimit != ~0UL && కొలత != oldsize) {  /* count limit in use? */
    ఒకవేళ (mc->countlimit == 0)
      తిరిగి ఖాళీ;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ఒకవేళ (కొలత > oldsize && mc->మొత్తం+కొలత-oldsize > mc->memlimit)
    తిరిగి ఖాళీ;  /* fake a memory allocation error */
  లేకపోతే {
    mem_శీర్షిక *newblock;
    పూర్ణాంకం i;
    size_t commonsize = (oldsize < కొలత) ? oldsize : కొలత;
    size_t realsize = పరిమాణం(mem_శీర్షిక) + కొలత + MARKSIZE;
    ఒకవేళ (realsize < కొలత) తిరిగి ఖాళీ;  /* arithmetic overflow! */
    newblock = cast(mem_శీర్షిక *, జ్ఞాపకం_ఇవ్వు(realsize));  /* alloc a new block */
    ఒకవేళ (newblock == ఖాళీ)
      తిరిగి ఖాళీ;  /* really out of memory? */
    ఒకవేళ (ఖండం) {
      జ్ఞాపకం_నకలు(newblock + 1, ఖండం + 1, commonsize);  /* copy old contents */
      freeblock(mc, ఖండం);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, కొలత - commonsize);
    /* initialize marks after block */
    కొరకు (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + కొలత + i) = MARK;
    newblock->d.కొలత = కొలత;
    newblock->d.type = type;
    mc->మొత్తం += కొలత;
    ఒకవేళ (mc->మొత్తం > mc->maxmem)
      mc->maxmem = mc->మొత్తం;
    mc->numblocks++;
    mc->objcount[type]++;
    తిరిగి newblock + 1;
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

#అన్తర్భావ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
స్థిర పూర్ణాంకం testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ఒకవేళ (isdead(g,t)) తిరిగి 0;
  ఒకవేళ (issweepphase(g))
    తిరిగి 1;  /* no invariants */
  లేకపోతే ఒకవేళ (g->gckind != KGC_GENMINOR)
    తిరిగి !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  లేకపోతే {  /* generational mode */
    ఒకవేళ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      తిరిగి 0;
    ఒకవేళ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      తిరిగి 0;
    తిరిగి 1;
  }
}


స్థిర శూన్య printobj (global_State *g, GCObject *o) {
  రాయి("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (శూన్య *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ఒకవేళ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    రాయి(" '%s'", getstr(gco2ts(o)));
}


శూన్య lua_printobj (lua_State *L, నిర్మాణం GCObject *o) {
  printobj(G(L), o);
}


శూన్య lua_printvalue (TValue *v) {
  ఎంపిక (ttypetag(v)) {
    స్థితి LUA_VNUMINT: స్థితి LUA_VNUMFLT: {
      అక్షరం buff[LUA_N2SBUFFSZ];
      గుర్తులేని len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      రాయి("%s", buff);
      ఆపు;
    }
    స్థితి LUA_VSHRSTR:
      రాయి("'%s'", getstr(tsvalue(v))); ఆపు;
    స్థితి LUA_VLNGSTR:
      రాయి("'%.30s...'", getstr(tsvalue(v))); ఆపు;
    స్థితి LUA_VFALSE:
      రాయి("%s", "false"); ఆపు;
    స్థితి LUA_VTRUE:
      రాయి("%s", "true"); ఆపు;
    స్థితి LUA_VLIGHTUSERDATA:
      రాయి("light udata: %p", pvalue(v)); ఆపు;
    స్థితి LUA_VUSERDATA:
      రాయి("full udata: %p", uvalue(v)); ఆపు;
    స్థితి LUA_VNIL:
      రాయి("nil"); ఆపు;
    స్థితి LUA_VLCF:
      రాయి("light C function: %p", fvalue(v)); ఆపు;
    స్థితి LUA_VCCL:
      రాయి("C closure: %p", clCvalue(v)); ఆపు;
    స్థితి LUA_VLCL:
      రాయి("Lua function: %p", clLvalue(v)); ఆపు;
    స్థితి LUA_VTHREAD:
      రాయి("thread: %p", thvalue(v)); ఆపు;
    స్థితి LUA_VTABLE:
      రాయి("table: %p", hvalue(v)); ఆపు;
    అప్రమేయ:
      lua_assert(0);
  }
}


స్థిర పూర్ణాంకం testobjref (global_State *g, GCObject *f, GCObject *t) {
  పూర్ణాంకం r1 = testobjref1(g, f, t);
  ఒకవేళ (!r1) {
    రాయి("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    రాయి("  ->  ");
    printobj(g, t);
    రాయి("\n");
  }
  తిరిగి r1;
}


స్థిర శూన్య checkobjref (global_State *g, GCObject *f, GCObject *t) {
    ప్రతిజ్ఞా(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#నిర్వచించు checkobjrefN(g,f,t)	{ ఒకవేళ (t) checkobjref(g,f,obj2gco(t)); }


స్థిర శూన్య checkvalref (global_State *g, GCObject *f, స్థిరరాశి TValue *t) {
  ప్రతిజ్ఞా(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


స్థిర శూన్య checktable (global_State *g, పట్టిక *h) {
  గుర్తులేని పూర్ణాంకం i;
  గుర్తులేని పూర్ణాంకం asize = h->asize;
  కణుపు_2 *n, *హద్దు = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  కొరకు (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  కొరకు (n = gnode(h, 0); n < హద్దు; n++) {
    ఒకవేళ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      ప్రతిజ్ఞా(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


స్థిర శూన్య checkudata (global_State *g, Udata *u) {
  పూర్ణాంకం i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  కొరకు (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


స్థిర శూన్య checkproto (global_State *g, Proto *f) {
  పూర్ణాంకం i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->ఆధారం);
  కొరకు (i=0; i<f->sizek; i++) {
    ఒకవేళ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  కొరకు (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  కొరకు (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  కొరకు (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


స్థిర శూన్య checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  పూర్ణాంకం i;
  కొరకు (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


స్థిర శూన్య checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  పూర్ణాంకం i;
  checkobjrefN(g, clgc, cl->p);
  కొరకు (i=0; i<cl->nupvalues; i++) {
    పైకి_Val *uv = cl->upvals[i];
    ఒకవేళ (uv) {
      checkobjrefN(g, clgc, uv);
      ఒకవేళ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


స్థిర పూర్ణాంకం lua_checkpc (CallInfo *ci) {
  ఒకవేళ (!isLua(ci)) తిరిగి 1;
  లేకపోతే {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    తిరిగి p->సంకేతం_2 <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->సంకేతం_2 + p->sizecode;
  }
}


స్థిర శూన్య check_దొంతర (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  పైకి_Val *uv;
  ప్రతిజ్ఞా(!isdead(g, L1));
  ఒకవేళ (L1->దొంతర.p == ఖాళీ) {  /* incomplete thread? */
    ప్రతిజ్ఞా(L1->openupval == ఖాళీ && L1->ci == ఖాళీ);
    తిరిగి;
  }
  కొరకు (uv = L1->openupval; uv != ఖాళీ; uv = uv->u.open.తదుపరి)
    ప్రతిజ్ఞా(upisopen(uv));  /* must be open */
  ప్రతిజ్ఞా(L1->శిఖరం.p <= L1->దొంతర_చివరి.p);
  ప్రతిజ్ఞా(L1->tbclist.p <= L1->శిఖరం.p);
  కొరకు (ci = L1->ci; ci != ఖాళీ; ci = ci->మునుపటి) {
    ప్రతిజ్ఞా(ci->శిఖరం.p <= L1->దొంతర_చివరి.p);
    ప్రతిజ్ఞా(lua_checkpc(ci));
  }
  కొరకు (o = L1->దొంతర.p; o < L1->దొంతర_చివరి.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


స్థిర శూన్య checkrefs (global_State *g, GCObject *o) {
  ఎంపిక (o->tt) {
    స్థితి LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      ఆపు;
    }
    స్థితి LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      ఆపు;
    }
    స్థితి LUA_VTABLE: {
      checktable(g, gco2t(o));
      ఆపు;
    }
    స్థితి LUA_VTHREAD: {
      check_దొంతర(g, gco2th(o));
      ఆపు;
    }
    స్థితి LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      ఆపు;
    }
    స్థితి LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      ఆపు;
    }
    స్థితి LUA_VPROTO: {
      checkproto(g, gco2p(o));
      ఆపు;
    }
    స్థితి LUA_VSHRSTR:
    స్థితి LUA_VLNGSTR: {
      ప్రతిజ్ఞా(!isgray(o));  /* strings are never gray */
      ఆపు;
    }
    అప్రమేయ: ప్రతిజ్ఞా(0);
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
స్థిర శూన్య checkobject (global_State *g, GCObject *o, పూర్ణాంకం maybedead,
                         పూర్ణాంకం listage) {
  ఒకవేళ (isdead(g, o))
    ప్రతిజ్ఞా(maybedead);
  లేకపోతే {
    ప్రతిజ్ఞా(g->gcstate != GCSpause || iswhite(o));
    ఒకవేళ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      ప్రతిజ్ఞా(getage(o) >= listage);
      ఒకవేళ (isold(o)) {
        ప్రతిజ్ఞా(!iswhite(o));
        ప్రతిజ్ఞా(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      ప్రతిజ్ఞా(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


స్థిర l_mem checkgraylist (global_State *g, GCObject *o) {
  పూర్ణాంకం మొత్తం = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  అయితే (o) {
    ప్రతిజ్ఞా(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    ప్రతిజ్ఞా(!testbit(o->marked, TESTBIT));
    ఒకవేళ (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    మొత్తం++;
    ఎంపిక (o->tt) {
      స్థితి LUA_VTABLE: o = gco2t(o)->gclist; ఆపు;
      స్థితి LUA_VLCL: o = gco2lcl(o)->gclist; ఆపు;
      స్థితి LUA_VCCL: o = gco2ccl(o)->gclist; ఆపు;
      స్థితి LUA_VTHREAD: o = gco2th(o)->gclist; ఆపు;
      స్థితి LUA_VPROTO: o = gco2p(o)->gclist; ఆపు;
      స్థితి LUA_VUSERDATA:
        ప్రతిజ్ఞా(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        ఆపు;
      అప్రమేయ: ప్రతిజ్ఞా(0);  /* other objects cannot be in a gray list */
    }
  }
  తిరిగి మొత్తం;
}


/*
** Check objects in gray lists.
*/
స్థిర l_mem checkgrays (global_State *g) {
  l_mem మొత్తం = 0;  /* count number of elements in all lists */
  ఒకవేళ (!keepinvariant(g)) తిరిగి మొత్తం;
  మొత్తం += checkgraylist(g, g->బూడిద);
  మొత్తం += checkgraylist(g, g->grayagain);
  మొత్తం += checkgraylist(g, g->weak);
  మొత్తం += checkgraylist(g, g->allweak);
  మొత్తం += checkgraylist(g, g->ephemeron);
  తిరిగి మొత్తం;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
స్థిర శూన్య incifingray (global_State *g, GCObject *o, l_mem *లెక్క) {
  ఒకవేళ (!keepinvariant(g))
    తిరిగి;  /* gray lists not being kept in these phases */
  ఒకవేళ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    ప్రతిజ్ఞా(!isgray(o) || upisopen(gco2upv(o)));
    తిరిగి;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ఒకవేళ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*లెక్క)++;
    ప్రతిజ్ఞా(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


స్థిర l_mem checklist (global_State *g, పూర్ణాంకం maybedead, పూర్ణాంకం tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem మొత్తం = 0;  /* number of object that should be in  gray lists */
  కొరకు (o = newl; o != survival; o = o->తదుపరి) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &మొత్తం);
    ప్రతిజ్ఞా(!tof == !tofinalize(o));
  }
  కొరకు (o = survival; o != old; o = o->తదుపరి) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &మొత్తం);
    ప్రతిజ్ఞా(!tof == !tofinalize(o));
  }
  కొరకు (o = old; o != reallyold; o = o->తదుపరి) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &మొత్తం);
    ప్రతిజ్ఞా(!tof == !tofinalize(o));
  }
  కొరకు (o = reallyold; o != ఖాళీ; o = o->తదుపరి) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &మొత్తం);
    ప్రతిజ్ఞా(!tof == !tofinalize(o));
  }
  తిరిగి మొత్తం;
}


పూర్ణాంకం lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  పూర్ణాంకం maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ఒకవేళ (keepinvariant(g)) {
    ప్రతిజ్ఞా(!iswhite(mainthread(g)));
    ప్రతిజ్ఞా(!iswhite(gcvalue(&g->l_registry)));
  }
  ప్రతిజ్ఞా(!isdead(g, gcvalue(&g->l_registry)));
  ప్రతిజ్ఞా(g->sweepgc == ఖాళీ || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  కొరకు (o = g->fixedgc; o != ఖాళీ; o = o->తదుపరి) {
    ప్రతిజ్ఞా(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  కొరకు (o = g->tobefnz; o != ఖాళీ; o = o->తదుపరి) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    ప్రతిజ్ఞా(tofinalize(o));
    ప్రతిజ్ఞా(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ఒకవేళ (keepinvariant(g))
    ప్రతిజ్ఞా(totalin == totalshould);
  తిరిగి 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


స్థిర అక్షరం *buildop (Proto *p, పూర్ణాంకం pc, అక్షరం *buff) {
  అక్షరం *obuff = buff;
  Instruction i = p->సంకేతం_2[pc];
  Op_సంకేతం o = GET_OPCODE(i);
  స్థిరరాశి అక్షరం *name = opnames[o];
  పూర్ణాంకం పంక్తి = luaG_getfuncline(p, pc);
  పూర్ణాంకం lineinfo = (p->lineinfo != ఖాళీ) ? p->lineinfo[pc] : 0;
  ఒకవేళ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  లేకపోతే
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", పంక్తి, pc);
  ఎంపిక (get_Op_బహుళకం(o)) {
    స్థితి iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      ఆపు;
    స్థితి ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      ఆపు;
    స్థితి iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      ఆపు;
    స్థితి iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      ఆపు;
    స్థితి iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      ఆపు;
    స్థితి isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      ఆపు;
  }
  తిరిగి obuff;
}


#ఒకవేళ 0
శూన్య luaI_printcode (Proto *pt, పూర్ణాంకం కొలత) {
  పూర్ణాంకం pc;
  కొరకు (pc=0; pc<కొలత; pc++) {
    అక్షరం buff[100];
    రాయి("%s\n", buildop(pt, pc, buff));
  }
  రాయి("-------\n");
}


శూన్య luaI_printinst (Proto *pt, పూర్ణాంకం pc) {
  అక్షరం buff[100];
  రాయి("%s\n", buildop(pt, pc, buff));
}
#అయితే_ముగింపు


స్థిర పూర్ణాంకం listcode (lua_State *L) {
  పూర్ణాంకం pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  కొరకు (pc=0; pc<p->sizecode; pc++) {
    అక్షరం buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం printcode (lua_State *L) {
  పూర్ణాంకం pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  రాయి("maxstack: %d\n", p->maxstacksize);
  రాయి("numparams: %d\n", p->numparams);
  కొరకు (pc=0; pc<p->sizecode; pc++) {
    అక్షరం buff[100];
    రాయి("%s\n", buildop(p, pc, buff));
  }
  తిరిగి 0;
}


స్థిర పూర్ణాంకం listk (lua_State *L) {
  Proto *p;
  పూర్ణాంకం i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  కొరకు (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం listabslineinfo (lua_State *L) {
  Proto *p;
  పూర్ణాంకం i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ఖాళీ, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  కొరకు (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].పంక్తి);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం listlocals (lua_State *L) {
  Proto *p;
  పూర్ణాంకం pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  పూర్ణాంకం i = 0;
  స్థిరరాశి అక్షరం *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  అయితే ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ఖాళీ)
    lua_pushstring(L, name);
  తిరిగి i-1;
}

/* }====================================================== */



శూన్య lua_printstack (lua_State *L) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = lua_gettop(L);
  రాయి("stack: >>\n");
  కొరకు (i = 1; i <= n; i++) {
    రాయి("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    రాయి("\n");
  }
  రాయి("<<\n");
}


పూర్ణాంకం lua_printallstack (lua_State *L) {
  StkId p;
  పూర్ణాంకం i = 1;
  CallInfo *ci = &L->పునాది_ci;
  రాయి("stack: >>\n");
  కొరకు (p = L->దొంతర.p; p < L->శిఖరం.p; p++) {
    ఒకవేళ (ci != ఖాళీ && p == ci->func.p) {
      రాయి("  ---\n");
      ఒకవేళ (ci == L->ci)
        ci = ఖాళీ;  /* printed last frame */
      లేకపోతే
        ci = ci->తదుపరి;
    }
    రాయి("%3d: ", i++);
    lua_printvalue(s2v(p));
    రాయి("\n");
  }
  రాయి("<<\n");
  తిరిగి 0;
}


స్థిర పూర్ణాంకం get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", స్థానభ్రంశం_s_Bx);
  setnameval(L, "NUM_OPCODES", అంకె_OPCODES);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", పరిమాణం(lua_State));
  setnameval(L, "global state", పరిమాణం(global_State));
  setnameval(L, "TValue", పరిమాణం(TValue));
  setnameval(L, "Node", పరిమాణం(కణుపు_2));
  setnameval(L, "stack Value", పరిమాణం(దొంతర_విలువ));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం mem_query (lua_State *L) {
  ఒకవేళ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.మొత్తం));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    తిరిగి 3;
  }
  లేకపోతే ఒకవేళ (lua_isnumber(L, 1)) {
    గుర్తులేని దీర్ఘ హద్దు = cast(గుర్తులేని దీర్ఘ, luaL_checkinteger(L, 1));
    ఒకవేళ (హద్దు == 0) హద్దు = ULONG_MAX;
    l_memcontrol.memlimit = హద్దు;
    తిరిగి 0;
  }
  లేకపోతే {
    స్థిరరాశి అక్షరం *t = luaL_checkstring(L, 1);
    పూర్ణాంకం i;
    కొరకు (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ఒకవేళ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        తిరిగి 1;
      }
    }
    తిరిగి luaL_error(L, "unknown type '%s'", t);
  }
}


స్థిర పూర్ణాంకం alloc_లెక్క (lua_State *L) {
  ఒకవేళ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(గుర్తులేని దీర్ఘ, ~0L);
  లేకపోతే
    l_memcontrol.countlimit = cast(గుర్తులేని దీర్ఘ, luaL_checkinteger(L, 1));
  తిరిగి 0;
}


స్థిర పూర్ణాంకం alloc_failnext (lua_State *L) {
  వాడని(L);
  l_memcontrol.failnext = 1;
  తిరిగి 0;
}


స్థిర పూర్ణాంకం settrick (lua_State *L) {
  ఒకవేళ (ttisnil(obj_at(L, 1)))
    l_Trick = ఖాళీ;
  లేకపోతే
    l_Trick = gcvalue(obj_at(L, 1));
  తిరిగి 0;
}


స్థిర పూర్ణాంకం gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ఒకవేళ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  లేకపోతే {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ఒకవేళ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  లేకపోతే {
    స్థిర స్థిరరాశి అక్షరం *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ఒకవేళ (!iscollectable(o))
    రాయి("no collectable\n");
  లేకపోతే {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    రాయి("\n");
  }
  తిరిగి 0;
}


స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

స్థిర పూర్ణాంకం gc_state (lua_State *L) {
  స్థిర స్థిరరాశి పూర్ణాంకం స్థితులు[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  పూర్ణాంకం option = స్థితులు[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ఒకవేళ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    తిరిగి 1;
  }
  లేకపోతే {
    ఒకవేళ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_తాళం(L);
    ఒకవేళ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    తిరిగి 0;
  }
}


స్థిర పూర్ణాంకం tracinggc = 0;
శూన్య luai_tracegctest (lua_State *L, పూర్ణాంకం మొదటి) {
  ఒకవేళ (!tracinggc) తిరిగి;
  లేకపోతే {
    global_State *g = G(L);
    lua_unlock(L);
    g->gcstp = GCSTPGC;
    lua_checkstack(L, 10);
    lua_getfield(L, LUA_REGISTRYINDEX, "tracegc");
    lua_pushboolean(L, మొదటి);
    lua_call(L, 1, 0);
    g->gcstp = 0;
    lua_తాళం(L);
  }
}


స్థిర పూర్ణాంకం tracegc (lua_State *L) {
  ఒకవేళ (lua_isnil(L, 1))
    tracinggc = 0;
  లేకపోతే {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  తిరిగి 0;
}


స్థిర పూర్ణాంకం hash_query (lua_State *L) {
  ఒకవేళ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ఒకవేళ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  లేకపోతే {
    TValue *o = obj_at(L, 1);
    పట్టిక *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->కణుపు));
  }
  తిరిగి 1;
}


స్థిర పూర్ణాంకం stacklevel (lua_State *L) {
  పూర్ణాంకం a = 0;
  lua_pushinteger(L, cast_Integer(L->శిఖరం.p - L->దొంతర.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  తిరిగి 5;
}


స్థిర పూర్ణాంకం resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  అయితే (ci->తదుపరి != ఖాళీ) {
    CallInfo *tofree = ci->తదుపరి;
    ci->తదుపరి = ci->తదుపరి->తదుపరి;
    luaM_free(L, tofree);
    L->nci--;
  }
  తిరిగి 0;
}


స్థిర పూర్ణాంకం reallocstack (lua_State *L) {
  పూర్ణాంకం n = cast_int(luaL_checkinteger(L, 1));
  lua_తాళం(L);
  luaD_reallocstack(L, cast_int(L->శిఖరం.p - L->దొంతర.p) + n, 1);
  lua_unlock(L);
  తిరిగి 0;
}


స్థిర పూర్ణాంకం పట్టిక_query (lua_State *L) {
  స్థిరరాశి పట్టిక *t;
  పూర్ణాంకం i = cast_int(luaL_optinteger(L, 2, -1));
  గుర్తులేని పూర్ణాంకం asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ఒకవేళ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    తిరిగి 3;
  }
  లేకపోతే ఒకవేళ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ఒకవేళ (!tagisempty(*get_Arr_ట్యాగ్(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->శిఖరం.p));
    లేకపోతే
      setnilvalue2s(L->శిఖరం.p);
    api_incr_శిఖరం(L);
    lua_pushnil(L);
  }
  లేకపోతే ఒకవేళ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ఒకవేళ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    లేకపోతే
      lua_pushliteral(L, "<undef>");
    ఒకవేళ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    లేకపోతే
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->కణుపు[i]));
  }
  తిరిగి 3;
}


స్థిర పూర్ణాంకం gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  తిరిగి 6;
}


స్థిర పూర్ణాంకం test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  పూర్ణాంకం s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ఒకవేళ (s == -1) {
    lua_pushinteger(L ,tb->కొలత);
    lua_pushinteger(L ,tb->nuse);
    తిరిగి 2;
  }
  లేకపోతే ఒకవేళ (s < tb->కొలత) {
    TString *ts;
    పూర్ణాంకం n = 0;
    కొరకు (ts = tb->hash[s]; ts != ఖాళీ; ts = ts->u.hnext) {
      setsvalue2s(L, L->శిఖరం.p, ts);
      api_incr_శిఖరం(L);
      n++;
    }
    తిరిగి n;
  }
  లేకపోతే తిరిగి 0;
}


స్థిర పూర్ణాంకం getreftable (lua_State *L) {
  ఒకవేళ (lua_istable(L, 2))  /* is there a table as second argument? */
    తిరిగి 2;  /* use it as the table */
  లేకపోతే
    తిరిగి LUA_REGISTRYINDEX;  /* default is to use the register */
}


స్థిర పూర్ణాంకం tref (lua_State *L) {
  పూర్ణాంకం t = getreftable(L);
  పూర్ణాంకం level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  తిరిగి 1;
}


స్థిర పూర్ణాంకం getref (lua_State *L) {
  పూర్ణాంకం t = getreftable(L);
  పూర్ణాంకం level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  తిరిగి 1;
}

స్థిర పూర్ణాంకం unref (lua_State *L) {
  పూర్ణాంకం t = getreftable(L);
  పూర్ణాంకం level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  తిరిగి 0;
}


స్థిర పూర్ణాంకం upvalue (lua_State *L) {
  పూర్ణాంకం n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ఒకవేళ (lua_isnone(L, 3)) {
    స్థిరరాశి అక్షరం *name = lua_getupvalue(L, 1, n);
    ఒకవేళ (name == ఖాళీ) తిరిగి 0;
    lua_pushstring(L, name);
    తిరిగి 2;
  }
  లేకపోతే {
    స్థిరరాశి అక్షరం *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    తిరిగి 1;
  }
}


స్థిర పూర్ణాంకం newuserdata (lua_State *L) {
  size_t కొలత = cast_sizet(luaL_optinteger(L, 1, 0));
  పూర్ణాంకం nuv = cast_int(luaL_optinteger(L, 2, 0));
  అక్షరం *p = cast_charp(lua_newuserdatauv(L, కొలత, nuv));
  అయితే (కొలత--) *p++ = '\0';
  తిరిగి 1;
}


స్థిర పూర్ణాంకం pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &l);
  పూర్ణాంకం స్థితి_2 = luaL_loadbufferx(L1, s, l, s, "t");
  ఒకవేళ (స్థితి_2 == LUA_OK)
    స్థితి_2 = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, స్థితి_2);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం s2d (lua_State *L) {
  lua_pushnumber(L, cast_అంకె(*cast(స్థిరరాశి ద్విగుణ *, luaL_checkstring(L, 1))));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం d2s (lua_State *L) {
  ద్విగుణ d = cast(ద్విగుణ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), పరిమాణం(d));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  తిరిగి 1;
}


స్థిర పూర్ణాంకం newstate (lua_State *L) {
  శూన్య *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ఒకవేళ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  లేకపోతే
    lua_pushnil(L);
  తిరిగి 1;
}


స్థిర lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ఖాళీ, 1, "state expected");
  తిరిగి L1;
}


స్థిర పూర్ణాంకం loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  పూర్ణాంకం ఎక్కించు = cast_int(luaL_checkinteger(L, 2));
  పూర్ణాంకం preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, ఎక్కించు, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ఖాళీ, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  తిరిగి 0;
}

స్థిర పూర్ణాంకం closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  తిరిగి 0;
}

స్థిర పూర్ణాంకం doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  స్థిరరాశి అక్షరం *సంకేతం_2 = luaL_checklstring(L, 2, &lcode);
  పూర్ణాంకం స్థితి_2;
  lua_settop(L1, 0);
  స్థితి_2 = luaL_loadbufferx(L1, సంకేతం_2, lcode, సంకేతం_2, "t");
  ఒకవేళ (స్థితి_2 == LUA_OK)
    స్థితి_2 = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ఒకవేళ (స్థితి_2 != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, స్థితి_2);
    తిరిగి 3;
  }
  లేకపోతే {
    పూర్ణాంకం i = 0;
    అయితే (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_తీయి(L1, i-1);
    తిరిగి i-1;
  }
}


స్థిర పూర్ణాంకం log2_aux (lua_State *L) {
  గుర్తులేని పూర్ణాంకం x = (గుర్తులేని పూర్ణాంకం)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  తిరిగి 1;
}


నిర్మాణం Aux { jmp_buf jb; స్థిరరాశి అక్షరం *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
స్థిర పూర్ణాంకం panicback (lua_State *L) {
  నిర్మాణం Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (నిర్మాణం Aux *)lua_touserdata(L, -1);
  lua_తీయి(L, 1);  /* remove 'Aux' struct */
  నిరంతరం_C(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  తిరిగి 1;  /* to avoid warnings */
}

స్థిర పూర్ణాంకం checkpanic (lua_State *L) {
  నిర్మాణం Aux b;
  శూన్య *ud;
  lua_State *L1;
  స్థిరరాశి అక్షరం *సంకేతం_2 = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ఒకవేళ (L1 == ఖాళీ) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    తిరిగి 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ఒకవేళ (setjmp(b.jb) == 0) {  /* set jump buffer */
    నిరంతరం_C(L, L1, సంకేతం_2);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  లేకపోతే {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  తిరిగి 1;
}


స్థిర పూర్ణాంకం externKstr (lua_State *L) {
  size_t len;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ఖాళీ, ఖాళీ);
  తిరిగి 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
స్థిర పూర్ణాంకం externstr (lua_State *L) {
  size_t len;
  స్థిరరాశి అక్షరం *s = luaL_checklstring(L, 1, &len);
  శూన్య *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  అక్షరం *buff = cast_charp((*allocf)(ud, ఖాళీ, 0, len + 1));
  ఒకవేళ (buff == ఖాళీ) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  జ్ఞాపకం_నకలు(buff, s, (len + 1) * పరిమాణం(అక్షరం));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  తిరిగి 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


స్థిర శూన్య sethookaux (lua_State *L, పూర్ణాంకం ముసుగు_2, పూర్ణాంకం లెక్క, స్థిరరాశి అక్షరం *సంకేతం_2);

స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి delimits = " \t\n,;";

స్థిర శూన్య skip (స్థిరరాశి అక్షరం **pc) {
  కొరకు (;;) {
    ఒకవేళ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    లేకపోతే ఒకవేళ (**pc == '#') {  /* comment? */
      అయితే (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    లేకపోతే ఆపు;
  }
}

స్థిర పూర్ణాంకం getnum_aux (lua_State *L, lua_State *L1, స్థిరరాశి అక్షరం **pc) {
  పూర్ణాంకం res = 0;
  పూర్ణాంకం sig = 1;
  skip(pc);
  ఒకవేళ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_తీయి(L1, 1);
    (*pc)++;
    తిరిగి res;
  }
  లేకపోతే ఒకవేళ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    తిరిగి res;
  }
  లేకపోతే ఒకవేళ (**pc == '!') {
    (*pc)++;
    ఒకవేళ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    లేకపోతే ఒకవేళ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    లేకపోతే lua_assert(0);
    (*pc)++;
    తిరిగి res;
  }
  లేకపోతే ఒకవేళ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ఒకవేళ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  అయితే (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  తిరిగి sig*res;
}

స్థిర స్థిరరాశి అక్షరం *getstring_aux (lua_State *L, అక్షరం *buff, స్థిరరాశి అక్షరం **pc) {
  పూర్ణాంకం i = 0;
  skip(pc);
  ఒకవేళ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    పూర్ణాంకం quote = *(*pc)++;
    అయితే (**pc != quote) {
      ఒకవేళ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  లేకపోతే {
    అయితే (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  తిరిగి buff;
}


స్థిర పూర్ణాంకం getindex_aux (lua_State *L, lua_State *L1, స్థిరరాశి అక్షరం **pc) {
  skip(pc);
  ఎంపిక (*(*pc)++) {
    స్థితి 'R': తిరిగి LUA_REGISTRYINDEX;
    స్థితి 'U': తిరిగి lua_upvalueindex(getnum_aux(L, L1, pc));
    అప్రమేయ: {
      పూర్ణాంకం n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ఒకవేళ (n == 0) తిరిగి 0;
      లేకపోతే తిరిగి lua_absindex(L1, n);
    }
  }
}


స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
స్థిర శూన్య regcodes (lua_State *L) {
  గుర్తులేని పూర్ణాంకం i;
  కొరకు (i = 0; i < పరిమాణం(statcodes) / పరిమాణం(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#నిర్వచించు EQ(s1)	(strcmp(s1, inst) == 0)

#నిర్వచించు getnum		(getnum_aux(L, L1, &pc))
#నిర్వచించు getstring	(getstring_aux(L, buff, &pc))
#నిర్వచించు getindex	(getindex_aux(L, L1, &pc))


స్థిర పూర్ణాంకం testC (lua_State *L);
స్థిర పూర్ణాంకం Cfunck (lua_State *L, పూర్ణాంకం స్థితి_2, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
స్థిర స్థిరరాశి అక్షరం ops[] = "+-*%^/\\&|~<>_!";

స్థిర పూర్ణాంకం నిరంతరం_C (lua_State *L, lua_State *L1, స్థిరరాశి అక్షరం *pc) {
  అక్షరం buff[300];
  పూర్ణాంకం స్థితి_2 = 0;
  ఒకవేళ (pc == ఖాళీ) తిరిగి luaL_error(L, "attempt to runC null script");
  కొరకు (;;) {
    స్థిరరాశి అక్షరం *inst = getstring;
    ఒకవేళ EQ("") తిరిగి 0;
    లేకపోతే ఒకవేళ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    లేకపోతే ఒకవేళ EQ("append") {
      పూర్ణాంకం t = getindex;
      పూర్ణాంకం i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    లేకపోతే ఒకవేళ EQ("arith") {
      పూర్ణాంకం op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    లేకపోతే ఒకవేళ EQ("call") {
      పూర్ణాంకం narg = getnum;
      పూర్ణాంకం nres = getnum;
      lua_call(L1, narg, nres);
    }
    లేకపోతే ఒకవేళ EQ("callk") {
      పూర్ణాంకం narg = getnum;
      పూర్ణాంకం nres = getnum;
      పూర్ణాంకం i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    లేకపోతే ఒకవేళ EQ("checkstack") {
      పూర్ణాంకం sz = getnum;
      స్థిరరాశి అక్షరం *సందేశం = getstring;
      ఒకవేళ (*సందేశం == '\0')
        సందేశం = ఖాళీ;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, సందేశం);
    }
    లేకపోతే ఒకవేళ EQ("rawcheckstack") {
      పూర్ణాంకం sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    లేకపోతే ఒకవేళ EQ("compare") {
      స్థిరరాశి అక్షరం *opt = getstring;  /* EQ, LT, or LE */
      పూర్ణాంకం op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      పూర్ణాంకం a = getindex;
      పూర్ణాంకం b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    లేకపోతే ఒకవేళ EQ("concat") {
      lua_concat(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("copy") {
      పూర్ణాంకం f = getindex;
      lua_copy(L1, f, getindex);
    }
    లేకపోతే ఒకవేళ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    లేకపోతే ఒకవేళ EQ("getfield") {
      పూర్ణాంకం t = getindex;
      పూర్ణాంకం tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    లేకపోతే ఒకవేళ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    లేకపోతే ఒకవేళ EQ("getmetatable") {
      ఒకవేళ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    లేకపోతే ఒకవేళ EQ("gettable") {
      పూర్ణాంకం tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    లేకపోతే ఒకవేళ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    లేకపోతే ఒకవేళ EQ("gsub") {
      పూర్ణాంకం a = getnum; పూర్ణాంకం b = getnum; పూర్ణాంకం c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    లేకపోతే ఒకవేళ EQ("insert") {
      lua_చొప్పించు(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("len") {
      lua_len(L1, getindex);
    }
    లేకపోతే ఒకవేళ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    లేకపోతే ఒకవేళ EQ("loadstring") {
      size_t slen;
      స్థిరరాశి అక్షరం *s = luaL_checklstring(L1, getnum, &slen);
      స్థిరరాశి అక్షరం *name = getstring;
      స్థిరరాశి అక్షరం *బహుళకం = getstring;
      luaL_loadbufferx(L1, s, slen, name, బహుళకం);
    }
    లేకపోతే ఒకవేళ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    లేకపోతే ఒకవేళ EQ("newtable") {
      lua_newtable(L1);
    }
    లేకపోతే ఒకవేళ EQ("newthread") {
      lua_newthread(L1);
    }
    లేకపోతే ఒకవేళ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    లేకపోతే ఒకవేళ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    లేకపోతే ఒకవేళ EQ("next") {
      lua_తదుపరి(L1, -2);
    }
    లేకపోతే ఒకవేళ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    లేకపోతే ఒకవేళ EQ("pcall") {
      పూర్ణాంకం narg = getnum;
      పూర్ణాంకం nres = getnum;
      స్థితి_2 = lua_pcall(L1, narg, nres, getnum);
    }
    లేకపోతే ఒకవేళ EQ("pcallk") {
      పూర్ణాంకం narg = getnum;
      పూర్ణాంకం nres = getnum;
      పూర్ణాంకం i = getindex;
      స్థితి_2 = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    లేకపోతే ఒకవేళ EQ("pop") {
      lua_తీయి(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("printstack") {
      పూర్ణాంకం n = getnum;
      ఒకవేళ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        రాయి("\n");
      }
      లేకపోతే lua_printstack(L1);
    }
    లేకపోతే ఒకవేళ EQ("print") {
      స్థిరరాశి అక్షరం *సందేశం = getstring;
      రాయి("%s\n", సందేశం);
    }
    లేకపోతే ఒకవేళ EQ("warningC") {
      స్థిరరాశి అక్షరం *సందేశం = getstring;
      lua_warning(L1, సందేశం, 1);
    }
    లేకపోతే ఒకవేళ EQ("warning") {
      స్థిరరాశి అక్షరం *సందేశం = getstring;
      lua_warning(L1, సందేశం, 0);
    }
    లేకపోతే ఒకవేళ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    లేకపోతే ఒకవేళ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("pushnil") {
      lua_pushnil(L1);
    }
    లేకపోతే ఒకవేళ EQ("pushnum") {
      lua_pushnumber(L1, (lua_సంఖ్య)getnum);
    }
    లేకపోతే ఒకవేళ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[స్థితి_2]);
    }
    లేకపోతే ఒకవేళ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    లేకపోతే ఒకవేళ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    లేకపోతే ఒకవేళ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    లేకపోతే ఒకవేళ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (పూర్ణాంకం)lua_tointeger(L, -1));
    }
    లేకపోతే ఒకవేళ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    లేకపోతే ఒకవేళ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    లేకపోతే ఒకవేళ EQ("rawget") {
      పూర్ణాంకం t = getindex;
      lua_rawget(L1, t);
    }
    లేకపోతే ఒకవేళ EQ("rawgeti") {
      పూర్ణాంకం t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    లేకపోతే ఒకవేళ EQ("rawgetp") {
      పూర్ణాంకం t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    లేకపోతే ఒకవేళ EQ("rawset") {
      పూర్ణాంకం t = getindex;
      lua_rawset(L1, t);
    }
    లేకపోతే ఒకవేళ EQ("rawseti") {
      పూర్ణాంకం t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    లేకపోతే ఒకవేళ EQ("rawsetp") {
      పూర్ణాంకం t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    లేకపోతే ఒకవేళ EQ("remove") {
      lua_remove(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("replace") {
      lua_replace(L1, getindex);
    }
    లేకపోతే ఒకవేళ EQ("resume") {
      పూర్ణాంకం i = getindex;
      పూర్ణాంకం nres;
      స్థితి_2 = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    లేకపోతే ఒకవేళ EQ("traceback") {
      స్థిరరాశి అక్షరం *సందేశం = getstring;
      పూర్ణాంకం level = getnum;
      luaL_traceback(L1, L1, సందేశం, level);
    }
    లేకపోతే ఒకవేళ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_స్థితి(L1)]);
    }
    లేకపోతే ఒకవేళ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    లేకపోతే ఒకవేళ EQ("return") {
      పూర్ణాంకం n = getnum;
      ఒకవేళ (L1 != L) {
        పూర్ణాంకం i;
        కొరకు (i = 0; i < n; i++) {
          పూర్ణాంకం idx = -(n - i);
          ఎంపిక (lua_type(L1, idx)) {
            స్థితి LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              ఆపు;
            అప్రమేయ:
              lua_pushstring(L, lua_tostring(L1, idx));
              ఆపు;
          }
        }
      }
      తిరిగి n;
    }
    లేకపోతే ఒకవేళ EQ("rotate") {
      పూర్ణాంకం i = getindex;
      lua_rotate(L1, i, getnum);
    }
    లేకపోతే ఒకవేళ EQ("setfield") {
      పూర్ణాంకం t = getindex;
      స్థిరరాశి అక్షరం *s = getstring;
      lua_setfield(L1, t, s);
    }
    లేకపోతే ఒకవేళ EQ("seti") {
      పూర్ణాంకం t = getindex;
      lua_seti(L1, t, getnum);
    }
    లేకపోతే ఒకవేళ EQ("setglobal") {
      స్థిరరాశి అక్షరం *s = getstring;
      lua_setglobal(L1, s);
    }
    లేకపోతే ఒకవేళ EQ("sethook") {
      పూర్ణాంకం ముసుగు_2 = getnum;
      పూర్ణాంకం లెక్క = getnum;
      స్థిరరాశి అక్షరం *s = getstring;
      sethookaux(L1, ముసుగు_2, లెక్క, s);
    }
    లేకపోతే ఒకవేళ EQ("setmetatable") {
      పూర్ణాంకం idx = getindex;
      lua_setmetatable(L1, idx);
    }
    లేకపోతే ఒకవేళ EQ("settable") {
      lua_settable(L1, getindex);
    }
    లేకపోతే ఒకవేళ EQ("settop") {
      lua_settop(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("testudata") {
      పూర్ణాంకం i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ఖాళీ);
    }
    లేకపోతే ఒకవేళ EQ("error") {
      lua_error(L1);
    }
    లేకపోతే ఒకవేళ EQ("abort") {
      abort();
    }
    లేకపోతే ఒకవేళ EQ("throw") {
#ఒకవేళ defined(__cplusplus)
స్థిర నిర్మాణం X { పూర్ణాంకం x; } x;
      throw x;
#లేకపోతే
      luaL_error(L1, "C++");
#అయితే_ముగింపు
      ఆపు;
    }
    లేకపోతే ఒకవేళ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    లేకపోతే ఒకవేళ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    లేకపోతే ఒకవేళ EQ("tostring") {
      స్థిరరాశి అక్షరం *s = lua_tostring(L1, getindex);
      స్థిరరాశి అక్షరం *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ఖాళీ && s1 == ఖాళీ) || strcmp(s, s1) == 0);
    }
    లేకపోతే ఒకవేళ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ఖాళీ);
    }
    లేకపోతే ఒకవేళ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    లేకపోతే ఒకవేళ EQ("xmove") {
      పూర్ణాంకం f = getindex;
      పూర్ణాంకం t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      పూర్ణాంకం n = getnum;
      ఒకవేళ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    లేకపోతే ఒకవేళ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    లేకపోతే ఒకవేళ EQ("yield") {
      తిరిగి lua_yield(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("yieldk") {
      పూర్ణాంకం nres = getnum;
      పూర్ణాంకం i = getindex;
      తిరిగి lua_yieldk(L1, nres, i, Cfunck);
    }
    లేకపోతే ఒకవేళ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    లేకపోతే ఒకవేళ EQ("argerror") {
      పూర్ణాంకం arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    లేకపోతే luaL_error(L, "unknown instruction %s", buff);
  }
  తిరిగి 0;
}


స్థిర పూర్ణాంకం testC (lua_State *L) {
  lua_State *L1;
  స్థిరరాశి అక్షరం *pc;
  ఒకవేళ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  లేకపోతే ఒకవేళ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  లేకపోతే {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  తిరిగి నిరంతరం_C(L, L1, pc);
}


స్థిర పూర్ణాంకం Cfunc (lua_State *L) {
  తిరిగి నిరంతరం_C(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


స్థిర పూర్ణాంకం Cfunck (lua_State *L, పూర్ణాంకం స్థితి_2, lua_KContext ctx) {
  lua_pushstring(L, statcodes[స్థితి_2]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  తిరిగి నిరంతరం_C(L, L, lua_tostring(L, cast_int(ctx)));
}


స్థిర పూర్ణాంకం makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  తిరిగి 1;
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
స్థిర శూన్య Chook (lua_State *L, lua_Debug *ar) {
  స్థిరరాశి అక్షరం *scpt;
  స్థిరరాశి అక్షరం *స్థిరరాశి events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_తీయి(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  నిరంతరం_C(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
స్థిర శూన్య sethookaux (lua_State *L, పూర్ణాంకం ముసుగు_2, పూర్ణాంకం లెక్క, స్థిరరాశి అక్షరం *scpt) {
  ఒకవేళ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ఖాళీ, 0, 0);  /* turn off hooks */
    తిరిగి;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ఒకవేళ (!lua_istable(L, -1)) {  /* no hook table? */
    lua_తీయి(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, ముసుగు_2, లెక్క);
}


స్థిర పూర్ణాంకం sethook (lua_State *L) {
  ఒకవేళ (lua_isnoneornil(L, 1))
    lua_sethook(L, ఖాళీ, 0, 0);  /* turn off hooks */
  లేకపోతే {
    స్థిరరాశి అక్షరం *scpt = luaL_checkstring(L, 1);
    స్థిరరాశి అక్షరం *smask = luaL_checkstring(L, 2);
    పూర్ణాంకం లెక్క = cast_int(luaL_optinteger(L, 3, 0));
    పూర్ణాంకం ముసుగు_2 = 0;
    ఒకవేళ (strchr(smask, 'c')) ముసుగు_2 |= LUA_MASKCALL;
    ఒకవేళ (strchr(smask, 'r')) ముసుగు_2 |= LUA_MASKRET;
    ఒకవేళ (strchr(smask, 'l')) ముసుగు_2 |= LUA_MASKLINE;
    ఒకవేళ (లెక్క > 0) ముసుగు_2 |= LUA_MASKCOUNT;
    sethookaux(L, ముసుగు_2, లెక్క, scpt);
  }
  తిరిగి 0;
}


స్థిర పూర్ణాంకం coresume (lua_State *L) {
  పూర్ణాంకం స్థితి_2, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  స్థితి_2 = lua_resume(co, L, 0, &nres);
  ఒకవేళ (స్థితి_2 != LUA_OK && స్థితి_2 != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_చొప్పించు(L, -2);
    తిరిగి 2;  /* return false + error message */
  }
  లేకపోతే {
    lua_pushboolean(L, 1);
    తిరిగి 1;
  }
}

#ఒకవేళ !defined(LUA_USE_POSIX)

#నిర్వచించు nonblock	ఖాళీ

#లేకపోతే

#అన్తర్భావ <unistd.h>
#అన్తర్భావ <fcntl.h>

స్థిర పూర్ణాంకం nonblock (lua_State *L) {
  FILE *f = cast(lua_L_ప్రవాహం*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  పూర్ణాంకం fd = fileno(f);
  పూర్ణాంకం flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  తిరిగి 0;
}
#అయితే_ముగింపు

/* }====================================================== */



స్థిర స్థిరరాశి నిర్మాణం luaL_Reg tests_funcs[] = {
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
  {"querytab", పట్టిక_query},
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
  {"alloccount", alloc_లెక్క},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {ఖాళీ, ఖాళీ}
};


స్థిర శూన్య checkfinalmem (శూన్య) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.మొత్తం == 0);
}


పూర్ణాంకం luaB_opentests (lua_State *L) {
  శూన్య *ud;
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
  తిరిగి 1;
}

#అయితే_ముగింపు

