/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#నిర్వచించు lstate_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stddef.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "llex.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "ltm.h"



#నిర్వచించు fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#ఒకవేళ !defined(luai_userstateopen)
#నిర్వచించు luai_userstateopen(L)		((శూన్య)L)
#అయితే_ముగింపు

#ఒకవేళ !defined(luai_userstateclose)
#నిర్వచించు luai_userstateclose(L)		((శూన్య)L)
#అయితే_ముగింపు

#ఒకవేళ !defined(luai_userstatethread)
#నిర్వచించు luai_userstatethread(L,L1)	((శూన్య)L)
#అయితే_ముగింపు

#ఒకవేళ !defined(luai_userstatefree)
#నిర్వచించు luai_userstatefree(L,L1)	((శూన్య)L)
#అయితే_ముగింపు


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
శూన్య luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  ఒకవేళ (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, పూర్ణాంకం err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, ఖాళీ, 0, 1, CallInfo);
  ఒకవేళ (l_unlikely(ci == ఖాళీ)) {  /* allocation failed? */
    ఒకవేళ (err)
      luaM_error(L);  /* raise the error */
    తిరిగి ఖాళీ;  /* else only report it */
  }
  ci->తదుపరి = L->ci->తదుపరి;
  ci->మునుపటి = L->ci;
  L->ci->తదుపరి = ci;
  ఒకవేళ (ci->తదుపరి)
    ci->తదుపరి->మునుపటి = ci;
  ci->u.l.trap = 0;
  L->nci++;
  తిరిగి ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
