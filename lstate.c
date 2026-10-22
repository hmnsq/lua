/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lapi.h"
#অন্তর্ভাৱ "ldebug.h"
#অন্তর্ভাৱ "ldo.h"
#অন্তর্ভাৱ "lfunc.h"
#অন্তর্ভাৱ "lgc.h"
#অন্তর্ভাৱ "llex.h"
#অন্তর্ভাৱ "lmem.h"
#অন্তর্ভাৱ "lstate.h"
#অন্তর্ভাৱ "lstring.h"
#অন্তর্ভাৱ "ltable.h"
#অন্তর্ভাৱ "ltm.h"



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


CallInfo *luaE_extendCI (lua_State *L, পূর্ণাঙ্ক err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, রিক্ত, 0, 1, CallInfo);
  যদি (l_unlikely(ci == রিক্ত)) {  /* allocation failed? */
    যদি (err)
      luaM_error(L);  /* raise the error */
    ঘূৰাই রিক্ত;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  যদি (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  ঘূৰাই ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
স্থির শূন্য freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = রিক্ত;
  যেতিয়ালৈ ((ci = next) != রিক্ত) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
শূন্য luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  যদি (ci == রিক্ত)
    ঘূৰাই;  /* no extra elements */
  যেতিয়ালৈ ((next = ci->next) != রিক্ত) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    যদি (next2 == রিক্ত)
      ভঙ্গ;  /* no more elements */
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
  ci->শীৰ্ষ.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = রিক্ত;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


স্থির শূন্য স্তূপ_init (lua_State *L1, lua_State *L) {
  পূর্ণাঙ্ক i;
  /* initialize stack array */
  L1->স্তূপ.p = luaM_newvector(L, BASIC_স্তূপ_SIZE + EXTRA_স্তূপ, স্তূপ_মান);
  L1->tbclist.p = L1->স্তূপ.p;
  বাবে (i = 0; i < BASIC_স্তূপ_SIZE + EXTRA_স্তূপ; i++)
    setnilvalue2s(L1->স্তূপ.p + i);  /* erase new stack */
  L1->স্তূপ_last.p = L1->স্তূপ.p + BASIC_স্তূপ_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->শীৰ্ষ.p = L1->স্তূপ.p + 1;  /* +1 for 'function' entry */
}


স্থির শূন্য freestack (lua_State *L) {
  যদি (L->স্তূপ.p == রিক্ত)
    ঘূৰাই;  /* stack not completely built yet */
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
  L->স্তূপ.p = রিক্ত;
  L->ci = রিক্ত;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = রিক্ত;
  L->hook = রিক্ত;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = রিক্ত;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = রিক্ত;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, পরিমাণ(LX))
            + cast_uint(L->nci) * পরিমাণ(CallInfo);
  যদি (L->স্তূপ.p != রিক্ত)
    sz += cast_uint(stacksize(L) + EXTRA_স্তূপ) * পরিমাণ(স্তূপ_মান);
  ঘূৰাই sz;
}


স্থির শূন্য close_state (lua_State *L) {
  global_State *g = G(L);
  যদি (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  অন্যথা {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->শীৰ্ষ.p = L->স্তূপ.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == পরিমাণ(global_State));
  (*g->frealloc)(g->ud, g, পরিমাণ(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, পরিমাণ(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->শীৰ্ষ.p, L1);
  api_incr_শীৰ্ষ(L);
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
  ঘূৰাই L1;
}


শূন্য luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->স্তূপ.p);  /* close all upvalues */
  lua_assert(L1->openupval == রিক্ত);
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
    L->শীৰ্ষ.p = L->স্তূপ.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->শীৰ্ষ.p - L->স্তূপ.p), 0);
  ঘূৰাই status;
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


LUA_API পূর্ণাঙ্ক lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->শীৰ্ষ.p == L->স্তূপ.p + (status == LUA_OK ? 1 : 2));
  যদি (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  ঘূৰাই APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, শূন্য *ud, unsigned seed) {
  পূর্ণাঙ্ক i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, রিক্ত, LUA_TTHREAD, পরিমাণ(global_State)));
  যদি (g == রিক্ত) ঘূৰাই রিক্ত;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = রিক্ত;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = রিক্ত;
  g->ud_warn = রিক্ত;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = রিক্ত;
  setnilvalue(&g->l_registry);
  g->panic = রিক্ত;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = রিক্ত;
  g->firstold1 = g->survival = g->old1 = g->reallyold = রিক্ত;
  g->finobjsur = g->finobjold1 = g->finobjrold = রিক্ত;
  g->sweepgc = রিক্ত;
  g->gray = g->grayagain = রিক্ত;
  g->weak = g->ephemeron = g->allweak = রিক্ত;
  g->twups = রিক্ত;
  g->GCtotalbytes = পরিমাণ(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  বাবে (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = রিক্ত;
  যদি (luaD_rawrunprotected(L, f_luaopen, রিক্ত) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = রিক্ত;
  }
  ঘূৰাই L;
}


LUA_API শূন্য lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


শূন্য luaE_warning (lua_State *L, ধ্রুৱ ৱর্ণ *বাৰ্তা, পূর্ণাঙ্ক tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  যদি (wf != রিক্ত)
    wf(G(L)->ud_warn, বাৰ্তা, tocont);
}


/*
** Generate a warning from an error message
*/
শূন্য luaE_warnerror (lua_State *L, ধ্রুৱ ৱর্ণ *where) {
  TValue *errobj = s2v(L->শীৰ্ষ.p - 1);  /* error object */
  ধ্রুৱ ৱর্ণ *বাৰ্তা = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, বাৰ্তা, 1);
  luaE_warning(L, ")", 0);
}

