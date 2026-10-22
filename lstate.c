/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lapi.h"
#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lfunc.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "llex.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#ଯଦି !defined(luai_userstateopen)
#define luai_userstateopen(L)		((ଶୂନ୍ୟ)L)
#endif

#ଯଦି !defined(luai_userstateclose)
#define luai_userstateclose(L)		((ଶୂନ୍ୟ)L)
#endif

#ଯଦି !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((ଶୂନ୍ୟ)L)
#endif

#ଯଦି !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((ଶୂନ୍ୟ)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
ଶୂନ୍ୟ luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  ଯଦି (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, ଖାଲି, 0, 1, CallInfo);
  ଯଦି (l_unlikely(ci == ଖାଲି)) {  /* allocation failed? */
    ଯଦି (err)
      luaM_error(L);  /* raise the error */
    ଫେରନ୍ତୁ ଖାଲି;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  ଯଦି (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  ଫେରନ୍ତୁ ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
ସ୍ଥିର ଶୂନ୍ୟ freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = ଖାଲି;
  ଯେତେବେଳେ ((ci = next) != ଖାଲି) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
ଶୂନ୍ୟ luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  ଯଦି (ci == ଖାଲି)
    ଫେରନ୍ତୁ;  /* no extra elements */
  ଯେତେବେଳେ ((next = ci->next) != ଖାଲି) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    ଯଦି (next2 == ଖାଲି)
      ଭାଙ୍ଗ;  /* no more elements */
    ଅନ୍ୟଥା {
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
ଶୂନ୍ୟ luaE_checkcstack (lua_State *L) {
  ଯଦି (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  ଅନ୍ୟଥା ଯଦି (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC ଶୂନ୍ୟ luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  ଯଦି (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


ସ୍ଥିର ଶୂନ୍ୟ resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->ଗଦା.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->ଶୀର୍ଷ.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = ଖାଲି;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


ସ୍ଥିର ଶୂନ୍ୟ ଗଦା_init (lua_State *L1, lua_State *L) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  /* initialize stack array */
  L1->ଗଦା.p = luaM_newvector(L, BASIC_ଗଦା_SIZE + EXTRA_ଗଦା, ଗଦା_ମୂଲ୍ୟ);
  L1->tbclist.p = L1->ଗଦା.p;
  ପାଇଁ (i = 0; i < BASIC_ଗଦା_SIZE + EXTRA_ଗଦା; i++)
    setnilvalue2s(L1->ଗଦା.p + i);  /* erase new stack */
  L1->ଗଦା_last.p = L1->ଗଦା.p + BASIC_ଗଦା_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->ଶୀର୍ଷ.p = L1->ଗଦା.p + 1;  /* +1 for 'function' entry */
}


ସ୍ଥିର ଶୂନ୍ୟ freestack (lua_State *L) {
  ଯଦି (L->ଗଦା.p == ଖାଲି)
    ଫେରନ୍ତୁ;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->ଗଦା.p, cast_sizet(stacksize(L) + EXTRA_ଗଦା));
}


/*
** Create registry table and its predefined values
*/
ସ୍ଥିର ଶୂନ୍ୟ init_registry (lua_State *L, global_State *g) {
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
ସ୍ଥିର ଶୂନ୍ୟ f_luaopen (lua_State *L, ଶୂନ୍ୟ *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  ଗଦା_init(L, L);  /* init stack */
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
ସ୍ଥିର ଶୂନ୍ୟ preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->ଗଦା.p = ଖାଲି;
  L->ci = ଖାଲି;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = ଖାଲି;
  L->hook = ଖାଲି;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = ଖାଲି;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = ଖାଲି;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, ଆକାର(LX))
            + cast_uint(L->nci) * ଆକାର(CallInfo);
  ଯଦି (L->ଗଦା.p != ଖାଲି)
    sz += cast_uint(stacksize(L) + EXTRA_ଗଦା) * ଆକାର(ଗଦା_ମୂଲ୍ୟ);
  ଫେରନ୍ତୁ sz;
}


ସ୍ଥିର ଶୂନ୍ୟ close_state (lua_State *L) {
  global_State *g = G(L);
  ଯଦି (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  ଅନ୍ୟଥା {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->ଶୀର୍ଷ.p = L->ଗଦା.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == ଆକାର(global_State));
  (*g->frealloc)(g->ud, g, ଆକାର(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, ଆକାର(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->ଶୀର୍ଷ.p, L1);
  api_incr_ଶୀର୍ଷ(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  ଗଦା_init(L1, L);  /* init stack */
  lua_unlock(L);
  ଫେରନ୍ତୁ L1;
}


ଶୂନ୍ୟ luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->ଗଦା.p);  /* close all upvalues */
  lua_assert(L1->openupval == ଖାଲି);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  ଯଦି (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  ଯଦି (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->ଗଦା.p + 1);
  ଅନ୍ୟଥା  /* keep in the stack only the fake function-entry for basic ci */
    L->ଶୀର୍ଷ.p = L->ଗଦା.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->ଶୀର୍ଷ.p - L->ଗଦା.p), 0);
  ଫେରନ୍ତୁ status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
ସ୍ଥିର ଶୂନ୍ୟ resetthread_aux (lua_State *L, ଶୂନ୍ୟ *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  ଯଦି (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->ଶୀର୍ଷ.p == L->ଗଦା.p + (status == LUA_OK ? 1 : 2));
  ଯଦି (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  ଫେରନ୍ତୁ APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, ଶୂନ୍ୟ *ud, unsigned seed) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, ଖାଲି, LUA_TTHREAD, ଆକାର(global_State)));
  ଯଦି (g == ଖାଲି) ଫେରନ୍ତୁ ଖାଲି;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = ଖାଲି;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = ଖାଲି;
  g->ud_warn = ଖାଲି;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = ଖାଲି;
  setnilvalue(&g->l_registry);
  g->panic = ଖାଲି;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = ଖାଲି;
  g->firstold1 = g->survival = g->old1 = g->reallyold = ଖାଲି;
  g->finobjsur = g->finobjold1 = g->finobjrold = ଖାଲି;
  g->sweepgc = ଖାଲି;
  g->gray = g->grayagain = ଖାଲି;
  g->weak = g->ephemeron = g->allweak = ଖାଲି;
  g->twups = ଖାଲି;
  g->GCtotalbytes = ଆକାର(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  ପାଇଁ (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = ଖାଲି;
  ଯଦି (luaD_rawrunprotected(L, f_luaopen, ଖାଲି) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = ଖାଲି;
  }
  ଫେରନ୍ତୁ L;
}


LUA_API ଶୂନ୍ୟ lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


ଶୂନ୍ୟ luaE_warning (lua_State *L, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  ଯଦି (wf != ଖାଲି)
    wf(G(L)->ud_warn, ସନ୍ଦେଶ, tocont);
}


/*
** Generate a warning from an error message
*/
ଶୂନ୍ୟ luaE_warnerror (lua_State *L, ଅଚଳ ଅକ୍ଷର *where) {
  TValue *errobj = s2v(L->ଶୀର୍ଷ.p - 1);  /* error object */
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, ସନ୍ଦେଶ, 1);
  luaE_warning(L, ")", 0);
}

