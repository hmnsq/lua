/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lstate_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "llex.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"



#ವ್ಯಾಖ್ಯಾನಿಸು fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#ಆದರೆ !defined(luai_userstateopen)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstateopen(L)		((ಶೂನ್ಯ)L)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(luai_userstateclose)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstateclose(L)		((ಶೂನ್ಯ)L)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(luai_userstatethread)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstatethread(L,L1)	((ಶೂನ್ಯ)L)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(luai_userstatefree)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstatefree(L,L1)	((ಶೂನ್ಯ)L)
#ಆದರೆ_ಅಂತ್ಯ


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
ಶೂನ್ಯ luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  ಆದರೆ (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, ಪೂರ್ಣಾಂಕ err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, ಖಾಲಿ, 0, 1, CallInfo);
  ಆದರೆ (l_unlikely(ci == ಖಾಲಿ)) {  /* allocation failed? */
    ಆದರೆ (err)
      luaM_error(L);  /* raise the error */
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* else only report it */
  }
  ci->ಮುಂದಿನ = L->ci->ಮುಂದಿನ;
  ci->ಹಿಂದಿನ = L->ci;
  L->ci->ಮುಂದಿನ = ci;
  ಆದರೆ (ci->ಮುಂದಿನ)
    ci->ಮುಂದಿನ->ಹಿಂದಿನ = ci;
  ci->u.l.trap = 0;
  L->nci++;
  ಹಿಂತಿರುಗಿ ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
ಸ್ಥಿರ ಶೂನ್ಯ freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *ಮುಂದಿನ = ci->ಮುಂದಿನ;
  ci->ಮುಂದಿನ = ಖಾಲಿ;
  ಆಗ ((ci = ಮುಂದಿನ) != ಖಾಲಿ) {
    ಮುಂದಿನ = ci->ಮುಂದಿನ;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
ಶೂನ್ಯ luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->ಮುಂದಿನ;  /* first free CallInfo */
  CallInfo *ಮುಂದಿನ;
  ಆದರೆ (ci == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ;  /* no extra elements */
  ಆಗ ((ಮುಂದಿನ = ci->ಮುಂದಿನ) != ಖಾಲಿ) {  /* two extra elements? */
    CallInfo *next2 = ಮುಂದಿನ->ಮುಂದಿನ;  /* next's next */
    ci->ಮುಂದಿನ = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, ಮುಂದಿನ);  /* free next */
    ಆದರೆ (next2 == ಖಾಲಿ)
      ನಿಲ್ಲಿಸು;  /* no more elements */
    ಇಲ್ಲದಿದ್ದರೆ {
      next2->ಹಿಂದಿನ = ci;
      ci = next2;  /* continue */
    }
  }
}


