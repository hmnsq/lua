/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ltests_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

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
#ಆದರೆ defined(LUA_DEBUG)


ಶೂನ್ಯ *l_Trick = 0;


#ವ್ಯಾಖ್ಯಾನಿಸು obj_at(L,k)	s2v(L->ci->func.p + (k))


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_C (lua_State *L, lua_State *L1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pc);


ಸ್ಥಿರ ಶೂನ್ಯ setnameval (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


ಸ್ಥಿರ ಶೂನ್ಯ pushobject (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o) {
  setobj2s(L, L->ತುದಿ.p, o);
  api_incr_ತುದಿ(L);
}


ಸ್ಥಿರ ಶೂನ್ಯ badexit (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2) {
  fprintf(stderr, fmt, s1);
  ಆದರೆ (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.ಒಟ್ಟು = 0;
  ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tpanic (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  ಹಿಂತಿರುಗಿ (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   ಸಂದೇಶ, ಖಾಲಿ),
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
ಸ್ಥಿರ ಶೂನ್ಯ warnf (ಶೂನ್ಯ *ud, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ tocont) {
  lua_State *L = cast(lua_State *, ud);
  ಸ್ಥಿರ ಅಕ್ಷರ buff[200] = "";  /* should be enough for tests... */
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ onoff = 0;
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಬಹುಲಕ = 0;  /* start in normal mode */
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lasttocont = 0;
  ಆದರೆ (!lasttocont && !tocont && *ಸಂದೇಶ == '@') {  /* control message? */
    ಆದರೆ (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", ಸಂದೇಶ, buff);
    ಆದರೆ (strcmp(ಸಂದೇಶ, "@off") == 0)
      onoff = 0;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ಸಂದೇಶ, "@on") == 0)
      onoff = 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ಸಂದೇಶ, "@normal") == 0)
      ಬಹುಲಕ = 0;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ಸಂದೇಶ, "@allow") == 0)
      ಬಹುಲಕ = 1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ಸಂದೇಶ, "@store") == 0)
      ಬಹುಲಕ = 2;
    ಇಲ್ಲದಿದ್ದರೆ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              ಸಂದೇಶ, ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ;
  }
  lasttocont = tocont;
  ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಸಂದೇಶ) >= ಗಾತ್ರ(buff) - ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buff))
    badexit("warnf-buffer overflow (%s)\n", ಸಂದೇಶ, buff);
  strcat(buff, ಸಂದೇಶ);  /* add new message to current warning */
  ಆದರೆ (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    ಆದರೆ (!lua_toboolean(L, -1))
      lua_ತೆಗೆ(L, 1);  /* ok, no previous unexpected warning */
    ಇಲ್ಲದಿದ್ದರೆ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_ಬೀಗ(L);
    ಆಯ್ಕೆ (ಬಹುಲಕ) {
      ಸ್ಥಿತಿ 0: {  /*ಸಾಮಾನ್ಯ*/
        ಆದರೆ (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, ಖಾಲಿ);
      }  /* FALLTHROUGH */
      ಸ್ಥಿತಿ 1: {  /* allow */
        ಆದರೆ (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_ಬೀಗ(L);
        ನಿಲ್ಲಿಸು;
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

#ವ್ಯಾಖ್ಯಾನಿಸು MARK		0x55  /* 01010101 (a nice pattern) */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ mem_ಶೀರ್ಷಿಕೆ {
  LUAI_MAXALIGN;
  ರಚನೆ {
    size_t ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ type;
  } d;
} mem_ಶೀರ್ಷಿಕೆ;


#ಆದರೆ !defined(EXTERNMEMCHECK)

/* full memory check */
#ವ್ಯಾಖ್ಯಾನಿಸು MARKSIZE	16  /* size of marks after each block */
#ವ್ಯಾಖ್ಯಾನಿಸು fillmem(mem,ಗಾತ್ರ_2)	memset(mem, -MARK, ಗಾತ್ರ_2)

#ಇಲ್ಲದಿದ್ದರೆ

/* external memory check: don't do it twice */
#ವ್ಯಾಖ್ಯಾನಿಸು MARKSIZE	0
#ವ್ಯಾಖ್ಯಾನಿಸು fillmem(mem,ಗಾತ್ರ_2)	/*ಖಾಲಿಗಳು*/

#ಆದರೆ_ಅಂತ್ಯ


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


ಸ್ಥಿರ ಶೂನ್ಯ freeblock (Memcontrol *mc, mem_ಶೀರ್ಷಿಕೆ *ಖಂಡ) {
  ಆದರೆ (ಖಂಡ) {
    size_t ಗಾತ್ರ_2 = ಖಂಡ->d.ಗಾತ್ರ_2;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(ಖಂಡ + 1) + ಗಾತ್ರ_2 + i) == MARK);
    mc->objcount[ಖಂಡ->d.type]--;
    fillmem(ಖಂಡ, ಗಾತ್ರ(mem_ಶೀರ್ಷಿಕೆ) + ಗಾತ್ರ_2 + MARKSIZE);  /* erase block */
    ಬಿಡುಗಡೆ(ಖಂಡ);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->ಒಟ್ಟು -= ಗಾತ್ರ_2;
  }
}


ಶೂನ್ಯ *debug_realloc (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *b, size_t oldsize, size_t ಗಾತ್ರ_2) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  mem_ಶೀರ್ಷಿಕೆ *ಖಂಡ = cast(mem_ಶೀರ್ಷಿಕೆ *, b);
  ಪೂರ್ಣಾಂಕ type;
  ಆದರೆ (mc->memlimit == 0) {  /* first time? */
    ಅಕ್ಷರ *ಎಲ್ಲೆ = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = ಎಲ್ಲೆ ? strtoul(ಎಲ್ಲೆ, ಖಾಲಿ, 10) : ULONG_MAX;
  }
  ಆದರೆ (ಖಂಡ == ಖಾಲಿ) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಖಂಡ--;  /* go to real header */
    type = ಖಂಡ->d.type;
    lua_assert(oldsize == ಖಂಡ->d.ಗಾತ್ರ_2);
  }
  ಆದರೆ (ಗಾತ್ರ_2 == 0) {
    freeblock(mc, ಖಂಡ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಆದರೆ (mc->failnext) {
    mc->failnext = 0;
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* fake a single memory allocation error */
  }
  ಆದರೆ (mc->countlimit != ~0UL && ಗಾತ್ರ_2 != oldsize) {  /* count limit in use? */
    ಆದರೆ (mc->countlimit == 0)
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  ಆದರೆ (ಗಾತ್ರ_2 > oldsize && mc->ಒಟ್ಟು+ಗಾತ್ರ_2-oldsize > mc->memlimit)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* fake a memory allocation error */
  ಇಲ್ಲದಿದ್ದರೆ {
    mem_ಶೀರ್ಷಿಕೆ *newblock;
    ಪೂರ್ಣಾಂಕ i;
    size_t commonsize = (oldsize < ಗಾತ್ರ_2) ? oldsize : ಗಾತ್ರ_2;
    size_t realsize = ಗಾತ್ರ(mem_ಶೀರ್ಷಿಕೆ) + ಗಾತ್ರ_2 + MARKSIZE;
    ಆದರೆ (realsize < ಗಾತ್ರ_2) ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* arithmetic overflow! */
    newblock = cast(mem_ಶೀರ್ಷಿಕೆ *, ಸ್ಮರಣೆ_ಕೊಡು(realsize));  /* alloc a new block */
    ಆದರೆ (newblock == ಖಾಲಿ)
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* really out of memory? */
    ಆದರೆ (ಖಂಡ) {
      ಸ್ಮರಣೆ_ನಕಲು(newblock + 1, ಖಂಡ + 1, commonsize);  /* copy old contents */
      freeblock(mc, ಖಂಡ);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, ಗಾತ್ರ_2 - commonsize);
    /* initialize marks after block */
    ಗಾಗಿ (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + ಗಾತ್ರ_2 + i) = MARK;
    newblock->d.ಗಾತ್ರ_2 = ಗಾತ್ರ_2;
    newblock->d.type = type;
    mc->ಒಟ್ಟು += ಗಾತ್ರ_2;
    ಆದರೆ (mc->ಒಟ್ಟು > mc->maxmem)
      mc->maxmem = mc->ಒಟ್ಟು;
    mc->numblocks++;
    mc->objcount[type]++;
    ಹಿಂತಿರುಗಿ newblock + 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  ಆದರೆ (isdead(g,t)) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (issweepphase(g))
    ಹಿಂತಿರುಗಿ 1;  /* no invariants */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (g->gckind != KGC_GENMINOR)
    ಹಿಂತಿರುಗಿ !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  ಇಲ್ಲದಿದ್ದರೆ {  /* generational mode */
    ಆದರೆ ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ 1;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ printobj (global_State *g, GCObject *o) {
  ಬರೆ("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (ಶೂನ್ಯ *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  ಆದರೆ (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    ಬರೆ(" '%s'", getstr(gco2ts(o)));
}


ಶೂನ್ಯ lua_printobj (lua_State *L, ರಚನೆ GCObject *o) {
  printobj(G(L), o);
}


ಶೂನ್ಯ lua_printvalue (TValue *v) {
  ಆಯ್ಕೆ (ttypetag(v)) {
    ಸ್ಥಿತಿ LUA_VNUMINT: ಸ್ಥಿತಿ LUA_VNUMFLT: {
      ಅಕ್ಷರ buff[LUA_N2SBUFFSZ];
      ಚಿಹ್ನೆರಹಿತ len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      ಬರೆ("%s", buff);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VSHRSTR:
      ಬರೆ("'%s'", getstr(tsvalue(v))); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VLNGSTR:
      ಬರೆ("'%.30s...'", getstr(tsvalue(v))); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VFALSE:
      ಬರೆ("%s", "false"); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VTRUE:
      ಬರೆ("%s", "true"); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VLIGHTUSERDATA:
      ಬರೆ("light udata: %p", pvalue(v)); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VUSERDATA:
      ಬರೆ("full udata: %p", uvalue(v)); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VNIL:
      ಬರೆ("nil"); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VLCF:
      ಬರೆ("light C function: %p", fvalue(v)); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VCCL:
      ಬರೆ("C closure: %p", clCvalue(v)); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VLCL:
      ಬರೆ("Lua function: %p", clLvalue(v)); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VTHREAD:
      ಬರೆ("thread: %p", thvalue(v)); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VTABLE:
      ಬರೆ("table: %p", hvalue(v)); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      lua_assert(0);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testobjref (global_State *g, GCObject *f, GCObject *t) {
  ಪೂರ್ಣಾಂಕ r1 = testobjref1(g, f, t);
  ಆದರೆ (!r1) {
    ಬರೆ("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    ಬರೆ("  ->  ");
    printobj(g, t);
    ಬರೆ("\n");
  }
  ಹಿಂತಿರುಗಿ r1;
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
#ವ್ಯಾಖ್ಯಾನಿಸು checkobjrefN(g,f,t)	{ ಆದರೆ (t) checkobjref(g,f,obj2gco(t)); }


ಸ್ಥಿರ ಶೂನ್ಯ checkvalref (global_State *g, GCObject *f, ಸ್ಥಿರಾಂಕ TValue *t) {
  ಪ್ರತಿಜ್ಞಾ(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


ಸ್ಥಿರ ಶೂನ್ಯ checktable (global_State *g, ಕೋಷ್ಟಕ *h) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ asize = h->asize;
  ಗಂಟು_2 *n, *ಎಲ್ಲೆ = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  ಗಾಗಿ (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  ಗಾಗಿ (n = gnode(h, 0); n < ಎಲ್ಲೆ; n++) {
    ಆದರೆ (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      ಪ್ರತಿಜ್ಞಾ(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ checkudata (global_State *g, Udata *u) {
  ಪೂರ್ಣಾಂಕ i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  ಗಾಗಿ (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkproto (global_State *g, Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->ಮೂಲ);
  ಗಾಗಿ (i=0; i<f->sizek; i++) {
    ಆದರೆ (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  ಗಾಗಿ (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  ಗಾಗಿ (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  ಗಾಗಿ (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  ಪೂರ್ಣಾಂಕ i;
  checkobjrefN(g, clgc, cl->p);
  ಗಾಗಿ (i=0; i<cl->nupvalues; i++) {
    ಮೇಲೆ_Val *uv = cl->upvals[i];
    ಆದರೆ (uv) {
      checkobjrefN(g, clgc, uv);
      ಆದರೆ (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lua_checkpc (CallInfo *ci) {
  ಆದರೆ (!isLua(ci)) ಹಿಂತಿರುಗಿ 1;
  ಇಲ್ಲದಿದ್ದರೆ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    ಹಿಂತಿರುಗಿ p->ಸಂಕೇತ_2 <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->ಸಂಕೇತ_2 + p->sizecode;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ check_ರಾಶಿ (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  ಮೇಲೆ_Val *uv;
  ಪ್ರತಿಜ್ಞಾ(!isdead(g, L1));
  ಆದರೆ (L1->ರಾಶಿ.p == ಖಾಲಿ) {  /* incomplete thread? */
    ಪ್ರತಿಜ್ಞಾ(L1->openupval == ಖಾಲಿ && L1->ci == ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ;
  }
  ಗಾಗಿ (uv = L1->openupval; uv != ಖಾಲಿ; uv = uv->u.open.ಮುಂದಿನ)
    ಪ್ರತಿಜ್ಞಾ(upisopen(uv));  /* must be open */
  ಪ್ರತಿಜ್ಞಾ(L1->ತುದಿ.p <= L1->ರಾಶಿ_ಕೊನೆಯ.p);
  ಪ್ರತಿಜ್ಞಾ(L1->tbclist.p <= L1->ತುದಿ.p);
  ಗಾಗಿ (ci = L1->ci; ci != ಖಾಲಿ; ci = ci->ಹಿಂದಿನ) {
    ಪ್ರತಿಜ್ಞಾ(ci->ತುದಿ.p <= L1->ರಾಶಿ_ಕೊನೆಯ.p);
    ಪ್ರತಿಜ್ಞಾ(lua_checkpc(ci));
  }
  ಗಾಗಿ (o = L1->ರಾಶಿ.p; o < L1->ರಾಶಿ_ಕೊನೆಯ.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


ಸ್ಥಿರ ಶೂನ್ಯ checkrefs (global_State *g, GCObject *o) {
  ಆಯ್ಕೆ (o->tt) {
    ಸ್ಥಿತಿ LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VTABLE: {
      checktable(g, gco2t(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VTHREAD: {
      check_ರಾಶಿ(g, gco2th(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VPROTO: {
      checkproto(g, gco2p(o));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ LUA_VSHRSTR:
    ಸ್ಥಿತಿ LUA_VLNGSTR: {
      ಪ್ರತಿಜ್ಞಾ(!isgray(o));  /* strings are never gray */
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: ಪ್ರತಿಜ್ಞಾ(0);
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
ಸ್ಥಿರ ಶೂನ್ಯ checkobject (global_State *g, GCObject *o, ಪೂರ್ಣಾಂಕ maybedead,
                         ಪೂರ್ಣಾಂಕ listage) {
  ಆದರೆ (isdead(g, o))
    ಪ್ರತಿಜ್ಞಾ(maybedead);
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪ್ರತಿಜ್ಞಾ(g->gcstate != GCSpause || iswhite(o));
    ಆದರೆ (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      ಪ್ರತಿಜ್ಞಾ(getage(o) >= listage);
      ಆದರೆ (isold(o)) {
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
  ಪೂರ್ಣಾಂಕ ಒಟ್ಟು = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  ಆಗ (o) {
    ಪ್ರತಿಜ್ಞಾ(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    ಪ್ರತಿಜ್ಞಾ(!testbit(o->marked, TESTBIT));
    ಆದರೆ (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    ಒಟ್ಟು++;
    ಆಯ್ಕೆ (o->tt) {
      ಸ್ಥಿತಿ LUA_VTABLE: o = gco2t(o)->gclist; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VLCL: o = gco2lcl(o)->gclist; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VCCL: o = gco2ccl(o)->gclist; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VTHREAD: o = gco2th(o)->gclist; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VPROTO: o = gco2p(o)->gclist; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VUSERDATA:
        ಪ್ರತಿಜ್ಞಾ(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ: ಪ್ರತಿಜ್ಞಾ(0);  /* other objects cannot be in a gray list */
    }
  }
  ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
}


/*
** Check objects in gray lists.
*/
ಸ್ಥಿರ l_mem checkgrays (global_State *g) {
  l_mem ಒಟ್ಟು = 0;  /* count number of elements in all lists */
  ಆದರೆ (!keepinvariant(g)) ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
  ಒಟ್ಟು += checkgraylist(g, g->ಬೂದು);
  ಒಟ್ಟು += checkgraylist(g, g->grayagain);
  ಒಟ್ಟು += checkgraylist(g, g->weak);
  ಒಟ್ಟು += checkgraylist(g, g->allweak);
  ಒಟ್ಟು += checkgraylist(g, g->ephemeron);
  ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ incifingray (global_State *g, GCObject *o, l_mem *ಎಣಿಕೆ_2) {
  ಆದರೆ (!keepinvariant(g))
    ಹಿಂತಿರುಗಿ;  /* gray lists not being kept in these phases */
  ಆದರೆ (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    ಪ್ರತಿಜ್ಞಾ(!isgray(o) || upisopen(gco2upv(o)));
    ಹಿಂತಿರುಗಿ;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  ಆದರೆ (isgray(o) || getage(o) == G_TOUCHED2) {
    (*ಎಣಿಕೆ_2)++;
    ಪ್ರತಿಜ್ಞಾ(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


ಸ್ಥಿರ l_mem checklist (global_State *g, ಪೂರ್ಣಾಂಕ maybedead, ಪೂರ್ಣಾಂಕ tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem ಒಟ್ಟು = 0;  /* number of object that should be in  gray lists */
  ಗಾಗಿ (o = newl; o != survival; o = o->ಮುಂದಿನ) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &ಒಟ್ಟು);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಗಾಗಿ (o = survival; o != old; o = o->ಮುಂದಿನ) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &ಒಟ್ಟು);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಗಾಗಿ (o = old; o != reallyold; o = o->ಮುಂದಿನ) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &ಒಟ್ಟು);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಗಾಗಿ (o = reallyold; o != ಖಾಲಿ; o = o->ಮುಂದಿನ) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &ಒಟ್ಟು);
    ಪ್ರತಿಜ್ಞಾ(!tof == !tofinalize(o));
  }
  ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
}


ಪೂರ್ಣಾಂಕ lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  ಪೂರ್ಣಾಂಕ maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  ಆದರೆ (keepinvariant(g)) {
    ಪ್ರತಿಜ್ಞಾ(!iswhite(mainthread(g)));
    ಪ್ರತಿಜ್ಞಾ(!iswhite(gcvalue(&g->l_registry)));
  }
  ಪ್ರತಿಜ್ಞಾ(!isdead(g, gcvalue(&g->l_registry)));
  ಪ್ರತಿಜ್ಞಾ(g->sweepgc == ಖಾಲಿ || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  ಗಾಗಿ (o = g->fixedgc; o != ಖಾಲಿ; o = o->ಮುಂದಿನ) {
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
  ಗಾಗಿ (o = g->tobefnz; o != ಖಾಲಿ; o = o->ಮುಂದಿನ) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    ಪ್ರತಿಜ್ಞಾ(tofinalize(o));
    ಪ್ರತಿಜ್ಞಾ(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  ಆದರೆ (keepinvariant(g))
    ಪ್ರತಿಜ್ಞಾ(totalin == totalshould);
  ಹಿಂತಿರುಗಿ 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


ಸ್ಥಿರ ಅಕ್ಷರ *buildop (Proto *p, ಪೂರ್ಣಾಂಕ pc, ಅಕ್ಷರ *buff) {
  ಅಕ್ಷರ *obuff = buff;
  Instruction i = p->ಸಂಕೇತ_2[pc];
  Op_ಸಂಕೇತ o = GET_OPCODE(i);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = opnames[o];
  ಪೂರ್ಣಾಂಕ ಸಾಲು = luaG_getfuncline(p, pc);
  ಪೂರ್ಣಾಂಕ lineinfo = (p->lineinfo != ಖಾಲಿ) ? p->lineinfo[pc] : 0;
  ಆದರೆ (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  ಇಲ್ಲದಿದ್ದರೆ
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", ಸಾಲು, pc);
  ಆಯ್ಕೆ (get_Op_ಬಹುಲಕ(o)) {
    ಸ್ಥಿತಿ iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ obuff;
}


#ಆದರೆ 0
ಶೂನ್ಯ luaI_printcode (Proto *pt, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2) {
  ಪೂರ್ಣಾಂಕ pc;
  ಗಾಗಿ (pc=0; pc<ಗಾತ್ರ_2; pc++) {
    ಅಕ್ಷರ buff[100];
    ಬರೆ("%s\n", buildop(pt, pc, buff));
  }
  ಬರೆ("-------\n");
}


ಶೂನ್ಯ luaI_printinst (Proto *pt, ಪೂರ್ಣಾಂಕ pc) {
  ಅಕ್ಷರ buff[100];
  ಬರೆ("%s\n", buildop(pt, pc, buff));
}
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ listcode (lua_State *L) {
  ಪೂರ್ಣಾಂಕ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  ಗಾಗಿ (pc=0; pc<p->sizecode; pc++) {
    ಅಕ್ಷರ buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ printcode (lua_State *L) {
  ಪೂರ್ಣಾಂಕ pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ಬರೆ("maxstack: %d\n", p->maxstacksize);
  ಬರೆ("numparams: %d\n", p->numparams);
  ಗಾಗಿ (pc=0; pc<p->sizecode; pc++) {
    ಅಕ್ಷರ buff[100];
    ಬರೆ("%s\n", buildop(p, pc, buff));
  }
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ listk (lua_State *L) {
  Proto *p;
  ಪೂರ್ಣಾಂಕ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  ಗಾಗಿ (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ listabslineinfo (lua_State *L) {
  Proto *p;
  ಪೂರ್ಣಾಂಕ i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != ಖಾಲಿ, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  ಗಾಗಿ (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].ಸಾಲು);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ listlocals (lua_State *L) {
  Proto *p;
  ಪೂರ್ಣಾಂಕ pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  ಪೂರ್ಣಾಂಕ i = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  ಆಗ ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != ಖಾಲಿ)
    lua_pushstring(L, name);
  ಹಿಂತಿರುಗಿ i-1;
}

/* }====================================================== */



ಶೂನ್ಯ lua_printstack (lua_State *L) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = lua_gettop(L);
  ಬರೆ("stack: >>\n");
  ಗಾಗಿ (i = 1; i <= n; i++) {
    ಬರೆ("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    ಬರೆ("\n");
  }
  ಬರೆ("<<\n");
}


ಪೂರ್ಣಾಂಕ lua_printallstack (lua_State *L) {
  StkId p;
  ಪೂರ್ಣಾಂಕ i = 1;
  CallInfo *ci = &L->ತಳ_ci;
  ಬರೆ("stack: >>\n");
  ಗಾಗಿ (p = L->ರಾಶಿ.p; p < L->ತುದಿ.p; p++) {
    ಆದರೆ (ci != ಖಾಲಿ && p == ci->func.p) {
      ಬರೆ("  ---\n");
      ಆದರೆ (ci == L->ci)
        ci = ಖಾಲಿ;  /* printed last frame */
      ಇಲ್ಲದಿದ್ದರೆ
        ci = ci->ಮುಂದಿನ;
    }
    ಬರೆ("%3d: ", i++);
    lua_printvalue(s2v(p));
    ಬರೆ("\n");
  }
  ಬರೆ("<<\n");
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", ಸ್ಥಾನಾಂತರ_s_Bx);
  setnameval(L, "NUM_OPCODES", ಅಂಕೆ_OPCODES);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", ಗಾತ್ರ(lua_State));
  setnameval(L, "global state", ಗಾತ್ರ(global_State));
  setnameval(L, "TValue", ಗಾತ್ರ(TValue));
  setnameval(L, "Node", ಗಾತ್ರ(ಗಂಟು_2));
  setnameval(L, "stack Value", ಗಾತ್ರ(ರಾಶಿ_ಮೌಲ್ಯ));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mem_query (lua_State *L) {
  ಆದರೆ (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.ಒಟ್ಟು));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    ಹಿಂತಿರುಗಿ 3;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lua_isnumber(L, 1)) {
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಎಲ್ಲೆ = cast(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ, luaL_checkinteger(L, 1));
    ಆದರೆ (ಎಲ್ಲೆ == 0) ಎಲ್ಲೆ = ULONG_MAX;
    l_memcontrol.memlimit = ಎಲ್ಲೆ;
    ಹಿಂತಿರುಗಿ 0;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t = luaL_checkstring(L, 1);
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      ಆದರೆ (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        ಹಿಂತಿರುಗಿ 1;
      }
    }
    ಹಿಂತಿರುಗಿ luaL_error(L, "unknown type '%s'", t);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ alloc_ಎಣಿಕೆ (lua_State *L) {
  ಆದರೆ (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ, ~0L);
  ಇಲ್ಲದಿದ್ದರೆ
    l_memcontrol.countlimit = cast(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ, luaL_checkinteger(L, 1));
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ alloc_failnext (lua_State *L) {
  ಬಳಸದ(L);
  l_memcontrol.failnext = 1;
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ settrick (lua_State *L) {
  ಆದರೆ (ttisnil(obj_at(L, 1)))
    l_Trick = ಖಾಲಿ;
  ಇಲ್ಲದಿದ್ದರೆ
    l_Trick = gcvalue(obj_at(L, 1));
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ಆದರೆ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ಇಲ್ಲದಿದ್ದರೆ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ಆದರೆ (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  ಆದರೆ (!iscollectable(o))
    ಬರೆ("no collectable\n");
  ಇಲ್ಲದಿದ್ದರೆ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    ಬರೆ("\n");
  }
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gc_state (lua_State *L) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿಗಳು[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  ಪೂರ್ಣಾಂಕ option = ಸ್ಥಿತಿಗಳು[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  ಆದರೆ (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_ಬೀಗ(L);
    ಆದರೆ (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    ಹಿಂತಿರುಗಿ 0;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tracinggc = 0;
ಶೂನ್ಯ luai_tracegctest (lua_State *L, ಪೂರ್ಣಾಂಕ ಮೊದಲ) {
  ಆದರೆ (!tracinggc) ಹಿಂತಿರುಗಿ;
  ಇಲ್ಲದಿದ್ದರೆ {
    global_State *g = G(L);
    lua_unlock(L);
    g->gcstp = GCSTPGC;
    lua_checkstack(L, 10);
    lua_getfield(L, LUA_REGISTRYINDEX, "tracegc");
    lua_pushboolean(L, ಮೊದಲ);
    lua_call(L, 1, 0);
    g->gcstp = 0;
    lua_ಬೀಗ(L);
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tracegc (lua_State *L) {
  ಆದರೆ (lua_isnil(L, 1))
    tracinggc = 0;
  ಇಲ್ಲದಿದ್ದರೆ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hash_query (lua_State *L) {
  ಆದರೆ (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    ಆದರೆ (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    TValue *o = obj_at(L, 1);
    ಕೋಷ್ಟಕ *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->ಗಂಟು));
  }
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stacklevel (lua_State *L) {
  ಪೂರ್ಣಾಂಕ a = 0;
  lua_pushinteger(L, cast_Integer(L->ತುದಿ.p - L->ರಾಶಿ.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  ಹಿಂತಿರುಗಿ 5;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  ಆಗ (ci->ಮುಂದಿನ != ಖಾಲಿ) {
    CallInfo *tofree = ci->ಮುಂದಿನ;
    ci->ಮುಂದಿನ = ci->ಮುಂದಿನ->ಮುಂದಿನ;
    luaM_free(L, tofree);
    L->nci--;
  }
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reallocstack (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = cast_int(luaL_checkinteger(L, 1));
  lua_ಬೀಗ(L);
  luaD_reallocstack(L, cast_int(L->ತುದಿ.p - L->ರಾಶಿ.p) + n, 1);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_query (lua_State *L) {
  ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *t;
  ಪೂರ್ಣಾಂಕ i = cast_int(luaL_optinteger(L, 2, -1));
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  ಆದರೆ (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    ಹಿಂತಿರುಗಿ 3;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    ಆದರೆ (!tagisempty(*get_Arr_ಟ್ಯಾಗ್(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->ತುದಿ.p));
    ಇಲ್ಲದಿದ್ದರೆ
      setnilvalue2s(L->ತುದಿ.p);
    api_incr_ತುದಿ(L);
    lua_pushnil(L);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    ಆದರೆ (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    ಇಲ್ಲದಿದ್ದರೆ
      lua_pushliteral(L, "<undef>");
    ಆದರೆ (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    ಇಲ್ಲದಿದ್ದರೆ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->ಗಂಟು[i]));
  }
  ಹಿಂತಿರುಗಿ 3;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  ಹಿಂತಿರುಗಿ 6;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  ಪೂರ್ಣಾಂಕ s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  ಆದರೆ (s == -1) {
    lua_pushinteger(L ,tb->ಗಾತ್ರ_2);
    lua_pushinteger(L ,tb->nuse);
    ಹಿಂತಿರುಗಿ 2;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s < tb->ಗಾತ್ರ_2) {
    TString *ts;
    ಪೂರ್ಣಾಂಕ n = 0;
    ಗಾಗಿ (ts = tb->hash[s]; ts != ಖಾಲಿ; ts = ts->u.hnext) {
      setsvalue2s(L, L->ತುದಿ.p, ts);
      api_incr_ತುದಿ(L);
      n++;
    }
    ಹಿಂತಿರುಗಿ n;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getreftable (lua_State *L) {
  ಆದರೆ (lua_istable(L, 2))  /* is there a table as second argument? */
    ಹಿಂತಿರುಗಿ 2;  /* use it as the table */
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ LUA_REGISTRYINDEX;  /* default is to use the register */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tref (lua_State *L) {
  ಪೂರ್ಣಾಂಕ t = getreftable(L);
  ಪೂರ್ಣಾಂಕ level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getref (lua_State *L) {
  ಪೂರ್ಣಾಂಕ t = getreftable(L);
  ಪೂರ್ಣಾಂಕ level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ unref (lua_State *L) {
  ಪೂರ್ಣಾಂಕ t = getreftable(L);
  ಪೂರ್ಣಾಂಕ level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ upvalue (lua_State *L) {
  ಪೂರ್ಣಾಂಕ n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ಆದರೆ (lua_isnone(L, 3)) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = lua_getupvalue(L, 1, n);
    ಆದರೆ (name == ಖಾಲಿ) ಹಿಂತಿರುಗಿ 0;
    lua_pushstring(L, name);
    ಹಿಂತಿರುಗಿ 2;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    ಹಿಂತಿರುಗಿ 1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newuserdata (lua_State *L) {
  size_t ಗಾತ್ರ_2 = cast_sizet(luaL_optinteger(L, 1, 0));
  ಪೂರ್ಣಾಂಕ nuv = cast_int(luaL_optinteger(L, 2, 0));
  ಅಕ್ಷರ *p = cast_charp(lua_newuserdatauv(L, ಗಾತ್ರ_2, nuv));
  ಆಗ (ಗಾತ್ರ_2--) *p++ = '\0';
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &l);
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = luaL_loadbufferx(L1, s, l, s, "t");
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK)
    ಸ್ಥಿತಿ_2 = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, ಸ್ಥಿತಿ_2);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ s2d (lua_State *L) {
  lua_pushnumber(L, cast_ಅಂಕೆ(*cast(ಸ್ಥಿರಾಂಕ ದ್ವಿಗುಣ *, luaL_checkstring(L, 1))));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ d2s (lua_State *L) {
  ದ್ವಿಗುಣ d = cast(ದ್ವಿಗುಣ, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), ಗಾತ್ರ(d));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newstate (lua_State *L) {
  ಶೂನ್ಯ *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  ಆದರೆ (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  ಇಲ್ಲದಿದ್ದರೆ
    lua_pushnil(L);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != ಖಾಲಿ, 1, "state expected");
  ಹಿಂತಿರುಗಿ L1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  ಪೂರ್ಣಾಂಕ ತುಂಬು = cast_int(luaL_checkinteger(L, 2));
  ಪೂರ್ಣಾಂಕ preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, ತುಂಬು, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", ಖಾಲಿ, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂಕೇತ_2 = luaL_checklstring(L, 2, &lcode);
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  lua_settop(L1, 0);
  ಸ್ಥಿತಿ_2 = luaL_loadbufferx(L1, ಸಂಕೇತ_2, lcode, ಸಂಕೇತ_2, "t");
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_OK)
    ಸ್ಥಿತಿ_2 = lua_pcall(L1, 0, LUA_MULTRET, 0);
  ಆದರೆ (ಸ್ಥಿತಿ_2 != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, ಸ್ಥಿತಿ_2);
    ಹಿಂತಿರುಗಿ 3;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ i = 0;
    ಆಗ (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_ತೆಗೆ(L1, i-1);
    ಹಿಂತಿರುಗಿ i-1;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ log2_aux (lua_State *L) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ x = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  ಹಿಂತಿರುಗಿ 1;
}


ರಚನೆ Aux { jmp_buf jb; ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ panicback (lua_State *L) {
  ರಚನೆ Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (ರಚನೆ Aux *)lua_touserdata(L, -1);
  lua_ತೆಗೆ(L, 1);  /* remove 'Aux' struct */
  ಸತತ_C(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  ಹಿಂತಿರುಗಿ 1;  /* to avoid warnings */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkpanic (lua_State *L) {
  ರಚನೆ Aux b;
  ಶೂನ್ಯ *ud;
  lua_State *L1;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂಕೇತ_2 = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  ಆದರೆ (L1 == ಖಾಲಿ) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    ಹಿಂತಿರುಗಿ 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  ಆದರೆ (setjmp(b.jb) == 0) {  /* set jump buffer */
    ಸತತ_C(L, L1, ಸಂಕೇತ_2);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  ಹಿಂತಿರುಗಿ 1;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ externKstr (lua_State *L) {
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, ಖಾಲಿ, ಖಾಲಿ);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ externstr (lua_State *L) {
  size_t len;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L, 1, &len);
  ಶೂನ್ಯ *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  ಅಕ್ಷರ *buff = cast_charp((*allocf)(ud, ಖಾಲಿ, 0, len + 1));
  ಆದರೆ (buff == ಖಾಲಿ) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  ಸ್ಮರಣೆ_ನಕಲು(buff, s, (len + 1) * ಗಾತ್ರ(ಅಕ್ಷರ));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  ಹಿಂತಿರುಗಿ 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


ಸ್ಥಿರ ಶೂನ್ಯ sethookaux (lua_State *L, ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂಕೇತ_2);

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ delimits = " \t\n,;";

ಸ್ಥಿರ ಶೂನ್ಯ skip (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pc) {
  ಗಾಗಿ (;;) {
    ಆದರೆ (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (**pc == '#') {  /* comment? */
      ಆಗ (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    ಇಲ್ಲದಿದ್ದರೆ ನಿಲ್ಲಿಸು;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getnum_aux (lua_State *L, lua_State *L1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pc) {
  ಪೂರ್ಣಾಂಕ res = 0;
  ಪೂರ್ಣಾಂಕ sig = 1;
  skip(pc);
  ಆದರೆ (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_ತೆಗೆ(L1, 1);
    (*pc)++;
    ಹಿಂತಿರುಗಿ res;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    ಹಿಂತಿರುಗಿ res;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (**pc == '!') {
    (*pc)++;
    ಆದರೆ (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    ಇಲ್ಲದಿದ್ದರೆ lua_assert(0);
    (*pc)++;
    ಹಿಂತಿರುಗಿ res;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  ಆದರೆ (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  ಆಗ (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  ಹಿಂತಿರುಗಿ sig*res;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getstring_aux (lua_State *L, ಅಕ್ಷರ *buff, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pc) {
  ಪೂರ್ಣಾಂಕ i = 0;
  skip(pc);
  ಆದರೆ (**pc == '"' || **pc == '\'') {  /* quoted string? */
    ಪೂರ್ಣಾಂಕ quote = *(*pc)++;
    ಆಗ (**pc != quote) {
      ಆದರೆ (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಆಗ (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  ಹಿಂತಿರುಗಿ buff;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getindex_aux (lua_State *L, lua_State *L1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pc) {
  skip(pc);
  ಆಯ್ಕೆ (*(*pc)++) {
    ಸ್ಥಿತಿ 'R': ಹಿಂತಿರುಗಿ LUA_REGISTRYINDEX;
    ಸ್ಥಿತಿ 'U': ಹಿಂತಿರುಗಿ lua_upvalueindex(getnum_aux(L, L1, pc));
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪೂರ್ಣಾಂಕ n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      ಆದರೆ (n == 0) ಹಿಂತಿರುಗಿ 0;
      ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ lua_absindex(L1, n);
    }
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
ಸ್ಥಿರ ಶೂನ್ಯ regcodes (lua_State *L) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < ಗಾತ್ರ(statcodes) / ಗಾತ್ರ(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#ವ್ಯಾಖ್ಯಾನಿಸು EQ(s1)	(strcmp(s1, inst) == 0)

#ವ್ಯಾಖ್ಯಾನಿಸು getnum		(getnum_aux(L, L1, &pc))
#ವ್ಯಾಖ್ಯಾನಿಸು getstring	(getstring_aux(L, buff, &pc))
#ವ್ಯಾಖ್ಯಾನಿಸು getindex	(getindex_aux(L, L1, &pc))


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testC (lua_State *L);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Cfunck (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ ops[] = "+-*%^/\\&|~<>_!";

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_C (lua_State *L, lua_State *L1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pc) {
  ಅಕ್ಷರ buff[300];
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = 0;
  ಆದರೆ (pc == ಖಾಲಿ) ಹಿಂತಿರುಗಿ luaL_error(L, "attempt to runC null script");
  ಗಾಗಿ (;;) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *inst = getstring;
    ಆದರೆ EQ("") ಹಿಂತಿರುಗಿ 0;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("append") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      ಪೂರ್ಣಾಂಕ i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("arith") {
      ಪೂರ್ಣಾಂಕ op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("call") {
      ಪೂರ್ಣಾಂಕ narg = getnum;
      ಪೂರ್ಣಾಂಕ nres = getnum;
      lua_call(L1, narg, nres);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("callk") {
      ಪೂರ್ಣಾಂಕ narg = getnum;
      ಪೂರ್ಣಾಂಕ nres = getnum;
      ಪೂರ್ಣಾಂಕ i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("checkstack") {
      ಪೂರ್ಣಾಂಕ sz = getnum;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = getstring;
      ಆದರೆ (*ಸಂದೇಶ == '\0')
        ಸಂದೇಶ = ಖಾಲಿ;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, ಸಂದೇಶ);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rawcheckstack") {
      ಪೂರ್ಣಾಂಕ sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("compare") {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt = getstring;  /* EQ, LT, or LE */
      ಪೂರ್ಣಾಂಕ op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      ಪೂರ್ಣಾಂಕ a = getindex;
      ಪೂರ್ಣಾಂಕ b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("concat") {
      lua_concat(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("copy") {
      ಪೂರ್ಣಾಂಕ f = getindex;
      lua_copy(L1, f, getindex);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("getfield") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      ಪೂರ್ಣಾಂಕ tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("getmetatable") {
      ಆದರೆ (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("gettable") {
      ಪೂರ್ಣಾಂಕ tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("gsub") {
      ಪೂರ್ಣಾಂಕ a = getnum; ಪೂರ್ಣಾಂಕ b = getnum; ಪೂರ್ಣಾಂಕ c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("insert") {
      lua_ಸೇರಿಸು(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("len") {
      lua_len(L1, getindex);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("loadstring") {
      size_t slen;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = luaL_checklstring(L1, getnum, &slen);
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = getstring;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ = getstring;
      luaL_loadbufferx(L1, s, slen, name, ಬಹುಲಕ);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("newtable") {
      lua_newtable(L1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("newthread") {
      lua_newthread(L1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("next") {
      lua_ಮುಂದಿನ(L1, -2);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pcall") {
      ಪೂರ್ಣಾಂಕ narg = getnum;
      ಪೂರ್ಣಾಂಕ nres = getnum;
      ಸ್ಥಿತಿ_2 = lua_pcall(L1, narg, nres, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pcallk") {
      ಪೂರ್ಣಾಂಕ narg = getnum;
      ಪೂರ್ಣಾಂಕ nres = getnum;
      ಪೂರ್ಣಾಂಕ i = getindex;
      ಸ್ಥಿತಿ_2 = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pop") {
      lua_ತೆಗೆ(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("printstack") {
      ಪೂರ್ಣಾಂಕ n = getnum;
      ಆದರೆ (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        ಬರೆ("\n");
      }
      ಇಲ್ಲದಿದ್ದರೆ lua_printstack(L1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("print") {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = getstring;
      ಬರೆ("%s\n", ಸಂದೇಶ);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("warningC") {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = getstring;
      lua_warning(L1, ಸಂದೇಶ, 1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("warning") {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = getstring;
      lua_warning(L1, ಸಂದೇಶ, 0);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushnil") {
      lua_pushnil(L1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushnum") {
      lua_pushnumber(L1, (lua_ಸಂಖ್ಯೆ)getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushstatus") {
      lua_pushstring(L1, statcodes[ಸ್ಥಿತಿ_2]);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (ಪೂರ್ಣಾಂಕ)lua_tointeger(L, -1));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rawget") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_rawget(L1, t);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rawgeti") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rawgetp") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rawset") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_rawset(L1, t);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rawseti") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rawsetp") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("remove") {
      lua_remove(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("replace") {
      lua_replace(L1, getindex);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("resume") {
      ಪೂರ್ಣಾಂಕ i = getindex;
      ಪೂರ್ಣಾಂಕ nres;
      ಸ್ಥಿತಿ_2 = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("traceback") {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = getstring;
      ಪೂರ್ಣಾಂಕ level = getnum;
      luaL_traceback(L1, L1, ಸಂದೇಶ, level);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_ಸ್ಥಿತಿ(L1)]);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("return") {
      ಪೂರ್ಣಾಂಕ n = getnum;
      ಆದರೆ (L1 != L) {
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ (i = 0; i < n; i++) {
          ಪೂರ್ಣಾಂಕ idx = -(n - i);
          ಆಯ್ಕೆ (lua_type(L1, idx)) {
            ಸ್ಥಿತಿ LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              ನಿಲ್ಲಿಸು;
            ಪೂರ್ವನಿಯೋಜಿತ:
              lua_pushstring(L, lua_tostring(L1, idx));
              ನಿಲ್ಲಿಸು;
          }
        }
      }
      ಹಿಂತಿರುಗಿ n;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("rotate") {
      ಪೂರ್ಣಾಂಕ i = getindex;
      lua_rotate(L1, i, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("setfield") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getstring;
      lua_setfield(L1, t, s);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("seti") {
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_seti(L1, t, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("setglobal") {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getstring;
      lua_setglobal(L1, s);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("sethook") {
      ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2 = getnum;
      ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = getnum;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getstring;
      sethookaux(L1, ಮುಖವಾಡ_2, ಎಣಿಕೆ_2, s);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("setmetatable") {
      ಪೂರ್ಣಾಂಕ idx = getindex;
      lua_setmetatable(L1, idx);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("settable") {
      lua_settable(L1, getindex);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("settop") {
      lua_settop(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("testudata") {
      ಪೂರ್ಣಾಂಕ i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != ಖಾಲಿ);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("error") {
      lua_error(L1);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("abort") {
      abort();
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("throw") {
#ಆದರೆ defined(__cplusplus)
ಸ್ಥಿರ ರಚನೆ X { ಪೂರ್ಣಾಂಕ x; } x;
      throw x;
#ಇಲ್ಲದಿದ್ದರೆ
      luaL_error(L1, "C++");
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("tostring") {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = lua_tostring(L1, getindex);
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == ಖಾಲಿ && s1 == ಖಾಲಿ) || strcmp(s, s1) == 0);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, ಖಾಲಿ);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("xmove") {
      ಪೂರ್ಣಾಂಕ f = getindex;
      ಪೂರ್ಣಾಂಕ t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      ಪೂರ್ಣಾಂಕ n = getnum;
      ಆದರೆ (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("yield") {
      ಹಿಂತಿರುಗಿ lua_yield(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("yieldk") {
      ಪೂರ್ಣಾಂಕ nres = getnum;
      ಪೂರ್ಣಾಂಕ i = getindex;
      ಹಿಂತಿರುಗಿ lua_yieldk(L1, nres, i, Cfunck);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ EQ("argerror") {
      ಪೂರ್ಣಾಂಕ arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    ಇಲ್ಲದಿದ್ದರೆ luaL_error(L, "unknown instruction %s", buff);
  }
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ testC (lua_State *L) {
  lua_State *L1;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pc;
  ಆದರೆ (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  ಹಿಂತಿರುಗಿ ಸತತ_C(L, L1, pc);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Cfunc (lua_State *L) {
  ಹಿಂತಿರುಗಿ ಸತತ_C(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ Cfunck (lua_State *L, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, lua_KContext ctx) {
  lua_pushstring(L, statcodes[ಸ್ಥಿತಿ_2]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  ಹಿಂತಿರುಗಿ ಸತತ_C(L, L, lua_tostring(L, cast_int(ctx)));
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  ಹಿಂತಿರುಗಿ 1;
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
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *scpt;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_ತೆಗೆ(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  ಸತತ_C(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
ಸ್ಥಿರ ಶೂನ್ಯ sethookaux (lua_State *L, ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *scpt) {
  ಆದರೆ (*scpt == '\0') {  /* no script? */
    lua_sethook(L, ಖಾಲಿ, 0, 0);  /* turn off hooks */
    ಹಿಂತಿರುಗಿ;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  ಆದರೆ (!lua_istable(L, -1)) {  /* no hook table? */
    lua_ತೆಗೆ(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, ಮುಖವಾಡ_2, ಎಣಿಕೆ_2);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sethook (lua_State *L) {
  ಆದರೆ (lua_isnoneornil(L, 1))
    lua_sethook(L, ಖಾಲಿ, 0, 0);  /* turn off hooks */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *scpt = luaL_checkstring(L, 1);
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *smask = luaL_checkstring(L, 2);
    ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = cast_int(luaL_optinteger(L, 3, 0));
    ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2 = 0;
    ಆದರೆ (strchr(smask, 'c')) ಮುಖವಾಡ_2 |= LUA_MASKCALL;
    ಆದರೆ (strchr(smask, 'r')) ಮುಖವಾಡ_2 |= LUA_MASKRET;
    ಆದರೆ (strchr(smask, 'l')) ಮುಖವಾಡ_2 |= LUA_MASKLINE;
    ಆದರೆ (ಎಣಿಕೆ_2 > 0) ಮುಖವಾಡ_2 |= LUA_MASKCOUNT;
    sethookaux(L, ಮುಖವಾಡ_2, ಎಣಿಕೆ_2, scpt);
  }
  ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ coresume (lua_State *L) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  ಸ್ಥಿತಿ_2 = lua_resume(co, L, 0, &nres);
  ಆದರೆ (ಸ್ಥಿತಿ_2 != LUA_OK && ಸ್ಥಿತಿ_2 != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_ಸೇರಿಸು(L, -2);
    ಹಿಂತಿರುಗಿ 2;  /* return false + error message */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_pushboolean(L, 1);
    ಹಿಂತಿರುಗಿ 1;
  }
}

#ಆದರೆ !defined(LUA_USE_POSIX)

#ವ್ಯಾಖ್ಯಾನಿಸು nonblock	ಖಾಲಿ

#ಇಲ್ಲದಿದ್ದರೆ

#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nonblock (lua_State *L) {
  FILE *f = cast(lua_L_ಹರಿವು*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  ಪೂರ್ಣಾಂಕ fd = fileno(f);
  ಪೂರ್ಣಾಂಕ flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/* }====================================================== */



ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ luaL_Reg tests_funcs[] = {
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
  {"querytab", ಕೋಷ್ಟಕ_query},
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
  {ಖಾಲಿ, ಖಾಲಿ}
};


ಸ್ಥಿರ ಶೂನ್ಯ checkfinalmem (ಶೂನ್ಯ) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.ಒಟ್ಟು == 0);
}


ಪೂರ್ಣಾಂಕ luaB_opentests (lua_State *L) {
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
  ಹಿಂತಿರುಗಿ 1;
}

#ಆದರೆ_ಅಂತ್ಯ

