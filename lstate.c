/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lstate_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#ਜੇ !defined(luai_userstateopen)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstateopen(L)		((ਖਾਲੀ)L)
#ਜੇ_ਅੰਤ

#ਜੇ !defined(luai_userstateclose)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstateclose(L)		((ਖਾਲੀ)L)
#ਜੇ_ਅੰਤ

#ਜੇ !defined(luai_userstatethread)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstatethread(L,L1)	((ਖਾਲੀ)L)
#ਜੇ_ਅੰਤ

#ਜੇ !defined(luai_userstatefree)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstatefree(L,L1)	((ਖਾਲੀ)L)
#ਜੇ_ਅੰਤ


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
ਖਾਲੀ luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  ਜੇ (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, ਪੂਰਨ err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, ਨਿਰਾ, 0, 1, CallInfo);
  ਜੇ (l_unlikely(ci == ਨਿਰਾ)) {  /* allocation failed? */
    ਜੇ (err)
      luaM_error(L);  /* raise the error */
    ਵਾਪਸ ਨਿਰਾ;  /* else only report it */
  }
  ci->ਅਗਲਾ = L->ci->ਅਗਲਾ;
  ci->ਪਿਛਲਾ = L->ci;
  L->ci->ਅਗਲਾ = ci;
  ਜੇ (ci->ਅਗਲਾ)
    ci->ਅਗਲਾ->ਪਿਛਲਾ = ci;
  ci->u.l.trap = 0;
  L->nci++;
  ਵਾਪਸ ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
