/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lapi.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lfunc.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "llex.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#જો !defined(luai_userstateopen)
#define luai_userstateopen(L)		((શૂન્ય)L)
#endif

#જો !defined(luai_userstateclose)
#define luai_userstateclose(L)		((શૂન્ય)L)
#endif

#જો !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((શૂન્ય)L)
#endif

#જો !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((શૂન્ય)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
શૂન્ય luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  જો (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, પૂર્ણાંક err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, ખાલી, 0, 1, CallInfo);
  જો (l_unlikely(ci == ખાલી)) {  /* allocation failed? */
    જો (err)
      luaM_error(L);  /* raise the error */
    પાછા ખાલી;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  જો (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  પાછા ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
સ્થિર શૂન્ય freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = ખાલી;
  જ્યારે ((ci = next) != ખાલી) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
શૂન્ય luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  જો (ci == ખાલી)
    પાછા;  /* no extra elements */
  જ્યારે ((next = ci->next) != ખાલી) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    જો (next2 == ખાલી)
      તોડો;  /* no more elements */
    નહીંતર {
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
શૂન્ય luaE_checkcstack (lua_State *L) {
  જો (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  નહીંતર જો (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC શૂન્ય luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  જો (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


સ્થિર શૂન્ય resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->ઢગલો.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->ટોચ.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = ખાલી;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


સ્થિર શૂન્ય ઢગલો_init (lua_State *L1, lua_State *L) {
  પૂર્ણાંક i;
  /* initialize stack array */
  L1->ઢગલો.p = luaM_newvector(L, BASIC_ઢગલો_SIZE + EXTRA_ઢગલો, ઢગલો_કિંમત);
  L1->tbclist.p = L1->ઢગલો.p;
  માટે (i = 0; i < BASIC_ઢગલો_SIZE + EXTRA_ઢગલો; i++)
    setnilvalue2s(L1->ઢગલો.p + i);  /* erase new stack */
  L1->ઢગલો_last.p = L1->ઢગલો.p + BASIC_ઢગલો_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->ટોચ.p = L1->ઢગલો.p + 1;  /* +1 for 'function' entry */
}


સ્થિર શૂન્ય freestack (lua_State *L) {
  જો (L->ઢગલો.p == ખાલી)
    પાછા;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->ઢગલો.p, cast_sizet(stacksize(L) + EXTRA_ઢગલો));
}


/*
** Create registry table and its predefined values
*/
સ્થિર શૂન્ય init_registry (lua_State *L, global_State *g) {
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
સ્થિર શૂન્ય f_luaopen (lua_State *L, શૂન્ય *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  ઢગલો_init(L, L);  /* init stack */
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
સ્થિર શૂન્ય preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->ઢગલો.p = ખાલી;
  L->ci = ખાલી;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = ખાલી;
  L->hook = ખાલી;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = ખાલી;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = ખાલી;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, કદ(LX))
            + cast_uint(L->nci) * કદ(CallInfo);
  જો (L->ઢગલો.p != ખાલી)
    sz += cast_uint(stacksize(L) + EXTRA_ઢગલો) * કદ(ઢગલો_કિંમત);
  પાછા sz;
}


સ્થિર શૂન્ય close_state (lua_State *L) {
  global_State *g = G(L);
  જો (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  નહીંતર {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->ટોચ.p = L->ઢગલો.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == કદ(global_State));
  (*g->frealloc)(g->ud, g, કદ(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, કદ(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->ટોચ.p, L1);
  api_incr_ટોચ(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  ઢગલો_init(L1, L);  /* init stack */
  lua_unlock(L);
  પાછા L1;
}


શૂન્ય luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->ઢગલો.p);  /* close all upvalues */
  lua_assert(L1->openupval == ખાલી);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  જો (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  જો (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->ઢગલો.p + 1);
  નહીંતર  /* keep in the stack only the fake function-entry for basic ci */
    L->ટોચ.p = L->ઢગલો.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->ટોચ.p - L->ઢગલો.p), 0);
  પાછા status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
સ્થિર શૂન્ય resetthread_aux (lua_State *L, શૂન્ય *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  જો (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API પૂર્ણાંક lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->ટોચ.p == L->ઢગલો.p + (status == LUA_OK ? 1 : 2));
  જો (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  પાછા APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, શૂન્ય *ud, unsigned seed) {
  પૂર્ણાંક i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, ખાલી, LUA_TTHREAD, કદ(global_State)));
  જો (g == ખાલી) પાછા ખાલી;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = ખાલી;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = ખાલી;
  g->ud_warn = ખાલી;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = ખાલી;
  setnilvalue(&g->l_registry);
  g->panic = ખાલી;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = ખાલી;
  g->firstold1 = g->survival = g->old1 = g->reallyold = ખાલી;
  g->finobjsur = g->finobjold1 = g->finobjrold = ખાલી;
  g->sweepgc = ખાલી;
  g->gray = g->grayagain = ખાલી;
  g->weak = g->ephemeron = g->allweak = ખાલી;
  g->twups = ખાલી;
  g->GCtotalbytes = કદ(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  માટે (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = ખાલી;
  જો (luaD_rawrunprotected(L, f_luaopen, ખાલી) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = ખાલી;
  }
  પાછા L;
}


LUA_API શૂન્ય lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


શૂન્ય luaE_warning (lua_State *L, અચલ અક્ષર *સંદેશ, પૂર્ણાંક tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  જો (wf != ખાલી)
    wf(G(L)->ud_warn, સંદેશ, tocont);
}


/*
** Generate a warning from an error message
*/
શૂન્ય luaE_warnerror (lua_State *L, અચલ અક્ષર *where) {
  TValue *errobj = s2v(L->ટોચ.p - 1);  /* error object */
  અચલ અક્ષર *સંદેશ = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, સંદેશ, 1);
  luaE_warning(L, ")", 0);
}

