/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <setjmp.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lapi.h"
#അന്തര്ഭാവ "lauxlib.h"
#അന്തര്ഭാവ "lcode.h"
#അന്തര്ഭാവ "lctype.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lopcodes.h"
#അന്തര്ഭാവ "lopnames.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#എങ്കിൽ defined(LUA_DEBUG)


ശൂന്യം *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


സ്ഥിതം പൂർണ്ണസംഖ്യ runC (lua_State *L, lua_State *L1, സ്ഥിരം അക്ഷരം *pc);


സ്ഥിതം ശൂന്യം setnameval (lua_State *L, സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


സ്ഥിതം ശൂന്യം pushobject (lua_State *L, സ്ഥിരം TValue *o) {
  setobj2s(L, L->മുകൾ.p, o);
  api_incr_മുകൾ(L);
}


സ്ഥിതം ശൂന്യം badexit (സ്ഥിരം അക്ഷരം *fmt, സ്ഥിരം അക്ഷരം *s1, സ്ഥിരം അക്ഷരം *s2) {
  fprintf(stderr, fmt, s1);
  എങ്കിൽ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tpanic (lua_State *L) {
  സ്ഥിരം അക്ഷരം *സന്ദേശം = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  തിരികെ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   സന്ദേശം, ശൂന്യ),
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
സ്ഥിതം ശൂന്യം warnf (ശൂന്യം *ud, സ്ഥിരം അക്ഷരം *സന്ദേശം, പൂർണ്ണസംഖ്യ tocont) {
  lua_State *L = cast(lua_State *, ud);
  സ്ഥിതം അക്ഷരം buff[200] = "";  /* should be enough for tests... */
  സ്ഥിതം പൂർണ്ണസംഖ്യ onoff = 0;
  സ്ഥിതം പൂർണ്ണസംഖ്യ mode = 0;  /* start in normal mode */
  സ്ഥിതം പൂർണ്ണസംഖ്യ lasttocont = 0;
  എങ്കിൽ (!lasttocont && !tocont && *സന്ദേശം == '@') {  /* control message? */
    എങ്കിൽ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", സന്ദേശം, buff);
    എങ്കിൽ (strcmp(സന്ദേശം, "@off") == 0)
      onoff = 0;
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(സന്ദേശം, "@on") == 0)
      onoff = 1;
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(സന്ദേശം, "@normal") == 0)
      mode = 0;
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(സന്ദേശം, "@allow") == 0)
      mode = 1;
    അല്ലെങ്കിൽ എങ്കിൽ (strcmp(സന്ദേശം, "@store") == 0)
      mode = 2;
    അല്ലെങ്കിൽ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              സന്ദേശം, ശൂന്യ);
    തിരികെ;
  }
  lasttocont = tocont;
  എങ്കിൽ (strlen(സന്ദേശം) >= വലുപ്പം(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", സന്ദേശം, buff);
  strcat(buff, സന്ദേശം);  /* add new message to current warning */
  എങ്കിൽ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    എങ്കിൽ (!lua_toboolean(L, -1))
      lua_എടുക്കുക(L, 1);  /* ok, no previous unexpected warning */
    അല്ലെങ്കിൽ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    തിരഞ്ഞെടുപ്പ് (mode) {
      അവസ്ഥ 0: {  /* normal */
        എങ്കിൽ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ശൂന്യ);
      }  /* FALLTHROUGH */
      അവസ്ഥ 1: {  /* allow */
        എങ്കിൽ (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        നിർത്തുക;
      }
      അവസ്ഥ 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        നിർത്തുക;
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

തരനിർവചനം സംഘം memHeader {
  LUAI_MAXALIGN;
  ഘടന {
    size_t size;
    പൂർണ്ണസംഖ്യ type;
  } d;
} memHeader;


#എങ്കിൽ !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#അല്ലെങ്കിൽ

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


സ്ഥിതം ശൂന്യം freeblock (Memcontrol *mc, memHeader *block) {
  എങ്കിൽ (block) {
    size_t size = block->d.size;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, വലുപ്പം(memHeader) + size + MARKSIZE);  /* erase block */
    സ്വതന്ത്രം(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


ശൂന്യം *debug_realloc (ശൂന്യം *ud, ശൂന്യം *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  പൂർണ്ണസംഖ്യ type;
  എങ്കിൽ (mc->memlimit == 0) {  /* first time? */
    അക്ഷരം *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, ശൂന്യ, 10) : ULONG_MAX;
  }
  എങ്കിൽ (block == ശൂന്യ) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  അല്ലെങ്കിൽ {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  എങ്കിൽ (size == 0) {
    freeblock(mc, block);
    തിരികെ ശൂന്യ;
  }
  എങ്കിൽ (mc->failnext) {
    mc->failnext = 0;
    തിരികെ ശൂന്യ;  /* fake a single memory allocation error */
  }
  എങ്കിൽ (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    എങ്കിൽ (mc->countlimit == 0)
      തിരികെ ശൂന്യ;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  എങ്കിൽ (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    തിരികെ ശൂന്യ;  /* fake a memory allocation error */
  അല്ലെങ്കിൽ {
    memHeader *newblock;
    പൂർണ്ണസംഖ്യ i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = വലുപ്പം(memHeader) + size + MARKSIZE;
    എങ്കിൽ (realsize < size) തിരികെ ശൂന്യ;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    എങ്കിൽ (newblock == ശൂന്യ)
      തിരികെ ശൂന്യ;  /* really out of memory? */
    എങ്കിൽ (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    വേണ്ടി (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    എങ്കിൽ (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    തിരികെ newblock + 1;
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

#അന്തര്ഭാവ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  എങ്കിൽ (isdead(g,t)) തിരികെ 0;
  എങ്കിൽ (issweepphase(g))
    തിരികെ 1;  /* no invariants */
  അല്ലെങ്കിൽ എങ്കിൽ (g->gckind != KGC_GENMINOR)
    തിരികെ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  അല്ലെങ്കിൽ {  /* generational mode */
    എങ്കിൽ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      തിരികെ 0;
    എങ്കിൽ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      തിരികെ 0;
    തിരികെ 1;
  }
}


സ്ഥിതം ശൂന്യം printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (ശൂന്യം *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  എങ്കിൽ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


ശൂന്യം lua_printobj (lua_State *L, ഘടന GCObject *o) {
  printobj(G(L), o);
}


ശൂന്യം lua_printvalue (TValue *v) {
  തിരഞ്ഞെടുപ്പ് (ttypetag(v)) {
    അവസ്ഥ LUA_VNUMINT: അവസ്ഥ LUA_VNUMFLT: {
      അക്ഷരം buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); നിർത്തുക;
    അവസ്ഥ LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); നിർത്തുക;
    അവസ്ഥ LUA_VFALSE:
      printf("%s", "false"); നിർത്തുക;
    അവസ്ഥ LUA_VTRUE:
      printf("%s", "true"); നിർത്തുക;
    അവസ്ഥ LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); നിർത്തുക;
    അവസ്ഥ LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); നിർത്തുക;
    അവസ്ഥ LUA_VNIL:
      printf("nil"); നിർത്തുക;
    അവസ്ഥ LUA_VLCF:
      printf("light C function: %p", fvalue(v)); നിർത്തുക;
    അവസ്ഥ LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); നിർത്തുക;
    അവസ്ഥ LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); നിർത്തുക;
    അവസ്ഥ LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); നിർത്തുക;
    അവസ്ഥ LUA_VTABLE:
      printf("table: %p", hvalue(v)); നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      lua_assert(0);
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ testobjref (global_State *g, GCObject *f, GCObject *t) {
  പൂർണ്ണസംഖ്യ r1 = testobjref1(g, f, t);
  എങ്കിൽ (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  തിരികെ r1;
}


സ്ഥിതം ശൂന്യം checkobjref (global_State *g, GCObject *f, GCObject *t) {
    പ്രതിജ്ഞാ(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ എങ്കിൽ (t) checkobjref(g,f,obj2gco(t)); }


സ്ഥിതം ശൂന്യം checkvalref (global_State *g, GCObject *f, സ്ഥിരം TValue *t) {
  പ്രതിജ്ഞാ(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


സ്ഥിതം ശൂന്യം checktable (global_State *g, Table *h) {
  unsigned പൂർണ്ണസംഖ്യ i;
  unsigned പൂർണ്ണസംഖ്യ asize = h->asize;
  കണ്ണി_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  വേണ്ടി (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  വേണ്ടി (n = gnode(h, 0); n < limit; n++) {
    എങ്കിൽ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      പ്രതിജ്ഞാ(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


സ്ഥിതം ശൂന്യം checkudata (global_State *g, Udata *u) {
  പൂർണ്ണസംഖ്യ i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  വേണ്ടി (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


സ്ഥിതം ശൂന്യം checkproto (global_State *g, Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  വേണ്ടി (i=0; i<f->sizek; i++) {
    എങ്കിൽ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  വേണ്ടി (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  വേണ്ടി (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  വേണ്ടി (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


സ്ഥിതം ശൂന്യം checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


സ്ഥിതം ശൂന്യം checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  പൂർണ്ണസംഖ്യ i;
  checkobjrefN(g, clgc, cl->p);
  വേണ്ടി (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    എങ്കിൽ (uv) {
      checkobjrefN(g, clgc, uv);
      എങ്കിൽ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ lua_checkpc (CallInfo *ci) {
  എങ്കിൽ (!isLua(ci)) തിരികെ 1;
  അല്ലെങ്കിൽ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    തിരികെ p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


സ്ഥിതം ശൂന്യം check_അട്ടി (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  പ്രതിജ്ഞാ(!isdead(g, L1));
  എങ്കിൽ (L1->അട്ടി.p == ശൂന്യ) {  /* incomplete thread? */
    പ്രതിജ്ഞാ(L1->openupval == ശൂന്യ && L1->ci == ശൂന്യ);
    തിരികെ;
  }
  വേണ്ടി (uv = L1->openupval; uv != ശൂന്യ; uv = uv->u.open.next)
    പ്രതിജ്ഞാ(upisopen(uv));  /* must be open */
  പ്രതിജ്ഞാ(L1->മുകൾ.p <= L1->അട്ടി_last.p);
  പ്രതിജ്ഞാ(L1->tbclist.p <= L1->മുകൾ.p);
  വേണ്ടി (ci = L1->ci; ci != ശൂന്യ; ci = ci->previous) {
    പ്രതിജ്ഞാ(ci->മുകൾ.p <= L1->അട്ടി_last.p);
    പ്രതിജ്ഞാ(lua_checkpc(ci));
  }
  വേണ്ടി (o = L1->അട്ടി.p; o < L1->അട്ടി_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


സ്ഥിതം ശൂന്യം checkrefs (global_State *g, GCObject *o) {
  തിരഞ്ഞെടുപ്പ് (o->tt) {
    അവസ്ഥ LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VTABLE: {
      checktable(g, gco2t(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VTHREAD: {
      check_അട്ടി(g, gco2th(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VPROTO: {
      checkproto(g, gco2p(o));
      നിർത്തുക;
    }
    അവസ്ഥ LUA_VSHRSTR:
    അവസ്ഥ LUA_VLNGSTR: {
      പ്രതിജ്ഞാ(!isgray(o));  /* strings are never gray */
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: പ്രതിജ്ഞാ(0);
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
സ്ഥിതം ശൂന്യം checkobject (global_State *g, GCObject *o, പൂർണ്ണസംഖ്യ maybedead,
                         പൂർണ്ണസംഖ്യ listage) {
  എങ്കിൽ (isdead(g, o))
    പ്രതിജ്ഞാ(maybedead);
  അല്ലെങ്കിൽ {
    പ്രതിജ്ഞാ(g->gcstate != GCSpause || iswhite(o));
    എങ്കിൽ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      പ്രതിജ്ഞാ(getage(o) >= listage);
      എങ്കിൽ (isold(o)) {
        പ്രതിജ്ഞാ(!iswhite(o));
        പ്രതിജ്ഞാ(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      പ്രതിജ്ഞാ(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


സ്ഥിതം l_mem checkgraylist (global_State *g, GCObject *o) {
  പൂർണ്ണസംഖ്യ total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ആകുമ്പോൾ (o) {
    പ്രതിജ്ഞാ(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    പ്രതിജ്ഞാ(!testbit(o->marked, TESTBIT));
    എങ്കിൽ (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    തിരഞ്ഞെടുപ്പ് (o->tt) {
      അവസ്ഥ LUA_VTABLE: o = gco2t(o)->gclist; നിർത്തുക;
      അവസ്ഥ LUA_VLCL: o = gco2lcl(o)->gclist; നിർത്തുക;
      അവസ്ഥ LUA_VCCL: o = gco2ccl(o)->gclist; നിർത്തുക;
      അവസ്ഥ LUA_VTHREAD: o = gco2th(o)->gclist; നിർത്തുക;
      അവസ്ഥ LUA_VPROTO: o = gco2p(o)->gclist; നിർത്തുക;
      അവസ്ഥ LUA_VUSERDATA:
        പ്രതിജ്ഞാ(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി: പ്രതിജ്ഞാ(0);  /* other objects cannot be in a gray list */
    }
  }
  തിരികെ total;
}


/*
** Check objects in gray lists.
*/
സ്ഥിതം l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  എങ്കിൽ (!keepinvariant(g)) തിരികെ total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  തിരികെ total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
സ്ഥിതം ശൂന്യം incifingray (global_State *g, GCObject *o, l_mem *എണ്ണം) {
  എങ്കിൽ (!keepinvariant(g))
    തിരികെ;  /* gray lists not being kept in these phases */
  എങ്കിൽ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    പ്രതിജ്ഞാ(!isgray(o) || upisopen(gco2upv(o)));
    തിരികെ;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  എങ്കിൽ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*എണ്ണം)++;
    പ്രതിജ്ഞാ(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


സ്ഥിതം l_mem checklist (global_State *g, പൂർണ്ണസംഖ്യ maybedead, പൂർണ്ണസംഖ്യ tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  വേണ്ടി (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    പ്രതിജ്ഞാ(!tof == !tofinalize(o));
  }
  വേണ്ടി (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    പ്രതിജ്ഞാ(!tof == !tofinalize(o));
  }
  വേണ്ടി (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    പ്രതിജ്ഞാ(!tof == !tofinalize(o));
  }
  വേണ്ടി (o = reallyold; o != ശൂന്യ; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    പ്രതിജ്ഞാ(!tof == !tofinalize(o));
  }
  തിരികെ total;
}


പൂർണ്ണസംഖ്യ lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  പൂർണ്ണസംഖ്യ maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  എങ്കിൽ (keepinvariant(g)) {
    പ്രതിജ്ഞാ(!iswhite(mainthread(g)));
    പ്രതിജ്ഞാ(!iswhite(gcvalue(&g->l_registry)));
  }
  പ്രതിജ്ഞാ(!isdead(g, gcvalue(&g->l_registry)));
  പ്രതിജ്ഞാ(g->sweepgc == ശൂന്യ || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  വേണ്ടി (o = g->fixedgc; o != ശൂന്യ; o = o->next) {
    പ്രതിജ്ഞാ(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  വേണ്ടി (o = g->tobefnz; o != ശൂന്യ; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    പ്രതിജ്ഞാ(tofinalize(o));
    പ്രതിജ്ഞാ(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  എങ്കിൽ (keepinvariant(g))
    പ്രതിജ്ഞാ(totalin == totalshould);
  തിരികെ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


സ്ഥിതം അക്ഷരം *buildop (Proto *p, പൂർണ്ണസംഖ്യ pc, അക്ഷരം *buff) {
  അക്ഷരം *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  സ്ഥിരം അക്ഷരം *name = opnames[o];
  പൂർണ്ണസംഖ്യ വരി = luaG_getfuncline(p, pc);
  പൂർണ്ണസംഖ്യ lineinfo = (p->lineinfo != ശൂന്യ) ? p->lineinfo[pc] : 0;
  എങ്കിൽ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  അല്ലെങ്കിൽ
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", വരി, pc);
  തിരഞ്ഞെടുപ്പ് (getOpMode(o)) {
    അവസ്ഥ iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      നിർത്തുക;
    അവസ്ഥ ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      നിർത്തുക;
    അവസ്ഥ iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      നിർത്തുക;
    അവസ്ഥ iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      നിർത്തുക;
    അവസ്ഥ iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      നിർത്തുക;
    അവസ്ഥ isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      നിർത്തുക;
  }
  തിരികെ obuff;
}


#എങ്കിൽ 0
ശൂന്യം luaI_printcode (Proto *pt, പൂർണ്ണസംഖ്യ size) {
  പൂർണ്ണസംഖ്യ pc;
  വേണ്ടി (pc=0; pc<size; pc++) {
    അക്ഷരം buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


ശൂന്യം luaI_printinst (Proto *pt, പൂർണ്ണസംഖ്യ pc) {
  അക്ഷരം buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


സ്ഥിതം പൂർണ്ണസംഖ്യ listcode (lua_State *L) {
  പൂർണ്ണസംഖ്യ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  വേണ്ടി (pc=0; pc<p->sizecode; pc++) {
    അക്ഷരം buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ printcode (lua_State *L) {
  പൂർണ്ണസംഖ്യ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  വേണ്ടി (pc=0; pc<p->sizecode; pc++) {
    അക്ഷരം buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ listk (lua_State *L) {
  Proto *p;
  പൂർണ്ണസംഖ്യ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  വേണ്ടി (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ listabslineinfo (lua_State *L) {
  Proto *p;
  പൂർണ്ണസംഖ്യ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ശൂന്യ, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  വേണ്ടി (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].വരി);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ listlocals (lua_State *L) {
  Proto *p;
  പൂർണ്ണസംഖ്യ pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  പൂർണ്ണസംഖ്യ i = 0;
  സ്ഥിരം അക്ഷരം *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ആകുമ്പോൾ ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ശൂന്യ)
    lua_pushstring(L, name);
  തിരികെ i-1;
}

/* }====================================================== */



ശൂന്യം lua_printstack (lua_State *L) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = lua_gettop(L);
  printf("stack: >>\n");
  വേണ്ടി (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


പൂർണ്ണസംഖ്യ lua_printallstack (lua_State *L) {
  StkId p;
  പൂർണ്ണസംഖ്യ i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  വേണ്ടി (p = L->അട്ടി.p; p < L->മുകൾ.p; p++) {
    എങ്കിൽ (ci != ശൂന്യ && p == ci->func.p) {
      printf("  ---\n");
      എങ്കിൽ (ci == L->ci)
        ci = ശൂന്യ;  /* printed last frame */
      അല്ലെങ്കിൽ
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", അക്കം_OPCODES);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", വലുപ്പം(lua_State));
  setnameval(L, "global state", വലുപ്പം(global_State));
  setnameval(L, "TValue", വലുപ്പം(TValue));
  setnameval(L, "Node", വലുപ്പം(കണ്ണി_2));
  setnameval(L, "stack Value", വലുപ്പം(അട്ടി_മൂല്യം));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ mem_query (lua_State *L) {
  എങ്കിൽ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    തിരികെ 3;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    എങ്കിൽ (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    തിരികെ 0;
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *t = luaL_checkstring(L, 1);
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      എങ്കിൽ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        തിരികെ 1;
      }
    }
    തിരികെ luaL_error(L, "unknown type '%s'", t);
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ alloc_എണ്ണം (lua_State *L) {
  എങ്കിൽ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  അല്ലെങ്കിൽ
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ settrick (lua_State *L) {
  എങ്കിൽ (ttisnil(obj_at(L, 1)))
    l_Trick = ശൂന്യ;
  അല്ലെങ്കിൽ
    l_Trick = gcvalue(obj_at(L, 1));
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  എങ്കിൽ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  അല്ലെങ്കിൽ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  എങ്കിൽ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  അല്ലെങ്കിൽ {
    സ്ഥിതം സ്ഥിരം അക്ഷരം *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  എങ്കിൽ (!iscollectable(o))
    printf("no collectable\n");
  അല്ലെങ്കിൽ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  തിരികെ 0;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

സ്ഥിതം പൂർണ്ണസംഖ്യ gc_state (lua_State *L) {
  സ്ഥിതം സ്ഥിരം പൂർണ്ണസംഖ്യ states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  പൂർണ്ണസംഖ്യ option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  എങ്കിൽ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ {
    എങ്കിൽ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    എങ്കിൽ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    തിരികെ 0;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tracinggc = 0;
ശൂന്യം luai_tracegctest (lua_State *L, പൂർണ്ണസംഖ്യ first) {
  എങ്കിൽ (!tracinggc) തിരികെ;
  അല്ലെങ്കിൽ {
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


സ്ഥിതം പൂർണ്ണസംഖ്യ tracegc (lua_State *L) {
  എങ്കിൽ (lua_isnil(L, 1))
    tracinggc = 0;
  അല്ലെങ്കിൽ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ hash_query (lua_State *L) {
  എങ്കിൽ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    എങ്കിൽ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  അല്ലെങ്കിൽ {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->കണ്ണി));
  }
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ stacklevel (lua_State *L) {
  പൂർണ്ണസംഖ്യ a = 0;
  lua_pushinteger(L, cast_Integer(L->മുകൾ.p - L->അട്ടി.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  തിരികെ 5;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ആകുമ്പോൾ (ci->next != ശൂന്യ) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ reallocstack (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->മുകൾ.p - L->അട്ടി.p) + n, 1);
  lua_unlock(L);
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ table_query (lua_State *L) {
  സ്ഥിരം Table *t;
  പൂർണ്ണസംഖ്യ i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned പൂർണ്ണസംഖ്യ asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  എങ്കിൽ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    തിരികെ 3;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    എങ്കിൽ (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->മുകൾ.p));
    അല്ലെങ്കിൽ
      setnilvalue2s(L->മുകൾ.p);
    api_incr_മുകൾ(L);
    lua_pushnil(L);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    എങ്കിൽ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    അല്ലെങ്കിൽ
      lua_pushliteral(L, "<undef>");
    എങ്കിൽ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    അല്ലെങ്കിൽ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->കണ്ണി[i]));
  }
  തിരികെ 3;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  തിരികെ 6;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  പൂർണ്ണസംഖ്യ s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  എങ്കിൽ (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    തിരികെ 2;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (s < tb->size) {
    TString *ts;
    പൂർണ്ണസംഖ്യ n = 0;
    വേണ്ടി (ts = tb->hash[s]; ts != ശൂന്യ; ts = ts->u.hnext) {
      setsvalue2s(L, L->മുകൾ.p, ts);
      api_incr_മുകൾ(L);
      n++;
    }
    തിരികെ n;
  }
  അല്ലെങ്കിൽ തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ getreftable (lua_State *L) {
  എങ്കിൽ (lua_istable(L, 2))  /* is there a table as second argument? */
    തിരികെ 2;  /* use it as the table */
  അല്ലെങ്കിൽ
    തിരികെ LUA_REGISTRYINDEX;  /* default is to use the register */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tref (lua_State *L) {
  പൂർണ്ണസംഖ്യ t = getreftable(L);
  പൂർണ്ണസംഖ്യ level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ getref (lua_State *L) {
  പൂർണ്ണസംഖ്യ t = getreftable(L);
  പൂർണ്ണസംഖ്യ level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ unref (lua_State *L) {
  പൂർണ്ണസംഖ്യ t = getreftable(L);
  പൂർണ്ണസംഖ്യ level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ upvalue (lua_State *L) {
  പൂർണ്ണസംഖ്യ n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  എങ്കിൽ (lua_isnone(L, 3)) {
    സ്ഥിരം അക്ഷരം *name = lua_getupvalue(L, 1, n);
    എങ്കിൽ (name == ശൂന്യ) തിരികെ 0;
    lua_pushstring(L, name);
    തിരികെ 2;
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    തിരികെ 1;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  പൂർണ്ണസംഖ്യ nuv = cast_int(luaL_optinteger(L, 2, 0));
  അക്ഷരം *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  ആകുമ്പോൾ (size--) *p++ = '\0';
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &l);
  പൂർണ്ണസംഖ്യ status = luaL_loadbufferx(L1, s, l, s, "t");
  എങ്കിൽ (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ s2d (lua_State *L) {
  lua_pushnumber(L, cast_അക്കം(*cast(സ്ഥിരം ഇരട്ട *, luaL_checkstring(L, 1))));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ d2s (lua_State *L) {
  ഇരട്ട d = cast(ഇരട്ട, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), വലുപ്പം(d));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ newstate (lua_State *L) {
  ശൂന്യം *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  എങ്കിൽ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  അല്ലെങ്കിൽ
    lua_pushnil(L);
  തിരികെ 1;
}


സ്ഥിതം lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ശൂന്യ, 1, "state expected");
  തിരികെ L1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  പൂർണ്ണസംഖ്യ load = cast_int(luaL_checkinteger(L, 2));
  പൂർണ്ണസംഖ്യ preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ശൂന്യ, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  സ്ഥിരം അക്ഷരം *code = luaL_checklstring(L, 2, &lcode);
  പൂർണ്ണസംഖ്യ status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  എങ്കിൽ (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  എങ്കിൽ (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    തിരികെ 3;
  }
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ i = 0;
    ആകുമ്പോൾ (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_എടുക്കുക(L1, i-1);
    തിരികെ i-1;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ log2_aux (lua_State *L) {
  unsigned പൂർണ്ണസംഖ്യ x = (unsigned പൂർണ്ണസംഖ്യ)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  തിരികെ 1;
}


ഘടന Aux { jmp_buf jb; സ്ഥിരം അക്ഷരം *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ panicback (lua_State *L) {
  ഘടന Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (ഘടന Aux *)lua_touserdata(L, -1);
  lua_എടുക്കുക(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  തിരികെ 1;  /* to avoid warnings */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ checkpanic (lua_State *L) {
  ഘടന Aux b;
  ശൂന്യം *ud;
  lua_State *L1;
  സ്ഥിരം അക്ഷരം *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  എങ്കിൽ (L1 == ശൂന്യ) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    തിരികെ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  എങ്കിൽ (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  അല്ലെങ്കിൽ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  തിരികെ 1;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ externKstr (lua_State *L) {
  size_t len;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ശൂന്യ, ശൂന്യ);
  തിരികെ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ externstr (lua_State *L) {
  size_t len;
  സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L, 1, &len);
  ശൂന്യം *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  അക്ഷരം *buff = cast_charp((*allocf)(ud, ശൂന്യ, 0, len + 1));
  എങ്കിൽ (buff == ശൂന്യ) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * വലുപ്പം(അക്ഷരം));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  തിരികെ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


സ്ഥിതം ശൂന്യം sethookaux (lua_State *L, പൂർണ്ണസംഖ്യ mask, പൂർണ്ണസംഖ്യ എണ്ണം, സ്ഥിരം അക്ഷരം *code);

സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം delimits = " \t\n,;";

സ്ഥിതം ശൂന്യം skip (സ്ഥിരം അക്ഷരം **pc) {
  വേണ്ടി (;;) {
    എങ്കിൽ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    അല്ലെങ്കിൽ എങ്കിൽ (**pc == '#') {  /* comment? */
      ആകുമ്പോൾ (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    അല്ലെങ്കിൽ നിർത്തുക;
  }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ getnum_aux (lua_State *L, lua_State *L1, സ്ഥിരം അക്ഷരം **pc) {
  പൂർണ്ണസംഖ്യ res = 0;
  പൂർണ്ണസംഖ്യ sig = 1;
  skip(pc);
  എങ്കിൽ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_എടുക്കുക(L1, 1);
    (*pc)++;
    തിരികെ res;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    തിരികെ res;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (**pc == '!') {
    (*pc)++;
    എങ്കിൽ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    അല്ലെങ്കിൽ എങ്കിൽ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    അല്ലെങ്കിൽ lua_assert(0);
    (*pc)++;
    തിരികെ res;
  }
  അല്ലെങ്കിൽ എങ്കിൽ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  എങ്കിൽ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ആകുമ്പോൾ (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  തിരികെ sig*res;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *getstring_aux (lua_State *L, അക്ഷരം *buff, സ്ഥിരം അക്ഷരം **pc) {
  പൂർണ്ണസംഖ്യ i = 0;
  skip(pc);
  എങ്കിൽ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    പൂർണ്ണസംഖ്യ quote = *(*pc)++;
    ആകുമ്പോൾ (**pc != quote) {
      എങ്കിൽ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  അല്ലെങ്കിൽ {
    ആകുമ്പോൾ (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  തിരികെ buff;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ getindex_aux (lua_State *L, lua_State *L1, സ്ഥിരം അക്ഷരം **pc) {
  skip(pc);
  തിരഞ്ഞെടുപ്പ് (*(*pc)++) {
    അവസ്ഥ 'R': തിരികെ LUA_REGISTRYINDEX;
    അവസ്ഥ 'U': തിരികെ lua_upvalueindex(getnum_aux(L, L1, pc));
    സ്ഥിരസ്ഥിതി: {
      പൂർണ്ണസംഖ്യ n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      എങ്കിൽ (n == 0) തിരികെ 0;
      അല്ലെങ്കിൽ തിരികെ lua_absindex(L1, n);
    }
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
സ്ഥിതം ശൂന്യം regcodes (lua_State *L) {
  unsigned പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < വലുപ്പം(statcodes) / വലുപ്പം(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


സ്ഥിതം പൂർണ്ണസംഖ്യ testC (lua_State *L);
സ്ഥിതം പൂർണ്ണസംഖ്യ Cfunck (lua_State *L, പൂർണ്ണസംഖ്യ status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം ops[] = "+-*%^/\\&|~<>_!";

സ്ഥിതം പൂർണ്ണസംഖ്യ runC (lua_State *L, lua_State *L1, സ്ഥിരം അക്ഷരം *pc) {
  അക്ഷരം buff[300];
  പൂർണ്ണസംഖ്യ status = 0;
  എങ്കിൽ (pc == ശൂന്യ) തിരികെ luaL_error(L, "attempt to runC null script");
  വേണ്ടി (;;) {
    സ്ഥിരം അക്ഷരം *inst = getstring;
    എങ്കിൽ EQ("") തിരികെ 0;
    അല്ലെങ്കിൽ എങ്കിൽ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("append") {
      പൂർണ്ണസംഖ്യ t = getindex;
      പൂർണ്ണസംഖ്യ i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("arith") {
      പൂർണ്ണസംഖ്യ op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("call") {
      പൂർണ്ണസംഖ്യ narg = getnum;
      പൂർണ്ണസംഖ്യ nres = getnum;
      lua_call(L1, narg, nres);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("callk") {
      പൂർണ്ണസംഖ്യ narg = getnum;
      പൂർണ്ണസംഖ്യ nres = getnum;
      പൂർണ്ണസംഖ്യ i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("checkstack") {
      പൂർണ്ണസംഖ്യ sz = getnum;
      സ്ഥിരം അക്ഷരം *സന്ദേശം = getstring;
      എങ്കിൽ (*സന്ദേശം == '\0')
        സന്ദേശം = ശൂന്യ;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, സന്ദേശം);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rawcheckstack") {
      പൂർണ്ണസംഖ്യ sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("compare") {
      സ്ഥിരം അക്ഷരം *opt = getstring;  /* EQ, LT, or LE */
      പൂർണ്ണസംഖ്യ op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      പൂർണ്ണസംഖ്യ a = getindex;
      പൂർണ്ണസംഖ്യ b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("concat") {
      lua_concat(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("copy") {
      പൂർണ്ണസംഖ്യ f = getindex;
      lua_copy(L1, f, getindex);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("getfield") {
      പൂർണ്ണസംഖ്യ t = getindex;
      പൂർണ്ണസംഖ്യ tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("getmetatable") {
      എങ്കിൽ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("gettable") {
      പൂർണ്ണസംഖ്യ tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("gsub") {
      പൂർണ്ണസംഖ്യ a = getnum; പൂർണ്ണസംഖ്യ b = getnum; പൂർണ്ണസംഖ്യ c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("insert") {
      lua_തിരുകുക(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("len") {
      lua_len(L1, getindex);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("loadstring") {
      size_t slen;
      സ്ഥിരം അക്ഷരം *s = luaL_checklstring(L1, getnum, &slen);
      സ്ഥിരം അക്ഷരം *name = getstring;
      സ്ഥിരം അക്ഷരം *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("newtable") {
      lua_newtable(L1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("newthread") {
      lua_newthread(L1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("next") {
      lua_next(L1, -2);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pcall") {
      പൂർണ്ണസംഖ്യ narg = getnum;
      പൂർണ്ണസംഖ്യ nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pcallk") {
      പൂർണ്ണസംഖ്യ narg = getnum;
      പൂർണ്ണസംഖ്യ nres = getnum;
      പൂർണ്ണസംഖ്യ i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pop") {
      lua_എടുക്കുക(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("printstack") {
      പൂർണ്ണസംഖ്യ n = getnum;
      എങ്കിൽ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      അല്ലെങ്കിൽ lua_printstack(L1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("print") {
      സ്ഥിരം അക്ഷരം *സന്ദേശം = getstring;
      printf("%s\n", സന്ദേശം);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("warningC") {
      സ്ഥിരം അക്ഷരം *സന്ദേശം = getstring;
      lua_warning(L1, സന്ദേശം, 1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("warning") {
      സ്ഥിരം അക്ഷരം *സന്ദേശം = getstring;
      lua_warning(L1, സന്ദേശം, 0);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushnil") {
      lua_pushnil(L1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushnum") {
      lua_pushnumber(L1, (lua_സംഖ്യ)getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (പൂർണ്ണസംഖ്യ)lua_tointeger(L, -1));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rawget") {
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_rawget(L1, t);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rawgeti") {
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rawgetp") {
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rawset") {
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_rawset(L1, t);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rawseti") {
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rawsetp") {
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("remove") {
      lua_remove(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("replace") {
      lua_replace(L1, getindex);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("resume") {
      പൂർണ്ണസംഖ്യ i = getindex;
      പൂർണ്ണസംഖ്യ nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("traceback") {
      സ്ഥിരം അക്ഷരം *സന്ദേശം = getstring;
      പൂർണ്ണസംഖ്യ level = getnum;
      luaL_traceback(L1, L1, സന്ദേശം, level);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("return") {
      പൂർണ്ണസംഖ്യ n = getnum;
      എങ്കിൽ (L1 != L) {
        പൂർണ്ണസംഖ്യ i;
        വേണ്ടി (i = 0; i < n; i++) {
          പൂർണ്ണസംഖ്യ idx = -(n - i);
          തിരഞ്ഞെടുപ്പ് (lua_type(L1, idx)) {
            അവസ്ഥ LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              നിർത്തുക;
            സ്ഥിരസ്ഥിതി:
              lua_pushstring(L, lua_tostring(L1, idx));
              നിർത്തുക;
          }
        }
      }
      തിരികെ n;
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("rotate") {
      പൂർണ്ണസംഖ്യ i = getindex;
      lua_rotate(L1, i, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("setfield") {
      പൂർണ്ണസംഖ്യ t = getindex;
      സ്ഥിരം അക്ഷരം *s = getstring;
      lua_setfield(L1, t, s);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("seti") {
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_seti(L1, t, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("setglobal") {
      സ്ഥിരം അക്ഷരം *s = getstring;
      lua_setglobal(L1, s);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("sethook") {
      പൂർണ്ണസംഖ്യ mask = getnum;
      പൂർണ്ണസംഖ്യ എണ്ണം = getnum;
      സ്ഥിരം അക്ഷരം *s = getstring;
      sethookaux(L1, mask, എണ്ണം, s);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("setmetatable") {
      പൂർണ്ണസംഖ്യ idx = getindex;
      lua_setmetatable(L1, idx);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("settable") {
      lua_settable(L1, getindex);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("settop") {
      lua_settop(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("testudata") {
      പൂർണ്ണസംഖ്യ i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ശൂന്യ);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("error") {
      lua_error(L1);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("abort") {
      abort();
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("throw") {
#എങ്കിൽ defined(__cplusplus)
സ്ഥിതം ഘടന X { പൂർണ്ണസംഖ്യ x; } x;
      throw x;
#അല്ലെങ്കിൽ
      luaL_error(L1, "C++");
#endif
      നിർത്തുക;
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("tostring") {
      സ്ഥിരം അക്ഷരം *s = lua_tostring(L1, getindex);
      സ്ഥിരം അക്ഷരം *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ശൂന്യ && s1 == ശൂന്യ) || strcmp(s, s1) == 0);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ശൂന്യ);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("xmove") {
      പൂർണ്ണസംഖ്യ f = getindex;
      പൂർണ്ണസംഖ്യ t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      പൂർണ്ണസംഖ്യ n = getnum;
      എങ്കിൽ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("yield") {
      തിരികെ lua_yield(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("yieldk") {
      പൂർണ്ണസംഖ്യ nres = getnum;
      പൂർണ്ണസംഖ്യ i = getindex;
      തിരികെ lua_yieldk(L1, nres, i, Cfunck);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    അല്ലെങ്കിൽ എങ്കിൽ EQ("argerror") {
      പൂർണ്ണസംഖ്യ arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    അല്ലെങ്കിൽ luaL_error(L, "unknown instruction %s", buff);
  }
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ testC (lua_State *L) {
  lua_State *L1;
  സ്ഥിരം അക്ഷരം *pc;
  എങ്കിൽ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  അല്ലെങ്കിൽ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  തിരികെ runC(L, L1, pc);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ Cfunc (lua_State *L) {
  തിരികെ runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ Cfunck (lua_State *L, പൂർണ്ണസംഖ്യ status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  തിരികെ runC(L, L, lua_tostring(L, cast_int(ctx)));
}


സ്ഥിതം പൂർണ്ണസംഖ്യ makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  തിരികെ 1;
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
സ്ഥിതം ശൂന്യം Chook (lua_State *L, lua_Debug *ar) {
  സ്ഥിരം അക്ഷരം *scpt;
  സ്ഥിരം അക്ഷരം *സ്ഥിരം events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_എടുക്കുക(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
സ്ഥിതം ശൂന്യം sethookaux (lua_State *L, പൂർണ്ണസംഖ്യ mask, പൂർണ്ണസംഖ്യ എണ്ണം, സ്ഥിരം അക്ഷരം *scpt) {
  എങ്കിൽ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ശൂന്യ, 0, 0);  /* turn off hooks */
    തിരികെ;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  എങ്കിൽ (!lua_istable(L, -1)) {  /* no hook table? */
    lua_എടുക്കുക(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, എണ്ണം);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ sethook (lua_State *L) {
  എങ്കിൽ (lua_isnoneornil(L, 1))
    lua_sethook(L, ശൂന്യ, 0, 0);  /* turn off hooks */
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *scpt = luaL_checkstring(L, 1);
    സ്ഥിരം അക്ഷരം *smask = luaL_checkstring(L, 2);
    പൂർണ്ണസംഖ്യ എണ്ണം = cast_int(luaL_optinteger(L, 3, 0));
    പൂർണ്ണസംഖ്യ mask = 0;
    എങ്കിൽ (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    എങ്കിൽ (strchr(smask, 'r')) mask |= LUA_MASKRET;
    എങ്കിൽ (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    എങ്കിൽ (എണ്ണം > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, എണ്ണം, scpt);
  }
  തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ coresume (lua_State *L) {
  പൂർണ്ണസംഖ്യ status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  എങ്കിൽ (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_തിരുകുക(L, -2);
    തിരികെ 2;  /* return false + error message */
  }
  അല്ലെങ്കിൽ {
    lua_pushboolean(L, 1);
    തിരികെ 1;
  }
}

#എങ്കിൽ !defined(LUA_USE_POSIX)

#define nonblock	ശൂന്യ

#അല്ലെങ്കിൽ

#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <fcntl.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  പൂർണ്ണസംഖ്യ fd = fileno(f);
  പൂർണ്ണസംഖ്യ flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  തിരികെ 0;
}
#endif

/* }====================================================== */



സ്ഥിതം സ്ഥിരം ഘടന luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_എണ്ണം},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {ശൂന്യ, ശൂന്യ}
};


സ്ഥിതം ശൂന്യം checkfinalmem (ശൂന്യം) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


പൂർണ്ണസംഖ്യ luaB_opentests (lua_State *L) {
  ശൂന്യം *ud;
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
  തിരികെ 1;
}

#endif

