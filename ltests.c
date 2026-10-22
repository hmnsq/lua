/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltests_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <setjmp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lauxlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lcode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#ਜੇ defined(LUA_DEBUG)


ਖਾਲੀ *l_Trick = 0;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ obj_at(L,k)	s2v(L->ci->func.p + (k))


ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_C (lua_State *L, lua_State *L1, ਅਚਲ ਅੱਖਰ *pc);


ਸਥਿਰ ਖਾਲੀ setnameval (lua_State *L, ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


ਸਥਿਰ ਖਾਲੀ pushobject (lua_State *L, ਅਚਲ TValue *o) {
  setobj2s(L, L->ਸਿਖਰ.p, o);
  api_incr_ਸਿਖਰ(L);
}


ਸਥਿਰ ਖਾਲੀ badexit (ਅਚਲ ਅੱਖਰ *fmt, ਅਚਲ ਅੱਖਰ *s1, ਅਚਲ ਅੱਖਰ *s2) {
  fprintf(stderr, fmt, s1);
  ਜੇ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.ਕੁੱਲ = 0;
  ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
}


ਸਥਿਰ ਪੂਰਨ tpanic (lua_State *L) {
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ਵਾਪਸ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   ਸੁਨੇਹਾ, ਨਿਰਾ),
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
ਸਥਿਰ ਖਾਲੀ warnf (ਖਾਲੀ *ud, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ tocont) {
  lua_State *L = cast(lua_State *, ud);
  ਸਥਿਰ ਅੱਖਰ buff[200] = "";  /* should be enough for tests... */
  ਸਥਿਰ ਪੂਰਨ onoff = 0;
  ਸਥਿਰ ਪੂਰਨ ਬਹੁਲਕ = 0;  /* start in normal mode */
  ਸਥਿਰ ਪੂਰਨ lasttocont = 0;
  ਜੇ (!lasttocont && !tocont && *ਸੁਨੇਹਾ == '@') {  /* control message? */
    ਜੇ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", ਸੁਨੇਹਾ, buff);
    ਜੇ (strcmp(ਸੁਨੇਹਾ, "@off") == 0)
      onoff = 0;
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਸੁਨੇਹਾ, "@on") == 0)
      onoff = 1;
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਸੁਨੇਹਾ, "@normal") == 0)
      ਬਹੁਲਕ = 0;
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਸੁਨੇਹਾ, "@allow") == 0)
      ਬਹੁਲਕ = 1;
    ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਸੁਨੇਹਾ, "@store") == 0)
      ਬਹੁਲਕ = 2;
    ਨਹੀਂਤਾਂ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              ਸੁਨੇਹਾ, ਨਿਰਾ);
    ਵਾਪਸ;
  }
  lasttocont = tocont;
  ਜੇ (ਲੜੀ_ਲੰਬਾਈ(ਸੁਨੇਹਾ) >= ਆਕਾਰ(buff) - ਲੜੀ_ਲੰਬਾਈ(buff))
    badexit("warnf-buffer overflow (%s)\n", ਸੁਨੇਹਾ, buff);
  strcat(buff, ਸੁਨੇਹਾ);  /* add new message to current warning */
  ਜੇ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ਜੇ (!lua_toboolean(L, -1))
      lua_ਕੱਢੋ(L, 1);  /* ok, no previous unexpected warning */
    ਨਹੀਂਤਾਂ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_ਤਾਲਾ(L);
    ਚੋਣ (ਬਹੁਲਕ) {
      ਹਾਲਤ 0: {  /*ਸਾਧਾਰਨ*/
        ਜੇ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ਨਿਰਾ);
      }  /* FALLTHROUGH */
      ਹਾਲਤ 1: {  /* allow */
        ਜੇ (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        ਰੋਕੋ;
      }
      ਹਾਲਤ 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_ਤਾਲਾ(L);
        ਰੋਕੋ;
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MARK		0x55  /* 01010101 (a nice pattern) */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ mem_ਸਿਰਲੇਖ {
  LUAI_MAXALIGN;
  ਬਣਤਰ {
    size_t ਮਾਪ;
    ਪੂਰਨ type;
  } d;
} mem_ਸਿਰਲੇਖ;


#ਜੇ !defined(EXTERNMEMCHECK)

/* full memory check */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MARKSIZE	16  /* size of marks after each block */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fillmem(mem,ਮਾਪ)	memset(mem, -MARK, ਮਾਪ)

#ਨਹੀਂਤਾਂ

/* external memory check: don't do it twice */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MARKSIZE	0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fillmem(mem,ਮਾਪ)	/*ਖਾਲੀ_ਥਾਵਾਂ*/

#ਜੇ_ਅੰਤ


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


ਸਥਿਰ ਖਾਲੀ freeblock (Memcontrol *mc, mem_ਸਿਰਲੇਖ *ਖੰਡ) {
  ਜੇ (ਖੰਡ) {
    size_t ਮਾਪ = ਖੰਡ->d.ਮਾਪ;
    ਪੂਰਨ i;
    ਲਈ (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(ਖੰਡ + 1) + ਮਾਪ + i) == MARK);
    mc->objcount[ਖੰਡ->d.type]--;
    fillmem(ਖੰਡ, ਆਕਾਰ(mem_ਸਿਰਲੇਖ) + ਮਾਪ + MARKSIZE);  /* erase block */
    ਮੁਕਤ(ਖੰਡ);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->ਕੁੱਲ -= ਮਾਪ;
  }
}


