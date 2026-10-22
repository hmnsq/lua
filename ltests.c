/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <setjmp.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lcode.h"
#अन्तर्भाव "lctype.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lopnames.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#सचे defined(LUA_DEBUG)


शून्य *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


स्थिर पूर्णाङ्क runC (lua_State *L, lua_State *L1, ध्रुव वर्ण *pc);


स्थिर शून्य setnameval (lua_State *L, ध्रुव वर्ण *name, पूर्णाङ्क val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


स्थिर शून्य pushobject (lua_State *L, ध्रुव TValue *o) {
  setobj2s(L, L->शिखरम्.p, o);
  api_incr_शिखरम्(L);
}


स्थिर शून्य badexit (ध्रुव वर्ण *fmt, ध्रुव वर्ण *s1, ध्रुव वर्ण *s2) {
  fprintf(stderr, fmt, s1);
  सचे (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


स्थिर पूर्णाङ्क tpanic (lua_State *L) {
  ध्रुव वर्ण *सन्देशः = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  पटिनिवत्त (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   सन्देशः, रिक्त),
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
स्थिर शून्य warnf (शून्य *ud, ध्रुव वर्ण *सन्देशः, पूर्णाङ्क tocont) {
  lua_State *L = cast(lua_State *, ud);
  स्थिर वर्ण buff[200] = "";  /* should be enough for tests... */
  स्थिर पूर्णाङ्क onoff = 0;
  स्थिर पूर्णाङ्क mode = 0;  /* start in normal mode */
  स्थिर पूर्णाङ्क lasttocont = 0;
  सचे (!lasttocont && !tocont && *सन्देशः == '@') {  /* control message? */
    सचे (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", सन्देशः, buff);
    सचे (strcmp(सन्देशः, "@off") == 0)
      onoff = 0;
    अञ्ञथा सचे (strcmp(सन्देशः, "@on") == 0)
      onoff = 1;
    अञ्ञथा सचे (strcmp(सन्देशः, "@normal") == 0)
      mode = 0;
    अञ्ञथा सचे (strcmp(सन्देशः, "@allow") == 0)
      mode = 1;
    अञ्ञथा सचे (strcmp(सन्देशः, "@store") == 0)
      mode = 2;
    अञ्ञथा
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              सन्देशः, रिक्त);
    पटिनिवत्त;
  }
  lasttocont = tocont;
  सचे (strlen(सन्देशः) >= परिमाण(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", सन्देशः, buff);
  strcat(buff, सन्देशः);  /* add new message to current warning */
  सचे (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    सचे (!lua_toboolean(L, -1))
      lua_निष्कासय(L, 1);  /* ok, no previous unexpected warning */
    अञ्ञथा {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    विकल्प (mode) {
      पक्ष 0: {  /* normal */
        सचे (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, रिक्त);
      }  /* FALLTHROUGH */
      पक्ष 1: {  /* allow */
        सचे (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        भङ्ग;
      }
      पक्ष 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        भङ्ग;
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

प्रकारनिर्देश सङ्घ memHeader {
  LUAI_MAXALIGN;
  संरचना {
    size_t size;
    पूर्णाङ्क type;
  } d;
} memHeader;


#सचे !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#अञ्ञथा

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


स्थिर शून्य freeblock (Memcontrol *mc, memHeader *block) {
  सचे (block) {
    size_t size = block->d.size;
    पूर्णाङ्क i;
    अत्थाय (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, परिमाण(memHeader) + size + MARKSIZE);  /* erase block */
    मुक्त(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


शून्य *debug_realloc (शून्य *ud, शून्य *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  पूर्णाङ्क type;
  सचे (mc->memlimit == 0) {  /* first time? */
    वर्ण *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, रिक्त, 10) : ULONG_MAX;
  }
  सचे (block == रिक्त) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  अञ्ञथा {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  सचे (size == 0) {
    freeblock(mc, block);
    पटिनिवत्त रिक्त;
  }
  सचे (mc->failnext) {
    mc->failnext = 0;
    पटिनिवत्त रिक्त;  /* fake a single memory allocation error */
  }
  सचे (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    सचे (mc->countlimit == 0)
      पटिनिवत्त रिक्त;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  सचे (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    पटिनिवत्त रिक्त;  /* fake a memory allocation error */
  अञ्ञथा {
    memHeader *newblock;
    पूर्णाङ्क i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = परिमाण(memHeader) + size + MARKSIZE;
    सचे (realsize < size) पटिनिवत्त रिक्त;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    सचे (newblock == रिक्त)
      पटिनिवत्त रिक्त;  /* really out of memory? */
    सचे (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    अत्थाय (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    सचे (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    पटिनिवत्त newblock + 1;
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

#अन्तर्भाव <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
स्थिर पूर्णाङ्क testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  सचे (isdead(g,t)) पटिनिवत्त 0;
  सचे (issweepphase(g))
    पटिनिवत्त 1;  /* no invariants */
  अञ्ञथा सचे (g->gckind != KGC_GENMINOR)
    पटिनिवत्त !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  अञ्ञथा {  /* generational mode */
    सचे ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      पटिनिवत्त 0;
    सचे ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      पटिनिवत्त 0;
    पटिनिवत्त 1;
  }
}


स्थिर शून्य printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (शून्य *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  सचे (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


शून्य lua_printobj (lua_State *L, संरचना GCObject *o) {
  printobj(G(L), o);
}


शून्य lua_printvalue (TValue *v) {
  विकल्प (ttypetag(v)) {
    पक्ष LUA_VNUMINT: पक्ष LUA_VNUMFLT: {
      वर्ण buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      भङ्ग;
    }
    पक्ष LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); भङ्ग;
    पक्ष LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); भङ्ग;
    पक्ष LUA_VFALSE:
      printf("%s", "false"); भङ्ग;
    पक्ष LUA_VTRUE:
      printf("%s", "true"); भङ्ग;
    पक्ष LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); भङ्ग;
    पक्ष LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); भङ्ग;
    पक्ष LUA_VNIL:
      printf("nil"); भङ्ग;
    पक्ष LUA_VLCF:
      printf("light C function: %p", fvalue(v)); भङ्ग;
    पक्ष LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); भङ्ग;
    पक्ष LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); भङ्ग;
    पक्ष LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); भङ्ग;
    पक्ष LUA_VTABLE:
      printf("table: %p", hvalue(v)); भङ्ग;
    स्वभाव:
      lua_assert(0);
  }
}


स्थिर पूर्णाङ्क testobjref (global_State *g, GCObject *f, GCObject *t) {
  पूर्णाङ्क r1 = testobjref1(g, f, t);
  सचे (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  पटिनिवत्त r1;
}


स्थिर शून्य checkobjref (global_State *g, GCObject *f, GCObject *t) {
    प्रतिज्ञा(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ सचे (t) checkobjref(g,f,obj2gco(t)); }


स्थिर शून्य checkvalref (global_State *g, GCObject *f, ध्रुव TValue *t) {
  प्रतिज्ञा(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


स्थिर शून्य checktable (global_State *g, Table *h) {
  unsigned पूर्णाङ्क i;
  unsigned पूर्णाङ्क asize = h->asize;
  ग्रन्थिः_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  अत्थाय (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  अत्थाय (n = gnode(h, 0); n < limit; n++) {
    सचे (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      प्रतिज्ञा(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


स्थिर शून्य checkudata (global_State *g, Udata *u) {
  पूर्णाङ्क i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  अत्थाय (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


स्थिर शून्य checkproto (global_State *g, Proto *f) {
  पूर्णाङ्क i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  अत्थाय (i=0; i<f->sizek; i++) {
    सचे (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  अत्थाय (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  अत्थाय (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  अत्थाय (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


स्थिर शून्य checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  पूर्णाङ्क i;
  अत्थाय (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


स्थिर शून्य checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  पूर्णाङ्क i;
  checkobjrefN(g, clgc, cl->p);
  अत्थाय (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    सचे (uv) {
      checkobjrefN(g, clgc, uv);
      सचे (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


स्थिर पूर्णाङ्क lua_checkpc (CallInfo *ci) {
  सचे (!isLua(ci)) पटिनिवत्त 1;
  अञ्ञथा {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    पटिनिवत्त p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


स्थिर शून्य check_राशिः (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  प्रतिज्ञा(!isdead(g, L1));
  सचे (L1->राशिः.p == रिक्त) {  /* incomplete thread? */
    प्रतिज्ञा(L1->openupval == रिक्त && L1->ci == रिक्त);
    पटिनिवत्त;
  }
  अत्थाय (uv = L1->openupval; uv != रिक्त; uv = uv->u.open.next)
    प्रतिज्ञा(upisopen(uv));  /* must be open */
  प्रतिज्ञा(L1->शिखरम्.p <= L1->राशिः_last.p);
  प्रतिज्ञा(L1->tbclist.p <= L1->शिखरम्.p);
  अत्थाय (ci = L1->ci; ci != रिक्त; ci = ci->previous) {
    प्रतिज्ञा(ci->शिखरम्.p <= L1->राशिः_last.p);
    प्रतिज्ञा(lua_checkpc(ci));
  }
  अत्थाय (o = L1->राशिः.p; o < L1->राशिः_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


स्थिर शून्य checkrefs (global_State *g, GCObject *o) {
  विकल्प (o->tt) {
    पक्ष LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      भङ्ग;
    }
    पक्ष LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      भङ्ग;
    }
    पक्ष LUA_VTABLE: {
      checktable(g, gco2t(o));
      भङ्ग;
    }
    पक्ष LUA_VTHREAD: {
      check_राशिः(g, gco2th(o));
      भङ्ग;
    }
    पक्ष LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      भङ्ग;
    }
    पक्ष LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      भङ्ग;
    }
    पक्ष LUA_VPROTO: {
      checkproto(g, gco2p(o));
      भङ्ग;
    }
    पक्ष LUA_VSHRSTR:
    पक्ष LUA_VLNGSTR: {
      प्रतिज्ञा(!isgray(o));  /* strings are never gray */
      भङ्ग;
    }
    स्वभाव: प्रतिज्ञा(0);
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
स्थिर शून्य checkobject (global_State *g, GCObject *o, पूर्णाङ्क maybedead,
                         पूर्णाङ्क listage) {
  सचे (isdead(g, o))
    प्रतिज्ञा(maybedead);
  अञ्ञथा {
    प्रतिज्ञा(g->gcstate != GCSpause || iswhite(o));
    सचे (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      प्रतिज्ञा(getage(o) >= listage);
      सचे (isold(o)) {
        प्रतिज्ञा(!iswhite(o));
        प्रतिज्ञा(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      प्रतिज्ञा(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


स्थिर l_mem checkgraylist (global_State *g, GCObject *o) {
  पूर्णाङ्क total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  याव (o) {
    प्रतिज्ञा(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    प्रतिज्ञा(!testbit(o->marked, TESTBIT));
    सचे (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    विकल्प (o->tt) {
      पक्ष LUA_VTABLE: o = gco2t(o)->gclist; भङ्ग;
      पक्ष LUA_VLCL: o = gco2lcl(o)->gclist; भङ्ग;
      पक्ष LUA_VCCL: o = gco2ccl(o)->gclist; भङ्ग;
      पक्ष LUA_VTHREAD: o = gco2th(o)->gclist; भङ्ग;
      पक्ष LUA_VPROTO: o = gco2p(o)->gclist; भङ्ग;
      पक्ष LUA_VUSERDATA:
        प्रतिज्ञा(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        भङ्ग;
      स्वभाव: प्रतिज्ञा(0);  /* other objects cannot be in a gray list */
    }
  }
  पटिनिवत्त total;
}


/*
** Check objects in gray lists.
*/
स्थिर l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  सचे (!keepinvariant(g)) पटिनिवत्त total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  पटिनिवत्त total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
स्थिर शून्य incifingray (global_State *g, GCObject *o, l_mem *गणनम्) {
  सचे (!keepinvariant(g))
    पटिनिवत्त;  /* gray lists not being kept in these phases */
  सचे (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    प्रतिज्ञा(!isgray(o) || upisopen(gco2upv(o)));
    पटिनिवत्त;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  सचे (isgray(o) || getage(o) == G_TOUCHED2) {
    (*गणनम्)++;
    प्रतिज्ञा(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


स्थिर l_mem checklist (global_State *g, पूर्णाङ्क maybedead, पूर्णाङ्क tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  अत्थाय (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  अत्थाय (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  अत्थाय (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  अत्थाय (o = reallyold; o != रिक्त; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  पटिनिवत्त total;
}


पूर्णाङ्क lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  पूर्णाङ्क maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  सचे (keepinvariant(g)) {
    प्रतिज्ञा(!iswhite(mainthread(g)));
    प्रतिज्ञा(!iswhite(gcvalue(&g->l_registry)));
  }
  प्रतिज्ञा(!isdead(g, gcvalue(&g->l_registry)));
  प्रतिज्ञा(g->sweepgc == रिक्त || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  अत्थाय (o = g->fixedgc; o != रिक्त; o = o->next) {
    प्रतिज्ञा(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  अत्थाय (o = g->tobefnz; o != रिक्त; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    प्रतिज्ञा(tofinalize(o));
    प्रतिज्ञा(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  सचे (keepinvariant(g))
    प्रतिज्ञा(totalin == totalshould);
  पटिनिवत्त 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


स्थिर वर्ण *buildop (Proto *p, पूर्णाङ्क pc, वर्ण *buff) {
  वर्ण *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  ध्रुव वर्ण *name = opnames[o];
  पूर्णाङ्क पङ्क्तिः = luaG_getfuncline(p, pc);
  पूर्णाङ्क lineinfo = (p->lineinfo != रिक्त) ? p->lineinfo[pc] : 0;
  सचे (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  अञ्ञथा
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", पङ्क्तिः, pc);
  विकल्प (getOpMode(o)) {
    पक्ष iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      भङ्ग;
    पक्ष ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      भङ्ग;
    पक्ष iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      भङ्ग;
    पक्ष iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      भङ्ग;
    पक्ष iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      भङ्ग;
    पक्ष isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      भङ्ग;
  }
  पटिनिवत्त obuff;
}


#सचे 0
शून्य luaI_printcode (Proto *pt, पूर्णाङ्क size) {
  पूर्णाङ्क pc;
  अत्थाय (pc=0; pc<size; pc++) {
    वर्ण buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


शून्य luaI_printinst (Proto *pt, पूर्णाङ्क pc) {
  वर्ण buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


स्थिर पूर्णाङ्क listcode (lua_State *L) {
  पूर्णाङ्क pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  अत्थाय (pc=0; pc<p->sizecode; pc++) {
    वर्ण buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क printcode (lua_State *L) {
  पूर्णाङ्क pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  अत्थाय (pc=0; pc<p->sizecode; pc++) {
    वर्ण buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क listk (lua_State *L) {
  Proto *p;
  पूर्णाङ्क i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  अत्थाय (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क listabslineinfo (lua_State *L) {
  Proto *p;
  पूर्णाङ्क i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != रिक्त, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  अत्थाय (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].पङ्क्तिः);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क listlocals (lua_State *L) {
  Proto *p;
  पूर्णाङ्क pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  पूर्णाङ्क i = 0;
  ध्रुव वर्ण *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  याव ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != रिक्त)
    lua_pushstring(L, name);
  पटिनिवत्त i-1;
}

/* }====================================================== */



शून्य lua_printstack (lua_State *L) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = lua_gettop(L);
  printf("stack: >>\n");
  अत्थाय (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


पूर्णाङ्क lua_printallstack (lua_State *L) {
  StkId p;
  पूर्णाङ्क i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  अत्थाय (p = L->राशिः.p; p < L->शिखरम्.p; p++) {
    सचे (ci != रिक्त && p == ci->func.p) {
      printf("  ---\n");
      सचे (ci == L->ci)
        ci = रिक्त;  /* printed last frame */
      अञ्ञथा
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", अङ्कः_OPCODES);
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", परिमाण(lua_State));
  setnameval(L, "global state", परिमाण(global_State));
  setnameval(L, "TValue", परिमाण(TValue));
  setnameval(L, "Node", परिमाण(ग्रन्थिः_2));
  setnameval(L, "stack Value", परिमाण(राशिः_मानम्));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क mem_query (lua_State *L) {
  सचे (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    पटिनिवत्त 3;
  }
  अञ्ञथा सचे (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    सचे (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    पटिनिवत्त 0;
  }
  अञ्ञथा {
    ध्रुव वर्ण *t = luaL_checkstring(L, 1);
    पूर्णाङ्क i;
    अत्थाय (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      सचे (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        पटिनिवत्त 1;
      }
    }
    पटिनिवत्त luaL_error(L, "unknown type '%s'", t);
  }
}


स्थिर पूर्णाङ्क alloc_गणनम् (lua_State *L) {
  सचे (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  अञ्ञथा
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क settrick (lua_State *L) {
  सचे (ttisnil(obj_at(L, 1)))
    l_Trick = रिक्त;
  अञ्ञथा
    l_Trick = gcvalue(obj_at(L, 1));
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  सचे (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  अञ्ञथा {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  सचे (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  अञ्ञथा {
    स्थिर ध्रुव वर्ण *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  सचे (!iscollectable(o))
    printf("no collectable\n");
  अञ्ञथा {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  पटिनिवत्त 0;
}


स्थिर ध्रुव वर्ण *ध्रुव statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

स्थिर पूर्णाङ्क gc_state (lua_State *L) {
  स्थिर ध्रुव पूर्णाङ्क states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  पूर्णाङ्क option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  सचे (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    पटिनिवत्त 1;
  }
  अञ्ञथा {
    सचे (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    सचे (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    पटिनिवत्त 0;
  }
}


स्थिर पूर्णाङ्क tracinggc = 0;
शून्य luai_tracegctest (lua_State *L, पूर्णाङ्क first) {
  सचे (!tracinggc) पटिनिवत्त;
  अञ्ञथा {
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


स्थिर पूर्णाङ्क tracegc (lua_State *L) {
  सचे (lua_isnil(L, 1))
    tracinggc = 0;
  अञ्ञथा {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क hash_query (lua_State *L) {
  सचे (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    सचे (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  अञ्ञथा {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->ग्रन्थिः));
  }
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क stacklevel (lua_State *L) {
  पूर्णाङ्क a = 0;
  lua_pushinteger(L, cast_Integer(L->शिखरम्.p - L->राशिः.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  पटिनिवत्त 5;
}


स्थिर पूर्णाङ्क resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  याव (ci->next != रिक्त) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क reallocstack (lua_State *L) {
  पूर्णाङ्क n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->शिखरम्.p - L->राशिः.p) + n, 1);
  lua_unlock(L);
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क table_query (lua_State *L) {
  ध्रुव Table *t;
  पूर्णाङ्क i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned पूर्णाङ्क asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  सचे (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    पटिनिवत्त 3;
  }
  अञ्ञथा सचे (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    सचे (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->शिखरम्.p));
    अञ्ञथा
      setnilvalue2s(L->शिखरम्.p);
    api_incr_शिखरम्(L);
    lua_pushnil(L);
  }
  अञ्ञथा सचे (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    सचे (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    अञ्ञथा
      lua_pushliteral(L, "<undef>");
    सचे (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    अञ्ञथा
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->ग्रन्थिः[i]));
  }
  पटिनिवत्त 3;
}


स्थिर पूर्णाङ्क gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  पटिनिवत्त 6;
}


स्थिर पूर्णाङ्क test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  पूर्णाङ्क s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  सचे (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    पटिनिवत्त 2;
  }
  अञ्ञथा सचे (s < tb->size) {
    TString *ts;
    पूर्णाङ्क n = 0;
    अत्थाय (ts = tb->hash[s]; ts != रिक्त; ts = ts->u.hnext) {
      setsvalue2s(L, L->शिखरम्.p, ts);
      api_incr_शिखरम्(L);
      n++;
    }
    पटिनिवत्त n;
  }
  अञ्ञथा पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क getreftable (lua_State *L) {
  सचे (lua_istable(L, 2))  /* is there a table as second argument? */
    पटिनिवत्त 2;  /* use it as the table */
  अञ्ञथा
    पटिनिवत्त LUA_REGISTRYINDEX;  /* default is to use the register */
}


स्थिर पूर्णाङ्क tref (lua_State *L) {
  पूर्णाङ्क t = getreftable(L);
  पूर्णाङ्क level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क getref (lua_State *L) {
  पूर्णाङ्क t = getreftable(L);
  पूर्णाङ्क level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  पटिनिवत्त 1;
}

स्थिर पूर्णाङ्क unref (lua_State *L) {
  पूर्णाङ्क t = getreftable(L);
  पूर्णाङ्क level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क upvalue (lua_State *L) {
  पूर्णाङ्क n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  सचे (lua_isnone(L, 3)) {
    ध्रुव वर्ण *name = lua_getupvalue(L, 1, n);
    सचे (name == रिक्त) पटिनिवत्त 0;
    lua_pushstring(L, name);
    पटिनिवत्त 2;
  }
  अञ्ञथा {
    ध्रुव वर्ण *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    पटिनिवत्त 1;
  }
}


स्थिर पूर्णाङ्क newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  पूर्णाङ्क nuv = cast_int(luaL_optinteger(L, 2, 0));
  वर्ण *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  याव (size--) *p++ = '\0';
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  पूर्णाङ्क status = luaL_loadbufferx(L1, s, l, s, "t");
  सचे (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क s2d (lua_State *L) {
  lua_pushnumber(L, cast_अङ्कः(*cast(ध्रुव द्विगुण *, luaL_checkstring(L, 1))));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क d2s (lua_State *L) {
  द्विगुण d = cast(द्विगुण, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), परिमाण(d));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क newstate (lua_State *L) {
  शून्य *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  सचे (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  अञ्ञथा
    lua_pushnil(L);
  पटिनिवत्त 1;
}


स्थिर lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != रिक्त, 1, "state expected");
  पटिनिवत्त L1;
}


स्थिर पूर्णाङ्क loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  पूर्णाङ्क load = cast_int(luaL_checkinteger(L, 2));
  पूर्णाङ्क preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", रिक्त, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ध्रुव वर्ण *code = luaL_checklstring(L, 2, &lcode);
  पूर्णाङ्क status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  सचे (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  सचे (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    पटिनिवत्त 3;
  }
  अञ्ञथा {
    पूर्णाङ्क i = 0;
    याव (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_निष्कासय(L1, i-1);
    पटिनिवत्त i-1;
  }
}


स्थिर पूर्णाङ्क log2_aux (lua_State *L) {
  unsigned पूर्णाङ्क x = (unsigned पूर्णाङ्क)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  पटिनिवत्त 1;
}


संरचना Aux { jmp_buf jb; ध्रुव वर्ण *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
स्थिर पूर्णाङ्क panicback (lua_State *L) {
  संरचना Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (संरचना Aux *)lua_touserdata(L, -1);
  lua_निष्कासय(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  पटिनिवत्त 1;  /* to avoid warnings */
}

स्थिर पूर्णाङ्क checkpanic (lua_State *L) {
  संरचना Aux b;
  शून्य *ud;
  lua_State *L1;
  ध्रुव वर्ण *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  सचे (L1 == रिक्त) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    पटिनिवत्त 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  सचे (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  अञ्ञथा {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  पटिनिवत्त 1;
}


स्थिर पूर्णाङ्क externKstr (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, रिक्त, रिक्त);
  पटिनिवत्त 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
स्थिर पूर्णाङ्क externstr (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  शून्य *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  वर्ण *buff = cast_charp((*allocf)(ud, रिक्त, 0, len + 1));
  सचे (buff == रिक्त) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * परिमाण(वर्ण));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  पटिनिवत्त 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


स्थिर शून्य sethookaux (lua_State *L, पूर्णाङ्क mask, पूर्णाङ्क गणनम्, ध्रुव वर्ण *code);

स्थिर ध्रुव वर्ण *ध्रुव delimits = " \t\n,;";

स्थिर शून्य skip (ध्रुव वर्ण **pc) {
  अत्थाय (;;) {
    सचे (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    अञ्ञथा सचे (**pc == '#') {  /* comment? */
      याव (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    अञ्ञथा भङ्ग;
  }
}

स्थिर पूर्णाङ्क getnum_aux (lua_State *L, lua_State *L1, ध्रुव वर्ण **pc) {
  पूर्णाङ्क res = 0;
  पूर्णाङ्क sig = 1;
  skip(pc);
  सचे (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_निष्कासय(L1, 1);
    (*pc)++;
    पटिनिवत्त res;
  }
  अञ्ञथा सचे (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    पटिनिवत्त res;
  }
  अञ्ञथा सचे (**pc == '!') {
    (*pc)++;
    सचे (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    अञ्ञथा सचे (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    अञ्ञथा lua_assert(0);
    (*pc)++;
    पटिनिवत्त res;
  }
  अञ्ञथा सचे (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  सचे (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  याव (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  पटिनिवत्त sig*res;
}

स्थिर ध्रुव वर्ण *getstring_aux (lua_State *L, वर्ण *buff, ध्रुव वर्ण **pc) {
  पूर्णाङ्क i = 0;
  skip(pc);
  सचे (**pc == '"' || **pc == '\'') {  /* quoted string? */
    पूर्णाङ्क quote = *(*pc)++;
    याव (**pc != quote) {
      सचे (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  अञ्ञथा {
    याव (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  पटिनिवत्त buff;
}


स्थिर पूर्णाङ्क getindex_aux (lua_State *L, lua_State *L1, ध्रुव वर्ण **pc) {
  skip(pc);
  विकल्प (*(*pc)++) {
    पक्ष 'R': पटिनिवत्त LUA_REGISTRYINDEX;
    पक्ष 'U': पटिनिवत्त lua_upvalueindex(getnum_aux(L, L1, pc));
    स्वभाव: {
      पूर्णाङ्क n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      सचे (n == 0) पटिनिवत्त 0;
      अञ्ञथा पटिनिवत्त lua_absindex(L1, n);
    }
  }
}


स्थिर ध्रुव वर्ण *ध्रुव statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
स्थिर शून्य regcodes (lua_State *L) {
  unsigned पूर्णाङ्क i;
  अत्थाय (i = 0; i < परिमाण(statcodes) / परिमाण(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


स्थिर पूर्णाङ्क testC (lua_State *L);
स्थिर पूर्णाङ्क Cfunck (lua_State *L, पूर्णाङ्क status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
स्थिर ध्रुव वर्ण ops[] = "+-*%^/\\&|~<>_!";

स्थिर पूर्णाङ्क runC (lua_State *L, lua_State *L1, ध्रुव वर्ण *pc) {
  वर्ण buff[300];
  पूर्णाङ्क status = 0;
  सचे (pc == रिक्त) पटिनिवत्त luaL_error(L, "attempt to runC null script");
  अत्थाय (;;) {
    ध्रुव वर्ण *inst = getstring;
    सचे EQ("") पटिनिवत्त 0;
    अञ्ञथा सचे EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    अञ्ञथा सचे EQ("append") {
      पूर्णाङ्क t = getindex;
      पूर्णाङ्क i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    अञ्ञथा सचे EQ("arith") {
      पूर्णाङ्क op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    अञ्ञथा सचे EQ("call") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      lua_call(L1, narg, nres);
    }
    अञ्ञथा सचे EQ("callk") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      पूर्णाङ्क i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    अञ्ञथा सचे EQ("checkstack") {
      पूर्णाङ्क sz = getnum;
      ध्रुव वर्ण *सन्देशः = getstring;
      सचे (*सन्देशः == '\0')
        सन्देशः = रिक्त;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, सन्देशः);
    }
    अञ्ञथा सचे EQ("rawcheckstack") {
      पूर्णाङ्क sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    अञ्ञथा सचे EQ("compare") {
      ध्रुव वर्ण *opt = getstring;  /* EQ, LT, or LE */
      पूर्णाङ्क op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      पूर्णाङ्क a = getindex;
      पूर्णाङ्क b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    अञ्ञथा सचे EQ("concat") {
      lua_concat(L1, getnum);
    }
    अञ्ञथा सचे EQ("copy") {
      पूर्णाङ्क f = getindex;
      lua_copy(L1, f, getindex);
    }
    अञ्ञथा सचे EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    अञ्ञथा सचे EQ("getfield") {
      पूर्णाङ्क t = getindex;
      पूर्णाङ्क tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    अञ्ञथा सचे EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    अञ्ञथा सचे EQ("getmetatable") {
      सचे (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    अञ्ञथा सचे EQ("gettable") {
      पूर्णाङ्क tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    अञ्ञथा सचे EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    अञ्ञथा सचे EQ("gsub") {
      पूर्णाङ्क a = getnum; पूर्णाङ्क b = getnum; पूर्णाङ्क c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    अञ्ञथा सचे EQ("insert") {
      lua_निवेशय(L1, getnum);
    }
    अञ्ञथा सचे EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    अञ्ञथा सचे EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    अञ्ञथा सचे EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    अञ्ञथा सचे EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    अञ्ञथा सचे EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    अञ्ञथा सचे EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    अञ्ञथा सचे EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    अञ्ञथा सचे EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    अञ्ञथा सचे EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    अञ्ञथा सचे EQ("len") {
      lua_len(L1, getindex);
    }
    अञ्ञथा सचे EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    अञ्ञथा सचे EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    अञ्ञथा सचे EQ("loadstring") {
      size_t slen;
      ध्रुव वर्ण *s = luaL_checklstring(L1, getnum, &slen);
      ध्रुव वर्ण *name = getstring;
      ध्रुव वर्ण *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    अञ्ञथा सचे EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    अञ्ञथा सचे EQ("newtable") {
      lua_newtable(L1);
    }
    अञ्ञथा सचे EQ("newthread") {
      lua_newthread(L1);
    }
    अञ्ञथा सचे EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    अञ्ञथा सचे EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    अञ्ञथा सचे EQ("next") {
      lua_next(L1, -2);
    }
    अञ्ञथा सचे EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    अञ्ञथा सचे EQ("pcall") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    अञ्ञथा सचे EQ("pcallk") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      पूर्णाङ्क i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    अञ्ञथा सचे EQ("pop") {
      lua_निष्कासय(L1, getnum);
    }
    अञ्ञथा सचे EQ("printstack") {
      पूर्णाङ्क n = getnum;
      सचे (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      अञ्ञथा lua_printstack(L1);
    }
    अञ्ञथा सचे EQ("print") {
      ध्रुव वर्ण *सन्देशः = getstring;
      printf("%s\n", सन्देशः);
    }
    अञ्ञथा सचे EQ("warningC") {
      ध्रुव वर्ण *सन्देशः = getstring;
      lua_warning(L1, सन्देशः, 1);
    }
    अञ्ञथा सचे EQ("warning") {
      ध्रुव वर्ण *सन्देशः = getstring;
      lua_warning(L1, सन्देशः, 0);
    }
    अञ्ञथा सचे EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    अञ्ञथा सचे EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    अञ्ञथा सचे EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    अञ्ञथा सचे EQ("pushnil") {
      lua_pushnil(L1);
    }
    अञ्ञथा सचे EQ("pushnum") {
      lua_pushnumber(L1, (lua_सङ्ख्या)getnum);
    }
    अञ्ञथा सचे EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    अञ्ञथा सचे EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    अञ्ञथा सचे EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    अञ्ञथा सचे EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    अञ्ञथा सचे EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (पूर्णाङ्क)lua_tointeger(L, -1));
    }
    अञ्ञथा सचे EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    अञ्ञथा सचे EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    अञ्ञथा सचे EQ("rawget") {
      पूर्णाङ्क t = getindex;
      lua_rawget(L1, t);
    }
    अञ्ञथा सचे EQ("rawgeti") {
      पूर्णाङ्क t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    अञ्ञथा सचे EQ("rawgetp") {
      पूर्णाङ्क t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    अञ्ञथा सचे EQ("rawset") {
      पूर्णाङ्क t = getindex;
      lua_rawset(L1, t);
    }
    अञ्ञथा सचे EQ("rawseti") {
      पूर्णाङ्क t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    अञ्ञथा सचे EQ("rawsetp") {
      पूर्णाङ्क t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    अञ्ञथा सचे EQ("remove") {
      lua_remove(L1, getnum);
    }
    अञ्ञथा सचे EQ("replace") {
      lua_replace(L1, getindex);
    }
    अञ्ञथा सचे EQ("resume") {
      पूर्णाङ्क i = getindex;
      पूर्णाङ्क nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    अञ्ञथा सचे EQ("traceback") {
      ध्रुव वर्ण *सन्देशः = getstring;
      पूर्णाङ्क level = getnum;
      luaL_traceback(L1, L1, सन्देशः, level);
    }
    अञ्ञथा सचे EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    अञ्ञथा सचे EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    अञ्ञथा सचे EQ("return") {
      पूर्णाङ्क n = getnum;
      सचे (L1 != L) {
        पूर्णाङ्क i;
        अत्थाय (i = 0; i < n; i++) {
          पूर्णाङ्क idx = -(n - i);
          विकल्प (lua_type(L1, idx)) {
            पक्ष LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              भङ्ग;
            स्वभाव:
              lua_pushstring(L, lua_tostring(L1, idx));
              भङ्ग;
          }
        }
      }
      पटिनिवत्त n;
    }
    अञ्ञथा सचे EQ("rotate") {
      पूर्णाङ्क i = getindex;
      lua_rotate(L1, i, getnum);
    }
    अञ्ञथा सचे EQ("setfield") {
      पूर्णाङ्क t = getindex;
      ध्रुव वर्ण *s = getstring;
      lua_setfield(L1, t, s);
    }
    अञ्ञथा सचे EQ("seti") {
      पूर्णाङ्क t = getindex;
      lua_seti(L1, t, getnum);
    }
    अञ्ञथा सचे EQ("setglobal") {
      ध्रुव वर्ण *s = getstring;
      lua_setglobal(L1, s);
    }
    अञ्ञथा सचे EQ("sethook") {
      पूर्णाङ्क mask = getnum;
      पूर्णाङ्क गणनम् = getnum;
      ध्रुव वर्ण *s = getstring;
      sethookaux(L1, mask, गणनम्, s);
    }
    अञ्ञथा सचे EQ("setmetatable") {
      पूर्णाङ्क idx = getindex;
      lua_setmetatable(L1, idx);
    }
    अञ्ञथा सचे EQ("settable") {
      lua_settable(L1, getindex);
    }
    अञ्ञथा सचे EQ("settop") {
      lua_settop(L1, getnum);
    }
    अञ्ञथा सचे EQ("testudata") {
      पूर्णाङ्क i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != रिक्त);
    }
    अञ्ञथा सचे EQ("error") {
      lua_error(L1);
    }
    अञ्ञथा सचे EQ("abort") {
      abort();
    }
    अञ्ञथा सचे EQ("throw") {
#सचे defined(__cplusplus)
स्थिर संरचना X { पूर्णाङ्क x; } x;
      throw x;
#अञ्ञथा
      luaL_error(L1, "C++");
#endif
      भङ्ग;
    }
    अञ्ञथा सचे EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    अञ्ञथा सचे EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    अञ्ञथा सचे EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    अञ्ञथा सचे EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    अञ्ञथा सचे EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    अञ्ञथा सचे EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    अञ्ञथा सचे EQ("tostring") {
      ध्रुव वर्ण *s = lua_tostring(L1, getindex);
      ध्रुव वर्ण *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == रिक्त && s1 == रिक्त) || strcmp(s, s1) == 0);
    }
    अञ्ञथा सचे EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, रिक्त);
    }
    अञ्ञथा सचे EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    अञ्ञथा सचे EQ("xmove") {
      पूर्णाङ्क f = getindex;
      पूर्णाङ्क t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      पूर्णाङ्क n = getnum;
      सचे (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    अञ्ञथा सचे EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    अञ्ञथा सचे EQ("yield") {
      पटिनिवत्त lua_yield(L1, getnum);
    }
    अञ्ञथा सचे EQ("yieldk") {
      पूर्णाङ्क nres = getnum;
      पूर्णाङ्क i = getindex;
      पटिनिवत्त lua_yieldk(L1, nres, i, Cfunck);
    }
    अञ्ञथा सचे EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    अञ्ञथा सचे EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    अञ्ञथा सचे EQ("argerror") {
      पूर्णाङ्क arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    अञ्ञथा luaL_error(L, "unknown instruction %s", buff);
  }
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क testC (lua_State *L) {
  lua_State *L1;
  ध्रुव वर्ण *pc;
  सचे (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  अञ्ञथा सचे (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  अञ्ञथा {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  पटिनिवत्त runC(L, L1, pc);
}


स्थिर पूर्णाङ्क Cfunc (lua_State *L) {
  पटिनिवत्त runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


स्थिर पूर्णाङ्क Cfunck (lua_State *L, पूर्णाङ्क status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  पटिनिवत्त runC(L, L, lua_tostring(L, cast_int(ctx)));
}


स्थिर पूर्णाङ्क makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  पटिनिवत्त 1;
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
स्थिर शून्य Chook (lua_State *L, lua_Debug *ar) {
  ध्रुव वर्ण *scpt;
  ध्रुव वर्ण *ध्रुव events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_निष्कासय(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
स्थिर शून्य sethookaux (lua_State *L, पूर्णाङ्क mask, पूर्णाङ्क गणनम्, ध्रुव वर्ण *scpt) {
  सचे (*scpt == '\0') {  /* no script? */
    lua_sethook(L, रिक्त, 0, 0);  /* turn off hooks */
    पटिनिवत्त;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  सचे (!lua_istable(L, -1)) {  /* no hook table? */
    lua_निष्कासय(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, गणनम्);
}


स्थिर पूर्णाङ्क sethook (lua_State *L) {
  सचे (lua_isnoneornil(L, 1))
    lua_sethook(L, रिक्त, 0, 0);  /* turn off hooks */
  अञ्ञथा {
    ध्रुव वर्ण *scpt = luaL_checkstring(L, 1);
    ध्रुव वर्ण *smask = luaL_checkstring(L, 2);
    पूर्णाङ्क गणनम् = cast_int(luaL_optinteger(L, 3, 0));
    पूर्णाङ्क mask = 0;
    सचे (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    सचे (strchr(smask, 'r')) mask |= LUA_MASKRET;
    सचे (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    सचे (गणनम् > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, गणनम्, scpt);
  }
  पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क coresume (lua_State *L) {
  पूर्णाङ्क status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  सचे (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_निवेशय(L, -2);
    पटिनिवत्त 2;  /* return false + error message */
  }
  अञ्ञथा {
    lua_pushboolean(L, 1);
    पटिनिवत्त 1;
  }
}

#सचे !defined(LUA_USE_POSIX)

#define nonblock	रिक्त

#अञ्ञथा

#अन्तर्भाव <unistd.h>
#अन्तर्भाव <fcntl.h>

स्थिर पूर्णाङ्क nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  पूर्णाङ्क fd = fileno(f);
  पूर्णाङ्क flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  पटिनिवत्त 0;
}
#endif

/* }====================================================== */



स्थिर ध्रुव संरचना luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_गणनम्},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {रिक्त, रिक्त}
};


स्थिर शून्य checkfinalmem (शून्य) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


पूर्णाङ्क luaB_opentests (lua_State *L) {
  शून्य *ud;
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
  पटिनिवत्त 1;
}

#endif

