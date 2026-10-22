/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <setjmp.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lapi.h"
#ଅନ୍ତର୍ଭାଵ "lauxlib.h"
#ଅନ୍ତର୍ଭାଵ "lcode.h"
#ଅନ୍ତର୍ଭାଵ "lctype.h"
#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lfunc.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lopcodes.h"
#ଅନ୍ତର୍ଭାଵ "lopnames.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#ଯଦି defined(LUA_DEBUG)


ଶୂନ୍ୟ *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା runC (lua_State *L, lua_State *L1, ଅଚଳ ଅକ୍ଷର *pc);


ସ୍ଥିର ଶୂନ୍ୟ setnameval (lua_State *L, ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


ସ୍ଥିର ଶୂନ୍ୟ pushobject (lua_State *L, ଅଚଳ TValue *o) {
  setobj2s(L, L->ଶୀର୍ଷ.p, o);
  api_incr_ଶୀର୍ଷ(L);
}


ସ୍ଥିର ଶୂନ୍ୟ badexit (ଅଚଳ ଅକ୍ଷର *fmt, ଅଚଳ ଅକ୍ଷର *s1, ଅଚଳ ଅକ୍ଷର *s2) {
  fprintf(stderr, fmt, s1);
  ଯଦି (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tpanic (lua_State *L) {
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ଫେରନ୍ତୁ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   ସନ୍ଦେଶ, ଖାଲି),
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
ସ୍ଥିର ଶୂନ୍ୟ warnf (ଶୂନ୍ୟ *ud, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont) {
  lua_State *L = cast(lua_State *, ud);
  ସ୍ଥିର ଅକ୍ଷର buff[200] = "";  /* should be enough for tests... */
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା onoff = 0;
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode = 0;  /* start in normal mode */
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା lasttocont = 0;
  ଯଦି (!lasttocont && !tocont && *ସନ୍ଦେଶ == '@') {  /* control message? */
    ଯଦି (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", ସନ୍ଦେଶ, buff);
    ଯଦି (strcmp(ସନ୍ଦେଶ, "@off") == 0)
      onoff = 0;
    ଅନ୍ୟଥା ଯଦି (strcmp(ସନ୍ଦେଶ, "@on") == 0)
      onoff = 1;
    ଅନ୍ୟଥା ଯଦି (strcmp(ସନ୍ଦେଶ, "@normal") == 0)
      mode = 0;
    ଅନ୍ୟଥା ଯଦି (strcmp(ସନ୍ଦେଶ, "@allow") == 0)
      mode = 1;
    ଅନ୍ୟଥା ଯଦି (strcmp(ସନ୍ଦେଶ, "@store") == 0)
      mode = 2;
    ଅନ୍ୟଥା
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              ସନ୍ଦେଶ, ଖାଲି);
    ଫେରନ୍ତୁ;
  }
  lasttocont = tocont;
  ଯଦି (strlen(ସନ୍ଦେଶ) >= ଆକାର(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", ସନ୍ଦେଶ, buff);
  strcat(buff, ସନ୍ଦେଶ);  /* add new message to current warning */
  ଯଦି (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ଯଦି (!lua_toboolean(L, -1))
      lua_କାଢ଼(L, 1);  /* ok, no previous unexpected warning */
    ଅନ୍ୟଥା {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    ବାଛ (mode) {
      ସ୍ଥିତି 0: {  /* normal */
        ଯଦି (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ଖାଲି);
      }  /* FALLTHROUGH */
      ସ୍ଥିତି 1: {  /* allow */
        ଯଦି (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        ଭାଙ୍ଗ;
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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂଘ memHeader {
  LUAI_MAXALIGN;
  ସଂରଚନା {
    size_t size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା type;
  } d;
} memHeader;


#ଯଦି !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#ଅନ୍ୟଥା

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


ସ୍ଥିର ଶୂନ୍ୟ freeblock (Memcontrol *mc, memHeader *block) {
  ଯଦି (block) {
    size_t size = block->d.size;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, ଆକାର(memHeader) + size + MARKSIZE);  /* erase block */
    ମୁକ୍ତ(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


ଶୂନ୍ୟ *debug_realloc (ଶୂନ୍ୟ *ud, ଶୂନ୍ୟ *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା type;
  ଯଦି (mc->memlimit == 0) {  /* first time? */
    ଅକ୍ଷର *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, ଖାଲି, 10) : ULONG_MAX;
  }
  ଯଦି (block == ଖାଲି) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  ଅନ୍ୟଥା {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  ଯଦି (size == 0) {
    freeblock(mc, block);
    ଫେରନ୍ତୁ ଖାଲି;
  }
  ଯଦି (mc->failnext) {
    mc->failnext = 0;
    ଫେରନ୍ତୁ ଖାଲି;  /* fake a single memory allocation error */
  }
  ଯଦି (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    ଯଦି (mc->countlimit == 0)
      ଫେରନ୍ତୁ ଖାଲି;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ଯଦି (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ଫେରନ୍ତୁ ଖାଲି;  /* fake a memory allocation error */
  ଅନ୍ୟଥା {
    memHeader *newblock;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = ଆକାର(memHeader) + size + MARKSIZE;
    ଯଦି (realsize < size) ଫେରନ୍ତୁ ଖାଲି;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    ଯଦି (newblock == ଖାଲି)
      ଫେରନ୍ତୁ ଖାଲି;  /* really out of memory? */
    ଯଦି (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    ପାଇଁ (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    ଯଦି (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ଫେରନ୍ତୁ newblock + 1;
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

#ଅନ୍ତର୍ଭାଵ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ଯଦି (isdead(g,t)) ଫେରନ୍ତୁ 0;
  ଯଦି (issweepphase(g))
    ଫେରନ୍ତୁ 1;  /* no invariants */
  ଅନ୍ୟଥା ଯଦି (g->gckind != KGC_GENMINOR)
    ଫେରନ୍ତୁ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  ଅନ୍ୟଥା {  /* generational mode */
    ଯଦି ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ଫେରନ୍ତୁ 0;
    ଯଦି ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ 1;
  }
}


ସ୍ଥିର ଶୂନ୍ୟ printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (ଶୂନ୍ୟ *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ଯଦି (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


ଶୂନ୍ୟ lua_printobj (lua_State *L, ସଂରଚନା GCObject *o) {
  printobj(G(L), o);
}


ଶୂନ୍ୟ lua_printvalue (TValue *v) {
  ବାଛ (ttypetag(v)) {
    ସ୍ଥିତି LUA_VNUMINT: ସ୍ଥିତି LUA_VNUMFLT: {
      ଅକ୍ଷର buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VFALSE:
      printf("%s", "false"); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VTRUE:
      printf("%s", "true"); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VNIL:
      printf("nil"); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VLCF:
      printf("light C function: %p", fvalue(v)); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VTABLE:
      printf("table: %p", hvalue(v)); ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      lua_assert(0);
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testobjref (global_State *g, GCObject *f, GCObject *t) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = testobjref1(g, f, t);
  ଯଦି (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ଫେରନ୍ତୁ r1;
}


ସ୍ଥିର ଶୂନ୍ୟ checkobjref (global_State *g, GCObject *f, GCObject *t) {
    ପ୍ରତିଜ୍ଞା(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ ଯଦି (t) checkobjref(g,f,obj2gco(t)); }


ସ୍ଥିର ଶୂନ୍ୟ checkvalref (global_State *g, GCObject *f, ଅଚଳ TValue *t) {
  ପ୍ରତିଜ୍ଞା(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


ସ୍ଥିର ଶୂନ୍ୟ checktable (global_State *g, Table *h) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା asize = h->asize;
  ଗଣ୍ଠି_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  ପାଇଁ (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  ପାଇଁ (n = gnode(h, 0); n < limit; n++) {
    ଯଦି (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      ପ୍ରତିଜ୍ଞା(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


ସ୍ଥିର ଶୂନ୍ୟ checkudata (global_State *g, Udata *u) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  ପାଇଁ (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


ସ୍ଥିର ଶୂନ୍ୟ checkproto (global_State *g, Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  ପାଇଁ (i=0; i<f->sizek; i++) {
    ଯଦି (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  ପାଇଁ (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  ପାଇଁ (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  ପାଇଁ (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


ସ୍ଥିର ଶୂନ୍ୟ checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


ସ୍ଥିର ଶୂନ୍ୟ checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  checkobjrefN(g, clgc, cl->p);
  ପାଇଁ (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    ଯଦି (uv) {
      checkobjrefN(g, clgc, uv);
      ଯଦି (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_checkpc (CallInfo *ci) {
  ଯଦି (!isLua(ci)) ଫେରନ୍ତୁ 1;
  ଅନ୍ୟଥା {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ଫେରନ୍ତୁ p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


ସ୍ଥିର ଶୂନ୍ୟ check_ଗଦା (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  ପ୍ରତିଜ୍ଞା(!isdead(g, L1));
  ଯଦି (L1->ଗଦା.p == ଖାଲି) {  /* incomplete thread? */
    ପ୍ରତିଜ୍ଞା(L1->openupval == ଖାଲି && L1->ci == ଖାଲି);
    ଫେରନ୍ତୁ;
  }
  ପାଇଁ (uv = L1->openupval; uv != ଖାଲି; uv = uv->u.open.next)
    ପ୍ରତିଜ୍ଞା(upisopen(uv));  /* must be open */
  ପ୍ରତିଜ୍ଞା(L1->ଶୀର୍ଷ.p <= L1->ଗଦା_last.p);
  ପ୍ରତିଜ୍ଞା(L1->tbclist.p <= L1->ଶୀର୍ଷ.p);
  ପାଇଁ (ci = L1->ci; ci != ଖାଲି; ci = ci->previous) {
    ପ୍ରତିଜ୍ଞା(ci->ଶୀର୍ଷ.p <= L1->ଗଦା_last.p);
    ପ୍ରତିଜ୍ଞା(lua_checkpc(ci));
  }
  ପାଇଁ (o = L1->ଗଦା.p; o < L1->ଗଦା_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


ସ୍ଥିର ଶୂନ୍ୟ checkrefs (global_State *g, GCObject *o) {
  ବାଛ (o->tt) {
    ସ୍ଥିତି LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VTABLE: {
      checktable(g, gco2t(o));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VTHREAD: {
      check_ଗଦା(g, gco2th(o));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VPROTO: {
      checkproto(g, gco2p(o));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି LUA_VSHRSTR:
    ସ୍ଥିତି LUA_VLNGSTR: {
      ପ୍ରତିଜ୍ଞା(!isgray(o));  /* strings are never gray */
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ପ୍ରତିଜ୍ଞା(0);
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
ସ୍ଥିର ଶୂନ୍ୟ checkobject (global_State *g, GCObject *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maybedead,
                         ପୂର୍ଣ୍ଣସଂଖ୍ୟା listage) {
  ଯଦି (isdead(g, o))
    ପ୍ରତିଜ୍ଞା(maybedead);
  ଅନ୍ୟଥା {
    ପ୍ରତିଜ୍ଞା(g->gcstate != GCSpause || iswhite(o));
    ଯଦି (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      ପ୍ରତିଜ୍ଞା(getage(o) >= listage);
      ଯଦି (isold(o)) {
        ପ୍ରତିଜ୍ଞା(!iswhite(o));
        ପ୍ରତିଜ୍ଞା(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      ପ୍ରତିଜ୍ଞା(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


ସ୍ଥିର l_mem checkgraylist (global_State *g, GCObject *o) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ଯେତେବେଳେ (o) {
    ପ୍ରତିଜ୍ଞା(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    ପ୍ରତିଜ୍ଞା(!testbit(o->marked, TESTBIT));
    ଯଦି (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    ବାଛ (o->tt) {
      ସ୍ଥିତି LUA_VTABLE: o = gco2t(o)->gclist; ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VLCL: o = gco2lcl(o)->gclist; ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VCCL: o = gco2ccl(o)->gclist; ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VTHREAD: o = gco2th(o)->gclist; ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VPROTO: o = gco2p(o)->gclist; ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VUSERDATA:
        ପ୍ରତିଜ୍ଞା(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ପ୍ରତିଜ୍ଞା(0);  /* other objects cannot be in a gray list */
    }
  }
  ଫେରନ୍ତୁ total;
}


/*
** Check objects in gray lists.
*/
ସ୍ଥିର l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  ଯଦି (!keepinvariant(g)) ଫେରନ୍ତୁ total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ଫେରନ୍ତୁ total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
ସ୍ଥିର ଶୂନ୍ୟ incifingray (global_State *g, GCObject *o, l_mem *ଗଣନା_2) {
  ଯଦି (!keepinvariant(g))
    ଫେରନ୍ତୁ;  /* gray lists not being kept in these phases */
  ଯଦି (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    ପ୍ରତିଜ୍ଞା(!isgray(o) || upisopen(gco2upv(o)));
    ଫେରନ୍ତୁ;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ଯଦି (isgray(o) || getage(o) == G_TOUCHED2) {
    (*ଗଣନା_2)++;
    ପ୍ରତିଜ୍ଞା(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


ସ୍ଥିର l_mem checklist (global_State *g, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maybedead, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  ପାଇଁ (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    ପ୍ରତିଜ୍ଞା(!tof == !tofinalize(o));
  }
  ପାଇଁ (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    ପ୍ରତିଜ୍ଞା(!tof == !tofinalize(o));
  }
  ପାଇଁ (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    ପ୍ରତିଜ୍ଞା(!tof == !tofinalize(o));
  }
  ପାଇଁ (o = reallyold; o != ଖାଲି; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    ପ୍ରତିଜ୍ଞା(!tof == !tofinalize(o));
  }
  ଫେରନ୍ତୁ total;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ଯଦି (keepinvariant(g)) {
    ପ୍ରତିଜ୍ଞା(!iswhite(mainthread(g)));
    ପ୍ରତିଜ୍ଞା(!iswhite(gcvalue(&g->l_registry)));
  }
  ପ୍ରତିଜ୍ଞା(!isdead(g, gcvalue(&g->l_registry)));
  ପ୍ରତିଜ୍ଞା(g->sweepgc == ଖାଲି || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  ପାଇଁ (o = g->fixedgc; o != ଖାଲି; o = o->next) {
    ପ୍ରତିଜ୍ଞା(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  ପାଇଁ (o = g->tobefnz; o != ଖାଲି; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    ପ୍ରତିଜ୍ଞା(tofinalize(o));
    ପ୍ରତିଜ୍ଞା(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ଯଦି (keepinvariant(g))
    ପ୍ରତିଜ୍ଞା(totalin == totalshould);
  ଫେରନ୍ତୁ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


ସ୍ଥିର ଅକ୍ଷର *buildop (Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc, ଅକ୍ଷର *buff) {
  ଅକ୍ଷର *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  ଅଚଳ ଅକ୍ଷର *name = opnames[o];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି = luaG_getfuncline(p, pc);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineinfo = (p->lineinfo != ଖାଲି) ? p->lineinfo[pc] : 0;
  ଯଦି (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  ଅନ୍ୟଥା
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", ଧାଡ଼ି, pc);
  ବାଛ (getOpMode(o)) {
    ସ୍ଥିତି iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      ଭାଙ୍ଗ;
    ସ୍ଥିତି ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      ଭାଙ୍ଗ;
    ସ୍ଥିତି iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ obuff;
}


#ଯଦି 0
ଶୂନ୍ୟ luaI_printcode (Proto *pt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc;
  ପାଇଁ (pc=0; pc<size; pc++) {
    ଅକ୍ଷର buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


ଶୂନ୍ୟ luaI_printinst (Proto *pt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc) {
  ଅକ୍ଷର buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା listcode (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  ପାଇଁ (pc=0; pc<p->sizecode; pc++) {
    ଅକ୍ଷର buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା printcode (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  ପାଇଁ (pc=0; pc<p->sizecode; pc++) {
    ଅକ୍ଷର buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା listk (lua_State *L) {
  Proto *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  ପାଇଁ (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା listabslineinfo (lua_State *L) {
  Proto *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ଖାଲି, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  ପାଇଁ (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].ଧାଡ଼ି);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା listlocals (lua_State *L) {
  Proto *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
  ଅଚଳ ଅକ୍ଷର *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ଯେତେବେଳେ ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ଖାଲି)
    lua_pushstring(L, name);
  ଫେରନ୍ତୁ i-1;
}

/* }====================================================== */



ଶୂନ୍ୟ lua_printstack (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = lua_gettop(L);
  printf("stack: >>\n");
  ପାଇଁ (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_printallstack (lua_State *L) {
  StkId p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  ପାଇଁ (p = L->ଗଦା.p; p < L->ଶୀର୍ଷ.p; p++) {
    ଯଦି (ci != ଖାଲି && p == ci->func.p) {
      printf("  ---\n");
      ଯଦି (ci == L->ci)
        ci = ଖାଲି;  /* printed last frame */
      ଅନ୍ୟଥା
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", ଅଙ୍କ_OPCODES);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", ଆକାର(lua_State));
  setnameval(L, "global state", ଆକାର(global_State));
  setnameval(L, "TValue", ଆକାର(TValue));
  setnameval(L, "Node", ଆକାର(ଗଣ୍ଠି_2));
  setnameval(L, "stack Value", ଆକାର(ଗଦା_ମୂଲ୍ୟ));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mem_query (lua_State *L) {
  ଯଦି (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ଫେରନ୍ତୁ 3;
  }
  ଅନ୍ୟଥା ଯଦି (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    ଯଦି (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ଫେରନ୍ତୁ 0;
  }
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *t = luaL_checkstring(L, 1);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ଯଦି (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ଫେରନ୍ତୁ 1;
      }
    }
    ଫେରନ୍ତୁ luaL_error(L, "unknown type '%s'", t);
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା alloc_ଗଣନା (lua_State *L) {
  ଯଦି (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  ଅନ୍ୟଥା
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା settrick (lua_State *L) {
  ଯଦି (ttisnil(obj_at(L, 1)))
    l_Trick = ଖାଲି;
  ଅନ୍ୟଥା
    l_Trick = gcvalue(obj_at(L, 1));
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ଯଦି (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ଅନ୍ୟଥା {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ଯଦି (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ଅନ୍ୟଥା {
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ଯଦି (!iscollectable(o))
    printf("no collectable\n");
  ଅନ୍ୟଥା {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gc_state (lua_State *L) {
  ସ୍ଥିର ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ଯଦି (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ଫେରନ୍ତୁ 1;
  }
  ଅନ୍ୟଥା {
    ଯଦି (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    ଯଦି (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ଫେରନ୍ତୁ 0;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tracinggc = 0;
ଶୂନ୍ୟ luai_tracegctest (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା first) {
  ଯଦି (!tracinggc) ଫେରନ୍ତୁ;
  ଅନ୍ୟଥା {
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


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tracegc (lua_State *L) {
  ଯଦି (lua_isnil(L, 1))
    tracinggc = 0;
  ଅନ୍ୟଥା {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hash_query (lua_State *L) {
  ଯଦି (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ଯଦି (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  ଅନ୍ୟଥା {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->ଗଣ୍ଠି));
  }
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stacklevel (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା a = 0;
  lua_pushinteger(L, cast_Integer(L->ଶୀର୍ଷ.p - L->ଗଦା.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ଫେରନ୍ତୁ 5;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ଯେତେବେଳେ (ci->next != ଖାଲି) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reallocstack (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->ଶୀର୍ଷ.p - L->ଗଦା.p) + n, 1);
  lua_unlock(L);
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା table_query (lua_State *L) {
  ଅଚଳ Table *t;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ଯଦି (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ଫେରନ୍ତୁ 3;
  }
  ଅନ୍ୟଥା ଯଦି (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ଯଦି (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->ଶୀର୍ଷ.p));
    ଅନ୍ୟଥା
      setnilvalue2s(L->ଶୀର୍ଷ.p);
    api_incr_ଶୀର୍ଷ(L);
    lua_pushnil(L);
  }
  ଅନ୍ୟଥା ଯଦି (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ଯଦି (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    ଅନ୍ୟଥା
      lua_pushliteral(L, "<undef>");
    ଯଦି (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    ଅନ୍ୟଥା
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->ଗଣ୍ଠି[i]));
  }
  ଫେରନ୍ତୁ 3;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ଫେରନ୍ତୁ 6;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ଯଦି (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ଫେରନ୍ତୁ 2;
  }
  ଅନ୍ୟଥା ଯଦି (s < tb->size) {
    TString *ts;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
    ପାଇଁ (ts = tb->hash[s]; ts != ଖାଲି; ts = ts->u.hnext) {
      setsvalue2s(L, L->ଶୀର୍ଷ.p, ts);
      api_incr_ଶୀର୍ଷ(L);
      n++;
    }
    ଫେରନ୍ତୁ n;
  }
  ଅନ୍ୟଥା ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getreftable (lua_State *L) {
  ଯଦି (lua_istable(L, 2))  /* is there a table as second argument? */
    ଫେରନ୍ତୁ 2;  /* use it as the table */
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ LUA_REGISTRYINDEX;  /* default is to use the register */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tref (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getreftable(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getref (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getreftable(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା unref (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getreftable(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା upvalue (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ଯଦି (lua_isnone(L, 3)) {
    ଅଚଳ ଅକ୍ଷର *name = lua_getupvalue(L, 1, n);
    ଯଦି (name == ଖାଲି) ଫେରନ୍ତୁ 0;
    lua_pushstring(L, name);
    ଫେରନ୍ତୁ 2;
  }
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ଫେରନ୍ତୁ 1;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nuv = cast_int(luaL_optinteger(L, 2, 0));
  ଅକ୍ଷର *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  ଯେତେବେଳେ (size--) *p++ = '\0';
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &l);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = luaL_loadbufferx(L1, s, l, s, "t");
  ଯଦି (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା s2d (lua_State *L) {
  lua_pushnumber(L, cast_ଅଙ୍କ(*cast(ଅଚଳ ଦ୍ୱିଗୁଣ *, luaL_checkstring(L, 1))));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା d2s (lua_State *L) {
  ଦ୍ୱିଗୁଣ d = cast(ଦ୍ୱିଗୁଣ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), ଆକାର(d));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା newstate (lua_State *L) {
  ଶୂନ୍ୟ *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ଯଦି (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  ଅନ୍ୟଥା
    lua_pushnil(L);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ଖାଲି, 1, "state expected");
  ଫେରନ୍ତୁ L1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା load = cast_int(luaL_checkinteger(L, 2));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ଖାଲି, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ଅଚଳ ଅକ୍ଷର *code = luaL_checklstring(L, 2, &lcode);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  ଯଦି (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ଯଦି (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ଫେରନ୍ତୁ 3;
  }
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
    ଯେତେବେଳେ (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_କାଢ଼(L1, i-1);
    ଫେରନ୍ତୁ i-1;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା log2_aux (lua_State *L) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ଫେରନ୍ତୁ 1;
}


ସଂରଚନା Aux { jmp_buf jb; ଅଚଳ ଅକ୍ଷର *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା panicback (lua_State *L) {
  ସଂରଚନା Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (ସଂରଚନା Aux *)lua_touserdata(L, -1);
  lua_କାଢ଼(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ଫେରନ୍ତୁ 1;  /* to avoid warnings */
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkpanic (lua_State *L) {
  ସଂରଚନା Aux b;
  ଶୂନ୍ୟ *ud;
  lua_State *L1;
  ଅଚଳ ଅକ୍ଷର *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ଯଦି (L1 == ଖାଲି) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ଫେରନ୍ତୁ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ଯଦି (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  ଅନ୍ୟଥା {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ଫେରନ୍ତୁ 1;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା externKstr (lua_State *L) {
  size_t len;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ଖାଲି, ଖାଲି);
  ଫେରନ୍ତୁ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା externstr (lua_State *L) {
  size_t len;
  ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L, 1, &len);
  ଶୂନ୍ୟ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  ଅକ୍ଷର *buff = cast_charp((*allocf)(ud, ଖାଲି, 0, len + 1));
  ଯଦି (buff == ଖାଲି) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * ଆକାର(ଅକ୍ଷର));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ଫେରନ୍ତୁ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


ସ୍ଥିର ଶୂନ୍ୟ sethookaux (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2, ଅଚଳ ଅକ୍ଷର *code);

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ delimits = " \t\n,;";

ସ୍ଥିର ଶୂନ୍ୟ skip (ଅଚଳ ଅକ୍ଷର **pc) {
  ପାଇଁ (;;) {
    ଯଦି (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    ଅନ୍ୟଥା ଯଦି (**pc == '#') {  /* comment? */
      ଯେତେବେଳେ (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    ଅନ୍ୟଥା ଭାଙ୍ଗ;
  }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getnum_aux (lua_State *L, lua_State *L1, ଅଚଳ ଅକ୍ଷର **pc) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig = 1;
  skip(pc);
  ଯଦି (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_କାଢ଼(L1, 1);
    (*pc)++;
    ଫେରନ୍ତୁ res;
  }
  ଅନ୍ୟଥା ଯଦି (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ଫେରନ୍ତୁ res;
  }
  ଅନ୍ୟଥା ଯଦି (**pc == '!') {
    (*pc)++;
    ଯଦି (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    ଅନ୍ୟଥା ଯଦି (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    ଅନ୍ୟଥା lua_assert(0);
    (*pc)++;
    ଫେରନ୍ତୁ res;
  }
  ଅନ୍ୟଥା ଯଦି (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ଯଦି (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ଯେତେବେଳେ (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ଫେରନ୍ତୁ sig*res;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getstring_aux (lua_State *L, ଅକ୍ଷର *buff, ଅଚଳ ଅକ୍ଷର **pc) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
  skip(pc);
  ଯଦି (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା quote = *(*pc)++;
    ଯେତେବେଳେ (**pc != quote) {
      ଯଦି (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  ଅନ୍ୟଥା {
    ଯେତେବେଳେ (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ଫେରନ୍ତୁ buff;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getindex_aux (lua_State *L, lua_State *L1, ଅଚଳ ଅକ୍ଷର **pc) {
  skip(pc);
  ବାଛ (*(*pc)++) {
    ସ୍ଥିତି 'R': ଫେରନ୍ତୁ LUA_REGISTRYINDEX;
    ସ୍ଥିତି 'U': ଫେରନ୍ତୁ lua_upvalueindex(getnum_aux(L, L1, pc));
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ଯଦି (n == 0) ଫେରନ୍ତୁ 0;
      ଅନ୍ୟଥା ଫେରନ୍ତୁ lua_absindex(L1, n);
    }
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
ସ୍ଥିର ଶୂନ୍ୟ regcodes (lua_State *L) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < ଆକାର(statcodes) / ଆକାର(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testC (lua_State *L);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା Cfunck (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର ops[] = "+-*%^/\\&|~<>_!";

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା runC (lua_State *L, lua_State *L1, ଅଚଳ ଅକ୍ଷର *pc) {
  ଅକ୍ଷର buff[300];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = 0;
  ଯଦି (pc == ଖାଲି) ଫେରନ୍ତୁ luaL_error(L, "attempt to runC null script");
  ପାଇଁ (;;) {
    ଅଚଳ ଅକ୍ଷର *inst = getstring;
    ଯଦି EQ("") ଫେରନ୍ତୁ 0;
    ଅନ୍ୟଥା ଯଦି EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    ଅନ୍ୟଥା ଯଦି EQ("append") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("arith") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    ଅନ୍ୟଥା ଯଦି EQ("call") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା narg = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres = getnum;
      lua_call(L1, narg, nres);
    }
    ଅନ୍ୟଥା ଯଦି EQ("callk") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା narg = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    ଅନ୍ୟଥା ଯଦି EQ("checkstack") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = getnum;
      ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = getstring;
      ଯଦି (*ସନ୍ଦେଶ == '\0')
        ସନ୍ଦେଶ = ଖାଲି;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, ସନ୍ଦେଶ);
    }
    ଅନ୍ୟଥା ଯଦି EQ("rawcheckstack") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    ଅନ୍ୟଥା ଯଦି EQ("compare") {
      ଅଚଳ ଅକ୍ଷର *opt = getstring;  /* EQ, LT, or LE */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା a = getindex;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    ଅନ୍ୟଥା ଯଦି EQ("concat") {
      lua_concat(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("copy") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା f = getindex;
      lua_copy(L1, f, getindex);
    }
    ଅନ୍ୟଥା ଯଦି EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    ଅନ୍ୟଥା ଯଦି EQ("getfield") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    ଅନ୍ୟଥା ଯଦି EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    ଅନ୍ୟଥା ଯଦି EQ("getmetatable") {
      ଯଦି (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("gettable") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    ଅନ୍ୟଥା ଯଦି EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    ଅନ୍ୟଥା ଯଦି EQ("gsub") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା a = getnum; ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = getnum; ପୂର୍ଣ୍ଣସଂଖ୍ୟା c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    ଅନ୍ୟଥା ଯଦି EQ("insert") {
      lua_ଭର୍ତ୍ତି(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("len") {
      lua_len(L1, getindex);
    }
    ଅନ୍ୟଥା ଯଦି EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    ଅନ୍ୟଥା ଯଦି EQ("loadstring") {
      size_t slen;
      ଅଚଳ ଅକ୍ଷର *s = luaL_checklstring(L1, getnum, &slen);
      ଅଚଳ ଅକ୍ଷର *name = getstring;
      ଅଚଳ ଅକ୍ଷର *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    ଅନ୍ୟଥା ଯଦି EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    ଅନ୍ୟଥା ଯଦି EQ("newtable") {
      lua_newtable(L1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("newthread") {
      lua_newthread(L1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    ଅନ୍ୟଥା ଯଦି EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    ଅନ୍ୟଥା ଯଦି EQ("next") {
      lua_next(L1, -2);
    }
    ଅନ୍ୟଥା ଯଦି EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    ଅନ୍ୟଥା ଯଦି EQ("pcall") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା narg = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pcallk") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା narg = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pop") {
      lua_କାଢ଼(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("printstack") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = getnum;
      ଯଦି (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      ଅନ୍ୟଥା lua_printstack(L1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("print") {
      ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = getstring;
      printf("%s\n", ସନ୍ଦେଶ);
    }
    ଅନ୍ୟଥା ଯଦି EQ("warningC") {
      ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = getstring;
      lua_warning(L1, ସନ୍ଦେଶ, 1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("warning") {
      ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = getstring;
      lua_warning(L1, ସନ୍ଦେଶ, 0);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushnil") {
      lua_pushnil(L1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushnum") {
      lua_pushnumber(L1, (lua_ସଂଖ୍ୟା)getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)lua_tointeger(L, -1));
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    ଅନ୍ୟଥା ଯଦି EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    ଅନ୍ୟଥା ଯଦି EQ("rawget") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_rawget(L1, t);
    }
    ଅନ୍ୟଥା ଯଦି EQ("rawgeti") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("rawgetp") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ଅନ୍ୟଥା ଯଦି EQ("rawset") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_rawset(L1, t);
    }
    ଅନ୍ୟଥା ଯଦି EQ("rawseti") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("rawsetp") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ଅନ୍ୟଥା ଯଦି EQ("remove") {
      lua_remove(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("replace") {
      lua_replace(L1, getindex);
    }
    ଅନ୍ୟଥା ଯଦି EQ("resume") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = getindex;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    ଅନ୍ୟଥା ଯଦି EQ("traceback") {
      ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = getstring;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = getnum;
      luaL_traceback(L1, L1, ସନ୍ଦେଶ, level);
    }
    ଅନ୍ୟଥା ଯଦି EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    ଅନ୍ୟଥା ଯଦି EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("return") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = getnum;
      ଯଦି (L1 != L) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ (i = 0; i < n; i++) {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx = -(n - i);
          ବାଛ (lua_type(L1, idx)) {
            ସ୍ଥିତି LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              ଭାଙ୍ଗ;
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
              lua_pushstring(L, lua_tostring(L1, idx));
              ଭାଙ୍ଗ;
          }
        }
      }
      ଫେରନ୍ତୁ n;
    }
    ଅନ୍ୟଥା ଯଦି EQ("rotate") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = getindex;
      lua_rotate(L1, i, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("setfield") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      ଅଚଳ ଅକ୍ଷର *s = getstring;
      lua_setfield(L1, t, s);
    }
    ଅନ୍ୟଥା ଯଦି EQ("seti") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_seti(L1, t, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("setglobal") {
      ଅଚଳ ଅକ୍ଷର *s = getstring;
      lua_setglobal(L1, s);
    }
    ଅନ୍ୟଥା ଯଦି EQ("sethook") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = getnum;
      ଅଚଳ ଅକ୍ଷର *s = getstring;
      sethookaux(L1, mask, ଗଣନା_2, s);
    }
    ଅନ୍ୟଥା ଯଦି EQ("setmetatable") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx = getindex;
      lua_setmetatable(L1, idx);
    }
    ଅନ୍ୟଥା ଯଦି EQ("settable") {
      lua_settable(L1, getindex);
    }
    ଅନ୍ୟଥା ଯଦି EQ("settop") {
      lua_settop(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("testudata") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ଖାଲି);
    }
    ଅନ୍ୟଥା ଯଦି EQ("error") {
      lua_error(L1);
    }
    ଅନ୍ୟଥା ଯଦି EQ("abort") {
      abort();
    }
    ଅନ୍ୟଥା ଯଦି EQ("throw") {
#ଯଦି defined(__cplusplus)
ସ୍ଥିର ସଂରଚନା X { ପୂର୍ଣ୍ଣସଂଖ୍ୟା x; } x;
      throw x;
#ଅନ୍ୟଥା
      luaL_error(L1, "C++");
#endif
      ଭାଙ୍ଗ;
    }
    ଅନ୍ୟଥା ଯଦି EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    ଅନ୍ୟଥା ଯଦି EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    ଅନ୍ୟଥା ଯଦି EQ("tostring") {
      ଅଚଳ ଅକ୍ଷର *s = lua_tostring(L1, getindex);
      ଅଚଳ ଅକ୍ଷର *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ଖାଲି && s1 == ଖାଲି) || strcmp(s, s1) == 0);
    }
    ଅନ୍ୟଥା ଯଦି EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ଖାଲି);
    }
    ଅନ୍ୟଥା ଯଦି EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    ଅନ୍ୟଥା ଯଦି EQ("xmove") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା f = getindex;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = getnum;
      ଯଦି (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    ଅନ୍ୟଥା ଯଦି EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    ଅନ୍ୟଥା ଯଦି EQ("yield") {
      ଫେରନ୍ତୁ lua_yield(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("yieldk") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nres = getnum;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = getindex;
      ଫେରନ୍ତୁ lua_yieldk(L1, nres, i, Cfunck);
    }
    ଅନ୍ୟଥା ଯଦି EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    ଅନ୍ୟଥା ଯଦି EQ("argerror") {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    ଅନ୍ୟଥା luaL_error(L, "unknown instruction %s", buff);
  }
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା testC (lua_State *L) {
  lua_State *L1;
  ଅଚଳ ଅକ୍ଷର *pc;
  ଯଦି (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  ଅନ୍ୟଥା ଯଦି (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  ଅନ୍ୟଥା {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ଫେରନ୍ତୁ runC(L, L1, pc);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା Cfunc (lua_State *L) {
  ଫେରନ୍ତୁ runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା Cfunck (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ଫେରନ୍ତୁ runC(L, L, lua_tostring(L, cast_int(ctx)));
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ଫେରନ୍ତୁ 1;
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
ସ୍ଥିର ଶୂନ୍ୟ Chook (lua_State *L, lua_Debug *ar) {
  ଅଚଳ ଅକ୍ଷର *scpt;
  ଅଚଳ ଅକ୍ଷର *ଅଚଳ events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_କାଢ଼(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
ସ୍ଥିର ଶୂନ୍ୟ sethookaux (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2, ଅଚଳ ଅକ୍ଷର *scpt) {
  ଯଦି (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ଖାଲି, 0, 0);  /* turn off hooks */
    ଫେରନ୍ତୁ;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ଯଦି (!lua_istable(L, -1)) {  /* no hook table? */
    lua_କାଢ଼(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, ଗଣନା_2);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sethook (lua_State *L) {
  ଯଦି (lua_isnoneornil(L, 1))
    lua_sethook(L, ଖାଲି, 0, 0);  /* turn off hooks */
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *scpt = luaL_checkstring(L, 1);
    ଅଚଳ ଅକ୍ଷର *smask = luaL_checkstring(L, 2);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = cast_int(luaL_optinteger(L, 3, 0));
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = 0;
    ଯଦି (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    ଯଦି (strchr(smask, 'r')) mask |= LUA_MASKRET;
    ଯଦି (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    ଯଦି (ଗଣନା_2 > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, ଗଣନା_2, scpt);
  }
  ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା coresume (lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  ଯଦି (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_ଭର୍ତ୍ତି(L, -2);
    ଫେରନ୍ତୁ 2;  /* return false + error message */
  }
  ଅନ୍ୟଥା {
    lua_pushboolean(L, 1);
    ଫେରନ୍ତୁ 1;
  }
}

#ଯଦି !defined(LUA_USE_POSIX)

#define nonblock	ଖାଲି

#ଅନ୍ୟଥା

#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = fileno(f);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ଫେରନ୍ତୁ 0;
}
#endif

/* }====================================================== */



ସ୍ଥିର ଅଚଳ ସଂରଚନା luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_ଗଣନା},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {ଖାଲି, ଖାଲି}
};


ସ୍ଥିର ଶୂନ୍ୟ checkfinalmem (ଶୂନ୍ୟ) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaB_opentests (lua_State *L) {
  ଶୂନ୍ୟ *ud;
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
  ଫେରନ୍ତୁ 1;
}

#endif

