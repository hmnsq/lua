/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <setjmp.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "lauxlib.h"
#ಅನ್ತರ್ಭಾವ "lcode.h"
#ಅನ್ತರ್ಭಾವ "lctype.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lopnames.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#ಯದಿ defined(LUA_DEBUG)


ಶೂನ್ಯ *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ runC (lua_State *L, lua_State *L1, ಧ್ರುವ ವರ್ಣ *pc);


ಸ್ಥಿರ ಶೂನ್ಯ setnameval (lua_State *L, ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


ಸ್ಥಿರ ಶೂನ್ಯ pushobject (lua_State *L, ಧ್ರುವ TValue *o) {
  setobj2s(L, L->ತುದಿ.p, o);
  api_incr_ತುದಿ(L);
}


ಸ್ಥಿರ ಶೂನ್ಯ badexit (ಧ್ರುವ ವರ್ಣ *fmt, ಧ್ರುವ ವರ್ಣ *s1, ಧ್ರುವ ವರ್ಣ *s2) {
  fprintf(stderr, fmt, s1);
  ಯದಿ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tpanic (lua_State *L) {
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ಪ್ರತ್ಯಾಗಮ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   ಸಂದೇಶ, ರಿಕ್ತ),
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
ಸ್ಥಿರ ಶೂನ್ಯ warnf (ಶೂನ್ಯ *ud, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ, ಪೂರ್ಣಾಙ್ಕ tocont) {
  lua_State *L = cast(lua_State *, ud);
  ಸ್ಥಿರ ವರ್ಣ buff[200] = "";  /* should be enough for tests... */
  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ onoff = 0;
  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mode = 0;  /* start in normal mode */
  ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ lasttocont = 0;
  ಯದಿ (!lasttocont && !tocont && *ಸಂದೇಶ == '@') {  /* control message? */
    ಯದಿ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", ಸಂದೇಶ, buff);
    ಯದಿ (strcmp(ಸಂದೇಶ, "@off") == 0)
      onoff = 0;
    ಅನ್ಯಥಾ ಯದಿ (strcmp(ಸಂದೇಶ, "@on") == 0)
      onoff = 1;
    ಅನ್ಯಥಾ ಯದಿ (strcmp(ಸಂದೇಶ, "@normal") == 0)
      mode = 0;
    ಅನ್ಯಥಾ ಯದಿ (strcmp(ಸಂದೇಶ, "@allow") == 0)
      mode = 1;
    ಅನ್ಯಥಾ ಯದಿ (strcmp(ಸಂದೇಶ, "@store") == 0)
      mode = 2;
    ಅನ್ಯಥಾ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              ಸಂದೇಶ, ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ;
  }
  lasttocont = tocont;
  ಯದಿ (strlen(ಸಂದೇಶ) >= ಪರಿಮಾಣ(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", ಸಂದೇಶ, buff);
  strcat(buff, ಸಂದೇಶ);  /* add new message to current warning */
  ಯದಿ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ಯದಿ (!lua_toboolean(L, -1))
      lua_ತೆಗೆ(L, 1);  /* ok, no previous unexpected warning */
    ಅನ್ಯಥಾ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    ವಿಕಲ್ಪ (mode) {
      ಪಕ್ಷ 0: {  /* normal */
        ಯದಿ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ರಿಕ್ತ);
      }  /* FALLTHROUGH */
      ಪಕ್ಷ 1: {  /* allow */
        ಯದಿ (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        ಭಙ್ಗ;
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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ memHeader {
  LUAI_MAXALIGN;
  ಸಂರಚನಾ {
    size_t size;
    ಪೂರ್ಣಾಙ್ಕ type;
  } d;
} memHeader;


#ಯದಿ !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#ಅನ್ಯಥಾ

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


ಸ್ಥಿರ ಶೂನ್ಯ freeblock (Memcontrol *mc, memHeader *block) {
  ಯದಿ (block) {
    size_t size = block->d.size;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, ಪರಿಮಾಣ(memHeader) + size + MARKSIZE);  /* erase block */
    ಮುಕ್ತ(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


ಶೂನ್ಯ *debug_realloc (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  ಪೂರ್ಣಾಙ್ಕ type;
  ಯದಿ (mc->memlimit == 0) {  /* first time? */
    ವರ್ಣ *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, ರಿಕ್ತ, 10) : ULONG_MAX;
  }
  ಯದಿ (block == ರಿಕ್ತ) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  ಅನ್ಯಥಾ {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  ಯದಿ (size == 0) {
    freeblock(mc, block);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಯದಿ (mc->failnext) {
    mc->failnext = 0;
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* fake a single memory allocation error */
  }
  ಯದಿ (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    ಯದಿ (mc->countlimit == 0)
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ಯದಿ (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* fake a memory allocation error */
  ಅನ್ಯಥಾ {
    memHeader *newblock;
    ಪೂರ್ಣಾಙ್ಕ i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = ಪರಿಮಾಣ(memHeader) + size + MARKSIZE;
    ಯದಿ (realsize < size) ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    ಯದಿ (newblock == ರಿಕ್ತ)
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* really out of memory? */
    ಯದಿ (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    ಪ್ರತಿ (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    ಯದಿ (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    ಪ್ರತ್ಯಾಗಮ newblock + 1;
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

#ಅನ್ತರ್ಭಾವ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ಯದಿ (isdead(g,t)) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (issweepphase(g))
    ಪ್ರತ್ಯಾಗಮ 1;  /* no invariants */
  ಅನ್ಯಥಾ ಯದಿ (g->gckind != KGC_GENMINOR)
    ಪ್ರತ್ಯಾಗಮ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  ಅನ್ಯಥಾ {  /* generational mode */
    ಯದಿ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (ಶೂನ್ಯ *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ಯದಿ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


ಶೂನ್ಯ lua_printobj (lua_State *L, ಸಂರಚನಾ GCObject *o) {
  printobj(G(L), o);
}


ಶೂನ್ಯ lua_printvalue (TValue *v) {
  ವಿಕಲ್ಪ (ttypetag(v)) {
    ಪಕ್ಷ LUA_VNUMINT: ಪಕ್ಷ LUA_VNUMFLT: {
      ವರ್ಣ buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VFALSE:
      printf("%s", "false"); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VTRUE:
      printf("%s", "true"); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VNIL:
      printf("nil"); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VLCF:
      printf("light C function: %p", fvalue(v)); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); ಭಙ್ಗ;
    ಪಕ್ಷ LUA_VTABLE:
      printf("table: %p", hvalue(v)); ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      lua_assert(0);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testobjref (global_State *g, GCObject *f, GCObject *t) {
  ಪೂರ್ಣಾಙ್ಕ r1 = testobjref1(g, f, t);
  ಯದಿ (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  ಪ್ರತ್ಯಾಗಮ r1;
}


ಸ್ಥಿರ ಶೂನ್ಯ checkobjref (global_State *g, GCObject *f, GCObject *t) {
    ಪ್ರತಿಜ್ಞಾ(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ ಯದಿ (t) checkobjref(g,f,obj2gco(t)); }


ಸ್ಥಿರ ಶೂನ್ಯ checkvalref (global_State *g, GCObject *f, ಧ್ರುವ TValue *t) {
  ಪ್ರತಿಜ್ಞಾ(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


ಸ್ಥಿರ ಶೂನ್ಯ checktable (global_State *g, Table *h) {
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  unsigned ಪೂರ್ಣಾಙ್ಕ asize = h->asize;
  ಗಂಟು_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  ಪ್ರತಿ (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  ಪ್ರತಿ (n = gnode(h, 0); n < limit; n++) {
    ಯದಿ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      ಪ್ರತಿಜ್ಞಾ(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ checkudata (global_State *g, Udata *u) {
  ಪೂರ್ಣಾಙ್ಕ i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  ಪ್ರತಿ (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkproto (global_State *g, Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  ಪ್ರತಿ (i=0; i<f->sizek; i++) {
    ಯದಿ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  ಪ್ರತಿ (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  ಪ್ರತಿ (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  ಪ್ರತಿ (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ಪೂರ್ಣಾಙ್ಕ i;
  checkobjrefN(g, clgc, cl->p);
  ಪ್ರತಿ (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    ಯದಿ (uv) {
      checkobjrefN(g, clgc, uv);
      ಯದಿ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ lua_checkpc (CallInfo *ci) {
  ಯದಿ (!isLua(ci)) ಪ್ರತ್ಯಾಗಮ 1;
  ಅನ್ಯಥಾ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ಪ್ರತ್ಯಾಗಮ p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ check_ರಾಶಿ (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  ಪ್ರತಿಜ್ಞಾ(!isdead(g, L1));
  ಯದಿ (L1->ರಾಶಿ.p == ರಿಕ್ತ) {  /* incomplete thread? */
    ಪ್ರತಿಜ್ಞಾ(L1->openupval == ರಿಕ್ತ && L1->ci == ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಪ್ರತಿ (uv = L1->openupval; uv != ರಿಕ್ತ; uv = uv->u.open.next)
    ಪ್ರತಿಜ್ಞಾ(upisopen(uv));  /* must be open */
  ಪ್ರತಿಜ್ಞಾ(L1->ತುದಿ.p <= L1->ರಾಶಿ_last.p);
  ಪ್ರತಿಜ್ಞಾ(L1->tbclist.p <= L1->ತುದಿ.p);
  ಪ್ರತಿ (ci = L1->ci; ci != ರಿಕ್ತ; ci = ci->previous) {
    ಪ್ರತಿಜ್ಞಾ(ci->ತುದಿ.p <= L1->ರಾಶಿ_last.p);
    ಪ್ರತಿಜ್ಞಾ(lua_checkpc(ci));
  }
  ಪ್ರತಿ (o = L1->ರಾಶಿ.p; o < L1->ರಾಶಿ_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


ಸ್ಥಿರ ಶೂನ್ಯ checkrefs (global_State *g, GCObject *o) {
  ವಿಕಲ್ಪ (o->tt) {
    ಪಕ್ಷ LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VTABLE: {
      checktable(g, gco2t(o));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VTHREAD: {
      check_ರಾಶಿ(g, gco2th(o));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VPROTO: {
      checkproto(g, gco2p(o));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ LUA_VSHRSTR:
    ಪಕ್ಷ LUA_VLNGSTR: {
      ಪ್ರತಿಜ್ಞಾ(!isgray(o));  /* strings are never gray */
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: ಪ್ರತಿಜ್ಞಾ(0);
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
ಸ್ಥಿರ ಶೂನ್ಯ checkobject (global_State *g, GCObject *o, ಪೂರ್ಣಾಙ್ಕ maybedead,
                         ಪೂರ್ಣಾಙ್ಕ listage) {
  ಯದಿ (isdead(g, o))
    ಪ್ರತಿಜ್ಞಾ(maybedead);
  ಅನ್ಯಥಾ {
    ಪ್ರತಿಜ್ಞಾ(g->gcstate != GCSpause || iswhite(o));
    ಯದಿ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      ಪ್ರತಿಜ್ಞಾ(getage(o) >= listage);
      ಯದಿ (isold(o)) {
        ಪ್ರತಿಜ್ಞಾ(!iswhite(o));
        ಪ್ರತಿಜ್ಞಾ(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      ಪ್ರತಿಜ್ಞಾ(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


ಸ್ಥಿರ l_mem checkgraylist (global_State *g, GCObject *o) {
  ಪೂರ್ಣಾಙ್ಕ total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ಯಾವತ್ (o) {
    ಪ್ರತಿಜ್ಞಾ(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    ಪ್ರತಿಜ್ಞಾ(!testbit(o->marked, TESTBIT));
    ಯದಿ (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    ವಿಕಲ್ಪ (o->tt) {
      ಪಕ್ಷ LUA_VTABLE: o = gco2t(o)->gclist; ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VLCL: o = gco2lcl(o)->gclist; ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VCCL: o = gco2ccl(o)->gclist; ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VTHREAD: o = gco2th(o)->gclist; ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VPROTO: o = gco2p(o)->gclist; ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VUSERDATA:
        ಪ್ರತಿಜ್ಞಾ(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        ಭಙ್ಗ;
      ಸ್ವಭಾವ: ಪ್ರತಿಜ್ಞಾ(0);  /* other objects cannot be in a gray list */
    }
  }
  ಪ್ರತ್ಯಾಗಮ total;
}


/*
** Check objects in gray lists.
*/
ಸ್ಥಿರ l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  ಯದಿ (!keepinvariant(g)) ಪ್ರತ್ಯಾಗಮ total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  ಪ್ರತ್ಯಾಗಮ total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ incifingray (global_State *g, GCObject *o, l_mem *ಎಣಿಕೆ) {
  ಯದಿ (!keepinvariant(g))
    ಪ್ರತ್ಯಾಗಮ;  /* gray lists not being kept in these phases */
  ಯದಿ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    ಪ್ರತಿಜ್ಞಾ(!isgray(o) || upisopen(gco2upv(o)));
    ಪ್ರತ್ಯಾಗಮ;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ಯದಿ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*ಎಣಿಕೆ)++;
    ಪ್ರತಿಜ್ಞಾ(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


ಸ್ಥಿರ l_mem checklist (global_State *g, ಪೂರ್ಣಾಙ್ಕ maybedead, ಪೂರ್ಣಾಙ್ಕ tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  ಪ್ರತಿ (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಪ್ರತಿ (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಪ್ರತಿ (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಪ್ರತಿ (o = reallyold; o != ರಿಕ್ತ; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಪ್ರತ್ಯಾಗಮ total;
}


ಪೂರ್ಣಾಙ್ಕ lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ಪೂರ್ಣಾಙ್ಕ maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ಯದಿ (keepinvariant(g)) {
    ಪ್ರತಿಜ್ಞಾ(!iswhite(mainthread(g)));
    ಪ್ರತಿಜ್ಞಾ(!iswhite(gcvalue(&g->l_registry)));
  }
  ಪ್ರತಿಜ್ಞಾ(!isdead(g, gcvalue(&g->l_registry)));
  ಪ್ರತಿಜ್ಞಾ(g->sweepgc == ರಿಕ್ತ || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  ಪ್ರತಿ (o = g->fixedgc; o != ರಿಕ್ತ; o = o->next) {
    ಪ್ರತಿಜ್ಞಾ(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  ಪ್ರತಿ (o = g->tobefnz; o != ರಿಕ್ತ; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    ಪ್ರತಿಜ್ಞಾ(tofinalize(o));
    ಪ್ರತಿಜ್ಞಾ(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ಯದಿ (keepinvariant(g))
    ಪ್ರತಿಜ್ಞಾ(totalin == totalshould);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


ಸ್ಥಿರ ವರ್ಣ *buildop (Proto *p, ಪೂರ್ಣಾಙ್ಕ pc, ವರ್ಣ *buff) {
  ವರ್ಣ *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  ಧ್ರುವ ವರ್ಣ *name = opnames[o];
  ಪೂರ್ಣಾಙ್ಕ ಸಾಲು = luaG_getfuncline(p, pc);
  ಪೂರ್ಣಾಙ್ಕ lineinfo = (p->lineinfo != ರಿಕ್ತ) ? p->lineinfo[pc] : 0;
  ಯದಿ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  ಅನ್ಯಥಾ
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", ಸಾಲು, pc);
  ವಿಕಲ್ಪ (getOpMode(o)) {
    ಪಕ್ಷ iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      ಭಙ್ಗ;
    ಪಕ್ಷ ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      ಭಙ್ಗ;
    ಪಕ್ಷ iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      ಭಙ್ಗ;
    ಪಕ್ಷ iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      ಭಙ್ಗ;
    ಪಕ್ಷ iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      ಭಙ್ಗ;
    ಪಕ್ಷ isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ obuff;
}


#ಯದಿ 0
ಶೂನ್ಯ luaI_printcode (Proto *pt, ಪೂರ್ಣಾಙ್ಕ size) {
  ಪೂರ್ಣಾಙ್ಕ pc;
  ಪ್ರತಿ (pc=0; pc<size; pc++) {
    ವರ್ಣ buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


ಶೂನ್ಯ luaI_printinst (Proto *pt, ಪೂರ್ಣಾಙ್ಕ pc) {
  ವರ್ಣ buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ listcode (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  ಪ್ರತಿ (pc=0; pc<p->sizecode; pc++) {
    ವರ್ಣ buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ printcode (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  ಪ್ರತಿ (pc=0; pc<p->sizecode; pc++) {
    ವರ್ಣ buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ listk (lua_State *L) {
  Proto *p;
  ಪೂರ್ಣಾಙ್ಕ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  ಪ್ರತಿ (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ listabslineinfo (lua_State *L) {
  Proto *p;
  ಪೂರ್ಣಾಙ್ಕ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ರಿಕ್ತ, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  ಪ್ರತಿ (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].ಸಾಲು);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ listlocals (lua_State *L) {
  Proto *p;
  ಪೂರ್ಣಾಙ್ಕ pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ಪೂರ್ಣಾಙ್ಕ i = 0;
  ಧ್ರುವ ವರ್ಣ *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ಯಾವತ್ ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ರಿಕ್ತ)
    lua_pushstring(L, name);
  ಪ್ರತ್ಯಾಗಮ i-1;
}

/* }====================================================== */



ಶೂನ್ಯ lua_printstack (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = lua_gettop(L);
  printf("stack: >>\n");
  ಪ್ರತಿ (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


ಪೂರ್ಣಾಙ್ಕ lua_printallstack (lua_State *L) {
  StkId p;
  ಪೂರ್ಣಾಙ್ಕ i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  ಪ್ರತಿ (p = L->ರಾಶಿ.p; p < L->ತುದಿ.p; p++) {
    ಯದಿ (ci != ರಿಕ್ತ && p == ci->func.p) {
      printf("  ---\n");
      ಯದಿ (ci == L->ci)
        ci = ರಿಕ್ತ;  /* printed last frame */
      ಅನ್ಯಥಾ
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", ಅಂಕೆ_OPCODES);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", ಪರಿಮಾಣ(lua_State));
  setnameval(L, "global state", ಪರಿಮಾಣ(global_State));
  setnameval(L, "TValue", ಪರಿಮಾಣ(TValue));
  setnameval(L, "Node", ಪರಿಮಾಣ(ಗಂಟು_2));
  setnameval(L, "stack Value", ಪರಿಮಾಣ(ರಾಶಿ_ಮೌಲ್ಯ));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mem_query (lua_State *L) {
  ಯದಿ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ಪ್ರತ್ಯಾಗಮ 3;
  }
  ಅನ್ಯಥಾ ಯದಿ (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    ಯದಿ (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *t = luaL_checkstring(L, 1);
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ಯದಿ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ಪ್ರತ್ಯಾಗಮ 1;
      }
    }
    ಪ್ರತ್ಯಾಗಮ luaL_error(L, "unknown type '%s'", t);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ alloc_ಎಣಿಕೆ (lua_State *L) {
  ಯದಿ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  ಅನ್ಯಥಾ
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ settrick (lua_State *L) {
  ಯದಿ (ttisnil(obj_at(L, 1)))
    l_Trick = ರಿಕ್ತ;
  ಅನ್ಯಥಾ
    l_Trick = gcvalue(obj_at(L, 1));
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ಯದಿ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ಅನ್ಯಥಾ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ಯದಿ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ಅನ್ಯಥಾ {
    ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ಯದಿ (!iscollectable(o))
    printf("no collectable\n");
  ಅನ್ಯಥಾ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gc_state (lua_State *L) {
  ಸ್ಥಿರ ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ಪೂರ್ಣಾಙ್ಕ option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ಯದಿ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಅನ್ಯಥಾ {
    ಯದಿ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    ಯದಿ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ಪ್ರತ್ಯಾಗಮ 0;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tracinggc = 0;
ಶೂನ್ಯ luai_tracegctest (lua_State *L, ಪೂರ್ಣಾಙ್ಕ first) {
  ಯದಿ (!tracinggc) ಪ್ರತ್ಯಾಗಮ;
  ಅನ್ಯಥಾ {
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


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tracegc (lua_State *L) {
  ಯದಿ (lua_isnil(L, 1))
    tracinggc = 0;
  ಅನ್ಯಥಾ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hash_query (lua_State *L) {
  ಯದಿ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ಯದಿ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  ಅನ್ಯಥಾ {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->ಗಂಟು));
  }
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ stacklevel (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ a = 0;
  lua_pushinteger(L, cast_Integer(L->ತುದಿ.p - L->ರಾಶಿ.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ಪ್ರತ್ಯಾಗಮ 5;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ಯಾವತ್ (ci->next != ರಿಕ್ತ) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reallocstack (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->ತುದಿ.p - L->ರಾಶಿ.p) + n, 1);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ table_query (lua_State *L) {
  ಧ್ರುವ Table *t;
  ಪೂರ್ಣಾಙ್ಕ i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned ಪೂರ್ಣಾಙ್ಕ asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ಯದಿ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ಪ್ರತ್ಯಾಗಮ 3;
  }
  ಅನ್ಯಥಾ ಯದಿ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ಯದಿ (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->ತುದಿ.p));
    ಅನ್ಯಥಾ
      setnilvalue2s(L->ತುದಿ.p);
    api_incr_ತುದಿ(L);
    lua_pushnil(L);
  }
  ಅನ್ಯಥಾ ಯದಿ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ಯದಿ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    ಅನ್ಯಥಾ
      lua_pushliteral(L, "<undef>");
    ಯದಿ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    ಅನ್ಯಥಾ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->ಗಂಟು[i]));
  }
  ಪ್ರತ್ಯಾಗಮ 3;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ಪ್ರತ್ಯಾಗಮ 6;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ಪೂರ್ಣಾಙ್ಕ s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ಯದಿ (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    ಪ್ರತ್ಯಾಗಮ 2;
  }
  ಅನ್ಯಥಾ ಯದಿ (s < tb->size) {
    TString *ts;
    ಪೂರ್ಣಾಙ್ಕ n = 0;
    ಪ್ರತಿ (ts = tb->hash[s]; ts != ರಿಕ್ತ; ts = ts->u.hnext) {
      setsvalue2s(L, L->ತುದಿ.p, ts);
      api_incr_ತುದಿ(L);
      n++;
    }
    ಪ್ರತ್ಯಾಗಮ n;
  }
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getreftable (lua_State *L) {
  ಯದಿ (lua_istable(L, 2))  /* is there a table as second argument? */
    ಪ್ರತ್ಯಾಗಮ 2;  /* use it as the table */
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ LUA_REGISTRYINDEX;  /* default is to use the register */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tref (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ t = getreftable(L);
  ಪೂರ್ಣಾಙ್ಕ level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getref (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ t = getreftable(L);
  ಪೂರ್ಣಾಙ್ಕ level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ unref (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ t = getreftable(L);
  ಪೂರ್ಣಾಙ್ಕ level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ upvalue (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ಯದಿ (lua_isnone(L, 3)) {
    ಧ್ರುವ ವರ್ಣ *name = lua_getupvalue(L, 1, n);
    ಯದಿ (name == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 0;
    lua_pushstring(L, name);
    ಪ್ರತ್ಯಾಗಮ 2;
  }
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  ಪೂರ್ಣಾಙ್ಕ nuv = cast_int(luaL_optinteger(L, 2, 0));
  ವರ್ಣ *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  ಯಾವತ್ (size--) *p++ = '\0';
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &l);
  ಪೂರ್ಣಾಙ್ಕ status = luaL_loadbufferx(L1, s, l, s, "t");
  ಯದಿ (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ s2d (lua_State *L) {
  lua_pushnumber(L, cast_ಅಂಕೆ(*cast(ಧ್ರುವ ದ್ವಿಗುಣ *, luaL_checkstring(L, 1))));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ d2s (lua_State *L) {
  ದ್ವಿಗುಣ d = cast(ದ್ವಿಗುಣ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), ಪರಿಮಾಣ(d));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ newstate (lua_State *L) {
  ಶೂನ್ಯ *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ಯದಿ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  ಅನ್ಯಥಾ
    lua_pushnil(L);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ರಿಕ್ತ, 1, "state expected");
  ಪ್ರತ್ಯಾಗಮ L1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ಪೂರ್ಣಾಙ್ಕ load = cast_int(luaL_checkinteger(L, 2));
  ಪೂರ್ಣಾಙ್ಕ preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ರಿಕ್ತ, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ಧ್ರುವ ವರ್ಣ *code = luaL_checklstring(L, 2, &lcode);
  ಪೂರ್ಣಾಙ್ಕ status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  ಯದಿ (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ಯದಿ (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    ಪ್ರತ್ಯಾಗಮ 3;
  }
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ i = 0;
    ಯಾವತ್ (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_ತೆಗೆ(L1, i-1);
    ಪ್ರತ್ಯಾಗಮ i-1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ log2_aux (lua_State *L) {
  unsigned ಪೂರ್ಣಾಙ್ಕ x = (unsigned ಪೂರ್ಣಾಙ್ಕ)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸಂರಚನಾ Aux { jmp_buf jb; ಧ್ರುವ ವರ್ಣ *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ panicback (lua_State *L) {
  ಸಂರಚನಾ Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (ಸಂರಚನಾ Aux *)lua_touserdata(L, -1);
  lua_ತೆಗೆ(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ಪ್ರತ್ಯಾಗಮ 1;  /* to avoid warnings */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkpanic (lua_State *L) {
  ಸಂರಚನಾ Aux b;
  ಶೂನ್ಯ *ud;
  lua_State *L1;
  ಧ್ರುವ ವರ್ಣ *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ಯದಿ (L1 == ರಿಕ್ತ) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ಯದಿ (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  ಅನ್ಯಥಾ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ಪ್ರತ್ಯಾಗಮ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ externKstr (lua_State *L) {
  size_t len;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ರಿಕ್ತ, ರಿಕ್ತ);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ externstr (lua_State *L) {
  size_t len;
  ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L, 1, &len);
  ಶೂನ್ಯ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  ವರ್ಣ *buff = cast_charp((*allocf)(ud, ರಿಕ್ತ, 0, len + 1));
  ಯದಿ (buff == ರಿಕ್ತ) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * ಪರಿಮಾಣ(ವರ್ಣ));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ಪ್ರತ್ಯಾಗಮ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


ಸ್ಥಿರ ಶೂನ್ಯ sethookaux (lua_State *L, ಪೂರ್ಣಾಙ್ಕ mask, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ, ಧ್ರುವ ವರ್ಣ *code);

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ delimits = " \t\n,;";

ಸ್ಥಿರ ಶೂನ್ಯ skip (ಧ್ರುವ ವರ್ಣ **pc) {
  ಪ್ರತಿ (;;) {
    ಯದಿ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    ಅನ್ಯಥಾ ಯದಿ (**pc == '#') {  /* comment? */
      ಯಾವತ್ (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    ಅನ್ಯಥಾ ಭಙ್ಗ;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getnum_aux (lua_State *L, lua_State *L1, ಧ್ರುವ ವರ್ಣ **pc) {
  ಪೂರ್ಣಾಙ್ಕ res = 0;
  ಪೂರ್ಣಾಙ್ಕ sig = 1;
  skip(pc);
  ಯದಿ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_ತೆಗೆ(L1, 1);
    (*pc)++;
    ಪ್ರತ್ಯಾಗಮ res;
  }
  ಅನ್ಯಥಾ ಯದಿ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ಪ್ರತ್ಯಾಗಮ res;
  }
  ಅನ್ಯಥಾ ಯದಿ (**pc == '!') {
    (*pc)++;
    ಯದಿ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    ಅನ್ಯಥಾ ಯದಿ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    ಅನ್ಯಥಾ lua_assert(0);
    (*pc)++;
    ಪ್ರತ್ಯಾಗಮ res;
  }
  ಅನ್ಯಥಾ ಯದಿ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ಯದಿ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ಯಾವತ್ (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ಪ್ರತ್ಯಾಗಮ sig*res;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getstring_aux (lua_State *L, ವರ್ಣ *buff, ಧ್ರುವ ವರ್ಣ **pc) {
  ಪೂರ್ಣಾಙ್ಕ i = 0;
  skip(pc);
  ಯದಿ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ಪೂರ್ಣಾಙ್ಕ quote = *(*pc)++;
    ಯಾವತ್ (**pc != quote) {
      ಯದಿ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  ಅನ್ಯಥಾ {
    ಯಾವತ್ (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ಪ್ರತ್ಯಾಗಮ buff;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getindex_aux (lua_State *L, lua_State *L1, ಧ್ರುವ ವರ್ಣ **pc) {
  skip(pc);
  ವಿಕಲ್ಪ (*(*pc)++) {
    ಪಕ್ಷ 'R': ಪ್ರತ್ಯಾಗಮ LUA_REGISTRYINDEX;
    ಪಕ್ಷ 'U': ಪ್ರತ್ಯಾಗಮ lua_upvalueindex(getnum_aux(L, L1, pc));
    ಸ್ವಭಾವ: {
      ಪೂರ್ಣಾಙ್ಕ n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ಯದಿ (n == 0) ಪ್ರತ್ಯಾಗಮ 0;
      ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ lua_absindex(L1, n);
    }
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
ಸ್ಥಿರ ಶೂನ್ಯ regcodes (lua_State *L) {
  unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < ಪರಿಮಾಣ(statcodes) / ಪರಿಮಾಣ(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testC (lua_State *L);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ Cfunck (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ ops[] = "+-*%^/\\&|~<>_!";

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ runC (lua_State *L, lua_State *L1, ಧ್ರುವ ವರ್ಣ *pc) {
  ವರ್ಣ buff[300];
  ಪೂರ್ಣಾಙ್ಕ status = 0;
  ಯದಿ (pc == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ luaL_error(L, "attempt to runC null script");
  ಪ್ರತಿ (;;) {
    ಧ್ರುವ ವರ್ಣ *inst = getstring;
    ಯದಿ EQ("") ಪ್ರತ್ಯಾಗಮ 0;
    ಅನ್ಯಥಾ ಯದಿ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("append") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      ಪೂರ್ಣಾಙ್ಕ i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("arith") {
      ಪೂರ್ಣಾಙ್ಕ op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("call") {
      ಪೂರ್ಣಾಙ್ಕ narg = getnum;
      ಪೂರ್ಣಾಙ್ಕ nres = getnum;
      lua_call(L1, narg, nres);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("callk") {
      ಪೂರ್ಣಾಙ್ಕ narg = getnum;
      ಪೂರ್ಣಾಙ್ಕ nres = getnum;
      ಪೂರ್ಣಾಙ್ಕ i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("checkstack") {
      ಪೂರ್ಣಾಙ್ಕ sz = getnum;
      ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = getstring;
      ಯದಿ (*ಸಂದೇಶ == '\0')
        ಸಂದೇಶ = ರಿಕ್ತ;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, ಸಂದೇಶ);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rawcheckstack") {
      ಪೂರ್ಣಾಙ್ಕ sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("compare") {
      ಧ್ರುವ ವರ್ಣ *opt = getstring;  /* EQ, LT, or LE */
      ಪೂರ್ಣಾಙ್ಕ op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ಪೂರ್ಣಾಙ್ಕ a = getindex;
      ಪೂರ್ಣಾಙ್ಕ b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("concat") {
      lua_concat(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("copy") {
      ಪೂರ್ಣಾಙ್ಕ f = getindex;
      lua_copy(L1, f, getindex);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("getfield") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      ಪೂರ್ಣಾಙ್ಕ tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("getmetatable") {
      ಯದಿ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("gettable") {
      ಪೂರ್ಣಾಙ್ಕ tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("gsub") {
      ಪೂರ್ಣಾಙ್ಕ a = getnum; ಪೂರ್ಣಾಙ್ಕ b = getnum; ಪೂರ್ಣಾಙ್ಕ c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("insert") {
      lua_ಸೇರಿಸು(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("len") {
      lua_len(L1, getindex);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("loadstring") {
      size_t slen;
      ಧ್ರುವ ವರ್ಣ *s = luaL_checklstring(L1, getnum, &slen);
      ಧ್ರುವ ವರ್ಣ *name = getstring;
      ಧ್ರುವ ವರ್ಣ *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("newtable") {
      lua_newtable(L1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("newthread") {
      lua_newthread(L1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("next") {
      lua_next(L1, -2);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pcall") {
      ಪೂರ್ಣಾಙ್ಕ narg = getnum;
      ಪೂರ್ಣಾಙ್ಕ nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pcallk") {
      ಪೂರ್ಣಾಙ್ಕ narg = getnum;
      ಪೂರ್ಣಾಙ್ಕ nres = getnum;
      ಪೂರ್ಣಾಙ್ಕ i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pop") {
      lua_ತೆಗೆ(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("printstack") {
      ಪೂರ್ಣಾಙ್ಕ n = getnum;
      ಯದಿ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      ಅನ್ಯಥಾ lua_printstack(L1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("print") {
      ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = getstring;
      printf("%s\n", ಸಂದೇಶ);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("warningC") {
      ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = getstring;
      lua_warning(L1, ಸಂದೇಶ, 1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("warning") {
      ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = getstring;
      lua_warning(L1, ಸಂದೇಶ, 0);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushnil") {
      lua_pushnil(L1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushnum") {
      lua_pushnumber(L1, (lua_ಸಂಖ್ಯೆ)getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ಪೂರ್ಣಾಙ್ಕ)lua_tointeger(L, -1));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rawget") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_rawget(L1, t);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rawgeti") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rawgetp") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rawset") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_rawset(L1, t);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rawseti") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rawsetp") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("remove") {
      lua_remove(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("replace") {
      lua_replace(L1, getindex);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("resume") {
      ಪೂರ್ಣಾಙ್ಕ i = getindex;
      ಪೂರ್ಣಾಙ್ಕ nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("traceback") {
      ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = getstring;
      ಪೂರ್ಣಾಙ್ಕ level = getnum;
      luaL_traceback(L1, L1, ಸಂದೇಶ, level);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("return") {
      ಪೂರ್ಣಾಙ್ಕ n = getnum;
      ಯದಿ (L1 != L) {
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ (i = 0; i < n; i++) {
          ಪೂರ್ಣಾಙ್ಕ idx = -(n - i);
          ವಿಕಲ್ಪ (lua_type(L1, idx)) {
            ಪಕ್ಷ LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              ಭಙ್ಗ;
            ಸ್ವಭಾವ:
              lua_pushstring(L, lua_tostring(L1, idx));
              ಭಙ್ಗ;
          }
        }
      }
      ಪ್ರತ್ಯಾಗಮ n;
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("rotate") {
      ಪೂರ್ಣಾಙ್ಕ i = getindex;
      lua_rotate(L1, i, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("setfield") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      ಧ್ರುವ ವರ್ಣ *s = getstring;
      lua_setfield(L1, t, s);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("seti") {
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_seti(L1, t, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("setglobal") {
      ಧ್ರುವ ವರ್ಣ *s = getstring;
      lua_setglobal(L1, s);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("sethook") {
      ಪೂರ್ಣಾಙ್ಕ mask = getnum;
      ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = getnum;
      ಧ್ರುವ ವರ್ಣ *s = getstring;
      sethookaux(L1, mask, ಎಣಿಕೆ, s);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("setmetatable") {
      ಪೂರ್ಣಾಙ್ಕ idx = getindex;
      lua_setmetatable(L1, idx);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("settable") {
      lua_settable(L1, getindex);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("settop") {
      lua_settop(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("testudata") {
      ಪೂರ್ಣಾಙ್ಕ i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ರಿಕ್ತ);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("error") {
      lua_error(L1);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("abort") {
      abort();
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("throw") {
#ಯದಿ defined(__cplusplus)
ಸ್ಥಿರ ಸಂರಚನಾ X { ಪೂರ್ಣಾಙ್ಕ x; } x;
      throw x;
#ಅನ್ಯಥಾ
      luaL_error(L1, "C++");
#endif
      ಭಙ್ಗ;
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("tostring") {
      ಧ್ರುವ ವರ್ಣ *s = lua_tostring(L1, getindex);
      ಧ್ರುವ ವರ್ಣ *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ರಿಕ್ತ && s1 == ರಿಕ್ತ) || strcmp(s, s1) == 0);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ರಿಕ್ತ);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("xmove") {
      ಪೂರ್ಣಾಙ್ಕ f = getindex;
      ಪೂರ್ಣಾಙ್ಕ t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ಪೂರ್ಣಾಙ್ಕ n = getnum;
      ಯದಿ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("yield") {
      ಪ್ರತ್ಯಾಗಮ lua_yield(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("yieldk") {
      ಪೂರ್ಣಾಙ್ಕ nres = getnum;
      ಪೂರ್ಣಾಙ್ಕ i = getindex;
      ಪ್ರತ್ಯಾಗಮ lua_yieldk(L1, nres, i, Cfunck);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    ಅನ್ಯಥಾ ಯದಿ EQ("argerror") {
      ಪೂರ್ಣಾಙ್ಕ arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    ಅನ್ಯಥಾ luaL_error(L, "unknown instruction %s", buff);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ testC (lua_State *L) {
  lua_State *L1;
  ಧ್ರುವ ವರ್ಣ *pc;
  ಯದಿ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  ಅನ್ಯಥಾ ಯದಿ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  ಅನ್ಯಥಾ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ಪ್ರತ್ಯಾಗಮ runC(L, L1, pc);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ Cfunc (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ Cfunck (lua_State *L, ಪೂರ್ಣಾಙ್ಕ status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ಪ್ರತ್ಯಾಗಮ runC(L, L, lua_tostring(L, cast_int(ctx)));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ಪ್ರತ್ಯಾಗಮ 1;
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
ಸ್ಥಿರ ಶೂನ್ಯ Chook (lua_State *L, lua_Debug *ar) {
  ಧ್ರುವ ವರ್ಣ *scpt;
  ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_ತೆಗೆ(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
ಸ್ಥಿರ ಶೂನ್ಯ sethookaux (lua_State *L, ಪೂರ್ಣಾಙ್ಕ mask, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ, ಧ್ರುವ ವರ್ಣ *scpt) {
  ಯದಿ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ರಿಕ್ತ, 0, 0);  /* turn off hooks */
    ಪ್ರತ್ಯಾಗಮ;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ಯದಿ (!lua_istable(L, -1)) {  /* no hook table? */
    lua_ತೆಗೆ(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, ಎಣಿಕೆ);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sethook (lua_State *L) {
  ಯದಿ (lua_isnoneornil(L, 1))
    lua_sethook(L, ರಿಕ್ತ, 0, 0);  /* turn off hooks */
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *scpt = luaL_checkstring(L, 1);
    ಧ್ರುವ ವರ್ಣ *smask = luaL_checkstring(L, 2);
    ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = cast_int(luaL_optinteger(L, 3, 0));
    ಪೂರ್ಣಾಙ್ಕ mask = 0;
    ಯದಿ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    ಯದಿ (strchr(smask, 'r')) mask |= LUA_MASKRET;
    ಯದಿ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    ಯದಿ (ಎಣಿಕೆ > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, ಎಣಿಕೆ, scpt);
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ coresume (lua_State *L) {
  ಪೂರ್ಣಾಙ್ಕ status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  ಯದಿ (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_ಸೇರಿಸು(L, -2);
    ಪ್ರತ್ಯಾಗಮ 2;  /* return false + error message */
  }
  ಅನ್ಯಥಾ {
    lua_pushboolean(L, 1);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
}

#ಯದಿ !defined(LUA_USE_POSIX)

#define nonblock	ರಿಕ್ತ

#ಅನ್ಯಥಾ

#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ಪೂರ್ಣಾಙ್ಕ fd = fileno(f);
  ಪೂರ್ಣಾಙ್ಕ flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

/* }====================================================== */



ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_ಎಣಿಕೆ},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {ರಿಕ್ತ, ರಿಕ್ತ}
};


ಸ್ಥಿರ ಶೂನ್ಯ checkfinalmem (ಶೂನ್ಯ) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


ಪೂರ್ಣಾಙ್ಕ luaB_opentests (lua_State *L) {
  ಶೂನ್ಯ *ud;
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
  ಪ್ರತ್ಯಾಗಮ 1;
}

#endif

