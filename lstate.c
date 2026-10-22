/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lapi.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "llex.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#যদি !defined(luai_userstateopen)
#define luai_userstateopen(L)		((শূন্য)L)
#endif

#যদি !defined(luai_userstateclose)
#define luai_userstateclose(L)		((শূন্য)L)
#endif

#যদি !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((শূন্য)L)
#endif

#যদি !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((শূন্য)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
শূন্য luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  যদি (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, পূর্ণসংখ্যা err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, ফাঁকা, 0, 1, CallInfo);
  যদি (l_unlikely(ci == ফাঁকা)) {  /* allocation failed? */
    যদি (err)
      luaM_error(L);  /* raise the error */
    ফেরত ফাঁকা;  /* else only report it */
  }
  ci->পরবর্তী = L->ci->পরবর্তী;
  ci->previous = L->ci;
  L->ci->পরবর্তী = ci;
  যদি (ci->পরবর্তী)
    ci->পরবর্তী->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  ফেরত ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
স্থির শূন্য freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *পরবর্তী = ci->পরবর্তী;
  ci->পরবর্তী = ফাঁকা;
  যখন ((ci = পরবর্তী) != ফাঁকা) {
    পরবর্তী = ci->পরবর্তী;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
শূন্য luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->পরবর্তী;  /* first free CallInfo */
  CallInfo *পরবর্তী;
  যদি (ci == ফাঁকা)
    ফেরত;  /* no extra elements */
  যখন ((পরবর্তী = ci->পরবর্তী) != ফাঁকা) {  /* two extra elements? */
    CallInfo *next2 = পরবর্তী->পরবর্তী;  /* next's next */
    ci->পরবর্তী = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, পরবর্তী);  /* free next */
    যদি (next2 == ফাঁকা)
      বিরতি;  /* no more elements */
    অন্যথা {
      next2->previous = ci;
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
শূন্য luaE_checkcstack (lua_State *L) {
  যদি (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  অন্যথা যদি (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC শূন্য luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  যদি (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


স্থির শূন্য resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->স্তূপ.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->শীর্ষ.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = ফাঁকা;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


স্থির শূন্য স্তূপ_init (lua_State *L1, lua_State *L) {
  পূর্ণসংখ্যা i;
  /* initialize stack array */
  L1->স্তূপ.p = luaM_newvector(L, BASIC_স্তূপ_আকার + EXTRA_স্তূপ, স্তূপ_মান);
  L1->tbclist.p = L1->স্তূপ.p;
  জন্য (i = 0; i < BASIC_স্তূপ_আকার + EXTRA_স্তূপ; i++)
    setnilvalue2s(L1->স্তূপ.p + i);  /* erase new stack */
  L1->স্তূপ_last.p = L1->স্তূপ.p + BASIC_স্তূপ_আকার;
  /* initialize first ci */
  resetCI(L1);
  L1->শীর্ষ.p = L1->স্তূপ.p + 1;  /* +1 for 'function' entry */
}


স্থির শূন্য freestack (lua_State *L) {
  যদি (L->স্তূপ.p == ফাঁকা)
    ফেরত;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->স্তূপ.p, cast_sizet(stacksize(L) + EXTRA_স্তূপ));
}


/*
** Create registry table and its predefined values
*/
স্থির শূন্য init_registry (lua_State *L, global_State *g) {
  /* create registry */
  TValue aux;
  Table *registry = luaH_new(L);
  sethvalue(L, &g->l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_LAST, 0);
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
স্থির শূন্য f_luaopen (lua_State *L, শূন্য *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  স্তূপ_init(L, L);  /* init stack */
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
স্থির শূন্য preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->স্তূপ.p = ফাঁকা;
  L->ci = ফাঁকা;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = ফাঁকা;
  L->hook = ফাঁকা;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = ফাঁকা;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.পরবর্তী = ফাঁকা;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, আকার(LX))
            + cast_uint(L->nci) * আকার(CallInfo);
  যদি (L->স্তূপ.p != ফাঁকা)
    sz += cast_uint(stacksize(L) + EXTRA_স্তূপ) * আকার(স্তূপ_মান);
  ফেরত sz;
}


স্থির শূন্য close_state (lua_State *L) {
  global_State *g = G(L);
  যদি (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  অন্যথা {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->শীর্ষ.p = L->স্তূপ.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.আকার_2));
  freestack(L);
  lua_assert(gettotalbytes(g) == আকার(global_State));
  (*g->frealloc)(g->ud, g, আকার(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, আকার(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->শীর্ষ.p, L1);
  api_incr_শীর্ষ(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  স্তূপ_init(L1, L);  /* init stack */
  lua_unlock(L);
  ফেরত L1;
}


শূন্য luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->স্তূপ.p);  /* close all upvalues */
  lua_assert(L1->openupval == ফাঁকা);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  যদি (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  যদি (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->স্তূপ.p + 1);
  অন্যথা  /* keep in the stack only the fake function-entry for basic ci */
    L->শীর্ষ.p = L->স্তূপ.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->শীর্ষ.p - L->স্তূপ.p), 0);
  ফেরত status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
স্থির শূন্য resetthread_aux (lua_State *L, শূন্য *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  যদি (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API পূর্ণসংখ্যা lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->শীর্ষ.p == L->স্তূপ.p + (status == LUA_OK ? 1 : 2));
  যদি (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  ফেরত APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, শূন্য *ud, unsigned seed) {
  পূর্ণসংখ্যা i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, ফাঁকা, LUA_TTHREAD, আকার(global_State)));
  যদি (g == ফাঁকা) ফেরত ফাঁকা;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->পরবর্তী = ফাঁকা;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = ফাঁকা;
  g->ud_warn = ফাঁকা;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.আকার_2 = g->strt.nuse = 0;
  g->strt.hash = ফাঁকা;
  setnilvalue(&g->l_registry);
  g->panic = ফাঁকা;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = ফাঁকা;
  g->firstold1 = g->survival = g->old1 = g->reallyold = ফাঁকা;
  g->finobjsur = g->finobjold1 = g->finobjrold = ফাঁকা;
  g->sweepgc = ফাঁকা;
  g->gray = g->grayagain = ফাঁকা;
  g->weak = g->ephemeron = g->allweak = ফাঁকা;
  g->twups = ফাঁকা;
  g->GCtotalbytes = আকার(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  জন্য (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = ফাঁকা;
  যদি (luaD_rawrunprotected(L, f_luaopen, ফাঁকা) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = ফাঁকা;
  }
  ফেরত L;
}


LUA_API শূন্য lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


শূন্য luaE_warning (lua_State *L, ধ্রুবক অক্ষর *বার্তা, পূর্ণসংখ্যা tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  যদি (wf != ফাঁকা)
    wf(G(L)->ud_warn, বার্তা, tocont);
}


/*
** Generate a warning from an error message
*/
শূন্য luaE_warnerror (lua_State *L, ধ্রুবক অক্ষর *where) {
  TValue *errobj = s2v(L->শীর্ষ.p - 1);  /* error object */
  ধ্রুবক অক্ষর *বার্তা = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, বার্তা, 1);
  luaE_warning(L, ")", 0);
}