స్థిర శూన్య freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *తదుపరి = ci->తదుపరి;
  ci->తదుపరి = ఖాళీ;
  అయితే ((ci = తదుపరి) != ఖాళీ) {
    తదుపరి = ci->తదుపరి;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
శూన్య luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->తదుపరి;  /* first free CallInfo */
  CallInfo *తదుపరి;
  ఒకవేళ (ci == ఖాళీ)
    తిరిగి;  /* no extra elements */
  అయితే ((తదుపరి = ci->తదుపరి) != ఖాళీ) {  /* two extra elements? */
    CallInfo *next2 = తదుపరి->తదుపరి;  /* next's next */
    ci->తదుపరి = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, తదుపరి);  /* free next */
    ఒకవేళ (next2 == ఖాళీ)
      ఆపు;  /* no more elements */
    లేకపోతే {
      next2->మునుపటి = ci;
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
శూన్య luaE_checkcstack (lua_State *L) {
  ఒకవేళ (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  లేకపోతే ఒకవేళ (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC శూన్య luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  ఒకవేళ (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


స్థిర శూన్య resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->పునాది_ci;
  ci->func.p = L->దొంతర.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->శిఖరం.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = ఖాళీ;
  ci->callstatus = CIST_C;
  L->స్థితి_2 = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


స్థిర శూన్య దొంతర_init (lua_State *L1, lua_State *L) {
  పూర్ణాంకం i;
  /* initialize stack array */
  L1->దొంతర.p = luaM_newvector(L, BASIC_దొంతర_కొలత + EXTRA_దొంతర, దొంతర_విలువ);
  L1->tbclist.p = L1->దొంతర.p;
  కొరకు (i = 0; i < BASIC_దొంతర_కొలత + EXTRA_దొంతర; i++)
    setnilvalue2s(L1->దొంతర.p + i);  /* erase new stack */
  L1->దొంతర_చివరి.p = L1->దొంతర.p + BASIC_దొంతర_కొలత;
  /* initialize first ci */
  resetCI(L1);
  L1->శిఖరం.p = L1->దొంతర.p + 1;  /* +1 for 'function' entry */
}


స్థిర శూన్య freestack (lua_State *L) {
  ఒకవేళ (L->దొంతర.p == ఖాళీ)
    తిరిగి;  /* stack not completely built yet */
  L->ci = &L->పునాది_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->దొంతర.p, cast_sizet(stacksize(L) + EXTRA_దొంతర));
}


/*
** Create registry table and its predefined values
*/
స్థిర శూన్య init_registry (lua_State *L, global_State *g) {
  /* create registry */
  TValue aux;
  పట్టిక *registry = luaH_new(L);
  sethvalue(L, &g->l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_చివరి, 0);
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
స్థిర శూన్య f_luaopen (lua_State *L, శూన్య *ud) {
  global_State *g = G(L);
  వాడని(ud);
  దొంతర_init(L, L);  /* init stack */
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
స్థిర శూన్య preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->దొంతర.p = ఖాళీ;
  L->ci = ఖాళీ;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = ఖాళీ;
  L->hook = ఖాళీ;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = ఖాళీ;
  L->స్థితి_2 = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->పునాది_ci.మునుపటి = L->పునాది_ci.తదుపరి = ఖాళీ;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, పరిమాణం(LX))
            + cast_uint(L->nci) * పరిమాణం(CallInfo);
  ఒకవేళ (L->దొంతర.p != ఖాళీ)
    sz += cast_uint(stacksize(L) + EXTRA_దొంతర) * పరిమాణం(దొంతర_విలువ);
  తిరిగి sz;
}


స్థిర శూన్య close_state (lua_State *L) {
  global_State *g = G(L);
  ఒకవేళ (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  లేకపోతే {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->శిఖరం.p = L->దొంతర.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.కొలత));
  freestack(L);
  lua_assert(gettotalbytes(g) == పరిమాణం(global_State));
  (*g->frealloc)(g->ud, g, పరిమాణం(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_తాళం(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, పరిమాణం(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->శిఖరం.p, L1);
  api_incr_శిఖరం(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  జ్ఞాపకం_నకలు(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  దొంతర_init(L1, L);  /* init stack */
  lua_unlock(L);
  తిరిగి L1;
}


శూన్య luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->దొంతర.p);  /* close all upvalues */
  lua_assert(L1->openupval == ఖాళీ);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus స్థితి_2) {
  resetCI(L);
  ఒకవేళ (స్థితి_2 == LUA_YIELD)
    స్థితి_2 = LUA_OK;
  స్థితి_2 = luaD_closeprotected(L, 1, స్థితి_2);
  ఒకవేళ (స్థితి_2 != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, స్థితి_2, L->దొంతర.p + 1);
  లేకపోతే  /* keep in the stack only the fake function-entry for basic ci */
    L->శిఖరం.p = L->దొంతర.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->శిఖరం.p - L->దొంతర.p), 0);
  తిరిగి స్థితి_2;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
స్థిర శూన్య resetthread_aux (lua_State *L, శూన్య *ud) {
  TStatus స్థితి_2 = *cast(TStatus*, ud);
  స్థితి_2 = luaE_resetthread(L, స్థితి_2);
  ఒకవేళ (స్థితి_2 != LUA_OK)
    luaD_throw(L, స్థితి_2);
}


LUA_API పూర్ణాంకం lua_closethread (lua_State *L, lua_State *from) {
  TStatus స్థితి_2;
  lua_తాళం(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  స్థితి_2 = luaD_rawrunprotected(L, resetthread_aux, &L->స్థితి_2);
  /* stack has error message iff there was an error */
  lua_assert(L->శిఖరం.p == L->దొంతర.p + (స్థితి_2 == LUA_OK ? 1 : 2));
  ఒకవేళ (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, స్థితి_2);
  lua_unlock(L);
  తిరిగి APIstatus(స్థితి_2);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, శూన్య *ud, గుర్తులేని విత్తనం) {
  పూర్ణాంకం i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, ఖాళీ, LUA_TTHREAD, పరిమాణం(global_State)));
  ఒకవేళ (g == ఖాళీ) తిరిగి ఖాళీ;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0_బిట్టు);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->తదుపరి = ఖాళీ;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = ఖాళీ;
  g->ud_warn = ఖాళీ;
  g->విత్తనం = విత్తనం;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.కొలత = g->strt.nuse = 0;
  g->strt.hash = ఖాళీ;
  setnilvalue(&g->l_registry);
  g->panic = ఖాళీ;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = ఖాళీ;
  g->firstold1 = g->survival = g->old1 = g->reallyold = ఖాళీ;
  g->finobjsur = g->finobjold1 = g->finobjrold = ఖాళీ;
  g->sweepgc = ఖాళీ;
  g->బూడిద = g->grayagain = ఖాళీ;
  g->weak = g->ephemeron = g->allweak = ఖాళీ;
  g->twups = ఖాళీ;
  g->GCtotalbytes = పరిమాణం(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  కొరకు (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = ఖాళీ;
  ఒకవేళ (luaD_rawrunprotected(L, f_luaopen, ఖాళీ) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = ఖాళీ;
  }
  తిరిగి L;
}


LUA_API శూన్య lua_close (lua_State *L) {
  lua_తాళం(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


శూన్య luaE_warning (lua_State *L, స్థిరరాశి అక్షరం *సందేశం, పూర్ణాంకం tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  ఒకవేళ (wf != ఖాళీ)
    wf(G(L)->ud_warn, సందేశం, tocont);
}


/*
** Generate a warning from an error message
*/
శూన్య luaE_warnerror (lua_State *L, స్థిరరాశి అక్షరం *where) {
  TValue *errobj = s2v(L->శిఖరం.p - 1);  /* error object */
  స్థిరరాశి అక్షరం *సందేశం = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, సందేశం, 1);
  luaE_warning(L, ")", 0);
}

