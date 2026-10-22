/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <setjmp.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lapi.h"
#அந்தர்பாவ "lauxlib.h"
#அந்தர்பாவ "lcode.h"
#அந்தர்பாவ "lctype.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lopcodes.h"
#அந்தர்பாவ "lopnames.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#எனில் defined(LUA_DEBUG)


வெறுமை *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


நிலையான முழு runC (lua_State *L, lua_State *L1, மாறிலி எழுத்து *pc);


நிலையான வெறுமை setnameval (lua_State *L, மாறிலி எழுத்து *name, முழு val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


நிலையான வெறுமை pushobject (lua_State *L, மாறிலி TValue *o) {
  setobj2s(L, L->உச்சி.p, o);
  api_incr_உச்சி(L);
}


நிலையான வெறுமை badexit (மாறிலி எழுத்து *fmt, மாறிலி எழுத்து *s1, மாறிலி எழுத்து *s2) {
  fprintf(stderr, fmt, s1);
  எனில் (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


நிலையான முழு tpanic (lua_State *L) {
  மாறிலி எழுத்து *செய்தி = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  திரும்பு (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   செய்தி, வெற்று),
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
நிலையான வெறுமை warnf (வெறுமை *ud, மாறிலி எழுத்து *செய்தி, முழு tocont) {
  lua_State *L = cast(lua_State *, ud);
  நிலையான எழுத்து buff[200] = "";  /* should be enough for tests... */
  நிலையான முழு onoff = 0;
  நிலையான முழு mode = 0;  /* start in normal mode */
  நிலையான முழு lasttocont = 0;
  எனில் (!lasttocont && !tocont && *செய்தி == '@') {  /* control message? */
    எனில் (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", செய்தி, buff);
    எனில் (strcmp(செய்தி, "@off") == 0)
      onoff = 0;
    இல்லையெனில் எனில் (strcmp(செய்தி, "@on") == 0)
      onoff = 1;
    இல்லையெனில் எனில் (strcmp(செய்தி, "@normal") == 0)
      mode = 0;
    இல்லையெனில் எனில் (strcmp(செய்தி, "@allow") == 0)
      mode = 1;
    இல்லையெனில் எனில் (strcmp(செய்தி, "@store") == 0)
      mode = 2;
    இல்லையெனில்
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              செய்தி, வெற்று);
    திரும்பு;
  }
  lasttocont = tocont;
  எனில் (strlen(செய்தி) >= அளவு(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", செய்தி, buff);
  strcat(buff, செய்தி);  /* add new message to current warning */
  எனில் (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    எனில் (!lua_toboolean(L, -1))
      lua_எடு(L, 1);  /* ok, no previous unexpected warning */
    இல்லையெனில் {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    தேர்வு (mode) {
      நிலை 0: {  /* normal */
        எனில் (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, வெற்று);
      }  /* FALLTHROUGH */
      நிலை 1: {  /* allow */
        எனில் (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        நிறுத்து;
      }
      நிலை 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        நிறுத்து;
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

வகைவரை ஒன்றிப்பு memHeader {
  LUAI_MAXALIGN;
  கட்டமைப்பு {
    size_t size;
    முழு type;
  } d;
} memHeader;


#எனில் !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#இல்லையெனில்

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


நிலையான வெறுமை freeblock (Memcontrol *mc, memHeader *block) {
  எனில் (block) {
    size_t size = block->d.size;
    முழு i;
    க்கு (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, அளவு(memHeader) + size + MARKSIZE);  /* erase block */
    விடுவி(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


வெறுமை *debug_realloc (வெறுமை *ud, வெறுமை *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  முழு type;
  எனில் (mc->memlimit == 0) {  /* first time? */
    எழுத்து *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, வெற்று, 10) : ULONG_MAX;
  }
  எனில் (block == வெற்று) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  இல்லையெனில் {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  எனில் (size == 0) {
    freeblock(mc, block);
    திரும்பு வெற்று;
  }
  எனில் (mc->failnext) {
    mc->failnext = 0;
    திரும்பு வெற்று;  /* fake a single memory allocation error */
  }
  எனில் (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    எனில் (mc->countlimit == 0)
      திரும்பு வெற்று;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  எனில் (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    திரும்பு வெற்று;  /* fake a memory allocation error */
  இல்லையெனில் {
    memHeader *newblock;
    முழு i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = அளவு(memHeader) + size + MARKSIZE;
    எனில் (realsize < size) திரும்பு வெற்று;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    எனில் (newblock == வெற்று)
      திரும்பு வெற்று;  /* really out of memory? */
    எனில் (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    க்கு (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    எனில் (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    திரும்பு newblock + 1;
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

#அந்தர்பாவ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
நிலையான முழு testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  எனில் (isdead(g,t)) திரும்பு 0;
  எனில் (issweepphase(g))
    திரும்பு 1;  /* no invariants */
  இல்லையெனில் எனில் (g->gckind != KGC_GENMINOR)
    திரும்பு !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  இல்லையெனில் {  /* generational mode */
    எனில் ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      திரும்பு 0;
    எனில் ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      திரும்பு 0;
    திரும்பு 1;
  }
}


நிலையான வெறுமை printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (வெறுமை *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  எனில் (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


வெறுமை lua_printobj (lua_State *L, கட்டமைப்பு GCObject *o) {
  printobj(G(L), o);
}


வெறுமை lua_printvalue (TValue *v) {
  தேர்வு (ttypetag(v)) {
    நிலை LUA_VNUMINT: நிலை LUA_VNUMFLT: {
      எழுத்து buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      நிறுத்து;
    }
    நிலை LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); நிறுத்து;
    நிலை LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); நிறுத்து;
    நிலை LUA_VFALSE:
      printf("%s", "false"); நிறுத்து;
    நிலை LUA_VTRUE:
      printf("%s", "true"); நிறுத்து;
    நிலை LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); நிறுத்து;
    நிலை LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); நிறுத்து;
    நிலை LUA_VNIL:
      printf("nil"); நிறுத்து;
    நிலை LUA_VLCF:
      printf("light C function: %p", fvalue(v)); நிறுத்து;
    நிலை LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); நிறுத்து;
    நிலை LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); நிறுத்து;
    நிலை LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); நிறுத்து;
    நிலை LUA_VTABLE:
      printf("table: %p", hvalue(v)); நிறுத்து;
    இயல்பு:
      lua_assert(0);
  }
}


நிலையான முழு testobjref (global_State *g, GCObject *f, GCObject *t) {
  முழு r1 = testobjref1(g, f, t);
  எனில் (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  திரும்பு r1;
}


நிலையான வெறுமை checkobjref (global_State *g, GCObject *f, GCObject *t) {
    ப்ரதிஜ்ஞா(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ எனில் (t) checkobjref(g,f,obj2gco(t)); }


நிலையான வெறுமை checkvalref (global_State *g, GCObject *f, மாறிலி TValue *t) {
  ப்ரதிஜ்ஞா(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


நிலையான வெறுமை checktable (global_State *g, Table *h) {
  unsigned முழு i;
  unsigned முழு asize = h->asize;
  முனை_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  க்கு (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  க்கு (n = gnode(h, 0); n < limit; n++) {
    எனில் (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      ப்ரதிஜ்ஞா(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


நிலையான வெறுமை checkudata (global_State *g, Udata *u) {
  முழு i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  க்கு (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


நிலையான வெறுமை checkproto (global_State *g, Proto *f) {
  முழு i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  க்கு (i=0; i<f->sizek; i++) {
    எனில் (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  க்கு (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  க்கு (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  க்கு (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


நிலையான வெறுமை checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  முழு i;
  க்கு (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


நிலையான வெறுமை checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  முழு i;
  checkobjrefN(g, clgc, cl->p);
  க்கு (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    எனில் (uv) {
      checkobjrefN(g, clgc, uv);
      எனில் (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


நிலையான முழு lua_checkpc (CallInfo *ci) {
  எனில் (!isLua(ci)) திரும்பு 1;
  இல்லையெனில் {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    திரும்பு p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


நிலையான வெறுமை check_அடுக்கு (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  ப்ரதிஜ்ஞா(!isdead(g, L1));
  எனில் (L1->அடுக்கு.p == வெற்று) {  /* incomplete thread? */
    ப்ரதிஜ்ஞா(L1->openupval == வெற்று && L1->ci == வெற்று);
    திரும்பு;
  }
  க்கு (uv = L1->openupval; uv != வெற்று; uv = uv->u.open.next)
    ப்ரதிஜ்ஞா(upisopen(uv));  /* must be open */
  ப்ரதிஜ்ஞா(L1->உச்சி.p <= L1->அடுக்கு_last.p);
  ப்ரதிஜ்ஞா(L1->tbclist.p <= L1->உச்சி.p);
  க்கு (ci = L1->ci; ci != வெற்று; ci = ci->previous) {
    ப்ரதிஜ்ஞா(ci->உச்சி.p <= L1->அடுக்கு_last.p);
    ப்ரதிஜ்ஞா(lua_checkpc(ci));
  }
  க்கு (o = L1->அடுக்கு.p; o < L1->அடுக்கு_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


நிலையான வெறுமை checkrefs (global_State *g, GCObject *o) {
  தேர்வு (o->tt) {
    நிலை LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      நிறுத்து;
    }
    நிலை LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      நிறுத்து;
    }
    நிலை LUA_VTABLE: {
      checktable(g, gco2t(o));
      நிறுத்து;
    }
    நிலை LUA_VTHREAD: {
      check_அடுக்கு(g, gco2th(o));
      நிறுத்து;
    }
    நிலை LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      நிறுத்து;
    }
    நிலை LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      நிறுத்து;
    }
    நிலை LUA_VPROTO: {
      checkproto(g, gco2p(o));
      நிறுத்து;
    }
    நிலை LUA_VSHRSTR:
    நிலை LUA_VLNGSTR: {
      ப்ரதிஜ்ஞா(!isgray(o));  /* strings are never gray */
      நிறுத்து;
    }
    இயல்பு: ப்ரதிஜ்ஞா(0);
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
நிலையான வெறுமை checkobject (global_State *g, GCObject *o, முழு maybedead,
                         முழு listage) {
  எனில் (isdead(g, o))
    ப்ரதிஜ்ஞா(maybedead);
  இல்லையெனில் {
    ப்ரதிஜ்ஞா(g->gcstate != GCSpause || iswhite(o));
    எனில் (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      ப்ரதிஜ்ஞா(getage(o) >= listage);
      எனில் (isold(o)) {
        ப்ரதிஜ்ஞா(!iswhite(o));
        ப்ரதிஜ்ஞா(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      ப்ரதிஜ்ஞா(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


நிலையான l_mem checkgraylist (global_State *g, GCObject *o) {
  முழு total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  போது (o) {
    ப்ரதிஜ்ஞா(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    ப்ரதிஜ்ஞா(!testbit(o->marked, TESTBIT));
    எனில் (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    தேர்வு (o->tt) {
      நிலை LUA_VTABLE: o = gco2t(o)->gclist; நிறுத்து;
      நிலை LUA_VLCL: o = gco2lcl(o)->gclist; நிறுத்து;
      நிலை LUA_VCCL: o = gco2ccl(o)->gclist; நிறுத்து;
      நிலை LUA_VTHREAD: o = gco2th(o)->gclist; நிறுத்து;
      நிலை LUA_VPROTO: o = gco2p(o)->gclist; நிறுத்து;
      நிலை LUA_VUSERDATA:
        ப்ரதிஜ்ஞா(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        நிறுத்து;
      இயல்பு: ப்ரதிஜ்ஞா(0);  /* other objects cannot be in a gray list */
    }
  }
  திரும்பு total;
}


/*
** Check objects in gray lists.
*/
நிலையான l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  எனில் (!keepinvariant(g)) திரும்பு total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  திரும்பு total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
நிலையான வெறுமை incifingray (global_State *g, GCObject *o, l_mem *மொத்தம்) {
  எனில் (!keepinvariant(g))
    திரும்பு;  /* gray lists not being kept in these phases */
  எனில் (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    ப்ரதிஜ்ஞா(!isgray(o) || upisopen(gco2upv(o)));
    திரும்பு;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  எனில் (isgray(o) || getage(o) == G_TOUCHED2) {
    (*மொத்தம்)++;
    ப்ரதிஜ்ஞா(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


நிலையான l_mem checklist (global_State *g, முழு maybedead, முழு tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  க்கு (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    ப்ரதிஜ்ஞா(!tof == !tofinalize(o));
  }
  க்கு (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    ப்ரதிஜ்ஞா(!tof == !tofinalize(o));
  }
  க்கு (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    ப்ரதிஜ்ஞா(!tof == !tofinalize(o));
  }
  க்கு (o = reallyold; o != வெற்று; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    ப்ரதிஜ்ஞா(!tof == !tofinalize(o));
  }
  திரும்பு total;
}


முழு lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  முழு maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  எனில் (keepinvariant(g)) {
    ப்ரதிஜ்ஞா(!iswhite(mainthread(g)));
    ப்ரதிஜ்ஞா(!iswhite(gcvalue(&g->l_registry)));
  }
  ப்ரதிஜ்ஞா(!isdead(g, gcvalue(&g->l_registry)));
  ப்ரதிஜ்ஞா(g->sweepgc == வெற்று || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  க்கு (o = g->fixedgc; o != வெற்று; o = o->next) {
    ப்ரதிஜ்ஞா(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  க்கு (o = g->tobefnz; o != வெற்று; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    ப்ரதிஜ்ஞா(tofinalize(o));
    ப்ரதிஜ்ஞா(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  எனில் (keepinvariant(g))
    ப்ரதிஜ்ஞா(totalin == totalshould);
  திரும்பு 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


நிலையான எழுத்து *buildop (Proto *p, முழு pc, எழுத்து *buff) {
  எழுத்து *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  மாறிலி எழுத்து *name = opnames[o];
  முழு வரி = luaG_getfuncline(p, pc);
  முழு lineinfo = (p->lineinfo != வெற்று) ? p->lineinfo[pc] : 0;
  எனில் (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  இல்லையெனில்
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", வரி, pc);
  தேர்வு (getOpMode(o)) {
    நிலை iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      நிறுத்து;
    நிலை ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      நிறுத்து;
    நிலை iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      நிறுத்து;
    நிலை iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      நிறுத்து;
    நிலை iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      நிறுத்து;
    நிலை isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      நிறுத்து;
  }
  திரும்பு obuff;
}


#எனில் 0
வெறுமை luaI_printcode (Proto *pt, முழு size) {
  முழு pc;
  க்கு (pc=0; pc<size; pc++) {
    எழுத்து buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


வெறுமை luaI_printinst (Proto *pt, முழு pc) {
  எழுத்து buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


நிலையான முழு listcode (lua_State *L) {
  முழு pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  க்கு (pc=0; pc<p->sizecode; pc++) {
    எழுத்து buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  திரும்பு 1;
}


நிலையான முழு printcode (lua_State *L) {
  முழு pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  க்கு (pc=0; pc<p->sizecode; pc++) {
    எழுத்து buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  திரும்பு 0;
}


நிலையான முழு listk (lua_State *L) {
  Proto *p;
  முழு i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  க்கு (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  திரும்பு 1;
}


நிலையான முழு listabslineinfo (lua_State *L) {
  Proto *p;
  முழு i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != வெற்று, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  க்கு (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].வரி);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  திரும்பு 1;
}


நிலையான முழு listlocals (lua_State *L) {
  Proto *p;
  முழு pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  முழு i = 0;
  மாறிலி எழுத்து *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  போது ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != வெற்று)
    lua_pushstring(L, name);
  திரும்பு i-1;
}

/* }====================================================== */



வெறுமை lua_printstack (lua_State *L) {
  முழு i;
  முழு n = lua_gettop(L);
  printf("stack: >>\n");
  க்கு (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


முழு lua_printallstack (lua_State *L) {
  StkId p;
  முழு i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  க்கு (p = L->அடுக்கு.p; p < L->உச்சி.p; p++) {
    எனில் (ci != வெற்று && p == ci->func.p) {
      printf("  ---\n");
      எனில் (ci == L->ci)
        ci = வெற்று;  /* printed last frame */
      இல்லையெனில்
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  திரும்பு 0;
}


நிலையான முழு get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", இலக்கம்_OPCODES);
  திரும்பு 1;
}


நிலையான முழு get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", அளவு(lua_State));
  setnameval(L, "global state", அளவு(global_State));
  setnameval(L, "TValue", அளவு(TValue));
  setnameval(L, "Node", அளவு(முனை_2));
  setnameval(L, "stack Value", அளவு(அடுக்கு_மதிப்பு));
  திரும்பு 1;
}


நிலையான முழு mem_query (lua_State *L) {
  எனில் (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    திரும்பு 3;
  }
  இல்லையெனில் எனில் (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    எனில் (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    திரும்பு 0;
  }
  இல்லையெனில் {
    மாறிலி எழுத்து *t = luaL_checkstring(L, 1);
    முழு i;
    க்கு (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      எனில் (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        திரும்பு 1;
      }
    }
    திரும்பு luaL_error(L, "unknown type '%s'", t);
  }
}


நிலையான முழு alloc_மொத்தம் (lua_State *L) {
  எனில் (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  இல்லையெனில்
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  திரும்பு 0;
}


நிலையான முழு alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  திரும்பு 0;
}


நிலையான முழு settrick (lua_State *L) {
  எனில் (ttisnil(obj_at(L, 1)))
    l_Trick = வெற்று;
  இல்லையெனில்
    l_Trick = gcvalue(obj_at(L, 1));
  திரும்பு 0;
}


நிலையான முழு gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  எனில் (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  இல்லையெனில் {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  திரும்பு 1;
}


நிலையான முழு gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  எனில் (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  இல்லையெனில் {
    நிலையான மாறிலி எழுத்து *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  திரும்பு 1;
}


நிலையான முழு gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  எனில் (!iscollectable(o))
    printf("no collectable\n");
  இல்லையெனில் {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  திரும்பு 0;
}


நிலையான மாறிலி எழுத்து *மாறிலி statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

நிலையான முழு gc_state (lua_State *L) {
  நிலையான மாறிலி முழு states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  முழு option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  எனில் (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    திரும்பு 1;
  }
  இல்லையெனில் {
    எனில் (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    எனில் (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    திரும்பு 0;
  }
}


நிலையான முழு tracinggc = 0;
வெறுமை luai_tracegctest (lua_State *L, முழு first) {
  எனில் (!tracinggc) திரும்பு;
  இல்லையெனில் {
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


நிலையான முழு tracegc (lua_State *L) {
  எனில் (lua_isnil(L, 1))
    tracinggc = 0;
  இல்லையெனில் {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  திரும்பு 0;
}


நிலையான முழு hash_query (lua_State *L) {
  எனில் (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    எனில் (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  இல்லையெனில் {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->முனை));
  }
  திரும்பு 1;
}


நிலையான முழு stacklevel (lua_State *L) {
  முழு a = 0;
  lua_pushinteger(L, cast_Integer(L->உச்சி.p - L->அடுக்கு.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  திரும்பு 5;
}


நிலையான முழு resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  போது (ci->next != வெற்று) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  திரும்பு 0;
}


நிலையான முழு reallocstack (lua_State *L) {
  முழு n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->உச்சி.p - L->அடுக்கு.p) + n, 1);
  lua_unlock(L);
  திரும்பு 0;
}


நிலையான முழு table_query (lua_State *L) {
  மாறிலி Table *t;
  முழு i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned முழு asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  எனில் (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    திரும்பு 3;
  }
  இல்லையெனில் எனில் (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    எனில் (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->உச்சி.p));
    இல்லையெனில்
      setnilvalue2s(L->உச்சி.p);
    api_incr_உச்சி(L);
    lua_pushnil(L);
  }
  இல்லையெனில் எனில் (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    எனில் (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    இல்லையெனில்
      lua_pushliteral(L, "<undef>");
    எனில் (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    இல்லையெனில்
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->முனை[i]));
  }
  திரும்பு 3;
}


நிலையான முழு gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  திரும்பு 6;
}


நிலையான முழு test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  திரும்பு 1;
}


நிலையான முழு test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  திரும்பு 1;
}


நிலையான முழு string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  முழு s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  எனில் (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    திரும்பு 2;
  }
  இல்லையெனில் எனில் (s < tb->size) {
    TString *ts;
    முழு n = 0;
    க்கு (ts = tb->hash[s]; ts != வெற்று; ts = ts->u.hnext) {
      setsvalue2s(L, L->உச்சி.p, ts);
      api_incr_உச்சி(L);
      n++;
    }
    திரும்பு n;
  }
  இல்லையெனில் திரும்பு 0;
}


நிலையான முழு getreftable (lua_State *L) {
  எனில் (lua_istable(L, 2))  /* is there a table as second argument? */
    திரும்பு 2;  /* use it as the table */
  இல்லையெனில்
    திரும்பு LUA_REGISTRYINDEX;  /* default is to use the register */
}


நிலையான முழு tref (lua_State *L) {
  முழு t = getreftable(L);
  முழு level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  திரும்பு 1;
}


நிலையான முழு getref (lua_State *L) {
  முழு t = getreftable(L);
  முழு level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  திரும்பு 1;
}

நிலையான முழு unref (lua_State *L) {
  முழு t = getreftable(L);
  முழு level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  திரும்பு 0;
}


நிலையான முழு upvalue (lua_State *L) {
  முழு n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  எனில் (lua_isnone(L, 3)) {
    மாறிலி எழுத்து *name = lua_getupvalue(L, 1, n);
    எனில் (name == வெற்று) திரும்பு 0;
    lua_pushstring(L, name);
    திரும்பு 2;
  }
  இல்லையெனில் {
    மாறிலி எழுத்து *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    திரும்பு 1;
  }
}


நிலையான முழு newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  முழு nuv = cast_int(luaL_optinteger(L, 2, 0));
  எழுத்து *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  போது (size--) *p++ = '\0';
  திரும்பு 1;
}


நிலையான முழு pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  திரும்பு 1;
}


நிலையான முழு udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  திரும்பு 1;
}


நிலையான முழு doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &l);
  முழு status = luaL_loadbufferx(L1, s, l, s, "t");
  எனில் (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  திரும்பு 1;
}


நிலையான முழு s2d (lua_State *L) {
  lua_pushnumber(L, cast_இலக்கம்(*cast(மாறிலி இரட்டை *, luaL_checkstring(L, 1))));
  திரும்பு 1;
}


நிலையான முழு d2s (lua_State *L) {
  இரட்டை d = cast(இரட்டை, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), அளவு(d));
  திரும்பு 1;
}


நிலையான முழு num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  திரும்பு 1;
}


நிலையான முழு makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  திரும்பு 1;
}


நிலையான முழு newstate (lua_State *L) {
  வெறுமை *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  எனில் (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  இல்லையெனில்
    lua_pushnil(L);
  திரும்பு 1;
}


நிலையான lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != வெற்று, 1, "state expected");
  திரும்பு L1;
}


நிலையான முழு loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  முழு load = cast_int(luaL_checkinteger(L, 2));
  முழு preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", வெற்று, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  திரும்பு 0;
}

நிலையான முழு closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  திரும்பு 0;
}

நிலையான முழு doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  மாறிலி எழுத்து *code = luaL_checklstring(L, 2, &lcode);
  முழு status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  எனில் (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  எனில் (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    திரும்பு 3;
  }
  இல்லையெனில் {
    முழு i = 0;
    போது (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_எடு(L1, i-1);
    திரும்பு i-1;
  }
}


நிலையான முழு log2_aux (lua_State *L) {
  unsigned முழு x = (unsigned முழு)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  திரும்பு 1;
}


கட்டமைப்பு Aux { jmp_buf jb; மாறிலி எழுத்து *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
நிலையான முழு panicback (lua_State *L) {
  கட்டமைப்பு Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (கட்டமைப்பு Aux *)lua_touserdata(L, -1);
  lua_எடு(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  திரும்பு 1;  /* to avoid warnings */
}

நிலையான முழு checkpanic (lua_State *L) {
  கட்டமைப்பு Aux b;
  வெறுமை *ud;
  lua_State *L1;
  மாறிலி எழுத்து *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  எனில் (L1 == வெற்று) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    திரும்பு 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  எனில் (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  இல்லையெனில் {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  திரும்பு 1;
}


நிலையான முழு externKstr (lua_State *L) {
  size_t len;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, வெற்று, வெற்று);
  திரும்பு 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
நிலையான முழு externstr (lua_State *L) {
  size_t len;
  மாறிலி எழுத்து *s = luaL_checklstring(L, 1, &len);
  வெறுமை *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  எழுத்து *buff = cast_charp((*allocf)(ud, வெற்று, 0, len + 1));
  எனில் (buff == வெற்று) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * அளவு(எழுத்து));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  திரும்பு 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


நிலையான வெறுமை sethookaux (lua_State *L, முழு mask, முழு மொத்தம், மாறிலி எழுத்து *code);

நிலையான மாறிலி எழுத்து *மாறிலி delimits = " \t\n,;";

நிலையான வெறுமை skip (மாறிலி எழுத்து **pc) {
  க்கு (;;) {
    எனில் (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    இல்லையெனில் எனில் (**pc == '#') {  /* comment? */
      போது (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    இல்லையெனில் நிறுத்து;
  }
}

நிலையான முழு getnum_aux (lua_State *L, lua_State *L1, மாறிலி எழுத்து **pc) {
  முழு res = 0;
  முழு sig = 1;
  skip(pc);
  எனில் (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_எடு(L1, 1);
    (*pc)++;
    திரும்பு res;
  }
  இல்லையெனில் எனில் (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    திரும்பு res;
  }
  இல்லையெனில் எனில் (**pc == '!') {
    (*pc)++;
    எனில் (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    இல்லையெனில் எனில் (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    இல்லையெனில் lua_assert(0);
    (*pc)++;
    திரும்பு res;
  }
  இல்லையெனில் எனில் (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  எனில் (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  போது (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  திரும்பு sig*res;
}

நிலையான மாறிலி எழுத்து *getstring_aux (lua_State *L, எழுத்து *buff, மாறிலி எழுத்து **pc) {
  முழு i = 0;
  skip(pc);
  எனில் (**pc == '"' || **pc == '\'') {  /* quoted string? */
    முழு quote = *(*pc)++;
    போது (**pc != quote) {
      எனில் (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  இல்லையெனில் {
    போது (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  திரும்பு buff;
}


நிலையான முழு getindex_aux (lua_State *L, lua_State *L1, மாறிலி எழுத்து **pc) {
  skip(pc);
  தேர்வு (*(*pc)++) {
    நிலை 'R': திரும்பு LUA_REGISTRYINDEX;
    நிலை 'U': திரும்பு lua_upvalueindex(getnum_aux(L, L1, pc));
    இயல்பு: {
      முழு n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      எனில் (n == 0) திரும்பு 0;
      இல்லையெனில் திரும்பு lua_absindex(L1, n);
    }
  }
}


நிலையான மாறிலி எழுத்து *மாறிலி statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
நிலையான வெறுமை regcodes (lua_State *L) {
  unsigned முழு i;
  க்கு (i = 0; i < அளவு(statcodes) / அளவு(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


நிலையான முழு testC (lua_State *L);
நிலையான முழு Cfunck (lua_State *L, முழு status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
நிலையான மாறிலி எழுத்து ops[] = "+-*%^/\\&|~<>_!";

நிலையான முழு runC (lua_State *L, lua_State *L1, மாறிலி எழுத்து *pc) {
  எழுத்து buff[300];
  முழு status = 0;
  எனில் (pc == வெற்று) திரும்பு luaL_error(L, "attempt to runC null script");
  க்கு (;;) {
    மாறிலி எழுத்து *inst = getstring;
    எனில் EQ("") திரும்பு 0;
    இல்லையெனில் எனில் EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    இல்லையெனில் எனில் EQ("append") {
      முழு t = getindex;
      முழு i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    இல்லையெனில் எனில் EQ("arith") {
      முழு op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    இல்லையெனில் எனில் EQ("call") {
      முழு narg = getnum;
      முழு nres = getnum;
      lua_call(L1, narg, nres);
    }
    இல்லையெனில் எனில் EQ("callk") {
      முழு narg = getnum;
      முழு nres = getnum;
      முழு i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    இல்லையெனில் எனில் EQ("checkstack") {
      முழு sz = getnum;
      மாறிலி எழுத்து *செய்தி = getstring;
      எனில் (*செய்தி == '\0')
        செய்தி = வெற்று;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, செய்தி);
    }
    இல்லையெனில் எனில் EQ("rawcheckstack") {
      முழு sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    இல்லையெனில் எனில் EQ("compare") {
      மாறிலி எழுத்து *opt = getstring;  /* EQ, LT, or LE */
      முழு op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      முழு a = getindex;
      முழு b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    இல்லையெனில் எனில் EQ("concat") {
      lua_concat(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("copy") {
      முழு f = getindex;
      lua_copy(L1, f, getindex);
    }
    இல்லையெனில் எனில் EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    இல்லையெனில் எனில் EQ("getfield") {
      முழு t = getindex;
      முழு tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    இல்லையெனில் எனில் EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    இல்லையெனில் எனில் EQ("getmetatable") {
      எனில் (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    இல்லையெனில் எனில் EQ("gettable") {
      முழு tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    இல்லையெனில் எனில் EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    இல்லையெனில் எனில் EQ("gsub") {
      முழு a = getnum; முழு b = getnum; முழு c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    இல்லையெனில் எனில் EQ("insert") {
      lua_செருகு(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("len") {
      lua_len(L1, getindex);
    }
    இல்லையெனில் எனில் EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    இல்லையெனில் எனில் EQ("loadstring") {
      size_t slen;
      மாறிலி எழுத்து *s = luaL_checklstring(L1, getnum, &slen);
      மாறிலி எழுத்து *name = getstring;
      மாறிலி எழுத்து *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    இல்லையெனில் எனில் EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    இல்லையெனில் எனில் EQ("newtable") {
      lua_newtable(L1);
    }
    இல்லையெனில் எனில் EQ("newthread") {
      lua_newthread(L1);
    }
    இல்லையெனில் எனில் EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    இல்லையெனில் எனில் EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    இல்லையெனில் எனில் EQ("next") {
      lua_next(L1, -2);
    }
    இல்லையெனில் எனில் EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    இல்லையெனில் எனில் EQ("pcall") {
      முழு narg = getnum;
      முழு nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    இல்லையெனில் எனில் EQ("pcallk") {
      முழு narg = getnum;
      முழு nres = getnum;
      முழு i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    இல்லையெனில் எனில் EQ("pop") {
      lua_எடு(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("printstack") {
      முழு n = getnum;
      எனில் (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      இல்லையெனில் lua_printstack(L1);
    }
    இல்லையெனில் எனில் EQ("print") {
      மாறிலி எழுத்து *செய்தி = getstring;
      printf("%s\n", செய்தி);
    }
    இல்லையெனில் எனில் EQ("warningC") {
      மாறிலி எழுத்து *செய்தி = getstring;
      lua_warning(L1, செய்தி, 1);
    }
    இல்லையெனில் எனில் EQ("warning") {
      மாறிலி எழுத்து *செய்தி = getstring;
      lua_warning(L1, செய்தி, 0);
    }
    இல்லையெனில் எனில் EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    இல்லையெனில் எனில் EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("pushnil") {
      lua_pushnil(L1);
    }
    இல்லையெனில் எனில் EQ("pushnum") {
      lua_pushnumber(L1, (lua_எண்)getnum);
    }
    இல்லையெனில் எனில் EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    இல்லையெனில் எனில் EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    இல்லையெனில் எனில் EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    இல்லையெனில் எனில் EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    இல்லையெனில் எனில் EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (முழு)lua_tointeger(L, -1));
    }
    இல்லையெனில் எனில் EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    இல்லையெனில் எனில் EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    இல்லையெனில் எனில் EQ("rawget") {
      முழு t = getindex;
      lua_rawget(L1, t);
    }
    இல்லையெனில் எனில் EQ("rawgeti") {
      முழு t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    இல்லையெனில் எனில் EQ("rawgetp") {
      முழு t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    இல்லையெனில் எனில் EQ("rawset") {
      முழு t = getindex;
      lua_rawset(L1, t);
    }
    இல்லையெனில் எனில் EQ("rawseti") {
      முழு t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    இல்லையெனில் எனில் EQ("rawsetp") {
      முழு t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    இல்லையெனில் எனில் EQ("remove") {
      lua_remove(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("replace") {
      lua_replace(L1, getindex);
    }
    இல்லையெனில் எனில் EQ("resume") {
      முழு i = getindex;
      முழு nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    இல்லையெனில் எனில் EQ("traceback") {
      மாறிலி எழுத்து *செய்தி = getstring;
      முழு level = getnum;
      luaL_traceback(L1, L1, செய்தி, level);
    }
    இல்லையெனில் எனில் EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    இல்லையெனில் எனில் EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    இல்லையெனில் எனில் EQ("return") {
      முழு n = getnum;
      எனில் (L1 != L) {
        முழு i;
        க்கு (i = 0; i < n; i++) {
          முழு idx = -(n - i);
          தேர்வு (lua_type(L1, idx)) {
            நிலை LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              நிறுத்து;
            இயல்பு:
              lua_pushstring(L, lua_tostring(L1, idx));
              நிறுத்து;
          }
        }
      }
      திரும்பு n;
    }
    இல்லையெனில் எனில் EQ("rotate") {
      முழு i = getindex;
      lua_rotate(L1, i, getnum);
    }
    இல்லையெனில் எனில் EQ("setfield") {
      முழு t = getindex;
      மாறிலி எழுத்து *s = getstring;
      lua_setfield(L1, t, s);
    }
    இல்லையெனில் எனில் EQ("seti") {
      முழு t = getindex;
      lua_seti(L1, t, getnum);
    }
    இல்லையெனில் எனில் EQ("setglobal") {
      மாறிலி எழுத்து *s = getstring;
      lua_setglobal(L1, s);
    }
    இல்லையெனில் எனில் EQ("sethook") {
      முழு mask = getnum;
      முழு மொத்தம் = getnum;
      மாறிலி எழுத்து *s = getstring;
      sethookaux(L1, mask, மொத்தம், s);
    }
    இல்லையெனில் எனில் EQ("setmetatable") {
      முழு idx = getindex;
      lua_setmetatable(L1, idx);
    }
    இல்லையெனில் எனில் EQ("settable") {
      lua_settable(L1, getindex);
    }
    இல்லையெனில் எனில் EQ("settop") {
      lua_settop(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("testudata") {
      முழு i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != வெற்று);
    }
    இல்லையெனில் எனில் EQ("error") {
      lua_error(L1);
    }
    இல்லையெனில் எனில் EQ("abort") {
      abort();
    }
    இல்லையெனில் எனில் EQ("throw") {
#எனில் defined(__cplusplus)
நிலையான கட்டமைப்பு X { முழு x; } x;
      throw x;
#இல்லையெனில்
      luaL_error(L1, "C++");
#endif
      நிறுத்து;
    }
    இல்லையெனில் எனில் EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    இல்லையெனில் எனில் EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    இல்லையெனில் எனில் EQ("tostring") {
      மாறிலி எழுத்து *s = lua_tostring(L1, getindex);
      மாறிலி எழுத்து *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == வெற்று && s1 == வெற்று) || strcmp(s, s1) == 0);
    }
    இல்லையெனில் எனில் EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, வெற்று);
    }
    இல்லையெனில் எனில் EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    இல்லையெனில் எனில் EQ("xmove") {
      முழு f = getindex;
      முழு t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      முழு n = getnum;
      எனில் (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    இல்லையெனில் எனில் EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    இல்லையெனில் எனில் EQ("yield") {
      திரும்பு lua_yield(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("yieldk") {
      முழு nres = getnum;
      முழு i = getindex;
      திரும்பு lua_yieldk(L1, nres, i, Cfunck);
    }
    இல்லையெனில் எனில் EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    இல்லையெனில் எனில் EQ("argerror") {
      முழு arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    இல்லையெனில் luaL_error(L, "unknown instruction %s", buff);
  }
  திரும்பு 0;
}


நிலையான முழு testC (lua_State *L) {
  lua_State *L1;
  மாறிலி எழுத்து *pc;
  எனில் (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  இல்லையெனில் எனில் (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  இல்லையெனில் {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  திரும்பு runC(L, L1, pc);
}


நிலையான முழு Cfunc (lua_State *L) {
  திரும்பு runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


நிலையான முழு Cfunck (lua_State *L, முழு status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  திரும்பு runC(L, L, lua_tostring(L, cast_int(ctx)));
}


நிலையான முழு makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  திரும்பு 1;
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
நிலையான வெறுமை Chook (lua_State *L, lua_Debug *ar) {
  மாறிலி எழுத்து *scpt;
  மாறிலி எழுத்து *மாறிலி events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_எடு(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
நிலையான வெறுமை sethookaux (lua_State *L, முழு mask, முழு மொத்தம், மாறிலி எழுத்து *scpt) {
  எனில் (*scpt == '\0') {  /* no script? */
    lua_sethook(L, வெற்று, 0, 0);  /* turn off hooks */
    திரும்பு;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  எனில் (!lua_istable(L, -1)) {  /* no hook table? */
    lua_எடு(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, மொத்தம்);
}


நிலையான முழு sethook (lua_State *L) {
  எனில் (lua_isnoneornil(L, 1))
    lua_sethook(L, வெற்று, 0, 0);  /* turn off hooks */
  இல்லையெனில் {
    மாறிலி எழுத்து *scpt = luaL_checkstring(L, 1);
    மாறிலி எழுத்து *smask = luaL_checkstring(L, 2);
    முழு மொத்தம் = cast_int(luaL_optinteger(L, 3, 0));
    முழு mask = 0;
    எனில் (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    எனில் (strchr(smask, 'r')) mask |= LUA_MASKRET;
    எனில் (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    எனில் (மொத்தம் > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, மொத்தம், scpt);
  }
  திரும்பு 0;
}


நிலையான முழு coresume (lua_State *L) {
  முழு status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  எனில் (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_செருகு(L, -2);
    திரும்பு 2;  /* return false + error message */
  }
  இல்லையெனில் {
    lua_pushboolean(L, 1);
    திரும்பு 1;
  }
}

#எனில் !defined(LUA_USE_POSIX)

#define nonblock	வெற்று

#இல்லையெனில்

#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <fcntl.h>

நிலையான முழு nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  முழு fd = fileno(f);
  முழு flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  திரும்பு 0;
}
#endif

/* }====================================================== */



நிலையான மாறிலி கட்டமைப்பு luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_மொத்தம்},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {வெற்று, வெற்று}
};


நிலையான வெறுமை checkfinalmem (வெறுமை) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


முழு luaB_opentests (lua_State *L) {
  வெறுமை *ud;
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
  திரும்பு 1;
}

#endif