ਖਾਲੀ *debug_realloc (ਖਾਲੀ *ud, ਖਾਲੀ *b, size_t oldsize, size_t ਮਾਪ) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  mem_ਸਿਰਲੇਖ *ਖੰਡ = cast(mem_ਸਿਰਲੇਖ *, b);
  ਪੂਰਨ type;
  ਜੇ (mc->memlimit == 0) {  /* first time? */
    ਅੱਖਰ *ਹੱਦਬੰਦੀ = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = ਹੱਦਬੰਦੀ ? strtoul(ਹੱਦਬੰਦੀ, ਨਿਰਾ, 10) : ULONG_MAX;
  }
  ਜੇ (ਖੰਡ == ਨਿਰਾ) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  ਨਹੀਂਤਾਂ {
    ਖੰਡ--;  /* go to real header */
    type = ਖੰਡ->d.type;
    lua_assert(oldsize == ਖੰਡ->d.ਮਾਪ);
  }
  ਜੇ (ਮਾਪ == 0) {
    freeblock(mc, ਖੰਡ);
    ਵਾਪਸ ਨਿਰਾ;
  }
  ਜੇ (mc->failnext) {
    mc->failnext = 0;
    ਵਾਪਸ ਨਿਰਾ;  /* fake a single memory allocation error */
  }
  ਜੇ (mc->countlimit != ~0UL && ਮਾਪ != oldsize) {  /* count limit in use? */
    ਜੇ (mc->countlimit == 0)
      ਵਾਪਸ ਨਿਰਾ;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ਜੇ (ਮਾਪ > oldsize && mc->ਕੁੱਲ+ਮਾਪ-oldsize > mc->memlimit)
    ਵਾਪਸ ਨਿਰਾ;  /* fake a memory allocation error */
  ਨਹੀਂਤਾਂ {
    mem_ਸਿਰਲੇਖ *newblock;
    ਪੂਰਨ i;
    size_t commonsize = (oldsize < ਮਾਪ) ? oldsize : ਮਾਪ;
    size_t realsize = ਆਕਾਰ(mem_ਸਿਰਲੇਖ) + ਮਾਪ + MARKSIZE;
    ਜੇ (realsize < ਮਾਪ) ਵਾਪਸ ਨਿਰਾ;  /* arithmetic overflow! */
    newblock = cast(mem_ਸਿਰਲੇਖ *, ਸਮ੍ਰਿਤੀ_ਦਿਓ(realsize));  /* alloc a new block */
    ਜੇ (newblock == ਨਿਰਾ)
      ਵਾਪਸ ਨਿਰਾ;  /* really out of memory? */
    ਜੇ (ਖੰਡ) {
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(newblock + 1, ਖੰਡ + 1, commonsize);  /* copy old contents */
      freeblock(mc, ਖੰਡ);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, ਮਾਪ - commonsize);
    /* initialize marks after block */
    ਲਈ (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + ਮਾਪ + i) = MARK;
    newblock->d.ਮਾਪ = ਮਾਪ;
    newblock->d.type = type;
    mc->ਕੁੱਲ += ਮਾਪ;
    ਜੇ (mc->ਕੁੱਲ > mc->maxmem)
      mc->maxmem = mc->ਕੁੱਲ;
    mc->numblocks++;
    mc->objcount[type]++;
    ਵਾਪਸ newblock + 1;
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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
ਸਥਿਰ ਪੂਰਨ testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ਜੇ (isdead(g,t)) ਵਾਪਸ 0;
  ਜੇ (issweepphase(g))
    ਵਾਪਸ 1;  /* no invariants */
  ਨਹੀਂਤਾਂ ਜੇ (g->gckind != KGC_GENMINOR)
    ਵਾਪਸ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  ਨਹੀਂਤਾਂ {  /* generational mode */
    ਜੇ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ਵਾਪਸ 0;
    ਜੇ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ਵਾਪਸ 0;
    ਵਾਪਸ 1;
  }
}


ਸਥਿਰ ਖਾਲੀ printobj (global_State *g, GCObject *o) {
  ਲਿਖੋ("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (ਖਾਲੀ *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ਜੇ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    ਲਿਖੋ(" '%s'", getstr(gco2ts(o)));
}


ਖਾਲੀ lua_printobj (lua_State *L, ਬਣਤਰ GCObject *o) {
  printobj(G(L), o);
}


ਖਾਲੀ lua_printvalue (TValue *v) {
  ਚੋਣ (ttypetag(v)) {
    ਹਾਲਤ LUA_VNUMINT: ਹਾਲਤ LUA_VNUMFLT: {
      ਅੱਖਰ buff[LUA_N2SBUFFSZ];
      ਬਿਨਾਨਿਸ਼ਾਨੀ len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      ਲਿਖੋ("%s", buff);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VSHRSTR:
      ਲਿਖੋ("'%s'", getstr(tsvalue(v))); ਰੋਕੋ;
    ਹਾਲਤ LUA_VLNGSTR:
      ਲਿਖੋ("'%.30s...'", getstr(tsvalue(v))); ਰੋਕੋ;
    ਹਾਲਤ LUA_VFALSE:
      ਲਿਖੋ("%s", "false"); ਰੋਕੋ;
    ਹਾਲਤ LUA_VTRUE:
      ਲਿਖੋ("%s", "true"); ਰੋਕੋ;
    ਹਾਲਤ LUA_VLIGHTUSERDATA:
      ਲਿਖੋ("light udata: %p", pvalue(v)); ਰੋਕੋ;
    ਹਾਲਤ LUA_VUSERDATA:
      ਲਿਖੋ("full udata: %p", uvalue(v)); ਰੋਕੋ;
    ਹਾਲਤ LUA_VNIL:
      ਲਿਖੋ("nil"); ਰੋਕੋ;
    ਹਾਲਤ LUA_VLCF:
      ਲਿਖੋ("light C function: %p", fvalue(v)); ਰੋਕੋ;
    ਹਾਲਤ LUA_VCCL:
      ਲਿਖੋ("C closure: %p", clCvalue(v)); ਰੋਕੋ;
    ਹਾਲਤ LUA_VLCL:
      ਲਿਖੋ("Lua function: %p", clLvalue(v)); ਰੋਕੋ;
    ਹਾਲਤ LUA_VTHREAD:
      ਲਿਖੋ("thread: %p", thvalue(v)); ਰੋਕੋ;
    ਹਾਲਤ LUA_VTABLE:
      ਲਿਖੋ("table: %p", hvalue(v)); ਰੋਕੋ;
    ਮੂਲ:
      lua_assert(0);
  }
}


ਸਥਿਰ ਪੂਰਨ testobjref (global_State *g, GCObject *f, GCObject *t) {
  ਪੂਰਨ r1 = testobjref1(g, f, t);
  ਜੇ (!r1) {
    ਲਿਖੋ("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    ਲਿਖੋ("  ->  ");
    printobj(g, t);
    ਲਿਖੋ("\n");
  }
  ਵਾਪਸ r1;
}


ਸਥਿਰ ਖਾਲੀ checkobjref (global_State *g, GCObject *f, GCObject *t) {
    ਪ੍ਰਤਿਜ੍ਞਾ(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checkobjrefN(g,f,t)	{ ਜੇ (t) checkobjref(g,f,obj2gco(t)); }


ਸਥਿਰ ਖਾਲੀ checkvalref (global_State *g, GCObject *f, ਅਚਲ TValue *t) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


ਸਥਿਰ ਖਾਲੀ checktable (global_State *g, ਸਾਰਣੀ *h) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ asize = h->asize;
  ਗੰਢ_2 *n, *ਹੱਦਬੰਦੀ = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  ਲਈ (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  ਲਈ (n = gnode(h, 0); n < ਹੱਦਬੰਦੀ; n++) {
    ਜੇ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      ਪ੍ਰਤਿਜ੍ਞਾ(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


ਸਥਿਰ ਖਾਲੀ checkudata (global_State *g, Udata *u) {
  ਪੂਰਨ i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  ਲਈ (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


ਸਥਿਰ ਖਾਲੀ checkproto (global_State *g, Proto *f) {
  ਪੂਰਨ i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->ਸਰੋਤ);
  ਲਈ (i=0; i<f->sizek; i++) {
    ਜੇ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  ਲਈ (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  ਲਈ (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  ਲਈ (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


ਸਥਿਰ ਖਾਲੀ checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


ਸਥਿਰ ਖਾਲੀ checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ਪੂਰਨ i;
  checkobjrefN(g, clgc, cl->p);
  ਲਈ (i=0; i<cl->nupvalues; i++) {
    ਉੱਪਰ_Val *uv = cl->upvals[i];
    ਜੇ (uv) {
      checkobjrefN(g, clgc, uv);
      ਜੇ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


ਸਥਿਰ ਪੂਰਨ lua_checkpc (CallInfo *ci) {
  ਜੇ (!isLua(ci)) ਵਾਪਸ 1;
  ਨਹੀਂਤਾਂ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ਵਾਪਸ p->ਕੋਡ <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->ਕੋਡ + p->sizecode;
  }
}


ਸਥਿਰ ਖਾਲੀ check_ਢੇਰ (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  ਉੱਪਰ_Val *uv;
  ਪ੍ਰਤਿਜ੍ਞਾ(!isdead(g, L1));
  ਜੇ (L1->ਢੇਰ.p == ਨਿਰਾ) {  /* incomplete thread? */
    ਪ੍ਰਤਿਜ੍ਞਾ(L1->openupval == ਨਿਰਾ && L1->ci == ਨਿਰਾ);
    ਵਾਪਸ;
  }
  ਲਈ (uv = L1->openupval; uv != ਨਿਰਾ; uv = uv->u.open.ਅਗਲਾ)
    ਪ੍ਰਤਿਜ੍ਞਾ(upisopen(uv));  /* must be open */
  ਪ੍ਰਤਿਜ੍ਞਾ(L1->ਸਿਖਰ.p <= L1->ਢੇਰ_ਆਖਰੀ.p);
  ਪ੍ਰਤਿਜ੍ਞਾ(L1->tbclist.p <= L1->ਸਿਖਰ.p);
  ਲਈ (ci = L1->ci; ci != ਨਿਰਾ; ci = ci->ਪਿਛਲਾ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ci->ਸਿਖਰ.p <= L1->ਢੇਰ_ਆਖਰੀ.p);
    ਪ੍ਰਤਿਜ੍ਞਾ(lua_checkpc(ci));
  }
  ਲਈ (o = L1->ਢੇਰ.p; o < L1->ਢੇਰ_ਆਖਰੀ.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


ਸਥਿਰ ਖਾਲੀ checkrefs (global_State *g, GCObject *o) {
  ਚੋਣ (o->tt) {
    ਹਾਲਤ LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VTABLE: {
      checktable(g, gco2t(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VTHREAD: {
      check_ਢੇਰ(g, gco2th(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VPROTO: {
      checkproto(g, gco2p(o));
      ਰੋਕੋ;
    }
    ਹਾਲਤ LUA_VSHRSTR:
    ਹਾਲਤ LUA_VLNGSTR: {
      ਪ੍ਰਤਿਜ੍ਞਾ(!isgray(o));  /* strings are never gray */
      ਰੋਕੋ;
    }
    ਮੂਲ: ਪ੍ਰਤਿਜ੍ਞਾ(0);
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
ਸਥਿਰ ਖਾਲੀ checkobject (global_State *g, GCObject *o, ਪੂਰਨ maybedead,
                         ਪੂਰਨ listage) {
  ਜੇ (isdead(g, o))
    ਪ੍ਰਤਿਜ੍ਞਾ(maybedead);
  ਨਹੀਂਤਾਂ {
    ਪ੍ਰਤਿਜ੍ਞਾ(g->gcstate != GCSpause || iswhite(o));
    ਜੇ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      ਪ੍ਰਤਿਜ੍ਞਾ(getage(o) >= listage);
      ਜੇ (isold(o)) {
        ਪ੍ਰਤਿਜ੍ਞਾ(!iswhite(o));
        ਪ੍ਰਤਿਜ੍ਞਾ(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      ਪ੍ਰਤਿਜ੍ਞਾ(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


ਸਥਿਰ l_mem checkgraylist (global_State *g, GCObject *o) {
  ਪੂਰਨ ਕੁੱਲ = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ਜਦੋਂ (o) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    ਪ੍ਰਤਿਜ੍ਞਾ(!testbit(o->marked, TESTBIT));
    ਜੇ (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    ਕੁੱਲ++;
    ਚੋਣ (o->tt) {
      ਹਾਲਤ LUA_VTABLE: o = gco2t(o)->gclist; ਰੋਕੋ;
      ਹਾਲਤ LUA_VLCL: o = gco2lcl(o)->gclist; ਰੋਕੋ;
      ਹਾਲਤ LUA_VCCL: o = gco2ccl(o)->gclist; ਰੋਕੋ;
      ਹਾਲਤ LUA_VTHREAD: o = gco2th(o)->gclist; ਰੋਕੋ;
      ਹਾਲਤ LUA_VPROTO: o = gco2p(o)->gclist; ਰੋਕੋ;
      ਹਾਲਤ LUA_VUSERDATA:
        ਪ੍ਰਤਿਜ੍ਞਾ(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        ਰੋਕੋ;
      ਮੂਲ: ਪ੍ਰਤਿਜ੍ਞਾ(0);  /* other objects cannot be in a gray list */
    }
  }
  ਵਾਪਸ ਕੁੱਲ;
}


/*
** Check objects in gray lists.
*/
ਸਥਿਰ l_mem checkgrays (global_State *g) {
  l_mem ਕੁੱਲ = 0;  /* count number of elements in all lists */
  ਜੇ (!keepinvariant(g)) ਵਾਪਸ ਕੁੱਲ;
  ਕੁੱਲ += checkgraylist(g, g->ਸਲੇਟੀ);
  ਕੁੱਲ += checkgraylist(g, g->grayagain);
  ਕੁੱਲ += checkgraylist(g, g->weak);
  ਕੁੱਲ += checkgraylist(g, g->allweak);
  ਕੁੱਲ += checkgraylist(g, g->ephemeron);
  ਵਾਪਸ ਕੁੱਲ;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
ਸਥਿਰ ਖਾਲੀ incifingray (global_State *g, GCObject *o, l_mem *ਗਿਣਤੀ_2) {
  ਜੇ (!keepinvariant(g))
    ਵਾਪਸ;  /* gray lists not being kept in these phases */
  ਜੇ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    ਪ੍ਰਤਿਜ੍ਞਾ(!isgray(o) || upisopen(gco2upv(o)));
    ਵਾਪਸ;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ਜੇ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*ਗਿਣਤੀ_2)++;
    ਪ੍ਰਤਿਜ੍ਞਾ(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


ਸਥਿਰ l_mem checklist (global_State *g, ਪੂਰਨ maybedead, ਪੂਰਨ tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem ਕੁੱਲ = 0;  /* number of object that should be in  gray lists */
  ਲਈ (o = newl; o != survival; o = o->ਅਗਲਾ) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &ਕੁੱਲ);
    ਪ੍ਰਤਿਜ੍ਞਾ(!tof == !tofinalize(o));
  }
  ਲਈ (o = survival; o != old; o = o->ਅਗਲਾ) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &ਕੁੱਲ);
    ਪ੍ਰਤਿਜ੍ਞਾ(!tof == !tofinalize(o));
  }
  ਲਈ (o = old; o != reallyold; o = o->ਅਗਲਾ) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &ਕੁੱਲ);
    ਪ੍ਰਤਿਜ੍ਞਾ(!tof == !tofinalize(o));
  }
  ਲਈ (o = reallyold; o != ਨਿਰਾ; o = o->ਅਗਲਾ) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &ਕੁੱਲ);
    ਪ੍ਰਤਿਜ੍ਞਾ(!tof == !tofinalize(o));
  }
  ਵਾਪਸ ਕੁੱਲ;
}


ਪੂਰਨ lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ਪੂਰਨ maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ਜੇ (keepinvariant(g)) {
    ਪ੍ਰਤਿਜ੍ਞਾ(!iswhite(mainthread(g)));
    ਪ੍ਰਤਿਜ੍ਞਾ(!iswhite(gcvalue(&g->l_registry)));
  }
  ਪ੍ਰਤਿਜ੍ਞਾ(!isdead(g, gcvalue(&g->l_registry)));
  ਪ੍ਰਤਿਜ੍ਞਾ(g->sweepgc == ਨਿਰਾ || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  ਲਈ (o = g->fixedgc; o != ਨਿਰਾ; o = o->ਅਗਲਾ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  ਲਈ (o = g->tobefnz; o != ਨਿਰਾ; o = o->ਅਗਲਾ) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    ਪ੍ਰਤਿਜ੍ਞਾ(tofinalize(o));
    ਪ੍ਰਤਿਜ੍ਞਾ(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ਜੇ (keepinvariant(g))
    ਪ੍ਰਤਿਜ੍ਞਾ(totalin == totalshould);
  ਵਾਪਸ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


ਸਥਿਰ ਅੱਖਰ *buildop (Proto *p, ਪੂਰਨ pc, ਅੱਖਰ *buff) {
  ਅੱਖਰ *obuff = buff;
  Instruction i = p->ਕੋਡ[pc];
  Op_ਕੋਡ o = GET_OPCODE(i);
  ਅਚਲ ਅੱਖਰ *name = opnames[o];
  ਪੂਰਨ ਸਤਰ = luaG_getfuncline(p, pc);
  ਪੂਰਨ lineinfo = (p->lineinfo != ਨਿਰਾ) ? p->lineinfo[pc] : 0;
  ਜੇ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  ਨਹੀਂਤਾਂ
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", ਸਤਰ, pc);
  ਚੋਣ (get_Op_ਬਹੁਲਕ(o)) {
    ਹਾਲਤ iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      ਰੋਕੋ;
    ਹਾਲਤ ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      ਰੋਕੋ;
    ਹਾਲਤ iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      ਰੋਕੋ;
    ਹਾਲਤ iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      ਰੋਕੋ;
    ਹਾਲਤ iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      ਰੋਕੋ;
    ਹਾਲਤ isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      ਰੋਕੋ;
  }
  ਵਾਪਸ obuff;
}


#ਜੇ 0
ਖਾਲੀ luaI_printcode (Proto *pt, ਪੂਰਨ ਮਾਪ) {
  ਪੂਰਨ pc;
  ਲਈ (pc=0; pc<ਮਾਪ; pc++) {
    ਅੱਖਰ buff[100];
    ਲਿਖੋ("%s\n", buildop(pt, pc, buff));
  }
  ਲਿਖੋ("-------\n");
}


ਖਾਲੀ luaI_printinst (Proto *pt, ਪੂਰਨ pc) {
  ਅੱਖਰ buff[100];
  ਲਿਖੋ("%s\n", buildop(pt, pc, buff));
}
#ਜੇ_ਅੰਤ


ਸਥਿਰ ਪੂਰਨ listcode (lua_State *L) {
  ਪੂਰਨ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  ਲਈ (pc=0; pc<p->sizecode; pc++) {
    ਅੱਖਰ buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ printcode (lua_State *L) {
  ਪੂਰਨ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ਲਿਖੋ("maxstack: %d\n", p->maxstacksize);
  ਲਿਖੋ("numparams: %d\n", p->numparams);
  ਲਈ (pc=0; pc<p->sizecode; pc++) {
    ਅੱਖਰ buff[100];
    ਲਿਖੋ("%s\n", buildop(p, pc, buff));
  }
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ listk (lua_State *L) {
  Proto *p;
  ਪੂਰਨ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  ਲਈ (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ listabslineinfo (lua_State *L) {
  Proto *p;
  ਪੂਰਨ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ਨਿਰਾ, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  ਲਈ (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].ਸਤਰ);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ listlocals (lua_State *L) {
  Proto *p;
  ਪੂਰਨ pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ਪੂਰਨ i = 0;
  ਅਚਲ ਅੱਖਰ *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ਜਦੋਂ ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ਨਿਰਾ)
    lua_pushstring(L, name);
  ਵਾਪਸ i-1;
}

/* }====================================================== */



ਖਾਲੀ lua_printstack (lua_State *L) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = lua_gettop(L);
  ਲਿਖੋ("stack: >>\n");
  ਲਈ (i = 1; i <= n; i++) {
    ਲਿਖੋ("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    ਲਿਖੋ("\n");
  }
  ਲਿਖੋ("<<\n");
}


ਪੂਰਨ lua_printallstack (lua_State *L) {
  StkId p;
  ਪੂਰਨ i = 1;
  CallInfo *ci = &L->ਅਧਾਰ_ci;
  ਲਿਖੋ("stack: >>\n");
  ਲਈ (p = L->ਢੇਰ.p; p < L->ਸਿਖਰ.p; p++) {
    ਜੇ (ci != ਨਿਰਾ && p == ci->func.p) {
      ਲਿਖੋ("  ---\n");
      ਜੇ (ci == L->ci)
        ci = ਨਿਰਾ;  /* printed last frame */
      ਨਹੀਂਤਾਂ
        ci = ci->ਅਗਲਾ;
    }
    ਲਿਖੋ("%3d: ", i++);
    lua_printvalue(s2v(p));
    ਲਿਖੋ("\n");
  }
  ਲਿਖੋ("<<\n");
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", ਵਿੱਥ_s_Bx);
  setnameval(L, "NUM_OPCODES", ਅੰਕ_OPCODES);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", ਆਕਾਰ(lua_State));
  setnameval(L, "global state", ਆਕਾਰ(global_State));
  setnameval(L, "TValue", ਆਕਾਰ(TValue));
  setnameval(L, "Node", ਆਕਾਰ(ਗੰਢ_2));
  setnameval(L, "stack Value", ਆਕਾਰ(ਢੇਰ_ਮੁੱਲ));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ mem_query (lua_State *L) {
  ਜੇ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.ਕੁੱਲ));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ਵਾਪਸ 3;
  }
  ਨਹੀਂਤਾਂ ਜੇ (lua_isnumber(L, 1)) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਹੱਦਬੰਦੀ = cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ, luaL_checkinteger(L, 1));
    ਜੇ (ਹੱਦਬੰਦੀ == 0) ਹੱਦਬੰਦੀ = ULONG_MAX;
    l_memcontrol.memlimit = ਹੱਦਬੰਦੀ;
    ਵਾਪਸ 0;
  }
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *t = luaL_checkstring(L, 1);
    ਪੂਰਨ i;
    ਲਈ (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ਜੇ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ਵਾਪਸ 1;
      }
    }
    ਵਾਪਸ luaL_error(L, "unknown type '%s'", t);
  }
}


ਸਥਿਰ ਪੂਰਨ alloc_ਗਿਣਤੀ (lua_State *L) {
  ਜੇ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ, ~0L);
  ਨਹੀਂਤਾਂ
    l_memcontrol.countlimit = cast(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ, luaL_checkinteger(L, 1));
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ alloc_failnext (lua_State *L) {
  ਅਣਵਰਤਿਆ(L);
  l_memcontrol.failnext = 1;
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ settrick (lua_State *L) {
  ਜੇ (ttisnil(obj_at(L, 1)))
    l_Trick = ਨਿਰਾ;
  ਨਹੀਂਤਾਂ
    l_Trick = gcvalue(obj_at(L, 1));
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ਜੇ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ਨਹੀਂਤਾਂ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ਜੇ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ਨਹੀਂਤਾਂ {
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ਜੇ (!iscollectable(o))
    ਲਿਖੋ("no collectable\n");
  ਨਹੀਂਤਾਂ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    ਲਿਖੋ("\n");
  }
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

ਸਥਿਰ ਪੂਰਨ gc_state (lua_State *L) {
  ਸਥਿਰ ਅਚਲ ਪੂਰਨ ਹਾਲਤਾਂ[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ਪੂਰਨ option = ਹਾਲਤਾਂ[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ਜੇ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ {
    ਜੇ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_ਤਾਲਾ(L);
    ਜੇ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ਵਾਪਸ 0;
  }
}


ਸਥਿਰ ਪੂਰਨ tracinggc = 0;
ਖਾਲੀ luai_tracegctest (lua_State *L, ਪੂਰਨ ਪਹਿਲਾ) {
  ਜੇ (!tracinggc) ਵਾਪਸ;
  ਨਹੀਂਤਾਂ {
    global_State *g = G(L);
    lua_unlock(L);
    g->gcstp = GCSTPGC;
    lua_checkstack(L, 10);
    lua_getfield(L, LUA_REGISTRYINDEX, "tracegc");
    lua_pushboolean(L, ਪਹਿਲਾ);
    lua_call(L, 1, 0);
    g->gcstp = 0;
    lua_ਤਾਲਾ(L);
  }
}


ਸਥਿਰ ਪੂਰਨ tracegc (lua_State *L) {
  ਜੇ (lua_isnil(L, 1))
    tracinggc = 0;
  ਨਹੀਂਤਾਂ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ hash_query (lua_State *L) {
  ਜੇ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ਜੇ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  ਨਹੀਂਤਾਂ {
    TValue *o = obj_at(L, 1);
    ਸਾਰਣੀ *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->ਗੰਢ));
  }
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ stacklevel (lua_State *L) {
  ਪੂਰਨ a = 0;
  lua_pushinteger(L, cast_Integer(L->ਸਿਖਰ.p - L->ਢੇਰ.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ਵਾਪਸ 5;
}


ਸਥਿਰ ਪੂਰਨ resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ਜਦੋਂ (ci->ਅਗਲਾ != ਨਿਰਾ) {
    CallInfo *tofree = ci->ਅਗਲਾ;
    ci->ਅਗਲਾ = ci->ਅਗਲਾ->ਅਗਲਾ;
    luaM_free(L, tofree);
    L->nci--;
  }
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ reallocstack (lua_State *L) {
  ਪੂਰਨ n = cast_int(luaL_checkinteger(L, 1));
  lua_ਤਾਲਾ(L);
  luaD_reallocstack(L, cast_int(L->ਸਿਖਰ.p - L->ਢੇਰ.p) + n, 1);
  lua_unlock(L);
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ ਸਾਰਣੀ_query (lua_State *L) {
  ਅਚਲ ਸਾਰਣੀ *t;
  ਪੂਰਨ i = cast_int(luaL_optinteger(L, 2, -1));
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ਜੇ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ਵਾਪਸ 3;
  }
  ਨਹੀਂਤਾਂ ਜੇ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ਜੇ (!tagisempty(*get_Arr_ਟੈਗ(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->ਸਿਖਰ.p));
    ਨਹੀਂਤਾਂ
      setnilvalue2s(L->ਸਿਖਰ.p);
    api_incr_ਸਿਖਰ(L);
    lua_pushnil(L);
  }
  ਨਹੀਂਤਾਂ ਜੇ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ਜੇ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    ਨਹੀਂਤਾਂ
      lua_pushliteral(L, "<undef>");
    ਜੇ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    ਨਹੀਂਤਾਂ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->ਗੰਢ[i]));
  }
  ਵਾਪਸ 3;
}


ਸਥਿਰ ਪੂਰਨ gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ਵਾਪਸ 6;
}


ਸਥਿਰ ਪੂਰਨ test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ਪੂਰਨ s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ਜੇ (s == -1) {
    lua_pushinteger(L ,tb->ਮਾਪ);
    lua_pushinteger(L ,tb->nuse);
    ਵਾਪਸ 2;
  }
  ਨਹੀਂਤਾਂ ਜੇ (s < tb->ਮਾਪ) {
    TString *ts;
    ਪੂਰਨ n = 0;
    ਲਈ (ts = tb->hash[s]; ts != ਨਿਰਾ; ts = ts->u.hnext) {
      setsvalue2s(L, L->ਸਿਖਰ.p, ts);
      api_incr_ਸਿਖਰ(L);
      n++;
    }
    ਵਾਪਸ n;
  }
  ਨਹੀਂਤਾਂ ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ getreftable (lua_State *L) {
  ਜੇ (lua_istable(L, 2))  /* is there a table as second argument? */
    ਵਾਪਸ 2;  /* use it as the table */
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ LUA_REGISTRYINDEX;  /* default is to use the register */
}


ਸਥਿਰ ਪੂਰਨ tref (lua_State *L) {
  ਪੂਰਨ t = getreftable(L);
  ਪੂਰਨ level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ getref (lua_State *L) {
  ਪੂਰਨ t = getreftable(L);
  ਪੂਰਨ level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ unref (lua_State *L) {
  ਪੂਰਨ t = getreftable(L);
  ਪੂਰਨ level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ upvalue (lua_State *L) {
  ਪੂਰਨ n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ਜੇ (lua_isnone(L, 3)) {
    ਅਚਲ ਅੱਖਰ *name = lua_getupvalue(L, 1, n);
    ਜੇ (name == ਨਿਰਾ) ਵਾਪਸ 0;
    lua_pushstring(L, name);
    ਵਾਪਸ 2;
  }
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ਵਾਪਸ 1;
  }
}


ਸਥਿਰ ਪੂਰਨ newuserdata (lua_State *L) {
  size_t ਮਾਪ = cast_sizet(luaL_optinteger(L, 1, 0));
  ਪੂਰਨ nuv = cast_int(luaL_optinteger(L, 2, 0));
  ਅੱਖਰ *p = cast_charp(lua_newuserdatauv(L, ਮਾਪ, nuv));
  ਜਦੋਂ (ਮਾਪ--) *p++ = '\0';
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &l);
  ਪੂਰਨ ਸਥਿਤੀ = luaL_loadbufferx(L1, s, l, s, "t");
  ਜੇ (ਸਥਿਤੀ == LUA_OK)
    ਸਥਿਤੀ = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, ਸਥਿਤੀ);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ s2d (lua_State *L) {
  lua_pushnumber(L, cast_ਅੰਕ(*cast(ਅਚਲ ਦੁੱਗਣਾ *, luaL_checkstring(L, 1))));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ d2s (lua_State *L) {
  ਦੁੱਗਣਾ d = cast(ਦੁੱਗਣਾ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), ਆਕਾਰ(d));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ newstate (lua_State *L) {
  ਖਾਲੀ *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ਜੇ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  ਨਹੀਂਤਾਂ
    lua_pushnil(L);
  ਵਾਪਸ 1;
}


ਸਥਿਰ lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ਨਿਰਾ, 1, "state expected");
  ਵਾਪਸ L1;
}


ਸਥਿਰ ਪੂਰਨ loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ਪੂਰਨ ਲੱਦੋ = cast_int(luaL_checkinteger(L, 2));
  ਪੂਰਨ preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, ਲੱਦੋ, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ਨਿਰਾ, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ਅਚਲ ਅੱਖਰ *ਕੋਡ = luaL_checklstring(L, 2, &lcode);
  ਪੂਰਨ ਸਥਿਤੀ;
  lua_settop(L1, 0);
  ਸਥਿਤੀ = luaL_loadbufferx(L1, ਕੋਡ, lcode, ਕੋਡ, "t");
  ਜੇ (ਸਥਿਤੀ == LUA_OK)
    ਸਥਿਤੀ = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ਜੇ (ਸਥਿਤੀ != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, ਸਥਿਤੀ);
    ਵਾਪਸ 3;
  }
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ i = 0;
    ਜਦੋਂ (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_ਕੱਢੋ(L1, i-1);
    ਵਾਪਸ i-1;
  }
}


ਸਥਿਰ ਪੂਰਨ log2_aux (lua_State *L) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ਵਾਪਸ 1;
}


ਬਣਤਰ Aux { jmp_buf jb; ਅਚਲ ਅੱਖਰ *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
ਸਥਿਰ ਪੂਰਨ panicback (lua_State *L) {
  ਬਣਤਰ Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (ਬਣਤਰ Aux *)lua_touserdata(L, -1);
  lua_ਕੱਢੋ(L, 1);  /* remove 'Aux' struct */
  ਲੜੀਵਾਰ_C(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ਵਾਪਸ 1;  /* to avoid warnings */
}

ਸਥਿਰ ਪੂਰਨ checkpanic (lua_State *L) {
  ਬਣਤਰ Aux b;
  ਖਾਲੀ *ud;
  lua_State *L1;
  ਅਚਲ ਅੱਖਰ *ਕੋਡ = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ਜੇ (L1 == ਨਿਰਾ) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ਵਾਪਸ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ਜੇ (setjmp(b.jb) == 0) {  /* set jump buffer */
    ਲੜੀਵਾਰ_C(L, L1, ਕੋਡ);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  ਨਹੀਂਤਾਂ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ਵਾਪਸ 1;
}


ਸਥਿਰ ਪੂਰਨ externKstr (lua_State *L) {
  size_t len;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ਨਿਰਾ, ਨਿਰਾ);
  ਵਾਪਸ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
ਸਥਿਰ ਪੂਰਨ externstr (lua_State *L) {
  size_t len;
  ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L, 1, &len);
  ਖਾਲੀ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  ਅੱਖਰ *buff = cast_charp((*allocf)(ud, ਨਿਰਾ, 0, len + 1));
  ਜੇ (buff == ਨਿਰਾ) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(buff, s, (len + 1) * ਆਕਾਰ(ਅੱਖਰ));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ਵਾਪਸ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


ਸਥਿਰ ਖਾਲੀ sethookaux (lua_State *L, ਪੂਰਨ ਨਕਾਬ_2, ਪੂਰਨ ਗਿਣਤੀ_2, ਅਚਲ ਅੱਖਰ *ਕੋਡ);

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ delimits = " \t\n,;";

ਸਥਿਰ ਖਾਲੀ skip (ਅਚਲ ਅੱਖਰ **pc) {
  ਲਈ (;;) {
    ਜੇ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    ਨਹੀਂਤਾਂ ਜੇ (**pc == '#') {  /* comment? */
      ਜਦੋਂ (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    ਨਹੀਂਤਾਂ ਰੋਕੋ;
  }
}

ਸਥਿਰ ਪੂਰਨ getnum_aux (lua_State *L, lua_State *L1, ਅਚਲ ਅੱਖਰ **pc) {
  ਪੂਰਨ res = 0;
  ਪੂਰਨ sig = 1;
  skip(pc);
  ਜੇ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_ਕੱਢੋ(L1, 1);
    (*pc)++;
    ਵਾਪਸ res;
  }
  ਨਹੀਂਤਾਂ ਜੇ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ਵਾਪਸ res;
  }
  ਨਹੀਂਤਾਂ ਜੇ (**pc == '!') {
    (*pc)++;
    ਜੇ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    ਨਹੀਂਤਾਂ ਜੇ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    ਨਹੀਂਤਾਂ lua_assert(0);
    (*pc)++;
    ਵਾਪਸ res;
  }
  ਨਹੀਂਤਾਂ ਜੇ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ਜੇ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ਜਦੋਂ (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ਵਾਪਸ sig*res;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getstring_aux (lua_State *L, ਅੱਖਰ *buff, ਅਚਲ ਅੱਖਰ **pc) {
  ਪੂਰਨ i = 0;
  skip(pc);
  ਜੇ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ਪੂਰਨ quote = *(*pc)++;
    ਜਦੋਂ (**pc != quote) {
      ਜੇ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  ਨਹੀਂਤਾਂ {
    ਜਦੋਂ (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ਵਾਪਸ buff;
}


ਸਥਿਰ ਪੂਰਨ getindex_aux (lua_State *L, lua_State *L1, ਅਚਲ ਅੱਖਰ **pc) {
  skip(pc);
  ਚੋਣ (*(*pc)++) {
    ਹਾਲਤ 'R': ਵਾਪਸ LUA_REGISTRYINDEX;
    ਹਾਲਤ 'U': ਵਾਪਸ lua_upvalueindex(getnum_aux(L, L1, pc));
    ਮੂਲ: {
      ਪੂਰਨ n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ਜੇ (n == 0) ਵਾਪਸ 0;
      ਨਹੀਂਤਾਂ ਵਾਪਸ lua_absindex(L1, n);
    }
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
ਸਥਿਰ ਖਾਲੀ regcodes (lua_State *L) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਲਈ (i = 0; i < ਆਕਾਰ(statcodes) / ਆਕਾਰ(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EQ(s1)	(strcmp(s1, inst) == 0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getnum		(getnum_aux(L, L1, &pc))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getstring	(getstring_aux(L, buff, &pc))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getindex	(getindex_aux(L, L1, &pc))


ਸਥਿਰ ਪੂਰਨ testC (lua_State *L);
ਸਥਿਰ ਪੂਰਨ Cfunck (lua_State *L, ਪੂਰਨ ਸਥਿਤੀ, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ ops[] = "+-*%^/\\&|~<>_!";

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_C (lua_State *L, lua_State *L1, ਅਚਲ ਅੱਖਰ *pc) {
  ਅੱਖਰ buff[300];
  ਪੂਰਨ ਸਥਿਤੀ = 0;
  ਜੇ (pc == ਨਿਰਾ) ਵਾਪਸ luaL_error(L, "attempt to runC null script");
  ਲਈ (;;) {
    ਅਚਲ ਅੱਖਰ *inst = getstring;
    ਜੇ EQ("") ਵਾਪਸ 0;
    ਨਹੀਂਤਾਂ ਜੇ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("append") {
      ਪੂਰਨ t = getindex;
      ਪੂਰਨ i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("arith") {
      ਪੂਰਨ op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("call") {
      ਪੂਰਨ narg = getnum;
      ਪੂਰਨ nres = getnum;
      lua_call(L1, narg, nres);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("callk") {
      ਪੂਰਨ narg = getnum;
      ਪੂਰਨ nres = getnum;
      ਪੂਰਨ i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("checkstack") {
      ਪੂਰਨ sz = getnum;
      ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = getstring;
      ਜੇ (*ਸੁਨੇਹਾ == '\0')
        ਸੁਨੇਹਾ = ਨਿਰਾ;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, ਸੁਨੇਹਾ);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rawcheckstack") {
      ਪੂਰਨ sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("compare") {
      ਅਚਲ ਅੱਖਰ *opt = getstring;  /* EQ, LT, or LE */
      ਪੂਰਨ op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ਪੂਰਨ a = getindex;
      ਪੂਰਨ b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("concat") {
      lua_concat(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("copy") {
      ਪੂਰਨ f = getindex;
      lua_copy(L1, f, getindex);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("getfield") {
      ਪੂਰਨ t = getindex;
      ਪੂਰਨ tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("getmetatable") {
      ਜੇ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("gettable") {
      ਪੂਰਨ tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("gsub") {
      ਪੂਰਨ a = getnum; ਪੂਰਨ b = getnum; ਪੂਰਨ c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("insert") {
      lua_ਪਾਓ(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("len") {
      lua_len(L1, getindex);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("loadstring") {
      size_t slen;
      ਅਚਲ ਅੱਖਰ *s = luaL_checklstring(L1, getnum, &slen);
      ਅਚਲ ਅੱਖਰ *name = getstring;
      ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ = getstring;
      luaL_loadbufferx(L1, s, slen, name, ਬਹੁਲਕ);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("newtable") {
      lua_newtable(L1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("newthread") {
      lua_newthread(L1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("next") {
      lua_ਅਗਲਾ(L1, -2);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pcall") {
      ਪੂਰਨ narg = getnum;
      ਪੂਰਨ nres = getnum;
      ਸਥਿਤੀ = lua_pcall(L1, narg, nres, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pcallk") {
      ਪੂਰਨ narg = getnum;
      ਪੂਰਨ nres = getnum;
      ਪੂਰਨ i = getindex;
      ਸਥਿਤੀ = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pop") {
      lua_ਕੱਢੋ(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("printstack") {
      ਪੂਰਨ n = getnum;
      ਜੇ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        ਲਿਖੋ("\n");
      }
      ਨਹੀਂਤਾਂ lua_printstack(L1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("print") {
      ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = getstring;
      ਲਿਖੋ("%s\n", ਸੁਨੇਹਾ);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("warningC") {
      ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = getstring;
      lua_warning(L1, ਸੁਨੇਹਾ, 1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("warning") {
      ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = getstring;
      lua_warning(L1, ਸੁਨੇਹਾ, 0);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushnil") {
      lua_pushnil(L1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushnum") {
      lua_pushnumber(L1, (lua_ਸੰਖਿਆ)getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[ਸਥਿਤੀ]);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ਪੂਰਨ)lua_tointeger(L, -1));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rawget") {
      ਪੂਰਨ t = getindex;
      lua_rawget(L1, t);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rawgeti") {
      ਪੂਰਨ t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rawgetp") {
      ਪੂਰਨ t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rawset") {
      ਪੂਰਨ t = getindex;
      lua_rawset(L1, t);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rawseti") {
      ਪੂਰਨ t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rawsetp") {
      ਪੂਰਨ t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("remove") {
      lua_remove(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("replace") {
      lua_replace(L1, getindex);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("resume") {
      ਪੂਰਨ i = getindex;
      ਪੂਰਨ nres;
      ਸਥਿਤੀ = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("traceback") {
      ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = getstring;
      ਪੂਰਨ level = getnum;
      luaL_traceback(L1, L1, ਸੁਨੇਹਾ, level);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_ਸਥਿਤੀ(L1)]);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("return") {
      ਪੂਰਨ n = getnum;
      ਜੇ (L1 != L) {
        ਪੂਰਨ i;
        ਲਈ (i = 0; i < n; i++) {
          ਪੂਰਨ idx = -(n - i);
          ਚੋਣ (lua_type(L1, idx)) {
            ਹਾਲਤ LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              ਰੋਕੋ;
            ਮੂਲ:
              lua_pushstring(L, lua_tostring(L1, idx));
              ਰੋਕੋ;
          }
        }
      }
      ਵਾਪਸ n;
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("rotate") {
      ਪੂਰਨ i = getindex;
      lua_rotate(L1, i, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("setfield") {
      ਪੂਰਨ t = getindex;
      ਅਚਲ ਅੱਖਰ *s = getstring;
      lua_setfield(L1, t, s);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("seti") {
      ਪੂਰਨ t = getindex;
      lua_seti(L1, t, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("setglobal") {
      ਅਚਲ ਅੱਖਰ *s = getstring;
      lua_setglobal(L1, s);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("sethook") {
      ਪੂਰਨ ਨਕਾਬ_2 = getnum;
      ਪੂਰਨ ਗਿਣਤੀ_2 = getnum;
      ਅਚਲ ਅੱਖਰ *s = getstring;
      sethookaux(L1, ਨਕਾਬ_2, ਗਿਣਤੀ_2, s);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("setmetatable") {
      ਪੂਰਨ idx = getindex;
      lua_setmetatable(L1, idx);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("settable") {
      lua_settable(L1, getindex);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("settop") {
      lua_settop(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("testudata") {
      ਪੂਰਨ i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ਨਿਰਾ);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("error") {
      lua_error(L1);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("abort") {
      abort();
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("throw") {
#ਜੇ defined(__cplusplus)
ਸਥਿਰ ਬਣਤਰ X { ਪੂਰਨ x; } x;
      throw x;
#ਨਹੀਂਤਾਂ
      luaL_error(L1, "C++");
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("tostring") {
      ਅਚਲ ਅੱਖਰ *s = lua_tostring(L1, getindex);
      ਅਚਲ ਅੱਖਰ *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ਨਿਰਾ && s1 == ਨਿਰਾ) || strcmp(s, s1) == 0);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ਨਿਰਾ);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("xmove") {
      ਪੂਰਨ f = getindex;
      ਪੂਰਨ t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ਪੂਰਨ n = getnum;
      ਜੇ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("yield") {
      ਵਾਪਸ lua_yield(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("yieldk") {
      ਪੂਰਨ nres = getnum;
      ਪੂਰਨ i = getindex;
      ਵਾਪਸ lua_yieldk(L1, nres, i, Cfunck);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    ਨਹੀਂਤਾਂ ਜੇ EQ("argerror") {
      ਪੂਰਨ arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    ਨਹੀਂਤਾਂ luaL_error(L, "unknown instruction %s", buff);
  }
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ testC (lua_State *L) {
  lua_State *L1;
  ਅਚਲ ਅੱਖਰ *pc;
  ਜੇ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  ਨਹੀਂਤਾਂ ਜੇ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  ਨਹੀਂਤਾਂ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ਵਾਪਸ ਲੜੀਵਾਰ_C(L, L1, pc);
}


ਸਥਿਰ ਪੂਰਨ Cfunc (lua_State *L) {
  ਵਾਪਸ ਲੜੀਵਾਰ_C(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


ਸਥਿਰ ਪੂਰਨ Cfunck (lua_State *L, ਪੂਰਨ ਸਥਿਤੀ, lua_KContext ctx) {
  lua_pushstring(L, statcodes[ਸਥਿਤੀ]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ਵਾਪਸ ਲੜੀਵਾਰ_C(L, L, lua_tostring(L, cast_int(ctx)));
}


ਸਥਿਰ ਪੂਰਨ makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਖਾਲੀ Chook (lua_State *L, lua_Debug *ar) {
  ਅਚਲ ਅੱਖਰ *scpt;
  ਅਚਲ ਅੱਖਰ *ਅਚਲ events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_ਕੱਢੋ(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  ਲੜੀਵਾਰ_C(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
ਸਥਿਰ ਖਾਲੀ sethookaux (lua_State *L, ਪੂਰਨ ਨਕਾਬ_2, ਪੂਰਨ ਗਿਣਤੀ_2, ਅਚਲ ਅੱਖਰ *scpt) {
  ਜੇ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ਨਿਰਾ, 0, 0);  /* turn off hooks */
    ਵਾਪਸ;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ਜੇ (!lua_istable(L, -1)) {  /* no hook table? */
    lua_ਕੱਢੋ(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, ਨਕਾਬ_2, ਗਿਣਤੀ_2);
}


ਸਥਿਰ ਪੂਰਨ sethook (lua_State *L) {
  ਜੇ (lua_isnoneornil(L, 1))
    lua_sethook(L, ਨਿਰਾ, 0, 0);  /* turn off hooks */
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *scpt = luaL_checkstring(L, 1);
    ਅਚਲ ਅੱਖਰ *smask = luaL_checkstring(L, 2);
    ਪੂਰਨ ਗਿਣਤੀ_2 = cast_int(luaL_optinteger(L, 3, 0));
    ਪੂਰਨ ਨਕਾਬ_2 = 0;
    ਜੇ (strchr(smask, 'c')) ਨਕਾਬ_2 |= LUA_MASKCALL;
    ਜੇ (strchr(smask, 'r')) ਨਕਾਬ_2 |= LUA_MASKRET;
    ਜੇ (strchr(smask, 'l')) ਨਕਾਬ_2 |= LUA_MASKLINE;
    ਜੇ (ਗਿਣਤੀ_2 > 0) ਨਕਾਬ_2 |= LUA_MASKCOUNT;
    sethookaux(L, ਨਕਾਬ_2, ਗਿਣਤੀ_2, scpt);
  }
  ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ coresume (lua_State *L) {
  ਪੂਰਨ ਸਥਿਤੀ, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  ਸਥਿਤੀ = lua_resume(co, L, 0, &nres);
  ਜੇ (ਸਥਿਤੀ != LUA_OK && ਸਥਿਤੀ != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_ਪਾਓ(L, -2);
    ਵਾਪਸ 2;  /* return false + error message */
  }
  ਨਹੀਂਤਾਂ {
    lua_pushboolean(L, 1);
    ਵਾਪਸ 1;
  }
}

#ਜੇ !defined(LUA_USE_POSIX)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ nonblock	ਨਿਰਾ

#ਨਹੀਂਤਾਂ

#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>

ਸਥਿਰ ਪੂਰਨ nonblock (lua_State *L) {
  FILE *f = cast(lua_L_ਧਾਰਾ*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ਪੂਰਨ fd = fileno(f);
  ਪੂਰਨ flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

/* }====================================================== */



ਸਥਿਰ ਅਚਲ ਬਣਤਰ luaL_Reg tests_funcs[] = {
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
  {"querytab", ਸਾਰਣੀ_query},
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
  {"alloccount", alloc_ਗਿਣਤੀ},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {ਨਿਰਾ, ਨਿਰਾ}
};


ਸਥਿਰ ਖਾਲੀ checkfinalmem (ਖਾਲੀ) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.ਕੁੱਲ == 0);
}


ਪੂਰਨ luaB_opentests (lua_State *L) {
  ਖਾਲੀ *ud;
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
  ਵਾਪਸ 1;
}

#ਜੇ_ਅੰਤ