ਸਥਿਰ ਖਾਲੀ freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *ਅਗਲਾ = ci->ਅਗਲਾ;
  ci->ਅਗਲਾ = ਨਿਰਾ;
  ਜਦੋਂ ((ci = ਅਗਲਾ) != ਨਿਰਾ) {
    ਅਗਲਾ = ci->ਅਗਲਾ;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
ਖਾਲੀ luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->ਅਗਲਾ;  /* first free CallInfo */
  CallInfo *ਅਗਲਾ;
  ਜੇ (ci == ਨਿਰਾ)
    ਵਾਪਸ;  /* no extra elements */
  ਜਦੋਂ ((ਅਗਲਾ = ci->ਅਗਲਾ) != ਨਿਰਾ) {  /* two extra elements? */
    CallInfo *next2 = ਅਗਲਾ->ਅਗਲਾ;  /* next's next */
    ci->ਅਗਲਾ = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, ਅਗਲਾ);  /* free next */
    ਜੇ (next2 == ਨਿਰਾ)
      ਰੋਕੋ;  /* no more elements */
    ਨਹੀਂਤਾਂ {
      next2->ਪਿਛਲਾ = ci;
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
ਖਾਲੀ luaE_checkcstack (lua_State *L) {
  ਜੇ (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  ਨਹੀਂਤਾਂ ਜੇ (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC ਖਾਲੀ luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  ਜੇ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


ਸਥਿਰ ਖਾਲੀ resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->ਅਧਾਰ_ci;
  ci->func.p = L->ਢੇਰ.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->ਸਿਖਰ.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = ਨਿਰਾ;
  ci->callstatus = CIST_C;
  L->ਸਥਿਤੀ = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


ਸਥਿਰ ਖਾਲੀ ਢੇਰ_init (lua_State *L1, lua_State *L) {
  ਪੂਰਨ i;
  /* initialize stack array */
  L1->ਢੇਰ.p = luaM_newvector(L, BASIC_ਢੇਰ_ਮਾਪ + EXTRA_ਢੇਰ, ਢੇਰ_ਮੁੱਲ);
  L1->tbclist.p = L1->ਢੇਰ.p;
  ਲਈ (i = 0; i < BASIC_ਢੇਰ_ਮਾਪ + EXTRA_ਢੇਰ; i++)
    setnilvalue2s(L1->ਢੇਰ.p + i);  /* erase new stack */
  L1->ਢੇਰ_ਆਖਰੀ.p = L1->ਢੇਰ.p + BASIC_ਢੇਰ_ਮਾਪ;
  /* initialize first ci */
  resetCI(L1);
  L1->ਸਿਖਰ.p = L1->ਢੇਰ.p + 1;  /* +1 for 'function' entry */
}


ਸਥਿਰ ਖਾਲੀ freestack (lua_State *L) {
  ਜੇ (L->ਢੇਰ.p == ਨਿਰਾ)
    ਵਾਪਸ;  /* stack not completely built yet */
  L->ci = &L->ਅਧਾਰ_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->ਢੇਰ.p, cast_sizet(stacksize(L) + EXTRA_ਢੇਰ));
}


/*
** Create registry table and its predefined values
*/
ਸਥਿਰ ਖਾਲੀ init_registry (lua_State *L, global_State *g) {
  /* create registry */
  TValue aux;
  ਸਾਰਣੀ *registry = luaH_new(L);
  sethvalue(L, &g->l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_ਆਖਰੀ, 0);
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
ਸਥਿਰ ਖਾਲੀ f_luaopen (lua_State *L, ਖਾਲੀ *ud) {
  global_State *g = G(L);
  ਅਣਵਰਤਿਆ(ud);
  ਢੇਰ_init(L, L);  /* init stack */
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
ਸਥਿਰ ਖਾਲੀ preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->ਢੇਰ.p = ਨਿਰਾ;
  L->ci = ਨਿਰਾ;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = ਨਿਰਾ;
  L->hook = ਨਿਰਾ;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = ਨਿਰਾ;
  L->ਸਥਿਤੀ = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->ਅਧਾਰ_ci.ਪਿਛਲਾ = L->ਅਧਾਰ_ci.ਅਗਲਾ = ਨਿਰਾ;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, ਆਕਾਰ(LX))
            + cast_uint(L->nci) * ਆਕਾਰ(CallInfo);
  ਜੇ (L->ਢੇਰ.p != ਨਿਰਾ)
    sz += cast_uint(stacksize(L) + EXTRA_ਢੇਰ) * ਆਕਾਰ(ਢੇਰ_ਮੁੱਲ);
  ਵਾਪਸ sz;
}


ਸਥਿਰ ਖਾਲੀ close_state (lua_State *L) {
  global_State *g = G(L);
  ਜੇ (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  ਨਹੀਂਤਾਂ {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->ਸਿਖਰ.p = L->ਢੇਰ.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.ਮਾਪ));
  freestack(L);
  lua_assert(gettotalbytes(g) == ਆਕਾਰ(global_State));
  (*g->frealloc)(g->ud, g, ਆਕਾਰ(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_ਤਾਲਾ(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, ਆਕਾਰ(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->ਸਿਖਰ.p, L1);
  api_incr_ਸਿਖਰ(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  ਢੇਰ_init(L1, L);  /* init stack */
  lua_unlock(L);
  ਵਾਪਸ L1;
}


ਖਾਲੀ luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->ਢੇਰ.p);  /* close all upvalues */
  lua_assert(L1->openupval == ਨਿਰਾ);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus ਸਥਿਤੀ) {
  resetCI(L);
  ਜੇ (ਸਥਿਤੀ == LUA_YIELD)
    ਸਥਿਤੀ = LUA_OK;
  ਸਥਿਤੀ = luaD_closeprotected(L, 1, ਸਥਿਤੀ);
  ਜੇ (ਸਥਿਤੀ != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, ਸਥਿਤੀ, L->ਢੇਰ.p + 1);
  ਨਹੀਂਤਾਂ  /* keep in the stack only the fake function-entry for basic ci */
    L->ਸਿਖਰ.p = L->ਢੇਰ.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->ਸਿਖਰ.p - L->ਢੇਰ.p), 0);
  ਵਾਪਸ ਸਥਿਤੀ;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
ਸਥਿਰ ਖਾਲੀ resetthread_aux (lua_State *L, ਖਾਲੀ *ud) {
  TStatus ਸਥਿਤੀ = *cast(TStatus*, ud);
  ਸਥਿਤੀ = luaE_resetthread(L, ਸਥਿਤੀ);
  ਜੇ (ਸਥਿਤੀ != LUA_OK)
    luaD_throw(L, ਸਥਿਤੀ);
}


LUA_API ਪੂਰਨ lua_closethread (lua_State *L, lua_State *from) {
  TStatus ਸਥਿਤੀ;
  lua_ਤਾਲਾ(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  ਸਥਿਤੀ = luaD_rawrunprotected(L, resetthread_aux, &L->ਸਥਿਤੀ);
  /* stack has error message iff there was an error */
  lua_assert(L->ਸਿਖਰ.p == L->ਢੇਰ.p + (ਸਥਿਤੀ == LUA_OK ? 1 : 2));
  ਜੇ (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, ਸਥਿਤੀ);
  lua_unlock(L);
  ਵਾਪਸ APIstatus(ਸਥਿਤੀ);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, ਖਾਲੀ *ud, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬੀਜ) {
  ਪੂਰਨ i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, ਨਿਰਾ, LUA_TTHREAD, ਆਕਾਰ(global_State)));
  ਜੇ (g == ਨਿਰਾ) ਵਾਪਸ ਨਿਰਾ;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0_ਦੋਆਧਾਰੀ_ਅੰਕ);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->ਅਗਲਾ = ਨਿਰਾ;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = ਨਿਰਾ;
  g->ud_warn = ਨਿਰਾ;
  g->ਬੀਜ = ਬੀਜ;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.ਮਾਪ = g->strt.nuse = 0;
  g->strt.hash = ਨਿਰਾ;
  setnilvalue(&g->l_registry);
  g->panic = ਨਿਰਾ;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = ਨਿਰਾ;
  g->firstold1 = g->survival = g->old1 = g->reallyold = ਨਿਰਾ;
  g->finobjsur = g->finobjold1 = g->finobjrold = ਨਿਰਾ;
  g->sweepgc = ਨਿਰਾ;
  g->ਸਲੇਟੀ = g->grayagain = ਨਿਰਾ;
  g->weak = g->ephemeron = g->allweak = ਨਿਰਾ;
  g->twups = ਨਿਰਾ;
  g->GCtotalbytes = ਆਕਾਰ(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  ਲਈ (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = ਨਿਰਾ;
  ਜੇ (luaD_rawrunprotected(L, f_luaopen, ਨਿਰਾ) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = ਨਿਰਾ;
  }
  ਵਾਪਸ L;
}


LUA_API ਖਾਲੀ lua_close (lua_State *L) {
  lua_ਤਾਲਾ(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


ਖਾਲੀ luaE_warning (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, ਪੂਰਨ tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  ਜੇ (wf != ਨਿਰਾ)
    wf(G(L)->ud_warn, ਸੁਨੇਹਾ, tocont);
}


/*
** Generate a warning from an error message
*/
ਖਾਲੀ luaE_warnerror (lua_State *L, ਅਚਲ ਅੱਖਰ *where) {
  TValue *errobj = s2v(L->ਸਿਖਰ.p - 1);  /* error object */
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, ਸੁਨੇਹਾ, 1);
  luaE_warning(L, ")", 0);
}

