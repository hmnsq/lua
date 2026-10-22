/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#define ltests_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <setjmp.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "lauxlib.h"
#شامل "lcode.h"
#شامل "lctype.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lmem.h"
#شامل "lopcodes.h"
#شامل "lopnames.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#اگر defined(LUA_DEBUG)


تهی *l_Trick = 0;


#define obj_at(L,k)	s2v(L->ci->func.p + (k))


ایستا صحیح runC (lua_State *L, lua_State *L1, ثابت نویسه *pc);


ایستا تهی setnameval (lua_State *L, ثابت نویسه *name, صحیح val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


ایستا تهی pushobject (lua_State *L, ثابت TValue *o) {
  setobj2s(L, L->بالاترین.p, o);
  api_incr_بالاترین(L);
}


ایستا تهی badexit (ثابت نویسه *fmt, ثابت نویسه *s1, ثابت نویسه *s2) {
  fprintf(stderr, fmt, s1);
  اگر (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.total = 0;
  exit(EXIT_FAILURE);
}


ایستا صحیح tpanic (lua_State *L) {
  ثابت نویسه *پیام = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  بازگشت (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   پیام, پوچ),
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
ایستا تهی warnf (تهی *ud, ثابت نویسه *پیام, صحیح tocont) {
  lua_State *L = cast(lua_State *, ud);
  ایستا نویسه buff[200] = "";  /* should be enough for tests... */
  ایستا صحیح onoff = 0;
  ایستا صحیح mode = 0;  /* start in normal mode */
  ایستا صحیح lasttocont = 0;
  اگر (!lasttocont && !tocont && *پیام == '@') {  /* control message? */
    اگر (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", پیام, buff);
    اگر (strcmp(پیام, "@off") == 0)
      onoff = 0;
    وگرنه اگر (strcmp(پیام, "@on") == 0)
      onoff = 1;
    وگرنه اگر (strcmp(پیام, "@normal") == 0)
      mode = 0;
    وگرنه اگر (strcmp(پیام, "@allow") == 0)
      mode = 1;
    وگرنه اگر (strcmp(پیام, "@store") == 0)
      mode = 2;
    وگرنه
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              پیام, پوچ);
    بازگشت;
  }
  lasttocont = tocont;
  اگر (strlen(پیام) >= اندازه(buff) - strlen(buff))
    badexit("warnf-buffer overflow (%s)\n", پیام, buff);
  strcat(buff, پیام);  /* add new message to current warning */
  اگر (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    اگر (!lua_toboolean(L, -1))
      lua_برداشت(L, 1);  /* ok, no previous unexpected warning */
    وگرنه {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    انتخاب (mode) {
      حالت 0: {  /* normal */
        اگر (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, پوچ);
      }  /* FALLTHROUGH */
      حالت 1: {  /* allow */
        اگر (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        بشکن;
      }
      حالت 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        بشکن;
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

تعریفنوع اجتماع memHeader {
  LUAI_MAXALIGN;
  ساختار {
    size_t size;
    صحیح type;
  } d;
} memHeader;


#اگر !defined(EXTERNMEMCHECK)

/* full memory check */
#define MARKSIZE	16  /* size of marks after each block */
#define fillmem(mem,size)	memset(mem, -MARK, size)

#وگرنه

/* external memory check: don't do it twice */
#define MARKSIZE	0
#define fillmem(mem,size)	/* empty */

#endif


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


ایستا تهی freeblock (Memcontrol *mc, memHeader *block) {
  اگر (block) {
    size_t size = block->d.size;
    صحیح i;
    برای (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + size + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, اندازه(memHeader) + size + MARKSIZE);  /* erase block */
    آزاد(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->total -= size;
  }
}


تهی *debug_realloc (تهی *ud, تهی *b, size_t oldsize, size_t size) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  صحیح type;
  اگر (mc->memlimit == 0) {  /* first time? */
    نویسه *limit = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? strtoul(limit, پوچ, 10) : ULONG_MAX;
  }
  اگر (block == پوچ) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  وگرنه {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.size);
  }
  اگر (size == 0) {
    freeblock(mc, block);
    بازگشت پوچ;
  }
  اگر (mc->failnext) {
    mc->failnext = 0;
    بازگشت پوچ;  /* fake a single memory allocation error */
  }
  اگر (mc->countlimit != ~0UL && size != oldsize) {  /* count limit in use? */
    اگر (mc->countlimit == 0)
      بازگشت پوچ;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  اگر (size > oldsize && mc->total+size-oldsize > mc->memlimit)
    بازگشت پوچ;  /* fake a memory allocation error */
  وگرنه {
    memHeader *newblock;
    صحیح i;
    size_t commonsize = (oldsize < size) ? oldsize : size;
    size_t realsize = اندازه(memHeader) + size + MARKSIZE;
    اگر (realsize < size) بازگشت پوچ;  /* arithmetic overflow! */
    newblock = cast(memHeader *, malloc(realsize));  /* alloc a new block */
    اگر (newblock == پوچ)
      بازگشت پوچ;  /* really out of memory? */
    اگر (block) {
      memcpy(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, size - commonsize);
    /* initialize marks after block */
    برای (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + size + i) = MARK;
    newblock->d.size = size;
    newblock->d.type = type;
    mc->total += size;
    اگر (mc->total > mc->maxmem)
      mc->maxmem = mc->total;
    mc->numblocks++;
    mc->objcount[type]++;
    بازگشت newblock + 1;
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

#شامل <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
ایستا صحیح testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  اگر (isdead(g,t)) بازگشت 0;
  اگر (issweepphase(g))
    بازگشت 1;  /* no invariants */
  وگرنه اگر (g->gckind != KGC_GENMINOR)
    بازگشت !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  وگرنه {  /* generational mode */
    اگر ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      بازگشت 0;
    اگر ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      بازگشت 0;
    بازگشت 1;
  }
}


ایستا تهی printobj (global_State *g, GCObject *o) {
  printf("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (تهی *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  اگر (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    printf(" '%s'", getstr(gco2ts(o)));
}


تهی lua_printobj (lua_State *L, ساختار GCObject *o) {
  printobj(G(L), o);
}


تهی lua_printvalue (TValue *v) {
  انتخاب (ttypetag(v)) {
    حالت LUA_VNUMINT: حالت LUA_VNUMFLT: {
      نویسه buff[LUA_N2SBUFFSZ];
      unsigned len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      printf("%s", buff);
      بشکن;
    }
    حالت LUA_VSHRSTR:
      printf("'%s'", getstr(tsvalue(v))); بشکن;
    حالت LUA_VLNGSTR:
      printf("'%.30s...'", getstr(tsvalue(v))); بشکن;
    حالت LUA_VFALSE:
      printf("%s", "false"); بشکن;
    حالت LUA_VTRUE:
      printf("%s", "true"); بشکن;
    حالت LUA_VLIGHTUSERDATA:
      printf("light udata: %p", pvalue(v)); بشکن;
    حالت LUA_VUSERDATA:
      printf("full udata: %p", uvalue(v)); بشکن;
    حالت LUA_VNIL:
      printf("nil"); بشکن;
    حالت LUA_VLCF:
      printf("light C function: %p", fvalue(v)); بشکن;
    حالت LUA_VCCL:
      printf("C closure: %p", clCvalue(v)); بشکن;
    حالت LUA_VLCL:
      printf("Lua function: %p", clLvalue(v)); بشکن;
    حالت LUA_VTHREAD:
      printf("thread: %p", thvalue(v)); بشکن;
    حالت LUA_VTABLE:
      printf("table: %p", hvalue(v)); بشکن;
    پیشفرض:
      lua_assert(0);
  }
}


ایستا صحیح testobjref (global_State *g, GCObject *f, GCObject *t) {
  صحیح r1 = testobjref1(g, f, t);
  اگر (!r1) {
    printf("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    printf("  ->  ");
    printobj(g, t);
    printf("\n");
  }
  بازگشت r1;
}


ایستا تهی checkobjref (global_State *g, GCObject *f, GCObject *t) {
    أكد(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#define checkobjrefN(g,f,t)	{ اگر (t) checkobjref(g,f,obj2gco(t)); }


ایستا تهی checkvalref (global_State *g, GCObject *f, ثابت TValue *t) {
  أكد(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


ایستا تهی checktable (global_State *g, Table *h) {
  unsigned صحیح i;
  unsigned صحیح asize = h->asize;
  گره_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  برای (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  برای (n = gnode(h, 0); n < limit; n++) {
    اگر (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      أكد(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


ایستا تهی checkudata (global_State *g, Udata *u) {
  صحیح i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  برای (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


ایستا تهی checkproto (global_State *g, Proto *f) {
  صحیح i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  برای (i=0; i<f->sizek; i++) {
    اگر (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  برای (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  برای (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  برای (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


ایستا تهی checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  صحیح i;
  برای (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


ایستا تهی checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  صحیح i;
  checkobjrefN(g, clgc, cl->p);
  برای (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    اگر (uv) {
      checkobjrefN(g, clgc, uv);
      اگر (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


ایستا صحیح lua_checkpc (CallInfo *ci) {
  اگر (!isLua(ci)) بازگشت 1;
  وگرنه {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    بازگشت p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


ایستا تهی check_پشته (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  أكد(!isdead(g, L1));
  اگر (L1->پشته.p == پوچ) {  /* incomplete thread? */
    أكد(L1->openupval == پوچ && L1->ci == پوچ);
    بازگشت;
  }
  برای (uv = L1->openupval; uv != پوچ; uv = uv->u.open.next)
    أكد(upisopen(uv));  /* must be open */
  أكد(L1->بالاترین.p <= L1->پشته_last.p);
  أكد(L1->tbclist.p <= L1->بالاترین.p);
  برای (ci = L1->ci; ci != پوچ; ci = ci->previous) {
    أكد(ci->بالاترین.p <= L1->پشته_last.p);
    أكد(lua_checkpc(ci));
  }
  برای (o = L1->پشته.p; o < L1->پشته_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


ایستا تهی checkrefs (global_State *g, GCObject *o) {
  انتخاب (o->tt) {
    حالت LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      بشکن;
    }
    حالت LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      بشکن;
    }
    حالت LUA_VTABLE: {
      checktable(g, gco2t(o));
      بشکن;
    }
    حالت LUA_VTHREAD: {
      check_پشته(g, gco2th(o));
      بشکن;
    }
    حالت LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      بشکن;
    }
    حالت LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      بشکن;
    }
    حالت LUA_VPROTO: {
      checkproto(g, gco2p(o));
      بشکن;
    }
    حالت LUA_VSHRSTR:
    حالت LUA_VLNGSTR: {
      أكد(!isgray(o));  /* strings are never gray */
      بشکن;
    }
    پیشفرض: أكد(0);
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
ایستا تهی checkobject (global_State *g, GCObject *o, صحیح maybedead,
                         صحیح listage) {
  اگر (isdead(g, o))
    أكد(maybedead);
  وگرنه {
    أكد(g->gcstate != GCSpause || iswhite(o));
    اگر (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      أكد(getage(o) >= listage);
      اگر (isold(o)) {
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


ایستا l_mem checkgraylist (global_State *g, GCObject *o) {
  صحیح total = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  وقتی (o) {
    أكد(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    أكد(!testbit(o->marked, TESTBIT));
    اگر (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    total++;
    انتخاب (o->tt) {
      حالت LUA_VTABLE: o = gco2t(o)->gclist; بشکن;
      حالت LUA_VLCL: o = gco2lcl(o)->gclist; بشکن;
      حالت LUA_VCCL: o = gco2ccl(o)->gclist; بشکن;
      حالت LUA_VTHREAD: o = gco2th(o)->gclist; بشکن;
      حالت LUA_VPROTO: o = gco2p(o)->gclist; بشکن;
      حالت LUA_VUSERDATA:
        أكد(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        بشکن;
      پیشفرض: أكد(0);  /* other objects cannot be in a gray list */
    }
  }
  بازگشت total;
}


/*
** Check objects in gray lists.
*/
ایستا l_mem checkgrays (global_State *g) {
  l_mem total = 0;  /* count number of elements in all lists */
  اگر (!keepinvariant(g)) بازگشت total;
  total += checkgraylist(g, g->gray);
  total += checkgraylist(g, g->grayagain);
  total += checkgraylist(g, g->weak);
  total += checkgraylist(g, g->allweak);
  total += checkgraylist(g, g->ephemeron);
  بازگشت total;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
ایستا تهی incifingray (global_State *g, GCObject *o, l_mem *شمار) {
  اگر (!keepinvariant(g))
    بازگشت;  /* gray lists not being kept in these phases */
  اگر (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    أكد(!isgray(o) || upisopen(gco2upv(o)));
    بازگشت;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  اگر (isgray(o) || getage(o) == G_TOUCHED2) {
    (*شمار)++;
    أكد(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


ایستا l_mem checklist (global_State *g, صحیح maybedead, صحیح tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem total = 0;  /* number of object that should be in  gray lists */
  برای (o = newl; o != survival; o = o->next) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  برای (o = survival; o != old; o = o->next) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  برای (o = old; o != reallyold; o = o->next) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  برای (o = reallyold; o != پوچ; o = o->next) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &total);
    أكد(!tof == !tofinalize(o));
  }
  بازگشت total;
}


صحیح lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  صحیح maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  اگر (keepinvariant(g)) {
    أكد(!iswhite(mainthread(g)));
    أكد(!iswhite(gcvalue(&g->l_registry)));
  }
  أكد(!isdead(g, gcvalue(&g->l_registry)));
  أكد(g->sweepgc == پوچ || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  برای (o = g->fixedgc; o != پوچ; o = o->next) {
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
  برای (o = g->tobefnz; o != پوچ; o = o->next) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    أكد(tofinalize(o));
    أكد(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  اگر (keepinvariant(g))
    أكد(totalin == totalshould);
  بازگشت 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


ایستا نویسه *buildop (Proto *p, صحیح pc, نویسه *buff) {
  نویسه *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  ثابت نویسه *name = opnames[o];
  صحیح سطر = luaG_getfuncline(p, pc);
  صحیح lineinfo = (p->lineinfo != پوچ) ? p->lineinfo[pc] : 0;
  اگر (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  وگرنه
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", سطر, pc);
  انتخاب (getOpMode(o)) {
    حالت iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      بشکن;
    حالت ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      بشکن;
    حالت iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      بشکن;
    حالت iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      بشکن;
    حالت iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      بشکن;
    حالت isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      بشکن;
  }
  بازگشت obuff;
}


#اگر 0
تهی luaI_printcode (Proto *pt, صحیح size) {
  صحیح pc;
  برای (pc=0; pc<size; pc++) {
    نویسه buff[100];
    printf("%s\n", buildop(pt, pc, buff));
  }
  printf("-------\n");
}


تهی luaI_printinst (Proto *pt, صحیح pc) {
  نویسه buff[100];
  printf("%s\n", buildop(pt, pc, buff));
}
#endif


ایستا صحیح listcode (lua_State *L) {
  صحیح pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  برای (pc=0; pc<p->sizecode; pc++) {
    نویسه buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  بازگشت 1;
}


ایستا صحیح printcode (lua_State *L) {
  صحیح pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  printf("maxstack: %d\n", p->maxstacksize);
  printf("numparams: %d\n", p->numparams);
  برای (pc=0; pc<p->sizecode; pc++) {
    نویسه buff[100];
    printf("%s\n", buildop(p, pc, buff));
  }
  بازگشت 0;
}


ایستا صحیح listk (lua_State *L) {
  Proto *p;
  صحیح i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  برای (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  بازگشت 1;
}


ایستا صحیح listabslineinfo (lua_State *L) {
  Proto *p;
  صحیح i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != پوچ, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  برای (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].سطر);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  بازگشت 1;
}


ایستا صحیح listlocals (lua_State *L) {
  Proto *p;
  صحیح pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  صحیح i = 0;
  ثابت نویسه *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  وقتی ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != پوچ)
    lua_pushstring(L, name);
  بازگشت i-1;
}

/* }====================================================== */



تهی lua_printstack (lua_State *L) {
  صحیح i;
  صحیح n = lua_gettop(L);
  printf("stack: >>\n");
  برای (i = 1; i <= n; i++) {
    printf("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    printf("\n");
  }
  printf("<<\n");
}


صحیح lua_printallstack (lua_State *L) {
  StkId p;
  صحیح i = 1;
  CallInfo *ci = &L->base_ci;
  printf("stack: >>\n");
  برای (p = L->پشته.p; p < L->بالاترین.p; p++) {
    اگر (ci != پوچ && p == ci->func.p) {
      printf("  ---\n");
      اگر (ci == L->ci)
        ci = پوچ;  /* printed last frame */
      وگرنه
        ci = ci->next;
    }
    printf("%3d: ", i++);
    lua_printvalue(s2v(p));
    printf("\n");
  }
  printf("<<\n");
  بازگشت 0;
}


ایستا صحیح get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", رقم_OPCODES);
  بازگشت 1;
}


ایستا صحیح get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", اندازه(lua_State));
  setnameval(L, "global state", اندازه(global_State));
  setnameval(L, "TValue", اندازه(TValue));
  setnameval(L, "Node", اندازه(گره_2));
  setnameval(L, "stack Value", اندازه(پشته_مقدار));
  بازگشت 1;
}


ایستا صحیح mem_query (lua_State *L) {
  اگر (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.total));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    بازگشت 3;
  }
  وگرنه اگر (lua_isnumber(L, 1)) {
    unsigned long limit = cast(unsigned long, luaL_checkinteger(L, 1));
    اگر (limit == 0) limit = ULONG_MAX;
    l_memcontrol.memlimit = limit;
    بازگشت 0;
  }
  وگرنه {
    ثابت نویسه *t = luaL_checkstring(L, 1);
    صحیح i;
    برای (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      اگر (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        بازگشت 1;
      }
    }
    بازگشت luaL_error(L, "unknown type '%s'", t);
  }
}


ایستا صحیح alloc_شمار (lua_State *L) {
  اگر (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(unsigned long, ~0L);
  وگرنه
    l_memcontrol.countlimit = cast(unsigned long, luaL_checkinteger(L, 1));
  بازگشت 0;
}


ایستا صحیح alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  بازگشت 0;
}


ایستا صحیح settrick (lua_State *L) {
  اگر (ttisnil(obj_at(L, 1)))
    l_Trick = پوچ;
  وگرنه
    l_Trick = gcvalue(obj_at(L, 1));
  بازگشت 0;
}


ایستا صحیح gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  اگر (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  وگرنه {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  بازگشت 1;
}


ایستا صحیح gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  اگر (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  وگرنه {
    ایستا ثابت نویسه *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  بازگشت 1;
}


ایستا صحیح gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  اگر (!iscollectable(o))
    printf("no collectable\n");
  وگرنه {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    printf("\n");
  }
  بازگشت 0;
}


ایستا ثابت نویسه *ثابت statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

ایستا صحیح gc_state (lua_State *L) {
  ایستا ثابت صحیح states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  صحیح option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  اگر (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    بازگشت 1;
  }
  وگرنه {
    اگر (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    اگر (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    بازگشت 0;
  }
}


ایستا صحیح tracinggc = 0;
تهی luai_tracegctest (lua_State *L, صحیح first) {
  اگر (!tracinggc) بازگشت;
  وگرنه {
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


ایستا صحیح tracegc (lua_State *L) {
  اگر (lua_isnil(L, 1))
    tracinggc = 0;
  وگرنه {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  بازگشت 0;
}


ایستا صحیح hash_query (lua_State *L) {
  اگر (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    اگر (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  وگرنه {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->گره));
  }
  بازگشت 1;
}


ایستا صحیح stacklevel (lua_State *L) {
  صحیح a = 0;
  lua_pushinteger(L, cast_Integer(L->بالاترین.p - L->پشته.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  بازگشت 5;
}


ایستا صحیح resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  وقتی (ci->next != پوچ) {
    CallInfo *tofree = ci->next;
    ci->next = ci->next->next;
    luaM_free(L, tofree);
    L->nci--;
  }
  بازگشت 0;
}


ایستا صحیح reallocstack (lua_State *L) {
  صحیح n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->بالاترین.p - L->پشته.p) + n, 1);
  lua_unlock(L);
  بازگشت 0;
}


ایستا صحیح table_query (lua_State *L) {
  ثابت Table *t;
  صحیح i = cast_int(luaL_optinteger(L, 2, -1));
  unsigned صحیح asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  اگر (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    بازگشت 3;
  }
  وگرنه اگر (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    اگر (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->بالاترین.p));
    وگرنه
      setnilvalue2s(L->بالاترین.p);
    api_incr_بالاترین(L);
    lua_pushnil(L);
  }
  وگرنه اگر (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    اگر (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    وگرنه
      lua_pushliteral(L, "<undef>");
    اگر (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    وگرنه
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->گره[i]));
  }
  بازگشت 3;
}


ایستا صحیح gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  بازگشت 6;
}


ایستا صحیح test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  بازگشت 1;
}


ایستا صحیح test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  بازگشت 1;
}


ایستا صحیح string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  صحیح s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  اگر (s == -1) {
    lua_pushinteger(L ,tb->size);
    lua_pushinteger(L ,tb->nuse);
    بازگشت 2;
  }
  وگرنه اگر (s < tb->size) {
    TString *ts;
    صحیح n = 0;
    برای (ts = tb->hash[s]; ts != پوچ; ts = ts->u.hnext) {
      setsvalue2s(L, L->بالاترین.p, ts);
      api_incr_بالاترین(L);
      n++;
    }
    بازگشت n;
  }
  وگرنه بازگشت 0;
}


ایستا صحیح getreftable (lua_State *L) {
  اگر (lua_istable(L, 2))  /* is there a table as second argument? */
    بازگشت 2;  /* use it as the table */
  وگرنه
    بازگشت LUA_REGISTRYINDEX;  /* default is to use the register */
}


ایستا صحیح tref (lua_State *L) {
  صحیح t = getreftable(L);
  صحیح level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  بازگشت 1;
}


ایستا صحیح getref (lua_State *L) {
  صحیح t = getreftable(L);
  صحیح level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  بازگشت 1;
}

ایستا صحیح unref (lua_State *L) {
  صحیح t = getreftable(L);
  صحیح level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  بازگشت 0;
}


ایستا صحیح upvalue (lua_State *L) {
  صحیح n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  اگر (lua_isnone(L, 3)) {
    ثابت نویسه *name = lua_getupvalue(L, 1, n);
    اگر (name == پوچ) بازگشت 0;
    lua_pushstring(L, name);
    بازگشت 2;
  }
  وگرنه {
    ثابت نویسه *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    بازگشت 1;
  }
}


ایستا صحیح newuserdata (lua_State *L) {
  size_t size = cast_sizet(luaL_optinteger(L, 1, 0));
  صحیح nuv = cast_int(luaL_optinteger(L, 2, 0));
  نویسه *p = cast_charp(lua_newuserdatauv(L, size, nuv));
  وقتی (size--) *p++ = '\0';
  بازگشت 1;
}


ایستا صحیح pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  بازگشت 1;
}


ایستا صحیح udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  بازگشت 1;
}


ایستا صحیح doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ثابت نویسه *s = luaL_checklstring(L, 1, &l);
  صحیح status = luaL_loadbufferx(L1, s, l, s, "t");
  اگر (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  بازگشت 1;
}


ایستا صحیح s2d (lua_State *L) {
  lua_pushnumber(L, cast_رقم(*cast(ثابت دوگانه *, luaL_checkstring(L, 1))));
  بازگشت 1;
}


ایستا صحیح d2s (lua_State *L) {
  دوگانه d = cast(دوگانه, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), اندازه(d));
  بازگشت 1;
}


ایستا صحیح num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  بازگشت 1;
}


ایستا صحیح makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  بازگشت 1;
}


ایستا صحیح newstate (lua_State *L) {
  تهی *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  اگر (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  وگرنه
    lua_pushnil(L);
  بازگشت 1;
}


ایستا lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != پوچ, 1, "state expected");
  بازگشت L1;
}


ایستا صحیح loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  صحیح load = cast_int(luaL_checkinteger(L, 2));
  صحیح preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", پوچ, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  بازگشت 0;
}

ایستا صحیح closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  بازگشت 0;
}

ایستا صحیح doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ثابت نویسه *code = luaL_checklstring(L, 2, &lcode);
  صحیح status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  اگر (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  اگر (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    بازگشت 3;
  }
  وگرنه {
    صحیح i = 0;
    وقتی (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_برداشت(L1, i-1);
    بازگشت i-1;
  }
}


ایستا صحیح log2_aux (lua_State *L) {
  unsigned صحیح x = (unsigned صحیح)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  بازگشت 1;
}


ساختار Aux { jmp_buf jb; ثابت نویسه *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
ایستا صحیح panicback (lua_State *L) {
  ساختار Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (ساختار Aux *)lua_touserdata(L, -1);
  lua_برداشت(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  بازگشت 1;  /* to avoid warnings */
}

ایستا صحیح checkpanic (lua_State *L) {
  ساختار Aux b;
  تهی *ud;
  lua_State *L1;
  ثابت نویسه *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  اگر (L1 == پوچ) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    بازگشت 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  اگر (setjmp(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  وگرنه {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  بازگشت 1;
}


ایستا صحیح externKstr (lua_State *L) {
  size_t len;
  ثابت نویسه *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, پوچ, پوچ);
  بازگشت 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
ایستا صحیح externstr (lua_State *L) {
  size_t len;
  ثابت نویسه *s = luaL_checklstring(L, 1, &len);
  تهی *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  نویسه *buff = cast_charp((*allocf)(ud, پوچ, 0, len + 1));
  اگر (buff == پوچ) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  memcpy(buff, s, (len + 1) * اندازه(نویسه));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  بازگشت 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


ایستا تهی sethookaux (lua_State *L, صحیح mask, صحیح شمار, ثابت نویسه *code);

ایستا ثابت نویسه *ثابت delimits = " \t\n,;";

ایستا تهی skip (ثابت نویسه **pc) {
  برای (;;) {
    اگر (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    وگرنه اگر (**pc == '#') {  /* comment? */
      وقتی (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    وگرنه بشکن;
  }
}

ایستا صحیح getnum_aux (lua_State *L, lua_State *L1, ثابت نویسه **pc) {
  صحیح res = 0;
  صحیح sig = 1;
  skip(pc);
  اگر (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_برداشت(L1, 1);
    (*pc)++;
    بازگشت res;
  }
  وگرنه اگر (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    بازگشت res;
  }
  وگرنه اگر (**pc == '!') {
    (*pc)++;
    اگر (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    وگرنه اگر (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    وگرنه lua_assert(0);
    (*pc)++;
    بازگشت res;
  }
  وگرنه اگر (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  اگر (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  وقتی (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  بازگشت sig*res;
}

ایستا ثابت نویسه *getstring_aux (lua_State *L, نویسه *buff, ثابت نویسه **pc) {
  صحیح i = 0;
  skip(pc);
  اگر (**pc == '"' || **pc == '\'') {  /* quoted string? */
    صحیح quote = *(*pc)++;
    وقتی (**pc != quote) {
      اگر (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  وگرنه {
    وقتی (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  بازگشت buff;
}


ایستا صحیح getindex_aux (lua_State *L, lua_State *L1, ثابت نویسه **pc) {
  skip(pc);
  انتخاب (*(*pc)++) {
    حالت 'R': بازگشت LUA_REGISTRYINDEX;
    حالت 'U': بازگشت lua_upvalueindex(getnum_aux(L, L1, pc));
    پیشفرض: {
      صحیح n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      اگر (n == 0) بازگشت 0;
      وگرنه بازگشت lua_absindex(L1, n);
    }
  }
}


ایستا ثابت نویسه *ثابت statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
ایستا تهی regcodes (lua_State *L) {
  unsigned صحیح i;
  برای (i = 0; i < اندازه(statcodes) / اندازه(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#define EQ(s1)	(strcmp(s1, inst) == 0)

#define getnum		(getnum_aux(L, L1, &pc))
#define getstring	(getstring_aux(L, buff, &pc))
#define getindex	(getindex_aux(L, L1, &pc))


ایستا صحیح testC (lua_State *L);
ایستا صحیح Cfunck (lua_State *L, صحیح status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
ایستا ثابت نویسه ops[] = "+-*%^/\\&|~<>_!";

ایستا صحیح runC (lua_State *L, lua_State *L1, ثابت نویسه *pc) {
  نویسه buff[300];
  صحیح status = 0;
  اگر (pc == پوچ) بازگشت luaL_error(L, "attempt to runC null script");
  برای (;;) {
    ثابت نویسه *inst = getstring;
    اگر EQ("") بازگشت 0;
    وگرنه اگر EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    وگرنه اگر EQ("append") {
      صحیح t = getindex;
      صحیح i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    وگرنه اگر EQ("arith") {
      صحیح op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    وگرنه اگر EQ("call") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      lua_call(L1, narg, nres);
    }
    وگرنه اگر EQ("callk") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      صحیح i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    وگرنه اگر EQ("checkstack") {
      صحیح sz = getnum;
      ثابت نویسه *پیام = getstring;
      اگر (*پیام == '\0')
        پیام = پوچ;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, پیام);
    }
    وگرنه اگر EQ("rawcheckstack") {
      صحیح sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    وگرنه اگر EQ("compare") {
      ثابت نویسه *opt = getstring;  /* EQ, LT, or LE */
      صحیح op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      صحیح a = getindex;
      صحیح b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    وگرنه اگر EQ("concat") {
      lua_concat(L1, getnum);
    }
    وگرنه اگر EQ("copy") {
      صحیح f = getindex;
      lua_copy(L1, f, getindex);
    }
    وگرنه اگر EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    وگرنه اگر EQ("getfield") {
      صحیح t = getindex;
      صحیح tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    وگرنه اگر EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    وگرنه اگر EQ("getmetatable") {
      اگر (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    وگرنه اگر EQ("gettable") {
      صحیح tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    وگرنه اگر EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    وگرنه اگر EQ("gsub") {
      صحیح a = getnum; صحیح b = getnum; صحیح c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    وگرنه اگر EQ("insert") {
      lua_درج(L1, getnum);
    }
    وگرنه اگر EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    وگرنه اگر EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    وگرنه اگر EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    وگرنه اگر EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    وگرنه اگر EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    وگرنه اگر EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    وگرنه اگر EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    وگرنه اگر EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    وگرنه اگر EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    وگرنه اگر EQ("len") {
      lua_len(L1, getindex);
    }
    وگرنه اگر EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    وگرنه اگر EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    وگرنه اگر EQ("loadstring") {
      size_t slen;
      ثابت نویسه *s = luaL_checklstring(L1, getnum, &slen);
      ثابت نویسه *name = getstring;
      ثابت نویسه *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    وگرنه اگر EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    وگرنه اگر EQ("newtable") {
      lua_newtable(L1);
    }
    وگرنه اگر EQ("newthread") {
      lua_newthread(L1);
    }
    وگرنه اگر EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    وگرنه اگر EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    وگرنه اگر EQ("next") {
      lua_next(L1, -2);
    }
    وگرنه اگر EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    وگرنه اگر EQ("pcall") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    وگرنه اگر EQ("pcallk") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      صحیح i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    وگرنه اگر EQ("pop") {
      lua_برداشت(L1, getnum);
    }
    وگرنه اگر EQ("printstack") {
      صحیح n = getnum;
      اگر (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        printf("\n");
      }
      وگرنه lua_printstack(L1);
    }
    وگرنه اگر EQ("print") {
      ثابت نویسه *پیام = getstring;
      printf("%s\n", پیام);
    }
    وگرنه اگر EQ("warningC") {
      ثابت نویسه *پیام = getstring;
      lua_warning(L1, پیام, 1);
    }
    وگرنه اگر EQ("warning") {
      ثابت نویسه *پیام = getstring;
      lua_warning(L1, پیام, 0);
    }
    وگرنه اگر EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    وگرنه اگر EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    وگرنه اگر EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    وگرنه اگر EQ("pushnil") {
      lua_pushnil(L1);
    }
    وگرنه اگر EQ("pushnum") {
      lua_pushnumber(L1, (lua_عدد)getnum);
    }
    وگرنه اگر EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    وگرنه اگر EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    وگرنه اگر EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    وگرنه اگر EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    وگرنه اگر EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (صحیح)lua_tointeger(L, -1));
    }
    وگرنه اگر EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    وگرنه اگر EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    وگرنه اگر EQ("rawget") {
      صحیح t = getindex;
      lua_rawget(L1, t);
    }
    وگرنه اگر EQ("rawgeti") {
      صحیح t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    وگرنه اگر EQ("rawgetp") {
      صحیح t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    وگرنه اگر EQ("rawset") {
      صحیح t = getindex;
      lua_rawset(L1, t);
    }
    وگرنه اگر EQ("rawseti") {
      صحیح t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    وگرنه اگر EQ("rawsetp") {
      صحیح t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    وگرنه اگر EQ("remove") {
      lua_remove(L1, getnum);
    }
    وگرنه اگر EQ("replace") {
      lua_replace(L1, getindex);
    }
    وگرنه اگر EQ("resume") {
      صحیح i = getindex;
      صحیح nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    وگرنه اگر EQ("traceback") {
      ثابت نویسه *پیام = getstring;
      صحیح level = getnum;
      luaL_traceback(L1, L1, پیام, level);
    }
    وگرنه اگر EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    وگرنه اگر EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    وگرنه اگر EQ("return") {
      صحیح n = getnum;
      اگر (L1 != L) {
        صحیح i;
        برای (i = 0; i < n; i++) {
          صحیح idx = -(n - i);
          انتخاب (lua_type(L1, idx)) {
            حالت LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              بشکن;
            پیشفرض:
              lua_pushstring(L, lua_tostring(L1, idx));
              بشکن;
          }
        }
      }
      بازگشت n;
    }
    وگرنه اگر EQ("rotate") {
      صحیح i = getindex;
      lua_rotate(L1, i, getnum);
    }
    وگرنه اگر EQ("setfield") {
      صحیح t = getindex;
      ثابت نویسه *s = getstring;
      lua_setfield(L1, t, s);
    }
    وگرنه اگر EQ("seti") {
      صحیح t = getindex;
      lua_seti(L1, t, getnum);
    }
    وگرنه اگر EQ("setglobal") {
      ثابت نویسه *s = getstring;
      lua_setglobal(L1, s);
    }
    وگرنه اگر EQ("sethook") {
      صحیح mask = getnum;
      صحیح شمار = getnum;
      ثابت نویسه *s = getstring;
      sethookaux(L1, mask, شمار, s);
    }
    وگرنه اگر EQ("setmetatable") {
      صحیح idx = getindex;
      lua_setmetatable(L1, idx);
    }
    وگرنه اگر EQ("settable") {
      lua_settable(L1, getindex);
    }
    وگرنه اگر EQ("settop") {
      lua_settop(L1, getnum);
    }
    وگرنه اگر EQ("testudata") {
      صحیح i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != پوچ);
    }
    وگرنه اگر EQ("error") {
      lua_error(L1);
    }
    وگرنه اگر EQ("abort") {
      abort();
    }
    وگرنه اگر EQ("throw") {
#اگر defined(__cplusplus)
ایستا ساختار X { صحیح x; } x;
      throw x;
#وگرنه
      luaL_error(L1, "C++");
#endif
      بشکن;
    }
    وگرنه اگر EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    وگرنه اگر EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    وگرنه اگر EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    وگرنه اگر EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    وگرنه اگر EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    وگرنه اگر EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    وگرنه اگر EQ("tostring") {
      ثابت نویسه *s = lua_tostring(L1, getindex);
      ثابت نویسه *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == پوچ && s1 == پوچ) || strcmp(s, s1) == 0);
    }
    وگرنه اگر EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, پوچ);
    }
    وگرنه اگر EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    وگرنه اگر EQ("xmove") {
      صحیح f = getindex;
      صحیح t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      صحیح n = getnum;
      اگر (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    وگرنه اگر EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    وگرنه اگر EQ("yield") {
      بازگشت lua_yield(L1, getnum);
    }
    وگرنه اگر EQ("yieldk") {
      صحیح nres = getnum;
      صحیح i = getindex;
      بازگشت lua_yieldk(L1, nres, i, Cfunck);
    }
    وگرنه اگر EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    وگرنه اگر EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    وگرنه اگر EQ("argerror") {
      صحیح arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    وگرنه luaL_error(L, "unknown instruction %s", buff);
  }
  بازگشت 0;
}


ایستا صحیح testC (lua_State *L) {
  lua_State *L1;
  ثابت نویسه *pc;
  اگر (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  وگرنه اگر (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  وگرنه {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  بازگشت runC(L, L1, pc);
}


ایستا صحیح Cfunc (lua_State *L) {
  بازگشت runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


ایستا صحیح Cfunck (lua_State *L, صحیح status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  بازگشت runC(L, L, lua_tostring(L, cast_int(ctx)));
}


ایستا صحیح makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  بازگشت 1;
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
ایستا تهی Chook (lua_State *L, lua_Debug *ar) {
  ثابت نویسه *scpt;
  ثابت نویسه *ثابت events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_برداشت(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
ایستا تهی sethookaux (lua_State *L, صحیح mask, صحیح شمار, ثابت نویسه *scpt) {
  اگر (*scpt == '\0') {  /* no script? */
    lua_sethook(L, پوچ, 0, 0);  /* turn off hooks */
    بازگشت;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  اگر (!lua_istable(L, -1)) {  /* no hook table? */
    lua_برداشت(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, شمار);
}


ایستا صحیح sethook (lua_State *L) {
  اگر (lua_isnoneornil(L, 1))
    lua_sethook(L, پوچ, 0, 0);  /* turn off hooks */
  وگرنه {
    ثابت نویسه *scpt = luaL_checkstring(L, 1);
    ثابت نویسه *smask = luaL_checkstring(L, 2);
    صحیح شمار = cast_int(luaL_optinteger(L, 3, 0));
    صحیح mask = 0;
    اگر (strchr(smask, 'c')) mask |= LUA_MASKCALL;
    اگر (strchr(smask, 'r')) mask |= LUA_MASKRET;
    اگر (strchr(smask, 'l')) mask |= LUA_MASKLINE;
    اگر (شمار > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, شمار, scpt);
  }
  بازگشت 0;
}


ایستا صحیح coresume (lua_State *L) {
  صحیح status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  اگر (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_درج(L, -2);
    بازگشت 2;  /* return false + error message */
  }
  وگرنه {
    lua_pushboolean(L, 1);
    بازگشت 1;
  }
}

#اگر !defined(LUA_USE_POSIX)

#define nonblock	پوچ

#وگرنه

#شامل <unistd.h>
#شامل <fcntl.h>

ایستا صحیح nonblock (lua_State *L) {
  FILE *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  صحیح fd = fileno(f);
  صحیح flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  بازگشت 0;
}
#endif

/* }====================================================== */



ایستا ثابت ساختار luaL_Reg tests_funcs[] = {
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
  {"alloccount", alloc_شمار},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {پوچ, پوچ}
};


ایستا تهی checkfinalmem (تهی) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.total == 0);
}


صحیح luaB_opentests (lua_State *L) {
  تهی *ud;
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
  بازگشت 1;
}

#endif

