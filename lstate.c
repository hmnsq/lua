/*
** $Id: lstate.c $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <stddef.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lgc.h"
#شامل "llex.h"
#شامل "lmem.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "ltm.h"



#define fromstate(L)	(cast(LX *, cast(lu_byte *, (L)) - offsetof(LX, l)))


/*
** these macros allow user-specific actions when a thread is
** created/deleted
*/
#اگر !defined(luai_userstateopen)
#define luai_userstateopen(L)		((تهی)L)
#endif

#اگر !defined(luai_userstateclose)
#define luai_userstateclose(L)		((تهی)L)
#endif

#اگر !defined(luai_userstatethread)
#define luai_userstatethread(L,L1)	((تهی)L)
#endif

#اگر !defined(luai_userstatefree)
#define luai_userstatefree(L,L1)	((تهی)L)
#endif


/*
** set GCdebt to a new value keeping the real number of allocated
** objects (GCtotalobjs - GCdebt) invariant and avoiding overflows in
** 'GCtotalobjs'.
*/
تهی luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = gettotalbytes(g);
  lua_assert(tb > 0);
  اگر (debt > MAX_LMEM - tb)
    debt = MAX_LMEM - tb;  /* will make GCtotalbytes == MAX_LMEM */
  g->GCtotalbytes = tb + debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L, صحیح err) {
  CallInfo *ci;
  ci = luaM_reallocvector(L, پوچ, 0, 1, CallInfo);
  اگر (l_unlikely(ci == پوچ)) {  /* allocation failed? */
    اگر (err)
      luaM_error(L);  /* raise the error */
    بازگشت پوچ;  /* else only report it */
  }
  ci->next = L->ci->next;
  ci->previous = L->ci;
  L->ci->next = ci;
  اگر (ci->next)
    ci->next->previous = ci;
  ci->u.l.trap = 0;
  L->nci++;
  بازگشت ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
ایستا تهی freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = پوچ;
  وقتی ((ci = next) != پوچ) {
    next = ci->next;
    luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread,
** keeping the first one.
*/
تهی luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;  /* first free CallInfo */
  CallInfo *next;
  اگر (ci == پوچ)
    بازگشت;  /* no extra elements */
  وقتی ((next = ci->next) != پوچ) {  /* two extra elements? */
    CallInfo *next2 = next->next;  /* next's next */
    ci->next = next2;  /* remove next from the list */
    L->nci--;
    luaM_free(L, next);  /* free next */
    اگر (next2 == پوچ)
      بشکن;  /* no more elements */
    وگرنه {
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
تهی luaE_checkcstack (lua_State *L) {
  اگر (getCcalls(L) == LUAI_MAXCCALLS)
    luaG_runerror(L, "C stack overflow");
  وگرنه اگر (getCcalls(L) >= (LUAI_MAXCCALLS / 10 * 11))
    luaD_errerr(L);  /* error while handling stack error */
}


LUAI_FUNC تهی luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  اگر (l_unlikely(getCcalls(L) >= LUAI_MAXCCALLS))
    luaE_checkcstack(L);
}


ایستا تهی resetCI (lua_State *L) {
  CallInfo *ci = L->ci = &L->base_ci;
  ci->func.p = L->پشته.p;
  setnilvalue2s(ci->func.p);  /* 'function' entry for basic 'ci' */
  ci->بالاترین.p = ci->func.p + 1 + LUA_MINSTACK;  /* +1 for 'function' entry */
  ci->u.c.k = پوچ;
  ci->callstatus = CIST_C;
  L->status = LUA_OK;
  L->errfunc = 0;  /* stack unwind can "throw away" the error function */
}


ایستا تهی پشته_init (lua_State *L1, lua_State *L) {
  صحیح i;
  /* initialize stack array */
  L1->پشته.p = luaM_newvector(L, BASIC_پشته_SIZE + EXTRA_پشته, پشته_مقدار);
  L1->tbclist.p = L1->پشته.p;
  برای (i = 0; i < BASIC_پشته_SIZE + EXTRA_پشته; i++)
    setnilvalue2s(L1->پشته.p + i);  /* erase new stack */
  L1->پشته_last.p = L1->پشته.p + BASIC_پشته_SIZE;
  /* initialize first ci */
  resetCI(L1);
  L1->بالاترین.p = L1->پشته.p + 1;  /* +1 for 'function' entry */
}


ایستا تهی freestack (lua_State *L) {
  اگر (L->پشته.p == پوچ)
    بازگشت;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  freeCI(L);
  lua_assert(L->nci == 0);
  /* free stack */
  luaM_freearray(L, L->پشته.p, cast_sizet(stacksize(L) + EXTRA_پشته));
}


/*
** Create registry table and its predefined values
*/
ایستا تهی init_registry (lua_State *L, global_State *g) {
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
ایستا تهی f_luaopen (lua_State *L, تهی *ud) {
  global_State *g = G(L);
  UNUSED(ud);
  پشته_init(L, L);  /* init stack */
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
ایستا تهی preinit_thread (lua_State *L, global_State *g) {
  G(L) = g;
  L->پشته.p = پوچ;
  L->ci = پوچ;
  L->nci = 0;
  L->twups = L;  /* thread has no upvalues */
  L->nCcalls = 0;
  L->errorJmp = پوچ;
  L->hook = پوچ;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = پوچ;
  L->status = LUA_OK;
  L->errfunc = 0;
  L->oldpc = 0;
  L->base_ci.previous = L->base_ci.next = پوچ;
}


lu_mem luaE_threadsize (lua_State *L) {
  lu_mem sz = cast(lu_mem, اندازه(LX))
            + cast_uint(L->nci) * اندازه(CallInfo);
  اگر (L->پشته.p != پوچ)
    sz += cast_uint(stacksize(L) + EXTRA_پشته) * اندازه(پشته_مقدار);
  بازگشت sz;
}


ایستا تهی close_state (lua_State *L) {
  global_State *g = G(L);
  اگر (!completestate(g))  /* closing a partially built state? */
    luaC_freeallobjects(L);  /* just collect its objects */
  وگرنه {  /* closing a fully built state */
    resetCI(L);
    luaD_closeprotected(L, 1, LUA_OK);  /* close all upvalues */
    L->بالاترین.p = L->پشته.p + 1;  /* empty the stack to run finalizers */
    luaC_freeallobjects(L);  /* collect all objects */
    luai_userstateclose(L);
  }
  luaM_freearray(L, G(L)->strt.hash, cast_sizet(G(L)->strt.size));
  freestack(L);
  lua_assert(gettotalbytes(g) == اندازه(global_State));
  (*g->frealloc)(g->ud, g, اندازه(global_State), 0);  /* free main block */
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  /* create new thread */
  o = luaC_newobjdt(L, LUA_TTHREAD, اندازه(LX), offsetof(LX, l));
  L1 = gco2th(o);
  /* anchor it on L stack */
  setthvalue2s(L, L->بالاترین.p, L1);
  api_incr_بالاترین(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  /* initialize L1 extra space */
  memcpy(lua_getextraspace(L1), lua_getextraspace(mainthread(g)),
         LUA_EXTRASPACE);
  luai_userstatethread(L, L1);
  پشته_init(L1, L);  /* init stack */
  lua_unlock(L);
  بازگشت L1;
}


تهی luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = fromstate(L1);
  luaF_closeupval(L1, L1->پشته.p);  /* close all upvalues */
  lua_assert(L1->openupval == پوچ);
  luai_userstatefree(L, L1);
  freestack(L1);
  luaM_free(L, l);
}


TStatus luaE_resetthread (lua_State *L, TStatus status) {
  resetCI(L);
  اگر (status == LUA_YIELD)
    status = LUA_OK;
  status = luaD_closeprotected(L, 1, status);
  اگر (status != LUA_OK)  /* errors? */
    luaD_seterrorobj(L, status, L->پشته.p + 1);
  وگرنه  /* keep in the stack only the fake function-entry for basic ci */
    L->بالاترین.p = L->پشته.p + 1;
  luaD_reallocstack(L, cast_int(L->ci->بالاترین.p - L->پشته.p), 0);
  بازگشت status;
}


/*
** Auxiliary function to call luaE_resetthread in protected mode.
** That function closes all tbc variables in protected mode, so the
** only possible "error" is when a tbc variable closes the thread being
** closed and therefore calls 'luaD_throwbaselevel'; in that case,
** control goes directly back to the first call to lua_closethread (not
** to the second), to properly return it.
*/
ایستا تهی resetthread_aux (lua_State *L, تهی *ud) {
  TStatus status = *cast(TStatus*, ud);
  status = luaE_resetthread(L, status);
  اگر (status != LUA_OK)
    luaD_throw(L, status);
}


LUA_API صحیح lua_closethread (lua_State *L, lua_State *from) {
  TStatus status;
  lua_lock(L);
  L->nCcalls = (from) ? getCcalls(from) : 0;
  status = luaD_rawrunprotected(L, resetthread_aux, &L->status);
  /* stack has error message iff there was an error */
  lua_assert(L->بالاترین.p == L->پشته.p + (status == LUA_OK ? 1 : 2));
  اگر (L == from)  /* closing itself? */
    luaD_throwbaselevel(L, status);
  lua_unlock(L);
  بازگشت APIstatus(status);
}


LUA_API lua_State *lua_newstate (lua_Alloc f, تهی *ud, unsigned seed) {
  صحیح i;
  lua_State *L;
  global_State *g = cast(global_State*,
                       (*f)(ud, پوچ, LUA_TTHREAD, اندازه(global_State)));
  اگر (g == پوچ) بازگشت پوچ;
  L = &g->mainth.l;
  L->tt = LUA_VTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  L->marked = luaC_white(g);
  preinit_thread(L, g);
  g->allgc = obj2gco(L);  /* by now, only object is the main thread */
  L->next = پوچ;
  incnny(L);  /* main thread is always non yieldable */
  g->frealloc = f;
  g->ud = ud;
  g->warnf = پوچ;
  g->ud_warn = پوچ;
  g->seed = seed;
  g->gcstp = GCSTPGC;  /* no GC while building state */
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = پوچ;
  setnilvalue(&g->l_registry);
  g->panic = پوچ;
  g->gcstate = GCSpause;
  g->gckind = KGC_INC;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = پوچ;
  g->firstold1 = g->survival = g->old1 = g->reallyold = پوچ;
  g->finobjsur = g->finobjold1 = g->finobjrold = پوچ;
  g->sweepgc = پوچ;
  g->gray = g->grayagain = پوچ;
  g->weak = g->ephemeron = g->allweak = پوچ;
  g->twups = پوچ;
  g->GCtotalbytes = اندازه(global_State);
  g->GCmarked = 0;
  g->GCdebt = 0;
  setivalue(&g->nilvalue, 0);  /* to signal that state is not yet built */
  setgcparam(g, PAUSE, LUAI_GCPAUSE);
  setgcparam(g, STEPMUL, LUAI_GCMUL);
  setgcparam(g, STEPSIZE, LUAI_GCSTEPSIZE);
  setgcparam(g, MINORMUL, LUAI_GENMINORMUL);
  setgcparam(g, MINORMAJOR, LUAI_MINORMAJOR);
  setgcparam(g, MAJORMINOR, LUAI_MAJORMINOR);
  برای (i=0; i < LUA_NUMTYPES; i++) g->mt[i] = پوچ;
  اگر (luaD_rawrunprotected(L, f_luaopen, پوچ) != LUA_OK) {
    /* memory allocation error: free partial state */
    close_state(L);
    L = پوچ;
  }
  بازگشت L;
}


LUA_API تهی lua_close (lua_State *L) {
  lua_lock(L);
  L = mainthread(G(L));  /* only the main thread can be closed */
  close_state(L);
}


تهی luaE_warning (lua_State *L, ثابت نویسه *پیام, صحیح tocont) {
  lua_WarnFunction wf = G(L)->warnf;
  اگر (wf != پوچ)
    wf(G(L)->ud_warn, پیام, tocont);
}


/*
** Generate a warning from an error message
*/
تهی luaE_warnerror (lua_State *L, ثابت نویسه *where) {
  TValue *errobj = s2v(L->بالاترین.p - 1);  /* error object */
  ثابت نویسه *پیام = (ttisstring(errobj))
                  ? getstr(tsvalue(errobj))
                  : "error object is not a string";
  /* produce warning "error in %s (%s)" (where, msg) */
  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, پیام, 1);
  luaE_warning(L, ")", 0);
}