/*
** Called when 'getCcalls(L)' larger or equal to LUAI_MAXCCALLS.
** If equal, raises an overflow error. If value is larger than
** LUAI_MAXCCALLS (which means it is handling an overflow) but
** not much larger, does not report an error (to allow overflow
** handling to work).
*/
ಶೂನ್ಯ luaE_checkcstack (lua_State *L) {
  ಆದರೆ (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC ಶೂನ್ಯ luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  ಆದರೆ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


ಸ್ಥಿರ ಶೂನ್ಯ resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->ತಳ_ci;
  ci->func.p = L->ರಾಶಿ.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->ತುದಿ.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = ಖಾಲಿ;
  ci->callstatus = CIST_C;
  L->ಸ್ಥಿತಿ_2 = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


ಸ್ಥಿರ ಶೂನ್ಯ ರಾಶಿ_init (lua_State *L1, lua_State *L) {
  ಪೂರ್ಣಾಂಕ i;
  /* initialize stack array */
  L1->ರಾಶಿ.p = luaM_newvector(L, BASIC_ರಾಶಿ_ಗಾತ್ರ + EXTRA_ರಾಶಿ, ರಾಶಿ_ಮೌಲ್ಯ);
  L1->tbclist.p = L1->ರಾಶಿ.p;
  ಗಾಗಿ (i = 0; i < BASIC_ರಾಶಿ_ಗಾತ್ರ + EXTRA_ರಾಶಿ; i++)
    setnilvalue2s(L1->ರಾಶಿ.p + i);  /* erase new stack */
  L1->ರಾಶಿ_ಕೊನೆಯ.p = L1->ರಾಶಿ.p + BASIC_ರಾಶಿ_ಗಾತ್ರ;
  /* initialize first ci */
  resetCI(L1);
  L1->ತುದಿ.p = L1->ರಾಶಿ.p + 1;  /* +1 for 'function' entry */
}


ಸ್ಥಿರ ಶೂನ್ಯ freestack (lua_State *L) {
  ಆದರೆ (L->ರಾಶಿ.p == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ;  /* stack not completely built yet */
  L->ci = &L->ತಳ_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->ರಾಶಿ.p, cast_sizet(stacksize(L) + EXTRA_ರಾಶಿ));
}


/*
** Create registry table and its predefined values
*/
ಸ್ಥಿರ ಶೂನ್ಯ init_registry (lua_State *L, global_State *g) {
  /* create registry */
  TValue aux;
  ಕೋಷ್ಟಕ *registry = luaH_new(L);
  sethvalue(L, &g->l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_ಕೊನೆಯ, 0);
  /* registry[1] = false */
  setbfvalue(&aux);
  luaH_setint(L, registry, 1, &aux);
  /* registry[LUA_RIDX_MAINTHREAD] = L */
  setthvalue(L, &aux, L);
  luaH_setint(L, registry, LUA_RIDX_MAINTHREAD, &aux);
  /* registry[LUA_RIDX_GLOBALS] = new table (table of globals) */
  sethvalue(L, &aux, luaH_new(L));
  luaH_setint(L, registry, LUA_RIDX_GLOBALS, &aux);
}


/*
** open parts of the state that may cause memory-allocation errors.
*/
ಸ್ಥಿರ ಶೂನ್ಯ f_luaopen (lua_State *L, ಶೂನ್ಯ *ud) {
  global_State *g = G(L);
  ಬಳಸದ(ud);
  ರಾಶಿ_init(L, L);  /* init stack */
  init_registry(L, g);
  luaS_init(L);
  luaT_init(L);
  luaX_init(L);
  g->gcstp = 0;  /* allow gc */
  setnilvalue(&g->nilvalue);  /* now state is complete */
  luai_userstateopen(L);
}


/*
** preinitialize a thread with consistent values without allocating
** any memory (to avoid errors)
*/
ಸ್ಥಿರ ಶೂನ್ಯ preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->ರಾಶಿ.p = ಖಾಲಿ;
  L->ci = ಖಾಲಿ;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = ಖಾಲಿ;
  L->hook = ಖಾಲಿ;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = ಖಾಲಿ;
  L->ಸ್ಥಿತಿ_2 = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->ತಳ_ci.ಹಿಂದಿನ = L->ತಳ_ci.ಮುಂದಿನ = ಖಾಲಿ;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, ಗಾತ್ರ(LX))
            + cast_uint(L->nci) * ಗಾತ್ರ(CallInfo);
  ಆದರೆ (L->ರಾಶಿ.p != ಖಾಲಿ)
    sz += cast_uint(stacksize(L) + EXTRA_ರಾಶಿ) * ಗಾತ್ರ(ರಾಶಿ_ಮೌಲ್ಯ);
  ಹಿಂತಿರುಗಿ sz;
}


