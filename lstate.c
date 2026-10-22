/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lapi.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "llex.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#எனில் !defined(luai_userstateopen)
#define luai_userstateopen(L)		((வெறுமை)L)
#endif

#எனில் !defined(luai_userstateclose)
#define luai_userstateclose(L)		((வெறுமை)L)
#endif

#எனில் !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((வெறுமை)L)
#endif

#எனில் !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((வெறுமை)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
வெறுமை luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  எனில் (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, முழு err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, வெற்று, 0, 1, CallInfo);
  எனில் (l_unlikely(ci == வெற்று)) {  /* allocation failed? */
    எனில் (err)
      luaM_error(L);  /* raise the error */
    திரும்பு வெற்று;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  எனில் (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  திரும்பு ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
நிலையான வெறுமை freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = வெற்று;
  போது ((ci = next) != வெற்று) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
வெறுமை luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  எனில் (ci == வெற்று)
    திரும்பு;  /* no extra elements */
  போது ((next = ci->next) != வெற்று) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    எனில் (next2 == வெற்று)
      நிறுத்து;  /* no more elements */
    இல்லையெனில் {
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
வெறுமை luaE_checkcstack (lua_State *L) {
  எனில் (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  இல்லையெனில் எனில் (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC வெறுமை luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  எனில் (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


நிலையான வெறுமை resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->அடுக்கு.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->உச்சி.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = வெற்று;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


நிலையான வெறுமை அடுக்கு_init (lua_State *L1, lua_State *L) {
  முழு i;
  /* initialize stack array */
  L1->அடுக்கு.p = luaM_newvector(L, BASIC_அடுக்கு_SIZE + EXTRA_அடுக்கு, அடுக்கு_மதிப்பு);
  L1->tbclist.p = L1->அடுக்கு.p;
  க்கு (i = 0; i < BASIC_அடுக்கு_SIZE + EXTRA_அடுக்கு; i++)
    setnilvalue2s(L1->அடுக்கு.p + i);  /* erase new stack */
  L1->அடுக்கு_last.p = L1->அடுக்கு.p + BASIC_அடுக்கு_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->உச்சி.p = L1->அடுக்கு.p + 1;  /* +1 for 'function' entry */
}


நிலையான வெறுமை freestack (lua_State *L) {
  எனில் (L->அடுக்கு.p == வெற்று)
    திரும்பு;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->அடுக்கு.p, cast_sizet(stacksize(L) + EXTRA_அடுக்கு));
}


/*
** Create registry table and its predefined values
*/
நிலையான வெறுமை init_registry (lua_State *L, global_State *g) {
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
நிலையான வெறுமை f_luaopen (lua_State *L, வெறுமை *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  அடுக்கு_init(L, L);  /* init stack */
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
நிலையான வெறுமை preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->அடுக்கு.p = வெற்று;
  L->ci = வெற்று;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = வெற்று;
  L->hook = வெற்று;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = வெற்று;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = வெற்று;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, அளவு(LX))
            + cast_uint(L->nci) * அளவு(CallInfo);
  எனில் (L->அடுக்கு.p != வெற்று)
    sz += cast_uint(stacksize(L) + EXTRA_அடுக்கு) * அளவு(அடுக்கு_மதிப்பு);
  திரும்பு sz;
}


நிலையான வெறுமை close_state (lua_State *L) {
  global_State *g = G(L);
  எனில் (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  இல்லையெனில் {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->உச்சி.p = L->அடுக்கு.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == அளவு(global_State));
  (*g->frealloc)(g->ud, g, அளவு(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, அளவு(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->உச்சி.p, L1);
  api_incr_உச்சி(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  அடுக்கு_init(L1, L);  /* init stack */
  lua_unlock(L);
  திரும்பு L1;
}


வெறுமை luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->அடுக்கு.p);  /* close all upvalues */
  lua_assert(L1->openupval == வெற்று);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  எனில் (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  எனில் (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->அடுக்கு.p + 1);
  இல்லையெனில்  /* keep in the stack only the fake function-entry for basic ci */
    L->உச்சி.p = L->அடுக்கு.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->உச்சி.p - L->அடுக்கு.p), 0);
  திரும்பு status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
நிலையான வெறுமை resetthread_aux (lua_State *L, வெறுமை *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  எனில் (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API முழு lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->உச்சி.p == L->அடுக்கு.p + (status == LUA_OK ? 1 : 2));
  எனில் (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  திரும்பு APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, வெறுமை *ud, unsigned seed) {
  முழு i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, வெற்று, LUA_TTHREAD, அளவு(global_State)));
  எனில் (g == வெற்று) திரும்பு வெற்று;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = வெற்று;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = வெற்று;
  g->ud_warn = வெற்று;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = வெற்று;
  setnilvalue(&g->l_registry);
  g->panic = வெற்று;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = வெற்று;
  g->firstold1 = g->survival = g->old1 = g->reallyold = வெற்று;
  g->finobjsur = g->finobjold1 = g->finobjrold = வெற்று;
  g->sweepgc = வெற்று;
  g->gray = g->grayagain = வெற்று;
  g->weak = g->ephemeron = g->allweak = வெற்று;
  g->twups = வெற்று;
  g->GCtotalbytes = அளவு(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  க்கு (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = வெற்று;
  எனில் (luaD_rawrunprotected(L, f_luaopen, வெற்று) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = வெற்று;
  }
  திரும்பு L;
}


LUA_API வெறுமை lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


வெறுமை luaE_warning (lua_State *L, மாறிலி எழுத்து *செய்தி, முழு tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  எனில் (wf != வெற்று)
    wf(G(L)->ud_warn, செய்தி, tocont);
}


/*
** Generate a warning from an error message
*/
வெறுமை luaE_warnerror (lua_State *L, மாறிலி எழுத்து *where) {
  TValue *errobj = s2v(L->உச்சி.p - 1);  /* error object */
  மாறிலி எழுத்து *செய்தி = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, செய்தி, 1);
  luaE_warning(L, ")", 0);
}