ಸ್ಥಿರ ಶೂನ್ಯ close_state (lua_State *L) {
  global_State *g = G(L);
  ಆದರೆ (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  ಇಲ್ಲದಿದ್ದರೆ {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->ತುದಿ.p = L->ರಾಶಿ.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.ಗಾತ್ರ_2));
  freestack(L);
  lua_assert(gettotalbytes(g) == ಗಾತ್ರ(global_State));
  (*g->frealloc)(g->ud, g, ಗಾತ್ರ(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_ಬೀಗ(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, ಗಾತ್ರ(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->ತುದಿ.p, L1);
  api_incr_ತುದಿ(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  ಸ್ಮರಣೆ_ನಕಲು(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  ರಾಶಿ_init(L1, L);  /* init stack */
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ L1;
}


ಶೂನ್ಯ luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->ರಾಶಿ.p);  /* close all upvalues */
  lua_assert(L1->openupval == ಖಾಲಿ);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus ಸ್ಥಿತಿ_2) {
  resetCI(L);
  ಆದರೆ (ಸ್ಥಿತಿ_2 == LUA_YIELD)
    ಸ್ಥಿತಿ_2 = LUA_OK;
  ಸ್ಥಿತಿ_2 = luaD_closeprotected(L, 1, ಸ್ಥಿತಿ_2);
  ಆದರೆ (ಸ್ಥಿತಿ_2 != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, ಸ್ಥಿತಿ_2, L->ರಾಶಿ.p + 1);
  ಇಲ್ಲದಿದ್ದರೆ  /* keep in the stack only the fake function-entry for basic ci */
    L->ತುದಿ.p = L->ರಾಶಿ.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->ತುದಿ.p - L->ರಾಶಿ.p), 0);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
ಸ್ಥಿರ ಶೂನ್ಯ resetthread_aux (lua_State *L, ಶೂನ್ಯ *ud) {
  TStatus ಸ್ಥಿತಿ_2 = *cast(TStatus*, ud);
  ಸ್ಥಿತಿ_2 = luaE_resetthread(L, ಸ್ಥಿತಿ_2);
  ಆದರೆ (ಸ್ಥಿತಿ_2 != LUA_OK)
    luaD_throw(L, ಸ್ಥಿತಿ_2);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_closethread (lua_State *L, lua_State *from) {
  TStatus ಸ್ಥಿತಿ_2;
  lua_ಬೀಗ(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  ಸ್ಥಿತಿ_2 = luaD_rawrunprotected(L, resetthread_aux, &L->ಸ್ಥಿತಿ_2);
  /* stack has error message iff there was an error */
  lua_assert(L->ತುದಿ.p == L->ರಾಶಿ.p + (ಸ್ಥಿತಿ_2 == LUA_OK ? 1 : 2));
  ಆದರೆ (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, ಸ್ಥಿತಿ_2);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ APIstatus(ಸ್ಥಿತಿ_2);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, ಶೂನ್ಯ *ud, ಚಿಹ್ನೆರಹಿತ ಬೀಜ) {
  ಪೂರ್ಣಾಂಕ i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, ಖಾಲಿ, LUA_TTHREAD, ಗಾತ್ರ(global_State)));
  ಆದರೆ (g == ಖಾಲಿ) ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0_ಬಿಟ್);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->ಮುಂದಿನ = ಖಾಲಿ;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = ಖಾಲಿ;
  g->ud_warn = ಖಾಲಿ;
  g->ಬೀಜ = ಬೀಜ;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.ಗಾತ್ರ_2 = g->strt.nuse = 0;
  g->strt.hash = ಖಾಲಿ;
  setnilvalue(&g->l_registry);
  g->panic = ಖಾಲಿ;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = ಖಾಲಿ;
  g->firstold1 = g->survival = g->old1 = g->reallyold = ಖಾಲಿ;
  g->finobjsur = g->finobjold1 = g->finobjrold = ಖಾಲಿ;
  g->sweepgc = ಖಾಲಿ;
  g->ಬೂದು = g->grayagain = ಖಾಲಿ;
  g->weak = g->ephemeron = g->allweak = ಖಾಲಿ;
  g->twups = ಖಾಲಿ;
  g->GCtotalbytes = ಗಾತ್ರ(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  ಗಾಗಿ (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = ಖಾಲಿ;
  ಆದರೆ (luaD_rawrunprotected(L, f_luaopen, ಖಾಲಿ) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = ಖಾಲಿ;
  }
  ಹಿಂತಿರುಗಿ L;
}


LUA_API ಶೂನ್ಯ lua_close (lua_State *L) {
  lua_ಬೀಗ(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


ಶೂನ್ಯ luaE_warning (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, ಪೂರ್ಣಾಂಕ tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  ಆದರೆ (wf != ಖಾಲಿ)
    wf(G(L)->ud_warn, ಸಂದೇಶ, tocont);
}


/*
** Generate a warning from an error message
*/
ಶೂನ್ಯ luaE_warnerror (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *where) {
  TValue *errobj = s2v(L->ತುದಿ.p - 1);  /* error object */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, ಸಂದೇಶ, 1);
  luaE_warning(L, ")", 0);
}

